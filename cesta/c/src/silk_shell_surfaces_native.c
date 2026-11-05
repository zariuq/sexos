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

#include "silk_shell_surfaces.h"

ShellActivationRaw shell_surfaces_activation_call(uint64_t slot, bool commit) {
    uint64_t status = 66, pd = commit ? 1 : 0, task;
    register uint64_t surface __asm__("r10");
    register uint64_t state __asm__("r8");
    __asm__ volatile("syscall"
                     : "+a"(status), "+S"(pd), "=d"(task), "=r"(surface), "=r"(state)
                     : "D"(slot)
                     : "rcx", "r11", "memory", "cc");
    return (ShellActivationRaw){status, pd, task, surface, state};
}

void shell_surfaces_debug_record(const ShellDebugRecord *record) {
    if (record->len > sizeof(record->bytes)) {
        sexos_fail();
        return;
    }
    sexos_raw_print((const char *)record->bytes, record->len);
}


