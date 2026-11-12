
#ifndef SEXOS_PROGRAM_RECORD_MEMORY_H
#define SEXOS_PROGRAM_RECORD_MEMORY_H
#include "program_records.h"

#define PROGRAM_RECORD_KEY 10u

typedef struct {
    uint64_t leaves[PROGRAM_RECORD_CAPACITY][PROGRAM_RECORD_LEAF_COUNT];
    uint8_t key;
} ProgramRecordMemory;

typedef struct {
    uint64_t status;
    uint32_t pd, task;
    uint64_t entry, stack_top;
} ProgramRecordLoad;

bool program_record_memory_init(ProgramRecordMemory *memory, uint8_t key);
ProgramRecordLoad program_record_memory_begin(ProgramRecords *table, ProgramRecordMemory *memory,
                                              const ProgramImage *image);
ProgramRecordReply program_record_memory_commit(ProgramRecords *table, ProgramRecordMemory *memory,
                                                uint64_t pd, uint32_t task);
ProgramRecordReply program_record_memory_cancel(ProgramRecords *table, ProgramRecordMemory *memory,
                                                uint64_t pd, uint32_t task);
ProgramRecordLoad program_record_memory_start(ProgramRecords *table, ProgramRecordMemory *memory,
                                              const ProgramImage *image);
uint64_t program_record_memory_before_dispatch(ProgramRecords *table, ProgramRecordMemory *memory,
                                               uint64_t pd, uint32_t task);
uint64_t program_record_memory_evict(ProgramRecords *table, ProgramRecordMemory *memory);
ProgramRecordReply program_record_memory_reap(ProgramRecords *table, ProgramRecordMemory *memory,
                                              const ProgramRecordTask *tasks);
bool program_record_memory_audit(ProgramRecords *table, ProgramRecordMemory *memory);
#endif
