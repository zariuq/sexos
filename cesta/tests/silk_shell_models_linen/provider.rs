static mut STATUS:u64=0;
fn pdx_call_checked(slot:u64,op:u64,a:u64,b:u64,c:u64)->Result<(),u64>{println!("call {} {} {} {} {}",slot,op,a,b,c);unsafe{if STATUS==0{Ok(())}else{Err(STATUS)}}}
unsafe fn linen_paint_surface_fast(){println!("paint {} {}",SELECTED_LINEN_OBJECT_ID,LINEN_REMOTE_REAL as u8);}
unsafe fn trace(){
 print!("S {} {} {}",SELECTED_LINEN_OBJECT_ID,LINEN_REMOTE_FETCHED as u8,LINEN_REMOTE_REAL as u8);
 for row in LINEN_OBJECTS{if let Some(o)=row{
  print!(" {}:{}:{}:{}:{}:",o.object_id,o.kind as u8,o.state as u8,o.name_len,o.disk_path);
  for byte in o.name{print!("{:02x}",byte);}
  print!(":{}:{}:{}:{}:{}:{}",o.parent_id,o.project_id,o.grant_ref,o.linked_surface_id,o.flags,o.display_name);
 }else{print!(" -");}}
 println!(" |pending {} {}",LINEN_ASYNC.busy()as u8,LINEN_ASYNC.can_send()as u8);
}
fn main(){use std::io::BufRead;for line in std::io::stdin().lock().lines(){let line=line.unwrap();let args:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();let n=|i:usize|args.get(i).copied().unwrap_or(0);unsafe{
 match n(0){
  0=>{LINEN_OBJECTS=[None;LINEN_MAX_OBJECTS];for i in 0..LINEN_SEED_OBJECTS.len(){LINEN_OBJECTS[i]=Some(LINEN_SEED_OBJECTS[i]);}SELECTED_LINEN_OBJECT_ID=0;LINEN_REMOTE_FETCHED=false;LINEN_REMOTE_REAL=false;LINEN_ASYNC=linen_async::Snapshot::new();STATUS=0;},
  1=>linen_fetch_remote_snapshot(),
  2=>linen_queue_open(n(1),n(2)),
  3=>linen_async_poll(),
  4=>{let m=PdxMessage{caller_pd:n(1)as u32,caller_task:700,type_id:n(2),arg0:n(3),arg1:0,arg2:0};println!("R {}",linen_async_reply(&m)as u8);},
  5=>SELECTED_LINEN_OBJECT_ID=n(1),
  6=>STATUS=n(1),
  _=>panic!("unknown operation")};trace();
 }}}
