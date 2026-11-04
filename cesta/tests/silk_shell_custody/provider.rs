#![allow(dead_code)]
#[path="../../../crates/sex-pdx/src/app_catalog.rs"] mod app_catalog;
#[path="../../../servers/silk-shell/src/app_custody.rs"] mod app_custody;
use app_custody::Entry;

fn state(e: &Entry) {
    let (x,y,w,h)=e.bounds.unwrap_or((0,0,0,0));
    println!("S {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
        e.slot,e.target,e.task,e.surface,e.pending as u8,e.dead as u8,e.retired as u8,
        e.acked as u8,e.close_pending as u8,e.close_attempt.is_some() as u8,
        e.close_attempt.unwrap_or(0),e.close_declined as u8,e.registration_reply as u8,
        e.bounds.is_some() as u8,x,y,w,h,0);
}
fn row(r: Option<&app_catalog::Incarnation>) {
    if let Some(r)=r {
        let mut children=[0;3]; children[..r.children.len()].copy_from_slice(r.children);
        println!("R 1 {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
            r.app as u32,r.ordinal,r.target,r.surface,r.shell_surface.unwrap_or(0),
            r.lifecycle_slot,r.close_slot,r.activation_slot.is_some() as u8,
            r.activation_slot.unwrap_or(0),r.route_slot,r.children.len(),children[0],children[1],children[2]);
    } else { println!("R 0"); }
}
fn word(w: Option<u64>) { println!("R {} {}",w.is_some() as u8,w.unwrap_or(0)); }
fn main() {
    use std::io::{self,Read};
    let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
    let mut e=Entry::new(0,0,0,0);
    for line in text.lines() {
        let v:Vec<u64>=line.split_whitespace().map(|s|s.parse().unwrap()).collect();
        let n=|i:usize|*v.get(i).unwrap_or(&0);
        match n(0) {
            100=>{e=Entry::new(n(1) as usize,n(2) as u32,n(3) as u32,n(4)); println!("R");},
            101=>{
                let f=n(1);e.pending=f&1!=0;e.dead=f&2!=0;e.retired=f&4!=0;e.acked=f&8!=0;
                e.close_pending=f&16!=0;e.close_attempt=if f&32!=0{Some(n(2))}else{None};
                e.close_declined=f&64!=0;e.registration_reply=f&128!=0;
                e.bounds=if f&256!=0{Some((n(3) as i32,n(4) as i32,n(5) as u32,n(6) as u32))}else{None};
                println!("R");
            },
            0=>word(e.shell_surface()),
            1=>println!("R {}",e.is_successor_frame(n(1)) as u8),
            2=>println!("R {}",e.owns_surface(n(1)) as u8),
            3=>word(e.close_slot()),
            4=>println!("R {}",e.request_close() as u8),
            5=>println!("R {}",e.close_due(n(1)) as u8),
            6=>{e.attempted_close(n(1));println!("R");},
            7=>println!("R {}",e.close_kind()),
            8=>println!("R {}",e.decline_close(n(1) as u32,[n(2),n(3),n(4)]) as u8),
            9=>println!("R {}",e.hint(n(1) as u32,n(2),n(3),n(4)) as u8),
            10=>println!("R {}",e.attest(n(1) as u32,n(2) as u32,n(3)) as u8),
            11=>println!("R {}",e.allows_operation(n(1) as u32,n(2),[n(3),n(4),n(5)]) as u8),
            12=>println!("R {}",e.allows(n(1) as u32,n(2),n(3) as u16) as u8),
            13=>println!("R {}",e.finish_ack() as u8),
            14=>println!("R {}",app_custody::buffer_sender(n(1) as u32) as u8),
            15=>row(app_catalog::INCARNATIONS.get(n(1) as usize)),
            16=>row(app_catalog::by_surface(n(1) as u32,n(2))),
            _=>panic!("unknown command"),
        }
        state(&e);
    }
}
