#!/usr/bin/env python3
"""Compare checked command completion with the pinned Rust baseline."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

HERE=Path(__file__).resolve().parent.parent
PIN='44053ea9b017fe89008413f7cc8a6b1a4d28d015'
RUST_DRIVER=r'''
#[path="COMPLETE"] mod complete;
use std::io::{self,BufRead};
fn hex(bytes:&[u8])->String{bytes.iter().map(|b|format!("{b:02x}")).collect()}
fn main(){for line in io::stdin().lock().lines(){let word=line.unwrap().into_bytes();let(extra,n)=complete::complete(&word);let mut row=Vec::new();for c in complete::candidates(&word){if row.len()+c.len()+2>96{break}row.extend_from_slice(b"  ");row.extend_from_slice(c)}println!("{} {} {}",n,hex(extra),hex(&row));}}
'''

def run(argv,*,cwd=HERE,data=None):
    result=subprocess.run(list(map(str,argv)),cwd=cwd,input=data,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True)
    parser.add_argument('--work',type=Path);parser.add_argument('--mutations',action='store_true')
    parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path);args=parser.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='spindle-complete-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    source=run(['git','show',PIN+':apps/spindle/src/complete.rs'],cwd=HERE.parent)
    rust=work/'complete.rs';rust.write_bytes(source)
    driver=work/'driver.rs';driver.write_text(RUST_DRIVER.replace('COMPLETE',str(rust)))
    reference=work/'reference';run(['rustc','--edition=2021','-O',driver,'-o',reference])
    commands=re.findall(rb'b"([a-z-]+)"',source.split(b'pub const COMMANDS:',1)[1].split(b'];',1)[0])
    words={b'',b'calc',b'his',b'history',b'history-',b'sext',b'zzz',b'app',b'browser',b'project-scene-'}
    for command in commands:
        for end in range(len(command)+1):words.add(command[:end])
    data=b'\n'.join(sorted(words))+b'\n'
    expected=run([reference],data=data)
    c=work/'spindle_complete.c';plan=work/'spindle_complete.plan'
    run([args.cesta,'export-c','src/spindle_complete.cesta','--std=c11','--plan',plan,'-o',c,'--','-Igenerated/include'])
    builds=[]
    for compiler in ('gcc','clang'):
        for mode,flags in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'])):
            binary=work/f'{compiler}-{mode}';run([compiler,'-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-Igenerated/include',c,'tests/spindle_complete/host.c','-o',binary])
            actual=run([binary],data=data);assert actual==expected,(compiler,mode);builds.append((compiler,mode))
    mutations=[]
    if args.mutations:
        base=c.read_text();changes=[
          ('exact-not-prefix','if(!prefix||!spindle_commands[i][word->len])continue','if(!prefix)continue'),
          ('prefix-byte','(uint8_t)spindle_commands[i][k]!=word->bytes[k]','(uint8_t)spindle_commands[i][k]!=word->bytes[0]'),
          ('common-prefix','common=k','common=0'),
          ('sextime-entry','"sextime"','"sextimf"'),
        ]
        for name,old,new in changes:
            assert base.count(old)==1,(name,base.count(old));mutant=work/f'{name}.c';mutant.write_text(base.replace(old,new,1));binary=work/name
            run(['gcc','-std=c11','-O2','-Igenerated/include',mutant,'tests/spindle_complete/host.c','-o',binary])
            result=subprocess.run([binary],cwd=HERE,input=data,capture_output=True)
            assert result.returncode!=0 or result.stdout!=expected,('survived',name);mutations.append(name)
    admission=None
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,work/'SpindleCompleteOwnership.lean',args.lean_root)
    summary={'pin':PIN,'rust_sha256':hashlib.sha256(source).hexdigest(),'commands':len(commands),'prefix_cases':len(words),'builds':builds,'mutations':mutations,'admission':admission}
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('spindle completion:',len(commands),'commands,',len(words),'prefixes,',len(builds),'builds,',len(mutations),'mutants')

if __name__=='__main__':main()
