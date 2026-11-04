"""Prove that the three checked partitions cover the complete pinned Proofs-A inventory."""
import hashlib,json,re,subprocess,sys
from pathlib import Path
from silk_shell_input_differential import item_end
HERE=Path(__file__).resolve().parent.parent
def main():
 complete=json.loads((HERE/'interfaces/silk_shell_proofs_a.complete.json').read_text());names=[];pins=set();rows=[]
 for part in complete['partitions']:
  manifest=json.loads((HERE/part['manifest']).read_text());pins.add(manifest['pin']);assert manifest['pin']==part['source_pin'];partition=manifest[part['field']];assert len(partition)==part['count'],(part['manifest'],len(partition));names.extend(row['name'] for row in partition);rows.extend(partition)
 base=json.loads((HERE/'interfaces/silk_shell_proofs_a.source.json').read_text());expected={row['name'] for row in base['functions']}|set(base['remaining']);actual=set(names)
 assert pins==set(complete['source_pins']);assert len(names)==len(actual)==complete['expected_functions'];assert actual==expected,(sorted(expected-actual),sorted(actual-expected));assert complete['residual']==[]
 pinned=subprocess.run(['git','show',complete['current_pin']+':servers/silk-shell/src/main.rs'],cwd=HERE.parent,capture_output=True,text=True,check=True).stdout;working=(HERE.parent/'servers/silk-shell/src/main.rs').read_text()
 for row in rows:
  for source in (pinned,working):
   match=re.search(r'(?m)^(?:unsafe )?fn '+re.escape(row['name'])+r'\b',source);assert match,row['name'];body=source[match.start():item_end(source,match.start())];assert hashlib.sha256(body.encode()).hexdigest()==row['source_sha256'],row['name']
 sources='\n'.join((HERE/path).read_text() for path in ('src/silk_shell_proofs_a.cesta','src/silk_shell_proofs_a_routes.cesta','src/silk_shell_proofs_a_lifecycle.cesta'))
 missing=[name for name in actual if not re.search(r'\b(?:shell_proofs_a_)?'+re.escape(name)+r'\s*\(',sources)];assert not missing,missing
 print('Proofs-A complete:',len(actual),'current functions across 51+23+4 checked partitions; residual 0')
if __name__=='__main__':main()
