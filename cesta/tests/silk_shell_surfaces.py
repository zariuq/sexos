"""Current Surfaces bodies with real Chrome/custody/viewport dependencies."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import silk_shell_chrome as chrome
from silk_shell_input_differential import item_end

HERE=Path(__file__).resolve().parent.parent
FIX=HERE/'tests/silk_shell_chrome'
PEERS=HERE/'tests/silk_shell_surfaces'
MANIFEST=HERE/'interfaces/silk_shell_surfaces.source.json'
CESTA_FOR_FIXTURE='cesta'
FAMILIES=('linen','quil','mesh','collar','spindle','bell','command_palette')
FRAME=(2,3,4,5,9,6,7)


def prepare(work,inventory):
    base=json.loads((HERE/'interfaces/silk_shell_chrome.source.json').read_text())
    assert (base['pin'],base['source_sha256'])==(inventory['pin'],inventory['source_sha256'])
    combined=dict(base);combined['functions']=base['functions']+inventory['functions']
    combined['dependencies']={**base['dependencies'],**inventory.get('dependencies',{})}
    import silk_shell_panels_fixture as panels
    combined['full_text_delivery']=True;combined['delivery_trace']=panels.delivery_trace()
    source=chrome.run(['git','show',inventory['pin']+':servers/silk-shell/src/main.rs']).decode()
    for name in ('bell_ring_count','bell_for_each_event','linen_object_by_id','linen_kind_color','quil_launcher_status','spindle_launcher_status','browser_launcher_status','shell_draw_text'):
        match=re.search(r'(?m)^(?:unsafe )?fn '+name+r'\b',source);assert match,name
        raw=source[match.start():item_end(source,match.start())]+'\n'
        combined['functions'].append(dict(name=name,sha256=hashlib.sha256(raw.encode()).hexdigest()))
    rust=(FIX/'provider.rs').read_text()
    cases=''.join(f'        {200+i}=>optional(ensure_{name}_frame().map(|f|f as u64)),\n'for i,name in enumerate(FAMILIES))
    cases+=''.join(f'        {210+i}=>optional({name}_frame_id().map(|f|f as u64)),\n'for i,name in enumerate(('linen','quil','mesh','collar','bell')))
    cases+=''.join(f'        {220+i}=>println!("R {{}}",open_{name}_in_active_scene()as u8),\n'for i,name in enumerate(FAMILIES[:6]))
    cases+=''.join(f'        {230+i}=>println!("R {{}}",focus_or_open_{name}()as u8),\n'for i,name in enumerate(FAMILIES[:6])if i!=4)
    cases+=''.join(f'        {240+i}=>println!("R {{}}",toggle_{name}()as u8),\n'for i,name in enumerate(FAMILIES[:6]))
    cases+='        260|261=>{let r=app_activation_info(if n(1)==0{app_catalog::App::Quil}else if n(1)==1{app_catalog::App::Spindle}else{app_catalog::App::Browser},n(0)==261);match r{Ok(i)=>println!("A 0 {} {} {} {}",i.successor,i.task,i.surface,i.state),Err(e)=>println!("A {} 0 0 0 0",e)}},\n        264=>if let Some((target,frame))=spindle_active_frame(){println!("R 1 {} {}",target,frame)}else{println!("R 0")},\n        265=>open_app_in_active_scene_by_sid(n(1)),\n        280=>ACTIVATIONS[n(1)as usize]=(n(2),n(3),n(4),n(5),n(6)),\n        281=>{LAUNCH_AVAILABLE=n(1)!=0;LAUNCH_FOCUS=if n(2)!=0{Some(n(3)!=0)}else{None};ACCESS_DECISION=n(4)as u8;STALE_STATUS=n(5);},\n        282=>{for row in app_catalog::INCARNATIONS.iter().filter(|row|row.app as u8==n(1)as u8&&row.ordinal!=0){let i=row.ordinal as usize;let mut entry=app_custody::Entry::new(row.lifecycle_slot,row.target,700+i as u32,row.surface);entry.bounds=Some((10,60,400,250));let at=app_catalog::INCARNATIONS.iter().position(|r|r.target==row.target).unwrap();APP_CUSTODY[at]=Some(entry);ACTIVATIONS[row.activation_slot.unwrap()]=(0,row.target as u64,700+i as u64,row.surface,n(2));}},\n'
    rust=rust.replace('unsafe fn toggle_bell()->bool{println!("peer toggle-bell");CALL_STATUS==0}\n','')
    rust=(PEERS/'peers.rs').read_text().replace('LAUNCHER_PATH',str(chrome.ROOT/'servers/silk-shell/src/launcher.rs'))+'\n'+rust
    rust=rust.replace('unsafe fn reset(){','unsafe fn reset(){\n    ACTIVATIONS=[(3,0,0,0,0);512];LAUNCH_AVAILABLE=true;LAUNCH_FOCUS=None;ACCESS_DECISION=0;STALE_STATUS=0xfffffffffffffffe;LINEN_REMOTE_FETCHED=false;COLLAR_OVERLAY_ENABLED=false;SPINDLE_ACTIVE_SESSION=0;')
    rust=rust.replace('ACTIVATIONS=[(3,0,0,0,0);512];LAUNCH_AVAILABLE','REPLAY_CORRUPT=false;COMMIT_COUNT=[0;512];ACTIVATIONS=[(3,0,0,0,0);512];LAUNCH_AVAILABLE')
    cases+='        285=>REPLAY_CORRUPT=n(1)!=0,\n'
    cases+='        286=>println!("R {}",handle_app_surface_req(n(1),n(2),n(3),n(4)as u32)as u8),\n        287=>{for i in 0..LIFECYCLE_MAX_SURFACES{LIFECYCLE_TABLE[i]=Some((1000+i as u64,SurfaceLifecycle{state:LifecycleState::Visible,generation:1}));}},\n'
    rust=rust.replace('    println!();\n}\nfn optional','    print!("|S {} {} {}",LINEN_REMOTE_FETCHED as u8,COLLAR_OVERLAY_ENABLED as u8,SPINDLE_ACTIVE_SESSION);\n    println!();\n}\nfn optional')
    rust=rust.replace('        100=>reset(),',cases+'        100=>reset(),')
    provider=work/'provider.rs';provider.write_text(rust)
    # Keep the real activation result validation; replace only its syscall.
    lifecycle=(chrome.ROOT/'crates/sex-pdx/src/app_lifecycle.rs').read_text()
    marker='pub fn activation(slot:usize,commit:bool)'
    begin=lifecycle.index(marker);tail=lifecycle[begin:]
    a=tail.index('unsafe{core::arch::asm!');b=tail.index('options(nostack));}',a)+len('options(nostack));}')
    lifecycle=lifecycle[:begin]+tail[:a]+'(status,pd,task,surface,state)=crate::activation_raw(slot,commit);'+tail[b:]
    module=work/'app_lifecycle.rs';module.write_text(lifecycle)
    import silk_shell_spindle_fixture as spindle
    rust,_=spindle.prepare(rust,'')
    rust,_=panels.prepare(rust,'','')
    provider.write_text(rust)
    chrome.prepare(work,combined,provider,{'app_lifecycle':module},('AppCapabilityBits','AppManifest','SpindleResult'))
    # Compare the numeric declarations actually pulled into this reference,
    # so stale shared C constants fail before their downstream state traces.
    oracle=work/'oracle.rs';raw=oracle.read_text()
    macros=set(re.findall(r'(?m)^#define (\w+)\b',(HERE/'generated/include/silk_shell.h').read_text()))
    constants=sorted(set(re.findall(r'(?m)^(?:pub )?const (\w+):\s*(?:bool|[ui](?:8|16|32|64)|usize|isize)\b',raw))&macros)
    assert 'LIFECYCLE_MAX_SURFACES'in constants
    constants.remove('LIFECYCLE_MAX_SURFACES')
    audit='        401=>println!(\"K LIFECYCLE_MAX_SURFACES {}\",LIFECYCLE_MAX_SURFACES),\n        400=>{'+''.join('println!("K '+name+' {}",'+name+' as u64);'for name in constants)+'},\n'
    oracle.write_text(raw.replace('        100=>reset(),',audit+'        100=>reset(),'))
    chrome.run(['rustc','--edition=2021','-O',oracle,'-o',work/'rust'])
    (work/'constant-keys.json').write_text(json.dumps(constants,indent=2)+'\n')
    source=chrome.run(['git','show',inventory['pin']+':servers/silk-shell/src/main.rs']).decode()
    budgets=[]
    for row in inventory['functions']:
        m=re.search(r'(?m)^(?:unsafe )?fn '+row['name']+r'\b',source)
        raw=source[m.start():item_end(source,m.start())]
        for name,value in re.findall(r'\bstatic mut (\w+): u32 = (\d+);',raw):
            budgets.append((row['name']+'__'+name,int(value)))
    c=(FIX/'provider.c').read_text().replace('#include "silk_shell_chrome.h"','#include "silk_shell_surfaces.h"')
    c=c.replace('static FocusRef test_ref;','static SilkShellSurfacesState surfaces;\nstatic ShellSurfacesContext surface_ctx;\nstatic FocusRef test_ref;')
    initialize=''.join('surfaces.'+field+'='+str(value)+';'for field,value in budgets)
    reset=''.join('uint32_t saved'+str(i)+'=surfaces.'+field+';'for i,(field,_)in enumerate(budgets))
    reset+='bool session_init=surfaces.open_spindle_in_active_scene__SESSION_INIT_DONE;memset(&surfaces,0,sizeof(surfaces));surfaces.open_spindle_in_active_scene__SESSION_INIT_DONE=session_init;for(size_t i=0;i<512;++i)activations[i]=(ShellActivationRaw){.status=3};launch_available=true;launch_focus=(ShellSurfaceBool){0};access_decision=0;stale_status=UINT64_C(0xfffffffffffffffe);'
    reset+=''.join('surfaces.'+field+'=saved'+str(i)+';'for i,(field,_)in enumerate(budgets))
    reset+='replay_corrupt=false;memset(commit_count,0,sizeof(commit_count));'
    c=c.replace('static void reset(void) {',(PEERS/'peers.c').read_text()+'\nstatic void reset(void) {')
    c=c.replace('static void reset(void) {','static void reset(void) {\n    '+reset+'\n    surface_ctx=(ShellSurfacesContext){.shell=ctx,.surfaces=&surfaces};')
    c=c.replace('init_budgets();reset();','init_budgets();'+initialize+'reset();')
    cases=''.join(f'        case {200+i}:found=shell_surfaces_ensure_{name}_frame(&surface_ctx,&word);optional(found,word);break;\n'for i,name in enumerate(FAMILIES))
    cases+=''.join(f'        case {210+i}:found=shell_surfaces_{name}_frame_id(&surface_ctx,&word);optional(found,word);break;\n'for i,name in enumerate(('linen','quil','mesh','collar','bell')))
    cases+=''.join(f'        case {220+i}:printf("R %u\\n",shell_surfaces_open_{name}_in_active_scene(&surface_ctx));break;\n'for i,name in enumerate(FAMILIES[:6]))
    cases+=''.join(f'        case {230+i}:printf("R %u\\n",shell_surfaces_focus_or_open_{name}(&surface_ctx));break;\n'for i,name in enumerate(FAMILIES[:6])if i!=4)
    cases+=''.join(f'        case {240+i}:printf("R %u\\n",shell_surfaces_toggle_{name}(&surface_ctx));break;\n'for i,name in enumerate(FAMILIES[:6]))
    cases+='        case 260:case 261:{ShellActivationResult a=shell_surfaces_app_activation_info(&surface_ctx,n[1],n[0]==261);printf("A %" PRIu64 " %u %u %" PRIu64 " %" PRIu64 "\\n",a.status,a.info.successor,a.info.task,a.info.surface,a.info.state);break;}\n        case 264:{ShellSurfaceSubject a=shell_surfaces_spindle_active_frame(&surface_ctx);if(a.present)printf("R 1 %u %" PRIu64 "\\n",a.target,a.frame);else puts("R 0");break;}\n        case 265:shell_surfaces_open_app_in_active_scene_by_sid(&surface_ctx,n[1]);break;\n        case 280:activations[n[1]]=(ShellActivationRaw){n[2],n[3],n[4],n[5],n[6]};break;\n        case 281:launch_available=n[1];launch_focus=(ShellSurfaceBool){n[2]!=0,n[3]!=0};access_decision=n[4];stale_status=n[5];break;\n        case 282:for(size_t i=0;i<SHELL_APP_COUNT;++i){ShellAppCatalogRow row=shell_custody_catalog_at(i);if(row.app==n[1]&&row.ordinal){ShellAppCustody e=shell_custody_new(row.lifecycle_slot,row.target,700+row.ordinal,row.surface);e.has_bounds=true;e.x=10;e.y=60;e.w=400;e.h=250;apps.present[i]=true;apps.entries[i]=e;activations[row.activation_slot]=(ShellActivationRaw){0,row.target,700+row.ordinal,row.surface,n[2]};}}break;\n'
    c=c.replace('bool shell_chrome_peer_toggle_bell(ShellChromeContext *c){(void)c;puts("peer toggle-bell");return call_status==0;}','bool shell_chrome_peer_toggle_bell(ShellChromeContext *c){(void)c;return shell_surfaces_toggle_bell(&surface_ctx);}')
    c=c.replace('        case 100:',cases+'        case 100:')
    c=c.replace('        case 100:','        case 285:replay_corrupt=n[1];break;\n        case 100:')
    c=c.replace('        case 100:','        case 286:printf("R %u\\n",shell_surfaces_handle_app_surface_req(&surface_ctx,n[1],n[2],n[3],n[4]));break;\n        case 287:for(size_t i=0;i<LIFECYCLE_MAX_SURFACES;++i)models.LIFECYCLE_TABLE[i]=(LifecycleTableEntry){true,1000+i,{LIFECYCLE_VISIBLE,1}};break;\n        case 100:')
    audit='        case 401:printf(\"K LIFECYCLE_MAX_SURFACES %zu\\n\",LIFECYCLE_MAX_SURFACES);break;\n        case 400:'+''.join('printf("K '+name+' %" PRIu64 "\\n",(uint64_t)'+name+');'for name in constants)+'break;\n'
    c=c.replace('        case 100:',audit+'        case 100:')
    c=c.replace('    puts("");\n}\nstatic void optional','    printf("|S %u %u %zu",models.LINEN_REMOTE_FETCHED,models.COLLAR_OVERLAY_ENABLED,surfaces.SPINDLE_ACTIVE_SESSION);\n    puts("");\n}\nstatic void optional')
    _,c=spindle.prepare('',c)
    exported=work/'models-reference.c'
    chrome.run([CESTA_FOR_FIXTURE,'export-c',HERE/'src/silk_shell_models.cesta','--std=c11','-o',exported,'--','-I'+str(HERE/'generated/include')])
    body=exported.read_text();match=re.search(r'(?m)^uint32_t linen_kind_color\(',body);assert match
    pure=body[match.start():item_end(body,match.start())]+'\n'
    _,c=panels.prepare('',c,pure)
    (work/'provider.c').write_text(c)


def corpus():
    rows=[]
    def add(*args):rows.append(' '.join(str(n&chrome.MASK)for n in args))
    add(400)
    for family in range(7):
        for scene in range(5):
            for fill in range(10):
                add(100)
                if family==6:add(8,152,0)
                add(103,0,scene,0,0,0,1280,720)
                for slot in range(fill):add(101,slot,100+slot,0,1,0,0,100+slot)
                add(200+family);add(200+family)
                for query in range(5):add(210+query)
    for family in (1,4):
        index=0 if family==1 else 1
        for dead in (0,1):
            for closing in (0,1):
                add(100);add(105,index,77,dead,closing,1);add(200+family)
                add(101,0,FRAME[family],0,1,0,0,201 if family==1 else 153);add(200+family)
    surfaces=(200,201,202,203,153,204)
    for family,sid in enumerate(surfaces):
        for scene in range(5):
            for flags in range(8):
                for existing in (0,1):
                    add(100);add(18);add(103,0,scene,0,0,0,1280,720)
                    if family in (1,4):add(282,0 if family==1 else 1,0)
                    add(10,sid,4 if flags&1 else 2)
                    if existing:add(101,0,FRAME[family],0,1,scene,flags,sid)
                    add(220+family);add(220+family)
                    if family!=4:add(230+family)
                    add(240+family);add(240+family)
    for app in range(3):
        for state in range(4):
            add(100);add(18);add(282,app,state);add(260,app);add(261,app);add(260,app)
            if app<2:add(221 if app==0 else 224);add(241 if app==0 else 244)
    for app,opcode in ((0,221),(1,224)):
        for state in (0,1,2,3):
            for available in (0,1):
                for has,value in ((0,0),(1,0),(1,1)):
                    add(100);add(18);add(282,app,state)
                    add(281,available,has,value,0,chrome.MASK-1);add(opcode)
    for decision in range(6):add(100);add(18);add(281,1,0,0,decision,chrome.MASK-1);add(245)
    for sid in (0,200,201,202,203,205,300):add(100);add(18);add(265,sid)
    add(100);add(282,0,1);add(285,1);add(221)
    table=(HERE/'interfaces/shell_apps.data').read_text().split('(Table BootLifecycle')[0]
    for index,values in enumerate(re.findall(r'\(Row ([^\n)]+)\)',table)):
        row=values.split();target,sid=int(row[2]),int(row[3])
        for caps in (0,1,2,3,4,128):
            for caller in (target,777):
                add(100);add(105,index,777,0,0,1);add(286,sid,77,(target<<8)|caps,caller)
    from silk_shell_spindle_fixture import corpus as spindle_corpus
    rows.extend(spindle_corpus())
    from silk_shell_panels_fixture import corpus as panels_corpus
    rows.extend(panels_corpus())
    return ('\n'.join(rows)+'\n').encode(),len(rows)


def independent(binary,work):
    def observe(commands):
        out=chrome.run([binary], ('\n'.join(commands)+'\n').encode()).decode()
        return [x for x in out.splitlines()if x.startswith(('R ','A '))],out
    count=0
    for status,pd,task,surface,state,want in ((0,16,701,301,1,0),(2,16,701,301,1,2),
            (0,1<<32,701,301,1,1),(0,16,1<<32,301,1,1),(0,16,702,301,1,3),
            (0,16,701,302,1,3),(0,16,701,301,4,3)):
        values,out=observe(['100','282 0 1',f'280 44 {status} {pd} {task} {surface} {state}','260 0'])
        assert values[-1].split()[1]==str(want),(values,want)
        assert [x for x in out.splitlines()if x.startswith('surface activation ')]==['surface activation 44 0'],out[-500:]
        count+=1
    values,out=observe(['100','282 0 1','221'])
    calls=[x for x in out.splitlines()if x.startswith('surface activation ')]
    assert calls==['surface activation 44 0','surface activation 44 0','surface activation 44 1','surface activation 44 0','surface activation 44 1'],calls
    assert values[-1]=='R 1'and 'surface action-record 16 301 0 0 1 1'in out
    assert not any(x.startswith('call ')for x in out.splitlines()),out[-600:]
    count+=1
    values,out=observe(['100','282 0 1','285 1','221'])
    assert values[-1]=='R 0'and'surface action-record'not in out and'[shell.app.relaunch]'not in out,out[-700:]
    count+=1
    values,out=observe(['100','282 0 2','221'])
    assert values[-1]=='R 1'and'placeholder.attach'not in out and not any(x.startswith('call ')for x in out.splitlines()),out[-500:]
    count+=1
    for family,index in ((1,0),(4,1)):
        values,out=observe(['100',f'105 {index} 77 1 0 1',str(200+family)])
        assert values==['R 0 0']and'placeholder.attach'not in out,out[-500:]
        count+=1
    for decision in range(1,6):
        values,out=observe(['100','18',f'281 1 0 0 {decision} {chrome.MASK-1}','245'])
        assert values[-1]=='R 0'and not any(x.startswith('call ')for x in out.splitlines()),out[-500:]
        count+=1
    values,out=observe(['100','105 3 701 0 0 1','287','286 301 77 4096 16'])
    states=[x for x in out.splitlines()if x.startswith('S ')]
    assert values==['R 0']and states[-1]==states[-2]and not any(x.startswith('call ')for x in out.splitlines()),out[-700:]
    count+=1
    (work/(binary.name+'-independent.json')).write_text(json.dumps(dict(controls=count))+'\n')
    return count


def source_registration_control(binary,work):
    commands=['100','105 3 701 0 0 1','287','286 301 77 4096 16']
    out=chrome.run([binary], ('\n'.join(commands)+'\n').encode()).decode()
    assert 'R 1\n'in out and'|F 10 0 1'in out and'call 5 236 301 'in out
    (work/'source-untracked-registration.out').write_text(out)
    return 1


def main():
    global CESTA_FOR_FIXTURE
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-surfaces-tests')
    parser.add_argument('--mutations',action='store_true');args=parser.parse_args()
    CESTA_FOR_FIXTURE=args.cesta
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    inventory=json.loads(MANIFEST.read_text());prepare(work,inventory)
    data,count=corpus();(work/'commands.txt').write_bytes(data)
    expected=chrome.run([work/'rust'],data);(work/'rust.out').write_bytes(expected)
    from silk_shell_spindle_fixture import tail_control,boundary_controls
    registration=source_registration_control(work/'rust',work)
    source_tail=tail_control(work/'rust',chrome.run,source=True)
    source_controls=registration+source_tail['controls']
    source_counterexamples=registration+source_tail['counterexamples']
    from silk_shell_panels_fixture import palette_lifecycle_controls
    palette_source=palette_lifecycle_controls(work/'rust',chrome.run,source=True)
    source_controls+=palette_source['controls'];source_counterexamples+=palette_source['counterexamples']
    include='-I'+str(HERE/'generated/include');exports={}
    for unit in ('silk_shell_surfaces','silk_shell_commands','shell_text_delivery','shell_text_delivery_native','silk_shell_chrome','silk_shell_chrome_atomic','silk_shell_custody','silk_shell_viewport','silkbar_geometry','silkbar_model'):
        target=work/(unit+'.c');chrome.run([args.cesta,'export-c',HERE/'src'/(unit+'.cesta'),'--std=c11','-o',target,'--',include]);exports[unit]=target
    configurations=[]
    for cc in ('gcc','clang'):
        for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=work/(cc+'-'+mode);chrome.run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,'-I'+str(work),*exports.values(),work/'provider.c','-o',binary])
            observed=chrome.run([binary],data)
            if expected!=observed:
                (work/(binary.name+'.mismatch')).write_bytes(observed)
                a,b=expected.splitlines(),observed.splitlines();line=next((i for i,(x,y)in enumerate(zip(a,b))if x!=y),min(len(a),len(b)))
                left=a[line]if line<len(a)else b'';right=b[line]if line<len(b)else b''
                column=next((i for i,(x,y)in enumerate(zip(left,right))if x!=y),min(len(left),len(right)))
                raise AssertionError((cc,mode,line,column,left[max(0,column-50):column+150],right[max(0,column-50):column+150]))
            configurations.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(observed).hexdigest()))
            controls=independent(binary,work)+tail_control(binary,chrome.run)+boundary_controls(binary,chrome.run)+palette_lifecycle_controls(binary,chrome.run)
    mutants=[]
    if args.mutations:
        body=exports['silk_shell_surfaces'].read_text()
        for name,old,new in [
            ('ignore_custody','if(spec.guard_custody)','if(false)'),
            ('wrong_scene','.scene_id=ctx->shell.chrome->ACTIVE_SCENE_IDX','.scene_id=0'),
            ('wrong_tab','frame.tabs[0]=(ShellTab){true,spec.sid,0,0}','frame.tabs[0]=(ShellTab){true,spec.sid+1,0,0}'),
            ('replace_existing','if(surface_frame(ctx,spec.frame,out))','if(false)'),
            ('activation_skips_error','if(result.status)return result;','if(result.status)continue;'),
            ('activation_wrong_task','info.task!=entry.task','false'),
            ('activation_wrong_surface','info.surface!=entry.surface','false'),
            ('activation_forgets_replay','if(replay.status || !surface_activation_equal(active.info,replay.info))','if(false)'),
            ('registration_ignores_lifecycle_capacity','if(!shell_chrome_lifecycle_register(&ctx->shell,frame_surface.value,LIFECYCLE_VISIBLE))','if(false)'),
            ('session_keeps_oldest','src->sb_write+SPINDLE_SB_LINES-keep+i','src->sb_write+SPINDLE_SB_LINES-source_lines+i'),
            ('undo_forgets_bytes','ctx->surfaces->SPINDLE_VI_PREV_BUF[i]=ctx->surfaces->YARN.cmd_buf[i]','ctx->surfaces->SPINDLE_VI_PREV_BUF[i]=0'),
            ('history_draft_lost','ctx->surfaces->YARN.cmd_len=ctx->surfaces->SPINDLE_HIST_DRAFT_LEN','ctx->surfaces->YARN.cmd_len=0'),
            ('ghost_never_matches','bool match=true;','bool match=false;'),
            ('wrong_command_route','SPINDLE_TOKEN(command,token,"echo")','SPINDLE_TOKEN(command,token,"not-echo")'),
            ('cursor_header_lost','4+(16+(uint32_t)cursor)*8','4+(15+(uint32_t)cursor)*8'),
            ('spider_not_subsequence','(*candidate)[i]==(*query)[at]','(*candidate)[i]!=(*query)[at]'),
            ('spider_accept_forgets_cursor','ctx->surfaces->SPINDLE_VI_CUR=len;','ctx->surfaces->SPINDLE_VI_CUR=0;'),
            ('output_no_shift','ctx->surfaces->YARN.output_lines[i-1][j]=ctx->surfaces->YARN.output_lines[i][j]','ctx->surfaces->YARN.output_lines[i-1][j]=0'),
            ('palette_without_lifecycle','if(family==6 && !surface_palette_lifecycle(ctx))return false;','if(false)return false;'),
            ('palette_false_open','if(!panel_show_checked(ctx)){','if(false){'),
            ('palette_cancel_omitted','shell_text_cancel(&ctx->surfaces->PALETTE_TEXT);','(void)0;'),
            ('browser_not_opened','ok=shell_surfaces_open_browser_in_active_scene(ctx)','ok=false'),
            ('palette_no_reserve_observation','if(hint && app!=255)','if(false)'),
            ('palette_unsanitized_labels',"byte>=32 && byte<=126?byte:'?'","byte"),


        ]:
            assert old in body,(name,old);path=work/(name+'.c');path.write_text(body.replace(old,new));binary=work/(name+'.bin')
            linked=[path if key=='silk_shell_surfaces'else value for key,value in exports.items()]
            chrome.run(['gcc','-std=c11','-O2',include,'-I'+str(work),*linked,work/'provider.c','-o',binary])
            rejected=chrome.run([binary],data)!=expected
            if not rejected:
                try:independent(binary,work);tail_control(binary,chrome.run);boundary_controls(binary,chrome.run);palette_lifecycle_controls(binary,chrome.run)
                except AssertionError:rejected=True
            assert rejected,(name,'mutant survived');mutants.append(name)
    summary=dict(pin=inventory['pin'],functions=len(inventory['functions']),remaining=len(inventory['remaining']),commands=count,configurations=configurations,mutants=mutants,independent_controls=controls,source_controls=source_controls,source_counterexamples=source_counterexamples,
                 lifecycle_capacity=dict(rust=43,cesta=44,reason='one explicitly declared lazy palette descriptor'),
                 scope='Complete original Surfaces partition plus current activation/Browser/status helpers, real Bell and palette rendering, and the atomic text outbox. Display admits and other subsystem/focus-intent peers remain scripted. Original-source completion is not a joined current shell boot.')
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('Surfaces:',len(inventory['functions']),'entries;',count,'commands/configuration;',len(mutants),'mutations caught')


if __name__=='__main__':main()
