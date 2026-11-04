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
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_halt_unexpected
    .space 0x7c
    b sex_arm_el0_sync
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
    b sex_arm_halt_unexpected
    .space 0x7c

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
    b.eq .Lskip
    cmp w0, #3
    b.eq .Lredirect
    cmp w0, #2
    b.eq .Lcomplete
.Lwarden_halt:
    wfe
    b .Lwarden_halt
.Lskip:
    mrs x16, ELR_EL1
    add x16, x16, #4
    msr ELR_EL1, x16
    b .Lrestore
.Lredirect:
    ldr x16, =0x400030
    msr ELR_EL1, x16
.Lrestore:
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
.Lcomplete:
    add sp, sp, #160
    b .Lwarden_halt

.global sex_arm_el0_enter
.type sex_arm_el0_enter, %function
sex_arm_el0_enter:
    msr ELR_EL1, x0
    msr SP_EL0, x1
    msr SPSR_EL1, xzr
    isb
    eret
.size sex_arm_el0_enter, .-sex_arm_el0_enter

sex_arm_halt_unexpected:
    mov x9, #0
    movk x9, #0x0900, lsl #16
    mov w10, #0x55
    str w10, [x9]
.Lunexpected_halt:
    wfe
    b .Lunexpected_halt

.section .note.GNU-stack,"",%progbits
