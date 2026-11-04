"""Current diagnostic drivers: actual Rust bodies, transport traces and one-shot state."""
import argparse,hashlib,json,os,random,re,subprocess
from pathlib import Path
from silk_shell_input_differential import declarations,identifiers,item_end

HERE=Path(__file__).resolve().parent.parent

def run(args,data=None,env=None):
    r=subprocess.run(list(map(str,args)),cwd=HERE,input=data,capture_output=True,env=env)
    assert r.returncode==0,(args,r.returncode,r.stderr[-6000:]);return r.stdout

def prepare(w,inventory):
    source=run(['git','show',inventory['pin']+':servers/silk-shell/src/main.rs']).decode();pdx=run(['git','show',inventory['pin']+':crates/sex-pdx/src/lib.rs']).decode();catalog={**declarations(pdx),**declarations(source)};functions=[];needed=set()
    for row in inventory['functions']:
        m=re.search(r'(?m)^(?:unsafe )?fn '+row['name']+r'\b',source);assert m;body=source[m.start():item_end(source,m.start())];assert hashlib.sha256(body.encode()).hexdigest()==row['source_sha256'];functions.append(body);needed|=identifiers(body)&catalog.keys()
    m=re.search(r'(?m)^fn shell_draw_text\b',source);helper=source[m.start():item_end(source,m.start())]
    # Preserve the previously qualified admission correction. The helper still
    # attempts every original chunk; only its success observation changes.
    corrected=helper.replace('let mut offset: usize = 0;','let mut offset: usize = 0;let mut admitted=true;').replace('pdx_call(SLOT_DISPLAY, 0xFB, sid, packed, arg2);','let(status,_)=pdx_call(SLOT_DISPLAY, 0xFB, sid, packed, arg2);admitted &= status==0;').replace('(offset, offset > 0)','(offset, offset > 0 && admitted)');assert corrected!=helper;functions.append(corrected);needed|=identifiers(helper)&catalog.keys()
    selected={}
    while needed:
        name=needed.pop()
        if name in selected:continue
        selected[name]=catalog[name];needed|=(identifiers(catalog[name])&catalog.keys())-selected.keys()
    prefix='''#![allow(dead_code,unused_variables,unused_mut,static_mut_refs)]
extern crate self as sex_pdx;
macro_rules! serial_println {($($arg:tt)*)=>{println!($($arg)*)};}
'''
    rust=prefix+'\n'.join(selected.values())+'\n'+'\n'.join(functions)
    rust+='''
static mut STATUS:u64=0;static mut ACCEPTED:u64=0;static mut SEED:u64=0;static mut CALLS:u64=0;
fn pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64){unsafe{let status=if CALLS<ACCEPTED{0}else{STATUS};CALLS=CALLS.wrapping_add(1);let value=a^b^c^op^SEED;println!("P {} {} {} {} {} {} {}",slot,op,a,b,c,status,value);(status,value)}}
unsafe fn mask()->u64{MASK}
unsafe fn assign(n:u64){ASSIGN}
unsafe fn dispatch(index:usize){match index{DISPATCH _=>panic!("bad index")}}
fn main(){use std::io::BufRead;let mut other=0u64;for line in std::io::stdin().lock().lines(){let line=line.unwrap();let args:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();let n=|i:usize|args.get(i).copied().unwrap_or(0);unsafe{
match n(0){0=>{assign(0);other=0;STATUS=0;ACCEPTED=0;SEED=0;CALLS=0;},1=>dispatch(n(1)as usize),2=>{STATUS=n(1);ACCEPTED=n(2);SEED=n(3);CALLS=0;},3=>assign(n(1)),4=>{let saved=mask();assign(other);dispatch(n(1)as usize);other=mask();assign(saved);},5=>other=n(1),_=>panic!("bad operation")}
println!("S {} {} {}",mask(),other,CALLS);
}}}
'''
    fields=[row['done']for row in inventory['functions']]
    rust=rust.replace('MASK','|'.join(f'(({name}as u64)<<{i})'for i,name in enumerate(fields)))
    # The space before `as` is needed independently of the field spelling.
    rust=rust.replace('as u64)', ' as u64)')
    rust=rust.replace('ASSIGN',''.join(f'{name}=n&(1<<{i})!=0;'for i,name in enumerate(fields)))
    rust=rust.replace('DISPATCH',''.join(f'{i}=>{row["name"]}(),'for i,row in enumerate(inventory['functions'])))
    (w/'oracle.rs').write_text(rust)
    c='''#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "silk_shell_proofs_a.h"
SilkShellState SILK_SHELL_STATE;
static SilkShellState other;
static uint64_t status,accepted,seed,calls;
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);va_end(ap);putchar('\\n');}
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c){uint64_t result=calls<accepted?0:status;++calls;uint64_t value=a^b^c^op^seed;printf("P %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\\n",slot,op,a,b,c,result,value);return(SexosReply){result,value};}
static uint64_t mask(const SilkShellState *state){return MASK;}
static void assign(SilkShellState *state,uint64_t n){ASSIGN}
static void dispatch(size_t index,bool separate){switch(index){DISPATCH default:exit(3);}}
int main(void){char line[512];while(fgets(line,sizeof(line),stdin)){uint64_t n[6]={0};char *at=line,*end;for(size_t i=0;i<6;++i){n[i]=strtoull(at,&end,10);if(at==end)break;at=end;}
switch(n[0]){case 0:memset(&SILK_SHELL_STATE,0,sizeof(SILK_SHELL_STATE));memset(&other,0,sizeof(other));status=accepted=seed=calls=0;break;case 1:dispatch(n[1],false);break;case 2:status=n[1];accepted=n[2];seed=n[3];calls=0;break;case 3:assign(&SILK_SHELL_STATE,n[1]);break;case 4:dispatch(n[1],true);break;case 5:assign(&other,n[1]);break;default:return 3;}
printf("S %" PRIu64 " %" PRIu64 " %" PRIu64 "\\n",mask(&SILK_SHELL_STATE),mask(&other),calls);
}return 0;}
'''
    c=c.replace('MASK','|'.join(f'((uint64_t)state->{row["state_component"]}.{row["done"]}<<{i})'for i,row in enumerate(inventory['functions'])))
    c=c.replace('ASSIGN',''.join(f'state->{row["state_component"]}.{row["done"]}=(n&(UINT64_C(1)<<{i}))!=0;'for i,row in enumerate(inventory['functions'])))
    c=c.replace('DISPATCH',''.join(f'case {i}:if(separate)shell_proofs_a_{row["name"]}(&other.{row["state_component"]});else {row["name"]}();break;'for i,row in enumerate(inventory['functions'])))
    (w/'provider.c').write_text(c)
    return helper

def corpus(count):
    rows=[]
    def add(*n):rows.append(' '.join(str(x&((1<<64)-1))for x in n))
    for i in range(count):
        for status in (0,1,2,(1<<64)-1):
            for accepted in (0,1,2,3,64):
                add(0);add(2,status,accepted,0xfedcba9876543210);add(1,i);add(1,i);add(4,i);add(4,i)
        add(0);add(3,(1<<count)-1);add(1,i);add(5,(1<<count)-1);add(4,i)
    rng=random.Random(20261102)
    for _ in range(500):
        add(0);add(3,rng.getrandbits(count));add(5,rng.getrandbits(count));add(2,rng.choice((0,1,2,(1<<64)-1)),rng.randrange(16),rng.getrandbits(64));add(1,rng.randrange(count));add(4,rng.randrange(count));add(1,rng.randrange(count))
    return ('\n'.join(rows)+'\n').encode(),len(rows)

def independent(binary,inventory,enabled):
    count=0
    for i,row in enumerate(inventory['functions']):
        for separate in (False,True):
            operation=4 if separate else 1
            out=run([binary],f'0\n{operation} {i}\n{operation} {i}\n'.encode()).decode().splitlines();states=[x for x in out if x.startswith('S ')]
            expected=(1<<i)if enabled else 0
            assert list(map(int,states[-1].split()[1:3]))==([0,expected]if separate else[expected,0]),(row['name'],separate,states)
            last=[i for i,line in enumerate(out)if line.startswith('S ')][-2];assert out[last+1:]==[states[-1]],(row['name'],'one-shot reexecuted',out[last+1:]);count+=1
    if not enabled:return count
    index=next(i for i,row in enumerate(inventory['functions'])if row['name']=='maybe_run_shell_draw_text_helper_proof')
    texts=(b'Browser / WebStub',b'Local doc stub',b'network=0 engine=0',b'URL: marker-only')
    for status,accepted in ((0,0),(1,0),(2,1),((1<<64)-1,3)):
        out=run([binary],f'0\n2 {status} {accepted} 0\n1 {index}\n'.encode()).decode().splitlines();calls=[list(map(int,x.split()[1:]))for x in out if x.startswith('P ')]
        expected=[];attempt=0;success=[]
        for text in texts:
            ok=True
            for off in range(0,len(text),8):
                chunk=text[off:off+8];result=0 if attempt<accepted else status;attempt+=1;ok&=result==0
                expected.append([5,0xfb,205,int.from_bytes(chunk,'little'),off|(len(chunk)<<8)|(0x00cdd6f4<<32),result])
            success.append(int(ok))
        assert [x[:6]for x in calls]==expected,(status,accepted,calls,expected)
        records=[x for x in out if x.startswith('[shell.text.draw.send]')]
        assert records==[f'[shell.text.draw.send] sid=205 len={len(text)} status={ok} err=0'for text,ok in zip(texts,success)],records;count+=1
    for name in ('maybe_run_e1000_driver_status_proof','maybe_run_http_client_status_proof'):
        i=next(i for i,row in enumerate(inventory['functions'])if row['name']==name)
        out=run([binary],f'0\n1 {i}\n'.encode()).decode();assert '\nP 'not in out,(name,out);count+=1
    return count

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/proofs-a-tests');args=parser.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/silk_shell_proofs_a.source.json').read_text());prepare(w,inventory);data,count=corpus(len(inventory['functions']));exports=[]
    for unit in ('silk_shell_proofs_a','silk_shell_proofs_a_root','silk_shell_diagnostics_a_native'):
        out=w/(unit+'.c');run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','-o',out,'--','-Igenerated/include']);exports.append(out)
    profiles=[('default',[]) ,('all',sorted({flag for row in inventory['functions']for flag in row['env_flags']}))];results=[]
    for profile,flags in profiles:
        env={k:v for k,v in os.environ.items()if not k.startswith(('SEXOS_','SILK_DE_'))};env.update({flag:'1'for flag in flags})
        rust=w/('rust-'+profile);run(['rustc','--edition=2021','-O',w/'oracle.rs','-o',rust],env=env);expected=run([rust],data)
        for cc in ('gcc','clang'):
            for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
                binary=w/(profile+'-'+cc+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*options,*('-D'+flag+'=1'for flag in flags),'-Igenerated/include',*exports,w/'provider.c','-o',binary]);actual=run([binary],data)
                if actual!=expected:
                    left,right=expected.splitlines(),actual.splitlines();i=next((i for i,pair in enumerate(zip(left,right))if pair[0]!=pair[1]),min(len(left),len(right)));raise AssertionError((profile,cc,mode,i,left[max(0,i-2):i+2],right[max(0,i-2):i+2]))
                results.append(dict(profile=profile,compiler=cc,mode=mode,sha256=hashlib.sha256(actual).hexdigest(),independent_controls=independent(binary,inventory,profile=='all')))
    isolated=[]
    # Isolate every build gate: all-on/all-off cannot detect aliased flags.
    isolated_data=('0\n'+''.join(f'1 {i}\n'for i in range(len(inventory['functions'])))+'4 0\n').encode()
    for index,row in enumerate(inventory['functions']):
        env={k:v for k,v in os.environ.items()if not k.startswith(('SEXOS_','SILK_DE_'))};env.update({flag:'1'for flag in row['env_flags']})
        rust=w/('isolated-rust-'+str(index));run(['rustc','--edition=2021','-O',w/'oracle.rs','-o',rust],env=env);expected=run([rust],isolated_data)
        active_mask=sum(1<<i for i,entry in enumerate(inventory['functions'])if set(entry['env_flags'])&set(row['env_flags']))
        states=[line for line in expected.decode().splitlines()if line.startswith('S ')];assert int(states[-2].split()[1])==active_mask,(index,states)
        (w/('isolated-'+str(index)+'.trace')).write_bytes(expected)
        for cc in ('gcc','clang'):
            binary=w/('isolated-'+cc+'-'+str(index));run([cc,'-std=c11','-O2',*('-D'+flag+'=1'for flag in row['env_flags']),'-Igenerated/include',*exports,w/'provider.c','-o',binary]);actual=run([binary],isolated_data);assert actual==expected,(index,cc,'build flag mismatch');isolated.append(dict(function=row['name'],compiler=cc,sha256=hashlib.sha256(actual).hexdigest()))
    # Each driver's one-shot ledger is checked separately, including functions
    # whose source contains only diagnostic records and no IPC.
    from silk_shell_models_differential import mutate_function
    base=exports[0].read_text();mutations=[];flags=profiles[-1][1];reference=run([w/'rust-all'],data)
    changes=[('repeat_'+row['name'],'shell_proofs_a_'+row['name'],'state->'+row['done']+'=true','state->'+row['done']+'=false')for row in inventory['functions']]
    changes+=[('big_endian','proof_draw_17','<<(i*8)','<<((7-i)*8)'),('wrong_text_offset','proof_draw_17','(offset&255)','((offset+1)&255)'),('wrong_color','proof_draw_17','(color<<32)','((color^1)<<32)'),('ignore_admission','proof_draw_17','if(result.status)admitted=false','if(false)admitted=false'),('truncate_tail','proof_draw_17','while(offset<17)','while(offset<16)')]
    for name,function,old,new in changes:
        path=w/('mutant-'+name+'.c');path.write_text(mutate_function(base,function,old,new));binary=w/('mutant-'+name);run(['gcc','-std=c11','-O2',*('-D'+flag+'=1'for flag in flags),'-Igenerated/include',path,*exports[1:],w/'provider.c','-o',binary]);r=subprocess.run([binary],input=data,capture_output=True);assert r.returncode!=0 or r.stdout!=reference,('survived',name);mutations.append(name)
    first,second=inventory['functions'][:2];path=w/'mutant-aliased-build-flag.c';path.write_text(mutate_function(base,'shell_proofs_a_'+first['name'],first['flag'],second['flag']));binary=w/'mutant-aliased-build-flag';run(['gcc','-std=c11','-O2',*('-D'+flag+'=1'for flag in first['env_flags']),'-Igenerated/include',path,*exports[1:],w/'provider.c','-o',binary]);assert run([binary],isolated_data)!=(w/'isolated-0.trace').read_bytes();mutations.append('aliased_build_flag')
    (w/'summary.json').write_text(json.dumps(dict(pin=inventory['pin'],functions=len(inventory['functions']),commands=count,configurations=results,isolated_build_flags=isolated,mutants_caught=mutations,boundary='Actual diagnostic bodies and corrected real byte-packing helper; hosted transport. Printed claims are not independent feature or pixel evidence.'),indent=2)+'\n');print('Proofs-A drivers:',len(inventory['functions']),'bodies;',count,'commands/configuration;',len(results),'configurations;',len(mutations),'mutants caught')

if __name__=='__main__':main()
