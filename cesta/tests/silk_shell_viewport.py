"""Actual Rust viewport modules against shared typed Cesta delivery and geometry."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import re
import subprocess
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent
ROOT=HERE.parent
FIX=HERE/'tests/silk_shell_viewport'
MASK=(1<<64)-1
FULL=MASK-1


def run(command,data=None):
    result=subprocess.run(list(map(str,command)),cwd=HERE,input=data,capture_output=True,
                          env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                               'UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})
    assert result.returncode==0,(command,result.returncode,result.stderr[-5000:])
    return result.stdout


def prepare(work):
    inventory=json.loads((HERE/'interfaces/silk_shell_viewport.source.json').read_text())
    source={name:(ROOT/name).read_bytes()for name in inventory['sources']}
    assert all(hashlib.sha256(data).hexdigest()==inventory['sources'][name]for name,data in source.items())
    introspection='''
impl Delivery {
 pub fn force_sequence(&mut self,n:u64){self.sequence=n;}
 pub fn trace_state(&self){
  let n=self.desired.unwrap_or(Notice{target:0,task:0,w:0,h:0,sequence:0});let(w,h)=self.last.unwrap_or((0,0));
  println!("D {} {} {} {} {} {} {} {} {} {} {}",self.sequence,self.dead as u8,self.desired.is_some()as u8,self.last.is_some()as u8,n.target,n.task,n.w,n.h,n.sequence,w,h);
 }
}
'''
    refresh='''
impl WorkspaceRefresh {
 pub fn trace_state(&self){println!("W {} {} {} {}",self.target,self.task,self.pending as u8,self.closed as u8);}
}
'''
    for module in ('document_view','spindle_view'):
        original=source['crates/sex-pdx/src/'+module+'.rs']
        (work/(module+'.rs')).write_bytes(original+introspection.encode()+(refresh.encode()if module=='document_view'else b''))
        assert (work/(module+'.rs')).read_bytes().startswith(original)
    prefix='''#![allow(dead_code)]
extern crate self as sex_pdx;
pub const ERR_SERVICE_NOT_READY:u64=0xfffffffffffffffe;
#[path="document_view.rs"]mod document_view;
#[path="spindle_view.rs"]mod spindle_view;
'''
    for module,path in [('app_catalog','crates/sex-pdx/src/app_catalog.rs'),('desktop_style','crates/sex-pdx/src/desktop_style.rs'),('desktop_content','servers/silk-shell/src/desktop_content.rs')]:
        prefix+='#[path='+json.dumps(str(ROOT/path))+']mod '+module+';\n'
    (work/'oracle.rs').write_text(prefix+(FIX/'provider.rs').read_text())
    run(['rustc','--edition=2021','-O',work/'oracle.rs','-o',work/'rust'])
    return inventory,source


def corpus():
    rows=[]
    for text in re.findall(r'\(Row ([^\n)]+)\)',(HERE/'interfaces/shell_apps.data').read_text().split('(Table BootLifecycle')[0]):
        rows.append([1 if v=='true'else 0 if v=='false'else int(v)for v in text.split()])
    commands=[]
    def add(*v):commands.append(' '.join(str(n&MASK)for n in v))
    for row in rows:
        app,ordinal,target,surface,frame,slot,close,has_activate,activate,route,n,*children=row
        kind=1 if app==1 else 0
        for status in (0,FULL,1,MASK):
            for live in (0,1):
                for ready in (0,1):
                    add(100,kind,target,target+1000)
                    add(0,target,target+1000,300,200);add(2);add(4,live,ready,route,status);add(2)
                    add(0,target,target+1000,300,200);add(0,target,target+1000,301,201)
                    add(4,1,1,route,FULL);add(1);add(2);add(4,1,1,route,0)
                    add(11);add(12);add(14,route,FULL);add(14,route,0);add(13);add(11);add(12)
        for kind in (0,1):
            for w,h in ((0,0),(1,1),(4096,4096),(4097,1),(1,4097),(MASK,MASK)):
                add(100,kind,target,target+1000);add(0,target,target+1000,w,h);add(4,1,1,route,0)
            add(100,kind,target,target+1000);add(101,MASK-1)
            add(0,target,target+1000,300,200);add(4,1,1,route,0);add(1);add(0,target,target+1000,300,201);add(2)
        add(100,1 if app==1 else 0)
        for caller in (0,3,target):
            for task in (target,target+1000):
                a=(task<<32)|target;b=(200<<32)|300
                for sequence,last in ((0,0),(1,0),(1,1),(MASK,0),(MASK,MASK)):
                    add(5,caller,target,task,a,b,sequence,last)
    widths=[0,1,7,15,16,17,31,63,127,128,255,256,4096,0xffffffff]
    heights=[0,1,11,12,35,36,43,44,45,79,80,128,4096,0xffffffff]
    for w in widths:
        for h in heights:
            add(15,w,h)
            for target in (0,9,12,15,16,17,30,40):
                for x,y in ((0,0),(8,12),(8,36),(8,48),(56,57),(104,90),(255,255),(0xffffffff,0xffffffff)):
                    add(16,target,w,h,x,y)
    for row in rows:
        frame=row[4]or row[3]
        for w in (0,15,16,17,128,256,4096,0xffffffff):
            for h in (0,43,44,45,80,128,4096,0xffffffff):
                for x,y in ((-2147483648,-2147483648),(-1,-1),(0,0),(2147483640,2147483640),(2147483647,2147483647)):
                    add(17,frame,x,y,w,h)
    for sid in range(401):add(18,sid)
    for seed in (53,71,317):
        rng=random.Random(seed)
        for _ in range(300):
            row=rng.choice(rows);target=row[2];kind=1 if row[0]==1 else 0;route=row[9]
            add(100,kind,target,target+1000)
            for _ in range(16):
                op=rng.randrange(5)
                if op==0:add(0,rng.choice((0,target,9,12,15)),rng.choice((0,target,target+1000)),rng.randrange(4200),rng.randrange(4200))
                elif op==4:add(4,rng.randrange(2),rng.randrange(2),route,rng.choice((0,0,FULL,1,MASK)))
                else:add(op)
    return ('\n'.join(commands)+'\n').encode(),len(commands)


def observations(binary,commands):
    output=run([binary], ('\n'.join(commands)+'\n').encode()).decode().splitlines()
    return [line for line in output if line.startswith(('R ','SEND '))],output


def independent(binary,rust=False):
    if rust:
        got,_=observations(binary,['100 1','5 3 12 0 12 343597383800 1 0'])
        assert got==['R 1 12 0 120 80 1'],got
        invalid=subprocess.run([binary],input=b'19 0\n',capture_output=True)
        assert invalid.returncode!=0 and b'divide by zero'in invalid.stderr,invalid.stderr
        return 2
    cases=[
        ('full_preserves_exact_notice',['100 0','0 9 71 300 200',f'4 1 1 11 {FULL}','4 1 1 11 0','4 1 1 11 0'],
         ['SEND 11 626 304942678025 858993459500 1']*2),
        ('same_size_after_admission',['100 0','0 9 71 300 200','4 1 1 11 0','0 9 71 300 200','2','1','0 9 71 300 200','2'],
         ['SEND 11 626 304942678025 858993459500 1','R 0','R 1']),
        ('not_ready_has_no_send',['100 1','0 12 71 300 200','4 1 0 14 0','2'],['R 1']),
        ('terminal_never_requeues',['100 1','0 12 71 300 200','4 0 1 14 0','0 12 71 400 300','2'],['R 0']),
        ('sequence_exhaustion_retires',['100 0',f'101 {MASK}','0 9 71 300 200','2','1','0 9 71 400 300','2'],['R 0','R 0']),
        ('zero_task_refused',['100 1','5 3 12 0 12 343597383800 1 0'],['R 0']),
        ('layout_scale_poison',['19 0','19 4294967295'],['R 0','R 0']),
        ('captured_workspace_task',['100 0 15 71','11',f'14 43 {FULL}','14 43 0','12'],['SEND 43 633 15 71 0','SEND 43 633 15 71 0','R 0']),
    ]
    for name,commands,expected in cases:
        observed,output=observations(binary,commands);assert observed==expected,(name,observed,expected)
        if name=='full_preserves_exact_notice':
            states=[line for line in output if line.startswith('D ')];assert states[1]==states[2],name
    return len(cases)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-viewport-tests');parser.add_argument('--mutations',action='store_true');args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True);inventory,sources=prepare(work)
    data,count=corpus();(work/'commands.txt').write_bytes(data);expected=run([work/'rust'],data);(work/'rust.out').write_bytes(expected)
    independent(work/'rust',True);include='-I'+str(HERE/'generated/include');exported=work/'viewport.c';custody=work/'custody.c'
    for source,target in [('silk_shell_viewport',exported),('silk_shell_custody',custody)]:
        run([args.cesta,'export-c','src/'+source+'.cesta','--std=c11','-o',target,'--',include])
    configurations=[]
    for compiler in ('gcc','clang'):
        for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=work/(compiler+'-'+mode)
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,exported,custody,FIX/'provider.c','-o',binary])
            observed=run([binary],data)
            if observed!=expected:
                (work/(compiler+'-'+mode+'.mismatch')).write_bytes(observed)
                a,b=expected.splitlines(),observed.splitlines();first=next((i for i,(x,y)in enumerate(zip(a,b))if x!=y),min(len(a),len(b)))
                raise AssertionError((compiler,mode,'trace differs',first,a[first:first+2],b[first:first+2]))
            controls=independent(binary);configurations.append(dict(compiler=compiler,mode=mode,sha256=hashlib.sha256(observed).hexdigest()))
    mutants=[]
    if args.mutations:
        body=exported.read_text()
        changes=[
            ('full_drops_notice','reply.status!=SEXOS_ERR_SERVICE_NOT_READY','reply.status!=0'),
            ('requeue_after_death','delivery->dead || !shell_viewport_target','false || !shell_viewport_target'),
            ('no_ready_barrier','if(!ready)return;','if(false)return;'),
            ('admission_keeps_notice','delivery->desired=false;','delivery->desired=true;'),
            ('sequence_reused','++delivery->sequence;','delivery->sequence+=0;'),
            ('no_refresh','delivery->last=false;','delivery->last=true;'),
            ('wrong_task_wire','((uint64_t)n.task<<32)|n.target','((uint64_t)n.target<<32)|n.target'),
            ('wrong_child_origin','view_sat_add(x,8)','view_sat_add(x,9)'),
            ('tall_view_unbounded','return rows<limit?rows:limit;','return rows;'),
        ]
        for name,old,new in changes:
            assert old in LayoutText(body),(name,old)
            target=work/(name+'.c');target.write_text(LayoutText(body).replace(old,new));binary=work/(name+'.bin')
            run(['gcc','-std=c11','-O2',include,target,custody,FIX/'provider.c','-o',binary])
            result=subprocess.run([binary],input=data,capture_output=True)
            assert result.returncode!=0 or result.stdout!=expected,(name,'mutant survived')
            mutants.append(name)
    assert all((ROOT/name).read_bytes()==data for name,data in sources.items()),'source changed during qualification'
    summary=dict(pin=inventory['pin'],commands=count,configurations=configurations,independent_controls=controls,original_counterexamples=2,mutants=mutants,
                 scope=inventory['scope'],source_sha256=hashlib.sha256((HERE/'src/silk_shell_viewport.cesta').read_bytes()).hexdigest())
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('viewport:',count,'commands/config;',controls,'independent controls;',len(mutants),'mutants caught')


if __name__=='__main__':main()
