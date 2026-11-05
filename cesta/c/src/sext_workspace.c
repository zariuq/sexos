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
#include "sext_service.h"
/*Native memory boundary: the caller holds the completed retirement grant for the entire alias. Volatile access and compiler fences preserve the upstream workspace behavior.*/

#include <stdatomic.h>

void sext_backend_zero(uint64_t address, uint64_t bytes) {
    uint8_t *p = (uint8_t *)(uintptr_t)address;
    for (uint64_t i = 0; i < bytes; ++i)
        p[i] = 0;
    atomic_signal_fence(memory_order_release);
}

void sext_workspace_admit(SextWorkspace *out, uint64_t base, uint64_t bytes) {
    if (!base || base % 4096 || bytes < 4096 || bytes % 4096 || bytes > SIZE_MAX)
        sext_fatal("WORKSPACE_SIZE", 0);
    sext_backend_zero(base, bytes);
    atomic_signal_fence(memory_order_seq_cst);
    for (size_t i = 0; i < (size_t)bytes / 8; ++i)
        if (((const volatile uint64_t *)(uintptr_t)base)[i] != 0)
            sext_fatal("WORKSPACE_ZERO", 0);
    *out = (SextWorkspace){base, (size_t)bytes, 0, 0};
}

uint64_t sext_workspace_record(SextWorkspace *w, uint32_t target, uint64_t token) {
    size_t offset = w->next;
    volatile uint64_t *p = (volatile uint64_t *)((uint8_t *)(uintptr_t)w->base + offset);
    uint8_t *bytes = (uint8_t *)(uintptr_t)p;
    for (size_t i = 0; i < 24; ++i)
        bytes[i] = 0;
    p[0] = target;
    p[1] = token;
    if (w->records == UINT64_MAX)
        sext_fatal("WORKSPACE_RECORD_WRAP", 0);
    ++w->records;
    p[2] = w->records;
    if (p[0] != target || p[1] != token || p[2] != w->records)
        sext_fatal("WORKSPACE_RECORD_VERIFY", 0);
    w->next = offset + 48 <= w->bytes ? offset + 24 : 0;
    return w->records;
}

void sext_workspace_rewind(SextWorkspace *w) {
    w->next = 0;
}


