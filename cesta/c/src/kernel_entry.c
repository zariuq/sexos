#include "sexos_kernel.h"
/*Boot, the processor's tables, and traps. Translated from kernel/src/lib.rs (kernel_init), hal/x86_64.rs, gdt.rs, pku.rs and interrupts.rs.*/
#define MSR_EFER 0xC0000080u
#define MSR_STAR 0xC0000081u
#define MSR_LSTAR 0xC0000082u
#define MSR_SFMASK 0xC0000084u
/*SYSCALL clears IF, TF, DF and AC: system calls run with interrupts off, and a user-set AC cannot lift SMAP for the kernel.*/
#define SYSCALL_FLAG_MASK UINT64_C(0x40700)
#define CR0_MP (UINT64_C(1) << 1)
#define CR0_EM (UINT64_C(1) << 2)
#define CR0_TS (UINT64_C(1) << 3)
#define CR0_WP (UINT64_C(1) << 16)
#define CR4_OSFXSR (UINT64_C(1) << 9)
#define CR4_OSXMMEXCPT (UINT64_C(1) << 10)
#define CR4_OSXSAVE (UINT64_C(1) << 18)
#define CR4_SMEP (UINT64_C(1) << 20)
#define CR4_SMAP (UINT64_C(1) << 21)
#define CR4_PKE (UINT64_C(1) << 22)
#define TIMER_VECTOR 0x20u
#define KEYBOARD_VECTOR 0x21u
#define SPURIOUS_VECTOR 0xFFu
/*The reference's GDT: null, kernel code, kernel data, a 32-bit user code pad (so SYSRET finds user data and user code above it), user data, user code, and the TSS in two entries.*/
static uint64_t gdt[8];
/*The TSS as 26 words: rsp0 at bytes 4 to 11, ist1 at 36 to 43, the I/O map base at 102.*/
static uint32_t tss[26];
static uint64_t idt[512];
static uint64_t double_fault_stack[2048];
/*Descriptor-table pointers: a 16-bit limit and a 64-bit base.*/
static uint16_t gdt_pointer[5];
static uint16_t idt_pointer[5];
void tss_set_rsp0(uint64_t top) {
  tss[1] = (uint32_t) top;
  tss[2] = (uint32_t)(top >> 32);
}
static void set_pointer(uint16_t *pointer, uint16_t limit, uint64_t base) {
  pointer[0] = limit;
  for (unsigned i = 0; i < 4; ++i) pointer[i + 1] = (uint16_t)(base >> 16 * i);
}
static void set_gate(unsigned vector, uint64_t handler, uint64_t ist) {
  idt[2 * vector] = (handler & 0xFFFF) | ((uint64_t) KERNEL_CS << 16 | (ist << 32 | ((uint64_t) 0x8E << 40 | (handler >> 16 & 0xFFFF) << 48)));
  idt[2 * vector + 1] = handler >> 32;
}
static void cpu_tables_init(void) {
  gdt[0] = 0;
  gdt[1] = 0x00af9b000000ffffULL;
  gdt[2] = 0x00cf93000000ffffULL;
  gdt[3] = 0x00cffb000000ffffULL;
  gdt[4] = 0x00cff3000000ffffULL;
  gdt[5] = 0x00affb000000ffffULL;
  memset(tss, 0, sizeof tss);
  tss[25] = 104u << 16;
  uint64_t ist = ((uint64_t)(uintptr_t) double_fault_stack + sizeof double_fault_stack) & ~(uint64_t) 15;
  tss[9] = (uint32_t) ist;
  tss[10] = (uint32_t)(ist >> 32);
  uint64_t base = (uint64_t)(uintptr_t) tss;
  uint64_t limit = sizeof tss - 1;
  gdt[6] = (limit & 0xFFFF) | ((base & 0xFFFFFF) << 16 | ((uint64_t) 0x89 << 40 | ((limit >> 16 & 0xF) << 48 | (base >> 24 & 0xFF) << 56)));
  gdt[7] = base >> 32;
  set_pointer(gdt_pointer, (uint16_t)(sizeof gdt - 1), (uint64_t)(uintptr_t) gdt);
  cpu_load_gdt(gdt_pointer, KERNEL_CS, KERNEL_DS);
  cpu_ltr(KERNEL_TSS_SELECTOR);
  for (unsigned v = 0; v < 35; ++v) set_gate(v, kernel_isr_table[v], v == 8 ? 1 : 0);
  set_gate(SPURIOUS_VECTOR, kernel_isr_table[35], 0);
  set_pointer(idt_pointer, (uint16_t)(sizeof idt - 1), (uint64_t)(uintptr_t) idt);
  cpu_lidt(idt_pointer);
  cpu_wrmsr(MSR_EFER, cpu_rdmsr(MSR_EFER) | 1);
  cpu_wrmsr(MSR_STAR, (uint64_t) 0x1B << 48 | (uint64_t) KERNEL_CS << 32);
  cpu_wrmsr(MSR_LSTAR, (uint64_t)(uintptr_t) kernel_syscall_entry);
  cpu_wrmsr(MSR_SFMASK, SYSCALL_FLAG_MASK);
  klog("X86Hal: GDT/TSS, IDT and SYSCALL ready (interrupts masked during system calls)\n");
}
/*Protection keys as the reference enables them, plus what it leaves off: SMEP (the kernel never executes user pages), SMAP (the kernel touches user pages only in its checked copies) and write protection of read-only pages in kernel mode.*/
static void features_init(void) {
  uint32_t r[4];
  cpu_cpuid(0, 0, r);
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  if (r[0] >= 7) {
    cpu_cpuid(7, 0, r);
    ebx = r[1];
    ecx = r[2];
  }
  if (SEXOS_KERNEL_CURRENT_FPU) {
    uint32_t edx1 = 0;
    cpu_cpuid(1, 0, r);
    edx1 = r[3];
    if ((edx1 & (1u << 24 | (1u << 25 | 1u << 26))) != (1u << 24 | (1u << 25 | 1u << 26))) kernel_halt("FXSAVE, SSE and SSE2 required for user contexts");
  }
  kernel.smep = (ebx & 1u << 7) != 0;
  kernel.smap = (ebx & 1u << 20) != 0;
  kernel.pku = (ecx & 1u << 3) != 0;
  if (!kernel.pku) kernel_halt("protection keys required for domain isolation");
  uint64_t cr4 = cpu_read_cr4();
  if (kernel.smep) cr4 |= CR4_SMEP;
  if (kernel.smap) cr4 |= CR4_SMAP;
  if (kernel.pku) cr4 |= CR4_PKE;
  if (SEXOS_KERNEL_CURRENT_FPU) cr4 |= CR4_OSFXSR | CR4_OSXMMEXCPT;
  cpu_write_cr4(cr4);
  cpu_write_cr0(SEXOS_KERNEL_CURRENT_FPU ? (cpu_read_cr0() | (CR0_MP | CR0_WP)) & ~(CR0_EM | CR0_TS) : cpu_read_cr0() | CR0_WP);
  kernel_smap_enabled = kernel.smap ? 1 : 0;
  cpu_wrpkru(0);
  klog("PKU: Protection Keys enabled in CR4.\n");
/*XRSTOR can load the key register only when XCR0 enables its state component (bit 9).*/
  if ((cr4 & CR4_OSXSAVE) != 0) if ((cpu_xgetbv(0) & (uint64_t) 1 << 9) != 0) kernel_halt("XCR0 lets XRSTOR load the key register");
  klog("kernel: SMEP=%u SMAP=%u PKU=%u WP=1\n", (unsigned) kernel.smep, (unsigned) kernel.smap, (unsigned) kernel.pku);
}
_Noreturn void kernel_halt(const char *reason) {
  klog("KERNEL PANIC: %s\n", reason);
  while (1) {
    cpu_cli();
    cpu_hlt();
  }
}
/*The current profile forwards eligible user data faults to the held pager; all other user faults terminate their domain. The frozen Rust reference halted the whole kernel on a protection fault or key violation.*/
static TrapFrame *fault_user(TrapFrame *frame) {
  uint64_t address = frame->vector == 14 ? cpu_read_cr2() : 0;
  if (kernel_current_record != NULL) {
    *kernel_record_storage(kernel_current_record)->frame = *frame;
#if SEXOS_KERNEL_CURRENT_FPU
kernel_record_fpu_save();

#endif
    return kernel_record_finish(frame, true);
  }
  Domain *faulted = domain_get(kernel.current);
  if (SEXOS_KERNEL_CURRENT_PDX && frame->vector == 14) klog("[paging.fault.trap] pd=%u addr=%#llx rip=%#llx error=%#llx eligible=%u\n", kernel.current, (unsigned long long) address, (unsigned long long) frame->rip, (unsigned long long) frame->error, (unsigned) pager_fault_eligible(address, frame->error, frame->cs));
  if (SEXOS_KERNEL_CURRENT_PDX && (faulted != NULL && (frame->vector == 14 && kernel_pager_forward(faulted, frame, address)))) return kernel_dispatch_blocked();
  klog("fault.kill pd=%u vector=%llu err=%#llx rip=%#llx addr=%#llx\n", kernel.current, (unsigned long long) frame->vector, (unsigned long long) frame->error, (unsigned long long) frame->rip, (unsigned long long) address);
  Domain *d = domain_get(kernel.current);
  if (d != NULL) {
    kernel_worker_fault(d);
    d->task_state = TASK_EXITED;
    kernel_worker_end(d);
    if (!kernel_region_writer_recover_dead(d)) kernel_halt("dead writer recovery invariant");
    if (!kernel_device_reset_owner_dead(d)) kernel_halt("dead device reset custody invariant");
  }
  return kernel_worker_schedule(frame, false);
}
TrapFrame *kernel_trap(TrapFrame *frame) {
  if (kernel.pku) cpu_wrpkru(0);
  uint64_t vector = frame->vector;
  bool from_user = (frame->cs & 3) == 3;
  if (SEXOS_KERNEL_CURRENT_PDX && from_user) {
    Domain *owner = domain_get(kernel.current);
    if (owner != NULL) if (!kernel_worker_entry(frame)) owner->frame = frame;
#if SEXOS_KERNEL_CURRENT_FPU
kernel_fpu_save_current();

#endif
  }
  if (SEXOS_KERNEL_CURRENT_PDX) kernel_task_poll();
  if (vector == TIMER_VECTOR) {
    kernel.ticks += 1;
    lapic_eoi();
    if (!kernel.scheduler_running) return frame;
    if (from_user) return schedule_tick(frame);
    if (kernel.current == 0) return kernel_worker_schedule(frame, false);
    return frame;
  }
  if (vector == KEYBOARD_VECTOR) {
    uint8_t scancode = 0;
    if (keyboard_read(&scancode)) input_push(scancode);
    lapic_eoi();
    return resume_frame(frame);
  }
  if (vector == SPURIOUS_VECTOR) return resume_frame(frame);
  if (vector < 32) {
    if (from_user) return fault_user(frame);
    klog("KERNEL PANIC: exception vector=%llu error=%#llx rip=%#llx cr2=%#llx\n", (unsigned long long) vector, (unsigned long long) frame->error, (unsigned long long) frame->rip, (unsigned long long) cpu_read_cr2());
    kernel_halt("kernel-mode exception");
  }
/*A routed device interrupt: the owning domain hears a hardware-interrupt message, which listen reports with type 0xFF as the reference does.*/
  kernel_device_interrupt_publish(vector);
  lapic_eoi();
  return resume_frame(frame);
}
void kernel_main(void) {
  kernel_serial_init();
  const LimineHhdmResponse *hhdm = limine_hhdm_request.response;
  if (hhdm == NULL) kernel_halt("FATAL: Limine HHDM missing");
  hhdm_offset = hhdm->offset;
  features_init();
  cpu_tables_init();
  const LimineMemmapResponse *map = limine_memmap_request.response;
  if (map == NULL) kernel_halt("FATAL: Limine memmap missing");
  memory_init(map);
  const LimineRsdpResponse *rsdp = limine_rsdp_request.response;
  if (rsdp != NULL) {
    uint64_t address = rsdp->address;
    if (address >= hhdm_offset) address -= hhdm_offset;
    klog("kernel: RSDP Physical Address: %#llx\n", (unsigned long long) address);
    platform_init(address);
  } else klog("X86Hal: WARNING - RSDP not found, skipping APIC/Timer.\n");
  keyboard_init();
  kernel_workers_boot();
  domains_boot();
#if SEXOS_KERNEL_CURRENT_SMP
if (!kernel_smp_park_boot()) kernel_halt("SMP park/quiescence failed");

#endif
  if (SEXOS_KERNEL_CURRENT_PDX) kernel_current_boot_audit();
  if (SEXOS_KERNEL_BOOT_AUDIT) kernel_idle();
  cpu_cli();
  kernel.scheduler_running = true;
  uint32_t next = run_pop();
  if (next == 0) {
    klog("scheduler: no runnable domain\n");
    kernel_idle();
  }
  kernel_resume(switch_to(next));
}
