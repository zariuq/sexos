#!/usr/bin/env python3
"""Compare the actual immutable network-config crate with checked Cesta."""
import argparse,hashlib,ipaddress,json,random,subprocess,sys,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent
MASK=(1<<64)-1

def run(argv,data=None):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
    if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-1500:],p.stderr[-5000:]))
    return p.stdout

def vectors(seed,count):
    base=[0,0x0a00020f,24,0x0a000202,0x0a000202,46079,0x0a000202,46078,46076,48000,1,0x0a000201,0,0,0,0]
    rows=[base,base[:]];rows[-1][0]=2
    def add(field,value):
        v=base[:];v[field]=value;rows.append(v)
    for prefix in range(256):add(2,prefix)
    for first in range(256):
        for field in (1,3,4,6,11):add(field,(first<<24)|0x00000203)
    for field in range(1,12):
        for value in (0,1,2,31,32,53,255,256,47999,48000,48001,65535,65536,1<<32,MASK):add(field,value)
    for prefix in (1,8,23,24,27,30):
        for last in (0,1,2,9,254,255):
            v=base[:];v[1:4]=[0xac100109,prefix,0xac100001];v[4]=0xac1001fe;v[6]=0xac1000ff;v[11]=0xac100000|last;rows.append(v)
    v=base[:];v[4]=0xc6336435;v[6]=0xcb007109;rows.append(v)
    for field in (3,4,6):v=base[:];v[field]=v[1];rows.append(v)
    words=[0x010001180a00020f,0x0a0002020a000202,0xbb80b3fcb3feb3ff,0x0a000202]
    for field in range(4):
        for bit in range(64):v=base[:];v[0]=1;v[12:16]=words;v[12+field]^=1<<bit;rows.append(v)
    rng=random.Random(seed)
    for _ in range(count):
        v=base[:]
        if rng.randrange(2):v[rng.randrange(1,12)]=rng.getrandbits(64)
        else:v[0]=1;v[12:16]=[rng.getrandbits(64) for _ in range(4)]
        rows.append(v)
    return rows

def independent(v):
    # Python's IP network objects independently define subnet boundaries.
    local,prefix,gw,dns,http=(v[1]&0xffffffff,v[2]&255,v[3]&0xffffffff,v[4]&0xffffffff,v[6]&0xffffffff)
    profile=v[10]&255;ports=[v[i]&65535 for i in (5,7,8,9)]
    def unicast(a):return 0<a>>24<224 and a>>24!=127
    net=ipaddress.IPv4Network((local,prefix),strict=False) if 1<=prefix<=30 else None
    def contains(a):return bool(net and unicast(local) and unicast(a) and ipaddress.IPv4Address(a) in net and local not in (int(net.network_address),int(net.broadcast_address)) and a not in (int(net.network_address),int(net.broadcast_address)))
    def endpoint(a):return bool(net and unicast(a) and a!=local and (ipaddress.IPv4Address(a) not in net or contains(a)))
    valid=bool(profile==1 and contains(local) and contains(gw) and endpoint(dns) and endpoint(http) and gw!=local and all(ports) and ports[2]<ports[3] and ports[3]==48000)
    return valid,contains(v[11]&0xffffffff)

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path)
    ap.add_argument('--seed',type=int,default=20261031);ap.add_argument('--cases',type=int,default=6000);ap.add_argument('--mutations',action='store_true')
    ap.add_argument('--lean-root',type=Path);ap.add_argument('--cesta-source',type=Path);args=ap.parse_args()
    w=args.work or Path(tempfile.mkdtemp(prefix='network-config-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/network_config.source.json').read_text())
    for entry in inventory['sources']:assert hashlib.sha256((HERE.parent/entry['path']).read_bytes()).hexdigest()==entry['sha256'],entry['path']
    actual=HERE.parent/'crates/sex-net-config/src/lib.rs';(w/'actual_network.rs').write_bytes(actual.read_bytes());(w/'oracle.rs').write_bytes((HERE/'tests/network_config/oracle.rs').read_bytes())
    run(['rustc','--edition=2021','--cap-lints','allow','-O',w/'oracle.rs','-o',w/'oracle'])
    rows=vectors(args.seed,args.cases);data=''.join(' '.join(map(str,v))+'\n' for v in rows).encode();(w/'commands.txt').write_bytes(data)
    expected=run([w/'oracle'],data);(w/'rust.out').write_bytes(expected);lines=expected.splitlines();controls=0
    for v,line in zip(rows,lines):
        if v[0]==0:
            out=list(map(int,line.split()));valid,contains=independent(v);assert out[1:3]==[int(valid),int(contains)],(v,out,valid,contains);controls+=1
    assert list(map(int,lines[1].split()))[9:13]==[0x010001180a00020f,0x0a0002020a000202,0xbb80b3fcb3feb3ff,0x0a000202]
    source=w/'network_config.c';plan=w/'network.plan';run([args.cesta,'export-c','src/network_config.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include'];builds=[]
    def check(binary):
        out=run([binary],data)
        if out!=expected:
            (w/(binary.name+'.out')).write_bytes(out);got=out.splitlines();i=next((i for i,p in enumerate(zip(got,lines)) if p[0]!=p[1]),min(len(got),len(lines)));raise AssertionError((binary.name,i,rows[i],got[i:i+1],lines[i:i+1]))
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            binary=w/(cc+'-'+mode);run([cc,*flags,*extra,'tests/network_config/host.c',source,'-o',binary]);check(binary);builds.append({'compiler':cc,'mode':mode,'requests':len(rows)})
            if mode=='O2':
                authored=w/(cc+'-authored');run([args.cesta,'cc','--cc',cc,*flags,*extra,'tests/network_config/host.c','src/network_config.cesta','-o',authored]);assert authored.read_bytes()==binary.read_bytes()
        print(cc,len(rows),'network config observations match actual Rust',flush=True)
    changes=[('reject_valid_prefix','prefix>30','prefix>24'),('allow_network_address','host && host!=~mask','host!=~mask'),('allow_loopback','first!=127','first!=126'),('fixed_prefix','32-prefix','32-24'),('require_onlink_endpoint','(address&mask)!=(c.local&mask)','false'),('allow_local_gateway','c.gateway!=c.local','true'),('unknown_profile','c.profile==1 &&','c.profile!=0 &&'),('source_port_wrap','c.source_base==NET_CONFIG_SOURCE_BASE && (uint32_t)c.source_base+NET_CONFIG_MAX_TICKET==UINT16_MAX','c.source_base!=0'),('reserved_bits','((w.word0>>48)&255)','false'),('unknown_version','w.word0>>56!=NET_CONFIG_VERSION','false'),('http_high_alias','w.word3>>32','false')];caught=[]
    if args.mutations:
        original=LayoutText(source.read_text())
        for name,before,after in changes:
            assert before in original,(name,'anchor')
            try:
                source.write_text(original.replace(before,after));binary=w/name;run(['gcc',*flags,'-O2','tests/network_config/host.c',source,'-o',binary]);assert run([binary],data)!=expected,('survived',name);caught.append(name)
            finally:source.write_text(original)
    admission=None
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'NetworkConfigOwnership.lean',args.lean_root)
    names=['src/network_config.cesta','src/network_config_interface.cesta','tests/network_config.py','tests/network_config/host.c','tests/network_config/oracle.rs']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'independent_controls':controls,'mutations':caught,'admission':admission,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'rust_sha256':hashlib.sha256(actual.read_bytes()).hexdigest(),'scope':'actual network metadata validity/codec/names/TLS endpoint; independent subnet controls. Metadata is not authority.'},indent=2)+'\n')
if __name__=='__main__':main()
