fn main(){use std::io::BufRead;for line in std::io::stdin().lock().lines(){let line=line.unwrap();let n:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();
 let names:Vec<Vec<u8>>=(0..n[0]).map(|i|(0..n[4]).map(|j|if n[5]==1{(i*7+j*13)as u8}else{(65+(i+j)%26)as u8}).collect()).collect();
 let items:Vec<Item>=(0..n[0]as usize).map(|i|Item{id:if n[5]==2{i as u64%2+1}else{i as u64+1},name:&names[i]}).collect();
 let out=list(&items,n[1],n[2]as u32,n[3]as u32);print!("{} {} {} {} {} {} {} ",out.first,out.rows,out.selected_row.is_some()as u8,out.selected_row.unwrap_or(0),out.row_y,out.row_h,out.len);for b in out.text{print!("{:02x}",b);}println!();
}}
