#ifndef SEXOS_CPU_X86_64_H
#define SEXOS_CPU_X86_64_H
#include <stdint.h>
#include <stdlib.h>
extern unsigned host_invalidations;
static inline uint64_t cpu_read_cr3(void) { return UINT64_C(0x1000); }
static inline void cpu_invlpg(uint64_t address) { (void)address; ++host_invalidations; }
static inline void cpu_stac(void) { abort(); }
static inline void cpu_clac(void) { abort(); }
#endif
