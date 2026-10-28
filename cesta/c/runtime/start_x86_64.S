/* Each program names its entry with -DSEXOS_MAIN=...; the first client is the default. */
#ifndef SEXOS_MAIN
#define SEXOS_MAIN kaleidoscope_main
#endif
.text
.globl _start
.type _start, @function
_start:
    xor %ebp, %ebp
    and $-16, %rsp
    call SEXOS_MAIN
1:
    pause
    jmp 1b
.size _start, .-_start
.section .note.GNU-stack,"",@progbits
