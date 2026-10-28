#!/usr/bin/env python3
"""Packet boundaries checked against independent byte/checksum construction."""
import argparse
from pathlib import Path
import struct
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent.parent
SOURCE=bytes((10,0,2,2));DEST=bytes((10,0,2,15))

def checksum(data):
    if len(data)%2:data+=b'\0'
    value=sum(struct.unpack('!'+str(len(data)//2)+'H',data))
    while value>>16:value=(value&65535)+(value>>16)
    return (~value)&65535

def packet(body=b'',*,ihl=5,offset=5,fragment=0,source=SOURCE,dest=DEST,sport=18081,dport=7777):
    tcp=bytearray(struct.pack('!HHIIBBHHH',sport,dport,73,99,offset<<4,0x18,8192,0,0)+b'\1'*(4*(max(offset,5)-5))+body)
    pseudo=source+dest+struct.pack('!BBH',0,6,len(tcp))
    tcp[16:18]=struct.pack('!H',checksum(pseudo+tcp))
    ip=bytearray(struct.pack('!BBHHHBBH4s4s',(4<<4)|ihl,0,ihl*4+len(tcp),0,fragment,64,6,0,source,dest)+b'\1'*(4*(ihl-5)))
    ip[10:12]=struct.pack('!H',checksum(ip))
    return b'\x52\x54\x00\x12\x34\x56'*2+b'\x08\x00'+ip+tcp

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True);args=parser.parse_args()
    work=Path(tempfile.mkdtemp(prefix='network-tcp-',dir=HERE/'build'))
    def run(command,data=None):
        result=subprocess.run(list(map(str,command)),cwd=HERE,input=data,capture_output=True)
        assert result.returncode==0,(command,result.stderr[-3000:]);return result.stdout
    for name in ('network_tcp','network_http'):
        (work/(name+'.c')).write_bytes(run([args.cesta,'export-c','src/'+name+'.cesta','--']))
    samples=[]
    for length in (0,1,17,256,1460):
        body=bytes(index%256 for index in range(length))
        for ihl in (5,6,15):
            for offset in (5,6,15):
                data=packet(body,ihl=ihl,offset=offset)
                samples.append((data,f'accepted {offset*4} {length} {body.hex()}'))
                samples.append((data+b'\0'*16,f'accepted {offset*4} {length} {body.hex()}'))
    valid=packet(b'checked bytes')
    for length in range(len(valid)):samples.append((valid[:length],'refused'))
    for changes in ({'fragment':0x2000},{'fragment':1},{'source':b'\x01'*4},{'dest':b'\x02'*4},
                    {'sport':18082},{'dport':7778},{'offset':4},{'offset':15}):
        data=bytearray(packet(b'bad',**changes))
        if changes=={'offset':15}:data=data[:54];data[16:18]=struct.pack('!H',40)
        samples.append((bytes(data),'refused'))
    for index in (12,14,24,30,34,46,50,54):
        bad=bytearray(valid);bad[index]^=1;samples.append((bytes(bad),'refused'))
    data=b''.join(value.hex().encode()+b'\n' for value,_ in samples)
    expected=[text.encode() for _,text in samples]
    for compiler in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            flags=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            binary=work/(compiler+'-'+mode)
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I',work,
                 '-Igenerated/include','tests/network_tcp/host.c',work/'network_http.c','-o',binary])
            actual=run([binary],data).splitlines()
            assert actual==expected,next(((i,a,e) for i,(a,e) in enumerate(zip(actual,expected)) if a!=e),len(actual))
            print(compiler,mode,len(samples),'packet bounds, options, addresses, ports and checksums passed')

if __name__=='__main__':main()
