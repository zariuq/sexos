#![allow(dead_code,unused_mut)]
extern crate self as sex_pdx;
#[path="TEXT_ENCODING_PATH"]mod text_encoding;
#[path="TEXT_BATCH_PATH"]mod text_batch;
#[path="APP_CATALOG_PATH"]mod app_catalog;
#[path="DELIVERY_PATH"]mod delivery;
use delivery::{Outbox,Metadata,Receipt,Poll,Admission};
fn receipt(r:Receipt){print!(" {} {} {} {} {}",r.bytes,r.metadata.count,r.metadata.first,r.metadata.rows,r.metadata.selected);}
fn main(){
 use std::io::{self,BufRead};let mut out=Outbox::new(0);let mut bytes=[0u8;512];let mut len=0usize;
 let mut mode=0;let mut remaining=0u64;let mut error=0u64;
 for line in io::stdin().lock().lines(){let line=line.unwrap();let words:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();let n=|i:usize|words.get(i).copied().unwrap_or(0);
 match n(0){
 0=>{out=if n(2)!=0{Outbox::atomic(n(1))}else{Outbox::new(n(1))};bytes=[0;512];len=0;mode=0;remaining=0;error=0;},
 1=>{len=n(1)as usize;for i in 0..512{bytes[i]=if n(3)!=0{b'a'+((i as u64+n(2))%26)as u8}else{(i as u64+n(2))as u8};}},
 2=>{let m=Metadata{count:n(2)as usize,first:n(3)as usize,rows:n(4)as usize,selected:n(5)};let accepted=if len>512{out.queue(&[0;513],n(1)as u32,m)}else{out.queue_selected(&bytes[..len],n(1)as u32,m,(n(6)as u16,n(7)as u16))};println!("Q {}",accepted as u8);},
 3=>{let p=out.poll(n(1)as usize,|op|{println!("O {} {} {} {}",op.opcode,op.surface,op.packed,op.control);if remaining>0{remaining-=1;Admission::Accepted}else{match mode{0=>Admission::Accepted,1=>Admission::Full,_=>Admission::Rejected(error)}}});let zero=Receipt{metadata:Metadata{count:0,first:0,rows:0,selected:0},bytes:0};match p{Poll::Idle=>{print!("P 0 0");receipt(zero)},Poll::Pending=>{print!("P 1 0");receipt(zero)},Poll::Deferred=>{print!("P 2 0");receipt(zero)},Poll::Complete(r)=>{print!("P 3 0");receipt(r)},Poll::Rejected(e)=>{print!("P 4 {}",e);receipt(zero)}}println!();},
 4=>{mode=n(1);remaining=n(2);error=n(3);},
 5=>{let r=out.receive(n(1)as u32,n(2));print!("R {}",r.is_some()as u8);receipt(r.unwrap_or(Receipt{metadata:Metadata{count:0,first:0,rows:0,selected:0},bytes:0}));println!();},
 6=>bytes[n(1)as usize]=n(2)as u8,
 7=>out.cancel(),
 8=>out.test_sequence(n(1)as u32),
 9=>{let text=&bytes[..len.min(512)];println!("V {} {} {} {}",text_encoding::valid_text(text)as u8,text_encoding::is_boundary(text,n(1)as usize)as u8,text_batch::hash(text),text_batch::selected_hash(text,n(2)as usize,n(3)as usize).unwrap_or(0));},
 10=>println!("E {}",delivery::test_erase()as u8),
 _=>panic!("command")}
 out.trace();
 }
}
