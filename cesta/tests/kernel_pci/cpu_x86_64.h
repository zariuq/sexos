#ifndef SEXOS_CPU_X86_64_H
#define SEXOS_CPU_X86_64_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
void cpu_outl(uint16_t port,uint32_t value);
uint32_t cpu_inl(uint16_t port);
void cpu_outw(uint16_t port,uint16_t value);
static inline void cpu_wrpkru(uint32_t value){(void)value;abort();}
static inline void cpu_wrmsr(uint32_t msr,uint64_t value){(void)msr;(void)value;}
static inline void cpu_sti_hlt(void){abort();}
/* FPU context switch: the host keeps no FPU state per task. */
static inline void cpu_fxsave(void *area){(void)area;}
static inline void cpu_fxrstor(const void *area){(void)area;}
static inline void cpu_fpu_state_init(void *area){(void)area;}
#endif
