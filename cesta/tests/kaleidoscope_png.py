#!/usr/bin/env python3
"""Qualify the bounded native PNG boundary against pinned Rust fixtures."""
import argparse,hashlib,json,os,subprocess,tempfile
from layout_text import LayoutText
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent;REPO=HERE.parent;PIN="44053ea9b017fe89008413f7cc8a6b1a4d28d015"
def run(a,input=None,env=None):
 r=subprocess.run(list(map(str,a)),cwd=HERE,input=input,capture_output=True,env=env)
 if r.returncode:raise RuntimeError((a,r.returncode,r.stdout[-2000:],r.stderr[-5000:]))
 return r.stdout
def main():
 p=argparse.ArgumentParser();p.add_argument("--cesta",type=Path,required=True);p.add_argument("--work",type=Path);p.add_argument("--mutations",action="store_true");a=p.parse_args()
 w=(a.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-png-",dir=HERE/"build"))).resolve();w.mkdir(parents=True,exist_ok=True)
 qoi=subprocess.check_output(["git","show",f"{PIN}:crates/sex-graphics/src/qoi.rs"],cwd=REPO).decode();png=subprocess.check_output(["git","show",f"{PIN}:crates/sex-graphics/src/png.rs"],cwd=REPO).decode();fixtures={n:subprocess.check_output(["git","show",f"{PIN}:tests/fixtures/png/{n}.rs"],cwd=REPO).decode()for n in("cases","variants","keyed")}
 modules="mod qoi{\n"+qoi.replace("//!","//")+"\n}\nmod png{\n"+png.replace("//!","//")+"\n}\n"
 dump=modules+"mod cases{\n"+fixtures["cases"]+"\n}\nmod variants{\n"+fixtures["variants"]+"\n}\nmod keyed{\n"+fixtures["keyed"]+"\n}\n"+r'''
use std::io::Write;fn main(){let args:Vec<String>=std::env::args().collect();let all:Vec<&[u8]>=cases::CASES.iter().map(|x|x.1).chain(variants::CASES.iter().map(|x|x.1)).chain(keyed::KEYED.iter().map(|x|x.1)).collect();if args[1]=="count"{println!("{}",all.len())}else{std::io::stdout().write_all(all[args[1].parse::<usize>().unwrap()]).unwrap()}}'''
 (w/"dump.rs").write_text(dump);dumper=w/"dump";run(["rustc","--edition=2021","-C","opt-level=2",w/"dump.rs","-o",dumper]);count=int(run([dumper,"count"]))
 inputs=[run([dumper,i])for i in range(count)];base=list(inputs)
 for data in base:
  for at in sorted({8,len(data)//2,max(0,len(data)-5)}):
   if at<len(data):b=bytearray(data);b[at]^=0x41;inputs.append(bytes(b))
  for cut in sorted({0,8,max(0,len(data)-1)}):inputs.append(data[:cut])
 inputs += [b"GIF89a",bytes(12289)]
 oracle=modules+r'''
use std::io::Read;fn code(e:png::Error)->u8{match e{png::Error::Signature=>1,png::Error::InputTooLarge=>2,png::Error::Truncated=>3,png::Error::Crc=>4,png::Error::Header=>5,png::Error::Unsupported=>6,png::Error::Palette=>7,png::Error::Zlib=>8,png::Error::Deflate=>9,png::Error::Adler=>10,png::Error::Size=>11,png::Error::Filter=>12}}fn bh(b:&[u8])->u64{b.iter().fold(0xcbf29ce484222325u64,|h,x|(h^*x as u64).wrapping_mul(0x100000001b3))}fn ph(p:&[u32])->u64{p.iter().flat_map(|x|x.to_le_bytes()).fold(0xcbf29ce484222325u64,|h,x|(h^x as u64).wrapping_mul(0x100000001b3))}fn main(){let mut input=Vec::new();std::io::stdin().read_to_end(&mut input).unwrap();let mut s=[0xa5u8;png::SCRATCH_BYTES];let mut p=[0xdeadbeefu32;qoi::MAX_PIXELS];match png::decode(&input,&mut s,&mut p){Ok((w,h))=>println!("O {} {} {:016x} {:016x}",w,h,ph(&p),bh(&s)),Err(e)=>println!("E {} {} {}",code(e),s.iter().all(|x|*x==0)as u8,p.iter().all(|x|*x==0)as u8)}}'''
 (w/"oracle.rs").write_text(oracle);ref=w/"ref";run(["rustc","--edition=2021","-C","opt-level=2",w/"oracle.rs","-o",ref]);expected=[run([ref],input=x)for x in inputs]
 defs=[];rows=[]
 for i,data in enumerate(inputs):defs.append(f"static const uint8_t case_{i}[]={{"+(",".join(map(str,data))if data else"0")+"};");rows.append(f"{{case_{i},{len(data)}}}")
 (w/"cases.h").write_text("typedef struct PngCase{const uint8_t*bytes;size_t length;}PngCase;"+"\n".join(defs)+"\nstatic const PngCase cases[]={"+",".join(rows)+"};")
 host=HERE/"tests/kaleidoscope_png/host.c";exports={};builds=[]
 for cc in("gcc","clang"):
  c=w/f"png-{cc}.c";run([a.cesta,"export-c","src/kaleidoscope_png.cesta","--cc",cc,"--std=c11","-o",c,"--","-Igenerated/include"]);exports[cc]=c;common=["-std=c11","-g0","-Wall","-Wextra","-Wpedantic","-Werror","-Igenerated/include","-I",w]
  for mode,flags in(("O0",["-O0"]),("O2",["-O2"]),("SAN",["-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all","-fno-pie","-no-pie"])):
   b=w/f"{cc}-{mode}";run([cc,*common,*flags,host,c,"-o",b])
   for i,want in enumerate(expected):
    got=run([b,i],env={**os.environ,"ASAN_OPTIONS":"detect_leaks=1:abort_on_error=1"});assert got==want,(cc,mode,i,got,want)
   builds.append((cc,mode))
 mutations=[]
 if a.mutations:
  basec=LayoutText(exports["gcc"].read_text());changes=(("crc","png_crc(source,kind,length)!=png_be32","false&&png_crc(source,kind,length)!=png_be32"),("adler","adler!=png_adler","adler==png_adler"),("filter","add=pa<=pb&&pa<=pc?a:pb<=pc?b:c","add=0"),("split-idat","idat.started&&!previous_idat","idat.started"),("palette","pixel=palette[s0]","pixel=palette[0]"),("row-order","pixels->values[y*w+x]=pixel","pixels->values[x*h+y]=pixel"))
  common=["gcc","-std=c11","-O2","-Wall","-Wextra","-Wpedantic","-Werror","-Igenerated/include","-I",w]
  for name,old,new in changes:
   assert basec.count(old)>=1,(name,basec.count(old));m=w/f"mutant-{name}.c";m.write_text(basec.replace(old,new,1));b=w/f"mutant-{name}";built=subprocess.run([*common,host,m,"-o",b],cwd=HERE,capture_output=True)
   if built.returncode==0:assert any(subprocess.run([b,str(i)],cwd=HERE,capture_output=True).stdout!=want for i,want in enumerate(expected)),("survived",name)
   mutations.append(name)
 (w/"summary.json").write_text(json.dumps({"pin":PIN,"fixtures":count,"cases":len(inputs),"builds":builds,"mutations":mutations},indent=2)+"\n");print("kaleidoscope png:",count,"fixtures,",len(inputs),"controls,",len(builds),"builds,",len(mutations),"mutants")
if __name__=="__main__":main()
