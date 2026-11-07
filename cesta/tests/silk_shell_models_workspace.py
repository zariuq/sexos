"""Actual workspace invalidation bodies with real custody and snapshot logic."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
from silk_shell_input_differential import declarations,item_end
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent
PIN='b6be1b451a6c21985d00d192674feedf66bb11dd'
def run(args,data=None):
    r=subprocess.run(list(map(str,args)),cwd=HERE,input=data,capture_output=True)
    assert r.returncode==0,(args,r.returncode,r.stderr[-6000:]);return r.stdout

C=r'''
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "silk_shell_models_workspace.h"
static ShellChromeApps apps;
static ShellLinenRuntime linen;
static ShellLinenWorkspaceContext ctx={&apps,&linen};
static uint64_t statuses[4096];
void sexos_fail(void){exit(88);}
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);va_end(ap);putchar('\n');}
uint64_t shell_models_workspace_info(uint64_t slot){if(slot>=4096)exit(88);printf("Q %" PRIu64 "\n",slot);return statuses[slot];}
static void trace(void){uint64_t mask=0;for(size_t i=0;i<SHELL_APP_COUNT;++i)if(linen.workspace_refresh_pending[i])mask|=UINT64_C(1)<<i;printf("S %" PRIu64 " %u %u\n",mask,(unsigned)shell_linen_snapshot_busy(&linen.snapshot),(unsigned)shell_linen_snapshot_can_send(&linen.snapshot));}
int main(void){char line[512];while(fgets(line,sizeof(line),stdin)){
 uint64_t n[10]={0};char *at=line,*end;for(size_t i=0;i<10;++i){n[i]=strtoull(at,&end,10);if(at==end)break;at=end;}
 switch(n[0]){
 case 0:memset(&apps,0,sizeof(apps));memset(&linen,0,sizeof(linen));memset(statuses,0,sizeof(statuses));linen.snapshot=shell_linen_snapshot_new();break;
 case 1:{if(n[1]>=SHELL_APP_COUNT)exit(88);ShellAppCatalogRow row=shell_custody_catalog_at(n[1]);ShellAppCustody e=shell_custody_new(row.lifecycle_slot,row.target,(uint32_t)n[2],row.surface);e.dead=(n[3]&1)!=0;e.close_pending=(n[3]&2)!=0;e.retired=(n[3]&4)!=0;apps.entries[n[1]]=e;apps.present[n[1]]=true;break;}
 case 2:if(n[1]>=SHELL_APP_COUNT||apps.entries[n[1]].slot>=4096)exit(88);statuses[apps.entries[n[1]].slot]=n[2];break;
 case 3:{SexosMessage m={.caller_pd=(uint32_t)n[1],.caller_task=(uint32_t)n[2],.type_id=n[3],.arg0=n[4],.arg1=n[5],.arg2=n[6]};printf("R %u\n",(unsigned)shell_models_workspace_receive(&ctx,&m));break;}
 case 4:shell_models_workspace_poll(&ctx);break;
 case 5:if(n[1]>=SHELL_APP_COUNT)exit(88);apps.entries[n[1]].dead=(n[2]&1)!=0;apps.entries[n[1]].close_pending=(n[2]&2)!=0;break;
 case 6:if(n[1]>=SHELL_APP_COUNT)exit(88);apps.present[n[1]]=false;break;
 case 7:if(n[1]>=SHELL_APP_COUNT)exit(88);linen.workspace_refresh_pending[n[1]]=true;break;
 case 8:linen.snapshot=shell_linen_snapshot_new();break;
 default:exit(3);
 }trace();
 }return 0;}
'''

R=r'''
static mut STATUS:[u64;4096]=[0;4096];
mod app_lifecycle {
 pub const LIVE:u64=5;pub const RETRY:u64=2;
 pub fn info(slot:usize)->Result<(),u64>{println!("Q {}",slot);unsafe{if super::STATUS[slot]==0{Ok(())}else{Err(super::STATUS[slot])}}}
}
mod document_view {pub const WORKSPACE_CHANGED:u64=0x279;}
static mut APP_CUSTODY:[Option<app_custody::Entry>;app_catalog::COUNT]=[None;app_catalog::COUNT];
static mut WORKSPACE_REFRESH_PENDING:[bool;app_catalog::COUNT]=[false;app_catalog::COUNT];
static mut LINEN_ASYNC:linen_async::Snapshot=linen_async::Snapshot::new();
unsafe fn trace(){let mut mask=0u64;for i in 0..app_catalog::COUNT{if WORKSPACE_REFRESH_PENDING[i]{mask|=1<<i;}}println!("S {} {} {}",mask,LINEN_ASYNC.busy()as u8,LINEN_ASYNC.can_send()as u8);}
fn main(){use std::io::BufRead;for line in std::io::stdin().lock().lines(){let line=line.unwrap();let args:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();let n=|i:usize|args.get(i).copied().unwrap_or(0);unsafe{
 match n(0){
 0=>{APP_CUSTODY=[None;app_catalog::COUNT];WORKSPACE_REFRESH_PENDING=[false;app_catalog::COUNT];LINEN_ASYNC=linen_async::Snapshot::new();STATUS=[0;4096];},
 1=>{let row=app_catalog::INCARNATIONS[n(1)as usize];let mut e=app_custody::Entry::new(row.lifecycle_slot,row.target,n(2)as u32,row.surface);e.dead=n(3)&1!=0;e.close_pending=n(3)&2!=0;e.retired=n(3)&4!=0;APP_CUSTODY[n(1)as usize]=Some(e);},
 2=>STATUS[APP_CUSTODY[n(1)as usize].as_ref().unwrap().slot]=n(2),
 3=>{let msg=PdxMessage{caller_pd:n(1)as u32,caller_task:n(2)as u32,type_id:n(3),arg0:n(4),arg1:n(5),arg2:n(6)};println!("R {}",workspace_refresh_receive(&msg)as u8);},
 4=>workspace_refresh_poll(),
 5=>{let e=APP_CUSTODY[n(1)as usize].as_mut().unwrap();e.dead=n(2)&1!=0;e.close_pending=n(2)&2!=0;},
 6=>APP_CUSTODY[n(1)as usize]=None,7=>WORKSPACE_REFRESH_PENDING[n(1)as usize]=true,
 8=>LINEN_ASYNC=linen_async::Snapshot::new(),
 10=>for(i,row)in app_catalog::INCARNATIONS.iter().enumerate(){println!("C {} {} {}",i,row.target,row.app==app_catalog::App::Spindle);},
 _=>panic!("unknown operation")
 }trace();
 }}}
'''


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/models-workspace-tests');args=parser.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True)
    source=run(['git','show',PIN+':servers/silk-shell/src/main.rs']).decode();functions=[]
    for name in ('workspace_refresh_receive','workspace_refresh_poll'):
        m=re.search(r'(?m)^unsafe fn '+name+r'\b',source);assert m;functions.append(source[m.start():item_end(source,m.start())])
    pdx=run(['git','show',PIN+':crates/sex-pdx/src/lib.rs']).decode();prefix='#![allow(dead_code,unused_imports,static_mut_refs)]\nextern crate self as sex_pdx;\n#[macro_export]macro_rules! serial_record{($($arg:tt)*)=>{println!($($arg)*)};}\n'+declarations(pdx)['PdxMessage']+'\n'
    for name,path in (('app_catalog','crates/sex-pdx/src/app_catalog.rs'),('app_custody','servers/silk-shell/src/app_custody.rs'),('linen_async','servers/silk-shell/src/linen_async.rs')):
        actual=HERE.parent/path;assert actual.read_bytes()==run(['git','show',PIN+':'+path]);prefix+='#[path='+json.dumps(str(actual))+']mod '+name+';\n'
    (w/'oracle.rs').write_text(prefix+'\n'.join(functions)+R);run(['rustc','--edition=2021','-O',w/'oracle.rs','-o',w/'rust']);(w/'provider.c').write_text(C)
    rows=[]
    def add(*n):rows.append(' '.join(map(str,n)))
    catalog=[line.split()for line in run([w/'rust'],b'10\n').decode().splitlines()if line.startswith('C ')]
    for _,index,target,spindle in catalog:
        index,target=int(index),int(target);task=700+index
        for status in (0,1,2,3,5,6,(1<<64)-1):
            for flags in (0,1,2,4):
                for corrupt in range(7):
                    add(0);add(1,index,task,flags);add(2,index,status)
                    fields=[target,task,0x279,target,task,0]
                    if corrupt:fields[corrupt-1]+=1
                    add(3,*fields);add(4);add(2,index,5);add(4);add(4)
        for cancel in (1,2,3):
            add(0);add(1,index,task,0);add(2,index,2);add(7,index);add(4)
            if cancel==3:add(6,index)
            else:add(5,index,cancel)
            add(4)
    add(0)
    for _,index,target,spindle in catalog:add(1,int(index),700+int(index),0);add(2,int(index),2);add(7,int(index))
    add(4);add(4)
    for _,index,target,spindle in catalog:add(2,int(index),5)
    add(4);add(4)
    data=('\n'.join(rows)+'\n').encode();expected=run([w/'rust'],data);exports=[]
    for unit in ('silk_shell_models_workspace','silk_shell_custody','shell_linen_snapshot'):
        out=w/(unit+'.c');run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','-o',out,'--','-Igenerated/include']);exports.append(out)
    configurations=[]
    for cc in ('gcc','clang'):
        for mode,flags in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
            binary=w/(cc+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-Igenerated/include',*exports,w/'provider.c','-o',binary]);actual=run([binary],data)
            if actual!=expected:
                left,right=expected.splitlines(),actual.splitlines();i=next((i for i,pair in enumerate(zip(left,right))if pair[0]!=pair[1]),min(len(left),len(right)));raise AssertionError((cc,mode,i,left[max(0,i-2):i+2],right[max(0,i-2):i+2]))
            configurations.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(actual).hexdigest()))
    base=exports[0].read_text();mutants=[]
    for name,old,new in (
        ('wrong_caller','entry->target!=message->caller_pd','false'),
        ('wrong_task','entry,message->caller_task,message->type_id','entry,entry->task,message->type_id'),
        ('drop_retry','status!=SHELL_WORKSPACE_RETRY','true'),
        ('refresh_before_live','status==SHELL_WORKSPACE_LIVE','status==SHELL_WORKSPACE_RETRY'),
        ('ignore_close','entry->dead || entry->close_pending','entry->dead'),
        ('repeat_complete','workspace_refresh_pending[i]=false;\n   shell_linen_snapshot_refresh','workspace_refresh_pending[i]=true;\n   shell_linen_snapshot_refresh'),
    ):
        assert LayoutText(base).count(old)==1,(name,LayoutText(base).count(old));path=w/('mutant-'+name+'.c');path.write_text(LayoutText(base).replace(old,new));binary=w/('mutant-'+name);run(['gcc','-std=c11','-O2','-Igenerated/include',path,*exports[1:],w/'provider.c','-o',binary]);r=subprocess.run([binary],input=data,capture_output=True);assert r.returncode!=0 or r.stdout!=expected,('survived',name);mutants.append(name)
    summary=dict(pin=PIN,functions_sha256=[hashlib.sha256(x.encode()).hexdigest()for x in functions],commands=len(rows),catalog_members=len(catalog),configurations=configurations,mutants_caught=mutants,boundary='Actual current custody/snapshot functions and two workspace bodies. Lifecycle status is scripted; the native syscall is compiled separately and never executed on the host.')
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Workspace invalidation:',len(rows),'commands/configuration;',len(mutants),'mutations caught')


if __name__=='__main__':main()
