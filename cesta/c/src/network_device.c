#include "network_service.h"
static uint32_t reg_read(NetworkDevice *d, unsigned offset) {
  return sexos_volatile_read32((const uint32_t *)(uintptr_t)(d->bar + offset));
}
static void reg_write(NetworkDevice *d, unsigned offset, uint32_t value) {
  sexos_volatile_write32((uint32_t *)(uintptr_t)(d->bar + offset), value);
}
static bool page(uint64_t *physical, uint64_t *virtual_address) {
  *physical = sexos_allocate_memory(4096);
  if (*physical == 0 || *physical == UINT64_MAX) return false;
  *virtual_address = sexos_map_memory(*physical, 4096);
  if (*virtual_address == 0 || *virtual_address == UINT64_MAX) return false;
  memset((void *)(uintptr_t) *virtual_address, 0, 4096);
  return true;
}
bool network_device_open(NetworkDevice *d) {
  memset(d, 0, sizeof *d);
  d->failure = NET_DEVICE_ERROR;
  d->bar = sexos_map_pci_bar(SEXOS_SLOT_nic, 0, 65536);
  if (d->bar == 0 || d->bar == UINT64_MAX) return false;
  uint32_t low = reg_read(d, 0x5400), high = reg_read(d, 0x5404);
  reg_write(d, 0x00D8, UINT32_MAX);
  reg_write(d, 0x0100, 0);
  reg_write(d, 0x0400, 0);
  reg_write(d, 0x0000, reg_read(d, 0x0000) | 1u << 26);
  unsigned reset;
  for (reset = 0; reset < 65536; ++reset) {
    if (!(reg_read(d, 0x0000) & 1u << 26)) break;
    sexos_yield();
  }
  if (reset == 65536) return false;
  for (unsigned i = 0; i < 4; ++i) d->mac[i] = (uint8_t)(low >> 8 * i);
  d->mac[4] = (uint8_t) high;
  d->mac[5] = (uint8_t)(high >> 8);
  reg_write(d, 0x5400, low);
  reg_write(d, 0x5404, high | 1u << 31);
  reg_write(d, 0x00D8, UINT32_MAX);
  reg_write(d, 0x0000, reg_read(d, 0x0000) | 1u << 6);
  uint64_t rx_phys, tx_phys;
  d->failure = NET_RESOURCE_ERROR;
  if (!page(&rx_phys, &d->rx_desc) || !page(&tx_phys, &d->tx_desc) || !page(&d->tx_phys, &d->tx_page)) return false;
  for (unsigned i = 0; i < 8; ++i) {
    uint64_t physical;
    if (!page(&physical, &d->rx_page[i])) return false;
    sexos_volatile_write64((uint64_t *)(uintptr_t)(d->rx_desc + i * 16), physical);
  }
  reg_write(d, 0x2800, (uint32_t) rx_phys);
  reg_write(d, 0x2804, (uint32_t)(rx_phys >> 32));
  reg_write(d, 0x2808, 128);
  reg_write(d, 0x2810, 0);
  reg_write(d, 0x2818, 7);
  reg_write(d, 0x2828, 0x02080404);
  reg_write(d, 0x3800, (uint32_t) tx_phys);
  reg_write(d, 0x3804, (uint32_t)(tx_phys >> 32));
  reg_write(d, 0x3808, 128);
  reg_write(d, 0x3810, 0);
  reg_write(d, 0x3818, 0);
  reg_write(d, 0x3828, 0x02080404);
  reg_write(d, 0x0410, 0x0060200A);
  sexos_sfence();
  reg_write(d, 0x0400, 1u << 1 | 0x10u << 4 | 0x40u << 12);
  reg_write(d, 0x0100, 1u << 1 | 1u << 3 | 1u << 4 | 1u << 15 | 1u << 26);
  d->failure = NET_OK;
  sexos_line("[network.device.ready] rx=8 tx=8 exclusive_tx_buffer=1");
  return true;
}
bool network_device_submit(NetworkDevice *d, cesta_read_bytes frame) {
  if (d->failed || d->tx_busy || frame.len > 1514 || frame.len < 14) return false;
  size_t count = frame.len < 60 ? 60 : frame.len;
  for (size_t i = 0; i < count; ++i) sexos_volatile_write8((uint8_t *)(uintptr_t)(d->tx_page + i), i < frame.len ? frame.data[i] : 0);
  uint64_t descriptor = d->tx_desc + d->tx_next * 16;
  sexos_volatile_write64((uint64_t *)(uintptr_t) descriptor, d->tx_phys);
  sexos_volatile_write16((uint16_t *)(uintptr_t)(descriptor + 8), (uint16_t) count);
  sexos_volatile_write8((uint8_t *)(uintptr_t)(descriptor + 10), 0);
  sexos_volatile_write8((uint8_t *)(uintptr_t)(descriptor + 11), 0x0B);
  sexos_volatile_write8((uint8_t *)(uintptr_t)(descriptor + 12), 0);
  sexos_volatile_write8((uint8_t *)(uintptr_t)(descriptor + 13), 0);
  sexos_volatile_write16((uint16_t *)(uintptr_t)(descriptor + 14), 0);
  d->tx_busy = true;
  sexos_sfence();
  reg_write(d, 0x3818, (d->tx_next + 1) % 8);
  return true;
}
NetworkStep network_device_poll_tx(NetworkDevice *d) {
  if (d->failed || !d->tx_busy) return NET_FAILED;
  uint64_t descriptor = d->tx_desc + d->tx_next * 16;
  uint8_t status = sexos_volatile_read8((const uint8_t *)(uintptr_t)(descriptor + 12));
  if (!(status & 1)) return NET_PENDING;
  sexos_compiler_fence_seq_cst();
  d->tx_busy = false;
  d->tx_next = (d->tx_next + 1) % 8;
  if (status & 14) {
    d->failed = true;
    d->failure = NET_DEVICE_ERROR;
    return NET_FAILED;
  }
  return NET_READY;
}
typedef struct RxContext {
  NetworkDevice *device;
  NetworkFrame *frame, *sealed_frame;
  uint64_t descriptor;
  unsigned index;
  size_t count;
  bool available, valid, descriptor_owned, bytes_owned, copied, settled;
}
RxContext;
static bool rx_binding(RxContext *c) {
  return c->device && c->frame && c->frame == c->sealed_frame && c->index < 8 && c->index == c->device->rx_next && c->descriptor == c->device->rx_desc + c->index * 16;
}
static bool rx_available(RxContext *c) {
  return c->available;
}
static bool rx_frame_fact(RxContext *c) {
  return c->valid;
}
static bool rx_copied(RxContext *c) {
  return c->copied && c->frame->length == c->count;
}
static bool rx_settled(RxContext *c) {
  return c->settled;
}
static bool rx_descriptor_owned(RxContext *c) {
  return c->descriptor_owned;
}
static bool rx_bytes_owned(RxContext *c) {
  return c->bytes_owned;
}
static bool rx_poll(RxContext *c) {
  uint8_t status = sexos_volatile_read8((const uint8_t *)(uintptr_t)(c->descriptor + 12));
  c->available = (status & 1) != 0;
  if (!c->available) return false;
  sexos_compiler_fence_seq_cst();
  c->count = sexos_volatile_read16((const uint16_t *)(uintptr_t)(c->descriptor + 8));
  uint8_t errors = sexos_volatile_read8((const uint8_t *)(uintptr_t)(c->descriptor + 13));
  c->valid = !errors && status & 2 && c->count >= 14 && c->count <= sizeof c->frame->data;
  return true;
}
static bool rx_valid(RxContext *c) {
  return c->valid;
}
static bool rx_descriptor_acquire(RxContext *c) {
  if (c->descriptor_owned || !c->available || !rx_binding(c)) return false;
  c->descriptor_owned = true;
  return true;
}
static bool rx_bytes_acquire(RxContext *c) {
  if (c->bytes_owned || !c->valid || !rx_binding(c)) return false;
  c->bytes_owned = true;
  return true;
}
static bool rx_copy(RxContext *c) {
  if (!c->descriptor_owned || !c->bytes_owned || !c->valid) return false;
  for (size_t i = 0; i < c->count; ++i) c->frame->data[i] = sexos_volatile_read8((const uint8_t *)(uintptr_t)(c->device->rx_page[c->index] + i));
  c->frame->length = c->count;
  c->copied = true;
  c->settled = true;
  return true;
}
static bool rx_discard(RxContext *c) {
  c->frame->length = 0;
  c->settled = true;
  return true;
}
static void rx_recycle(RxContext *c) {
  sexos_volatile_write16((uint16_t *)(uintptr_t)(c->descriptor + 8), 0);
  sexos_volatile_write8((uint8_t *)(uintptr_t)(c->descriptor + 12), 0);
  sexos_sfence();
  reg_write(c->device, 0x2818, c->index);
  c->device->rx_next = (c->index + 1) % 8;
  c->descriptor_owned = false;
}
static void rx_descriptor_release(RxContext *c) {
  rx_recycle(c);
}
static void rx_descriptor_transfer(RxContext *c) {
  rx_recycle(c);
}
static void rx_bytes_release(RxContext *c) {
  c->frame->length = 0;
  c->bytes_owned = false;
}
static void rx_bytes_transfer(RxContext *c) {
  c->bytes_owned = false;
}
#define network_rx_run_CHECKED 1
#define network_rx_run_CHECK_FAILURE(ctx, kind, id, expected) sexos_fail()

/* Generated from cesta-protocol/1. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_network_rx_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_network_rx_run_DEFINED

#if defined(network_rx_run) || \
    defined(RxContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(network_rx_run_outcome) || \
    defined(network_rx_run_declined) || \
    defined(network_rx_run_answered) || \
    defined(network_rx_run_done) || \
    defined(network_rx_run_raised) || \
    defined(network_rx_run_suspended) || \
    defined(network_rx_run_resource_failure) || \
    defined(network_rx_run_engine_failure) || \
    defined(rx_poll) || \
    defined(rx_valid) || \
    defined(rx_copy) || \
    defined(rx_discard) || \
    defined(rx_descriptor_acquire) || \
    defined(rx_descriptor_release) || \
    defined(rx_descriptor_transfer) || \
    defined(rx_bytes_acquire) || \
    defined(rx_bytes_release) || \
    defined(rx_bytes_transfer) || \
    defined(rx_binding) || \
    defined(rx_available) || \
    defined(rx_frame_fact) || \
    defined(rx_copied) || \
    defined(rx_settled) || \
    defined(rx_descriptor_owned) || \
    defined(rx_bytes_owned) || \
    defined(network_rx_run_decline_reason) || \
    defined(network_rx_run_decline_count) || \
    defined(network_rx_run_counters) || \
    defined(network_rx_run_count_decline) || \
    defined(network_rx_run_counts) || \
    defined(network_rx_run_reason) || \
    defined(network_rx_run_COUNTED)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef network_rx_run_CHECKED
#define network_rx_run_CHECKED 0
#endif
#ifdef network_rx_run_COUNT_DECLINE
#define network_rx_run_COUNTED 1
#else
#define network_rx_run_COUNTED 0
#define network_rx_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if network_rx_run_CHECKED
#ifndef network_rx_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define network_rx_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol network_rx_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum network_rx_run_decline_reason {
    network_rx_run_decline_count
} network_rx_run_decline_reason;

typedef struct network_rx_run_counters {
    uint64_t declines[network_rx_run_decline_count ? network_rx_run_decline_count : 1];
} network_rx_run_counters;

typedef enum network_rx_run_outcome {
    network_rx_run_declined,
    network_rx_run_answered,
    network_rx_run_done,
    network_rx_run_raised,
    network_rx_run_suspended,
    network_rx_run_resource_failure,
    network_rx_run_engine_failure
} network_rx_run_outcome;

static inline network_rx_run_outcome
network_rx_run(RxContext *ctx)
{
    (void)ctx;
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (rx_bytes_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
    if (!rx_poll(ctx)) {
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_suspended;
    }
#if network_rx_run_CHECKED
    if (!rx_available(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "available", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
        for (;;) {}
    }
#endif
    if (!rx_descriptor_acquire(ctx)) {
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_raised;
    }
#if network_rx_run_CHECKED
    if (!rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_available(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "available", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
        for (;;) {}
    }
#endif
    if (rx_valid(ctx)) {
#if network_rx_run_CHECKED
        if (!rx_frame_fact(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "valid_frame", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_binding(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        if (!rx_bytes_acquire(ctx)) {
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
                for (;;) {}
            }
#endif
            rx_descriptor_release(ctx);
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
            return network_rx_run_resource_failure;
        }
#if network_rx_run_CHECKED
        if (!rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_frame_fact(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "valid_frame", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_binding(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
            for (;;) {}
        }
#endif
        if (rx_copy(ctx)) {
#if network_rx_run_CHECKED
            if (!rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_copied(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "copied", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_binding(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
            rx_bytes_transfer(ctx);
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_settled(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "settled", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_binding(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
            rx_descriptor_transfer(ctx);
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
            return network_rx_run_answered;
        }
#if network_rx_run_CHECKED
        if (!rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
            for (;;) {}
        }
#endif
        rx_bytes_release(ctx);
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
        rx_descriptor_release(ctx);
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_raised;
    } else {
#if network_rx_run_CHECKED
        if (!rx_available(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "available", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_binding(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
        if (rx_discard(ctx)) {
#if network_rx_run_CHECKED
            if (!rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_settled(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "settled", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_binding(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
            rx_descriptor_transfer(ctx);
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
            return network_rx_run_suspended;
        }
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
        rx_descriptor_release(ctx);
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_raised;
    }
}
NetworkStep network_device_receive(NetworkDevice *d, NetworkFrame *frame) {
  frame->length = 0;
  if (d->failed) return NET_FAILED;
  RxContext context = {
    .device = d,.frame = frame,.sealed_frame = frame,.index = d->rx_next,.descriptor = d->rx_desc + d->rx_next * 16
  };
  unsigned outcome = (unsigned) network_rx_run(&context);
  return outcome == network_rx_run_answered ? NET_READY : outcome == network_rx_run_suspended ? NET_PENDING : NET_FAILED;
}
