#include "sexos_server.h"

/* Native ABI obligation: syscall 34 returns the kernel tick counter in RAX
 * and clobbers only RCX and R11, as get_ticks in crates/sex-pdx assumes. */
uint64_t sexos_get_ticks(void)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_ticks;
    __asm__ volatile ("syscall" : "+a"(rax) : : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 42 takes the service state in RDI and
 * returns the kernel's answer in RAX, as sys_set_state in crates/sex-pdx. */
uint64_t sexos_set_state(uint64_t state)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_set_state;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(state) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 31 takes a page-aligned size in RDI and
 * returns a frame number, or all ones on failure (expand_heap in sex-rt). */
uint64_t sexos_allocate_memory(uint64_t size)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_allocate_memory;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(size) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 30 maps SIZE bytes of frame PFN and returns
 * the virtual address, or all ones on failure (expand_heap in sex-rt). */
uint64_t sexos_map_memory(uint64_t pfn, uint64_t size)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_map_memory;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(pfn), "S"(size) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 50 grants a memory-lend region to DOMAIN_SLOT
 * of LENGTH bytes at LEND_SLOT and returns the producer virtual address in RAX
 * (sys_grant_mem_lend in crates/sex-pdx). */
uint64_t sexos_grant_mem_lend(uint64_t domain_slot, uint64_t length, uint64_t lend_slot)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_grant_mem_lend;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(domain_slot), "S"(length), "d"(lend_slot) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 51 maps the memory-lend region in CAP_SLOT
 * and returns the consumer virtual address in RAX (sys_map_mem_lend in crates/sex-pdx). */
uint64_t sexos_map_mem_lend(uint64_t cap_slot)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_map_mem_lend;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(cap_slot) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 29 takes the caller's domain in RDI and the
 * reply word in RSI and returns the kernel's status in RAX (pdx_reply). */
uint64_t sexos_reply(uint32_t target_pd, uint64_t value)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_reply;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"((uint64_t)target_pd), "S"(value) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* core::hint::spin_loop in a loop: the pause instruction, forever. */
_Noreturn void sexos_spin_forever(void)
{
    for (;;) __asm__ volatile ("pause");
}

void sexos_spin_loop_hint(void)
{
    __asm__ volatile ("pause");
}
