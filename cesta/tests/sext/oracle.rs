extern crate self as sex_pdx;
mod memory_region { include!(concat!(env!("SEXT_REF_DIR"),"/crates/sex-pdx/src/memory_region.rs")); }
mod paging { include!(concat!(env!("SEXT_REF_DIR"),"/crates/sex-pdx/src/paging.rs")); }
mod allocator {
    include!(concat!(env!("SEXT_REF_DIR"),"/servers/sext/src/allocator.rs"));
    impl<const N:usize> FrameAllocator<N> {
        pub fn projection(&self) {
            print!(" q{}",self.quota);
            for f in &self.frames {print!(" F {} {} {} {} {}",f.state as u8,f.token,f.target,f.page,f.zeroed as u8);}
        }
    }
}
mod pager {
    include!(concat!(env!("SEXT_REF_DIR"),"/servers/sext/src/pager.rs"));
    impl<const N:usize> Pager<N> {
        pub fn projection(&self) {
            print!(" pool {} {} {} {}",self.pool.base,self.pool.length,self.pool.target_pd,self.pool.rights);self.allocator.projection();
            for i in 0..2 {
                if let Some(m)=self.mappings[i]{print!(" M {} {} {} {} {}",m.slot,m.info.base,m.info.length,m.info.target_pd,m.info.rights);}else{print!(" M-");}
                if let Some(a)=self.active[i]{
                    print!(" A {} {} {} {} {}",a.notice.fault_addr,a.notice.error_code,a.notice.token,a.notice.caller_pd,a.may_have_published as u8);
                    match a.operation {Operation::Commit{index,rights}=>print!(" C {} {}",index,rights),Operation::Reject(r)=>print!(" R {}",r as u8)}
                }else{print!(" A-");}
                if let Some(r)=self.releases[i]{print!(" L {} {} {}",r.request.page,r.request.request_id,r.request.caller_pd);crate::binding(r.binding);
                    if let Some(s)=r.result{print!(" D {}",s);}else{print!(" P");}
                }else{print!(" L-");}
                if let Some(x)=self.closed[i]{print!(" X {} {}",x.frame_bitmap,x.task_id);}else{print!(" X-");}
            }
        }
    }
}
use allocator::*;
use pager::*;
use paging::*;
use std::io::{self,Read};
const N:usize=@COUNT@;
struct Fake { query_status:u64,query:FaultQuery,commit:u64,reject:u64,reclaim:u64,cleanup:u64,closed:CleanupResult,bytes:Vec<u8> }
impl Backend for Fake {
    fn query(&mut self,m:u64,t:u64)->Result<FaultQuery,u64>{println!("query {} {}",m,t);if self.query_status==0{Ok(self.query)}else{Err(self.query_status)}}
    fn zero(&mut self,address:u64,bytes:u64){println!("zero {} {}",address,bytes);let start=(address-0x400000)as usize;self.bytes[start..start+bytes as usize].fill(0);}
    fn commit(&mut self,m:u64,i:usize,t:u64,r:u8)->u64{println!("commit {} {} {} {}",m,i,t,r);self.commit}
    fn reject(&mut self,m:u64,t:u64)->u64{println!("reject {} {}",m,t);self.reject}
    fn reclaim(&mut self,m:u64,i:usize,t:u64)->u64{println!("reclaim {} {} {}",m,i,t);self.reclaim}
    fn cleanup(&mut self,m:u64)->Result<CleanupResult,u64>{println!("cleanup {}",m);if self.cleanup==0{Ok(self.closed)}else{Err(self.cleanup)}}
}
fn binding(b:Option<Binding>){if let Some(b)=b{print!(" B {} {} {} {}",b.index,b.token,b.target,b.page);}else{print!(" -");}}
fn bindings<const M:usize>(b:[Option<Binding>;M]){for b in b{binding(b);}}
fn state(a:&FrameAllocator<N>,p:&Option<Pager<N>>,fake:&Fake){
    print!("state A");a.projection();print!(" P{}",p.is_some()as u8);if let Some(p)=p{p.projection();}
    let hash=fake.bytes.iter().fold(14695981039346656037u64,|h,b|(h^*b as u64).wrapping_mul(1099511628211));println!(" bytes {}",hash);
}
fn outcome(o:Outcome){match o{
    Outcome::Retry=>println!("out 0"),Outcome::Ignored=>println!("out 1"),Outcome::Fatal(s)=>println!("out 2 {}",s),
    Outcome::Committed{index,page,target,token}=>println!("out 3 {} {} {} {}",index,page,target,token),
    Outcome::Rejected{reason,page,target,token}=>println!("out 4 {} {} {} {}",reason as u8,page,target,token)}}
fn codec(kind:u64,op:u64,caller:u64,a:u64,b:u64,c:u64){
    match kind{
        0=>match FaultNotice::decode(op,caller,[a,b,c]){Ok(o)=>println!("codec 1 {} {} {} {}",o.fault_addr,o.error_code,o.token,o.caller_pd),Err(_)=>println!("codec 0")},
        1=>match ReleaseRequest::decode(op,caller,[a,b,c]){Ok(o)=>println!("codec 1 {} {} {}",o.page,o.request_id,o.caller_pd),Err(_)=>println!("codec 0")},
        2=>match ExitNotice::decode(op,caller,[a,b,c]){Ok(o)=>println!("codec 1 {} {}",o.target_pd,o.task_id),Err(_)=>println!("codec 0")},
        3=>match WaitCleanupRequest::decode(op,caller,[a,b,c]){Ok(o)=>println!("codec 1 {} {} {}",o.target_pd,o.request_id,o.caller_pd),Err(_)=>println!("codec 0")},
        4=>println!("value {}",release_reply(a,b)),
        _=>match release_reply_status(a,b){Ok(s)=>println!("codec 1 {}",s),Err(_)=>println!("codec 0")}}
}
fn main(){
    let mut allocator=FrameAllocator::<N>::new(4);let mut pager:Option<Pager<N>>=None;
    let mut pool=PagingInfo{base:0x400000,length:N as u64*4096,target_pd:0,rights:6};
    let mut mappings=[Some(Mapping{slot:36,info:PagingInfo{base:0x800000,length:32768,target_pd:15,rights:6}}),
                      Some(Mapping{slot:37,info:PagingInfo{base:0x900000,length:32768,target_pd:10,rights:6}})];
    let mut fake=Fake{query_status:0,query:FaultQuery{page_va:0x800000,error_code:4,target_pd:15,allowed_rights:6},commit:0,reject:0,reclaim:0,cleanup:5,closed:CleanupResult{frame_bitmap:0,task_id:0},bytes:vec![0xa5;N*4096]};
    let mut input=String::new();io::stdin().read_to_string(&mut input).unwrap();
    for line in input.lines(){let v:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();assert_eq!(v.len(),9);
        print!("cmd");for x in &v{print!(" {}",x);}println!();
        let(a,b,c,d,e,f,g)=(v[1],v[2],v[3],v[4],v[5],v[6],v[7]);
        match v[0]{
            0=>allocator=FrameAllocator::new(a as usize),
            1=>match allocator.reserve(a,b as u32,c){Ok(r)=>println!("reserve 1 {} {}",r.index,r.needs_zero as u8),Err(e)=>println!("reserve 0 {}",e as u8)},
            2=>println!("bool {}",allocator.mark_zeroed(a as usize,b)as u8),
            3=>println!("bool {}",allocator.commit(a as usize,b)as u8),
            4=>println!("bool {}",allocator.rollback_unpublished(a as usize,b)as u8),
            5=>{print!("binding");binding(allocator.begin_reclaim(a as u32,b));println!();},
            6=>println!("bool {}",allocator.finish_reclaim(Binding{index:a as usize,token:b,target:c as u32,page:d})as u8),
            7=>println!("count {}",allocator.finish_target_cleanup(a as u32)),
            8=>println!("count {} {}",allocator.used_by(a as u32),allocator.used()),
            9=>{print!("bindings");bindings(allocator.bindings_for(a as u32));println!();},
            10=>println!("mask {}",allocator.target_mask(a as u32)),
            11=>pool=PagingInfo{base:a,length:b,target_pd:c as u32,rights:d as u8},
            12=>mappings[a as usize]=if b!=0{Some(Mapping{slot:c,info:PagingInfo{base:d,length:e,target_pd:f as u32,rights:g as u8}})}else{None},
            13=>match Pager::<N>::new_multi(pool,mappings,a as usize){Ok(p)=>{pager=Some(p);println!("init 0");},Err(e)=>println!("init {}",e)},
            14=>if let Some(p)=&mut pager{outcome(p.step(&mut fake,FaultNotice{fault_addr:a,error_code:b,token:c,caller_pd:d as u32}));}else{println!("uninitialized");},
            15=>if let Some(p)=&mut pager{match p.release(&mut fake,ReleaseRequest{page:a,request_id:b,caller_pd:c as u32}){
                ReleaseOutcome::Retry=>println!("release 0"),ReleaseOutcome::Fatal(s)=>println!("release 1 {}",s),
                ReleaseOutcome::Done{status,binding:b,replay}=>{print!("release 2 {}",status);binding(b);println!(" {}",replay as u8);}}}else{println!("uninitialized");},
            16=>if let Some(p)=&mut pager{match p.cleanup(&mut fake,ExitNotice{target_pd:a as u32,task_id:b}){
                CleanupOutcome::Retry=>println!("cleanup_out 0"),CleanupOutcome::Live=>println!("cleanup_out 1"),CleanupOutcome::Ignored=>println!("cleanup_out 2"),
                CleanupOutcome::Fatal(s)=>println!("cleanup_out 3 {}",s),CleanupOutcome::Closed{result,released,replay}=>println!("cleanup_out 4 {} {} {} {}",result.frame_bitmap,result.task_id,released,replay as u8)}}else{println!("uninitialized");},
            17=>if let Some(p)=&pager{match p.wait_cleanup(WaitCleanupRequest{target_pd:a as u32,request_id:b,caller_pd:c as u32}){WaitOutcome::Pending=>println!("wait 0"),WaitOutcome::Reply(s)=>println!("wait 1 {}",s)}}else{println!("uninitialized");},
            18=>if let Some(p)=&pager{let target=a as u32;print!("lookup {}",p.client_index(target).unwrap_or(2));if let Some(m)=p.mapping_for(target){print!(" M {} {} {} {} {}",m.slot,m.info.base,m.info.length,m.info.target_pd,m.info.rights);}else{print!(" M-");}if let Some(x)=p.closed_result(target){print!(" X {} {}",x.frame_bitmap,x.task_id);}else{print!(" X-");}print!(" used {} {}",p.used_by(target),p.used());bindings(p.bindings_for(target));println!();}else{println!("uninitialized");},
            19=>codec(a,b,c,d,e,f),
            20=>{fake.query_status=a;fake.query=FaultQuery{page_va:b,error_code:c,target_pd:d as u32,allowed_rights:e as u8};},
            21=>{fake.commit=a;fake.reject=b;fake.reclaim=c;fake.cleanup=d;fake.closed=CleanupResult{frame_bitmap:e,task_id:f};},
            22=>{if let Some(cell)=fake.bytes.get_mut(a as usize){*cell=b as u8;}},
            23=>fake.bytes.fill(0xa5),
            _=>panic!("unknown operation")
        }
        state(&allocator,&pager,&fake);
    }
}

