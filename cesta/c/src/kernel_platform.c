#include "sexos_kernel.h"
/*The platform: ACPI's interrupt table, the local and I/O APICs, the PIT-calibrated local APIC timer, the PS/2 keyboard, and PCI configuration space. Translated from kernel/src/apic.rs, keyboard.rs, hal/pci.rs and drivers/pci.rs; the reference parses ACPI with the acpi crate, this kernel reads only the tables it uses, by their layouts in the ACPI specification.*/
#define MAX_IOAPICS 4u
#define LAPIC_ID 0x020u
#define LAPIC_EOI 0x0B0u
#define LAPIC_SVR 0x0F0u
#define LAPIC_LVT_TIMER 0x320u
#define LAPIC_TIMER_INITIAL 0x380u
#define LAPIC_TIMER_CURRENT 0x390u
#define LAPIC_TIMER_DIVIDE 0x3E0u
#define TIMER_VECTOR 0x20u
#define KEYBOARD_VECTOR 0x21u
typedef struct IoApic {
  uint8_t id;
  uint32_t address;
  uint32_t gsi_base;
}
IoApic;
typedef struct SourceOverride {
  bool present;
  uint8_t isa_source;
  bool active_low;
  bool level;
}
SourceOverride;
typedef struct Platform {
  uint64_t lapic;
  IoApic ioapics[MAX_IOAPICS];
  uint32_t ioapic_count;
  SourceOverride overrides[256];
}
Platform;
static Platform platform;
static uint32_t mmio_read32(uint64_t address) {
  return *(volatile uint32_t *)(uintptr_t) address;
}
static void mmio_write32(uint64_t address, uint32_t value) {
  *(volatile uint32_t *)(uintptr_t) address = value;
}
static uint32_t lapic_read(uint32_t reg) {
  return mmio_read32(platform.lapic + reg);
}
static void lapic_write(uint32_t reg, uint32_t value) {
  mmio_write32(platform.lapic + reg, value);
}
void lapic_eoi(void) {
  if (platform.lapic != 0) lapic_write(LAPIC_EOI, 0);
  else {
    cpu_outb(0x20, 0x20);
    cpu_outb(0xA0, 0x20);
  }
}
static uint8_t read8(uint64_t phys) {
  return *(const uint8_t *) phys_to_virt(phys);
}
static uint16_t read16(uint64_t phys) {
  return (uint16_t)(read8(phys) | (uint16_t) read8(phys + 1) << 8);
}
static uint32_t read32(uint64_t phys) {
  return read16(phys) | (uint32_t) read16(phys + 2) << 16;
}
static uint64_t read64(uint64_t phys) {
  return read32(phys) | (uint64_t) read32(phys + 4) << 32;
}
static bool signature_is(uint64_t phys, const char *signature) {
  for (unsigned i = 0; i < 4; ++i) if (read8(phys + i) != (uint8_t) signature[i]) return false;
  return true;
}
/*The MADT, through the XSDT when the RSDP has revision 2 or later, otherwise through the RSDT.*/
static uint64_t find_madt(uint64_t rsdp) {
  bool extended = read8(rsdp + 15) >= 2;
  uint64_t root = extended ? read64(rsdp + 24) : read32(rsdp + 16);
  if (root == 0) return 0;
  uint32_t length = read32(root + 4);
  uint32_t width = extended ? 8 : 4;
  for (uint32_t offset = 36; offset + width <= length; offset += width) {
    uint64_t table = extended ? read64(root + offset) : read32(root + offset);
    if (table != 0 && signature_is(table, "APIC")) return table;
  }
  return 0;
}
/*LAPIC timer calibration against PIT channel 2, as apic.rs::init_timer does it: divide by 16, count down from all ones while the PIT counts 10 ms, then program the measured count per millisecond as the periodic interval. A failed calibration falls back to 1,000,000.*/
static void timer_init(void) {
  lapic_write(LAPIC_TIMER_DIVIDE, 0x3);
  uint32_t pit_count = (uint32_t)(1193182 * 10 / 1000);
  cpu_cli();
  lapic_write(LAPIC_LVT_TIMER, 0x10000 | TIMER_VECTOR);
  lapic_write(LAPIC_TIMER_INITIAL, 0xFFFFFFFFu);
  lapic_write(LAPIC_LVT_TIMER, TIMER_VECTOR);
  cpu_outb(0x43, 0xB0);
  cpu_outb(0x42, (uint8_t)(pit_count & 0xFF));
  cpu_outb(0x42, (uint8_t)(pit_count >> 8 & 0xFF));
  bool timed_out = false;
  for (uint32_t i = 0; i < 1000000; ++i) {
    if ((cpu_inb(0x61) & 0x20) != 0) break;
    cpu_pause();
  }
  if ((cpu_inb(0x61) & 0x20) == 0) timed_out = true;
  uint32_t remaining = lapic_read(LAPIC_TIMER_CURRENT);
  uint32_t elapsed = timed_out || remaining >= 0xFFFFFFFEu ? 0 : 0xFFFFFFFFu - remaining + 1;
  uint32_t ticks_per_ms = elapsed / 10;
  cpu_sti();
  if (ticks_per_ms == 0 || ticks_per_ms > 10000000) {
    klog("APIC: LAPIC timer calibration failed (elapsed=%u pit_count=%u); using fallback=1000000\n", elapsed, pit_count);
    ticks_per_ms = 1000000;
  } else klog("APIC: LAPIC timer calibrated: %u ticks/ms (elapsed=%u over 10ms)\n", ticks_per_ms, elapsed);
  lapic_write(LAPIC_LVT_TIMER, 0x20000 | TIMER_VECTOR);
  lapic_write(LAPIC_TIMER_INITIAL, ticks_per_ms);
  klog("APIC: LAPIC Timer initialized at Vector 0x20.\n");
}
/*Route an ISA interrupt to a vector on the boot processor, with the polarity and trigger of its source override. As in the reference, the ISA line is taken as its GSI.*/
static void ioapic_map(uint32_t gsi, uint8_t vector) {
  if (platform.ioapic_count == 0) return;
  const IoApic *io = &platform.ioapics[0];
  for (uint32_t i = 0; i < platform.ioapic_count; ++i) {
    const IoApic *candidate = &platform.ioapics[i];
    if (gsi >= candidate->gsi_base && gsi < candidate->gsi_base + 24) {
      io = candidate;
      break;
    }
  }
  uint64_t base = hhdm_offset + io->address;
  uint32_t index = 0x10 + (gsi - io->gsi_base) * 2;
  uint32_t low = vector;
  if (gsi < 256 && platform.overrides[gsi].present) {
    if (platform.overrides[gsi].active_low) low |= 1 << 13;
    if (platform.overrides[gsi].level) low |= 1 << 15;
  }
  mmio_write32(base, index);
  mmio_write32(base + 0x10, low);
  mmio_write32(base, index + 1);
  mmio_write32(base + 0x10, 0);
  klog("APIC: Mapped GSI %u (IOAPIC %u) to Vector %u (Dest LAPIC 0, low=%#x)\n", gsi, io->id, vector, low);
}
bool platform_init(uint64_t rsdp) {
/*Mask both legacy PICs: interrupts come through the APICs.*/
  cpu_outb(0x21, 0xFF);
  cpu_outb(0xA1, 0xFF);
  uint64_t madt = find_madt(rsdp);
  if (madt == 0) {
    klog("APIC: MADT not found\n");
    return false;
  }
  uint64_t lapic_phys = read32(madt + 36);
  uint32_t length = read32(madt + 4);
  uint32_t offset = 44;
  while (offset + 2 < length) {
    uint8_t type = read8(madt + offset);
    uint8_t entry_length = read8(madt + offset + 1);
    if (entry_length < 2) break;
    uint64_t e = madt + offset;
    if (type == 1 && platform.ioapic_count < MAX_IOAPICS) {
      platform.ioapics[platform.ioapic_count] = (IoApic) {
        .id = read8(e + 2),.address = read32(e + 4),.gsi_base = read32(e + 8)
      };
      platform.ioapic_count += 1;
    }
    if (type == 2) {
      uint32_t gsi = read32(e + 4);
      uint16_t flags = read16(e + 8);
      if (gsi < 256) platform.overrides[gsi] = (SourceOverride) {
        .present = true,.isa_source = read8(e + 3),.active_low = (flags & 3) == 3,.level = (flags >> 2 & 3) == 3
      };
    }
    if (type == 5) lapic_phys = read64(e + 4);
    offset += entry_length;
  }
  platform.lapic = hhdm_offset + lapic_phys;
  klog("APIC: Found LAPIC at %#llx\n", (unsigned long long) lapic_phys);
  for (uint32_t i = 0; i < platform.ioapic_count; ++i) klog("APIC: Found I/O APIC %u at %#x\n", platform.ioapics[i].id, platform.ioapics[i].address);
  for (uint32_t gsi = 0; gsi < 256; ++gsi) if (platform.overrides[gsi].present) klog("APIC: ISO GSI %u <- ISA %u (active_low=%s, level=%s)\n", gsi, platform.overrides[gsi].isa_source, platform.overrides[gsi].active_low ? "true" : "false", platform.overrides[gsi].level ? "true" : "false");
/*Software-enable the local APIC with spurious vector 0xFF.*/
  lapic_write(LAPIC_SVR, lapic_read(LAPIC_SVR) | 0x1FF);
  timer_init();
  ioapic_map(1, KEYBOARD_VECTOR);
  return true;
}
/*The i8042 controller as keyboard.rs sets it up: translation and the keyboard interrupt on, the keyboard clock enabled, scanning enabled. Bytes arriving before setup completes are drained and dropped by the interrupt handler.*/
void keyboard_init(void) {
  if (cpu_inb(0x64) == 0xFF) return;
  cpu_outb(0x64, 0x20);
  uint8_t config = cpu_inb(0x60);
  if (config == 0xFF) return;
  config |= 0x41;
  config &= (uint8_t) ~0x10;
  cpu_outb(0x64, 0x60);
  cpu_outb(0x60, config);
  cpu_outb(0x64, 0xAE);
  cpu_outb(0x60, 0xF4);
  cpu_inb(0x60);
  kernel.keyboard_ready = true;
}
/*Read port 0x60 on every keyboard interrupt, so the controller can raise the next one; report the byte only once the keyboard is ready.*/
bool keyboard_read(uint8_t *scancode) {
  uint8_t byte = cpu_inb(0x60);
  if (!kernel.keyboard_ready) return false;
  *scancode = byte;
  return true;
}
static uint32_t pci_address(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
  return 0x80000000u | ((uint32_t) bus << 16 | ((uint32_t) dev << 11 | ((uint32_t) func << 8 | (offset & 0xFC))));
}
uint32_t pci_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
  cpu_outl(0xCF8, pci_address(bus, dev, func, offset));
  return cpu_inl(0xCFC);
}
void pci_write(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value) {
  cpu_outl(0xCF8, pci_address(bus, dev, func, offset));
  cpu_outl(0xCFC, value);
}
/*A BAR's base: I/O BARs drop their two low bits, 64-bit memory BARs join the next register.*/
uint64_t pci_bar(uint8_t bus, uint8_t dev, uint8_t func, uint8_t index) {
  uint8_t offset = (uint8_t)(0x10 + index * 4);
  uint32_t bar = pci_read(bus, dev, func, offset);
  if ((bar & 1) != 0) return bar & 0xFFFFFFFCu;
  if ((bar >> 1 & 3) == 2) return (uint64_t) pci_read(bus, dev, func, (uint8_t)(offset + 4)) << 32 | (bar & 0xFFFFFFF0u);
  return bar & 0xFFFFFFF0u;
}
/*Every present function, bus by bus, as hal/pci.rs::enumerate_bus lists them.*/
uint32_t pci_scan(PciFunction *out, uint32_t max) {
  uint32_t count = 0;
  for (uint32_t bus = 0; bus < 256; ++bus) for (uint32_t dev = 0; dev < 32; ++dev) {
    if ((pci_read((uint8_t) bus, (uint8_t) dev, 0, 0) & 0xFFFF) == 0xFFFF) continue;
    for (uint32_t func = 0; func < 8; ++func) {
      uint32_t id = pci_read((uint8_t) bus, (uint8_t) dev, (uint8_t) func, 0);
      if ((id & 0xFFFF) == 0xFFFF || count >= max) continue;
      uint32_t class_rev = pci_read((uint8_t) bus, (uint8_t) dev, (uint8_t) func, 8);
      out[count] = (PciFunction) {
        .bus = (uint8_t) bus,.dev = (uint8_t) dev,.func = (uint8_t) func,.vendor_id = (uint16_t)(id & 0xFFFF),.device_id = (uint16_t)(id >> 16),.class_id = (uint8_t)(class_rev >> 24),.subclass_id = (uint8_t)(class_rev >> 16),.prog_if = (uint8_t)(class_rev >> 8)
      };
      count += 1;
    }
  }
  return count;
}
