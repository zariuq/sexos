#include "sexos_smp_park.h"
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

SmpParkStatus smp_park_init(SmpParkState *state, size_t cpu_count, uint32_t bsp_lapic_id,
                            uint64_t generation) {
    if (!state || !cpu_count || cpu_count > SMP_PARK_MAX_CORES)
        return SMP_PARK_INVALID;
    *state = (SmpParkState){
        .cpu_count = cpu_count, .bsp_lapic_id = bsp_lapic_id, .generation = generation};
    return SMP_PARK_OK;
}

SmpParkStatus smp_park_enroll(SmpParkState *state, size_t slot, uint32_t lapic_id, bool bsp) {
    if (!state || state->sealed || slot >= state->cpu_count ||
        state->enrolled_count >= state->cpu_count || state->cores[slot].enrolled ||
        (bsp != (lapic_id == state->bsp_lapic_id)))
        return SMP_PARK_INVALID;
    for (size_t i = 0; i < state->cpu_count; ++i)
        if (state->cores[i].enrolled && state->cores[i].lapic_id == lapic_id)
            return SMP_PARK_DUPLICATE;
    SmpParkCore core = {
        .lapic_id = lapic_id, .enrolled = true, .bsp = bsp, .arrived = bsp, .parked = bsp};
    state->cores[slot] = core;
    ++state->enrolled_count;
    if (bsp)
        ++state->arrived_count;
    return SMP_PARK_OK;
}

SmpParkStatus smp_park_arrive(SmpParkState *state, size_t slot, uint32_t lapic_id) {
    if (!state || state->sealed || slot >= state->cpu_count)
        return SMP_PARK_INVALID;
    SmpParkCore *core = &state->cores[slot];
    if (!core->enrolled || core->bsp || core->lapic_id != lapic_id)
        return SMP_PARK_INVALID;
    if (core->arrived || core->parked)
        return SMP_PARK_DUPLICATE;
    core->arrived = true;
    core->parked = true;
    ++state->arrived_count;
    return SMP_PARK_OK;
}

bool smp_park_all_quiescent(const SmpParkState *state) {
    if (!state || !state->sealed || state->arrived_count != state->cpu_count)
        return false;
    for (size_t i = 0; i < state->cpu_count; ++i)
        if (!state->cores[i].enrolled || !state->cores[i].arrived || !state->cores[i].parked)
            return false;
    return true;
}

SmpParkStatus smp_park_seal(SmpParkState *state) {
    if (!state || state->sealed || state->enrolled_count != state->cpu_count ||
        state->arrived_count != state->cpu_count)
        return SMP_PARK_NOT_READY;
    size_t bsp_count = 0;
    for (size_t i = 0; i < state->cpu_count; ++i) {
        SmpParkCore core = state->cores[i];
        if (!core.enrolled || !core.arrived || !core.parked)
            return SMP_PARK_NOT_READY;
        if (core.bsp)
            ++bsp_count;
    }
    if (bsp_count != 1)
        return SMP_PARK_INVALID;
    state->sealed = true;
    return SMP_PARK_OK;
}

bool smp_park_can_commit(const SmpParkState *state, uint64_t expected) {
    return state && expected < UINT64_MAX && state->generation == expected &&
           smp_park_all_quiescent(state);
}

SmpParkStatus smp_park_advance(SmpParkState *state, uint64_t expected, uint64_t next) {
    if (!smp_park_can_commit(state, expected))
        return expected == UINT64_MAX ? SMP_PARK_EXHAUSTED : SMP_PARK_STALE;
    if (next != expected + 1u)
        return SMP_PARK_INVALID;
    state->generation = next;
    return SMP_PARK_OK;
}


