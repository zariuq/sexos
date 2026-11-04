#[path="actual_network.rs"]mod network;
use network::*;
fn hash(bytes:&[u8])->u64{bytes.iter().fold(14695981039346656037,|h,b|(h^u64::from(*b)).wrapping_mul(1099511628211))}
fn main(){
    use std::io::{self,BufRead};
    for line in io::stdin().lock().lines(){
        let v:Vec<u64>=line.unwrap().split_whitespace().map(|v|v.parse().unwrap()).collect();assert_eq!(v.len(),16);
        let c=Config {local:(v[1]as u32).to_be_bytes(),prefix:v[2]as u8,gateway:(v[3]as u32).to_be_bytes(),dns:(v[4]as u32).to_be_bytes(),
            dns_port:v[5]as u16,http:(v[6]as u32).to_be_bytes(),http_port:v[7]as u16,listen_port:v[8]as u16,source_base:v[9]as u16,profile:v[10]as u8};
        let c=match v[0]{0=>Some(c),1=>Config::from_words([v[12],v[13],v[14],v[15]]),2=>Some(Config::CONTROLLED),_=>panic!()};
        let Some(c)=c else{println!("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");continue;};
        let name=c.hostname();let tls=c.tls_endpoint();let w=c.to_words();
        println!("1 {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",c.valid()as u8,c.contains((v[11]as u32).to_be_bytes())as u8,name.is_some()as u8,
            name.map_or(0,|n|n.len()),name.map_or(0,hash),tls.is_some()as u8,tls.map_or(0,|p|u32::from_be_bytes(p.0)),tls.map_or(0,|p|p.1),
            w[0],w[1],w[2],w[3],Config::from_words(w).is_some()as u8,hash(DNS_NAME),hash(DNS_QUESTION));
    }
}
