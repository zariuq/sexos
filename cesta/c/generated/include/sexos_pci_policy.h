
#ifndef SEXOS_PCI_POLICY_H
#define SEXOS_PCI_POLICY_H
#include <stdint.h>
#include <stdbool.h>
#include "sexos_memory_regions.h"

enum {
    PCI_MEMORY = 2,
    PCI_BUS_MASTER = 4,
    PCI_NIC_BITS = 6,
    PCI_DMA_REGION_ID = 2,
    PCI_DMA_BYTES = 256 * 1024,
    PCI_REGISTERS_END = 0x5408
};

#define PCI_DENIED UINT64_MAX
#define PCI_IO (UINT64_MAX - 1)
#define PCI_EXHAUSTED (UINT64_MAX - 2)

/* Policy records are data. The native adapter must obtain them from the
 * actual holder's typed capability table before any hardware effect. */
typedef struct {
    bool present;
    uint32_t subject;
    uint8_t bus, device, function;
    uint16_t vendor_id, device_id, allowed;
} PciCommandGrant;

typedef struct {
    bool present;
    PciCommandGrant grant;
    uint16_t set, clear;
} PciUpdate;

typedef struct {
    bool present;
    uint8_t bus, device, function;
    uint16_t vendor_id, device_id;
    PciCommandGrant command;
    MemoryRegionCapability bar0;
} PciDeviceCapability;

typedef struct {
    uint64_t status, value0, value1, value2, value3;
} PciReply;

typedef struct {
    PciReply reply;
    uint32_t identity;
    uint16_t before, expected, after;
    bool attempted;
} PciApplyTrace;

PciUpdate pci_policy_authorize(PciCommandGrant grant, uint32_t caller, uint64_t set,
                               uint64_t clear);
PciUpdate pci_policy_held(PciDeviceCapability device, uint32_t caller, uint64_t slot, uint64_t set,
                          uint64_t clear);
bool pci_policy_needs_dma(PciUpdate update);
bool pci_policy_dma(MemoryRegionCapability region);
uint16_t pci_policy_next(PciUpdate update, uint16_t previous);
uint8_t pci_policy_model(uint16_t vendor, uint16_t product);
PciReply pci_policy_apply(PciUpdate update);
PciApplyTrace pci_policy_apply_traced(PciUpdate update);
uint32_t pci_policy_read(PciCommandGrant grant, uint8_t offset);
void pci_policy_write_command(PciCommandGrant grant, uint16_t value);
#endif
