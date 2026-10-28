#!/usr/bin/env python3
"""Actual device code against a deterministic MMIO/DMA ownership boundary."""
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
    work=Path(tempfile.mkdtemp(prefix='network-device-',dir=HERE/'build'))
    for name in ('network_device','network_send'):
        (work/(name+'.c')).write_text(run([args.cesta,'export-c','src/'+name+'.cesta','--','-Igenerated/include']))
    for compiler in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            flags=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            binary=work/(compiler+'-'+mode)
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-DSEXOS_HOST=1',*flags,
                 '-I',work,'-Igenerated/include','tests/network_device/host.c','-o',binary])
            for case in ('ordinary','timeout','error'):run([binary,case])
            run([binary,'binding'],expected=77)
            run([binary,'rx-binding'],expected=77)
            print(compiler,mode,'ring/buffer/timeout/error controls and broken binding passed')
    source=work/'network_device.c';original=source.read_text()
    assert original.count('c->copied = true;')==1
    source.write_text(original.replace('c->copied = true;','c->copied = false;'))
    binary=work/'copy-fact-mutant'
    run(['gcc','-std=c11','-O2','-DSEXOS_HOST=1','-I',work,'-Igenerated/include','tests/network_device/host.c','-o',binary])
    run([binary,'ordinary'],expected=77)
    source.write_text(original)
    print('RX: immediate DMA reuse preserves the copied frame; forged copy fact refused')
if __name__=='__main__':main()
