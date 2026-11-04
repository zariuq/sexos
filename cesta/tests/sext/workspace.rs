mod workspace {
@SOURCE@
impl Workspace{
    pub fn seed_count(&mut self,n:u64){self.records=n;}
    pub fn projection(&self,origin:u64){println!("workspace 1 {} {} {} {}",self.base-origin,self.bytes,self.next,self.records);}
}}
#[repr(align(4096))]struct Bytes([u8;5*4096]);
fn main(){
    let mode:u32=std::env::args().nth(1).unwrap().parse().unwrap();
    let mut bytes=Box::new(Bytes([0xa5;5*4096]));
    let origin=bytes.0.as_mut_ptr()as u64;
    let mut base=origin+4096;let mut length=if mode<3{(mode as u64+1)*4096}else{8192};
    if mode==3{base+=1;}if mode==4{length+=1;}
    let mut workspace=None;
    std::panic::set_hook(Box::new(|_|{}));
    let result=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||{
        workspace=Some(unsafe{workspace::Workspace::admit(base,length)});
        let w=workspace.as_mut().unwrap();
        if mode==5{w.seed_count(u64::MAX-1);}
        for i in 1..=if mode==5{2}else{1000}{println!("record {}",unsafe{w.record(10,i)});}
        w.rewind();println!("rewound {}",unsafe{w.record(14,123)});
    }));
    if result.is_err(){println!("failure");}
    if let Some(w)=&workspace{w.projection(origin);}else{println!("workspace 0");}
    print!("bytes ");for b in &bytes.0{print!("{:02x}",b);}println!();
}

