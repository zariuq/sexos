#[path="count.rs"]mod app_catalog;
#[path="actual_history.rs"]mod history;
use history::*;
use app_catalog::SPINDLE_COUNT as N;
fn main(){
 use std::io::{self,BufRead};
 for line in io::stdin().lock().lines(){
  let v:Vec<u64>=line.unwrap().split_whitespace().map(|v|v.parse().unwrap()).collect();assert_eq!(v.len(),20);
  let mut g=RecoveryGrant::<N>{holder:v[1]as u32,logical:v[2]as u32,participants:core::array::from_fn(|i|Participant{target:12+i as u32,task:1012+13*i as u32}),base:v[3],sectors:v[4]};
  let mut targets:[u32;N]=core::array::from_fn(|i|12+i as u32);
  if v[7]>0 && v[7]-1<N as u64{g.participants[(v[7]-1)as usize]=Participant{target:v[8]as u32,task:v[9]as u32};}
  if v[12]>0 && v[12]-1<N as u64{targets[(v[12]-1)as usize]=v[13]as u32;}
  let mut calls=Vec::new();let mut result=g;
  let r=if v[0]!=0{
   let d=decode_for(v[10]as u32,v[11]as u32,&targets,|kind|{
    let mut r=g.query(v[5]as u32,kind);
    if v[14]!=0 && kind==v[14]-1{if v[15]==0&&v[17]!=0{r=[0;5];}r[v[15]as usize]=v[16];}
    calls.push((kind,r));
    if r[0]!=0{Err(if r[1..]==[0;4]{r[0]}else{1})}else{Ok([r[1],r[2],r[3],r[4]])}
   });
   match d{Ok(d)=>{result=d;[0,d.logical as u64,d.holder as u64,d.base,d.sectors]},Err(e)=>{result=RecoveryGrant{holder:0,logical:0,participants:[Participant::EMPTY;N],base:0,sectors:0};[e,0,0,0,0]}}
  }else{g.query(v[5]as u32,v[6])};
  let found=g.participant(v[18]as u32);
  print!("{} {} {} {} {}|V{}|P{}:{}:{}:{}|G{}:{}:{}:{}",r[0],r[1],r[2],r[3],r[4],g.valid()as u8,found.is_some()as u8,found.map_or(0,|p|p.target),found.map_or(0,|p|p.task),g.permits(v[5]as u32,v[18]as u32,v[19]as u32)as u8,result.holder,result.logical,result.base,result.sectors);
  for p in result.participants{print!(":{}:{}",p.target,p.task);}
  for(k,r)in calls{print!("|Q{}:{}:{}:{}:{}:{}",k,r[0],r[1],r[2],r[3],r[4]);}
  println!();
 }
}
