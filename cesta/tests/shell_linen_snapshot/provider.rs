mod snapshot;
mod policy;
fn main(){
 use std::io::BufRead;
 let mut s=snapshot::Snapshot::new();
 if let Some(name)=std::env::args().nth(1){
  if name=="sent-twice"{s.refresh(false);s.request();s.sent();}s.sent();return;
 }
 for line in std::io::stdin().lock().lines(){let line=line.unwrap();let values:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();let n=|i:usize|values.get(i).copied().unwrap_or(0);
  match n(0){
   0=>s=snapshot::Snapshot::new(),
   1=>s.refresh(n(1)!=0),
   2=>println!("R {}",s.queue_open(n(1),n(2))as u8),
   3=>println!("R {} {}",s.busy()as u8,s.can_send()as u8),
   4=>if let Some(r)=s.request(){println!("R 1 {} {} {} {}",r.opcode,r.args[0],r.args[1],r.args[2]);}else{println!("R 0");},
   5=>s.sent(),
   6=>{use snapshot::Reply::*;let(kind,value)=match s.reply(n(1)as u32,n(2),n(3)){Ignored=>(0,0),Progress=>(1,0),Published=>(2,0),Rejected=>(3,0),OpenComplete(v)=>(4,v)};println!("R {} {}",kind,value);},
   7=>{println!("R {}",policy::allows(n(1),n(2)as u8,n(3),n(4),n(5)as u8,n(6)!=0,n(7)!=0,n(8)!=0)as u8);continue;},
   _=>panic!("unknown operation")
  }s.trace();
 }
}
