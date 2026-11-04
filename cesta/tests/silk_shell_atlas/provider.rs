static mut FLAGS: [u8;256] = [253;256];
static mut LIFE: [u8;256] = [2;256];
static mut SELECTED: Option<u32> = None;
static mut GATE: bool = true;
static mut DW: i32 = 1280;
static mut DH: i32 = 720;
static mut CALL_STATUS: u64 = 0;

fn flags(sid:u64)->u8 { unsafe{FLAGS[(sid%256)as usize]} }
fn surface_is_alive(sid:u64)->bool { println!("peer alive {}",sid); flags(sid)&1!=0 }
unsafe fn is_tombstoned(sid:u64)->bool { println!("peer tomb {}",sid); flags(sid)&2!=0 }
unsafe fn surface_is_lifecycle_focusable(sid:u64)->bool { println!("peer focusable {}",sid);flags(sid)&4!=0 }
unsafe fn try_set_focus(sid:u64)->bool {
 println!("peer focus {}",sid);
 if GATE && (sid==0 || flags(sid)&7==5){FOCUSED_SURFACE_ID=sid;true}else{false}
}
unsafe fn lifecycle_state(sid:u64)->Option<LifecycleState>{
 println!("peer lifecycle {}",sid);if flags(sid)&16==0{return None}
 Some(match LIFE[(sid%256)as usize]{0=>LifecycleState::Allocated,1=>LifecycleState::Mapped,2=>LifecycleState::Visible,3=>LifecycleState::Hidden,4=>LifecycleState::Minimized,5=>LifecycleState::Closing,6=>LifecycleState::Tombstoned,_=>LifecycleState::Destroyed})
}
unsafe fn set_lifecycle_state(sid:u64,next:LifecycleState)->bool {
 println!("peer set-lifecycle {} {}",sid,next as u8);if GATE{LIFE[(sid%256)as usize]=next as u8;}GATE
}
unsafe fn active_surface_for_frame(id:u32)->Option<u64>{
 println!("peer active {}",id);
 FRAMES.iter().flatten().find(|f|f.frame_id==id).and_then(|f|f.tabs.get(f.active_tab as usize).and_then(|t|t.map(|v|v.surface_id)))
}
unsafe fn get_surface_bounds(sid:u64)->Option<(i32,i32,u32,u32)>{
 println!("peer bounds {}",sid);if flags(sid)&128==0{None}else{Some(((sid%91)as i32,(sid%57)as i32,333,222))}
}
unsafe fn make_focus_ref(sid:u64)->Option<FocusRef>{
 println!("peer make-ref {}",sid);if flags(sid)&64==0{None}else{Some(FocusRef{surface_id:sid,generation:7})}
}
unsafe fn focus_ref_is_current(r:&FocusRef)->bool {println!("peer current {} {}",r.surface_id,r.generation);flags(r.surface_id)&8!=0}
unsafe fn selected_frame_id()->Option<u32>{println!("peer selected");SELECTED}
unsafe fn clear_focus_if_dead(){println!("peer clear-focus");if FOCUSED_SURFACE_ID!=0 && flags(FOCUSED_SURFACE_ID)&3!=1{FOCUSED_SURFACE_ID=0}}
unsafe fn clear_drag_if_dead(){println!("peer clear-drag-dead")}
unsafe fn clear_drag_if_wrong_scene(){println!("peer clear-drag-scene")}
unsafe fn clear_hover_if_dead(){println!("peer clear-hover-dead")}
unsafe fn clear_hover_if_wrong_scene(){println!("peer clear-hover-scene")}
unsafe fn launcher_focus_discard(){println!("peer discard-launcher")}
unsafe fn tile_active_scene_frames(){println!("peer tile")}
unsafe fn cycle_scene_render_token_preset(){println!("peer preset-next");SCENE_APPEARANCE_STATE.preset_idx=SCENE_APPEARANCE_STATE.preset_idx.wrapping_add(1)%3}
unsafe fn cycle_prev_scene_render_token_preset(){println!("peer preset-prev");SCENE_APPEARANCE_STATE.preset_idx=if SCENE_APPEARANCE_STATE.preset_idx==0{2}else{SCENE_APPEARANCE_STATE.preset_idx-1}}
fn get_preset_name(idx:u8)->&'static str {println!("peer preset-name {}",idx);match idx{0=>"p0",1=>"p1",2=>"p2",_=>"unknown"}}
unsafe fn apply_custom_tint_bundle(idx:usize){println!("peer tint {}",idx);SCENE_APPEARANCE_STATE.use_custom_colors=1;for i in 0..8{SCENE_APPEARANCE_STATE.custom_colors[i]=(idx as u32).wrapping_mul(97).wrapping_add(i as u32)}}
unsafe fn resolve_scene_render_tokens()->TokenPreset {println!("peer tokens");let mut out=[0;8];for i in 0..8{out[i]=1000+i as u32+ACTIVE_TINT_IDX as u32}out}
unsafe fn push_token_preset(p:&TokenPreset){print!("peer push");for v in p{print!(" {}",v)}println!()}
unsafe fn send_silkbar_phase2_update(kind:u32,a:u64,b:u64)->bool {println!("peer update {} {} {}",kind,a,b);GATE}
unsafe fn bell_ring_count()->usize {println!("peer bells");3}
unsafe fn frame_is_minimized(id:u32)->bool {println!("peer minimized {}",id);FRAMES.iter().flatten().find(|f|f.frame_id==id).map_or(false,|f|f.flags&FRAME_FLAG_MINIMIZED!=0)}
unsafe fn alter_frame(id:u32,kind:u8)->bool {
 if !GATE{return false}for f in FRAMES.iter_mut().flatten(){if f.frame_id==id{if kind==0{f.flags|=FRAME_FLAG_MINIMIZED}else if kind==1{f.flags&=!FRAME_FLAG_MINIMIZED}else{f.flags^=FRAME_FLAG_ZOOMED}return true}}false
}
unsafe fn minimize_frame(id:u32)->bool {println!("peer minimize {}",id);alter_frame(id,0)}
unsafe fn restore_minimized_frame(id:u32)->bool {println!("peer restore {}",id);alter_frame(id,1)}
unsafe fn toggle_zoom_frame(id:u32)->bool {println!("peer zoom {}",id);alter_frame(id,2)}
unsafe fn switch_to_tab(id:u32,tab:u32)->bool {
 println!("peer tab {} {}",id,tab);if !GATE{return false}for f in FRAMES.iter_mut().flatten(){if f.frame_id==id && tab<f.tab_count as u32{f.active_tab=tab as u8;return true}}false
}
unsafe fn is_closeable_surface(sid:u64)->bool{println!("peer closeable {}",sid);flags(sid)&32!=0}
unsafe fn close_surface_from_frame_light(sid:u64)->bool{println!("peer close {}",sid);GATE}
fn clamp_position(x:i32,y:i32,w:u32,h:u32)->(i32,i32){println!("peer clamp-pos {} {} {} {}",x,y,w,h);(x.clamp(0,1000),y.clamp(0,600))}
fn clamp_surface_size(x:i32,y:i32,w:u32,h:u32)->(u32,u32){println!("peer clamp-size {} {} {} {}",x,y,w,h);(w.clamp(1,1000),h.clamp(1,600))}
fn desktop_width()->i32{println!("peer width");unsafe{DW}}
fn desktop_height()->i32{println!("peer height");unsafe{DH}}
fn pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64){println!("call {} {} {} {} {}",slot,op,a,b,c);unsafe{(CALL_STATUS,a^b^c^op)}}

unsafe fn reset(){
 FRAMES=[None;MAX_FRAMES];SCENES=core::mem::zeroed();ATLAS_SNAPSHOT=core::mem::zeroed();SCENE_SNAPSHOT=core::mem::zeroed();
 ACTIVE_SCENE_IDX=0;FOCUSED_SURFACE_ID=0;ATLAS_MODE_ENABLED=false;ATLAS_SELECTED_SCENE=0;ATLAS_DRAG_INTENT=core::mem::zeroed();
 ACTIVE_TINT_IDX=0;SCENE_APPEARANCE_STATE=DEFAULT_SCENE_APPEARANCE;APP_CUSTODY=[None;app_catalog::COUNT];
 FLAGS=[253;256];LIFE=[2;256];SELECTED=None;GATE=true;DW=1280;DH=720;CALL_STATUS=0;
}

unsafe fn state(){
 print!("STATE {} {} {} {} {} {} {} {} {} {} {} {}",ACTIVE_SCENE_IDX,FOCUSED_SURFACE_ID,ATLAS_MODE_ENABLED as u8,ATLAS_SELECTED_SCENE,ACTIVE_TINT_IDX,
  SCENE_APPEARANCE_STATE.preset_idx,SCENE_APPEARANCE_STATE.use_custom_colors,ATLAS_DRAG_INTENT.active as u8,ATLAS_DRAG_INTENT.scene_id,ATLAS_DRAG_INTENT.frame_id,ATLAS_DRAG_INTENT.start_x,ATLAS_DRAG_INTENT.start_y);
 for color in SCENE_APPEARANCE_STATE.custom_colors{print!(" {}",color)}
 for scene in &SCENES {print!("|S {} {} {} ",scene.flags,scene.accent,scene.pinned as u8);for b in scene.label{print!("{:02x}",b)}}
 for slot in FRAMES {if let Some(f)=slot{print!("|F 1 {} {} {} {} {} {} {} {} {}",f.frame_id,f.scene_id,f.active_tab,f.tab_count,f.flags,f.normal_x,f.normal_y,f.normal_w,f.normal_h);
  for t in f.tabs{if let Some(t)=t{print!(":1:{}:{}:{}",t.surface_id,t.title_id,t.flags)}else{print!(":0:0:0:0")}}
 }else{print!("|F 0")}}
 print!("|A {} {}",ATLAS_SNAPSHOT.active_scene_id,ATLAS_SNAPSHOT.scene_count);
 for s in ATLAS_SNAPSHOT.scenes{print!(":{}:{}:{}:{}:{}:{} ",s.scene_id,s.flags,s.accent,s.pinned as u8,s.focused_frame_id,s.frame_count);for b in s.label{print!("{:02x}",b)}for id in s.frame_ids{print!(":{}",id)}}
 print!("|N {} {} {} {} {}",SCENE_SNAPSHOT.magic,SCENE_SNAPSHOT.version,SCENE_SNAPSHOT.active_scene,SCENE_SNAPSHOT.frame_count,SCENE_SNAPSHOT.checksum);
 for f in SCENE_SNAPSHOT.frames{print!(":{}:{}:{}:{}:{}:{}:{}:{}:{}:{}",f.present,f.frame_id,f.scene_id,f.active_tab,f.tab_count,f.flags,f.normal_x,f.normal_y,f.normal_w,f.normal_h);for s in f.tab_surfaces{print!(":{}",s)}}
 println!();
}

fn main(){
 use std::io::{self,Read};let mut input=String::new();io::stdin().read_to_string(&mut input).unwrap();unsafe{reset()};
 for line in input.lines(){let v:Vec<u64>=line.split_whitespace().map(|s|s.parse().unwrap()).collect();if v.is_empty(){continue}let n=|i:usize|*v.get(i).unwrap_or(&0);
 unsafe{
 match n(0){
  100=>reset(),
  101=>{let mut f:ShellFrame=core::mem::zeroed();f.frame_id=n(2)as u32;f.scene_id=n(3)as u8;f.flags=n(4)as u32;f.active_tab=n(5)as u8;f.tab_count=n(6)as u8;f.normal_x=n(7)as i32;f.normal_y=n(8)as i32;f.normal_w=n(9)as u32;f.normal_h=n(10)as u32;for j in 0..MAX_TABS_PER_FRAME as usize{if n(11+j)!=0{f.tabs[j]=Some(ShellTab{surface_id:n(11+j),title_id:j as u64,flags:j as u32})}}FRAMES[n(1)as usize]=Some(f)},
  102=>FRAMES[n(1)as usize]=None,
  103=>{FLAGS[n(1)as usize]=n(2)as u8;LIFE[n(1)as usize]=n(3)as u8},
  104=>{ACTIVE_SCENE_IDX=n(1)as u8;FOCUSED_SURFACE_ID=n(2);ATLAS_MODE_ENABLED=n(3)!=0;ATLAS_SELECTED_SCENE=n(4)as u8;SELECTED=if n(5)!=0{Some(n(6)as u32)}else{None};GATE=n(7)!=0},
  105=>{DW=n(1)as i32;DH=n(2)as i32;CALL_STATUS=n(3)},
  106=>{let i=n(1)as usize;SCENES[i].accent=n(2)as u8;SCENES[i].pinned=n(3)!=0;SCENES[i].flags=n(4)as u8;SCENES[i].label=atlas_default_label(n(5)as u32)},
  107=>{let row=app_catalog::INCARNATIONS[n(1)as usize];let mut e=app_custody::Entry::new(row.lifecycle_slot,row.target,n(2)as u32,row.surface);e.dead=n(3)!=0;e.close_pending=n(4)!=0;APP_CUSTODY[n(1)as usize]=Some(e)},
  108=>{SCENE_APPEARANCE_STATE.preset_idx=n(1)as u8;SCENE_APPEARANCE_STATE.use_custom_colors=n(2)as u8;ACTIVE_TINT_IDX=n(3)as u8},
  120=>{let mut s:SceneLayoutSnapshot=core::mem::zeroed();s.magic=SNAP_MAGIC;s.version=SNAP_VERSION;s.frame_count=1;
   s.frames[MAX_FRAMES-1].present=1;s.frames[MAX_FRAMES-1].frame_id=73;s.frames[MAX_FRAMES-1].tab_count=255;
   s.checksum=snap_compute_checksum(&s);println!("COUNTEREXAMPLE {} {}",snap_validate(&s)as u8,s.frames[MAX_FRAMES-1].tab_count)},
  0=>collect_atlas_snapshot(),1=>println!("R {}",surface_in_active_scene(n(1))as u8),2=>println!("R {}",surface_scene_id(n(1)).map_or(256,|x|x as u32)),
  3=>clear_focus_if_wrong_scene(),4=>sync_lifecycle_scene_visibility(),5=>sync_scene_visibility(),
  6=>{print!("R ");for b in atlas_default_label(n(1)as u32){print!("{:02x}",b)}println!()},7=>scene_init_all(),8=>scene_update_flags(n(1)as u8),9=>println!("R {}",validate_scene_id(n(1)as u8)as u8),
  10=>{let x=scene_accent_token(n(1)as u8);println!("R {}",x)},11=>{let x=scene_is_pinned(n(1)as u8);println!("R {}",x as u8)},12=>{let x=scene_label_token(n(1)as u8);print!("R ");for b in x{print!("{:02x}",b)}println!()},
  13=>atlas_capture_snapshot(),14=>println!("R {}",atlas_is_enabled()as u8),15=>atlas_toggle(),16=>atlas_exit(),17=>{let(x,y,w,h)=atlas_card_pos(n(1)as usize,n(2)as u32);println!("R {} {} {} {}",x,y,w,h)},
  18=>{let x=atlas_scene_at_point(n(1)as i32,n(2)as i32);println!("R {}",x.map_or(256,|x|x as u32))},19=>{let x=handle_atlas_keyboard(n(1)as u8);println!("R {}",x as u8)},20=>atlas_render_stub(),21=>atlas_clear_stub(),
  22=>atlas_apply_scene_accent_to_chrome(n(1)as u8),23=>switch_scene(n(1)as u8),24=>next_scene(),25=>prev_scene(),26=>{let x=atlas_cycle_scene(n(1)as i32,"driver");println!("R {}",x as u8)},
  27=>{let x=next_frame_in_scene(n(1)as u32,n(2)!=0);println!("R {}",x.map_or(u64::MAX,|x|x as u64))},28=>focus_next_frame(),29=>focus_prev_frame(),30=>focus_next_tab(),31=>focus_prev_tab(),
  32=>{let x=toggle_minimize_focused_frame();println!("R {}",x as u8)},33=>{let x=toggle_zoom_focused_frame();println!("R {}",x as u8)},34=>{let x=close_focused_tab_or_frame_safe();println!("R {}",x as u8)},
  35=>println!("R {}",atlas_same_scene_drop_noop(n(1)as u32,n(2)as u8,n(3)as u8)as u8),36=>{let x=reparent_frame_to_scene(n(1)as u32,n(2)as u8,"driver");println!("R {}",x as u8)},
  37=>{let x=atlas_pointer_drag_begin_at(n(1)as i32,n(2)as i32);println!("R {}",x as u8)},38=>{let x=atlas_pointer_drop_at(n(1)as i32,n(2)as i32);println!("R {}",x as u8)},39=>{let x=atlas_pointer_drag_cancel("driver");println!("R {}",x as u8)},
  40=>println!("R {}",snap_compute_checksum(&SCENE_SNAPSHOT)),41=>snap_capture_layout(),42=>println!("R {}",snap_validate(&SCENE_SNAPSHOT)as u8),43=>{let x=snap_restore_layout();println!("R {}",x as u8)},
  _=>panic!("command")
 }state();}
 }
}
