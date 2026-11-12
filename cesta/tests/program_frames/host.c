/* Production program-frame leases, exact PTE detachment and RAM reuse controls.
 * CPU/TLB operations are providers; this does not execute user instructions. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define memcpy sexos_memcpy
#define memmove sexos_memmove
#define memset sexos_memset
#define memcmp sexos_memcmp
#include "kernel_memory.c"
#undef memcpy
#undef memmove
#undef memset
#undef memcmp

Kernel kernel;
unsigned host_invalidations;
void klog(const char *format, ...) { (void)format; }

static unsigned char *ram;
static const uint64_t va = UINT64_C(0x400000000000);
static const uint64_t flags = PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE;

static void setup(void)
{
    enum { BYTES = 64 * 4096 };
    memset(ram, 0xa5, BYTES);
    memset(ram + 0x1000, 0, 4096);
    memset(&kernel, 0, sizeof kernel);
    hhdm_offset = (uint64_t)(uintptr_t)ram;
    LimineMemmapEntry entry = {.base=0x2000, .length=BYTES-0x2000, .type=0};
    LimineMemmapEntry *entries[] = {&entry};
    LimineMemmapResponse memory = {.entry_count=1, .entries=entries};
    memory_init(&memory);
    frames.leases.reserve = 2;
    host_invalidations = 0;
    assert(frame_lease_audit(&frames.leases));
}

static size_t core_slot(FrameLease lease)
{
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i) {
        FrameLeaseEntry *entry = &frames.leases.entries[i];
        if (entry->phase != FRAME_LEASE_EMPTY && entry->base == lease.base &&
            entry->order == lease.order && entry->kind == lease.kind &&
            entry->holder == lease.holder && entry->incarnation == lease.incarnation)
            return i;
    }
    return FRAME_LEASE_CAPACITY;
}

static FrameLease take(uint32_t holder)
{
    FrameLeaseResult result = program_frame_take(holder);
    assert(result.status == FRAME_LEASE_OK);
    assert(program_frame_find(result.value) < PROGRAM_FRAME_CAPACITY);
    assert(core_slot(result.value) < FRAME_LEASE_CAPACITY);
    assert(frame_lease_audit(&frames.leases));
    return result.value;
}

static void lifecycle_and_reuse(void)
{
    setup();
    FrameLease a = take(11), b = take(22);
    assert(a.base != b.base && a.incarnation != b.incarnation);
    assert(program_frame_lent() == 2 && program_frame_available() > 0);
    assert(program_frame_map(a, va, flags, 4));
    assert(!program_frame_map(a, va + 4096, flags, 4));
    assert(!program_frame_map(b, va, flags, 4));
    assert(!map_page(va + 8192, a.base, flags, 4));
    uint64_t pte = 0;
    assert(!read_pte(va + 4096, &pte) && !read_pte(va + 8192, &pte));
    assert(read_pte(va, &pte) && (pte & PTE_ADDRESS) == a.base);

    size_t slot = program_frame_find(a), core = core_slot(a);
    ProgramFrameBinding before_binding = program_frames.entries[slot];
    FrameLeaseState before_leases;
    memcpy(&before_leases, &frames.leases, sizeof before_leases);
    unsigned before_invalidations = host_invalidations;
    assert(program_frame_give(a) == FRAME_LEASE_RETRY);
    assert(!memcmp(&before_leases, &frames.leases, sizeof before_leases));
    assert(!memcmp(&before_binding, &program_frames.entries[slot], sizeof before_binding));
    assert(read_pte(va, &pte) && pte == before_binding.expected);
    assert(host_invalidations == before_invalidations);

    FrameLease wrong = a; wrong.holder++;
    assert(program_frame_detach(wrong) == FRAME_LEASE_DENIED);
    wrong = a; wrong.base += 4096;
    assert(program_frame_detach(wrong) == FRAME_LEASE_DENIED);
    assert(read_pte(va, &pte) && pte == before_binding.expected);
    assert(!memcmp(&before_leases, &frames.leases, sizeof before_leases));

    assert(program_frame_detach(a) == FRAME_LEASE_OK);
    assert(host_invalidations > before_invalidations && !read_pte(va, &pte));
    assert(!program_frames.entries[slot].mapped && program_frames.entries[slot].returning);
    assert(frames.leases.entries[core].phase == FRAME_LEASE_DETACHED);
    assert(program_frame_give(a) == FRAME_LEASE_OK);
    assert(!program_frames.entries[slot].present && program_frame_lent() == 1);
    assert(program_frame_give(a) == FRAME_LEASE_DENIED);
    assert(frame_lease_audit(&frames.leases));

    for (size_t i = 0; i < 4096; ++i) ram[a.base + i] = 0xcc;
    FrameLease same_holder = take(11);
    assert(same_holder.base == a.base && same_holder.incarnation != a.incarnation);
    assert(program_frame_find(a) == PROGRAM_FRAME_CAPACITY);
    assert(program_frame_detach(a) == FRAME_LEASE_DENIED);
    assert(program_frame_give(a) == FRAME_LEASE_DENIED);
    for (size_t i = 0; i < 4096; ++i) assert(ram[same_holder.base + i] == 0);
    assert(program_frame_give(same_holder) == FRAME_LEASE_OK);

    FrameLease other_owner = take(33);
    assert(other_owner.base == a.base && other_owner.incarnation != same_holder.incarnation);
    for (size_t i = 0; i < 4096; ++i) assert(ram[other_owner.base + i] == 0);
    assert(program_frame_give(other_owner) == FRAME_LEASE_OK);
    assert(program_frame_give(b) == FRAME_LEASE_OK);
    assert(program_frame_lent() == 0 && frame_lease_audit(&frames.leases));
}

static void refusal_and_quarantine(void)
{
    setup();
    FrameLease a = take(44);
    assert(program_frame_map(a, va, flags, 4));
    size_t slot = program_frame_find(a);
    uint64_t *leaf = (uint64_t *)(uintptr_t)program_frames.entries[slot].leaf;
    uint64_t expected = *leaf;
    *leaf ^= PTE_WRITABLE;
    unsigned before_invalidations = host_invalidations;
    assert(program_frame_detach(a) == FRAME_LEASE_DENIED);
    assert(*leaf == (expected ^ PTE_WRITABLE) && host_invalidations == before_invalidations);
    assert(program_frame_give(a) == FRAME_LEASE_RETRY);
    assert(program_frames.entries[slot].present && program_frames.entries[slot].mapped);
    *leaf = expected;
    assert(program_frame_detach(a) == FRAME_LEASE_OK);
    assert(program_frame_give(a) == FRAME_LEASE_OK);

    FrameLease b = take(55);
    assert(program_frame_map(b, va, flags, 4));
    slot = program_frame_find(b);
    size_t core = core_slot(b);
    uint32_t incarnation = frames.leases.entries[core].incarnation;
    ++frames.leases.entries[core].incarnation;
    assert(program_frame_detach(b) == FRAME_LEASE_DENIED);
    assert(!read_pte(va, &(uint64_t){0}));
    assert(program_frames.entries[slot].present && program_frames.entries[slot].returning &&
           !program_frames.entries[slot].mapped);
    assert(program_frame_give(b) == FRAME_LEASE_DENIED);
    assert(!program_frame_map(b, va + 4096, flags, 4));
    frames.leases.entries[core].incarnation = incarnation;
    assert(program_frame_give(b) == FRAME_LEASE_OK);
    assert(frame_lease_audit(&frames.leases));
}

static void transient_commit(void)
{
    setup();
    assert(program_frame_take_transient(0).status == FRAME_LEASE_INVALID);
    FrameLeaseResult result = program_frame_take_transient(66);
    assert(result.status == FRAME_LEASE_OK && result.value.kind == FRAME_LEASE_TRANSIENT);
    FrameLease lease = result.value;
    assert(program_frame_lent() == 0);
    assert(program_frame_map(lease, va, flags, 4));
    size_t core = core_slot(lease);
    uint64_t expected = 0;
    assert(read_pte(va, &expected));
    ram[lease.base + 37] = 0x5a;
    assert(program_frame_commit(lease) == FRAME_LEASE_OK);
    assert(program_frame_find(lease) == PROGRAM_FRAME_CAPACITY);
    assert(frames.leases.entries[core].kind == FRAME_LEASE_PINNED &&
           frames.leases.entries[core].holder == 0 &&
           frames.leases.entries[core].incarnation == lease.incarnation);
    uint64_t actual = 0;
    assert(read_pte(va, &actual) && actual == expected && ram[lease.base + 37] == 0x5a);
    assert(program_frame_commit(lease) == FRAME_LEASE_DENIED);
    assert(program_frame_detach(lease) == FRAME_LEASE_DENIED);
    assert(program_frame_give(lease) == FRAME_LEASE_DENIED);
    assert(!map_page(va, lease.base, flags, 4));

    FrameLease first = program_frame_take_transient(77).value;
    FrameLease second = program_frame_take_transient(88).value;
    FrameLease invalid = second;
    ++invalid.incarnation;
    FrameLease batch[] = {first, invalid};
    assert(program_frame_commit_all(batch, 2) == FRAME_LEASE_DENIED);
    assert(program_frame_find(first) < PROGRAM_FRAME_CAPACITY);
    assert(frames.leases.entries[core_slot(first)].kind == FRAME_LEASE_TRANSIENT);
    batch[1] = first;
    assert(program_frame_commit_all(batch, 2) == FRAME_LEASE_DENIED);
    assert(frames.leases.entries[core_slot(first)].kind == FRAME_LEASE_TRANSIENT);
    assert(program_frame_give(first) == FRAME_LEASE_OK);
    assert(program_frame_give(second) == FRAME_LEASE_OK);
    assert(frame_lease_audit(&frames.leases));
}

static void run_commit_and_reuse(void)
{
    setup();
    uint64_t available = program_frame_available();
    FrameLease lease = program_frame_take_transient(66).value;
    assert(program_frame_map_run(lease, 700, va, flags, 4));
    uint64_t mapped_available = program_frame_available();
    assert(mapped_available < available);
    FrameLease stale = lease;
    assert(program_frame_commit_all(&lease, 1) == FRAME_LEASE_OK);
    assert(lease.kind == FRAME_LEASE_PROGRAM && program_frame_lent() == 1);
    size_t slot = program_frame_find(lease), core = core_slot(lease);
    assert(slot < PROGRAM_FRAME_CAPACITY && core < FRAME_LEASE_CAPACITY);
    assert(program_frames.entries[slot].run == 700 && program_frames.entries[slot].mapped);
    assert(frames.leases.entries[core].kind == FRAME_LEASE_PROGRAM &&
           frames.leases.entries[core].holder == 66);
    assert(program_frame_available() == mapped_available);
    assert(frame_lease_audit(&frames.leases));
    assert(program_frame_commit(lease) == FRAME_LEASE_DENIED);
    assert(!program_frame_release(stale));
    assert(program_frame_release_run(66, 701));
    assert(!program_frame_release_run(66, 0));
    uint64_t expected = 0;
    assert(read_pte(va, &expected) && program_frames.entries[slot].present);
    assert(program_frame_release_run(66, 700));
    assert(!read_pte(va, &expected) && !program_frames.entries[slot].present);
    assert(!program_frame_lent() && program_frame_available() == mapped_available + 1);
    for (size_t i = 0; i < 4096; ++i) ram[lease.base + i] = 0xcc;
    FrameLease reused = take(66);
    assert(reused.base == lease.base && reused.incarnation != lease.incarnation);
    for (size_t i = 0; i < 4096; ++i) assert(ram[reused.base + i] == 0);
    assert(program_frame_map_run(reused, 701, va, flags, 4));
    assert(program_frame_release_run(66, 701));
    assert(frame_lease_audit(&frames.leases));
}

static void workspace_handover(void)
{
    setup();
    const uint64_t workspace_va = va + UINT64_C(0x1000000);
    FrameLease lease = program_frame_take_transient(66).value;
    assert(program_frame_map_run(lease, 700, va, flags, 4));
    assert(program_frame_commit_all(&lease, 1) == FRAME_LEASE_OK);
    uint64_t expected = 0;
    assert(read_pte(va, &expected));
    AppPrivatePages image = {.count = 1,
        .pages = {{va, page_leaf_address(va), expected & ~UINT64_C(0x60)}}};
    assert(program_frame_workspace_map(&image.pages[0], 66, 700, workspace_va, 5));
    assert(read_pte(workspace_va, &expected));
    AppPrivatePages workspace = {.count = 1,
        .pages = {{workspace_va, page_leaf_address(workspace_va), expected & ~UINT64_C(0x60)}}};
    *(volatile uint64_t *)(uintptr_t)workspace.pages[0].leaf = 0;
    cpu_invlpg(workspace_va);
    assert(!program_frame_handover_check(&image, &workspace, 66, 701, 77));
    assert(!program_frame_handover_check(&image, &workspace, 66, 700, 66));
    assert(program_frame_handover_check(&image, &workspace, 66, 700, 77));
    *(volatile uint64_t *)(uintptr_t)image.pages[0].leaf = 0;
    cpu_invlpg(va);
    *(volatile uint64_t *)(uintptr_t)workspace.pages[0].leaf = workspace.pages[0].expected;
    cpu_invlpg(workspace_va);
    assert(!program_frame_handover_commit(&image, &workspace, 66, 701, 77));
    assert(program_frame_handover_commit(&image, &workspace, 66, 700, 77));
    size_t slot = PROGRAM_FRAME_CAPACITY;
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        if (program_frames.entries[i].present && program_frames.entries[i].va == workspace_va)
            slot = i;
    assert(slot < PROGRAM_FRAME_CAPACITY);
    ProgramFrameBinding *binding = &program_frames.entries[slot];
    assert(binding->lease.holder == 77 && !binding->run && binding->leaf == workspace.pages[0].leaf &&
           binding->expected == workspace.pages[0].expected && binding->mapped);
    assert(core_slot(binding->lease) < FRAME_LEASE_CAPACITY && program_frame_lent() == 1);
    assert(!program_frame_release(lease));
    assert(program_frame_release_run(66, 700));
    assert(program_frame_release_run(77, 700));
    assert(!program_frame_release_run(77, 0));
    assert(!program_frame_handover_commit(&image, &workspace, 66, 700, 77));
    assert(read_pte(workspace_va, &expected) &&
           (expected & ~UINT64_C(0x60)) == workspace.pages[0].expected);
    assert(binding->present && binding->lease.holder == 77 && frame_lease_audit(&frames.leases));
}

static void reserve_and_exhaustion(void)
{
    setup();
    unsigned count = 0;
    for (;;) {
        FrameLeaseResult result = program_frame_take(77);
        if (result.status == FRAME_LEASE_FULL) break;
        assert(result.status == FRAME_LEASE_OK && ++count < 64);
    }
    assert(count && program_frame_lent() == count);
    assert(program_frame_available() == 0 && frames.leases.free_pages == frames.leases.reserve);
    uint64_t pinned = frames_alloc(0);
    assert(pinned != NO_ADDRESS);
    for (size_t i = 0; i < 4096; ++i) assert(ram[pinned + i] == 0);
    assert(frame_lease_audit(&frames.leases));
}

int main(void)
{
    ram = aligned_alloc(4096, 64 * 4096);
    assert(ram);
    lifecycle_and_reuse();
    refusal_and_quarantine();
    transient_commit();
    run_commit_and_reuse();
    workspace_handover();
    reserve_and_exhaustion();
    free(ram);
    puts("program frames: exact run reclaim, zeroed reuse, workspace ownership transfer, alias refusal and stale/double-return controls passed");
    return 0;
}
