#!/usr/bin/env python3
"""Execute native adapters with only their syscall instruction intercepted."""
import argparse,hashlib,json,re,subprocess,tempfile,tomllib
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
U32=(1<<32)-1;U64=(1<<64)-1

def run(args,data=None):
    p=subprocess.run(list(map(str,args)),cwd=HERE,input=data,capture_output=True)
    assert p.returncode==0,(args,p.returncode,p.stdout[-1000:],p.stderr[-5000:])
    return p.stdout

def cases(profile):
    result=[]
    for kind in range(11 if profile else 10):
        raw=[(0,901,15,8192,10,6),(0,0,0,0,0,0),(0,1,U32,U64,U32,255),
             (0,U64,U64,U64,U64,U64),(0,901,15,8192,1<<32,6),
             (0,901,15,8192,10,256),(0,901,1<<32,8192,10,6),
             (1,901,15,8192,10,6),(2,901,15,8192,10,6),(U64,901,15,8192,10,6)]
        if kind in (8,9):raw += [(0x103,901,15,0x800019,6,77),(0x104,1<<32,15,1,2,3)]
        for outputs in raw:result.append((kind,36,0x123456789abcdef0,0x23456789abcdef01,0xff,0,*outputs))
    return result

def expected(v,profile,ids):
    k,a,b,c,d,_,status,task,rsi,rdx,r10,r8=v
    values=[0]*6
    inputs={0:[ids['info'],a],1:[ids['query'],a,b],2:[ids['commit'],a,ids['pool'],b,c,d&255],
            3:[ids['reject'],a,b],4:[ids['reclaim'],a,ids['pool'],b,c],5:[ids['cleanup'],a,ids['pool']],
            6:[ids['catalog'],a,1],7:[ids['retire'],a],8:[ids['listen'],a],9:[ids['listen'],a],10:[ids['wait'],0]}[k]
    if k in (0,1):
        values[0]=status or (1 if r10>U32 or r8>255 else 0)
        if not values[0]:values[1:5]=[rsi,rdx,r10,r8]
    elif k in (2,3,4):values[0]=status
    elif k==5:
        values[0]=status
        if not status:values[1:3]=[rsi,rdx]
    elif k==6:
        values[0]=status or (1 if rsi>U32 or rdx>U32 or r8>U32 else 0)
        if not values[0]:values[1:5]=[rsi,rdx,r10,r8]
    elif k==7:
        values[0]=status or (1 if r10>U32 else 0)
        if not values[0]:values[1:4]=[rsi,rdx,r10]
    elif k==8:values=[status,rdx,r10,r8,rsi&U32,task if profile and task<=U32 else 0]
    elif k==9:
        values=[int(status!=0),status,rdx,r10,rsi&U32,task if profile and task<=U32 else 0] if status else [0,U64,91,92,U32,U32]
    return values,[inputs]+([[32]] if k==9 and not status else [])

def check(binary,profile,ids):
    vectors=cases(profile);data=''.join(' '.join(map(str,v))+'\n' for v in vectors).encode()
    output=run([binary],data=data).decode().splitlines()
    assert len(output)==len(vectors)
    for v,line in zip(vectors,output):
        words=line.split();assert words[0]=='out' and words[7]=='traps',line
        values,inputs=expected(v,profile,ids)
        assert list(map(int,words[1:7]))==values,(v,line,values)
        count=int(words[8]);assert count==len(inputs),(v,line,inputs)
        for i,needed in enumerate(inputs):
            start=9+i*7;assert words[start]=='in'
            actual=list(map(int,words[start+1:start+7]));assert actual[:len(needed)]==needed,(v,line,needed)
    return len(vectors)

def build(work,cc,profile,override=None):
    case=work/(cc+'-'+str(profile));case.mkdir(parents=True,exist_ok=True)
    flags=['-std=c11','-O2','-g0','-ffreestanding','-fno-builtin','-mno-red-zone','-mgeneral-regs-only',
           '-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include',
           '-DSEXOS_PDX_SENDER_TASK='+str(profile)]
    sources=['runtime/sext_native.c','runtime/traps_x86_64.c']
    assembly=[];intercepts=0
    for name in sources:
        source=override.get(name,HERE/name) if override else HERE/name
        destination=case/(Path(name).stem+'.s')
        run([cc,*flags,'-S',source,'-o',destination])
        text,n=re.subn(r'(?m)^([ \t]*)syscall[ \t]*$',r'\1call sext_abi_trap',destination.read_text())
        assert n,(cc,name,'no native syscall sites found')
        intercepts+=n;destination.write_text(text);assembly.append(destination)
    binary=case/'probe'
    run([cc,*flags,HERE/'tests/sext/native.c',HERE/'tests/sext/trap.S',HERE/'runtime/server_runtime.c',
         *assembly,'-Wl,--gc-sections','-o',binary])
    return binary,intercepts

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--work',type=Path);args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='sext-native-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/sext.source.json').read_text());pin=inventory['pin']
    contract=tomllib.loads(run(['git','show',pin+':sexos_contract.toml']).decode())
    assert contract['ipc']['pdx_listen']['caller_task_reg']=='rdi'
    paging=run(['git','show',pin+':crates/sex-pdx/src/paging.rs']).decode()
    life=run(['git','show',pin+':crates/sex-pdx/src/app_lifecycle.rs']).decode()
    def constant(source,name):
        return int(re.search(r'pub const '+name+r':\s*u64\s*=\s*(\d+);',source)[1])
    ids={name:constant(paging,'SYS_PAGING_'+name.upper()) for name in ('info','query','commit','reject','reclaim','cleanup')}
    ids.update(pool=constant(paging,'SLOT_FRAME_POOL'),catalog=constant(life,'SYS_INFO'),retire=constant(life,'SYS_RETIRE'),
               listen=contract['ipc']['pdx_listen']['syscall'],wait=contract['ipc']['wait_self']['syscall'])
    rows=[]
    for cc in ('gcc','clang'):
        for profile in (0,1):
            binary,intercepts=build(work,cc,profile)
            rows.append(dict(compiler=cc,sender_task=profile,cases=check(binary,profile,ids),intercepted_sites=intercepts))
    killed=[]
    for name,filename,before,after in [
        ('pool_slot','runtime/sext_native.c','b __asm__("rsi")=SEXT_SLOT_POOL','b __asm__("rsi")=SEXT_SLOT_POOL+1'),
        ('unchecked_width','runtime/sext_native.c','if(d>UINT32_MAX || e>UINT8_MAX)return SEXT_INVALID;',''),
        ('ignored_task','runtime/traps_x86_64.c','task = rdi <= UINT32_MAX ? (uint32_t)rdi : 0;','task = 0;')]:
        original=(HERE/filename).read_text();assert before in original
        source=work/(name+'.c');source.write_text(original.replace(before,after))
        binary,_=build(work/name,'gcc',1,{filename:source})
        try:check(binary,1,ids)
        except AssertionError:killed.append(name)
        else:raise AssertionError(('native ABI mutation survived',name))
    summary=dict(pin=pin,builds=rows,mutations=killed,scope='actual compiled C adapters; syscall instruction replaced by an assembly register provider, CPU kernel-entry code remains outside this test')
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('sext native ABI:',sum(r['cases'] for r in rows),'executed adapter controls;',len(killed),'mutations caught')
if __name__=='__main__':main()

