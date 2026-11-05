#!/usr/bin/env python3
"""Differential & mutation verifier for servers/sexusb (xHCI host controller + USB HID)."""

import argparse
import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
UPSTREAM_REV = "4a040a86"
EXPECTED_RUST_SHA256 = "5f1c32f6070fd3c0862d42b82287e6d91bb0ab6a38b7a75508f3916bd24251e1"

ENV_FLAGS = (
    "SEXUSB_SLOT2_OWNERSHIP_PROOF",
    "SEXUSB_SYNTHETIC_SLOT2",
)

CORE_VARIANTS = (
    ("default", ()),
    ("slot2_ownership", ("SEXUSB_SLOT2_OWNERSHIP_PROOF",)),
)

ALL_VARIANTS = CORE_VARIANTS + (
    ("synthetic_slot2", ("SEXUSB_SYNTHETIC_SLOT2",)),
    (
        "slot2_ownership_plus_synthetic_slot2",
        ("SEXUSB_SLOT2_OWNERSHIP_PROOF", "SEXUSB_SYNTHETIC_SLOT2"),
    ),
)

PROVIDER_C = r"""
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sexos_mmio.h"

extern void sexusb_main(void);

#define FAKE_XHCI_BAR_VA UINT64_C(0x71000000)
#define FAKE_PHYS_BASE UINT64_C(0x100000)
#define MAX_DMA_PAGES 64

#define SLOT_USB_HOST UINT64_C(8)
#define SLOT_USB_SEXINPUT UINT64_C(9)
#define ERR_CAP_INVALID UINT64_C(0xFFFFFFFFFFFFFFFC)

typedef struct {
    uint64_t phys;
    uint8_t *mem;
    uint64_t va;
    bool mapped;
} DmaPage;

static DmaPage dma_pages[MAX_DMA_PAGES];
static size_t dma_page_count = 0;
static uint8_t xhci_bar[0x10000];

static int current_case = 0;
static uint32_t bar_map_fail_Remaining = 0;
static bool cfg_halt_timeout = false;
static bool cfg_hcrst_timeout = false;
static bool cfg_cnr_timeout = false;
static bool cfg_csz1 = false;
static uint8_t cfg_cap_length = 0x20;
static uint32_t cfg_sexinput_missing_calls = 0;
static uint32_t cfg_max_Outer_loops = 6;
static uint32_t cfg_empty_polls_before_first_event = 0;
static bool cfg_timeout_on_poll = false;

typedef struct {
    bool connected;
    bool enabled;
    uint8_t speed;
    uint8_t b_max_packet_size0;
    uint8_t b_interface_class;
    uint8_t b_interface_subclass;
    uint8_t b_interface_protocol;
    uint8_t ep_addr;
    uint16_t ep_mps;
    uint8_t ep_interval;
} UsbPortDeviceCfg;

static UsbPortDeviceCfg port_cfgs[4];

typedef struct {
    uint8_t slot_id;
    uint8_t ep_dci;
    uint8_t cc;
    uint8_t actual_len;
    uint32_t custom_residue;
    bool use_custom_residue;
    uint64_t custom_trb_ptr;
    bool use_custom_trb_ptr;
    uint8_t bytes[8];
} ScriptedIntrEvent;

static ScriptedIntrEvent intr_events[128];
static size_t intr_event_count = 0;
static size_t intr_event_idx = 0;
static uint32_t intr_Empty_wait = 0;

typedef struct {
    bool allocated;
    uint8_t port;
    uint64_t dev_ctx_phys;
    uint64_t ep0_ring_phys;
    uint64_t ep0_deq_idx;
    uint32_t ep0_dcs;
    uint64_t intr_ring_phys;
    uint64_t intr_deq_idx;
    uint32_t intr_dci;
} SlotModel;

static SlotModel slots[4];
static uint32_t next_slot_id = 1;
static uint64_t model_cmd_ring_phys = 0;
static uint64_t model_cmd_idx = 0;
static uint64_t model_dcbaa_phys = 0;
static uint64_t model_event_ring_phys = 0;
static uint64_t model_ev_prod_idx = 0;
static uint32_t model_ev_pcs = 1;

static uint64_t mmio_order = 0;
static uint64_t dma_reads = 0;
static uint64_t dma_writes = 0;
static uint64_t dma_fnv = UINT64_C(0xcbf29ce484222325);
static uint32_t yield_count = 0;
static uint32_t poll_loop_iters = 0;
static bool in_poll_loop = false;

static void dma_hash_mix(uint64_t v)
{
    dma_fnv ^= v;
    dma_fnv *= UINT64_C(0x100000001b3);
}

static uint32_t read_le32_raw(const uint8_t *p)
{
    uint32_t v;
    memcpy(&v, p, 4);
    return v;
}

static void write_le32_raw(uint8_t *p, uint32_t v)
{
    memcpy(p, &v, 4);
}

static uint64_t read_le64_raw(const uint8_t *p)
{
    uint64_t v;
    memcpy(&v, p, 8);
    return v;
}

static DmaPage *find_page_by_phys(uint64_t phys)
{
    for (size_t i = 0; i < dma_page_count; ++i) {
        if (dma_pages[i].phys == phys) return &dma_pages[i];
    }
    return NULL;
}

static uint8_t *resolve_dma_addr(uintptr_t addr, size_t width)
{
    for (size_t i = 0; i < dma_page_count; ++i) {
        if (!dma_pages[i].mapped) continue;
        uintptr_t base = (uintptr_t)dma_pages[i].va;
        if (addr >= base && addr + width <= base + 4096) {
            return dma_pages[i].mem + (addr - base);
        }
    }
    return NULL;
}

static void push_event_trb(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3_no_cycle)
{
    DmaPage *ev_pg = find_page_by_phys(model_event_ring_phys);
    if (!ev_pg) {
        printf("FATAL|no_event_ring_page phys=%#llx\n", (unsigned long long)model_event_ring_phys);
        exit(2);
    }
    uint8_t *p = ev_pg->mem + model_ev_prod_idx * 16ULL;
    write_le32_raw(p + 0, d0);
    write_le32_raw(p + 4, d1);
    write_le32_raw(p + 8, d2);
    write_le32_raw(p + 12, (d3_no_cycle & ~1u) | (model_ev_pcs & 1u));
    model_ev_prod_idx++;
    if (model_ev_prod_idx >= 64ULL) {
        model_ev_prod_idx = 0;
        model_ev_pcs ^= 1u;
    }
}

static void push_cmd_completion(uint64_t cmd_trb_phys, uint32_t cc, uint32_t slot_id)
{
    uint32_t d0 = (uint32_t)(cmd_trb_phys & 0xFFFFFFFFULL);
    uint32_t d1 = (uint32_t)(cmd_trb_phys >> 32);
    uint32_t d2 = (cc & 0xFFu) << 24;
    uint32_t d3 = (33u << 10) | ((slot_id & 0xFFu) << 24);
    push_event_trb(d0, d1, d2, d3);
}

static void push_transfer_event(uint64_t trb_phys, uint32_t cc, uint32_t residue, uint32_t slot_id, uint32_t ep_dci)
{
    uint32_t d0 = (uint32_t)(trb_phys & 0xFFFFFFFFULL);
    uint32_t d1 = (uint32_t)(trb_phys >> 32);
    uint32_t d2 = ((cc & 0xFFu) << 24) | (residue & 0xFFFFFFu);
    uint32_t d3 = (32u << 10) | ((ep_dci & 0x1Fu) << 16) | ((slot_id & 0xFFu) << 24);
    push_event_trb(d0, d1, d2, d3);
}

static void handle_hc_command_doorbell(void)
{
    DmaPage *cmd_pg = find_page_by_phys(model_cmd_ring_phys);
    if (!cmd_pg) return;
    uint64_t trb_phys = model_cmd_ring_phys + model_cmd_idx * 16ULL;
    const uint8_t *trb = cmd_pg->mem + model_cmd_idx * 16ULL;
    uint32_t d0 = read_le32_raw(trb + 0);
    uint32_t d1 = read_le32_raw(trb + 4);
    uint32_t d2 = read_le32_raw(trb + 8);
    uint32_t d3 = read_le32_raw(trb + 12);
    (void)d2;
    uint32_t trb_type = (d3 >> 10) & 0x3Fu;
    uint32_t slot_id = (d3 >> 24) & 0xFFu;
    model_cmd_idx++;

    printf("XHCI_CMD|idx=%llu|type=%u|slot=%u|d0=%#x|d1=%#x\n",
           (unsigned long long)(model_cmd_idx - 1), (unsigned)trb_type,
           (unsigned)slot_id, (unsigned)d0, (unsigned)d1);

    if (trb_type == 23u) {
        /* NOOP_CMD */
        push_cmd_completion(trb_phys, 1u, 0u);
    } else if (trb_type == 9u) {
        /* ENABLE_SLOT_CMD */
        uint32_t s = next_slot_id++;
        slots[s].allocated = true;
        push_cmd_completion(trb_phys, 1u, s);
    } else if (trb_type == 11u) {
        /* ADDRESS_DEVICE_CMD */
        uint64_t in_ctx_phys = ((uint64_t)d1 << 32) | d0;
        DmaPage *in_pg = find_page_by_phys(in_ctx_phys);
        DmaPage *dcbaa_pg = find_page_by_phys(model_dcbaa_phys);
        if (!in_pg || !dcbaa_pg || slot_id == 0 || slot_id >= 4) {
            push_cmd_completion(trb_phys, 4u, slot_id);
            return;
        }
        uint64_t dev_ctx_phys = read_le64_raw(dcbaa_pg->mem + (size_t)slot_id * 8);
        DmaPage *dev_pg = find_page_by_phys(dev_ctx_phys);
        if (!dev_pg) {
            push_cmd_completion(trb_phys, 4u, slot_id);
            return;
        }
        slots[slot_id].dev_ctx_phys = dev_ctx_phys;
        /* Input Slot Context is at +0x20, Input EP0 Context is at +0x40 */
        uint32_t in_slot_dw0 = read_le32_raw(in_pg->mem + 0x20 + 0);
        uint32_t in_slot_dw1 = read_le32_raw(in_pg->mem + 0x20 + 4);
        uint8_t root_port = (uint8_t)((in_slot_dw1 >> 16) & 0xFFu);
        slots[slot_id].port = root_port;

        uint32_t in_ep0_dw1 = read_le32_raw(in_pg->mem + 0x40 + 4);
        uint32_t in_ep0_dw2 = read_le32_raw(in_pg->mem + 0x40 + 8);
        uint32_t in_ep0_dw3 = read_le32_raw(in_pg->mem + 0x40 + 12);
        uint64_t ep0_tr = (((uint64_t)in_ep0_dw3 << 32) | in_ep0_dw2) & ~0xFULL;
        slots[slot_id].ep0_ring_phys = ep0_tr;
        slots[slot_id].ep0_deq_idx = 0;
        slots[slot_id].ep0_dcs = in_ep0_dw2 & 1u;

        /* Write Output Slot Context (+0x00): slot_state=2 (Addressed) in dw3[31:27] */
        write_le32_raw(dev_pg->mem + 0x00 + 0, in_slot_dw0);
        write_le32_raw(dev_pg->mem + 0x00 + 4, in_slot_dw1);
        write_le32_raw(dev_pg->mem + 0x00 + 12, (2u << 27) | slot_id);
        /* Write Output EP0 Context (+0x20): ep_state=1 (Running) in dw0[2:0] */
        write_le32_raw(dev_pg->mem + 0x20 + 0, 1u);
        write_le32_raw(dev_pg->mem + 0x20 + 4, in_ep0_dw1);
        write_le32_raw(dev_pg->mem + 0x20 + 8, in_ep0_dw2);
        write_le32_raw(dev_pg->mem + 0x20 + 12, in_ep0_dw3);

        push_cmd_completion(trb_phys, 1u, slot_id);
    } else if (trb_type == 13u) {
        /* EVALUATE_CONTEXT_CMD */
        uint64_t in_ctx_phys = ((uint64_t)d1 << 32) | d0;
        DmaPage *in_pg = find_page_by_phys(in_ctx_phys);
        DmaPage *dev_pg = find_page_by_phys(slots[slot_id].dev_ctx_phys);
        if (in_pg && dev_pg) {
            uint32_t in_ep0_dw1 = read_le32_raw(in_pg->mem + 0x40 + 4);
            write_le32_raw(dev_pg->mem + 0x20 + 4, in_ep0_dw1);
        }
        push_cmd_completion(trb_phys, 1u, slot_id);
    } else if (trb_type == 12u) {
        /* CONFIGURE_ENDPOINT_CMD */
        uint64_t in_ctx_phys = ((uint64_t)d1 << 32) | d0;
        DmaPage *in_pg = find_page_by_phys(in_ctx_phys);
        DmaPage *dev_pg = find_page_by_phys(slots[slot_id].dev_ctx_phys);
        if (in_pg && dev_pg) {
            uint32_t add_flags = read_le32_raw(in_pg->mem + 0x04);
            uint32_t in_slot_dw0 = read_le32_raw(in_pg->mem + 0x20 + 0);
            write_le32_raw(dev_pg->mem + 0x00 + 0, in_slot_dw0);
            write_le32_raw(dev_pg->mem + 0x00 + 12, (3u << 27) | slot_id);
            for (uint32_t dci = 2; dci < 31; ++dci) {
                if (add_flags & (1u << dci)) {
                    const uint8_t *in_ep = in_pg->mem + (size_t)(dci + 1) * 0x20;
                    uint8_t *out_ep = dev_pg->mem + (size_t)dci * 0x20;
                    uint32_t ep_dw0 = read_le32_raw(in_ep + 0);
                    uint32_t ep_dw1 = read_le32_raw(in_ep + 4);
                    uint32_t ep_dw2 = read_le32_raw(in_ep + 8);
                    uint32_t ep_dw3 = read_le32_raw(in_ep + 12);
                    uint32_t ep_dw4 = read_le32_raw(in_ep + 16);
                    write_le32_raw(out_ep + 0, (ep_dw0 & ~0x7u) | 1u);
                    write_le32_raw(out_ep + 4, ep_dw1);
                    write_le32_raw(out_ep + 8, ep_dw2);
                    write_le32_raw(out_ep + 12, ep_dw3);
                    write_le32_raw(out_ep + 16, ep_dw4);
                    slots[slot_id].intr_ring_phys = (((uint64_t)ep_dw3 << 32) | ep_dw2) & ~0xFULL;
                    slots[slot_id].intr_deq_idx = 0;
                    slots[slot_id].intr_dci = dci;
                }
            }
        }
        push_cmd_completion(trb_phys, 1u, slot_id);
    }
}

static void handle_ep0_doorbell(uint32_t slot_id)
{
    if (slot_id == 0 || slot_id >= 4 || !slots[slot_id].allocated) return;
    SlotModel *sm = &slots[slot_id];
    DmaPage *ep0_pg = find_page_by_phys(sm->ep0_ring_phys);
    DmaPage *dev_pg = find_page_by_phys(sm->dev_ctx_phys);
    if (!ep0_pg || !dev_pg) return;

    uint64_t idx = sm->ep0_deq_idx;
    const uint8_t *t0 = ep0_pg->mem + idx * 16ULL;
    uint32_t setup0 = read_le32_raw(t0 + 0);
    uint32_t setup1 = read_le32_raw(t0 + 4);

    uint8_t bm_req = (uint8_t)(setup0 & 0xFFu);
    uint8_t b_req = (uint8_t)((setup0 >> 8) & 0xFFu);
    uint16_t w_value = (uint16_t)((setup0 >> 16) & 0xFFFFu);
    uint16_t w_index = (uint16_t)(setup1 & 0xFFFFu);
    uint16_t w_length = (uint16_t)((setup1 >> 16) & 0xFFFFu);
    (void)bm_req;
    (void)w_index;

    const uint8_t *t1 = ep0_pg->mem + (idx + 1ULL) * 16ULL;
    uint32_t t1_d0 = read_le32_raw(t1 + 0);
    uint32_t t1_d1 = read_le32_raw(t1 + 4);
    uint32_t t1_d3 = read_le32_raw(t1 + 12);
    uint32_t t1_type = (t1_d3 >> 10) & 0x3Fu;

    uint8_t port_idx = (sm->port >= 1 && sm->port <= 4) ? (sm->port - 1) : 0;
    const UsbPortDeviceCfg *pcfg = &port_cfgs[port_idx];

    printf("EP0_XFER|slot=%u|port=%u|req=%#x|val=%#x|len=%u|t1_type=%u\n",
           (unsigned)slot_id, (unsigned)sm->port, (unsigned)b_req,
           (unsigned)w_value, (unsigned)w_length, (unsigned)t1_type);

    uint64_t status_trb_idx = idx + 1ULL;
    if (t1_type == 3u) {
        /* DATA_STAGE */
        status_trb_idx = idx + 2ULL;
        uint64_t data_phys = ((uint64_t)t1_d1 << 32) | t1_d0;
        DmaPage *data_pg = find_page_by_phys(data_phys);
        if (data_pg) {
            if (b_req == 6u && w_value == 0x0100u) {
                /* Device Descriptor (18 bytes) */
                uint8_t dev_desc[18] = {
                    18, 1, 0x00, 0x02,
                    0x00, 0x00, 0x00, pcfg->b_max_packet_size0,
                    0x27, 0x06, 0x01, (uint8_t)(0x00 + sm->port),
                    0x00, 0x01, 1, 2, 3, 1
                };
                size_t n = w_length < 18 ? w_length : 18;
                memcpy(data_pg->mem, dev_desc, n);
            } else if (b_req == 6u && w_value == 0x0200u) {
                /* Configuration Descriptor (34 bytes: Config(9)+Iface(9)+HID(9)+EP(7)) */
                uint8_t cfg_desc[34] = {
                    /* Config (9) */
                    9, 2, 34, 0, 1, 1, 0, 0x80, 50,
                    /* Interface (9) */
                    9, 4, 0, 0, 1,
                    pcfg->b_interface_class,
                    pcfg->b_interface_subclass,
                    pcfg->b_interface_protocol,
                    0,
                    /* HID (9) */
                    9, 0x21, 0x11, 0x01, 0x00, 1, 0x22, 50, 0x00,
                    /* Endpoint (7) */
                    7, 5, pcfg->ep_addr, 0x03,
                    (uint8_t)(pcfg->ep_mps & 0xFFu),
                    (uint8_t)(pcfg->ep_mps >> 8),
                    pcfg->ep_interval
                };
                size_t n = w_length < 34 ? w_length : 34;
                memcpy(data_pg->mem, cfg_desc, n);
            } else if (b_req == 6u && w_value == 0x2200u) {
                /* HID Report Descriptor (50 bytes) */
                uint8_t rpt_desc[50];
                memset(rpt_desc, 0xA5, sizeof(rpt_desc));
                rpt_desc[0] = 0x05;
                rpt_desc[1] = 0x01;
                rpt_desc[2] = 0x09;
                rpt_desc[3] = 0x02;
                size_t n = w_length < 50 ? w_length : 50;
                memcpy(data_pg->mem, rpt_desc, n);
            }
        }
    }

    uint64_t next_idx = status_trb_idx + 1ULL;
    sm->ep0_deq_idx = next_idx;
    uint64_t new_deq_ptr = (sm->ep0_ring_phys + next_idx * 16ULL) | (sm->ep0_dcs & 1u);
    write_le32_raw(dev_pg->mem + 0x20 + 8, (uint32_t)(new_deq_ptr & 0xFFFFFFFFULL));
    write_le32_raw(dev_pg->mem + 0x20 + 12, (uint32_t)(new_deq_ptr >> 32));

    uint64_t status_trb_phys = sm->ep0_ring_phys + status_trb_idx * 16ULL;
    push_transfer_event(status_trb_phys, 1u, 0u, slot_id, 1u);
}

static void deliver_next_scripted_intr_event(void)
{
    if (intr_event_idx >= intr_event_count) return;
    if (intr_Empty_wait > 0) {
        intr_Empty_wait--;
        return;
    }
    const ScriptedIntrEvent *ev = &intr_events[intr_event_idx++];
    uint8_t s = ev->slot_id;
    uint64_t trb_ptr = 0;
    uint32_t req_len = 8;

    if (s >= 1 && s < 4 && slots[s].allocated && slots[s].intr_ring_phys != 0) {
        SlotModel *sm = &slots[s];
        DmaPage *ring_pg = find_page_by_phys(sm->intr_ring_phys);
        uint64_t pos = sm->intr_deq_idx;
        sm->intr_deq_idx = (pos + 1ULL) % 8ULL;
        trb_ptr = sm->intr_ring_phys + pos * 16ULL;
        if (ring_pg) {
            const uint8_t *trb = ring_pg->mem + pos * 16ULL;
            uint32_t d0 = read_le32_raw(trb + 0);
            uint32_t d1 = read_le32_raw(trb + 4);
            uint32_t d2 = read_le32_raw(trb + 8);
            req_len = d2 & 0x1FFFFu;
            uint64_t buf_phys = ((uint64_t)d1 << 32) | d0;
            uint64_t page_phys = buf_phys & ~0xFFFULL;
            uint64_t page_off = buf_phys & 0xFFFULL;
            DmaPage *buf_pg = find_page_by_phys(page_phys);
            if (buf_pg && page_off + 8 <= 4096) {
                memcpy(buf_pg->mem + page_off, ev->bytes, 8);
            }
        }
    }
    if (ev->use_custom_trb_ptr) {
        trb_ptr = ev->custom_trb_ptr;
    }
    uint32_t residue = ev->use_custom_residue
        ? ev->custom_residue
        : (req_len >= ev->actual_len ? (req_len - ev->actual_len) : 0u);
    push_transfer_event(trb_ptr, ev->cc, residue, ev->slot_id, ev->ep_dci);
}

uint8_t sexos_volatile_read8(const uint8_t *addr)
{
    uint8_t *p = resolve_dma_addr((uintptr_t)addr, 1);
    if (!p) {
        printf("FATAL|bad_read8 addr=%p\n", (const void *)addr);
        exit(2);
    }
    uint8_t v = *p;
    dma_reads++;
    dma_hash_mix(((uint64_t)1 << 56) | v);
    return v;
}

uint16_t sexos_volatile_read16(const uint16_t *addr)
{
    uint8_t *p = resolve_dma_addr((uintptr_t)addr, 2);
    if (!p) {
        printf("FATAL|bad_read16 addr=%p\n", (const void *)addr);
        exit(2);
    }
    uint16_t v;
    memcpy(&v, p, 2);
    dma_reads++;
    dma_hash_mix(((uint64_t)2 << 56) | v);
    return v;
}

uint32_t sexos_volatile_read32(const uint32_t *addr)
{
    uintptr_t a = (uintptr_t)addr;
    if (a >= FAKE_XHCI_BAR_VA && a + 4 <= FAKE_XHCI_BAR_VA + sizeof(xhci_bar)) {
        uint64_t off = (uint64_t)(a - FAKE_XHCI_BAR_VA);
        uint32_t v = read_le32_raw(xhci_bar + off);
        mmio_order++;
        return v;
    }
    uint8_t *p = resolve_dma_addr(a, 4);
    if (!p) {
        printf("FATAL|bad_read32 addr=%p\n", (const void *)addr);
        exit(2);
    }
    uint32_t v = read_le32_raw(p);
    dma_reads++;
    dma_hash_mix(((uint64_t)4 << 56) | v);
    return v;
}

uint64_t sexos_volatile_read64(const uint64_t *addr)
{
    uint8_t *p = resolve_dma_addr((uintptr_t)addr, 8);
    if (!p) {
        printf("FATAL|bad_read64 addr=%p\n", (const void *)addr);
        exit(2);
    }
    uint64_t v = read_le64_raw(p);
    dma_reads++;
    dma_hash_mix(((uint64_t)8 << 56) ^ v);
    return v;
}

void sexos_volatile_write8(uint8_t *addr, uint8_t value)
{
    uint8_t *p = resolve_dma_addr((uintptr_t)addr, 1);
    if (!p) {
        printf("FATAL|bad_write8 addr=%p\n", (void *)addr);
        exit(2);
    }
    *p = value;
    dma_writes++;
    dma_hash_mix(((uint64_t)0x11 << 56) | value);
}

void sexos_volatile_write16(uint16_t *addr, uint16_t value)
{
    uint8_t *p = resolve_dma_addr((uintptr_t)addr, 2);
    if (!p) {
        printf("FATAL|bad_write16 addr=%p\n", (void *)addr);
        exit(2);
    }
    memcpy(p, &value, 2);
    dma_writes++;
    dma_hash_mix(((uint64_t)0x12 << 56) | value);
}

void sexos_volatile_write32(uint32_t *addr, uint32_t value)
{
    uintptr_t a = (uintptr_t)addr;
    if (a >= FAKE_XHCI_BAR_VA && a + 4 <= FAKE_XHCI_BAR_VA + sizeof(xhci_bar)) {
        uint64_t off = (uint64_t)(a - FAKE_XHCI_BAR_VA);
        write_le32_raw(xhci_bar + off, value);
        mmio_order++;
        printf("MMIO|xhci|off=%#llx|w=32|dir=W|val=%#x|seq=%llu\n",
               (unsigned long long)off, (unsigned)value, (unsigned long long)mmio_order);

        uint64_t op_base = cfg_cap_length;
        if (off == op_base + 0x00ULL) {
            /* USBCMD */
            uint32_t st = read_le32_raw(xhci_bar + op_base + 0x04ULL);
            if (value & (1u << 1)) {
                /* HCRST */
                if (!cfg_hcrst_timeout) {
                    write_le32_raw(xhci_bar + op_base + 0x00ULL, value & ~(1u << 1));
                }
                if (!cfg_cnr_timeout) {
                    st &= ~(1u << 11);
                } else {
                    st |= (1u << 11);
                }
                st |= 1u;
                write_le32_raw(xhci_bar + op_base + 0x04ULL, st);
            } else if (value & 1u) {
                /* RUN_STOP = 1 */
                if (!cfg_halt_timeout) {
                    st &= ~1u;
                    write_le32_raw(xhci_bar + op_base + 0x04ULL, st);
                }
            } else {
                /* RUN_STOP = 0 */
                if (!cfg_halt_timeout) {
                    st |= 1u;
                    write_le32_raw(xhci_bar + op_base + 0x04ULL, st);
                }
            }
        } else if (off == op_base + 0x18ULL || off == op_base + 0x1CULL) {
            uint64_t crcr = read_le64_raw(xhci_bar + op_base + 0x18ULL);
            model_cmd_ring_phys = crcr & ~0x3FULL;
        } else if (off == op_base + 0x30ULL || off == op_base + 0x34ULL) {
            model_dcbaa_phys = read_le64_raw(xhci_bar + op_base + 0x30ULL);
        } else if (off == 0x320ULL + 0x10ULL || off == 0x320ULL + 0x14ULL) {
            uint64_t erstba = read_le64_raw(xhci_bar + 0x320ULL + 0x10ULL);
            DmaPage *erst_pg = find_page_by_phys(erstba & ~0x3FULL);
            if (erst_pg) {
                model_event_ring_phys = read_le64_raw(erst_pg->mem + 0);
            }
        } else if (off >= 0x200ULL && off < 0x240ULL) {
            uint32_t db_slot = (uint32_t)((off - 0x200ULL) / 4ULL);
            if (db_slot == 0 && value == 0) {
                handle_hc_command_doorbell();
            } else if (db_slot >= 1 && db_slot < 4 && value == 1u) {
                handle_ep0_doorbell(db_slot);
            } else if (db_slot >= 1 && db_slot < 4 && value >= 2u) {
                if (!in_poll_loop && db_slot == 1) {
                    in_poll_loop = true;
                    if (!cfg_timeout_on_poll && cfg_empty_polls_before_first_event == 0) {
                        deliver_next_scripted_intr_event();
                    }
                }
            }
        }
        return;
    }
    uint8_t *p = resolve_dma_addr(a, 4);
    if (!p) {
        printf("FATAL|bad_write32 addr=%p\n", (void *)addr);
        exit(2);
    }
    write_le32_raw(p, value);
    dma_writes++;
    dma_hash_mix(((uint64_t)0x14 << 56) | value);
}

void sexos_volatile_write64(uint64_t *addr, uint64_t value)
{
    uint8_t *p = resolve_dma_addr((uintptr_t)addr, 8);
    if (!p) {
        printf("FATAL|bad_write64 addr=%p\n", (void *)addr);
        exit(2);
    }
    memcpy(p, &value, 8);
    dma_writes++;
    dma_hash_mix(((uint64_t)0x18 << 56) ^ value);
}

void sexos_compiler_fence_seq_cst(void) {}
void sexos_sfence(void) {}

uint64_t sexos_map_pci_bar(uint64_t cap_slot, uint64_t bar_index, uint64_t map_size)
{
    printf("SYSCALL|map_pci_bar|slot=%llu|bar=%llu|size=%#llx\n",
           (unsigned long long)cap_slot, (unsigned long long)bar_index, (unsigned long long)map_size);
    if (cap_slot != SLOT_USB_HOST || bar_index != 0) return UINT64_MAX;
    if (bar_map_fail_Remaining > 0) {
        bar_map_fail_Remaining--;
        return 0ULL;
    }
    return FAKE_XHCI_BAR_VA;
}

uint64_t sexos_alloc_shared_buffer(uint64_t size, uint64_t consumer_domain)
{
    (void)size;
    (void)consumer_domain;
    return 0;
}

uint64_t sexos_allocate_memory(uint64_t size)
{
    if (dma_page_count >= MAX_DMA_PAGES || size != 4096ULL) {
        return UINT64_MAX;
    }
    DmaPage *pg = &dma_pages[dma_page_count];
    pg->phys = FAKE_PHYS_BASE + (uint64_t)dma_page_count * 0x1000ULL;
    pg->mem = (uint8_t *)aligned_alloc(4096, 4096);
    memset(pg->mem, 0, 4096);
    pg->va = (uint64_t)(uintptr_t)pg->mem;
    pg->mapped = false;
    dma_page_count++;
    printf("SYSCALL|alloc_phys|size=%#llx->phys=%#llx\n",
           (unsigned long long)size, (unsigned long long)pg->phys);
    return pg->phys;
}

uint64_t sexos_map_memory(uint64_t phys, uint64_t size)
{
    DmaPage *pg = find_page_by_phys(phys);
    if (!pg || size != 4096ULL) {
        return UINT64_MAX;
    }
    pg->mapped = true;
    uint64_t rel_va = 0x80000000ULL + (phys - FAKE_PHYS_BASE);
    printf("SYSCALL|map_phys|phys=%#llx|size=%#llx->rel_va=%#llx\n",
           (unsigned long long)phys, (unsigned long long)size, (unsigned long long)rel_va);
    return pg->va;
}

uint64_t sexos_map_mem_lend(uint64_t cap_slot)
{
    (void)cap_slot;
    return UINT64_MAX;
}

uint64_t sexos_set_state(uint64_t state)
{
    (void)state;
    return 0;
}

static void print_normalized_line(const char *text, size_t length)
{
    char buf[1024];
    size_t n = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
    memcpy(buf, text, n);
    buf[n] = '\0';
    if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = '\0';

    char out[1024];
    strcpy(out, buf);
    for (size_t i = 0; i < dma_page_count; ++i) {
        char pat[32], rep[32];
        snprintf(pat, sizeof(pat), "0x%llx", (unsigned long long)dma_pages[i].va);
        snprintf(rep, sizeof(rep), "0x%llx", (unsigned long long)(0x80000000ULL + (dma_pages[i].phys - FAKE_PHYS_BASE)));
        char *pos;
        while ((pos = strstr(out, pat)) != NULL) {
            char tmp[1024];
            size_t prefix = (size_t)(pos - out);
            snprintf(tmp, sizeof(tmp), "%.*s%s%s", (int)prefix, out, rep, pos + strlen(pat));
            strcpy(out, tmp);
        }
    }
    printf("SERIAL|%s\n", out);
}

SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
    if (slot == SEXOS_SLOT_kernel && opcode == SEXOS_KERNEL_raw_print) {
        print_normalized_line((const char *)(uintptr_t)arg0, (size_t)arg1);
        SexosReply r = {0, 0};
        return r;
    }
    printf("PDX_CALL|slot=%llu|op=%#llx|a0=%#llx|a1=%#llx|a2=%#llx\n",
           (unsigned long long)slot, (unsigned long long)opcode,
           (unsigned long long)arg0, (unsigned long long)arg1, (unsigned long long)arg2);
    if (slot == SLOT_USB_SEXINPUT && cfg_sexinput_missing_calls > 0) {
        cfg_sexinput_missing_calls--;
        SexosReply r = {ERR_CAP_INVALID, 0};
        return r;
    }
    SexosReply r = {0, 0};
    return r;
}

SexosMessage sexos_listen_once(uint64_t slot)
{
    (void)slot;
    SexosMessage m = {0, 0, 0, 0, 0};
    return m;
}

uint64_t sexos_reply(uint32_t caller_pd, uint64_t val)
{
    (void)caller_pd;
    (void)val;
    return 0;
}

_Noreturn static void finish_and_exit(void)
{
    printf("SUMMARY|case=%d|mmio=%llu|dma_r=%llu|dma_w=%llu|fnv=%#018llx\n",
           current_case,
           (unsigned long long)mmio_order,
           (unsigned long long)dma_reads,
           (unsigned long long)dma_writes,
           (unsigned long long)dma_fnv);
    for (size_t i = 0; i < dma_page_count; ++i) free(dma_pages[i].mem);
    exit(0);
}

void sexos_yield(void)
{
    yield_count++;
    if (!in_poll_loop) {
        if (yield_count > 500000u) {
            finish_and_exit();
        }
        return;
    }
    if (cfg_timeout_on_poll) {
        if (yield_count > 450000u) {
            finish_and_exit();
        }
        return;
    }
    if (intr_event_idx < intr_event_count) {
        deliver_next_scripted_intr_event();
        return;
    }
    poll_loop_iters++;
    if (poll_loop_iters >= cfg_max_Outer_loops) {
        finish_and_exit();
    }
}

void sexos_spin_loop_hint(void) {}
void sexos_spin_forever(void) { finish_and_exit(); }

static void add_intr_event(uint8_t slot_id, uint8_t ep_dci, uint8_t cc, uint8_t actual_len,
                           uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
    if (intr_event_count >= 128) return;
    ScriptedIntrEvent *e = &intr_events[intr_event_count++];
    memset(e, 0, sizeof(*e));
    e->slot_id = slot_id;
    e->ep_dci = ep_dci;
    e->cc = cc;
    e->actual_len = actual_len;
    e->bytes[0] = b0;
    e->bytes[1] = b1;
    e->bytes[2] = b2;
    e->bytes[3] = b3;
    e->bytes[4] = b4;
}

static void setup_case(int c)
{
    current_case = c;
    memset(xhci_bar, 0, sizeof(xhci_bar));
    memset(port_cfgs, 0, sizeof(port_cfgs));
    memset(slots, 0, sizeof(slots));
    memset(intr_events, 0, sizeof(intr_events));
    intr_event_count = 0;
    intr_event_idx = 0;
    next_slot_id = 1;

    /* Default: port 1 connected, HighSpeed (3), Keyboard (class=3, sub=1, proto=1), EP 0x81, mps=8, interval=10 */
    port_cfgs[0].connected = true;
    port_cfgs[0].enabled = true;
    port_cfgs[0].speed = 3;
    port_cfgs[0].b_max_packet_size0 = 64;
    port_cfgs[0].b_interface_class = 3;
    port_cfgs[0].b_interface_subclass = 1;
    port_cfgs[0].b_interface_protocol = 1;
    port_cfgs[0].ep_addr = 0x81;
    port_cfgs[0].ep_mps = 8;
    port_cfgs[0].ep_interval = 10;

    if (c == 0) {
        /* Single USB Keyboard on Port 1 + sexinput missing first call + key press/release + short packet */
        bar_map_fail_Remaining = 1;
        cfg_sexinput_missing_calls = 1;
        add_intr_event(1, 3, 1, 8, 0x02, 0x00, 0x04, 0x00, 0x00);
        add_intr_event(1, 3, 13, 8, 0x00, 0x00, 0x02, 0x00, 0x00);
        add_intr_event(1, 3, 1, 8, 0x00, 0x00, 0x00, 0x00, 0x00);
    } else if (c == 1) {
        /* Single USB Tablet on Port 1 (mps0=8 so Evaluate Context skipped), 66 reports for active/idle/nonzero */
        port_cfgs[0].b_max_packet_size0 = 8;
        port_cfgs[0].b_interface_subclass = 0;
        port_cfgs[0].b_interface_protocol = 0;
        for (int i = 0; i < 66; ++i) {
            uint16_t x = (i < 3) ? 1000u : (uint16_t)(1000 + i * 10);
            uint16_t y = (i < 3) ? 2000u : (uint16_t)(2000 + i * 5);
            uint8_t btn = (i == 4) ? 5u : 0u;
            add_intr_event(1, 3, 1, 6, btn, (uint8_t)(x & 0xFF), (uint8_t)(x >> 8),
                           (uint8_t)(y & 0xFF), (uint8_t)(y >> 8));
        }
        add_intr_event(1, 3, 1, 3, 0, 0, 0, 0, 0); /* decode.bad (len < 5) */
    } else if (c == 2) {
        /* Single USB Boot Mouse on Port 2 (Port 1 disconnected) */
        port_cfgs[0].connected = false;
        port_cfgs[0].enabled = false;
        port_cfgs[1] = port_cfgs[0];
        port_cfgs[1].connected = true;
        port_cfgs[1].enabled = true;
        port_cfgs[1].speed = 3;
        port_cfgs[1].b_max_packet_size0 = 64;
        port_cfgs[1].b_interface_class = 3;
        port_cfgs[1].b_interface_subclass = 1;
        port_cfgs[1].b_interface_protocol = 2; /* Boot Mouse */
        port_cfgs[1].ep_addr = 0x81;
        port_cfgs[1].ep_mps = 4;
        port_cfgs[1].ep_interval = 10;
        add_intr_event(1, 3, 1, 4, 0x00, 0x00, 0x00, 0x00, 0x00); /* idle */
        add_intr_event(1, 3, 1, 4, 0x01, (uint8_t)(-5), 7, (uint8_t)(-1), 0x00); /* motion + button */
        add_intr_event(1, 3, 1, 2, 0x00, 0x00, 0x00, 0x00, 0x00); /* decode.bad (len < 3) */
    } else if (c == 3) {
        /* Dual device: Port 1 = Keyboard (Slot 1), Port 2 = Tablet (Slot 2) */
        port_cfgs[1].connected = true;
        port_cfgs[1].enabled = true;
        port_cfgs[1].speed = 3;
        port_cfgs[1].b_max_packet_size0 = 64;
        port_cfgs[1].b_interface_class = 3;
        port_cfgs[1].b_interface_subclass = 0;
        port_cfgs[1].b_interface_protocol = 0;
        port_cfgs[1].ep_addr = 0x81;
        port_cfgs[1].ep_mps = 8;
        port_cfgs[1].ep_interval = 10;
        /* Slot 2 Tablet events (all_zero, button, motion, bad len) followed by Slot 1 Keyboard events */
        add_intr_event(2, 3, 1, 6, 0x00, 0x00, 0x00, 0x00, 0x00);
        add_intr_event(2, 3, 1, 6, 0x01, 0x10, 0x20, 0x30, 0x40);
        add_intr_event(2, 3, 1, 6, 0x00, 0x50, 0x20, 0x60, 0x40);
        add_intr_event(2, 3, 1, 2, 0x00, 0x00, 0x00, 0x00, 0x00);
        add_intr_event(1, 3, 1, 8, 0x00, 0x00, 0x04, 0x00, 0x00);
        add_intr_event(1, 3, 1, 8, 0x00, 0x00, 0x00, 0x00, 0x00);
    } else if (c == 4) {
        /* Dual device: Port 1 = Keyboard (Slot 1), Port 3 = Boot Mouse (Slot 2) */
        port_cfgs[2].connected = true;
        port_cfgs[2].enabled = true;
        port_cfgs[2].speed = 3;
        port_cfgs[2].b_max_packet_size0 = 8;
        port_cfgs[2].b_interface_class = 3;
        port_cfgs[2].b_interface_subclass = 1;
        port_cfgs[2].b_interface_protocol = 2;
        port_cfgs[2].ep_addr = 0x81;
        port_cfgs[2].ep_mps = 4;
        port_cfgs[2].ep_interval = 10;
        add_intr_event(2, 3, 1, 4, 0x01, 0x05, (uint8_t)(-3), 0x01, 0x00);
        add_intr_event(2, 3, 1, 2, 0x00, 0x00, 0x00, 0x00, 0x00);
        add_intr_event(1, 3, 1, 8, 0x00, 0x00, 0x05, 0x00, 0x00);
    } else if (c == 5) {
        /* Liveness kick (20000 empty polls) + unmatched event + bad trb_ptr + short_packet reject (0 payload) */
        cfg_empty_polls_before_first_event = 20001;
        intr_Empty_wait = 20001;
        /* Event 0: unmatched slot 7 */
        add_intr_event(7, 3, 1, 8, 0, 0, 0, 0, 0);
        /* Event 1: bad trb_ptr on slot 1 */
        add_intr_event(1, 3, 1, 8, 0, 0, 0, 0, 0);
        intr_events[intr_event_count - 1].use_custom_trb_ptr = true;
        intr_events[intr_event_count - 1].custom_trb_ptr = 0x100ULL;
        /* Event 2: short packet with 0 actual bytes (reject + stop) */
        add_intr_event(1, 3, 13, 0, 0, 0, 0, 0, 0);
    } else if (c == 6) {
        /* Residue > report_len -> residue.bad break */
        add_intr_event(1, 3, 1, 8, 0, 0, 0, 0, 0);
        intr_events[0].use_custom_residue = true;
        intr_events[0].custom_residue = 99u;
    } else if (c == 7) {
        /* Poll timeout (no events within HID_INTR_LIVENESS_BUDGET) */
        cfg_timeout_on_poll = true;
    } else if (c == 8) {
        /* BAR0 map failure after 3 retries */
        bar_map_fail_Remaining = 5;
    } else if (c == 9) {
        /* CSZ=1 unsupported */
        cfg_csz1 = true;
    } else if (c == 10) {
        /* No connected ports */
        port_cfgs[0].connected = false;
        port_cfgs[0].enabled = false;
    } else if (c == 11) {
        /* Non-HID device (Mass Storage class=8) */
        port_cfgs[0].b_interface_class = 8;
    }

    /* Populate Capability & Operational registers in xhci_bar */
    write_le32_raw(xhci_bar + 0x00, 0x01000000u | cfg_cap_length);
    write_le32_raw(xhci_bar + 0x04, (4u << 24) | 8u);
    write_le32_raw(xhci_bar + 0x10, cfg_csz1 ? (1u << 2) : 0u);
    write_le32_raw(xhci_bar + 0x14, 0x200u);
    write_le32_raw(xhci_bar + 0x18, 0x300u);

    uint64_t op_base = cfg_cap_length;
    write_le32_raw(xhci_bar + op_base + 0x00, 1u); /* Initially running so stop path is exercised */
    write_le32_raw(xhci_bar + op_base + 0x04, 0u);

    for (int p = 0; p < 4; ++p) {
        uint32_t portsc = 0;
        if (port_cfgs[p].connected) portsc |= 1u;
        if (port_cfgs[p].enabled) portsc |= 2u;
        portsc |= ((uint32_t)port_cfgs[p].speed << 10);
        write_le32_raw(xhci_bar + op_base + 0x400 + (uint64_t)p * 0x10ULL, portsc);
    }
}

int main(int argc, char **argv)
{
    int c = (argc > 1) ? atoi(argv[1]) : 0;
    setup_case(c);
    sexusb_main();
    finish_and_exit();
    return 0;
}
"""

RUST_WRAPPER_PREFIX = r"""
#![allow(dead_code, unused_mut, unused_variables, non_upper_case_globals)]

#[repr(C)]
#[derive(Copy, Clone)]
pub struct SexosReply {
    pub status: u64,
    pub value: u64,
}

extern "C" {
    fn sexos_volatile_read8(addr: *const u8) -> u8;
    fn sexos_volatile_read16(addr: *const u16) -> u16;
    fn sexos_volatile_read32(addr: *const u32) -> u32;
    fn sexos_volatile_read64(addr: *const u64) -> u64;
    fn sexos_volatile_write8(addr: *mut u8, value: u8);
    fn sexos_volatile_write16(addr: *mut u16, value: u16);
    fn sexos_volatile_write32(addr: *mut u32, value: u32);
    fn sexos_volatile_write64(addr: *mut u64, value: u64);
    fn sexos_map_pci_bar(cap_slot: u64, bar_index: u64, map_size: u64) -> u64;
    fn sexos_allocate_memory(size: u64) -> u64;
    fn sexos_map_memory(phys: u64, size: u64) -> u64;
    fn sexos_pdx_call(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> SexosReply;
    fn sexos_yield() -> u64;
}

trait HostedVolatile: Sized {
    unsafe fn hread(src: *const Self) -> Self;
    unsafe fn hwrite(dst: *mut Self, val: Self);
}

impl HostedVolatile for u8 {
    unsafe fn hread(src: *const Self) -> Self { unsafe { sexos_volatile_read8(src) } }
    unsafe fn hwrite(dst: *mut Self, val: Self) { unsafe { sexos_volatile_write8(dst, val) } }
}
impl HostedVolatile for u16 {
    unsafe fn hread(src: *const Self) -> Self { unsafe { sexos_volatile_read16(src) } }
    unsafe fn hwrite(dst: *mut Self, val: Self) { unsafe { sexos_volatile_write16(dst, val) } }
}
impl HostedVolatile for u32 {
    unsafe fn hread(src: *const Self) -> Self { unsafe { sexos_volatile_read32(src) } }
    unsafe fn hwrite(dst: *mut Self, val: Self) { unsafe { sexos_volatile_write32(dst, val) } }
}
impl HostedVolatile for u64 {
    unsafe fn hread(src: *const Self) -> Self { unsafe { sexos_volatile_read64(src) } }
    unsafe fn hwrite(dst: *mut Self, val: Self) { unsafe { sexos_volatile_write64(dst, val) } }
}

unsafe fn hosted_read_volatile<T: HostedVolatile>(src: *const T) -> T {
    unsafe { T::hread(src) }
}
unsafe fn hosted_write_volatile<T: HostedVolatile>(dst: *mut T, val: T) {
    unsafe { T::hwrite(dst, val) }
}

mod sex_pdx {
    use super::*;
    pub const SLOT_KERNEL: u64 = 0;
    pub const SLOT_USB_HOST: u64 = 8;
    pub const ERR_CAP_INVALID: u64 = 0xFFFF_FFFF_FFFF_FFFC;

    pub fn raw_print(s: &str) {
        unsafe {
            let _ = sexos_pdx_call(SLOT_KERNEL, 0x45, s.as_ptr() as u64, s.len() as u64, 0);
        }
    }

    pub fn pdx_call_checked(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> Result<u64, u64> {
        let r = unsafe { sexos_pdx_call(slot, opcode, arg0, arg1, arg2) };
        if r.status == 0 {
            Ok(r.value)
        } else {
            Err(r.status)
        }
    }

    pub fn sys_yield() {
        unsafe { sexos_yield(); }
    }

    macro_rules! serial_println_impl {
        ($($arg:tt)*) => {{
            let mut s = std::format!($($arg)*);
            s.push('\n');
            crate::sex_pdx::raw_print(&s);
        }};
    }
    pub(crate) use serial_println_impl as serial_println;
}
"""

MUTATIONS = (
    (
        "wrong_crcr_write64_high_before_low",
        "(call mmio_write32 op_base XHCI_CRCR (cast uint32_t (| cmd_ring_phys 1ULL)))\n        (call mmio_write32 op_base (+ XHCI_CRCR 4ULL) (cast uint32_t (>> (| cmd_ring_phys 1ULL) 32)))",
        "(call mmio_write32 op_base (+ XHCI_CRCR 4ULL) (cast uint32_t (>> (| cmd_ring_phys 1ULL) 32)))\n        (call mmio_write32 op_base XHCI_CRCR (cast uint32_t (| cmd_ring_phys 1ULL)))",
        0,
    ),
    (
        "wrong_xhci_usbcmd_offset",
        '(define XHCI_USBCMD "UINT64_C(0x00)")',
        '(define XHCI_USBCMD "UINT64_C(0x04)")',
        0,
    ),
    (
        "wrong_xhci_crcr_offset",
        '(define XHCI_CRCR "UINT64_C(0x18)")',
        '(define XHCI_CRCR "UINT64_C(0x20)")',
        0,
    ),
    (
        "wrong_xhci_dcbaap_offset",
        '(define XHCI_DCBAAP "UINT64_C(0x30)")',
        '(define XHCI_DCBAAP "UINT64_C(0x38)")',
        0,
    ),
    (
        "wrong_trb_type_enable_slot",
        '(define TRB_TYPE_ENABLE_SLOT_CMD "9u")',
        '(define TRB_TYPE_ENABLE_SLOT_CMD "10u")',
        0,
    ),
    (
        "wrong_trb_type_address_device",
        '(define TRB_TYPE_ADDRESS_DEVICE_CMD "11u")',
        '(define TRB_TYPE_ADDRESS_DEVICE_CMD "12u")',
        0,
    ),
    (
        "wrong_trb_type_configure_endpoint",
        '(define TRB_TYPE_CONFIGURE_ENDPOINT_CMD "12u")',
        '(define TRB_TYPE_CONFIGURE_ENDPOINT_CMD "13u")',
        0,
    ),
    (
        "wrong_trb_type_evaluate_context",
        '(define TRB_TYPE_EVALUATE_CONTEXT_CMD "13u")',
        '(define TRB_TYPE_EVALUATE_CONTEXT_CMD "14u")',
        0,
    ),
    (
        "wrong_portsc_stride",
        '(define PORTSC_STRIDE "UINT64_C(0x10)")',
        '(define PORTSC_STRIDE "UINT64_C(0x08)")',
        2,
    ),
    (
        "wrong_hid_inflight_count",
        '(define HID_INFLIGHT "UINT64_C(8)")',
        '(define HID_INFLIGHT "UINT64_C(4)")',
        0,
    ),
    (
        "wrong_op_usb_keyboard_report",
        '(define OP_USB_KEYBOARD_REPORT "UINT64_C(0x261)")',
        '(define OP_USB_KEYBOARD_REPORT "UINT64_C(0x262)")',
        0,
    ),
    (
        "wrong_op_usb_mouse_report",
        '(define OP_USB_MOUSE_REPORT "UINT64_C(0x260)")',
        '(define OP_USB_MOUSE_REPORT "UINT64_C(0x262)")',
        0,
    ),
    (
        "wrong_tablet_button_mask",
        "(= (-> out buttons) (& ([] buf 0) 0x07u))",
        "(= (-> out buttons) (& ([] buf 0) 0x03u))",
        1,
    ),
    (
        "wrong_tablet_abs_x_shift",
        "(var uint16_t abs_x (| (cast uint16_t ([] buf 1)) (cast uint16_t (<< (cast uint16_t ([] buf 2)) 8))))",
        "(var uint16_t abs_x (| (cast uint16_t ([] buf 1)) (cast uint16_t (<< (cast uint16_t ([] buf 2)) 4))))",
        1,
    ),
    (
        "wrong_boot_mouse_wheel_index",
        "(var int8_t wheel (?: (>= len 4) (call wrap_i8 ([] buf 3)) 0))",
        "(var int8_t wheel (?: (>= len 4) (call wrap_i8 ([] buf 2)) 0))",
        2,
    ),
)


def clean_env(extra=None):
    env = {k: v for k, v in os.environ.items() if k not in ENV_FLAGS}
    if extra:
        env.update(extra)
    return env


def run_cmd(cmd, *, cwd=HERE, env=None):
    proc = subprocess.run(cmd, cwd=cwd, env=clean_env(env), capture_output=True, text=True)
    if proc.returncode != 0:
        sys.stderr.write(f"CMD FAILED: {cmd}\nSTDOUT:\n{proc.stdout}\nSTDERR:\n{proc.stderr}\n")
        proc.check_returncode()
    return proc


def build_hosted_rust_source():
    raw = run_cmd(["git", "-C", str(REPO), "show", f"{UPSTREAM_REV}:servers/sexusb/src/main.rs"]).stdout
    sha = hashlib.sha256(raw.encode("utf-8")).hexdigest()
    if EXPECTED_RUST_SHA256 is not None and sha != EXPECTED_RUST_SHA256:
        raise AssertionError(f"unexpected Rust SHA256: {sha} != {EXPECTED_RUST_SHA256}")

    src = raw
    src = src.replace("#![no_std]", "")
    src = src.replace("#![no_main]", "")
    src = re.sub(
        r"#\[panic_handler\]\s*fn panic\(_: &core::panic::PanicInfo\) -> ! \{\s*loop \{ sys_yield\(\); \}\s*\}",
        "",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = re.sub(
        r"fn map_xhci_bar0\(map_size: u64\) -> u64 \{.*?\n\}",
        "fn map_xhci_bar0(map_size: u64) -> u64 { unsafe { sexos_map_pci_bar(SLOT_USB_HOST, 0, map_size) } }",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = re.sub(
        r"fn sys_alloc_phys\(size: u64\) -> u64 \{.*?\n\}",
        "fn sys_alloc_phys(size: u64) -> u64 { unsafe { sexos_allocate_memory(size) } }",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = re.sub(
        r"fn sys_map_phys\(phys: u64, size: u64\) -> u64 \{.*?\n\}",
        "fn sys_map_phys(phys: u64, size: u64) -> u64 { unsafe { sexos_map_memory(phys, size) } }",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = src.replace("core::ptr::read_volatile(", "hosted_read_volatile(")
    src = src.replace("core::ptr::write_volatile(", "hosted_write_volatile(")
    src = src.replace('pub extern "C" fn _start() -> !', 'pub extern "C" fn sexusb_main()')
    return RUST_WRAPPER_PREFIX + "\n" + src, sha


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cases", type=int, default=8)
    parser.add_argument("--all-variants", action="store_true")
    args = parser.parse_args()

    cesta_bin = Path(shutil.which(str(args.cesta)) or args.cesta).resolve()
    cesta_src = HERE / "src/sexusb.cesta"
    exported_c = HERE / "c/src/sexusb.c"

    cesta_text = LayoutText(cesta_src.read_text())
    fn_split = cesta_text.split("(fn ", 1)[1]
    if "cpp-if" in fn_split:
        raise AssertionError("cpp-if found inside function bodies of src/sexusb.cesta")

    variants = ALL_VARIANTS if args.all_variants else CORE_VARIANTS

    with tempfile.TemporaryDirectory() as tmp:
        tmpdir = Path(tmp)
        provider_c = tmpdir / "provider.c"
        provider_c.write_text(PROVIDER_C)
        rust_rs = tmpdir / "rust_sexusb.rs"
        rust_code, rust_sha = build_hosted_rust_source()
        rust_rs.write_text(rust_code)

        authored_c = tmpdir / "sexusb_authored.c"
        exported = run_cmd([str(cesta_bin), "export-c", str(cesta_src), "--"]).stdout
        authored_c.write_text(exported)
        if exported_c.exists() and exported_c.read_text().strip() != exported.strip():
            raise AssertionError("c/src/sexusb.c is out of date with src/sexusb.cesta")

        common_c_sources = [
            str(HERE / "runtime/client_runtime.c"),
            str(HERE / "runtime/server_runtime.c"),
            str(HERE / "generated/runtime/format_bytes.c"),
            str(HERE / "generated/runtime/packed_words.c"),
        ]
        inc_flags = ["-I", str(HERE / "generated/include"), "-DSEXOS_HOST=1"]

        total_lines = 0
        for vname, flags in variants:
            dflags = [f"-D{f}=1" for f in flags]
            rust_env = {f: "1" for f in flags}

            rust_lib = tmpdir / f"librust_{vname}.a"
            run_cmd(
                ["rustc", "--edition=2021", "-O", "--crate-type=staticlib", str(rust_rs), "-o", str(rust_lib)],
                env=rust_env,
            )
            rust_bin = tmpdir / f"rust_{vname}"
            run_cmd(
                ["gcc", "-O2", *inc_flags, *dflags, str(provider_c), str(rust_lib), "-lpthread", "-ldl", "-o", str(rust_bin)]
            )

            gcc_auth = tmpdir / f"gcc_auth_{vname}"
            run_cmd(
                [
                    "gcc",
                    "-std=c11",
                    "-O2",
                    "-fsanitize=address,undefined",
                    "-fno-sanitize-recover=all",
                    *inc_flags,
                    *dflags,
                    str(provider_c),
                    str(authored_c),
                    *common_c_sources,
                    "-o",
                    str(gcc_auth),
                ]
            )

            clang_exp = tmpdir / f"clang_exp_{vname}"
            run_cmd(
                [
                    "clang",
                    "-std=c11",
                    "-O2",
                    *inc_flags,
                    *dflags,
                    str(provider_c),
                    str(exported_c if exported_c.exists() else authored_c),
                    *common_c_sources,
                    "-o",
                    str(clang_exp),
                ]
            )

            for c in range(args.cases):
                r_out = run_cmd([str(rust_bin), str(c)]).stdout
                g_out = run_cmd([str(gcc_auth), str(c)]).stdout
                cl_out = run_cmd([str(clang_exp), str(c)]).stdout
                if r_out != g_out or r_out != cl_out:
                    r_lines = r_out.splitlines()
                    g_lines = g_out.splitlines()
                    for idx, (rl, gl) in enumerate(zip(r_lines, g_lines)):
                        if rl != gl:
                            sys.stderr.write(f"First diff at line {idx + 1} (variant={vname} case={c}):\n  RUST:  {rl}\n  CESTA: {gl}\n")
                            break
                    else:
                        sys.stderr.write(f"Length diff (variant={vname} case={c}): rust={len(r_lines)} cesta={len(g_lines)}\n")
                    raise AssertionError(f"trace mismatch in variant={vname} case={c}")
                total_lines += len(r_out.splitlines())

        ref_bin = tmpdir / "rust_default"
        ref_traces = {c: run_cmd([str(ref_bin), str(c)]).stdout for c in range(3)}
        killed = 0
        for mname, target, replacement, case_id in MUTATIONS:
            if cesta_text.count(target) == 0:
                raise AssertionError(f"mutation target not found for {mname}")
            mut_cesta = tmpdir / f"mut_{mname}.cesta"
            mut_cesta.write_text(cesta_text.replace(target, replacement))
            mut_c = tmpdir / f"mut_{mname}.c"
            mut_c.write_text(run_cmd([str(cesta_bin), "export-c", str(mut_cesta), "--"]).stdout)
            mut_bin = tmpdir / f"mut_{mname}"
            run_cmd(
                [
                    "gcc",
                    "-std=c11",
                    "-O2",
                    *inc_flags,
                    str(provider_c),
                    str(mut_c),
                    *common_c_sources,
                    "-o",
                    str(mut_bin),
                ]
            )
            proc = subprocess.run([str(mut_bin), str(case_id)], capture_output=True, text=True)
            if proc.returncode != 0 or proc.stdout != ref_traces[case_id]:
                killed += 1
            else:
                raise AssertionError(f"mutation {mname} survived on case {case_id}")

        print(
            f"sexusb differential: ok ({len(variants)} variants x {args.cases} cases, "
            f"{total_lines} trace lines, {killed}/{len(MUTATIONS)} mutations killed, "
            f"rust_sha256={rust_sha[:16]})"
        )


if __name__ == "__main__":
    main()
