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
#include "epoch_journal.h"

static EpochLogBatch *kernel_epoch_batch;
static uint64_t kernel_epoch_flush_task;
static bool kernel_epoch_flush_active;
static uint64_t kernel_epoch_stall_report;
static uint64_t kernel_epoch_schedule_seen;

bool kernel_epoch_log_boot(void) {
    if (!SEXOS_KERNEL_CURRENT_PDX || kernel.genesis_sealed || kernel_epoch_batch)
        return false;
    uint64_t physical = 0;
    if (!boot_epoch_log_region(CURRENT_PD_sexfiles, &physical))
        return false;
    EpochLogBatch *batch = phys_to_virt(physical);
    if (!batch || kernel.epoch == UINT64_MAX)
        return false;
    batch->flushed_through = 0;
    batch->failed = 0;
    __atomic_store_n(&batch->producer, kernel.epoch + 1, __ATOMIC_RELEASE);
    const Cap *cap = cap_get(domain_get(CURRENT_PD_sexfiles), SEXOS_SLOT_epoch_log);
    if (!cap || cap->kind != CAP_REGION || cap->region.grant.region_id != EPOCH_LOG_REGION_ID ||
        cap->region.grant.length != EPOCH_LOG_REGION_BYTES ||
        cap->region.grant.rights != (REGION_QUERY | REGION_READ | REGION_WRITE) ||
        cap->region.grant.has_dma || cap->region.grant.has_peer)
        return false;
    kernel_epoch_batch = batch;
    kernel_epoch_flush_task = 0;
    kernel_epoch_flush_active = false;
    klog("[boot.epoch.log] region=%u slot=%llu bytes=%llu events=%u producer=%llu\n",
         EPOCH_LOG_REGION_ID, (unsigned long long)SEXOS_SLOT_epoch_log,
         (unsigned long long)EPOCH_LOG_REGION_BYTES, EPOCH_LOG_BATCH_EVENTS,
         (unsigned long long)(kernel.epoch + 1));
    return true;
}

static bool kernel_epoch_log_backlogged(void) {
    if (!kernel_epoch_batch) return false;
    uint64_t producer = __atomic_load_n(&kernel_epoch_batch->producer, __ATOMIC_ACQUIRE);
    uint64_t flushed = __atomic_load_n(&kernel_epoch_batch->flushed_through, __ATOMIC_ACQUIRE);
    return producer > flushed && producer - 1 - flushed >= EPOCH_LOG_BATCH_EVENTS;
}

bool kernel_epoch_log_internal(void) {
    return (kernel_epoch_flush_active || kernel_epoch_log_backlogged()) &&
           (kernel.current == CURRENT_PD_sexfiles || kernel.current == CURRENT_PD_sexdrive);
}

bool kernel_epoch_log_eligible(uint32_t id) {
    if (kernel_epoch_log_backlogged() && id < 64 && !(kernel_epoch_schedule_seen & (UINT64_C(1) << id))) {
        kernel_epoch_schedule_seen |= UINT64_C(1) << id;
        klog("[kernel.epoch.schedule] id=%u active=%u\n", id, kernel_epoch_flush_active);
    }
    return (!kernel_epoch_flush_active && !kernel_epoch_log_backlogged()) ||
           id == CURRENT_PD_sexfiles || id == CURRENT_PD_sexdrive;
}

static void kernel_epoch_log_kick(void) {
    Domain *files = domain_get(CURRENT_PD_sexfiles);
    kernel_task_wake(files);
}

bool kernel_epoch_log_ready(uint64_t expected) {
    if (kernel_epoch_log_internal())
        return expected == kernel.epoch;
    if (kernel_epoch_flush_active)
        return false;
    if (!kernel_epoch_batch)
        return true;
    if (expected != kernel.epoch || expected == UINT64_MAX - 1)
        return false;
    uint64_t next = expected + 1;
    if (__atomic_load_n(&kernel_epoch_batch->failed, __ATOMIC_ACQUIRE) ||
        __atomic_load_n(&kernel_epoch_batch->producer, __ATOMIC_ACQUIRE) != next)
        return false;
    uint64_t flushed = __atomic_load_n(&kernel_epoch_batch->flushed_through, __ATOMIC_ACQUIRE);
    uint64_t elided = next > KERNEL_EPOCH_EVENT_CAPACITY ? next - KERNEL_EPOCH_EVENT_CAPACITY : 0;
    if (flushed > next || next - flushed > EPOCH_LOG_BATCH_EVENTS || flushed < elided) {
        if (next != kernel_epoch_stall_report) {
            kernel_epoch_stall_report = next;
            kernel_epoch_schedule_seen = 0;
            Domain *files = domain_get(CURRENT_PD_sexfiles);
            klog("[kernel.epoch.backpressure] next=%llu flushed=%llu producer=%llu current=%u files_state=%u queued=%u run_count=%zu flush=%u wake=%u\n",
                 (unsigned long long)next, (unsigned long long)flushed,
                 (unsigned long long)__atomic_load_n(&kernel_epoch_batch->producer, __ATOMIC_ACQUIRE),
                 kernel.current, files ? files->task_state : 0, files ? files->queued : 0,
                 kernel.run_queue.count, kernel_epoch_flush_active, files ? files->wake_pending : 0);
        }
        kernel_epoch_log_kick();
        return false;
    }
    return true;
}

bool kernel_epoch_log_append(uint64_t next, const KernelEpochEvent *event) {
    if (!kernel_epoch_batch)
        return true;
    if (!event || event->epoch != next || !kernel_epoch_log_ready(next - 1))
        return false;
    uint8_t *sector = kernel_epoch_batch->sectors[(next - 1) % EPOCH_LOG_BATCH_EVENTS];
    if (epoch_journal_event_encode(sector, event) != EPOCH_JOURNAL_OK)
        return false;
    __atomic_store_n(&kernel_epoch_batch->producer, next + 1, __ATOMIC_RELEASE);
    return true;
}

bool kernel_epoch_log_flush(Domain *d, bool enter) {
    if (!SEXOS_KERNEL_CURRENT_PDX || !d || d->id != CURRENT_PD_sexfiles ||
        kernel.current != d->id || !d->task_id || d->program.run != d->task_id)
        return false;
    if (enter) {
        if (kernel_epoch_flush_active || !kernel_epoch_batch || !kernel.genesis_sealed ||
            !kernel.scheduler_running || !domain_get(CURRENT_PD_sexdrive))
            return false;
        kernel_epoch_flush_task = d->task_id;
        kernel_epoch_flush_active = true;
        return true;
    }
    if (!kernel_epoch_flush_active || kernel_epoch_flush_task != d->task_id)
        return false;
    kernel_epoch_flush_active = false;
    kernel_epoch_flush_task = 0;
    return true;
}

void kernel_epoch_log_poll(void) {
    if (!kernel_epoch_flush_active)
        return;
    Domain *owner = domain_get(CURRENT_PD_sexfiles);
    if (owner && owner->task_id == kernel_epoch_flush_task && owner->task_state != TASK_EXITED &&
        owner->task_state != TASK_DORMANT)
        return;
    kernel_epoch_flush_active = false;
    uint64_t abandoned = kernel_epoch_flush_task;
    kernel_epoch_flush_task = 0;
    if (kernel_epoch_batch)
        __atomic_store_n(&kernel_epoch_batch->failed, EPOCH_JOURNAL_IO, __ATOMIC_RELEASE);
    klog("[kernel.epoch.log.abandoned] task=%llu failed=%u\n",
         (unsigned long long)abandoned, EPOCH_JOURNAL_IO);
}
