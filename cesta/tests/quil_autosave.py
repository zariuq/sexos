#!/usr/bin/env python3
"""Qualify the pinned Quil idle-draft policy."""
import argparse,hashlib,json,subprocess,sys,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent;PIN='b6be1b451a6c21985d00d192674feedf66bb11dd'
def run(argv):
 r=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
 if r.returncode:raise RuntimeError((argv,r.returncode,r.stdout[-1000:],r.stderr[-5000:]))
 return r.stdout
def main():
 p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true');p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args();w=(a.work or Path(tempfile.mkdtemp(prefix='quil-autosave-',dir=HERE/'build'))).resolve();w.mkdir(parents=True,exist_ok=True)
 rust=subprocess.check_output(['git','show',PIN+':servers/quil/src/autosave.rs'],cwd=HERE.parent);assert hashlib.sha256(rust).hexdigest()=='401c8470c8a10bfb2662c67cae89da32a75079f92578a191b0d584a47211b512'
 c=w/'quil_autosave.c';plan=w/'quil_autosave.plan';run([a.cesta,'export-c','src/quil_autosave.cesta','--std=c11','--plan',plan,'-o',c,'--','-Igenerated/include']);flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include'];builds=[]
 def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/quil_autosave/host.c',c,'-o',out]);return out
 for cc in('gcc','clang'):
  for mode,extra in(('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'])):run([build(cc,extra,cc+'-'+mode)]);builds.append((cc,mode))
 changes=[('dirty-required','if(!dirty||','if(false||'),('pending-exclusive','state->pending||','false||'),('same-revision','state->drafted_revision==revision','false'),('idle-bound','elapsed<QUIL_AUTOSAVE_IDLE_MS','false'),('ready-bound','if(!ready||','if(false||'),('name-sanitize',"out.bytes[n++]=valid?byte:'_'","out.bytes[n++]=byte"),('collision','error==QUIL_AUTOSAVE_EXISTS','false'),('lost-draft','if(state->draft_present){state->draft_present=false;}','(void)error;')];caught=[]
 if a.mutations:
  base=LayoutText(c.read_text())
  for name,old,new in changes:
   assert base.count(old)>=1,(name,base.count(old));m=w/f'{name}.c';m.write_text(base.replace(old,new,1));b=w/name;run(['gcc','-std=c11','-O2','-Igenerated/include','tests/quil_autosave/host.c',m,'-o',b]);r=subprocess.run([b],cwd=HERE,capture_output=True);assert r.returncode!=0,('survived',name);caught.append(name)
 admission=None
 if a.lean_root:
  assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
  record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'QuilAutosaveOwnership.lean',a.lean_root)
 (w/'summary.json').write_text(json.dumps({'pin':PIN,'builds':builds,'mutations':caught,'admission':admission},indent=2)+'\n');print('Quil autosave:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
