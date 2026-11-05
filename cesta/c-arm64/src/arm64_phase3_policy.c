#include "sexos_arm64_boot.h"
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

#define ARM64_PAGE UINT64_C(0x1000)
#define ARM64_BLOCK_2M UINT64_C(0x200000)
#define ARM64_BLOCK_1G UINT64_C(0x40000000)
#define ARM64_KERNEL_BASE UINT64_C(0x40080000)
#define ARM64_UART_BASE UINT64_C(0x09000000)
#define ARM64_USER_CODE UINT64_C(0x00400000)
#define ARM64_USER_CODE2 UINT64_C(0x00430000)
#define ARM64_USER_DATA UINT64_C(0x00410000)
#define ARM64_USER_STACK UINT64_C(0x00420000)
#define ARM64_HHDM UINT64_C(0xffff800000000000)
#define ARM64_USER_UNMAPPED UINT64_C(0x50000000)

static bool aligned_span(Arm64MapContract c, uint64_t alignment) {
    return c.bytes != 0 && (c.va & (alignment - 1u)) == 0 && (c.pa & (alignment - 1u)) == 0 &&
           (c.bytes & (alignment - 1u)) == 0;
}

bool arm64_phase3_map_valid(Arm64MapContract c) {
    switch (c.role) {
    case ARM64_MAP_KERNEL_TEXT:
        return aligned_span(c, ARM64_PAGE) && c.va == c.pa && c.va >= ARM64_KERNEL_BASE && !c.el0 &&
               !c.write && c.execute && !c.device;
    case ARM64_MAP_KERNEL_DATA:
        return aligned_span(c, ARM64_PAGE) && c.va == c.pa && c.va >= ARM64_KERNEL_BASE && !c.el0 &&
               c.write && !c.execute && !c.device;
    case ARM64_MAP_UART:
        return aligned_span(c, ARM64_BLOCK_2M) && c.va == ARM64_UART_BASE &&
               c.pa == ARM64_UART_BASE && c.bytes == ARM64_BLOCK_2M && !c.el0 && c.write &&
               !c.execute && c.device;
    case ARM64_MAP_USER_CODE:
        return aligned_span(c, ARM64_PAGE) && c.va == ARM64_USER_CODE && c.bytes == ARM64_PAGE &&
               c.el0 && !c.write && c.execute && !c.device;
    case ARM64_MAP_USER_CODE2:
        return aligned_span(c, ARM64_PAGE) && c.va == ARM64_USER_CODE2 && c.bytes == ARM64_PAGE &&
               c.el0 && !c.write && c.execute && !c.device;
    case ARM64_MAP_USER_DATA:
        return aligned_span(c, ARM64_PAGE) && c.va == ARM64_USER_DATA && c.bytes == ARM64_PAGE &&
               c.el0 && c.write && !c.execute && !c.device;
    case ARM64_MAP_USER_STACK:
        return aligned_span(c, ARM64_PAGE) && c.va == ARM64_USER_STACK && c.bytes == ARM64_PAGE &&
               c.el0 && c.write && !c.execute && !c.device;
    case ARM64_MAP_HHDM_DEVICE:
        return aligned_span(c, ARM64_BLOCK_1G) && c.va == ARM64_HHDM && c.pa == 0 &&
               c.bytes == ARM64_BLOCK_1G && !c.el0 && c.write && !c.execute && c.device;
    case ARM64_MAP_HHDM_DRAM:
        return aligned_span(c, ARM64_BLOCK_1G) && c.va == ARM64_HHDM + ARM64_BLOCK_1G &&
               c.pa == ARM64_BLOCK_1G && c.bytes == ARM64_BLOCK_1G && !c.el0 && c.write &&
               !c.execute && !c.device;
    }
    return false;
}

Arm64FaultAction arm64_phase3_fault_decide(Arm64FaultObservation o) {
    switch (o.tag) {
    case 1u:
    case 2u:
        return o.ec == UINT8_C(0x24) && o.far == ARM64_KERNEL_BASE ? ARM64_FAULT_SKIP
                                                                   : ARM64_FAULT_HALT;
    case 3u:
        return o.ec == UINT8_C(0x20) && o.far == ARM64_USER_DATA ? ARM64_FAULT_REDIRECT
                                                                 : ARM64_FAULT_HALT;
    case 4u:
        return o.ec == UINT8_C(0x24) && o.far == ARM64_USER_UNMAPPED ? ARM64_FAULT_COMPLETE
                                                                     : ARM64_FAULT_HALT;
    default:
        return ARM64_FAULT_HALT;
    }
}


