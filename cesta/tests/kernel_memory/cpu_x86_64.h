#ifndef SEXOS_CPU_X86_64_H
#define SEXOS_CPU_X86_64_H
#include <stdint.h>
extern unsigned host_invalidations;
static inline uint64_t cpu_read_cr3(void) { return UINT64_C(0x1000); }
static inline uint64_t cpu_read_rsp(void) { return 0; }
static inline void cpu_invlpg(uint64_t address) { (void)address; ++host_invalidations; }
static inline void cpu_wrpkru(uint32_t value) { (void)value; __builtin_abort(); }
static inline void cpu_sti_hlt(void) { __builtin_abort(); }
static inline void cpu_stac(void) { __builtin_abort(); }
static inline void cpu_clac(void) { __builtin_abort(); }
/* FPU context switch: the host keeps no FPU state per task. */
static inline void cpu_fxsave(void *area){(void)area;}
static inline void cpu_fxrstor(const void *area){(void)area;}
static inline void cpu_fpu_state_init(void *area){(void)area;}
#endif
