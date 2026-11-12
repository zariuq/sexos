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

/* Publication and park are serialized on the BSP with IF masked. A queued
 * task owns one admission; an I/O wake cannot release a fault continuation.
 * Idle runs on a separate retained stack before detached cleanup is possible. */
void kernel_task_wake(Domain *d) {
    if (!SEXOS_KERNEL_CURRENT_PDX || !d || d->task_state == TASK_EXITED ||
        d->task_state == TASK_DORMANT)
        return;
    d->wait_address = 0;
    d->wake_pending = true;
    if (d->task_state == TASK_WAIT_IO) {
        d->wait_deadline = 0;
        d->task_state = TASK_READY;
        run_push(d->id);
    }
}

void kernel_task_poll(void) {
    if (!SEXOS_KERNEL_CURRENT_PDX || !kernel.genesis_sealed || !kernel.scheduler_running)
        return;
    kernel_epoch_log_poll();
    for (uint32_t vector = 0; vector < 256; ++vector)
        (void)kernel_device_interrupt_retry(vector);
    kernel_worker_poll();
    kernel_records_poll();
    for (uint32_t id = 1; id < KERNEL_MAX_DOMAINS; ++id) {
        Domain *d = domain_get(id);
        if (!d)
            continue;
        if (d->admission_pending && d->task_state == TASK_READY) run_push(id);
        if (d->task_state == TASK_WAIT_IO && d->wait_deadline && kernel.ticks >= d->wait_deadline) {
            uint64_t at = d->wait_deadline;
            bool program_wait = d->wait_address != 0;
            d->wait_address = 0;
            d->wait_deadline = 0;
            ++d->expired_count;
            d->frame->rsi = program_wait ? 0 : 1;
            kernel_task_wake(d);
            if (d->expired_count <= 4 || !(d->expired_count & (d->expired_count - 1)))
                klog("[scheduler.self_wait.expired] task=%u deadline=%llu now=%llu count=%llu\n",
                     d->task_id, (unsigned long long)at, (unsigned long long)kernel.ticks,
                     (unsigned long long)d->expired_count);
        }
    }
    for (uint32_t id = 1; id < KERNEL_MAX_DOMAINS; ++id) {
        Domain *d = domain_get(id);
        if (d && d->task_state == TASK_EXITED) kernel_worker_memory_end(d);
    }
    kernel_app_poll();
    KernelPaging *s = &kernel.paging;
    if (!s->initialized)
        return;
    Domain *pager = domain_get(s->pager);
    if (!pager)
        return;
    for (size_t i = 0; i < PG_CLIENTS; ++i) {
        Domain *d = domain_get(s->mappings[i].target_pd);
        if (!d || d->task_state != TASK_EXITED || d->exit_notice || d->id == kernel.current ||
            d->queued || !kernel_worker_all_dead(d->id) || s->ledger.closed[i].present)
            continue;
        RingEntry m = {.type_id = 0x105, .arg0 = d->id, .arg1 = d->task_id};
        if (domain_enqueue(pager, m)) {
            d->exit_notice = true;
            klog("[paging.exit.notice] target=%u task=%u pager=%u detached=1 accepted=1\n", d->id,
                 d->task_id, pager->id);
        }
    }
}

TrapFrame *kernel_dispatch_blocked(void) {
    Domain *d = domain_get(kernel.current);
    TrapFrame *frame = kernel_current_worker ? kernel_worker_storage(kernel_current_worker)->frame : d ? d->frame : NULL;
    if (!frame)
        kernel_halt("blocked dispatch without a saved frame");
    return kernel_worker_schedule(frame, false);
}

TrapFrame *kernel_task_block(Domain *d, TrapFrame *frame, uint64_t ticks) {
    if (kernel_current_task(d) != d->task_id || !d->task_id || ticks > 64 ||
        (ticks && kernel.ticks > UINT64_MAX - ticks)) {
        frame->rax = ERR_CAP_INVALID;
        return resume_frame(frame);
    }
    frame->rax = 0;
    d->frame = frame;
    bool ready = false;
    for (size_t i = 0; i < KERNEL_RPC_CAPACITY; ++i)
        if (d->rpc[i].state == RPC_READY)
            ready = true;
    if (d->ring.count || d->reply_ready || d->wake_pending || ready) {
        d->wake_pending = false;
        return resume_frame(frame);
    }
    d->wait_deadline = ticks ? kernel.ticks + ticks : 0;
    if (ticks)
        frame->rsi = 0;
    d->task_state = TASK_WAIT_IO;
    if (ticks) {
        ++d->wait_count;
        if (d->wait_count <= 4 || !(d->wait_count & (d->wait_count - 1)))
            klog("[scheduler.self_wait.park] task=%u ticks=%llu deadline=%llu count=%llu\n",
                 d->task_id, (unsigned long long)ticks, (unsigned long long)d->wait_deadline,
                 (unsigned long long)d->wait_count);
    }
    return kernel_dispatch_blocked();
}

TrapFrame *kernel_task_finish(Domain *d, TrapFrame *frame) {
    d->frame = frame;
    d->task_state = TASK_EXITED;
    d->wait_deadline = 0;
    d->wait_address = 0;
    d->wake_pending = false;
    kernel_worker_end(d);
    if (!kernel_region_writer_recover_dead(d))
        kernel_halt("dead writer recovery invariant");
    if (!kernel_device_reset_owner_dead(d))
        kernel_halt("dead device reset custody invariant");
    klog("[task.finish] pd=%u task=%u reason=%llu terminal=1\n", d->id, d->task_id,
         (unsigned long long)frame->rdi);
    return kernel_dispatch_blocked();
}

void kernel_idle_observe(void) {
    if (!SEXOS_KERNEL_CURRENT_PDX || kernel.idle_timed_logged)
        return;
    for (uint32_t id = 1; id < KERNEL_MAX_DOMAINS; ++id) {
        Domain *d = domain_get(id);
        if (d && d->task_state == TASK_WAIT_IO && d->wait_deadline &&
            kernel.ticks < d->wait_deadline) {
            uint64_t sp = cpu_read_rsp();
            kernel.idle_timed_logged = true;
            klog("[scheduler.idle.timed] task=%u deadline=%llu now=%llu rsp=%#llx\n", d->task_id,
                 (unsigned long long)d->wait_deadline, (unsigned long long)kernel.ticks,
                 (unsigned long long)sp);
            return;
        }
    }
}


