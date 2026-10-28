#!/usr/bin/env python3
"""Compile and exercise the actual correlated-call kernel on a host boundary."""
import argparse
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent

def run(args):
    result = subprocess.run(list(map(str, args)), cwd=HERE, capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError((args, result.stdout[-4000:], result.stderr[-4000:]))
    return result.stdout

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    args = parser.parse_args()
    work = Path(tempfile.mkdtemp(prefix='kernel-rpc-', dir=HERE/'build'))
    for unit in ['kernel_domains', 'kernel_syscalls', 'kernel_rpc']:
        (work/(unit+'.c')).write_text(run([args.cesta, 'export-c', 'src/'+unit+'.cesta', '--',
                                          '-Iruntime/kernel', '-Igenerated/include']))
    for compiler in ['gcc', 'clang']:
        for mode in ['-O0', '-O2', 'sanitize']:
            binary = work/(compiler+'-'+mode.removeprefix('-'))
            flags = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all'] if mode == 'sanitize' else [mode]
            run([compiler, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', *flags,
                 '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                 '-I', work, '-Igenerated/include', '-Iruntime/kernel', 'tests/kernel_rpc/host.c', '-o', binary])
            print(compiler, mode, run([binary]).strip())

    source=work/'kernel_rpc.c';original=source.read_text()
    for index,member in enumerate(('r10','r8')):
        needle='f->'+member
        assert needle in original
        source.write_text(original.replace(needle,'('+needle+' + 1)',1))
        binary=work/('payload-mutant-'+str(index))
        run(['gcc','-std=c11','-O2','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
             '-I',work,'-Igenerated/include','-Iruntime/kernel','tests/kernel_rpc/host.c','-o',binary])
        result=subprocess.run([binary],cwd=HERE,capture_output=True)
        assert result.returncode!=0,('payload mutation survived',member)
    source.write_text(original)
    print('kernel RPC: full-width payload and reply controls detect both altered argument lanes')

if __name__ == '__main__':
    main()
