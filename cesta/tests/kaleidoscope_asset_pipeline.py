#!/usr/bin/env python3
"""Exercise the composed QOI/PNG/geometry/bitmap/presentation lifecycle."""
import argparse,binascii,json,os,subprocess,tempfile,zlib
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(a,env=None):
 r=subprocess.run(list(map(str,a)),cwd=HERE,capture_output=True,env=env)
 if r.returncode:raise RuntimeError((a,r.returncode,r.stdout[-2000:],r.stderr[-5000:]))
 return r.stdout
def chunk(kind,data):
 return len(data).to_bytes(4,"big")+kind+data+binascii.crc32(kind+data).to_bytes(4,"big")
def main():
 p=argparse.ArgumentParser();p.add_argument("--cesta",type=Path,required=True);p.add_argument("--work",type=Path);a=p.parse_args();w=(a.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-asset-pipeline-",dir=HERE/"build"))).resolve();w.mkdir(parents=True,exist_ok=True)
 end=bytes([0,0,0,0,0,0,0,1]);qoi=b"qoif"+(2).to_bytes(4,"big")+(1).to_bytes(4,"big")+bytes([4,0,0xff,1,2,3,255,0xff,4,5,6,128])+end
 raw=bytes([0,1,2,3,255,4,5,6,128]);png=bytes([0x89])+b"PNG\r\n\x1a\n"+chunk(b"IHDR",(2).to_bytes(4,"big")+(1).to_bytes(4,"big")+bytes([8,6,0,0,0]))+chunk(b"IDAT",zlib.compress(raw))+chunk(b"IEND",b"")
 def arr(name,data):return f"static const uint8_t {name}[]={{"+",".join(map(str,data))+"};"
 (w/"fixtures.h").write_text(arr("qoi_fixture",qoi)+"\n"+arr("png_fixture",png)+"\n")
 units=("kaleidoscope_qoi","kaleidoscope_png","kaleidoscope_asset_pipeline","kaleidoscope_bitmap","kaleidoscope_presentation","kaleidoscope_asset_view","kaleidoscope_asset_view_render");host=HERE/"tests/kaleidoscope_asset_pipeline/host.c";builds=[]
 for cc in("gcc","clang"):
  sources=[]
  for unit in units:
   c=w/f"{unit}-{cc}.c";run([a.cesta,"export-c",f"src/{unit}.cesta","--cc",cc,"--std=c11","-o",c,"--","-Igenerated/include"]);sources.append(c)
  common=["-std=c11","-g0","-Wall","-Wextra","-Wpedantic","-Werror","-Igenerated/include","-I",w]
  for mode,flags in(("O2",["-O2"]),("SAN",["-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all","-fno-pie","-no-pie"])):
   b=w/f"{cc}-{mode}";run([cc,*common,*flags,host,*sources,"-o",b]);out=run([b],env={**os.environ,"ASAN_OPTIONS":"detect_leaks=1:abort_on_error=1"});assert out==b"asset pipeline ok: qoi png geometry begin chunk commit receipt clear\n";builds.append((cc,mode))
 (w/"summary.json").write_text(json.dumps({"builds":builds,"qoi_bytes":len(qoi),"png_bytes":len(png)},indent=2)+"\n");print("kaleidoscope asset pipeline:",len(builds),"builds, composed lifecycle")
if __name__=="__main__":main()
