#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "sexos_dma_lease.h"

int main(void)
{
    DmaLeaseRegion region={7,UINT64_C(0x400000),UINT64_C(0x100000),3*4096};
    DmaLeasePool pool={0};
    assert(dma_lease_pool_init(region,&pool)==DMA_LEASE_OK && pool.valid && !pool.used);
    DmaLeaseResult first=dma_lease_reserve(&pool,1),second=dma_lease_reserve(&pool,4097);
    assert(!first.status && first.buffer.virtual_address==region.virtual_address &&
           first.buffer.physical_address==region.physical_address && first.buffer.length==4096);
    assert(!second.status && second.buffer.virtual_address==region.virtual_address+4096 &&
           second.buffer.physical_address==region.physical_address+4096 && second.buffer.length==8192);
    assert(pool.used==region.length && dma_lease_reserve(&pool,1).status==DMA_LEASE_EXHAUSTED);
    assert(dma_lease_reserve(&pool,0).status==DMA_LEASE_INVALID_SIZE);
    DmaDeviceEpoch device=dma_device_epoch_init(9);
    assert(dma_device_epoch_active(&device) && device.generation==1);
    DmaLease lease=dma_lease_init(first.buffer,&device);
    DmaLease available_old=dma_lease_init(second.buffer,&device);
    assert(lease.phase==DMA_AVAILABLE && !lease.tag && lease.device_id==9 && lease.generation==1);
    DmaDeviceEpoch inactive=device;inactive.phase=DMA_DEVICE_RESETTING;
    assert(dma_lease_prepare(&lease,&inactive,16).status==DMA_LEASE_INVALID_TRANSITION);
    DmaDeviceEpoch invalid_reset=device;invalid_reset.phase=DMA_DEVICE_QUIESCENT;
    assert(dma_device_reset_begin(&invalid_reset)==DMA_LEASE_INVALID_TRANSITION);
    DmaDeviceEpoch future=device;future.generation=2;
    assert(dma_lease_prepare(&lease,&future,16).status==DMA_LEASE_INVALID_TRANSITION);
    assert(!dma_lease_prepare(&lease,&device,17).status && lease.phase==DMA_PREPARED && lease.tag==17);
    assert(dma_lease_prepare(&lease,&device,18).status==DMA_LEASE_BUSY);
    assert(dma_lease_submit(&lease,18)==DMA_LEASE_INVALID_TRANSITION && lease.phase==DMA_PREPARED);
    assert(dma_lease_submit(&lease,17)==DMA_LEASE_OK && lease.phase==DMA_IN_FLIGHT);
    assert(dma_lease_abort(&lease,17)==DMA_LEASE_INVALID_TRANSITION && lease.phase==DMA_IN_FLIGHT);
    assert(dma_lease_complete_observed(&lease,17,17,1,1)==DMA_LEASE_OK && lease.phase==DMA_AVAILABLE);
    assert(!dma_lease_prepare(&lease,&device,18).status && !dma_lease_submit(&lease,18));
    assert(dma_lease_complete_observed(&lease,18,19,1,1)==DMA_LEASE_INVALID_TRANSITION);
    assert(dma_lease_is_quarantined(&lease));
    assert(dma_lease_prepare(&lease,&device,20).status==DMA_LEASE_BUSY);
    assert(dma_device_reset_begin(&device)==DMA_LEASE_OK && device.phase==DMA_DEVICE_RESETTING);
    assert(dma_device_observe_quiescent(&device,true,false)==DMA_LEASE_INVALID_TRANSITION);
    assert(device.phase==DMA_DEVICE_RESETTING);
    assert(dma_device_observe_quiescent(&device,false,false)==DMA_LEASE_OK &&
           device.phase==DMA_DEVICE_QUIESCENT);
    DmaDeviceEpoch wrong_device=device;wrong_device.device_id++;
    assert(dma_lease_retire_quarantined(&lease,&wrong_device)==DMA_LEASE_INVALID_TRANSITION);
    DmaLease wrong_generation=lease;wrong_generation.generation++;
    assert(dma_lease_retire_quarantined(&wrong_generation,&device)==DMA_LEASE_INVALID_TRANSITION);
    assert(dma_lease_retire_quarantined(&available_old,&device)==DMA_LEASE_INVALID_TRANSITION);
    assert(dma_lease_retire_quarantined(&lease,&device)==DMA_LEASE_OK &&
           lease.phase==DMA_AVAILABLE && lease.generation==2);
    assert(dma_lease_prepare(&lease,&device,20).status==DMA_LEASE_INVALID_TRANSITION);
    DmaLease second_lease=available_old;
    assert(dma_lease_rebind_available(&second_lease,&device)==DMA_LEASE_OK &&
           second_lease.generation==2);
    assert(dma_device_resume(&device,false)==DMA_LEASE_INVALID_TRANSITION &&
           device.phase==DMA_DEVICE_QUIESCENT);
    assert(dma_device_resume(&device,true)==DMA_LEASE_OK && dma_device_epoch_active(&device) &&
           device.generation==2);
    assert(!dma_lease_prepare(&lease,&device,20).status);
    assert(dma_lease_abort(&lease,20)==DMA_LEASE_OK);
    lease=second_lease;
    assert(!dma_lease_prepare(&lease,&device,21).status);
    assert(dma_lease_uncertain(&lease,21,false)==DMA_LEASE_OK && lease.phase==DMA_AVAILABLE);
    assert(!dma_lease_prepare(&lease,&device,22).status && !dma_lease_submit(&lease,22));
    assert(dma_lease_uncertain(&lease,22,true)==DMA_LEASE_OK && dma_lease_is_quarantined(&lease));
    DmaDeviceEpoch exhausted={9,UINT64_MAX,DMA_DEVICE_ACTIVE};
    assert(dma_device_reset_begin(&exhausted)==DMA_LEASE_INVALID_TRANSITION);
    assert(dma_lease_init(first.buffer,NULL).phase==DMA_QUARANTINED);
    DmaLeaseRegion bad=region;bad.virtual_address++;
    assert(dma_lease_pool_init(bad,&pool)==DMA_LEASE_INVALID_REGION);
    bad=region;bad.virtual_address=DMA_LEASE_USER_END-4096;bad.length=8192;
    assert(!dma_lease_region_valid(bad));
    bad=region;bad.physical_address=DMA_LEASE_PHYSICAL_END-4096;bad.length=8192;
    assert(!dma_lease_region_valid(bad));
    bad=region;bad.length=UINT64_MAX;
    assert(!dma_lease_region_valid(bad));
    puts("DMA lease controls passed: bounded pool, exact completion, uncertainty quarantine, observed reset and generation-bound retirement");
}
