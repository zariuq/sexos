#if !defined(SEXOS_EPOCH_TYPES_H)
#define SEXOS_EPOCH_TYPES_H 1

#include <stdint.h>

#include <stddef.h>

#include <stdbool.h>

#define EPOCH_LOG_REGION_ID 12u

#define EPOCH_LOG_REGION_BYTES (9u * 4096u)

#define EPOCH_JOURNAL_BASE_LBA 4096u

#define EPOCH_JOURNAL_DATA_SECTORS 16384u

#define EPOCH_JOURNAL_META_SECTORS 2u

typedef struct RingEntry {
  uint64_t type_id;
  uint64_t caller;
  uint64_t arg0;
  uint64_t arg1;
  uint64_t arg2;
  uint32_t sender;
  uint32_t caller_task;
  bool from_call;
  uint64_t request_id;
}
RingEntry;

typedef enum KernelEpochEventKind {
  KERNEL_EPOCH_RING_PUSH = 1, KERNEL_EPOCH_RING_POP = 2, KERNEL_EPOCH_INPUT_PUSH = 3, KERNEL_EPOCH_INPUT_POP = 4, KERNEL_EPOCH_PAGE_RECLAIM = 5, KERNEL_EPOCH_PAGE_INSTALL = 6, KERNEL_EPOCH_PAGE_CLEANUP = 7, KERNEL_EPOCH_APP_HANDOVER = 8, KERNEL_EPOCH_APP_ACTIVATION = 9, KERNEL_EPOCH_PCI_COMMAND = 10, KERNEL_EPOCH_REGION_WRITER = 11, KERNEL_EPOCH_DEVICE_RESET = 12
}
KernelEpochEventKind;

typedef struct KernelEpochEvent {
  uint64_t epoch;
  uint32_t subject;
  uint32_t peer;
  uint8_t kind;
  uint8_t scancode;
  uint8_t rights;
  uint8_t key;
  uint8_t source_key;
  uint64_t revision;
  uint64_t address;
  uint64_t token;
  uint64_t frame;
  uint64_t bitmap;
  uint64_t pages;
  uint32_t device;
  uint32_t device_identity;
  uint32_t device_expected_identity;
  uint16_t device_before;
  uint16_t device_expected;
  uint16_t device_after;
  bool device_attempted;
  uint64_t device_status;
  RingEntry message;
}
KernelEpochEvent;

#endif
