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
#include "sexos_arm64_boot.h"

#define ARM64_PL011_BASE UINT64_C(0x09000000)
#define ARM64_PL011_DR UINT64_C(0x00)
#define ARM64_PL011_FR UINT64_C(0x18)
#define ARM64_PL011_TXFF (UINT32_C(1) << 5)

uint32_t arm64_mmio_read32(uint64_t address) {
    return *(volatile const uint32_t *)(uintptr_t)address;
}

void arm64_mmio_write32(uint64_t address, uint32_t value) {
    *(volatile uint32_t *)(uintptr_t)address = value;
}

void arm64_write_byte(uint8_t byte) {
    while ((arm64_mmio_read32(ARM64_PL011_BASE + ARM64_PL011_FR) & ARM64_PL011_TXFF) != 0)
        __asm__ volatile("yield");
    arm64_mmio_write32(ARM64_PL011_BASE + ARM64_PL011_DR, byte);
}

void arm64_write_str(const char *text) {
    while (*text)
        arm64_write_byte((uint8_t)*text++);
}

void arm64_write_hex(uint64_t value) {
    static const char hex[] = "0123456789abcdef";
    for (int shift = 60; shift >= 0; shift -= 4)
        arm64_write_byte((uint8_t)hex[(value >> (unsigned)shift) & 15u]);
}

void arm64_write_dec(uint64_t value) {
    uint8_t digits[20];
    size_t at = sizeof digits;
    do {
        digits[--at] = (uint8_t)('0' + value % 10u);
        value /= 10u;
    } while (value);
    while (at < sizeof digits)
        arm64_write_byte(digits[at++]);
}

uint8_t arm64_current_el(void) {
    uint64_t value;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(value));
    return (uint8_t)((value >> 2) & 3u);
}

_Noreturn void arm64_stop(void) {
    for (;;)
        __asm__ volatile("wfe" ::: "memory");
}


