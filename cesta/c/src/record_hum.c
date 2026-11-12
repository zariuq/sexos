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
#include "sexos_client.h"

static uint64_t memory[64];

static uint64_t word_at(uint64_t seed, size_t index) {
    uint32_t turn = (uint32_t)(index % 64);
    return (((seed << turn) | (seed >> ((64 - turn) % 64))) ^ (uint64_t)index);
}

void record_hum_main(void) {
    uint64_t seed = (uint64_t)(uintptr_t)memory;
    for (size_t i = 0; i < 64; ++i) memory[i] = word_at(seed, i);
    sexos_line("[record.hum.start] seed=%#llx", (unsigned long long)seed);
    uint64_t cycles = 0;
    for (;;) {
        sexos_wait_self_ticks(1);
        for (size_t i = 0; i < 64; ++i) {
            if (memory[i] != word_at(seed, i)) {
                sexos_line("[record.hum.corrupt] seed=%#llx word=%zu", (unsigned long long)seed, i);
                sexos_task_finish(3);
            }
        }
        if (cycles < 2) {
            sexos_line("[record.hum.alive] seed=%#llx cycle=%llu",
                       (unsigned long long)seed, (unsigned long long)cycles);
        }
        ++cycles;
        if (sexos_record_call(3, 0, 0).first) {
            sexos_line("[record.hum.exit] seed=%#llx cycles=%llu",
                       (unsigned long long)seed, (unsigned long long)cycles);
            sexos_task_finish(0);
        }
    }
}
