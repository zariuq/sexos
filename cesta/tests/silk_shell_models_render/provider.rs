fn pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64){println!("D {} {} {} {} {}",slot,op,a,b,c);(0,0)}
unsafe fn set_frame_top_bar(id:u32,value:bool){println!("T {} {}",id,value as u8);}
unsafe fn send_frame_tab_info(id:u32){println!("F {}",id);}
static mut STARTUP:bool=false;
fn startup_messages_pending()->bool{unsafe{STARTUP}}
fn pdx_call_checked(slot:u64,op:u64,a:u64,b:u64,c:u64)->Result<(),u64>{let (status,_)=pdx_call(slot,op,a,b,c);if status==0{Ok(())}else{Err(status)}}
impl linen_text_delivery::Outbox {
 fn trace(&self){
  fn batch(b:&linen_text_delivery::Batch){print!(" {} {} {} {} {} {} {} {} {} {} ",b.len,b.color,b.offset,b.sequence,b.cleared as u8,b.awaiting as u8,b.metadata.count,b.metadata.first,b.metadata.rows,b.metadata.selected);for byte in &b.bytes[..b.len]{print!("{:02x}",byte);}}
  print!("S {} {} {}",self.sequence,self.active.is_some()as u8,self.latest.is_some()as u8);if let Some(b)=&self.active{batch(b)}print!("|");if let Some(b)=&self.latest{batch(b)}println!();
 }
}
fn main(){use std::io::BufRead;for line in std::io::stdin().lock().lines(){let line=line.unwrap();let words:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();let n=|i:usize|words.get(i).copied().unwrap_or(0);unsafe{
 match n(0){
 0=>{LINEN_OBJECTS=[None;16];SURFACE_200_W=0;SURFACE_200_H=0;SURFACE_200_X=0;SURFACE_200_Y=0;LINEN_UI_SELECTED=0;SELECTED_LINEN_OBJECT_ID=0;LINEN_TEXT=linen_text_delivery::Outbox::atomic(200);LINEN_ASYNC=linen_async::Snapshot::new();LINEN_REMOTE_FETCHED=false;LINEN_REMOTE_REAL=false;},
 1=>{SURFACE_200_W=n(1)as u32;SURFACE_200_H=n(2)as u32;SURFACE_200_X=n(3)as u32 as i32;SURFACE_200_Y=n(4)as u32 as i32;SELECTED_LINEN_OBJECT_ID=n(5);LINEN_UI_SELECTED=n(6)as u8;},
 2=>{let mut o=LINEN_SEED_OBJECTS[0];o.object_id=n(2);o.display_name="Fallback label with a deliberately long but static lifetime";o.name_len=n(3)as u8;for i in 0..24{o.name[i]=if n(4)!=0{(i as u64*13+n(1)*7)as u8}else{65+((i as u64+n(1))%26)as u8};}LINEN_OBJECTS[n(1)as usize]=Some(o);},
 3=>linen_render_object_list(),4=>linen_render_static_ui(),
 5=>{use linen_text_delivery::{Admission,Poll};let p=LINEN_TEXT.poll(n(1)as usize,|op|{println!("O {} {} {} {}",op.opcode,op.surface,op.packed,op.control);Admission::Accepted});println!("P {}",match p{Poll::Idle=>0,Poll::Pending=>1,Poll::Deferred=>2,Poll::Complete(_)=>3,Poll::Rejected(_)=>4});},
 6=>{if let Some(r)=LINEN_TEXT.receive(n(1)as u32,n(2)){println!("R 1 {} {} {} {} {}",r.bytes,r.metadata.count,r.metadata.first,r.metadata.rows,r.metadata.selected);}else{println!("R 0 0 0 0 0 0");}},
 7=>LINEN_TEXT.cancel(),
 8=>{STARTUP=n(1)!=0;linen_paint_surface();},9=>linen_paint_surface_fast(),
 10=>{LINEN_REMOTE_FETCHED=n(1)!=0;LINEN_REMOTE_REAL=n(2)!=0;},11=>linen_async_poll(),
 12=>{let m=PdxMessage{caller_pd:n(1)as u32,caller_task:700,type_id:n(2),arg0:n(3),arg1:0,arg2:0};println!("A {}",linen_async_reply(&m)as u8);},
 _=>panic!("unknown operation")
 }LINEN_TEXT.trace();
 }}}
