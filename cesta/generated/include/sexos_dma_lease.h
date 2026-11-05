
#ifndef SEXOS_DMA_LEASE_H
#define SEXOS_DMA_LEASE_H
#include <stdint.h>
#include <stdbool.h>
#define DMA_LEASE_PAGE_BYTES UINT64_C(4096)
#define DMA_LEASE_USER_END (UINT64_C(1) << 47)
#define DMA_LEASE_PHYSICAL_END (UINT64_C(1) << 52)

typedef enum {
    DMA_LEASE_OK,
    DMA_LEASE_INVALID_REGION,
    DMA_LEASE_INVALID_SIZE,
    DMA_LEASE_EXHAUSTED,
    DMA_LEASE_BUSY,
    DMA_LEASE_INVALID_TRANSITION
} DmaLeaseStatus;

typedef struct {
    uint32_t region_id;
    uint64_t virtual_address, physical_address, length;
} DmaLeaseRegion;

typedef struct {
    uint64_t virtual_address, physical_address, length;
} DmaLeaseBuffer;

typedef struct {
    DmaLeaseRegion region;
    uint64_t used;
    bool valid;
} DmaLeasePool;

typedef enum {
    DMA_AVAILABLE,
    DMA_PREPARED,
    DMA_IN_FLIGHT,
    DMA_QUARANTINED
} DmaLeasePhase;

typedef enum {
    DMA_DEVICE_ACTIVE,
    DMA_DEVICE_RESETTING,
    DMA_DEVICE_QUIESCENT
} DmaDevicePhase;

typedef struct {
    uint64_t device_id, generation;
    DmaDevicePhase phase;
} DmaDeviceEpoch;

typedef struct {
    DmaLeaseBuffer buffer;
    DmaLeasePhase phase;
    uint64_t tag, device_id, generation;
} DmaLease;

typedef struct {
    DmaLeaseStatus status;
    DmaLeaseBuffer buffer;
} DmaLeaseResult;

bool dma_lease_region_valid(DmaLeaseRegion region);
DmaLeaseStatus dma_lease_pool_init(DmaLeaseRegion region, DmaLeasePool *out);
DmaLeaseResult dma_lease_reserve(DmaLeasePool *pool, uint64_t bytes);
DmaDeviceEpoch dma_device_epoch_init(uint64_t device_id);
bool dma_device_epoch_active(const DmaDeviceEpoch *device);
DmaLeaseStatus dma_device_reset_begin(DmaDeviceEpoch *device);
DmaLeaseStatus dma_device_observe_quiescent(DmaDeviceEpoch *device, bool cc_enabled,
                                            bool csts_ready);
DmaLeaseStatus dma_device_resume(DmaDeviceEpoch *device, bool queues_ready);
DmaLease dma_lease_init(DmaLeaseBuffer buffer, const DmaDeviceEpoch *device);
DmaLeaseResult dma_lease_prepare(DmaLease *lease, const DmaDeviceEpoch *device, uint64_t tag);
DmaLeaseStatus dma_lease_submit(DmaLease *lease, uint64_t tag);
DmaLeaseStatus dma_lease_abort(DmaLease *lease, uint64_t tag);
DmaLeaseStatus dma_lease_complete(DmaLease *lease, uint64_t tag);
DmaLeaseStatus dma_lease_complete_observed(DmaLease *lease, uint64_t tag, uint64_t observed_tag,
                                           uint32_t observed_phase, uint32_t expected_phase);
DmaLeaseStatus dma_lease_uncertain(DmaLease *lease, uint64_t tag, bool submitted);
void dma_lease_quarantine(DmaLease *lease);
bool dma_lease_is_quarantined(const DmaLease *lease);
DmaLeaseStatus dma_lease_retire_quarantined(DmaLease *lease, const DmaDeviceEpoch *device);
DmaLeaseStatus dma_lease_rebind_available(DmaLease *lease, const DmaDeviceEpoch *device);
#endif
