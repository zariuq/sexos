#!/usr/bin/env python3
"""Load all current layouts and verify private successor admission on real PTE code."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent


def run(argv):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if p.returncode:raise RuntimeError((argv,p.returncode,p.stdout[-2000:],p.stderr[-7000:]))
    return p.stdout


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True)
    ap.add_argument('--work',type=Path);ap.add_argument('--mutations',action='store_true');args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='kernel-private-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    units=('frame_lease','kernel_domains','kernel_memory','kernel_apps','app_private','kernel_current_boot','kernel_boot_memory','memory_regions','kernel_paging','kernel_task_wait','pager_state','kernel_sources','source_policy','kernel_pci','pci_policy','network_config','kernel_history','history_recovery','history_decode','region_writer','kernel_region_writer','device_reset','kernel_device_reset','kernel_syscalls','program_memory','kernel_epoch_log','epoch_journal','kernel_device_interrupt','program_records','program_record_memory','program_record_tasks','kernel_records','kernel_rpc')
    for unit in units:
        (work/(unit+'.c')).write_bytes(run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','--','-Igenerated/include','-Iruntime/kernel']))
    (work/'program_image_bytes.c').write_bytes(run([args.cesta,'export-c','src/program_image_bytes.cesta']))
    (work/'program_image.c').write_bytes(run([args.cesta,'export-c','src/program_image.cesta','--','-Igenerated/include']))
    flags=['-std=c11','-g0','-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections',
           '-DSEXOS_KERNEL_CURRENT_PDX=1','-DSEXOS_DEVICE_RESET_HOST_REAL=1','-DSEXOS_EPOCH_LOG_HOST_REAL=1','-DSEXOS_RECORD_HOST_REAL=1',
           '-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include']
    renames=[a for n in ('memcpy','memmove','memset','memcmp') for a in ('--redefine-sym',n+'=sexos_'+n)]

    def compile(cc,extra,name):
        obj=work/(name+'-memory.o');renamed=work/(name+'-renamed.o');apps=work/(name+'-apps.o');binary=work/name
        run([cc,*flags,*extra,'-Itests/kernel_memory','-Iruntime/kernel','-c',work/'kernel_memory.c','-o',obj])
        run(['objcopy',*renames,obj,renamed])
        run([cc,*flags,*extra,'-Itests/kernel_memory','-Iruntime/kernel','-c',work/'kernel_apps.c','-o',apps])
        run([cc,*flags,*extra,'-I'+str(work),'-Itests/kernel_paging','-Iruntime/kernel','-Wl,--gc-sections',
             work/'program_image.c',work/'program_image_bytes.c','runtime/kernel/image_failure.c',
             'tests/kernel_private/host.c','tests/kernel_memory/worker_stubs.c',work/'kernel_syscalls.c',work/'region_writer.c',work/'kernel_region_writer.c',work/'device_reset.c',work/'kernel_device_reset.c',work/'kernel_sources.c',work/'source_policy.c',work/'pci_policy.c',work/'network_config.c',work/'history_recovery.c',work/'history_decode.c',work/'kernel_history.c',work/'kernel_pci.c',work/'kernel_paging.c',work/'kernel_task_wait.c',work/'program_memory.c',work/'pager_state.c',work/'app_private.c',work/'kernel_boot_memory.c',work/'memory_regions.c',work/'frame_lease.c',work/'kernel_epoch_log.c',work/'epoch_journal.c',work/'kernel_device_interrupt.c',work/'program_records.c',work/'program_record_memory.c',work/'program_record_tasks.c',work/'kernel_records.c',work/'kernel_rpc.c',renamed,apps,'-o',binary])
        return binary

    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            name=cc+'-'+mode;run([compile(cc,extra,name)]);builds.append(name)
            print(name,'catalog layouts/manifests/detached successors/activation chain passed',flush=True)
    mutants=[
        ('main_task_namespace','kernel_domains',
         'next_task_id >= PROGRAM_WORKER_TASK_FIRST','false'),
        ('main_task_last_valid','kernel_domains',
         'next_task_id >= PROGRAM_WORKER_TASK_FIRST','next_task_id >= PROGRAM_WORKER_TASK_FIRST - 1'),
        ('main_task_wrap','kernel_domains','next_task_id += 1;',
         'next_task_id = next_task_id + 1 == PROGRAM_WORKER_TASK_FIRST ? 1 : next_task_id + 1;'),
        ('worker_run_authority','kernel_domains','kernel_current_worker->run != d->task_id','false'),
        ('worker_lower_authority','kernel_domains','kernel_current_worker->id < PROGRAM_WORKER_TASK_FIRST','false'),
        ('worker_upper_authority','kernel_domains','kernel_current_worker->id >= PROGRAM_WORKER_TASK_END','false'),
        ('dead_run_authority','kernel_domains','d->task_state == TASK_EXITED || d->task_state == TASK_DORMANT','false'),
        ('retire_without_ACK','kernel_apps','a->acked!=APP_PRIVATE_REQUIRED','false'),
        ('retire_before_demand_cleanup','kernel_apps','!kernel_paging_cleanup_complete(a->target)','false'),
        ('retain_dead_capabilities','kernel_apps','c->target->caps[slot]=NULL;','(void)slot;'),
        ('handover_keeps_source_leaf','kernel_apps','app_private_leaf_write(c->app->image.pages[i].leaf,0);','(void)i;'),
        ('handover_skips_workspace_leaf','kernel_apps','app_private_leaf_write(c->app->workspace.pages[i].leaf,c->app->workspace.pages[i].expected);','(void)i;'),
        ('handover_keeps_frame_lease','kernel_apps','if (!program_frame_handover_commit(&c->app->image, &c->app->workspace,','if (false && !program_frame_handover_commit(&c->app->image, &c->app->workspace,'),
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
        ('activation_ignores_epoch_exhaustion','kernel_apps','if(!kernel_epoch_current(kernel.epoch) || kernel.run_queue.count>=KERNEL_RUN_QUEUE_CAPACITY)','if(false || kernel.run_queue.count>=KERNEL_RUN_QUEUE_CAPACITY)'),
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
        ('web_client_slot','kernel_current_boot','340 + index, 259,','340 + index, 260,'),
        ('web_client_not_shared','kernel_boot_memory','if (writer_transfer)','if (true)'),
        ('record_shell_slot','kernel_records','cap_set(shell, PROGRAM_RECORD_SLOT_SHELL,','cap_set(shell, PROGRAM_RECORD_SLOT_SHELL + 1,'),
        ('record_files_route','kernel_records','cap_set(shell, PROGRAM_RECORD_FILES_ROUTE_SLOT, (Cap){.kind = CAP_DOMAIN, .target = files->id})','cap_set(shell, PROGRAM_RECORD_FILES_ROUTE_SLOT, (Cap){.kind = CAP_DOMAIN, .target = files->id + 1})'),
        ('record_image_identity','kernel_boot_memory','MemoryRegionGrant grant = {.region_id = PROGRAM_RECORD_IMAGE_REGION_ID,','MemoryRegionGrant grant = {.region_id = 0x4001,'),
        ('record_prepare_table','kernel_records','if (!prepare_page_table(program_record_virtual_page(index, 0), PROGRAM_RECORD_KEY))','if (false)'),
        ('record_pool_backing','kernel_records','boot_backing_reserve(PROGRAM_RECORD_POOL_REGION_ID,','boot_backing_reserve(PROGRAM_RECORD_POOL_REGION_ID - 1,'),
        ('domain_admission_retry','kernel_domains','d->admission_pending = true;','d->admission_pending = false;'),
        ('record_unauthorized','kernel_records','if (!held || held->kind != CAP_RECORD_POOL)','if (!held || held->kind == CAP_NONE)'),
        ('record_rearm','program_record_tasks','record->phase = PROGRAM_WORKER_DETACHED;','record->phase = PROGRAM_WORKER_CONSTRUCTED;'),
        ('record_shared_pop','program_record_tasks','queue->head = (queue->head + 1) % PROGRAM_RECORD_QUEUE_CAPACITY;','queue->head = queue->head;'),
        ('record_sleep_pending','program_record_tasks','if (task->pending) {','if (true) {'),
        ('record_sleep_evict','program_record_memory','memory_present(table, memory, index, false);\n    program_records_evict(table);','program_records_evict(table);'),
        ('record_reap_stale','program_record_memory','snapshot->id != record->task','snapshot->id == record->task'),
        ('record_reap_owner','program_record_memory','snapshot->owner_core != PROGRAM_RECORD_NO_CORE','snapshot->owner_core == PROGRAM_RECORD_NO_CORE'),
        ('record_activate_mapping','kernel_records','uint64_t status = program_record_memory_before_dispatch(\n        &kernel_program_records, &kernel_program_record_memory, task->pd, task->id);','uint64_t status = PROGRAM_RECORD_OK;'),
        ('record_preempt_requeue','kernel_records','if (requeue && !program_record_task_activate(&kernel.run_queue, task))','if (requeue && false && !program_record_task_activate(&kernel.run_queue, task))'),
        ('record_print_bounds','kernel_records','copy_from_user(text, address, (size_t)length, mask_for_key(PROGRAM_RECORD_KEY))','memset(text, 0, (size_t)length)'),
    ];caught=[]
    if args.mutations:
        for name,unit,before,after in mutants:
            path=work/(unit+'.c');original=LayoutText(path.read_text());assert original.count(before)>=1,(name,'mutation anchor')
            try:
                path.write_text(original.replace(before,after));binary=compile('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('mutation survived',name);caught.append(name)
            finally:path.write_text(original)
    names=['src/'+u+'.cesta' for u in units]+['src/kernel_interface.cesta','src/app_private_interface.cesta','interfaces/kernel_app_handover_protocol.module','interfaces/kernel_app_activation_protocol.module','tests/kernel_private.py','tests/kernel_private/host.c','interfaces/kernel_current.data','generated/include/kernel_current.h']
    names+=['src/program_image_bytes.cesta','generated/include/program_image_bytes.h','runtime/kernel/image_failure.c']
    names+=['src/program_workers_interface.cesta','generated/include/program_workers.h',
            'tests/kernel_memory/worker_stubs.c']
    summary={'builds':builds,'mutations_caught':caught,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},
             'scope':'actual loader/allocator/pagewalks/private-manifest, task authority, reset and epoch producer code on RAM, CPU/TLB providers; synthetic ELF payload; no durable journal, user execution or current-image boot/refinement claim'}
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('private admission qualification passed',flush=True)


if __name__=='__main__':main()
