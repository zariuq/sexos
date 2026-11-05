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

enum {
    KERNEL_NVME_SLOT = 16,
    KERNEL_NVME_VECTOR = 0x22
};

static bool kernel_device_reset_cap(Domain *d, uint64_t slot, const Cap **out, uint32_t *device,
                                    uint32_t *identity) {
    const Cap *cap = cap_get(d, slot);
    if (!d || domain_get(d->id) != d || kernel_current_task(d) != d->task_id ||
        slot != KERNEL_NVME_SLOT || !cap || cap->kind != CAP_PCI ||
        kernel.vector_owner[KERNEL_NVME_VECTOR] != d->id)
        return false;
    uint32_t class_revision = pci_read(cap->bus, cap->dev, cap->func, 8);
    if ((class_revision >> 24) != 1 || ((class_revision >> 16) & 0xff) != 8)
        return false;
    uint32_t observed = pci_read(cap->bus, cap->dev, cap->func, 0);
    uint32_t expected = (uint32_t)cap->vendor_id | ((uint32_t)cap->device_id << 16);
    MemoryRegionViewResult view = sexos_region_register_view(
        &cap->region, 0, 0x20, REGION_QUERY | REGION_READ | REGION_WRITE);
    if (observed != expected || view.error || !view.value.virtual_address)
        return false;
    *out = cap;
    *device = ((uint32_t)cap->bus << 16) | ((uint32_t)cap->dev << 8) | cap->func;
    *identity = observed;
    return true;
}

static bool kernel_device_reset_observe(const Cap *cap, uint32_t *cc, uint32_t *csts) {
    MemoryRegionViewResult view = sexos_region_register_view(
        &cap->region, 0, 0x20, REGION_QUERY | REGION_READ | REGION_WRITE);
    if (view.error || !view.value.virtual_address)
        return false;
    uintptr_t base = (uintptr_t)view.value.virtual_address;
    if (kernel.smap)
        cpu_stac();
    *cc = *(const volatile uint32_t *)(base + 0x14);
    *csts = *(const volatile uint32_t *)(base + 0x1c);
    if (kernel.smap)
        cpu_clac();
    return true;
}

void kernel_device_reset_syscall(Domain *d, TrapFrame *f) {
    uint64_t slot = f->rdi, action = f->rsi, expected = f->rdx, argument = f->r10, unused = f->r8;
    f->rax = DEVICE_RESET_INVALID;
    f->rsi = f->rdx = f->r10 = f->r8 = f->r9 = 0;
    if (action > DEVICE_RESET_ACK_INTERRUPT || unused ||
        (action != DEVICE_RESET_ACK_INTERRUPT && argument > 1))
        return;
    const Cap *cap = NULL;
    uint32_t device = 0, identity = 0, cc = 0, csts = 0;
    if (!kernel_device_reset_cap(d, slot, &cap, &device, &identity) ||
        !kernel_device_reset_observe(cap, &cc, &csts))
        return;
    DeviceResetState *state = &kernel.device_reset;
    if (!state->present) {
        *state = device_reset_init(d->id, slot, device, identity, KERNEL_NVME_VECTOR,
                                   kernel.vector_revision[KERNEL_NVME_VECTOR]);
        if (!device_reset_state_valid(state))
            return;
    }
    if (state->owner != d->id || state->slot != slot || state->device != device ||
        state->identity != identity || state->vector != KERNEL_NVME_VECTOR)
        return;
    uint64_t epoch = kernel.epoch;
    if (action != DEVICE_RESET_QUERY && !kernel_epoch_current(epoch)) {
        f->rax = DEVICE_RESET_EXHAUSTED;
        return;
    }
    uint64_t acknowledged = kernel.vector_ack_revision[KERNEL_NVME_VECTOR];
    if (action == DEVICE_RESET_ACK_INTERRUPT) {
        if (!argument || argument != kernel.vector_heard_revision[KERNEL_NVME_VECTOR])
            return;
        acknowledged = argument;
    }
    DeviceResetRequest request = {.action = (DeviceResetAction)action,
                                  .expected_generation = expected,
                                  .interrupt_revision = kernel.vector_revision[KERNEL_NVME_VECTOR],
                                  .acknowledged_revision = acknowledged,
                                  .cc = cc,
                                  .csts = csts,
                                  .queues_ready = action == DEVICE_RESET_RESUME && argument != 0};
    DeviceResetState next = *state;
    DeviceResetReceipt receipt = device_reset_apply(&next, request);
    if (receipt.status == DEVICE_RESET_OK && action == DEVICE_RESET_BEGIN &&
        !kernel_nvme_msix_set_mask(cap, true))
        receipt.status = DEVICE_RESET_HARDWARE;
    if (receipt.status == DEVICE_RESET_OK && action == DEVICE_RESET_RESUME &&
        !kernel_nvme_msix_set_mask(cap, false))
        receipt.status = DEVICE_RESET_HARDWARE;
    if (receipt.status == DEVICE_RESET_OK)
        *state = next;
    if (receipt.status == DEVICE_RESET_OK && action == DEVICE_RESET_ACK_INTERRUPT)
        kernel.vector_ack_revision[KERNEL_NVME_VECTOR] = receipt.acknowledged_revision;
    uint64_t event_revision = action == DEVICE_RESET_ACK_INTERRUPT ? receipt.acknowledged_revision
                                                                   : receipt.interrupt_revision;
    if (receipt.status == DEVICE_RESET_OK && action != DEVICE_RESET_QUERY)
        kernel_epoch_record_device_reset(epoch, d->id, slot, device, identity, KERNEL_NVME_VECTOR,
                                         (uint8_t)action, receipt.generation, event_revision, cc,
                                         csts);
    if (receipt.status == DEVICE_RESET_OK && action != DEVICE_RESET_QUERY)
        klog(
            "[device.reset.epoch] pd=%u action=%llu generation=%llu phase=%u vector=%u interrupt_revision=%llu cc=%#x csts=%#x epoch=%llu\n",
            d->id, (unsigned long long)action, (unsigned long long)receipt.generation,
            (unsigned)receipt.phase, KERNEL_NVME_VECTOR, (unsigned long long)event_revision,
            (unsigned)cc, (unsigned)csts, (unsigned long long)kernel.epoch);
    if (receipt.status == DEVICE_RESET_OK && action == DEVICE_RESET_QUERY)
        klog(
            "[device.reset.query] pd=%u generation=%llu phase=%u vector=%u interrupt_revision=%llu acknowledged_revision=%llu\n",
            d->id, (unsigned long long)receipt.generation, (unsigned)receipt.phase,
            KERNEL_NVME_VECTOR, (unsigned long long)receipt.interrupt_revision,
            (unsigned long long)receipt.acknowledged_revision);
    f->rax = receipt.status;
    f->rsi = receipt.generation;
    f->rdx = receipt.interrupt_revision;
    f->r9 = receipt.acknowledged_revision;
    f->r10 = receipt.phase;
    f->r8 = ((uint64_t)receipt.cc << 32) | receipt.csts;
}

bool kernel_device_reset_owner_dead(Domain *dead) {
    DeviceResetState *state = &kernel.device_reset;
    if (!state->present || !dead || state->owner != dead->id)
        return true;
    uint64_t epoch = kernel.epoch;
    if (!kernel_epoch_current(epoch))
        return false;
    const Cap *cap = cap_get(dead, state->slot);
    if (!cap || cap->kind != CAP_PCI)
        return false;
    DeviceResetRequest request = {.action = DEVICE_RESET_OWNER_DEAD,
                                  .expected_generation = state->generation,
                                  .interrupt_revision = kernel.vector_revision[state->vector],
                                  .acknowledged_revision =
                                      kernel.vector_ack_revision[state->vector]};
    DeviceResetState next = *state;
    DeviceResetReceipt receipt = device_reset_apply(&next, request);
    if (receipt.status != DEVICE_RESET_OK)
        return false;
    if (!kernel_nvme_msix_set_mask(cap, true))
        return false;
    *state = next;
    kernel_epoch_record_device_reset(epoch, dead->id, state->slot, state->device, state->identity,
                                     state->vector, DEVICE_RESET_OWNER_DEAD, receipt.generation,
                                     receipt.interrupt_revision, 0, 0);
    kernel.vector_owner[state->vector] = 0;
    klog(
        "[device.reset.owner.dead] pd=%u generation=%llu phase=%u vector=%u pending=%llu epoch=%llu\n",
        dead->id, (unsigned long long)receipt.generation, (unsigned)receipt.phase,
        (unsigned)state->vector,
        (unsigned long long)(kernel.vector_revision[state->vector] -
                             kernel.vector_ack_revision[state->vector]),
        (unsigned long long)kernel.epoch);
    return true;
}


