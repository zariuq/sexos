
#ifndef SEXOS_PROGRAM_WORKERS_H
#define SEXOS_PROGRAM_WORKERS_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef struct TrapFrame TrapFrame;

#define PROGRAM_WORKER_MAX 63u
#define PROGRAM_WORKER_MAX_CORES 128u
#define PROGRAM_WORKER_REUSE_TICKS 4u
#define PROGRAM_WORKER_STACK_BYTES 65536u
#define PROGRAM_WORKER_FPU_BYTES 512u
#define PROGRAM_WORKER_NO_CORE UINT32_MAX
/* Main tasks use [1, FIRST); workers use [FIRST, END). Neither counter wraps. */
#define PROGRAM_WORKER_TASK_FIRST UINT32_C(0x40000000)
#define PROGRAM_WORKER_TASK_END UINT32_C(0x80000000)

enum {
    PROGRAM_WORKER_OK = 0,
    PROGRAM_WORKER_INVALID = 1,
    PROGRAM_WORKER_RETRY = 2,
    PROGRAM_WORKER_DENIED = 3,
    PROGRAM_WORKER_FULL = 6,
    PROGRAM_WORKER_AGAIN = 8,
    PROGRAM_WORKER_RECORDED = 9,
    PROGRAM_WORKER_NEEDS_ADMISSION = 10,
    PROGRAM_WORKER_BLOCKED = 11,
    PROGRAM_WORKER_WAKE_CONSUMED = 12,
    PROGRAM_WORKER_EXITED = 13
};

enum {
    PROGRAM_WORKER_CONSTRUCTED = 0,
    PROGRAM_WORKER_DETACHED = 1,
    PROGRAM_WORKER_QUEUED = 2,
    PROGRAM_WORKER_DISPATCHING = 3,
    PROGRAM_WORKER_ATTACHED = 4,
    PROGRAM_WORKER_SWITCHING_OUT = 5,
    PROGRAM_WORKER_DEAD = 6
};

typedef struct ProgramWorker {
    uint64_t kstack_top, dead_since, wait_deadline, wait_address, fs_base;
    uint32_t id, target, run, home, owner_core, phase, slot;
    bool present, pending, admission_pending, blocked, fault_blocked, exited;
    bool memory_accounted, memory_released;
} ProgramWorker;

typedef struct {
    ProgramWorker entries[PROGRAM_WORKER_MAX];
    size_t count;
    uint32_t next_id;
} ProgramWorkers;

#include "program_record_tasks.h"

typedef struct {
    uint64_t status;
    ProgramWorker *value;
} ProgramWorkerResult;

void program_workers_init(ProgramWorkers *table);
void program_worker_queue_init(ProgramRecordQueue *queue);
bool program_worker_admit_queue(ProgramRecordQueue *queue, ProgramWorker *worker);
ProgramWorker *program_worker_dispatch_next(ProgramRecordQueue *queue, uint32_t core);
uint64_t program_worker_wake_queue(ProgramRecordQueue *queue, ProgramWorker *worker);
bool program_worker_poll_queue(ProgramWorkers *table, ProgramRecordQueue *queue, uint64_t now);
ProgramWorkerResult program_worker_spawn(ProgramWorkers *table, struct ProgramWorkerStorage *storage,
                                         uint32_t target, uint32_t run, uint64_t entry,
                                         uint64_t stack, uint64_t arg, uint32_t home, uint64_t now);
bool program_worker_admit(ProgramWorker *worker);
bool program_worker_begin_dispatch(ProgramWorker *worker, uint32_t core);
bool program_worker_attach(ProgramWorker *worker, uint32_t core);
bool program_worker_begin_switch_out(ProgramWorker *worker, uint32_t core);
bool program_worker_release_after_stack_switch(ProgramWorker *worker, uint32_t core);
bool program_worker_exit(ProgramWorker *worker);
uint64_t program_worker_wake(ProgramWorker *worker);
uint64_t program_worker_park(ProgramWorker *worker, uint32_t core);
void program_worker_poll(ProgramWorkers *table, uint64_t now);
void program_worker_end(ProgramWorkers *table, uint32_t target);
bool program_worker_all_dead(const ProgramWorkers *table, uint32_t target);
ProgramWorker *program_worker_find(ProgramWorkers *table, uint32_t id);
bool program_worker_dead(const ProgramWorker *worker);
#endif
