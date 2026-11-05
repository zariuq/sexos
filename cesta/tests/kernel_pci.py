#!/usr/bin/env python3
"""Compare real PCI adapters and complete config-port effects with upstream Rust."""
import argparse,hashlib,json,random,re,subprocess,sys,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent
U64=(1<<64)-1

def run(argv,data=None):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
    if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-1200:],p.stderr[-5000:]))
    return p.stdout

def base(op=2):
    v=[0]*48;v[0:10]=[op,13,19,0,0,0,0,1,1,1];v[10:22]=[13,0,3,0,0x8086,0x100e,0,3,0,0x8086,0x100e,6]
    v[22:31]=[1,0x10000+13*1024+19,0x400000000000,65536,7,0,0,0,0]
    v[31:42]=[1,1,1,2,0x400000010000,256*1024,15,1,0x100000,0,0]
    v[42:48]=[0x100e8086,0xf9000401,0,1,13,0]
    return v

def vectors(seed,count):
    rows=[];controls=[];exhausted=[]
    def add(v,denied=False):
        if denied:controls.append(len(rows))
        rows.append(v)
    for op in range(4):
        for a in (0,2,4,6,1,8,65535,65536,(1<<32)|6,U64):
            for b in (0,2,4,6,1,8,65535,65536,(1<<32)|6,U64):
                v=base(op);v[3:5]=[a,b];add(v)
        for field in range(7,47):
            for value in (0,1,2,6,7,8,13,19,31,32,255,256,65535,65536,1<<32,U64):
                v=base(op);v[field]=value;add(v)
    for op in (1,2,3):
        for field in (2,3,4,5,6):
            for bit in range(64):
                if op==2 and field in (3,4) and bit in (1,2):continue
                v=base(op);v[field]|=1<<bit
                # Only selector's new high bits are categorically forbidden;
                # operations1/3 reject every nonzero reserved input.
                denied=(field==2 and bit>=11) or(field!=2 and (op!=2 or field in (5,6) or bit>=16))
                add(v,denied)
    for fault in range(3):
        for old in (0,1,2,4,6,7,0xffff,0xf9000000,0xf9000007):
            for set_bits,clear_bits in ((0,0),(2,0),(4,0),(6,0),(0,2),(0,4),(0,6)):
                v=base();v[3:5]=[set_bits,clear_bits];v[43]=old;v[44]=fault;add(v)
    for caller in (1,13,31,1023):
        v=base();v[1]=v[10]=v[46]=caller;v[23]=0x10000+caller*1024+19;v[3]=6;add(v)
    # A missing/invalid DMA grant blocks enable, but must not block query/disable.
    for held in (0,1):
        for kind in (0,1):
            for set_bits,clear_bits in ((0,0),(0,4),(2,0),(4,0)):
                v=base();v[31:33]=[held,kind];v[3:5]=[set_bits,clear_bits];add(v,set_bits==4 and (not held or not kind))
    for set_bits in (4,6):
        for field in range(31,42):
            for value in (0,1,2,3,7,8,15,4096,65536,262143,262144,262145,1<<47,U64):
                v=base();v[3]=set_bits;v[field]=value;add(v)
    rng=random.Random(seed)
    for _ in range(count):
        v=base(rng.randrange(4));v[rng.randrange(2,47)]=rng.getrandbits(64);add(v)
    v=base(2);v[3]=6;v[47]=1;exhausted.append(len(rows));add(v)
    return rows,controls,exhausted

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path)
    ap.add_argument('--seed',type=int,default=20261031);ap.add_argument('--cases',type=int,default=5000);ap.add_argument('--mutations',action='store_true')
    ap.add_argument('--lean-root',type=Path);ap.add_argument('--cesta-source',type=Path);args=ap.parse_args()
    w=args.work or Path(tempfile.mkdtemp(prefix='kernel-pci-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/pci_policy.source.json').read_text())
    for entry in inventory['sources']:assert hashlib.sha256((HERE.parent/entry['path']).read_bytes()).hexdigest()==entry['sha256'],entry['path']
    actual={'pci':'crates/sex-pdx/src/pci_command.rs','region':'crates/sex-pdx/src/memory_region.rs','network':'crates/sex-net-config/src/lib.rs',
        'slot':'kernel/src/capability_slot.rs','kernel_pci':'kernel/src/pci_command.rs','boot_network_state':'kernel/src/boot_network_state.rs'}
    for name,path in actual.items():(w/('actual_'+name+'.rs')).write_bytes((HERE.parent/path).read_bytes())
    hal=(HERE.parent/'kernel/src/hal/pci.rs').read_text();start=hal.index('pub(crate) fn apply_command(');body=hal.index('{',start);depth=1;i=body+1
    while depth:
        depth+=(hal[i]=='{')-(hal[i]=='}');i+=1
    (w/'actual_apply.rs').write_text(hal[start:i]+'\n')
    (w/'oracle.rs').write_bytes((HERE/'tests/kernel_pci/oracle.rs').read_bytes())
    run(['rustc','--edition=2021','--cap-lints','allow','-O',w/'oracle.rs','-o',w/'oracle'])
    rows,controls,exhausted=vectors(args.seed,args.cases);data=''.join(' '.join(map(str,v))+'\n' for v in rows).encode();(w/'commands.txt').write_bytes(data)
    expected=run([w/'oracle'],data);lines=expected.splitlines()
    for i in exhausted:lines[i]=f'{U64-2} 0 0 0 0 {rows[i][43]&0xffffffff}'.encode()
    expected=b'\n'.join(lines)+b'\n';(w/'rust.out').write_bytes(expected)
    for i in controls:assert list(map(int,lines[i].split(b'|')[0].split()))[:5]==[U64,0,0,0,0] and b'|' not in lines[i],(i,rows[i],lines[i])
    units=('kernel_domains','kernel_pci','pci_policy','network_config','memory_regions')
    for name in units:
        run([args.cesta,'export-c','src/'+name+'.cesta','--std=c11','--plan',w/(name+'.plan'),'-o',w/(name+'.c'),'--','-Igenerated/include','-Iruntime/kernel'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-DSEXOS_KERNEL_CURRENT_PDX=1','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
        '-Itests/kernel_pci','-Igenerated/include','-I'+str(w),'-Iruntime/kernel']
    def build(cc,extra,name):
        binary=w/name;run([cc,*flags,*extra,'tests/kernel_pci/host.c',w/'pci_policy.c',w/'network_config.c',w/'memory_regions.c','-o',binary]);return binary
    def check(binary):
        out=run([binary],data)
        if out!=expected:
            (w/(binary.name+'.out')).write_bytes(out);got=out.splitlines();i=next((i for i,p in enumerate(zip(got,lines)) if p[0]!=p[1]),min(len(got),len(lines)));raise AssertionError((binary.name,i,rows[i],got[i:i+1],lines[i:i+1]))
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            binary=build(cc,extra,cc+'-'+mode);check(binary);builds.append({'compiler':cc,'mode':mode,'requests':len(rows)})
        print(cc,len(rows),'complete PCI replies/config effects match actual Rust',flush=True)
    changes=[('selector_alias','kernel_pci','slot=f->rdi','slot=(uint32_t)f->rdi'),
        ('wrong_cap_kind','kernel_pci','c->kind!=CAP_PCI','c->kind==CAP_NONE'),
        ('unheld_command','pci_policy','!g.present ||','false ||'),
        ('wrong_subject','pci_policy','g.subject!=caller','false'),
        ('narrow_before_authorization','pci_policy','if(!g.present ||','set=(uint16_t)set;clear=(uint16_t)clear;\n    if(!g.present ||'),
        ('overlapping_bits','pci_policy','(set&clear)','false'),
        ('mmio_rights','pci_policy','d.bar0.grant.rights!=(REGION_QUERY|REGION_READ|REGION_WRITE)','false'),
        ('mmio_identity','pci_policy','expected!=d.bar0.grant.region_id','false'),
        ('skip_dma','kernel_pci','if(pci_policy_needs_dma(update))','if(false)'),
        ('dma_size','pci_policy','c.grant.length!=PCI_DMA_BYTES','false'),
        ('wide_command_write','kernel_pci','cpu_outw(0xcfc,value)','cpu_outl(0xcfc,value)'),
        ('skip_device_identity','pci_policy','if(identity!=','if(false && identity!='),
        ('skip_readback','pci_policy','if(after!=expected)','if(false)'),
        ('config_ambient_owner','kernel_pci','kernel.network_config_owner!=d->id','false'),
        ('pci_epoch_event_omitted','kernel_pci','kernel_epoch_record_pci(c->epoch,','if(false)kernel_epoch_record_pci(c->epoch,'),
        ('pci_epoch_wrong_subject','kernel_pci','c->epoch,c->subject->id,c->slot','c->epoch,1u,c->slot'),
        ('pci_epoch_wrong_device','kernel_domains','((uint32_t)grant.device<<8)','((uint32_t)grant.device<<7)'),
        ('pci_epoch_loses_identity','kernel_domains','.device_identity=trace.identity','.device_identity=0'),
        ('pci_epoch_wrong_expected_identity','kernel_domains','.device_expected_identity=expected_identity','.device_expected_identity=(expected_identity^1u)'),
        ('pci_epoch_wrong_status','kernel_domains','.device_status=trace.reply.status','.device_status=PCI_DENIED'),
        ('pci_epoch_loses_before','kernel_domains','.device_before=trace.before','.device_before=0'),
        ('pci_epoch_loses_attempt','kernel_domains','.device_attempted=trace.attempted','.device_attempted=false'),
        ('pci_epoch_ignores_exhaustion','kernel_pci','if(!kernel_epoch_current(kernel.epoch)){r.status=PCI_EXHAUSTED;goto pci_done;}','if(false){r.status=PCI_EXHAUSTED;goto pci_done;}')]
    caught=[]
    if args.mutations:
        for name,unit,before,after in changes:
            path=w/(unit+'.c');original=LayoutText(path.read_text());assert before in original,(name,'anchor')
            try:
                path.write_text(original.replace(before,after));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,input=data,capture_output=True)
                assert result.returncode or result.stdout!=expected,('survived',name);caught.append(name)
            finally:path.write_text(original)
    admission=None
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse((w/'pci_policy.plan').read_bytes());admission=ownership_admission.admit_record(record,w/'PciPolicyOwnership.lean',args.lean_root)
    names=['src/'+n+'.cesta' for n in units]+['src/pci_policy_interface.cesta','src/kernel_interface.cesta','interfaces/kernel_pci_command_protocol.module','tests/kernel_pci.py','tests/kernel_pci/host.c','tests/kernel_pci/oracle.rs','tests/kernel_pci/cpu_x86_64.h']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'independent_denials':len(controls),'epoch_exhaustion_controls':len(exhausted),'mutations':caught,'admission':admission,'actual_apply_sha256':hashlib.sha256(hal[start:i].encode()).hexdigest(),
        'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'rust_sha256':{path:hashlib.sha256((HERE.parent/path).read_bytes()).hexdigest() for path in [*actual.values(),'kernel/src/hal/pci.rs']},
        'scope':'actual production Cesta adapter plus upstream Rust kernel/policy/apply bodies; complete register replies and ordered config-port effects. Scripted hardware; BSP serialization,real DMA and device-present CPU boots remain separate.'},indent=2)+'\n')
if __name__=='__main__':main()
