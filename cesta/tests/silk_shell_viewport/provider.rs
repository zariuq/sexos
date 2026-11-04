fn main(){
 use std::io::{self,Read};let mut input=String::new();io::stdin().read_to_string(&mut input).unwrap();
 let mut kind=0;let mut document=document_view::Delivery::new();let mut spindle=spindle_view::Delivery::new();
 let mut refresh=document_view::WorkspaceRefresh::new(0,0);
 for line in input.lines(){let v:Vec<u64>=line.split_whitespace().map(|s|s.parse().unwrap()).collect();let n=|i:usize|*v.get(i).unwrap_or(&0);
  match n(0){
   100=>{kind=n(1);document=document_view::Delivery::new();spindle=spindle_view::Delivery::new();refresh=document_view::WorkspaceRefresh::new(n(2)as u32,n(3)as u32);},
   101=>{if kind==0{document.force_sequence(n(1))}else{spindle.force_sequence(n(1))}},
   0=>if kind==0{document.queue(n(1)as u32,n(2)as u32,n(3)as u32,n(4)as u32)}else{spindle.queue(n(1)as u32,n(2)as u32,n(3)as u32,n(4)as u32)},
   1=>if kind==0{document.refresh()}else{spindle.refresh()},
   2=>println!("R {}",if kind==0{document.pending()}else{spindle.pending()}as u8),
   3=>if kind==0{document.retire()}else{spindle.retire()},
   4=>{
    if kind==0{document.poll(n(1)!=0,n(2)!=0,|notice|{let a=notice.args();println!("SEND {} {} {} {} {}",n(3),document_view::OP,a[0],a[1],a[2]);if n(4)==0{Ok(())}else{Err(n(4))}})}
    else{spindle.poll(n(1)!=0,n(2)!=0,|notice|{let a=notice.args();println!("SEND {} {} {} {} {}",n(3),spindle_view::OP,a[0],a[1],a[2]);if n(4)==0{Ok(())}else{Err(n(4))}})}
   },
   5=>{
    if kind==0{if let Some(a)=document_view::Notice::decode(n(1)as u32,n(2)as u32,n(3)as u32,[n(4),n(5),n(6)],n(7)){println!("R 1 {} {} {} {} {}",a.target,a.task,a.w,a.h,a.sequence)}else{println!("R 0")}}
    else{if let Some(a)=spindle_view::Notice::decode(n(1)as u32,n(2)as u32,n(3)as u32,[n(4),n(5),n(6)],n(7)){println!("R 1 {} {} {} {} {}",a.target,a.task,a.w,a.h,a.sequence)}else{println!("R 0")}}
   },
   10=>refresh=document_view::WorkspaceRefresh::new(n(1)as u32,n(2)as u32),
   11=>refresh.changed(),12=>println!("R {}",refresh.pending()as u8),13=>refresh.close(),
   14=>refresh.poll(|a|{println!("SEND {} {} {} {} {}",n(1),document_view::WORKSPACE_CHANGED,a[0],a[1],a[2]);if n(2)==0{Ok(())}else{Err(n(2))}}),
   15=>{let a=spindle_view::Layout::new(n(1)as u32,n(2)as u32);print!("R {} {} {} {} {}",a.cols,a.rows[0],a.rows[1],a.columns(),a.lines());for p in a.panes{print!(" {} {} {} {}",p.x,p.y,p.w,p.h)}println!();},
   16=>{let a=document_view::Layout::new(n(1)as u32,n(2)as u32,n(3)as u32);println!("R {} {} {} {} {} {} {} {} {} {} {} {} {}",a.w,a.h,a.scale,a.cols,a.rows,a.top,a.toolbar_rows,a.header_rows(),a.toolbar_per_row(),a.hint_rows(),a.chrome_rows(),a.body_rows(),a.hit(n(4)as u32,n(5)as u32).unwrap_or(0));},
   17=>{let a=desktop_content::children(n(1),n(2)as i32,n(3)as i32,n(4)as u32,n(5)as u32);print!("R");for child in a{if let Some(c)=child{print!(" 1 {} {} {} {} {}",c.sid,c.x,c.y,c.w,c.h)}else{print!(" 0")}}println!();},
   18=>{let a=desktop_content::parent_frame(n(1));println!("R {} {}",a.is_some()as u8,a.unwrap_or(0));},
   19=>{let mut a=document_view::Layout::new(9,512,512);a.scale=n(1)as u32;println!("R {}",a.hit(8,48).unwrap_or(0));},
   _=>panic!("command"),
  }
  if kind==0{document.trace_state()}else{spindle.trace_state()};refresh.trace_state();
 }
}
