/* The kernel's native boundary in assembly: the Limine request block, the entry point, segment
 * reloading, and the trap entry and exit paths.
 *
 * Every entry from user mode, by syscall or by interrupt, builds one trap frame on the current task's
 * kernel stack, in this layout from the lowest address: r15 r14 r13 r12 r11 r10 r9 r8 rbp rdi rsi rdx
 * rcx rbx rax vector error rip cs rflags rsp ss (22 words, 176 bytes). The handlers return the frame to
 * resume, which may belong to another task, and every exit is the same pop and iretq. */

#ifndef SEXOS_KERNEL_CURRENT_SMP
#define SEXOS_KERNEL_CURRENT_SMP 0
#endif

/* The request block, word for word as the reference kernel declares it (Limine 7 protocol). */
.section .limine_requests_start, "aw"
.balign 8
    .quad 0xf6b8f4b39de7d1ae, 0xfab91a6940fcb9cf, 0x785c6ed015d3e316, 0x181e920a7852b9d9

.section .limine_requests, "aw"
.balign 8
.globl limine_base_revision
limine_base_revision:
    .quad 0xf9562b2d5c95a6c8, 0x6a7b384944536bdc, 6
.globl limine_stack_size_request
limine_stack_size_request:
    .quad 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b, 0x224ef0460a8e8926, 0xe1cb0fc25f46ea3d, 0, 0, 0x100000
.globl limine_framebuffer_request
limine_framebuffer_request:
    .quad 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b, 0x9d5827dcd881dd75, 0xa3148604f6fab11b, 0, 0
.globl limine_memmap_request
limine_memmap_request:
    .quad 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62, 0, 0
.globl limine_hhdm_request
limine_hhdm_request:
    .quad 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b, 0x48dcf1cb8ad2b852, 0x63984e959a98244b, 0, 0
.globl limine_rsdp_request
limine_rsdp_request:
    .quad 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b, 0xc5e77b6b397e7b43, 0x27637845accdcf3c, 0, 0
#if SEXOS_KERNEL_CURRENT_SMP
.globl limine_mp_request
limine_mp_request:
    .quad 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b, 0x95a67b819a1b857e, 0xa0b61b723b6a73e0, 0, 0, 0
#endif
.globl limine_module_request
limine_module_request:
    .quad 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b, 0x3e7e279702be32af, 0xca1c4f3bd1280cee, 0, 0, 0, 0

.section .limine_requests_end, "aw"
.balign 8
    .quad 0xadc0e0531bb10d03, 0x9572709f31764c62

.text

.globl _start
.type _start, @function
_start:
    xor %ebp, %ebp
    and $-16, %rsp
    call kernel_main
1:
    cli
    hlt
    jmp 1b
.size _start, .-_start

/* Leave a blocked or exited task's retained stack before enabling interrupts.
 * This stack is kernel-owned and never part of app/pager reclamation. */
.bss
.balign 16
kernel_idle_stack:
    .skip 16384
kernel_idle_stack_top:
.text
.globl kernel_idle
.type kernel_idle, @function
kernel_idle:
    cli
    lea kernel_idle_stack_top(%rip), %rsp
    xor %ebp, %ebp
    call kernel_idle_loop
    ud2
.size kernel_idle, .-kernel_idle

/* cpu_load_gdt(pointer, code selector, data selector): load the GDT, the data segment registers, and
 * CS through a far return. */
.globl cpu_load_gdt
.type cpu_load_gdt, @function
cpu_load_gdt:
    lgdt (%rdi)
    mov %dx, %ds
    mov %dx, %es
    mov %dx, %fs
    mov %dx, %gs
    mov %dx, %ss
    movzwq %si, %rsi
    lea 1f(%rip), %rax
    pushq %rsi
    pushq %rax
    lretq
1:
    ret
.size cpu_load_gdt, .-cpu_load_gdt

.macro PUSH_GPRS
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

.macro POP_GPRS
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
.endm

/* SYSCALL arrives with interrupts, the trap flag, the direction flag and AC masked (SFMASK), on the
 * user stack. The return address is in rcx and the flags in r11; both are restored as SYSRET would. */
.globl kernel_syscall_entry
.type kernel_syscall_entry, @function
kernel_syscall_entry:
    mov %rsp, kernel_syscall_user_rsp(%rip)
    mov kernel_syscall_stack_top(%rip), %rsp
    pushq $0x23
    pushq kernel_syscall_user_rsp(%rip)
    pushq %r11
    pushq $0x2b
    pushq %rcx
    pushq $0
    pushq $0x80
    PUSH_GPRS
    mov %rsp, %rdi
    cld
    call kernel_syscall
    jmp kernel_resume_rax
.size kernel_syscall_entry, .-kernel_syscall_entry

kernel_trap_common:
    PUSH_GPRS
    testb $1, kernel_smap_enabled(%rip)
    jz 1f
    clac
1:
    mov %rsp, %rdi
    cld
    call kernel_trap
kernel_resume_rax:
    mov %rax, %rsp
    POP_GPRS
    add $16, %rsp
    iretq

/* kernel_resume(frame): resume a saved trap frame; never returns. */
.globl kernel_resume
.type kernel_resume, @function
kernel_resume:
    mov %rdi, %rax
    jmp kernel_resume_rax
.size kernel_resume, .-kernel_resume

.macro ISR_NOERR vector
kernel_isr_\vector:
    pushq $0
    pushq $\vector
    jmp kernel_trap_common
.endm

.macro ISR_ERR vector
kernel_isr_\vector:
    pushq $\vector
    jmp kernel_trap_common
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR 8
ISR_NOERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR 29
ISR_ERR 30
ISR_NOERR 31
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 255

/* Entry points of vectors 0 to 34, then 255. */
.section .rodata
.balign 8
.globl kernel_isr_table
kernel_isr_table:
    .quad kernel_isr_0, kernel_isr_1, kernel_isr_2, kernel_isr_3, kernel_isr_4, kernel_isr_5
    .quad kernel_isr_6, kernel_isr_7, kernel_isr_8, kernel_isr_9, kernel_isr_10, kernel_isr_11
    .quad kernel_isr_12, kernel_isr_13, kernel_isr_14, kernel_isr_15, kernel_isr_16, kernel_isr_17
    .quad kernel_isr_18, kernel_isr_19, kernel_isr_20, kernel_isr_21, kernel_isr_22, kernel_isr_23
    .quad kernel_isr_24, kernel_isr_25, kernel_isr_26, kernel_isr_27, kernel_isr_28, kernel_isr_29
    .quad kernel_isr_30, kernel_isr_31, kernel_isr_32, kernel_isr_33, kernel_isr_34, kernel_isr_255

.section .note.GNU-stack,"",@progbits
