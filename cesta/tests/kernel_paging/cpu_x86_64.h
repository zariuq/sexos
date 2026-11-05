#ifndef SEXOS_CPU_X86_64_H
#define SEXOS_CPU_X86_64_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
extern unsigned host_invalidations;
static inline void cpu_cpuid(uint32_t a,uint32_t b,uint32_t r[4]){(void)a;(void)b;for(unsigned i=0;i<4;++i)r[i]=0;}
static inline bool cpu_rdseed(uint64_t *out){(void)out;abort();}
static inline void cpu_outb(uint16_t port,uint8_t value){(void)port;(void)value;abort();}
static inline uint8_t cpu_inb(uint16_t port){(void)port;abort();return 0;}
static inline void cpu_outl(uint16_t port,uint32_t value){(void)port;(void)value;abort();}
static inline uint32_t cpu_inl(uint16_t port){(void)port;abort();return 0;}
static inline void cpu_outw(uint16_t port,uint16_t value){(void)port;(void)value;abort();}
static inline uint64_t cpu_read_rsp(void){return 0;}
static inline uint64_t cpu_read_cr3(void){return UINT64_C(0x1000);}
#ifdef HOST_PAGING_EVENTS
void host_invalidate(uint64_t address);
static inline void cpu_invlpg(uint64_t address){host_invalidate(address);}
#else
static inline void cpu_invlpg(uint64_t address){(void)address;++host_invalidations;}
#endif
static inline void cpu_wrpkru(uint32_t mask){(void)mask;abort();}
static inline void cpu_sti_hlt(void){abort();}
static inline void cpu_stac(void){abort();}
static inline void cpu_clac(void){abort();}
/* FPU context switch: the host keeps no FPU state per task. */
static inline void cpu_fxsave(void *area){(void)area;}
static inline void cpu_fxrstor(const void *area){(void)area;}
static inline void cpu_fpu_state_init(void *area){(void)area;}
#endif
