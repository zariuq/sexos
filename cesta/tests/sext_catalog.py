#!/usr/bin/env python3
"""Compare every generated lifecycle slot and service notice match with real Rust catalog."""
import argparse,hashlib,json,subprocess,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent

def run(argv,data=None):
 p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
 if p.returncode:raise RuntimeError((argv,p.returncode,p.stdout[-1000:],p.stderr[-5000:]))
 return p.stdout

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path);args=ap.parse_args()
 w=args.work or Path(tempfile.mkdtemp(prefix='sext-catalog-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
 (w/'catalog.rs').write_bytes((HERE.parent/'crates/sex-pdx/src/app_catalog.rs').read_bytes())
 (w/'oracle.rs').write_text('''#[path="catalog.rs"]mod catalog;
fn main(){println!("{}",catalog::COUNT);for i in catalog::INCARNATIONS{println!("{} {} {}",i.target,i.surface,i.lifecycle_slot);}}
''')
 run(['rustc','--edition=2021','--cap-lints','allow',w/'oracle.rs','-o',w/'oracle']);data=run([w/'oracle']);(w/'catalog.txt').write_bytes(data)
 for unit in ('sext_service','sext'):
  run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','-o',w/(unit+'.c'),'--','-Igenerated/include'])
 flags=['-std=c11','-g0','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include','-I'+str(w)]
 builds=[]
 for cc in ('gcc','clang'):
  for mode in ('O0','O2','sanitize'):
   extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
   binary=w/(cc+'-'+mode);run([cc,*flags,*extra,'tests/sext/catalog.c',w/'sext.c','-o',binary]);out=run([binary],data);builds.append({'compiler':cc,'mode':mode,'output':out.decode().strip()})
 original=LayoutText((w/'sext_service.c').read_text());bad=original.replace('i<SEXOS_APP_CATALOG_COUNT','i<6');assert bad!=original
 try:
  (w/'sext_service.c').write_text(bad);binary=w/'stale-six-entry-limit';run(['gcc',*flags,'-O2','tests/sext/catalog.c',w/'sext.c','-o',binary])
  p=subprocess.run([binary],input=data,capture_output=True);assert p.returncode!=0,'stale capacity mutation survived'
 finally:(w/'sext_service.c').write_text(original)
 names=['src/sext_service.cesta','src/sext_service_interface.cesta','interfaces/kernel_current.data','interfaces/app_catalog.export','src/app_catalog_interface.cesta','tests/sext/catalog.c','tests/sext_catalog.py']
 (w/'summary.json').write_text(json.dumps({'builds':builds,'actual_rust_rows':int(data.splitlines()[0]),'mutation':'stale-six-entry-limit','source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'scope':'actual service slot lookup and kernel-notice intake for every catalog row,including the expanded tail; no retirement/workspace claim'},indent=2)+'\n');print('all Rust catalog rows and source-limit mutation passed',flush=True)
if __name__=='__main__':main()
