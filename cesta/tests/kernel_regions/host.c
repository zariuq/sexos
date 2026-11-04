/* The actual kernel dispatch and sparse BOOT stores; hardware effects are the
 * inherited providers, whose counters must not change during region queries. */
#define main historical_trace_main
#include "../kernel_model/host.c"
#undef main

_Static_assert(SEXOS_KERNEL_CURRENT_PDX == 1, "current memory-authority profile");

static Domain *make_domain(uint32_t id, uint8_t key)
{
    Domain *d=domain_boot_record(id);
    assert(d);
    d->live=true; d->id=id; d->key=key; d->mask=mask_for_key(key);
    d->task_id=7000+id; d->task_state=TASK_READY;
    return d;
}

static TrapFrame query(Domain *d, uint64_t number, uint64_t slot, uint64_t offset,
                       uint64_t length, uint64_t rights)
{
    kernel.current=d->id; d->task_state=TASK_RUNNING;
    TrapFrame f={.rax=number,.rdi=slot,.rsi=offset,.rdx=length,.r10=rights,.r8=UINT64_MAX};
    assert(kernel_syscall(&f)==&f);
    return f;
}

static void denied(TrapFrame f)
{
    assert(f.rax==UINT64_MAX && !(f.rsi|f.rdx|f.r10|f.r8));
}

static TrapFrame bridge(Domain *d,uint64_t op,uint64_t kind,uint64_t reserved0,uint64_t reserved1)
{
    kernel.current=d->id; d->task_state=TASK_RUNNING;
    TrapFrame f={.rax=0,.rdi=0,.rsi=op,.rdx=kind,.r10=reserved0,.r8=reserved1};
    assert(kernel_syscall(&f)==&f); return f;
}

static void views(void)
{
    memset(&kernel,0,sizeof kernel);
    fresh=0; mapping_count=0;
    Domain *a=make_domain(1,1), *b=make_domain(1023,3), *stranger=make_domain(3,5);
    assert(a!=b && b->id!=b->storage_index && domain_get(1023)==b);
    assert(!domain_boot_record(1) && domain_get(1024)==NULL && domain_get(UINT32_MAX)==NULL);
    MemoryRegionGrant rw={.region_id=77,.virtual_base=0x400000,.length=8192,
        .rights=REGION_QUERY|REGION_READ|REGION_WRITE,.has_peer=true,.peer_pd=b->id};
    Cap held={.kind=CAP_REGION,.region={true,rw}};
    assert(cap_set(a,1024,held));
    const Cap *saved=cap_get(a,1024);
    held.region.grant.virtual_base=0x123000;
    assert(saved && saved->region.grant.virtual_base==0x400000);
    assert(cap_set(a,5,(Cap){.kind=CAP_DOMAIN,.target=b->id,.region={true,rw}}));
    MemoryRegionGrant dma={.region_id=99,.virtual_base=0x500000,.length=4096,
        .rights=REGION_KNOWN,.has_dma=true,.dma_base=0x123000};
    assert(cap_set(a,34,(Cap){.kind=CAP_REGION,.region={true,dma}}));
    assert(cap_set(a,35,(Cap){.kind=CAP_REGION,.region={false,rw}}));
    assert(cap_set(a,19,(Cap){.kind=CAP_PCI,.region={true,rw}}));
    rw.rights=REGION_QUERY|REGION_READ;
    assert(cap_set(a,36,(Cap){.kind=CAP_REGION,.region={true,rw}}));
    assert(!cap_set(a,1024,held));
    kernel.display_pd=a->id; kernel.shell_pd=b->id;
    kernel.fb_address=0x444000; kernel.fb_width=1280; kernel.fb_height=800; kernel.fb_pitch=5120;
    uint32_t used=kernel.cap_storage.count;
    kernel_seal_genesis();
    assert(!cap_set(a,37,held) && !cap_set(a,1024,held));
    assert(kernel.cap_storage.count==used && cap_get(a,1024)==saved);
    assert(!domain_boot_record(2));
    for (unsigned i=0;i<1000;++i) {
        a->mask=(i&1) ? UINT32_MAX : 0; /* Authority is independent of PKU state. */
        TrapFrame f=query(a,54,1024,40,24,REGION_QUERY|REGION_READ);
        assert(f.rax==0 && f.rsi==0x400028 && f.rdx==24 && f.r10==0 && f.r8==77);
        f=query(a,55,1024,UINT64_MAX,UINT64_MAX,UINT64_MAX);
        assert(f.rax==0 && f.rsi==8192 && f.rdx==7 && f.r10==1023 && f.r8==77);
    }
    TrapFrame f=query(a,54,34,8,16,REGION_QUERY|REGION_DMA);
    assert(f.rax==0 && f.rsi==0x500008 && f.rdx==16 && f.r10==0x123008 && f.r8==99);
    f=query(a,54,34,8,16,REGION_QUERY|REGION_READ);
    assert(f.rax==0 && f.r10==0);
    denied(query(a,54,36,0,16,REGION_QUERY|REGION_WRITE));
    denied(query(a,54,35,0,16,REGION_QUERY));
    denied(query(a,54,5,0,16,REGION_QUERY)); /* Descriptor data is not a region cap. */
    denied(query(b,54,1024,0,16,REGION_QUERY)); /* Another PD's held slot grants nothing. */
    for (uint64_t slot=0;slot<2;++slot) denied(query(a,55,slot,17,18,19));
    for (unsigned i=0;i<3;++i) {
        uint64_t slot=(UINT64_C(1)<<(32+i))|1024;
        denied(query(a,54,slot,17,18,19));
        denied(query(a,55,slot,17,18,19));
    }
    denied(query(a,54,1024,UINT64_MAX,2,REGION_QUERY));
    denied(query(a,54,1024,8191,2,REGION_QUERY));
    denied(query(a,54,1024,0,0,REGION_QUERY));
    denied(query(a,54,1024,0,1,UINT64_C(0x100000003)));
    denied(query(a,54,1024,0,1,REGION_QUERY|REGION_DMA));
    f=query(a,43,19,0,0,0);
    assert(f.rax==0x400000); /* Existing BAR0 alias, no allocation or mapping. */
    assert(query(a,43,19,1,4096,0).rax==UINT64_MAX);
    assert(query(a,43,5,0,4096,0).rax==UINT64_MAX);
    assert(query(a,43,19,0,8193,0).rax==UINT64_MAX);
    assert(query(a,43,(UINT64_C(1)<<32)|19,0,4096,0).rax==UINT64_MAX);
    assert(query(a,30,4096,4096,0,0).rax==UINT64_MAX);
    assert(query(a,31,4096,0,0,0).rax==UINT64_MAX);
    assert(query(a,40,4096,b->id,0,0).rax==0);
    assert(query(a,50,5,4096,17,0).rax==UINT64_MAX);
    assert(query(a,51,34,0,0,0).rax==UINT64_MAX);
    assert(bridge(a,0xE6,0,0,0).rsi==(UINT64_C(800)<<32|1280));
    assert(bridge(b,0xE6,0,0,0).rsi==(UINT64_C(800)<<32|1280));
    assert(bridge(a,0xE6,1,0,0).rsi==kernel.fb_address);
    assert(bridge(a,0xE6,2,0,0).rsi==5120);
    TrapFrame rejected=bridge(b,0xE6,1,0,0);
    assert(rejected.rax==ERR_CAP_INVALID && rejected.rsi==0);
    rejected=bridge(stranger,0xE6,0,0,0);
    assert(rejected.rax==ERR_CAP_INVALID && rejected.rsi==0);
    for (unsigned i=0;i<2;++i) {
        rejected=bridge(a,0xE6,0,i==0,i==1);
        assert(rejected.rax==ERR_CAP_INVALID && rejected.rsi==0);
    }
    rejected=bridge(a,0xE3,0x123456,0,0);
    assert(rejected.rax==ERR_CAP_INVALID && rejected.rsi==0);
    assert(bridge(a,0xBB,0,0,0).rax==ERR_CAP_INVALID);
    assert(query(a,42,0,0,0,0).rax==UINT64_MAX);
    assert(query(a,52,0,0,0,0).rax==UINT64_MAX);
    /* Unknown current operations return a clean five-word failure. The source
     * decoders must not reinterpret input words as a malformed IO result. */
    for(uint64_t number=71;number<=74;++number)
        denied(query(a,number,UINT64_C(1)<<32|60,1,0xdead,0xbeef));
    denied(query(a,0x400,5,1,2,3));
    assert(fresh==0 && mapping_count==0 && kernel.block_count==0 && kernel.shared_count==0);
    assert(kernel.cap_storage.count==used && cap_get(a,1024)==saved);
}

static void budgets(void)
{
    memset(&kernel,0,sizeof kernel);
    Domain *a=make_domain(1,1),*b=make_domain(1023,2);
    for (uint64_t slot=1;slot<KERNEL_CAP_SLOTS;++slot)
        assert(cap_set(a,slot,(Cap){.kind=CAP_DOMAIN,.target=b->id}));
    assert(kernel.cap_storage.count==KERNEL_BOOT_CAPS);
    const Cap *first=cap_get(a,1),*last=cap_get(a,1024);
    assert(first && last && first!=last && first->target==1023 && last->target==1023);
    assert(!cap_set(b,34,(Cap){.kind=CAP_DOMAIN,.target=a->id}));
    assert(!cap_get(b,34) && cap_get(a,1)==first && cap_get(a,1024)==last);
    assert(!cap_set(b,0,(Cap){.kind=CAP_DOMAIN,.target=a->id}));
    assert(!cap_set(b,1025,(Cap){.kind=CAP_DOMAIN,.target=a->id}));
    assert(!cap_get(b,0) && !cap_get(b,1025));
    memset(&kernel,0,sizeof kernel);
    for (uint32_t i=0;i<KERNEL_BOOT_DOMAINS;++i) {
        Domain *d=make_domain(i+1,1);
        assert(d->storage_index==i);
    }
    assert(!domain_boot_record(1023) && kernel.domains.count==KERNEL_BOOT_DOMAINS);
    for (uint32_t i=1;i<=KERNEL_BOOT_DOMAINS;++i) assert(domain_get(i)->id==i);
    assert(!domain_boot_record(0) && !domain_boot_record(1024));
}

int main(void)
{
    views(); budgets();
    puts("kernel region controls passed: held lookup, rights, clean outputs, immutable identity, finite BOOT budgets, zero runtime mapping effects");
    return 0;
}
