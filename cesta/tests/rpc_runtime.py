#!/usr/bin/env python3
"""Actual generated request waits against controlled transport outcomes."""
import argparse
from pathlib import Path
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent.parent

def run(args,expected=0):
    result=subprocess.run(list(map(str,args)),cwd=HERE,capture_output=True,text=True)
    assert result.returncode==expected,(args,result.returncode,result.stdout[-3000:],result.stderr[-3000:])
    return result.stdout

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True);args=parser.parse_args()
    work=Path(tempfile.mkdtemp(prefix='rpc-runtime-',dir=HERE/'build'))
    for name in ('rpc_runtime','rpc_wait'):
        (work/(name+'.c')).write_text(run([args.cesta,'export-c','src/'+name+'.cesta','--','-Igenerated/include']))
    for compiler in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            flags=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            binary=work/(compiler+'-'+mode)
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,
                 '-I',work,'-Igenerated/include','tests/rpc_runtime/host.c','-o',binary])
            for scenario in range(10):run([binary,scenario])
            run([binary,10],expected=77)
            for scenario in (11,12,13):run([binary,scenario])
            print(compiler,mode,'ten query outcomes, three notification outcomes and identity-check failure passed')

if __name__=='__main__':main()
