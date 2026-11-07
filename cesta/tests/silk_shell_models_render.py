"""Current Linen renderers with actual list composition and atomic text delivery.

The original static fallback contains forbidden NUL gaps. Its assertion is a
negative control; the qualified variant fills the same columns with spaces.
Chrome and display admission are scripted peers, not actual rendered pixels.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
from silk_shell_input_differential import declarations,identifiers,item_end
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent
FIX=HERE/'tests/silk_shell_models_render'
PIN='b6be1b451a6c21985d00d192674feedf66bb11dd'


def run(args,data=None):
    r=subprocess.run(list(map(str,args)),cwd=HERE,input=data,capture_output=True)
    assert r.returncode==0,(args,r.returncode,r.stderr[-6000:])
    return r.stdout


def prepare(w):
    source=run(['git','show',PIN+':servers/silk-shell/src/main.rs']).decode()
    functions=[]
    for name in ('linen_render_object_list','linen_render_static_ui','linen_queue_text',
                 'linen_paint_surface','linen_paint_surface_fast','linen_fetch_remote_snapshot',
                 'linen_async_poll','linen_async_reply','linen_select_first_valid_object','linen_object_count'):
        m=re.search(r'(?m)^unsafe fn '+name+r'\b',source);assert m,name
        functions.append(source[m.start():item_end(source,m.start())])
    pdx=run(['git','show',PIN+':crates/sex-pdx/src/lib.rs']).decode()
    catalog={**declarations(pdx),**declarations(source)};needed={'LINEN_SEED_OBJECTS','PdxMessage'}
    for body in functions:needed|=identifiers(body)&catalog.keys()
    selected={}
    while needed:
        name=needed.pop()
        if name in selected:continue
        selected[name]=catalog[name]
        needed|=(identifiers(catalog[name])&catalog.keys())-selected.keys()
    provider=(FIX/'provider.rs').read_text()
    a=provider.index('impl linen_text_delivery::Outbox');b=provider.index('fn main()',a)
    trace=provider[a:b].replace('linen_text_delivery::','').replace(' fn trace',' pub fn trace')
    delivery=HERE.parent/'servers/silk-shell/src/linen_text_delivery.rs'
    assert delivery.read_bytes()==run(['git','show',PIN+':servers/silk-shell/src/linen_text_delivery.rs'])
    (w/'delivery.rs').write_text(delivery.read_text()+trace)
    prefix='''#![allow(dead_code,unused_variables,unused_mut,static_mut_refs)]
extern crate self as sex_pdx;
macro_rules! serial_println {($($arg:tt)*)=>{println!($($arg)*)};}
#[macro_export]macro_rules! serial_record {($($arg:tt)*)=>{println!($($arg)*)};}
'''
    desktop=run(['git','show',PIN+':servers/silk-shell/src/desktop_content.rs']).decode()
    desktop_items=declarations(desktop);pieces=[desktop_items['Item'],desktop_items['List']]
    for pattern in (r'(?m)^impl List\b',r'(?m)^pub fn list\b'):
        match=re.search(pattern,desktop);assert match;pieces.append(desktop[match.start():item_end(desktop,match.start())])
    (w/'desktop.rs').write_text('\n'.join(pieces)+'\n')
    paths={'linen_text_delivery':w/'delivery.rs','desktop_content':w/'desktop.rs','linen_async':HERE.parent/'servers/silk-shell/src/linen_async.rs'}
    assert paths['linen_async'].read_bytes()==run(['git','show',PIN+':servers/silk-shell/src/linen_async.rs'])
    for name in ('text_encoding','text_batch','app_catalog'):
        paths[name]=HERE.parent/('crates/sex-pdx/src/'+name+'.rs')
        assert paths[name].read_bytes()==run(['git','show',PIN+':crates/sex-pdx/src/'+name+'.rs'])
    for name,path in paths.items():prefix+='#[path='+json.dumps(str(path))+']mod '+name+';\n'
    original=prefix+'\n'.join(selected.values())+'\n'+'\n'.join(functions)+'\n'+provider[:a]+provider[b:]
    (w/'original.rs').write_text(original)
    old='let mut text = [0u8; (LINEN_UI_ROW_COUNT + 1) * 20];'
    assert original.count(old)==1
    (w/'oracle.rs').write_text(original.replace(old,"let mut text = [b' '; (LINEN_UI_ROW_COUNT + 1) * 20];"))
    for name in ('original','oracle'):run(['rustc','--edition=2021','-O',w/(name+'.rs'),'-o',w/name])
    r=subprocess.run([w/'original'],input=b'0\n1 320 168\n4\n',capture_output=True)
    assert r.returncode!=0 and b'Linen text exceeds fixed buffer'in r.stderr,(r.returncode,r.stderr)
    (w/'original-fallback-failure.txt').write_bytes(r.stderr)
    return {'pin':PIN,'main_sha256':hashlib.sha256(source.encode()).hexdigest(),'functions':[hashlib.sha256(x.encode()).hexdigest()for x in functions],'original_fallback_assertion_reproduced':True,'oracle_delta':'Static fallback gaps are spaces instead of forbidden NUL bytes. Object-list and queue bodies are unchanged.'}


def corpus():
    rows=[]
    def add(*n):rows.append(' '.join(str(x&((1<<64)-1))for x in n))
    for w,h in ((0,0),(1,1),(40,54),(255,108),(256,108),(320,168),(1280,720),(0xffffffff,0xffffffff)):
        for count in (0,1,5,6,16):
            for selected in (0,1,5,16,999):
                for length in (0,1,24,255):
                    add(0);add(1,w,h,-17,-29,selected,selected)
                    for i in range(count):add(2,i,i+1,length,i%2)
                    add(3);add(5,100);add(4);add(4);add(6,1,0x56|(200<<8)|(1<<24));add(5,100);add(6,1,0x56|(200<<8)|(3<<24));add(5,1)
    for startup in (0,1):
        for fetched in (0,1):
            for real in (0,1):
                add(0);add(1,320,168,0,0,0,0);add(10,fetched,real);add(8,startup);add(11);add(8,startup);add(9)
    for empty in (0,1):
        add(0);add(1,320,720,0,0,105,0)
        for i in range(6):add(2,i,100+i,8,0)
        add(8,0)
        for i in range(16):
            add(11);add(12,8,1,0);add(12,7,2,0)
            length=0 if empty else (8 if i%2 else 9)
            add(12,7,1,0 if empty else (100+i)|(i%3<<32)|(length<<40)|(3<<48))
            for off in range(0,length,8):add(11);add(12,7,1,sum((65+(i+off+j)%26)<<(j*8)for j in range(min(8,length-off))))
            # Repaint during staging must keep using the previous publication.
            add(9)
    return ('\n'.join(rows)+'\n').encode(),len(rows)


def independent(binary):
    out=run([binary],b'0\n1 320 168 0 0 0 255\n4\n').decode().splitlines()
    state=out[-1].split('|')[0].split();assert state[:4]==['S','1','1','0'],state[:4]
    assert list(map(int,state[4:14]))==[108,0xb8cce0,0,1,0,0,0,0,5,4],state[4:14]
    text=bytes.fromhex(state[14]);assert len(text)==108 and text[:5]==b'LINEN'
    labels=(b'PROJECTS',b'SEX MICR',b'HANDOUTS',b'HANDOFFS',b'QUIL DFT')
    expected=bytearray(b' '*108);expected[:5]=b'LINEN'
    for i,label in enumerate(labels):expected[(i+1)*20:(i+1)*20+8]=label
    assert text==expected and not any(x<32 for x in text),text
    calls=[x for x in out if x.startswith('D ')];assert len(calls)==8,calls
    assert not any(x.startswith(('T ','F '))for x in out)
    for width,height in ((0,168),(320,0),(0,0)):
        out=run([binary],f'0\n1 {width} {height}\n4\n'.encode()).decode().splitlines()
        assert out[-1]=='S 0 0 0|' and not any(x.startswith('D ')for x in out),out
    for startup,should_refresh in ((1,False),(0,True)):
        out=run([binary],f'0\n1 320 168\n8 {startup}\n11\n'.encode()).decode().splitlines()
        began=any(x.startswith('[linen.remote.snapshot.begin]')for x in out)
        storage=[x for x in out if x.startswith('D 13 ')]
        requests=[x for x in storage if x.split()[2]=='68']
        assert began==should_refresh,(startup,out)
        assert bool(requests)==should_refresh,(startup,storage,requests)
    return 6


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/models-render-tests');args=parser.parse_args()
    w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);summary=prepare(w)
    exports=[]
    units=('silk_shell_models_render','silk_shell_models_render_native','silk_shell_models_paint','silk_shell_models_paint_native',
           'silk_shell_models_linen','silk_shell_models_publish','silk_shell_models_publish_native','silk_shell_models_projection',
           'linen_capture','linen_capture_client','shell_linen_snapshot','shell_linen_list','shell_text_delivery',
           'shell_text_delivery_native','shell_linen_presentation')
    for name in units:
        out=w/(name+'.c');run([args.cesta,'export-c','src/'+name+'.cesta','--std=c11','-o',out,'--','-Igenerated/include']);exports.append(out)
    run([args.cesta,'export-c','src/silk_shell_models.cesta','--std=c11','-o',w/'models-source.c','--','-Igenerated/include'])
    source=(w/'models-source.c').read_text();m=re.search(r'(?m)^const uint32_t LINEN_UI_ROW_COLORS\b',source);assert m
    seed=w/'colors.c';constants=[source[m.start():item_end(source,m.start())]]
    m=re.search(r'(?m)^const LinenObject LINEN_SEED_OBJECTS\b',source);assert m;constants.append(source[m.start():item_end(source,m.start())])
    seed.write_text('#include "silk_shell_models_render.h"\n'+'\n'.join(constants)+'\n')
    data,count=corpus();expected=run([w/'oracle'],data);configs=[]
    for cc in ('gcc','clang'):
        for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
            binary=w/(cc+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*options,'-DSEXOS_LINEN_CORRELATED_REQUIRED=0','-Igenerated/include',*exports,seed,FIX/'provider.c','-o',binary]);actual=run([binary],data)
            if actual!=expected:
                left,right=expected.splitlines(),actual.splitlines();i=next((i for i,pair in enumerate(zip(left,right))if pair[0]!=pair[1]),min(len(left),len(right)))
                raise AssertionError((cc,mode,i,left[max(0,i-2):i+2],right[max(0,i-2):i+2]))
            configs.append({'compiler':cc,'mode':mode,'sha256':hashlib.sha256(actual).hexdigest(),'independent_controls':independent(binary)})
    mutants=[];base={path.stem:path.read_text()for path in exports}
    for name,unit,old,new in (
        ('fallback_nul_gap','silk_shell_models_render','text.bytes[i]=32','text.bytes[i]=0'),
        ('fallback_wrong_selection','silk_shell_models_render','selected = LINEN_UI_ROW_COUNT - 1','selected = 0'),
        ('fallback_wrong_color','silk_shell_models_render','0x00b8cce0','0x00ffffff'),
        ('fallback_legacy_clear','silk_shell_models_render','render_queue(ctx,&text','(void)sexos_pdx_call(SLOT_DISPLAY,0xfa,SURFACE_ID_LINEN,0,0);render_queue(ctx,&text'),
        ('list_wrong_selection','silk_shell_models_render','uint64_t selected=ctx->shell.models->SELECTED_LINEN_OBJECT_ID','uint64_t selected=0'),
        ('list_wrong_metadata','silk_shell_models_render','items.count,view.first,view.rows,selected','0,view.first,view.rows,selected'),
        ('list_omitted_clear','silk_shell_models_render','index<8','index<7'),
        ('label_omitted_name','silk_shell_models_render_native','out->name[j]=object->name[j]','out->name[j]=0'),
        ('fallback_label_omitted','silk_shell_models_render_native','out->len<SHELL_LINEN_LIST_NAME_CAPACITY','out->len<1'),
        ('storage_before_startup_fifo','silk_shell_models_paint','!startup_pending &&','true &&'),
        ('skip_empty_refresh','silk_shell_models_paint','!ctx->shell.models->LINEN_REMOTE_REAL','false'),
        ('fast_path_fetches','silk_shell_models_paint','bool remote=ctx->shell.models->LINEN_REMOTE_FETCHED','ShellLinenModelsContext c={ctx->shell.models,ctx->linen};shell_models_fetch_remote_snapshot(&c);bool remote=ctx->shell.models->LINEN_REMOTE_FETCHED'),
    ):
        source=base[unit];assert LayoutText(source).count(old)==1,(name,LayoutText(source).count(old));path=w/('mutant-'+name+'.c');path.write_text(LayoutText(source).replace(old,new));binary=w/('mutant-'+name)
        linked=[path if x.stem==unit else x for x in exports]
        run(['gcc','-std=c11','-O2','-DSEXOS_LINEN_CORRELATED_REQUIRED=0','-Igenerated/include',*linked,seed,FIX/'provider.c','-o',binary])
        changed=subprocess.run([binary],input=data,capture_output=True)
        assert changed.returncode!=0 or changed.stdout!=expected,('survived',name);mutants.append(name)
    summary.update(commands=count,configurations=configs,boundary='Actual bounded published-table extraction, list composition, static renderer and atomic outbox; Chrome and transport scripted; no pixel evidence.')
    summary['mutants_caught']=mutants
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('Linen renderers:',count,'commands/configuration; six GCC/Clang/optimization/sanitizer configurations agree; original NUL fallback assertion reproduced')


if __name__=='__main__':main()
