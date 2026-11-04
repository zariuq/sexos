.section .text, "ax"
.balign 2048
.global sex_arm_vectors
sex_arm_vectors:
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_el1_sync
    .space 0x7c
    b sex_arm_irq_el1
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_el0_sync
    .space 0x7c
    b sex_arm_irq_el1
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c

#if defined(ARM64_PHASE5) && ARM64_PHASE5
sex_arm_el0_sync:
    sub sp, sp, #272
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
    mrs x9, ELR_EL1
    str x9, [sp, #248]
    mrs x9, SPSR_EL1
    str x9, [sp, #256]
    mrs x9, ESR_EL1
    lsr x10, x9, #26
    and x10, x10, #0x3f
    cmp x10, #0x15
    b.eq .Lsvc
    mov x0, x9
    mrs x1, FAR_EL1
    mrs x2, ELR_EL1
    bl arm64_phase5_warden_live
    cmp w0, #0
    b.eq .Lphase5_skip
    cmp w0, #3
    b.eq .Lphase5_redirect
    cmp w0, #2
    b.eq .Lphase5_phase4
    cmp w0, #4
    b.eq .Lphase5_phase6
.Lwarden_halt:
    wfe
    b .Lwarden_halt
.Lphase5_skip:
    ldr x16, [sp, #248]
    add x16, x16, #4
    str x16, [sp, #248]
    b sex_arm_restore_eret
.Lphase5_redirect:
    ldr x16, =0x400030
    str x16, [sp, #248]
    b sex_arm_restore_eret
.Lphase5_phase4:
    add sp, sp, #272
    ldr x0, =arm64_phase4_entry
    msr ELR_EL1, x0
    mov x1, #0x3c5
    msr SPSR_EL1, x1
    eret
.Lphase5_phase6:
    add sp, sp, #272
    ldr x0, =arm64_phase6_entry
    msr ELR_EL1, x0
    mov x1, #0x3c5
    msr SPSR_EL1, x1
    eret
.Lsvc:
    mov x0, sp
    bl sex_arm_svc
    mov sp, x0
    b sex_arm_restore_eret
#else
sex_arm_el0_sync:
    sub sp, sp, #160
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x30, xzr, [sp, #128]
    mrs x0, ESR_EL1
    mrs x1, FAR_EL1
    mrs x2, ELR_EL1
    bl arm64_phase3_warden_live
    cmp w0, #0
    b.eq .Lel0_skip
    cmp w0, #3
    b.eq .Lel0_redirect
    cmp w0, #2
    b.eq .Lel0_phase4
.Lwarden_halt:
    wfe
    b .Lwarden_halt
.Lel0_skip:
    mrs x16, ELR_EL1
    add x16, x16, #4
    msr ELR_EL1, x16
    b .Lel0_restore
.Lel0_redirect:
    ldr x16, =0x400030
    msr ELR_EL1, x16
.Lel0_restore:
    ldp x0, x1, [sp, #0]
    ldp x2, x3, [sp, #16]
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x30, xzr, [sp, #128]
    add sp, sp, #160
    eret
.Lel0_phase4:
    add sp, sp, #160
    ldr x0, =arm64_phase4_entry
    msr ELR_EL1, x0
    mov x1, #0x3c5
    msr SPSR_EL1, x1
    eret
#endif

.macro SAVE_FRAME
    sub sp, sp, #272
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
    mrs x9, ELR_EL1
    str x9, [sp, #248]
    mrs x9, SPSR_EL1
    str x9, [sp, #256]
.endm

sex_arm_irq_el1:
    SAVE_FRAME
    mrs x19, ICC_IAR1_EL1
    cmp x19, #1023
    b.eq sex_arm_restore_eret
    mov x0, sp
    mov x1, x19
    bl sex_arm_tick
    mov sp, x0
    msr ICC_EOIR1_EL1, x19
    b sex_arm_restore_eret

sex_arm_el1_sync:
    SAVE_FRAME
    mrs x0, ESR_EL1
    mrs x1, FAR_EL1
    mrs x2, ELR_EL1
    mov x3, sp
    bl sex_arm_kill_warden
    cbz x0, .Lwarden_halt
    mov sp, x0
    b sex_arm_restore_eret

.global sex_arm_restore_eret
sex_arm_restore_eret:
    ldr x9, [sp, #256]
    msr SPSR_EL1, x9
    ldr x9, [sp, #248]
    msr ELR_EL1, x9
    ldp x0, x1, [sp, #0]
    ldp x2, x3, [sp, #16]
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldr x30, [sp, #240]
    add sp, sp, #272
    eret

.global sex_arm_first
sex_arm_first:
    mov sp, x0
    b sex_arm_restore_eret

.global sex_arm_el0_enter
sex_arm_el0_enter:
    msr ELR_EL1, x0
    msr SP_EL0, x1
    msr SPSR_EL1, xzr
    isb
    eret

.global sex_arm_ap_entry
sex_arm_ap_entry:
    msr DAIFSet, #0xf
    mrs x21, CurrentEL
    lsr x21, x21, #2
    cmp x21, #2
    b.eq .Lap_el2
    b .Lap_el1
.Lap_el2:
    mov x9, #3
    msr CNTHCTL_EL2, x9
    mov x9, #(1 << 31)
    msr HCR_EL2, x9
    ldr x9, =0x30d00800
    msr SCTLR_EL1, x9
    msr CPACR_EL1, xzr
    mov x9, #0x3c5
    msr SPSR_EL2, x9
    ldr x9, =.Lap_el1
    msr ELR_EL2, x9
    ldr x9, =arm64_ap_stack
    add x9, x9, #16384
    msr SP_EL1, x9
    eret
.Lap_el1:
    ldr x9, =arm64_ap_stack
    add x9, x9, #16384
    mov sp, x9
    bl sex_arm_ap_main
    b .Lwarden_halt

sex_arm_halt_unexpected:
    mov x9, #0
    movk x9, #0x0900, lsl #16
    mov w10, #0x55
    str w10, [x9]
    b .Lwarden_halt

.section .note.GNU-stack,"",%progbits
