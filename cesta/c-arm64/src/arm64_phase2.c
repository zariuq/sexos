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

_Noreturn void arm64_phase2_main(uint64_t dtb, uint64_t entered_el) {
    uint8_t current = arm64_current_el();
    bool magic_ok = false;
    arm64_write_str("[sex.arm.entry] current_el=");
    arm64_write_dec(entered_el);
    arm64_write_byte('\n');
    arm64_write_str("[sex.arm.el1] ok=");
    arm64_write_dec(current == 1u);
    arm64_write_byte('\n');
    if (!dtb)
        arm64_write_str("[sex.arm.dtb] x0=0 present=0 note=elf_boot_no_fdt\n");
    else {
        arm64_write_str("[sex.arm.dtb] ptr=0x");
        arm64_write_hex(dtb);
        arm64_write_byte('\n');
        if ((dtb & 7u) == 0) {
            uint32_t raw = *(volatile const uint32_t *)(uintptr_t)dtb;
            magic_ok = __builtin_bswap32(raw) == ARM64_DTB_MAGIC;
        }
        arm64_write_str(magic_ok ? "[sex.arm.dtb] magic=ok\n" : "[sex.arm.dtb] magic=bad\n");
    }
    Arm64BootStatus status =
        arm64_boot_validate((Arm64BootObservation){dtb, (uint8_t)entered_el, current, magic_ok});
    if (status != ARM64_BOOT_OK) {
        arm64_write_str("[sex.arm.phase2.fail] code=");
        arm64_write_dec(status);
        arm64_write_byte('\n');
        arm64_stop();
    }
    arm64_write_str("[sex.arm.uart] pl011=ok\nSEX_ARM_ALIVE\nSEX_ARM_PHASE2_OK\n");
    arm64_stop();
}


