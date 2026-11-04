#include "sexos_device_reset.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

bool device_reset_state_valid(const DeviceResetState *s)
{
    return s && s->present && s->owner && s->slot && s->device && s->identity &&
        s->vector>=32 && s->generation && s->phase<=DEVICE_RESET_ORPHANED;
}
DeviceResetState device_reset_init(uint32_t owner,uint64_t slot,uint32_t device,
    uint32_t identity,uint8_t vector,uint64_t interrupt_revision)
{
    DeviceResetState state={.present=true,.owner=owner,.device=device,.identity=identity,
        .slot=slot,.generation=1,.interrupt_revision=interrupt_revision,
        .acknowledged_revision=interrupt_revision,.vector=vector,.phase=DEVICE_RESET_ACTIVE};
    return device_reset_state_valid(&state)?state:(DeviceResetState){0};
}
static DeviceResetReceipt device_reset_receipt(DeviceResetStatus status,
    const DeviceResetState *state,DeviceResetRequest request)
{
    return(DeviceResetReceipt){.status=status,.generation=state?state->generation:0,
        .interrupt_revision=state?state->interrupt_revision:0,
        .acknowledged_revision=state?state->acknowledged_revision:0,
        .phase=state?state->phase:DEVICE_RESET_ACTIVE,.cc=request.cc,.csts=request.csts};
}
DeviceResetReceipt device_reset_apply(DeviceResetState *state,DeviceResetRequest q)
{
    if(!device_reset_state_valid(state))return device_reset_receipt(DEVICE_RESET_INVALID,state,q);
    if(q.action==DEVICE_RESET_QUERY){
        DeviceResetReceipt receipt=device_reset_receipt(DEVICE_RESET_OK,state,q);
        receipt.interrupt_revision=q.interrupt_revision;
        receipt.acknowledged_revision=q.acknowledged_revision;
        return receipt;
    }
    if(q.expected_generation!=state->generation)return device_reset_receipt(DEVICE_RESET_STALE,state,q);
    if(q.action==DEVICE_RESET_OWNER_DEAD){
        if(state->phase==DEVICE_RESET_ORPHANED)return device_reset_receipt(DEVICE_RESET_INVALID,state,q);
        state->phase=DEVICE_RESET_ORPHANED;
        return device_reset_receipt(DEVICE_RESET_OK,state,q);
    }
    if(state->phase==DEVICE_RESET_ORPHANED)return device_reset_receipt(DEVICE_RESET_INVALID,state,q);
    if(q.action==DEVICE_RESET_ACK_INTERRUPT){
        if(state->acknowledged_revision==UINT64_MAX ||
           q.acknowledged_revision!=state->acknowledged_revision+1 ||
           q.acknowledged_revision>q.interrupt_revision)
            return device_reset_receipt(DEVICE_RESET_INTERRUPTED,state,q);
        state->acknowledged_revision=q.acknowledged_revision;
        return device_reset_receipt(DEVICE_RESET_OK,state,q);
    }
    if(q.action==DEVICE_RESET_BEGIN){
        if(state->phase!=DEVICE_RESET_ACTIVE)return device_reset_receipt(DEVICE_RESET_INVALID,state,q);
        if(state->generation==UINT64_MAX)return device_reset_receipt(DEVICE_RESET_EXHAUSTED,state,q);
        if(q.interrupt_revision!=q.acknowledged_revision)
            return device_reset_receipt(DEVICE_RESET_INTERRUPTED,state,q);
        state->phase=DEVICE_RESET_RESETTING;state->interrupt_revision=q.interrupt_revision;
        state->acknowledged_revision=q.acknowledged_revision;
        return device_reset_receipt(DEVICE_RESET_OK,state,q);
    }
    if(q.interrupt_revision!=q.acknowledged_revision)
        return device_reset_receipt(DEVICE_RESET_INTERRUPTED,state,q);
    if(q.action==DEVICE_RESET_OBSERVE_QUIESCENT){
        if(state->phase!=DEVICE_RESET_RESETTING)return device_reset_receipt(DEVICE_RESET_INVALID,state,q);
        if((q.cc&1u)||(q.csts&1u))return device_reset_receipt(DEVICE_RESET_HARDWARE,state,q);
        state->phase=DEVICE_RESET_QUIESCENT;state->interrupt_revision=q.interrupt_revision;
        state->acknowledged_revision=q.acknowledged_revision;
        return device_reset_receipt(DEVICE_RESET_OK,state,q);
    }
    if(q.action==DEVICE_RESET_RESUME){
        if(state->phase!=DEVICE_RESET_QUIESCENT || !q.queues_ready)
            return device_reset_receipt(DEVICE_RESET_INVALID,state,q);
        if(q.interrupt_revision!=state->interrupt_revision)
            return device_reset_receipt(DEVICE_RESET_INTERRUPTED,state,q);
        if(!(q.cc&1u)||!(q.csts&1u))return device_reset_receipt(DEVICE_RESET_HARDWARE,state,q);
        if(state->generation==UINT64_MAX)return device_reset_receipt(DEVICE_RESET_EXHAUSTED,state,q);
        ++state->generation;state->phase=DEVICE_RESET_ACTIVE;
        return device_reset_receipt(DEVICE_RESET_OK,state,q);
    }
    return device_reset_receipt(DEVICE_RESET_INVALID,state,q);
}


