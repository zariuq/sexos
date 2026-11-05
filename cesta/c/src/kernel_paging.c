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

/* Native leaf and task adapters run on the BSP with IF masked. APs stay
 * parked. There is one registered main task per range; no signal task exists.
 * The checked state core owns metadata. These adapters authenticate held
 * grants, preserve the retained frame, and perform volatile PTE/TLB effects. */
static bool mapping_equal(PgMapping a, PgMapping b) {
    return a.range_id == b.range_id && a.pool_id == b.pool_id && a.target_pd == b.target_pd &&
           a.base_va == b.base_va && a.length == b.length && a.rights == b.rights &&
           a.quota == b.quota;
}

static bool pool_equal(PgFramePool a, PgFramePool b) {
    return a.pool_id == b.pool_id && a.base_va == b.base_va && a.length == b.length;
}

static const Cap *mapping_cap(Domain *d, uint64_t slot) {
    const Cap *c = cap_get(d, slot);
    return c && c->kind == CAP_PAGER_MAPPING ? c : NULL;
}

static bool registered_map(Domain *d, PgMapping m) {
    KernelPaging *s = &kernel.paging;
    if (!s->initialized || d->id != s->pager)
        return false;
    for (size_t i = 0; i < PG_CLIENTS; ++i)
        if (mapping_equal(s->mappings[i], m))
            return true;
    return false;
}

void kernel_paging_boot(void) {
    KernelPaging *s = &kernel.paging;
    Domain *pager = domain_get(CURRENT_PD_sext);
    if (kernel.genesis_sealed || kernel.scheduler_running || s->initialized || !pager)
        kernel_halt("paging BOOT phase");
    BootBacking b = {0};
    uint64_t va = 0;
    if (!boot_backing_reserve(KERNEL_PAGING_BACKING, PG_PAGES * 4096u, &b) ||
        !boot_region_map(pager->id, &b, true, &va))
        kernel_halt("paging BOOT backing");
    s->pool = (PgFramePool){1, va, PG_PAGES * 4096u};
    s->pager = pager->id;
    s->physical = b.physical;
    const uint32_t clients[] = {CURRENT_PD_kaleidoscope, CURRENT_PD_sexbell};
    for (size_t i = 0; i < PG_CLIENTS; ++i) {
        Domain *d = domain_get(clients[i]);
        uint64_t target = 0;
        if (!d || d->key == pager->key || (i && d->key == s->keys[0]) ||
            !boot_region_map(d->id, &b, true, &target))
            kernel_halt("paging BOOT client");
        s->keys[i] = d->key;
        for (size_t page = 0; page < PG_PAGES; ++page) {
            uint64_t address = target + page * 4096, leaf = page_leaf_address(address);
            if (!leaf || !(*(volatile uint64_t *)(uintptr_t)leaf & 1))
                kernel_halt("paging BOOT leaf");
            *(volatile uint64_t *)(uintptr_t)leaf = 0;
            cpu_invlpg(address);
            s->leaves[i][page] = leaf;
        }
        PgMapping m = {(uint32_t)i + 1, 1, d->id, target, PG_PAGES * 4096u, PG_READ | PG_WRITE, 4};
        s->mappings[i] = m;
        PgDemand demand = {m.range_id, m.base_va, m.length, m.rights};
        if (!cap_set(pager, 36 + i, (Cap){.kind = CAP_PAGER_MAPPING, .mapping = m}) ||
            !cap_set(d, 35, (Cap){.kind = CAP_DEMAND_REGION, .demand = demand}))
            kernel_halt("paging BOOT map grant");
        klog(
            "[boot.paging.genesis] pager=%u target=%u pool_pages=8 arena_pages=8 quota=4 target_pkey=%u leaves_absent=8 tables_preallocated=1\n",
            pager->id, d->id, d->key);
    }
    if (!cap_set(pager, 34, (Cap){.kind = CAP_FRAME_POOL, .pool = s->pool}))
        kernel_halt("paging BOOT pool grant");
    s->initialized = true;
}

static PgFaultResult native_fault_query(PgMapping m, uint32_t pager, uint64_t token, bool claim) {
    Domain *target = domain_get(m.target_pd);
    if (!target)
        return (PgFaultResult){.status = PG_INVALID};
    if (target->task_state != TASK_WAIT_FAULT)
        return (PgFaultResult){.status = PG_DENIED};
    PgFaultResult r = claim ? pager_fault_claim(&target->fault, pager, token)
                            : pager_fault_query(&target->fault, pager, token);
    if (!r.status && (r.record.target_pd != target->id || r.record.task_id != target->task_id))
        kernel_halt("fault task identity");
    return r;
}

static uint64_t native_fault_complete(PgBinding b) {
    Domain *target = domain_get(b.target_pd);
    if (!target)
        return PG_RETRY;
    PgFaultResult r = pager_fault_complete(&target->fault, b.pager_pd, b.token);
    if (r.status)
        return PG_RETRY;
    if (r.present) {
        if (r.record.target_pd != target->id || r.record.task_id != target->task_id ||
            r.record.saved_frame != (uint64_t)(uintptr_t)target->frame ||
            r.record.saved_rip != target->frame->rip)
            kernel_halt("committed fault task or frame identity");
        bool resume = target->task_state == TASK_WAIT_FAULT;
        if (resume) {
            target->task_state = TASK_READY;
            run_push(target->id);
        }
        klog("[paging.fault.completed] target=%u task=%u token=%llu resume=%u saved_rip=%#llx\n",
             target->id, target->task_id, (unsigned long long)b.token, (unsigned)resume,
             (unsigned long long)r.record.saved_rip);
    }
    if (pager_ledger_finalize(&kernel.paging.ledger, b))
        kernel_halt("paging finalize identity");
    return PG_OK;
}

static uint64_t page_expected_leaf(const KernelPaging *state, PgBinding binding) {
    return (state->physical + binding.frame_index * 4096) | PTE_PRESENT | PTE_USER |
           PTE_NO_EXECUTE | ((uint64_t)state->keys[binding.range_id - 1] << 59) |
           (binding.rights & PG_WRITE ? PTE_WRITABLE : 0);
}

typedef struct KernelPageInstallContext {
    KernelPaging *state;
    Domain *target;
    PgMapping mapping;
    PgBinding binding;
    PgLedger before_ledger, after_ledger;
    PgFaultState before_fault, after_fault;
    PgFaultRecord record;
    uint64_t *leaf;
    uint64_t address, expected, epoch;
    uint8_t key;
    bool snapshotted, transformed, swapped;
} KernelPageInstallContext;

static bool kernel_page_install_current(const KernelPageInstallContext *c) {
    if (!c->snapshotted || c->state != &kernel.paging || !c->state->initialized || !c->target ||
        c->target != domain_get(c->binding.target_pd) || !c->binding.token ||
        !c->binding.range_id || c->binding.range_id > PG_CLIENTS ||
        c->binding.frame_index >= PG_PAGES || c->binding.page_index >= PG_PAGES ||
        !mapping_equal(c->state->mappings[c->binding.range_id - 1], c->mapping) ||
        c->mapping.target_pd != c->target->id || c->target->key != c->key || !c->key ||
        c->key >= 16 || c->target->task_state != TASK_WAIT_FAULT || c->target->queued ||
        kernel.run_count >= KERNEL_RUN_QUEUE_CAPACITY ||
        memcmp(&c->state->ledger, &c->before_ledger, sizeof c->before_ledger) ||
        memcmp(&c->target->fault, &c->before_fault, sizeof c->before_fault) ||
        !kernel_epoch_current(c->epoch))
        return false;
    for (size_t i = 0; i < PG_CLIENTS; ++i)
        if (i != c->binding.range_id - 1 && c->state->keys[i] == c->key)
            return false;
    return c->before_fault.phase == PG_CLAIMED && c->record.token == c->binding.token &&
           c->record.target_pd == c->target->id && c->record.pager_pd == c->binding.pager_pd &&
           c->record.task_id == c->target->task_id &&
           c->record.saved_frame == (uint64_t)(uintptr_t)c->target->frame &&
           c->record.saved_rip == c->target->frame->rip && c->record.page_addr == c->address &&
           c->address == c->mapping.base_va + c->binding.page_index * 4096u &&
           c->leaf == &c->state->leaves[c->binding.range_id - 1][c->binding.page_index] &&
           *(volatile uint64_t *)(uintptr_t)*c->leaf == 0 &&
           c->expected == page_expected_leaf(c->state, c->binding);
}

static bool kernel_page_install_is_snapshotted(KernelPageInstallContext *c) {
    return kernel_page_install_current(c);
}

static bool kernel_page_install_after_valid(const KernelPageInstallContext *c) {
    if (!c->transformed || !kernel_page_install_current(c))
        return false;
    PgCommit installed = c->after_ledger.entries[c->binding.frame_index];
    return installed.present && installed.finalized &&
           pager_binding_equal(installed.binding, c->binding) && c->after_fault.phase == PG_IDLE &&
           c->after_fault.has_completed && c->after_fault.completed.token == c->binding.token &&
           c->after_fault.completed.target_pd == c->target->id;
}

static bool kernel_page_install_is_transformed(KernelPageInstallContext *c) {
    return kernel_page_install_after_valid(c);
}

static bool kernel_page_install_is_swapped(KernelPageInstallContext *c) {
    return c->swapped && kernel_page_install_after_valid(c);
}

static bool kernel_page_install_snapshot(KernelPageInstallContext *c) {
    if (!c->state || !c->binding.range_id || c->binding.range_id > PG_CLIENTS ||
        c->binding.frame_index >= PG_PAGES || c->binding.page_index >= PG_PAGES ||
        !kernel_epoch_current(kernel.epoch))
        return false;
    c->target = domain_get(c->binding.target_pd);
    if (!c->target)
        return false;
    c->epoch = kernel.epoch;
    c->key = c->state->keys[c->binding.range_id - 1];
    c->before_ledger = c->state->ledger;
    c->after_ledger = c->before_ledger;
    c->before_fault = c->target->fault;
    c->after_fault = c->before_fault;
    c->record = c->before_fault.active;
    c->leaf = &c->state->leaves[c->binding.range_id - 1][c->binding.page_index];
    c->address = c->record.page_addr;
    c->expected = page_expected_leaf(c->state, c->binding);
    c->snapshotted = true;
    return kernel_page_install_current(c);
}

static bool kernel_page_install_transform(KernelPageInstallContext *c) {
    if (!kernel_page_install_current(c) ||
        pager_ledger_install(&c->after_ledger, c->binding, c->mapping.quota) ||
        pager_ledger_finalize(&c->after_ledger, c->binding))
        return false;
    PgFaultResult completed =
        pager_fault_complete(&c->after_fault, c->binding.pager_pd, c->binding.token);
    if (completed.status || !completed.present ||
        completed.record.saved_frame != c->record.saved_frame ||
        completed.record.saved_rip != c->record.saved_rip)
        return false;
    c->transformed = true;
    return kernel_page_install_after_valid(c);
}

static bool kernel_page_install_swap(KernelPageInstallContext *c) {
    if (!kernel_page_install_after_valid(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_page_install_commit(KernelPageInstallContext *c) {
    if (!kernel_page_install_is_swapped(c))
        return false;
    *(volatile uint64_t *)(uintptr_t)*c->leaf = c->expected;
    c->state->ledger = c->after_ledger;
    c->target->fault = c->after_fault;
    c->target->task_state = TASK_READY;
    run_push(c->target->id);
    klog("[paging.fault.completed] target=%u task=%u token=%llu resume=1 saved_rip=%#llx\n",
         c->target->id, c->target->task_id, (unsigned long long)c->binding.token,
         (unsigned long long)c->record.saved_rip);
    cpu_invlpg(c->address);
    kernel_epoch_record_page_key(c->epoch, KERNEL_EPOCH_PAGE_INSTALL, c->target->id, c->address,
                                 c->binding.token, c->binding.frame_index, c->binding.rights,
                                 c->key);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_page_install_run_CHECKED 1
#define kernel_page_install_run_CHECK_FAILURE(ctx, kind, id, expected)                             \
    kernel_halt("page install epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_page_install_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_page_install_run_DEFINED

#if defined(kernel_page_install_run) || \
    defined(KernelPageInstallContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_page_install_run_outcome) || \
    defined(kernel_page_install_run_declined) || \
    defined(kernel_page_install_run_answered) || \
    defined(kernel_page_install_run_done) || \
    defined(kernel_page_install_run_raised) || \
    defined(kernel_page_install_run_suspended) || \
    defined(kernel_page_install_run_resource_failure) || \
    defined(kernel_page_install_run_engine_failure) || \
    defined(kernel_page_install_snapshot) || \
    defined(kernel_page_install_transform) || \
    defined(kernel_page_install_swap) || \
    defined(kernel_page_install_commit) || \
    defined(kernel_page_install_is_snapshotted) || \
    defined(kernel_page_install_is_transformed) || \
    defined(kernel_page_install_is_swapped) || \
    defined(kernel_page_install_run_decline_reason) || \
    defined(kernel_page_install_run_decline_count) || \
    defined(kernel_page_install_run_counters) || \
    defined(kernel_page_install_run_count_decline) || \
    defined(kernel_page_install_run_counts) || \
    defined(kernel_page_install_run_reason) || \
    defined(kernel_page_install_run_COUNTED) || \
    defined(kernel_page_install_run_declined_after_kernel_page_install_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_page_install_run_CHECKED
#define kernel_page_install_run_CHECKED 0
#endif
#ifdef kernel_page_install_run_COUNT_DECLINE
#define kernel_page_install_run_COUNTED 1
#else
#define kernel_page_install_run_COUNTED 0
#define kernel_page_install_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_page_install_run_CHECKED
#ifndef kernel_page_install_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_page_install_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_page_install_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_page_install_run_decline_reason {
    kernel_page_install_run_declined_after_kernel_page_install_snapshot_false,
    kernel_page_install_run_decline_count
} kernel_page_install_run_decline_reason;

typedef struct kernel_page_install_run_counters {
    uint64_t declines[kernel_page_install_run_decline_count ? kernel_page_install_run_decline_count : 1];
} kernel_page_install_run_counters;

#if kernel_page_install_run_COUNTED
static inline void
kernel_page_install_run_count_decline(kernel_page_install_run_counters *kernel_page_install_run_counts, kernel_page_install_run_decline_reason kernel_page_install_run_reason)
{
    if (kernel_page_install_run_counts && (unsigned)kernel_page_install_run_reason < (unsigned)kernel_page_install_run_decline_count &&
        kernel_page_install_run_counts->declines[kernel_page_install_run_reason] != UINT64_MAX)
        ++kernel_page_install_run_counts->declines[kernel_page_install_run_reason];
}
#endif

typedef enum kernel_page_install_run_outcome {
    kernel_page_install_run_declined,
    kernel_page_install_run_answered,
    kernel_page_install_run_done,
    kernel_page_install_run_raised,
    kernel_page_install_run_suspended,
    kernel_page_install_run_resource_failure,
    kernel_page_install_run_engine_failure
} kernel_page_install_run_outcome;

static inline kernel_page_install_run_outcome
kernel_page_install_run(KernelPageInstallContext *ctx)
{
    (void)ctx;
    if (!kernel_page_install_snapshot(ctx)) {
        kernel_page_install_run_COUNT_DECLINE(ctx, kernel_page_install_run_declined_after_kernel_page_install_snapshot_false);
        return kernel_page_install_run_declined;
    }
#if kernel_page_install_run_CHECKED
    if (!kernel_page_install_is_snapshotted(ctx)) {
        kernel_page_install_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_page_install_transform(ctx)) {
        return kernel_page_install_run_engine_failure;
    }
#if kernel_page_install_run_CHECKED
    if (!kernel_page_install_is_snapshotted(ctx)) {
        kernel_page_install_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_page_install_run_CHECKED
    if (!kernel_page_install_is_transformed(ctx)) {
        kernel_page_install_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_page_install_swap(ctx)) {
        return kernel_page_install_run_engine_failure;
    }
#if kernel_page_install_run_CHECKED
    if (!kernel_page_install_is_snapshotted(ctx)) {
        kernel_page_install_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_page_install_run_CHECKED
    if (!kernel_page_install_is_transformed(ctx)) {
        kernel_page_install_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_page_install_run_CHECKED
    if (!kernel_page_install_is_swapped(ctx)) {
        kernel_page_install_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_page_install_commit(ctx)) {
        return kernel_page_install_run_done;
    }
    return kernel_page_install_run_engine_failure;
}

static uint64_t native_commit(Domain *d, uint64_t map_slot, uint64_t pool_slot, uint64_t frame,
                              uint64_t token, uint64_t rights) {
    const Cap *held = mapping_cap(d, map_slot), *pool = cap_get(d, pool_slot);
    KernelPaging *s = &kernel.paging;
    if (!held || !pool || pool->kind != CAP_FRAME_POOL)
        return PG_INVALID;
    PgMapping m = held->mapping;
    if (!registered_map(d, m) || !pool_equal(pool->pool, s->pool) ||
        s->ledger.closed[m.range_id - 1].present)
        return PG_DENIED;
    PgCommit old = pager_ledger_token(&s->ledger, token);
    if (old.present) {
        PgBinding b = old.binding;
        if (b.frame_index != frame || b.rights != rights || b.target_pd != m.target_pd ||
            b.pager_pd != d->id || b.range_id != m.range_id)
            return PG_DENIED;
        return old.finalized ? PG_OK : native_fault_complete(b);
    }
    if (pager_ledger_is_retired(&s->ledger, m.range_id, token))
        return PG_DENIED;
    PgFaultResult fault = native_fault_query(m, d->id, token, true);
    if (fault.status)
        return fault.status;
    PgFaultRecord f = fault.record;
    PgBindingResult checked = pager_mapping_validate(
        m, pool->pool, d->id,
        (PgFaultView){f.task_id, f.token, f.target_pd, f.pager_pd, f.page_addr, f.error_code},
        frame, rights);
    uint64_t error = checked.status;
    PgBinding b = checked.value;
    if (!error)
        error = pager_ledger_check(&s->ledger, b, m.quota);
    uint64_t leaf = 0;
    if (!error) {
        leaf = s->leaves[b.range_id - 1][b.page_index];
        if (*(volatile uint64_t *)(uintptr_t)leaf)
            error = PG_DENIED;
        const volatile uint64_t *words = phys_to_virt(s->physical + frame * 4096);
        for (size_t i = 0; i < 512 && !error; ++i)
            if (words[i])
                error = PG_DENIED;
    }
    if (error) {
        Domain *target = domain_get(m.target_pd);
        if (!target || pager_fault_rollback(&target->fault, d->id, token))
            kernel_halt("paging no-effect rollback");
        return error;
    }
    if (!kernel_epoch_current(kernel.epoch)) {
        Domain *target = domain_get(m.target_pd);
        if (!target || pager_fault_rollback(&target->fault, d->id, token))
            kernel_halt("paging exhausted rollback");
        return PG_EXHAUSTED;
    }
    KernelPageInstallContext context = {.state = s, .mapping = m, .binding = b};
    if (kernel_page_install_run(&context) != kernel_page_install_run_done) {
        Domain *target = domain_get(m.target_pd);
        if (!target || pager_fault_rollback(&target->fault, d->id, token))
            kernel_halt("paging install rollback");
        return PG_DENIED;
    }
    klog(
        "[paging.leaf.commit] token=%llu target=%u page=%#llx frame=%llu rights=%llu pkey=%u zero=1 epoch=%llu runtime_alloc=0\n",
        (unsigned long long)token, m.target_pd, (unsigned long long)f.page_addr,
        (unsigned long long)frame, (unsigned long long)rights, s->keys[b.range_id - 1],
        (unsigned long long)kernel.epoch);
    return PG_OK;
}

static bool reclaim_identity(PgBinding b, PgMapping m, uint32_t pager, uint64_t frame,
                             uint64_t token) {
    return b.token == token && b.target_pd == m.target_pd && b.pager_pd == pager &&
           b.range_id == m.range_id && b.frame_index == frame;
}

static uint64_t expected_leaf(PgBinding b) {
    return page_expected_leaf(&kernel.paging, b);
}

typedef struct KernelPageReclaimContext {
    KernelPaging *state;
    PgMapping mapping;
    PgBinding binding;
    PgLedger before, after;
    uint64_t *leaf;
    uint64_t expected, address, epoch;
    bool snapshotted, transformed, swapped;
} KernelPageReclaimContext;

static bool kernel_page_reclaim_current(const KernelPageReclaimContext *c) {
    if (!c->snapshotted || c->state != &kernel.paging || !c->state->initialized ||
        !c->binding.token || !c->binding.target_pd || !c->binding.pager_pd ||
        !c->binding.range_id || c->binding.range_id > PG_CLIENTS ||
        c->binding.frame_index >= PG_PAGES || c->binding.page_index >= PG_PAGES ||
        !mapping_equal(c->state->mappings[c->binding.range_id - 1], c->mapping) ||
        c->mapping.target_pd != c->binding.target_pd ||
        memcmp(&c->state->ledger, &c->before, sizeof c->before) || !kernel_epoch_current(c->epoch))
        return false;
    PgCommit live = c->before.entries[c->binding.frame_index];
    return live.present && live.finalized && pager_binding_equal(live.binding, c->binding) &&
           c->leaf == &c->state->leaves[c->binding.range_id - 1][c->binding.page_index] &&
           c->address == c->mapping.base_va + c->binding.page_index * 4096u &&
           c->expected == expected_leaf(c->binding) &&
           (*(volatile uint64_t *)(uintptr_t)*c->leaf & ~UINT64_C(0x60)) == c->expected;
}

static bool kernel_page_reclaim_is_snapshotted(KernelPageReclaimContext *c) {
    return kernel_page_reclaim_current(c);
}

static bool kernel_page_reclaim_after_valid(const KernelPageReclaimContext *c) {
    if (!c->transformed || !kernel_page_reclaim_current(c))
        return false;
    PgMaybeBinding retired = c->after.retired[c->binding.frame_index];
    return !c->after.entries[c->binding.frame_index].present && retired.present &&
           pager_binding_equal(retired.value, c->binding) &&
           c->after.retired_highwater[c->binding.range_id - 1] >= c->binding.token;
}

static bool kernel_page_reclaim_is_transformed(KernelPageReclaimContext *c) {
    return kernel_page_reclaim_after_valid(c);
}

static bool kernel_page_reclaim_is_swapped(KernelPageReclaimContext *c) {
    return c->swapped && kernel_page_reclaim_after_valid(c);
}

static bool kernel_page_reclaim_snapshot(KernelPageReclaimContext *c) {
    if (!c->state || !c->binding.range_id || c->binding.range_id > PG_CLIENTS ||
        c->binding.frame_index >= PG_PAGES || c->binding.page_index >= PG_PAGES ||
        !kernel_epoch_current(kernel.epoch))
        return false;
    c->epoch = kernel.epoch;
    c->before = c->state->ledger;
    c->after = c->before;
    c->leaf = &c->state->leaves[c->binding.range_id - 1][c->binding.page_index];
    c->address = c->mapping.base_va + c->binding.page_index * 4096u;
    c->expected = expected_leaf(c->binding);
    c->snapshotted = true;
    return kernel_page_reclaim_current(c);
}

static bool kernel_page_reclaim_transform(KernelPageReclaimContext *c) {
    if (!kernel_page_reclaim_current(c) || pager_ledger_retire(&c->after, c->binding))
        return false;
    c->transformed = true;
    return kernel_page_reclaim_after_valid(c);
}

static bool kernel_page_reclaim_swap(KernelPageReclaimContext *c) {
    if (!kernel_page_reclaim_after_valid(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_page_reclaim_commit(KernelPageReclaimContext *c) {
    if (!kernel_page_reclaim_is_swapped(c))
        return false;
    *(volatile uint64_t *)(uintptr_t)*c->leaf = 0;
    c->state->ledger = c->after;
    cpu_invlpg(c->address);
    kernel_epoch_record_page(c->epoch, KERNEL_EPOCH_PAGE_RECLAIM, c->binding.target_pd, c->address,
                             c->binding.token, c->binding.frame_index, c->binding.rights);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_page_reclaim_run_CHECKED 1
#define kernel_page_reclaim_run_CHECK_FAILURE(ctx, kind, id, expected)                             \
    kernel_halt("page reclaim epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_page_reclaim_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_page_reclaim_run_DEFINED

#if defined(kernel_page_reclaim_run) || \
    defined(KernelPageReclaimContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_page_reclaim_run_outcome) || \
    defined(kernel_page_reclaim_run_declined) || \
    defined(kernel_page_reclaim_run_answered) || \
    defined(kernel_page_reclaim_run_done) || \
    defined(kernel_page_reclaim_run_raised) || \
    defined(kernel_page_reclaim_run_suspended) || \
    defined(kernel_page_reclaim_run_resource_failure) || \
    defined(kernel_page_reclaim_run_engine_failure) || \
    defined(kernel_page_reclaim_snapshot) || \
    defined(kernel_page_reclaim_transform) || \
    defined(kernel_page_reclaim_swap) || \
    defined(kernel_page_reclaim_commit) || \
    defined(kernel_page_reclaim_is_snapshotted) || \
    defined(kernel_page_reclaim_is_transformed) || \
    defined(kernel_page_reclaim_is_swapped) || \
    defined(kernel_page_reclaim_run_decline_reason) || \
    defined(kernel_page_reclaim_run_decline_count) || \
    defined(kernel_page_reclaim_run_counters) || \
    defined(kernel_page_reclaim_run_count_decline) || \
    defined(kernel_page_reclaim_run_counts) || \
    defined(kernel_page_reclaim_run_reason) || \
    defined(kernel_page_reclaim_run_COUNTED) || \
    defined(kernel_page_reclaim_run_declined_after_kernel_page_reclaim_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_page_reclaim_run_CHECKED
#define kernel_page_reclaim_run_CHECKED 0
#endif
#ifdef kernel_page_reclaim_run_COUNT_DECLINE
#define kernel_page_reclaim_run_COUNTED 1
#else
#define kernel_page_reclaim_run_COUNTED 0
#define kernel_page_reclaim_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_page_reclaim_run_CHECKED
#ifndef kernel_page_reclaim_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_page_reclaim_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_page_reclaim_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_page_reclaim_run_decline_reason {
    kernel_page_reclaim_run_declined_after_kernel_page_reclaim_snapshot_false,
    kernel_page_reclaim_run_decline_count
} kernel_page_reclaim_run_decline_reason;

typedef struct kernel_page_reclaim_run_counters {
    uint64_t declines[kernel_page_reclaim_run_decline_count ? kernel_page_reclaim_run_decline_count : 1];
} kernel_page_reclaim_run_counters;

#if kernel_page_reclaim_run_COUNTED
static inline void
kernel_page_reclaim_run_count_decline(kernel_page_reclaim_run_counters *kernel_page_reclaim_run_counts, kernel_page_reclaim_run_decline_reason kernel_page_reclaim_run_reason)
{
    if (kernel_page_reclaim_run_counts && (unsigned)kernel_page_reclaim_run_reason < (unsigned)kernel_page_reclaim_run_decline_count &&
        kernel_page_reclaim_run_counts->declines[kernel_page_reclaim_run_reason] != UINT64_MAX)
        ++kernel_page_reclaim_run_counts->declines[kernel_page_reclaim_run_reason];
}
#endif

typedef enum kernel_page_reclaim_run_outcome {
    kernel_page_reclaim_run_declined,
    kernel_page_reclaim_run_answered,
    kernel_page_reclaim_run_done,
    kernel_page_reclaim_run_raised,
    kernel_page_reclaim_run_suspended,
    kernel_page_reclaim_run_resource_failure,
    kernel_page_reclaim_run_engine_failure
} kernel_page_reclaim_run_outcome;

static inline kernel_page_reclaim_run_outcome
kernel_page_reclaim_run(KernelPageReclaimContext *ctx)
{
    (void)ctx;
    if (!kernel_page_reclaim_snapshot(ctx)) {
        kernel_page_reclaim_run_COUNT_DECLINE(ctx, kernel_page_reclaim_run_declined_after_kernel_page_reclaim_snapshot_false);
        return kernel_page_reclaim_run_declined;
    }
#if kernel_page_reclaim_run_CHECKED
    if (!kernel_page_reclaim_is_snapshotted(ctx)) {
        kernel_page_reclaim_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_page_reclaim_transform(ctx)) {
        return kernel_page_reclaim_run_engine_failure;
    }
#if kernel_page_reclaim_run_CHECKED
    if (!kernel_page_reclaim_is_snapshotted(ctx)) {
        kernel_page_reclaim_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_page_reclaim_run_CHECKED
    if (!kernel_page_reclaim_is_transformed(ctx)) {
        kernel_page_reclaim_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_page_reclaim_swap(ctx)) {
        return kernel_page_reclaim_run_engine_failure;
    }
#if kernel_page_reclaim_run_CHECKED
    if (!kernel_page_reclaim_is_snapshotted(ctx)) {
        kernel_page_reclaim_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_page_reclaim_run_CHECKED
    if (!kernel_page_reclaim_is_transformed(ctx)) {
        kernel_page_reclaim_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_page_reclaim_run_CHECKED
    if (!kernel_page_reclaim_is_swapped(ctx)) {
        kernel_page_reclaim_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_page_reclaim_commit(ctx)) {
        return kernel_page_reclaim_run_done;
    }
    return kernel_page_reclaim_run_engine_failure;
}

static uint64_t native_reclaim(Domain *d, uint64_t map_slot, uint64_t pool_slot, uint64_t frame,
                               uint64_t token) {
    const Cap *held = mapping_cap(d, map_slot), *pool = cap_get(d, pool_slot);
    KernelPaging *s = &kernel.paging;
    if (!held || !pool || pool->kind != CAP_FRAME_POOL)
        return PG_INVALID;
    PgMapping m = held->mapping;
    if (frame >= PG_PAGES || !token)
        return PG_DENIED;
    if (!registered_map(d, m) || !pool_equal(pool->pool, s->pool))
        return PG_DENIED;
    PgMaybeBinding done = s->ledger.retired[frame];
    if (done.present && reclaim_identity(done.value, m, d->id, frame, token))
        return PG_OK;
    PgCommit old = s->ledger.entries[frame];
    if (!old.present || !old.finalized || !reclaim_identity(old.binding, m, d->id, frame, token))
        return PG_DENIED;
    if (!kernel_epoch_current(kernel.epoch))
        return PG_EXHAUSTED;
    PgBinding b = old.binding;
    KernelPageReclaimContext context = {.state = s, .mapping = m, .binding = b};
    if (kernel_page_reclaim_run(&context) != kernel_page_reclaim_run_done)
        return PG_DENIED;
    klog(
        "[paging.leaf.reclaim] token=%llu target=%u page=%#llx frame=%llu cleared=1 tlb=1 epoch=%llu runtime_alloc=0\n",
        (unsigned long long)token, m.target_pd, (unsigned long long)context.address,
        (unsigned long long)frame, (unsigned long long)kernel.epoch);
    return PG_OK;
}

typedef struct KernelPageCleanupContext {
    KernelPaging *state;
    Domain *target;
    PgMapping mapping;
    PgCleanupPlan plan;
    PgLedger before_ledger, after_ledger;
    PgFaultState before_fault, after_fault;
    uint64_t epoch;
    uint32_t pager;
    uint8_t bitmap, key;
    bool snapshotted, transformed, swapped;
} KernelPageCleanupContext;

static bool kernel_page_cleanup_current(const KernelPageCleanupContext *c) {
    if (!c->snapshotted || c->state != &kernel.paging || !c->state->initialized || !c->target ||
        c->target != domain_get(c->mapping.target_pd) || !c->mapping.range_id ||
        c->mapping.range_id > PG_CLIENTS ||
        !mapping_equal(c->state->mappings[c->mapping.range_id - 1], c->mapping) ||
        c->pager != c->state->pager || c->target->task_state != TASK_EXITED || c->target->queued ||
        kernel.current == c->target->id || c->target->key != c->key || !c->key || c->key >= 16 ||
        memcmp(&c->state->ledger, &c->before_ledger, sizeof c->before_ledger) ||
        memcmp(&c->target->fault, &c->before_fault, sizeof c->before_fault) ||
        !kernel_epoch_current(c->epoch))
        return false;
    size_t range = c->mapping.range_id - 1;
    uint8_t bitmap = 0;
    for (size_t page = 0; page < PG_PAGES; ++page) {
        bool owned = false;
        uint64_t expected = 0;
        for (size_t frame = 0; frame < PG_PAGES; ++frame)
            if (c->plan.entries[frame].present && c->plan.entries[frame].value.page_index == page) {
                PgBinding b = c->plan.entries[frame].value;
                if (owned || b.frame_index != frame || b.range_id != c->mapping.range_id ||
                    b.target_pd != c->target->id || b.pager_pd != c->pager)
                    return false;
                owned = true;
                expected = page_expected_leaf(c->state, b);
                bitmap |= (uint8_t)(1u << frame);
            }
        uint64_t actual = *(volatile uint64_t *)(uintptr_t)c->state->leaves[range][page];
        if (owned ? (actual & ~UINT64_C(0x60)) != expected : actual != 0)
            return false;
    }
    return bitmap == c->bitmap;
}

static bool kernel_page_cleanup_is_snapshotted(KernelPageCleanupContext *c) {
    return kernel_page_cleanup_current(c);
}

static bool kernel_page_cleanup_after_valid(const KernelPageCleanupContext *c) {
    if (!c->transformed || !kernel_page_cleanup_current(c))
        return false;
    size_t range = c->mapping.range_id - 1;
    PgCleanup closed = c->after_ledger.closed[range];
    if (!closed.present || closed.target_pd != c->target->id ||
        closed.task_id != c->target->task_id || closed.frames != c->bitmap ||
        !c->after_fault.retired)
        return false;
    for (size_t frame = 0; frame < PG_PAGES; ++frame)
        if (c->plan.entries[frame].present) {
            PgMaybeBinding retired = c->after_ledger.retired[frame];
            if (c->after_ledger.entries[frame].present || !retired.present ||
                !pager_binding_equal(retired.value, c->plan.entries[frame].value))
                return false;
        }
    return true;
}

static bool kernel_page_cleanup_is_transformed(KernelPageCleanupContext *c) {
    return kernel_page_cleanup_after_valid(c);
}

static bool kernel_page_cleanup_is_swapped(KernelPageCleanupContext *c) {
    return c->swapped && kernel_page_cleanup_after_valid(c);
}

static bool kernel_page_cleanup_snapshot(KernelPageCleanupContext *c) {
    if (!c->state || !c->mapping.range_id || c->mapping.range_id > PG_CLIENTS ||
        !kernel_epoch_current(kernel.epoch))
        return false;
    c->target = domain_get(c->mapping.target_pd);
    if (!c->target)
        return false;
    c->epoch = kernel.epoch;
    c->key = c->state->keys[c->mapping.range_id - 1];
    c->before_ledger = c->state->ledger;
    c->after_ledger = c->before_ledger;
    c->before_fault = c->target->fault;
    c->after_fault = c->before_fault;
    if (pager_ledger_cleanup_plan(&c->before_ledger, c->mapping.range_id, c->target->id, c->pager,
                                  c->target->task_id, &c->plan))
        return false;
    c->bitmap = 0;
    for (size_t frame = 0; frame < PG_PAGES; ++frame)
        if (c->plan.entries[frame].present)
            c->bitmap |= (uint8_t)(1u << frame);
    c->snapshotted = true;
    return kernel_page_cleanup_current(c);
}

static bool kernel_page_cleanup_transform(KernelPageCleanupContext *c) {
    if (!kernel_page_cleanup_current(c))
        return false;
    if (pager_fault_retire(&c->after_fault, c->target->id, c->pager, c->target->task_id) ||
        pager_ledger_close(&c->after_ledger, c->mapping.range_id, c->target->id, c->pager,
                           c->target->task_id))
        return false;
    c->transformed = true;
    return kernel_page_cleanup_after_valid(c);
}

static bool kernel_page_cleanup_swap(KernelPageCleanupContext *c) {
    if (!kernel_page_cleanup_after_valid(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_page_cleanup_commit(KernelPageCleanupContext *c) {
    if (!kernel_page_cleanup_is_swapped(c))
        return false;
    size_t range = c->mapping.range_id - 1;
    for (size_t frame = 0; frame < PG_PAGES; ++frame)
        if (c->plan.entries[frame].present) {
            PgBinding b = c->plan.entries[frame].value;
            *(volatile uint64_t *)(uintptr_t)c->state->leaves[range][b.page_index] = 0;
        }
    c->state->ledger = c->after_ledger;
    c->target->fault = c->after_fault;
    for (size_t frame = 0; frame < PG_PAGES; ++frame)
        if (c->plan.entries[frame].present) {
            PgBinding b = c->plan.entries[frame].value;
            cpu_invlpg(c->mapping.base_va + b.page_index * 4096u);
        }
    kernel_epoch_record_cleanup(c->epoch, c->target->id, c->mapping.base_va, c->target->task_id,
                                c->bitmap, c->key);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_page_cleanup_run_CHECKED 1
#define kernel_page_cleanup_run_CHECK_FAILURE(ctx, kind, id, expected)                             \
    kernel_halt("page cleanup epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_page_cleanup_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_page_cleanup_run_DEFINED

#if defined(kernel_page_cleanup_run) || \
    defined(KernelPageCleanupContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_page_cleanup_run_outcome) || \
    defined(kernel_page_cleanup_run_declined) || \
    defined(kernel_page_cleanup_run_answered) || \
    defined(kernel_page_cleanup_run_done) || \
    defined(kernel_page_cleanup_run_raised) || \
    defined(kernel_page_cleanup_run_suspended) || \
    defined(kernel_page_cleanup_run_resource_failure) || \
    defined(kernel_page_cleanup_run_engine_failure) || \
    defined(kernel_page_cleanup_snapshot) || \
    defined(kernel_page_cleanup_transform) || \
    defined(kernel_page_cleanup_swap) || \
    defined(kernel_page_cleanup_commit) || \
    defined(kernel_page_cleanup_is_snapshotted) || \
    defined(kernel_page_cleanup_is_transformed) || \
    defined(kernel_page_cleanup_is_swapped) || \
    defined(kernel_page_cleanup_run_decline_reason) || \
    defined(kernel_page_cleanup_run_decline_count) || \
    defined(kernel_page_cleanup_run_counters) || \
    defined(kernel_page_cleanup_run_count_decline) || \
    defined(kernel_page_cleanup_run_counts) || \
    defined(kernel_page_cleanup_run_reason) || \
    defined(kernel_page_cleanup_run_COUNTED) || \
    defined(kernel_page_cleanup_run_declined_after_kernel_page_cleanup_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_page_cleanup_run_CHECKED
#define kernel_page_cleanup_run_CHECKED 0
#endif
#ifdef kernel_page_cleanup_run_COUNT_DECLINE
#define kernel_page_cleanup_run_COUNTED 1
#else
#define kernel_page_cleanup_run_COUNTED 0
#define kernel_page_cleanup_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_page_cleanup_run_CHECKED
#ifndef kernel_page_cleanup_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_page_cleanup_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_page_cleanup_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_page_cleanup_run_decline_reason {
    kernel_page_cleanup_run_declined_after_kernel_page_cleanup_snapshot_false,
    kernel_page_cleanup_run_decline_count
} kernel_page_cleanup_run_decline_reason;

typedef struct kernel_page_cleanup_run_counters {
    uint64_t declines[kernel_page_cleanup_run_decline_count ? kernel_page_cleanup_run_decline_count : 1];
} kernel_page_cleanup_run_counters;

#if kernel_page_cleanup_run_COUNTED
static inline void
kernel_page_cleanup_run_count_decline(kernel_page_cleanup_run_counters *kernel_page_cleanup_run_counts, kernel_page_cleanup_run_decline_reason kernel_page_cleanup_run_reason)
{
    if (kernel_page_cleanup_run_counts && (unsigned)kernel_page_cleanup_run_reason < (unsigned)kernel_page_cleanup_run_decline_count &&
        kernel_page_cleanup_run_counts->declines[kernel_page_cleanup_run_reason] != UINT64_MAX)
        ++kernel_page_cleanup_run_counts->declines[kernel_page_cleanup_run_reason];
}
#endif

typedef enum kernel_page_cleanup_run_outcome {
    kernel_page_cleanup_run_declined,
    kernel_page_cleanup_run_answered,
    kernel_page_cleanup_run_done,
    kernel_page_cleanup_run_raised,
    kernel_page_cleanup_run_suspended,
    kernel_page_cleanup_run_resource_failure,
    kernel_page_cleanup_run_engine_failure
} kernel_page_cleanup_run_outcome;

static inline kernel_page_cleanup_run_outcome
kernel_page_cleanup_run(KernelPageCleanupContext *ctx)
{
    (void)ctx;
    if (!kernel_page_cleanup_snapshot(ctx)) {
        kernel_page_cleanup_run_COUNT_DECLINE(ctx, kernel_page_cleanup_run_declined_after_kernel_page_cleanup_snapshot_false);
        return kernel_page_cleanup_run_declined;
    }
#if kernel_page_cleanup_run_CHECKED
    if (!kernel_page_cleanup_is_snapshotted(ctx)) {
        kernel_page_cleanup_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_page_cleanup_transform(ctx)) {
        return kernel_page_cleanup_run_engine_failure;
    }
#if kernel_page_cleanup_run_CHECKED
    if (!kernel_page_cleanup_is_snapshotted(ctx)) {
        kernel_page_cleanup_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_page_cleanup_run_CHECKED
    if (!kernel_page_cleanup_is_transformed(ctx)) {
        kernel_page_cleanup_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_page_cleanup_swap(ctx)) {
        return kernel_page_cleanup_run_engine_failure;
    }
#if kernel_page_cleanup_run_CHECKED
    if (!kernel_page_cleanup_is_snapshotted(ctx)) {
        kernel_page_cleanup_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_page_cleanup_run_CHECKED
    if (!kernel_page_cleanup_is_transformed(ctx)) {
        kernel_page_cleanup_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_page_cleanup_run_CHECKED
    if (!kernel_page_cleanup_is_swapped(ctx)) {
        kernel_page_cleanup_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_page_cleanup_commit(ctx)) {
        return kernel_page_cleanup_run_done;
    }
    return kernel_page_cleanup_run_engine_failure;
}

static uint64_t native_cleanup(Domain *d, uint64_t map_slot, uint64_t pool_slot, uint64_t *bitmap,
                               uint64_t *task) {
    const Cap *held = mapping_cap(d, map_slot), *pool = cap_get(d, pool_slot);
    KernelPaging *s = &kernel.paging;
    if (!held || !pool || pool->kind != CAP_FRAME_POOL)
        return PG_INVALID;
    PgMapping m = held->mapping;
    if (!registered_map(d, m) || !pool_equal(pool->pool, s->pool))
        return PG_DENIED;
    PgCleanup done = s->ledger.closed[m.range_id - 1];
    if (done.present) {
        *bitmap = done.frames;
        *task = done.task_id;
        return PG_OK;
    }
    Domain *target = domain_get(m.target_pd);
    if (!target)
        return PG_DENIED;
    if (target->task_state != TASK_EXITED)
        return PG_LIVE;
    if (kernel.current == target->id || target->queued)
        return PG_RETRY;
    if (!kernel_epoch_current(kernel.epoch))
        return PG_EXHAUSTED;
    KernelPageCleanupContext context = {.state = s, .target = target, .mapping = m, .pager = d->id};
    if (kernel_page_cleanup_run(&context) != kernel_page_cleanup_run_done)
        return PG_DENIED;
    done = s->ledger.closed[m.range_id - 1];
    *bitmap = done.frames;
    *task = done.task_id;
    klog(
        "[paging.target.cleanup] target=%u task=%u frames=%#x closed=1 detached=1 all_flushed=1 epoch=%llu runtime_alloc=0\n",
        target->id, target->task_id, (unsigned)done.frames, (unsigned long long)kernel.epoch);
    return PG_OK;
}

bool kernel_paging_cleanup_complete(uint32_t target) {
    KernelPaging *s = &kernel.paging;
    if (!s->initialized)
        return false;
    for (size_t i = 0; i < PG_CLIENTS; ++i)
        if (s->mappings[i].target_pd == target && !s->ledger.closed[i].present)
            return false;
    return true;
}

void kernel_paging_syscall(Domain *d, TrapFrame *f, uint64_t number) {
    uint64_t slot = f->rdi, second = f->rsi, frame = f->rdx, token = f->r10, rights = f->r8;
    f->rax = PG_INVALID;
    f->rsi = f->rdx = f->r10 = f->r8 = 0;
    const Cap *held = cap_get(d, slot);
    if (number == 59) {
        if (!held)
            return;
        if (held->kind == CAP_FRAME_POOL) {
            f->rax = 0;
            f->rsi = held->pool.base_va;
            f->rdx = held->pool.length;
            f->r8 = PG_READ | PG_WRITE;
        }
        if (held->kind == CAP_PAGER_MAPPING) {
            f->rax = 0;
            f->rsi = held->mapping.base_va;
            f->rdx = held->mapping.length;
            f->r10 = held->mapping.target_pd;
            f->r8 = held->mapping.rights;
        }
        if (held->kind == CAP_DEMAND_REGION) {
            f->rax = 0;
            f->rsi = held->demand.base_va;
            f->rdx = held->demand.length;
            f->r10 = d->id;
            f->r8 = held->demand.rights;
        }
    } else if (number == 56)
        f->rax = native_commit(d, slot, second, frame, token, rights);
    else if (number == 60)
        f->rax = native_reclaim(d, slot, second, frame, token);
    else if (number == 61)
        f->rax = native_cleanup(d, slot, second, &f->rsi, &f->rdx);
    else if (held && held->kind == CAP_PAGER_MAPPING) {
        PgMapping m = held->mapping;
        if (number == 57) {
            if (!registered_map(d, m) || kernel.paging.ledger.closed[m.range_id - 1].present) {
                f->rax = PG_DENIED;
                return;
            }
            PgFaultResult r = native_fault_query(m, d->id, second, false);
            f->rax = r.status;
            if (!r.status) {
                f->rsi = r.record.page_addr;
                f->rdx = r.record.error_code;
                f->r10 = m.target_pd;
                f->r8 = m.rights;
            }
        } else if (number == 58) {
            Domain *target = domain_get(m.target_pd);
            if (!target)
                return;
            PgFaultResult r = pager_fault_reject(&target->fault, d->id, second);
            f->rax = r.status;
            if (!r.status) {
                if (r.record.target_pd != target->id || r.record.task_id != target->task_id)
                    kernel_halt("rejected fault task identity");
                target->task_state = TASK_EXITED;
            }
        }
    }
}

bool kernel_pager_forward(Domain *d, TrapFrame *f, uint64_t address) {
    if (!kernel.paging.initialized || kernel.current != d->id || d->task_state != TASK_RUNNING ||
        !d->task_id || !pager_fault_eligible(address, f->error, f->cs))
        return false;
    const Cap *held = cap_get(d, 2);
    if (!held || held->kind != CAP_DOMAIN || held->target == d->id)
        return false;
    Domain *pager = domain_get(held->target);
    if (!pager)
        return false;
    uint64_t token = 0;
    if (pager_token_issue(&kernel.paging.last_token, &token))
        return false;
    PgFaultRecord r = {token,      d->id,   pager->id,
                       d->task_id, address, address & ~UINT64_C(4095),
                       f->error,   f->rip,  (uint64_t)(uintptr_t)f};
    if (pager_fault_prepare(&d->fault, r))
        return false;
    if (pager->reply_rights[d->id] == UINT64_MAX) {
        if (pager_fault_cancel(&d->fault, token))
            kernel_halt("fault reply reservation exhaustion");
        return false;
    }
    ++pager->reply_rights[d->id];
    RingEntry notice = {.type_id = 0x103,
                        .caller = d->id,
                        .arg0 = address,
                        .arg1 = f->error,
                        .arg2 = token,
                        .sender = d->id,
                        .caller_task = d->task_id,
                        .from_call = true};
    if (!domain_enqueue(pager, notice)) {
        --pager->reply_rights[d->id];
        if (pager_fault_cancel(&d->fault, token))
            kernel_halt("fault publication rollback");
        return false;
    }
    if (pager_fault_publish(&d->fault, token))
        kernel_halt("fault publication identity");
    d->frame = f;
    d->task_state = TASK_WAIT_FAULT;
    klog("[paging.fault.pending] target=%u task=%u token=%llu page=%#llx rip=%#llx\n", d->id,
         d->task_id, (unsigned long long)token, (unsigned long long)r.page_addr,
         (unsigned long long)r.saved_rip);
    return true;
}


