#!/usr/bin/env python3
"""Compare checked asset geometry and bounded rendering with pinned Rust."""
import argparse,hashlib,json,os,subprocess,sys,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent;REPO=HERE.parent;PIN="b6be1b451a6c21985d00d192674feedf66bb11dd"
def run(a,env=None):
 r=subprocess.run(list(map(str,a)),cwd=HERE,capture_output=True,env=env)
 if r.returncode:raise RuntimeError((a,r.returncode,r.stdout[-2000:],r.stderr[-5000:]))
 return r.stdout
def corpus():
 rows=[]
 for w,h in ((1280,750),(512,219),(512,400),(240,150),(256,180),(512,49),(512,50),(65536,65536)):
  for iw,ih in ((0,0),(1,1),(48,48),(48,24),(3,47),(47,3),(49,1)):
   for mode in (0,1):
    for status in (0,3,5,8,9):rows.append((w,h,iw,ih,mode,status,(w+iw+status)%2))
 return rows
def main():
 p=argparse.ArgumentParser();p.add_argument("--cesta",type=Path,required=True);p.add_argument("--work",type=Path);p.add_argument("--mutations",action="store_true");p.add_argument("--cesta-source",type=Path);p.add_argument("--lean-root",type=Path);a=p.parse_args()
 w=(a.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-asset-view-",dir=HERE/"build"))).resolve();w.mkdir(parents=True,exist_ok=True)
 src=subprocess.check_output(["git","show",f"{PIN}:apps/kaleidoscope/src/asset_view.rs"],cwd=REPO).decode();rows=corpus()
 text='''extern crate self as sex_pdx;
pub mod bitmap_batch{#[derive(Clone,Copy,Debug,PartialEq,Eq)]pub struct Geometry{pub x:u16,pub y:u16,pub width:u8,pub height:u8,pub scale:u8}}
pub mod desktop_style{pub const INSET:u32=8;pub const ADVANCE:u32=6;pub const LINE:u32=9;pub const fn scale(w:u32)->u32{if w>=256{2}else{1}}}
mod view{\n'''+src.replace("//!","//")+'''\n}
#[derive(Clone,Copy)]struct Case{w:u32,h:u32,iw:usize,ih:usize,mode:u8,status:u8,custom:u8}
'''
 text+="static CASES:&[Case]=&["+",".join("Case{w:%d,h:%d,iw:%d,ih:%d,mode:%d,status:%d,custom:%d}"%r for r in rows)+"];\n"
 text+=r'''fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();let c=CASES[n];let mode=if c.mode==0{view::Mode::Fit}else{view::Mode::Actual};let g=view::geometry(c.w,c.h,c.iw,c.ih,mode);let title=if c.custom!=0{b"PNG".as_slice()}else{b"QOI / controlled HTTPS lab root".as_slice()};let f=view::render_titled(c.w,c.h,unsafe{core::mem::transmute(c.status)},mode,c.iw,c.ih,title);print!("G {}",g.is_some()as u8);if let Some(g)=g{print!(" {},{},{},{},{}",g.x,g.y,g.width,g.height,g.scale)}else{print!(" 0,0,0,0,0")}print!(" C {} F {},{},{} ",view::chrome_rows(c.w),f.len,f.rows,f.cols);for b in &f.bytes[..f.len]{print!("{:02x}",b)}println!()}'''
 (w/"ref.rs").write_text(text);ref=w/"ref";run(["rustc","--edition=2021","-C","opt-level=2",w/"ref.rs","-o",ref]);expected=[run([ref,i])for i in range(len(rows))]
 (w/"cases.h").write_text("typedef struct Case{uint32_t w,h;size_t iw,ih;uint8_t mode,status,custom;}Case;static const Case cases[]={"+",".join("{%d,%d,%d,%d,%d,%d,%d}"%r for r in rows)+"};")
 exports={};plans={};builds=[];host=HERE/"tests/kaleidoscope_asset_view/host.c"
 for cc in ("gcc","clang"):
  geom=w/f"geom-{cc}.c";render=w/f"render-{cc}.c";plan=w/f"{cc}.plan";run([a.cesta,"export-c","src/kaleidoscope_asset_view.cesta","--cc",cc,"--std=c11","-o",geom,"--plan",plan,"--","-Igenerated/include"]);run([a.cesta,"export-c","src/kaleidoscope_asset_view_render.cesta","--cc",cc,"--std=c11","-o",render,"--","-Igenerated/include"]);plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan;exports[cc]=geom
  common=["-std=c11","-g0","-Wall","-Wextra","-Wpedantic","-Werror","-Igenerated/include","-I",w]
  for mode,flags in(("O0",["-O0"]),("O2",["-O2"]),("SAN",["-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all","-fno-pie","-no-pie"])):
   b=w/f"{cc}-{mode}";run([cc,*common,*flags,host,geom,render,"-o",b])
   for i,want in enumerate(expected):
    got=run([b,i],env={**os.environ,"ASAN_OPTIONS":"detect_leaks=1:abort_on_error=1"});assert got==want,(cc,mode,i,got,want)
   builds.append((cc,mode))
 mutations=[]
 if a.mutations:
  base=LayoutText(exports["gcc"].read_text());changes=(("dimension","width>48","width>49"),("columns","columns--","columns=0"),("chrome","asset_columns(width)>=30","asset_columns(width)>=80"),("scale","if(factor>4)factor=4","if(factor>3)factor=3"),("actual-factor","factor=1;if(mode","factor=0;if(mode"))
  render=w/"render-gcc.c";common=["gcc","-std=c11","-O2","-Wall","-Wextra","-Wpedantic","-Werror","-Igenerated/include","-I",w]
  for name,old,new in changes:
   assert base.count(old)>=1,(name,base.count(old));m=w/f"mutant-{name}.c";m.write_text(base.replace(old,new,1));b=w/f"mutant-{name}";built=subprocess.run([*common,host,m,render,"-o",b],cwd=HERE,capture_output=True)
   if built.returncode==0:assert any(subprocess.run([b,str(i)],cwd=HERE,capture_output=True).stdout!=want for i,want in enumerate(expected)),("survived",name)
   mutations.append(name)
 admission=[]
 if a.lean_root:
  assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/"tests"));import ownership_admission
  for i,plan in enumerate(plans.values()):
   record,=ownership_admission.parse(plan.read_bytes());admission.append(ownership_admission.admit_record(record,w/f"KaleidoscopeAssetViewOwnership{i}.lean",a.lean_root))
 (w/"summary.json").write_text(json.dumps({"pin":PIN,"cases":len(rows),"builds":builds,"mutations":mutations,"admission":admission},indent=2)+"\n");print("kaleidoscope asset view:",len(rows),"cases,",len(builds),"builds,",len(mutations),"mutants,",sum(x["functions"]for x in admission),"admitted")
if __name__=="__main__":main()
