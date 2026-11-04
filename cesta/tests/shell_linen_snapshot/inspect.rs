impl Snapshot {
 pub fn trace(&self){
  fn object(o:&Object){print!("{}:{}:{}:{}:",o.id,o.kind,o.len,o.disk);for b in o.name{print!("{:02x}",b);}}
  let(phase,id,choice)=match self.phase{Phase::Idle=>(0,0,0),Phase::Rescan=>(1,0,0),Phase::Metadata=>(2,0,0),Phase::Name=>(3,0,0),Phase::Open(id,choice)=>(4,id,choice)};
  print!("S {} {} {} {} {} {} {} {} {} ",phase,self.awaiting as u8,self.refresh as u8,self.rescan as u8,self.slot,self.offset,self.count,id,choice);object(&self.current);
  print!(" |objects");for row in self.objects{if let Some(o)=row{print!(" ");object(&o);}else{print!(" -");}}
  print!(" |opens {}",self.open_count);for row in self.opens{if let Some((id,choice))=row{print!(" {}:{}",id,choice);}else{print!(" -");}}println!();
 }
}
