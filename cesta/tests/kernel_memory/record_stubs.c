#include "sexos_kernel.h"

#ifndef SEXOS_RECORD_HOST_REAL
ProgramRecords kernel_program_records;
ProgramRecordMemory kernel_program_record_memory;
ProgramRecordTask *kernel_current_record;
ProgramRecordTask *kernel_outgoing_record;

#ifndef SEXOS_RECORD_STUBS_NO_KMEMSET
void *kmemset(void *destination, int value, size_t count)
{
    uint8_t *bytes = destination;
    for (size_t i = 0; i < count; ++i) bytes[i] = (uint8_t)value;
    return destination;
}
#endif

bool program_records_is_record(const ProgramRecords *table, uint64_t pd)
{
    (void)table;
    (void)pd;
    return false;
}

bool kernel_record_run_push(uint32_t id) { (void)id; return false; }
ProgramRecordTask *kernel_record_dispatch(uintptr_t id) { (void)id; return NULL; }
ProgramRecordTaskStorage *kernel_record_storage(ProgramRecordTask *task) { (void)task; return NULL; }
TrapFrame *kernel_record_switch(uintptr_t id) { (void)id; return NULL; }
bool kernel_record_outgoing(TrapFrame *frame) { (void)frame; return false; }
void kernel_record_stack_departed(void) {}
TrapFrame *kernel_record_pdx_call(TrapFrame *frame) { return frame; }
TrapFrame *kernel_record_syscall(TrapFrame *frame) { return frame; }
TrapFrame *kernel_record_sleep(TrapFrame *frame) { return frame; }
TrapFrame *kernel_record_finish(TrapFrame *frame, bool fault) { (void)fault; return frame; }
void kernel_records_poll(void) {}
void kernel_record_fpu_save(void) {}
void kernel_record_fpu_restore(void) {}
#endif
