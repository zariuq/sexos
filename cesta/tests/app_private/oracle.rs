extern crate self as sex_pdx;
#[path="actual_catalog.rs"]pub mod app_catalog;
#[path="actual_state.rs"] mod state;
use state::*;
use std::io::{self,Read};
use std::cell::RefCell;
fn main(){
 let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
 let mut image=Vec::<PrivatePage>::new();let leaves=RefCell::new([0u64;MAX_PAGES+1]);
 for line in text.lines(){
  let v:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();assert_eq!(v.len(),24);
  let events=RefCell::new(Vec::<(char,u64,u64)>::new());let mut out=[0u64;4];
  let read=|leaf:u64|{let value=if leaf<=MAX_PAGES as u64{leaves.borrow()[leaf as usize]}else{0};events.borrow_mut().push(('R',leaf,value));value};
  let write=|leaf:u64,value:u64|{events.borrow_mut().push(('W',leaf,value));if leaf<=MAX_PAGES as u64{leaves.borrow_mut()[leaf as usize]=value;}};
  let flush=|va:u64|events.borrow_mut().push(('F',va,0));
  match v[0]{
   0=>{image=vec![PrivatePage::EMPTY;v[1] as usize];*leaves.borrow_mut()=[0;MAX_PAGES+1];for (i,p) in image.iter_mut().take(MAX_PAGES).enumerate(){*p=PrivatePage{va:(i as u64+1)*4096,leaf:i as u64+1,expected:((i as u64+1)*4096)|5};leaves.borrow_mut()[i+1]=p.expected|0x60;}},
   1=>{image[v[1] as usize]=PrivatePage{va:v[2],leaf:v[3],expected:v[4]};if v[3]<=MAX_PAGES as u64{leaves.borrow_mut()[v[3] as usize]=v[5];}},
   2|3=>{let r=if v[0]==2{retire_private_pages(&image,read,|l|write(l,0),flush)}else{restore_reserved_pages(&image,read,write,flush)};match r{Ok(n)=>out[1]=n as u64,Err(e)=>out[0]=e}},
   4=>{let g=LifecycleGrant{target_pd:v[1] as u32,task_id:v[2] as u32,holder_pd:v[3] as u32,rights:v[4] as u8,ack_bit:v[5] as u8};out[0]=g.authorize(v[6] as u32,v[7] as u32,v[8] as u32,v[9] as u8).err().unwrap_or(0)},
   5=>{if let Some(n)=(Incarnation{logical_app:v[1] as u32,number:v[2] as u32}).next(){out[0]=1;out[1]=n.logical_app as u64;out[2]=n.number as u64;}},
   6=>{let g=ActivationGrant{holder:v[1] as u32,logical_app:v[2] as u32,incarnation:v[3] as u32,predecessor:v[4] as u32,predecessor_task:v[5] as u32,successor:v[6] as u32,successor_task:v[7] as u32};out[0]=g.authorize(v[8] as u32,v[9] as u32,v[10] as u32,v[11] as u32,v[12] as u32,Incarnation{logical_app:v[13] as u32,number:v[14] as u32},Incarnation{logical_app:v[15] as u32,number:v[16] as u32}).err().unwrap_or(0)},
   7=>out[0]=final_elf_flags(v[1],v[2] as u8),
   8=>if v[1]<=MAX_PAGES as u64{leaves.borrow_mut()[v[1] as usize]=v[2]},
   9=>{let n=Incarnation::original(v[1] as u32);out[0]=n.logical_app as u64;out[1]=n.number as u64},
   _=>panic!()
  }
  print!("out {} {} {} {} E",out[0],out[1],out[2],out[3]);for (k,a,b) in events.borrow().iter(){print!(" {}:{}:{}",k,a,b);}print!(" L");for (i,value) in leaves.borrow().iter().enumerate(){if *value!=0{print!(" {}:{}",i,value);}}println!();
 }
}
