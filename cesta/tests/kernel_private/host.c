/* Actual loader, manifest capture and page tables. CPU/TLB are providers. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_domains.c"
#include "kernel_current_boot.c"

unsigned host_invalidations;
void klog(const char *format,...) { (void)format; }
_Noreturn void kernel_halt(const char *reason) { fprintf(stderr,"%s\n",reason);abort(); }
HistoryReply history_query_provider(HistoryQueryContext context,uint64_t kind)
{
    TrapFrame f={.rdi=context.slot,.rsi=kind,.rdx=77,.r10=88,.r8=99};
    kernel_history_query(domain_get(kernel.current),&f);
    return (HistoryReply){f.rax,f.rsi,f.rdx,f.r10,f.r8};
}

static uint32_t pci_command_word=0xabcd0007u,pci_low=0xf0000000u;
static bool pci_sizing;
static unsigned pci_writes;
uint32_t pci_read(uint8_t bus,uint8_t dev,uint8_t fun,uint8_t offset)
{
    assert(bus==0 && dev==3 && fun==0);
    if(offset==4)return pci_command_word;
    if(offset==0x10)return pci_sizing ? 0xffff0000u : pci_low;
    abort();
}
void pci_write(uint8_t bus,uint8_t dev,uint8_t fun,uint8_t offset,uint32_t value)
{
    assert(bus==0 && dev==3 && fun==0);++pci_writes;
    if(offset==4){assert((value&0xffff0000u)==0);pci_command_word=value;return;}
    assert(offset==0x10);pci_sizing=value==UINT32_MAX;if(!pci_sizing)pci_low=value;
}

typedef struct { uint32_t id; uint8_t key,layout; bool active; const char *name; } Descriptor;
static const Descriptor descriptors[]={
#define DOMAIN_ROW(p_id,p_name,p_module,p_log,p_key,p_layout,p_active) {p_id,p_key,p_layout,p_active,p_log},
    SEXOS_CURRENT_DOMAIN_ROWS(DOMAIN_ROW)
#undef DOMAIN_ROW
};

static void task_namespace(void)
{
    Domain unattached = {0};
    next_task_id = PROGRAM_WORKER_TASK_FIRST - 1;
    assert(task_attach(&unattached));
    assert(unattached.task_id == PROGRAM_WORKER_TASK_FIRST - 1);
    assert(next_task_id == PROGRAM_WORKER_TASK_FIRST);
    Domain untouched = {0};
    for (unsigned attempt = 0; attempt < 2; ++attempt) {
        assert(!task_attach(&untouched) && !untouched.task_id);
        assert(next_task_id == PROGRAM_WORKER_TASK_FIRST);
        /* Refuse before allocating a domain record or touching any image. */
        size_t count = kernel.domains.count;
        assert(!domain_spawn_at(1, 1, 1, "exhausted", NULL));
        assert(kernel.domains.count == count && !kernel.domains.slots[1]);
    }
    next_task_id = 0;
    assert(!task_attach(&untouched) && !untouched.task_id && !next_task_id);
    next_task_id = UINT32_MAX;
    assert(!task_attach(&untouched) && !untouched.task_id && next_task_id == UINT32_MAX);
    next_task_id = 1;
    assert(!task_attach(&unattached) && next_task_id == 1);
}

static void denied_task_authority(Domain *d)
{
    assert(!kernel_current_task(d));
    ProgramMemoryState before = d->program;
    assert(kernel_program_memory(d, 4096).status == PROGRAM_MEMORY_DENIED);
    assert(!memcmp(&before, &d->program, sizeof before));
}

static void task_authority(Domain *d)
{
    uint32_t saved_current = kernel.current;
    uint8_t saved_state = d->task_state;
    uint32_t saved_task = d->task_id;
    assert(!kernel_current_worker);
    kernel.current = d->id;
    d->task_state = TASK_RUNNING;
    assert(kernel_current_task(d) == saved_task);
    ProgramWorker valid = {.id = PROGRAM_WORKER_TASK_FIRST, .target = d->id,
        .run = saved_task, .phase = PROGRAM_WORKER_ATTACHED, .owner_core = 0};
    ProgramWorker worker = valid;
    kernel_current_worker = &worker;
    assert(kernel_current_task(d) == valid.id);
    ++worker.run; denied_task_authority(d); worker = valid;
    ++worker.target; denied_task_authority(d); worker = valid;
    worker.owner_core = 1; denied_task_authority(d); worker = valid;
    worker.phase = PROGRAM_WORKER_DISPATCHING; denied_task_authority(d); worker = valid;
    worker.exited = true; denied_task_authority(d); worker = valid;
    worker.id = PROGRAM_WORKER_TASK_FIRST - 1; denied_task_authority(d); worker = valid;
    worker.id = PROGRAM_WORKER_TASK_END; denied_task_authority(d); worker = valid;
    d->task_state = TASK_EXITED; denied_task_authority(d);
    d->task_state = TASK_DORMANT; denied_task_authority(d);
    d->task_state = TASK_RUNNING;
    ++d->task_id; denied_task_authority(d); d->task_id = saved_task;
    kernel_current_worker = NULL;
    d->task_id = PROGRAM_WORKER_TASK_FIRST; denied_task_authority(d);
    d->task_id = saved_task;
    d->task_state = TASK_EXITED; denied_task_authority(d);
    d->task_state = saved_state;
    kernel.current = saved_current;
}

static void domain_admission_backpressure(Domain *shell)
{
    for (uintptr_t i = 0; i < PROGRAM_RECORD_QUEUE_CAPACITY; ++i)
        assert(program_record_queue_push(&kernel.run_queue, UINT64_C(0x100000000000) + i * 16));
    run_push(shell->id);
    assert(shell->admission_pending && !shell->queued && shell->task_state == TASK_READY);
    assert(program_record_queue_pop(&kernel.run_queue));
    kernel_task_poll();
    assert(!shell->admission_pending && shell->queued);
    assert(kernel.run_queue.count == PROGRAM_RECORD_QUEUE_CAPACITY);
    bool shell_seen = false;
    while (kernel.run_queue.count)
        shell_seen |= program_record_queue_pop(&kernel.run_queue) == shell->id;
    assert(shell_seen);
    program_record_queue_init(&kernel.run_queue);
    shell->queued = false;
    shell->admission_pending = false;
}

static void record_lifecycle(const uint8_t *elf)
{
    Domain *shell=domain_get(CURRENT_PD_silk_shell);
    program_record_queue_init(&kernel.run_queue);
    for (uint32_t id=1; id<KERNEL_MAX_DOMAINS; ++id)
        if (domain_get(id)) domain_get(id)->queued=false;
    kernel.scheduler_running=true;
    domain_admission_backpressure(shell);
    kernel.current=shell->id;shell->task_state=TASK_RUNNING;
    assert(kernel_records_boot_audit() && program_record_memory_audit(&kernel_program_records,&kernel_program_record_memory));
    memcpy(phys_to_virt(kernel_record_image_physical()),elf,512);

    uint64_t lent_before=program_frame_lent();
    TrapFrame denied={.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_START,.rsi=512,.rdx=1};
    Domain *other=domain_get(CURRENT_PD_sexnet);kernel.current=other->id;other->task_state=TASK_RUNNING;
    kernel_syscall(&denied);
    assert(denied.rax==PROGRAM_RECORD_INVALID && !kernel_program_records.live && program_frame_lent()==lent_before);

    const Cap *record_pool=cap_get(shell,PROGRAM_RECORD_SLOT_SHELL);
    Cap wrong_kind=*record_pool;wrong_kind.kind=CAP_REGION;
    shell->caps[PROGRAM_RECORD_SLOT_SHELL]=&wrong_kind;
    kernel.current=shell->id;
    denied=(TrapFrame){.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_START,.rsi=512,.rdx=1};
    kernel_syscall(&denied);
    assert(denied.rax==PROGRAM_RECORD_INVALID && !kernel_program_records.live && program_frame_lent()==lent_before);
    shell->caps[PROGRAM_RECORD_SLOT_SHELL]=record_pool;

    run_push(shell->id);kernel.current=shell->id;
    TrapFrame start=(TrapFrame){.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_START,.rsi=512,.rdx=1};kernel_syscall(&start);
    assert(start.rax==PROGRAM_RECORD_OK && start.rsi==1 && start.rdx==1 && program_frame_lent()==lent_before);
    assert(run_pop()==shell->id);switch_to(shell->id);
    size_t index=kernel_program_records.start_cursor;
    uint64_t pd=PROGRAM_RECORD_BASE+index;
    ProgramRecordTask *task=&kernel_record_tasks[index];
    assert(task->pd==pd && task->id==PROGRAM_RECORD_TASK_FIRST && task->phase==PROGRAM_WORKER_QUEUED);
    assert(run_pop()==(uintptr_t)task && task->phase==PROGRAM_WORKER_DISPATCHING);
    TrapFrame *record_frame=switch_to((uintptr_t)task);
    assert(record_frame==kernel_record_storage(task)->frame && kernel_current_record==task);
    assert(record_frame->rip==program_record_virtual_page(index,0)+0x40 &&
           record_frame->rsp==program_record_stack_top(index)-8);
    assert(read_pte(record_frame->rip,&(uint64_t){0}));

    static uint8_t host_print[4096] __attribute__((aligned(4096)));
    uint64_t print_va=(uint64_t)(uintptr_t)host_print,print_physical=frames_alloc(0);
    assert(print_physical!=NO_ADDRESS);
    assert(map_page(print_va,print_physical,PTE_USER|PTE_WRITABLE|PTE_NO_EXECUTE,PROGRAM_RECORD_KEY));
    const uint8_t printed[]={'o','k','\n'};
    memcpy(host_print,printed,sizeof printed);
    TrapFrame print=*record_frame;
    print.rax=SEXOS_SYSCALL_call;print.rdi=0;print.rsi=69;print.rdx=print_va;print.r10=sizeof printed;
    assert(user_range_ok(print_va,sizeof printed,false,mask_for_key(PROGRAM_RECORD_KEY)));
    assert(kernel_syscall(&print)==&print);
    assert(!print.rax && !print.rsi && !print.rdx);
    print.rax=SEXOS_SYSCALL_call;print.rsi=69;print.rdx=0;print.r10=0;
    assert(kernel_syscall(&print)==&print && !print.rax);
    print.rax=SEXOS_SYSCALL_call;print.rsi=69;print.rdx=UINT64_MAX-1;print.r10=4;
    assert(kernel_syscall(&print)==&print && print.rax==ERR_CAP_INVALID);

    run_push(shell->id);
    TrapFrame slept={.rax=70,.rdi=2,.rip=record_frame->rip};
    assert(kernel_record_sleep(&slept)==shell->frame && !kernel_current_record && !kernel_program_records.tenant);
    assert(task->phase==PROGRAM_WORKER_SWITCHING_OUT);
    kernel_record_stack_departed();
    assert(task->phase==PROGRAM_WORKER_DETACHED && task->owner_core==PROGRAM_RECORD_NO_CORE);
    kernel.ticks+=2;kernel_records_poll();
    assert(run_pop()==(uintptr_t)task);switch_to((uintptr_t)task);
    assert(read_pte(record_frame->rip,&(uint64_t){0}));
    TrapFrame stop_query={.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_STOP_REQUESTED};
    kernel_record_syscall(&stop_query);assert(!stop_query.rax && !stop_query.rsi);

    ++task->id;
    assert(program_record_memory_reap(&kernel_program_records,&kernel_program_record_memory,
                                      kernel_record_tasks).first == 0);
    --task->id;
    task->exited=true;task->phase=PROGRAM_WORKER_DEAD;task->owner_core=0;
    assert(program_record_memory_reap(&kernel_program_records,&kernel_program_record_memory,
                                      kernel_record_tasks).first == 0);
    task->phase=PROGRAM_WORKER_ATTACHED;task->exited=false;task->owner_core=0;
    uint32_t old_task=task->id;
    run_push(shell->id);
    TrapFrame finished={.rax=62};kernel_record_finish(&finished,false);
    kernel_record_stack_departed();kernel_records_poll();
    assert(kernel_program_records.entries[index].state==PROGRAM_RECORD_FREE && !kernel_program_records.live);
    assert(kernel_program_records.ended==1 && kernel_program_records.pool_free==kernel_program_records.pool_total);
    assert(program_record_memory_audit(&kernel_program_records,&kernel_program_record_memory));

    kernel.current=shell->id;shell->task_state=TASK_RUNNING;
    start=(TrapFrame){.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_START,.rsi=512,.rdx=1};kernel_syscall(&start);
    assert(start.rax==PROGRAM_RECORD_OK);
    index=kernel_program_records.start_cursor;pd=PROGRAM_RECORD_BASE+index;task=&kernel_record_tasks[index];
    assert(task->id>old_task);
    assert(run_pop()==(uintptr_t)task);switch_to((uintptr_t)task);
    run_push(shell->id);finished=(TrapFrame){.rax=62};kernel_record_finish(&finished,false);
    kernel_record_stack_departed();kernel_records_poll();
    assert(kernel_program_records.ended==2 && kernel_program_records.pool_free==kernel_program_records.pool_total);

    kernel.current=shell->id;shell->task_state=TASK_RUNNING;
    start=(TrapFrame){.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_START,.rsi=512,.rdx=1};kernel_syscall(&start);assert(start.rax==PROGRAM_RECORD_OK && start.rsi==1);
    index=kernel_program_records.start_cursor;pd=PROGRAM_RECORD_BASE+index;task=&kernel_record_tasks[index];
    TrapFrame stop={.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_STOP,.rsi=1,.rdx=0};
    kernel_record_syscall(&stop);
    assert(stop.rax==PROGRAM_RECORD_OK && stop.rsi==1 && stop.rdx==1);
    assert(!run_pop());
    kernel_records_poll();
    assert(kernel_program_records.ended==3 && !kernel_program_records.live &&
           kernel_program_records.pool_free==kernel_program_records.pool_total);
    TrapFrame census={.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_CENSUS};
    kernel_record_syscall(&census);
    assert(!census.rax && !census.rsi && census.rdx==kernel_program_records.pool_total);
    assert(program_record_memory_audit(&kernel_program_records,&kernel_program_record_memory));

    kernel.current=shell->id;shell->task_state=TASK_RUNNING;
    start=(TrapFrame){.rax=PROGRAM_RECORD_SYSCALL,.rdi=PROGRAM_RECORD_START,.rsi=512,.rdx=2};
    kernel_syscall(&start);
    assert(start.rax==PROGRAM_RECORD_OK && start.rsi==2 && kernel_program_records.live==2);
    size_t second_index=kernel_program_records.start_cursor;
    size_t first_index=(second_index+kernel_program_records.count-1)%kernel_program_records.count;
    ProgramRecordTask *first=&kernel_record_tasks[first_index];
    ProgramRecordTask *second=&kernel_record_tasks[second_index];
    assert(run_pop()==(uintptr_t)first && first->phase==PROGRAM_WORKER_DISPATCHING);
    TrapFrame *first_frame=switch_to((uintptr_t)first);
    assert(read_pte(first_frame->rip,&(uint64_t){0}));
    assert(!read_pte(kernel_record_storage(second)->frame->rip,&(uint64_t){0}));
    assert(schedule_tick(first_frame)==kernel_record_storage(second)->frame);
    assert(kernel_current_record==second && second->phase==PROGRAM_WORKER_ATTACHED);
    assert(!read_pte(first_frame->rip,&(uint64_t){0}));
    assert(read_pte(kernel_record_storage(second)->frame->rip,&(uint64_t){0}));
    kernel_record_stack_departed();
    assert(first->phase==PROGRAM_WORKER_QUEUED && !kernel_outgoing_record);
    TrapFrame peer_sleep={.rax=70,.rdi=2,.rip=kernel_record_storage(second)->frame->rip};
    assert(kernel_record_sleep(&peer_sleep)==kernel_record_storage(first)->frame);
    assert(kernel_current_record==first && second->phase==PROGRAM_WORKER_SWITCHING_OUT);
    kernel_record_stack_departed();
    assert(second->phase==PROGRAM_WORKER_DETACHED && second->blocked);
    run_push(shell->id);
    finished=(TrapFrame){.rax=62};kernel_record_finish(&finished,false);
    kernel_record_stack_departed();
    kernel.ticks+=2;kernel_records_poll();
    assert(run_pop()==(uintptr_t)second);switch_to((uintptr_t)second);
    run_push(shell->id);finished=(TrapFrame){.rax=62};kernel_record_finish(&finished,false);
    kernel_record_stack_departed();kernel_records_poll();
    assert(kernel_program_records.ended==5 && !kernel_program_records.live &&
           kernel_program_records.pool_free==kernel_program_records.pool_total);
    assert(program_record_memory_audit(&kernel_program_records,&kernel_program_record_memory));
}

int main(void)
{
    task_namespace();
    enum { BYTES=96*1024*1024 };
    unsigned char *ram=aligned_alloc(4096,BYTES);assert(ram);memset(ram,0xa5,BYTES);
    hhdm_offset=(uint64_t)(uintptr_t)ram;memset(ram+0x1000,0,4096);
    LimineMemmapEntry usable={.base=0x2000,.length=BYTES-0x2000,.type=0};
    LimineMemmapEntry *entries[]={&usable};LimineMemmapResponse memory={.entry_count=1,.entries=entries};
    memory_init(&memory);kernel_apps_plan();assert(kernel.app_count==KERNEL_CURRENT_APPS);
    unsigned char elf[512]={0};
    ElfHeader header={.ident={0x7f,'E','L','F',2,1,1},.type=3,.machine=62,.version=1,
        .phoff=sizeof(ElfHeader),.ehsize=sizeof(ElfHeader),.phentsize=sizeof(ProgramHeader),.phnum=1};
    ProgramHeader program={.type=PT_LOAD,.flags=PF_X,.offset=256,.filesz=4,.memsz=4096,.align=4096};
    memcpy(elf,&header,sizeof header);memcpy(elf+sizeof header,&program,sizeof program);
    memset(elf+256,0x90,4);
    LimineFile module={.address=elf,.size=sizeof elf};
    for (size_t i=0;i<sizeof descriptors/sizeof *descriptors;++i) {
        const Descriptor *g=&descriptors[i];
        if(g->id==1)next_task_id=1000;
        if(g->id==6)next_task_id=6;
        if(g->id==7)next_task_id=2000;
        assert(domain_spawn_at(g->id,g->layout,g->key,g->name,&module)==g->id);
        Domain *d=domain_get(g->id);assert(d && d->key==g->key && d->layout==g->layout);
        assert(d->task_id!=0 && d->storage_index==i);
        uint64_t pte=0;assert(read_pte(d->entry,&pte) && ((pte>>59)&15)==d->key && !(pte&PTE_NO_EXECUTE));
        assert(d->code_base==CODE_REGION_BASE+(uint64_t)(g->layout-1)*DOMAIN_STRIDE);
    }
    assert(kernel.domains.count==KERNEL_CURRENT_DOMAINS);
    task_authority(domain_get(CURRENT_PD_sexfiles));
    kernel.display_pd=CURRENT_PD_sexdisplay;kernel.shell_pd=CURRENT_PD_silk_shell;kernel.input_pd=CURRENT_PD_sexinput;
    kernel_pci_boot();
    assert(kernel.network_config_present && kernel.network_config_owner==CURRENT_PD_sexnet);
    assert(net_config_valid(kernel.network_config));
    NetConfigWords config_words=net_config_to_words(kernel.network_config);
    assert(config_words.word0==UINT64_C(0x010001180a00020f) && config_words.word1==UINT64_C(0x0a0002020a000202));
    assert(config_words.word2==UINT64_C(0xbb80b3fcb3feb3ff) && config_words.word3==UINT64_C(0x0a000202));
    kernel_current_routes();kernel_current_lifecycle_grants();kernel_history_boot();kernel_apps_workspaces_boot();
    assert(kernel_history_boot_valid());
    Domain *files=domain_get(CURRENT_PD_sexfiles);HistoryTargets roster={0};size_t roster_count=0;
#define EXPECT_HISTORY(p_target,p_ordinal,p_surface,p_shell_surface,p_lifecycle,p_close,p_activation,p_predecessor,p_logical,p_route) do {if(p_logical==CURRENT_PD_spindle)roster.values[roster_count++]=CURRENT_PD_##p_target;}while(0);
    SEXOS_CURRENT_APP_ROWS(EXPECT_HISTORY)
#undef EXPECT_HISTORY
    assert(roster_count==HISTORY_ROSTER_COUNT);
    kernel.current=files->id;files->task_state=TASK_RUNNING;
    HistoryDecode decoded=history_decode_for(files->id,CURRENT_PD_spindle,roster,(HistoryQueryContext){HISTORY_SLOT,0});
    assert(!decoded.status && history_valid(decoded.value));
    for(size_t i=0;i<roster_count;++i){
        HistoryParticipant p=decoded.value.participants[i];
        assert(p.target==roster.values[i] && p.task==domain_get(p.target)->task_id && p.task!=p.target);
    }
    decoded=history_decode_for(files->id,CURRENT_PD_spindle,roster,(HistoryQueryContext){HISTORY_SLOT|(UINT64_C(1)<<32),0});
    assert(decoded.status==1 && !decoded.value.holder);
    files->task_state=TASK_READY;kernel.current=0;
    Cap *history=(Cap *)(uintptr_t)cap_get(files,HISTORY_SLOT);HistoryGrant saved_history=history->history;
    ++history->history.participants[0].task;assert(!kernel_history_boot_valid());history->history=saved_history;
    assert(kernel_history_boot_valid());
    assert(kernel.cap_storage.count==KERNEL_CURRENT_ROUTES+1+7*KERNEL_CURRENT_APPS+KERNEL_CURRENT_DORMANT+HISTORY_ROSTER_COUNT+1);
    const Cap *catalog=cap_get(domain_get(3),45);
    assert(catalog && catalog->kind==CAP_APP_LIFECYCLE && catalog->lifecycle.target_pd==16 && catalog->lifecycle.task_id==domain_get(16)->task_id);
    const Cap *activation=cap_get(domain_get(3),44);
    assert(activation && activation->kind==CAP_APP_ACTIVATION && activation->activation.predecessor==9 && activation->activation.successor==16);

    Domain *original=domain_get(9),*second=domain_get(16),*third=domain_get(18);
    assert(original->key==second->key && second->key==third->key);
    assert(original->code_base!=second->code_base && second->code_base!=third->code_base);
    for (uint32_t i=0;i<kernel.app_count;++i) {
        KernelAppPrivate *a=&kernel.apps[i];
        assert(a->captured && a->image.count==17 && a->task==domain_get(a->target)->task_id);
        for (size_t j=0;j<a->image.count;++j) {
            AppPrivatePage page=a->image.pages[j];
            assert(app_private_leaf_read(page.leaf)==page.expected);
            assert(((page.expected>>59)&15)==a->key);
        }
    }
    /* A corrupted manifest cannot acquire another image's physical frame. */
    KernelAppPrivate *candidate=kernel_app_get(16);assert(candidate);
    KernelAppPrivate saved=*candidate;
    candidate->captured=false;
    candidate->image.pages[0].expected=kernel_app_get(9)->image.pages[0].expected;
    assert(!kernel_app_finish_capture(second) && !candidate->captured);
    *candidate=saved;
    /* A mismatch on the final leaf cannot clear any earlier leaf. */
    KernelAppPrivate *a=kernel_app_get(16);assert(a);
    AppPrivatePage last=a->image.pages[a->image.count-1];
    app_private_leaf_write(last.leaf,last.expected^PTE_WRITABLE);
    unsigned before=host_invalidations;
    assert(!kernel_app_reserve_successor(9,16));
    assert(host_invalidations==before && second->task_state==TASK_READY);
    for (size_t j=0;j+1<a->image.count;++j)
        assert(app_private_leaf_read(a->image.pages[j].leaf)==a->image.pages[j].expected);
    app_private_leaf_write(last.leaf,last.expected|0x60);
    before=host_invalidations;
    assert(kernel_app_reserve_successor(9,16));
    assert(kernel_app_reserve_successor(12,17));
    assert(kernel_app_reserve_successor(16,18));
    for(size_t i=0;i<sizeof descriptors/sizeof *descriptors;++i) {
        const Descriptor *d=&descriptors[i];
        if(!d->active && d->id>18)assert(kernel_app_reserve_successor(kernel_app_get(d->id)->predecessor,d->id));
    }
    size_t detached=0;for(size_t i=0;i<kernel.app_count;++i)if(kernel.apps[i].dormant)detached+=kernel.apps[i].image.count;
    assert(host_invalidations==before+detached);
    assert(second->task_state==TASK_DORMANT && third->task_state==TASK_DORMANT);
    assert(!domain_enqueue(second,(RingEntry){.type_id=0x600,.caller=1}) && second->ring.count==0);
    for (uint32_t i=0;i<kernel.app_count;++i) {
        KernelAppPrivate *app=&kernel.apps[i];Domain *d=domain_get(app->target);
        for (size_t j=0;j<app->image.count;++j) {
            AppPrivatePage page=app->image.pages[j];
            if (app->dormant) {
                assert(app_private_leaf_read(page.leaf)==0);
                assert(!user_range_ok(page.va,1,false,d->mask));
                assert(!user_range_ok(page.va,1,false,original->mask));
            } else assert(app_private_leaf_read(page.leaf)==page.expected);
        }
    }
    Domain *browser=domain_get(15),*bell=domain_get(10);KernelAppPrivate *browser_app=kernel_app_get(15);
    TrapFrame query={.rdi=48,.rsi=1};kernel_app_query(browser,&query,63);
    assert(!query.rax && query.rsi==15 && query.rdx==browser->task_id && query.r10==300 && query.r8==3);
    query=(TrapFrame){.rdi=(UINT64_C(1)<<32)|48,.rsi=1};kernel_app_query(browser,&query,63);
    assert(query.rax==APP_PRIVATE_INVALID && !(query.rsi|query.rdx|query.r10|query.r8));
    query=(TrapFrame){.rdi=48,.rsi=2};kernel_app_query(browser,&query,63);assert(query.rax==APP_PRIVATE_INVALID);
    query=(TrapFrame){.rdi=40,.rsi=browser->task_id};kernel_app_query(bell,&query,64);assert(query.rax==APP_PRIVATE_LIVE && !browser_app->acked);
    query=(TrapFrame){.rdi=40,.rsi=(UINT64_C(1)<<32)|browser->task_id};kernel_app_query(bell,&query,64);assert(query.rax==APP_PRIVATE_DENIED);
    browser->task_state=TASK_EXITED;kernel.current=15;
    query=(TrapFrame){.rdi=40};kernel_app_query(bell,&query,63);assert(query.rax==APP_PRIVATE_RETRY);
    kernel.current=10;query=(TrapFrame){.rdi=40};kernel_app_query(bell,&query,63);
    assert(!query.rax && query.rsi==15 && query.rdx==browser->task_id && query.r10==31 && !query.r8);
    query=(TrapFrame){.rdi=40,.rsi=browser->task_id};kernel_app_query(bell,&query,64);assert(!query.rax && browser_app->acked==16);
    query=(TrapFrame){.rdi=40,.rsi=browser->task_id};kernel_app_query(bell,&query,64);assert(!query.rax && browser_app->acked==16);
    query=(TrapFrame){.rdi=48,.rsi=browser->task_id};kernel_app_query(browser,&query,64);assert(query.rax==APP_PRIVATE_DENIED);
    query=(TrapFrame){.rdi=40,.rsi=browser->task_id};kernel_app_query(bell,&query,99);assert(query.rax==APP_PRIVATE_INVALID && browser_app->acked==16);
    browser->task_state=TASK_READY;kernel.current=0;browser_app->acked=0;
    assert(kernel_current_catalog_valid());
    Domain *subject=domain_get(16);Cap *self=(Cap *)(uintptr_t)cap_get(subject,48);Cap saved_cap=*self;
    self->lifecycle.rights|=APP_PRIVATE_RETIRE;assert(!kernel_current_catalog_valid());*self=saved_cap;
    ++subject->task_id;assert(!kernel_current_catalog_valid());--subject->task_id;
    Cap *changed_activation=(Cap *)(uintptr_t)cap_get(domain_get(3),44);saved_cap=*changed_activation;
    ++changed_activation->activation.predecessor_task;assert(!kernel_current_catalog_valid());*changed_activation=saved_cap;
    Cap *route=(Cap *)(uintptr_t)cap_get(domain_get(3),47);saved_cap=*route;
    route->target=18;assert(!kernel_current_catalog_valid());*route=saved_cap;
    assert(kernel_current_catalog_valid());
    current_regions();
    size_t web_index=0;
#define CHECK_WEB_REGION(p_target,p_ordinal,p_surface,p_shell_surface,p_lifecycle,p_close,p_activation,p_predecessor,p_logical,p_route) \
    do { \
        if (p_logical==CURRENT_PD_spindle || p_logical==CURRENT_PD_kaleidoscope) { \
            const Cap *provider=cap_get(domain_get(CURRENT_PD_sexnet),340+web_index); \
            const Cap *client=cap_get(domain_get(CURRENT_PD_##p_target),259); \
            assert(provider && client && provider->kind==CAP_REGION && client->kind==CAP_REGION); \
            const MemoryRegionGrant *p=&provider->region.grant,*c=&client->region.grant; \
            assert(p->region_id==0x5100+web_index && c->region_id==p->region_id && \
                   p->length==64*1024 && c->length==p->length && p->rights==7 && c->rights==7 && \
                   p->virtual_base!=c->virtual_base && p->has_peer && c->has_peer && \
                   p->peer_pd==CURRENT_PD_##p_target && c->peer_pd==CURRENT_PD_sexnet && \
                   kernel_region_writer_expected_write(p->region_id,CURRENT_PD_sexnet,true) && \
                   kernel_region_writer_expected_write(c->region_id,CURRENT_PD_##p_target,true)); \
            ++web_index; \
        } \
    } while (0);
    SEXOS_CURRENT_APP_ROWS(CHECK_WEB_REGION)
#undef CHECK_WEB_REGION
    assert(web_index==KERNEL_CURRENT_WEB);
    const Cap *record_pool=cap_get(domain_get(CURRENT_PD_silk_shell),PROGRAM_RECORD_SLOT_SHELL);
    const Cap *record_route=cap_get(domain_get(CURRENT_PD_silk_shell),PROGRAM_RECORD_FILES_ROUTE_SLOT);
    const Cap *record_image=cap_get(domain_get(CURRENT_PD_sexfiles),PROGRAM_RECORD_IMAGE_SLOT_FILES);
    assert(record_pool && record_pool->kind==CAP_RECORD_POOL);
    assert(record_route && record_route->kind==CAP_DOMAIN && record_route->target==CURRENT_PD_sexfiles);
    assert(record_image && record_image->kind==CAP_REGION);
    assert(record_image->region.grant.region_id==PROGRAM_RECORD_IMAGE_REGION_ID &&
           record_image->region.grant.length==PROGRAM_RECORD_IMAGE_BYTES &&
           record_image->region.grant.rights==7 && !record_image->region.grant.has_dma);
    uint64_t record_pte=0;
    assert(read_pte(record_image->region.grant.virtual_base,&record_pte) &&
           (record_pte&PTE_ADDRESS)==kernel_record_image_physical());
    assert(kernel_program_records.count==16 && !kernel_program_records.live &&
           kernel_program_records.pool_free==kernel_program_records.pool_total);
    for(size_t i=0;i<kernel_program_records.count;++i){
        uint64_t *entry=(uint64_t *)(uintptr_t)page_leaf_address(program_record_virtual_page(i,0));
        assert(entry && !*entry);
    }
    assert(kernel_records_boot_audit());
    assert(!kernel_records_boot() && kernel_records_boot_audit());
    size_t expected_backings = KERNEL_CURRENT_BACKINGS - 2;
    /* The epoch batch and Shell's typed Files route each add one private grant. */
    size_t expected_caps = KERNEL_CURRENT_ROUTES+1+7*KERNEL_CURRENT_APPS+KERNEL_CURRENT_DORMANT+HISTORY_ROSTER_COUNT+1+14+2*KERNEL_CURRENT_QUILS+2*KERNEL_CURRENT_WEB+1;
    if (kernel.boot_backing_count != expected_backings || kernel.cap_storage.count != expected_caps)
        fprintf(stderr, "BOOT resources: backings=%zu/%zu capabilities=%zu/%zu\n",
                (size_t)kernel.boot_backing_count, expected_backings,
                (size_t)kernel.cap_storage.count, expected_caps);
    assert(kernel.boot_backing_count == expected_backings && kernel.cap_storage.count == expected_caps);
    PciFunction nic={.bus=0,.dev=3,.func=0,.vendor_id=0x8086,.device_id=0x100e};
    PciFunction gpu=nic;gpu.class_id=3;
    Cap gpu_lease=pci_cap(domain_get(1),4,&gpu);
    assert(gpu_lease.kind==CAP_PCI && !gpu_lease.region.present && !gpu_lease.command.present && !pci_writes);
    MemoryRegionCapability view={0};
    assert(kernel_boot_pci_region(domain_get(13),19,&nic,&view));
    assert(pci_command_word==7 && pci_low==0xf0000000u && !pci_sizing && pci_writes==4);
    assert(view.present && view.grant.length==65536 && !view.grant.has_dma);
    uint64_t mmio=0;assert(read_pte(view.grant.virtual_base,&mmio));
    assert((mmio&PTE_ADDRESS)==0xf0000000u && (mmio&(PTE_NO_CACHE|PTE_WRITE_THROUGH|PTE_NO_EXECUTE))==(PTE_NO_CACHE|PTE_WRITE_THROUGH|PTE_NO_EXECUTE));
    unsigned writes=pci_writes;
    assert(!kernel_boot_pci_region(domain_get(13),(UINT64_C(1)<<32)|19,&nic,&view) && pci_writes==writes);
    kernel_paging_boot();kernel_sources_boot();
    /* Display's frames through the production reservation, at a small geometry
     * this RAM model holds; malformed geometry is refused before any backing. */
    DisplayFrameLayout frames_layout={0};
    assert(!display_frame_layout(64,32,255,&frames_layout) && !display_frame_layout(0,32,0,&frames_layout));
    assert(display_frame_layout(64,32,256,&frames_layout) && frames_layout.frame_bytes==8192 && frames_layout.span_bytes==8192 && frames_layout.total_bytes==16384 && frames_layout.buddy_bytes==16384);
    kernel_boot_display_frames(1,&frames_layout);
    const Cap *frames_cap=cap_get(domain_get(1),DISPLAY_FRAME_SLOT);
    assert(frames_cap && frames_cap->kind==CAP_REGION && frames_cap->region.grant.region_id==DISPLAY_FRAME_REGION_ID && frames_cap->region.grant.length==16384 && !frames_cap->region.grant.has_dma);
    assert(kernel.boot_backing_count == KERNEL_CURRENT_BACKINGS &&
           kernel.cap_storage.count == expected_caps + 5 + KERNEL_CURRENT_SOURCE_CAPS + 1);
    kernel_seal_genesis();
    for (size_t i=0;i<sizeof descriptors/sizeof *descriptors;++i)run_push(descriptors[i].id);
    assert(kernel.run_queue.count==15);
    kernel_current_boot_audit();
    uint32_t count=0,id;
    while ((id=run_pop())!=0) {assert(id<=15 && domain_get(id)->task_state==TASK_READY);++count;}
    assert(count==15);
    assert(!kernel_app_reserve_successor(9,16));
    Domain *coordinator=domain_get(3);KernelAppPrivate *old_app=kernel_app_get(9),*next_app=kernel_app_get(16);
    TrapFrame activate={.rdi=44,.rsi=0};kernel_app_activation(coordinator,&activate);
    assert(!activate.rax && activate.rsi==16 && activate.rdx==domain_get(16)->task_id && activate.r10==301 && !activate.r8);
    activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(activate.rax==APP_PRIVATE_RETRY && !(activate.rsi|activate.rdx|activate.r10|activate.r8));
    activate=(TrapFrame){.rdi=(UINT64_C(1)<<32)|44,.rsi=0};kernel_app_activation(coordinator,&activate);assert(activate.rax==APP_PRIVATE_INVALID);
    activate=(TrapFrame){.rdi=44,.rsi=2};kernel_app_activation(coordinator,&activate);assert(activate.rax==APP_PRIVATE_INVALID);
    original->task_state=TASK_EXITED;old_app->acked=31;old_app->retired=true;
    before=host_invalidations;activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(activate.rax==APP_PRIVATE_DENIED && host_invalidations==before && next_app->dormant);
    original->task_state=TASK_READY;old_app->acked=0;old_app->retired=false;
    /* The production path retains live bytes and requires the complete ACK
     * quorum before handing the preallocated aliases to the actual pager. */
    Domain *pager=domain_get(CURRENT_PD_sext);kernel.current=pager->id;pager->task_state=TASK_RUNNING;
    browser->task_state=TASK_EXITED;browser_app->acked=31;
    TrapFrame demand_retire={.rdi=40};kernel_app_retire(pager,&demand_retire);
    assert(demand_retire.rax==APP_PRIVATE_RETRY && !browser_app->retired && !kernel.paging.ledger.closed[0].present);
    for(size_t i=0;i<browser_app->image.count;++i)assert(app_private_leaf_read(browser_app->image.pages[i].leaf)==browser_app->image.pages[i].expected && !app_private_leaf_read(browser_app->workspace.pages[i].leaf));
    browser->task_state=TASK_READY;browser_app->acked=0;
    for(size_t i=0;i<old_app->workspace.count;++i)assert(!app_private_leaf_read(old_app->workspace.pages[i].leaf));
    TrapFrame retire={.rdi=38};kernel_app_retire(pager,&retire);
    assert(retire.rax==APP_PRIVATE_LIVE && !(retire.rsi|retire.rdx|retire.r10|retire.r8));
    original->task_state=TASK_EXITED;old_app->acked=0;original->queued=true;
    retire=(TrapFrame){.rdi=38};kernel_app_retire(pager,&retire);assert(retire.rax==APP_PRIVATE_RETRY);original->queued=false;
    const uint32_t service_ids[]={3,1,11,13,10,14};
    for(size_t role=0;role<6;++role)domain_get(service_ids[role])->ring=(Ring){0};
    Domain *blocked=domain_get(1);blocked->ring.count=KERNEL_RING_CAPACITY;blocked->ring.revision=1;
    kernel.ticks=100;kernel_app_poll();assert(old_app->hinted==63 && old_app->last_hint[1]==100);
    for(size_t role=0;role<6;++role)assert(domain_get(service_ids[role])->ring.count==(role==1?KERNEL_RING_CAPACITY:1));
    kernel_app_poll();kernel.ticks=115;kernel_app_poll();
    for(size_t role=0;role<6;++role)assert(domain_get(service_ids[role])->ring.count==(role==1?KERNEL_RING_CAPACITY:1));
    blocked->ring=(Ring){0};kernel.ticks=116;kernel_app_poll();assert(blocked->ring.count==1);
    for(size_t role=0;role<6;++role){
        Domain *service=domain_get(service_ids[role]);
        for(size_t i=0;i<service->ring.count;++i){RingEntry m=service->ring.entries[i];assert(m.type_id==0x107 && !m.caller && !m.caller_task && !m.from_call && m.arg0==9 && m.arg1==original->task_id);}
        service->ring=(Ring){0};
        if(role<5){TrapFrame ack={.rdi=38,.rsi=original->task_id};kernel_app_query(service,&ack,64);assert(!ack.rax);}
    }
    assert(old_app->acked==31);
    old_app->acked=30;retire=(TrapFrame){.rdi=38};kernel_app_retire(pager,&retire);assert(retire.rax==APP_PRIVATE_RETRY);old_app->acked=31;
    AppPrivatePage late_alias=old_app->workspace.pages[old_app->workspace.count-1];
    app_private_leaf_write(late_alias.leaf,5);before=host_invalidations;
    retire=(TrapFrame){.rdi=38};kernel_app_retire(pager,&retire);
    assert(retire.rax==APP_PRIVATE_DENIED && host_invalidations==before && !old_app->retired);
    for(size_t i=0;i<old_app->image.count;++i)assert(app_private_leaf_read(old_app->image.pages[i].leaf)==old_app->image.pages[i].expected);
    app_private_leaf_write(late_alias.leaf,0);
    original->ring=(Ring){.count=1,.revision=17};original->reply_ready=true;original->reply_value=99;
    original->reply_rights[3]=7;original->rpc[0]=(RpcSlot){.id=17,.state=RPC_HEARD,.peer=3};
    uint64_t issuer=original->rpc_next;before=host_invalidations;
    original->ring.revision=UINT64_MAX;
    retire=(TrapFrame){.rdi=38};kernel_app_retire(pager,&retire);
    assert(retire.rax==APP_PRIVATE_DENIED && !old_app->retired && host_invalidations==before && original->ring.count==1);
    original->ring.revision=17;
    uint64_t handover_epoch=kernel.epoch;uint32_t handover_events=kernel.epoch_event_count;
    kernel.epoch=kernel.epoch_event_count=UINT64_MAX;
    retire=(TrapFrame){.rdi=38};kernel_app_retire(pager,&retire);
    assert(retire.rax==APP_PRIVATE_RETRY && !old_app->retired && host_invalidations==before &&
           original->ring.count==1 && original->reply_ready && original->reply_rights[3]==7 &&
           original->rpc[0].state==RPC_HEARD);
    kernel.epoch=handover_epoch;kernel.epoch_event_count=handover_events;
    retire=(TrapFrame){.rdi=38};kernel_app_retire(pager,&retire);
    assert(!retire.rax && retire.rsi==old_app->pool && retire.rdx==old_app->image.count*4096 && retire.r10==original->task_id && !retire.r8);
    assert(old_app->retired && host_invalidations==before+2*old_app->image.count && !original->ring.count && original->ring.revision==18 && !original->reply_ready);
    assert(!original->reply_rights[3] && original->rpc[0].state==RPC_FREE && original->rpc_next==issuer);
    assert(kernel.epoch==handover_epoch+1 && kernel.epoch_event_count==handover_events+1);
    KernelEpochEvent handed=kernel.epoch_events[handover_events];
    assert(handed.epoch==handover_epoch+1 && handed.kind==KERNEL_EPOCH_APP_HANDOVER &&
           handed.subject==original->id && handed.token==original->task_id && handed.revision==original->task_id &&
           handed.address==old_app->pool && handed.pages==old_app->image.count &&
           handed.source_key==old_app->key && handed.key==pager->key);
    for(uint64_t slot=1;slot<KERNEL_CAP_SLOTS;++slot)assert(!cap_get(original,slot));
    for(size_t i=0;i<old_app->image.count;++i){
        AppPrivatePage page=old_app->workspace.pages[i];
        assert(!app_private_leaf_read(old_app->image.pages[i].leaf) && app_private_leaf_read(page.leaf)==page.expected);
        assert(user_range_ok(page.va,4096,true,pager->mask) && !user_range_ok(page.va,4096,false,original->mask));
    }
    uint64_t workspace_lent=program_frame_lent();
    assert(program_memory_main_exit(&original->program)==PROGRAM_MEMORY_OK);
    assert(program_memory_end(&original->program)==PROGRAM_MEMORY_OK);
    assert(original->program.phase==PROGRAM_MEMORY_DEAD && program_frame_lent()==workspace_lent);
    for(size_t i=0;i<old_app->workspace.count;++i)
        assert(app_private_leaf_read(old_app->workspace.pages[i].leaf)==old_app->workspace.pages[i].expected);
    before=host_invalidations;handover_epoch=kernel.epoch;handover_events=kernel.epoch_event_count;
    TrapFrame receipt=retire;retire=(TrapFrame){.rdi=38};kernel_app_retire(pager,&retire);
    assert(!retire.rax && retire.rsi==receipt.rsi && retire.rdx==receipt.rdx && retire.r10==receipt.r10 &&
           host_invalidations==before && kernel.epoch==handover_epoch && kernel.epoch_event_count==handover_events);
    pager->task_state=TASK_READY;kernel.current=0;
    assert(old_app->retired);
    old_app->acked=30;activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(activate.rax==APP_PRIVATE_RETRY && next_app->dormant && kernel.run_queue.count==0);old_app->acked=31;
    uint64_t rip=second->frame->rip;second->frame->rip=0;
    activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);assert(activate.rax==APP_PRIVATE_DENIED);second->frame->rip=rip;
    AppPrivatePage late=next_app->image.pages[next_app->image.count-1];app_private_leaf_write(late.leaf,5);
    before=host_invalidations;activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(activate.rax==APP_PRIVATE_DENIED && host_invalidations==before && kernel.run_queue.count==0);app_private_leaf_write(late.leaf,0);
    activate=(TrapFrame){.rdi=44,.rsi=0};kernel_app_activation(coordinator,&activate);assert(!activate.rax && activate.r8==1);
    uint64_t activation_epoch=kernel.epoch;uint32_t activation_events=kernel.epoch_event_count;
    kernel.epoch=kernel.epoch_event_count=UINT64_MAX;
    activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(activate.rax==APP_PRIVATE_RETRY && next_app->dormant && !next_app->activated &&
           !second->queued && kernel.run_queue.count==0 && host_invalidations==before);
    kernel.epoch=activation_epoch;kernel.epoch_event_count=activation_events;
    activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(!activate.rax && activate.r8==2 && !next_app->dormant && next_app->activated && second->queued && kernel.run_queue.count==1);
    assert(host_invalidations==before+next_app->image.count);
    assert(kernel.epoch==activation_epoch+1 && kernel.epoch_event_count==activation_events+1);
    KernelEpochEvent activated_event=kernel.epoch_events[activation_events];
    assert(activated_event.epoch==activation_epoch+1 && activated_event.kind==KERNEL_EPOCH_APP_ACTIVATION &&
           activated_event.subject==second->id && activated_event.peer==original->id &&
           activated_event.token==second->task_id && activated_event.revision==second->task_id &&
           activated_event.address==next_app->surface && activated_event.pages==next_app->image.count &&
           activated_event.key==second->key);
    for(size_t i=0;i<next_app->image.count;++i)assert(app_private_leaf_read(next_app->image.pages[i].leaf)==next_app->image.pages[i].expected);
    assert(!user_range_ok(original->entry,1,false,second->mask));
    before=host_invalidations;activation_epoch=kernel.epoch;activation_events=kernel.epoch_event_count;
    activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(!activate.rax && activate.r8==2 && kernel.run_queue.count==1 && host_invalidations==before &&
           kernel.epoch==activation_epoch && kernel.epoch_event_count==activation_events);
    assert(run_pop()==16);second->task_state=TASK_EXITED;next_app->acked=31;
    assert(app_private_retire(&next_app->image).status==0);next_app->retired=true;
    before=host_invalidations;activate=(TrapFrame){.rdi=44,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(!activate.rax && activate.r8==3 && kernel.run_queue.count==0 && host_invalidations==before);
    activate=(TrapFrame){.rdi=55,.rsi=1};kernel_app_activation(coordinator,&activate);
    assert(!activate.rax && activate.r8==2 && third->queued && kernel_app_get(18)->activated);
    assert(!user_range_ok(second->entry,1,false,third->mask));
    for(size_t i=0;i<KERNEL_CURRENT_APPS;++i) {
        CurrentAppRow row=current_apps[i];if(row.logical!=CURRENT_PD_quil || row.ordinal<=2)continue;
        KernelAppPrivate *previous=kernel_app_get(row.predecessor);Domain *pd=domain_get(row.predecessor);
        assert(run_pop()==pd->id);pd->task_state=TASK_EXITED;previous->acked=APP_PRIVATE_REQUIRED;
        assert(app_private_retire(&previous->image).status==0);previous->retired=true;
        activate=(TrapFrame){.rdi=row.activation,.rsi=1};kernel_app_activation(coordinator,&activate);
        assert(!activate.rax && activate.rsi==row.target && activate.r8==2 && domain_get(row.target)->queued);
        assert(!user_range_ok(pd->entry,1,false,domain_get(row.target)->mask));
    }
    CurrentAppRow last_quil={0};
    for(size_t i=0;i<KERNEL_CURRENT_APPS;++i)if(current_apps[i].logical==CURRENT_PD_quil)last_quil=current_apps[i];
    assert(last_quil.ordinal==KERNEL_CURRENT_QUILS-1);
    assert(kernel_app_get(last_quil.target)->identity.number==last_quil.ordinal+1 && domain_get(last_quil.target)->task_state==TASK_READY);
    program.vaddr=0x400000;program.paddr=0x400000;header.entry=0x400040;
    memcpy(elf,&header,sizeof header);memcpy(elf+sizeof header,&program,sizeof program);
    record_lifecycle(elf);
    free(ram);
    printf("private BOOT controls passed:catalog layouts/identities,disjoint manifests,detached successors,initial admissions,%u-incarnation Quil chain\n",(unsigned)KERNEL_CURRENT_QUILS);
    return 0;
}
