#include "sexos_rpc.h"

uint64_t sexos_rpc_register(void)
{
    register uint64_t rax __asm__("rax") = SEXOS_RPC_register;
    __asm__ volatile ("syscall" : "+a"(rax) : : "rcx", "r11", "memory", "cc");
    return rax;
}

SexosReply sexos_rpc_begin(uint64_t slot, uint64_t opcode, uint64_t arg0,
                            uint64_t arg1, uint64_t arg2)
{
    register uint64_t rax __asm__("rax") = SEXOS_RPC_call;
    register uint64_t rsi __asm__("rsi") = opcode;
    register uint64_t rdx __asm__("rdx") = arg0;
    register uint64_t r10 __asm__("r10") = arg1;
    register uint64_t r8 __asm__("r8") = arg2;
    __asm__ volatile ("syscall"
        : "+a"(rax), "+S"(rsi), "+d"(rdx), "+r"(r10), "+r"(r8)
        : "D"(slot) : "rcx", "r11", "memory", "cc");
    return (SexosReply){rax, rsi};
}

SexosReply sexos_rpc_poll(uint64_t id)
{
    register uint64_t rax __asm__("rax") = SEXOS_RPC_poll;
    register uint64_t rsi __asm__("rsi");
    __asm__ volatile ("syscall" : "+a"(rax), "=S"(rsi) : "D"(id)
                      : "rcx", "r11", "memory", "cc");
    return (SexosReply){rax, rsi};
}

uint64_t sexos_rpc_cancel(uint64_t id)
{
    register uint64_t rax __asm__("rax") = SEXOS_RPC_cancel;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(id) : "rcx", "r11", "memory", "cc");
    return rax;
}

SexosRequest sexos_rpc_listen_once(uint64_t slot)
{
    register uint64_t rax __asm__("rax") = SEXOS_RPC_listen;
    register uint64_t rsi __asm__("rsi");
    register uint64_t rdx __asm__("rdx");
    register uint64_t r10 __asm__("r10");
    register uint64_t r8 __asm__("r8");
    register uint64_t r9 __asm__("r9");
    register uint64_t rdi __asm__("rdi") = slot;
    __asm__ volatile ("syscall"
        : "+a"(rax), "=S"(rsi), "=d"(rdx), "=r"(r10), "=r"(r8), "=r"(r9), "+D"(rdi)
        : : "rcx", "r11", "memory", "cc");
    uint32_t task = rdi <= UINT32_MAX ? (uint32_t)rdi : 0;
    return (SexosRequest){{rax, rdx, r10, r8, (uint32_t)rsi, task}, r9};
}

uint64_t sexos_rpc_reply(uint32_t caller, uint64_t id, uint64_t value)
{
    register uint64_t rax __asm__("rax") = SEXOS_RPC_reply;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"((uint64_t)caller), "S"(id), "d"(value)
                      : "rcx", "r11", "memory", "cc");
    return rax;
}
