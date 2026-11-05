/* Actual frame allocator and page-table walks on a deterministic RAM provider.
 * CPU/TLB effects are providers; this does not test hardware or DMA semantics. */
#include "sexos_kernel.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Kernel kernel;
unsigned host_invalidations;
void klog(const char *format, ...) { (void)format; }

static uint64_t *table(uint64_t physical)
{
    return phys_to_virt(physical & PTE_ADDRESS);
}

int main(void)
{
    enum { BYTES = 256 * 4096 };
    unsigned char *ram = aligned_alloc(4096, BYTES);
    assert(ram);
    memset(ram, 0xA5, BYTES);
    hhdm_offset = (uint64_t)(uintptr_t)ram;
    memset(table(0x1000), 0, 4096);
    LimineMemmapEntry usable = {.base = 0x2000, .length = BYTES - 0x2000, .type = 0};
    LimineMemmapEntry *entries[] = {&usable};
    LimineMemmapResponse memory = {.entry_count = 1, .entries = entries};
    memory_init(&memory);
    assert(frames_alloc(52) == NO_ADDRESS);
    assert(frames_alloc(18) == NO_ADDRESS);
    uint64_t frame = frames_alloc(0);
    assert(frame == 0x2000);
    for (size_t i = 0; i < 4096; ++i) assert(ram[frame + i] == 0);
    /* A reservation names at least one page; refused requests leave the cursor. */
    assert(va_alloc(0) == NO_ADDRESS);
    uint64_t first = va_alloc(1);
    assert(first != NO_ADDRESS && va_alloc(UINT64_MAX) == NO_ADDRESS && va_alloc(0) == NO_ADDRESS);
    assert(va_alloc(0x700000000000ULL - (first + 4096) + 1) == NO_ADDRESS);
    assert(va_alloc(4097) == first + 4096 && va_alloc(1) == first + 3 * 4096);
    uint64_t va = UINT64_C(0x400000000000), pte = 0;
    uint64_t flags = PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE;
    assert(map_page(va, frame, flags, 4));
    assert(read_pte(va, &pte) && (pte & PTE_ADDRESS) == frame);
    assert((pte & flags) == flags && ((pte >> PTE_KEY_SHIFT) & 15) == 4);
    uint64_t before = pte;
    assert(!map_page(va, frame, flags, 5) && read_pte(va, &pte) && pte == before);
    uint32_t mask = UINT32_MAX & ~(UINT32_C(3) << 8);
    assert(user_range_ok(va, 4096, true, mask));
    assert(!user_range_ok(va, 4096, false, UINT32_MAX));
    assert(!user_range_ok(va + 4095, 2, false, mask));
    assert(!user_range_ok(UINT64_MAX, 2, false, mask));
    assert(!user_range_ok(UINT64_C(0x7fffffffffff), 2, false, mask));
    uint64_t second = frames_alloc(0);
    assert(map_page(va + 4096, second, PTE_USER | PTE_NO_EXECUTE, 4));
    assert(user_range_ok(va + 4095, 2, false, mask));
    assert(!user_range_ok(va + 4095, 2, true, mask));
    assert(map_page(va + 8192, frames_alloc(0), flags, 5));
    assert(!user_range_ok(va + 8192, 1, false, mask));

    /* Split an executable supervisor 1-GiB mapping. Only one requested 4-KiB
     * page gains user access; it must also become non-executable. */
    uint64_t high = UINT64_C(0xffff800000000000);
    uint64_t directory = frames_alloc(0);
    table(0x1000)[(high >> 39) & 511] = directory | PTE_PRESENT | PTE_WRITABLE;
    table(directory)[(high >> 30) & 511] = UINT64_C(0x80000000) | PTE_PRESENT | PTE_WRITABLE | PTE_HUGE;
    uint64_t exposed = high + 0x345000;
    assert(expose_to_user(exposed + 12, 3000, 1));
    assert(read_pte(exposed, &pte));
    assert((pte & (PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE)) == (PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE));
    assert((pte & PTE_ADDRESS) == UINT64_C(0x80345000) && ((pte >> PTE_KEY_SHIFT) & 15) == 1);
    assert(read_pte(exposed - 4096, &pte) && (pte & PTE_USER) == 0);
    assert(read_pte(exposed + 4096, &pte) && (pte & PTE_USER) == 0);
    assert(host_invalidations >= 4);
    unsigned allocated = 0;
    while (frames_alloc(0) != NO_ADDRESS) assert(++allocated < 256);
    assert(allocated > 0 && frames_alloc(0) == NO_ADDRESS);
    assert(!map_page(va + (UINT64_C(1) << 30), frame, flags, 4));
    free(ram);
    puts("kernel memory provider controls passed");
    return 0;
}
