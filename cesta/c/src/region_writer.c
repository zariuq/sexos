#include "sexos_region_writer.h"
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

bool region_writer_valid(const RegionWriterState *state) {
    if (!state->present || !state->region_id || !state->home || !state->peer ||
        state->home == state->peer || !state->revision ||
        (state->current != state->home && state->current != state->peer))
        return false;
    return state->current == state->home || state->job != 0;
}

RegionWriterResult region_writer_query(RegionWriterState state, uint32_t caller) {
    if (!region_writer_valid(&state))
        return (RegionWriterResult){.status = REGION_WRITER_INVALID};
    if (caller != state.home && caller != state.peer)
        return (RegionWriterResult){.status = REGION_WRITER_DENIED};
    return (RegionWriterResult){.state = state};
}

RegionWriterResult region_writer_change(RegionWriterState state, uint32_t caller,
                                        RegionWriterAction action, uint64_t expected_revision,
                                        uint64_t job) {
    RegionWriterResult queried = region_writer_query(state, caller);
    if (queried.status)
        return queried;
    if (!job || action < REGION_WRITER_OFFER || action > REGION_WRITER_RECOVER)
        return (RegionWriterResult){.status = REGION_WRITER_INVALID, .state = state};
    if (expected_revision != state.revision)
        return (RegionWriterResult){.status = REGION_WRITER_STALE, .state = state};
    if (state.revision == UINT64_MAX)
        return (RegionWriterResult){.status = REGION_WRITER_EXHAUSTED, .state = state};
    RegionWriterState next = state;
    if (action == REGION_WRITER_OFFER) {
        if (caller != state.home || state.current != state.home || state.job)
            return (RegionWriterResult){.status = REGION_WRITER_BUSY, .state = state};
        next.job = job;
    } else if (action == REGION_WRITER_CLAIM) {
        if (caller != state.peer || state.current != state.home || state.job != job)
            return (RegionWriterResult){.status = REGION_WRITER_BUSY, .state = state};
        next.current = state.peer;
    } else if (action == REGION_WRITER_RELEASE) {
        if (caller != state.peer || state.current != state.peer || state.job != job)
            return (RegionWriterResult){.status = REGION_WRITER_BUSY, .state = state};
        next.current = state.home;
        next.job = 0;
    } else if (action == REGION_WRITER_CANCEL) {
        if (caller != state.home || state.current != state.home || state.job != job)
            return (RegionWriterResult){.status = REGION_WRITER_BUSY, .state = state};
        next.job = 0;
    } else {
        if (caller != state.home || state.current != state.peer || state.job != job)
            return (RegionWriterResult){.status = REGION_WRITER_BUSY, .state = state};
        next.current = state.home;
        next.job = 0;
    }
    ++next.revision;
    return (RegionWriterResult){.state = next};
}


