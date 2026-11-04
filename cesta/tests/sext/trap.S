.text
.globl sext_abi_trap
.type sext_abi_trap,@function
sext_abi_trap:
    movq sext_abi_count(%rip),%r11
    cmpq $2,%r11
    jae .Loverflow
    imulq $48,%r11,%r11
    leaq sext_abi_inputs(%rip),%rcx
    addq %r11,%rcx
    movq %rax,0(%rcx)
    movq %rdi,8(%rcx)
    movq %rsi,16(%rcx)
    movq %rdx,24(%rcx)
    movq %r10,32(%rcx)
    movq %r8,40(%rcx)
    incq sext_abi_count(%rip)
    cmpq $32,%rax
    je .Lstatus_only
    cmpq $53,%rax
    je .Lstatus_only
    cmpq $28,%rax
    jne .Lpayload
    movq sext_abi_outputs+8(%rip),%rdi
.Lpayload:
    movq sext_abi_outputs+16(%rip),%rsi
    movq sext_abi_outputs+24(%rip),%rdx
    movq sext_abi_outputs+32(%rip),%r10
    movq sext_abi_outputs+40(%rip),%r8
.Lstatus_only:
    movq sext_abi_outputs+0(%rip),%rax
    movq $0x1234,%rcx
    movq $0x5678,%r11
    ret
.Loverflow:
    ud2
.size sext_abi_trap,.-sext_abi_trap
.section .note.GNU-stack,"",@progbits

