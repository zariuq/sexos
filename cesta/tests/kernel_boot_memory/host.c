/* Production BOOT constructors and actual allocator/page walks on RAM.
 * Only CPU/TLB effects are providers. Permission checks inspect actual PTEs. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_domains.c"
#include "kernel_boot_memory.c"
#include "kernel_region_writer.c"

unsigned host_invalidations;
void klog(const char *format, ...) { (void)format; }
_Noreturn void kernel_halt(const char *reason) { fprintf(stderr,"%s\n",reason); abort(); }

static Domain *holder(uint32_t id, uint8_t key)
{
    Domain *d=domain_boot_record(id); assert(d);
    d->id=id; d->live=true; d->key=key; d->mask=mask_for_key(key);
    return d;
}

static uint64_t leaf(uint64_t va)
{
    uint64_t pte=0; assert(read_pte(va,&pte)); return pte;
}

static void header_bounds(void)
{
    uint8_t image[128]={0};
    ElfHeader h={.phoff=1,.phentsize=sizeof(ProgramHeader),.phnum=1};
    ProgramHeader expected={.type=PT_LOAD,.vaddr=0x400000,.memsz=4096}, out={0};
    memcpy(image+1,&expected,sizeof expected);
    assert(program_header(image,sizeof image,&h,0,&out) && memcmp(&out,&expected,sizeof out)==0);
    const volatile uint64_t offsets[]={UINT64_MAX,UINT64_MAX-31,129,128,128-sizeof(ProgramHeader)+1};
    for (unsigned i=0;i<sizeof offsets/sizeof *offsets;++i) {
        h.phoff=offsets[i]; out.type=77;
        assert(!program_header(image,sizeof image,&h,0,&out) && out.type==77);
    }
    h.phoff=1;
    assert(!program_header(image,sizeof image,&h,1,&out));
    assert(!program_header(image,sizeof image,&h,UINT64_MAX,&out));
    h.phentsize=sizeof(ProgramHeader)-1;
    assert(!program_header(image,sizeof image,&h,0,&out));
    h.phentsize=0;
    assert(!program_header(image,sizeof image,&h,0,&out));
}

int main(void)
{
    header_bounds();
    enum { BYTES=16*1024*1024 };
    unsigned char *ram=aligned_alloc(4096,BYTES); assert(ram);
    memset(ram,0xa5,BYTES);
    hhdm_offset=(uint64_t)(uintptr_t)ram;
    memset(ram+0x1000,0,4096);
    LimineMemmapEntry usable={.base=0x2000,.length=BYTES-0x2000,.type=0};
    LimineMemmapEntry *entries[]={&usable};
    LimineMemmapResponse memory={.entry_count=1,.entries=entries};
    memory_init(&memory);
    Domain *a=holder(2,2),*b=holder(1,1),*pager=holder(14,14);
    for (unsigned key=1;key<16;++key)
        assert(mask_for_key((uint8_t)key)==(UINT32_MAX & ~(UINT32_C(3)<<(2*key))));
    assert(mask_for_key(0)==UINT32_MAX && mask_for_key(16)==UINT32_MAX);
    assert(boot_region_pair(a->id,b->id,33,33,5,3*4096,false));
    const Cap *producer=cap_get(a,33),*consumer=cap_get(b,33);
    assert(producer && consumer && producer->kind==CAP_REGION && consumer->kind==CAP_REGION);
    const MemoryRegionGrant *pa=&producer->region.grant,*pb=&consumer->region.grant;
    assert(pa->region_id==pb->region_id && pa->region_id==5 && pa->length==3*4096);
    assert(pa->virtual_base!=pb->virtual_base && pa->rights==7 && pb->rights==3);
    assert(pa->has_peer && pa->peer_pd==b->id && pb->has_peer && pb->peer_pd==a->id);
    assert(!pa->has_dma && !pb->has_dma);
    assert(kernel.boot_backing_count==1 && kernel.boot_backings[0].reserved==4*4096);
    BootBacking backing=kernel.boot_backings[0];
    for (uint64_t i=0;i<backing.reserved;++i) assert(ram[backing.physical+i]==0);
    for (uint64_t offset=0;offset<3*4096;offset+=4096) {
        uint64_t ap=leaf(pa->virtual_base+offset),bp=leaf(pb->virtual_base+offset);
        assert((ap&PTE_ADDRESS)==backing.physical+offset && (ap&PTE_ADDRESS)==(bp&PTE_ADDRESS));
        assert((ap&(PTE_USER|PTE_WRITABLE|PTE_NO_EXECUTE))==(PTE_USER|PTE_WRITABLE|PTE_NO_EXECUTE));
        assert((bp&(PTE_USER|PTE_WRITABLE|PTE_NO_EXECUTE))==(PTE_USER|PTE_NO_EXECUTE));
        assert(((ap>>PTE_KEY_SHIFT)&15)==a->key && ((bp>>PTE_KEY_SHIFT)&15)==b->key);
        ram[(ap&PTE_ADDRESS)+23]=(unsigned char)(offset/4096+19);
        assert(ram[(bp&PTE_ADDRESS)+23]==(unsigned char)(offset/4096+19));
        assert(user_range_ok(pa->virtual_base+offset,4096,true,a->mask));
        assert(!user_range_ok(pa->virtual_base+offset,4096,false,b->mask));
        assert(user_range_ok(pb->virtual_base+offset,4096,false,b->mask));
        assert(!user_range_ok(pb->virtual_base+offset,4096,true,b->mask));
        assert(!user_range_ok(pb->virtual_base+offset,4096,false,pager->mask));
    }
    uint64_t spare=0;
    assert(!read_pte(pb->virtual_base+3*4096,&spare));
    assert(boot_region_pair(b->id,a->id,64,64,6,4096,true));
    const Cap *mutual=cap_get(a,64),*mutual_home=cap_get(b,64); assert(mutual && mutual_home);
    BootBacking *leased=backing_record(6);assert(leased && leased->alias_count==2);
    assert(region_writer_valid(&leased->writer) && leased->writer.home==b->id &&
           leased->writer.peer==a->id && leased->writer.current==b->id &&
           leased->writer.revision==1 && !leased->writer.job);
    assert(!user_range_ok(mutual->region.grant.virtual_base,4096,true,a->mask));
    assert(user_range_ok(mutual_home->region.grant.virtual_base,4096,true,b->mask));
    assert(boot_region_dma(pager->id,32,1,5*4096));
    const Cap *dma=cap_get(pager,32); assert(dma && dma->region.grant.has_dma);
    assert(dma->region.grant.rights==15 && !dma->region.grant.has_peer);
    assert((leaf(dma->region.grant.virtual_base)&PTE_ADDRESS)==dma->region.grant.dma_base);
    MemoryRegionReply view=sexos_region_query(&dma->region,54,31,17,REGION_QUERY|REGION_DMA);
    assert(view.status==0 && view.value2==dma->region.grant.dma_base+31);
    view=sexos_region_query(&producer->region,54,0,1,REGION_QUERY|REGION_DMA);
    assert(view.status==UINT64_MAX && !(view.value0|view.value1|view.value2|view.value3));

    uint64_t va_before=va_alloc(0); uint32_t roots_before=kernel.boot_backing_count;
    uint32_t caps_before=kernel.cap_storage.count;
    assert(!boot_region_pair(a->id,a->id,70,71,10,4096,false));
    assert(!boot_region_pair(a->id,999,70,71,10,4096,false));
    assert(!boot_region_pair(a->id,b->id,33,71,10,4096,false));
    assert(!boot_region_pair(a->id,b->id,70,71,5,4096,false));
    assert(!boot_region_dma(a->id,70,10,0));
    assert(!boot_region_dma(a->id,70,10,4097));
    assert(!boot_region_dma(a->id,1025,10,4096));
    uint8_t old_key=b->key; b->key=a->key;
    assert(!boot_region_pair(a->id,b->id,70,71,10,4096,false)); b->key=old_key;
    BootBacking invented=backing; invented.physical+=4096;
    uint64_t output=UINT64_MAX;
    assert(!boot_region_map(a->id,&invented,true,&output) && output==UINT64_MAX);
    assert(kernel.boot_backing_count==roots_before && kernel.cap_storage.count==caps_before && va_alloc(0)==va_before);
    kernel_seal_genesis();
    assert(!boot_region_pair(a->id,b->id,70,71,10,4096,false));
    assert(!boot_region_dma(a->id,70,10,4096));
    assert(!boot_backing_reserve(10,4096,&invented));
    assert(!boot_region_map(a->id,&backing,true,&output));
    assert(kernel.boot_backing_count==roots_before && kernel.cap_storage.count==caps_before && va_alloc(0)==va_before);
    assert(cap_get(a,33)==producer && cap_get(b,33)==consumer);
    b->task_id=22;b->task_state=TASK_RUNNING;kernel.current=b->id;
    TrapFrame query={.rdi=64};kernel_region_writer_syscall(b,&query);
    assert(query.rax==REGION_WRITER_OK && query.rsi==1 && query.rdx==b->id && !query.r10 && query.r8==a->id);
    TrapFrame offered={.rdi=64,.rsi=REGION_WRITER_OFFER,.rdx=1,.r10=99};
    kernel_region_writer_syscall(b,&offered);
    assert(offered.rax==REGION_WRITER_OK && offered.rsi==2 && offered.rdx==b->id && offered.r10==99);
    assert(user_range_ok(mutual_home->region.grant.virtual_base,4096,true,b->mask));
    assert(!user_range_ok(mutual->region.grant.virtual_base,4096,true,a->mask));
    assert(kernel.epoch==1 && kernel.epoch_event_count==1 &&
           kernel.epoch_events[0].kind==KERNEL_EPOCH_REGION_WRITER &&
           kernel.epoch_events[0].subject==b->id && kernel.epoch_events[0].peer==a->id &&
           kernel.epoch_events[0].rights==REGION_WRITER_OFFER &&
           kernel.epoch_events[0].address==6 && kernel.epoch_events[0].revision==2 &&
           kernel.epoch_events[0].token==99);
    a->task_id=11;a->task_state=TASK_RUNNING;b->task_state=TASK_READY;kernel.current=a->id;
    TrapFrame claimed={.rdi=64,.rsi=REGION_WRITER_CLAIM,.rdx=2,.r10=99};
    kernel_region_writer_syscall(a,&claimed);
    assert(claimed.rax==REGION_WRITER_OK && claimed.rsi==3 && claimed.rdx==a->id && claimed.r10==99);
    assert(!user_range_ok(mutual_home->region.grant.virtual_base,4096,true,b->mask));
    assert(user_range_ok(mutual->region.grant.virtual_base,4096,true,a->mask));
    assert(kernel.epoch==2 && kernel.epoch_event_count==2 &&
           kernel.epoch_events[1].subject==a->id && kernel.epoch_events[1].peer==b->id &&
           kernel.epoch_events[1].rights==REGION_WRITER_CLAIM);
    TrapFrame stale={.rdi=64,.rsi=REGION_WRITER_RELEASE,.rdx=1,.r10=99};
    kernel_region_writer_syscall(a,&stale);assert(stale.rax==REGION_WRITER_STALE && stale.rsi==3);
    TrapFrame wrong_job={.rdi=64,.rsi=REGION_WRITER_RELEASE,.rdx=3,.r10=100};
    kernel_region_writer_syscall(a,&wrong_job);assert(wrong_job.rax==REGION_WRITER_BUSY && wrong_job.rsi==3);
    TrapFrame returned={.rdi=64,.rsi=REGION_WRITER_RELEASE,.rdx=3,.r10=99};
    kernel_region_writer_syscall(a,&returned);
    assert(returned.rax==REGION_WRITER_OK && returned.rsi==4 && returned.rdx==b->id && !returned.r10);
    assert(user_range_ok(mutual_home->region.grant.virtual_base,4096,true,b->mask));
    assert(!user_range_ok(mutual->region.grant.virtual_base,4096,true,a->mask));
    assert(kernel.epoch==3 && kernel.epoch_event_count==3 && kernel.epoch_events[2].subject==a->id &&
           kernel.epoch_events[2].peer==b->id && kernel.epoch_events[2].rights==REGION_WRITER_RELEASE &&
           kernel.epoch_events[2].token==99);
    b->task_state=TASK_RUNNING;a->task_state=TASK_READY;kernel.current=b->id;
    TrapFrame offer_cancel={.rdi=64,.rsi=REGION_WRITER_OFFER,.rdx=4,.r10=100};
    kernel_region_writer_syscall(b,&offer_cancel);assert(offer_cancel.rax==REGION_WRITER_OK && offer_cancel.rsi==5);
    TrapFrame cancelled={.rdi=64,.rsi=REGION_WRITER_CANCEL,.rdx=5,.r10=100};
    kernel_region_writer_syscall(b,&cancelled);
    assert(cancelled.rax==REGION_WRITER_OK && cancelled.rsi==6 && cancelled.rdx==b->id && !cancelled.r10);
    TrapFrame death_offer={.rdi=64,.rsi=REGION_WRITER_OFFER,.rdx=6,.r10=101};
    kernel_region_writer_syscall(b,&death_offer);assert(death_offer.rax==REGION_WRITER_OK && death_offer.rsi==7);
    a->task_state=TASK_RUNNING;b->task_state=TASK_READY;kernel.current=a->id;
    TrapFrame death_claim={.rdi=64,.rsi=REGION_WRITER_CLAIM,.rdx=7,.r10=101};
    kernel_region_writer_syscall(a,&death_claim);assert(death_claim.rax==REGION_WRITER_OK && death_claim.rsi==8);
    assert(user_range_ok(mutual->region.grant.virtual_base,4096,true,a->mask));
    b->task_state=TASK_RUNNING;a->task_state=TASK_READY;kernel.current=b->id;
    TrapFrame live_recovery={.rdi=64,.rsi=REGION_WRITER_RECOVER,.rdx=8,.r10=101};
    kernel_region_writer_syscall(b,&live_recovery);
    assert(live_recovery.rax==REGION_WRITER_DENIED && leased->writer.current==a->id && leased->writer.revision==8);
    a->task_state=TASK_EXITED;a->reply_rights[b->id]=1;
    b->task_state=TASK_WAIT_IO;b->queued=false;b->reply_ready=false;kernel.current=0;
    Cap *mutable_home=(Cap *)(uintptr_t)mutual_home;mutable_home->region.grant.has_dma=true;
    assert(!kernel_region_writer_recover_dead(a) && leased->writer.current==a->id && a->reply_rights[b->id]==1);
    mutable_home->region.grant.has_dma=false;
    assert(kernel_region_writer_recover_dead(a));
    assert(leased->writer.current==b->id && leased->writer.revision==9 && !leased->writer.job);
    assert(user_range_ok(mutual_home->region.grant.virtual_base,4096,true,b->mask));
    assert(!user_range_ok(mutual->region.grant.virtual_base,4096,true,a->mask));
    assert(b->reply_ready && b->reply_value==ERR_SERVICE_NOT_READY && b->reply_caller==a->id &&
           b->reply_task==a->task_id && !a->reply_rights[b->id] && b->task_state==TASK_READY && b->queued);
    assert(kernel.epoch_events[7].rights==REGION_WRITER_RECOVER &&
           kernel.epoch_events[7].subject==b->id && kernel.epoch_events[7].peer==a->id);
    b->queued=false;b->reply_ready=false;b->task_state=TASK_RUNNING;kernel.current=b->id;
    kernel.epoch_event_count=KERNEL_EPOCH_EVENT_CAPACITY;
    TrapFrame exhausted={.rdi=64,.rsi=REGION_WRITER_OFFER,.rdx=9,.r10=102};kernel_region_writer_syscall(b,&exhausted);
    assert(exhausted.rax==REGION_WRITER_EXHAUSTED && leased->writer.current==b->id && leased->writer.revision==9);
    assert(user_range_ok(mutual_home->region.grant.virtual_base,4096,true,b->mask));
    assert(!user_range_ok(mutual->region.grant.virtual_base,4096,true,a->mask));
    assert(host_invalidations>0);
    free(ram);
    puts("BOOT backing controls passed: zeroed rounding, distinct aliases, holder keys, exclusive writer transfer, NX, DMA policy, sealed constructors");
    return 0;
}
