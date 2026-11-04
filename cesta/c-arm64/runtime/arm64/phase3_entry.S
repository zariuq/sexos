.section .text.boot, "ax"
.global _start
.type _start, %function
_start:
    mov x20, x0
    mrs x21, CurrentEL
    lsr x21, x21, #2
    and x21, x21, #3
    cmp x21, #2
    b.eq .Lel2_to_el1
    cmp x21, #1
    b.eq .Lin_el1
    b .Lunexpected_el
.Lel2_to_el1:
    mov x9, #(1 << 31)
    msr hcr_el2, x9
    mov x9, #3
    msr cnthctl_el2, x9
    ldr x9, =0x30d00800
    msr sctlr_el1, x9
    msr cpacr_el1, xzr
    ldr x9, =0x3c5
    msr spsr_el2, x9
    adr x9, .Lin_el1
    msr elr_el2, x9
    adrp x9, __boot_stack_top
    add x9, x9, :lo12:__boot_stack_top
    msr sp_el1, x9
    isb
    eret
.Lin_el1:
    adrp x9, __boot_stack_top
    add x9, x9, :lo12:__boot_stack_top
    mov sp, x9
    adrp x10, __bss_start
    add x10, x10, :lo12:__bss_start
    adrp x11, __bss_end
    add x11, x11, :lo12:__bss_end
.Lbss_clear:
    cmp x10, x11
    b.hs .Lbss_done
    str xzr, [x10], #8
    b .Lbss_clear
.Lbss_done:
    mov x0, x20
    mov x1, x21
    bl arm64_phase3_main
.Lhalt:
    wfe
    b .Lhalt
.Lunexpected_el:
    mov x9, #0x09000000
    mov w10, #'E'
    str w10, [x9]
    b .Lhalt
.size _start, .-_start

.section .note.GNU-stack,"",%progbits
