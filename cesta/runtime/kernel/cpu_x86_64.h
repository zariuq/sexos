/* The kernel's native boundary to the processor: one inline function per instruction the kernel
 * needs, and nothing else. Every other part of the kernel is Cesta. */
#ifndef SEXOS_CPU_X86_64_H
#define SEXOS_CPU_X86_64_H

#include <stdint.h>

static inline void cpu_outb(uint16_t port, uint8_t value) { __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port)); }
static inline uint8_t cpu_inb(uint16_t port) { uint8_t v; __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port)); return v; }
static inline void cpu_outl(uint16_t port, uint32_t value) { __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port)); }
static inline uint32_t cpu_inl(uint16_t port) { uint32_t v; __asm__ volatile ("inl %1, %0" : "=a"(v) : "Nd"(port)); return v; }

static inline uint64_t cpu_rdmsr(uint32_t msr)
{
    uint32_t lo, hi;
    __asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

static inline void cpu_wrmsr(uint32_t msr, uint64_t value)
{
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"((uint32_t)value), "d"((uint32_t)(value >> 32)));
}

static inline uint64_t cpu_read_cr0(void) { uint64_t v; __asm__ volatile ("mov %%cr0, %0" : "=r"(v)); return v; }
static inline uint64_t cpu_read_cr2(void) { uint64_t v; __asm__ volatile ("mov %%cr2, %0" : "=r"(v)); return v; }
static inline uint64_t cpu_read_cr3(void) { uint64_t v; __asm__ volatile ("mov %%cr3, %0" : "=r"(v)); return v; }
static inline uint64_t cpu_read_cr4(void) { uint64_t v; __asm__ volatile ("mov %%cr4, %0" : "=r"(v)); return v; }
static inline void cpu_write_cr0(uint64_t v) { __asm__ volatile ("mov %0, %%cr0" : : "r"(v) : "memory"); }
static inline void cpu_write_cr4(uint64_t v) { __asm__ volatile ("mov %0, %%cr4" : : "r"(v) : "memory"); }

static inline void cpu_invlpg(uint64_t va) { __asm__ volatile ("invlpg (%0)" : : "r"(va) : "memory"); }

/* r[0..3] = eax, ebx, ecx, edx of CPUID leaf/subleaf. */
static inline void cpu_cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *r)
{
    __asm__ volatile ("cpuid" : "=a"(r[0]), "=b"(r[1]), "=c"(r[2]), "=d"(r[3]) : "a"(leaf), "c"(subleaf));
}

static inline uint64_t cpu_xgetbv(uint32_t index)
{
    uint32_t lo, hi;
    __asm__ volatile ("xgetbv" : "=a"(lo), "=d"(hi) : "c"(index));
    return ((uint64_t)hi << 32) | lo;
}

/* Valid only after CR4.PKE is set. */
static inline void cpu_wrpkru(uint32_t value) { __asm__ volatile ("wrpkru" : : "a"(value), "c"(0), "d"(0) : "memory"); }

/* Valid only after CR4.SMAP is set. */
static inline void cpu_stac(void) { __asm__ volatile ("stac" : : : "memory", "cc"); }
static inline void cpu_clac(void) { __asm__ volatile ("clac" : : : "memory", "cc"); }

/* Disable interrupts and return whether they were enabled. */
static inline int cpu_irq_save(void)
{
    uint64_t flags;
    __asm__ volatile ("pushfq; popq %0; cli" : "=r"(flags) : : "memory");
    return (flags & 0x200) != 0;
}
static inline void cpu_irq_restore(int enabled) { if (enabled) __asm__ volatile ("sti" : : : "memory"); }

static inline void cpu_cli(void) { __asm__ volatile ("cli" : : : "memory"); }
static inline void cpu_sti(void) { __asm__ volatile ("sti" : : : "memory"); }
static inline void cpu_hlt(void) { __asm__ volatile ("hlt" : : : "memory"); }
static inline void cpu_pause(void) { __asm__ volatile ("pause"); }
/* sti takes effect after the next instruction, so no interrupt is lost between them. */
static inline void cpu_sti_hlt(void) { __asm__ volatile ("sti; hlt; cli" : : : "memory"); }

static inline void cpu_lidt(const void *pointer) { __asm__ volatile ("lidt (%0)" : : "r"(pointer) : "memory"); }
static inline void cpu_ltr(uint16_t selector) { __asm__ volatile ("ltr %0" : : "r"(selector) : "memory"); }

/* Loads the GDT and reloads every segment register; defined in entry_x86_64.S. */
void cpu_load_gdt(const void *pointer, uint16_t code_selector, uint16_t data_selector);

#endif
