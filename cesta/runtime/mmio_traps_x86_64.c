#include "sexos_mmio.h"

/* Native ABI obligation: syscall 43 maps BAR_INDEX of the PCI capability in
 * CAP_SLOT for MAP_SIZE bytes and returns the virtual address in RAX, or all
 * ones on failure (sys_map_pci_bar in crates/sex-pdx). */
uint64_t sexos_map_pci_bar(uint64_t cap_slot, uint64_t bar_index, uint64_t map_size)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_map_pci_bar;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(cap_slot), "S"(bar_index), "d"(map_size) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 40 allocates a shared buffer of SIZE bytes
 * for CONSUMER_DOMAIN and returns its virtual address in RAX, or 0 on failure
 * (SYSCALL_ALLOC_SHARED_BUFFER in kernel/src/syscalls/mod.rs). */
uint64_t sexos_alloc_shared_buffer(uint64_t size, uint64_t consumer_domain)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_alloc_shared_buffer;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(size), "S"(consumer_domain) : "rcx", "r11", "memory", "cc");
    return rax;
}

/* Native ABI obligation: syscall 52 queries kernel network diagnostic counter
 * SELECTOR and returns its value in RAX (sys_net_diag in servers/sexnet). */
uint64_t sexos_net_diag(uint64_t selector)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_net_diag;
    __asm__ volatile ("syscall" : "+a"(rax) : "D"(selector) : "rcx", "r11", "memory", "cc");
    return rax;
}

