#ifndef SEXOS_CPU_X86_64_H
#define SEXOS_CPU_X86_64_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
void cpu_cpuid(uint32_t leaf,uint32_t subleaf,uint32_t result[4]);
bool cpu_rdseed(uint64_t *out);
void cpu_outb(uint16_t port,uint8_t value);
uint8_t cpu_inb(uint16_t port);
static inline void cpu_wrpkru(uint32_t value){(void)value;abort();}
static inline void cpu_sti_hlt(void){abort();}
/* FPU context switch: the host keeps no FPU state per task. */
static inline void cpu_fxsave(void *area){(void)area;}
static inline void cpu_fxrstor(const void *area){(void)area;}
static inline void cpu_fpu_state_init(void *area){(void)area;}
#endif
