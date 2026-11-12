
#ifndef SEXOS_PROGRAM_RECORD_TASKS_H
#define SEXOS_PROGRAM_RECORD_TASKS_H
#include "program_records.h"

#define PROGRAM_RECORD_KSTACK_BYTES 16384u
#ifndef PROGRAM_RECORD_QUEUE_CAPACITY
#define PROGRAM_RECORD_QUEUE_CAPACITY 512u
#endif
#define PROGRAM_RECORD_MAX_CORES 128u
#define PROGRAM_RECORD_HANDOFF_STREAK 8u

enum {
    PROGRAM_RECORD_BLOCKED = 20,
    PROGRAM_RECORD_WAKE_CONSUMED = 21,
    PROGRAM_RECORD_NEEDS_ADMISSION = 22,
    PROGRAM_RECORD_EXITED = 23,
    PROGRAM_RECORD_RECORDED = 24
};

typedef struct TrapFrame TrapFrame;
typedef struct {
    TrapFrame *frame;
    uint8_t stack[PROGRAM_RECORD_KSTACK_BYTES] __attribute__((aligned(16)));
    uint8_t fpu[512] __attribute__((aligned(16)));
} ProgramRecordTaskStorage;

typedef struct {
    uintptr_t entries[PROGRAM_RECORD_QUEUE_CAPACITY];
    size_t head, count;
    uintptr_t handoff;
    uint32_t handoff_streak;
    bool handoff_armed;
} ProgramRecordQueue;

bool program_record_queue_push(ProgramRecordQueue *queue, uintptr_t token);
uintptr_t program_record_queue_pop(ProgramRecordQueue *queue);
void program_record_queue_arm(ProgramRecordQueue *queue, bool enabled);
bool program_record_queue_admit(ProgramRecordQueue *queue, uintptr_t token);
uintptr_t program_record_queue_select(ProgramRecordQueue *queue);
bool program_record_task_begin_dispatch(ProgramRecordTask *task, uint32_t core);
void program_record_tasks_init(ProgramRecordTask *tasks, ProgramRecordTaskStorage *storage,
                               size_t count);
bool program_record_task_rearm(ProgramRecordTask *tasks, ProgramRecordTaskStorage *storage,
                               size_t count, size_t index, uint32_t task,
                               uint64_t entry, uint64_t stack_top);
void program_record_queue_init(ProgramRecordQueue *queue);
bool program_record_task_activate(ProgramRecordQueue *queue, ProgramRecordTask *task);
ProgramRecordTask *program_record_task_next(ProgramRecordQueue *queue, uint32_t core);
bool program_record_task_attach(ProgramRecordTask *task, uint32_t core);
bool program_record_task_begin_switch_out(ProgramRecordTask *task, uint32_t core);
bool program_record_task_release_after_stack_switch(ProgramRecordTask *task, uint32_t core);
bool program_record_task_exit(ProgramRecordTask *task);
uint64_t program_record_task_wake(ProgramRecordQueue *queue, ProgramRecordTask *task);
uint64_t program_record_task_park(ProgramRecordTask *task, uint32_t core,
                                  uint64_t now, uint64_t ticks);
bool program_record_task_poll(ProgramRecordTask *tasks, ProgramRecordQueue *queue,
                              size_t count, uint64_t now);
#endif
