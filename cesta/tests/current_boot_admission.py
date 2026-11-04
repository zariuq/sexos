#!/usr/bin/env python3
"""Admit the real logical BOOT declarations; reject key/layout/seal/pager changes."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent.parent


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True)
    ap.add_argument('--lean-root',type=Path,required=True);ap.add_argument('--work',type=Path);args=ap.parse_args()
    w=args.work or Path(tempfile.mkdtemp(prefix='boot-admission-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    generated=w/'CurrentBoot.lean'
    subprocess.run(['python3',str(HERE/'tools/current_boot_lean.py'),'--cesta',str(args.cesta),str(generated)],cwd=HERE,check=True)
    installed=args.lean_root/'Mettapedia/Machines/Kernel/SexOSCurrentBoot.lean'
    assert installed.read_bytes()==generated.read_bytes(),'current BOOT Lean instance is stale'
    source=generated.read_text()
    def check(path):
        p=subprocess.run(['lake','env','lean',str(path)],cwd=args.lean_root,capture_output=True)
        (w/(path.stem+'.log')).write_bytes(p.stdout+p.stderr)
        return p
    result=check(generated);assert result.returncode==0,(result.stdout[-2000:],result.stderr[-2000:])
    assert b'sorryAx' not in result.stdout and b'Classical.choice' not in result.stdout
    mutants=[
        ('admit_same_key_successor','⟨16, 9, 16, false⟩','⟨16, 9, 16, true⟩'),
        ('overlap_layout','⟨16, 9, 16, false⟩','⟨16, 9, 9, false⟩'),
        ('omit_seal','(actions ++ [.finishBoot])','actions'),
        ('wrong_pager','(10, 2, 14)','(10, 2, 13)'),
        ('unregistered_source_consumer','⟨13, 60, .entropy, 15⟩','⟨13, 60, .entropy, 0⟩'),
        ('ambient_browser_source','⟨13, 60, .entropy, 15⟩','⟨15, 60, .entropy, 15⟩'),
        ('source_replaces_route','⟨13, 60, .entropy, 15⟩','⟨13, 2, .entropy, 15⟩'),
        ('omit_all_sources',re.search(r'def sources : List Source :=\n  \[[^\n]*\]',source).group(),
         'def sources : List Source :=\n  []'),
    ]
    caught=[]
    for name,before,after in mutants:
        assert before in source,(name,'mutation anchor')
        # Change the declaration, preserving the independent required facts.
        path=w/(name+'.lean');path.write_text(source.replace(before,after,1))
        result=check(path);assert result.returncode!=0,('certificate mutation accepted',name)
        caught.append(name)
    (w/'summary.json').write_text(json.dumps({'instance_sha256':hashlib.sha256(generated.read_bytes()).hexdigest(),'refused':caught,
        'scope':'actual finite logical BOOT declarations and BootAuthority sealing laws; native C/PTE realization is separately tested'},indent=2)+'\n')
    print('actual current BOOT declarations admitted;',len(caught),'certificate mutations refused')


if __name__=='__main__':main()
