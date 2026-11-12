/* Worker records, custody phases, delayed rearm and identity exhaustion. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sexos_kernel.h"
#include "program_workers.h"

static ProgramWorkers table;
static ProgramWorkerStorage storage[PROGRAM_WORKER_MAX];
static ProgramWorker fake[PROGRAM_RECORD_QUEUE_CAPACITY];

static ProgramWorkerStorage *worker_storage(ProgramWorker *worker)
{
    assert(worker->slot < PROGRAM_WORKER_MAX);
    return &storage[worker->slot];
}

static ProgramWorker *spawn(uint32_t target, uint32_t run, uint64_t now)
{
    ProgramWorkerResult result = program_worker_spawn(&table, storage, target, run,
                                                       0x4000 + run, 0x8000, run + 17,
                                                       (run % 3) + 1, now);
    if (result.status != PROGRAM_WORKER_OK)
        fprintf(stderr, "spawn target=%u run=%u status=%llu now=%llu count=%zu next=%x\n",
                target, run, (unsigned long long)result.status,
                (unsigned long long)now, table.count, table.next_id);
    assert(result.status == PROGRAM_WORKER_OK && result.value);
    return result.value;
}

static void attach(ProgramWorker *worker, uint32_t core)
{
    assert(program_worker_admit(worker));
    assert(program_worker_begin_dispatch(worker, core));
    assert(program_worker_attach(worker, core));
}

static void detach(ProgramWorker *worker, uint32_t core)
{
    assert(program_worker_begin_switch_out(worker, core));
    assert(program_worker_release_after_stack_switch(worker, core));
}

static void phase_and_rearm(void)
{
    program_workers_init(&table);
    ProgramWorker *first = spawn(11, 101, 1);
    ProgramWorkerStorage *first_state = worker_storage(first);
    assert(first->id == 0x40000000 && first->phase == PROGRAM_WORKER_DETACHED);
    assert(first->owner_core == PROGRAM_WORKER_NO_CORE);
    assert(first->kstack_top == (uint64_t)(uintptr_t)(first_state->stack + PROGRAM_WORKER_STACK_BYTES));
    assert(first_state->frame == (TrapFrame *)(first_state->stack + PROGRAM_WORKER_STACK_BYTES - sizeof *first_state->frame));
    assert(first->kstack_top % 16 == 0 && (uintptr_t)first_state->fpu % 16 == 0);
    assert(first_state->frame->rip == 0x4000 + 101 && first_state->frame->rdi == 101 + 17);
    assert(first_state->frame->rsp == 0x7ff8);
    assert(first->fs_base == 0 && first->wait_address == 0);
    attach(first, 2);
    assert(first->phase == PROGRAM_WORKER_ATTACHED && first->owner_core == 2);
    assert(program_worker_park(first, 2) == PROGRAM_WORKER_BLOCKED);
    assert(program_worker_wake(first) == PROGRAM_WORKER_RECORDED);
    assert(program_worker_park(first, 2) == PROGRAM_WORKER_WAKE_CONSUMED);
    detach(first, 2);
    assert(first->phase == PROGRAM_WORKER_DETACHED && first->owner_core == PROGRAM_WORKER_NO_CORE);
    assert(program_worker_exit(first));
    assert(program_worker_dead(first));
    program_worker_poll(&table, 1);
    assert(first->dead_since == 1);
    memset(first_state->stack, 0xcc, 64);

    ProgramWorker *second = spawn(11, 102, 2);
    assert(second != first && second->id == first->id + 1);
    attach(second, 3);
    detach(second, 3);
    assert(program_worker_exit(second));
    program_worker_poll(&table, 2);

    ProgramWorker *third = spawn(11, 103, 4);
    assert(third != first && third != second);
    attach(third, 1);
    detach(third, 1);
    assert(program_worker_exit(third));
    program_worker_poll(&table, 4);

    ProgramWorker *reused = spawn(11, 104, 5);
    ProgramWorkerStorage *reused_state = worker_storage(reused);
    assert(reused == first && reused_state == first_state && reused->id == third->id + 1);
    assert(reused->run == 104 && reused->phase == PROGRAM_WORKER_DETACHED);
    assert(reused->owner_core == PROGRAM_WORKER_NO_CORE && reused->dead_since == 0);
    for (size_t i = 0; i < 64; ++i) assert(reused_state->stack[i] == 0);
    assert(reused_state->frame->rip == 0x4000 + 104 && reused_state->frame->rdi == 104 + 17);
    assert(reused->fs_base == 0 && reused->wait_address == 0);
    assert(program_worker_find(&table, reused->id) == reused);
    program_worker_end(&table, 11);
    assert(program_worker_all_dead(&table, 11));
    assert(program_worker_wake(reused) == PROGRAM_WORKER_EXITED);
}

static void queue_and_admission(void)
{
    program_workers_init(&table);
    ProgramRecordQueue queue;
    program_worker_queue_init(&queue);
    ProgramWorker *worker = spawn(66, 606, 1);
    assert(program_worker_wake_queue(&queue, worker) == PROGRAM_WORKER_NEEDS_ADMISSION);
    assert(queue.count == 1 && worker->phase == PROGRAM_WORKER_QUEUED);
    assert(!program_worker_admit_queue(&queue, worker));
    assert(queue.count == 1);
    assert(program_worker_wake_queue(&queue, worker) == PROGRAM_WORKER_RECORDED);
    assert(queue.count == 1);
    assert(program_worker_dispatch_next(&queue, PROGRAM_WORKER_MAX_CORES) == NULL);
    assert(queue.count == 1 && worker->phase == PROGRAM_WORKER_QUEUED);
    assert(program_worker_dispatch_next(&queue, 2) == worker);
    assert(queue.count == 0 && worker->phase == PROGRAM_WORKER_DISPATCHING);
    assert(program_worker_attach(worker, 2));
    detach(worker, 2);
    assert(program_worker_wake_queue(&queue, worker) == PROGRAM_WORKER_NEEDS_ADMISSION);
    assert(program_worker_exit(worker));
    assert(program_worker_dispatch_next(&queue, 2) == NULL);
    assert(program_worker_dead(worker));
    assert(!program_worker_admit_queue(&queue, worker));

    ProgramRecordQueue full;
    program_worker_queue_init(&full);
    for (size_t i = 0; i < PROGRAM_RECORD_QUEUE_CAPACITY; ++i) {
        fake[i].phase = PROGRAM_WORKER_DETACHED;
        assert(program_worker_admit_queue(&full, &fake[i]));
    }
    ProgramWorker *extra = spawn(77, 707, 1);
    extra->pending = true;
    assert(!program_worker_admit_queue(&full, extra));
    assert(extra->pending && extra->phase == PROGRAM_WORKER_DETACHED);
    ProgramWorker *waiting = spawn(88, 808, 1);
    assert(program_worker_wake_queue(&full, waiting) == PROGRAM_WORKER_RETRY);
    assert(waiting->pending && waiting->phase == PROGRAM_WORKER_DETACHED);
    assert(!program_worker_poll_queue(&table, &full, 1));
    assert(extra->pending && extra->phase == PROGRAM_WORKER_DETACHED);
    ProgramWorker *first = (ProgramWorker *)full.entries[full.head];
    assert(program_worker_dispatch_next(&full, 1) == first);
    assert(!program_worker_poll_queue(&table, &full, 2));
    assert(extra->phase == PROGRAM_WORKER_QUEUED);
    assert(waiting->pending && waiting->phase == PROGRAM_WORKER_DETACHED);
    assert(program_worker_dispatch_next(&full, 1) != NULL);
    assert(program_worker_poll_queue(&table, &full, 3));
    assert(waiting->phase == PROGRAM_WORKER_QUEUED);
}

static void fault_blocked_admission(void)
{
    program_workers_init(&table);
    ProgramRecordQueue queue;
    program_worker_queue_init(&queue);
    ProgramWorker *worker = spawn(99, 909, 1);
    worker->fault_blocked = true;
    assert(program_worker_wake(worker) == PROGRAM_WORKER_RECORDED);
    assert(worker->pending && !program_worker_admit_queue(&queue, worker));
    assert(program_worker_poll_queue(&table, &queue, 2));
    assert(queue.count == 0 && worker->phase == PROGRAM_WORKER_DETACHED);
    worker->fault_blocked = false;
    assert(program_worker_poll_queue(&table, &queue, 3));
    assert(queue.count == 1 && worker->phase == PROGRAM_WORKER_QUEUED);
}

static void deadline_and_capacity(void)
{
    program_workers_init(&table);
    ProgramRecordQueue queue;
    program_worker_queue_init(&queue);
    ProgramWorker *worker = spawn(22, 202, 1);
    worker->wait_deadline = 3;
    program_worker_poll_queue(&table, &queue, 2);
    assert(!worker->pending && worker->wait_deadline == 3);
    program_worker_poll_queue(&table, &queue, 3);
    assert(worker->pending && worker->phase == PROGRAM_WORKER_QUEUED && queue.count == 1);
    assert(program_worker_dispatch_next(&queue, 1) == worker);
    assert(program_worker_exit(worker));
    assert(!program_worker_attach(worker, 1));
    assert(program_worker_dead(worker));

    for (unsigned i = 0; i < PROGRAM_WORKER_MAX - 1; ++i) spawn(33, 303 + i, 1);
    ProgramWorkerResult full = program_worker_spawn(&table, storage, 33, 999, 0x5000, 0x9000, 0, 1, 1);
    assert(full.status == PROGRAM_WORKER_FULL && !full.value);
    assert(!program_worker_all_dead(&table, 33));
    program_worker_end(&table, 33);
    program_worker_poll(&table, 10);
    assert(program_worker_all_dead(&table, 33));

    table.next_id = PROGRAM_WORKER_TASK_END - 1;
    ProgramWorkerResult last = program_worker_spawn(&table, storage, 44, 404, 0x5000, 0x9000, 0, 1, 14);
    assert(last.status == PROGRAM_WORKER_OK && last.value->id == PROGRAM_WORKER_TASK_END - 1);
    assert(table.next_id == PROGRAM_WORKER_TASK_END);
    ProgramWorkerResult exhausted = program_worker_spawn(&table, storage, 44, 405, 0x5000, 0x9000, 0, 1, 15);
    assert(exhausted.status == PROGRAM_WORKER_FULL && !exhausted.value);
}

static void refused_spawn_is_inert(uint32_t next_id, uint64_t now)
{
    static ProgramWorkers saved;
    static unsigned char saved_storage[sizeof storage];
    table.next_id = next_id;
    memcpy(&saved, &table, sizeof saved);
    memcpy(saved_storage, storage, sizeof storage);
    for (unsigned attempt = 0; attempt < 2; ++attempt) {
        ProgramWorkerResult result = program_worker_spawn(
            &table, storage, 44, 405, 0x5000, 0x9000, 0, 1, now);
        assert(result.status == PROGRAM_WORKER_FULL && !result.value);
        assert(!memcmp(&saved, &table, sizeof saved));
        assert(!memcmp(saved_storage, storage, sizeof storage));
    }
}

static void task_namespace(void)
{
    program_workers_init(&table);
    const uint32_t invalid[] = {0, PROGRAM_WORKER_TASK_FIRST - 1,
        PROGRAM_WORKER_TASK_END, PROGRAM_WORKER_TASK_END + 1, UINT32_MAX};
    for (size_t i = 0; i < sizeof invalid / sizeof *invalid; ++i)
        refused_spawn_is_inert(invalid[i], 1);
    table.next_id = PROGRAM_WORKER_TASK_FIRST;
    ProgramWorker *first = spawn(44, 401, 1);
    uint32_t old_id = first->id;
    assert(old_id == PROGRAM_WORKER_TASK_FIRST);
    assert(program_worker_exit(first));
    program_worker_poll(&table, 1);
    table.next_id = PROGRAM_WORKER_TASK_END - 1;
    ProgramWorker *last = spawn(44, 402, 5);
    assert(last == first && last->id == PROGRAM_WORKER_TASK_END - 1);
    assert(!program_worker_find(&table, old_id));
    assert(program_worker_find(&table, last->id) == last);
    assert(program_worker_exit(last));
    program_worker_poll(&table, 6);
    /* An eligible dead slot must not be cleared or rearmed on exhaustion. */
    refused_spawn_is_inert(PROGRAM_WORKER_TASK_END, 10);
}

static void invalid_transitions(void)
{
    program_workers_init(&table);
    ProgramWorkerResult bad = program_worker_spawn(&table, storage, 0, 1, 0, 0, 0, 0, 1);
    assert(bad.status == PROGRAM_WORKER_INVALID && !bad.value);
    ProgramWorker *worker = spawn(55, 505, 1);
    assert(!program_worker_begin_switch_out(worker, 0));
    assert(program_worker_park(worker, 0) == PROGRAM_WORKER_DENIED);
    assert(!program_worker_release_after_stack_switch(worker, 0));
    attach(worker, 0);
    assert(!program_worker_begin_switch_out(worker, 1));
    detach(worker, 0);
    assert(program_worker_exit(worker));
    assert(!program_worker_exit(worker));
    assert(program_worker_wake(worker) == PROGRAM_WORKER_EXITED);
}

static void handoff_admission(void)
{
    program_workers_init(&table);
    ProgramRecordQueue queue;
    program_worker_queue_init(&queue);
    ProgramWorker *worker = spawn(111, 1111, 1);
    program_record_queue_arm(&queue, true);
    assert(program_worker_admit_queue(&queue, worker));
    assert(queue.handoff == (uintptr_t)worker && !queue.count);
    assert(worker->phase == PROGRAM_WORKER_QUEUED);
    assert(program_worker_dispatch_next(&queue, 1) == worker);
    assert(!queue.handoff && worker->phase == PROGRAM_WORKER_DISPATCHING);

    program_worker_queue_init(&queue);
    for (size_t i = 0; i < PROGRAM_RECORD_HANDOFF_STREAK; ++i) {
        uintptr_t token = (uintptr_t)&fake[i & 1];
        program_record_queue_arm(&queue, true);
        assert(program_record_queue_admit(&queue, token));
        assert(program_record_queue_select(&queue) == token);
    }
    for (size_t i = 0; i < PROGRAM_RECORD_QUEUE_CAPACITY; ++i)
        assert(program_record_queue_push(&queue, (uintptr_t)&fake[i]));
    program_record_queue_arm(&queue, true);
    assert(program_record_queue_admit(&queue, (uintptr_t)worker));
    assert(program_record_queue_select(&queue) == (uintptr_t)&fake[0]);
    for (size_t i = 1; i < PROGRAM_RECORD_QUEUE_CAPACITY; ++i)
        assert(program_record_queue_select(&queue) == (uintptr_t)&fake[i]);
    assert(program_record_queue_select(&queue) == (uintptr_t)worker);
    assert(!queue.handoff && !queue.count);
}

int main(void)
{
    phase_and_rearm();
    queue_and_admission();
    handoff_admission();
    fault_blocked_admission();
    deadline_and_capacity();
    task_namespace();
    invalid_transitions();
    puts("program workers: admission, park/wake, stack departure, delayed rearm, fresh state, capacity and identity exhaustion passed");
    return 0;
}
