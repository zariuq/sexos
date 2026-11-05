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

uint64_t app_private_leaf_read(uint64_t leaf) {
    return *(volatile uint64_t *)(uintptr_t)leaf;
}

void app_private_leaf_write(uint64_t leaf, uint64_t value) {
    *(volatile uint64_t *)(uintptr_t)leaf = value;
}

void app_private_leaf_flush(uint64_t va) {
    cpu_invlpg(va);
}

KernelAppPrivate *kernel_app_get(uint32_t target) {
    for (uint32_t i = 0; i < kernel.app_count; ++i)
        if (kernel.apps[i].target == target)
            return &kernel.apps[i];
    return NULL;
}

void kernel_apps_plan(void) {
    if (kernel.genesis_sealed || kernel.app_count)
        kernel_halt("app genesis repeated");
#define APP_PLAN(p_target, p_ordinal, p_surface, p_shell_surface, p_lifecycle, p_close,            \
                 p_activation, p_predecessor, p_logical, p_route)                                  \
    do {                                                                                           \
        KernelAppPrivate *a = &kernel.apps[kernel.app_count++];                                    \
        *a = (KernelAppPrivate){.target = CURRENT_PD_##p_target,                                   \
                                .predecessor = p_predecessor,                                      \
                                .surface = p_surface,                                              \
                                .identity = {p_logical, p_ordinal + 1},                            \
                                .dormant = (p_ordinal != 0)};                                      \
    } while (0);
    SEXOS_CURRENT_APP_ROWS(APP_PLAN)
#undef APP_PLAN
}

bool kernel_app_capture_range(Domain *d, uint64_t start, uint64_t length) {
    KernelAppPrivate *a = kernel_app_get(d->id);
    if (!a)
        return true;
    if (kernel.genesis_sealed || a->captured || !d->key || d->key >= 16)
        return false;
    if (!length)
        return true;
    if (start > UINT64_MAX - (length - 1))
        return false;
    uint64_t first = start & ~UINT64_C(4095), last = (start + length - 1) & ~UINT64_C(4095);
    for (uint64_t va = first;; va += 4096) {
        bool found = false;
        for (size_t i = 0; i < a->image.count; ++i)
            if (a->image.pages[i].va == va)
                found = true;
        if (!found) {
            if (a->image.count == APP_PRIVATE_MAX_PAGES)
                return false;
            uint64_t leaf = page_leaf_address(va);
            if (!leaf)
                return false;
            uint64_t expected = app_private_leaf_read(leaf) & ~UINT64_C(0x60);
            if ((expected & 5) != 5 || ((expected >> PTE_KEY_SHIFT) & 15) != d->key)
                return false;
            for (size_t i = 0; i < a->image.count; ++i)
                if ((a->image.pages[i].expected & APP_PRIVATE_PHYSICAL) ==
                    (expected & APP_PRIVATE_PHYSICAL))
                    return false;
            a->image.pages[a->image.count++] = (AppPrivatePage){va, leaf, expected};
        }
        if (va == last)
            break;
    }
    return true;
}

bool kernel_app_finish_capture(Domain *d) {
    KernelAppPrivate *a = kernel_app_get(d->id);
    if (!a)
        return true;
    if (kernel.genesis_sealed || a->captured || !a->image.count || !d->task_id)
        return false;
    for (uint32_t i = 0; i < kernel.app_count; ++i) {
        const KernelAppPrivate *other = &kernel.apps[i];
        if (other == a)
            continue;
        for (size_t j = 0; j < other->image.count; ++j)
            for (size_t k = 0; k < a->image.count; ++k)
                if ((other->image.pages[j].expected & APP_PRIVATE_PHYSICAL) ==
                    (a->image.pages[k].expected & APP_PRIVATE_PHYSICAL))
                    return false;
    }
    a->task = d->task_id;
    a->key = d->key;
    a->captured = true;
    return true;
}

bool kernel_app_reserve_successor(uint32_t predecessor, uint32_t successor) {
    if (kernel.genesis_sealed || kernel.scheduler_running || kernel.current)
        return false;
    KernelAppPrivate *old = kernel_app_get(predecessor), *next = kernel_app_get(successor);
    Domain *before = domain_get(predecessor), *after = domain_get(successor);
    if (!old || !next || !before || !after || !old->captured || !next->captured ||
        next->predecessor != predecessor || !next->dormant || before == after ||
        before->key != after->key || old->task != before->task_id || next->task != after->task_id ||
        after->task_state != TASK_READY)
        return false;
    AppNextIncarnation expected = app_incarnation_next(old->identity);
    if (!expected.present || expected.value.logical_app != next->identity.logical_app ||
        expected.value.number != next->identity.number)
        return false;
    AppPrivateResult result = app_private_retire(&next->image);
    if (result.status || result.count != next->image.count)
        return false;
    after->task_state = TASK_DORMANT;
    return true;
}

void kernel_apps_workspaces_boot(void) {
    Domain *pager = domain_get(CURRENT_PD_sext);
    if (kernel.genesis_sealed || kernel.scheduler_running || kernel.current || !pager)
        kernel_halt("workspace BOOT phase");
    for (uint32_t index = 0; index < kernel.app_count; ++index) {
        KernelAppPrivate *a = &kernel.apps[index];
        if (!a->ready || !a->captured || !a->image.count ||
            a->image.count > APP_PRIVATE_MAX_PAGES || a->pool || a->workspace.count)
            kernel_halt("workspace BOOT manifest");
        uint64_t bytes = a->image.count * UINT64_C(4096), alias = va_alloc(bytes);
        if (alias == NO_ADDRESS)
            kernel_halt("workspace BOOT address");
        for (size_t i = 0; i < a->image.count; ++i) {
            uint64_t address = alias + i * 4096,
                     physical = a->image.pages[i].expected & APP_PRIVATE_PHYSICAL;
            if (!map_page(address, physical, PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE, pager->key))
                kernel_halt("workspace BOOT tables");
            uint64_t leaf = page_leaf_address(address);
            if (!leaf)
                kernel_halt("workspace BOOT leaf address");
            uint64_t expected = app_private_leaf_read(leaf) & ~UINT64_C(0x60);
            if ((expected & 7) != 7 || !(expected & PTE_NO_EXECUTE) ||
                ((expected >> 59) & 15) != pager->key)
                kernel_halt("workspace BOOT leaf");
            a->workspace.pages[a->workspace.count++] = (AppPrivatePage){address, leaf, expected};
            app_private_leaf_write(leaf, 0);
            app_private_leaf_flush(address);
        }
        a->pool = alias;
        klog(
            "[boot.app.workspace] target=%u task=%u bytes=%llu pager=%u aliases_absent=%llu tables_preallocated=1\n",
            a->target, a->task, (unsigned long long)bytes, pager->id,
            (unsigned long long)a->workspace.count);
    }
}

static uint64_t kernel_app_dead(const KernelAppPrivate *a) {
    Domain *d = domain_get(a->target);
    if (!d || d->task_id != a->task)
        return APP_PRIVATE_DENIED;
    if (d->task_state != TASK_EXITED)
        return APP_PRIVATE_LIVE;
    if (kernel.current == d->id || d->queued)
        return APP_PRIVATE_RETRY;
    return APP_PRIVATE_OK;
}

void kernel_app_query(Domain *holder, TrapFrame *f, uint64_t number) {
    uint64_t slot = f->rdi, kind = f->rsi;
    f->rax = APP_PRIVATE_INVALID;
    f->rsi = f->rdx = f->r10 = f->r8 = 0;
    if (number != 63 && number != 64)
        return;
    const Cap *held = cap_get(holder, slot);
    if (!held || held->kind != CAP_APP_LIFECYCLE)
        return;
    AppLifecycleGrant g = held->lifecycle;
    KernelAppPrivate *a = kernel_app_get(g.target_pd);
    if (!a || !a->ready) {
        f->rax = APP_PRIVATE_DENIED;
        return;
    }
    uint8_t right = number == 64 ? APP_PRIVATE_ACK : APP_PRIVATE_QUERY;
    uint64_t error = app_lifecycle_authorize(g, holder->id, a->target, a->task, right);
    if (error) {
        f->rax = error;
        return;
    }
    if (number == 63 && kind == 1) {
        f->rax = 0;
        f->rsi = a->target;
        f->rdx = a->task;
        f->r10 = a->surface;
        f->r8 = a->coordinator;
        return;
    }
    if (number == 63 && kind != 0)
        return;
    if (number == 64 && kind != a->task) {
        f->rax = APP_PRIVATE_DENIED;
        return;
    }
    error = kernel_app_dead(a);
    if (error) {
        f->rax = error;
        return;
    }
    f->rax = 0;
    if (number == 63) {
        f->rsi = a->target;
        f->rdx = a->task;
        f->r10 = APP_PRIVATE_REQUIRED;
        f->r8 = a->acked;
        return;
    }
    uint8_t before = a->acked;
    a->acked |= g.ack_bit;
    if (a->acked != before)
        klog("[app.service.ack] target=%u task=%u holder=%u role=%u acked=%u required=31\n",
             a->target, a->task, holder->id, g.ack_bit, a->acked);
}

typedef struct KernelAppHandoverContext {
    KernelAppPrivate *app;
    Domain *target, *pager;
    uint64_t epoch, ring_revision;
    uint32_t ring_head, messages, revoked;
    bool snapshotted, transformed, swapped;
} KernelAppHandoverContext;

static uint32_t kernel_app_cap_count(Domain *target) {
    uint32_t count = 0;
    for (uint64_t slot = 1; slot < KERNEL_CAP_SLOTS; ++slot)
        if (cap_get(target, slot))
            ++count;
    return count;
}

static bool kernel_app_handover_current(const KernelAppHandoverContext *c) {
    if (!c->snapshotted || !c->app || c->app != kernel_app_get(c->target ? c->target->id : 0) ||
        !c->target || c->target != domain_get(c->app->target) || !c->pager ||
        c->pager != domain_get(c->app->pager) || c->app->retired || !c->app->ready ||
        c->app->task != c->target->task_id || c->app->key != c->target->key || !c->app->key ||
        c->app->key >= 16 || !c->pager->key || c->pager->key >= 16 ||
        c->app->key == c->pager->key || c->target->task_state != TASK_EXITED || c->target->queued ||
        kernel.current == c->target->id || c->app->acked != APP_PRIVATE_REQUIRED ||
        !kernel_paging_cleanup_complete(c->target->id) || !c->app->pool ||
        c->app->image.count != c->app->workspace.count || !c->app->image.count ||
        c->app->image.count > APP_PRIVATE_MAX_PAGES || c->target->ring.head != c->ring_head ||
        c->target->ring.count != c->messages || c->target->ring.revision != c->ring_revision ||
        c->ring_revision == UINT64_MAX || kernel_app_cap_count(c->target) != c->revoked ||
        !kernel_epoch_current(c->epoch))
        return false;
    AppPrivateResult checked = app_private_handover_check(&c->app->image, &c->app->workspace);
    if (checked.status || checked.count != c->app->image.count)
        return false;
    for (size_t i = 0; i < c->app->image.count; ++i) {
        AppPrivatePage old = c->app->image.pages[i], next = c->app->workspace.pages[i];
        if (((old.expected >> 59) & 15) != c->app->key ||
            ((next.expected >> 59) & 15) != c->pager->key || next.va != c->app->pool + i * 4096 ||
            next.leaf != page_leaf_address(next.va) || old.leaf != page_leaf_address(old.va))
            return false;
    }
    return true;
}

static bool kernel_app_handover_is_snapshotted(KernelAppHandoverContext *c) {
    return kernel_app_handover_current(c);
}

static bool kernel_app_handover_is_transformed(KernelAppHandoverContext *c) {
    return c->transformed && kernel_app_handover_current(c);
}

static bool kernel_app_handover_is_swapped(KernelAppHandoverContext *c) {
    return c->swapped && c->transformed && kernel_app_handover_current(c);
}

static bool kernel_app_handover_snapshot(KernelAppHandoverContext *c) {
    if (!c->app || !kernel_epoch_current(kernel.epoch))
        return false;
    c->target = domain_get(c->app->target);
    c->pager = domain_get(c->app->pager);
    if (!c->target || !c->pager)
        return false;
    c->epoch = kernel.epoch;
    c->ring_head = c->target->ring.head;
    c->messages = c->target->ring.count;
    c->ring_revision = c->target->ring.revision;
    c->revoked = kernel_app_cap_count(c->target);
    c->snapshotted = true;
    return kernel_app_handover_current(c);
}

static bool kernel_app_handover_transform(KernelAppHandoverContext *c) {
    if (!kernel_app_handover_current(c))
        return false;
    c->transformed = true;
    return true;
}

static bool kernel_app_handover_swap(KernelAppHandoverContext *c) {
    if (!kernel_app_handover_is_transformed(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_app_handover_commit(KernelAppHandoverContext *c) {
    if (!kernel_app_handover_is_swapped(c))
        return false;
    for (size_t i = 0; i < c->app->image.count; ++i) {
        app_private_leaf_write(c->app->image.pages[i].leaf, 0);
        app_private_leaf_flush(c->app->image.pages[i].va);
    }
    for (size_t i = 0; i < c->app->workspace.count; ++i) {
        app_private_leaf_write(c->app->workspace.pages[i].leaf,
                               c->app->workspace.pages[i].expected);
        app_private_leaf_flush(c->app->workspace.pages[i].va);
    }
    c->target->ring = (Ring){.revision = c->ring_revision + 1u};
    c->target->reply_ready = false;
    c->target->reply_value = 0;
    c->target->reply_caller = c->target->reply_task = 0;
    for (size_t i = 0; i < KERNEL_MAX_DOMAINS; ++i)
        c->target->reply_rights[i] = 0;
    for (size_t i = 0; i < KERNEL_RPC_CAPACITY; ++i)
        c->target->rpc[i] = (RpcSlot){.state = RPC_FREE};
    c->target->rpc_enabled = false;
    for (uint64_t slot = 1; slot < KERNEL_CAP_SLOTS; ++slot)
        if (cap_get(c->target, slot)) {
#if SEXOS_KERNEL_CURRENT_PDX
            c->target->caps[slot] = NULL;
#else
            c->target->caps[slot] = (Cap){0};
#endif
        }
    c->app->retired = true;
    kernel_epoch_record_handover(c->epoch, c->target->id, c->target->task_id, c->app->pool,
                                 c->app->image.count, c->app->key, c->pager->key);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_app_handover_run_CHECKED 1
#define kernel_app_handover_run_CHECK_FAILURE(ctx, kind, id, expected)                             \
    kernel_halt("app handover epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_app_handover_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_app_handover_run_DEFINED

#if defined(kernel_app_handover_run) || \
    defined(KernelAppHandoverContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_app_handover_run_outcome) || \
    defined(kernel_app_handover_run_declined) || \
    defined(kernel_app_handover_run_answered) || \
    defined(kernel_app_handover_run_done) || \
    defined(kernel_app_handover_run_raised) || \
    defined(kernel_app_handover_run_suspended) || \
    defined(kernel_app_handover_run_resource_failure) || \
    defined(kernel_app_handover_run_engine_failure) || \
    defined(kernel_app_handover_snapshot) || \
    defined(kernel_app_handover_transform) || \
    defined(kernel_app_handover_swap) || \
    defined(kernel_app_handover_commit) || \
    defined(kernel_app_handover_is_snapshotted) || \
    defined(kernel_app_handover_is_transformed) || \
    defined(kernel_app_handover_is_swapped) || \
    defined(kernel_app_handover_run_decline_reason) || \
    defined(kernel_app_handover_run_decline_count) || \
    defined(kernel_app_handover_run_counters) || \
    defined(kernel_app_handover_run_count_decline) || \
    defined(kernel_app_handover_run_counts) || \
    defined(kernel_app_handover_run_reason) || \
    defined(kernel_app_handover_run_COUNTED) || \
    defined(kernel_app_handover_run_declined_after_kernel_app_handover_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_app_handover_run_CHECKED
#define kernel_app_handover_run_CHECKED 0
#endif
#ifdef kernel_app_handover_run_COUNT_DECLINE
#define kernel_app_handover_run_COUNTED 1
#else
#define kernel_app_handover_run_COUNTED 0
#define kernel_app_handover_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_app_handover_run_CHECKED
#ifndef kernel_app_handover_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_app_handover_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_app_handover_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_app_handover_run_decline_reason {
    kernel_app_handover_run_declined_after_kernel_app_handover_snapshot_false,
    kernel_app_handover_run_decline_count
} kernel_app_handover_run_decline_reason;

typedef struct kernel_app_handover_run_counters {
    uint64_t declines[kernel_app_handover_run_decline_count ? kernel_app_handover_run_decline_count : 1];
} kernel_app_handover_run_counters;

#if kernel_app_handover_run_COUNTED
static inline void
kernel_app_handover_run_count_decline(kernel_app_handover_run_counters *kernel_app_handover_run_counts, kernel_app_handover_run_decline_reason kernel_app_handover_run_reason)
{
    if (kernel_app_handover_run_counts && (unsigned)kernel_app_handover_run_reason < (unsigned)kernel_app_handover_run_decline_count &&
        kernel_app_handover_run_counts->declines[kernel_app_handover_run_reason] != UINT64_MAX)
        ++kernel_app_handover_run_counts->declines[kernel_app_handover_run_reason];
}
#endif

typedef enum kernel_app_handover_run_outcome {
    kernel_app_handover_run_declined,
    kernel_app_handover_run_answered,
    kernel_app_handover_run_done,
    kernel_app_handover_run_raised,
    kernel_app_handover_run_suspended,
    kernel_app_handover_run_resource_failure,
    kernel_app_handover_run_engine_failure
} kernel_app_handover_run_outcome;

static inline kernel_app_handover_run_outcome
kernel_app_handover_run(KernelAppHandoverContext *ctx)
{
    (void)ctx;
    if (!kernel_app_handover_snapshot(ctx)) {
        kernel_app_handover_run_COUNT_DECLINE(ctx, kernel_app_handover_run_declined_after_kernel_app_handover_snapshot_false);
        return kernel_app_handover_run_declined;
    }
#if kernel_app_handover_run_CHECKED
    if (!kernel_app_handover_is_snapshotted(ctx)) {
        kernel_app_handover_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_app_handover_transform(ctx)) {
        return kernel_app_handover_run_engine_failure;
    }
#if kernel_app_handover_run_CHECKED
    if (!kernel_app_handover_is_snapshotted(ctx)) {
        kernel_app_handover_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_app_handover_run_CHECKED
    if (!kernel_app_handover_is_transformed(ctx)) {
        kernel_app_handover_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_app_handover_swap(ctx)) {
        return kernel_app_handover_run_engine_failure;
    }
#if kernel_app_handover_run_CHECKED
    if (!kernel_app_handover_is_snapshotted(ctx)) {
        kernel_app_handover_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_app_handover_run_CHECKED
    if (!kernel_app_handover_is_transformed(ctx)) {
        kernel_app_handover_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_app_handover_run_CHECKED
    if (!kernel_app_handover_is_swapped(ctx)) {
        kernel_app_handover_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_app_handover_commit(ctx)) {
        return kernel_app_handover_run_done;
    }
    return kernel_app_handover_run_engine_failure;
}

void kernel_app_retire(Domain *holder, TrapFrame *f) {
    uint64_t slot = f->rdi;
    f->rax = APP_PRIVATE_INVALID;
    f->rsi = f->rdx = f->r10 = f->r8 = 0;
    if (!kernel_current_task(holder))
        return;
    const Cap *held = cap_get(holder, slot);
    if (!held || held->kind != CAP_APP_LIFECYCLE)
        return;
    AppLifecycleGrant g = held->lifecycle;
    KernelAppPrivate *a = kernel_app_get(g.target_pd);
    if (!a || !a->ready || a->pager != holder->id) {
        f->rax = APP_PRIVATE_DENIED;
        return;
    }
    uint64_t error = app_lifecycle_authorize(g, holder->id, a->target, a->task, APP_PRIVATE_RETIRE);
    if (error) {
        f->rax = error;
        return;
    }
    if (!a->retired) {
        error = kernel_app_dead(a);
        if (error) {
            f->rax = error;
            return;
        }
        if (a->acked != APP_PRIVATE_REQUIRED || !kernel_paging_cleanup_complete(a->target)) {
            f->rax = APP_PRIVATE_RETRY;
            return;
        }
        if (!kernel_epoch_current(kernel.epoch)) {
            f->rax = APP_PRIVATE_RETRY;
            return;
        }
        KernelAppHandoverContext context = {.app = a};
        if (kernel_app_handover_run(&context) != kernel_app_handover_run_done) {
            f->rax = APP_PRIVATE_DENIED;
            return;
        }
        klog("[app.caps.revoked] target=%u task=%u revoked=%u remaining=0\n", a->target, a->task,
             context.revoked);
        klog(
            "[app.private.retired] target=%u task=%u pages=%llu bytes=%llu acked=%u required=31 all_flushed=1 pool_reusable=1 messages=%u signals=0 metadata=retained epoch=%llu runtime_alloc=0\n",
            a->target, a->task, (unsigned long long)a->image.count,
            (unsigned long long)(a->image.count * 4096), a->acked, context.messages,
            (unsigned long long)kernel.epoch);
    }
    f->rax = 0;
    f->rsi = a->pool;
    f->rdx = a->image.count * UINT64_C(4096);
    f->r10 = a->task;
}

void kernel_app_poll(void) {
    if (!SEXOS_KERNEL_CURRENT_PDX || !kernel.genesis_sealed)
        return;
    const uint32_t services[] = {CURRENT_PD_silk_shell, CURRENT_PD_sexdisplay, CURRENT_PD_sexfiles,
                                 CURRENT_PD_sexnet, CURRENT_PD_sexbell};
    for (uint32_t index = 0; index < kernel.app_count; ++index) {
        KernelAppPrivate *a = &kernel.apps[index];
        Domain *target = domain_get(a->target);
        if (!a->ready || a->retired || !target || target->task_state != TASK_EXITED)
            continue;
        if (!a->kicked) {
            for (size_t role = 0; role < 6; ++role)
                kernel_task_wake(domain_get(role < 5 ? services[role] : a->pager));
            a->kicked = true;
        }
        if (kernel_app_dead(a))
            continue;
        for (uint8_t role = 0; role < 6; ++role) {
            if (!app_notice_due(a->acked, a->hinted, a->last_hint[role], kernel.ticks, role))
                continue;
            Domain *holder = domain_get(role < 5 ? services[role] : a->pager);
            if (!holder)
                continue;
            a->hinted |= (uint8_t)(1u << role);
            a->last_hint[role] = kernel.ticks;
            RingEntry notice = {.type_id = 0x107, .arg0 = a->target, .arg1 = a->task};
            if (domain_enqueue(holder, notice))
                klog("[app.exit.notice] target=%u task=%u holder=%u detached=1 accepted=1\n",
                     a->target, a->task, holder->id);
        }
    }
}

typedef struct KernelAppActivationContext {
    KernelAppPrivate *old, *next;
    Domain *target;
    uint64_t epoch;
    uint32_t run_count;
    bool snapshotted, transformed, swapped;
} KernelAppActivationContext;

static bool kernel_app_activation_current(const KernelAppActivationContext *c) {
    if (!c->snapshotted || !c->old || !c->next || !c->target ||
        c->old != kernel_app_get(c->old->target) || c->next != kernel_app_get(c->next->target) ||
        c->target != domain_get(c->next->target) || !c->old->retired ||
        c->old->acked != APP_PRIVATE_REQUIRED || kernel_app_dead(c->old) != APP_PRIVATE_OK ||
        c->next->activated || !c->next->dormant || c->next->predecessor != c->old->target ||
        c->target->task_id != c->next->task || !c->target->frame || !c->target->frame->rip ||
        c->target->task_state != TASK_DORMANT || c->target->queued || c->old->key != c->next->key ||
        c->next->key != c->target->key || !c->target->key || c->target->key >= 16 ||
        !c->old->image.count || c->old->image.count > APP_PRIVATE_MAX_PAGES ||
        !c->next->image.count || c->next->image.count > APP_PRIVATE_MAX_PAGES ||
        kernel.run_count != c->run_count || c->run_count >= KERNEL_RUN_QUEUE_CAPACITY ||
        !kernel_epoch_current(c->epoch))
        return false;
    for (size_t i = 0; i < c->old->image.count; ++i)
        if (app_private_leaf_read(c->old->image.pages[i].leaf))
            return false;
    AppPrivateResult checked = app_private_restore_check(&c->next->image);
    return !checked.status && checked.count == c->next->image.count;
}

static bool kernel_app_activation_is_snapshotted(KernelAppActivationContext *c) {
    return kernel_app_activation_current(c);
}

static bool kernel_app_activation_is_transformed(KernelAppActivationContext *c) {
    return c->transformed && kernel_app_activation_current(c);
}

static bool kernel_app_activation_is_swapped(KernelAppActivationContext *c) {
    return c->swapped && c->transformed && kernel_app_activation_current(c);
}

static bool kernel_app_activation_snapshot(KernelAppActivationContext *c) {
    if (!c->old || !c->next || !kernel_epoch_current(kernel.epoch))
        return false;
    c->target = domain_get(c->next->target);
    if (!c->target)
        return false;
    c->epoch = kernel.epoch;
    c->run_count = kernel.run_count;
    c->snapshotted = true;
    return kernel_app_activation_current(c);
}

static bool kernel_app_activation_transform(KernelAppActivationContext *c) {
    if (!kernel_app_activation_current(c))
        return false;
    c->transformed = true;
    return true;
}

static bool kernel_app_activation_swap(KernelAppActivationContext *c) {
    if (!kernel_app_activation_is_transformed(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_app_activation_commit(KernelAppActivationContext *c) {
    if (!kernel_app_activation_is_swapped(c))
        return false;
    for (size_t i = 0; i < c->next->image.count; ++i) {
        app_private_leaf_write(c->next->image.pages[i].leaf, c->next->image.pages[i].expected);
        app_private_leaf_flush(c->next->image.pages[i].va);
    }
    c->next->activated = true;
    c->next->dormant = false;
    c->target->task_state = TASK_READY;
    run_push(c->target->id);
    kernel_epoch_record_activation(c->epoch, c->old->target, c->next->target, c->next->task,
                                   c->next->surface, c->next->image.count, c->target->key);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_app_activation_run_CHECKED 1
#define kernel_app_activation_run_CHECK_FAILURE(ctx, kind, id, expected)                           \
    kernel_halt("app activation epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_app_activation_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_app_activation_run_DEFINED

#if defined(kernel_app_activation_run) || \
    defined(KernelAppActivationContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_app_activation_run_outcome) || \
    defined(kernel_app_activation_run_declined) || \
    defined(kernel_app_activation_run_answered) || \
    defined(kernel_app_activation_run_done) || \
    defined(kernel_app_activation_run_raised) || \
    defined(kernel_app_activation_run_suspended) || \
    defined(kernel_app_activation_run_resource_failure) || \
    defined(kernel_app_activation_run_engine_failure) || \
    defined(kernel_app_activation_snapshot) || \
    defined(kernel_app_activation_transform) || \
    defined(kernel_app_activation_swap) || \
    defined(kernel_app_activation_commit) || \
    defined(kernel_app_activation_is_snapshotted) || \
    defined(kernel_app_activation_is_transformed) || \
    defined(kernel_app_activation_is_swapped) || \
    defined(kernel_app_activation_run_decline_reason) || \
    defined(kernel_app_activation_run_decline_count) || \
    defined(kernel_app_activation_run_counters) || \
    defined(kernel_app_activation_run_count_decline) || \
    defined(kernel_app_activation_run_counts) || \
    defined(kernel_app_activation_run_reason) || \
    defined(kernel_app_activation_run_COUNTED) || \
    defined(kernel_app_activation_run_declined_after_kernel_app_activation_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_app_activation_run_CHECKED
#define kernel_app_activation_run_CHECKED 0
#endif
#ifdef kernel_app_activation_run_COUNT_DECLINE
#define kernel_app_activation_run_COUNTED 1
#else
#define kernel_app_activation_run_COUNTED 0
#define kernel_app_activation_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_app_activation_run_CHECKED
#ifndef kernel_app_activation_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_app_activation_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_app_activation_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_app_activation_run_decline_reason {
    kernel_app_activation_run_declined_after_kernel_app_activation_snapshot_false,
    kernel_app_activation_run_decline_count
} kernel_app_activation_run_decline_reason;

typedef struct kernel_app_activation_run_counters {
    uint64_t declines[kernel_app_activation_run_decline_count ? kernel_app_activation_run_decline_count : 1];
} kernel_app_activation_run_counters;

#if kernel_app_activation_run_COUNTED
static inline void
kernel_app_activation_run_count_decline(kernel_app_activation_run_counters *kernel_app_activation_run_counts, kernel_app_activation_run_decline_reason kernel_app_activation_run_reason)
{
    if (kernel_app_activation_run_counts && (unsigned)kernel_app_activation_run_reason < (unsigned)kernel_app_activation_run_decline_count &&
        kernel_app_activation_run_counts->declines[kernel_app_activation_run_reason] != UINT64_MAX)
        ++kernel_app_activation_run_counts->declines[kernel_app_activation_run_reason];
}
#endif

typedef enum kernel_app_activation_run_outcome {
    kernel_app_activation_run_declined,
    kernel_app_activation_run_answered,
    kernel_app_activation_run_done,
    kernel_app_activation_run_raised,
    kernel_app_activation_run_suspended,
    kernel_app_activation_run_resource_failure,
    kernel_app_activation_run_engine_failure
} kernel_app_activation_run_outcome;

static inline kernel_app_activation_run_outcome
kernel_app_activation_run(KernelAppActivationContext *ctx)
{
    (void)ctx;
    if (!kernel_app_activation_snapshot(ctx)) {
        kernel_app_activation_run_COUNT_DECLINE(ctx, kernel_app_activation_run_declined_after_kernel_app_activation_snapshot_false);
        return kernel_app_activation_run_declined;
    }
#if kernel_app_activation_run_CHECKED
    if (!kernel_app_activation_is_snapshotted(ctx)) {
        kernel_app_activation_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_app_activation_transform(ctx)) {
        return kernel_app_activation_run_engine_failure;
    }
#if kernel_app_activation_run_CHECKED
    if (!kernel_app_activation_is_snapshotted(ctx)) {
        kernel_app_activation_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_app_activation_run_CHECKED
    if (!kernel_app_activation_is_transformed(ctx)) {
        kernel_app_activation_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_app_activation_swap(ctx)) {
        return kernel_app_activation_run_engine_failure;
    }
#if kernel_app_activation_run_CHECKED
    if (!kernel_app_activation_is_snapshotted(ctx)) {
        kernel_app_activation_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_app_activation_run_CHECKED
    if (!kernel_app_activation_is_transformed(ctx)) {
        kernel_app_activation_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_app_activation_run_CHECKED
    if (!kernel_app_activation_is_swapped(ctx)) {
        kernel_app_activation_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_app_activation_commit(ctx)) {
        return kernel_app_activation_run_done;
    }
    return kernel_app_activation_run_engine_failure;
}

void kernel_app_activation(Domain *holder, TrapFrame *f) {
    uint64_t slot = f->rdi, kind = f->rsi;
    f->rax = APP_PRIVATE_INVALID;
    f->rsi = f->rdx = f->r10 = f->r8 = 0;
    if (kind > 1)
        return;
    const Cap *held = cap_get(holder, slot);
    if (!held || held->kind != CAP_APP_ACTIVATION)
        return;
    AppActivationGrant g = held->activation;
    KernelAppPrivate *old = kernel_app_get(g.predecessor), *next = kernel_app_get(g.successor);
    if (!old || !next || !old->ready || !next->ready) {
        f->rax = APP_PRIVATE_DENIED;
        return;
    }
    uint64_t error = app_activation_authorize(g, holder->id, old->target, old->task, next->target,
                                              next->task, old->identity, next->identity);
    if (error || next->predecessor != old->target) {
        f->rax = APP_PRIVATE_DENIED;
        return;
    }
    Domain *target = domain_get(next->target);
    if (next->activated) {
        f->rax = 0;
        f->rsi = next->target;
        f->rdx = next->task;
        f->r10 = next->surface;
        f->r8 = next->retired || (target && target->task_state == TASK_EXITED) ? 3 : 2;
        return;
    }
    bool ready = old->retired && old->acked == APP_PRIVATE_REQUIRED &&
                 kernel_app_dead(old) == APP_PRIVATE_OK;
    if (!kind) {
        f->rax = 0;
        f->rsi = next->target;
        f->rdx = next->task;
        f->r10 = next->surface;
        f->r8 = ready ? 1 : 0;
        return;
    }
    if (!ready) {
        f->rax = APP_PRIVATE_RETRY;
        return;
    }
    if (!kernel_epoch_current(kernel.epoch) || kernel.run_count >= KERNEL_RUN_QUEUE_CAPACITY) {
        f->rax = APP_PRIVATE_RETRY;
        return;
    }
    KernelAppActivationContext context = {.old = old, .next = next};
    if (kernel_app_activation_run(&context) != kernel_app_activation_run_done) {
        f->rax = APP_PRIVATE_DENIED;
        return;
    }
    f->rax = 0;
    f->rsi = next->target;
    f->rdx = next->task;
    f->r10 = next->surface;
    f->r8 = 2;
    klog(
        "[app.activation.committed] logical=%u incarnation=%u predecessor=%u successor=%u task=%u surface=%llu pages=%llu key=%u epoch=%llu runtime_alloc=0 runtime_mint=0 bsp=1\n",
        g.logical_app, g.incarnation, g.predecessor, g.successor, g.successor_task,
        (unsigned long long)next->surface, (unsigned long long)next->image.count,
        (unsigned)target->key, (unsigned long long)kernel.epoch);
}


