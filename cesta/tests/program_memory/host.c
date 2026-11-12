/* Program-memory growth, rollback and run-lifecycle controls on production RAM. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define memcpy sexos_memcpy
#define memmove sexos_memmove
#define memset sexos_memset
#define memcmp sexos_memcmp
#define program_frame_take actual_program_frame_take
#define program_frame_map_run actual_program_frame_map_run
#include "kernel_memory.c"
#undef program_frame_take
#undef program_frame_map_run

static int fail_take = -1, fail_map = -1;
static unsigned take_calls, map_calls;
FrameLeaseResult program_frame_take(uint32_t holder)
{
    if (fail_take >= 0 && take_calls++ >= (unsigned)fail_take)
        return (FrameLeaseResult){.status = FRAME_LEASE_FULL};
    return actual_program_frame_take(holder);
}
bool program_frame_map_run(FrameLease lease, uint32_t run, uint64_t va,
                           uint64_t flags, uint8_t key)
{
    if (fail_map >= 0 && map_calls++ >= (unsigned)fail_map) return false;
    return actual_program_frame_map_run(lease, run, va, flags, key);
}
#include "program_memory.c"
#undef memcpy
#undef memmove
#undef memset
#undef memcmp

Kernel kernel;
unsigned host_invalidations;
void klog(const char *format, ...) { (void)format; }

static unsigned char *ram;
static const uint64_t arena = UINT64_C(0x600000000000);
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
    fail_take = fail_map = -1;
    take_calls = map_calls = 0;
    host_invalidations = 0;
    assert(frame_lease_audit(&frames.leases));
}

static ProgramMemoryState begin(uint32_t target, uint32_t run, uint64_t image, uint64_t stack)
{
    ProgramMemoryState state;
    program_memory_init(&state);
    assert(program_memory_begin(&state, target, run, 4, arena, 8, image, stack) == PROGRAM_MEMORY_OK);
    return state;
}

static void no_bindings(void)
{
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        assert(!program_frames.entries[i].present);
    assert(program_frame_lent() == 0 && frame_lease_audit(&frames.leases));
}

static void absent(uint64_t va)
{
    uint64_t pte = 0;
    assert(!read_pte(va, &pte));
}

static void successful_lifecycle(void)
{
    setup();
    ProgramMemoryState state = begin(11, 101, 1, 1);
    assert(program_memory_map_at(&state, arena, flags));
    assert(program_memory_map_at(&state, arena + 7 * 4096, flags));
    assert(program_frame_lent() == 2);
    ProgramMemoryReply reply = program_memory_grow(&state, 0);
    assert(reply.status == PROGRAM_MEMORY_OK && reply.base == arena + 4096 && !reply.held);
    reply = program_memory_grow(&state, 2);
    assert(reply.status == PROGRAM_MEMORY_OK && reply.base == arena + 4096 && reply.held == 2);
    assert(read_pte(arena + 4096, &(uint64_t){0}) && read_pte(arena + 8192, &(uint64_t){0}));
    reply = program_memory_grow(&state, 2);
    assert(reply.status == PROGRAM_MEMORY_OK && reply.base == arena + 4096 && reply.held == 4);
    reply = program_memory_grow(&state, 0);
    assert(reply.status == PROGRAM_MEMORY_OK && reply.base == arena + 4096 && reply.held == 4);
    reply = program_memory_grow(&state, 2);
    assert(reply.status == PROGRAM_MEMORY_FULL && !reply.base && !reply.held && state.grown == 4);
    assert(program_memory_grow(&state, 257).status == PROGRAM_MEMORY_INVALID);
    assert(program_memory_end(&state) == PROGRAM_MEMORY_RETRY);
    assert(program_memory_worker_spawn(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_main_exit(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&state) == PROGRAM_MEMORY_RETRY);
    assert(program_memory_worker_exit(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&state) == PROGRAM_MEMORY_OK);
    assert(state.phase == PROGRAM_MEMORY_DEAD && !state.grown);
    no_bindings();
    absent(arena); absent(arena + 7 * 4096);
    for (uint64_t page = arena + 4096; page < arena + 5 * 4096; page += 4096) absent(page);
    assert(program_memory_end(&state) == PROGRAM_MEMORY_DENIED);
    assert(program_memory_begin(&state, 11, 101, 4, arena, 8, 0, 0) == PROGRAM_MEMORY_INVALID);
    assert(program_memory_begin(&state, 11, 102, 4, arena, 8, 0, 0) == PROGRAM_MEMORY_OK);
    assert(program_memory_main_exit(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&state) == PROGRAM_MEMORY_OK);
}

static void worker_capacity(void)
{
    setup();
    ProgramMemoryState state = begin(12, 201, 0, 0);
    for (unsigned i = 0; i < PROGRAM_MEMORY_MAX_WORKERS; ++i)
        assert(program_memory_worker_spawn(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_worker_spawn(&state) == PROGRAM_MEMORY_FULL);
    for (unsigned i = 0; i < PROGRAM_MEMORY_MAX_WORKERS; ++i)
        assert(program_memory_worker_exit(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_worker_exit(&state) == PROGRAM_MEMORY_DENIED);
    assert(program_memory_main_exit(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&state) == PROGRAM_MEMORY_OK);
}

static void rollback_and_progress(void)
{
    for (unsigned kind = 0; kind < 2; ++kind) {
        for (unsigned index = 0; index < 2; ++index) {
            setup();
            ProgramMemoryState state = begin(13, 301, 0, 0);
            if (kind == 0) fail_take = (int)index; else fail_map = (int)index;
            ProgramMemoryReply reply = program_memory_grow(&state, 2);
            assert(reply.status == PROGRAM_MEMORY_FULL && reply.held == 0 && state.grown == 0);
            no_bindings();
            absent(arena); absent(arena + 4096);
            fail_take = fail_map = -1;
            reply = program_memory_grow(&state, 1);
            assert(reply.status == PROGRAM_MEMORY_OK && reply.held == 1);
            assert(program_memory_main_exit(&state) == PROGRAM_MEMORY_OK);
            assert(program_memory_end(&state) == PROGRAM_MEMORY_OK);
            no_bindings();
        }
    }
    setup();
    ProgramMemoryState state = begin(14, 401, 0, 0);
    fail_take = 0;
    assert(!program_memory_map_at(&state, arena, flags));
    no_bindings();
}

static void quarantine_and_isolation(void)
{
    setup();
    ProgramMemoryState state = begin(15, 501, 0, 0);
    assert(program_memory_grow(&state, 2).status == PROGRAM_MEMORY_OK);
    size_t slot = PROGRAM_FRAME_CAPACITY;
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        if (program_frames.entries[i].present) { slot = i; break; }
    assert(slot < PROGRAM_FRAME_CAPACITY);
    uint64_t expected = program_frames.entries[slot].expected;
    program_frames.entries[slot].expected ^= PTE_WRITABLE;
    assert(program_memory_main_exit(&state) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&state) == PROGRAM_MEMORY_RETRY);
    assert(state.phase == PROGRAM_MEMORY_QUARANTINE && program_frames.entries[slot].present);
    program_frames.entries[slot].expected = expected;
    assert(program_memory_end(&state) == PROGRAM_MEMORY_OK);
    no_bindings();

    ProgramMemoryState first = begin(16, 601, 0, 0);
    ProgramMemoryState second;
    program_memory_init(&second);
    assert(program_memory_begin(&second, 16, 602, 4, arena + 2 * 1024 * 1024, 8, 0, 0) == PROGRAM_MEMORY_OK);
    assert(program_memory_grow(&first, 1).status == PROGRAM_MEMORY_OK);
    assert(program_memory_grow(&second, 1).status == PROGRAM_MEMORY_OK);
    assert(program_memory_main_exit(&first) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&first) == PROGRAM_MEMORY_OK);
    assert(program_frame_lent() == 1);
    assert(program_memory_main_exit(&second) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&second) == PROGRAM_MEMORY_OK);
    no_bindings();
    ProgramMemoryState restart = begin(16, 603, 0, 0);
    assert(program_memory_grow(&restart, 1).status == PROGRAM_MEMORY_OK);
    assert(program_memory_main_exit(&restart) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&restart) == PROGRAM_MEMORY_OK);
    no_bindings();
}

static void pressure_hysteresis(void)
{
    setup();
    frames.leases.free_pages = 10000;
    assert(program_frame_pressure() == PROGRAM_PRESSURE_NORMAL);
    frames.leases.free_pages = 1000;
    assert(program_frame_pressure() == PROGRAM_PRESSURE_CRITICAL);
    frames.leases.free_pages = 3000;
    assert(program_frame_pressure() == PROGRAM_PRESSURE_LOW);
    frames.leases.free_pages = 4000;
    assert(program_frame_pressure() == PROGRAM_PRESSURE_LOW);
    frames.leases.free_pages = 9000;
    assert(program_frame_pressure() == PROGRAM_PRESSURE_LOW);
    frames.leases.free_pages = 10000;
    assert(program_frame_pressure() == PROGRAM_PRESSURE_LOW);
    frames.leases.free_pages = 11000;
    assert(program_frame_pressure() == PROGRAM_PRESSURE_NORMAL);
}

int main(void)
{
    ram = aligned_alloc(4096, 64 * 4096);
    assert(ram);
    successful_lifecycle();
    worker_capacity();
    rollback_and_progress();
    quarantine_and_isolation();
    pressure_hysteresis();
    free(ram);
    puts("program memory: contiguous growth, all-or-none rollback, worker gates, exact run cleanup, quarantine and pressure hysteresis passed");
    return 0;
}
