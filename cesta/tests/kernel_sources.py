#!/usr/bin/env python3
"""Qualify the actual typed-source adapter with scripted processor operations."""
import argparse,hashlib,json,subprocess,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent

def run(argv):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-1000:],p.stderr[-5000:]))
    return p.stdout

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True)
    ap.add_argument('--work',type=Path);ap.add_argument('--mutations',action='store_true');args=ap.parse_args()
    w=args.work or Path(tempfile.mkdtemp(prefix='kernel-sources-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    for unit in ('kernel_domains','kernel_sources','source_policy'):
        run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','-o',w/(unit+'.c'),'--','-Igenerated/include','-Iruntime/kernel'])
    flags=['-std=c11','-g0','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
           '-DSEXOS_KERNEL_CURRENT_PDX=1','-Wall','-Wextra','-Wpedantic','-Werror',
           '-Itests/kernel_sources','-Igenerated/include','-I'+str(w),'-Iruntime/kernel']
    def build(cc,extra,name):
        binary=w/name;run([cc,*flags,*extra,'tests/kernel_sources/host.c',w/'source_policy.c','-o',binary]);return binary
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            output=run([build(cc,extra,cc+'-'+mode)]).decode().strip()
            builds.append({'compiler':cc,'mode':mode,'output':output})
    mutants=[
        ('slot_narrowing','slot=f->rdi','slot=(uint32_t)f->rdi'),
        ('skip_task_check','uint32_t task=kernel_current_task(d)','uint32_t task=d->task_id'),
        ('ignore_cap_kind','held->kind!=expected','held->kind==CAP_NONE'),
        ('wrong_clock_port','cpu_inb(0x71)','cpu_inb(0x70)'),
        ('read_irq_status','source_cmos_register(0x0d)','source_cmos_register(0x0c)'),
        ('disable_nmi','cpu_outb(0x70,index)','cpu_outb(0x70,index|0x80)'),
        ('ignore_seconds_rollover','source_cmos_register(0)!=sec','source_cmos_register(0)==0xff'),
        ('ignore_mode_change','source_cmos_register(0x0b)!=b','source_cmos_register(0x0b)==0xff'),
        ('skip_cpuid_bound','if(r[0]<7)return false;','if(false)return false;'),
        ('wrong_boot_consumer','boot_source_grant(h,c)','boot_source_grant(h,h)'),
    ];caught=[];source=w/'kernel_sources.c';original=source.read_text()
    if args.mutations:
        for name,before,after in mutants:
            assert before in original,(name,'anchor')
            try:
                source.write_text(original.replace(before,after));binary=build('gcc',['-O2'],name)
                p=subprocess.run([binary],cwd=HERE,capture_output=True)
                assert p.returncode!=0,('mutation survived',name);caught.append(name)
            finally:source.write_text(original)
    # Compile the immutable policy constants from the actual upstream crate.
    config=HERE.parent/'crates/sex-trust-config/src/lib.rs'
    (w/'actual_trust_config.rs').write_bytes(config.read_bytes())
    (w/'trust.rs').write_text('#[path="actual_trust_config.rs"]mod config;fn main(){println!("{} {} {}",config::PROFILE,config::VERSION,config::ROOT_DER.len());}\n')
    run(['rustc','--edition=2021','--cap-lints','allow',w/'trust.rs','-o',w/'trust'])
    expected=list(map(int,run([w/'trust']).split()))
    (w/'trust.c').write_text('#include <stdio.h>\n#include "sexos_source_profile.h"\nint main(void){printf("%u %u %u\\n",SOURCE_TRUST_PROFILE,SOURCE_TRUST_VERSION,SOURCE_TRUST_ROOT_BYTES);}\n')
    run(['gcc','-std=c11','-Igenerated/include',w/'trust.c','-o',w/'trust-c'])
    assert list(map(int,run([w/'trust-c']).split()))==expected
    names=['src/kernel_sources.cesta','src/kernel_domains.cesta','runtime/kernel/cpu_x86_64.h',
           'interfaces/kernel_current.data','interfaces/source_profile.data','tests/kernel_sources.py',
           'tests/kernel_sources/host.c','tests/kernel_sources/cpu_x86_64.h']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'actual_trust_policy':expected,
        'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},
        'scope':'actual capability/task/BOOT adapter and bounded processor-operation trace. BSP serialization is assumed; no physical clock/entropy integrity or concurrent-CMOS claim.'},indent=2)+'\n')
    print('typed source adapter qualification passed',flush=True)

if __name__=='__main__':main()
