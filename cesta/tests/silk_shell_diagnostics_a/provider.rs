static mut STATUS:u64=0;
static mut FAIL_AT:u64=u64::MAX;
static mut CALL_COUNT:u64=0;
const LABELS:[&str;5]=["begin","move","release","shell",""];
fn pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64){unsafe{
    println!("call {} {} {} {} {}",slot,op,a,b,c);
    let result=if FAIL_AT==u64::MAX || CALL_COUNT==FAIL_AT{STATUS}else{0};
    CALL_COUNT+=1;(result,73)
}}
unsafe fn snapshot(){
    let flags:u64=/* OBSERVE_FIELDS */;
    print!("S {} {} {} {} ",SILK_TEXT_LEN,SILK_TEXT_PROOF_DONE_EMITTED as u8,flags,SHELL_INTERACTION_CONTRACT_STAGE_MASK);
    for i in 0..SILK_TEXT_SINK_CAP{print!("{:02x}",SILK_TEXT_BUF[i]);}println!();
}
fn main(){
    use std::io::BufRead;
    for line in std::io::stdin().lock().lines(){
        let line=line.unwrap();let values:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();
        let n=|i:usize|values.get(i).copied().unwrap_or(0);
        unsafe{match n(0){
        0=>{/* RESET_FIELDS */SILK_TEXT_BUF=[0;SILK_TEXT_SINK_CAP];SILK_TEXT_LEN=0;SILK_TEXT_PROOF_DONE_EMITTED=false;SHELL_INTERACTION_CONTRACT_STAGE_MASK=0;FOCUSED_SURFACE_ID=0;SURFACE_100_ALIVE=false;STATUS=0;FAIL_AT=u64::MAX;CALL_COUNT=0;},
        1=>println!("R {}",silk_text_sink_key(n(1)as u8,n(2))as u8),
        2=>{FOCUSED_SURFACE_ID=n(1);SURFACE_100_ALIVE=n(2)!=0;},
        3=>{SILK_TEXT_LEN=n(1)as usize;for i in 0..SILK_TEXT_SINK_CAP{SILK_TEXT_BUF[i]=(i*17+31)as u8;}},
        4=>println!("R {}",silk_text_sink_redraw()as u8),
        5=>{let text:Vec<u8>=(0..n(1)).map(|i|(i.wrapping_mul(37).wrapping_add(n(4)))as u8).collect();let r=shell_draw_text(n(3),&text,n(2));println!("R {} {}",r.0,r.1 as u8);},
        6=>println!("R {}",send_silkbar_phase2_update(n(1)as u32,n(2),n(3))as u8),
        7=>bell_send_app_event(LABELS[n(1)as usize],n(2)),
        8=>{STATUS=n(1);FAIL_AT=n(2);CALL_COUNT=0;},
        9=>match n(1){
            0=>input_negative_begin_once(),
            1=>input_negative_mark_done(),
            2=>surface_input_lifetime_begin_once(),
            3=>surface_input_lifetime_mark_focus_live(n(2),n(3)as u8,n(5)as u8),
            4=>surface_input_lifetime_mark_key_route(n(2),n(3)as u8,n(4)as u8,n(5)as u8),
            5=>surface_input_lifetime_mark_click_target(n(2),n(3)as u8,n(4)as u8,n(5)as u8),
            6=>surface_input_lifetime_mark_drag_target(n(2),n(3)as u8,LABELS[n(6)as usize],n(5)as u8),
            7=>surface_input_lifetime_mark_dead_clear(n(2)),
            8=>surface_input_lifetime_try_done(),
            9=>shell_interaction_contract_begin_once(),
            10=>{click_drag_proof_begin_click_once();click_drag_proof_begin_drag_once();},
            11=>shell_interaction_contract_mark_stage(n(2)as u8),
            12=>shell_interaction_contract_try_done(),
            _=>panic!("unknown marker")},
        10=>{/* SET_FIELDS */},
        11=>{let ch=sink_scancode_to_char(n(1)as u8);println!("R {} {} {} {}",is_spindle_text_key(n(1)as u8)as u8,is_quil_text_key(n(1)as u8)as u8,ch.is_some()as u8,ch.unwrap_or(0));},
        _=>panic!("unknown command")};snapshot();}
    }
}
