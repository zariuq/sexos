extern crate self as sex_pdx;
#[path="actual_memory_region.rs"] pub mod memory_region;
#[path="actual_capability_slot.rs"] mod capability_slot;
mod ipc { pub const MAX_DOMAINS: usize=1024; }
mod capability {
    use crate::memory_region::RegionGrant;
    #[derive(Clone,Copy)] pub struct Pci {pub bar0_view:Option<RegionGrant>}
    #[derive(Clone,Copy)] pub enum CapabilityData {Region(RegionGrant),Pci(Pci),Other}
    #[derive(Clone,Copy)] pub struct Capability {pub data:CapabilityData}
    pub struct ProtectionDomain {pub held:Option<Capability>}
    impl ProtectionDomain {
        pub fn find_capability(&self,slot:u32)->Option<Capability> {if slot==37 {self.held}else{None}}
    }
}
include!("actual_region_query.rs");
use memory_region::*;
use std::io::{self,Read};
fn error(e:RegionError)->u64 {match e {
    RegionError::InvalidCapability=>1,RegionError::InvalidRequest=>2,
    RegionError::OutOfRange=>3,RegionError::PermissionDenied=>4,RegionError::WrongPeer=>5}}
fn view(result:Result<RegionView,RegionError>)->[u64;7] {match result {
    Ok(r)=>[0,r.region_id as u64,r.virtual_address,r.length,r.dma_address.is_some() as u64,r.dma_address.unwrap_or(0),0],
    Err(e)=>[error(e),0,0,0,0,0,0]}}
fn main(){
    let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
    for line in text.lines(){
        let v:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();assert_eq!(v.len(),14);
        let g=RegionGrant{region_id:v[1] as u32,virtual_base:v[2],length:v[3],rights:v[4] as u8,
            dma_base:if v[5]!=0{Some(v[6])}else{None},peer_pd:if v[7]!=0{Some(v[8] as u32)}else{None}};
        let c=if v[0]!=0{Some(&g)}else{None};let q=ViewRequest{offset:v[10],length:v[11],rights:v[12] as u8};
        let out=match v[9] {
            0=>view(resolve_view(c,q)),1=>view(resolve_register_view(c,v[10],v[11],v[12])),
            2=>match resolve_info(c){Ok(r)=>[0,r.region_id as u64,r.length,r.rights as u64,r.peer_pd.is_some() as u64,r.peer_pd.unwrap_or(0) as u64,0],Err(e)=>[error(e),0,0,0,0,0,0]},
            3=>view(resolve_peer_view(c,v[13] as u32,q)),4=>[0,valid_probe(&g) as u64,0,0,0,0,0],
            n=>{let pd=capability::ProtectionDomain{held:c.copied().map(|g|capability::Capability{data:capability::CapabilityData::Region(g)})};
                let r=region_query(&pd,n,37,v[10],v[11],v[12]);[r[0],r[1],r[2],r[3],r[4],0,0]}
        };
        for (i,x) in out.iter().enumerate(){print!("{}{}",if i==0{""}else{" "},x);}println!();
    }
}
