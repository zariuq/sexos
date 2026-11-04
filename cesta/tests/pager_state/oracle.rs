#[path="actual_fault.rs"]mod fault;
#[path="actual_paging.rs"]mod pg;
use std::io::{self,Read};
fn word(n:u64){print!(" {}",n);}
fn fr(r:fault::FaultRecord){for x in [r.token,r.target_pd as u64,r.pager_pd as u64,r.task_id as u64,r.fault_addr,r.page_addr,r.error_code,r.saved_rip,r.saved_frame]{word(x);}}
fn binding(b:pg::Binding){for x in [b.token,b.task_id as u64,b.target_pd as u64,b.pager_pd as u64,b.range_id as u64,b.page_index as u64,b.frame_index as u64,b.rights as u64]{word(x);}}
fn main(){
 let mut input=String::new();io::stdin().read_to_string(&mut input).unwrap();
 let mut f=fault::FaultSlot::new();let mut ledger=pg::MappingLedger::new();let mut issuer=fault::probe_issuer(0);
 for line in input.lines(){
  let v:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();assert_eq!(v.len(),24);
  let b=pg::Binding{token:v[1],task_id:v[2] as u32,target_pd:v[3] as u32,pager_pd:v[4] as u32,range_id:v[5] as u32,page_index:v[6] as usize,frame_index:v[7] as usize,rights:v[8] as u8};
  let mut status=0;let mut extra=0;let mut record=None;let mut validated=None;let mut commit=None;let mut plan=None;
  match v[0]{
   0=>{f=fault::FaultSlot::new();ledger=pg::MappingLedger::new();issuer=fault::probe_issuer(v[1]);},
   1=>extra=fault::eligible_user_fault(v[1],v[2],v[3]) as u64,
   2=>match issuer.issue(){Ok(n)=>extra=n,Err(e)=>status=e},
   3=>status=f.prepare(fault::FaultRecord{token:v[1],target_pd:v[2] as u32,pager_pd:v[3] as u32,task_id:v[4] as u32,fault_addr:v[5],page_addr:v[6],error_code:v[7],saved_rip:v[8],saved_frame:v[9]}).err().unwrap_or(0),
   4=>status=f.publish(v[1]).err().unwrap_or(0),
   5=>status=f.cancel_prepared(v[1]).err().unwrap_or(0),
   6|7|11=>{let r=match v[0]{6=>f.query(v[1] as u32,v[2]),7=>f.claim(v[1] as u32,v[2]),_=>f.reject(v[1] as u32,v[2])};match r{Ok(r)=>record=Some(r),Err(e)=>status=e}},
   8=>status=f.rollback(v[1] as u32,v[2]).err().unwrap_or(0),
   9=>match f.complete(v[1] as u32,v[2]){Ok(r)=>record=r,Err(e)=>status=e},
   10=>status=unsafe{f.retire_exited(v[1] as u32,v[2] as u32,v[3] as u32)}.err().unwrap_or(0),
   20=>match pg::validate(pg::PagerMappingGrant{range_id:v[1] as u32,pool_id:v[2] as u32,target_pd:v[3] as u32,base_va:v[4],length:v[5],rights:v[6] as u8,quota:v[7] as u8},pg::FramePoolGrant{pool_id:v[8] as u32,base_va:v[9],length:v[10]},v[11] as u32,pg::FaultView{task_id:v[12] as u32,token:v[13],target_pd:v[14] as u32,pager_pd:v[15] as u32,page:v[16],error:v[17]},v[18],v[19]){Ok(b)=>validated=Some(b),Err(e)=>status=e},
   21=>status=ledger.install(b,v[9] as u8).err().unwrap_or(0),
   22=>status=ledger.check_new(b,v[9] as u8).err().unwrap_or(0),
   23=>status=ledger.finalize(b).err().unwrap_or(0),
   24=>status=ledger.retire(b).err().unwrap_or(0),
   25=>commit=ledger.token(v[1]),
   26=>extra=ledger.is_retired(v[1] as u32,v[2]) as u64,
   27=>match ledger.cleanup_plan(v[1] as u32,v[2] as u32,v[3] as u32,v[4] as u32){Ok(p)=>plan=Some(p),Err(e)=>status=e},
   28=>status=ledger.close_exited_range(v[1] as u32,v[2] as u32,v[3] as u32,v[4] as u32).err().unwrap_or(0),
   _=>panic!()
  }
  print!("R");word(status);word(extra);word(record.is_some() as u64);if let Some(r)=record{fr(r)}
  if let Some(b)=validated{binding(b)}
  if v[0]==25{word(commit.is_some() as u64);if let Some(c)=commit{word(c.finalized as u64);binding(c.binding)}}
  if let Some(p)=plan{for b in p{word(b.is_some() as u64);if let Some(b)=b{binding(b)}}}
  print!(" F");word(fault::probe_last(&issuer));for x in fault::probe_snapshot(&f){word(x)}
  print!(" L");for c in ledger.entries{word(c.is_some() as u64);if let Some(c)=c{word(c.finalized as u64);binding(c.binding)}}
  for b in pg::probe_retired(&ledger){word(b.is_some() as u64);if let Some(b)=b{binding(b)}}
  for (high,closed) in pg::probe_closed(&ledger){word(high);word(closed.is_some() as u64);if let Some(c)=closed{word(c.target_pd as u64);word(c.task_id as u64);word(c.frames as u64)}}
  println!();
 }
}
