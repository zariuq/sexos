"""Qualify the actual Linen dispatch with the correlated capture service."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--output',type=Path,default=HERE/'build/linen-capture-live');args=ap.parse_args()
 w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);include='-I'+str(HERE/'generated/include');units=('linen','linen_capture','linen_capture_service');records=[]
 sources={}
 for unit in units:
  path=w/(unit+'.c');subprocess.run([args.cesta,'export-c',HERE/('src/'+unit+'.cesta'),'--std=c11','-o',path,'--',include],check=True);sources[unit]=path
 for cc in ('gcc','clang'):
  for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])]:
   binary=w/(cc+'-'+mode)
   subprocess.run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-ffunction-sections','-fdata-sections','-Wl,--gc-sections',include,'-I'+str(w),HERE/'tests/linen_capture/live_host.c',sources['linen_capture'],sources['linen_capture_service'],'-o',binary],check=True)
   result=subprocess.run([binary],capture_output=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'})
   assert result.returncode==0,result.stderr[-4000:]
   records.append(dict(compiler=cc,mode=mode,output=result.stdout.decode().strip(),binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest()))
 original=(HERE/'src/linen.cesta').read_text();mutations=(
  ('drop_request_id','(= (. ([] (-> l defer) (-> l defer_len)) request) (* request))','(block (= (. ([] (-> l defer) (-> l defer_len)) request) (* request)) (= (. (. ([] (-> l defer) (-> l defer_len)) request) id) 0))'),
  ('live_capture_reads','(call linen_capture_public_table l (& table))','(= table (compound LinenCaptureTable))'),
  ('drop_saturation_reply','(if (-> l correlated) (cast void (call linen_answer l request REPLY_NOT_READY)))','(cast void request)'),
 )
 for name,old,new in mutations:
  assert original.count(old)==1,(name,original.count(old));source=w/(name+'.cesta');source.write_text(original.replace(old,new));c=w/(name+'.c')
  directory=w/(name+'-source');directory.mkdir(exist_ok=True);c=directory/'linen.c'
  subprocess.run([args.cesta,'export-c',source,'--std=c11','-o',c,'--',include],check=True);binary=w/name
  subprocess.run(['gcc','-std=c11','-O2','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',include,'-I'+str(directory),'-I'+str(w),HERE/'tests/linen_capture/live_host.c',sources['linen_capture'],sources['linen_capture_service'],'-o',binary],check=True)
  result=subprocess.run([binary],capture_output=True);assert result.returncode!=0,('surviving mutation',name)
 (w/'summary.json').write_text(json.dumps(dict(runtime=records,mutants_caught=len(mutations),scope='Actual Linen dispatch and capture service on host providers; startup, storage I/O, CPU traps and QEMU are outside this gate.'),indent=2)+'\n')
 print('Live Linen capture dispatch:',len(records),'configurations;',len(mutations),'mutations caught')

if __name__=='__main__':main()
