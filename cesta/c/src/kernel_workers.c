#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif
#include "sexos_kernel.h"
#include "program_workers.h"

#define PROGRAM_THREAD_SPAWN 1u
#define PROGRAM_THREAD_EXIT 2u
#define PROGRAM_THREAD_WAIT 3u
#define PROGRAM_THREAD_WAKE 4u
#define PROGRAM_THREAD_CORES 5u
#define PROGRAM_THREAD_TLS 6u
#define PROGRAM_THREAD_TICK_LIMIT 64u

ProgramWorkers program_workers;
struct ProgramWorkerStorage program_worker_storage[PROGRAM_WORKER_MAX];
ProgramWorker *kernel_current_worker;
ProgramWorker *kernel_outgoing_worker;

struct ProgramWorkerStorage *kernel_worker_storage(ProgramWorker *worker) {
    if (!worker || worker->slot >= PROGRAM_WORKER_MAX) return NULL;
    return &program_worker_storage[worker->slot];
}

void kernel_workers_boot(void) {
    program_workers_init(&program_workers);
    program_worker_queue_init(&kernel.run_queue);
    kernel_current_worker = NULL;
    kernel_outgoing_worker = NULL;
    kernel_current_record = NULL;
    kernel_outgoing_record = NULL;
}

static bool worker_live(Domain *d) {
    return d && d->task_id && d->task_state != TASK_EXITED && d->task_state != TASK_DORMANT &&
           kernel_current_task(d) != 0;
}

bool kernel_worker_entry(TrapFrame *frame) {
    if (!kernel_current_worker) return false;
    *kernel_worker_storage(kernel_current_worker)->frame = *frame;
    return true;
}

static void worker_memory_dead(ProgramWorker *worker) {
    if (!worker->memory_accounted || worker->memory_released || !program_worker_dead(worker))
        return;
    Domain *d = domain_get(worker->target);
    if (!d || d->program.run != worker->run ||
        program_memory_worker_exit(&d->program) != PROGRAM_MEMORY_OK)
        kernel_halt("worker memory lifecycle");
    worker->memory_released = true;
}

bool kernel_worker_token(uintptr_t token) {
    uintptr_t base = (uintptr_t)program_workers.entries;
    return token >= base && token < base + sizeof(program_workers.entries) &&
           (token - base) % sizeof(ProgramWorker) == 0;
}

ProgramWorker *kernel_worker_dispatch(uintptr_t token) {
    if (!kernel_worker_token(token)) return NULL;
    ProgramWorker *worker = (ProgramWorker *)token;
    if (program_worker_begin_dispatch(worker, 0)) return worker;
    worker_memory_dead(worker);
    return NULL;
}

static TrapFrame *worker_switch(ProgramWorker *worker) {
    Domain *d = domain_get(worker->target);
    if (!d || d->task_state == TASK_EXITED || d->task_state == TASK_DORMANT ||
        !program_worker_attach(worker, 0))
        kernel_halt("worker dispatch without live custody");
    kernel_current_worker = worker;
    kernel.current = worker->target;
    tss_set_rsp0(worker->kstack_top);
    kernel_syscall_stack_top = worker->kstack_top;
    cpu_wrmsr(MSR_FS_BASE, worker->fs_base);
    if (kernel.pku) cpu_wrpkru(d->mask);
#if SEXOS_KERNEL_CURRENT_FPU
    kernel_fpu_restore(d);
#endif
    return kernel_worker_storage(worker)->frame;
}

static void worker_outgoing(TrapFrame *frame) {
    ProgramWorker *worker = kernel_current_worker;
    if (!worker) return;
    *kernel_worker_storage(worker)->frame = *frame;
    if (!program_worker_begin_switch_out(worker, 0) || kernel_outgoing_worker)
        kernel_halt("worker stack departure custody");
    kernel_outgoing_worker = worker;
    kernel_current_worker = NULL;
}

void kernel_worker_stack_departed(void) {
    if (kernel_outgoing_record) {
        if (kernel_outgoing_worker) kernel_halt("record and worker departure collision");
        kernel_record_stack_departed();
    }
    ProgramWorker *worker = kernel_outgoing_worker;
    if (!worker) return;
    bool requeue = !worker->exited && (!worker->blocked || worker->pending);
    kernel_outgoing_worker = NULL;
    if (!program_worker_release_after_stack_switch(worker, 0))
        kernel_halt("worker release before stack departure");
    worker_memory_dead(worker);
    if (requeue && !program_worker_admit_queue(&kernel.run_queue, worker))
        worker->admission_pending = true;
}

TrapFrame *kernel_worker_schedule(TrapFrame *frame, bool resume_current) {
    uintptr_t next = run_pop();
    ProgramWorker *next_worker = kernel_worker_token(next) ? (ProgramWorker *)next : NULL;
    if (!next) {
        if (resume_current) return resume_frame(frame);
        if (kernel_current_worker) {
            worker_outgoing(frame);
        } else if (kernel_current_record) {
            (void)kernel_record_outgoing(frame);
        } else {
            Domain *current = domain_get(kernel.current);
            if (current) current->frame = frame;
        }
        kernel_idle();
    }
    if (kernel_current_worker) {
        worker_outgoing(frame);
    } else if (kernel_current_record) {
        (void)kernel_record_outgoing(frame);
    } else {
        Domain *current = domain_get(kernel.current);
        if (current) {
            current->frame = frame;
            if (current->task_state == TASK_RUNNING) run_push(current->id);
        }
    }
    return next_worker ? worker_switch(next_worker) : switch_to(next);
}

TrapFrame *kernel_worker_yield(void) {
    Domain *current = domain_get(kernel.current);
    TrapFrame *frame = kernel_current_record ? kernel_record_storage(kernel_current_record)->frame :
                       kernel_current_worker ? kernel_worker_storage(kernel_current_worker)->frame :
                       current ? current->frame : NULL;
    if (!frame) kernel_halt("worker yield without a saved frame");
    return kernel_worker_schedule(frame, true);
}

void kernel_worker_poll(void) {
    (void)program_worker_poll_queue(&program_workers, &kernel.run_queue, kernel.ticks);
}

void kernel_worker_memory_end(Domain *d) {
    if (!d || d->task_state != TASK_EXITED) return;
    if (d->program.phase == PROGRAM_MEMORY_LIVE && d->program.main_alive &&
        program_memory_main_exit(&d->program) != PROGRAM_MEMORY_OK)
        kernel_halt("program main memory lifecycle");
    if (d->program.phase == PROGRAM_MEMORY_FREE || d->program.phase == PROGRAM_MEMORY_DEAD ||
        !kernel_worker_all_dead(d->id) || kernel.current == d->id || d->queued ||
        !kernel_paging_cleanup_complete(d->id))
        return;
    KernelAppPrivate *app = kernel_app_get(d->id);
    if (app && (app->task != d->task_id || !app->retired || app->acked != APP_PRIVATE_REQUIRED))
        return;
    uint64_t status = program_memory_end(&d->program);
    if (status == PROGRAM_MEMORY_RETRY) return;
    if (status != PROGRAM_MEMORY_OK) kernel_halt("program memory release");
    klog("[program.memory.released] target=%u run=%u workers=0 main=0\n", d->id, d->program.run);
}

void kernel_worker_end(Domain *d) {
    if (!d) return;
    for (size_t i = 0; i < program_workers.count; ++i) {
        ProgramWorker *worker = &program_workers.entries[i];
        if (worker->target != d->id) continue;
        (void)program_worker_exit(worker);
        worker_memory_dead(worker);
    }
    kernel_worker_memory_end(d);
}

void kernel_worker_fault(Domain *d) {
    if (!kernel_current_worker || !d) return;
    klog("[program.worker.fault] target=%u worker=%u run_ended=1\n",
         d->id, kernel_current_worker->id);
    (void)program_worker_exit(kernel_current_worker);
    d->task_state = TASK_EXITED;
    kernel_worker_end(d);
}

bool kernel_worker_fault_begin(Domain *d) {
    ProgramWorker *worker = kernel_current_worker;
    if (!worker || !d || worker->target != d->id || worker->fault_blocked) return false;
    d->fault_main_state = d->task_state;
    worker->fault_blocked = true;
    worker->blocked = true;
    worker->wait_deadline = 0;
    worker->wait_address = 0;
    return true;
}

bool kernel_worker_fault_matches(Domain *d, const PgFaultRecord *record) {
    if (!d || !record || record->target_pd != d->id || !record->task_id) return false;
    if (record->task_id == d->task_id)
        return d->frame && record->saved_frame == (uint64_t)(uintptr_t)d->frame &&
               record->saved_rip == d->frame->rip;
    ProgramWorker *worker = program_worker_find(&program_workers, record->task_id);
    if (!worker || worker->target != d->id || worker->run != d->task_id ||
        !worker->fault_blocked)
        return false;
    ProgramWorkerStorage *storage = kernel_worker_storage(worker);
    return storage && record->saved_frame == (uint64_t)(uintptr_t)storage->frame &&
           record->saved_rip == storage->frame->rip;
}

bool kernel_worker_fault_complete(Domain *d, const PgFaultRecord *record) {
    if (!kernel_worker_fault_matches(d, record) || record->task_id == d->task_id) return false;
    ProgramWorker *worker = program_worker_find(&program_workers, record->task_id);
    worker->fault_blocked = false;
    worker->blocked = false;
    if (!program_worker_admit_queue(&kernel.run_queue, worker))
        worker->admission_pending = true;
    uint8_t main_state = d->fault_main_state;
    d->fault_main_state = 0;
    if (main_state == TASK_RUNNING || main_state == TASK_READY) {
        d->task_state = TASK_READY;
        run_push(d->id);
    } else {
        d->task_state = main_state;
    }
    klog("[program.worker.fault.resume] target=%u worker=%u\n", d->id, worker->id);
    return true;
}

bool kernel_worker_fault_reject(Domain *d, const PgFaultRecord *record) {
    if (!kernel_worker_fault_matches(d, record) || record->task_id == d->task_id) return false;
    ProgramWorker *worker = program_worker_find(&program_workers, record->task_id);
    (void)program_worker_exit(worker);
    d->fault_main_state = 0;
    d->task_state = TASK_EXITED;
    kernel_worker_end(d);
    return true;
}

bool kernel_worker_all_dead(uint32_t target) {
    return (!kernel_current_worker || kernel_current_worker->target != target) &&
           program_worker_all_dead(&program_workers, target);
}

static uint64_t worker_wake(Domain *d, uint64_t address, uint64_t most) {
    uint64_t woken = 0;
    for (size_t i = 0; i < program_workers.count && woken < most; ++i) {
        ProgramWorker *worker = &program_workers.entries[i];
        if (!worker->present || worker->target != d->id || !worker->blocked ||
            worker->wait_address != address)
            continue;
        worker->wait_address = 0;
        worker->wait_deadline = 0;
        (void)program_worker_wake_queue(&kernel.run_queue, worker);
        ++woken;
    }
    for (uint32_t id = 1; id < KERNEL_MAX_DOMAINS && woken < most; ++id) {
        Domain *waiter = domain_get(id);
        if (!waiter) continue;
        if (waiter->id != d->id || waiter->task_state != TASK_WAIT_IO ||
            waiter->wait_address != address)
            continue;
        waiter->wait_address = 0;
        kernel_task_wake(waiter);
        ++woken;
    }
    return woken;
}

static TrapFrame *worker_wait(Domain *d, TrapFrame *frame, uint64_t address,
                              uint64_t expected, uint64_t milliseconds) {
    uint32_t value = 0;
    frame->rsi = 0;
    if (address % 4 || !copy_from_user(&value, address, sizeof value, d->mask)) {
        frame->rax = PROGRAM_WORKER_INVALID;
        return frame;
    }
    if (!worker_live(d)) {
        frame->rax = PROGRAM_WORKER_DENIED;
        return frame;
    }
    if (value != (uint32_t)expected) {
        frame->rax = PROGRAM_WORKER_AGAIN;
        return frame;
    }
    uint64_t ticks = milliseconds > PROGRAM_THREAD_TICK_LIMIT ?
                     PROGRAM_THREAD_TICK_LIMIT : milliseconds;
    frame->rax = PROGRAM_WORKER_OK;
    if (!kernel_current_worker) {
        d->wait_address = address;
        TrapFrame *next = kernel_task_block(d, frame, ticks);
        if (next == frame) d->wait_address = 0;
        return next;
    }
    ProgramWorker *worker = kernel_current_worker;
    worker->wait_address = address;
    worker->wait_deadline = ticks ? kernel.ticks + ticks : 0;
    uint64_t status = program_worker_park(worker, 0);
    if (status == PROGRAM_WORKER_DENIED) kernel_halt("worker wait without ownership");
    if (status == PROGRAM_WORKER_WAKE_CONSUMED) {
        worker->wait_address = 0;
        worker->wait_deadline = 0;
        return frame;
    }
    return kernel_worker_schedule(frame, false);
}

TrapFrame *kernel_worker_syscall(Domain *d, TrapFrame *frame) {
    uint64_t op = frame->rdi;
    uint64_t a = frame->rsi;
    uint64_t b = frame->rdx;
    uint64_t c = frame->r10;
    frame->rsi = 0;
    frame->rax = PROGRAM_WORKER_INVALID;
    if (op == PROGRAM_THREAD_SPAWN) {
        if (!worker_live(d)) {
            frame->rax = PROGRAM_WORKER_DENIED;
            return frame;
        }
        if (d->program.run != d->task_id) {
            frame->rax = PROGRAM_WORKER_DENIED;
            return frame;
        }
        uint64_t memory_status = program_memory_worker_spawn(&d->program);
        if (memory_status != PROGRAM_MEMORY_OK) {
            frame->rax = memory_status;
            return frame;
        }
        size_t before_count = program_workers.count;
        ProgramWorkerResult result = program_worker_spawn(&program_workers, program_worker_storage,
                                                          d->id, d->task_id, a, b, c, 0, kernel.ticks);
        if (result.status != PROGRAM_WORKER_OK) {
            (void)program_memory_worker_exit(&d->program);
            frame->rax = result.status;
            return frame;
        }
        if (!program_worker_admit_queue(&kernel.run_queue, result.value)) {
            (void)program_memory_worker_exit(&d->program);
            (void)program_worker_exit(result.value);
            frame->rax = PROGRAM_WORKER_RETRY;
            return frame;
        }
        result.value->memory_accounted = true;
        bool reused = before_count != 0 && program_workers.count == before_count;
        frame->rax = PROGRAM_WORKER_OK;
        frame->rsi = result.value->id;
        klog("[program.worker.start] target=%u run=%u worker=%u core=0 reused=%u slot=%u\n",
             d->id, d->task_id, result.value->id, reused, result.value->slot);
        return frame;
    }
    if (op == PROGRAM_THREAD_EXIT) {
        ProgramWorker *worker = kernel_current_worker;
        if (!worker || worker->target != d->id || kernel_current_task(d) != worker->id) {
            frame->rax = PROGRAM_WORKER_DENIED;
            return frame;
        }
        (void)program_worker_exit(worker);
        if (a && !(a % 4) && user_range_ok(a, 4, true, d->mask)) {
            uint32_t done = 1;
            if (copy_to_user(a, &done, sizeof done, d->mask))
                (void)worker_wake(d, a, UINT64_MAX);
        }
        klog("[program.worker.exit] target=%u run=%u worker=%u\n",
             d->id, worker->run, worker->id);
        return kernel_worker_schedule(frame, false);
    }
    if (op == PROGRAM_THREAD_WAIT) return worker_wait(d, frame, a, b, c);
    if (op == PROGRAM_THREAD_WAKE) {
        if (a % 4) return frame;
        if (!worker_live(d)) {
            frame->rax = PROGRAM_WORKER_DENIED;
            return frame;
        }
        frame->rax = PROGRAM_WORKER_OK;
        frame->rsi = worker_wake(d, a, b);
        return frame;
    }
    if (op == PROGRAM_THREAD_CORES) {
        if (!worker_live(d)) {
            frame->rax = PROGRAM_WORKER_DENIED;
            return frame;
        }
        frame->rax = PROGRAM_WORKER_OK;
        frame->rsi = 1;
        return frame;
    }
    if (op == PROGRAM_THREAD_TLS) {
        if (a >= UINT64_C(0x0000800000000000) || !worker_live(d)) {
            frame->rax = a >= UINT64_C(0x0000800000000000) ?
                         PROGRAM_WORKER_INVALID : PROGRAM_WORKER_DENIED;
            return frame;
        }
        if (kernel_current_worker) kernel_current_worker->fs_base = a;
        else d->fs_base = a;
        cpu_wrmsr(MSR_FS_BASE, a);
        frame->rax = PROGRAM_WORKER_OK;
        return frame;
    }
    return frame;
}
