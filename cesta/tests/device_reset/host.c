#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "sexos_device_reset.h"

int main(void)
{
    DeviceResetState state=device_reset_init(2,16,0x00000100,0x5845101b,0x22,7);
    assert(device_reset_state_valid(&state));
    assert(!device_reset_state_valid(&(DeviceResetState){.present=true,.owner=0,.slot=16,
        .device=0x100,.identity=0x5845101b,.generation=1,.vector=0x22}));
    assert(!device_reset_state_valid(&(DeviceResetState){.present=true,.owner=2,.slot=16,
        .device=0x100,.identity=0,.generation=1,.vector=0x22}));
    DeviceResetReceipt r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_QUERY,
        .interrupt_revision=9,.acknowledged_revision=8});
    assert(r.status==DEVICE_RESET_OK && r.generation==1 && r.phase==DEVICE_RESET_ACTIVE &&
        r.interrupt_revision==9 && r.acknowledged_revision==8 && state.interrupt_revision==7 &&
        state.acknowledged_revision==7);
    DeviceResetState wrong_phase=state;wrong_phase.phase=DEVICE_RESET_QUIESCENT;
    assert(device_reset_apply(&wrong_phase,(DeviceResetRequest){.action=DEVICE_RESET_BEGIN,
        .expected_generation=1,.interrupt_revision=7}).status==DEVICE_RESET_INVALID);
    assert(device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_OBSERVE_QUIESCENT,
        .expected_generation=1,.interrupt_revision=7,.acknowledged_revision=7}).status==DEVICE_RESET_INVALID);
    assert(device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_RESUME,
        .expected_generation=1,.interrupt_revision=7,.acknowledged_revision=7,
        .cc=1,.csts=1,.queues_ready=true}).status==DEVICE_RESET_INVALID);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_BEGIN,.expected_generation=2,.interrupt_revision=7});
    assert(r.status==DEVICE_RESET_STALE && state.phase==DEVICE_RESET_ACTIVE);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_BEGIN,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=7});
    assert(r.status==DEVICE_RESET_INTERRUPTED && state.phase==DEVICE_RESET_ACTIVE);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_BEGIN,.expected_generation=1,
        .interrupt_revision=7,.acknowledged_revision=7});
    assert(r.status==DEVICE_RESET_OK && state.phase==DEVICE_RESET_RESETTING && state.interrupt_revision==7);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_OBSERVE_QUIESCENT,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=7});
    assert(r.status==DEVICE_RESET_INTERRUPTED && state.phase==DEVICE_RESET_RESETTING);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_ACK_INTERRUPT,.expected_generation=1,
        .interrupt_revision=9,.acknowledged_revision=9});
    assert(r.status==DEVICE_RESET_INTERRUPTED && state.acknowledged_revision==7);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_ACK_INTERRUPT,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=9});
    assert(r.status==DEVICE_RESET_INTERRUPTED && state.acknowledged_revision==7);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_ACK_INTERRUPT,.expected_generation=1,
        .interrupt_revision=7,.acknowledged_revision=8});
    assert(r.status==DEVICE_RESET_INTERRUPTED && state.acknowledged_revision==7);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_ACK_INTERRUPT,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=8});
    assert(r.status==DEVICE_RESET_OK && state.acknowledged_revision==8);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_OBSERVE_QUIESCENT,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=8,.cc=1});
    assert(r.status==DEVICE_RESET_HARDWARE && state.phase==DEVICE_RESET_RESETTING);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_OBSERVE_QUIESCENT,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=8});
    assert(r.status==DEVICE_RESET_OK && state.phase==DEVICE_RESET_QUIESCENT);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_RESUME,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=8,.cc=1,.csts=1});
    assert(r.status==DEVICE_RESET_INVALID && state.phase==DEVICE_RESET_QUIESCENT);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_RESUME,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=8,.cc=1,.queues_ready=true});
    assert(r.status==DEVICE_RESET_HARDWARE && state.phase==DEVICE_RESET_QUIESCENT);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_RESUME,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=8,.csts=1,.queues_ready=true});
    assert(r.status==DEVICE_RESET_HARDWARE && state.phase==DEVICE_RESET_QUIESCENT);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_RESUME,.expected_generation=1,
        .interrupt_revision=9,.acknowledged_revision=9,.cc=1,.csts=1,.queues_ready=true});
    assert(r.status==DEVICE_RESET_INTERRUPTED && state.phase==DEVICE_RESET_QUIESCENT);
    r=device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_RESUME,.expected_generation=1,
        .interrupt_revision=8,.acknowledged_revision=8,.cc=1,.csts=1,.queues_ready=true});
    assert(r.status==DEVICE_RESET_OK && state.phase==DEVICE_RESET_ACTIVE && state.generation==2);
    assert(device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_BEGIN,.expected_generation=1,.interrupt_revision=8}).status==DEVICE_RESET_STALE);
    state.generation=UINT64_MAX;
    assert(device_reset_apply(&state,(DeviceResetRequest){.action=DEVICE_RESET_BEGIN,.expected_generation=UINT64_MAX,.interrupt_revision=8}).status==DEVICE_RESET_EXHAUSTED);
    assert(!device_reset_state_valid(&(DeviceResetState){0}));
    DeviceResetState orphan=device_reset_init(2,16,0x100,0x5845101b,0x22,0);
    r=device_reset_apply(&orphan,(DeviceResetRequest){.action=DEVICE_RESET_OWNER_DEAD,.expected_generation=1});
    assert(r.status==DEVICE_RESET_OK&&orphan.phase==DEVICE_RESET_ORPHANED);
    assert(device_reset_apply(&orphan,(DeviceResetRequest){.action=DEVICE_RESET_BEGIN,
        .expected_generation=1}).status==DEVICE_RESET_INVALID);
    puts("device reset controls passed: stale, interrupt, hardware, quiescence, queue and exhaustion boundaries");
}
