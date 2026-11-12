#!/usr/bin/env python3
"""BOOT aliases through production allocator/page-table code on a RAM provider."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import sys
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(argv):
    p = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True)
    if p.returncode:
        raise RuntimeError((argv, p.returncode, p.stdout[-2000:], p.stderr[-6000:]))
    return p.stdout


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--cesta', type=Path, required=True)
    ap.add_argument('--work', type=Path)
    ap.add_argument('--mutations', action='store_true')
    ap.add_argument('--cesta-source', type=Path)
    ap.add_argument('--lean-root', type=Path)
    args = ap.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='kernel-boot-memory-', dir=HERE/'build'))
    work.mkdir(parents=True, exist_ok=True)
    units = ('frame_lease', 'kernel_domains', 'kernel_memory', 'kernel_boot_memory', 'kernel_region_writer',
             'region_writer', 'memory_regions', 'kernel_task_wait')
    for unit in units:
        command=[args.cesta, 'export-c', 'src/'+unit+'.cesta', '--std=c11']
        if unit=='region_writer':command+=['--plan',work/'region_writer.plan']
        command+=['--','-Igenerated/include','-Iruntime/kernel']
        (work/(unit+'.c')).write_bytes(run(command))
    common = ['-std=c11', '-g0', '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
              '-DSEXOS_KERNEL_CURRENT_PDX=1', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-Igenerated/include']
    renames = [arg for name in ('memcpy', 'memmove', 'memset', 'memcmp')
               for arg in ('--redefine-sym', name+'=sexos_'+name)]

    def compile(cc, extra, name):
        obj, renamed, binary = (work/(name+s) for s in ('.o', '-renamed.o', ''))
        run([cc, *common, *extra, '-Itests/kernel_memory', '-Iruntime/kernel',
             '-c', work/'kernel_memory.c', '-o', obj])
        run(['objcopy', *renames, obj, renamed])
        run([cc, *common, *extra, '-Itests/kernel_memory', '-I'+str(work), '-Iruntime/kernel', '-Wl,--gc-sections',
             HERE/'tests/kernel_boot_memory/host.c', HERE/'tests/kernel_memory/worker_stubs.c',
             work/'frame_lease.c', work/'region_writer.c', work/'memory_regions.c',
             work/'kernel_task_wait.c', renamed, '-o', binary])
        return binary

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all'] if mode == 'sanitize' else ['-'+mode]
            name = cc+'-'+mode
            run([compile(cc, extra, name)])
            builds.append(name)
            print(name, 'actual BOOT/allocator/page-table controls passed', flush=True)
    mutations = [
        ('elf_header_offset_wrap', 'kernel_domains', 'if (h->phoff > size) return false;', 'if (false) return false;'),
        ('executable_views', 'kernel_boot_memory', 'uint64_t flags=PTE_USER|PTE_NO_EXECUTE;', 'uint64_t flags=PTE_USER;'),
        ('writable_consumer', 'kernel_boot_memory', 'boot_region_map(second,&backing,false,&bv)', 'boot_region_map(second,&backing,true,&bv)'),
        ('wrong_holder_key', 'kernel_boot_memory', 'flags,d->key)', 'flags,(uint8_t)(d->key^1))'),
        ('aliased_physical_pages', 'kernel_boot_memory', 'b->physical+offset', 'b->physical'),
        ('invented_backing', 'kernel_boot_memory', '!backing_is_held(b)', '(!backing_is_held(b) && b->region_id==0)'),
        ('duplicate_root_identity', 'kernel_boot_memory', 'if (kernel.boot_backings[i].region_id==id) return false;', 'if (kernel.boot_backings[i].region_id==id && id==0) return false;'),
        ('shared_key_fourteen', 'kernel_domains', 'if (SEXOS_KERNEL_CURRENT_PDX) return mask;', 'if (false) return mask;'),
        ('writer_accepts_stale', 'region_writer', 'if (expected_revision != state.revision)', 'if (false && expected_revision != state.revision)'),
        ('writer_loses_job', 'region_writer', 'next.job=job;', 'next.job=1;'),
        ('writer_offer_moves_early', 'region_writer', 'next.job=job;', 'next.current=state.peer;next.job=job;'),
        ('writer_claim_keeps_home', 'region_writer', 'next.current=state.peer;', 'next.current=state.home;'),
        ('writer_retains_source_write', 'kernel_region_writer', '*source_leaf&=~PTE_WRITABLE;', '*source_leaf|=PTE_WRITABLE;'),
        ('writer_omits_target_write', 'kernel_region_writer', '*target_leaf|=PTE_WRITABLE;', '*target_leaf&=~PTE_WRITABLE;'),
        ('writer_omits_event', 'kernel_region_writer', 'kernel_epoch_record_writer(context->epoch,', 'if(false)kernel_epoch_record_writer(context->epoch,'),
        ('writer_ignores_epoch_exhaustion', 'kernel_region_writer', 'if(!kernel_epoch_current(kernel.epoch)){', 'if(false){'),
        ('writer_loses_action', 'kernel_domains', '.rights=action,.key=target_key', '.rights=(uint8_t)(action^1u),.key=target_key'),
        ('writer_recovers_live_peer', 'kernel_region_writer', 'if(!peer || (peer->task_state!=TASK_EXITED && peer->task_state!=TASK_DORMANT))', 'if(!peer || false)'),
        ('writer_recovers_dma_lease', 'kernel_region_writer', 'capability->region.grant.has_dma || !held->writer.job', 'false || !held->writer.job'),
        ('writer_loses_failure_reply', 'kernel_region_writer', 'context->subject->reply_value=ERR_SERVICE_NOT_READY;', 'context->subject->reply_value=0;'),
        ('writer_retains_reply_right', 'kernel_region_writer', '--dead->reply_rights[context->subject->id];', 'dead->reply_rights[context->subject->id]-=0;'),
    ]
    caught = []
    if args.mutations:
        for name, unit, before, after in mutations:
            path = work/(unit+'.c'); original = LayoutText(path.read_text())
            assert original.count(before) == 1, (name, 'mutation anchor')
            try:
                path.write_text(original.replace(before, after))
                binary = compile('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ('mutation survived', name)
                caught.append(name)
            finally:
                path.write_text(original)
    names = ['src/'+n+'.cesta' for n in units] + ['src/kernel_interface.cesta',
             'tests/kernel_boot_memory.py', 'tests/kernel_boot_memory/host.c', 'tests/kernel_memory/cpu_x86_64.h']
    admission=None
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse((work/'region_writer.plan').read_bytes())
        admission=ownership_admission.admit_record(record,work/'RegionWriterOwnership.lean',args.lean_root)
    summary = {'builds': builds, 'mutations_caught': caught, 'admission': admission,
               'source_sha256': {n: hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},
               'scope': 'actual BOOT constructors, physical zeroing and allocator/page walks on RAM; CPU/TLB providers; no executed hardware isolation, DMA transfer or current-image boot claim'}
    (work/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print('BOOT memory qualification passed;', len(caught), 'semantic mutations caught', flush=True)


if __name__ == '__main__':
    main()
