pub fn probe_issuer(last:u64)->TokenIssuer { TokenIssuer{last:AtomicU64::new(last)} }
pub fn probe_last(s:&TokenIssuer)->u64{s.last.load(Ordering::Relaxed)}
pub fn probe_snapshot(slot:&FaultSlot)->Vec<u64>{
 let s=unsafe{&*slot.state.get()};let mut out=Vec::new();
 let mut push=|r:FaultRecord|out.extend([r.token,r.target_pd as u64,r.pager_pd as u64,r.task_id as u64,r.fault_addr,r.page_addr,r.error_code,r.saved_rip,r.saved_frame]);
 let tag=match s.active{Active::Idle=>0,Active::Prepared(_)=>1,Active::Pending(_)=>2,Active::Claimed(_)=>3};
 // Tag precedes optional record; build separately to keep borrow scopes explicit.
 match s.active{Active::Idle=>{},Active::Prepared(r)|Active::Pending(r)|Active::Claimed(r)=>push(r)}
 let mut full=vec![tag];full.extend(out);full.push(s.completed.is_some() as u64);
 if let Some(r)=s.completed{full.extend([r.token,r.target_pd as u64,r.pager_pd as u64,r.task_id as u64,r.fault_addr,r.page_addr,r.error_code,r.saved_rip,r.saved_frame]);}
 full.extend([s.last_token,s.retired as u64]);full
}
