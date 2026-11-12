#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "program_records.h"

static ProgramRecords table;
static ProgramRecordTask tasks[PROGRAM_RECORD_CAPACITY];
static ProgramImage image;

static void setup(size_t count, size_t frames)
{
    memset(&table, 0, sizeof table);
    memset(tasks, 0, sizeof tasks);
    memset(&image, 0, sizeof image);
    assert(program_records_init(&table, count, UINT64_C(0x400000), frames) == PROGRAM_RECORD_OK);
    image.window = PROGRAM_RECORD_WINDOW_BYTES;
    image.entry = 0x40;
    image.count = 2;
    image.segments[0] = (ProgramSegment){.file = 4096, .window = 0, .memory = 8192,
                                         .writable = false, .executable = true};
    image.segments[1] = (ProgramSegment){.offset = 4096, .file = 4096, .window = 8192,
                                         .memory = 4096, .writable = true, .executable = false};
    assert(program_records_audit(&table));
}

static void dead(size_t index, uint32_t id)
{
    tasks[index] = (ProgramRecordTask){.id = id, .owner_core = PROGRAM_RECORD_NO_CORE,
                                       .phase = PROGRAM_RECORD_TASK_DEAD, .exited = true};
}

static void assert_absent(const ProgramRecord *record)
{
    for (size_t page = 0; page < PROGRAM_RECORD_LEAF_COUNT; ++page)
        assert(!record->pages[page].present);
}

static void sizing_and_layout(void)
{
    const uint64_t mib = UINT64_C(1024) * 1024;
    ProgramRecordSizing sizing = program_records_sizing(0);
    assert(sizing.count == 16 && sizing.pool_bytes == 4 * mib);
    sizing = program_records_sizing(768 * mib);
    assert(sizing.count == 2048 && sizing.pool_bytes == 256 * mib);
    sizing = program_records_sizing(1024 * mib);
    assert(sizing.count == 4096 && sizing.pool_bytes == 512 * mib);
    sizing = program_records_sizing(1280 * mib);
    assert(sizing.count == 4096 && sizing.pool_bytes == 512 * mib);
    sizing = program_records_sizing(1536 * mib);
    assert(sizing.count == 4096 && sizing.pool_bytes == 1024 * mib);
    sizing = program_records_sizing(UINT64_MAX);
    assert(sizing.count == 4096 && sizing.pool_bytes == 1024 * mib);

    assert(program_record_virtual_page(0, 0) == PROGRAM_RECORD_ARENA);
    assert(program_record_virtual_page(1, 0) == PROGRAM_RECORD_ARENA + PROGRAM_RECORD_SLICE);
    assert(program_record_virtual_page(0, PROGRAM_RECORD_WINDOW_PAGES) ==
           PROGRAM_RECORD_ARENA + PROGRAM_RECORD_WINDOW_BYTES);
    assert(program_record_virtual_page(0, PROGRAM_RECORD_LEAF_COUNT) == UINT64_MAX);
    assert(program_record_virtual_page(PROGRAM_RECORD_CAPACITY, 0) == UINT64_MAX);
    assert(program_record_stack_top(0) ==
           PROGRAM_RECORD_ARENA + PROGRAM_RECORD_LEAF_COUNT * UINT64_C(4096) - 16);
    assert(program_record_stack_top(1) ==
           PROGRAM_RECORD_ARENA + PROGRAM_RECORD_SLICE +
           PROGRAM_RECORD_LEAF_COUNT * UINT64_C(4096) - 16);

    ProgramRecords invalid;
    assert(program_records_init(&invalid, 1, 0x400001, 8) == PROGRAM_RECORD_INVALID);
    assert(program_records_init(&invalid, 0, 0x400000, 8) == PROGRAM_RECORD_INVALID);
    assert(program_records_init(&invalid, PROGRAM_RECORD_CAPACITY + 1, 0x400000, 8) == PROGRAM_RECORD_INVALID);
    assert(program_records_init(&invalid, 1, 0x400000, 0) == PROGRAM_RECORD_INVALID);
    assert(program_records_init(&invalid, 1, 0x400000, PROGRAM_RECORD_POOL_FRAMES + 1) == PROGRAM_RECORD_INVALID);
    assert(program_records_init(&invalid, 1, (uint64_t)UINT32_MAX * 4096, 2) == PROGRAM_RECORD_INVALID);
}

static void lifecycle(void)
{
    setup(4, 40);
    ProgramRecordPlan plan = {0};
    assert(program_records_plan(&image, &plan) == PROGRAM_RECORD_OK);
    assert(plan.image_pages == 3 && plan.needed == 7);
    ProgramRecordReply reply = program_records_start(&table, &image, 2);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == 2 && reply.second == 2);
    assert(table.pool_free == 26 && table.live == 2 && table.started == 2 && !table.ended);
    ProgramRecord *first = &table.entries[1], *second = &table.entries[2];
    assert(first->state == PROGRAM_RECORD_LIVE && second->state == PROGRAM_RECORD_LIVE);
    assert(first->task >= PROGRAM_RECORD_TASK_FIRST && second->task > first->task);
    assert(first->image_pages == 3 && second->image_pages == 3);
    for (size_t page = 0; page < 2; ++page) {
        assert(first->pages[page].allocated && first->pages[page].executable);
        assert(!first->pages[page].writable && !first->pages[page].present);
    }
    assert(first->pages[2].allocated && first->pages[2].writable && !first->pages[2].executable);
    assert(!first->pages[PROGRAM_RECORD_WINDOW_PAGES].allocated);
    for (size_t page = PROGRAM_RECORD_STACK_FIRST;
         page < PROGRAM_RECORD_STACK_FIRST + PROGRAM_RECORD_STACK_PAGES; ++page) {
        assert(first->pages[page].allocated && first->pages[page].writable);
        assert(!first->pages[page].executable && !first->pages[page].present);
    }
    assert(program_records_audit(&table));

    ProgramRecords before = table;
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE + 1, first->task + 1) ==
           PROGRAM_RECORD_DENIED);
    assert(!memcmp(&before, &table, sizeof table));
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE, first->task) ==
           PROGRAM_RECORD_DENIED);
    assert(!memcmp(&before, &table, sizeof table));
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE + 3, first->task) ==
           PROGRAM_RECORD_DENIED);
    assert(!memcmp(&before, &table, sizeof table));

    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE + 1, first->task) ==
           PROGRAM_RECORD_OK);
    assert(table.tenant == 2 && table.switches == 1 && first->pages[0].present);
    assert_absent(second);
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE + 1, first->task) ==
           PROGRAM_RECORD_OK);
    assert(table.switches == 1);
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE + 2, second->task) ==
           PROGRAM_RECORD_OK);
    assert(table.tenant == 3 && table.switches == 2 && second->pages[0].present);
    assert_absent(first);
    assert(program_records_audit(&table));

    reply = program_records_stop(&table, false);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == 2 && reply.second == 2);
    assert(table.stop && !first->killed && !second->killed);
    assert(program_records_stop_requested(&table, PROGRAM_RECORD_BASE + 1));
    assert(!program_records_stop_requested(&table, 12));

    tasks[1] = (ProgramRecordTask){.id = first->task, .owner_core = 0,
                                   .phase = PROGRAM_RECORD_TASK_DEAD, .exited = true};
    reply = program_records_reap(&table, tasks);
    assert(reply.status == PROGRAM_RECORD_OK && !reply.first && table.live == 2 && table.pool_free == 26);
    tasks[1].owner_core = PROGRAM_RECORD_NO_CORE;
    tasks[1].id = first->task + 1;
    reply = program_records_reap(&table, tasks);
    assert(!reply.first && table.live == 2 && table.pool_free == 26);
    tasks[1].id = first->task;
    first->reply_pending = true;
    reply = program_records_reap(&table, tasks);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == 1 && table.pool_free == 33);
    assert(table.live == 1 && table.ended == 1 && first->state == PROGRAM_RECORD_FREE);
    assert(!first->task && !first->reply_pending && !first->image_pages);
    for (size_t page = 0; page < PROGRAM_RECORD_LEAF_COUNT; ++page)
        assert(!first->pages[page].allocated);
    assert(program_records_audit(&table));
    reply = program_records_reap(&table, tasks);
    assert(!reply.first && table.live == 1 && table.ended == 1 && table.pool_free == 33);

    reply = program_records_stop(&table, true);
    assert(reply.first == 1 && second->killed);
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE + 2, second->task) ==
           PROGRAM_RECORD_DENIED);
    assert(!table.tenant);
    assert_absent(second);
    assert(program_records_audit(&table));
    dead(2, second->task);
    reply = program_records_reap(&table, tasks);
    assert(reply.first == 1 && table.live == 0 && table.ended == 2 && table.pool_free == 40);
    reply = program_records_census(&table);
    assert(reply.status == PROGRAM_RECORD_OK && !reply.first && reply.second == 40);
    assert(program_records_audit(&table));
}

static void refusal_and_exhaustion(void)
{
    setup(4, 6);
    ProgramRecords before = table;
    ProgramRecordReply reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_FULL && !reply.first);
    assert(!memcmp(&before, &table, sizeof table));

    setup(4, 40);
    ProgramImage bad = image;
    bad.segments[0].writable = true;
    before = table;
    reply = program_records_start(&table, &bad, 1);
    assert(reply.status == PROGRAM_RECORD_INVALID && !memcmp(&before, &table, sizeof table));
    bad = image;
    bad.segments[0].window = 4096;
    reply = program_records_start(&table, &bad, 1);
    assert(reply.status == PROGRAM_RECORD_INVALID && !memcmp(&before, &table, sizeof table));
    bad = image;
    bad.entry = PROGRAM_RECORD_WINDOW_BYTES;
    reply = program_records_start(&table, &bad, 1);
    assert(reply.status == PROGRAM_RECORD_INVALID && !memcmp(&before, &table, sizeof table));
    bad = image;
    bad.segments[1].window = PROGRAM_RECORD_WINDOW_BYTES - 4096;
    bad.segments[1].memory = 8192;
    reply = program_records_start(&table, &bad, 1);
    assert(reply.status == PROGRAM_RECORD_INVALID && !memcmp(&before, &table, sizeof table));

    setup(4, 14);
    reply = program_records_start(&table, &image, 3);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == 2 && table.live == 2 && !table.pool_free);
    before = table;
    reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_FULL && !memcmp(&before, &table, sizeof table));

    setup(4, 28);
    reply = program_records_start(&table, &image, 5);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == 4 && table.live == 4 && !table.pool_free);
    before = table;
    reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_FULL && !memcmp(&before, &table, sizeof table));

    setup(2, 14);
    table.next_task = 0;
    before = table;
    reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_FULL && !memcmp(&before, &table, sizeof table));
    table.next_task = UINT32_MAX;
    reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == 1);
    assert(table.entries[1].task == UINT32_MAX && !table.next_task);
    before = table;
    reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_FULL && !memcmp(&before, &table, sizeof table));
}

static void sparse_restart_and_batches(void)
{
    setup(1, 20);
    image.count = 2;
    image.segments[0].memory = 4096;
    image.segments[1].window = 8192;
    image.segments[1].memory = 4096;
    ProgramRecordPlan plan = {0};
    assert(program_records_plan(&image, &plan) == PROGRAM_RECORD_OK);
    assert(plan.image_pages == 3 && plan.needed == 6);
    ProgramRecordReply reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == 1);
    ProgramRecord *record = &table.entries[0];
    assert(record->pages[0].allocated && !record->pages[1].allocated && record->pages[2].allocated);
    assert(program_records_audit(&table));
    uint32_t old_task = record->task;
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE, old_task) == PROGRAM_RECORD_OK);
    dead(0, old_task);
    assert(program_records_reap(&table, tasks).first == 1);
    assert(table.pool_free == 20 && program_records_audit(&table));
    reply = program_records_start(&table, &image, 1);
    assert(reply.status == PROGRAM_RECORD_OK && record->task != old_task && record->task > old_task);
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE, old_task) == PROGRAM_RECORD_DENIED);
    assert(program_records_before_dispatch(&table, PROGRAM_RECORD_BASE, record->task) == PROGRAM_RECORD_OK);
    assert(program_records_audit(&table));

    setup(PROGRAM_RECORD_CAPACITY, PROGRAM_RECORD_CAPACITY * 7);
    reply = program_records_start(&table, &image, PROGRAM_RECORD_CAPACITY);
    assert(reply.status == PROGRAM_RECORD_OK && reply.first == PROGRAM_RECORD_START_BATCH);
    assert(table.live == PROGRAM_RECORD_START_BATCH);
    reply = program_records_start(&table, &image, PROGRAM_RECORD_CAPACITY);
    assert(reply.status == PROGRAM_RECORD_OK &&
           reply.first == PROGRAM_RECORD_CAPACITY - PROGRAM_RECORD_START_BATCH);
    assert(table.live == PROGRAM_RECORD_CAPACITY);
    for (size_t i = 0; i < table.count; ++i)
        if (table.entries[i].state == PROGRAM_RECORD_LIVE) dead(i, table.entries[i].task);
    size_t remaining = table.live;
    while (remaining) {
        reply = program_records_reap(&table, tasks);
        assert(reply.status == PROGRAM_RECORD_OK && reply.first &&
               reply.first <= PROGRAM_RECORD_REAP_BATCH);
        assert(reply.first <= remaining);
        remaining -= (size_t)reply.first;
        assert(table.live == remaining && program_records_audit(&table));
    }
    assert(table.pool_free == table.pool_total && table.ended == table.started);
}

static void loading_commit_and_abort(void)
{
    setup(1, 20);
    ProgramRecordReply began = program_record_begin(&table, &image);
    assert(began.status == PROGRAM_RECORD_OK && began.first == PROGRAM_RECORD_BASE);
    assert(began.second >= PROGRAM_RECORD_TASK_FIRST);
    ProgramRecord *record = &table.entries[0];
    assert(record->state == PROGRAM_RECORD_LOADING && record->task == began.second);
    assert(!table.live && !table.started && !table.ended && table.pool_free == 13);
    assert(program_records_audit(&table));
    assert(program_records_before_dispatch(&table, began.first, (uint32_t)began.second) ==
           PROGRAM_RECORD_DENIED);

    ProgramRecords before = table;
    assert(program_record_commit(&table, began.first, (uint32_t)began.second + 1).status ==
           PROGRAM_RECORD_DENIED);
    assert(!memcmp(&before, &table, sizeof table));
    assert(program_record_abort(&table, began.first, (uint32_t)began.second + 1).status ==
           PROGRAM_RECORD_DENIED);
    assert(!memcmp(&before, &table, sizeof table));

    ProgramRecordReply committed = program_record_commit(&table, began.first, (uint32_t)began.second);
    assert(committed.status == PROGRAM_RECORD_OK && committed.first == 1);
    assert(record->state == PROGRAM_RECORD_LIVE && table.live == 1 && table.started == 1);
    assert(program_records_audit(&table));
    dead(0, record->task);
    assert(program_records_reap(&table, tasks).first == 1);
    assert(table.ended == 1 && table.pool_free == 20 && program_records_audit(&table));

    ProgramRecordReply second = program_record_begin(&table, &image);
    assert(second.status == PROGRAM_RECORD_OK && second.second > began.second);
    assert(!table.live && table.started == 1 && table.ended == 1 && table.pool_free == 13);
    ProgramRecordReply aborted = program_record_abort(&table, second.first, (uint32_t)second.second);
    assert(aborted.status == PROGRAM_RECORD_OK && aborted.second == 20);
    assert(record->state == PROGRAM_RECORD_FREE && !record->task && !table.live);
    assert(table.started == 1 && table.ended == 1 && table.pool_free == 20);
    assert(program_records_audit(&table));
    before = table;
    assert(program_record_abort(&table, second.first, (uint32_t)second.second).status ==
           PROGRAM_RECORD_DENIED);
    assert(!memcmp(&before, &table, sizeof table));
}

int main(void)
{
    sizing_and_layout();
    lifecycle();
    refusal_and_exhaustion();
    sparse_restart_and_batches();
    loading_commit_and_abort();
    puts("program records: bounded pool, exact task incarnations, loading rollback, tenancy, stop, bounded reap and restart passed");
    return 0;
}
