#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define memcpy sexos_memcpy
#define memmove sexos_memmove
#define memset sexos_memset
#define memcmp sexos_memcmp
#define map_page actual_map_page
#include "kernel_memory.c"
#undef map_page
#undef memcpy
#undef memmove
#undef memset
#undef memcmp
#include "program_record_memory.h"
#include "program_record_tasks.h"

Kernel kernel;
unsigned host_invalidations;
void klog(const char *format, ...) { (void)format; }
void program_image_bounds_failure(void) { abort(); }

static unsigned char *ram;
static ProgramRecords table;
static ProgramRecordMemory memory;
static ProgramRecordTask tasks[PROGRAM_RECORD_CAPACITY];
static ProgramImage image;
static uint8_t image_bytes[4096];
static int fail_map = -1;
static unsigned map_calls;

bool map_page(uint64_t va, uint64_t physical, uint64_t flags, uint8_t key)
{
    if (fail_map >= 0 && map_calls++ >= (unsigned)fail_map) return false;
    return actual_map_page(va, physical, flags, key);
}

static void put64(uint8_t *at, uint64_t value)
{
    for (unsigned i = 0; i < 8; ++i) at[i] = (uint8_t)(value >> (8 * i));
}

static uint64_t read64(const ProgramRecord *record, uint64_t at)
{
    size_t page = (size_t)(at / 4096), inside = (size_t)(at % 4096);
    uint64_t physical = (uint64_t)record->pages[page].frame * 4096 + inside, value = 0;
    for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)ram[physical + i] << (8 * i);
    return value;
}

static uint64_t expected(const ProgramRecordPage *page, bool present)
{
    uint64_t value = (uint64_t)page->frame * 4096 | PTE_USER | (UINT64_C(10) << PTE_KEY_SHIFT);
    if (page->writable) value |= PTE_WRITABLE;
    if (!page->executable) value |= PTE_NO_EXECUTE;
    if (present) value |= PTE_PRESENT;
    return value;
}

static uint64_t *leaf(size_t index, size_t page)
{
    return (uint64_t *)(uintptr_t)page_leaf_address(program_record_virtual_page(index, page));
}

static void setup(size_t count)
{
    enum { BYTES = 1152 * 4096, POOL = 0x400000, POOL_BYTES = 64 * 4096 };
    memset(ram, 0xa5, BYTES);
    memset(ram + 0x1000, 0, 4096);
    memset(&kernel, 0, sizeof kernel);
    memset(&table, 0, sizeof table);
    memset(&memory, 0, sizeof memory);
    memset(tasks, 0, sizeof tasks);
    memset(&image, 0, sizeof image);
    hhdm_offset = (uint64_t)(uintptr_t)ram;
    LimineMemmapEntry usable[] = {
        {.base = 0x2000, .length = POOL - 0x2000, .type = 0},
        {.base = POOL + POOL_BYTES, .length = BYTES - POOL - POOL_BYTES, .type = 0},
    };
    LimineMemmapEntry *entries[] = {&usable[0], &usable[1]};
    LimineMemmapResponse map = {.entry_count = 2, .entries = entries};
    memory_init(&map);
    frames.leases.reserve = 2;
    assert(program_records_init(&table, count, POOL, 64) == PROGRAM_RECORD_OK);
    assert(program_record_memory_init(&memory, PROGRAM_RECORD_KEY));
    fail_map = -1;
    map_calls = 0;
    host_invalidations = 0;
    for (size_t i = 0; i < sizeof image_bytes; ++i) image_bytes[i] = (uint8_t)(i * 37 + 11);
    image.source = (cesta_read_bytes){image_bytes, sizeof image_bytes};
    image.entry = 0x40;
    image.rela = 0x400;
    image.rela_count = 1;
    image.window = PROGRAM_RECORD_WINDOW_BYTES;
    image.count = 2;
    image.segments[0] = (ProgramSegment){.offset = 0x100, .file = 0x80, .window = 0,
                                         .memory = 0x2000, .executable = true};
    image.segments[1] = (ProgramSegment){.offset = 0x200, .file = 0x40, .window = 0x2000,
                                         .memory = 0x3000, .writable = true};
    put64(image_bytes + 0x400, 0x2040);
    put64(image_bytes + 0x408, 8);
    put64(image_bytes + 0x410, 0x1234);
    assert(program_record_memory_audit(&table, &memory));
}

static void dead(size_t index, uint32_t id)
{
    tasks[index] = (ProgramRecordTask){.id = id, .owner_core = PROGRAM_RECORD_NO_CORE,
                                       .phase = PROGRAM_RECORD_TASK_DEAD, .exited = true};
}

static void assert_absent(size_t index)
{
    ProgramRecord *record = &table.entries[index];
    for (size_t page = 0; page < PROGRAM_RECORD_LEAF_COUNT; ++page) {
        uint64_t *entry = leaf(index, page);
        if (record->pages[page].allocated) {
            assert(entry && !read_pte(program_record_virtual_page(index, page), &(uint64_t){0}));
            assert(*entry == expected(&record->pages[page], false));
        } else {
            assert(!memory.leaves[index][page]);
            assert(!entry || !*entry);
        }
    }
}

static void assert_image(size_t index)
{
    ProgramRecord *record = &table.entries[index];
    for (size_t at = 0; at < 0x2000; ++at) {
        uint8_t expected_byte = at < 0x80 ? image_bytes[0x100 + at] : 0;
        uint64_t physical = (uint64_t)record->pages[at / 4096].frame * 4096 + at % 4096;
        assert(ram[physical] == expected_byte);
    }
    for (size_t off = 0; off < 0x3000; ++off) {
        if (off >= 0x40 && off < 0x48) continue;
        uint8_t expected_byte = off < 0x40 ? image_bytes[0x200 + off] : 0;
        uint64_t physical = (uint64_t)record->pages[2 + off / 4096].frame * 4096 + off % 4096;
        assert(ram[physical] == expected_byte);
    }
    assert(read64(record, 0x2040) == program_record_virtual_page(index, 0) + 0x1234);
    for (size_t page = PROGRAM_RECORD_STACK_FIRST;
         page < PROGRAM_RECORD_STACK_FIRST + PROGRAM_RECORD_STACK_PAGES; ++page) {
        assert(record->pages[page].allocated && record->pages[page].writable);
        assert(!record->pages[page].executable);
        uint64_t physical = (uint64_t)record->pages[page].frame * 4096;
        for (size_t i = 0; i < 4096; ++i) assert(ram[physical + i] == 0);
    }
    assert(!record->pages[PROGRAM_RECORD_WINDOW_PAGES].allocated);
    assert_absent(index);
    for (size_t page = 0; page < 2; ++page) {
        assert(record->pages[page].executable && !record->pages[page].writable);
    }
    for (size_t page = 2; page < 5; ++page) {
        assert(!record->pages[page].executable && record->pages[page].writable);
    }
}

static void mapping_failures(void)
{
    for (unsigned failure = 0; failure < 9; ++failure) {
        setup(1);
        fail_map = (int)failure;
        ProgramRecordLoad load = program_record_memory_start(&table, &memory, &image);
        assert(load.status == PROGRAM_RECORD_FULL && !load.pd && !load.task);
        assert(map_calls == failure + 1 && !table.live && !table.started && !table.ended);
        assert(table.pool_free == table.pool_total && table.entries[0].state == PROGRAM_RECORD_FREE);
        assert(program_record_memory_audit(&table, &memory));
        assert_absent(0);
        fail_map = -1;
        load = program_record_memory_start(&table, &memory, &image);
        assert(load.status == PROGRAM_RECORD_OK && load.task > PROGRAM_RECORD_TASK_FIRST);
        assert(table.live == 1 && table.started == 1 && table.pool_free == 55);
        assert_image(0);
        assert(program_record_memory_audit(&table, &memory));
    }
}

static void refusals(void)
{
    setup(1);
    ProgramRecords before = table;
    ProgramImage bad = image;
    bad.count = PROGRAM_IMAGE_SEGMENTS + 1;
    assert(program_record_memory_start(&table, &memory, &bad).status == PROGRAM_RECORD_INVALID);
    assert(!memcmp(&before, &table, sizeof table));
    bad = image;
    bad.segments[0].offset = UINT64_MAX;
    assert(program_record_memory_start(&table, &memory, &bad).status == PROGRAM_RECORD_INVALID);
    assert(!memcmp(&before, &table, sizeof table));

    put64(image_bytes + 0x400, 0x8000);
    assert(program_record_memory_start(&table, &memory, &image).status == PROGRAM_RECORD_INVALID);
    assert(!table.live && !table.started && !table.ended && table.pool_free == table.pool_total);
    assert(table.next_task == PROGRAM_RECORD_TASK_FIRST + 1);
    assert(program_record_memory_audit(&table, &memory));

    setup(1);
    uint64_t frame = frames_alloc(0), va = program_record_virtual_page(0, 0);
    assert(frame != NO_ADDRESS);
    assert(actual_map_page(va, frame, PTE_USER | PTE_NO_EXECUTE, PROGRAM_RECORD_KEY));
    uint64_t before_pte = 0;
    assert(read_pte(va, &before_pte));
    assert(program_record_memory_start(&table, &memory, &image).status == PROGRAM_RECORD_INVALID);
    assert(!table.live && !table.started && !table.ended && table.pool_free == table.pool_total);
    assert(read_pte(va, &(uint64_t){0}) && *leaf(0, 0) == before_pte);
    *leaf(0, 0) = 0;
    cpu_invlpg(va);
    assert(program_record_memory_audit(&table, &memory));
}

static void lifecycle_and_reuse(void)
{
    setup(2);
    ProgramRecordLoad first = program_record_memory_start(&table, &memory, &image);
    ProgramRecordLoad second = program_record_memory_start(&table, &memory, &image);
    assert(first.status == PROGRAM_RECORD_OK && second.status == PROGRAM_RECORD_OK);
    size_t fi = first.pd - PROGRAM_RECORD_BASE, si = second.pd - PROGRAM_RECORD_BASE;
    assert(fi < 2 && si < 2 && fi != si && first.task != second.task);
    assert(first.entry == program_record_virtual_page(fi, 0) + 0x40);
    assert(second.entry == program_record_virtual_page(si, 0) + 0x40);
    assert(first.stack_top == program_record_stack_top(fi));
    assert(second.stack_top == program_record_stack_top(si));
    assert(table.live == 2 && table.started == 2 && table.pool_free == 46);
    assert_image(fi);
    assert_image(si);
    assert(program_record_memory_audit(&table, &memory));

    ProgramRecords before = table;
    unsigned invalidations = host_invalidations;
    assert(program_record_memory_before_dispatch(&table, &memory, first.pd, first.task + 1) ==
           PROGRAM_RECORD_DENIED);
    assert(!memcmp(&before, &table, sizeof table) && invalidations == host_invalidations);
    assert(program_record_memory_before_dispatch(&table, &memory, first.pd, first.task) ==
           PROGRAM_RECORD_OK);
    assert(read_pte(program_record_virtual_page(fi, 0), &(uint64_t){0}));
    assert(!read_pte(program_record_virtual_page(si, 0), &(uint64_t){0}));
    assert(program_record_memory_audit(&table, &memory));
    *leaf(fi, 0) |= UINT64_C(0x20);
    *leaf(fi, 2) |= UINT64_C(0x60);
    *leaf(fi, PROGRAM_RECORD_STACK_FIRST) |= UINT64_C(0x60);
    assert(program_record_memory_audit(&table, &memory));
    assert(program_record_memory_before_dispatch(&table, &memory, second.pd, second.task) ==
           PROGRAM_RECORD_OK);
    assert(!read_pte(program_record_virtual_page(fi, 0), &(uint64_t){0}));
    assert(read_pte(program_record_virtual_page(si, 0), &(uint64_t){0}));
    assert(program_record_memory_audit(&table, &memory));

    before = table;
    invalidations = host_invalidations;
    *leaf(fi, 0) ^= PTE_WRITABLE;
    assert(program_record_memory_before_dispatch(&table, &memory, first.pd, first.task) ==
           PROGRAM_RECORD_RETRY);
    assert(!memcmp(&before, &table, sizeof table) && invalidations == host_invalidations);
    *leaf(fi, 0) = expected(&table.entries[fi].pages[0], false);

    assert(program_records_stop(&table, true).first == 2);
    assert(program_record_memory_before_dispatch(&table, &memory, second.pd, second.task) ==
           PROGRAM_RECORD_DENIED);
    assert(!table.tenant && program_record_memory_audit(&table, &memory));
    assert_absent(fi);
    assert_absent(si);

    dead(fi, table.entries[fi].task + 1);
    tasks[si] = (ProgramRecordTask){0};
    assert(program_record_memory_reap(&table, &memory, tasks).first == 0);
    assert(table.live == 2 && table.pool_free == 46);
    dead(fi, table.entries[fi].task);
    tasks[fi].owner_core = 0;
    assert(program_record_memory_reap(&table, &memory, tasks).first == 0);
    assert(table.live == 2 && table.pool_free == 46);
    tasks[fi].owner_core = PROGRAM_RECORD_NO_CORE;
    dead(si, table.entries[si].task);
    uint32_t old_task = table.entries[fi].task;
    ProgramRecordReply reaped = program_record_memory_reap(&table, &memory, tasks);
    assert(reaped.status == PROGRAM_RECORD_OK && reaped.first == 2 && reaped.second == 64);
    assert(!table.live && table.ended == 2 && table.pool_free == 64);
    assert(program_record_memory_audit(&table, &memory));
    assert_absent(fi);
    assert_absent(si);

    for (size_t i = 0; i < 64 * 4096; ++i) ram[0x400000 + i] = 0xcc;
    ProgramRecordLoad restarted = program_record_memory_start(&table, &memory, &image);
    size_t ri = restarted.pd - PROGRAM_RECORD_BASE;
    assert(restarted.status == PROGRAM_RECORD_OK && ri == fi && restarted.task != old_task);
    assert(program_record_memory_before_dispatch(&table, &memory, restarted.pd, old_task) ==
           PROGRAM_RECORD_DENIED);
    assert_image(ri);
    assert(program_record_memory_audit(&table, &memory));
}

static void loading_handoff(void)
{
    setup(1);
    static ProgramRecordTask task;
    static ProgramRecordTaskStorage storage;
    program_record_tasks_init(&task, &storage, 1);
    ProgramRecordLoad load = program_record_memory_begin(&table, &memory, &image);
    assert(load.status == PROGRAM_RECORD_OK && table.entries[0].state == PROGRAM_RECORD_LOADING);
    assert(!table.live && !table.started);
    assert(program_record_task_rearm(&task, &storage, 1, 0, load.task, load.entry, load.stack_top));
    assert(task.id == load.task && task.pd == load.pd && task.phase == PROGRAM_WORKER_DETACHED);
    assert(storage.frame->rip == load.entry);
    assert(storage.frame->rsp == load.stack_top - 8);
    assert(table.entries[0].state == PROGRAM_RECORD_LOADING && !table.live);
    ProgramRecordReply committed = program_record_memory_commit(&table, &memory, load.pd, load.task);
    assert(committed.status == PROGRAM_RECORD_OK && committed.first == 1);
    assert(table.entries[0].state == PROGRAM_RECORD_LIVE && table.live == 1 && table.started == 1);
    assert(program_record_memory_audit(&table, &memory));

    setup(1);
    load = program_record_memory_begin(&table, &memory, &image);
    assert(load.status == PROGRAM_RECORD_OK && table.entries[0].state == PROGRAM_RECORD_LOADING);
    ProgramRecordReply cancelled = program_record_memory_cancel(&table, &memory, load.pd, load.task);
    assert(cancelled.status == PROGRAM_RECORD_OK && cancelled.second == 64);
    assert(table.entries[0].state == PROGRAM_RECORD_FREE && !table.live && !table.started && !table.ended);
    assert(table.pool_free == table.pool_total && program_record_memory_audit(&table, &memory));
}

int main(void)
{
    enum { BYTES = 1152 * 4096 };
    ram = aligned_alloc(4096, BYTES);
    assert(ram);
    mapping_failures();
    refusals();
    lifecycle_and_reuse();
    loading_handoff();
    free(ram);
    puts("program record memory: real RAM, ELF writes, absent PTEs, rollback, tenancy, task handoff and zeroed reuse passed");
    return 0;
}
