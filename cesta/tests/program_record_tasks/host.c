#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_kernel.h"
#include "program_record_tasks.h"

unsigned host_invalidations;
void *kmemset(void *destination, int value, size_t count) { return memset(destination, value, count); }

static ProgramRecordTask tasks[4];
static ProgramRecordTaskStorage storage[4];
static ProgramRecordQueue queue;

static void setup(void)
{
    memset(tasks, 0, sizeof tasks);
    memset(storage, 0xa5, sizeof storage);
    program_record_tasks_init(tasks, storage, 4);
    program_record_queue_init(&queue);
}

static void arm(size_t index, uint32_t id)
{
    memset(storage[index].stack, 0xcc, 64);
    assert(program_record_task_rearm(tasks, storage, 4, index, id,
                                     UINT64_C(0x4000) + id, UINT64_C(0x9003)));
    for (size_t i = 0; i < 64; ++i) assert(!storage[index].stack[i]);
    assert(tasks[index].phase == PROGRAM_WORKER_DETACHED && !tasks[index].queued);
    assert(storage[index].frame->rip == UINT64_C(0x4000) + id);
    assert(storage[index].frame->rsp == UINT64_C(0x8ff8));
    assert(storage[index].frame->cs == 0x2b && storage[index].frame->ss == 0x23);
    assert(tasks[index].kstack_top == (uint64_t)(uintptr_t)(storage[index].stack +
                                                            PROGRAM_RECORD_KSTACK_BYTES));
}

static void lifecycle_and_custody(void)
{
    setup();
    assert(!program_record_task_rearm(tasks, storage, 4, 0, 0, 0x4000, 0x9000));
    arm(0, PROGRAM_RECORD_TASK_FIRST);
    arm(1, PROGRAM_RECORD_TASK_FIRST + 1);
    assert(!program_record_task_rearm(tasks, storage, 4, 0, PROGRAM_RECORD_TASK_FIRST + 9,
                                      0x5000, 0x9000));
    assert(program_record_task_activate(&queue, &tasks[0]));
    assert(!program_record_task_activate(&queue, &tasks[0]) && queue.count == 1);
    assert(!program_record_task_rearm(tasks, storage, 4, 0, PROGRAM_RECORD_TASK_FIRST + 9,
                                      0x5000, 0x9000));
    tasks[0].phase = PROGRAM_WORKER_CONSTRUCTED;
    assert(!program_record_task_rearm(tasks, storage, 4, 0, PROGRAM_RECORD_TASK_FIRST + 9,
                                      0x5000, 0x9000));
    tasks[0].phase = PROGRAM_WORKER_QUEUED;
    assert(program_record_task_activate(&queue, &tasks[1]));
    assert(program_record_task_next(&queue, 0) == &tasks[0]);
    assert(tasks[0].phase == PROGRAM_WORKER_DISPATCHING && tasks[0].owner_core == 0);
    assert(program_record_task_attach(&tasks[0], 0));
    assert(!program_record_task_attach(&tasks[0], 0));
    assert(program_record_task_next(&queue, 0) == &tasks[1]);
    assert(program_record_task_attach(&tasks[1], 0));
    assert(!program_record_task_release_after_stack_switch(&tasks[0], 0));
    assert(program_record_task_begin_switch_out(&tasks[0], 0));
    assert(program_record_task_release_after_stack_switch(&tasks[0], 0));
    assert(tasks[0].phase == PROGRAM_WORKER_DETACHED &&
           tasks[0].owner_core == PROGRAM_RECORD_NO_CORE);
    assert(program_record_task_activate(&queue, &tasks[0]));
    assert(program_record_task_exit(&tasks[1]));
    assert(!program_record_task_exit(&tasks[1]));
    assert(program_record_task_begin_switch_out(&tasks[1], 0));
    assert(program_record_task_release_after_stack_switch(&tasks[1], 0));
    assert(tasks[1].phase == PROGRAM_WORKER_DEAD &&
           tasks[1].owner_core == PROGRAM_RECORD_NO_CORE);
    assert(program_record_task_next(&queue, 0) == &tasks[0]);
    assert(program_record_task_attach(&tasks[0], 0));
    assert(!program_record_task_next(&queue, 1));
    assert(program_record_task_exit(&tasks[0]));
    assert(program_record_task_begin_switch_out(&tasks[0], 0));
    assert(program_record_task_release_after_stack_switch(&tasks[0], 0));

    setup();
    arm(0, PROGRAM_RECORD_TASK_FIRST);
    arm(1, PROGRAM_RECORD_TASK_FIRST + 1);
    assert(program_record_task_activate(&queue, &tasks[0]));
    assert(program_record_task_activate(&queue, &tasks[1]));
    assert(program_record_task_exit(&tasks[0]));
    assert(program_record_task_next(&queue, 0) == &tasks[1]);
    assert(tasks[0].phase == PROGRAM_WORKER_DEAD && !queue.count);
    assert(program_record_task_rearm(tasks, storage, 4, 0, PROGRAM_RECORD_TASK_FIRST + 7,
                                     0x7000, 0x9000));
    for (size_t i = 0; i < 64; ++i) assert(!storage[0].stack[i]);
}

static void wait_wake_and_backpressure(void)
{
    setup();
    arm(0, PROGRAM_RECORD_TASK_FIRST);
    assert(program_record_task_activate(&queue, &tasks[0]));
    assert(program_record_task_next(&queue, 0) == &tasks[0]);
    assert(program_record_task_attach(&tasks[0], 0));
    assert(program_record_task_park(&tasks[0], 0, 10, 5) == PROGRAM_RECORD_BLOCKED);
    assert(tasks[0].wait_deadline == 15);
    assert(program_record_task_begin_switch_out(&tasks[0], 0));
    assert(program_record_task_release_after_stack_switch(&tasks[0], 0));
    assert(program_record_task_poll(tasks, &queue, 4, 14) && !queue.count);
    assert(program_record_task_poll(tasks, &queue, 4, 15) && queue.count == 1);
    assert(!tasks[0].blocked && !tasks[0].pending);
    assert(program_record_task_next(&queue, 0) == &tasks[0]);
    assert(program_record_task_attach(&tasks[0], 0));
    assert(program_record_task_park(&tasks[0], 0, 20, 0) == PROGRAM_RECORD_BLOCKED);
    assert(program_record_task_wake(&queue, &tasks[0]) == PROGRAM_RECORD_RECORDED);
    assert(program_record_task_park(&tasks[0], 0, 20, 0) == PROGRAM_RECORD_WAKE_CONSUMED);
    assert(program_record_task_begin_switch_out(&tasks[0], 0));
    assert(program_record_task_release_after_stack_switch(&tasks[0], 0));
    assert(program_record_task_wake(&queue, &tasks[0]) == PROGRAM_RECORD_NEEDS_ADMISSION);
    assert(program_record_task_next(&queue, 0) == &tasks[0]);
    assert(program_record_task_attach(&tasks[0], 0));
    assert(program_record_task_park(&tasks[0], 1, 20, 0) == PROGRAM_RECORD_DENIED);
    assert(program_record_task_park(&tasks[0], 0, 20, 0) == PROGRAM_RECORD_WAKE_CONSUMED);
    assert(program_record_task_park(&tasks[0], 0, UINT64_MAX - 1, 5) == PROGRAM_RECORD_BLOCKED);
    assert(tasks[0].wait_deadline == UINT64_MAX);

    setup();
    for (size_t i = 0; i < 3; ++i) arm(i, PROGRAM_RECORD_TASK_FIRST + i);
    assert(program_record_task_activate(&queue, &tasks[0]));
    assert(program_record_task_activate(&queue, &tasks[1]));
    assert(!program_record_task_activate(&queue, &tasks[2]) && tasks[2].admission_pending);
    assert(program_record_task_wake(&queue, &tasks[2]) == PROGRAM_RECORD_RETRY);
    assert(!program_record_task_poll(tasks, &queue, 4, 0));
    assert(program_record_task_next(&queue, 0) == &tasks[0]);
    assert(program_record_task_poll(tasks, &queue, 4, 0) && queue.count == 2);
    assert(program_record_task_next(&queue, 0) == &tasks[1]);
    assert(program_record_task_next(&queue, 0) == &tasks[2]);
    assert(!program_record_task_next(&queue, 0));
}

static void generic_tokens(void)
{
    setup();
    assert(program_record_queue_push(&queue, 1));
    assert(program_record_queue_pop(&queue) == 1);
    assert(program_record_queue_push(&queue, 1));
    assert(program_record_queue_push(&queue, 2));
    assert(!program_record_queue_push(&queue, 3));
    assert(!program_record_queue_push(&queue, 2));
    assert(program_record_queue_pop(&queue) == 1);
    assert(program_record_queue_pop(&queue) == 2);
    assert(!program_record_queue_pop(&queue));
}

static void handoff_policy(void)
{
    setup();
    for (size_t i = 0; i < PROGRAM_RECORD_HANDOFF_STREAK; ++i) {
        uintptr_t token = 0x1000 + (i & 1);
        program_record_queue_arm(&queue, true);
        assert(program_record_queue_admit(&queue, token));
        assert(queue.handoff == token);
        assert(program_record_queue_select(&queue) == token);
        assert(queue.handoff_streak == i + 1 && !queue.handoff);
    }
    assert(program_record_queue_push(&queue, 1));
    assert(program_record_queue_push(&queue, 2));
    program_record_queue_arm(&queue, true);
    assert(program_record_queue_admit(&queue, 0x2000));
    assert(queue.handoff == 0x2000 && queue.count == 2);
    assert(program_record_queue_select(&queue) == 1);
    assert(!queue.handoff && !queue.handoff_streak && queue.count == 2);
    assert(program_record_queue_select(&queue) == 2);
    assert(program_record_queue_select(&queue) == 0x2000);
    assert(!program_record_queue_select(&queue));

    program_record_queue_arm(&queue, true);
    assert(program_record_queue_admit(&queue, 0x3000));
    assert(!program_record_queue_admit(&queue, 0x3000));
    assert(!program_record_queue_push(&queue, 0x3000));
    assert(program_record_queue_select(&queue) == 0x3000);
    assert(!program_record_queue_admit(&queue, 0));

    setup();
    for (size_t i = 0; i < PROGRAM_RECORD_HANDOFF_STREAK; ++i) {
        program_record_queue_arm(&queue, true);
        assert(program_record_queue_admit(&queue, 0x4000 + i));
        assert(program_record_queue_select(&queue) == 0x4000 + i);
    }
    program_record_queue_arm(&queue, true);
    assert(program_record_queue_admit(&queue, 0x5000));
    assert(program_record_queue_select(&queue) == 0x5000);
    assert(!queue.handoff && !queue.handoff_streak);
}

int main(void)
{
    generic_tokens();
    handoff_policy();
    lifecycle_and_custody();
    wait_wake_and_backpressure();
    puts("program record tasks: rearm, bounded queue, custody, exit, wait/wake and backpressure passed");
    return 0;
}
