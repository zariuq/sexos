#include "epoch_files.h"

uint64_t sexos_epoch_flush(uint64_t enter)
{
    register uint64_t rax __asm__("rax") = SEXOS_SYSCALL_epoch_flush;
    register uint64_t rdi __asm__("rdi") = enter;
    __asm__ volatile ("syscall" : "+a"(rax), "+D"(rdi) : : "rcx", "r11", "memory", "cc");
    return rax;
}
