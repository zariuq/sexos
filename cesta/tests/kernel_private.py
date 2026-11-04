#!/usr/bin/env python3
"""Load all current layouts and verify private successor admission on real PTE code."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent.parent


def run(argv):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if p.returncode:raise RuntimeError((argv,p.returncode,p.stdout[-2000:],p.stderr[-7000:]))
    return p.stdout


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True)
    ap.add_argument('--work',type=Path);ap.add_argument('--mutations',action='store_true');args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='kernel-private-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    units=('kernel_domains','kernel_memory','kernel_apps','app_private','kernel_current_boot','kernel_boot_memory','memory_regions','kernel_paging','kernel_task_wait','pager_state','kernel_sources','source_policy','kernel_pci','pci_policy','network_config','kernel_history','history_recovery','history_decode')
    for unit in units:
        (work/(unit+'.c')).write_bytes(run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','--','-Igenerated/include','-Iruntime/kernel']))
    flags=['-std=c11','-g0','-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections',
           '-DSEXOS_KERNEL_CURRENT_PDX=1','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include']
    renames=[a for n in ('memcpy','memmove','memset','memcmp') for a in ('--redefine-sym',n+'=sexos_'+n)]

    def compile(cc,extra,name):
        obj=work/(name+'-memory.o');renamed=work/(name+'-renamed.o');apps=work/(name+'-apps.o');binary=work/name
        run([cc,*flags,*extra,'-Itests/kernel_memory','-Iruntime/kernel','-c',work/'kernel_memory.c','-o',obj])
        run(['objcopy',*renames,obj,renamed])
        run([cc,*flags,*extra,'-Itests/kernel_memory','-Iruntime/kernel','-c',work/'kernel_apps.c','-o',apps])
        run([cc,*flags,*extra,'-I'+str(work),'-Itests/kernel_paging','-Iruntime/kernel','-Wl,--gc-sections',
             'tests/kernel_private/host.c',work/'kernel_sources.c',work/'source_policy.c',work/'pci_policy.c',work/'network_config.c',work/'history_recovery.c',work/'history_decode.c',work/'kernel_history.c',work/'kernel_pci.c',work/'kernel_paging.c',work/'kernel_task_wait.c',work/'pager_state.c',work/'app_private.c',work/'kernel_boot_memory.c',work/'memory_regions.c',renamed,apps,'-o',binary])
        return binary

    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            name=cc+'-'+mode;run([compile(cc,extra,name)]);builds.append(name)
            print(name,'catalog layouts/manifests/detached successors/activation chain passed',flush=True)
    mutants=[
        ('retire_without_ACK','kernel_apps','a->acked!=APP_PRIVATE_REQUIRED','false'),
        ('retire_before_demand_cleanup','kernel_apps','!kernel_paging_cleanup_complete(a->target)','false'),
        ('retain_dead_capabilities','kernel_apps','c->target->caps[slot]=NULL;','(void)slot;'),
        ('handover_keeps_source_leaf','kernel_apps','app_private_leaf_write(c->app->image.pages[i].leaf,0);','(void)i;'),
        ('handover_skips_workspace_leaf','kernel_apps','app_private_leaf_write(c->app->workspace.pages[i].leaf,c->app->workspace.pages[i].expected);','(void)i;'),
        ('handover_keeps_messages','kernel_apps','c->target->ring=(Ring){.revision=c->ring_revision+1u};','c->target->ring=(Ring){.count=c->messages,.revision=c->ring_revision+1u};'),
        ('handover_event_omitted','kernel_apps','kernel_epoch_record_handover(c->epoch,','if(false)kernel_epoch_record_handover(c->epoch,'),
        ('handover_event_wrong_source_key','kernel_apps','c->app->image.count,c->app->key,c->pager->key);','c->app->image.count,c->pager->key,c->pager->key);'),
        ('handover_ignores_epoch_exhaustion','kernel_apps','if(!kernel_epoch_current(kernel.epoch)){f->rax=APP_PRIVATE_RETRY;return;}','if(false){f->rax=APP_PRIVATE_RETRY;return;}'),
        ('receipt_task_from_domain','kernel_apps','f->r10=a->task;','f->r10=a->target;'),
        ('task_is_domain','kernel_current_boot','a->task!=d->task_id','a->task!=d->id'),
        ('history_task_is_domain','kernel_history','(HistoryParticipant){d->id,d->task_id}','(HistoryParticipant){d->id,d->id}'),
        ('activate_without_private_detach','kernel_apps','if(app_private_leaf_read(c->old->image.pages[i].leaf))','if(false)'),
        ('activation_leaf_not_restored','kernel_apps','app_private_leaf_write(c->next->image.pages[i].leaf,c->next->image.pages[i].expected);','(void)i;'),
        ('activation_not_queued','kernel_apps','c->target->task_state=TASK_READY;run_push(c->target->id);','c->target->task_state=TASK_READY;'),
        ('activation_event_omitted','kernel_apps','kernel_epoch_record_activation(c->epoch,','if(false)kernel_epoch_record_activation(c->epoch,'),
        ('activation_event_wrong_predecessor','kernel_apps','kernel_epoch_record_activation(c->epoch,c->old->target,','kernel_epoch_record_activation(c->epoch,1u,'),
        ('activation_event_wrong_key','kernel_apps','c->next->surface,c->next->image.count,c->target->key);','c->next->surface,c->next->image.count,1u);'),
        ('activation_ignores_epoch_exhaustion','kernel_apps','if(!kernel_epoch_current(kernel.epoch) || kernel.run_count>=KERNEL_RUN_QUEUE_CAPACITY)','if(false || kernel.run_count>=KERNEL_RUN_QUEUE_CAPACITY)'),
        ('lose_activation_receipt','kernel_apps','if(next->activated)','if(false)'),
        ('skip_retirement_quorum','kernel_apps','old->acked==APP_PRIVATE_REQUIRED','true'),
        ('key_is_not_layout','kernel_domains','(layout - 1)','(key - 1)'),
        ('uncleared_successor','app_private','app_private_leaf_write(p->pages[i].leaf,0);','(void)0;'),
        ('missing_invalidation','kernel_apps','cpu_invlpg(va);','(void)va;'),
        ('schedule_dormant','kernel_domains',
         'if (d == NULL || (d->task_state == TASK_EXITED || d->task_state == TASK_DORMANT)) return;\n  if (SEXOS_KERNEL_CURRENT_PDX && (d->queued || (d->task_state != TASK_READY && d->task_state != TASK_RUNNING))) return;',
         'if (d == NULL || d->task_state == TASK_EXITED) return;\n  if (SEXOS_KERNEL_CURRENT_PDX && (d->queued || (d->task_state != TASK_READY && d->task_state != TASK_RUNNING && d->task_state != TASK_DORMANT))) return;'),
        ('cross_image_alias','kernel_apps','(a->image.pages[k].expected&APP_PRIVATE_PHYSICAL)) return false;',
         '(a->image.pages[k].expected&APP_PRIVATE_PHYSICAL)) return true;'),
    ];caught=[]
    if args.mutations:
        for name,unit,before,after in mutants:
            path=work/(unit+'.c');original=path.read_text();assert original.count(before)>=1,(name,'mutation anchor')
            try:
                path.write_text(original.replace(before,after));binary=compile('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('mutation survived',name);caught.append(name)
            finally:path.write_text(original)
    names=['src/'+u+'.cesta' for u in units]+['src/kernel_interface.cesta','src/app_private_interface.cesta','interfaces/kernel_app_handover_protocol.module','interfaces/kernel_app_activation_protocol.module','tests/kernel_private.py','tests/kernel_private/host.c','interfaces/kernel_current.data','generated/include/kernel_current.h']
    summary={'builds':builds,'mutations_caught':caught,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},
             'scope':'actual loader/allocator/pagewalks/private-manifest code on RAM, CPU/TLB providers; synthetic ELF payload; no user execution or current-image boot/refinement claim'}
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('private admission qualification passed',flush=True)


if __name__=='__main__':main()
