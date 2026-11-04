/* Production BOOT allocator, loader, fault metadata, ledger and leaf adapters.
 * CPU effects are recorded providers; no user instruction executes in this gate. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HOST_PAGING_EVENTS 1
#include "kernel_domains.c"
#include "kernel_paging.c"
#include "kernel_task_wait.c"

unsigned host_invalidations;
static unsigned effect_phase;
static uint64_t effect_frame;
static Domain *effect_target;
static uint64_t effect_addresses[4096];
void host_invalidate(uint64_t address)
{
    assert(host_invalidations<4096);effect_addresses[host_invalidations++]=address;
    if(effect_phase) {
        KernelPaging *s=&kernel.paging;
        if(effect_phase==1){
            assert(s->ledger.entries[effect_frame].present && s->ledger.entries[effect_frame].finalized);
            PgBinding b=s->ledger.entries[effect_frame].binding;
            uint64_t leaf=*(volatile uint64_t *)(uintptr_t)s->leaves[b.range_id-1][b.page_index];
            assert(leaf==expected_leaf(b) && ((leaf>>59)&15)==effect_target->key);
            assert(effect_target->task_state==TASK_READY && effect_target->queued && effect_target->fault.phase==PG_IDLE);
        }
        else if(effect_phase==2) {
            assert(!s->ledger.entries[effect_frame].present && s->ledger.retired[effect_frame].present);
            PgBinding b=s->ledger.retired[effect_frame].value;
            assert(*(volatile uint64_t *)(uintptr_t)s->leaves[b.range_id-1][b.page_index]==0);
        }
        else {
            assert(effect_phase==3 && s->ledger.closed[1].present && effect_target->fault.retired);
            assert(s->ledger.closed[1].target_pd==effect_target->id);
        }
    }
}
void klog(const char *format,...){(void)format;}
void tss_set_rsp0(uint64_t top){(void)top;}
_Noreturn void kernel_idle(void){assert(!"unexpected idle in RAM paging control");abort();}
_Noreturn void kernel_halt(const char *reason){fprintf(stderr,"%s\n",reason);abort();}
static TrapFrame invoke(Domain *actor,uint64_t number,uint64_t a,uint64_t b,uint64_t c,uint64_t d,uint64_t e)
{
    kernel.current=actor->id;
    TrapFrame f={.rdi=a,.rsi=b,.rdx=c,.r10=d,.r8=e};kernel_paging_syscall(actor,&f,number);return f;
}
static void status(TrapFrame f,uint64_t expected){assert(f.rax==expected);}
static uint64_t forward(Domain *target,uint64_t address)
{
    target->task_state=TASK_RUNNING;kernel.current=target->id;target->frame->error=6;target->frame->cs=3;
    target->frame->r12=0x123456789abcdef0;target->frame->rax=0xabcdef;
    assert(kernel_pager_forward(target,target->frame,address));assert(target->task_state==TASK_WAIT_FAULT);
    return target->fault.active.token;
}
static void drain(void){while(run_pop()){};}
static void clear_inbox(Domain *d){RingEntry m;while(domain_dequeue(d,&m)){};d->wake_pending=false;}
int main(void)
{
    enum{BYTES=16*1024*1024};unsigned char *ram=aligned_alloc(4096,BYTES);assert(ram);memset(ram,0xa5,BYTES);
    hhdm_offset=(uint64_t)(uintptr_t)ram;memset(ram+0x1000,0,4096);
    LimineMemmapEntry usable={.base=0x2000,.length=BYTES-0x2000,.type=0};
    LimineMemmapEntry *entries[]={&usable};LimineMemmapResponse memory={.entry_count=1,.entries=entries};memory_init(&memory);
    unsigned char elf[512]={0};ElfHeader h={.ident={0x7f,'E','L','F',2,1,1},.type=3,.machine=62,.version=1,.phoff=sizeof(ElfHeader),.ehsize=sizeof(ElfHeader),.phentsize=sizeof(ProgramHeader),.phnum=1};
    ProgramHeader ph={.type=PT_LOAD,.flags=PF_X,.offset=256,.filesz=4,.memsz=4096,.align=4096};
    memcpy(elf,&h,sizeof h);memcpy(elf+sizeof h,&ph,sizeof ph);memset(elf+256,0x90,4);
    LimineFile module={.address=elf,.size=sizeof elf};
    for(uint32_t id=10;id<=15;++id)if(id==10 || id==14 || id==15)assert(domain_spawn_at(id,(uint8_t)id,(uint8_t)id,"paging-client",&module)==id);
    Domain *pager=domain_get(14),*browser=domain_get(15),*bell=domain_get(10);
    assert(cap_set(browser,2,(Cap){.kind=CAP_DOMAIN,.target=14}) && cap_set(bell,2,(Cap){.kind=CAP_DOMAIN,.target=14}));
    kernel_paging_boot();KernelPaging *s=&kernel.paging;assert(s->initialized && kernel.boot_backing_count==1);
    assert(s->mappings[0].target_pd==15 && s->mappings[1].target_pd==10);
    assert(cap_get(pager,34)->kind==CAP_FRAME_POOL && cap_get(browser,35)->kind==CAP_DEMAND_REGION);
    for(size_t i=0;i<PG_CLIENTS;++i)for(size_t page=0;page<PG_PAGES;++page)assert(*(volatile uint64_t *)(uintptr_t)s->leaves[i][page]==0);
    for(size_t word=0;word<PG_PAGES*512;++word)assert(((uint64_t *)phys_to_virt(s->physical))[word]==0);
    Cap fake=*cap_get(pager,34);++fake.pool.pool_id;assert(cap_set(pager,80,fake));
    assert(cap_set(bell,36,*cap_get(pager,36)) && cap_set(bell,34,*cap_get(pager,34)));
    kernel.genesis_sealed=true;kernel.scheduler_running=true;
    assert(!kernel_paging_cleanup_complete(10) && !kernel_paging_cleanup_complete(15) && kernel_paging_cleanup_complete(9));
    TrapFrame f=invoke(pager,59,34,0,0,0,0);assert(f.rax==0 && f.rsi==s->pool.base_va && f.rdx==32768 && f.r10==0 && f.r8==6);
    f=invoke(browser,59,35,0,0,0,0);assert(f.rax==0 && f.rsi==s->mappings[0].base_va && f.r10==15 && f.r8==6);
    f=invoke(pager,59,(UINT64_C(1)<<32)|34,0,0,0,0);assert(f.rax==PG_INVALID && !(f.rsi|f.rdx|f.r10|f.r8));
    browser->task_state=TASK_RUNNING;kernel.current=15;browser->frame->cs=3;browser->frame->error=4;
    pager->reply_rights[15]=UINT64_MAX;uint32_t prior=pager->ring.count;
    assert(!kernel_pager_forward(browser,browser->frame,s->mappings[0].base_va));
    assert(browser->fault.phase==PG_IDLE && browser->task_state==TASK_RUNNING && pager->reply_rights[15]==UINT64_MAX && pager->ring.count==prior);
    pager->reply_rights[15]=0;
    uint64_t page=s->mappings[0].base_va,token=forward(browser,page+7);RingEntry notice;
    assert(domain_dequeue(pager,&notice) && notice.type_id==0x103 && notice.caller==15 && notice.caller_task==browser->task_id && notice.arg2==token);
    kernel_task_wake(browser);assert(browser->task_state==TASK_WAIT_FAULT && !browser->queued);
    f=invoke(pager,57,36,token,0,0,0);assert(f.rax==0 && f.rsi==page && f.rdx==6 && f.r10==15 && f.r8==6);
    status(invoke(bell,56,36,34,0,token,6),PG_DENIED);
    unsigned before=host_invalidations;
    status(invoke(pager,56,36,34,0,token,262),PG_DENIED);
    status(invoke(pager,56,36,80,0,token,6),PG_DENIED);
    uint64_t *words=phys_to_virt(s->physical);words[511]=99;
    status(invoke(pager,56,36,34,0,token,6),PG_DENIED);assert(browser->fault.phase==PG_PENDING && host_invalidations==before && !s->ledger.entries[0].present);words[511]=0;
    uint8_t bell_key=s->keys[1];s->keys[1]=s->keys[0];
    status(invoke(pager,56,36,34,0,token,6),PG_DENIED);
    assert(browser->fault.phase==PG_PENDING && !s->ledger.entries[0].present &&
           !*(volatile uint64_t *)(uintptr_t)s->leaves[0][0]);
    s->keys[1]=bell_key;
    uint32_t install_events=kernel.epoch_event_count;uint64_t install_epoch=kernel.epoch;
    kernel.epoch_event_count=KERNEL_EPOCH_EVENT_CAPACITY;
    status(invoke(pager,56,36,34,0,token,6),PG_EXHAUSTED);
    assert(browser->fault.phase==PG_PENDING && browser->task_state==TASK_WAIT_FAULT &&
           !s->ledger.entries[0].present && !*(volatile uint64_t *)(uintptr_t)s->leaves[0][0] &&
           host_invalidations==before);
    kernel.epoch_event_count=install_events;
    effect_phase=1;effect_frame=0;effect_target=browser;
    status(invoke(pager,56,36,34,0,token,6),PG_OK);effect_phase=0;
    assert(host_invalidations==before+1 && browser->task_state==TASK_READY && browser->queued && s->ledger.entries[0].finalized);
    assert(browser->frame->r12==UINT64_C(0x123456789abcdef0) && browser->frame->rax==0xabcdef && browser->fault.phase==PG_IDLE);
    assert(kernel.epoch==install_epoch+1 && kernel.epoch_event_count==install_events+1);
    KernelEpochEvent installed=kernel.epoch_events[install_events];
    assert(installed.epoch==install_epoch+1 && installed.kind==KERNEL_EPOCH_PAGE_INSTALL &&
           installed.subject==browser->id && installed.address==page && installed.token==token &&
           installed.revision==token && installed.frame==0 && installed.rights==6 && installed.key==browser->key);
    uint32_t count=kernel.run_count;run_push(browser->id);assert(kernel.run_count==count);status(invoke(pager,56,36,34,0,token,6),PG_OK);assert(kernel.run_count==count && host_invalidations==before+1);
    assert(run_pop()==15);assert(switch_to(15)==browser->frame);
    uint64_t old_leaf=*(volatile uint64_t *)(uintptr_t)s->leaves[0][0];assert(old_leaf==expected_leaf(s->ledger.entries[0].binding));
    *(volatile uint64_t *)(uintptr_t)s->leaves[0][0]|=0x60;
    uint32_t epoch_events=kernel.epoch_event_count;uint64_t epoch=kernel.epoch;
    kernel.epoch_event_count=KERNEL_EPOCH_EVENT_CAPACITY;
    status(invoke(pager,60,36,34,0,token,0),PG_EXHAUSTED);
    assert(*(volatile uint64_t *)(uintptr_t)s->leaves[0][0]==(old_leaf|0x60) && s->ledger.entries[0].present);
    kernel.epoch_event_count=epoch_events;
    before=host_invalidations;effect_phase=2;status(invoke(pager,60,36,34,0,token,0),PG_OK);effect_phase=0;assert(host_invalidations==before+1);
    assert(!s->ledger.entries[0].present && !*(volatile uint64_t *)(uintptr_t)s->leaves[0][0]);
    assert(kernel.epoch==epoch+1 && kernel.epoch_event_count==epoch_events+1);
    KernelEpochEvent reclaimed=kernel.epoch_events[epoch_events];
    assert(reclaimed.epoch==epoch+1 && reclaimed.kind==KERNEL_EPOCH_PAGE_RECLAIM &&
           reclaimed.subject==browser->id && reclaimed.address==page && reclaimed.token==token &&
           reclaimed.revision==token && reclaimed.frame==0 && reclaimed.rights==6);
    status(invoke(pager,56,36,34,0,token,6),PG_DENIED);
    uint64_t bell_token=forward(bell,s->mappings[1].base_va);
    status(invoke(pager,56,37,34,0,bell_token,6),PG_OK);drain();
    uint64_t bell_leaf=*(volatile uint64_t *)(uintptr_t)s->leaves[1][0];before=host_invalidations;
    epoch=kernel.epoch;epoch_events=kernel.epoch_event_count;
    status(invoke(pager,60,36,34,0,token,0),PG_OK);
    assert(host_invalidations==before && kernel.epoch==epoch && kernel.epoch_event_count==epoch_events &&
           *(volatile uint64_t *)(uintptr_t)s->leaves[1][0]==bell_leaf && s->ledger.entries[0].binding.target_pd==10);
    // Persisted point between leaf publication and completion: exact retry
    // finishes without re-zeroing a frame that already contains client data.
    uint64_t pending=forward(bell,s->mappings[1].base_va+4096);
    PgFaultResult claimed=pager_fault_claim(&bell->fault,14,pending);assert(!claimed.status);
    PgBindingResult checked=pager_mapping_validate(s->mappings[1],s->pool,14,(PgFaultView){bell->task_id,pending,10,14,s->mappings[1].base_va+4096,6},1,6);assert(!checked.status);
    assert(!pager_ledger_install(&s->ledger,checked.value,4));
    *(volatile uint64_t *)(uintptr_t)s->leaves[1][1]=expected_leaf(checked.value);((uint64_t *)phys_to_virt(s->physical+4096))[0]=0xfeed;
    before=host_invalidations;status(invoke(pager,56,37,34,1,pending,6),PG_OK);
    assert(host_invalidations==before && s->ledger.entries[1].finalized && ((uint64_t *)phys_to_virt(s->physical+4096))[0]==0xfeed);drain();
    status(invoke(pager,61,37,34,0,0,0),PG_LIVE);
    // An authenticated query outside the grant still allows a policy rejection.
    uint64_t outside=forward(bell,s->mappings[1].base_va+9*4096);
    f=invoke(pager,57,37,outside,0,0,0);assert(!f.rax && f.rsi==s->mappings[1].base_va+9*4096);
    status(invoke(pager,56,37,34,2,outside,6),PG_DENIED);status(invoke(pager,58,37,outside,0,0,0),PG_OK);assert(bell->task_state==TASK_EXITED);
    before=host_invalidations;
    uint32_t inbox=pager->ring.count;kernel.current=bell->id;kernel_task_poll();assert(pager->ring.count==inbox);
    kernel.current=pager->id;kernel_task_poll();assert(pager->ring.count==inbox+1);
    kernel_task_poll();assert(pager->ring.count==inbox+1 && bell->exit_notice);
    bool exit_seen=false;for(uint32_t i=0;i<pager->ring.count;++i) {
        RingEntry m=pager->ring.entries[(pager->ring.head+i)%KERNEL_RING_CAPACITY];
        if(m.type_id==0x105 && m.arg0==10){assert(!m.caller && !m.caller_task && m.arg1==bell->task_id && !m.arg2);exit_seen=true;}
    }assert(exit_seen);
    kernel.current=bell->id;uint64_t bitmap=0,task=0;assert(native_cleanup(pager,37,34,&bitmap,&task)==PG_RETRY);
    kernel.current=pager->id;
    *(volatile uint64_t *)(uintptr_t)s->leaves[1][7]=PTE_PRESENT;
    status(invoke(pager,61,37,34,0,0,0),PG_DENIED);assert(host_invalidations==before && !bell->fault.retired && s->ledger.entries[0].present && s->ledger.entries[1].present);
    *(volatile uint64_t *)(uintptr_t)s->leaves[1][7]=0;
    *(volatile uint64_t *)(uintptr_t)s->leaves[1][1]^=PTE_WRITABLE;
    status(invoke(pager,61,37,34,0,0,0),PG_DENIED);assert(host_invalidations==before && *(volatile uint64_t *)(uintptr_t)s->leaves[1][0]);
    *(volatile uint64_t *)(uintptr_t)s->leaves[1][1]^=PTE_WRITABLE;
    // Closure must reclaim an unfinished published binding too.
    s->ledger.entries[1].finalized=false;
    epoch=kernel.epoch;epoch_events=kernel.epoch_event_count;
    kernel.epoch_event_count=KERNEL_EPOCH_EVENT_CAPACITY;
    status(invoke(pager,61,37,34,0,0,0),PG_EXHAUSTED);
    assert(!bell->fault.retired && s->ledger.entries[0].present && s->ledger.entries[1].present &&
           *(volatile uint64_t *)(uintptr_t)s->leaves[1][0] && *(volatile uint64_t *)(uintptr_t)s->leaves[1][1]);
    kernel.epoch_event_count=epoch_events;
    effect_phase=3;effect_target=bell;
    f=invoke(pager,61,37,34,0,0,0);effect_phase=0;assert(!f.rax && f.rsi==3 && f.rdx==bell->task_id && !(f.r10|f.r8));
    assert(host_invalidations==before+2 && bell->fault.retired && s->ledger.closed[1].present &&
           !s->ledger.entries[0].present && !s->ledger.entries[1].present &&
           !*(volatile uint64_t *)(uintptr_t)s->leaves[1][0] && !*(volatile uint64_t *)(uintptr_t)s->leaves[1][1]);
    assert(kernel.epoch==epoch+1 && kernel.epoch_event_count==epoch_events+1);
    KernelEpochEvent cleaned=kernel.epoch_events[epoch_events];
    assert(cleaned.epoch==epoch+1 && cleaned.kind==KERNEL_EPOCH_PAGE_CLEANUP &&
           cleaned.subject==bell->id && cleaned.address==s->mappings[1].base_va &&
           cleaned.token==bell->task_id && cleaned.revision==bell->task_id &&
           cleaned.bitmap==3 && cleaned.key==bell->key);
    assert(kernel_paging_cleanup_complete(10) && !kernel_paging_cleanup_complete(15));
    memset(phys_to_virt(s->physical),0,4096);
    uint64_t fresh=forward(browser,page+4096);status(invoke(pager,56,36,34,0,fresh,6),PG_OK);drain();
    uint64_t fresh_leaf=*(volatile uint64_t *)(uintptr_t)s->leaves[0][1];before=host_invalidations;
    epoch=kernel.epoch;epoch_events=kernel.epoch_event_count;
    f=invoke(pager,61,37,34,0,0,0);assert(!f.rax && f.rsi==3 && f.rdx==bell->task_id);
    assert(host_invalidations==before && kernel.epoch==epoch && kernel.epoch_event_count==epoch_events &&
           *(volatile uint64_t *)(uintptr_t)s->leaves[0][1]==fresh_leaf && s->ledger.entries[0].binding.target_pd==15);
    // Wake/park: publication keeps the payload, and queues each waiter once.
    clear_inbox(browser);clear_inbox(pager);browser->task_state=TASK_RUNNING;kernel.current=15;pager->task_state=TASK_READY;run_push(14);
    TrapFrame *saved=browser->frame;assert(kernel_task_block(browser,saved,0)==pager->frame && browser->task_state==TASK_WAIT_IO);
    assert(domain_enqueue(browser,(RingEntry){.type_id=0x600,.arg0=99}));assert(browser->task_state==TASK_READY && browser->queued);
    count=kernel.run_count;kernel_task_wake(browser);assert(kernel.run_count==count && browser->ring.count==1);
    assert(run_pop()==15);assert(switch_to(15)==saved);assert(kernel_task_block(browser,saved,0)==saved && browser->ring.count==1);
    clear_inbox(browser);pager->task_state=TASK_READY;run_push(14);kernel.ticks=100;
    assert(kernel_task_block(browser,saved,3)==pager->frame && browser->task_state==TASK_WAIT_IO);
    kernel.ticks=102;kernel_task_poll();assert(browser->task_state==TASK_WAIT_IO);
    kernel.ticks=103;kernel_task_poll();assert(browser->task_state==TASK_READY && browser->wait_deadline==0 && browser->frame->rsi==1);
    count=kernel.run_count;kernel_task_poll();assert(kernel.run_count==count);drain();
    browser->task_state=TASK_RUNNING;kernel.current=15;browser->wake_pending=false;kernel.ticks=UINT64_MAX;
    assert(kernel_task_block(browser,saved,1)==saved && saved->rax==ERR_CAP_INVALID && browser->task_state==TASK_RUNNING);
    puts("native paging/wake controls passed:held grants,zeroing,exact retries,detach-before-reuse,atomic cleanup,frame custody,wake isolation");
    free(ram);return 0;
}
