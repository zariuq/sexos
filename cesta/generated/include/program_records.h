
#ifndef SEXOS_PROGRAM_RECORDS_H
#define SEXOS_PROGRAM_RECORDS_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "program_image.h"

#define PROGRAM_RECORD_BASE 1024u
#define PROGRAM_RECORD_MAX_CAPACITY 4096u
#ifndef PROGRAM_RECORD_CAPACITY
#define PROGRAM_RECORD_CAPACITY 4096u
#endif
#ifndef PROGRAM_RECORD_POOL_FRAMES
#define PROGRAM_RECORD_POOL_FRAMES 262144u
#endif
#define PROGRAM_RECORD_PAGE_BYTES 4096u
#define PROGRAM_RECORD_WINDOW_PAGES 256u
#define PROGRAM_RECORD_WINDOW_BYTES (PROGRAM_RECORD_WINDOW_PAGES * UINT64_C(4096))
#define PROGRAM_RECORD_STACK_FIRST (PROGRAM_RECORD_WINDOW_PAGES + 1u)
#define PROGRAM_RECORD_STACK_PAGES 4u
#define PROGRAM_RECORD_LEAF_COUNT (PROGRAM_RECORD_STACK_FIRST + PROGRAM_RECORD_STACK_PAGES)
#define PROGRAM_RECORD_ARENA UINT64_C(0x600000000000)
#define PROGRAM_RECORD_SLICE UINT64_C(0x200000)
#define PROGRAM_RECORD_START_BATCH 16u
#define PROGRAM_RECORD_REAP_BATCH 4u
#define PROGRAM_RECORD_REAP_SCAN 64u
#define PROGRAM_RECORD_TASK_FIRST UINT32_C(0x80000000)
#define PROGRAM_RECORD_NO_CORE UINT32_MAX
#define PROGRAM_RECORD_TASK_DEAD 6u

#define PROGRAM_RECORD_SLOT_SHELL 257u
#define PROGRAM_RECORD_FILES_ROUTE_SLOT 256u
#define PROGRAM_RECORD_IMAGE_SLOT_FILES 100u
#define PROGRAM_RECORD_IMAGE_REGION_ID 0x4000u
#define PROGRAM_RECORD_POOL_REGION_ID 0x6000u
#define PROGRAM_RECORD_IMAGE_BYTES UINT64_C(33554432)
#define PROGRAM_RECORD_SYSCALL 79u
#define PROGRAM_RECORD_START 0u
#define PROGRAM_RECORD_STOP 1u
#define PROGRAM_RECORD_CENSUS 2u
#define PROGRAM_RECORD_STOP_REQUESTED 3u

enum {
    PROGRAM_RECORD_OK = 0,
    PROGRAM_RECORD_INVALID = 1,
    PROGRAM_RECORD_RETRY = 2,
    PROGRAM_RECORD_DENIED = 3,
    PROGRAM_RECORD_FULL = 4
};

enum {
    PROGRAM_RECORD_FREE = 0,
    PROGRAM_RECORD_LIVE = 1,
    PROGRAM_RECORD_LOADING = 2
};

typedef struct {
    uint32_t frame;
    bool allocated, present, writable, executable;
} ProgramRecordPage;

typedef struct {
    ProgramRecordPage pages[PROGRAM_RECORD_LEAF_COUNT];
    uint32_t task, image_pages;
    uint8_t state;
    bool killed, reply_pending;
} ProgramRecord;

typedef struct {
    ProgramRecord entries[PROGRAM_RECORD_CAPACITY];
    uint32_t pool[PROGRAM_RECORD_POOL_FRAMES];
    uint64_t physical_base, started, ended, switches;
    size_t count, pool_free, pool_total, tenant, live, start_cursor, reap_cursor;
    uint32_t next_task;
    bool stop;
} ProgramRecords;

typedef struct {
    size_t count;
    uint64_t pool_bytes;
} ProgramRecordSizing;

typedef struct {
    size_t image_pages, needed;
} ProgramRecordPlan;

typedef struct {
    uint64_t status, first, second;
} ProgramRecordReply;

typedef struct {
    uint32_t id, owner_core;
    uint8_t phase;
    bool exited;
    uint64_t kstack_top, wait_deadline, fs_base;
    uint32_t pd;
    bool queued, pending, admission_pending, blocked;
} ProgramRecordTask;

ProgramRecordSizing program_records_sizing(uint64_t free_bytes);
uint64_t program_records_init(ProgramRecords *table, size_t count,
                              uint64_t physical_base, size_t pool_frames);
uint64_t program_records_plan(const ProgramImage *image, ProgramRecordPlan *plan);
ProgramRecordReply program_record_begin(ProgramRecords *table, const ProgramImage *image);
ProgramRecordReply program_record_commit(ProgramRecords *table, uint64_t pd, uint32_t task);
ProgramRecordReply program_record_abort(ProgramRecords *table, uint64_t pd, uint32_t task);
ProgramRecordReply program_records_start(ProgramRecords *table,
                                         const ProgramImage *image, uint64_t count);
ProgramRecordReply program_records_stop(ProgramRecords *table, bool force);
ProgramRecordReply program_records_census(const ProgramRecords *table);
bool program_records_is_record(const ProgramRecords *table, uint64_t pd);
bool program_records_stop_requested(const ProgramRecords *table, uint64_t pd);
uint64_t program_records_kill(ProgramRecords *table, uint64_t pd, uint32_t task);
void program_records_evict(ProgramRecords *table);
uint64_t program_records_before_dispatch(ProgramRecords *table, uint64_t pd, uint32_t task);
ProgramRecordReply program_records_reap(ProgramRecords *table,
                                        const ProgramRecordTask *tasks);
bool program_records_audit(const ProgramRecords *table);
uint64_t program_record_virtual_page(size_t index, size_t page);
uint64_t program_record_stack_top(size_t index);
#endif
