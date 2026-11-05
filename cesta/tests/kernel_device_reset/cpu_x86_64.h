#ifndef SEXOS_CPU_X86_64_H
#define SEXOS_CPU_X86_64_H
#include <stdint.h>
static inline void cpu_stac(void) {}
static inline void cpu_clac(void) {}
/* FPU context switch: the host keeps no FPU state per task. */
static inline void cpu_fxsave(void *area){(void)area;}
static inline void cpu_fxrstor(const void *area){(void)area;}
static inline void cpu_fpu_state_init(void *area){(void)area;}
#endif
