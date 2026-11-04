#include "sexos_kernel.h"
/*BOOT reservations and per-holder aliases. A failed construction must stop BOOT before task admission; these helpers do not promise rollback of already-created page tables. Runtime queries use the immutable grants and never invoke this layer.*/

static bool boot_size(uint64_t length)
{
    return length && !(length % KERNEL_PAGE_SIZE) && length <= (KERNEL_PAGE_SIZE << 18);
}
static bool backing_id_free(uint32_t id)
{
    if (!id || kernel.boot_backing_count >= KERNEL_BOOT_REGIONS) return false;
    for (uint32_t i=0;i<kernel.boot_backing_count;++i)
        if (kernel.boot_backings[i].region_id==id) return false;
    return true;
}
bool boot_backing_reserve(uint32_t id, uint64_t length, BootBacking *out)
{
    if (kernel.genesis_sealed || !boot_size(length) || !backing_id_free(id)) return false;
    uint32_t order=0;
    while ((KERNEL_PAGE_SIZE << order)<length) ++order;
    uint64_t physical=frames_alloc(order), reserved=KERNEL_PAGE_SIZE << order;
    if (physical==NO_ADDRESS) return false;
    BootBacking b={.region_id=id,.physical=physical,.length=length,.reserved=reserved};
    kernel.boot_backings[kernel.boot_backing_count++]=b;
    *out=b;
    return true;
}
static bool backing_is_held(const BootBacking *b)
{
    for (uint32_t i=0;i<kernel.boot_backing_count;++i) {
        BootBacking held=kernel.boot_backings[i];
        if (held.region_id==b->region_id && held.physical==b->physical &&
            held.length==b->length && held.reserved==b->reserved) return true;
    }
    return false;
}
static BootBacking *backing_record(uint32_t id)
{
    for (uint32_t i=0;i<kernel.boot_backing_count;++i)
        if (kernel.boot_backings[i].region_id==id) return &kernel.boot_backings[i];
    return NULL;
}
bool boot_region_map(uint32_t holder, const BootBacking *b, bool writable, uint64_t *out)
{
    if (kernel.genesis_sealed || !backing_is_held(b)) return false;
    Domain *d=domain_get(holder);
    if (!d || !d->key || d->key>=16 || !boot_size(b->length) ||
        b->physical % KERNEL_PAGE_SIZE || b->physical >= REGION_PHYSICAL_END ||
        b->length > REGION_PHYSICAL_END-b->physical) return false;
    uint64_t va=va_alloc(b->length);
    if (va==NO_ADDRESS) return false;
    uint64_t flags=PTE_USER|PTE_NO_EXECUTE;
    if (writable) flags|=PTE_WRITABLE;
    for (uint64_t offset=0;offset<b->length;offset+=KERNEL_PAGE_SIZE)
        if (!map_page(va+offset,b->physical+offset,flags,d->key)) return false;
    *out=va;
    return true;
}
static bool boot_slot_free(Domain *d, uint64_t slot)
{
    return d && slot && slot<KERNEL_CAP_SLOTS && !cap_get(d,slot);
}
static bool boot_region_publish(Domain *d, uint64_t slot, MemoryRegionGrant grant)
{
    MemoryRegionCapability capability={true,grant};
    if (sexos_region_info(&capability).error) return false;
    return cap_set(d,slot,(Cap){.kind=CAP_REGION,.region=capability});
}
bool boot_region_dma(uint32_t holder, uint64_t slot, uint32_t id, uint64_t length)
{
    Domain *d=domain_get(holder);
    if (kernel.genesis_sealed || !boot_slot_free(d,slot) || !d->key || d->key>=16 || !cap_boot_available(1) ||
        !boot_size(length) || !backing_id_free(id)) return false;
    BootBacking backing={0};
    uint64_t va=0;
    if (!boot_backing_reserve(id,length,&backing) || !boot_region_map(holder,&backing,true,&va)) return false;
    BootBacking *held=backing_record(id);
    if (!held) return false;
    held->aliases[0]=(BootRegionAlias){holder,va,d->key,true};held->alias_count=1;
    MemoryRegionGrant grant={.region_id=id,.virtual_base=va,.length=length,
        .rights=REGION_QUERY|REGION_READ|REGION_WRITE|REGION_DMA,
        .has_dma=true,.dma_base=backing.physical};
    return boot_region_publish(d,slot,grant);
}
bool boot_region_pair(uint32_t first, uint32_t second, uint64_t first_slot, uint64_t second_slot,
                      uint32_t id, uint64_t length, bool second_writable)
{
    Domain *a=domain_get(first), *b=domain_get(second);
    if (kernel.genesis_sealed || first==second || !boot_slot_free(a,first_slot) ||
        !boot_slot_free(b,second_slot) || !cap_boot_available(2) ||
        !a->key || !b->key || a->key>=16 || b->key>=16 || a->key==b->key ||
        !boot_size(length) || !backing_id_free(id)) return false;
    BootBacking backing={0}; uint64_t av=0,bv=0;
    if (!boot_backing_reserve(id,length,&backing) || !boot_region_map(first,&backing,true,&av) ||
        !boot_region_map(second,&backing,false,&bv)) return false;
    BootBacking *held=backing_record(id);
    if (!held) return false;
    held->aliases[0]=(BootRegionAlias){first,av,a->key,true};
    held->aliases[1]=(BootRegionAlias){second,bv,b->key,second_writable};
    held->alias_count=2;
    if (second_writable) held->writer=(RegionWriterState){true,id,first,second,first,1,0};
    MemoryRegionGrant producer={.region_id=id,.virtual_base=av,.length=length,
        .rights=REGION_QUERY|REGION_READ|REGION_WRITE,.has_peer=true,.peer_pd=second};
    MemoryRegionGrant consumer={.region_id=id,.virtual_base=bv,.length=length,
        .rights=(uint8_t)(REGION_QUERY|REGION_READ|(second_writable ? REGION_WRITE : 0)),
        .has_peer=true,.peer_pd=first};
    return boot_region_publish(a,first_slot,producer) && boot_region_publish(b,second_slot,consumer);
}


