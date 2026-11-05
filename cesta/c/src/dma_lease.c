#include "sexos_dma_lease.h"
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

bool dma_lease_region_valid(DmaLeaseRegion region) {
    return region.region_id && region.length && !(region.length % DMA_LEASE_PAGE_BYTES) &&
           region.virtual_address && !(region.virtual_address % DMA_LEASE_PAGE_BYTES) &&
           region.physical_address && !(region.physical_address % DMA_LEASE_PAGE_BYTES) &&
           region.virtual_address < DMA_LEASE_USER_END &&
           region.length <= DMA_LEASE_USER_END - region.virtual_address &&
           region.physical_address < DMA_LEASE_PHYSICAL_END &&
           region.length <= DMA_LEASE_PHYSICAL_END - region.physical_address;
}

DmaLeaseStatus dma_lease_pool_init(DmaLeaseRegion region, DmaLeasePool *out) {
    if (!out || !dma_lease_region_valid(region))
        return DMA_LEASE_INVALID_REGION;
    *out = (DmaLeasePool){.region = region, .valid = true};
    return DMA_LEASE_OK;
}

DmaLeaseResult dma_lease_reserve(DmaLeasePool *pool, uint64_t bytes) {
    if (!pool || !pool->valid || !dma_lease_region_valid(pool->region))
        return (DmaLeaseResult){.status = DMA_LEASE_INVALID_REGION};
    if (!bytes || bytes > UINT64_MAX - (DMA_LEASE_PAGE_BYTES - 1))
        return (DmaLeaseResult){.status = DMA_LEASE_INVALID_SIZE};
    uint64_t length = (bytes + DMA_LEASE_PAGE_BYTES - 1) & ~(DMA_LEASE_PAGE_BYTES - 1);
    if (pool->used > pool->region.length || length > pool->region.length - pool->used)
        return (DmaLeaseResult){.status = DMA_LEASE_EXHAUSTED};
    DmaLeaseBuffer buffer = {pool->region.virtual_address + pool->used,
                             pool->region.physical_address + pool->used, length};
    pool->used += length;
    return (DmaLeaseResult){.buffer = buffer};
}

DmaDeviceEpoch dma_device_epoch_init(uint64_t device_id) {
    return (DmaDeviceEpoch){.device_id = device_id,
                            .generation = device_id ? 1 : 0,
                            .phase = device_id ? DMA_DEVICE_ACTIVE : DMA_DEVICE_QUIESCENT};
}

bool dma_device_epoch_active(const DmaDeviceEpoch *device) {
    return device && device->device_id && device->generation && device->phase == DMA_DEVICE_ACTIVE;
}

DmaLeaseStatus dma_device_reset_begin(DmaDeviceEpoch *device) {
    if (!dma_device_epoch_active(device) || device->generation == UINT64_MAX)
        return DMA_LEASE_INVALID_TRANSITION;
    device->phase = DMA_DEVICE_RESETTING;
    return DMA_LEASE_OK;
}

DmaLeaseStatus dma_device_observe_quiescent(DmaDeviceEpoch *device, bool cc_enabled,
                                            bool csts_ready) {
    if (!device || device->phase != DMA_DEVICE_RESETTING || cc_enabled || csts_ready)
        return DMA_LEASE_INVALID_TRANSITION;
    device->phase = DMA_DEVICE_QUIESCENT;
    return DMA_LEASE_OK;
}

DmaLeaseStatus dma_device_resume(DmaDeviceEpoch *device, bool queues_ready) {
    if (!device || device->phase != DMA_DEVICE_QUIESCENT || !queues_ready || !device->device_id ||
        !device->generation || device->generation == UINT64_MAX)
        return DMA_LEASE_INVALID_TRANSITION;
    ++device->generation;
    device->phase = DMA_DEVICE_ACTIVE;
    return DMA_LEASE_OK;
}

DmaLease dma_lease_init(DmaLeaseBuffer buffer, const DmaDeviceEpoch *device) {
    if (!dma_device_epoch_active(device))
        return (DmaLease){.buffer = buffer, .phase = DMA_QUARANTINED};
    return (DmaLease){.buffer = buffer,
                      .phase = DMA_AVAILABLE,
                      .device_id = device->device_id,
                      .generation = device->generation};
}

DmaLeaseResult dma_lease_prepare(DmaLease *lease, const DmaDeviceEpoch *device, uint64_t tag) {
    if (!lease || !tag)
        return (DmaLeaseResult){.status = DMA_LEASE_INVALID_TRANSITION};
    if (lease->phase != DMA_AVAILABLE)
        return (DmaLeaseResult){.status = DMA_LEASE_BUSY};
    if (!dma_device_epoch_active(device) || lease->device_id != device->device_id ||
        lease->generation != device->generation)
        return (DmaLeaseResult){.status = DMA_LEASE_INVALID_TRANSITION};
    lease->phase = DMA_PREPARED;
    lease->tag = tag;
    return (DmaLeaseResult){.buffer = lease->buffer};
}

DmaLeaseStatus dma_lease_submit(DmaLease *lease, uint64_t tag) {
    if (!lease || lease->phase != DMA_PREPARED || lease->tag != tag)
        return DMA_LEASE_INVALID_TRANSITION;
    lease->phase = DMA_IN_FLIGHT;
    return DMA_LEASE_OK;
}

DmaLeaseStatus dma_lease_abort(DmaLease *lease, uint64_t tag) {
    if (!lease || lease->phase != DMA_PREPARED || lease->tag != tag)
        return DMA_LEASE_INVALID_TRANSITION;
    lease->phase = DMA_AVAILABLE;
    lease->tag = 0;
    return DMA_LEASE_OK;
}

DmaLeaseStatus dma_lease_complete(DmaLease *lease, uint64_t tag) {
    if (!lease || lease->phase != DMA_IN_FLIGHT || lease->tag != tag)
        return DMA_LEASE_INVALID_TRANSITION;
    lease->phase = DMA_AVAILABLE;
    lease->tag = 0;
    return DMA_LEASE_OK;
}

DmaLeaseStatus dma_lease_complete_observed(DmaLease *lease, uint64_t tag, uint64_t observed_tag,
                                           uint32_t observed_phase, uint32_t expected_phase) {
    if (observed_tag != tag || observed_phase != expected_phase) {
        dma_lease_quarantine(lease);
        return DMA_LEASE_INVALID_TRANSITION;
    }
    DmaLeaseStatus status = dma_lease_complete(lease, tag);
    if (status)
        dma_lease_quarantine(lease);
    return status;
}

DmaLeaseStatus dma_lease_uncertain(DmaLease *lease, uint64_t tag, bool submitted) {
    if (!lease)
        return DMA_LEASE_INVALID_TRANSITION;
    if (submitted) {
        bool valid = lease->phase == DMA_IN_FLIGHT && lease->tag == tag;
        dma_lease_quarantine(lease);
        return valid ? DMA_LEASE_OK : DMA_LEASE_INVALID_TRANSITION;
    }
    DmaLeaseStatus status = dma_lease_abort(lease, tag);
    if (status)
        dma_lease_quarantine(lease);
    return status;
}

void dma_lease_quarantine(DmaLease *lease) {
    if (lease) {
        lease->phase = DMA_QUARANTINED;
        lease->tag = 0;
    }
}

bool dma_lease_is_quarantined(const DmaLease *lease) {
    return lease && lease->phase == DMA_QUARANTINED;
}

DmaLeaseStatus dma_lease_retire_quarantined(DmaLease *lease, const DmaDeviceEpoch *device) {
    if (!lease || !device || device->phase != DMA_DEVICE_QUIESCENT ||
        device->generation == UINT64_MAX || lease->phase != DMA_QUARANTINED ||
        lease->device_id != device->device_id || lease->generation != device->generation)
        return DMA_LEASE_INVALID_TRANSITION;
    lease->generation = device->generation + 1;
    lease->phase = DMA_AVAILABLE;
    lease->tag = 0;
    return DMA_LEASE_OK;
}

DmaLeaseStatus dma_lease_rebind_available(DmaLease *lease, const DmaDeviceEpoch *device) {
    if (!lease || !device || device->phase != DMA_DEVICE_QUIESCENT ||
        device->generation == UINT64_MAX || lease->phase != DMA_AVAILABLE || lease->tag ||
        lease->device_id != device->device_id || lease->generation != device->generation)
        return DMA_LEASE_INVALID_TRANSITION;
    lease->generation = device->generation + 1;
    return DMA_LEASE_OK;
}


