#[path="LAUNCHER_PATH"]mod launcher;
mod linen_async{
    pub struct Snapshot;
    impl Snapshot{pub const fn new()->Self{Self}pub fn refresh(&mut self,notify:bool){println!("surface linen-refresh {}",notify as u8);}}
}
static mut ACTIVATIONS:[(u64,u64,u64,u64,u64);512]=[(3,0,0,0,0);512];
static mut LAUNCH_AVAILABLE:bool=true;
static mut LAUNCH_FOCUS:Option<bool>=None;
static mut ACCESS_DECISION:u8=0;
static mut STALE_STATUS:u64=0xfffffffffffffffe;
static mut REPLAY_CORRUPT:bool=false;
static mut COMMIT_COUNT:[u32;512]=[0;512];
fn activation_raw(slot:usize,commit:bool)->(u64,u64,u64,u64,u64){
    println!("surface activation {} {}",slot,commit as u8);
    unsafe{if commit{COMMIT_COUNT[slot]+=1;}if commit && ACTIVATIONS[slot].0==0 && ACTIVATIONS[slot].4==1{ACTIVATIONS[slot].4=2;}let mut result=ACTIVATIONS[slot];if REPLAY_CORRUPT&&commit&&COMMIT_COUNT[slot]>=2{result.4=3;}result}
}
unsafe fn native_launcher_status(target:u32)->(bool,&'static str,&'static str){println!("surface available {}",target);(LAUNCH_AVAILABLE,"fixture","fixture")}
unsafe fn launcher_focus_existing(target:u32,frame:u64,action:Option<launcher::LiveAction>)->Option<bool>{
    println!("surface focus-existing {} {} {} {}",target,frame,action.is_some()as u8,action.map_or(0,|x|x as u8));LAUNCH_FOCUS
}
unsafe fn launcher_action_record(target:u32,frame:u64,action:launcher::LiveAction,from:u8,activated:bool,ok:bool){
    println!("surface action-record {} {} {} {} {} {}",target,frame,action as u8,from,activated as u8,ok as u8);
}
fn raw_pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64){println!("surface rawcall {} {} {} {} {}",slot,op,a,b,c);unsafe{(STALE_STATUS,0)}}
unsafe fn linen_paint_surface(){println!("surface linen-paint");}
unsafe fn linen_paint_surface_fast(){println!("surface linen-fast");}
unsafe fn quil_buffer_count()->usize{println!("surface quil-count");5}
unsafe fn quil_render_buffer_list(){println!("surface quil-render");}
unsafe fn mesh_render_fact_list(){println!("surface mesh-render");}
unsafe fn mesh_emit_linen_quil_links(){println!("surface mesh-links");}
unsafe fn collar_render_grants_text(){println!("surface collar-render");}
unsafe fn collar_auto_grant_from_manifest(m:&AppManifest){println!("surface autogrant {} {}",m.surface_id,m.capabilities.bits());}
unsafe fn shell_registration_reply_poll(){print!("surface registration-poll");for e in APP_CUSTODY.iter().flatten(){if e.registration_reply{print!(" {}",e.target)}}println!();}
unsafe fn spindle_render(){println!("surface spindle-render");}
unsafe fn bell_render_event_list(){println!("surface bell-render");}
unsafe fn collar_check_operation(_op:CollarOperation,caller:u64,object:u64)->CollarDecision{
    println!("surface collar-check {} {} {}",7,caller,object);
    match ACCESS_DECISION{0=>CollarDecision::Allow,1=>CollarDecision::Deny,2=>CollarDecision::DenyMissingObject,3=>CollarDecision::DenyMissingBuffer,4=>CollarDecision::NeedsGrantLater,_=>CollarDecision::BlockedStopFirst}
}
