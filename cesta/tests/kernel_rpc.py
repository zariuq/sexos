#!/usr/bin/env python3
"""Compile and exercise the actual correlated-call kernel on a host boundary."""
import argparse
import hashlib
import json
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
    parser.add_argument('--work', type=Path)
    args = parser.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='kernel-rpc-', dir=HERE/'build'))
    work.mkdir(parents=True, exist_ok=True)
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
    selector_mutations = [
        ('call_slot_truncation', 'kernel_syscalls', 'uint64_t slot = f->rdi;',
         'uint64_t slot = (uint32_t)f->rdi;'),
        ('listen_slot_truncation', 'kernel_syscalls', 'kernel_listen_message(d, f->rdi)',
         'kernel_listen_message(d, (uint32_t)f->rdi)'),
        ('reply_target_truncation', 'kernel_syscalls',
         '  if (f->rdi >= KERNEL_MAX_DOMAINS) {\n    f->rax = 1;\n    return;\n  }\n', ''),
        ('rpc_listen_slot_truncation', 'kernel_rpc', 'kernel_listen_message(d, f->rdi)',
         'kernel_listen_message(d, (uint32_t)f->rdi)'),
        ('rpc_reply_target_truncation', 'kernel_rpc',
         '    if (c->frame->rdi >= KERNEL_MAX_DOMAINS) return false;\n', ''),
    ]
    for name, unit, before, after in selector_mutations:
        source = work/(unit+'.c')
        original = source.read_text()
        assert original.count(before) == 1, (name, 'mutation anchor changed')
        try:
            source.write_text(original.replace(before, after))
            binary = work/name
            run(['gcc', '-std=c11', '-O2', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                 '-I', work, '-Igenerated/include', '-Iruntime/kernel', 'tests/kernel_rpc/host.c', '-o', binary])
            result = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert result.returncode != 0, ('selector mutation survived', name)
        finally:
            source.write_text(original)
    summary = {'builds': 6, 'payload_mutations': 2,
               'selector_mutations': [item[0] for item in selector_mutations],
               'source_sha256': {name: hashlib.sha256((HERE/name).read_bytes()).hexdigest()
                                 for name in ('src/kernel_syscalls.cesta', 'src/kernel_rpc.cesta',
                                              'tests/kernel_rpc/host.c', 'tests/kernel_rpc.py')},
               'scope': 'compiled kernel units with host providers; no CPU entry or boot execution'}
    (work/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print('kernel RPC: full-width selector controls detect all five truncation mutations')

if __name__ == '__main__':
    main()
