/* Main-only host scheduler adapters; never linked with kernel_workers.c. */
#include "sexos_kernel.h"
#include <stdlib.h>

#ifndef SEXOS_WORKER_STUBS_NO_PLATFORM
void tss_set_rsp0(uint64_t top) { (void)top; }
_Noreturn void kernel_idle(void) { abort(); }
void kernel_serial_write(const uint8_t *bytes, size_t count) { (void)bytes; (void)count; }
#endif

ProgramWorker *kernel_current_worker;
ProgramWorker *kernel_outgoing_worker;

#ifndef SEXOS_EPOCH_LOG_HOST_REAL
bool kernel_epoch_log_ready(uint64_t expected) { (void)expected; return true; }
bool kernel_epoch_log_append(uint64_t next, const KernelEpochEvent *event)
{
    (void)next; (void)event; return true;
}
bool kernel_epoch_log_internal(void) { return false; }
bool kernel_epoch_log_eligible(uint32_t id) { (void)id; return true; }
bool kernel_epoch_log_flush(Domain *d, bool enter) { (void)d; (void)enter; return false; }
void kernel_epoch_log_poll(void) {}
#endif

#if !defined(SEXOS_DEVICE_RESET_HOST_REAL) && !defined(SEXOS_DEVICE_INTERRUPT_HOST_REAL)
bool kernel_device_interrupt_retry(uint32_t vector) { (void)vector; return true; }
void kernel_device_interrupt_drain(Domain *d) { (void)d; }
#endif

void kernel_workers_boot(void) {}
bool kernel_worker_entry(TrapFrame *frame) { (void)frame; return false; }
bool kernel_worker_token(uintptr_t token) { (void)token; return false; }
ProgramWorker *kernel_worker_dispatch(uintptr_t token) { (void)token; return NULL; }
void kernel_worker_stack_departed(void) {}
void kernel_worker_poll(void) {}
#if !SEXOS_KERNEL_CURRENT_PDX
void kernel_task_poll(void) {}
#endif
void kernel_worker_end(Domain *d) { (void)d; }
void kernel_worker_memory_end(Domain *d) { (void)d; }
void kernel_worker_fault(Domain *d) { (void)d; }
bool kernel_worker_all_dead(uint32_t target) { (void)target; return true; }
ProgramWorkerStorage *kernel_worker_storage(ProgramWorker *worker) { (void)worker; return NULL; }
bool kernel_worker_fault_begin(Domain *d) { (void)d; return false; }
bool kernel_worker_fault_matches(Domain *d, const PgFaultRecord *record)
{
    return d && record && record->target_pd == d->id && record->task_id == d->task_id &&
           d->frame && record->saved_frame == (uint64_t)(uintptr_t)d->frame &&
           record->saved_rip == d->frame->rip;
}
bool kernel_worker_fault_complete(Domain *d, const PgFaultRecord *record)
{
    (void)d; (void)record; return false;
}
bool kernel_worker_fault_reject(Domain *d, const PgFaultRecord *record)
{
    (void)d; (void)record; return false;
}

TrapFrame *kernel_worker_schedule(TrapFrame *frame, bool resume_current)
{
    uintptr_t next = run_pop();
    if (next) {
        if (kernel_current_record)
            (void)kernel_record_outgoing(frame);
        else if (resume_current)
            run_push(kernel.current);
        return switch_to(next);
    }
    if (resume_current) return resume_frame(frame);
    kernel_idle();
}

TrapFrame *kernel_worker_yield(void)
{
    if (kernel_current_record)
        (void)kernel_record_outgoing(kernel_record_storage(kernel_current_record)->frame);
    else
        run_push(kernel.current);
    return switch_to(run_pop());
}

TrapFrame *kernel_worker_syscall(Domain *d, TrapFrame *frame)
{
    (void)d;
    frame->rax = NO_ADDRESS;
    frame->rsi = 0;
    return frame;
}
