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

/* One serialized BSP syscall lane owns config-address/data access. IF is
 * masked and APs remain parked. SMP requires a shared config transaction lock. */
static uint32_t kernel_pci_address(PciCommandGrant g, uint8_t offset) {
    return UINT32_C(0x80000000) | ((uint32_t)g.bus << 16) | ((uint32_t)g.device << 11) |
           ((uint32_t)g.function << 8) | (offset & 0xfc);
}

uint32_t pci_policy_read(PciCommandGrant g, uint8_t offset) {
    cpu_outl(0xcf8, kernel_pci_address(g, offset));
    return cpu_inl(0xcfc);
}

void pci_policy_write_command(PciCommandGrant g, uint16_t value) {
    cpu_outl(0xcf8, kernel_pci_address(g, 4));
    cpu_outw(0xcfc, value);
}

void kernel_pci_boot(void) {
    if (kernel.genesis_sealed || kernel.scheduler_running || kernel.network_config_present)
        kernel_halt("network descriptor outside BOOT");
    Domain *owner = domain_get(CURRENT_PD_sexnet);
    if (!owner)
        return;
    NetConfig config = net_config_controlled();
    if (!net_config_valid(config))
        kernel_halt("invalid BOOT network descriptor");
    kernel.network_config = config;
    kernel.network_config_owner = owner->id;
    kernel.network_config_present = true;
    klog(
        "[boot.network.config] owner=%u version=1 profile=1 origin=boot_descriptor immutable=1 runtime_mint=0\n",
        owner->id);
}

static PciDeviceCapability kernel_pci_capability(const Cap *c) {
    if (!c || c->kind != CAP_PCI)
        return (PciDeviceCapability){0};
    return (PciDeviceCapability){.present = true,
                                 .bus = c->bus,
                                 .device = c->dev,
                                 .function = c->func,
                                 .vendor_id = c->vendor_id,
                                 .device_id = c->device_id,
                                 .command = c->command,
                                 .bar0 = c->region};
}

bool kernel_pci_boot_valid(Domain *d, uint64_t slot) {
    if (!d || domain_get(d->id) != d || d->id != CURRENT_PD_sexnet || slot != 19)
        return false;
    PciUpdate update = pci_policy_held(kernel_pci_capability(cap_get(d, slot)), d->id, slot, 0, 0);
    if (!update.present || !pci_policy_model(update.grant.vendor_id, update.grant.device_id))
        return false;
    const Cap *dma = cap_get(d, 32);
    return dma && dma->kind == CAP_REGION && pci_policy_dma(dma->region);
}

typedef struct KernelPciCommandContext {
    Domain *subject;
    PciUpdate update;
    PciApplyTrace trace;
    uint64_t slot, set, clear, epoch;
    bool snapshotted, transformed, swapped;
} KernelPciCommandContext;

static bool kernel_pci_command_current(const KernelPciCommandContext *c) {
    return c->snapshotted && c->subject && domain_get(c->subject->id) == c->subject &&
           kernel_current_task(c->subject) == c->subject->task_id && c->update.present && c->slot &&
           c->slot <= 1024 && kernel_epoch_current(c->epoch);
}

static bool kernel_pci_command_is_snapshotted(KernelPciCommandContext *c) {
    return kernel_pci_command_current(c);
}

static bool kernel_pci_command_is_transformed(KernelPciCommandContext *c) {
    return c->transformed && kernel_pci_command_current(c);
}

static bool kernel_pci_command_is_swapped(KernelPciCommandContext *c) {
    return c->swapped && c->transformed && kernel_pci_command_current(c);
}

static bool kernel_pci_command_snapshot(KernelPciCommandContext *c) {
    if (!c->subject || !c->update.present || !kernel_epoch_current(kernel.epoch))
        return false;
    c->epoch = kernel.epoch;
    c->snapshotted = true;
    return kernel_pci_command_current(c);
}

static bool kernel_pci_command_transform(KernelPciCommandContext *c) {
    if (!kernel_pci_command_current(c))
        return false;
    c->trace = pci_policy_apply_traced(c->update);
    c->transformed = true;
    return true;
}

static bool kernel_pci_command_swap(KernelPciCommandContext *c) {
    if (!kernel_pci_command_is_transformed(c))
        return false;
    c->swapped = true;
    return true;
}

static bool kernel_pci_command_commit(KernelPciCommandContext *c) {
    if (!kernel_pci_command_is_swapped(c))
        return false;
    kernel_epoch_record_pci(c->epoch, c->subject->id, c->slot, c->set, c->clear, c->update.grant,
                            c->trace);
    c->snapshotted = c->transformed = c->swapped = false;
    return true;
}

#define kernel_pci_command_run_CHECKED 1
#define kernel_pci_command_run_CHECK_FAILURE(ctx, kind, id, expected)                              \
    kernel_halt("PCI command epoch invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_pci_command_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_pci_command_run_DEFINED

#if defined(kernel_pci_command_run) || \
    defined(KernelPciCommandContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_pci_command_run_outcome) || \
    defined(kernel_pci_command_run_declined) || \
    defined(kernel_pci_command_run_answered) || \
    defined(kernel_pci_command_run_done) || \
    defined(kernel_pci_command_run_raised) || \
    defined(kernel_pci_command_run_suspended) || \
    defined(kernel_pci_command_run_resource_failure) || \
    defined(kernel_pci_command_run_engine_failure) || \
    defined(kernel_pci_command_snapshot) || \
    defined(kernel_pci_command_transform) || \
    defined(kernel_pci_command_swap) || \
    defined(kernel_pci_command_commit) || \
    defined(kernel_pci_command_is_snapshotted) || \
    defined(kernel_pci_command_is_transformed) || \
    defined(kernel_pci_command_is_swapped) || \
    defined(kernel_pci_command_run_decline_reason) || \
    defined(kernel_pci_command_run_decline_count) || \
    defined(kernel_pci_command_run_counters) || \
    defined(kernel_pci_command_run_count_decline) || \
    defined(kernel_pci_command_run_counts) || \
    defined(kernel_pci_command_run_reason) || \
    defined(kernel_pci_command_run_COUNTED) || \
    defined(kernel_pci_command_run_declined_after_kernel_pci_command_snapshot_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_pci_command_run_CHECKED
#define kernel_pci_command_run_CHECKED 0
#endif
#ifdef kernel_pci_command_run_COUNT_DECLINE
#define kernel_pci_command_run_COUNTED 1
#else
#define kernel_pci_command_run_COUNTED 0
#define kernel_pci_command_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_pci_command_run_CHECKED
#ifndef kernel_pci_command_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_pci_command_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_pci_command_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_pci_command_run_decline_reason {
    kernel_pci_command_run_declined_after_kernel_pci_command_snapshot_false,
    kernel_pci_command_run_decline_count
} kernel_pci_command_run_decline_reason;

typedef struct kernel_pci_command_run_counters {
    uint64_t declines[kernel_pci_command_run_decline_count ? kernel_pci_command_run_decline_count : 1];
} kernel_pci_command_run_counters;

#if kernel_pci_command_run_COUNTED
static inline void
kernel_pci_command_run_count_decline(kernel_pci_command_run_counters *kernel_pci_command_run_counts, kernel_pci_command_run_decline_reason kernel_pci_command_run_reason)
{
    if (kernel_pci_command_run_counts && (unsigned)kernel_pci_command_run_reason < (unsigned)kernel_pci_command_run_decline_count &&
        kernel_pci_command_run_counts->declines[kernel_pci_command_run_reason] != UINT64_MAX)
        ++kernel_pci_command_run_counts->declines[kernel_pci_command_run_reason];
}
#endif

typedef enum kernel_pci_command_run_outcome {
    kernel_pci_command_run_declined,
    kernel_pci_command_run_answered,
    kernel_pci_command_run_done,
    kernel_pci_command_run_raised,
    kernel_pci_command_run_suspended,
    kernel_pci_command_run_resource_failure,
    kernel_pci_command_run_engine_failure
} kernel_pci_command_run_outcome;

static inline kernel_pci_command_run_outcome
kernel_pci_command_run(KernelPciCommandContext *ctx)
{
    (void)ctx;
    if (!kernel_pci_command_snapshot(ctx)) {
        kernel_pci_command_run_COUNT_DECLINE(ctx, kernel_pci_command_run_declined_after_kernel_pci_command_snapshot_false);
        return kernel_pci_command_run_declined;
    }
#if kernel_pci_command_run_CHECKED
    if (!kernel_pci_command_is_snapshotted(ctx)) {
        kernel_pci_command_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
    if (!kernel_pci_command_transform(ctx)) {
        return kernel_pci_command_run_engine_failure;
    }
#if kernel_pci_command_run_CHECKED
    if (!kernel_pci_command_is_snapshotted(ctx)) {
        kernel_pci_command_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_pci_command_run_CHECKED
    if (!kernel_pci_command_is_transformed(ctx)) {
        kernel_pci_command_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
    if (!kernel_pci_command_swap(ctx)) {
        return kernel_pci_command_run_engine_failure;
    }
#if kernel_pci_command_run_CHECKED
    if (!kernel_pci_command_is_snapshotted(ctx)) {
        kernel_pci_command_run_CHECK_FAILURE(ctx, "fact", "snapshotted", 1);
        for (;;) {}
    }
#endif
#if kernel_pci_command_run_CHECKED
    if (!kernel_pci_command_is_transformed(ctx)) {
        kernel_pci_command_run_CHECK_FAILURE(ctx, "fact", "transformed", 1);
        for (;;) {}
    }
#endif
#if kernel_pci_command_run_CHECKED
    if (!kernel_pci_command_is_swapped(ctx)) {
        kernel_pci_command_run_CHECK_FAILURE(ctx, "fact", "swapped", 1);
        for (;;) {}
    }
#endif
    if (kernel_pci_command_commit(ctx)) {
        return kernel_pci_command_run_done;
    }
    return kernel_pci_command_run_engine_failure;
}

void kernel_pci_syscall(Domain *d, TrapFrame *f, uint64_t number) {
    uint64_t slot = f->rdi, a = f->rsi, b = f->rdx, c = f->r10, e = f->r8;
    f->rax = PCI_DENIED;
    f->rsi = f->rdx = f->r10 = f->r8 = 0;
    if (!kernel_current_task(d) || (number != 68 && number != 69 && number != 71))
        return;
    if (number == 68 ? (c | e) != 0 : (a | b | c | e) != 0)
        return;
    PciUpdate update = pci_policy_held(kernel_pci_capability(cap_get(d, slot)), d->id, slot,
                                       number == 68 ? a : 0, number == 68 ? b : 0);
    if (!update.present)
        return;
    PciReply r = {.status = PCI_DENIED};
    if (number == 69) {
        r = (PciReply){0, update.grant.vendor_id, update.grant.device_id, 0, 0};
        klog("[pci.device.identity] pd=%u slot=%llu vendor=%u product=%u runtime_mint=0\n", d->id,
             (unsigned long long)slot, update.grant.vendor_id, update.grant.device_id);
    } else if (number == 71) {
        if (!kernel.network_config_present || kernel.network_config_owner != d->id ||
            !net_config_valid(kernel.network_config))
            return;
        NetConfigWords words = net_config_to_words(kernel.network_config);
        r = (PciReply){0, words.word0, words.word1, words.word2, words.word3};
        klog("[pci.network.config] pd=%u slot=%llu origin=boot_descriptor runtime_mint=0\n", d->id,
             (unsigned long long)slot);
    } else {
        if (pci_policy_needs_dma(update)) {
            const Cap *dma = cap_get(d, 32);
            if (!dma || dma->kind != CAP_REGION || !pci_policy_dma(dma->region))
                return;
        }
        if (!kernel_epoch_current(kernel.epoch)) {
            r.status = PCI_EXHAUSTED;
            goto pci_done;
        }
        KernelPciCommandContext context = {
            .subject = d, .update = update, .slot = slot, .set = a, .clear = b};
        if (kernel_pci_command_run(&context) != kernel_pci_command_run_done)
            kernel_halt("PCI command protocol invariant");
        r = context.trace.reply;
        klog(
            "[pci.command.epoch] pd=%u slot=%llu status=%llu attempted=%u before=%u expected=%u after=%u epoch=%llu\n",
            d->id, (unsigned long long)slot, (unsigned long long)r.status,
            (unsigned)context.trace.attempted, (unsigned)context.trace.before,
            (unsigned)context.trace.expected, (unsigned)context.trace.after,
            (unsigned long long)kernel.epoch);
        if (!r.status)
            klog(
                "[pci.command.apply] pd=%u slot=%llu before=%llu after=%llu set=%llu clear=%llu changed=%llu\n",
                d->id, (unsigned long long)slot, (unsigned long long)r.value0,
                (unsigned long long)r.value1, (unsigned long long)a, (unsigned long long)b,
                (unsigned long long)r.value2);
    }
pci_done:
    f->rax = r.status;
    f->rsi = r.value0;
    f->rdx = r.value1;
    f->r10 = r.value2;
    f->r8 = r.value3;
}


