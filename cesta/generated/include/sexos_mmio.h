#if !defined(SEXOS_CESTA_MMIO_H)
#define SEXOS_CESTA_MMIO_H 1

#include "sexos_server.h"

/*Device-driver syscalls and typed volatile MMIO/DMA accessors for sexdrive, sexusb and sexnet.*/

void *memcpy(void *destination, const void *source, size_t count);

void *memset(void *destination, int value, size_t count);

int memcmp(const void *first, const void *second, size_t count);

#define SEXOS_SLOT_usb_host UINT64_C(8)

#define SEXOS_SLOT_nvme_host UINT64_C(16)

#define SEXOS_SLOT_nic UINT64_C(19)

/*syscall 43: map BAR_INDEX of the PCI device in CAP_SLOT for MAP_SIZE bytes.*/

uint64_t sexos_map_pci_bar(uint64_t cap_slot, uint64_t bar_index, uint64_t map_size);

/*syscall 40: allocate a shared buffer of SIZE bytes for CONSUMER_DOMAIN.*/

uint64_t sexos_alloc_shared_buffer(uint64_t size, uint64_t consumer_domain);

/*syscall 52: query kernel network diagnostic counter SELECTOR.*/

uint64_t sexos_net_diag(uint64_t selector);

void sexdrive_main(void);

void sexusb_main(void);

void sexnet_main(void);

#if defined(SEXOS_HOST)
uint8_t sexos_volatile_read8(const uint8_t *addr);

uint16_t sexos_volatile_read16(const uint16_t *addr);

uint32_t sexos_volatile_read32(const uint32_t *addr);

uint64_t sexos_volatile_read64(const uint64_t *addr);

void sexos_volatile_write8(uint8_t *addr, uint8_t value);

void sexos_volatile_write16(uint16_t *addr, uint16_t value);

void sexos_volatile_write32(uint32_t *addr, uint32_t value);

void sexos_volatile_write64(uint64_t *addr, uint64_t value);

void sexos_compiler_fence_seq_cst(void);

void sexos_sfence(void);

#else
static inline uint8_t sexos_volatile_read8(const uint8_t *addr) {
  return *(const volatile uint8_t *) addr;
}
static inline uint16_t sexos_volatile_read16(const uint16_t *addr) {
  return *(const volatile uint16_t *) addr;
}
static inline uint32_t sexos_volatile_read32(const uint32_t *addr) {
  return *(const volatile uint32_t *) addr;
}
static inline uint64_t sexos_volatile_read64(const uint64_t *addr) {
  return *(const volatile uint64_t *) addr;
}
static inline void sexos_volatile_write8(uint8_t *addr, uint8_t value) {
  *(volatile uint8_t *) addr = value;
}
static inline void sexos_volatile_write16(uint16_t *addr, uint16_t value) {
  *(volatile uint16_t *) addr = value;
}
static inline void sexos_volatile_write32(uint32_t *addr, uint32_t value) {
  *(volatile uint32_t *) addr = value;
}
static inline void sexos_volatile_write64(uint64_t *addr, uint64_t value) {
  *(volatile uint64_t *) addr = value;
}
static inline void sexos_compiler_fence_seq_cst(void) { __asm__ volatile ("" : : : "memory"); }
static inline void sexos_sfence(void) { __asm__ volatile ("sfence" : : : "memory"); }


#endif

#endif
