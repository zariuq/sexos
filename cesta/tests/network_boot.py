#!/usr/bin/env python3
"""A frozen image must deliver the local peer's actual status and byte content.

The image contains the native-network service and browser. Assertions observe
wire input, service publication, browser receipt and accepted compositor text
chunks separately. Ordinary boot milestones and faults remain mandatory.
"""
import argparse
import hashlib
import http.server
import json
from pathlib import Path
import re
import socket
import subprocess
import threading
import time

HERE=Path(__file__).resolve().parent.parent
CASES=('ordinary','same_a','same_b','delayed','empty','404','chunked','binary','maximum',
       'truncated','oversized','unsupported','refused')

def peer_case(name):
    body=b'Network proof alpha'
    if name=='same_b':body=b'Network proof bravo'
    if name=='empty':body=b''
    if name=='binary':body=bytes([0,10,13,32,255,65])*11
    if name=='maximum':body=bytes(range(256))
    if name=='oversized':body=b'x'*257
    return body,404 if name=='404' else 200

def compositor_bytes(text):
    result={}
    for offset,count,word,status in re.findall(
            r'\[browser\.network\.draw\] offset=(\d+) bytes=(\d+) word=0x([0-9a-fA-F]+) status=(\d+)',text):
        assert int(status)==0,'compositor rejected a text chunk'
        data=int(word,16).to_bytes(8,'little')[:int(count)]
        assert 0<len(data)<=8
        for index,byte in enumerate(data,int(offset)):
            assert index not in result,'overlapping text chunks'
            result[index]=byte
    return result

def verify(text,case,body,status,display_denied=False):
    failed=case in ('truncated','oversized','unsupported','refused')
    matches=re.findall(r'\[browser\.network\.result\] complete=(\d+) status=(\d+) bytes=(\d+)',text)
    assert len(matches)==1,'missing or repeated final browser result'
    expected=(0,0,0) if failed else (1,status,len(body))
    assert tuple(map(int,matches[0]))==expected,(matches,expected)
    draws=compositor_bytes(text)
    if not failed:
        for label in ('network.fetch.body','browser.network.body'):
            chunks=re.findall(r'^\['+re.escape(label)+r'\] offset=(\d+) bytes=(\d+) hex=([0-9a-f]*)$',text,re.M)
            assert chunks,('missing byte receipt',label)
            assembled=b''
            for offset,count,hexadecimal in chunks:
                part=bytes.fromhex(hexadecimal)
                assert int(offset)==len(assembled) and len(part)==int(count) and len(part)<=64
                assembled+=part
            assert assembled==body,('incorrect receipt',label,assembled.hex())
        if not display_denied:
            status_line=f'HTTP {status} from 10.0.2.2'.encode()
            assert bytes(draws[32+i] for i in range(len(status_line)))==status_line
            projected=bytes(byte if 32<=byte<127 else 32 for byte in body[:64])
            assert bytes(draws[64+i] for i in range(len(projected)))==projected
            assert not any(index>=64+len(projected) for index in draws),'unexpected body tail'
    else:
        assert '[browser.network.body]' not in text,'failure published a body'
        assert not any(index>=64 for index in draws),'failure rendered fixture content'
        assert '[network.fetch.result] kind=2 ' in text
    surface=re.findall(r'\[browser.network.surface\] kind=(\d+) detail=(\d+)',text)
    commit=re.findall(r'\[browser.network.commit\] kind=(\d+) detail=(\d+)',text)
    if display_denied:
        assert surface==[('2',str((1<<64)-4))] and not commit
        assert '[browser.visible.failed]' in text and '[browser.visible.polish.done]' not in text
        assert not draws,'a denied surface must not receive drawing submissions'
    else:
        assert surface==[('0','0')] and commit==[('0','0')]
    return {'complete':not failed,'status':status if not failed else 0,'bytes':len(body) if not failed else 0}

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--image',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--case',choices=CASES,default='ordinary')
    parser.add_argument('--seconds',type=float,default=180)
    parser.add_argument('--display-denied',action='store_true',help='negative control: original kernel grants no compositor capability')
    parser.add_argument('--qemu',default='qemu-system-x86_64')
    args=parser.parse_args()
    if args.seconds<=0:parser.error('--seconds must be positive')
    image=args.image.resolve();output=args.out.resolve();output.mkdir(parents=True,exist_ok=False)
    body,status=peer_case(args.case)
    hits=[]
    class Peer(http.server.BaseHTTPRequestHandler):
        protocol_version='HTTP/1.1'
        def do_GET(self):
            hits.append(self.path)
            self.send_response(status)
            self.send_header('Content-Type','text/plain')
            if args.case=='chunked':self.send_header('Transfer-Encoding','chunked')
            else:self.send_header('Content-Length',str(len(body)+(5 if args.case=='truncated' else 0)))
            if args.case=='unsupported':self.send_header('Content-Encoding','gzip')
            self.send_header('Connection','close');self.end_headers()
            if args.case=='delayed':time.sleep(0.25)
            try:
                if args.case=='chunked':
                    for start in range(0,len(body),3):
                        part=body[start:start+3]
                        self.wfile.write(f'{len(part):x}\r\n'.encode()+part+b'\r\n');self.wfile.flush()
                    self.wfile.write(b'0\r\n\r\n')
                else:self.wfile.write(body)
            except (BrokenPipeError,ConnectionResetError):pass
        def log_message(self,*_args):pass
    peer=None;reservation=None
    if args.case=='refused':
        reservation=socket.socket();reservation.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
        reservation.bind(('127.0.0.1',18081))
    else:
        peer=http.server.HTTPServer(('127.0.0.1',18081),Peer)
        threading.Thread(target=peer.serve_forever,daemon=True).start()
    markers=[line for line in (HERE/'oracle/milestones.txt').read_text().splitlines() if line]
    markers.append('[browser.visible.failed]' if args.display_denied else '[browser.visible.polish.done]')
    serial=output/'serial.log';capture=output/'wire.pcap'
    command=[args.qemu,'-M','q35','-m','512M','-cdrom',str(image),
        '-device','nec-usb-xhci,id=xhci','-device','usb-tablet,bus=xhci.0',
        '-netdev','user,id=net0','-device','e1000,netdev=net0',
        '-object','filter-dump,id=f0,netdev=net0,file='+str(capture),
        '-serial','file:'+str(serial),'-display','none','-no-reboot','-no-shutdown','-cpu','max,+pku']
    identity={'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'case':args.case,
              'peer_status':status,'peer_body_hex':body.hex(),'command':command}
    (output/'inputs.json').write_text(json.dumps(identity,indent=2)+'\n')
    process=None;started=time.monotonic()
    try:
        with (output/'qemu.log').open('xb') as log:
            process=subprocess.Popen(command,stdout=log,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+args.seconds
            while process.poll() is None and time.monotonic()<deadline:
                text=serial.read_text(errors='replace') if serial.exists() else ''
                if 'KERNEL PANIC' in text or '[cesta.client.fault]' in text:break
                if all(marker in text for marker in markers):break
                time.sleep(0.25)
    finally:
        if process is not None and process.poll() is None:
            process.terminate()
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
        if peer:peer.shutdown();peer.server_close()
        if reservation:reservation.close()
    text=serial.read_text(errors='replace')
    observation=subprocess.run(['python3','-I',HERE/'oracle/oracle.py','observe',serial],
                                capture_output=True,check=True).stdout
    (output/'observation.json').write_bytes(observation)
    faults=json.loads(observation)['faults']
    missing=[marker for marker in markers if marker not in text]
    record={'seconds':time.monotonic()-started,'peer_requests':len(hits),'faults':faults,'missing':missing}
    (output/'result.json').write_text(json.dumps(record,indent=2)+'\n')
    assert not missing,('missing ordinary milestones',missing)
    assert all(value==0 for value in faults.values()),faults
    assert args.case=='refused' or len(hits)==1,'missing or duplicated peer request'
    record.update(verify(text,args.case,body,status,args.display_denied))
    (output/'result.json').write_text(json.dumps(record,indent=2)+'\n')
    print('network boot:',args.case,'exact byte receipts;', 'explicit display denial;' if args.display_denied else 'accepted surface/text/commit;',len(markers),'milestones; no faults')

if __name__=='__main__':main()
