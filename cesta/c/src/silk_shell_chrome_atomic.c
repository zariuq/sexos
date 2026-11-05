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

#include "silk_shell_chrome.h"

uint32_t shell_chrome_peer_load_dimension(const atomic_uint_least32_t *dimension) {
    return atomic_load_explicit(dimension, memory_order_acquire);
}

bool shell_chrome_peer_exchange_bool(atomic_bool *value, bool next) {
    return atomic_exchange_explicit(value, next, memory_order_relaxed);
}

uint32_t shell_chrome_peer_load_relaxed(const atomic_uint_least32_t *value) {
    return atomic_load_explicit(value, memory_order_relaxed);
}

void shell_chrome_peer_store_relaxed(atomic_uint_least32_t *value, uint32_t next) {
    atomic_store_explicit(value, next, memory_order_relaxed);
}


