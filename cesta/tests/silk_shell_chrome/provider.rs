static mut POINT_GEOMETRY:bool=false;
static mut BOUNDS:[Option<(i32,i32,u32,u32)>;1024]=[None;1024];
static mut ATLAS_PICK:Option<u8>=None;
unsafe fn point_in_surface(x:i32,y:i32,sid:u64)->bool{
 println!("peer point {} {} {}",x,y,sid);
 if POINT_GEOMETRY{let(sx,sy,w,h)=BOUNDS[(sid%1024)as usize].unwrap_or(((sid%91)as i32,(sid%57)as i32,333,222));return FLAGS[(sid%1024)as usize]&1!=0 && x as i64>=sx as i64 && (x as i64)<sx as i64+w as i64 && y as i64>=sy as i64 && (y as i64)<sy as i64+h as i64;}
 match HIT{HitTarget::Surface(id)=>sid==id,HitTarget::FrameChrome{frame_id,..}=>FRAMES.iter().flatten().any(|f|f.frame_id==frame_id&&f.tabs.get(f.active_tab as usize).and_then(|t|*t).is_some_and(|t|t.surface_id==sid)),_=>false}
}
unsafe fn atlas_scene_at_point(x:i32,y:i32)->Option<u8>{println!("peer atlas-point {} {}",x,y);ATLAS_PICK}
unsafe fn switch_scene(scene:u8){println!("peer switch-scene {}",scene);ACTIVE_SCENE_IDX=scene;}
unsafe fn sync_scene_visibility(){println!("peer sync-scene");}
unsafe fn clear_focus_if_wrong_scene(){println!("peer clear-focus-scene");}
unsafe fn clear_drag_if_wrong_scene(){println!("peer clear-drag-scene");}
unsafe fn atlas_capture_snapshot(){println!("peer atlas-capture");}
unsafe fn toggle_bell()->bool{println!("peer toggle-bell");CALL_STATUS==0}
unsafe fn surface_input_lifetime_mark_drag_target(sid:u64,live:u8,phase:&str,ok:u8){println!("peer mark-drag {} {} {} {}",sid,live,phase,ok);}
unsafe fn surface_input_lifetime_mark_click_target(sid:u64,live:u8,committed:u8,ok:u8){println!("peer mark-click {} {} {} {}",sid,live,committed,ok);}
unsafe fn shell_interaction_contract_mark_stage(stage:u8){println!("peer interaction-stage {}",stage);}
unsafe fn shell_interaction_contract_try_done(){println!("peer interaction-done");}
unsafe fn click_drag_proof_begin_drag_once(){println!("peer proof-drag-begin");}
fn hit_dump(t:HitTarget){match t{HitTarget::None=>println!("R 0 0 0 0"),HitTarget::Surface(sid)=>println!("R 1 {} 0 0",sid),HitTarget::FrameChrome{frame_id,kind}=>println!("R 2 0 {} {}",frame_id,kind)}}
fn state_of(n:&dyn Fn(usize)->u64)->InteractionState{match n(1){0=>InteractionState::Idle,1=>InteractionState::ClickPending,2=>InteractionState::Dragging{surface_id:n(2),current_x:n(3)as i32,current_y:n(4)as i32},3=>InteractionState::PanelActive{panel:panel(n(5))},4=>InteractionState::Resizing{surface_id:n(2),edge:edge(n(6)),start_x:n(7)as i32,start_y:n(8)as i32,start_w:n(9)as u32,start_h:n(10)as u32,pending_dx:n(11)as i32,pending_dy:n(12)as i32},_=>InteractionState::TabDragging{frame_id:n(13)as u32,start_tab:n(14)as u8,start_x:n(7)as i32,start_y:n(8)as i32,current_tab:n(15)as u8}}}
fn panel(n:u64)->PanelKind{match n{0=>PanelKind::Launcher,1=>PanelKind::Status,2=>PanelKind::Clock,3=>PanelKind::Bell,_=>PanelKind::Settings}}
static mut HIT:HitTarget=HitTarget::None;
static mut TEST_NODES:[Option<AccessNode>;MAX_ACCESS_NODES]=[None;MAX_ACCESS_NODES];
fn validate_scene_id(scene:u8)->bool{println!("peer validate-scene {}",scene);(scene as usize)<ATLAS_MAX_SCENES}
unsafe fn next_scene(){println!("peer next-scene");ACTIVE_SCENE_IDX=(ACTIVE_SCENE_IDX+1)%ATLAS_MAX_SCENES as u8;}
unsafe fn prev_scene(){println!("peer prev-scene");ACTIVE_SCENE_IDX=(ACTIVE_SCENE_IDX+ATLAS_MAX_SCENES as u8-1)%ATLAS_MAX_SCENES as u8;}
fn nodes_dump(nodes:&[Option<AccessNode>;MAX_ACCESS_NODES]){for(i,node)in nodes.iter().enumerate(){if let Some(n)=node{print!("N {} {} {} {} {} {} {} {} ",i,n.node_id,n.role as u8,n.state,n.actions,n.target.surface_id,n.target.frame_id,n.target.scene_id);for b in n.label{print!("{:02x}",b)}println!();}}}

unsafe fn launcher_focus_discard(){println!("peer discard-launcher");}
unsafe fn surface_scene_id(sid:u64)->Option<u8>{println!("peer scene {}",sid);for f in FRAMES.iter().flatten(){if f.tabs.iter().flatten().any(|t|t.surface_id==sid){return Some(f.scene_id)}}None}
unsafe fn surface_in_active_scene(sid:u64)->bool{println!("peer in-scene {}",sid);FRAMES.iter().flatten().find(|f|f.tabs.iter().flatten().any(|t|t.surface_id==sid)).map_or(true,|f|f.scene_id==ACTIVE_SCENE_IDX)}
unsafe fn mesh_is_visible_in_active_scene()->bool{println!("peer mesh-visible");false}
unsafe fn mesh_render_pd_graph(){println!("peer mesh-render");}
unsafe fn bell_ring_count()->usize{println!("peer bell-count");3}
unsafe fn send_silkbar_phase2_update(kind:u32,a:u64,b:u64)->bool{println!("peer bar-update {} {} {}",kind,a,b);true}
unsafe fn surface_input_lifetime_mark_focus_live(sid:u64,alive:u8,ok:u8){println!("peer mark-focus {} {} {}",sid,alive,ok);}
unsafe fn surface_input_lifetime_try_done(){println!("peer lifetime-done");}
fn app_surface_spec_by_frame(id:u32)->Option<&'static AppSurfaceSpec>{println!("peer frame-name {}",id);APP_SURFACES.iter().find(|s|s.frame_id==id)}

static mut CALL_STATUS:u64=0;
static mut LINEN_COUNT:usize=0;
fn pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64){println!("call {} {} {} {} {}",slot,op,a,b,c);unsafe{(CALL_STATUS,a^b^c)}}
unsafe fn shell_request_app_close(sid:u64){println!("peer request-close {}",sid);if let Some(e)=APP_CUSTODY.iter_mut().flatten().find(|e|e.owns_surface(sid)){e.request_close();}}
unsafe fn clear_focus_if_dead(){println!("peer clear-focus");if FOCUSED_SURFACE_ID!=0&&!surface_is_lifecycle_live(FOCUSED_SURFACE_ID){FOCUSED_SURFACE_ID=0;FOCUSED_SURFACE=None;}}
unsafe fn clear_drag_if_dead(){println!("peer clear-drag");if let InteractionState::Dragging{surface_id,..}=INTERACTION{if !surface_is_lifecycle_focusable(surface_id){INTERACTION=InteractionState::Idle;}}}
unsafe fn clear_hover_if_dead(){println!("peer clear-hover");}
unsafe fn record_tombstone_event(sid:u64,old:LifecycleState,next:LifecycleState,why:TombstoneReason){println!("peer tomb-event {} {} {} {}",sid,old as u8,next as u8,why as u8);}
unsafe fn snap_capture_layout(){println!("peer capture");}
unsafe fn linen_object_count()->usize{println!("peer linen-count");LINEN_COUNT}
unsafe fn linen_render_object_list(){println!("peer linen-render");}
static mut TEST_REF: Option<FocusRef> = None;
static mut FLAGS: [u8;1024] = [1;1024];
fn surface_is_alive(sid:u64)->bool {println!("peer alive {}",sid);unsafe{FLAGS[(sid%1024)as usize]&1!=0}}
unsafe fn is_tombstoned(sid:u64)->bool {println!("peer tomb {}",sid);FLAGS[(sid%1024)as usize]&2!=0}
fn app_surface_spec(sid:u64)->Option<&'static AppSurfaceSpec> {println!("peer app {}",sid);APP_SURFACES.iter().find(|s|s.surface_id==sid)}
unsafe fn get_surface_bounds(sid:u64)->Option<(i32,i32,u32,u32)> {
    println!("peer bounds {}",sid);if FLAGS[(sid%1024)as usize]&4!=0{return None}
    BOUNDS[(sid%1024)as usize].or(Some(((sid%91)as i32,(sid%57)as i32,333,222)))
}
fn clamp_surface_size(x:i32,y:i32,w:u32,h:u32)->(u32,u32) {
    println!("peer clamp {} {} {} {}",x,y,w,h);(w.clamp(1,1000),h.clamp(1,600))
}
unsafe fn reset(){
    SCENES=[Scene{flags:SCENE_FLAG_EMPTY,label:[0;ATLAS_LABEL_LEN],accent:0,pinned:false};ATLAS_MAX_SCENES];TEST_NODES=[None;MAX_ACCESS_NODES];
    POINT_GEOMETRY=false;BOUNDS=[None;1024];ATLAS_PICK=None;ATLAS_MODE_ENABLED=false;
    DRAG_MOVED_THIS_DRAG=false;WINDOW_MOVE_PROOF_DONE_EMITTED=false;DRAG_CAPTURE_MOVE_SEEN=false;
    DOCUMENT_POINTER_SEQUENCE=0;DOCUMENT_POINTER=None;POINTER_X=0;POINTER_Y=0;INPUT_TRACE_DRAG_MOVES=0;
    DRAG_PENDING_ACTIVE=false;DRAG_PENDING_TARGET=0;DRAG_PENDING_KIND=0;DRAG_PENDING_START_X=0;DRAG_PENDING_START_Y=0;TAB_DRAG_PENDING_FRAME_ID=0;TAB_DRAG_PENDING_START_TAB=0;
    LAUNCHER_ACTIVE=false;STATUS_ACTIVE=false;CLOCK_ACTIVE=false;SCENE_SETTINGS_ACTIVE=false;
    FRAMES=[None;MAX_FRAMES];APP_CUSTODY=[None;app_catalog::COUNT];
    HIT=HitTarget::None;ACTIVE_TINT_IDX=0;
    WINDOWS.clear();WINDOWS.push(core::mem::zeroed());WINDOWS.push(core::mem::zeroed());
    DOCUMENT_VIEWS=[const{document_view::Delivery::new()};app_catalog::DOCUMENT_COUNT];SPINDLE_VIEWS=[const{spindle_view::Delivery::new()};app_catalog::SPINDLE_COUNT];
    PALETTE_VISIBILITY=linen_text_delivery::LatestOperation::new();DESKTOP_STATES=linen_text_delivery::DesktopStates::new();INTERACTION=InteractionState::Idle;CALL_STATUS=0;LINEN_COUNT=0;
    LIFECYCLE_TABLE=[None;LIFECYCLE_MAX_SURFACES];LIFECYCLE_GENERATION=1;
    FOCUSED_SURFACE=None;FOCUSED_SURFACE_ID=0;ACTIVE_SCENE_IDX=0;
    HOVERED_FRAME_ID=0;HOVER_KIND=0;HOVERED_FRAME_LIGHT=0;TEST_REF=None;FLAGS=[1;1024];
    SURFACE_100_ALIVE=false;SURFACE_101_ALIVE=false;SURFACE_102_ALIVE=false;SURFACE_103_ALIVE=false;SURFACE_100_W=0;SURFACE_100_H=0;SURFACE_101_X=0;SURFACE_101_Y=0;SURFACE_101_W=0;SURFACE_101_H=0;SURFACE_102_X=0;SURFACE_102_Y=0;SURFACE_102_W=0;SURFACE_102_H=0;SURFACE_103_X=0;SURFACE_103_Y=0;SURFACE_103_W=0;SURFACE_103_H=0;SURFACE_200_X=0;SURFACE_200_Y=0;SURFACE_200_W=0;SURFACE_200_H=0;SURFACE_201_X=0;SURFACE_201_Y=0;SURFACE_201_W=0;SURFACE_201_H=0;SURFACE_202_X=0;SURFACE_202_Y=0;SURFACE_202_W=0;SURFACE_202_H=0;SURFACE_203_X=0;SURFACE_203_Y=0;SURFACE_203_W=0;SURFACE_203_H=0;SURFACE_204_X=0;SURFACE_204_Y=0;SURFACE_204_W=0;SURFACE_204_H=0;SURFACE_205_X=0;SURFACE_205_Y=0;SURFACE_205_W=0;SURFACE_205_H=0;SURFACE_0x99_X=0;SURFACE_0x99_Y=0;SURFACE_0x99_W=0;SURFACE_0x99_H=0;
    DESKTOP_WIDTH.store(1280,Ordering::Relaxed);DESKTOP_HEIGHT.store(720,Ordering::Relaxed);
}
unsafe fn state(){
    print!("S {} {} {} {} {} {}",FOCUSED_SURFACE_ID,ACTIVE_SCENE_IDX,HOVERED_FRAME_ID,HOVER_KIND,HOVERED_FRAME_LIGHT,LIFECYCLE_GENERATION);
    for f in FRAMES {if let Some(f)=f {print!("|F {} {} {} {} {}",f.frame_id,f.active_tab,f.tab_count,f.scene_id,f.flags);print!("|N {} {} {} {}",f.normal_x,f.normal_y,f.normal_w,f.normal_h);
        for t in f.tabs{if let Some(t)=t{print!(":{}:{}:{}",t.surface_id,t.title_id,t.flags)}else{print!(":-")}}
    }else{print!("|-")}}
    // Compare every occupied entry; spare capacity has an independent control.
    for e in LIFECYCLE_TABLE{if let Some((sid,lc))=e{print!("|L {} {} {}",sid,lc.state as u8,lc.generation)}}
    for r in [FOCUSED_SURFACE,TEST_REF]{if let Some(r)=r{print!("|R {} {}",r.surface_id,r.generation)}else{print!("|-")}}

    for e in APP_CUSTODY{if let Some(e)=e{let(x,y,w,h)=e.bounds.unwrap_or((0,0,0,0));print!("|C {} {} {} {} {} {} {} {} {} {}",e.target,e.task,e.surface,e.dead as u8,e.close_pending as u8,e.bounds.is_some()as u8,x,y,w,h);}else{print!("|-");}}
    for d in &DOCUMENT_VIEWS{d.trace_chrome();}for d in &SPINDLE_VIEWS{d.trace_chrome();}PALETTE_VISIBILITY.trace_chrome();
    for w in &WINDOWS{let d=&w.desc;print!("|W {} {} {} {}",d.x,d.y,d.width,d.height);}
    print!("|A {} {} {} {} {} {}",SURFACE_100_ALIVE as u8,SURFACE_101_ALIVE as u8,SURFACE_102_ALIVE as u8,SURFACE_103_ALIVE as u8,SURFACE_100_W,SURFACE_100_H);
    print!("|G {} {} {} {}",SURFACE_101_X,SURFACE_101_Y,SURFACE_101_W,SURFACE_101_H);
    print!("|G {} {} {} {}",SURFACE_102_X,SURFACE_102_Y,SURFACE_102_W,SURFACE_102_H);
    print!("|G {} {} {} {}",SURFACE_103_X,SURFACE_103_Y,SURFACE_103_W,SURFACE_103_H);
    print!("|G {} {} {} {}",SURFACE_200_X,SURFACE_200_Y,SURFACE_200_W,SURFACE_200_H);
    print!("|G {} {} {} {}",SURFACE_201_X,SURFACE_201_Y,SURFACE_201_W,SURFACE_201_H);
    print!("|G {} {} {} {}",SURFACE_202_X,SURFACE_202_Y,SURFACE_202_W,SURFACE_202_H);
    print!("|G {} {} {} {}",SURFACE_203_X,SURFACE_203_Y,SURFACE_203_W,SURFACE_203_H);
    print!("|G {} {} {} {}",SURFACE_204_X,SURFACE_204_Y,SURFACE_204_W,SURFACE_204_H);
    print!("|G {} {} {} {}",SURFACE_205_X,SURFACE_205_Y,SURFACE_205_W,SURFACE_205_H);
    print!("|G {} {} {} {}",SURFACE_0x99_X,SURFACE_0x99_Y,SURFACE_0x99_W,SURFACE_0x99_H);
    let(tag,id)=match INTERACTION{InteractionState::Idle=>(0,0),InteractionState::ClickPending=>(1,0),InteractionState::Dragging{surface_id,..}=>(2,surface_id),InteractionState::PanelActive{..}=>(3,0),InteractionState::Resizing{surface_id,..}=>(4,surface_id),InteractionState::TabDragging{frame_id,..}=>(5,frame_id as u64)};print!("|I {} {}",tag,id);
    print!("|D {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",DRAG_PENDING_ACTIVE as u8,DRAG_PENDING_TARGET,DRAG_PENDING_KIND,DRAG_PENDING_START_X,DRAG_PENDING_START_Y,TAB_DRAG_PENDING_FRAME_ID,TAB_DRAG_PENDING_START_TAB,POINTER_X,POINTER_Y,INPUT_TRACE_DRAG_MOVES,DRAG_MOVED_THIS_DRAG as u8,WINDOW_MOVE_PROOF_DONE_EMITTED as u8,DRAG_CAPTURE_MOVE_SEEN as u8,LAUNCHER_ACTIVE as u8,STATUS_ACTIVE as u8,CLOCK_ACTIVE as u8,SCENE_SETTINGS_ACTIVE as u8);
    let mut a=[0u64;15];a[0]=match INTERACTION{InteractionState::Idle=>0,InteractionState::ClickPending=>1,InteractionState::Dragging{surface_id,current_x,current_y}=>{a[1]=surface_id;a[2]=current_x as i64 as u64;a[3]=current_y as i64 as u64;2},InteractionState::PanelActive{panel}=>{a[4]=panel as u64;3},InteractionState::Resizing{surface_id,edge,start_x,start_y,start_w,start_h,pending_dx,pending_dy}=>{a[1]=surface_id;a[5]=edge as u64;a[6]=start_x as i64 as u64;a[7]=start_y as i64 as u64;a[8]=start_w as u64;a[9]=start_h as u64;a[10]=pending_dx as i64 as u64;a[11]=pending_dy as i64 as u64;4},InteractionState::TabDragging{frame_id,start_tab,start_x,start_y,current_tab}=>{a[12]=frame_id as u64;a[13]=start_tab as u64;a[6]=start_x as i64 as u64;a[7]=start_y as i64 as u64;a[14]=current_tab as u64;5}};
    print!("|E");for(i,v)in a.iter().enumerate(){if matches!(i,2|3|6|7|10|11){print!(" {}",*v as i64)}else{print!(" {}",v)}}
    if let Some((slot,target,action,sequence))=DOCUMENT_POINTER{print!("|DP {} 1 {} {} {} {}",DOCUMENT_POINTER_SEQUENCE,slot,target,action,sequence)}else{print!("|DP {} 0",DOCUMENT_POINTER_SEQUENCE)}
    println!();
}
fn optional(n:Option<u64>){println!("R {} {}",n.is_some()as u8,n.unwrap_or(0));}
fn life(n:u64)->LifecycleState{match n{0=>LifecycleState::Allocated,1=>LifecycleState::Mapped,2=>LifecycleState::Visible,3=>LifecycleState::Hidden,4=>LifecycleState::Minimized,5=>LifecycleState::Closing,6=>LifecycleState::Tombstoned,7=>LifecycleState::Destroyed,_=>panic!("state")}}
fn edge(n:u64)->ResizeEdge{match n{0=>ResizeEdge::Left,1=>ResizeEdge::Right,2=>ResizeEdge::Top,3=>ResizeEdge::Bottom,4=>ResizeEdge::TopLeft,5=>ResizeEdge::TopRight,6=>ResizeEdge::BottomLeft,7=>ResizeEdge::BottomRight,_=>panic!("edge")}}
fn action(n:u64)->SurfaceAction{assert!(n<=SurfaceAction::AccessScenePrev as u64);unsafe{core::mem::transmute(n as u8)}}
fn main(){
    use std::io::{self,Read};let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();unsafe{reset()};
    for line in text.lines(){let v:Vec<u64>=line.split_whitespace().map(|s|s.parse().unwrap()).collect();let n=|i:usize|*v.get(i).unwrap_or(&0);
        unsafe{match n(0){
        97=>println!("R {}",LIFECYCLE_MAX_SURFACES),
        100=>reset(),
        101=>{let mut f:ShellFrame=core::mem::zeroed();f.frame_id=n(2)as u32;f.active_tab=n(3)as u8;f.tab_count=n(4)as u8;f.scene_id=n(5)as u8;f.flags=n(6)as u32;
            for i in 0..MAX_TABS_PER_FRAME as usize{if n(i+7)!=0{f.tabs[i]=Some(ShellTab{surface_id:n(i+7),title_id:i as u64,flags:i as u32})}}FRAMES[n(1)as usize]=Some(f);},
        102=>FRAMES[n(1)as usize]=None,
        103=>{FOCUSED_SURFACE_ID=n(1);ACTIVE_SCENE_IDX=n(2)as u8;HOVERED_FRAME_ID=n(3)as u32;HOVER_KIND=n(4)as u32;HOVERED_FRAME_LIGHT=n(5)as u32;DESKTOP_WIDTH.store(n(6)as u32,Ordering::Relaxed);DESKTOP_HEIGHT.store(n(7)as u32,Ordering::Relaxed);},
        104=>FLAGS[n(1)as usize]=n(2)as u8,
        105=>{let row=app_catalog::INCARNATIONS[n(1)as usize];let mut e=app_custody::Entry::new(row.lifecycle_slot,row.target,n(2)as u32,row.surface);e.dead=n(3)!=0;e.close_pending=n(4)!=0;e.bounds=if n(5)!=0{Some((0,0,300,200))}else{None};APP_CUSTODY[n(1)as usize]=Some(e);},
        106=>{LIFECYCLE_GENERATION=n(1);LIFECYCLE_TABLE[0]=Some((n(2),SurfaceLifecycle{state:life(n(3)),generation:n(4)}));},
        0=>optional(scancode_to_action(n(1)as u8).map(|x|x as u64)),1=>println!("R {}",action_name(action(n(1)))),
        2=>println!("R {} {}",desktop_width(),desktop_height()),
        3|4|5=>{let(x,y,w,h)=match n(0){3=>layout_left(),4=>layout_right(),_=>layout_maximize()};println!("R {} {} {} {}",x,y,w,h);},
        6=>{let(x,y)=clamp_position(n(1)as i32,n(2)as i32,n(3)as u32,n(4)as u32);println!("R {} {}",x,y);},
        7=>{let(x,y)=snap_end_pos(n(1)as u32,n(2)as u32);println!("R {} {}",x,y);},
        8=>println!("R {}",lifecycle_register(n(1),life(n(2)))as u8),9=>optional(lifecycle_state(n(1)).map(|x|x as u64)),
        10=>println!("R {}",set_lifecycle_state(n(1),life(n(2)))as u8),11=>optional(surface_generation(n(1))),
        12=>println!("R {}",bump_surface_generation(n(1))as u8),
        13=>{TEST_REF=make_focus_ref(n(1));optional(TEST_REF.map(|r|r.generation));},
        14=>println!("R {}",TEST_REF.map_or(false,|r|focus_ref_is_current(&r))as u8),15=>sync_focus_ref(),
        16=>println!("R {}",surface_is_lifecycle_live(n(1))as u8),17=>println!("R {}",surface_is_lifecycle_focusable(n(1))as u8),18=>lifecycle_init_all(),
        19=>println!("R {}",is_shell_surface(n(1))as u8),20=>println!("R {}",is_focusable_surface(n(1))as u8),
        21=>optional(frame_for_surface(n(1)).map(|x|x as u64)),22=>optional(active_surface_for_frame(n(1)as u32)),
        23=>optional(selected_frame_id().map(|x|x as u64)),24=>optional(selected_surface_id()),25=>println!("R {}",selected_window_options_mask()),
        26=>println!("R {}",frame_is_minimized(n(1)as u32)as u8),27=>set_frame_minimized(n(1)as u32,n(2)!=0),
        28=>println!("R {}",frame_is_zoomed(n(1)as u32)as u8),29=>set_frame_zoomed(n(1)as u32,n(2)!=0),
        30=>println!("R {}",frame_has_top_bar(n(1)as u32)as u8),31=>set_frame_top_bar(n(1)as u32,n(2)!=0),
        32=>optional(first_minimized_frame_id().map(|x|x as u64)),33=>println!("R {}",frame_accepts_input(n(1)as u32)as u8),
        34=>println!("R {}",frame_chrome_visible(n(1)as u32)as u8),35=>clear_hover_if_wrong_scene(),
        36=>println!("R {}",frame_tab_count(n(1)as u32)),37=>println!("R {}",frame_active_tab_index(n(1)as u32)),
        38=>optional(frame_tab_at(n(1)as u32,n(2)as i32,n(3)as i32).map(|x|x as u64)),
        39=>println!("R {}",swap_frame_tabs(n(1)as u32,n(2)as u8,n(3)as u8)as u8),
        40=>println!("R {}",frame_light_to_option_mask(n(1)as u32)),41=>println!("R {}",abs_screen_to_raw(n(1)as i32,n(2)as i32)),
        42=>println!("R {}",compute_resize_edge(n(1)as i32,n(2)as i32,n(3)as i32,n(4)as i32,n(5)as u32,n(6)as u32)as u8),
        43=>{let(x,y,w,h)=compute_resize_rect(n(1)as i32,n(2)as i32,n(3)as u32,n(4)as u32,edge(n(5)),n(6)as i32,n(7)as i32);println!("R {} {} {} {}",x,y,w,h);},
        44=>{let r=Rect::new(n(1)as i32,n(2)as i32,n(3)as u32,n(4)as u32);println!("R {}",r.contains(n(5)as i32,n(6)as i32)as u8);},

        120=>INTERACTION=match n(1){1=>InteractionState::Dragging{surface_id:n(2),current_x:0,current_y:0},2=>InteractionState::Resizing{surface_id:n(2),edge:ResizeEdge::Left,start_x:0,start_y:0,start_w:0,start_h:0,pending_dx:0,pending_dy:0},3=>InteractionState::TabDragging{frame_id:n(2)as u32,start_tab:0,start_x:0,start_y:0,current_tab:0},_=>InteractionState::Idle},
        121=>{SURFACE_100_ALIVE=n(1)!=0;SURFACE_101_ALIVE=n(1)!=0;SURFACE_102_ALIVE=n(1)!=0;SURFACE_103_ALIVE=n(1)!=0;LINEN_COUNT=n(2)as usize;CALL_STATUS=n(3);},
        122=>DESKTOP_STATES.test_ready(n(1),n(2)!=0,n(3)!=0),
        123=>if n(1)==0{DOCUMENT_VIEWS[n(2)as usize].test_last(n(3)as u32,n(4)as u32,n(5))}else{SPINDLE_VIEWS[n(2)as usize].test_last(n(3)as u32,n(4)as u32,n(5))},
        124=>{HIT=match n(1){1=>HitTarget::Surface(n(2)),2=>HitTarget::FrameChrome{frame_id:n(2)as u32,kind:n(3)as u32},_=>HitTarget::None};ACTIVE_TINT_IDX=n(4)as u8;},
        126=>{let mut label=[0;ATLAS_LABEL_LEN];for i in 0..ATLAS_LABEL_LEN{label[i]=n(i+3)as u8}SCENES[n(1)as usize]=Scene{flags:n(2)as u8,label,accent:0,pinned:false};},
        127=>{TEST_NODES[n(1)as usize]=if n(2)==0{None}else{Some(AccessNode{node_id:n(1)as u32,role:AccessRole::Frame,state:n(3)as u16,actions:n(4)as u16,target:AccessTargetRef{surface_id:n(5),frame_id:n(6)as u32,scene_id:n(7)as u8},label:[0;32]})};},
        67=>{let mut dst=[n(2)as u8;32];let src:[u8;32]=core::array::from_fn(|i|n(i+3)as u8);access_copy_label(&mut dst,&src[..n(1).min(32)as usize]);print!("B ");for b in dst{print!("{:02x}",b)}println!(" {}",access_label_token(&dst));},
        68=>println!("R {}",access_node_is_valid_target(&AccessTargetRef{surface_id:n(1),frame_id:n(2)as u32,scene_id:n(3)as u8})as u8),
        69=>{let count=access_emit_scene_node(&mut TEST_NODES,n(1)as usize,n(2)as u8);println!("R {}",count);nodes_dump(&TEST_NODES);},
        70=>{let f=FRAMES[n(2)as usize].unwrap();let count=access_emit_frame_node(&mut TEST_NODES,n(1)as usize,&f);println!("R {}",count);nodes_dump(&TEST_NODES);},
        71=>{let count=access_emit_shell_nodes(&mut TEST_NODES);println!("R {}",count);nodes_dump(&TEST_NODES);},
        72=>println!("R {}",access_handle_keyboard_action(action(n(1)))as u8),
        73=>access_describe_focus(),
        74=>println!("R {}",access_move_focus(&TEST_NODES[..n(1).min(MAX_ACCESS_NODES as u64)as usize],n(2)!=0)as u8),
        128=>{POINT_GEOMETRY=n(1)!=0;POINTER_X=n(2)as i32;POINTER_Y=n(3)as i32;},
        129=>INTERACTION=state_of(&n),
        130=>BOUNDS[n(1)as usize]=if n(2)!=0{Some((n(3)as i32,n(4)as i32,n(5)as u32,n(6)as u32))}else{None},
        131=>{ATLAS_MODE_ENABLED=n(1)!=0;ATLAS_PICK=if n(2)!=0{Some(n(3)as u8)}else{None};},
        132=>{DOCUMENT_POINTER_SEQUENCE=n(1);DOCUMENT_POINTER=if n(2)!=0{Some((n(3),n(4)as u32,n(5)as u8,n(6)))}else{None};},
        75=>try_transition(state_of(&n)),
        76=>if let Some((x,y,c))=move_surface_by(n(1),n(2)as i32,n(3)as i32){println!("R 1 {} {} {}",x,y,c)}else{println!("R 0")},
        77=>if let Some((f,s,x,y,w,h,px,py))=synthetic_window_drag_target(){println!("R 1 {} {} {} {} {} {} {} {}",f,s,x,y,w,h,px,py)}else{println!("R 0")},
        78=>silk_drag_begin_mark(n(1),n(2)as i32,n(3)as i32),
        79=>silk_drag_end_mark(n(1)),
        80=>println!("R {}",drag_move_focused(n(1)as i32,n(2)as i32)as u8),
        81=>println!("R {}",resize_accumulate_delta(n(1)as i32,n(2)as i32)as u8),
        82=>clear_resize_if_dead(),
        83=>println!("R {}",try_snap_on_drag_release(n(1),n(2)as i32,n(3)as i32)as u8),
        84=>println!("R {}",apply_resize_geometry(n(1)as i32,n(2)as i32)as u8),
        85=>if let Some(t)=hit_test_surface_chrome(n(1)as i32,n(2)as i32,n(3)){hit_dump(t)}else{println!("R 0 0 0 0")},
        86=>hit_dump(hit_test_at(n(1)as i32,n(2)as i32)),
        87=>{let t=match n(1){1=>HitTarget::Surface(n(2)),2=>HitTarget::FrameChrome{frame_id:n(3)as u32,kind:n(4)as u32},_=>HitTarget::None};optional(hit_target_surface(t));let(label,id)=hit_target_label(t,n(5)!=0);println!("T {} {}",label,id);},
        88=>{let(t,bar)=click_hit_test_and_focus(n(1)as i32,n(2)as i32,n(3)as u8);hit_dump(t);println!("R {}",bar as u8);},
        89=>{let mut active=n(1)!=0;let ok=toggle_os_panel(&mut active,panel(n(2)),n(3),"driver",n(4)as u32,n(5)as u32,n(6)as u32,n(7)as u32);println!("R {} {}",ok as u8,active as u8);},
        90=>toggle_scene_settings_panel(),
        91=>println!("R {}",handle_silkbar_click(n(1)as i32,n(2)as i32)as u8),
        125=>{if let Some(r)=FOCUSED_SURFACE{println!("R 1 {} {}",r.surface_id,r.generation)}else{println!("R 0")}},
        59=>tile_active_scene_frames(),60=>tile_visible_frames(),61=>println!("R {}",frame_light_at(n(1)as u32,n(2)as i32,n(3)as i32)),
        62=>println!("R {}",toggle_top_bar_for_active_frame()as u8),63=>println!("R {}",switch_to_tab(n(1)as u32,n(2)as u32)as u8),
        64=>println!("R {}",try_set_focus(n(1))as u8),65=>println!("R {}",try_set_focus_inner(n(1),n(2)!=0)as u8),66=>println!("R {}",update_frame_hover_at(n(1)as i32,n(2)as i32)as u8),
        45=>println!("R {}",is_closeable_surface(n(1))as u8),46=>println!("R {}",frame_close_allowed(n(1)as u32)as u8),
        47=>println!("R {}",close_surface_from_frame_light(n(1))as u8),48=>println!("R {}",minimize_frame(n(1)as u32)as u8),
        49=>println!("R {}",restore_minimized_frame(n(1)as u32)as u8),50=>println!("R {}",restore_minimized_frame_inner(n(1)as u32,n(2)!=0)as u8),
        51=>emit_chrome_diagnostics(n(1)as u32,"driver"),52=>update_local_geometry(n(1),n(2)as i32,n(3)as i32,n(4)as u32,n(5)as u32),
        53=>println!("R {}",zoom_frame(n(1)as u32)as u8),54=>println!("R {}",unzoom_frame(n(1)as u32)as u8),55=>println!("R {}",toggle_zoom_frame(n(1)as u32)as u8),
        56=>native_child_geometry(n(1),n(2)as i32,n(3)as i32,n(4)as u32,n(5)as u32),
        57=>if let Some(op)=frame_chrome_operation(n(1)as u32){println!("R 1 {} {} {} {}",op.opcode,op.surface,op.packed,op.control)}else{println!("R 0")},
        58=>send_frame_tab_info(n(1)as u32),
        _=>panic!("command")};state();}
    }
}
