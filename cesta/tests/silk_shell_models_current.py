"""Current Models functions and real Linen root path against current Rust bodies."""
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
import silk_shell_models_differential as historical
from silk_shell_input_differential import declarations,identifiers,item_end
from silk_shell_models_root import HERE

PIN='c3d987c9fba6d9e6243ad1af9f7f3d4a7b457321'
UNITS=('silk_shell_models','silk_shell_models_root','silk_shell_models_render','silk_shell_models_render_native','silk_shell_models_paint','silk_shell_models_paint_native','silk_shell_models_linen','silk_shell_models_publish','silk_shell_models_publish_native','silk_shell_models_projection','linen_capture','linen_capture_client','shell_linen_snapshot','shell_linen_list','shell_text_delivery','shell_text_delivery_native','shell_linen_presentation')

def output(args):return subprocess.check_output(list(map(str,args)),cwd=HERE).decode()

def prepare(w):
    source=output(['git','show',PIN+':servers/silk-shell/src/main.rs']);catalog=declarations(source)
    old=output(['git','show',historical.REVISION+':servers/silk-shell/src/main.rs']).splitlines(keepends=True)
    names=[];needed=set();functions=[]
    for name,start,end in historical.MAIN_ITEMS:
        raw=''.join(old[start-1:end])
        if re.search(r'(?m)^(?:unsafe )?fn '+re.escape(name)+r'\b',raw):
            if name not in ('linen_sync_reply','sanitize_ascii'):names.append(name)
        else:needed.add(name)
    names+=['linen_queue_text','linen_queue_open','linen_async_poll','linen_async_reply']
    for name in names:
        match=re.search(r'(?m)^(?:unsafe )?fn '+re.escape(name)+r'\b',source);assert match,name
        body=source[match.start():item_end(source,match.start())];functions.append(body);needed|=identifiers(body)&catalog.keys()
    selected={}
    while needed:
        name=needed.pop()
        if name in selected:continue
        assert name in catalog,name
        selected[name]=catalog[name];needed|=(identifiers(catalog[name])&catalog.keys())-selected.keys()
    prefix=historical.RUST_PROVIDER.replace('/* PDX_CONSTANTS */','/* PDX_CONSTANTS */\n    pub use crate::serial_record;\n    #[path="'+str(HERE.parent/'crates/sex-pdx/src/text_encoding.rs')+'"]pub mod text_encoding;\n    #[path="'+str(HERE.parent/'crates/sex-pdx/src/text_batch.rs')+'"]pub mod text_batch;\n    #[path="'+str(HERE.parent/'crates/sex-pdx/src/app_catalog.rs')+'"]pub mod app_catalog;')
    for path,(_,constant_names)in historical.DEPENDENCIES.items():
        raw=output(['git','show',PIN+':'+path]);chosen=[]
        for name in constant_names:
            matches=re.findall(r'(?m)^pub const\s+'+re.escape(name)+r'\s*:[^;]+;',raw);assert len(matches)==1,(path,name);chosen.append(matches[0])
        if path.endswith('sex-pdx/src/lib.rs'):
            prefix=prefix.replace('/* PDX_CONSTANTS */','\n'.join(chosen))
        else:prefix=prefix.replace('/* TOKEN_CONSTANTS */','\n'.join(chosen))
    modules=''
    for name in ('linen_async','linen_text_delivery'):
        path=HERE.parent/('servers/silk-shell/src/'+name+'.rs');raw=path.read_text();assert raw==output(['git','show',PIN+':servers/silk-shell/src/'+name+'.rs']);bound=w/(name+'.rs');bound.write_text(raw+'\nuse crate::sex_pdx;\n');modules+='#[path='+json.dumps(str(bound))+']mod '+name+';\n'
    desktop=(w/'desktop.rs');raw=output(['git','show',PIN+':servers/silk-shell/src/desktop_content.rs']);decl=declarations(raw);pieces=[decl['Item'],decl['List']]
    for pattern in (r'(?m)^impl List\b',r'(?m)^pub fn list\b'):
        m=re.search(pattern,raw);pieces.append(raw[m.start():item_end(raw,m.start())])
    desktop.write_text('\n'.join(pieces));modules+='#[path='+json.dumps(str(desktop))+']mod desktop_content;\n'
    modules+='#[path='+json.dumps(str(HERE.parent/'servers/silk-shell/src/document_policy.rs'))+']mod document_policy;\n'
    rust=(HERE/'tests/silk_shell_models/oracle.rs').read_text()
    rust=re.sub(r'(?m)^\s*"ascii" =>[^\n]*\n','',rust);rust=re.sub(r'(?m)^\s*"sync" =>[^\n]*\n','',rust)
    rust=rust.replace('unsafe fn reset() {','unsafe fn reset() {\n SURFACE_200_X=0;SURFACE_200_Y=0;LINEN_TEXT=linen_text_delivery::Outbox::atomic(SURFACE_ID_LINEN);LINEN_ASYNC=linen_async::Snapshot::new();')
    rust=rust.replace('"grants" => collar_init_grants(),','"published" => LINEN_REMOTE_REAL=number(&mut words)!=0,\n                "grants" => collar_init_grants(),')
    hooks='''
#[macro_export]macro_rules! serial_record {($($arg:tt)*)=>{println!($($arg)*)};}
unsafe fn set_frame_top_bar(id:u32,value:bool){println!("peer topbar {} {}",id,value as u8);}
unsafe fn send_frame_tab_info(id:u32){println!("peer tabinfo {}",id);}
fn startup_messages_pending()->bool{false}
fn pdx_call_checked(s:u64,o:u64,a:u64,b:u64,c:u64)->Result<(),u64>{let (status,_)=sex_pdx::pdx_call(s,o,a,b,c);if status==0{Ok(())}else{Err(status)}}
'''
    # Current PdxMessage is needed by the asynchronous reply body, but the
    # historical scripted receive queue retains its separate HostMessage type.
    pdx=output(['git','show',PIN+':crates/sex-pdx/src/lib.rs']);message=declarations(pdx)['PdxMessage'];prefix=prefix.replace('mod sex_pdx {','mod sex_pdx {\n'+message)
    library=output(['git','show',PIN+':servers/silk-shell/src/lib.rs']).replace('#![no_std]','')
    text=library+prefix+modules+'\n'.join(selected.values())+'\n'+'\n'.join(functions)+hooks+rust
    old_gap='let mut text = [0u8; (LINEN_UI_ROW_COUNT + 1) * 20];';assert text.count(old_gap)==1;text=text.replace(old_gap,"let mut text = [b' '; (LINEN_UI_ROW_COUNT + 1) * 20];")
    (w/'oracle.rs').write_text(text)
    c=(HERE/'tests/silk_shell_models/provider.c').read_text();c=c.replace('#include "silk_shell.h"','#include "silk_shell_models_root.h"\n#include "linen_capture_transport.h"')
    c='\n'.join(line for line in c.splitlines()if 'linen_sync_reply__LINEN_SYNC_NONREPLY_BUDGET'not in line)+'\n'
    c=c.replace('static void clear_state(void)\n{','static void clear_state(void)\n{\n')
    c=c.replace('    message_count = 0;','    silk_shell_models_runtime_init();\n    message_count = 0;')
    c=c.replace('void sexos_fail(void) { abort(); }','void sexos_fail(void) { fflush(stdout); abort(); }')
    c=re.sub(r'\} else if \(strcmp\(command, "ascii"\) == 0\) \{.*?(?=\} else if)', '',c,flags=re.S)
    c=re.sub(r'\} else if \(strcmp\(command, "sync"\) == 0\) \{.*?(?=\} else if)', '',c,flags=re.S)
    c=c.replace('} else if (strcmp(command, "grants") == 0) {','} else if (strcmp(command, "published") == 0) {\n            SILK_SHELL_STATE.models.LINEN_REMOTE_REAL=number()!=0;\n        } else if (strcmp(command, "grants") == 0) {')
    c+='''
bool startup_messages_pending(void){return false;}
void shell_chrome_set_frame_top_bar(ShellChromeContext *ctx,uint32_t id,bool value){(void)ctx;printf("peer topbar %u %u\\n",id,(unsigned)value);}
void shell_chrome_send_frame_tab_info(ShellChromeContext *ctx,uint32_t id){(void)ctx;printf("peer tabinfo %u\\n",id);}
ShellTextAdmission shell_text_display_admit(ShellTextOperation op){(void)op;return(ShellTextAdmission){SHELL_TEXT_ACCEPTED,0};}
LinenCaptureStep linen_capture_transport_step(LinenCaptureClient *state,uint64_t slot){(void)state;(void)slot;return(LinenCaptureStep){0};}
'''
    (w/'provider.c').write_text(c)
    return dict(pin=PIN,functions=names,functions_sha256={name:hashlib.sha256(body.encode()).hexdigest()for name,body in zip(names,functions)},main_sha256=hashlib.sha256(source.encode()).hexdigest(),declarations=sorted(selected),named_deviation='Static fallback gaps are spaces; original NUL assertion is retained in the composed renderer gate.')


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/models-current-tests');parser.add_argument('--quick',action='store_true');args=parser.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True)
    summary=prepare(w);raw=w/'old-vectors.txt';historical.vectors(raw,1 if args.quick else 4,16 if args.quick else 256)
    # These operations no longer exist upstream. Current list sanitization and
    # asynchronous receive are qualified by their corresponding real bodies.
    rows=[];present=set();boot_position=0;relocated=0
    for line in raw.read_text().splitlines():
        words=line.split();op=words[0]
        if op in ('ascii','sync'):continue
        if op=='reset':present=set(range(6));boot_position=len(rows)+1
        elif op=='clear':present=set();boot_position=len(rows)+1
        elif op=='put':present.add(int(words[1]))
        elif op=='erase':present.discard(int(words[1]))
        if op=='grants'and len(present)==16:
            # Boot seed grants precede later publication/table changes. Keep
            # the full-table boot assertion as an explicit negative control.
            rows.insert(boot_position,line);relocated+=1
        else:rows.append(line)
    for subject in (200,202,300):
        for disk in (0,1,2,3,15,16):
            for published in (0,1):
                for state in (0,1,2):
                    for mask in (0,1,64,65):
                        rows+=['clear','put 0 900',f'disk 0 {disk}',f'published {published}',f'focus {subject}',f'grant 0 {subject} 18446744073709551615 {mask} {state}','policy 0 900 0','policy 6 900 0','open 900','audit-dump']
    for denied_slot,workspace_slot in ((0,31),(31,0)):
        for state in (1,2):
            rows+=['clear','put 0 900','disk 0 3','published 1','focus 200',f'grant {workspace_slot} 200 18446744073709551615 65 0',f'grant {denied_slot} 200 900 65 {state}','policy 0 900 0','policy 6 900 0','audit-dump']
    rows+=['clear','grant-generation 18446744073709551615','grants','grant-dump']
    rows+=['clear','grant 0 200 18446744073709551615 65 0','collar-render','grant-dump']
    vectors=w/'vectors.txt';vectors.write_text('\n'.join(rows)+'\n')
    exports=[]
    for name in UNITS:
        out=w/(name+'.c');subprocess.run([args.cesta,'export-c','src/'+name+'.cesta','--std=c11','-o',out,'--','-Igenerated/include'],cwd=HERE,check=True);exports.append(out)
    results=[];negative_controls=[]
    for profile,flags in (('default',()),('enforce',('SEXOS_COLLAR_ENFORCE_PROOF',))):
        variant=w/profile;variant.mkdir(exist_ok=True);oracle=variant/'rust';historical.run(['rustc','--edition=2021','-O','--cap-lints=allow',str(w/'oracle.rs'),'-o',str(oracle)],variant,'rust-build',flags=flags)
        expected=variant/'expected.trace';historical.run([str(oracle)],variant,'rust-run',stdin=vectors,stdout=expected,flags=flags)
        for cc in ('gcc','clang'):
            modes=(('O2',['-O2']),)if args.quick else(('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie']))
            for mode,options in modes:
                label=cc+mode;binary=variant/label;historical.run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-ffunction-sections','-fdata-sections',*options,'-Igenerated/include','-DSEXOS_LINEN_CORRELATED_REQUIRED=0',*(f'-D{flag}=1'for flag in flags),str(w/'provider.c'),*map(str,exports),'-Wl,--gc-sections','-o',str(binary)],variant,label+'-build')
                observed=variant/(label+'.trace');historical.run([str(binary)],variant,label+'-run',stdin=vectors,stdout=observed);historical.compare(expected,observed,profile+'/'+label);results.append(dict(profile=profile,compiler=cc,mode=mode,sha256=hashlib.sha256(observed.read_bytes()).hexdigest()))
                for case,commands in (
                    ('full_boot_grant_table',['clear',*[f'put {i} {i+1}'for i in range(16)],'grants']),
                    ('occupied_workspace_slot',['clear','grant 0 200 900 65 0','grants']),
                ):
                    data=('\n'.join(commands)+'\n').encode();ref=subprocess.run([oracle],input=data,capture_output=True);actual=subprocess.run([binary],input=data,capture_output=True)
                    assert ref.returncode!=0 and b'WORKSPACE_POLICY_SLOT_OCCUPIED'in ref.stderr,(case,ref.returncode,ref.stderr)
                    assert actual.returncode!=0 and actual.stdout==ref.stdout,(case,cc,mode,actual.returncode,actual.stdout[-600:],ref.stdout[-600:])
                    negative_controls.append(dict(profile=profile,compiler=cc,mode=mode,case=case))
    mutations=[];base=(w/'silk_shell_models.c').read_text()
    changes=(
        ('reserved_manifest_bit','app_manifest_unpack','0xFFFFFFFFFF000000','0xFFFFFFFFFE000000'),
        ('unknown_capability','app_capability_bits_validate','APP_CAP_KNOWN','UINT8_MAX'),
        ('wrong_project_color','linen_kind_color','0x004080C0','0x004080C1'),
        ('wrong_document_color','linen_kind_color','0x0040C080','0x0040C081'),
        ('wrong_text_color','quil_buffer_kind_color','0x00808080','0x00808081'),
        ('checksum_status_bit','pack_scene_settings_blob','0x7f','0xff'),
        ('short_checksum','unpack_scene_settings_blob','0x7f','0x3f'),
        ('accept_revoked_grant','collar_check_operation','grant->state!=COLLAR_GRANT_ACTIVE','grant->state>COLLAR_GRANT_TOMBSTONED'),
        ('accept_wrong_subject','collar_check_operation','grant->subject_id!=caller','false'),
        ('count_revoked_grants','collar_grant_count','COLLAR_GRANT_ACTIVE','COLLAR_GRANT_REVOKED'),
        ('wrong_audit_reason','record_collar_audit','.reason = reason','.reason = 0'),
        ('reverse_mesh_order','mesh_for_each_fact','MESH_FACT_RING_CAP - i','MESH_FACT_RING_CAP + i'),
        ('reverse_bell_order','bell_for_each_event','BELL_RING_CAP - i','BELL_RING_CAP + i'),
        ('wrong_reused_buffer','open_linen_object_in_quil','linked_id = buffer->buffer_id','linked_id = dynamic_id'),
        ('ignore_buffer_collision','open_linen_object_in_quil','buffer->buffer_id == dynamic_id','buffer->buffer_id == UINT64_MAX'),
        ('draw_extra_mesh_row','model_mesh_render_row','state->rows >= MESH_LIST_ROW_RECTS','state->rows > MESH_LIST_ROW_RECTS'),
        ('workspace_bypasses_denial','collar_check_operation','if(explicit_denial){','if(false){'),
        ('workspace_rejects_publication','collar_check_operation','o->present && shell_workspace_allows','false && shell_workspace_allows'),
        ('protected_slot_open','open_linen_object_in_quil','if(object.disk_path!=0 && object.disk_path<3)','if(false)'),
        ('wrong_workspace_label','collar_render_grants_text',"buffer[base+10]='W'","buffer[base+10]='X'"),
    )
    for name,function,old,new in changes:
        path=w/('mutant-'+name+'.c');path.write_text(historical.mutate_function(base,function,old,new));binary=w/('mutant-'+name);linked=[path if x.stem=='silk_shell_models'else x for x in exports]
        subprocess.run(['gcc','-std=c11','-O2','-ffunction-sections','-fdata-sections','-Igenerated/include',str(w/'provider.c'),*map(str,linked),'-Wl,--gc-sections','-o',binary],cwd=HERE,check=True)
        r=subprocess.run([binary],input=vectors.read_bytes(),capture_output=True);reference=(w/'default/expected.trace').read_bytes();assert r.returncode!=0 or r.stdout!=reference,('survived',name);mutations.append(name)
    summary.update(commands=len(rows),configurations=results,negative_controls=negative_controls,mutants_caught=mutations,boot_seed_calls_relocated_before_table_mutation=relocated,boundary='Actual current Models functions and global Linen adapters; scripted Chrome/display/other service peers. No Core event loop or boot.')
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Current Models:',len(summary['functions']),'actual function bodies,',len(rows),'commands/configuration,',len(results),'configurations')


if __name__=='__main__':main()
