#!/usr/bin/env python3
"""Production paging adapters on RAM-backed actual allocator/loader/page tables."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent.parent


def run(argv):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-2000:],p.stderr[-5000:]))
    return p.stdout


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path);ap.add_argument('--mutations',action='store_true');args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='kernel-paging-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    units=['kernel_domains','kernel_paging','kernel_task_wait','pager_state','kernel_memory','kernel_boot_memory','kernel_apps','app_private','memory_regions']
    for unit in units:(work/(unit+'.c')).write_bytes(run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','--','-Igenerated/include','-Iruntime/kernel']))
    flags=['-std=c11','-g0','-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections','-DSEXOS_KERNEL_CURRENT_PDX=1','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include','-Itests/kernel_paging','-Iruntime/kernel']
    renames=[a for n in ('memcpy','memmove','memset','memcmp') for a in ('--redefine-sym',n+'=sexos_'+n)]
    def compile(cc,extra,name):
        obj=work/(name+'-memory.o');renamed=work/(name+'-renamed.o');binary=work/name
        run([cc,*flags,*extra,'-c',work/'kernel_memory.c','-o',obj]);run(['objcopy',*renames,obj,renamed])
        run([cc,*flags,*extra,'-I'+str(work),'-Wl,--gc-sections','tests/kernel_paging/host.c',renamed,
            *[work/(u+'.c') for u in units if u not in ('kernel_domains','kernel_paging','kernel_task_wait','kernel_memory')],'-o',binary])
        return binary
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            run([compile(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode});print(cc,mode,'native paging and wake controls passed',flush=True)
    mutations=[
        ('wake_fault_without_commit','kernel_task_wait','if(d->task_state==TASK_WAIT_IO)','if(d->task_state==TASK_WAIT_IO || d->task_state==TASK_WAIT_FAULT)'),
        ('duplicate_admission','kernel_domains','d->queued || ','false || '),
        ('narrow_selector','kernel_domains','if (!slot || slot >= KERNEL_CAP_SLOTS) return NULL;','slot=(uint32_t)slot; if (!slot || slot >= KERNEL_CAP_SLOTS) return NULL;'),
        ('premature_exit_notice','kernel_task_wait','d->id==kernel.current || ','false || '),
        ('duplicate_exit_notice','kernel_task_wait','d->exit_notice || ','false || '),
        ('expiry_receipt_lost','kernel_task_wait','d->frame->rsi=1;','d->frame->rsi=0;'),
        ('skip_zeroing','kernel_paging','if(words[i])error=PG_DENIED;','if(words[i] && false)error=PG_DENIED;'),
        ('retry_rechecks_client_contents','kernel_paging','if(old.present) {','if(old.present && old.finalized) {'),
        ('install_keeps_old_ledger','kernel_paging','c->state->ledger = c->after_ledger;','c->state->ledger = c->before_ledger;'),
        ('install_keeps_claimed_fault','kernel_paging','c->target->fault = c->after_fault;','c->target->fault = c->before_fault;'),
        ('install_event_wrong_subject','kernel_paging','KERNEL_EPOCH_PAGE_INSTALL,c->target->id,','KERNEL_EPOCH_PAGE_INSTALL,1u,'),
        ('install_event_wrong_key','kernel_paging','c->binding.rights,c->key);','c->binding.rights,c->key+1u);'),
        ('install_duplicate_writer_key','kernel_paging','i!=c->binding.range_id-1 && c->state->keys[i]==c->key','false && c->state->keys[i]==c->key'),
        ('install_leaf_key_lost','kernel_paging','((uint64_t)state->keys[binding.range_id-1]<<59)','(UINT64_C(0)<<59)'),
        ('install_ignores_epoch_exhaustion','kernel_paging','if(!kernel_epoch_current(kernel.epoch)) {','if(false) {'),
        ('reclaim_without_flush','kernel_paging','cpu_invlpg(c->address);','(void)c->address;'),
        ('reclaim_keeps_live_ledger','kernel_paging','c->state->ledger=c->after;','c->state->ledger=c->before;'),
        ('reclaim_event_wrong_subject','kernel_paging','KERNEL_EPOCH_PAGE_RECLAIM,c->binding.target_pd,','KERNEL_EPOCH_PAGE_RECLAIM,1u,'),
        ('reclaim_event_wrong_token','kernel_paging','c->address,c->binding.token,c->binding.frame_index,c->binding.rights);','c->address,c->binding.token+1u,c->binding.frame_index,c->binding.rights);'),
        ('reclaim_ignores_epoch_exhaustion','kernel_paging','if(!kernel_epoch_current(kernel.epoch))return PG_EXHAUSTED;','if(false)return PG_EXHAUSTED;'),
        ('ignore_orphan_leaf','kernel_paging',' : actual!=0', ' : false'),
        ('ignore_changed_leaf','kernel_paging','(actual&~UINT64_C(0x60))!=expected','(actual==expected && false)'),
        ('cleanup_keeps_old_ledger','kernel_paging','c->state->ledger=c->after_ledger;c->target->fault=c->after_fault;','c->state->ledger=c->before_ledger;c->target->fault=c->after_fault;'),
        ('cleanup_keeps_live_fault','kernel_paging','c->state->ledger=c->after_ledger;c->target->fault=c->after_fault;','c->state->ledger=c->after_ledger;c->target->fault=c->before_fault;'),
        ('cleanup_leaf_not_cleared','kernel_paging','*(volatile uint64_t *)(uintptr_t)c->state->leaves[range][b.page_index]=0;','*(volatile uint64_t *)(uintptr_t)c->state->leaves[range][b.page_index]=page_expected_leaf(c->state,b);'),
        ('cleanup_event_wrong_subject','kernel_paging','kernel_epoch_record_cleanup(c->epoch,c->target->id,','kernel_epoch_record_cleanup(c->epoch,1u,'),
        ('cleanup_event_wrong_bitmap','kernel_paging','c->target->task_id,c->bitmap,c->key);','c->target->task_id,0,c->key);'),
        ('cleanup_event_wrong_key','kernel_paging','c->target->task_id,c->bitmap,c->key);','c->target->task_id,c->bitmap,c->key+1u);'),
        ('cleanup_attached_stack','kernel_paging','kernel.current==target->id || target->queued','target->queued'),
        ('skip_fault_retirement','kernel_paging','pager_fault_retire(&c->after_fault,c->target->id,c->pager,c->target->task_id)','false'),
    ];caught=[]
    if args.mutations:
        for name,unit,before,after in mutations:
            p=work/(unit+'.c');original=p.read_text();assert before in original,(name,'anchor')
            try:
                p.write_text(original.replace(before,after));binary=compile('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('mutation survived',name);caught.append(name)
            finally:p.write_text(original)
    names=['src/'+u+'.cesta' for u in units]+['src/kernel_interface.cesta','interfaces/kernel_page_install_protocol.module','interfaces/kernel_page_reclaim_protocol.module','interfaces/kernel_page_cleanup_protocol.module','tests/kernel_paging.py','tests/kernel_paging/host.c','tests/kernel_paging/cpu_x86_64.h']
    summary={'builds':builds,'mutations':caught,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'scope':'actual BOOT backing, ELF loader, retained frames, page walks, checked state core and native paging/wake adapters on RAM. CPU/TLB providers are observed; no CPU execution, SMP or whole-C refinement claim.'}
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')


if __name__=='__main__':main()
