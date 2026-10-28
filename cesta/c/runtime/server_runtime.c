#include "sexos_server.h"

/* pdx_try_listen_raw in crates/sex-pdx: a listen that finds nothing yields
 * once, so other protection domains run even without a preemptive timer. */
bool sexos_try_listen(uint64_t slot, SexosMessage *out)
{
    SexosMessage message = sexos_listen_once(slot);
    if (!message.type_id) {
        sexos_yield();
        return false;
    }
    *out = message;
    return true;
}

/* The bump heap of sex-rt: syscalls 31 and 30 supply each mapping, the limit
 * follows the newest one and the top starts at the first. Servers translated
 * so far allocate nothing; heap_init still reserves 64 KiB as sex-rt does. */
static uintptr_t heap_top;
static uintptr_t heap_limit;

static bool expand_heap(size_t needed_size)
{
    size_t size_aligned = (needed_size + 4095) & ~(size_t)4095;
    uint64_t pfn = sexos_allocate_memory(size_aligned);
    if (pfn == UINT64_MAX) return false;
    uint64_t vaddr = sexos_map_memory(pfn, size_aligned);
    if (vaddr == UINT64_MAX) return false;
    heap_limit = (uintptr_t)vaddr + size_aligned;
    if (heap_top == 0) heap_top = (uintptr_t)vaddr;
    return true;
}

void sexos_heap_init(void)
{
    (void)expand_heap(65536);
}
