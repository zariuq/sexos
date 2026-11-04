#include "sexos_arm64_boot.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

Arm64Phase4Status arm64_phase4_state_init(Arm64Phase4State *state,uint64_t frame_a,uint64_t frame_b)
{
    if(!state || !frame_a || !frame_b || frame_a==frame_b)return ARM64_PHASE4_BAD_STATE;
    *state=(Arm64Phase4State){frame_a,frame_b,ARM64_TASK_A,true,false};
    return ARM64_PHASE4_OK;
}
Arm64Phase4Status arm64_phase4_tick(Arm64Phase4State *state,uint64_t frame,uint32_t intid,uint64_t *next)
{
    if(!state || !next || !state->started || !frame || !state->frame_a || !state->frame_b ||
       state->current>ARM64_TASK_B)return ARM64_PHASE4_BAD_STATE;
    if(intid!=30u)return ARM64_PHASE4_BAD_INTERRUPT;
    if(state->current==ARM64_TASK_A)state->frame_a=frame;else state->frame_b=frame;
    state->current=state->b_dead?ARM64_TASK_A:(state->current==ARM64_TASK_A?ARM64_TASK_B:ARM64_TASK_A);
    *next=state->current==ARM64_TASK_A?state->frame_a:state->frame_b;
    return *next?ARM64_PHASE4_OK:ARM64_PHASE4_BAD_STATE;
}
Arm64Phase4Status arm64_phase4_kill(Arm64Phase4State *state,uint64_t frame,uint64_t *next)
{
    if(!state || !next || !frame || !state->frame_a || !state->frame_b)
        return ARM64_PHASE4_BAD_STATE;
    if(!state->started)return ARM64_PHASE4_BOOT_FAULT;
    if(state->current!=ARM64_TASK_B)return ARM64_PHASE4_SURVIVOR_FAULT;
    state->frame_b=frame;state->b_dead=true;state->current=ARM64_TASK_A;*next=state->frame_a;
    return ARM64_PHASE4_OK;
}
bool arm64_phase4_gic_valid(Arm64GicObservation o)
{
    return (o.distributor_control&(UINT32_C(1)<<1))!=0 &&
        (o.redistributor_waker&(UINT32_C(1)<<2))==0 && (o.sre&1u)!=0 &&
        (o.pmr==UINT64_C(0xf8) || o.pmr==UINT64_C(0xfc) ||
         o.pmr==UINT64_C(0xfe) || o.pmr==UINT64_C(0xff)) &&
        (o.group_enable&1u)!=0 &&
        (o.timer_group&(UINT32_C(1)<<30))!=0 && (o.timer_enable&(UINT32_C(1)<<30))!=0;
}
bool arm64_phase4_ap_valid(Arm64ApObservation o)
{
    return o.psci_result==0 && o.ttbr0!=0 && o.ttbr1!=0 && o.ttbr0!=o.ttbr1 &&
        (o.ttbr0&UINT64_C(0xfff))==0 && (o.ttbr1&UINT64_C(0xfff))==0 && o.acknowledged;
}
bool arm64_phase4_timer_interval(uint64_t frequency,uint64_t *interval)
{
    if(!interval)return false;
    *interval=frequency/100u;return *interval!=0;
}


