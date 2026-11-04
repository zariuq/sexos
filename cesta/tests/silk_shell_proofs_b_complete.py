#!/usr/bin/env python3
"""Prove the three current partitions cover every named Proofs-B driver."""
import hashlib,json,re,subprocess
from pathlib import Path
from silk_shell_input_differential import item_end
HERE=Path(__file__).resolve().parent.parent
def main():
 complete=json.loads((HERE/'interfaces/silk_shell_proofs_b.complete.json').read_text());names=[];rows=[]
 for part in complete['partitions']:
  manifest=json.loads((HERE/part['manifest']).read_text());assert manifest['pin']==complete['current_pin'];assert len(manifest['functions'])==part['count'];rows.extend(manifest['functions']);names.extend(row['name'] for row in manifest['functions'])
 assert len(names)==len(set(names))==complete['expected_drivers'];assert complete['residual_drivers']==[]
 pinned=subprocess.run(['git','show',complete['current_pin']+':servers/silk-shell/src/main.rs'],cwd=HERE.parent,capture_output=True,text=True,check=True).stdout;working=(HERE.parent/'servers/silk-shell/src/main.rs').read_text();start=pinned.index('unsafe fn maybe_run_command_palette_status_proof');expected=re.findall(r'(?m)^(?:unsafe )?fn (maybe_run_[A-Za-z0-9_]+)\(',pinned[start:]);assert set(expected)==set(names),(sorted(set(expected)-set(names)),sorted(set(names)-set(expected)))
 for row in rows:
  for source in (pinned,working):
   match=re.search(r'(?m)^(?:unsafe )?fn '+re.escape(row['name'])+r'\b',source);assert match,row['name'];body=source[match.start():item_end(source,match.start())];assert hashlib.sha256(body.encode()).hexdigest()==row['source_sha256'],row['name']
 joined='\n'.join((HERE/path).read_text() for path in ('src/silk_shell_proofs_b_palette.cesta','src/silk_shell_proofs_b_atlas.cesta','src/silk_shell_proofs_b_input.cesta'));missing=[name for name in names if not re.search(r'\bshell_proofs_b_'+re.escape(name)+r'\s*\(',joined)];assert not missing,missing
 for helper in complete['production_helpers']:
  source=(HERE/helper['source']).read_text();assert re.search(r'\b'+re.escape(helper['cesta'])+r'\s*\(',source),helper
 print('Proofs-B complete:',len(names),'current drivers across 18+8+11 checked partitions; 3 production helpers; residual drivers 0')
if __name__=='__main__':main()
