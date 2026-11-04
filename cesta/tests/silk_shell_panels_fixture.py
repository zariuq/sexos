"""Bell and palette controls with explicit external subsystem peers."""
from pathlib import Path
import re

HERE=Path(__file__).resolve().parent.parent


def delivery_trace():
    return '''
impl Outbox {pub fn trace_palette(&self){
 fn batch(b:&Batch){print!(" {} {} {} {} {} {} {} {} {} {} ",b.len,b.color,b.offset,b.sequence,b.awaiting as u8,b.cleared as u8,b.metadata.count,b.metadata.first,b.metadata.rows,b.metadata.selected);for v in &b.bytes[..b.len]{print!("{:02x}",v)}}
 print!("T {} {} {} {} {}",self.surface,self.atomic as u8,self.sequence,self.active.is_some()as u8,self.latest.is_some()as u8);if let Some(b)=&self.active{batch(b)}print!("|");if let Some(b)=&self.latest{batch(b)}println!();
}}
'''


def prepare(rust,c,models):
    rust=rust.replace('unsafe fn bell_render_event_list(){println!("surface bell-render");}\n','')
    rust=rust.replace('unsafe fn bell_ring_count()->usize{println!("peer bell-count");3}\n','')
    rust=rust.replace('unsafe fn launcher_focus_discard(){println!("peer discard-launcher");}', 'unsafe fn launcher_focus_discard(){println!("peer discard-launcher");LAUNCHER_FOCUS=None;}')
    rust=rust.replace('(LAUNCH_AVAILABLE,"fixture","fixture")','if PANEL_STATUS==1{(LAUNCH_AVAILABLE,"RUNNING / FOCUS","original_incarnation_live")}else if PANEL_STATUS==2{(false,"CLOSED / REBOOT TO REOPEN","no_boot_successor")}else{(LAUNCH_AVAILABLE,"fixture","fixture")}')
    rust=rust.replace('unsafe fn native_launcher_status(target:u32)->(bool,&\'static str,&\'static str){println!(\"surface available {}\",target);', 'fn native_launcher_status(target:u32)->(bool,&\'static str,&\'static str){unsafe{println!(\"surface available {}\",target);').replace('(LAUNCH_AVAILABLE,\"fixture\",\"fixture\")}}\n','(LAUNCH_AVAILABLE,\"fixture\",\"fixture\")}}}\n')
    rs='''static mut PANEL_STATUS:u8=0;static mut PANEL_HISTORY:u8=0;static mut PANEL_TITLE:u8=0;
unsafe fn spindle_history_label(fallback:&'static str,_available:bool,_reason:&'static str)->&'static str{println!("surface history-label");if PANEL_HISTORY==1{"LOADING HISTORY"}else{fallback}}
fn launcher_row_title(command:Command,fallback:&'static str)->&'static str{println!("surface row-title {}",command as u8);unsafe{if PANEL_TITLE==1{"Ω\\nfixture"}else{fallback}}}
unsafe fn atlas_toggle(){println!("surface atlas-toggle");}
unsafe fn launcher_palette_closed(pending:Option<LauncherFocus>){if let Some(p)=pending{println!("surface palette-closed 1 {} {} {}",p.target,p.task,p.frame)}else{println!("surface palette-closed 0 0 0 0")}LAUNCHER_FOCUS=pending;}
fn display_admit(op:linen_text_delivery::Operation)->linen_text_delivery::Admission{let(status,_)=pdx_call(SLOT_DISPLAY,op.opcode,op.surface,op.packed,op.control);if status==0{linen_text_delivery::Admission::Accepted}else if status==sex_pdx::ERR_SERVICE_NOT_READY{linen_text_delivery::Admission::Full}else{linen_text_delivery::Admission::Rejected(status)}}
unsafe fn panel_fixture_reset(){PANEL_STATUS=0;PANEL_HISTORY=0;PANEL_TITLE=0;BELL_RING_WRITE_INDEX=0;BELL_EVENTS=[None;BELL_RING_CAP];LINEN_OBJECTS=[None;LINEN_MAX_OBJECTS];BELL_SELECTED_ROW=0;BELL_SELECTED_LANE=0;BELL_DETAIL_OPEN=false;COMMAND_PALETTE_SELECTED=0;COMMAND_PALETTE_OPEN=false;PALETTE_TEXT=linen_text_delivery::Outbox::atomic(SURFACE_ID_COMMAND_PALETTE);LAUNCHER_FOCUS=None;}
unsafe fn panel_fixture_state(){print!("B {} {} {} {}",BELL_RING_WRITE_INDEX,BELL_SELECTED_ROW,BELL_DETAIL_OPEN as u8,BELL_SELECTED_LANE);print!("|C {} {}",COMMAND_PALETTE_OPEN as u8,COMMAND_PALETTE_SELECTED);if let Some(p)=LAUNCHER_FOCUS{print!("|L 1 {} {} {}",p.target,p.task,p.frame)}else{print!("|L 0")}println!();PALETTE_TEXT.trace_palette();}
'''
    cs='''static uint8_t panel_status,panel_history,panel_title;
static ShellPaletteString panel_fixture_label(const char *text){ShellPaletteString result={0};while(result.len<127&&text[result.len]){result.bytes[result.len]=text[result.len];++result.len;}return result;}
ShellPaletteStatus shell_surfaces_peer_native_launcher_status(ShellSurfacesContext *c,uint32_t target){bool available=shell_surfaces_peer_native_launcher_available(c,target);if(panel_status==1)return(ShellPaletteStatus){available,panel_fixture_label("RUNNING / FOCUS"),panel_fixture_label("original_incarnation_live")};if(panel_status==2)return(ShellPaletteStatus){false,panel_fixture_label("CLOSED / REBOOT TO REOPEN"),panel_fixture_label("no_boot_successor")};return(ShellPaletteStatus){available,panel_fixture_label("fixture"),panel_fixture_label("fixture")};}
ShellPaletteString shell_surfaces_peer_spindle_history_label(ShellSurfacesContext *c,ShellPaletteString fallback,bool available,ShellPaletteString reason){(void)c;(void)available;(void)reason;puts("surface history-label");return panel_history==1?panel_fixture_label("LOADING HISTORY"):fallback;}
ShellPaletteString shell_surfaces_peer_launcher_row_title(ShellSurfacesContext *c,uint8_t command,ShellPaletteString fallback){(void)c;printf("surface row-title %u\\n",command);return panel_title==1?panel_fixture_label("Ω\\nfixture"):fallback;}
bool shell_surfaces_peer_send_silkbar_phase2_update(ShellSurfacesContext *c,uint32_t kind,uint64_t a,uint64_t b){return shell_chrome_peer_send_silkbar_phase2_update(&c->shell,kind,a,b);}
void shell_surfaces_peer_atlas_toggle(ShellSurfacesContext *c){(void)c;puts("surface atlas-toggle");}
void shell_surfaces_peer_launcher_palette_closed(ShellSurfacesContext *c,ShellLauncherFocus pending){if(pending.present)printf("surface palette-closed 1 %u %u %" PRIu64 "\\n",pending.target,pending.task,pending.frame);else puts("surface palette-closed 0 0 0 0");c->surfaces->LAUNCHER_FOCUS=pending;}
ShellTextAdmission shell_text_display_admit(ShellTextOperation op){SexosReply r=shell_surfaces_peer_pdx_call(&surface_ctx,SLOT_DISPLAY,op.opcode,op.surface,op.packed,op.control);return(ShellTextAdmission){r.status==0?SHELL_TEXT_ACCEPTED:r.status==UINT64_C(0xfffffffffffffffe)?SHELL_TEXT_FULL:SHELL_TEXT_REJECTED,r.status==0||r.status==UINT64_C(0xfffffffffffffffe)?0:r.status};}
static void panel_fixture_reset(void){panel_status=0;panel_history=0;panel_title=0;surfaces.PALETTE_TEXT=shell_text_new(SURFACE_ID_COMMAND_PALETTE,true);}
static void panel_fixture_batch(const ShellTextBatch *b){printf(" %zu %u %zu %u %u %u %zu %zu %zu %" PRIu64 " ",b->text.len,b->color,b->offset,b->sequence,b->awaiting,b->cleared,b->metadata.count,b->metadata.first,b->metadata.rows,b->metadata.selected);for(size_t i=0;i<b->text.len;++i)printf("%02x",b->text.bytes[i]);}
static void panel_fixture_state(void){printf("B %" PRIu64 " %u %u %u|C %u %u",models.BELL_RING_WRITE_INDEX,surfaces.BELL_SELECTED_ROW,surfaces.BELL_DETAIL_OPEN,surfaces.BELL_SELECTED_LANE,surfaces.COMMAND_PALETTE_OPEN,surfaces.COMMAND_PALETTE_SELECTED);if(surfaces.LAUNCHER_FOCUS.present)printf("|L 1 %u %u %" PRIu64,surfaces.LAUNCHER_FOCUS.target,surfaces.LAUNCHER_FOCUS.task,surfaces.LAUNCHER_FOCUS.frame);else printf("|L 0");puts("");ShellTextOutbox *o=&surfaces.PALETTE_TEXT;printf("T %" PRIu64 " %u %u %u %u",o->surface,o->atomic,o->sequence,o->active,o->latest);if(o->active)panel_fixture_batch(&o->current);putchar('|');if(o->latest)panel_fixture_batch(&o->desired);putchar(10);}
'''
    cr='''
        500=>panel_fixture_state(),
        501=>{BELL_RING_WRITE_INDEX=n(1);BELL_SELECTED_ROW=n(2)as u8;BELL_SELECTED_LANE=n(3)as u8;SURFACE_204_W=n(4)as u32;SURFACE_204_H=n(5)as u32;},
        502=>BELL_EVENTS[n(1)as usize]=if n(2)==0{None}else{Some(BellEvent{event_id:n(3),kind:match n(4){0=>BellEventKind::ObjectLinkedToBuffer,1=>BellEventKind::ObjectOpenRequested,2=>BellEventKind::OperationNeedsGrant,_=>BellEventKind::DiagnosticOnly},object_id:n(5),buffer_id:n(6),sequence:n(7)})},
        503=>{LINEN_OBJECTS[n(1)as usize]=Some(LinenObject{object_id:n(2),kind:match n(3){0=>LinenObjectKind::Project,1=>LinenObjectKind::Document,2=>LinenObjectKind::CodeFile,3=>LinenObjectKind::MediaAsset,4=>LinenObjectKind::BuildArtifact,5=>LinenObjectKind::Folder,6=>LinenObjectKind::Reference,7=>LinenObjectKind::ImportPlaceholder,8=>LinenObjectKind::BellEventReference,9=>LinenObjectKind::QuilWorkspaceReference,_=>LinenObjectKind::MeshDiagnosticReference},state:LinenObjectState::Allocated,parent_id:0,project_id:0,grant_ref:0,linked_surface_id:0,flags:0,display_name:"fixture",name:[0;24],name_len:0,disk_path:0});},
        504=>if let Some(ev)=bell_selected_event_snapshot(){println!("E 1 {} {} {} {} {}",ev.event_id,ev.kind as u8,ev.object_id,ev.buffer_id,ev.sequence)}else{println!("E 0")},
        505=>println!("R {}",bell_selected_row_highlight(n(1)as u32)),
        510=>{COMMAND_PALETTE_SELECTED=n(1)as u8;COMMAND_PALETTE_OPEN=n(2)!=0;PANEL_STATUS=n(3)as u8;PANEL_HISTORY=n(4)as u8;PANEL_TITLE=n(5)as u8;},
        511=>for(i,row)in COMMAND_LIST.iter().enumerate(){println!("C {} {} {}",i,row.command as u8,row.name);},
        512=>{let(a,b,c)=palette_item_status(COMMAND_LIST[n(1)as usize].command);println!("U {} {} {}",a as u8,b,c);},
        513=>{let p=PALETTE_TEXT.poll(n(1)as usize,display_admit);println!("P {}",match p{linen_text_delivery::Poll::Idle=>0,linen_text_delivery::Poll::Pending=>1,linen_text_delivery::Poll::Deferred=>2,linen_text_delivery::Poll::Complete(_)=>3,linen_text_delivery::Poll::Rejected(_)=>4});},
        514=>{let r=PALETTE_TEXT.receive(n(1)as u32,n(2));println!("R {}",r.is_some()as u8);},
        516=>palette_batch_emit_app_focus("fixture",n(1),n(2)!=0,"accepted"),
        517=>println!("R {}",command_kind_color(COMMAND_LIST[n(1)as usize].command)),
        515=>LAUNCHER_FOCUS=if n(1)==0{None}else{Some(LauncherFocus{target:n(2)as u32,task:n(3)as u32,frame:n(4),action:launcher::LiveAction::Focus,from:0,to:0,expected_focus:SURFACE_ID_COMMAND_PALETTE,requested:false,turns:0,start:0,last:0})},
'''
    cc='''
        case 500:panel_fixture_state();break;
        case 501:models.BELL_RING_WRITE_INDEX=n[1];surfaces.BELL_SELECTED_ROW=n[2];surfaces.BELL_SELECTED_LANE=n[3];input.SURFACE_204_W=n[4];input.SURFACE_204_H=n[5];break;
        case 502:models.BELL_EVENTS[n[1]]=(BellEvent){n[2]!=0,n[3],n[4],n[5],n[6],n[7]};break;
        case 503:models.LINEN_OBJECTS[n[1]]=(LinenObject){.present=true,.object_id=n[2],.kind=n[3],.display_name="fixture"};break;
        case 504:{BellEvent e={0};if(shell_surfaces_bell_selected_event_snapshot(&surface_ctx,&e))printf("E 1 %" PRIu64 " %u %" PRIu64 " %" PRIu64 " %" PRIu64 "\\n",e.event_id,e.kind,e.object_id,e.buffer_id,e.sequence);else puts("E 0");break;}
        case 505:printf("R %u\\n",shell_surfaces_bell_selected_row_highlight(n[1]));break;
        case 510:surfaces.COMMAND_PALETTE_SELECTED=n[1];surfaces.COMMAND_PALETTE_OPEN=n[2];panel_status=n[3];panel_history=n[4];panel_title=n[5];break;
        case 511:for(size_t i=0;i<11;++i)printf("C %zu %u %s\\n",i,shell_surfaces_command_code(i),shell_surfaces_command_label(i).bytes);break;
        case 512:{ShellPaletteStatus s=shell_surfaces_palette_item_status(&surface_ctx,n[1]);printf("U %u %s %s\\n",s.available,s.status.bytes,s.reason.bytes);break;}
        case 513:{ShellTextPoll r=shell_text_poll(&surfaces.PALETTE_TEXT,n[1]);printf("P %u\\n",r.kind);break;}
        case 514:{ShellTextReceipt r={0};printf("R %u\\n",shell_text_receive(&surfaces.PALETTE_TEXT,n[1],n[2],&r));break;}
        case 516:{const ShellPaletteString app={.bytes="fixture",.len=7},reason={.bytes="accepted",.len=8};shell_surfaces_palette_batch_emit_app_focus(&surface_ctx,&app,n[1],n[2],&reason);break;}
        case 517:printf("R %u\\n",shell_surfaces_command_kind_color(shell_surfaces_command_code(n[1])));break;
        case 515:surfaces.LAUNCHER_FOCUS=(ShellLauncherFocus){.present=n[1]!=0,.target=n[2],.task=n[3],.frame=n[4],.action=3,.expected_focus=SURFACE_ID_COMMAND_PALETTE};break;
'''
    operations=[('bell_visible_event_count',True),('bell_select_next_row',False),('bell_select_prev_row',False),('bell_emit_selected_event_detail_proof',False),('bell_close_detail',False),('bell_cycle_lane',True),('bell_is_visible_in_active_scene',True),('bell_render_event_list',False),('bell_render_lane_text',False),('command_palette_selected_accent',True),('palette_render_list',False),('palette_show',False),('toggle_command_palette',True),('palette_select_next',False),('palette_select_prev',False),('palette_execute_selected',True),('open_browser_in_active_scene',True)]
    cr+='        506=>match n(1){\n';cc+='        case 506:switch(n[1]){\n'
    for i,(name,result)in enumerate(operations):
        cr+=f'            {i}=>'+(f'println!("R {{}}",{name}()as u64)'if result else name+'()')+',\n'
        cc+=f'            case {i}:'+(f'printf("R %" PRIu64 "\\n",(uint64_t)shell_surfaces_{name}(&surface_ctx));'if result else f'shell_surfaces_{name}(&surface_ctx);')+'break;\n'
    cr+='            _=>panic!("panel operation")},\n';cc+='            default:abort();}break;\n'
    rust=rs+rust.replace('unsafe fn reset(){','unsafe fn reset(){panel_fixture_reset();').replace('        100=>reset(),',cr+'        100=>reset(),')
    c=c.replace('void shell_surfaces_peer_bell_render_event_list(ShellSurfacesContext *c){(void)c;puts("surface bell-render");}\n','')
    c=c.replace('shell_chrome_peer_launcher_focus_discard(ShellChromeContext *c){(void)c;puts("peer discard-launcher");}','shell_chrome_peer_launcher_focus_discard(ShellChromeContext *c){(void)c;puts("peer discard-launcher");surfaces.LAUNCHER_FOCUS=(ShellLauncherFocus){0};}')
    c=c.replace('shell_chrome_peer_bell_ring_count(ShellChromeContext *c){(void)c;puts("peer bell-count");return 3;}','shell_chrome_peer_bell_ring_count(ShellChromeContext *c){return c->models->BELL_RING_WRITE_INDEX<BELL_RING_CAP?(size_t)c->models->BELL_RING_WRITE_INDEX:BELL_RING_CAP;}')
    c=c.replace('return launch_available;}','return panel_status==2?false:launch_available;}')
    # The peer's status mode must be declared before the existing availability adapter.
    c=c.replace('static ShellActivationRaw activations[512];','static uint8_t panel_status,panel_history,panel_title;\nstatic ShellActivationRaw activations[512];')
    cs=cs.replace('static uint8_t panel_status,panel_history,panel_title;\n','')
    c=c.replace('static void reset(void) {',cs+'\n'+models+'\nstatic void reset(void) {')
    c=c.replace('    spindle_fixture_reset();','    panel_fixture_reset();spindle_fixture_reset();')
    c=c.replace('        case 100:',cc+'        case 100:')
    return rust,c


def corpus():
    rows=[]
    def add(*n):rows.append(' '.join(str(x&((1<<64)-1))for x in n))
    add(511)
    for index in range(11):add(517,index)
    for sid in (0,201,300):add(516,sid,1)
    for total in (0,1,2,7,15,16,17,33,0xffffffffffffffff):
        for selected in (0,1,6,7,15,255):
            add(100);add(501,total,selected,0,400,240)
            for at in range(min(total,16)):
                index=(total-1-at)%16;add(502,index,1,total-1-at,at%4,at+1,at+101,at)
            add(503,0,1,1);add(504)
            for operation in (0,7,1,2,3,4,5,8):add(506,operation);add(500)
            add(103,204,0,0,0,0,1280,720);add(506,3);add(506,4);add(500)
    for color in (0,0x010203,0x405060,0x7fffff,0xffffff,0xffffffff):add(505,color)
    for index in (*range(11),11,254,255):
        for state in (0,1,2,3):
            for status in (0,1,2):
                add(100);add(18);add(282,0,state);add(282,1,state);add(282,2,state);add(510,index,1,status,0,0)
                add(506,9);add(506,10);add(500)
                if index<11:add(512,index)
                add(506,15);add(500)
    for history,title in ((0,0),(1,0),(0,1)):
        add(100);add(18);add(282,0,2);add(282,1,2);add(282,2,2);add(510,0,1,1,history,title)
        for operation in (10,13,14):add(506,operation);add(500)
        add(513,100);add(500);add(514,2,0x56|(152<<8)|(1<<24));add(514,1,0x56|(152<<8)|(1<<24));add(500)
    for flags in (0,1,2):
        add(100);add(18);add(8,152,0);add(101,0,7,0,1,0,flags,152);add(515,1,11,701,201);add(510,3,1,0,0,0);add(506,12);add(500)
        add(506,12);add(500)
    add(100);add(18);add(8,152,2);add(510,0,1,0,0,0);add(506,10);add(506,10);add(506,12);add(500)
    for index in (0,5,10,255):add(100);add(510,index,1,0,0,0);add(506,13);add(500);add(506,14);add(500)
    return rows


def palette_lifecycle_controls(binary,run,source=False):
    count=0;counterexamples=0
    def observe(commands):return run([binary],('\n'.join(commands)+'\n').encode()).decode()
    capacity=observe(['100','401']);assert ('K LIFECYCLE_MAX_SURFACES '+('43'if source else '44'))in capacity,capacity[-1000:]
    count+=1
    out=observe(['100','206','9 152'])
    if source:
        assert out.splitlines()[-2]=='R 0 0',out[-1000:]
        counterexamples+=1
    else:assert out.splitlines()[-2]=='R 1 0',out[-1000:]
    count+=1
    out=observe(['100','287','510 5 0 0 0 0','506 12','500'])
    if source:
        assert '|C 1 0'in out and'|F 7 'in out and'|P 1 236 152'in out,out[-1200:]
        counterexamples+=1
    else:
        assert '[shell.palette.open.reject]'in out and'|C 0 5'in out and'|F 7 'not in out and'|P 0'in out,out[-1200:]
        assert not any(line.startswith(('call ','peer bar-update'))for line in out.splitlines()),out[-1200:]
    count+=1
    table=(HERE/'interfaces/shell_apps.data').read_text().split('(Table BootLifecycle')[0]
    rows=[line.split()for line in re.findall(r'\(Row ([^\n)]+)\)',table)]
    native=[int(row[4])or int(row[3])for row in rows if int(row[1])>0 or int(row[0])==2]
    assert len(native)==len(set(native))==26,native
    commands=['100','18']+[f'8 {surface} 0'for surface in native]+['206','9 152']+[f'9 {surface}'for surface in native]
    out=observe(commands);results=[line for line in out.splitlines()if line.startswith('R ')]
    assert results[:26]==['R 1']*26,results
    assert results[26]=='R 1 7'and results[28:]==['R 1 0']*26,results
    if source:assert results[27]=='R 0 0',results;counterexamples+=1
    else:assert results[27]=='R 1 0',results
    count+=1
    return dict(controls=count,counterexamples=counterexamples)if source else count
