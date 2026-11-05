#!/usr/bin/env python3
"""Differential & mutation verifier for servers/sexnet (e1000 NIC + ARP/IPv4/ICMP/UDP/TCP/HTTP/DNS)."""

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
EXPECTED_RUST_SHA256 = "2de8937d443ec87797e2247d545c336910dc23ac8ae21b936d4a506518dec926"

ENV_FLAGS = (
    "SEXOS_SEXNET_PROOF_PROFILE",
    "SEXNET_DNS_SOURCE3_PROOF",
    "SEXNET_PHASE_M_RELIABILITY_PROOF",
)

CORE_VARIANTS = (
    ("default", ()),
    ("proof_profile", ("SEXOS_SEXNET_PROOF_PROFILE",)),
)

ALL_VARIANTS = CORE_VARIANTS + (
    ("proof_plus_dns", ("SEXOS_SEXNET_PROOF_PROFILE", "SEXNET_DNS_SOURCE3_PROOF")),
    ("proof_plus_phase_m", ("SEXOS_SEXNET_PROOF_PROFILE", "SEXNET_PHASE_M_RELIABILITY_PROOF")),
    (
        "all_flags",
        (
            "SEXOS_SEXNET_PROOF_PROFILE",
            "SEXNET_DNS_SOURCE3_PROOF",
            "SEXNET_PHASE_M_RELIABILITY_PROOF",
        ),
    ),
)

PROVIDER_C = r"""
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sexos_mmio.h"

extern void sexnet_main(void);

#define FAKE_NIC_BAR_VA UINT64_C(0x72000000)
#define FAKE_PHYS_BASE  UINT64_C(0x100000)
#define MAX_DMA_PAGES   64

typedef struct {
    uint64_t phys;
    uint8_t *mem;
    uint64_t va;
    bool mapped;
} DmaPage;

static DmaPage dma_pages[MAX_DMA_PAGES];
static size_t dma_page_count = 0;
static uint8_t nic_bar[0x10000];

static int current_case = 0;
static bool cfg_bar_map_fail = false;
static uint32_t cfg_alloc_fail_after = 0xFFFFFFFFu;
static uint32_t alloc_call_count = 0;
static uint32_t cfg_tx_delay_polls = 0;
static uint32_t cfg_rx_delay_polls = 0;
static uint32_t cur_tx_delay = 0;
static uint32_t cur_rx_delay = 0;

static uint64_t mmio_order = 0;
static uint64_t dma_reads = 0;
static uint64_t dma_writes = 0;
static uint64_t dma_fnv = UINT64_C(0xcbf29ce484222325);

static void fnv_mix(uint64_t v)
{
    dma_fnv ^= v;
    dma_fnv *= UINT64_C(0x100000001b3);
}

static uint32_t bar_read_le32(uint32_t off)
{
    uint32_t v = 0;
    memcpy(&v, &nic_bar[off], 4);
    return v;
}

static void bar_write_le32(uint32_t off, uint32_t v)
{
    memcpy(&nic_bar[off], &v, 4);
}

static uint8_t *phys_to_ptr(uint64_t phys)
{
    for (size_t i = 0; i < dma_page_count; ++i) {
        if (dma_pages[i].phys == phys) {
            return dma_pages[i].mem;
        }
    }
    return NULL;
}

static DmaPage *va_to_page(uint64_t va, size_t *off_out)
{
    for (size_t i = 0; i < dma_page_count; ++i) {
        if (dma_pages[i].mapped && va >= dma_pages[i].va && va < dma_pages[i].va + 4096ULL) {
            *off_out = (size_t)(va - dma_pages[i].va);
            return &dma_pages[i];
        }
    }
    return NULL;
}

/* RX frame queue per poll phase */
typedef enum {
    PHASE_NONE = 0,
    PHASE_RX_OBSERVE,
    PHASE_RX_PERMANENT,
    PHASE_ARP_RX,
    PHASE_ARP_CACHE,
    PHASE_L2_RX,
    PHASE_IPV4_RX,
    PHASE_TCP_RETRY,
    PHASE_HTTP_RX,
    PHASE_DNS_RX,
    PHASE_M_ITER0,
    PHASE_M_ITER1,
    PHASE_M_ITER2,
    PHASE_COUNT
} PollPhase;

typedef struct {
    uint8_t bytes[256];
    uint16_t len;
} RxFrame;

typedef struct {
    RxFrame frames[16];
    size_t count;
    size_t idx;
} PhaseQueue;

static PhaseQueue phase_queues[PHASE_COUNT];
static PollPhase active_phase = PHASE_NONE;
static uint32_t rx_hw_next_slot = 0;
static uint8_t *pending_tx_sta_ptr = NULL;

static void enqueue_frame(PollPhase ph, const uint8_t *data, uint16_t len)
{
    PhaseQueue *q = &phase_queues[ph];
    if (q->count >= 16) return;
    RxFrame *f = &q->frames[q->count++];
    memset(f->bytes, 0, sizeof(f->bytes));
    if (len > sizeof(f->bytes)) len = sizeof(f->bytes);
    memcpy(f->bytes, data, len);
    f->len = len;
}

static uint16_t ones_complement_csum(const uint8_t *buf, size_t len, uint32_t init_sum)
{
    uint32_t sum = init_sum;
    for (size_t i = 0; i + 1 < len; i += 2) {
        uint16_t w = (uint16_t)(((uint16_t)buf[i] << 8) | (uint16_t)buf[i + 1]);
        sum += (uint32_t)w;
    }
    if (len & 1) {
        uint16_t w = (uint16_t)((uint16_t)buf[len - 1] << 8);
        sum += (uint32_t)w;
    }
    while ((sum >> 16) != 0) {
        sum = (sum & 0xFFFFu) + (sum >> 16);
    }
    return (uint16_t)(~sum & 0xFFFFu);
}

static void build_arp_request(uint8_t *out, const uint8_t sender_mac[6], const uint8_t sender_ip[4], const uint8_t target_ip[4])
{
    memset(out, 0, 60);
    memset(&out[0], 0xFF, 6);
    memcpy(&out[6], sender_mac, 6);
    out[12] = 0x08; out[13] = 0x06;
    out[14] = 0x00; out[15] = 0x01; /* htype = 1 */
    out[16] = 0x08; out[17] = 0x00; /* ptype = 0x0800 */
    out[18] = 6;    out[19] = 4;
    out[20] = 0x00; out[21] = 0x01; /* oper = 1 */
    memcpy(&out[22], sender_mac, 6);
    memcpy(&out[28], sender_ip, 4);
    memset(&out[32], 0, 6);
    memcpy(&out[38], target_ip, 4);
}

static uint16_t build_ipv4_header(uint8_t *pkt, uint8_t ver_ihl, uint16_t total_len, uint16_t flags_frag,
                                  uint8_t ttl, uint8_t proto, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                                  bool bad_csum)
{
    static const uint8_t dst_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
    static const uint8_t src_mac[6] = {0x52, 0x55, 0x0A, 0x00, 0x02, 0x02};
    memcpy(&pkt[0], dst_mac, 6);
    memcpy(&pkt[6], src_mac, 6);
    pkt[12] = 0x08; pkt[13] = 0x00;
    pkt[14] = ver_ihl;
    pkt[15] = 0x00;
    pkt[16] = (uint8_t)(total_len >> 8);
    pkt[17] = (uint8_t)(total_len & 0xFF);
    pkt[18] = 0x12; pkt[19] = 0x34;
    pkt[20] = (uint8_t)(flags_frag >> 8);
    pkt[21] = (uint8_t)(flags_frag & 0xFF);
    pkt[22] = ttl;
    pkt[23] = proto;
    pkt[24] = 0; pkt[25] = 0;
    memcpy(&pkt[26], src_ip, 4);
    memcpy(&pkt[30], dst_ip, 4);
    uint16_t csum = ones_complement_csum(&pkt[14], 20, 0);
    if (bad_csum) csum ^= 0x1234u;
    pkt[24] = (uint8_t)(csum >> 8);
    pkt[25] = (uint8_t)(csum & 0xFF);
    return (uint16_t)(14 + total_len);
}

static uint16_t build_icmp_frame(uint8_t *pkt, uint8_t icmp_type, uint8_t icmp_code, bool bad_icmp_csum)
{
    static const uint8_t src_ip[4] = {10, 0, 2, 2};
    static const uint8_t dst_ip[4] = {10, 0, 2, 15};
    memset(pkt, 0, 64);
    build_ipv4_header(pkt, 0x45, 32, 0, 64, 1, src_ip, dst_ip, false);
    pkt[34] = icmp_type;
    pkt[35] = icmp_code;
    pkt[36] = 0; pkt[37] = 0;
    pkt[38] = 0x00; pkt[39] = 0x01;
    pkt[40] = 0x00; pkt[41] = 0x01;
    pkt[42] = 'p'; pkt[43] = 'i'; pkt[44] = 'n'; pkt[45] = 'g';
    uint16_t csum = ones_complement_csum(&pkt[34], 12, 0);
    if (bad_icmp_csum) csum ^= 0xFFFFu;
    pkt[36] = (uint8_t)(csum >> 8);
    pkt[37] = (uint8_t)(csum & 0xFF);
    return 60;
}

static uint16_t build_udp_frame(uint8_t *pkt, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                                uint16_t src_port, uint16_t dst_port,
                                const uint8_t *payload, uint16_t payload_len,
                                int csum_mode, uint16_t override_udp_len, uint16_t override_ip_total_len)
{
    uint16_t udp_len = (override_udp_len != 0) ? override_udp_len : (uint16_t)(8 + payload_len);
    uint16_t ip_total = (override_ip_total_len != 0) ? override_ip_total_len : (uint16_t)(20 + 8 + payload_len);
    memset(pkt, 0, 256);
    build_ipv4_header(pkt, 0x45, ip_total, 0, 64, 17, src_ip, dst_ip, false);
    pkt[34] = (uint8_t)(src_port >> 8);
    pkt[35] = (uint8_t)(src_port & 0xFF);
    pkt[36] = (uint8_t)(dst_port >> 8);
    pkt[37] = (uint8_t)(dst_port & 0xFF);
    pkt[38] = (uint8_t)(udp_len >> 8);
    pkt[39] = (uint8_t)(udp_len & 0xFF);
    pkt[40] = 0; pkt[41] = 0;
    if (payload_len > 0) {
        memcpy(&pkt[42], payload, payload_len);
    }
    if (csum_mode != 0) {
        uint32_t pseudo = 0;
        pseudo += ((uint32_t)src_ip[0] << 8) | (uint32_t)src_ip[1];
        pseudo += ((uint32_t)src_ip[2] << 8) | (uint32_t)src_ip[3];
        pseudo += ((uint32_t)dst_ip[0] << 8) | (uint32_t)dst_ip[1];
        pseudo += ((uint32_t)dst_ip[2] << 8) | (uint32_t)dst_ip[3];
        pseudo += 17u;
        pseudo += (uint32_t)udp_len;
        uint16_t csum = ones_complement_csum(&pkt[34], (size_t)(8 + payload_len), pseudo);
        if (csum == 0) csum = 0xFFFFu;
        if (csum_mode < 0) csum ^= 0x1111u;
        pkt[40] = (uint8_t)(csum >> 8);
        pkt[41] = (uint8_t)(csum & 0xFF);
    }
    uint16_t frame_len = (uint16_t)(14 + 20 + 8 + payload_len);
    return (frame_len < 60) ? 60 : frame_len;
}

static uint16_t build_tcp_frame(uint8_t *pkt, uint16_t src_port, uint16_t dst_port,
                                uint32_t seq, uint32_t ack, uint8_t data_offset_byte, uint8_t flags,
                                const uint8_t *payload, uint16_t payload_len, bool bad_csum)
{
    static const uint8_t src_ip[4] = {10, 0, 2, 2};
    static const uint8_t dst_ip[4] = {10, 0, 2, 15};
    uint16_t tcp_len = (uint16_t)(20 + payload_len);
    uint16_t ip_total = (uint16_t)(20 + tcp_len);
    memset(pkt, 0, 256);
    build_ipv4_header(pkt, 0x45, ip_total, 0, 64, 6, src_ip, dst_ip, false);
    pkt[34] = (uint8_t)(src_port >> 8);
    pkt[35] = (uint8_t)(src_port & 0xFF);
    pkt[36] = (uint8_t)(dst_port >> 8);
    pkt[37] = (uint8_t)(dst_port & 0xFF);
    pkt[38] = (uint8_t)(seq >> 24);
    pkt[39] = (uint8_t)((seq >> 16) & 0xFF);
    pkt[40] = (uint8_t)((seq >> 8) & 0xFF);
    pkt[41] = (uint8_t)(seq & 0xFF);
    pkt[42] = (uint8_t)(ack >> 24);
    pkt[43] = (uint8_t)((ack >> 16) & 0xFF);
    pkt[44] = (uint8_t)((ack >> 8) & 0xFF);
    pkt[45] = (uint8_t)(ack & 0xFF);
    pkt[46] = data_offset_byte;
    pkt[47] = flags;
    pkt[48] = 0xFF; pkt[49] = 0xFF;
    pkt[50] = 0;    pkt[51] = 0;
    pkt[52] = 0;    pkt[53] = 0;
    if (payload_len > 0) {
        memcpy(&pkt[54], payload, payload_len);
    }
    uint32_t pseudo = 0;
    pseudo += ((uint32_t)src_ip[0] << 8) | (uint32_t)src_ip[1];
    pseudo += ((uint32_t)src_ip[2] << 8) | (uint32_t)src_ip[3];
    pseudo += ((uint32_t)dst_ip[0] << 8) | (uint32_t)dst_ip[1];
    pseudo += ((uint32_t)dst_ip[2] << 8) | (uint32_t)dst_ip[3];
    pseudo += 6u;
    pseudo += (uint32_t)tcp_len;
    uint16_t csum = ones_complement_csum(&pkt[34], tcp_len, pseudo);
    if (bad_csum) csum ^= 0x2222u;
    pkt[50] = (uint8_t)(csum >> 8);
    pkt[51] = (uint8_t)(csum & 0xFF);
    return (uint16_t)(14 + ip_total);
}

static uint16_t build_dns_response(uint8_t *pkt, uint16_t txid, uint16_t flags,
                                   uint16_t qdcount, uint16_t ancount, uint16_t rdlen)
{
    static const uint8_t dns_ip[4] = {10, 0, 2, 3};
    static const uint8_t guest_ip[4] = {10, 0, 2, 15};
    uint8_t dns[64];
    memset(dns, 0, sizeof(dns));
    dns[0] = (uint8_t)(txid >> 8);
    dns[1] = (uint8_t)(txid & 0xFF);
    dns[2] = (uint8_t)(flags >> 8);
    dns[3] = (uint8_t)(flags & 0xFF);
    dns[4] = (uint8_t)(qdcount >> 8);
    dns[5] = (uint8_t)(qdcount & 0xFF);
    dns[6] = (uint8_t)(ancount >> 8);
    dns[7] = (uint8_t)(ancount & 0xFF);
    /* Question: \x07example\x03com\x00 + qtype=1 + qclass=1 (17 bytes: 12..29) */
    static const uint8_t qsect[17] = {
        7, 'e', 'x', 'a', 'm', 'p', 'l', 'e',
        3, 'c', 'o', 'm', 0,
        0, 1, 0, 1
    };
    memcpy(&dns[12], qsect, 17);
    /* Answer at 29..45 (16 bytes): ptr 0xC00C, type=1, class=1, ttl=300, rdlen, 93.184.216.34 */
    dns[29] = 0xC0; dns[30] = 0x0C;
    dns[31] = 0x00; dns[32] = 0x01;
    dns[33] = 0x00; dns[34] = 0x01;
    dns[35] = 0x00; dns[36] = 0x00; dns[37] = 0x01; dns[38] = 0x2C;
    dns[39] = (uint8_t)(rdlen >> 8);
    dns[40] = (uint8_t)(rdlen & 0xFF);
    dns[41] = 93; dns[42] = 184; dns[43] = 216; dns[44] = 34;
    return build_udp_frame(pkt, dns_ip, guest_ip, 53, 49152, dns, 45, 0, 0, 0);
}

uint64_t sexos_map_pci_bar(uint64_t cap_slot, uint64_t bar_index, uint64_t map_size)
{
    printf("MAP_BAR|slot=%llu|bar=%llu|size=%#llx\n",
           (unsigned long long)cap_slot,
           (unsigned long long)bar_index,
           (unsigned long long)map_size);
    if (cfg_bar_map_fail) {
        return UINT64_MAX;
    }
    return FAKE_NIC_BAR_VA;
}

uint64_t sexos_allocate_memory(uint64_t size)
{
    alloc_call_count++;
    if (alloc_call_count > cfg_alloc_fail_after) {
        printf("ALLOC_PHYS|size=%llu|ret=0\n", (unsigned long long)size);
        return 0;
    }
    if (dma_page_count >= MAX_DMA_PAGES) {
        return 0;
    }
    uint64_t phys = FAKE_PHYS_BASE + (uint64_t)dma_page_count * 4096ULL;
    uint8_t *mem = (uint8_t *)calloc(1, 4096);
    dma_pages[dma_page_count].phys = phys;
    dma_pages[dma_page_count].mem = mem;
    dma_pages[dma_page_count].va = (uint64_t)(uintptr_t)mem;
    dma_pages[dma_page_count].mapped = false;
    dma_page_count++;
    printf("ALLOC_PHYS|size=%llu|phys=%#llx\n", (unsigned long long)size, (unsigned long long)phys);
    return phys;
}

uint64_t sexos_map_memory(uint64_t phys, uint64_t size)
{
    (void)size;
    for (size_t i = 0; i < dma_page_count; ++i) {
        if (dma_pages[i].phys == phys) {
            dma_pages[i].mapped = true;
            return dma_pages[i].va;
        }
    }
    return 0;
}

uint64_t sexos_net_diag(uint64_t selector)
{
    uint64_t ret = 0;
    if (current_case == 0) {
        /* source = 2, status = 200, bytes = 13 ("Hello Domain\n") */
        static const char body[13] = "Hello Domain\n";
        if (selector == 0) {
            ret = (200ULL << 32) | (2ULL << 16) | 13ULL;
        } else if (selector == 1) {
            ret = 13ULL;
        } else if (selector >= 2 && selector <= 3) {
            size_t base = (size_t)(selector - 2) * 8;
            uint64_t packed = 0;
            for (size_t i = 0; i < 8 && (base + i) < 13; ++i) {
                packed |= ((uint64_t)(uint8_t)body[base + i]) << (i * 8);
            }
            ret = packed;
        }
    } else if (current_case == 1) {
        if (selector == 0) {
            ret = (200ULL << 32) | (1ULL << 16) | 19ULL;
        }
    } else {
        if (selector == 0) {
            ret = (200ULL << 32) | (3ULL << 16) | 13ULL;
        }
    }
    printf("NET_DIAG|sel=%llu|ret=%#llx\n", (unsigned long long)selector, (unsigned long long)ret);
    return ret;
}

static void maybe_populate_rx_desc(DmaPage *pg, size_t off)
{
    if (active_phase == PHASE_NONE) return;
    uint32_t rctl = bar_read_le32(0x0100);
    if ((rctl & 2u) == 0) return;
    uint32_t rdbal = bar_read_le32(0x2800);
    if ((uint32_t)pg->phys != rdbal) return;
    if (off >= 128 || (off % 16) != 12) return;
    uint32_t slot = (uint32_t)(off / 16);
    if (slot != rx_hw_next_slot) return;
    if (pg->mem[off] != 0) return;

    PhaseQueue *q = &phase_queues[active_phase];
    if (q->idx >= q->count) return;

    if (cur_rx_delay > 0) {
        cur_rx_delay--;
        return;
    }
    cur_rx_delay = cfg_rx_delay_polls;

    RxFrame *f = &q->frames[q->idx++];
    uint64_t buf_phys = 0;
    memcpy(&buf_phys, &pg->mem[slot * 16], 8);
    uint8_t *pkt_mem = phys_to_ptr(buf_phys);
    if (pkt_mem != NULL) {
        memcpy(pkt_mem, f->bytes, f->len);
    }
    memcpy(&pg->mem[slot * 16 + 8], &f->len, 2);
    pg->mem[slot * 16 + 12] = 0x03u; /* DD | EOP */
    rx_hw_next_slot = (rx_hw_next_slot + 1u) & 7u;
}

uint8_t sexos_volatile_read8(const uint8_t *addr)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    size_t off = 0;
    DmaPage *pg = va_to_page(va, &off);
    if (pg != NULL) {
        if (pending_tx_sta_ptr == &pg->mem[off]) {
            if (cur_tx_delay > 0) {
                cur_tx_delay--;
            } else {
                *pending_tx_sta_ptr |= 1u;
                pending_tx_sta_ptr = NULL;
            }
        }
        maybe_populate_rx_desc(pg, off);
        uint8_t v = pg->mem[off];
        dma_reads++;
        fnv_mix(v);
        return v;
    }
    return *addr;
}

uint16_t sexos_volatile_read16(const uint16_t *addr)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    size_t off = 0;
    DmaPage *pg = va_to_page(va, &off);
    if (pg != NULL) {
        uint16_t v = 0;
        memcpy(&v, &pg->mem[off], 2);
        dma_reads++;
        fnv_mix(v);
        return v;
    }
    return *addr;
}

uint32_t sexos_volatile_read32(const uint32_t *addr)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    if (va >= FAKE_NIC_BAR_VA && va < FAKE_NIC_BAR_VA + 0x10000ULL) {
        uint32_t off = (uint32_t)(va - FAKE_NIC_BAR_VA);
        uint32_t v = bar_read_le32(off);
        if (off == 0x00C0u) {
            bar_write_le32(off, 0u);
        }
        mmio_order++;
        printf("MMIO_R32|off=%#06x|val=%#010x\n", off, v);
        return v;
    }
    return *addr;
}

uint64_t sexos_volatile_read64(const uint64_t *addr)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    size_t off = 0;
    DmaPage *pg = va_to_page(va, &off);
    if (pg != NULL) {
        uint64_t v = 0;
        memcpy(&v, &pg->mem[off], 8);
        dma_reads++;
        fnv_mix(v);
        return v;
    }
    return *addr;
}

void sexos_volatile_write8(uint8_t *addr, uint8_t value)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    size_t off = 0;
    DmaPage *pg = va_to_page(va, &off);
    if (pg != NULL) {
        pg->mem[off] = value;
        dma_writes++;
        fnv_mix(value);
        return;
    }
    *addr = value;
}

void sexos_volatile_write16(uint16_t *addr, uint16_t value)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    size_t off = 0;
    DmaPage *pg = va_to_page(va, &off);
    if (pg != NULL) {
        memcpy(&pg->mem[off], &value, 2);
        dma_writes++;
        fnv_mix(value);
        return;
    }
    *addr = value;
}

void sexos_volatile_write32(uint32_t *addr, uint32_t value)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    if (va >= FAKE_NIC_BAR_VA && va < FAKE_NIC_BAR_VA + 0x10000ULL) {
        uint32_t off = (uint32_t)(va - FAKE_NIC_BAR_VA);
        mmio_order++;
        printf("MMIO_W32|off=%#06x|val=%#010x\n", off, value);
        if (off == 0x0000u && (value & (1u << 26)) != 0) {
            value &= ~(1u << 26);
        }
        bar_write_le32(off, value);
        if (off == 0x2810u && value == 0u) {
            rx_hw_next_slot = 0;
        }
        if (off == 0x3818u) {
            uint32_t tctl = bar_read_le32(0x0400u);
            uint32_t tdbal = bar_read_le32(0x3800u);
            uint8_t *tx_ring = phys_to_ptr((uint64_t)tdbal);
            if ((tctl & 2u) != 0 && tx_ring != NULL) {
                uint32_t slot = (value - 1u) & 7u;
                uint64_t frame_phys = 0;
                uint16_t frame_len = 0;
                uint8_t cmd = tx_ring[slot * 16 + 11];
                memcpy(&frame_phys, &tx_ring[slot * 16], 8);
                memcpy(&frame_len, &tx_ring[slot * 16 + 8], 2);
                uint8_t *frame_ptr = phys_to_ptr(frame_phys);
                uint64_t frame_hash = 0;
                if (frame_ptr != NULL && frame_len <= 4096) {
                    for (uint16_t i = 0; i < frame_len; ++i) {
                        frame_hash = (frame_hash * 131ULL) ^ frame_ptr[i];
                    }
                }
                printf("TX_POST|tdt=%u|slot=%u|len=%u|cmd=%#04x|fhash=%#018llx\n",
                       value, slot, frame_len, cmd, (unsigned long long)frame_hash);
                if (cfg_tx_delay_polls == 0) {
                    tx_ring[slot * 16 + 12] |= 1u;
                    pending_tx_sta_ptr = NULL;
                } else {
                    pending_tx_sta_ptr = &tx_ring[slot * 16 + 12];
                    cur_tx_delay = cfg_tx_delay_polls;
                }
            }
        }
        return;
    }
    *addr = value;
}

void sexos_volatile_write64(uint64_t *addr, uint64_t value)
{
    uint64_t va = (uint64_t)(uintptr_t)addr;
    size_t off = 0;
    DmaPage *pg = va_to_page(va, &off);
    if (pg != NULL) {
        memcpy(&pg->mem[off], &value, 8);
        dma_writes++;
        fnv_mix(value);
        return;
    }
    *addr = value;
}

void sexos_compiler_fence_seq_cst(void) {}
void sexos_sfence(void) {}

static void detect_phase_marker(const char *s)
{
    if (strstr(s, "[sexnet.nic.rx.observe.poll.begin]") != NULL) {
        active_phase = PHASE_RX_OBSERVE;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.nic.rx.permanent.poll.begin]") != NULL) {
        active_phase = PHASE_RX_PERMANENT;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.arp.rx.poll.begin]") != NULL) {
        active_phase = PHASE_ARP_RX;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.arp.cache.poll.begin]") != NULL) {
        active_phase = PHASE_ARP_CACHE;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.l2.rx.poll.begin]") != NULL) {
        active_phase = PHASE_L2_RX;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.ipv4.rx.poll.begin]") != NULL) {
        active_phase = PHASE_IPV4_RX;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.tcp.synack.retry.poll.begin]") != NULL) {
        active_phase = PHASE_TCP_RETRY;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.tcp.payload.tx.proof.done]") != NULL) {
        active_phase = PHASE_HTTP_RX;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.dns.source3.udp.tx]") != NULL) {
        active_phase = PHASE_DNS_RX;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.phaseM.multi.debug] iter=1") != NULL) {
        active_phase = PHASE_M_ITER1;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.phaseM.multi.debug] iter=2") != NULL) {
        active_phase = PHASE_M_ITER2;
        cur_rx_delay = cfg_rx_delay_polls;
    } else if (strstr(s, "[sexnet.nic.rx.observe.poll.done]") != NULL ||
               strstr(s, "[sexnet.nic.rx.permanent.poll.done]") != NULL ||
               strstr(s, "[sexnet.arp.rx.poll.done]") != NULL ||
               strstr(s, "[sexnet.arp.cache.poll.done]") != NULL ||
               strstr(s, "[sexnet.l2.rx.poll.done]") != NULL ||
               strstr(s, "[sexnet.ipv4.rx.poll.done]") != NULL ||
               strstr(s, "[sexnet.tcp.synack.retry.poll.done]") != NULL ||
               strstr(s, "[sexnet.http.rx.poll.done]") != NULL ||
               strstr(s, "[sexnet.dns.source3.proof.done]") != NULL ||
               strstr(s, "[sexnet.source3.multi_fetch.done]") != NULL) {
        active_phase = PHASE_NONE;
    }
}

static uint8_t scan_ap_buf[36 * 4];
static const char ssid_good[] = "SexOS_Network";
static const char ssid_long[] = "0123456789012345678901234567890123456789";

static void replace_hex_ptr(char *out, size_t out_cap, uint64_t real_ptr, uint64_t norm_ptr)
{
    char pat[32], rep[32];
    snprintf(pat, sizeof(pat), "0x%llx", (unsigned long long)real_ptr);
    snprintf(rep, sizeof(rep), "0x%llx", (unsigned long long)norm_ptr);
    char *pos;
    while ((pos = strstr(out, pat)) != NULL) {
        char tmp[1024];
        size_t prefix = (size_t)(pos - out);
        snprintf(tmp, sizeof(tmp), "%.*s%s%s", (int)prefix, out, rep, pos + strlen(pat));
        strncpy(out, tmp, out_cap - 1);
        out[out_cap - 1] = '\0';
    }
}

static void print_normalized_line(const char *buf, size_t len)
{
    char out[1024];
    if (len >= sizeof(out)) len = sizeof(out) - 1;
    memcpy(out, buf, len);
    out[len] = '\0';
    while (len > 0 && (out[len - 1] == '\n' || out[len - 1] == '\r')) {
        out[--len] = '\0';
    }
    replace_hex_ptr(out, sizeof(out), (uint64_t)(uintptr_t)scan_ap_buf, 0x90001000ULL);
    replace_hex_ptr(out, sizeof(out), (uint64_t)(uintptr_t)ssid_good,   0x90002000ULL);
    replace_hex_ptr(out, sizeof(out), (uint64_t)(uintptr_t)ssid_long,   0x90003000ULL);
    detect_phase_marker(out);
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
    SexosReply r = {0, 0};
    return r;
}

typedef struct {
    uint64_t type_id;
    uint64_t arg0;
    uint64_t arg1;
} ScriptedReq;

static ScriptedReq req_script[64];
static size_t req_count = 0;
static size_t req_idx = 0;

static void add_req(uint64_t type_id, uint64_t arg0, uint64_t arg1)
{
    if (req_count >= 64) return;
    req_script[req_count].type_id = type_id;
    req_script[req_count].arg0 = arg0;
    req_script[req_count].arg1 = arg1;
    req_count++;
}

_Noreturn static void finish_and_exit(void)
{
    printf("SUMMARY|case=%d|mmio=%llu|dma_r=%llu|dma_w=%llu|fnv=%#018llx\n",
           current_case,
           (unsigned long long)mmio_order,
           (unsigned long long)dma_reads,
           (unsigned long long)dma_writes,
           (unsigned long long)dma_fnv);
    for (size_t i = 0; i < dma_page_count; ++i) {
        free(dma_pages[i].mem);
    }
    exit(0);
}

SexosMessage sexos_listen_once(uint64_t slot)
{
    (void)slot;
    if (req_idx >= req_count) {
        finish_and_exit();
    }
    ScriptedReq r = req_script[req_idx++];
    printf("LISTEN|idx=%zu|type=%#llx\n", req_idx - 1, (unsigned long long)r.type_id);
    SexosMessage m = {r.type_id, r.arg0, r.arg1, 0, 7u, 0};
    return m;
}

uint64_t sexos_reply(uint32_t caller_pd, uint64_t val)
{
    uint64_t scan_hash = 0;
    for (size_t i = 0; i < sizeof(scan_ap_buf); ++i) {
        scan_hash = (scan_hash * 33ULL) ^ scan_ap_buf[i];
    }
    printf("REPLY|caller=%u|val=%#llx|shash=%#llx\n",
           (unsigned)caller_pd,
           (unsigned long long)val,
           (unsigned long long)scan_hash);
    return 0;
}

void sexos_yield(void) {}
void sexos_spin_loop_hint(void) {}
void sexos_spin_forever(void) { finish_and_exit(); }

static void populate_default_happy_queues(void)
{
    uint8_t pkt[256];
    static const uint8_t gw_mac[6] = {0x52, 0x55, 0x0A, 0x00, 0x02, 0x02};
    static const uint8_t gw_ip[4]  = {10, 0, 2, 2};
    static const uint8_t my_ip[4]  = {10, 0, 2, 15};

    /* RX observe: 60-byte ARP request */
    build_arp_request(pkt, gw_mac, gw_ip, my_ip);
    enqueue_frame(PHASE_RX_OBSERVE, pkt, 60);

    /* RX permanent initial poll: 60-byte IPv4 ICMP echo request */
    uint16_t len = build_icmp_frame(pkt, 8, 0, false);
    enqueue_frame(PHASE_RX_PERMANENT, pkt, len);

    /* ARP RX poll: 1 ARP request */
    build_arp_request(pkt, gw_mac, gw_ip, my_ip);
    enqueue_frame(PHASE_ARP_RX, pkt, 60);

    /* ARP cache poll: 2 ARP requests */
    static const uint8_t peer2_mac[6] = {0x52, 0x55, 0x0A, 0x00, 0x02, 0x03};
    static const uint8_t peer2_ip[4]  = {10, 0, 2, 3};
    build_arp_request(pkt, gw_mac, gw_ip, my_ip);
    enqueue_frame(PHASE_ARP_CACHE, pkt, 60);
    build_arp_request(pkt, peer2_mac, peer2_ip, my_ip);
    enqueue_frame(PHASE_ARP_CACHE, pkt, 60);

    /* L2 RX poll: 3 non-ARP frames */
    len = build_icmp_frame(pkt, 8, 0, false);
    enqueue_frame(PHASE_L2_RX, pkt, len);
    enqueue_frame(PHASE_L2_RX, pkt, len);
    enqueue_frame(PHASE_L2_RX, pkt, len);

    /* IPv4 RX poll: 2 valid IPv4 frames (ICMP echo req + TCP SYN-ACK) */
    len = build_icmp_frame(pkt, 8, 0, false);
    enqueue_frame(PHASE_IPV4_RX, pkt, len);

    len = build_tcp_frame(pkt, 18081, 7777, 0x20000000u, 43u, 0x50u, 0x12u, NULL, 0, false);
    enqueue_frame(PHASE_IPV4_RX, pkt, len);

    /* HTTP response RX poll */
    static const char http_resp[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello Domain\n";
    len = build_tcp_frame(pkt, 18081, 7777, 0x20000001u, 127u, 0x50u, 0x18u,
                          (const uint8_t *)http_resp, (uint16_t)(sizeof(http_resp) - 1), false);
    enqueue_frame(PHASE_HTTP_RX, pkt, len);

    /* DNS source3 RX poll */
    len = build_dns_response(pkt, 0x1234u, 0x8180u, 1u, 1u, 4u);
    enqueue_frame(PHASE_DNS_RX, pkt, len);

    /* Phase M iterations 1 and 2 (iteration 0 reuses PHASE_HTTP_RX result) */
    len = build_tcp_frame(pkt, 18081, 7777, 0x20000100u, 43u + 168u, 0x50u, 0x18u,
                          (const uint8_t *)http_resp, (uint16_t)(sizeof(http_resp) - 1), false);
    enqueue_frame(PHASE_M_ITER1, pkt, len);
    len = build_tcp_frame(pkt, 18081, 7777, 0x20000200u, 43u + 252u, 0x50u, 0x18u,
                          (const uint8_t *)http_resp, (uint16_t)(sizeof(http_resp) - 1), false);
    enqueue_frame(PHASE_M_ITER2, pkt, len);
}

static void populate_pdx_requests(void)
{
    add_req(0x200, 0, 0); /* GET_STATUS (disconnected) */
    add_req(0x204, 0, 0); /* VPN_UP while disconnected -> 2 */
    add_req(0x201, 0, 3); /* SCAN_WIFI null ptr -> MAX */
    add_req(0x201, (uint64_t)(uintptr_t)scan_ap_buf, 0); /* SCAN_WIFI max=0 -> MAX */
    add_req(0x201, (uint64_t)(uintptr_t)scan_ap_buf, 2); /* SCAN_WIFI max=2 -> 2 */
    add_req(0x201, (uint64_t)(uintptr_t)scan_ap_buf, 5); /* SCAN_WIFI max=5 -> 3 */
    add_req(0x202, 0, 5); /* CONNECT null -> 1 */
    add_req(0x202, (uint64_t)(uintptr_t)ssid_good, 0); /* CONNECT len=0 -> 1 */
    add_req(0x202, (uint64_t)(uintptr_t)ssid_long, 33); /* CONNECT len=33 -> 1 */
    add_req(0x202, (uint64_t)(uintptr_t)ssid_good, 13); /* CONNECT ok -> 0 */
    add_req(0x200, 0, 0); /* GET_STATUS (connected) */
    add_req(0x204, 0, 0); /* VPN_UP (connected) -> 0 */
    add_req(0x200, 0, 0); /* GET_STATUS (connected + vpn) */
    add_req(0x206, 0, 0); /* GET_IP */
    add_req(0x205, 0, 0); /* VPN_DOWN */
    add_req(0x203, 0, 0); /* DISCONNECT */
    add_req(0x200, 0, 0); /* GET_STATUS */
    add_req(0x207, 0, 0); /* HTTP_PROOF_LEN */
    for (uint64_t i = 0; i < 5; ++i) {
        add_req(0x208, i, 0); /* HTTP_PROOF_CHUNK */
    }
    add_req(0x209, 0, 0); /* HTTP_BODY_LEN */
    for (uint64_t i = 0; i < 4; ++i) {
        add_req(0x20A, i, 0); /* HTTP_BODY_CHUNK */
    }
    add_req(0x20B, 1, 0); /* DNS_RESOLVE host_id=1 */
    add_req(0x20B, 2, 0); /* DNS_RESOLVE host_id=2 (unsupported) */
    add_req(0x2FF, 0, 0); /* unknown opcode */
}

static void setup_case(int c)
{
    current_case = c;
    memset(nic_bar, 0, sizeof(nic_bar));
    memset(phase_queues, 0, sizeof(phase_queues));
    memset(scan_ap_buf, 0, sizeof(scan_ap_buf));
    active_phase = PHASE_NONE;
    rx_hw_next_slot = 0;
    pending_tx_sta_ptr = NULL;
    cfg_bar_map_fail = false;
    cfg_alloc_fail_after = 0xFFFFFFFFu;
    alloc_call_count = 0;
    cfg_tx_delay_polls = 0;
    cfg_rx_delay_polls = 0;

    /* Initial e1000 register state */
    bar_write_le32(0x0000, 0x00180240u); /* CTRL */
    bar_write_le32(0x0008, 0x00080083u); /* STATUS */
    bar_write_le32(0x0100, 0x04008002u); /* RCTL (EN=1) */
    bar_write_le32(0x0400, 0x00040102u); /* TCTL (EN=1) */
    bar_write_le32(0x2808, 128u);        /* RDLEN */
    bar_write_le32(0x3808, 128u);        /* TDLEN */
    bar_write_le32(0x5400, 0x12005452u); /* RAL0 */
    bar_write_le32(0x5404, 0x80005634u); /* RAH0 */

    populate_default_happy_queues();
    populate_pdx_requests();

    uint8_t pkt[256];
    static const uint8_t gw_mac[6] = {0x52, 0x55, 0x0A, 0x00, 0x02, 0x02};
    static const uint8_t gw_ip[4]  = {10, 0, 2, 2};
    static const uint8_t my_ip[4]  = {10, 0, 2, 15};

    if (c == 1) {
        cfg_bar_map_fail = true;
    } else if (c == 2) {
        /* Fail allocation right after first RX ring (2 pages) so rx.observe and rx.permanent fail alloc */
        cfg_alloc_fail_after = 2;
    } else if (c == 3) {
        /* Delayed TX & RX polling + IPv4 in rx.observe + unknown ethertype in rx.permanent */
        cfg_tx_delay_polls = 4;
        cfg_rx_delay_polls = 3;
        phase_queues[PHASE_RX_OBSERVE].count = 0;
        uint16_t len = build_icmp_frame(pkt, 8, 0, false);
        enqueue_frame(PHASE_RX_OBSERVE, pkt, len);

        phase_queues[PHASE_RX_PERMANENT].count = 0;
        memset(pkt, 0x42, 60);
        pkt[12] = 0x88; pkt[13] = 0xB5;
        enqueue_frame(PHASE_RX_PERMANENT, pkt, 60);
    } else if (c == 4) {
        /* Runt frames in rx.observe and rx.permanent, L2 poll break on ARP (0x0806) */
        phase_queues[PHASE_RX_OBSERVE].count = 0;
        memset(pkt, 0xAA, 10);
        enqueue_frame(PHASE_RX_OBSERVE, pkt, 10);

        phase_queues[PHASE_RX_PERMANENT].count = 0;
        enqueue_frame(PHASE_RX_PERMANENT, pkt, 10);

        phase_queues[PHASE_L2_RX].count = 0;
        uint16_t len = build_icmp_frame(pkt, 8, 0, false);
        enqueue_frame(PHASE_L2_RX, pkt, len);
        build_arp_request(pkt, gw_mac, gw_ip, my_ip);
        enqueue_frame(PHASE_L2_RX, pkt, 60);
    } else if (c == 5) {
        /* IPv4 malformed frame rejects in PHASE_IPV4_RX */
        phase_queues[PHASE_IPV4_RX].count = 0;
        /* 1. Runt (<14) */
        memset(pkt, 0, 60);
        enqueue_frame(PHASE_IPV4_RX, pkt, 10);
        /* 2. Non-IPv4 ethertype (0x86DD) */
        pkt[12] = 0x86; pkt[13] = 0xDD;
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* 3. Short IPv4 (<34) */
        pkt[12] = 0x08; pkt[13] = 0x00;
        enqueue_frame(PHASE_IPV4_RX, pkt, 20);
        /* 4. Bad version (0x65) */
        build_ipv4_header(pkt, 0x65, 32, 0, 64, 1, gw_ip, my_ip, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* 5. Bad IHL (0x46) */
        build_ipv4_header(pkt, 0x46, 32, 0, 64, 1, gw_ip, my_ip, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* 6. total_len_min (<20) */
        build_ipv4_header(pkt, 0x45, 16, 0, 64, 1, gw_ip, my_ip, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* 7. total_len_max (> pkt_len - 14) */
        build_ipv4_header(pkt, 0x45, 100, 0, 64, 1, gw_ip, my_ip, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* 8. Fragmented (0x2001) */
        build_ipv4_header(pkt, 0x45, 32, 0x2001u, 64, 1, gw_ip, my_ip, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* 9. Wrong dst IP */
        static const uint8_t wrong_dst[4] = {10, 0, 2, 99};
        build_ipv4_header(pkt, 0x45, 32, 0, 64, 1, gw_ip, wrong_dst, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* 10. Bad IPv4 header checksum */
        build_ipv4_header(pkt, 0x45, 32, 0, 64, 1, gw_ip, my_ip, true);
        enqueue_frame(PHASE_IPV4_RX, pkt, 60);
        /* Plus 6 valid frames so total slot-recycles fit in 16 queue slots; wait: queue size is 16! */
        for (int i = 0; i < 6; ++i) {
            uint16_t len = build_icmp_frame(pkt, 8, 0, false);
            enqueue_frame(PHASE_IPV4_RX, pkt, len);
        }
    } else if (c == 6) {
        /* UDP malformed rejects + validated non-zero checksum + bad ICMP checksum */
        phase_queues[PHASE_IPV4_RX].count = 0;
        static const uint8_t udata[5] = {'h', 'e', 'l', 'l', 'o'};
        /* 1. Validated non-zero UDP checksum on port 7 */
        uint16_t len = build_udp_frame(pkt, gw_ip, my_ip, 50000, 7, udata, 5, 1, 0, 0);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 2. Bad non-zero UDP checksum */
        len = build_udp_frame(pkt, gw_ip, my_ip, 50000, 7, udata, 5, -1, 0, 0);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 3. UDP len < 8 */
        len = build_udp_frame(pkt, gw_ip, my_ip, 50000, 7, udata, 5, 0, 6, 28);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 4. 20 + udp_len > total_len */
        len = build_udp_frame(pkt, gw_ip, my_ip, 50000, 7, udata, 5, 0, 20, 32);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 5. UDP wrong dst port (port 9999) */
        len = build_udp_frame(pkt, gw_ip, my_ip, 50000, 9999, udata, 5, 1, 0, 0);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 6. Bad ICMP checksum */
        len = build_icmp_frame(pkt, 8, 0, true);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 7 & 8. TCP SYN-ACK + valid ICMP */
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000000u, 0x10203041u, 0x50u, 0x12u, NULL, 0, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        len = build_icmp_frame(pkt, 8, 0, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
    } else if (c == 7) {
        /* TCP malformed rejects & RST -> TCP_FAILED_RST */
        phase_queues[PHASE_IPV4_RX].count = 0;
        /* 1. Wrong ports */
        uint16_t len = build_tcp_frame(pkt, 8080, 7777, 0x20000000u, 0x10203041u, 0x50u, 0x12u, NULL, 0, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 2. TCP RST (0x04) -> TCP_FAILED_RST */
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000000u, 0x10203041u, 0x50u, 0x04u, NULL, 0, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
    } else if (c == 8) {
        /* ICMP Destination Unreachable -> TCP SYN retry -> SYN-ACK in retry poll -> HTTP/1.0 response */
        phase_queues[PHASE_IPV4_RX].count = 0;
        uint16_t len = build_icmp_frame(pkt, 3, 1, false);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* 2nd valid IPv4 frame in PHASE_IPV4_RX: TCP segment with bad checksum */
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000000u, 0x10203041u, 0x50u, 0x12u, NULL, 0, true);
        enqueue_frame(PHASE_IPV4_RX, pkt, len);
        /* Retry poll: 1 wrong_ports + 1 valid SYN-ACK */
        len = build_tcp_frame(pkt, 8080, 7777, 0x20000000u, 0x10203041u, 0x50u, 0x12u, NULL, 0, false);
        enqueue_frame(PHASE_TCP_RETRY, pkt, len);
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000000u, 0x10203041u, 0x50u, 0x12u, NULL, 0, false);
        enqueue_frame(PHASE_TCP_RETRY, pkt, len);

        /* HTTP/1.0 200 OK response */
        phase_queues[PHASE_HTTP_RX].count = 0;
        static const char http10_resp[] = "HTTP/1.0 200 OK\r\n\r\nHello 1.0\r\n";
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000001u, 0x10203095u, 0x50u, 0x18u,
                              (const uint8_t *)http10_resp, (uint16_t)(sizeof(http10_resp) - 1), false);
        enqueue_frame(PHASE_HTTP_RX, pkt, len);
    } else if (c == 9) {
        /* HTTP response RX skip branches + malformed HTTP status line */
        phase_queues[PHASE_HTTP_RX].count = 0;
        /* 1. ACK with no payload */
        uint16_t len = build_tcp_frame(pkt, 18081, 7777, 0x20000001u, 0x10203095u, 0x50u, 0x10u, NULL, 0, false);
        enqueue_frame(PHASE_HTTP_RX, pkt, len);
        /* 2. RST segment */
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000001u, 0x10203095u, 0x50u, 0x04u, NULL, 0, false);
        enqueue_frame(PHASE_HTTP_RX, pkt, len);
        /* 3. ACK not set */
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000001u, 0x10203095u, 0x50u, 0x08u, NULL, 0, false);
        enqueue_frame(PHASE_HTTP_RX, pkt, len);
        /* 4. Malformed status line (HTTP/1.1 20X OK) */
        static const char bad_http[] = "HTTP/1.1 20X OK\r\n\r\nBadStatus";
        len = build_tcp_frame(pkt, 18081, 7777, 0x20000001u, 0x10203095u, 0x50u, 0x18u,
                              (const uint8_t *)bad_http, (uint16_t)(sizeof(bad_http) - 1), false);
        enqueue_frame(PHASE_HTTP_RX, pkt, len);
    } else if (c == 10) {
        /* DNS malformed rejects (7 malformed + 1 valid) */
        phase_queues[PHASE_DNS_RX].count = 0;
        /* 1. rx_too_short (<54) */
        memset(pkt, 0, 60);
        enqueue_frame(PHASE_DNS_RX, pkt, 40);
        /* 2. non_ipv4_ethertype */
        pkt[12] = 0x86; pkt[13] = 0xDD;
        enqueue_frame(PHASE_DNS_RX, pkt, 60);
        /* 3. ipv4_header_invalid (ver=6) */
        static const uint8_t dns_ip[4] = {10, 0, 2, 3};
        build_ipv4_header(pkt, 0x65, 46, 0, 64, 17, dns_ip, my_ip, false);
        enqueue_frame(PHASE_DNS_RX, pkt, 60);
        /* 4. not_udp (proto=6) */
        build_ipv4_header(pkt, 0x45, 46, 0, 64, 6, dns_ip, my_ip, false);
        enqueue_frame(PHASE_DNS_RX, pkt, 60);
        /* 5. unexpected_src_ip (10.0.2.2 instead of 10.0.2.3) */
        uint8_t dummy_dns[20] = {0};
        uint16_t len = build_udp_frame(pkt, gw_ip, my_ip, 53, 49152, dummy_dns, 20, 0, 0, 0);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 6. unexpected_udp_ports (src_port=5353) */
        len = build_udp_frame(pkt, dns_ip, my_ip, 5353, 49152, dummy_dns, 20, 0, 0, 0);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 7. txid_mismatch (0x9999) */
        len = build_dns_response(pkt, 0x9999u, 0x8180u, 1u, 1u, 4u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 8. valid DNS response */
        len = build_dns_response(pkt, 0x1234u, 0x8180u, 1u, 1u, 4u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
    } else if (c == 11) {
        /* More DNS malformed rejects + synthetic UDP self-test fallback (0 IPv4 frames in PHASE_IPV4_RX) */
        phase_queues[PHASE_IPV4_RX].count = 0;
        phase_queues[PHASE_DNS_RX].count = 0;
        /* 1. flags_not_response (0x0100) */
        uint16_t len = build_dns_response(pkt, 0x1234u, 0x0100u, 1u, 1u, 4u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 2. rcode_nonzero (0x8183 NXDOMAIN) */
        len = build_dns_response(pkt, 0x1234u, 0x8183u, 1u, 1u, 4u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 3. qdcount_invalid (qdcount=0) */
        len = build_dns_response(pkt, 0x1234u, 0x8180u, 0u, 1u, 4u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 4. ancount_zero (ancount=0) */
        len = build_dns_response(pkt, 0x1234u, 0x8180u, 1u, 0u, 4u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 5. rdata_oob (rdlen=100) */
        len = build_dns_response(pkt, 0x1234u, 0x8180u, 1u, 1u, 100u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
        /* 6. valid DNS response */
        len = build_dns_response(pkt, 0x1234u, 0x8180u, 1u, 1u, 4u);
        enqueue_frame(PHASE_DNS_RX, pkt, len);
    }
}

int main(int argc, char **argv)
{
    int c = (argc > 1) ? atoi(argv[1]) : 0;
    setup_case(c);
    sexnet_main();
    finish_and_exit();
}
"""

RUST_WRAPPER_PREFIX = r"""
#![allow(dead_code, unused_mut, unused_variables, unused_assignments, unused_parens, unused_unsafe, non_upper_case_globals)]

extern "C" {
    fn sexos_pdx_call(slot: u64, op: u64, a0: u64, a1: u64, a2: u64) -> PdxCResult;
    fn sexos_listen_once(slot: u64) -> sex_pdx::PdxMessage;
    fn sexos_reply(caller_pd: u32, val: u64) -> u64;
    fn sexos_yield();
    fn sexos_spin_forever() -> !;
    fn sexos_map_pci_bar(cap_slot: u64, bar_index: u64, map_size: u64) -> u64;
    fn sexos_allocate_memory(size: u64) -> u64;
    fn sexos_map_memory(phys: u64, size: u64) -> u64;
    fn sexos_net_diag(selector: u64) -> u64;

    fn sexos_volatile_read8(addr: *const u8) -> u8;
    fn sexos_volatile_read16(addr: *const u16) -> u16;
    fn sexos_volatile_read32(addr: *const u32) -> u32;
    fn sexos_volatile_read64(addr: *const u64) -> u64;
    fn sexos_volatile_write8(addr: *mut u8, val: u8);
    fn sexos_volatile_write16(addr: *mut u16, val: u16);
    fn sexos_volatile_write32(addr: *mut u32, val: u32);
    fn sexos_volatile_write64(addr: *mut u64, val: u64);
}

#[repr(C)]
struct PdxCResult {
    status: u64,
    value: u64,
}

trait VolatileAccess: Copy {
    unsafe fn vread(ptr: *const Self) -> Self;
    unsafe fn vwrite(ptr: *mut Self, val: Self);
}

impl VolatileAccess for u8 {
    unsafe fn vread(ptr: *const Self) -> Self { sexos_volatile_read8(ptr) }
    unsafe fn vwrite(ptr: *mut Self, val: Self) { sexos_volatile_write8(ptr, val); }
}
impl VolatileAccess for u16 {
    unsafe fn vread(ptr: *const Self) -> Self { sexos_volatile_read16(ptr) }
    unsafe fn vwrite(ptr: *mut Self, val: Self) { sexos_volatile_write16(ptr, val); }
}
impl VolatileAccess for u32 {
    unsafe fn vread(ptr: *const Self) -> Self { sexos_volatile_read32(ptr) }
    unsafe fn vwrite(ptr: *mut Self, val: Self) { sexos_volatile_write32(ptr, val); }
}
impl VolatileAccess for u64 {
    unsafe fn vread(ptr: *const Self) -> Self { sexos_volatile_read64(ptr) }
    unsafe fn vwrite(ptr: *mut Self, val: Self) { sexos_volatile_write64(ptr, val); }
}

unsafe fn hosted_read_volatile<T: VolatileAccess>(ptr: *const T) -> T {
    T::vread(ptr)
}

unsafe fn hosted_write_volatile<T: VolatileAccess>(ptr: *mut T, val: T) {
    T::vwrite(ptr, val);
}

mod spin {
    use std::cell::UnsafeCell;
    use std::ops::{Deref, DerefMut};
    pub struct Mutex<T>(UnsafeCell<T>);
    unsafe impl<T> Sync for Mutex<T> {}
    impl<T> Mutex<T> {
        pub const fn new(v: T) -> Self { Self(UnsafeCell::new(v)) }
        pub fn lock(&self) -> Guard<'_, T> { Guard(self.0.get(), std::marker::PhantomData) }
    }
    pub struct Guard<'a, T>(*mut T, std::marker::PhantomData<&'a mut T>);
    impl<'a, T> Deref for Guard<'a, T> {
        type Target = T;
        fn deref(&self) -> &T { unsafe { &*self.0 } }
    }
    impl<'a, T> DerefMut for Guard<'a, T> {
        fn deref_mut(&mut self) -> &mut T { unsafe { &mut *self.0 } }
    }
}

mod sex_pdx {
    pub const SLOT_KERNEL: u64 = 0;
    pub const SLOT_NIC: u64 = 19;
    pub const KERNEL_RAW_PRINT: u64 = 0x45;

    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct PdxMessage {
        pub type_id: u64,
        pub arg0: u64,
        pub arg1: u64,
        pub arg2: u64,
        pub caller_pd: u32,
        pub padding: u32,
    }

    pub fn pdx_listen_raw(slot: u64) -> PdxMessage {
        static mut PDX_LISTEN_WRAPPER_BUDGET: u32 = 8;
        loop {
            let m = unsafe { super::sexos_listen_once(slot) };
            if m.type_id == 0 {
                unsafe { super::sexos_yield(); }
                continue;
            }
            unsafe {
                if PDX_LISTEN_WRAPPER_BUDGET > 0 {
                    PDX_LISTEN_WRAPPER_BUDGET -= 1;
                    serial_println!(
                        "[pdx.listen.raw.wrapper] type={:#x} caller={} a0={:#x} a1={:#x}",
                        m.type_id, m.caller_pd, m.arg0, m.arg1
                    );
                }
            }
            return m;
        }
    }

    pub fn pdx_reply(caller_pd: u32, val: u64) -> Result<u64, u64> {
        Ok(unsafe { super::sexos_reply(caller_pd, val) })
    }

    pub fn sys_map_pci_bar(cap_slot: u64, bar_index: u64, map_size: u64) -> u64 {
        unsafe { super::sexos_map_pci_bar(cap_slot, bar_index, map_size) }
    }

    pub fn raw_print_str(s: &str) {
        unsafe {
            super::sexos_pdx_call(SLOT_KERNEL, KERNEL_RAW_PRINT, s.as_ptr() as u64, s.len() as u64, 0);
        }
    }

    macro_rules! serial_println {
        ($($arg:tt)*) => {{
            let s = format!($($arg)*);
            $crate::sex_pdx::raw_print_str(&s);
        }};
    }
    pub(crate) use serial_println;
}
"""

MUTATIONS = (
    (
        "wrong_diag_status_shift",
        "(var uint32_t status (cast uint32_t (>> raw 32)))",
        "(var uint32_t status (cast uint32_t (>> raw 24)))",
        0,
        (),
    ),
    (
        "wrong_diag_source_shift",
        "(var uint8_t source (cast uint8_t (& (>> raw 16) 0xFFULL)))",
        "(var uint8_t source (cast uint8_t (& (>> raw 8) 0xFFULL)))",
        0,
        (),
    ),
    (
        "wrong_wifi_connect_speed",
        "(= (. (-> st state) link_speed_mbps) 300u)",
        "(= (. (-> st state) link_speed_mbps) 100u)",
        0,
        (),
    ),
    (
        "wrong_vpn_flag_bit",
        "(?: (== (. (-> st state) vpn) VPN_UP) 2ULL 0ULL)))",
        "(?: (== (. (-> st state) vpn) VPN_UP) 4ULL 0ULL)))",
        0,
        (),
    ),
    (
        "wrong_scan_wifi_cap",
        "(var size_t count (?: (< 3 max) 3 max))",
        "(var size_t count (?: (< 2 max) 2 max))",
        0,
        (),
    ),
    (
        "wrong_ral_offset",
        "(var uint32_t ral (call vread32 (+ nic_va 0x5400ULL)))",
        "(var uint32_t ral (call vread32 (+ nic_va 0x5404ULL)))",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_rctl_disable_bit",
        "(var uint32_t rctl_clear (& rctl_orig (~ (<< 1u 1))))",
        "(var uint32_t rctl_clear (& rctl_orig (~ (<< 1u 2))))",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_rx_ring_rdlen",
        "(call vwrite32 (+ nic_va 0x2808ULL) 128u)",
        "(call vwrite32 (+ nic_va 0x2808ULL) 64u)",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_tx_observe_cmd",
        "(call vwrite8 (+ tx_desc_va 11ULL) 0x0Bu)",
        "(call vwrite8 (+ tx_desc_va 11ULL) 0x09u)",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_arp_reply_oper",
        "(call vwrite8 (+ arp_tx_va 21ULL) 0x02u)",
        "(call vwrite8 (+ arp_tx_va 21ULL) 0x01u)",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_tcp_isn",
        "(= (-> st tcp_local_seq) 42u)",
        "(= (-> st tcp_local_seq) 43u)",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_icmp_echo_reply_type",
        "(call vwrite8 (+ tx_va 34ULL) 0x00u)",
        "(call vwrite8 (+ tx_va 34ULL) 0x08u)",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_udp_echo_proto",
        "(call vwrite8 (+ tx_va 23ULL) 17u)",
        "(call vwrite8 (+ tx_va 23ULL) 6u)",
        6,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_http_status_hundreds",
        "(cast uint16_t (+ (+ (* (cast uint16_t (- d0 48u)) 100u) (* (cast uint16_t (- d1 48u)) 10u)) (cast uint16_t (- d2 48u)))))",
        "(cast uint16_t (+ (+ (* (cast uint16_t (- d0 48u)) 10u) (* (cast uint16_t (- d1 48u)) 10u)) (cast uint16_t (- d2 48u)))))",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE",),
    ),
    (
        "wrong_dns_txid",
        "(define SEXNET_DNS_TXID \"((uint16_t)0x1234)\")",
        "(define SEXNET_DNS_TXID \"((uint16_t)0x1235)\")",
        0,
        ("SEXOS_SEXNET_PROOF_PROFILE", "SEXNET_DNS_SOURCE3_PROOF"),
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
    raw = run_cmd(["git", "-C", str(REPO), "show", f"{UPSTREAM_REV}:servers/sexnet/src/main.rs"]).stdout
    sha = hashlib.sha256(raw.encode("utf-8")).hexdigest()
    if EXPECTED_RUST_SHA256 is not None and sha != EXPECTED_RUST_SHA256:
        raise AssertionError(f"unexpected Rust SHA256: {sha} != {EXPECTED_RUST_SHA256}")

    src = raw
    src = src.replace("#![no_std]", "")
    src = src.replace("#![no_main]", "")
    src = re.sub(
        r"#\[panic_handler\]\s*fn panic\(_info: &core::panic::PanicInfo\) -> ! \{\s*loop \{\}\s*\}",
        "",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = re.sub(
        r"unsafe fn sys_net_diag\(selector: u64\) -> u64 \{.*?\n\}",
        "unsafe fn sys_net_diag(selector: u64) -> u64 { sexos_net_diag(selector) }",
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
    src = src.replace('pub extern "C" fn _start() -> !', 'pub extern "C" fn sexnet_main()')
    return RUST_WRAPPER_PREFIX + "\n" + src, sha


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cases", type=int, default=8)
    parser.add_argument("--all-variants", action="store_true")
    args = parser.parse_args()

    cesta_bin = Path(shutil.which(str(args.cesta)) or args.cesta).resolve()
    cesta_src = HERE / "src/sexnet.cesta"
    exported_c = HERE / "c/src/sexnet.c"

    cesta_text = LayoutText(cesta_src.read_text())
    fn_split = cesta_text.split("(fn ", 1)[1]
    if "cpp-if" in fn_split:
        raise AssertionError("cpp-if found inside function bodies of src/sexnet.cesta")

    variants = ALL_VARIANTS if args.all_variants else CORE_VARIANTS

    with tempfile.TemporaryDirectory() as tmp:
        tmpdir = Path(tmp)
        provider_c = tmpdir / "provider.c"
        provider_c.write_text(PROVIDER_C)
        rust_rs = tmpdir / "rust_sexnet.rs"
        rust_code, rust_sha = build_hosted_rust_source()
        rust_rs.write_text(rust_code)

        authored_c = tmpdir / "sexnet_authored.c"
        exported = run_cmd([str(cesta_bin), "export-c", str(cesta_src), "--"]).stdout
        authored_c.write_text(exported)
        if exported_c.exists() and exported_c.read_text().strip() != exported.strip():
            raise AssertionError("c/src/sexnet.c is out of date with src/sexnet.cesta")

        common_c_sources = [
            str(HERE / "runtime/client_runtime.c"),
            str(HERE / "runtime/server_runtime.c"),
            str(HERE / "generated/runtime/format_bytes.c"),
            str(HERE / "generated/runtime/packed_words.c"),
        ]
        inc_flags = ["-I", str(HERE / "generated/include"), "-DSEXOS_HOST=1"]

        total_lines = 0
        ref_traces = {}
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
                ref_traces[(flags, c)] = r_out
                if r_out != g_out or r_out != cl_out:
                    r_lines = r_out.splitlines()
                    g_lines = g_out.splitlines()
                    for idx, (rl, gl) in enumerate(zip(r_lines, g_lines)):
                        if rl != gl:
                            sys.stderr.write(
                                f"First diff at line {idx + 1} (variant={vname} case={c}):\n"
                                f"  RUST:  {rl}\n  CESTA: {gl}\n"
                            )
                            break
                    else:
                        sys.stderr.write(
                            f"Length diff (variant={vname} case={c}): rust={len(r_lines)} cesta={len(g_lines)}\n"
                        )
                    raise AssertionError(f"trace mismatch in variant={vname} case={c}")
                total_lines += len(r_out.splitlines())

        killed = 0
        for mname, target, replacement, case_id, mflags in MUTATIONS:
            if cesta_text.count(target) == 0:
                raise AssertionError(f"mutation target not found for {mname}")
            if (mflags, case_id) not in ref_traces:
                rust_env = {f: "1" for f in mflags}
                dflags = [f"-D{f}=1" for f in mflags]
                rust_lib = tmpdir / f"librust_mutref_{len(ref_traces)}.a"
                run_cmd(
                    ["rustc", "--edition=2021", "-O", "--crate-type=staticlib", str(rust_rs), "-o", str(rust_lib)],
                    env=rust_env,
                )
                rust_bin = tmpdir / f"rust_mutref_{len(ref_traces)}"
                run_cmd(
                    ["gcc", "-O2", *inc_flags, *dflags, str(provider_c), str(rust_lib), "-lpthread", "-ldl", "-o", str(rust_bin)]
                )
                ref_traces[(mflags, case_id)] = run_cmd([str(rust_bin), str(case_id)]).stdout

            mut_cesta = tmpdir / f"mut_{mname}.cesta"
            mut_cesta.write_text(cesta_text.replace(target, replacement))
            mut_c = tmpdir / f"mut_{mname}.c"
            mut_c.write_text(run_cmd([str(cesta_bin), "export-c", str(mut_cesta), "--"]).stdout)
            mut_bin = tmpdir / f"mut_{mname}"
            dflags = [f"-D{f}=1" for f in mflags]
            run_cmd(
                [
                    "gcc",
                    "-std=c11",
                    "-O2",
                    *inc_flags,
                    *dflags,
                    str(provider_c),
                    str(mut_c),
                    *common_c_sources,
                    "-o",
                    str(mut_bin),
                ]
            )
            proc = subprocess.run([str(mut_bin), str(case_id)], capture_output=True, text=True)
            if proc.returncode != 0 or proc.stdout != ref_traces[(mflags, case_id)]:
                killed += 1
            else:
                raise AssertionError(f"mutation {mname} survived on case {case_id}")

        print(
            f"sexnet differential: ok ({len(variants)} variants x {args.cases} cases, "
            f"{total_lines} trace lines, {killed}/{len(MUTATIONS)} mutations killed, "
            f"rust_sha256={rust_sha[:16]})"
        )


if __name__ == "__main__":
    main()
