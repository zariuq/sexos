#[path="actual_entropy.rs"]mod entropy;
#[path="actual_clock.rs"]mod clock;
#[path="actual_trust.rs"]mod trust;
#[path="actual_trust_config.rs"]mod trust_config;
use std::io::{self,Read};use std::cell::Cell;
fn raw(v:&[u64])->clock::Raw{clock::Raw{sec:v[18] as u8,min:v[19] as u8,hour:v[20] as u8,day:v[21] as u8,month:v[22] as u8,year:v[23] as u8,century:v[24] as u8,status_a:v[25] as u8,status_b:v[26] as u8,status_d:v[27] as u8}}
fn main(){
 assert_eq!((trust_config::PROFILE,trust_config::VERSION,trust_config::ROOT_DER.len()),(1,1,376));
 let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
 for line in text.lines(){let v:Vec<u64>=line.split_whitespace().map(|s|s.parse().unwrap()).collect();assert_eq!(v.len(),32);
  let f=Cell::new(0);let mut samples=0;let mut pairs=0;
  let result=match v[0]{
   0=>{let g=entropy::Grant{holder:entropy::Identity{pd:v[2] as u32,task:v[3] as u32},consumer:entropy::Identity{pd:v[4] as u32,task:v[5] as u32},source:v[6] as u32};entropy::invoke(if v[1]!=0{Some(&g)}else{None},entropy::Identity{pd:v[7] as u32,task:v[8] as u32},v[9],[v[10],v[11],v[12]],||{f.set(f.get()+1);v[13]!=0},||{samples+=1;if samples==v[14]{Some(v[15])}else{None}})},
   1=>{let g=clock::Grant{holder:clock::Identity{pd:v[2] as u32,task:v[3] as u32},consumer:clock::Identity{pd:v[4] as u32,task:v[5] as u32},source:v[6] as u32};clock::invoke(if v[1]!=0{Some(&g)}else{None},clock::Identity{pd:v[7] as u32,task:v[8] as u32},v[9],[v[10],v[11],v[12]],||{
    let mode=(v[16]>>(pairs*2))&3;pairs+=1;let mut a=raw(&v);let mut b=a;
    if mode==0{return None;}
    if mode==2{match v[28]{0=>b.sec^=v[29] as u8,1=>b.min^=v[29] as u8,2=>b.hour^=v[29] as u8,3=>b.day^=v[29] as u8,4=>b.month^=v[29] as u8,5=>b.year^=v[29] as u8,6=>b.century^=v[29] as u8,7=>b.status_a^=v[29] as u8,8=>b.status_b^=v[29] as u8,9=>b.status_d^=v[29] as u8,_=>return None}}
    if mode==3{a.status_d=0;b=a;}
    Some((a,b))
   })},
   2=>{let g=trust::Grant{holder:trust::Identity{pd:v[2] as u32,task:v[3] as u32},consumer:trust::Identity{pd:v[4] as u32,task:v[5] as u32},profile:v[6] as u32};trust::invoke(if v[1]!=0{Some(&g)}else{None},trust::Identity{pd:v[7] as u32,task:v[8] as u32},v[9],[v[10],v[11],v[12]])},
   3=>match clock::probe_seconds(raw(&v)){Some(n)=>[0,1,n,0,0],None=>[0,0,0,0,0]},
   _=>panic!()
  };
  println!("{} {} {} {} {} {} {} {}",result[0],result[1],result[2],result[3],result[4],f.get(),samples,pairs);
 }
}
