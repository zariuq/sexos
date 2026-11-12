#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_kernel.h"
#include "program_workers.h"

Kernel kernel;
uint64_t kernel_syscall_stack_top;
static Domain domain;
static TrapFrame main_frame;
static bool main_queued, idle_seen, halt_seen, worker_fpu_seen;
static uint32_t start_word, done_word;
static uint64_t fs_writes[16];
static size_t fs_count;
static uint8_t main_fpu[512];
static jmp_buf escape;

extern ProgramWorkers program_workers;

void host_cpu_wrmsr(uint32_t msr, uint64_t value)
{
    assert(msr == MSR_FS_BASE);
    assert(fs_count < sizeof fs_writes / sizeof fs_writes[0]);
    fs_writes[fs_count++] = value;
}

void host_cpu_wrpkru(uint32_t mask) { (void)mask; }

Domain *domain_get(uint32_t id)
{
    return id == domain.id && domain.live ? &domain : NULL;
}

uint32_t kernel_current_task(Domain *d)
{
    if (!d || d != domain_get(kernel.current)) return 0;
    if (kernel_current_worker)
        return kernel_current_worker->phase == PROGRAM_WORKER_ATTACHED &&
               kernel_current_worker->target == d->id && kernel_current_worker->run == d->task_id &&
               kernel_current_worker->owner_core == 0 && !kernel_current_worker->exited &&
               kernel_current_worker->id >= PROGRAM_WORKER_TASK_FIRST &&
               kernel_current_worker->id < PROGRAM_WORKER_TASK_END ?
               kernel_current_worker->id : 0;
    return d->task_state == TASK_RUNNING ? d->task_id : 0;
}

void run_push(uint32_t id)
{
    Domain *d = domain_get(id);
    if (!d || d->queued || d->task_state == TASK_EXITED || d->task_state == TASK_DORMANT)
        return;
    assert(!main_queued);
    if (!program_record_queue_admit(&kernel.run_queue, id)) return;
    d->queued = true;
    d->task_state = TASK_READY;
    main_queued = true;
}

uintptr_t run_pop(void)
{
    while (kernel.run_queue.count || kernel.run_queue.handoff) {
        uintptr_t token = program_record_queue_select(&kernel.run_queue);
        if (kernel_worker_token(token)) {
            if (kernel_worker_dispatch(token)) return token;
            continue;
        }
        if (token == domain.id) {
            main_queued = false;
            domain.queued = false;
            if (domain.task_state == TASK_READY) return token;
        }
    }
    return 0;
}

void tss_set_rsp0(uint64_t top) { kernel_syscall_stack_top = top; }

void kernel_fpu_restore(Domain *d)
{
    assert(d == &domain);
    if (kernel_current_worker) {
        assert(kernel_worker_storage(kernel_current_worker)->fpu != main_fpu);
        worker_fpu_seen = true;
    }
}

TrapFrame *resume_frame(TrapFrame *frame) { return frame; }

TrapFrame *switch_to(uintptr_t id)
{
    assert(id == domain.id && domain.task_state == TASK_READY && !kernel_current_worker);
    kernel.current = (uint32_t)id;
    domain.task_state = TASK_RUNNING;
    host_cpu_wrmsr(MSR_FS_BASE, domain.fs_base);
    kernel_worker_stack_departed();
    return domain.frame;
}

_Noreturn void kernel_idle(void)
{
    idle_seen = true;
    kernel_worker_stack_departed();
    longjmp(escape, 1);
}

_Noreturn void kernel_halt(const char *reason)
{
    fprintf(stderr, "kernel halt: %s\n", reason);
    halt_seen = true;
    longjmp(escape, 1);
}

void klog(const char *format, ...) { (void)format; }

void kernel_task_wake(Domain *d)
{
    assert(d == &domain && d->task_state != TASK_EXITED);
    d->wait_address = 0;
    d->wake_pending = true;
    if (d->task_state == TASK_WAIT_IO) {
        d->wait_deadline = 0;
        run_push(d->id);
    }
}

TrapFrame *kernel_task_block(Domain *d, TrapFrame *frame, uint64_t ticks)
{
    assert(d == &domain && kernel_current_task(d) == d->task_id);
    d->frame = frame;
    d->wait_deadline = ticks ? kernel.ticks + ticks : 0;
    d->task_state = TASK_WAIT_IO;
    return kernel_worker_schedule(frame, false);
}

static unsigned release_calls;
static bool paging_complete;

bool program_frame_release_run(uint32_t target, uint32_t run)
{
    assert(target == domain.id && run == domain.task_id);
    ++release_calls;
    return true;
}

bool kernel_paging_cleanup_complete(uint32_t target)
{
    return target == domain.id && paging_complete;
}

KernelAppPrivate *kernel_app_get(uint32_t target)
{
    for (uint32_t i = 0; i < kernel.app_count; ++i)
        if (kernel.apps[i].target == target) return &kernel.apps[i];
    return NULL;
}

bool user_range_ok(uint64_t start, uint64_t length, bool write, uint32_t mask)
{
    (void)start; (void)length; (void)write; (void)mask;
    return true;
}

bool copy_from_user(void *destination, uint64_t source, size_t length, uint32_t mask)
{
    (void)mask;
    memcpy(destination, (const void *)(uintptr_t)source, length);
    return true;
}

bool copy_to_user(uint64_t destination, const void *source, size_t length, uint32_t mask)
{
    (void)mask;
    memcpy((void *)(uintptr_t)destination, source, length);
    return true;
}

static TrapFrame call(Domain *d, TrapFrame *frame, uint64_t op, uint64_t a, uint64_t b, uint64_t c)
{
    *frame = (TrapFrame){.rax = 83, .rdi = op, .rsi = a, .rdx = b, .r10 = c, .cs = 3};
    return *kernel_worker_syscall(d, frame);
}

static void reset(void)
{
    memset(&kernel, 0, sizeof kernel);
    memset(&domain, 0, sizeof domain);
    domain.live = true;
    domain.id = 7;
    domain.key = 1;
    domain.mask = 2;
    domain.task_id = 101;
    domain.task_state = TASK_RUNNING;
    program_memory_init(&domain.program);
    domain.program.target = domain.id;
    domain.program.run = domain.task_id;
    domain.program.key = domain.key;
    domain.program.phase = PROGRAM_MEMORY_LIVE;
    domain.program.main_alive = true;
    release_calls = 0;
    paging_complete = true;
    domain.frame = &main_frame;
    kernel.current = domain.id;
    kernel.genesis_sealed = true;
    kernel.domains.slots[domain.id] = &domain;
    main_queued = idle_seen = halt_seen = worker_fpu_seen = false;
    start_word = done_word = fs_count = 0;
    kernel_workers_boot();
}

static uint32_t spawn_worker(void)
{
    TrapFrame reply = call(&domain, &main_frame, 1, 0x4000, 0x8000, 0x1234);
    assert(reply.rax == PROGRAM_WORKER_OK && reply.rsi >= 0x40000000);
    return (uint32_t)reply.rsi;
}

static void execution_roundtrip(void)
{
    reset();
    uint32_t id = spawn_worker();
    ProgramWorker *worker = program_worker_find(&program_workers, id);
    assert(worker && worker->slot == 0 && kernel_worker_storage(worker)->frame);
    assert(!kernel_worker_all_dead(domain.id));
    assert(domain.program.workers == 1 && worker->memory_accounted && !worker->memory_released);

    (void)kernel_worker_yield();
    assert(kernel_current_worker == worker && worker_fpu_seen);

    TrapFrame reply = call(&domain, kernel_worker_storage(worker)->frame, 3,
                           (uintptr_t)&start_word, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_OK && !kernel_current_worker);
    assert(worker->phase == PROGRAM_WORKER_DETACHED && worker->blocked);
    assert(domain.task_state == TASK_RUNNING && worker->wait_address == (uintptr_t)&start_word);

    reply = call(&domain, &main_frame, 4, (uintptr_t)&start_word, 1, 0);
    assert(reply.rax == PROGRAM_WORKER_OK && reply.rsi == 1);
    assert(worker->phase == PROGRAM_WORKER_QUEUED && kernel.run_queue.count == 1);

    reply = call(&domain, &main_frame, 6, 0x12340, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_OK && domain.fs_base == 0x12340);
    reply = call(&domain, &main_frame, 3, (uintptr_t)&done_word, 0, 0);
    assert(kernel_current_worker == worker && domain.task_state == TASK_WAIT_IO);
    assert(worker->phase == PROGRAM_WORKER_ATTACHED);

    reply = call(&domain, kernel_worker_storage(worker)->frame, 6, 0x56780, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_OK && worker->fs_base == 0x56780);
    kernel_task_wake(&domain);
    (void)kernel_worker_yield();
    assert(!kernel_current_worker && domain.task_state == TASK_RUNNING);
    reply = call(&domain, &main_frame, 3, (uintptr_t)&done_word, 0, 0);
    assert(kernel_current_worker == worker && fs_writes[fs_count - 1] == 0x56780);
    reply = call(&domain, kernel_worker_storage(worker)->frame, 2, (uintptr_t)&done_word, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_OK && done_word == 1);
    assert(!kernel_current_worker && program_worker_dead(worker));
    assert(worker->memory_released && domain.program.workers == 0);
    assert(domain.task_state == TASK_RUNNING && kernel_worker_all_dead(domain.id));
    assert(fs_writes[fs_count - 1] == 0x12340);
    bool saw_worker_tls = false;
    for (size_t i = 0; i < fs_count; ++i) if (fs_writes[i] == 0x56780) saw_worker_tls = true;
    assert(saw_worker_tls);

    memset(kernel_worker_storage(worker)->stack, 0xcc, 64);
    kernel.ticks = 1;
    kernel_worker_poll();
    kernel.ticks = 5;
    uint32_t next = spawn_worker();
    ProgramWorker *reused = program_worker_find(&program_workers, next);
    assert(reused == worker && next != id && reused->fs_base == 0);
    for (size_t i = 0; i < 64; ++i) assert(kernel_worker_storage(reused)->stack[i] == 0);
    kernel_worker_end(&domain);
    assert(!kernel_worker_all_dead(domain.id));
    if (!setjmp(escape))
        (void)kernel_worker_schedule(&main_frame, false);
    assert(idle_seen && !kernel_current_worker);
    assert(kernel_worker_all_dead(domain.id));
    if (domain.program.workers != 0)
        fprintf(stderr, "memory state: workers=%u phase=%u accounted=%u released=%u run=%u program_run=%u next=%u\n",
                domain.program.workers, reused->phase, reused->memory_accounted,
                reused->memory_released, reused->run, domain.program.run, next);
    assert(domain.program.workers == 0);
    domain.task_state = TASK_EXITED;
    kernel_worker_memory_end(&domain);
    assert(domain.program.phase == PROGRAM_MEMORY_LIVE && !domain.program.main_alive && !release_calls);
    kernel.current = 0;
    paging_complete = false;
    kernel_worker_memory_end(&domain);
    assert(!release_calls && domain.program.phase == PROGRAM_MEMORY_LIVE);
    paging_complete = true;
    domain.queued = true;
    kernel_worker_memory_end(&domain);
    assert(!release_calls);
    domain.queued = false;
    kernel.app_count = 1;
    kernel.apps[0] = (KernelAppPrivate){.target = domain.id, .task = domain.task_id,
                                        .acked = APP_PRIVATE_REQUIRED};
    kernel_worker_memory_end(&domain);
    assert(!release_calls && domain.program.phase == PROGRAM_MEMORY_LIVE);
    kernel.apps[0].retired = true;
    kernel_worker_memory_end(&domain);
    assert(domain.program.phase == PROGRAM_MEMORY_DEAD && release_calls == 1);
    kernel_worker_memory_end(&domain);
    assert(release_calls == 1);
}

static void pager_continuation(void)
{
    reset();
    uint32_t id = spawn_worker();
    ProgramWorker *worker = program_worker_find(&program_workers, id);
    (void)kernel_worker_yield();
    assert(kernel_current_worker == worker);
    assert(program_record_queue_pop(&kernel.run_queue) == domain.id);
    main_queued = false;
    domain.queued = false;
    assert(kernel_worker_fault_begin(&domain));
    domain.task_state = TASK_WAIT_FAULT;
    ProgramWorkerStorage *storage = kernel_worker_storage(worker);
    PgFaultRecord record = {
        .token = 1, .target_pd = domain.id, .pager_pd = 14, .task_id = id,
        .fault_addr = 0x5000, .page_addr = 0x5000, .error_code = 6,
        .saved_rip = storage->frame->rip,
        .saved_frame = (uint64_t)(uintptr_t)storage->frame
    };
    assert(kernel_worker_fault_matches(&domain, &record));
    record.saved_frame = (uint64_t)(uintptr_t)&main_frame;
    assert(!kernel_worker_fault_matches(&domain, &record));
    record.saved_frame = (uint64_t)(uintptr_t)storage->frame;
    record.saved_rip++;
    assert(!kernel_worker_fault_matches(&domain, &record));
    --record.saved_rip;
    ++record.task_id;
    assert(!kernel_worker_fault_matches(&domain, &record));
    --record.task_id;
    if (!setjmp(escape))
        (void)kernel_worker_schedule(storage->frame, false);
    assert(idle_seen && worker->phase == PROGRAM_WORKER_DETACHED && worker->fault_blocked);
    assert(!program_worker_admit_queue(&kernel.run_queue, worker));
    assert(program_worker_wake(worker) == PROGRAM_WORKER_RECORDED);
    assert(kernel.run_queue.count == 0);
    assert(kernel_worker_fault_complete(&domain, &record));
    assert(!worker->fault_blocked && worker->phase == PROGRAM_WORKER_QUEUED);
    assert(kernel.run_queue.count == 2 && main_queued);

    (void)kernel_worker_schedule(&main_frame, true);
    assert(kernel_current_worker == worker && worker->phase == PROGRAM_WORKER_ATTACHED);
    assert(kernel_worker_fault_begin(&domain));
    assert(kernel_worker_fault_matches(&domain, &record));
    assert(kernel_worker_fault_reject(&domain, &record));
    assert(worker->exited && domain.task_state == TASK_EXITED);
    idle_seen = false;
    if (!setjmp(escape))
        (void)kernel_worker_schedule(storage->frame, false);
    assert(idle_seen && !kernel_current_worker && program_worker_dead(worker));
}

static void fault_and_refusals(void)
{
    reset();
    TrapFrame reply = call(&domain, &main_frame, 2, 0, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_DENIED);
    reply = call(&domain, &main_frame, 3, 1, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_INVALID);
    reply = call(&domain, &main_frame, 4, 1, 1, 0);
    assert(reply.rax == PROGRAM_WORKER_INVALID);
    reply = call(&domain, &main_frame, 99, 0, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_INVALID);
    reply = call(&domain, &main_frame, 1, 0, 0x8000, 0);
    assert(reply.rax == PROGRAM_WORKER_INVALID && !kernel.run_queue.count);
    reply = call(&domain, &main_frame, 5, 0, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_OK && reply.rsi == 1);

    uint32_t id = spawn_worker();
    ProgramWorker *worker = program_worker_find(&program_workers, id);
    worker->slot = PROGRAM_WORKER_MAX;
    assert(!kernel_worker_storage(worker));
    worker->slot = 0;
    (void)kernel_worker_yield();
    assert(kernel_current_worker == worker);
    if (!setjmp(escape)) {
        kernel_worker_fault(&domain);
        assert(worker->exited && domain.task_state == TASK_EXITED);
        (void)kernel_worker_schedule(kernel_worker_storage(worker)->frame, false);
        assert(!"fault schedule returned");
    }
    if (!idle_seen || halt_seen || kernel_current_worker)
        fprintf(stderr, "fault state: idle=%u halt=%u current=%p phase=%u queued=%u main_state=%u\n",
                idle_seen, halt_seen, (void *)kernel_current_worker, worker->phase,
                main_queued, domain.task_state);
    assert(idle_seen && !halt_seen && !kernel_current_worker);
    assert(program_worker_dead(worker) && kernel_worker_all_dead(domain.id));
}

static void identity_refusals(void)
{
    reset();
    program_workers.next_id = PROGRAM_WORKER_TASK_END - 1;
    uint32_t id = spawn_worker();
    assert(id == PROGRAM_WORKER_TASK_END - 1);
    assert(domain.program.workers == 1 && kernel.run_queue.count == 1);
    TrapFrame reply = call(&domain, &main_frame, 1, 0x4000, 0x8000, 0);
    assert(reply.rax == PROGRAM_WORKER_FULL && !reply.rsi);
    assert(domain.program.workers == 1 && kernel.run_queue.count == 1);
    assert(program_workers.count == 1 && program_workers.next_id == PROGRAM_WORKER_TASK_END);
    (void)kernel_worker_yield();
    ProgramWorker *worker = program_worker_find(&program_workers, id);
    assert(worker && kernel_current_worker == worker);
    TrapFrame *frame = kernel_worker_storage(worker)->frame;
    ++domain.task_id;
    reply = call(&domain, frame, 2, 0, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_DENIED && !worker->exited);
    reply = call(&domain, frame, 1, 0x4000, 0x8000, 0);
    assert(reply.rax == PROGRAM_WORKER_DENIED && domain.program.workers == 1);
    reply = call(&domain, frame, 4, (uint64_t)(uintptr_t)&start_word, 1, 0);
    assert(reply.rax == PROGRAM_WORKER_DENIED && !reply.rsi);
    --domain.task_id;
    Domain other = domain;
    ++other.id;
    reply = call(&other, frame, 2, 0, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_DENIED && !worker->exited);
    worker->phase = PROGRAM_WORKER_DISPATCHING;
    reply = call(&domain, frame, 2, 0, 0, 0);
    assert(reply.rax == PROGRAM_WORKER_DENIED && !worker->exited);
    worker->phase = PROGRAM_WORKER_ATTACHED;
    assert(kernel_current_task(&domain) == id);
}

static void shared_selection_controls(void)
{
    reset();
    uint32_t first_id = spawn_worker();
    ProgramWorker *first = program_worker_find(&program_workers, first_id);
    program_record_queue_arm(&kernel.run_queue, true);
    run_push(domain.id);
    assert(kernel.run_queue.handoff == domain.id && kernel.run_queue.count == 1);
    assert(run_pop() == domain.id && !main_queued);
    assert(run_pop() == (uintptr_t)first && first->phase == PROGRAM_WORKER_DISPATCHING);

    reset();
    first_id = spawn_worker();
    first = program_worker_find(&program_workers, first_id);
    uint32_t second_id = spawn_worker();
    ProgramWorker *second = program_worker_find(&program_workers, second_id);
    assert(kernel.run_queue.count == 2);
    first->exited = true;
    assert(run_pop() == (uintptr_t)second);
    assert(first->phase == PROGRAM_WORKER_DEAD && first->memory_released);
    assert(second->phase == PROGRAM_WORKER_DISPATCHING);

    reset();
    first_id = spawn_worker();
    first = program_worker_find(&program_workers, first_id);
    assert(program_record_queue_pop(&kernel.run_queue) == (uintptr_t)first);
    first->phase = PROGRAM_WORKER_DETACHED;
    program_record_queue_arm(&kernel.run_queue, true);
    assert(program_worker_wake_queue(&kernel.run_queue, first) == PROGRAM_WORKER_NEEDS_ADMISSION);
    assert(kernel.run_queue.handoff == (uintptr_t)first && !kernel.run_queue.count);
    assert(program_worker_wake_queue(&kernel.run_queue, first) == PROGRAM_WORKER_RECORDED);
    assert(run_pop() == (uintptr_t)first);
    assert(!run_pop());
}

int main(void)
{
    shared_selection_controls();
    execution_roundtrip();
    pager_continuation();
    fault_and_refusals();
    identity_refusals();
    puts("kernel workers: native execution, pager continuation, deferred release, reuse, fault and refusal controls passed");
    return 0;
}
