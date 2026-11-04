"""Actual current Linen wrappers and bounded publication, with scripted paint/transport."""
import argparse
from pathlib import Path
import subprocess,sys,re,json,hashlib
from silk_shell_input_differential import declarations,identifiers,item_end
p=Path(__file__).resolve().parent.parent

def prepare(w):
    pin='c3d987c9fba6d9e6243ad1af9f7f3d4a7b457321';source=subprocess.check_output(['git','show',pin+':servers/silk-shell/src/main.rs'],cwd=p).decode();pdx=subprocess.check_output(['git','show',pin+':crates/sex-pdx/src/lib.rs'],cwd=p).decode()
    names=('linen_fetch_remote_snapshot','linen_queue_open','linen_async_poll','linen_async_reply','linen_select_first_valid_object');functions=[];rows=[]
    for name in names:
     m=re.search(r'(?m)^(?:unsafe )?fn '+name+r'\b',source);assert m,name;raw=source[m.start():item_end(source,m.start())]+'\n';functions.append(raw);rows.append(dict(name=name,sha256=hashlib.sha256(raw.encode()).hexdigest()))
    catalog={**declarations(pdx),**declarations(source)};needed={'LINEN_SEED_OBJECTS','LINEN_OBJECTS','LINEN_REMOTE_REAL','LINEN_REMOTE_FETCHED','SELECTED_LINEN_OBJECT_ID','LINEN_ASYNC','PdxMessage'}
    for raw in functions:needed|=identifiers(raw)&catalog.keys()
    selected={}
    while needed:
     name=needed.pop()
     if name in selected:continue
     assert name in catalog,name
     selected[name]=catalog[name];needed|=(identifiers(catalog[name])&catalog.keys())-selected.keys()
    prefix='''#![allow(dead_code,unused_variables,unused_mut,static_mut_refs)]
    extern crate self as sex_pdx;
    macro_rules! serial_println {($($arg:tt)*)=>{println!($($arg)*)};}
    #[macro_export]macro_rules! serial_record {($($arg:tt)*)=>{println!($($arg)*)};}
    '''
    prefix+='#[path='+json.dumps(str(p.parent/'servers/silk-shell/src/linen_async.rs'))+']mod linen_async;\n'
    (w/'live-prefix.rs').write_text(prefix+'\n'.join(selected.values())+'\n'+'\n'.join(functions))
    (w/'live-source.json').write_text(json.dumps({'pin':pin,'source_sha256':hashlib.sha256(source.encode()).hexdigest(),'functions':rows,'selected':sorted(selected)},indent=2)+'\n');print(len(functions),'actual current wrapper bodies;',len(selected),'original declarations')

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,default=Path('cesta'));parser.add_argument('--output',type=Path,default=p/'build/models-linen-tests');args=parser.parse_args()
    w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);t=args.cesta.resolve() if args.cesta.is_file() else args.cesta
    fix=p/'tests/silk_shell_models_linen'
    def run(args,data=None):
     r=subprocess.run(list(map(str,args)),cwd=p,input=data,capture_output=True)
     assert r.returncode==0,(args,r.returncode,r.stdout[-1000:],r.stderr[-5000:])
     return r.stdout
    prepare(w);(w/'live-oracle.rs').write_text((w/'live-prefix.rs').read_text()+'\n'+(fix/'provider.rs').read_text());run(['rustc','--edition=2021','-C','opt-level=1',w/'live-oracle.rs','-o',w/'live-rust'])
    run([t,'export-c','src/silk_shell_models.cesta','--std=c11','-o',w/'models-seed-source.c','--','-Igenerated/include']);s=(w/'models-seed-source.c').read_text();m=re.search(r'(?m)^const LinenObject LINEN_SEED_OBJECTS\b',s);assert m;seed='#include "silk_shell_models_linen.h"\n'+s[m.start():item_end(s,m.start())]+'\n';(w/'live-seed.c').write_text(seed)
    exports=[]
    for name in ('silk_shell_models_linen','silk_shell_models_projection','silk_shell_models_publish','silk_shell_models_publish_native','linen_capture','linen_capture_client','shell_linen_snapshot','shell_text_delivery','shell_text_delivery_native','shell_text_delivery_display'):
     out=w/(name+'.c');run([t,'export-c','src/'+name+'.cesta','--std=c11','-o',out,'--','-Igenerated/include']);exports.append(out)
    rows=[]
    def add(*values):rows.append(' '.join(str(x&((1<<64)-1))for x in values))
    for length in (0,1,8,9,24):
     for selected in (0,1,105,999):
      add(0);add(5,selected);add(1)
      for i in range(16):
       if i==8:add(5,105)
       add(3);add(4,8,1,0);add(4,7,2,0);add(4,7,1,(100+i)|(i%3<<32)|(length<<40)|(i%16<<48))
       for off in range(0,length,8):add(3);add(4,7,1,sum((65+(i+off+j)%26)<<(j*8)for j in range(min(8,length-off))))
    for status in (1,(1<<64)-2,(1<<64)-1):add(0);add(1);add(6,status);add(3);add(3);add(6,0);add(3);add(4,7,1,1<<40|1);add(3);add(4,7,1,0)
    add(0);add(1)
    for i in range(16):add(3);add(4,7,1,0)
    for i in range(6):add(2,i+1,90+i)
    for i in range(4):add(3);add(4,7,1,i)
    data=('\n'.join(rows)+'\n').encode();expected=run([w/'live-rust'],data)
    for cc in ('gcc','clang'):
     for mode,opts in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
      binary=w/('live-'+cc+'-'+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*opts,'-Igenerated/include',*exports,w/'live-seed.c',fix/'provider.c','-o',binary]);actual=run([binary],data)
      if actual!=expected:
       left,right=expected.splitlines(),actual.splitlines();line=next((i for i,x in enumerate(zip(left,right))if x[0]!=x[1]),min(len(left),len(right)));raise AssertionError((cc,mode,line,left[max(0,line-2):line+2],right[max(0,line-2):line+2]))
    print('Live publication wrappers:',len(rows),'commands/configuration, actual current Rust and GCC/Clang O0/O2/SAN agree')
    (w/'live-runtime-summary.json').write_text(json.dumps({'commands':len(rows),'configurations':6,'source_functions':5,'boundary':'Actual checked serial publication and converter; rendering peer and correlated transport are separately scripted in this legacy comparison.'},indent=2)+'\n')
    mutants=[]
    base={out.stem:out.read_text()for out in exports}
    for name,unit,old,new in (
     ('publish_early','silk_shell_models_linen','case SHELL_LINEN_PUBLISHED:','case SHELL_LINEN_PROGRESS:'),
     ('lose_selection','silk_shell_models_publish','if(retained)selected=previous;','if(false)selected=previous;'),
     ('partial_table_visible','silk_shell_models_publish','models->LINEN_OBJECTS[i]=next[i]','models->LINEN_OBJECTS[i]=(LinenObject){0}'),
     ('rejection_claims_fetched','silk_shell_models_linen','case SHELL_LINEN_REJECTED:ctx->models->LINEN_REMOTE_FETCHED=false;','case SHELL_LINEN_REJECTED:ctx->models->LINEN_REMOTE_FETCHED=true;'),
     ('foreign_reply_consumed','silk_shell_models_linen','if(message->caller_pd!=7)','if(message->caller_pd==0)'),
     ('projection_wrong_kind','silk_shell_models_projection','source->kind==1?LINEN_OBJ_KIND_PROJECT:LINEN_OBJ_KIND_DOCUMENT','source->kind==1?LINEN_OBJ_KIND_DOCUMENT:LINEN_OBJ_KIND_PROJECT'),
     ('projection_omits_name','silk_shell_models_projection','out->name[i]=source->name[i]','out->name[i]=0'),
    ):
     text=base[unit];assert text.count(old)==1,(name,text.count(old));path=w/('live-mutant-'+name+'.c');path.write_text(text.replace(old,new));binary=w/('live-mutant-'+name)
     linked=[path if out.stem==unit else out for out in exports];run(['gcc','-std=c11','-O2','-Igenerated/include',*linked,w/'live-seed.c',fix/'provider.c','-o',binary]);changed=subprocess.run([binary],input=data,capture_output=True);assert changed.returncode!=0 or changed.stdout!=expected,('survived',name);mutants.append(name)
    summary=json.loads((w/'live-runtime-summary.json').read_text());summary['mutants']=mutants;(w/'live-runtime-summary.json').write_text(json.dumps(summary,indent=2)+'\n');print(len(mutants),'live publication mutations caught')

if __name__=='__main__':main()
