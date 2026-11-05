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
#if !defined(SEXOS_NVME_INTERRUPT_MODE)
#define SEXOS_NVME_INTERRUPT_MODE 0

#endif
/*Protection domains: their capability tables and rings, loading their images, the boot genesis, device leases, the framebuffer hand-off, and the scheduler. Translated from kernel/src/init.rs, pd/create.rs, loader/elf.rs, elf.rs, capability.rs, devmgr.rs, graphics/gpu_lease.rs and scheduler.rs.*/
#define CODE_REGION_BASE UINT64_C(0x40000000)
#define STACK_REGION_BASE UINT64_C(0x700000000000)
#define DOMAIN_STRIDE UINT64_C(0x01000000)
#define USER_STACK_SIZE UINT64_C(0x10000)
/*The reference's ProtectionDomain::new grants three capabilities by insertion: a mock VFS node, the domain's own ring, a mock signal interrupt. Only the ring at slot 2 can be used; the other two answer every request as an empty slot does, so they are not kept.*/
#define SELF_RING_SLOT 2u
/*The first slot insertion gives after those three, where devmgr.rs inserts the display controller's lease.*/
#define FIRST_INSERTED_SLOT 4u
#define SLOT_USB_HOST 8u
#define SLOT_NVME_HOST 16u
#define SLOT_NIC 19u
#define NVME_VECTOR 0x22u
#define DISPLAY_VECTOR 0x23u
#define MAX_PCI_FUNCTIONS 64u
#define PF_X 1u
#define PF_W 2u
#define PT_LOAD 1u
#define PT_DYNAMIC 2u

enum KernelDomainKey {
#define KERNEL_DOMAIN_KEY(id, name, module, log_name) KERNEL_DOMAIN_KEY_##name = id,
    SEXOS_KERNEL_DOMAIN_ROWS(KERNEL_DOMAIN_KEY)
#undef KERNEL_DOMAIN_KEY
};


typedef struct GenesisDomain {
    uint8_t key;
    const char *module;
    const char *log_name;
    uint32_t label;
} GenesisDomain;


static const GenesisDomain genesis_domains[] = {
#define KERNEL_DOMAIN_ROW(id, name, module, log_name)                                              \
    {(uint8_t)(id), module, log_name, KERNEL_DOMAIN_KEY_##name},
    SEXOS_KERNEL_DOMAIN_ROWS(KERNEL_DOMAIN_ROW)
#undef KERNEL_DOMAIN_ROW
};

Kernel kernel;
uint64_t kernel_syscall_stack_top = 0;
uint64_t kernel_syscall_user_rsp = 0;
uint8_t kernel_smap_enabled = 0;
/*One kernel stack per domain; a domain's saved user context is the trap frame at the top of its stack.*/

static uint8_t kernel_stacks[KERNEL_STACK_RECORDS][KERNEL_STACK_SIZE] __attribute__((aligned(16)));


#if SEXOS_KERNEL_CURRENT_FPU
static uint8_t kernel_fpu_states[KERNEL_STACK_RECORDS][512] __attribute__((aligned(16)));

void kernel_fpu_save_current(void) {
    Domain *current = domain_get(kernel.current);
    if (current)
        cpu_fxsave(kernel_fpu_states[current->storage_index]);
}

static void kernel_fpu_restore(Domain *domain) {
#if SEXOS_FPU_CONTEXT_CORRUPT_PROBE
    (void)domain;
#else
    cpu_fxrstor(kernel_fpu_states[domain->storage_index]);
#endif
}
#endif

/*Domain ids by protection key (the genesis table's id column), filled by spawning.*/
static uint32_t pd_of_key[KERNEL_MAX_DOMAINS];
static uint32_t next_pd_id = 1;
static uint32_t next_task_id = 1;

Domain *domain_get(uint32_t id) {
    if (!id || id >= KERNEL_MAX_DOMAINS)
        return NULL;
#if SEXOS_KERNEL_CURRENT_PDX
    Domain *d = kernel.domains.slots[id];
#else
    Domain *d = &kernel.domains[id];
#endif
    return d && d->live && d->id == id ? d : NULL;
}

/* Allocate metadata at BOOT only. Logical IDs are not dense storage indexes.
 * Failed construction retains its identity reservation and cannot alias a later PD. */
static Domain *domain_boot_record(uint32_t id) {
    if (kernel.genesis_sealed || !id || id >= KERNEL_MAX_DOMAINS)
        return NULL;
#if SEXOS_KERNEL_CURRENT_PDX
    if (kernel.domains.slots[id] || kernel.domains.count >= KERNEL_BOOT_DOMAINS)
        return NULL;
    uint32_t index = kernel.domains.count++;
    Domain *d = &kernel.domains.records[index];
    kernel.domains.slots[id] = d;
    d->storage_index = (uint16_t)index;
#else
    Domain *d = &kernel.domains[id];
    if (d->id)
        return NULL;
    d->storage_index = (uint16_t)id;
#endif
    d->id = id;
    return d;
}

/*BOOT attaches a fresh task identity once. Exhaustion leaves subsequent records unattached; identities never wrap or derive from a domain number.*/
static bool task_attach(Domain *d) {
  if (next_task_id == 0 || d->task_id != 0) return false;
  d->task_id = next_task_id;
  next_task_id = next_task_id == UINT32_MAX ? 0 : next_task_id + 1;
  return true;
}
/*Transport provenance comes from the scheduled task, independently of its domain number. The current kernel has one main task per domain; a copied, unscheduled, exited or unattached record supplies no identity.*/
uint32_t kernel_current_task(Domain *d) {
  if (d == NULL || domain_get(kernel.current) != d) return 0;
  if (d->id != kernel.current || (d->task_state != TASK_RUNNING || d->task_id == 0)) return 0;
  return d->task_id;
}
static Domain *domain_of_key(uint64_t key) {
  if (key >= KERNEL_MAX_DOMAINS) return NULL;
  return domain_get(pd_of_key[key]);
}

bool kernel_epoch_current(uint64_t epoch) {
    return kernel.genesis_sealed && kernel.epoch == epoch && epoch < UINT64_MAX &&
           kernel.epoch_event_count < KERNEL_EPOCH_EVENT_CAPACITY
#if SEXOS_KERNEL_CURRENT_SMP
           && kernel_smp_epoch_current(epoch)
#endif
        ;
}

static void kernel_epoch_record_event(uint64_t expected, KernelEpochEvent event) {
    if (!kernel_epoch_current(expected))
        kernel_halt("epoch ledger commit invariant");
    uint64_t next = expected + 1u;
    uint32_t slot = kernel.epoch_event_count;
#if SEXOS_KERNEL_CURRENT_SMP
    if (!kernel_smp_epoch_advance(expected, next))
        kernel_halt("SMP epoch quiescence invariant");
#endif
    event.epoch = next;
    kernel.epoch_events[slot] = event;
    kernel.epoch = next;
    kernel.epoch_event_count = slot + 1u;
    uint32_t count = slot + 1u;
    if (count <= 8u || (count & (count - 1u)) == 0)
        klog("[kernel.epoch.commit] epoch=%llu count=%u kind=%u subject=%u revision=%llu\n",
             (unsigned long long)next, (unsigned)count, (unsigned)event.kind,
             (unsigned)event.subject, (unsigned long long)event.revision);
}

static void kernel_epoch_record(uint64_t expected, uint8_t kind, uint32_t subject,
                                uint64_t revision, const RingEntry *message, uint8_t scancode) {
    KernelEpochEvent event = {
        .subject = subject, .kind = kind, .scancode = scancode, .revision = revision};
    if (message)
        event.message = *message;
    kernel_epoch_record_event(expected, event);
}

void kernel_epoch_record_page(uint64_t expected, uint8_t kind, uint32_t subject, uint64_t address,
                              uint64_t token, uint64_t frame, uint8_t rights) {
    KernelEpochEvent event = {.subject = subject,
                              .kind = kind,
                              .rights = rights,
                              .revision = token,
                              .address = address,
                              .token = token,
                              .frame = frame};
    kernel_epoch_record_event(expected, event);
}


void kernel_epoch_record_page_key(uint64_t expected, uint8_t kind, uint32_t subject,
                                  uint64_t address, uint64_t token, uint64_t frame, uint8_t rights,
                                  uint8_t key) {
    KernelEpochEvent event = {.subject = subject,
                              .kind = kind,
                              .rights = rights,
                              .key = key,
                              .revision = token,
                              .address = address,
                              .token = token,
                              .frame = frame};
    kernel_epoch_record_event(expected, event);
}


void kernel_epoch_record_cleanup(uint64_t expected, uint32_t subject, uint64_t address,
                                 uint64_t task, uint64_t bitmap, uint8_t key) {
    KernelEpochEvent event = {.subject = subject,
                              .kind = KERNEL_EPOCH_PAGE_CLEANUP,
                              .key = key,
                              .revision = task,
                              .address = address,
                              .token = task,
                              .bitmap = bitmap};
    kernel_epoch_record_event(expected, event);
}


void kernel_epoch_record_handover(uint64_t expected, uint32_t subject, uint64_t task, uint64_t pool,
                                  uint64_t pages, uint8_t source_key, uint8_t key) {
    KernelEpochEvent event = {.subject = subject,
                              .kind = KERNEL_EPOCH_APP_HANDOVER,
                              .key = key,
                              .source_key = source_key,
                              .revision = task,
                              .address = pool,
                              .token = task,
                              .pages = pages};
    kernel_epoch_record_event(expected, event);
}


void kernel_epoch_record_activation(uint64_t expected, uint32_t predecessor, uint32_t successor,
                                    uint64_t task, uint64_t surface, uint64_t pages, uint8_t key) {
    KernelEpochEvent event = {.subject = successor,
                              .peer = predecessor,
                              .kind = KERNEL_EPOCH_APP_ACTIVATION,
                              .key = key,
                              .revision = task,
                              .address = surface,
                              .token = task,
                              .pages = pages};
    kernel_epoch_record_event(expected, event);
}


void kernel_epoch_record_pci(uint64_t expected, uint32_t subject, uint64_t slot, uint64_t set,
                             uint64_t clear, PciCommandGrant grant, PciApplyTrace trace) {
    uint32_t device = ((uint32_t)grant.bus << 16) | ((uint32_t)grant.device << 8) | grant.function;
    uint32_t expected_identity = (uint32_t)grant.vendor_id | ((uint32_t)grant.device_id << 16);
    KernelEpochEvent event = {.subject = subject,
                              .kind = KERNEL_EPOCH_PCI_COMMAND,
                              .revision = slot,
                              .address = slot,
                              .token = set,
                              .frame = clear,
                              .device = device,
                              .device_identity = trace.identity,
                              .device_expected_identity = expected_identity,
                              .device_before = trace.before,
                              .device_expected = trace.expected,
                              .device_after = trace.after,
                              .device_attempted = trace.attempted,
                              .device_status = trace.reply.status};
    kernel_epoch_record_event(expected, event);
}


void kernel_epoch_record_writer(uint64_t expected, uint32_t subject, uint32_t peer, uint32_t region,
                                uint64_t revision, uint64_t job, uint64_t pages, uint8_t source_key,
                                uint8_t target_key, uint8_t action) {
    KernelEpochEvent event = {.subject = subject,
                              .peer = peer,
                              .kind = KERNEL_EPOCH_REGION_WRITER,
                              .rights = action,
                              .key = target_key,
                              .source_key = source_key,
                              .revision = revision,
                              .address = region,
                              .token = job,
                              .pages = pages};
    kernel_epoch_record_event(expected, event);
}


void kernel_epoch_record_device_reset(uint64_t expected, uint32_t subject, uint64_t slot,
                                      uint32_t device, uint32_t identity, uint8_t vector,
                                      uint8_t action, uint64_t generation,
                                      uint64_t interrupt_revision, uint32_t cc, uint32_t csts) {
    KernelEpochEvent event = {.subject = subject,
                              .kind = KERNEL_EPOCH_DEVICE_RESET,
                              .rights = action,
                              .revision = generation,
                              .address = slot,
                              .token = interrupt_revision,
                              .frame = ((uint64_t)cc << 32) | csts,
                              .device = device,
                              .device_identity = identity,
                              .device_expected_identity = identity,
                              .pages = vector};
    kernel_epoch_record_event(expected, event);
}

/*Enqueue is a real snapshot-transform-swap-commit boundary. The tail slot is inactive until commit advances count and revision. An aborted transform may change only that inactive slot; the published FIFO remains unchanged.*/

typedef struct KernelRingPushContext {
    Ring *ring;
    RingEntry entry;
    uint32_t subject, head, count, tail;
    uint64_t revision, epoch;
    bool snapshotted, transformed, swapped;
} KernelRingPushContext;

static bool kernel_ring_push_current(const KernelRingPushContext *c) {
    return c->snapshotted && c->ring && c->head < KERNEL_RING_CAPACITY &&
           c->count < KERNEL_RING_CAPACITY && c->revision < UINT64_MAX &&
           c->ring->head == c->head && c->ring->count == c->count &&
           c->ring->revision == c->revision && kernel_epoch_current(c->epoch) &&
           c->tail == (c->head + c->count) % KERNEL_RING_CAPACITY;
}

static bool kernel_ring_push_is_snapshotted(KernelRingPushContext *c) {
    return kernel_ring_push_current(c);
}

static bool kernel_ring_push_is_transformed(KernelRingPushContext *c) {
    return c->transformed && kernel_ring_push_current(c);
}

static bool kernel_ring_push_is_swapped(KernelRingPushContext *c) {
    return c->swapped && c->transformed && kernel_ring_push_current(c);
}

static bool kernel_ring_push_snapshot(KernelRingPushContext *c) {
    if (!c->ring || !c->subject || !kernel_epoch_current(kernel.epoch) ||
        c->ring->head >= KERNEL_RING_CAPACITY || c->ring->count >= KERNEL_RING_CAPACITY ||
        c->ring->revision == UINT64_MAX)
        return false;
    c->head = c->ring->head;
    c->count = c->ring->count;
    c->revision = c->ring->revision;
    c->epoch = kernel.epoch;
    c->tail = (c->head + c->count) % KERNEL_RING_CAPACITY;
    c->snapshotted = true;
    return kernel_ring_push_current(c);
}

static bool kernel_ring_push_transform(KernelRingPushContext *c) {
    if (!kernel_ring_push_current(c))
        return false;
    c->transformed = true;
    return true;
}

static bool kernel_ring_push_swap(KernelRingPushContext *c) {
    if (!c->transformed || !kernel_ring_push_current(c))
        return false;
    c->ring->entries[c->tail] = c->entry;
    c->swapped = true;
    return true;
}

static bool kernel_ring_push_commit(KernelRingPushContext *c) {
    if (!c->swapped || !kernel_ring_push_current(c))
        return false;
    c->ring->count = c->count + 1u;
    c->ring->revision = c->revision + 1u;
    kernel_epoch_record(c->epoch, KERNEL_EPOCH_RING_PUSH, c->subject, c->revision + 1u, &c->entry,
                        0);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_ring_push_run_CHECKED 1
#define kernel_ring_push_run_CHECK_FAILURE(ctx, kind, id, expected)                                \
    kernel_halt("ring push epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_ring_push_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_ring_push_run_DEFINED

#if defined(kernel_ring_push_run) || \
    defined(KernelRingPushContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_ring_push_run_outcome) || \
    defined(kernel_ring_push_run_declined) || \
    defined(kernel_ring_push_run_answered) || \
    defined(kernel_ring_push_run_done) || \
    defined(kernel_ring_push_run_raised) || \
    defined(kernel_ring_push_run_suspended) || \
    defined(kernel_ring_push_run_resource_failure) || \
    defined(kernel_ring_push_run_engine_failure) || \
    defined(kernel_ring_push_snapshot) || \
    defined(kernel_ring_push_transform) || \
    defined(kernel_ring_push_swap) || \
    defined(kernel_ring_push_commit) || \
    defined(kernel_ring_push_is_snapshotted) || \
    defined(kernel_ring_push_is_transformed) || \
    defined(kernel_ring_push_is_swapped) || \
    defined(kernel_ring_push_run_decline_reason) || \
    defined(kernel_ring_push_run_decline_count) || \
    defined(kernel_ring_push_run_counters) || \
    defined(kernel_ring_push_run_count_decline) || \
    defined(kernel_ring_push_run_counts) || \
    defined(kernel_ring_push_run_reason) || \
    defined(kernel_ring_push_run_COUNTED) || \
    defined(kernel_ring_push_run_declined_after_kernel_ring_push_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_ring_push_run_CHECKED
#define kernel_ring_push_run_CHECKED 0
#endif
#ifdef kernel_ring_push_run_COUNT_DECLINE
#define kernel_ring_push_run_COUNTED 1
#else
#define kernel_ring_push_run_COUNTED 0
#define kernel_ring_push_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_ring_push_run_CHECKED
#ifndef kernel_ring_push_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_ring_push_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_ring_push_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_ring_push_run_decline_reason {
    kernel_ring_push_run_declined_after_kernel_ring_push_snapshot_false,
    kernel_ring_push_run_decline_count
} kernel_ring_push_run_decline_reason;

typedef struct kernel_ring_push_run_counters {
    uint64_t declines[kernel_ring_push_run_decline_count ? kernel_ring_push_run_decline_count : 1];
} kernel_ring_push_run_counters;

#if kernel_ring_push_run_COUNTED
static inline void
kernel_ring_push_run_count_decline(kernel_ring_push_run_counters *kernel_ring_push_run_counts, kernel_ring_push_run_decline_reason kernel_ring_push_run_reason)
{
    if (kernel_ring_push_run_counts && (unsigned)kernel_ring_push_run_reason < (unsigned)kernel_ring_push_run_decline_count &&
        kernel_ring_push_run_counts->declines[kernel_ring_push_run_reason] != UINT64_MAX)
        ++kernel_ring_push_run_counts->declines[kernel_ring_push_run_reason];
}
#endif

typedef enum kernel_ring_push_run_outcome {
    kernel_ring_push_run_declined,
    kernel_ring_push_run_answered,
    kernel_ring_push_run_done,
    kernel_ring_push_run_raised,
    kernel_ring_push_run_suspended,
    kernel_ring_push_run_resource_failure,
    kernel_ring_push_run_engine_failure
} kernel_ring_push_run_outcome;

static inline kernel_ring_push_run_outcome
kernel_ring_push_run(KernelRingPushContext *ctx)
{
    (void)ctx;
    if (!kernel_ring_push_snapshot(ctx)) {
        kernel_ring_push_run_COUNT_DECLINE(ctx, kernel_ring_push_run_declined_after_kernel_ring_push_snapshot_false);
        return kernel_ring_push_run_declined;
    }
#if kernel_ring_push_run_CHECKED
    if (!kernel_ring_push_is_snapshotted(ctx)) {
        kernel_ring_push_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_ring_push_transform(ctx)) {
        return kernel_ring_push_run_engine_failure;
    }
#if kernel_ring_push_run_CHECKED
    if (!kernel_ring_push_is_snapshotted(ctx)) {
        kernel_ring_push_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_ring_push_run_CHECKED
    if (!kernel_ring_push_is_transformed(ctx)) {
        kernel_ring_push_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_ring_push_swap(ctx)) {
        return kernel_ring_push_run_engine_failure;
    }
#if kernel_ring_push_run_CHECKED
    if (!kernel_ring_push_is_snapshotted(ctx)) {
        kernel_ring_push_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_ring_push_run_CHECKED
    if (!kernel_ring_push_is_transformed(ctx)) {
        kernel_ring_push_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_ring_push_run_CHECKED
    if (!kernel_ring_push_is_swapped(ctx)) {
        kernel_ring_push_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_ring_push_commit(ctx)) {
        return kernel_ring_push_run_done;
    }
    return kernel_ring_push_run_engine_failure;
}
static bool ring_push(Ring *r, uint32_t subject, RingEntry entry) {
  KernelRingPushContext context = (KernelRingPushContext) {
    .ring = r,.subject = subject,.entry = entry
  };
  return kernel_ring_push_run(&context) == kernel_ring_push_run_done;
}
/*A ring cursor is current only until the head or count changes. Copy the entry before advancing, and discard the cursor certificate on advance.*/

typedef struct KernelRingContext {
    Ring *ring;
    RingEntry *out;
    uint32_t subject, head, count;
    uint64_t revision, epoch;
    bool copied, swapped;
} KernelRingContext;

static bool kernel_ring_is_current(KernelRingContext *c) {
    return c->ring && c->out && c->subject && c->ring->revision == c->revision &&
           c->revision < UINT64_MAX && kernel_epoch_current(c->epoch) && c->ring->head == c->head &&
           c->ring->count == c->count && c->head < KERNEL_RING_CAPACITY && c->count > 0 &&
           c->count <= KERNEL_RING_CAPACITY;
}

static bool kernel_ring_is_copied(KernelRingContext *c) {
    return c->copied && kernel_ring_is_current(c);
}

static bool kernel_ring_is_swapped(KernelRingContext *c) {
    return c->swapped && c->copied && kernel_ring_is_current(c);
}

static bool kernel_ring_inspect(KernelRingContext *c) {
    if (!c->ring || !c->out || !c->subject || !kernel_epoch_current(kernel.epoch))
        return false;
    c->revision = c->ring->revision;
    c->epoch = kernel.epoch;
    c->head = c->ring->head;
    c->count = c->ring->count;
    return kernel_ring_is_current(c);
}

static bool kernel_ring_copy(KernelRingContext *c) {
    if (!kernel_ring_is_current(c))
        return false;
    *c->out = c->ring->entries[c->head];
    c->copied = true;
    return true;
}

static bool kernel_ring_swap(KernelRingContext *c) {
    if (!kernel_ring_is_copied(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_ring_advance(KernelRingContext *c) {
    if (!kernel_ring_is_swapped(c))
        return false;
    c->ring->head = (c->head + 1u) % KERNEL_RING_CAPACITY;
    c->ring->count = c->count - 1u;
    c->ring->revision = c->revision + 1u;
    kernel_epoch_record(c->epoch, KERNEL_EPOCH_RING_POP, c->subject, c->revision + 1u, c->out, 0);
    c->copied = c->swapped = false;
    return true;
}

#define kernel_ring_run_CHECKED 1
#define kernel_ring_run_CHECK_FAILURE(ctx, kind, id, expected)                                     \
    kernel_halt("ring protocol invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_ring_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_ring_run_DEFINED

#if defined(kernel_ring_run) || \
    defined(KernelRingContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_ring_run_outcome) || \
    defined(kernel_ring_run_declined) || \
    defined(kernel_ring_run_answered) || \
    defined(kernel_ring_run_done) || \
    defined(kernel_ring_run_raised) || \
    defined(kernel_ring_run_suspended) || \
    defined(kernel_ring_run_resource_failure) || \
    defined(kernel_ring_run_engine_failure) || \
    defined(kernel_ring_inspect) || \
    defined(kernel_ring_copy) || \
    defined(kernel_ring_swap) || \
    defined(kernel_ring_advance) || \
    defined(kernel_ring_is_current) || \
    defined(kernel_ring_is_copied) || \
    defined(kernel_ring_is_swapped) || \
    defined(kernel_ring_run_decline_reason) || \
    defined(kernel_ring_run_decline_count) || \
    defined(kernel_ring_run_counters) || \
    defined(kernel_ring_run_count_decline) || \
    defined(kernel_ring_run_counts) || \
    defined(kernel_ring_run_reason) || \
    defined(kernel_ring_run_COUNTED) || \
    defined(kernel_ring_run_declined_after_kernel_ring_inspect_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_ring_run_CHECKED
#define kernel_ring_run_CHECKED 0
#endif
#ifdef kernel_ring_run_COUNT_DECLINE
#define kernel_ring_run_COUNTED 1
#else
#define kernel_ring_run_COUNTED 0
#define kernel_ring_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_ring_run_CHECKED
#ifndef kernel_ring_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_ring_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_ring_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_ring_run_decline_reason {
    kernel_ring_run_declined_after_kernel_ring_inspect_false,
    kernel_ring_run_decline_count
} kernel_ring_run_decline_reason;

typedef struct kernel_ring_run_counters {
    uint64_t declines[kernel_ring_run_decline_count ? kernel_ring_run_decline_count : 1];
} kernel_ring_run_counters;

#if kernel_ring_run_COUNTED
static inline void
kernel_ring_run_count_decline(kernel_ring_run_counters *kernel_ring_run_counts, kernel_ring_run_decline_reason kernel_ring_run_reason)
{
    if (kernel_ring_run_counts && (unsigned)kernel_ring_run_reason < (unsigned)kernel_ring_run_decline_count &&
        kernel_ring_run_counts->declines[kernel_ring_run_reason] != UINT64_MAX)
        ++kernel_ring_run_counts->declines[kernel_ring_run_reason];
}
#endif

typedef enum kernel_ring_run_outcome {
    kernel_ring_run_declined,
    kernel_ring_run_answered,
    kernel_ring_run_done,
    kernel_ring_run_raised,
    kernel_ring_run_suspended,
    kernel_ring_run_resource_failure,
    kernel_ring_run_engine_failure
} kernel_ring_run_outcome;

static inline kernel_ring_run_outcome
kernel_ring_run(KernelRingContext *ctx)
{
    (void)ctx;
    if (!kernel_ring_inspect(ctx)) {
        kernel_ring_run_COUNT_DECLINE(ctx, kernel_ring_run_declined_after_kernel_ring_inspect_false);
        return kernel_ring_run_declined;
    }
#if kernel_ring_run_CHECKED
    if (!kernel_ring_is_current(ctx)) {
        kernel_ring_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_ring_copy(ctx)) {
        return kernel_ring_run_engine_failure;
    }
#if kernel_ring_run_CHECKED
    if (!kernel_ring_is_current(ctx)) {
        kernel_ring_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_ring_run_CHECKED
    if (!kernel_ring_is_copied(ctx)) {
        kernel_ring_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_ring_swap(ctx)) {
        return kernel_ring_run_engine_failure;
    }
#if kernel_ring_run_CHECKED
    if (!kernel_ring_is_current(ctx)) {
        kernel_ring_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_ring_run_CHECKED
    if (!kernel_ring_is_copied(ctx)) {
        kernel_ring_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_ring_run_CHECKED
    if (!kernel_ring_is_swapped(ctx)) {
        kernel_ring_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_ring_advance(ctx)) {
        return kernel_ring_run_done;
    }
    return kernel_ring_run_engine_failure;
}
static bool ring_pop(Ring *r, uint32_t subject, RingEntry *out) {
  KernelRingContext context = (KernelRingContext) {
    .ring = r,.subject = subject,.out = out
  };
  return kernel_ring_run(&context) == kernel_ring_run_done;
}
bool domain_enqueue(Domain *d, RingEntry entry) {
  if (SEXOS_KERNEL_CURRENT_PDX && (d->task_state == TASK_EXITED || d->task_state == TASK_DORMANT)) return false;
  bool accepted = ring_push(&d->ring, d->id, entry);
  if (SEXOS_KERNEL_CURRENT_PDX && accepted) kernel_task_wake(d);
  return accepted;
}
bool domain_dequeue(Domain *d, RingEntry *out) {
  return ring_pop(&d->ring, d->id, out);
}
/*External keyboard bytes enter the deterministic input stream only when the input epoch commits. The inactive tail and private pop output are not published state.*/

typedef struct KernelInputPushContext {
    Kernel *state;
    uint8_t scancode;
    uint32_t head, count, tail;
    uint64_t revision, epoch;
    bool snapshotted, transformed, swapped;
} KernelInputPushContext;

static bool kernel_input_push_current(const KernelInputPushContext *c) {
    return c->snapshotted && c->state && c->head < KERNEL_INPUT_CAPACITY &&
           c->count < KERNEL_INPUT_CAPACITY && c->revision < UINT64_MAX &&
           c->state->input_head == c->head && c->state->input_count == c->count &&
           c->state->input_revision == c->revision && kernel_epoch_current(c->epoch) &&
           c->tail == (c->head + c->count) % KERNEL_INPUT_CAPACITY;
}

static bool kernel_input_push_is_snapshotted(KernelInputPushContext *c) {
    return kernel_input_push_current(c);
}

static bool kernel_input_push_is_transformed(KernelInputPushContext *c) {
    return c->transformed && kernel_input_push_current(c);
}

static bool kernel_input_push_is_swapped(KernelInputPushContext *c) {
    return c->swapped && c->transformed && kernel_input_push_current(c);
}

static bool kernel_input_push_snapshot(KernelInputPushContext *c) {
    if (!c->state || !kernel.input_pd || !kernel_epoch_current(kernel.epoch) ||
        c->state->input_head >= KERNEL_INPUT_CAPACITY ||
        c->state->input_count >= KERNEL_INPUT_CAPACITY || c->state->input_revision == UINT64_MAX)
        return false;
    c->head = c->state->input_head;
    c->count = c->state->input_count;
    c->revision = c->state->input_revision;
    c->epoch = kernel.epoch;
    c->tail = (c->head + c->count) % KERNEL_INPUT_CAPACITY;
    c->snapshotted = true;
    return kernel_input_push_current(c);
}

static bool kernel_input_push_transform(KernelInputPushContext *c) {
    if (!kernel_input_push_current(c))
        return false;
    c->transformed = true;
    return true;
}

static bool kernel_input_push_swap(KernelInputPushContext *c) {
    if (!kernel_input_push_is_transformed(c))
        return false;
    c->state->input[c->tail] = c->scancode;
    c->swapped = true;
    return true;
}

static bool kernel_input_push_commit(KernelInputPushContext *c) {
    if (!kernel_input_push_is_swapped(c))
        return false;
    c->state->input_count = c->count + 1u;
    c->state->input_revision = c->revision + 1u;
    kernel_epoch_record(c->epoch, KERNEL_EPOCH_INPUT_PUSH, kernel.input_pd, c->revision + 1u, NULL,
                        c->scancode);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

typedef struct KernelInputPopContext {
    Kernel *state;
    uint8_t *out;
    uint32_t head, count;
    uint64_t revision, epoch;
    bool snapshotted, transformed, swapped;
} KernelInputPopContext;

static bool kernel_input_pop_current(const KernelInputPopContext *c) {
    return c->snapshotted && c->state && c->out && c->head < KERNEL_INPUT_CAPACITY &&
           c->count > 0 && c->count <= KERNEL_INPUT_CAPACITY && c->revision < UINT64_MAX &&
           c->state->input_head == c->head && c->state->input_count == c->count &&
           c->state->input_revision == c->revision && kernel_epoch_current(c->epoch);
}

static bool kernel_input_pop_is_snapshotted(KernelInputPopContext *c) {
    return kernel_input_pop_current(c);
}

static bool kernel_input_pop_is_transformed(KernelInputPopContext *c) {
    return c->transformed && kernel_input_pop_current(c);
}

static bool kernel_input_pop_is_swapped(KernelInputPopContext *c) {
    return c->swapped && c->transformed && kernel_input_pop_current(c);
}

static bool kernel_input_pop_snapshot(KernelInputPopContext *c) {
    if (!c->state || !c->out || !kernel.input_pd || !kernel_epoch_current(kernel.epoch) ||
        c->state->input_head >= KERNEL_INPUT_CAPACITY || !c->state->input_count ||
        c->state->input_count > KERNEL_INPUT_CAPACITY || c->state->input_revision == UINT64_MAX)
        return false;
    c->head = c->state->input_head;
    c->count = c->state->input_count;
    c->revision = c->state->input_revision;
    c->epoch = kernel.epoch;
    c->snapshotted = true;
    return kernel_input_pop_current(c);
}

static bool kernel_input_pop_transform(KernelInputPopContext *c) {
    if (!kernel_input_pop_current(c))
        return false;
    *c->out = c->state->input[c->head];
    c->transformed = true;
    return true;
}

static bool kernel_input_pop_swap(KernelInputPopContext *c) {
    if (!kernel_input_pop_is_transformed(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_input_pop_commit(KernelInputPopContext *c) {
    if (!kernel_input_pop_is_swapped(c))
        return false;
    c->state->input_head = (c->head + 1u) % KERNEL_INPUT_CAPACITY;
    c->state->input_count = c->count - 1u;
    c->state->input_revision = c->revision + 1u;
    kernel_epoch_record(c->epoch, KERNEL_EPOCH_INPUT_POP, kernel.input_pd, c->revision + 1u, NULL,
                        *c->out);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_input_push_run_CHECKED 1
#define kernel_input_pop_run_CHECKED 1
#define kernel_input_push_run_CHECK_FAILURE(ctx, kind, id, expected)                               \
    kernel_halt("input push epoch invariant")
#define kernel_input_pop_run_CHECK_FAILURE(ctx, kind, id, expected)                                \
    kernel_halt("input pop epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_input_push_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_input_push_run_DEFINED

#if defined(kernel_input_push_run) || \
    defined(KernelInputPushContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_input_push_run_outcome) || \
    defined(kernel_input_push_run_declined) || \
    defined(kernel_input_push_run_answered) || \
    defined(kernel_input_push_run_done) || \
    defined(kernel_input_push_run_raised) || \
    defined(kernel_input_push_run_suspended) || \
    defined(kernel_input_push_run_resource_failure) || \
    defined(kernel_input_push_run_engine_failure) || \
    defined(kernel_input_push_snapshot) || \
    defined(kernel_input_push_transform) || \
    defined(kernel_input_push_swap) || \
    defined(kernel_input_push_commit) || \
    defined(kernel_input_push_is_snapshotted) || \
    defined(kernel_input_push_is_transformed) || \
    defined(kernel_input_push_is_swapped) || \
    defined(kernel_input_push_run_decline_reason) || \
    defined(kernel_input_push_run_decline_count) || \
    defined(kernel_input_push_run_counters) || \
    defined(kernel_input_push_run_count_decline) || \
    defined(kernel_input_push_run_counts) || \
    defined(kernel_input_push_run_reason) || \
    defined(kernel_input_push_run_COUNTED) || \
    defined(kernel_input_push_run_declined_after_kernel_input_push_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_input_push_run_CHECKED
#define kernel_input_push_run_CHECKED 0
#endif
#ifdef kernel_input_push_run_COUNT_DECLINE
#define kernel_input_push_run_COUNTED 1
#else
#define kernel_input_push_run_COUNTED 0
#define kernel_input_push_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_input_push_run_CHECKED
#ifndef kernel_input_push_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_input_push_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_input_push_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_input_push_run_decline_reason {
    kernel_input_push_run_declined_after_kernel_input_push_snapshot_false,
    kernel_input_push_run_decline_count
} kernel_input_push_run_decline_reason;

typedef struct kernel_input_push_run_counters {
    uint64_t declines[kernel_input_push_run_decline_count ? kernel_input_push_run_decline_count : 1];
} kernel_input_push_run_counters;

#if kernel_input_push_run_COUNTED
static inline void
kernel_input_push_run_count_decline(kernel_input_push_run_counters *kernel_input_push_run_counts, kernel_input_push_run_decline_reason kernel_input_push_run_reason)
{
    if (kernel_input_push_run_counts && (unsigned)kernel_input_push_run_reason < (unsigned)kernel_input_push_run_decline_count &&
        kernel_input_push_run_counts->declines[kernel_input_push_run_reason] != UINT64_MAX)
        ++kernel_input_push_run_counts->declines[kernel_input_push_run_reason];
}
#endif

typedef enum kernel_input_push_run_outcome {
    kernel_input_push_run_declined,
    kernel_input_push_run_answered,
    kernel_input_push_run_done,
    kernel_input_push_run_raised,
    kernel_input_push_run_suspended,
    kernel_input_push_run_resource_failure,
    kernel_input_push_run_engine_failure
} kernel_input_push_run_outcome;

static inline kernel_input_push_run_outcome
kernel_input_push_run(KernelInputPushContext *ctx)
{
    (void)ctx;
    if (!kernel_input_push_snapshot(ctx)) {
        kernel_input_push_run_COUNT_DECLINE(ctx, kernel_input_push_run_declined_after_kernel_input_push_snapshot_false);
        return kernel_input_push_run_declined;
    }
#if kernel_input_push_run_CHECKED
    if (!kernel_input_push_is_snapshotted(ctx)) {
        kernel_input_push_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_input_push_transform(ctx)) {
        return kernel_input_push_run_engine_failure;
    }
#if kernel_input_push_run_CHECKED
    if (!kernel_input_push_is_snapshotted(ctx)) {
        kernel_input_push_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_input_push_run_CHECKED
    if (!kernel_input_push_is_transformed(ctx)) {
        kernel_input_push_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_input_push_swap(ctx)) {
        return kernel_input_push_run_engine_failure;
    }
#if kernel_input_push_run_CHECKED
    if (!kernel_input_push_is_snapshotted(ctx)) {
        kernel_input_push_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_input_push_run_CHECKED
    if (!kernel_input_push_is_transformed(ctx)) {
        kernel_input_push_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_input_push_run_CHECKED
    if (!kernel_input_push_is_swapped(ctx)) {
        kernel_input_push_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_input_push_commit(ctx)) {
        return kernel_input_push_run_done;
    }
    return kernel_input_push_run_engine_failure;
}

#ifdef CESTA_PROTOCOL_kernel_input_pop_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_input_pop_run_DEFINED

#if defined(kernel_input_pop_run) || \
    defined(KernelInputPopContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_input_pop_run_outcome) || \
    defined(kernel_input_pop_run_declined) || \
    defined(kernel_input_pop_run_answered) || \
    defined(kernel_input_pop_run_done) || \
    defined(kernel_input_pop_run_raised) || \
    defined(kernel_input_pop_run_suspended) || \
    defined(kernel_input_pop_run_resource_failure) || \
    defined(kernel_input_pop_run_engine_failure) || \
    defined(kernel_input_pop_snapshot) || \
    defined(kernel_input_pop_transform) || \
    defined(kernel_input_pop_swap) || \
    defined(kernel_input_pop_commit) || \
    defined(kernel_input_pop_is_snapshotted) || \
    defined(kernel_input_pop_is_transformed) || \
    defined(kernel_input_pop_is_swapped) || \
    defined(kernel_input_pop_run_decline_reason) || \
    defined(kernel_input_pop_run_decline_count) || \
    defined(kernel_input_pop_run_counters) || \
    defined(kernel_input_pop_run_count_decline) || \
    defined(kernel_input_pop_run_counts) || \
    defined(kernel_input_pop_run_reason) || \
    defined(kernel_input_pop_run_COUNTED) || \
    defined(kernel_input_pop_run_declined_after_kernel_input_pop_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_input_pop_run_CHECKED
#define kernel_input_pop_run_CHECKED 0
#endif
#ifdef kernel_input_pop_run_COUNT_DECLINE
#define kernel_input_pop_run_COUNTED 1
#else
#define kernel_input_pop_run_COUNTED 0
#define kernel_input_pop_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_input_pop_run_CHECKED
#ifndef kernel_input_pop_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_input_pop_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_input_pop_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_input_pop_run_decline_reason {
    kernel_input_pop_run_declined_after_kernel_input_pop_snapshot_false,
    kernel_input_pop_run_decline_count
} kernel_input_pop_run_decline_reason;

typedef struct kernel_input_pop_run_counters {
    uint64_t declines[kernel_input_pop_run_decline_count ? kernel_input_pop_run_decline_count : 1];
} kernel_input_pop_run_counters;

#if kernel_input_pop_run_COUNTED
static inline void
kernel_input_pop_run_count_decline(kernel_input_pop_run_counters *kernel_input_pop_run_counts, kernel_input_pop_run_decline_reason kernel_input_pop_run_reason)
{
    if (kernel_input_pop_run_counts && (unsigned)kernel_input_pop_run_reason < (unsigned)kernel_input_pop_run_decline_count &&
        kernel_input_pop_run_counts->declines[kernel_input_pop_run_reason] != UINT64_MAX)
        ++kernel_input_pop_run_counts->declines[kernel_input_pop_run_reason];
}
#endif

typedef enum kernel_input_pop_run_outcome {
    kernel_input_pop_run_declined,
    kernel_input_pop_run_answered,
    kernel_input_pop_run_done,
    kernel_input_pop_run_raised,
    kernel_input_pop_run_suspended,
    kernel_input_pop_run_resource_failure,
    kernel_input_pop_run_engine_failure
} kernel_input_pop_run_outcome;

static inline kernel_input_pop_run_outcome
kernel_input_pop_run(KernelInputPopContext *ctx)
{
    (void)ctx;
    if (!kernel_input_pop_snapshot(ctx)) {
        kernel_input_pop_run_COUNT_DECLINE(ctx, kernel_input_pop_run_declined_after_kernel_input_pop_snapshot_false);
        return kernel_input_pop_run_declined;
    }
#if kernel_input_pop_run_CHECKED
    if (!kernel_input_pop_is_snapshotted(ctx)) {
        kernel_input_pop_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_input_pop_transform(ctx)) {
        return kernel_input_pop_run_engine_failure;
    }
#if kernel_input_pop_run_CHECKED
    if (!kernel_input_pop_is_snapshotted(ctx)) {
        kernel_input_pop_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_input_pop_run_CHECKED
    if (!kernel_input_pop_is_transformed(ctx)) {
        kernel_input_pop_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_input_pop_swap(ctx)) {
        return kernel_input_pop_run_engine_failure;
    }
#if kernel_input_pop_run_CHECKED
    if (!kernel_input_pop_is_snapshotted(ctx)) {
        kernel_input_pop_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_input_pop_run_CHECKED
    if (!kernel_input_pop_is_transformed(ctx)) {
        kernel_input_pop_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_input_pop_run_CHECKED
    if (!kernel_input_pop_is_swapped(ctx)) {
        kernel_input_pop_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_input_pop_commit(ctx)) {
        return kernel_input_pop_run_done;
    }
    return kernel_input_pop_run_engine_failure;
}

static uint32_t kernel_input_epoch_log_budget = 16;

static void kernel_input_epoch_log(const char *kind, uint8_t scancode) {
    if (!kernel_input_epoch_log_budget)
        return;
    --kernel_input_epoch_log_budget;
    klog("[kernel.input.epoch.commit] kind=%s revision=%llu scancode=%u count=%u\n", kind,
         (unsigned long long)kernel.input_revision, (unsigned)scancode,
         (unsigned)kernel.input_count);
}

bool input_push(uint8_t scancode) {
  KernelInputPushContext context = (KernelInputPushContext) {
    .state = &kernel,.scancode = scancode
  };
  bool accepted = kernel_input_push_run(&context) == kernel_input_push_run_done;
  if (accepted) kernel_input_epoch_log("push", scancode);
  return accepted;
}
bool input_pop(uint8_t *out) {
  KernelInputPopContext context = (KernelInputPopContext) {
    .state = &kernel,.out = out
  };
  bool accepted = kernel_input_pop_run(&context) == kernel_input_pop_run_done;
  if (accepted) kernel_input_epoch_log("pop", *out);
  return accepted;
}

const Cap *cap_get(Domain *d, uint64_t slot) {
    if (!slot || slot >= KERNEL_CAP_SLOTS)
        return NULL;
#if SEXOS_KERNEL_CURRENT_PDX
    const Cap *c = d->caps[slot];
#else
    const Cap *c = &d->caps[slot];
#endif
    return c && c->kind != CAP_NONE ? c : NULL;
}

bool cap_set(Domain *d, uint64_t slot, Cap c) {
    if (kernel.genesis_sealed || !slot || slot >= KERNEL_CAP_SLOTS || c.kind == CAP_NONE ||
        c.kind == CAP_MEM_LEND || cap_get(d, slot))
        return false;
#if SEXOS_KERNEL_CURRENT_PDX
    if (kernel.cap_storage.count >= KERNEL_BOOT_CAPS)
        return false;
    Cap *stored = &kernel.cap_storage.entries[kernel.cap_storage.count++];
    *stored = c;
    d->caps[slot] = stored;
#else
    d->caps[slot] = c;
#endif
    return true;
}

bool cap_boot_available(uint32_t count) {
#if SEXOS_KERNEL_CURRENT_PDX
    return kernel.cap_storage.count <= KERNEL_BOOT_CAPS &&
           count <= KERNEL_BOOT_CAPS - kernel.cap_storage.count;
#else
    (void)count;
    return true;
#endif
}

bool cap_install_lend(Domain *d, uint64_t slot, Cap c) {
#if SEXOS_KERNEL_CURRENT_PDX
    (void)d;
    (void)slot;
    (void)c;
    return false;
#else
    if (!slot || slot >= KERNEL_CAP_SLOTS || cap_get(d, slot) || c.kind != CAP_MEM_LEND)
        return false;
    d->caps[slot] = c;
    return true;
#endif
}

static void kernel_seal_genesis(void) {
  kernel.genesis_sealed = true;
}
static Cap domain_cap(uint32_t target) {
  Cap c = (Cap) {
    .kind = CAP_DOMAIN
  };
  c.target = target;
  return c;
}
/*Current masks open only the domain's own hardware key; key 0 stays closed. The frozen profile retains its producer/consumer policy for shared key 14. Authority still comes from held grants rather than this enforcement mask.*/
uint32_t mask_for_key(uint8_t key) {
  uint32_t mask = 0xFFFFFFFFu;
  if (SEXOS_KERNEL_CURRENT_PDX && (key == 0 || key >= 16)) return mask;
  if (key < 16) mask &= ~(3u << 2 * key);
  if (SEXOS_KERNEL_CURRENT_PDX) return mask;
  uint32_t shift = 2 * KEY_SHARED;
  mask &= ~(3u << shift);
  if (key == 2) return mask;
  if (key == 1) return mask | 2u << shift;
  return mask | 3u << shift;
}
static bool contains(const char *haystack, const char *needle) {
  for (size_t i = 0; haystack[i] != 0; ++i) {
    size_t k = 0;
    while (needle[k] != 0 && haystack[i + k] == needle[k]) k += 1;
    if (needle[k] == 0) return true;
  }
  return needle[0] == 0;
}
typedef struct ElfHeader {
  uint8_t ident[16];
  uint16_t type;
  uint16_t machine;
  uint32_t version;
  uint64_t entry;
  uint64_t phoff;
  uint64_t shoff;
  uint32_t flags;
  uint16_t ehsize;
  uint16_t phentsize;
  uint16_t phnum;
  uint16_t shentsize;
  uint16_t shnum;
  uint16_t shstrndx;
}
ElfHeader;
typedef struct ProgramHeader {
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t vaddr;
  uint64_t paddr;
  uint64_t filesz;
  uint64_t memsz;
  uint64_t align;
}
ProgramHeader;
typedef struct RelaEntry {
  uint64_t offset;
  uint64_t info;
  int64_t addend;
}
RelaEntry;
static bool program_header(const uint8_t *image, uint64_t size, const ElfHeader *h, uint64_t i, ProgramHeader *out) {
  if (i >= h->phnum || h->phentsize < sizeof(ProgramHeader)) return false;
  if (h->phoff > size) return false;
  if (i > (size - h->phoff) / h->phentsize) return false;
  uint64_t at = h->phoff + i * h->phentsize;
  if (sizeof(ProgramHeader) > size - at) return false;
  memcpy(out, image + at, sizeof(ProgramHeader));
  return true;
}
/*WRPKRU is 0F 01 EF. Found at any byte offset of executable code, it would let the domain open every protection key; the reference loads such code.*/
static bool has_key_write(const uint8_t *bytes, uint64_t length) {
  for (uint64_t i = 0; i + 2 < length; ++i) if (bytes[i] == 0x0F && (bytes[i + 1] == 0x01 && bytes[i + 2] == 0xEF)) return true;
  return false;
}
/*The frame mapped at a user address, for writing the image through the higher-half map.*/
static uint8_t *frame_byte(uint64_t va) {
  uint64_t pte = 0;
  if (!read_pte(va, &pte)) return NULL;
  return (uint8_t *) phys_to_virt(pte & PTE_ADDRESS) + (va & 0xFFF);
}
/*Load a static PIE at code_base, as elf.rs::load_elf_for_pd: every PT_LOAD at code_base plus its offset from the lowest one, then the R_X86_64_RELATIVE relocations of PT_DYNAMIC. Pages get their final permissions when mapped, and the image is written through the kernel's own map, so no user page is ever writable and executable at once. Refused: segments both writable and executable, executable bytes containing WRPKRU. Returns the entry point, or 0.*/
static uint64_t elf_load(Domain *d, const uint8_t *image, uint64_t size) {
  ElfHeader h;
  if (size < sizeof(ElfHeader)) return 0;
  memcpy(&h, image, sizeof(ElfHeader));
  if (h.ident[0] != 0x7F || (h.ident[1] != 0x45 || (h.ident[2] != 0x4C || h.ident[3] != 0x46))) {
    klog("ELF: Invalid magic number\n");
    return 0;
  }
  uint64_t min_vaddr = UINT64_MAX;
  for (uint64_t i = 0; i < h.phnum; ++i) {
    ProgramHeader ph;
    if (!program_header(image, size, &h, i, &ph)) return 0;
    if (ph.type != PT_LOAD) continue;
    if ((ph.flags & PF_W) != 0 && (ph.flags & PF_X) != 0) {
      klog("[kernel.elf.refused] reason=writable_executable_segment idx=%llu\n", (unsigned long long) i);
      return 0;
    }
    if (ph.offset > size || ph.filesz > size - ph.offset) return 0;
    if ((ph.flags & PF_X) != 0 && has_key_write(image + ph.offset, ph.filesz)) {
      klog("[kernel.elf.refused] reason=key_register_write idx=%llu\n", (unsigned long long) i);
      return 0;
    }
    if (ph.memsz > 0 && ph.vaddr < min_vaddr) min_vaddr = ph.vaddr;
  }
  if (min_vaddr == UINT64_MAX) return 0;
  uint64_t base = d->code_base;
  for (uint64_t i = 0; i < h.phnum; ++i) {
    ProgramHeader ph;
    if (!program_header(image, size, &h, i, &ph)) return 0;
    if (ph.type != PT_LOAD) continue;
    uint64_t va = base + (ph.vaddr - min_vaddr);
    klog("ELF: Loading segment: vaddr=%#llx, memsz=%#llx (Key: %u)\n", (unsigned long long) va, (unsigned long long) ph.memsz, d->key);
    uint64_t flags = PTE_USER | ((ph.flags & PF_W) != 0 ? PTE_WRITABLE : 0);
    if ((ph.flags & PF_X) == 0) flags |= PTE_NO_EXECUTE;
    uint64_t end = va + ph.memsz;
    for (uint64_t page = va & ~(uint64_t) 0xFFF; page < end; page += KERNEL_PAGE_SIZE) {
      uint64_t pte = 0;
      if (read_pte(page, &pte)) {
/*A page shared with the previous segment keeps its frame; both permissions must agree with W^X.*/
        bool exec = (pte & PTE_NO_EXECUTE) == 0 || (flags & PTE_NO_EXECUTE) == 0;
        bool write = (pte & PTE_WRITABLE) != 0 || (flags & PTE_WRITABLE) != 0;
        if (exec && write) {
          klog("[kernel.elf.refused] reason=shared_page_writable_executable\n");
          return 0;
        }
        continue;
      }
      uint64_t frame = frames_alloc(0);
      if (frame == NO_ADDRESS || !map_page(page, frame, flags, d->key)) return 0;
    }
    uint64_t done = 0;
    while (done < ph.filesz) {
      uint64_t at = va + done;
      uint64_t chunk = KERNEL_PAGE_SIZE - (at & 0xFFF);
      if (chunk > ph.filesz - done) chunk = ph.filesz - done;
      memcpy(frame_byte(at), image + (ph.offset + done), chunk);
      done += chunk;
    }
    if (SEXOS_KERNEL_CURRENT_PDX && !kernel_app_capture_range(d, va, ph.memsz)) return 0;
  }
/*Relocations: the RELA table is found through the PT_LOAD segment whose file bytes cover DT_RELA.*/
  for (uint64_t i = 0; i < h.phnum; ++i) {
    ProgramHeader ph;
    if (!program_header(image, size, &h, i, &ph)) return 0;
    if (ph.type != PT_DYNAMIC) continue;
    uint64_t rela = 0;
    uint64_t rela_size = 0;
    uint64_t rela_entry = 24;
    uint64_t rela_count = 0;
    for (uint64_t j = 0; j < ph.memsz / 16 && ph.offset + (j + 1) * 16 <= size; ++j) {
      uint64_t dyn[2];
      memcpy(dyn, image + (ph.offset + j * 16), 16);
      if (dyn[0] == 0) break;
      if (dyn[0] == 7) rela = dyn[1];
      if (dyn[0] == 8) rela_size = dyn[1];
      if (dyn[0] == 9) rela_entry = dyn[1];
      if (dyn[0] == 0x6ffffff9) rela_count = dyn[1];
    }
    if (rela_count == 0 || (rela_size == 0 || rela_entry < sizeof(RelaEntry))) break;
    uint64_t rela_file = 0;
    for (uint64_t k = 0; k < h.phnum; ++k) {
      ProgramHeader seg;
      if (!program_header(image, size, &h, k, &seg)) return 0;
      if (seg.type == PT_LOAD && (rela >= seg.vaddr && rela < seg.vaddr + seg.filesz)) {
        rela_file = seg.offset + (rela - seg.vaddr);
        break;
      }
    }
    if (rela_file == 0 || rela_file + rela_count * rela_entry > size) {
      klog("ELF: R_X86_64_RELATIVE table not in LOAD segment, skipping\n");
      break;
    }
    uint64_t applied = 0;
    for (uint64_t k = 0; k < rela_count; ++k) {
      RelaEntry r;
      memcpy(&r, image + (rela_file + k * rela_entry), sizeof(RelaEntry));
      if ((r.info & 0xFFFFFFFFu) != 8) continue;
      uint64_t target = base + (r.offset - min_vaddr);
      uint8_t *slot = frame_byte(target);
      if (slot == NULL || (target & 0xFFF) > KERNEL_PAGE_SIZE - 8) return 0;
      uint64_t value = base + (uint64_t) r.addend;
      memcpy(slot, &value, 8);
      applied += 1;
    }
    klog("ELF: applied %llu R_X86_64_RELATIVE relocations\n", (unsigned long long) applied);
    break;
  }
/*The stack: 64 KiB below the stack top, writable and not executable.*/
  for (uint64_t page = d->stack_top - USER_STACK_SIZE; page < d->stack_top; page += KERNEL_PAGE_SIZE) {
    uint64_t frame = frames_alloc(0);
    if (frame == NO_ADDRESS || !map_page(page, frame, PTE_USER | (PTE_WRITABLE | PTE_NO_EXECUTE), d->key)) return 0;
  }
  if (SEXOS_KERNEL_CURRENT_PDX && !kernel_app_capture_range(d, d->stack_top - USER_STACK_SIZE, USER_STACK_SIZE)) return 0;
  uint64_t entry = base + (h.entry - min_vaddr);
  uint64_t pte = 0;
  if (!read_pte(entry, &pte) || ((pte & PTE_USER) == 0 || (pte & PTE_NO_EXECUTE) != 0)) {
    klog("loader: entrypoint mapping not user-executable\n");
    return 0;
  }
  return entry;
}
/*Create a domain from a module: the next domain id, the image at the key's code region, the stack at the key's stack region, the domain's own ring at slot 2, and a first trap frame that enters the image in user mode.*/
uint32_t domain_spawn_at(uint32_t id, uint8_t layout, uint8_t key, const char *name, const LimineFile *module) {
  if (next_task_id == 0) return 0;
  if (layout == 0 || (key == 0 || key >= 16)) return 0;
  if (id >= KERNEL_MAX_DOMAINS) return 0;
  Domain *d = domain_boot_record(id);
  if (d == NULL) return 0;
  uint16_t storage = d->storage_index;
  memset(d, 0, sizeof(Domain));
  d->storage_index = storage;
#if SEXOS_KERNEL_CURRENT_FPU
cpu_fpu_state_init(kernel_fpu_states[storage]);

#endif
  d->id = id;
  d->key = key;
  d->layout = layout;
  d->mask = mask_for_key(key);
  d->name = name;
  d->code_base = CODE_REGION_BASE + (layout - 1) * DOMAIN_STRIDE;
  d->stack_top = STACK_REGION_BASE + (layout - 1) * DOMAIN_STRIDE + 0x100000;
  uint64_t entry = elf_load(d, (const uint8_t *) module->address, module->size);
  if (entry == 0) return 0;
  d->entry = entry;
  klog("pd: checkpoint=elf.after_load entry=%#llx\n", (unsigned long long) entry);
  if (!SEXOS_KERNEL_CURRENT_PDX) cap_set(d, SELF_RING_SLOT, domain_cap(id));
  d->kstack_top = (uint64_t)(uintptr_t) kernel_stacks[d->storage_index] + KERNEL_STACK_SIZE;
  d->frame = (TrapFrame *)(uintptr_t)(d->kstack_top - sizeof(TrapFrame));
  memset(d->frame, 0, sizeof(TrapFrame));
  d->frame->rip = entry;
  d->frame->cs = USER_CS;
  d->frame->rflags = USER_RFLAGS;
  d->frame->rsp = d->stack_top;
  d->frame->ss = USER_SS;
  if (!task_attach(d)) return 0;
  if (SEXOS_KERNEL_CURRENT_PDX && !kernel_app_finish_capture(d)) return 0;
  d->task_state = TASK_READY;
  d->live = true;
  return id;
}
static uint32_t domain_spawn(uint8_t key, const char *name, const LimineFile *module) {
  uint32_t id = next_pd_id;
  next_pd_id += 1;
  return domain_spawn_at(id, key, key, name, module);
}
/*The primary display controller, as gpu_lease.rs::select_primary_gpu chooses it: buses 0 to 7, function 0, class 3; Intel first, then AMD, then virtio, then the first other. The lease is packed as pdx_get_init_arg returns it.*/
static void select_display_lease(void) {
  int best_rank = 99;
  uint64_t lease = 0;
  for (uint32_t bus = 0; bus < 8; ++bus) for (uint32_t dev = 0; dev < 32; ++dev) {
    uint32_t id = pci_read((uint8_t) bus, (uint8_t) dev, 0, 0);
    if ((id & 0xFFFF) == 0xFFFF) continue;
    if (pci_read((uint8_t) bus, (uint8_t) dev, 0, 8) >> 24 != 3) continue;
    uint32_t vendor = id & 0xFFFF;
    int rank = vendor == 0x8086 ? 0 : vendor == 0x1002 ? 1 : vendor == 0x1af4 ? 2 : 3;
    if (rank < best_rank) {
      best_rank = rank;
      lease = (uint64_t) bus | ((uint64_t) dev << 8 | ((uint64_t) vendor << 24 | (uint64_t)(id >> 16) << 40));
    }
  }
  if (best_rank < 99) {
    kernel.display_lease = lease;
    kernel.display_lease_ready = true;
    klog("init: PRIMARY_GPU_LEASE populated\n");
  }
}
static Cap pci_cap(Domain *owner, uint64_t slot, const PciFunction *f) {
  Cap c = (Cap) {
    .kind = CAP_PCI
  };
  c.bus = f->bus;
  c.dev = f->dev;
  c.func = f->func;
  c.vendor_id = f->vendor_id;
  c.device_id = f->device_id;
  if (SEXOS_KERNEL_CURRENT_PDX && f->class_id == 3) return c;
  if (SEXOS_KERNEL_CURRENT_PDX) {
    if (!kernel_boot_pci_region(owner, slot, f, &c.region)) return (Cap) {
      .kind = CAP_NONE
    };
    if (slot == SLOT_NIC) c.command = (PciCommandGrant) {
      .present = true,.subject = owner->id,.bus = f->bus,.device = f->dev,.function = f->func,.vendor_id = f->vendor_id,.device_id = f->device_id,.allowed = PCI_NIC_BITS
    };
  }
  return c;
}

static bool kernel_nvme_msix_enable(Domain *owner, const PciFunction *device, Cap *lease) {
#if !SEXOS_NVME_INTERRUPT_MODE
    (void)owner;
    (void)device;
    (void)lease;
    return true;
#else
    if (!owner || !device || !lease || lease->kind != CAP_PCI ||
        kernel.vector_owner[NVME_VECTOR] != owner->id)
        return false;
    uint32_t command_status = pci_read(device->bus, device->dev, device->func, 4);
    if (!(command_status & (1u << 20)))
        return false;
    uint8_t offset = (uint8_t)(pci_read(device->bus, device->dev, device->func, 0x34) & 0xfcu);
    NvmeMsixPlan plan = {0};
    bool found = false;
    uint64_t visited = 0;
    for (uint32_t step = 0; step < 48u && offset; step++) {
        if (offset < 0x40u || (offset & 3u))
            return false;
        uint32_t bit = (uint32_t)(offset >> 2);
        uint64_t mask = UINT64_C(1) << (bit & 63u);
        if (visited & mask) {
            return false;
        }
        visited |= mask;
        uint32_t capability = pci_read(device->bus, device->dev, device->func, offset);
        if ((capability & 0xffu) == 0x11u) {
            plan = nvme_msix_plan(
                offset, capability,
                pci_read(device->bus, device->dev, device->func, (uint8_t)(offset + 4u)),
                pci_read(device->bus, device->dev, device->func, (uint8_t)(offset + 8u)),
                lease->region.grant.length, NVME_VECTOR, 0);
            found = plan.status == NVME_MSIX_OK;
            break;
        }
        uint8_t next = (uint8_t)((capability >> 8) & 0xffu);
        if (next & 3u) {
            return false;
        }
        offset = next;
    }
    if (!found)
        return false;
    MemoryRegionViewResult table_view = sexos_region_register_view(
        &lease->region, plan.table_offset, 16, REGION_QUERY | REGION_READ | REGION_WRITE);
    MemoryRegionViewResult mask_view = sexos_region_register_view(
        &lease->region, 0x10, 4, REGION_QUERY | REGION_READ | REGION_WRITE);
    if (table_view.error || mask_view.error || !table_view.value.virtual_address ||
        !mask_view.value.virtual_address)
        return false;
    pci_write(device->bus, device->dev, device->func, plan.capability_offset, plan.config_masked);
    volatile uint32_t *entry = (volatile uint32_t *)(uintptr_t)table_view.value.virtual_address;
    if (kernel.smap)
        cpu_stac();
    entry[3] = plan.vector_masked;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    entry[0] = plan.address_low;
    entry[1] = plan.address_high;
    entry[2] = plan.message_data;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    uint32_t masked_address = entry[0], masked_high = entry[1], masked_data = entry[2],
             masked_control = entry[3];
    entry[3] = plan.vector_enabled;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    *(volatile uint32_t *)(uintptr_t)mask_view.value.virtual_address = 1u;
    if (kernel.smap)
        cpu_clac();
    if (masked_address != plan.address_low || masked_high != plan.address_high ||
        masked_data != plan.message_data || masked_control != plan.vector_masked)
        return false;
    pci_write(device->bus, device->dev, device->func, plan.capability_offset, plan.config_enabled);
    uint32_t config = pci_read(device->bus, device->dev, device->func, plan.capability_offset);
    if (kernel.smap)
        cpu_stac();
    uint32_t address = entry[0], high = entry[1], data = entry[2], control = entry[3];
    if (kernel.smap)
        cpu_clac();
    if (!nvme_msix_observed(&plan, config, address, high, data, control))
        return false;
    klog(
        "[kernel.nvme.msix.ready] pd=%u vector=%u cap=%#x table=%#x entries=%u address=%#x data=%#x\n",
        owner->id, (unsigned)plan.vector, (unsigned)plan.capability_offset,
        (unsigned)plan.table_offset, (unsigned)plan.table_entries, (unsigned)plan.address_low,
        (unsigned)plan.message_data);
    return true;
#endif
}

static bool kernel_nvme_reset_bind(Domain *owner, const PciFunction *device) {
#if !SEXOS_NVME_INTERRUPT_MODE
    (void)owner;
    (void)device;
    return true;
#else
    if (!owner || !device || kernel.device_reset.present ||
        kernel.vector_revision[NVME_VECTOR] != 0 || kernel.vector_ack_revision[NVME_VECTOR] != 0 ||
        kernel.vector_heard_revision[NVME_VECTOR] != 0)
        return false;
    uint32_t identity = (uint32_t)device->vendor_id | ((uint32_t)device->device_id << 16);
    uint32_t bdf = ((uint32_t)device->bus << 16) | ((uint32_t)device->dev << 8) | device->func;
    kernel.device_reset =
        device_reset_init(owner->id, SLOT_NVME_HOST, bdf, identity, NVME_VECTOR, 0);
    if (!device_reset_state_valid(&kernel.device_reset))
        return false;
    klog("[kernel.nvme.reset.bound] pd=%u vector=%u generation=1 acknowledged=0\n", owner->id,
         (unsigned)NVME_VECTOR);
    return true;
#endif
}

bool kernel_nvme_msix_set_mask(const Cap *lease, bool masked) {
#if !SEXOS_NVME_INTERRUPT_MODE
    (void)lease;
    (void)masked;
    return true;
#else
    if (!lease || lease->kind != CAP_PCI)
        return false;
    uint8_t offset = (uint8_t)(pci_read(lease->bus, lease->dev, lease->func, 0x34) & 0xfcu);
    NvmeMsixPlan plan = {0};
    uint64_t visited = 0;
    for (uint32_t step = 0; step < 48u && offset; step++) {
        if (offset < 0x40u || (offset & 3u))
            return false;
        uint32_t bit = (uint32_t)(offset >> 2);
        uint64_t mark = UINT64_C(1) << (bit & 63u);
        if (visited & mark) {
            return false;
        }
        visited |= mark;
        uint32_t capability = pci_read(lease->bus, lease->dev, lease->func, offset);
        if ((capability & 0xffu) == 0x11u) {
            plan = nvme_msix_plan(
                offset, capability,
                pci_read(lease->bus, lease->dev, lease->func, (uint8_t)(offset + 4u)),
                pci_read(lease->bus, lease->dev, lease->func, (uint8_t)(offset + 8u)),
                lease->region.grant.length, NVME_VECTOR, 0);
            break;
        }
        uint8_t next = (uint8_t)((capability >> 8) & 0xffu);
        if (next & 3u) {
            return false;
        }
        offset = next;
    }
    if (plan.status != NVME_MSIX_OK)
        return false;
    MemoryRegionViewResult view = sexos_region_register_view(
        &lease->region, plan.table_offset, 16, REGION_QUERY | REGION_READ | REGION_WRITE);
    if (view.error || !view.value.virtual_address)
        return false;
    volatile uint32_t *entry = (volatile uint32_t *)(uintptr_t)view.value.virtual_address;
    if (kernel.smap) {
        cpu_stac();
    }
    entry[3] = masked ? plan.vector_masked : plan.vector_enabled;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    uint32_t address = entry[0], high = entry[1], data = entry[2], control = entry[3];
    if (kernel.smap)
        cpu_clac();
    uint32_t config = masked ? plan.config_masked : plan.config_enabled;
    pci_write(lease->bus, lease->dev, lease->func, plan.capability_offset, config);
    uint32_t observed = pci_read(lease->bus, lease->dev, lease->func, plan.capability_offset);
    if (masked)
        return observed == plan.config_masked && address == plan.address_low &&
               high == plan.address_high && data == plan.message_data &&
               control == plan.vector_masked;
    return nvme_msix_observed(&plan, observed, address, high, data, control);
#endif
}

/*Device leases, as devmgr.rs grants them: an NVMe controller to sexdrive at slot 16 with vector 0x22, a display controller to sexdisplay at its first inserted slot, an xHCI controller to sexusb (else sexdrive) at slot 8, a network controller to sexnet at slot 19 with memory decoding, I/O decoding and bus mastering enabled as hal/pci.rs enables them.*/
static void lease_devices(void) {
  PciFunction functions[MAX_PCI_FUNCTIONS];
  uint32_t count = pci_scan(functions, MAX_PCI_FUNCTIONS);
  klog("DevMgr: Enumerated %u PCI devices.\n", count);
  bool nvme_found = false;
  Domain *sexdrive = SEXOS_KERNEL_CURRENT_PDX ? domain_get(CURRENT_PD_sexdrive) : domain_of_key(KERNEL_DOMAIN_KEY_sexdrive);
  Domain *sexdisplay = SEXOS_KERNEL_CURRENT_PDX ? domain_get(CURRENT_PD_sexdisplay) : domain_of_key(KERNEL_DOMAIN_KEY_sexdisplay);
  Domain *sexusb = SEXOS_KERNEL_CURRENT_PDX ? domain_get(CURRENT_PD_sexusb) : domain_of_key(KERNEL_DOMAIN_KEY_sexusb);
  Domain *sexnet = SEXOS_KERNEL_CURRENT_PDX ? domain_get(CURRENT_PD_sexnet) : domain_of_key(KERNEL_DOMAIN_KEY_sexnet);
  for (uint32_t i = 0; i < count; ++i) {
    const PciFunction *f = &functions[i];
    if (f->class_id == 0x01 && f->subclass_id == 0x08) {
      klog("[kernel.pci.nvme.found] %02x:%02x.%u vendor=%04x device=%04x\n", f->bus, f->dev, f->func, f->vendor_id, f->device_id);
      if (sexdrive != NULL) {
        Cap lease = pci_cap(sexdrive, SLOT_NVME_HOST, f);
        if (lease.kind == CAP_NONE) kernel_halt("BOOT NVMe capability construction");
        if (!cap_set(sexdrive, SLOT_NVME_HOST, lease)) kernel_halt("BOOT NVMe capability insertion");
        kernel.vector_owner[NVME_VECTOR] = sexdrive->id;
        if (!kernel_nvme_reset_bind(sexdrive, f)) kernel_halt("BOOT NVMe reset binding");
        if (!kernel_nvme_msix_enable(sexdrive, f, &lease)) kernel_halt("BOOT NVMe MSI-X configuration");
        klog("[kernel.cap.nvme_bar.grant] pd=%u slot=%u\n", sexdrive->id, SLOT_NVME_HOST);
      }
      nvme_found = true;
    }
    if (f->class_id == 0x03 && sexdisplay != NULL) {
      cap_set(sexdisplay, FIRST_INSERTED_SLOT, pci_cap(sexdisplay, FIRST_INSERTED_SLOT, f));
      kernel.vector_owner[DISPLAY_VECTOR] = sexdisplay->id;
    } else if (f->class_id == 0x0C && (f->subclass_id == 0x03 && f->prog_if == 0x30)) {
      Domain *owner = sexusb != NULL ? sexusb : sexdrive;
      if (owner != NULL) {
        cap_set(owner, SLOT_USB_HOST, pci_cap(owner, SLOT_USB_HOST, f));
        klog("DevMgr: Leased XHCI to pd=%u slot=%u\n", owner->id, SLOT_USB_HOST);
      }
    } else if (f->class_id == 0x02 && f->subclass_id == 0x00) {
      uint32_t command = pci_read(f->bus, f->dev, f->func, 0x04);
      if (!SEXOS_KERNEL_CURRENT_PDX) pci_write(f->bus, f->dev, f->func, 0x04, command | 0x7);
      klog("[kernel.pci.nic.found] %02x:%02x.%u vendor=%04x device=%04x\n", f->bus, f->dev, f->func, f->vendor_id, f->device_id);
      if (sexnet != NULL) {
        Cap lease = pci_cap(sexnet, SLOT_NIC, f);
        if (SEXOS_KERNEL_CURRENT_PDX && lease.kind == CAP_NONE) {
          klog("[kernel.cap.nic.skip] reason=unsupported_invalid_or_occupied_bar vendor=%04x device=%04x\n", f->vendor_id, f->device_id);
          continue;
        }
        if (!cap_set(sexnet, SLOT_NIC, lease)) {
          if (SEXOS_KERNEL_CURRENT_PDX) kernel_halt("BOOT NIC capability insertion");
        }
        klog("[kernel.cap.nic.grant] pd=%u slot=%u\n", sexnet->id, SLOT_NIC);
      }
    }
  }
  if (!nvme_found) klog("[kernel.pci.nvme.absent]\n");
}
/*One genesis row: the bootgraph line the reference prints in its bracketed form, which both the boot oracle and the upstream checker read.*/
static void grant_ring(uint64_t holder_key, uint64_t slot, const char *label, uint64_t target_key, bool optional) {
  Domain *holder = domain_of_key(holder_key);
  Domain *target = domain_of_key(target_key);
  if (holder == NULL || target == NULL) {
    klog("[bootgraph.cap.grant from=kernel to=%u slot=%s target=missing ok=0 optional=1 reason=pd_missing]\n", holder != NULL ? holder->id : 0u, label);
    return;
  }
  cap_set(holder, slot, domain_cap(target->id));
  klog("[bootgraph.cap.grant from=kernel to=%u slot=%s target=%u ok=1%s]\n", holder->id, label, target->id, optional ? " optional=1" : "");
}
static void grant_input(uint64_t holder_key, uint64_t slot, const char *label) {
  Domain *holder = domain_of_key(holder_key);
  if (holder == NULL) return;
  cap_set(holder, slot, (Cap) {
    .kind = CAP_INPUT_RING
  });
  klog("[bootgraph.cap.grant from=kernel to=%u slot=%s target=input_ring ok=1]\n", holder->id, label);
}
/*The grant rows of the genesis table, in their order.*/

static void apply_genesis_grants(void) {
#define KERNEL_RING_GRANT(holder, slot, label, target, optional)                                   \
    grant_ring(KERNEL_DOMAIN_KEY_##holder, slot, label, KERNEL_DOMAIN_KEY_##target, optional);
    SEXOS_KERNEL_RING_GRANT_ROWS(KERNEL_RING_GRANT)
#ifdef SEXOS_NETWORK_PROFILE
    SEXOS_KERNEL_NETWORK_GRANT_ROWS(KERNEL_RING_GRANT)
#endif
#undef KERNEL_RING_GRANT
#define KERNEL_INPUT_GRANT(holder, slot, label)                                                    \
    grant_input(KERNEL_DOMAIN_KEY_##holder, slot, label);
    SEXOS_KERNEL_INPUT_GRANT_ROWS(KERNEL_INPUT_GRANT)
#undef KERNEL_INPUT_GRANT
}

void kernel_boot_framebuffer(void) {
/*The framebuffer: exactly its pages become reachable from sexdisplay, under sexdisplay's key, and its geometry is the first message on sexdisplay's ring.*/
  Domain *display = domain_get(kernel.display_pd);
  const LimineFramebufferResponse *fbs = limine_framebuffer_request.response;
  if (display != NULL && (fbs != NULL && fbs->framebuffer_count > 0)) {
    const LimineFramebuffer *fb = fbs->framebuffers[0];
    uint64_t address = (uint64_t)(uintptr_t) fb->address;
    kernel.fb_address = address;
    kernel.fb_width = fb->width;
    kernel.fb_height = fb->height;
    kernel.fb_pitch = fb->pitch;
/*The current kernel reserves Display's two private frame buffers at this geometry-known genesis; their layout also bounds the framebuffer extent.*/
    if (SEXOS_KERNEL_CURRENT_PDX) {
      DisplayFrameLayout layout = (DisplayFrameLayout) {
        .width = 0
      };
      if (!display_frame_layout(fb->width, fb->height, fb->pitch, &layout)) kernel_halt("BOOT_DISPLAY_FRAME_GEOMETRY");
      if (fb->bpp != 32) kernel_halt("BOOT_DISPLAY_FRAME_FORMAT");
      if (address > UINT64_MAX - (layout.frame_bytes + 4095)) kernel_halt("BOOT_DISPLAY_FRAMEBUFFER_EXTENT");
      kernel_boot_display_frames(display->id, &layout);
    }
    if (expose_to_user(address, fb->pitch * fb->height, display->key)) klog("init: FB remapped USER_ACCESSIBLE (%#llx, %llu bytes) key=%u\n", (unsigned long long) address, (unsigned long long)(fb->pitch * fb->height), display->key);
    else klog("init: FB remap failed\n");
    RingEntry message = (RingEntry) {
      .type_id = 0x11,.caller = 1
    };
    message.arg0 = address;
    message.arg1 = fb->width | fb->height << 32;
    message.arg2 = fb->pitch / 4;
    domain_enqueue(display, message);
    klog("init: FB handed to sexdisplay (%llux%llu @ %#llx)\n", (unsigned long long) fb->width, (unsigned long long) fb->height, (unsigned long long) address);
  }
}
void kernel_boot_display_lease(void) {
  select_display_lease();
}
void kernel_boot_devices(void) {
  lease_devices();
}
/*Boot: choose the display lease, spawn every domain of the genesis table in its order from the module whose path contains its name, grant the table's capabilities, lease the devices, hand the framebuffer to sexdisplay, and queue every domain. init.rs does the same with a list of names and 777 lines of grants, each printed twice.*/
void domains_boot(void) {
  if (SEXOS_KERNEL_CURRENT_PDX) {
    domains_boot_current();
    return;
  }
  select_display_lease();
  const LimineModuleResponse *modules = limine_module_request.response;
  if (modules == NULL) kernel_halt("FATAL: Limine returned no modules. Check limine.cfg and ISO layout.");
  klog("init: Found %llu Limine modules\n", (unsigned long long) modules->module_count);
  for (size_t r = 0; r < sizeof genesis_domains / sizeof genesis_domains[0]; ++r) {
    const GenesisDomain *g = &genesis_domains[r];
    for (uint64_t m = 0; m < modules->module_count; ++m) {
      const LimineFile *module = modules->modules[m];
      if (!contains(module->path, g->module)) continue;
      klog("[bootgraph.pd.spawn.begin] pd=%s\n", g->log_name);
      uint32_t id = domain_spawn(g->key, g->log_name, module);
      if (id == 0) klog("[bootgraph.pd.spawn.err] pd=%s reason=spawn_failed\n", g->log_name);
      else {
        pd_of_key[g->key] = id;
        if (g->label == KERNEL_DOMAIN_KEY_sexdisplay) kernel.display_pd = id;
        if (g->label == KERNEL_DOMAIN_KEY_silk_shell) kernel.shell_pd = id;
        if (g->label == KERNEL_DOMAIN_KEY_sexinput) kernel.input_pd = id;
        klog("[bootgraph.pd.spawn.ok] pd=%s id=%u pkey=%u\n", g->log_name, id, g->key);
        if (g->key == KERNEL_DOMAIN_KEY_sexdisplay && kernel.display_lease_ready) {
          domain_get(id)->init_arg = kernel.display_lease;
          domain_get(id)->init_arg_ready = true;
          kernel.display_lease_ready = false;
        }
      }
      break;
    }
  }
  klog("[bootgraph.phase25.begin]\n");
  apply_genesis_grants();
  lease_devices();
  klog("[bootgraph.phase25.complete]\n");
  kernel_boot_framebuffer();
  for (uint32_t id = 1; id < KERNEL_MAX_DOMAINS; ++id) {
    Domain *d = domain_get(id);
    if (d == NULL) continue;
    klog("[bootgraph.boot.handoff] target=%s id=%u entry=%#llx\n", d->name, id, (unsigned long long) d->entry);
    run_push(id);
  }
  kernel_seal_genesis();
  klog("init: Ready for Scheduler.\n");
}
/*The scheduler: one queue, first in first out; a preempted or yielding task goes to the back.*/
void run_push(uint32_t id) {
  Domain *d = domain_get(id);
  if (d == NULL || (d->task_state == TASK_EXITED || d->task_state == TASK_DORMANT)) return;
  if (SEXOS_KERNEL_CURRENT_PDX && (d->queued || (d->task_state != TASK_READY && d->task_state != TASK_RUNNING))) return;
  if (kernel.run_count >= KERNEL_RUN_QUEUE_CAPACITY) {
    if (SEXOS_KERNEL_CURRENT_PDX) kernel_halt("run admission capacity");
    else return;
  }
  if (SEXOS_KERNEL_CURRENT_PDX) d->queued = true;
  kernel.run_queue[(kernel.run_head + kernel.run_count) % KERNEL_RUN_QUEUE_CAPACITY] = id;
  kernel.run_count += 1;
  d->task_state = TASK_READY;
}
uint32_t run_pop(void) {
  while (kernel.run_count > 0) {
    uint32_t id = kernel.run_queue[kernel.run_head];
    kernel.run_head = (kernel.run_head + 1) % KERNEL_RUN_QUEUE_CAPACITY;
    kernel.run_count -= 1;
    if (SEXOS_KERNEL_CURRENT_PDX && domain_get(id) != NULL) domain_get(id)->queued = false;
    if (domain_get(id) != NULL && domain_get(id)->task_state == TASK_READY) return id;
  }
  return 0;
}
/*Make a domain current and return its frame: its kernel stack becomes the stack of the next entry from user mode, and its key register value is loaded on the way out.*/
TrapFrame *switch_to(uint32_t id) {
  Domain *d = domain_get(id);
  if (d == NULL || d->task_state == TASK_DORMANT) kernel_halt("schedule of absent or dormant domain");
  if (SEXOS_KERNEL_CURRENT_PDX && d->task_state != TASK_READY) kernel_halt("schedule without ready ownership");
  kernel.current = id;
  d->task_state = TASK_RUNNING;
  tss_set_rsp0(d->kstack_top);
  kernel_syscall_stack_top = d->kstack_top;
  if (!kernel.first_scheduled_logged) {
    kernel.first_scheduled_logged = true;
    if (SEXOS_KERNEL_CURRENT_PDX) klog("[bootgraph.boot.handoff] target=%s id=%u entry=%#llx\n", d->name, id, (unsigned long long) d->entry);
    klog("first scheduled pd_id=%u\n", id);
  }
  if (kernel.pku) cpu_wrpkru(d->mask);
#if SEXOS_KERNEL_CURRENT_FPU
kernel_fpu_restore(d);

#endif
  return d->frame;
}
/*Resume the interrupted context, restoring the current domain's key register when it is user mode.*/
TrapFrame *resume_frame(TrapFrame *frame) {
  if ((frame->cs & 3) == 3) {
    Domain *d = domain_get(kernel.current);
    if (d != NULL) {
      if (kernel.pku) cpu_wrpkru(d->mask);
#if SEXOS_KERNEL_CURRENT_FPU
kernel_fpu_restore(d);

#endif
    }
  }
  return frame;
}
/*Timer preemption from user mode: the next queued domain runs, the current one goes to the back. With nothing queued the current one continues.*/
TrapFrame *schedule_tick(TrapFrame *frame) {
  if (kernel.run_count == 0) return resume_frame(frame);
  uint32_t next = run_pop();
  if (next == 0) return resume_frame(frame);
  run_push(kernel.current);
  return switch_to(next);
}
/*SYSCALL_YIELD: the current domain goes to the back and the first queued domain runs, possibly itself.*/
TrapFrame *schedule_yield(void) {
  run_push(kernel.current);
  return switch_to(run_pop());
}
/*With no domain to run, wait for an interrupt with interrupts enabled; the timer handler switches to a domain once one is queued.*/
_Noreturn void kernel_idle_loop(void) {
  kernel.current = 0;
  if (SEXOS_KERNEL_CURRENT_PDX) kernel_idle_observe();
  while (1) cpu_sti_hlt();
}
