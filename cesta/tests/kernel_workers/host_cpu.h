#ifndef SEXOS_TEST_KERNEL_WORKERS_CPU_H
#define SEXOS_TEST_KERNEL_WORKERS_CPU_H
#include "sexos_kernel.h"
#undef cpu_wrmsr
#undef cpu_wrpkru
#define cpu_wrmsr(msr, value) host_cpu_wrmsr(msr, value)
#define cpu_wrpkru(value) host_cpu_wrpkru(value)
void host_cpu_wrmsr(uint32_t msr, uint64_t value);
void host_cpu_wrpkru(uint32_t value);
#endif
