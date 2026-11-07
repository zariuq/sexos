"""Current pure Linen list composition against its actual Rust implementation."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
from silk_shell_input_differential import declarations,item_end
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent
FIX=HERE/'tests/shell_linen_list'
PIN='b6be1b451a6c21985d00d192674feedf66bb11dd'
def run(args,data=None):
 r=subprocess.run(list(map(str,args)),cwd=HERE,input=data,capture_output=True);assert r.returncode==0,(args,r.returncode,r.stderr[-6000:]);return r.stdout
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/linen-list-tests');args=parser.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True)
 raw=run(['git','show',PIN+':servers/silk-shell/src/desktop_content.rs']).decode();items=declarations(raw);pieces=[items['Item'],items['List']]
 for pattern in (r'(?m)^impl List\b',r'(?m)^pub fn list\b'):
  match=re.search(pattern,raw);assert match;pieces.append(raw[match.start():item_end(raw,match.start())])
 (w/'oracle.rs').write_text('\n'.join(pieces)+'\n'+(FIX/'provider.rs').read_text());run(['rustc','--edition=2021','-O',w/'oracle.rs','-o',w/'rust'])
 rows=[]
 def add(*values):rows.append(' '.join(str(x)for x in values))
 for width in (0,1,16,17,25,40,255,256,320,1280,0xffffffff):
  for height in (0,53,54,71,72,89,108,720,0xffffffff):
   for count in (0,1,5,6,16):
    for selected in (0,1,4,16,999):
     for length in (0,1,24,79,127):add(count,selected,width,height,length,0)
 for mode in (1,2):
  for length in (0,1,2,7,24,79,80,125,126,127,512):
   for width in (40,255,256,0xffffffff):
    for selected in (1,2,16):add(16,selected,width,720,length,mode)
 data=('\n'.join(rows)+'\n').encode();expected=run([w/'rust'],data);out=w/'list.c';run([args.cesta,'export-c','src/shell_linen_list.cesta','--std=c11','-o',out,'--','-Igenerated/include']);configs=[]
 for cc in ('gcc','clang'):
  for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   binary=w/(cc+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*options,'-Igenerated/include',out,FIX/'provider.c','-o',binary]);observed=run([binary],data)
   if observed!=expected:
    left,right=expected.splitlines(),observed.splitlines();i=next((i for i,pair in enumerate(zip(left,right))if pair[0]!=pair[1]),min(len(left),len(right)));raise AssertionError((cc,mode,i,rows[i],left[i][:150],right[i][:150]))
   configs.append({'compiler':cc,'mode':mode,'sha256':hashlib.sha256(observed).hexdigest()})
 mutants=[];base=out.read_text()
 for name,old,new in (
  ('wrong_scale','width>=256?2:1','width>=256?1:2'),
  ('no_page_selection','out.first=index/rows*rows','out.first=0'),
  ('wrong_truncation',"?'~':","?'!':"),
  ('wrong_sanitization',"b:'?'","b:'!'"),
  ('wrong_row_budget','if(rows>5)rows=5','if(rows>5)rows=4'),
  ('lost_selection','out.has_selected=true','out.has_selected=false'),
  ('wrong_untitled','item.len?item.len:10','item.len?item.len:0'),
 ):
  assert LayoutText(base).count(old)==1,(name,LayoutText(base).count(old));path=w/('mutant-'+name+'.c');path.write_text(LayoutText(base).replace(old,new));binary=w/('mutant-'+name);run(['gcc','-std=c11','-O2','-Igenerated/include',path,FIX/'provider.c','-o',binary]);changed=subprocess.run([binary],input=data,capture_output=True);assert changed.returncode!=0 or changed.stdout!=expected,('survived',name);mutants.append(name)
 (w/'summary.json').write_text(json.dumps({'pin':PIN,'source_sha256':hashlib.sha256(raw.encode()).hexdigest(),'commands':len(rows),'configurations':configs,'mutants_caught':mutants},indent=2)+'\n');print('Linen list:',len(rows),'layout cases per configuration; six compiler/sanitizer configurations agree;',len(mutants),'mutants caught')
if __name__=='__main__':main()
