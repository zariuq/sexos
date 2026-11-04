extern crate self as sex_pdx;
extern crate self as x86_64;
#[path="actual_pci.rs"]pub mod pci_command;
#[path="actual_region.rs"]pub mod memory_region;
#[path="actual_network.rs"]pub mod network_config;
#[path="actual_boot_network_state.rs"]pub mod boot_network_state;
#[path="actual_slot.rs"]pub mod capability_slot;
#[path="actual_kernel_pci.rs"]mod kernel_pci;
pub mod ipc{pub const MAX_DOMAINS:usize=1024;}
pub const SLOT_DMA_REGION:u64=32;
pub mod nic_boot{pub const REGISTERS_END:u64=0x5408;}
#[macro_export]macro_rules! serial_println{($($x:tt)*)=>{{}}}

pub mod capability {
    use crate::{memory_region::RegionGrant,pci_command::Grant};
    #[derive(Clone,Copy)]pub struct Pci {pub bus:u8,pub dev:u8,pub func:u8,pub vendor_id:u16,pub device_id:u16,pub command:Option<Grant>,pub bar0_view:Option<RegionGrant>}
    #[derive(Clone,Copy)]pub enum CapabilityData{Pci(Pci),Region(RegionGrant),Domain}
    pub struct Capability {pub data:CapabilityData}
    pub struct ProtectionDomain{pub id:u32,pub device:Option<Capability>,pub dma:Option<Capability>}
    impl ProtectionDomain{pub fn find_capability(&self,slot:u32)->Option<&Capability>{match slot{19=>self.device.as_ref(),32=>self.dma.as_ref(),_=>None}}}
}
thread_local!{
    static INPUT:std::cell::RefCell<Vec<u64>>=const{std::cell::RefCell::new(Vec::new())};
    static DEVICE:std::cell::RefCell<Device>=const{std::cell::RefCell::new(Device{selected:0,word:0,reads:0,events:Vec::new()})};
}
struct Device {selected:u32,word:u32,reads:u32,events:Vec<(char,u32,u16,u32)>}
fn input(i:usize)->u64{INPUT.with(|v|v.borrow()[i])}
fn out(port:u16,value:u32,width:u32){DEVICE.with(|d|{
    let mut d=d.borrow_mut();d.events.push(('O',width,port,value));
    if port==0xcf8{assert_eq!(width,32);d.selected=value;}
    else{assert_eq!((port,width,d.selected&0xfc),(0xcfc,16,4));if input(44)!=1{d.word=(d.word&0xffff0000)|value;}}
})}
fn read(bus:u8,dev:u8,fun:u8,offset:u8)->u32{
    let address=hal::pci::config_address(bus,dev,fun,offset);out(0xcf8,address,32);
    DEVICE.with(|d|{let mut d=d.borrow_mut();let expected=hal::pci::config_address(input(11)as u8,input(12)as u8,input(13)as u8,offset);assert_eq!(address,expected);
        let value=if offset==0{input(42)as u32}else{assert_eq!(offset,4);d.reads+=1;d.word^if input(44)==2&&d.reads==2{1}else{0}};
        d.events.push(('I',32,0xcfc,value));value})
}
pub mod instructions {pub mod port {
    pub trait Value{fn value(self)->u32;fn width()->u32;}
    impl Value for u32{fn value(self)->u32{self}fn width()->u32{32}}
    impl Value for u16{fn value(self)->u32{u32::from(self)}fn width()->u32{16}}
    pub struct Port<T>{port:u16,kind:core::marker::PhantomData<T>}
    impl<T:Value>Port<T>{pub fn new(port:u16)->Self{Self{port,kind:core::marker::PhantomData}}
        pub unsafe fn write(&mut self,value:T){crate::out(self.port,value.value(),T::width());}}
}}
pub mod hal {pub mod pci {
    pub fn config_address(bus:u8,dev:u8,fun:u8,offset:u8)->u32{0x80000000|((bus as u32)<<16)|((dev as u32)<<11)|((fun as u32)<<8)|u32::from(offset&0xfc)}
    pub fn config_locked<R>(f:impl FnOnce()->R)->R{f()}
    pub unsafe fn config_read_unlocked(bus:u8,dev:u8,fun:u8,offset:u8)->u32{crate::read(bus,dev,fun,offset)}
    include!("actual_apply.rs");
}}
pub mod boot_network {pub fn query(caller:u32)->Option<[u64;4]>{
    if crate::input(45)==0{return None;}
    crate::boot_network_state::Info::new(crate::input(46)as u32,crate::network_config::Config::CONTROLLED)?.query(caller)
}}
fn region(at:usize)->memory_region::RegionGrant{memory_region::RegionGrant{region_id:input(at+1)as u32,virtual_base:input(at+2),length:input(at+3),rights:input(at+4)as u8,
    dma_base:if input(at+5)!=0{Some(input(at+6))}else{None},peer_pd:if input(at+7)!=0{Some(input(at+8)as u32)}else{None}}}
fn main(){
    use std::io::{self,BufRead};use capability::*;
    for line in io::stdin().lock().lines(){
        let v:Vec<u64>=line.unwrap().split_whitespace().map(|v|v.parse().unwrap()).collect();assert_eq!(v.len(),48);
        INPUT.with(|p|*p.borrow_mut()=v);DEVICE.with(|p|*p.borrow_mut()=Device{selected:0,word:input(43)as u32,reads:0,events:Vec::new()});
        let grant=pci_command::Grant {subject:input(10)as u32,bus:input(16)as u8,device:input(17)as u8,function:input(18)as u8,vendor_id:input(19)as u16,device_id:input(20)as u16,allowed:input(21)as u16};
        let g=if input(9)!=0{Some(grant)}else{None};
        let result=if input(0)==0{
            let u=pci_command::authorize(g.as_ref(),input(1)as u32,input(3),input(4)).ok();
            let model=match(pci_command::Identity{vendor:input(14)as u16,product:input(15)as u16}).model(){None=>0,Some(pci_command::Model::I82540)=>1,Some(pci_command::Model::I82574)=>2};
            [u.is_some()as u64,u.is_some_and(|u|u.needs_dma())as u64,u.map_or(input(43)as u16,|u|u.next(input(43)as u16))as u64,model,0]
        }else{
            let pci=Pci{bus:input(11)as u8,dev:input(12)as u8,func:input(13)as u8,vendor_id:input(14)as u16,device_id:input(15)as u16,
                command:g,bar0_view:if input(22)!=0{Some(region(22))}else{None}};
            let pd=ProtectionDomain{id:input(1)as u32,device:if input(7)==0{None}else{Some(Capability{data:if input(8)!=0{CapabilityData::Pci(pci)}else{CapabilityData::Domain}})},
                dma:if input(31)==0{None}else{Some(Capability{data:if input(32)!=0&&input(33)!=0{CapabilityData::Region(region(33))}else{CapabilityData::Domain}})}};
            match input(0){1=>kernel_pci::identity(&pd,input(2),input(3),input(4),input(5),input(6)),
                2=>kernel_pci::invoke(&pd,input(2),input(3),input(4),input(5),input(6)),
                3=>kernel_pci::network_config(&pd,input(2),input(3),input(4),input(5),input(6)),_=>panic!()}
        };
        DEVICE.with(|d|{let d=d.borrow();print!("{} {} {} {} {} {}",result[0],result[1],result[2],result[3],result[4],d.word);
            for e in &d.events{print!("|{}{}:{}:{}",e.0,e.1,e.2,e.3);}println!();});
    }
}
