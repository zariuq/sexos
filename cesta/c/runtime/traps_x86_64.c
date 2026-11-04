#include "sexos_client.h"

/* Native ABI obligation: the kernel implements the register-only contract
 * in sexos_contract.toml and preserves the System V callee-saved registers.
 * These traps grant no authority; each slot is validated by the selected kernel. */
SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode,
                         uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_call;
    register uint64_t rsi __asm__("rsi") = opcode;
    register uint64_t rdx __asm__("rdx") = arg0;
    register uint64_t r10 __asm__("r10") = arg1;
    register uint64_t r8 __asm__("r8") = arg2;
    __asm__ volatile ("syscall"
        : "+a"(rax), "+S"(rsi), "+d"(rdx), "+r"(r10), "+r"(r8)
        : "D"(slot)
        : "rcx", "r11", "memory", "cc");
    return (SexosReply){rax, rsi};
}

SexosMessage sexos_listen_once(uint64_t slot)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_listen;
    register uint64_t rsi __asm__("rsi");
    register uint64_t rdx __asm__("rdx");
    register uint64_t r10 __asm__("r10");
    register uint64_t r8 __asm__("r8");
    register uint64_t rdi __asm__("rdi") = slot;
    __asm__ volatile ("syscall"
        : "+a"(rax), "=S"(rsi), "=d"(rdx), "=r"(r10), "=r"(r8), "+D"(rdi)
        :
        : "rcx", "r11", "memory", "cc");
    uint32_t task = 0;
#if defined(SEXOS_PDX_SENDER_TASK) && SEXOS_PDX_SENDER_TASK
    task = rdi <= UINT32_MAX ? (uint32_t)rdi : 0;
#endif
    return (SexosMessage){rax, rdx, r10, r8, (uint32_t)rsi, task};
}

#if defined(SEXOS_PDX_SENDER_TASK) && SEXOS_PDX_SENDER_TASK
void sexos_wait_self(void)
{
    register uint64_t rax __asm__("rax") = UINT64_C(53);
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(UINT64_C(0)) : "rcx", "r11", "memory", "cc");
}
#endif

void sexos_yield(void)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_yield;
    __asm__ volatile ("syscall" : "+a"(rax) : : "rcx", "r11", "memory", "cc");
}
