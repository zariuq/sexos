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

#define P5_USER_CODE UINT64_C(0x00430000)
#define P5_USER_STACK_TOP UINT64_C(0x00421000)
extern _Noreturn void sex_arm_el0_enter(uint64_t entry, uint64_t stack);

uint64_t sex_arm_svc(uint64_t frame) {
    volatile uint64_t *words = (volatile uint64_t *)(uintptr_t)frame;
    words[0] = arm64_phase5_svc_result(words[0], words[1]);
    return frame;
}

uint32_t arm64_phase5_warden_live(uint64_t esr, uint64_t far, uint64_t elr) {
    uint64_t tag = arm64_phase3_mailbox(0);
    if (tag != 5u)
        return arm64_phase3_warden_live(esr, far, elr);
    Arm64Phase5Completion completion = {
        (uint32_t)tag,           (uint8_t)((esr >> 26) & 63u), far,
        arm64_phase3_mailbox(2), arm64_phase3_mailbox(3),      arm64_phase3_mailbox(4)};
    arm64_write_str("[sex.arm.el0] done2=1\n");
    if (arm64_phase5_completion_valid(completion)) {
        arm64_write_str("[sex.arm.svc] echo=ok\n[sex.arm.svc] cap_bad=ERR_CAP_INVALID\n");
        arm64_write_str("[sex.arm.svc] cap_good=ok\nSEX_ARM_PHASE5_OK\n");
        return ARM64_FAULT_PHASE6;
    }
    arm64_write_str("[sex.arm.svc] MISMATCH echo=0x");
    arm64_write_hex(completion.echo);
    arm64_write_str(" cap_bad=0x");
    arm64_write_hex(completion.cap_bad);
    arm64_write_str(" cap_good=0x");
    arm64_write_hex(completion.cap_good);
    arm64_write_str("\nSEX_ARM_PHASE5_FAIL\n");
    return ARM64_FAULT_HALT;
}

_Noreturn void arm64_phase5_entry(void) {
    arm64_write_str("[sex.arm.p5] entry=1\n");
    __asm__ volatile("msr CNTP_CTL_EL0, %0" ::"r"(UINT64_C(0)) : "memory");
    arm64_write_str("[sex.arm.timer] off=1\n[sex.arm.svc] test=echo,cap_bad,cap_good\n");
    arm64_write_str("[sex.arm.el0] enter2=1\n");
    sex_arm_el0_enter(P5_USER_CODE, P5_USER_STACK_TOP);
}

_Noreturn void arm64_phase6_entry(void) {
    arm64_write_str("[sex.arm.p6] entry=1\n");
    uint64_t id;
    __asm__ volatile("mrs %0, ID_AA64MMFR3_EL1" : "=r"(id));
    uint64_t tcrx = id & 15u, sctlrx = (id >> 4) & 15u, s1pie = (id >> 8) & 15u,
             s2pie = (id >> 12) & 15u, s1poe = (id >> 16) & 15u;
    arm64_write_str("[sex.arm.poe] id_aa64mmfr3=0x");
    arm64_write_hex(id);
    arm64_write_str("\n[sex.arm.poe] fields=VERIFIED(arm_tfa+qemu_xcheck) behavior=UNPROVEN tcrx=");
    arm64_write_dec(tcrx);
    arm64_write_str(" sctlrx=");
    arm64_write_dec(sctlrx);
    arm64_write_str(" s1pie=");
    arm64_write_dec(s1pie);
    arm64_write_str(" s2pie=");
    arm64_write_dec(s2pie);
    arm64_write_str(" s1poe=");
    arm64_write_dec(s1poe);
    arm64_write_byte('\n');
    arm64_write_str(s1poe ? "[sex.arm.poe] s1poe=present status=UNPROVEN isolation=none\n"
                          : "[sex.arm.poe] isolation=none dev=1\n");
    arm64_write_str("SEX_ARM_POE_TAGGED poindex_default=0 nonzero_tags=0\nSEX_ARM_PHASE6_OK\n");
    arm64_write_str("[sex.arm.p7] deferred=1 reason=runtime_probes_required\n");
    arm64_stop();
}


