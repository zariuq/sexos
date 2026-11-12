#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif
#include "sexos_kernel.h"

enum { KERNEL_NVME_VECTOR = 0x22 };

static RingEntry kernel_device_interrupt_message(uint32_t vector, uint64_t revision) {
    uint32_t owner = kernel.vector_owner[vector];
    uint64_t generation = kernel.device_reset.present && kernel.device_reset.vector == vector &&
                          kernel.device_reset.owner == owner ? kernel.device_reset.generation :
                          vector == KERNEL_NVME_VECTOR ? 1 : 0;
    return (RingEntry){.type_id = 0xFF, .caller = 1, .arg0 = vector, .arg1 = revision,
                       .arg2 = generation};
}

void kernel_device_interrupt_publish(uint32_t vector) {
    if (vector >= 256)
        kernel_halt("device interrupt vector");
    uint64_t revision = kernel.vector_pending[vector];
    if (!revision) {
        if (kernel.vector_revision[vector] == UINT64_MAX)
            kernel_halt("device interrupt revision exhausted");
        revision = ++kernel.vector_revision[vector];
    }
    Domain *d = domain_get(kernel.vector_owner[vector]);
    if (!d) {
        kernel.vector_pending[vector] = 0;
        return;
    }
    if (domain_enqueue(d, kernel_device_interrupt_message(vector, revision)))
        kernel.vector_pending[vector] = 0;
    else
        kernel.vector_pending[vector] = revision;
}

bool kernel_device_interrupt_retry(uint32_t vector) {
    if (vector >= 256 || !kernel.vector_pending[vector])
        return true;
    uint64_t revision = kernel.vector_pending[vector];
    Domain *d = domain_get(kernel.vector_owner[vector]);
    if (!d) {
        kernel.vector_pending[vector] = 0;
        return true;
    }
    if (!domain_enqueue(d, kernel_device_interrupt_message(vector, revision)))
        return false;
    kernel.vector_pending[vector] = 0;
    return true;
}

void kernel_device_interrupt_drain(Domain *d) {
    if (!SEXOS_KERNEL_CURRENT_PDX || !d || domain_get(kernel.current) != d ||
        !kernel_current_task(d))
        return;
    for (uint32_t vector = 0; vector < 256; ++vector)
        if (kernel.vector_owner[vector] == d->id && kernel.vector_pending[vector]) {
            uint64_t revision = kernel.vector_pending[vector];
            if (kernel_device_interrupt_retry(vector))
                klog("[device.interrupt.drain] pd=%u vector=%u revision=%llu heard=%llu acknowledged=%llu current=%u\n",
                     d->id, vector, (unsigned long long)revision,
                     (unsigned long long)kernel.vector_heard_revision[vector],
                     (unsigned long long)kernel.vector_ack_revision[vector], kernel.current);
        }
}
