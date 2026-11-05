#!/usr/bin/env python3
"""Differential & mutation verifier for apps/sexdrive (NVMe + xHCI + Block service)."""

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
EXPECTED_RUST_SHA256 = "47b0abfc289a23edc70b72b7adc43460d14570d465ef2e9fc9c788fc808aa3dc"

ENV_FLAGS = (
    "SEXOS_STORAGE_100_PERSIST_WRITE",
    "SEXOS_STORAGE_100_PERSIST_READ",
    "SEXOS_STORAGE_100_FLUSH_AUDIT",
    "SEXOS_STORAGE_100_NEGATIVE",
    "SEXOS_STORAGE_100_NEG_MISMATCH",
    "SEXOS_STORAGE_100_IO_READ_PROBE",
)

CORE_VARIANTS = (
    ("default", ()),
    ("persist_write", ("SEXOS_STORAGE_100_PERSIST_WRITE",)),
    ("persist_read", ("SEXOS_STORAGE_100_PERSIST_READ",)),
    (
        "neg_mismatch_plus_io_probe",
        (
            "SEXOS_STORAGE_100_NEGATIVE",
            "SEXOS_STORAGE_100_NEG_MISMATCH",
            "SEXOS_STORAGE_100_IO_READ_PROBE",
        ),
    ),
)

ALL_VARIANTS = CORE_VARIANTS + (
    ("flush_audit", ("SEXOS_STORAGE_100_FLUSH_AUDIT",)),
    ("io_read_probe_only", ("SEXOS_STORAGE_100_IO_READ_PROBE",)),
    (
        "persist_write_plus_io_probe",
        ("SEXOS_STORAGE_100_PERSIST_WRITE", "SEXOS_STORAGE_100_IO_READ_PROBE"),
    ),
    (
        "persist_read_plus_io_probe",
        ("SEXOS_STORAGE_100_PERSIST_READ", "SEXOS_STORAGE_100_IO_READ_PROBE"),
    ),
)

PROVIDER_C = r"""
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sexos_mmio.h"

extern void sexdrive_main(void);

#define FAKE_NVME_BAR_VA UINT64_C(0x70000000)
#define FAKE_XHCI_BAR_VA UINT64_C(0x71000000)
#define FAKE_SHARED_BUF_TOKEN UINT64_C(0x72000000)
#define FAKE_PHYS_BASE UINT64_C(0x100000)
#define MAX_DMA_PAGES 128
#define DISK_SECTORS 2048

#define SLOT_SHELL UINT64_C(6)
#define SLOT_USB_HOST UINT64_C(8)
#define SLOT_NVME_HOST UINT64_C(16)
#define SLOT_BUF_LEND UINT64_C(17)

#define BLOCK_READ UINT64_C(1)
#define BLOCK_WRITE UINT64_C(2)
#define BLOCK_SYNC UINT64_C(3)

typedef struct {
    uint64_t phys;
    uint8_t *mem;
    uint64_t va;
    bool mapped;
} DmaPage;

static DmaPage dma_pages[MAX_DMA_PAGES];
static size_t dma_page_count = 0;
static uint8_t nvme_bar[0x4000];
static uint8_t xhci_bar[0x1000];
static uint8_t lend_buf[4096];
static uint32_t shared_fb[1024 * 768];
static uint8_t disk_data[DISK_SECTORS][512];

static int current_case = 0;
static bool cfg_nvme_bar_fail = false;
static bool cfg_xhci_bar_fail = false;
static bool cfg_preconfigured = true;
static uint32_t cfg_dstrd = 0;
static uint32_t cfg_disable_rdy_delay = 0;
static uint32_t cfg_enable_rdy_delay = 0;
static uint32_t cfg_admin_cqe_delay = 0;
static uint32_t cfg_io_cqe_delay = 0;
static bool cfg_memlend_fail_once = false;
static bool cfg_io_status_fail_once = false;

static uint64_t mmio_order = 0;
static uint64_t dma_reads = 0;
static uint64_t dma_writes = 0;
static uint64_t spin_hints = 0;
static uint64_t dma_fnv = UINT64_C(0xcbf29ce484222325);
static bool fence_since_last_sqe_write = false;

static uint32_t csts_disable_polls_left = 0;
static uint32_t csts_enable_polls_left = 0;

static uint16_t model_admin_sq_tail = 0;
static uint16_t model_admin_cq_head = 0;
static uint8_t model_admin_cq_phase = 1;
static bool pending_admin_cqe = false;
static uint32_t pending_admin_cqe_delay = 0;
static uint32_t staged_admin_cqe[4];
static uint64_t staged_admin_cqe_va = 0;

static uint64_t model_io_cq_phys = 0;
static uint64_t model_io_sq_phys = 0;
static uint16_t model_io_sq_tail = 0;
static uint16_t model_io_cq_head = 0;
static uint8_t model_io_cq_phase = 1;
static bool pending_io_cqe = false;
static uint32_t pending_io_cqe_delay = 0;
static uint32_t staged_io_cqe[4];
static uint64_t staged_io_cqe_va = 0;

typedef struct {
    uint64_t type_id;
    uint32_t caller_pd;
    uint64_t arg0;
    uint64_t arg1;
    uint64_t arg2;
    bool fill_lend;
    uint8_t lend_seed;
} ScriptMsg;

static ScriptMsg msg_queue[64];
static size_t msg_count = 0;
static size_t msg_index = 0;
static uint32_t yield_count = 0;
static RegionWriterState writer_state = {true, 4, 11, 2, 11, 1, 0};

static void dma_hash_mix(uint64_t v)
{
    dma_fnv ^= v;
    dma_fnv *= UINT64_C(0x100000001b3);
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
    if (addr >= (uintptr_t)lend_buf && addr + width <= (uintptr_t)lend_buf + sizeof(lend_buf)) {
        return (uint8_t *)addr;
    }
    for (size_t i = 0; i < dma_page_count; ++i) {
        if (!dma_pages[i].mapped) continue;
        uintptr_t base = (uintptr_t)dma_pages[i].va;
        if (addr >= base && addr + width <= base + 4096) {
            return dma_pages[i].mem + (addr - base);
        }
    }
    return NULL;
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

static void write_le64_raw(uint8_t *p, uint64_t v)
{
    memcpy(p, &v, 8);
}

static void commit_staged_cqe(uint64_t cqe_va, const uint32_t cqe[4])
{
    uint8_t *p = resolve_dma_addr((uintptr_t)cqe_va, 16);
    if (!p) {
        printf("FATAL|bad_cqe_va=%#llx\n", (unsigned long long)cqe_va);
        exit(2);
    }
    for (int i = 0; i < 4; ++i) write_le32_raw(p + i * 4, cqe[i]);
}

static void handle_admin_doorbell(uint32_t new_tail)
{
    if (!fence_since_last_sqe_write) {
        printf("ORDER_ERR|missing_fence_before_sq0tdbl\n");
    }
    uint64_t asq_phys = read_le64_raw(nvme_bar + 0x28);
    uint64_t acq_phys = read_le64_raw(nvme_bar + 0x30);
    uint32_t aqa = read_le32_raw(nvme_bar + 0x24);
    uint16_t q_entries = (uint16_t)((aqa & 0x0FFFu) + 1u);
    DmaPage *asq_page = find_page_by_phys(asq_phys);
    DmaPage *acq_page = find_page_by_phys(acq_phys);
    if (!asq_page || !acq_page) {
        printf("ADMIN_ERR|missing_queue_pages asq=%#llx acq=%#llx\n",
               (unsigned long long)asq_phys, (unsigned long long)acq_phys);
        return;
    }
    uint16_t idx = model_admin_sq_tail;
    model_admin_sq_tail = (uint16_t)new_tail;
    const uint8_t *sqe = asq_page->mem + (size_t)idx * 64;
    uint32_t cdw0 = read_le32_raw(sqe + 0);
    uint32_t nsid = read_le32_raw(sqe + 4);
    uint64_t prp1 = read_le64_raw(sqe + 24);
    uint32_t cdw10 = read_le32_raw(sqe + 40);
    uint32_t cdw11 = read_le32_raw(sqe + 44);
    uint8_t opc = (uint8_t)(cdw0 & 0xFFu);
    uint16_t cid = (uint16_t)(cdw0 >> 16);
    printf("ADMIN_SQE|idx=%u|opc=%#x|cid=%u|nsid=%u|prp1=%#llx|cdw10=%#x|cdw11=%#x\n",
           (unsigned)idx, (unsigned)opc, (unsigned)cid, (unsigned)nsid,
           (unsigned long long)prp1, (unsigned)cdw10, (unsigned)cdw11);

    uint8_t sc = 0, sct = 0;
    if (opc == 0x06u) {
        DmaPage *id_page = find_page_by_phys(prp1);
        if (!id_page || cdw10 != 1u) {
            sc = 1;
        } else {
            memset(id_page->mem, 0, 4096);
            write_le64_raw(id_page->mem + 4, UINT64_C(0x313030564d4e5853));
            write_le64_raw(id_page->mem + 24, UINT64_C(0x564e2d534f584553));
            write_le32_raw(id_page->mem + 516, 1u);
        }
    } else if (opc == 0x05u) {
        uint16_t qid = (uint16_t)(cdw10 & 0xFFFFu);
        uint16_t qsize = (uint16_t)((cdw10 >> 16) + 1u);
        if (qid != 1u || qsize != 16u || (cdw11 & 1u) == 0u || !find_page_by_phys(prp1)) {
            sc = 2;
        } else {
            model_io_cq_phys = prp1;
            model_io_cq_head = 0;
            model_io_cq_phase = 1;
        }
    } else if (opc == 0x01u) {
        uint16_t qid = (uint16_t)(cdw10 & 0xFFFFu);
        uint16_t qsize = (uint16_t)((cdw10 >> 16) + 1u);
        uint16_t cqid = (uint16_t)(cdw11 >> 16);
        if (qid != 1u || qsize != 16u || cqid != 1u || (cdw11 & 1u) == 0u || !find_page_by_phys(prp1)) {
            sc = 3;
        } else {
            model_io_sq_phys = prp1;
            model_io_sq_tail = 0;
        }
    } else {
        sc = 4;
    }

    uint16_t status_field = (uint16_t)(((uint16_t)sct << 8) | sc);
    staged_admin_cqe[0] = 0;
    staged_admin_cqe[1] = 0;
    staged_admin_cqe[2] = ((uint32_t)model_admin_sq_tail << 16);
    staged_admin_cqe[3] = ((uint32_t)status_field << 17) | ((uint32_t)(model_admin_cq_phase & 1u) << 16) | (uint32_t)cid;
    staged_admin_cqe_va = acq_page->va + (uint64_t)model_admin_cq_head * 16ULL;

    uint16_t next_head = (uint16_t)(model_admin_cq_head + 1u);
    if (next_head >= q_entries) {
        model_admin_cq_head = 0;
        model_admin_cq_phase ^= 1u;
    } else {
        model_admin_cq_head = next_head;
    }

    if (cfg_admin_cqe_delay == 0) {
        commit_staged_cqe(staged_admin_cqe_va, staged_admin_cqe);
        pending_admin_cqe = false;
    } else {
        pending_admin_cqe = true;
        pending_admin_cqe_delay = cfg_admin_cqe_delay;
    }
}

static void handle_io_doorbell(uint32_t new_tail)
{
    if (!fence_since_last_sqe_write) {
        printf("ORDER_ERR|missing_fence_before_sq1tdbl\n");
    }
    DmaPage *io_cq_page = find_page_by_phys(model_io_cq_phys);
    DmaPage *io_sq_page = find_page_by_phys(model_io_sq_phys);
    if (!io_sq_page || !io_cq_page) {
        printf("IO_ERR|missing_io_queue_pages tail=%u\n", (unsigned)new_tail);
        return;
    }

    uint16_t idx = (uint16_t)((new_tail + 15u) % 16u);
    model_io_sq_tail = (uint16_t)new_tail;

    const uint8_t *sqe = io_sq_page->mem + (size_t)idx * 64;
    uint32_t cdw0 = read_le32_raw(sqe + 0);
    uint32_t nsid = read_le32_raw(sqe + 4);
    uint64_t prp1 = read_le64_raw(sqe + 24);
    uint32_t cdw10 = read_le32_raw(sqe + 40);
    uint32_t cdw11 = read_le32_raw(sqe + 44);
    uint32_t cdw12 = read_le32_raw(sqe + 48);
    uint8_t opc = (uint8_t)(cdw0 & 0xFFu);
    uint16_t cid = (uint16_t)(cdw0 >> 16);
    uint64_t slba = ((uint64_t)cdw11 << 32) | cdw10;
    uint32_t nlb = (cdw12 & 0xFFFFu) + 1u;

    /* Legacy SEXOS_STORAGE_100_IO_READ_PROBE resets local sq_tail/cq_head/cq_phase */
    if (cid == 0x0045u) {
        model_io_cq_head = 0;
        model_io_cq_phase = 1;
    }

    printf("IO_SQE|idx=%u|opc=%#x|cid=%u|nsid=%u|prp1=%#llx|slba=%llu|nlb=%u\n",
           (unsigned)idx, (unsigned)opc, (unsigned)cid, (unsigned)nsid,
           (unsigned long long)prp1, (unsigned long long)slba, (unsigned)nlb);

    uint8_t sc = 0, sct = 0;
    if (cfg_io_status_fail_once && cid >= 0x0510u) {
        cfg_io_status_fail_once = false;
        sc = 2;
    } else if (opc == 0x02u) {
        DmaPage *data_page = find_page_by_phys(prp1);
        if (!data_page || nsid != 1u || slba + nlb > DISK_SECTORS || nlb > 8u) {
            sc = 1;
        } else {
            for (uint32_t b = 0; b < nlb; ++b) {
                memcpy(data_page->mem + b * 512, disk_data[slba + b], 512);
            }
        }
    } else if (opc == 0x01u) {
        DmaPage *data_page = find_page_by_phys(prp1);
        if (!data_page || nsid != 1u || slba + nlb > DISK_SECTORS || nlb > 8u) {
            sc = 1;
        } else {
            for (uint32_t b = 0; b < nlb; ++b) {
                memcpy(disk_data[slba + b], data_page->mem + b * 512, 512);
            }
        }
    } else if (opc == 0x00u) {
        if (nsid != 1u) sc = 1;
    } else {
        sc = 5;
    }

    uint16_t status_field = (uint16_t)(((uint16_t)sct << 8) | sc);
    staged_io_cqe[0] = 0;
    staged_io_cqe[1] = 0;
    staged_io_cqe[2] = (1u << 16) | (uint32_t)model_io_sq_tail;
    staged_io_cqe[3] = ((uint32_t)status_field << 17) | ((uint32_t)(model_io_cq_phase & 1u) << 16) | (uint32_t)cid;
    staged_io_cqe_va = io_cq_page->va + (uint64_t)model_io_cq_head * 16ULL;

    uint16_t next_head = (uint16_t)(model_io_cq_head + 1u);
    if (next_head >= 16u) {
        model_io_cq_head = 0;
        model_io_cq_phase ^= 1u;
    } else {
        model_io_cq_head = next_head;
    }

    if (cfg_io_cqe_delay == 0) {
        commit_staged_cqe(staged_io_cqe_va, staged_io_cqe);
        pending_io_cqe = false;
    } else {
        pending_io_cqe = true;
        pending_io_cqe_delay = cfg_io_cqe_delay;
    }
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
    if (a >= FAKE_NVME_BAR_VA && a + 4 <= FAKE_NVME_BAR_VA + sizeof(nvme_bar)) {
        uint64_t off = (uint64_t)(a - FAKE_NVME_BAR_VA);
        if ((off & 3ULL) != 0 || (off != 0x08 && off != 0x14 && off != 0x1C && off != 0x24)) {
            printf("FATAL|unsupported_nvme_read32 off=%#llx\n", (unsigned long long)off);
            exit(2);
        }
        if (off == 0x1C) {
            if (csts_disable_polls_left > 0) {
                csts_disable_polls_left--;
                if (csts_disable_polls_left == 0) {
                    write_le32_raw(nvme_bar + 0x1C, 0u);
                }
            } else if (csts_enable_polls_left > 0) {
                csts_enable_polls_left--;
                if (csts_enable_polls_left == 0) {
                    write_le32_raw(nvme_bar + 0x1C, 1u);
                }
            }
        }
        uint32_t v = read_le32_raw(nvme_bar + off);
        mmio_order++;
        printf("MMIO|nvme|off=%#llx|w=32|dir=R|val=%#x|seq=%llu\n",
               (unsigned long long)off, (unsigned)v, (unsigned long long)mmio_order);
        return v;
    }
    if (a >= FAKE_XHCI_BAR_VA && a + 4 <= FAKE_XHCI_BAR_VA + sizeof(xhci_bar)) {
        uint64_t off = (uint64_t)(a - FAKE_XHCI_BAR_VA);
        if ((off & 3ULL) != 0) {
            printf("FATAL|unaligned_xhci_read32 off=%#llx\n", (unsigned long long)off);
            exit(2);
        }
        uint32_t v = read_le32_raw(xhci_bar + off);
        mmio_order++;
        printf("MMIO|xhci|off=%#llx|w=32|dir=R|val=%#x|seq=%llu\n",
               (unsigned long long)off, (unsigned)v, (unsigned long long)mmio_order);
        return v;
    }
    if (pending_admin_cqe && a == (uintptr_t)(staged_admin_cqe_va + 12ULL)) {
        if (pending_admin_cqe_delay > 0) {
            pending_admin_cqe_delay--;
            if (pending_admin_cqe_delay == 0) {
                commit_staged_cqe(staged_admin_cqe_va, staged_admin_cqe);
                pending_admin_cqe = false;
            }
        }
    }
    if (pending_io_cqe && a == (uintptr_t)(staged_io_cqe_va + 12ULL)) {
        if (pending_io_cqe_delay > 0) {
            pending_io_cqe_delay--;
            if (pending_io_cqe_delay == 0) {
                commit_staged_cqe(staged_io_cqe_va, staged_io_cqe);
                pending_io_cqe = false;
            }
        }
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
    uintptr_t a = (uintptr_t)addr;
    if (a >= FAKE_NVME_BAR_VA && a + 8 <= FAKE_NVME_BAR_VA + sizeof(nvme_bar)) {
        uint64_t off = (uint64_t)(a - FAKE_NVME_BAR_VA);
        if ((off & 7ULL) != 0 || (off != 0x00 && off != 0x28 && off != 0x30)) {
            printf("FATAL|unsupported_nvme_read64 off=%#llx\n", (unsigned long long)off);
            exit(2);
        }
        uint64_t v = read_le64_raw(nvme_bar + off);
        mmio_order++;
        printf("MMIO|nvme|off=%#llx|w=64|dir=R|val=%#llx|seq=%llu\n",
               (unsigned long long)off, (unsigned long long)v, (unsigned long long)mmio_order);
        return v;
    }
    uint8_t *p = resolve_dma_addr(a, 8);
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
    if (a >= FAKE_NVME_BAR_VA && a + 4 <= FAKE_NVME_BAR_VA + sizeof(nvme_bar)) {
        uint64_t off = (uint64_t)(a - FAKE_NVME_BAR_VA);
        uint64_t db_stride = 4ULL << cfg_dstrd;
        uint64_t sq1tdbl = 0x1000ULL + 2ULL * db_stride;
        uint64_t cq1hdbl = 0x1000ULL + 3ULL * db_stride;
        if ((off & 3ULL) != 0 ||
            (off != 0x14 && off != 0x24 && off != 0x1000 && off != 0x1004 && off != sq1tdbl && off != cq1hdbl)) {
            printf("FATAL|unsupported_nvme_write32 off=%#llx val=%#x\n",
                   (unsigned long long)off, (unsigned)value);
            exit(2);
        }
        if (off == 0x24 && (read_le32_raw(nvme_bar + 0x14) & 1u) != 0) {
            printf("ORDER_ERR|aqa_written_while_enabled\n");
        }
        uint32_t prev_cc = read_le32_raw(nvme_bar + 0x14);
        write_le32_raw(nvme_bar + off, value);
        mmio_order++;
        printf("MMIO|nvme|off=%#llx|w=32|dir=W|val=%#x|seq=%llu\n",
               (unsigned long long)off, (unsigned)value, (unsigned long long)mmio_order);
        if (off == 0x14) {
            if ((prev_cc & 1u) != 0 && (value & 1u) == 0) {
                if (cfg_disable_rdy_delay == 0) {
                    write_le32_raw(nvme_bar + 0x1C, 0u);
                } else {
                    csts_disable_polls_left = cfg_disable_rdy_delay;
                }
            } else if ((prev_cc & 1u) == 0 && (value & 1u) != 0) {
                model_admin_sq_tail = 0;
                model_admin_cq_head = 0;
                model_admin_cq_phase = 1;
                if (cfg_enable_rdy_delay == 0) {
                    write_le32_raw(nvme_bar + 0x1C, 1u);
                } else {
                    csts_enable_polls_left = cfg_enable_rdy_delay;
                }
            }
        } else if (off == 0x1000) {
            handle_admin_doorbell(value);
            fence_since_last_sqe_write = false;
        } else if (off == 0x1004) {
            printf("ADMIN_CQHDBL|head=%u\n", (unsigned)value);
        } else if (off == sq1tdbl) {
            handle_io_doorbell(value);
            fence_since_last_sqe_write = false;
        } else if (off == cq1hdbl) {
            printf("IO_CQHDBL|head=%u\n", (unsigned)value);
        }
        return;
    }
    uint8_t *p = resolve_dma_addr(a, 4);
    if (!p) {
        printf("FATAL|bad_write32 addr=%p\n", (void *)addr);
        exit(2);
    }
    write_le32_raw(p, value);
    fence_since_last_sqe_write = false;
    dma_writes++;
    dma_hash_mix(((uint64_t)0x14 << 56) | value);
}

void sexos_volatile_write64(uint64_t *addr, uint64_t value)
{
    uintptr_t a = (uintptr_t)addr;
    if (a >= FAKE_NVME_BAR_VA && a + 8 <= FAKE_NVME_BAR_VA + sizeof(nvme_bar)) {
        uint64_t off = (uint64_t)(a - FAKE_NVME_BAR_VA);
        if ((off & 7ULL) != 0 || (off != 0x28 && off != 0x30)) {
            printf("FATAL|unsupported_nvme_write64 off=%#llx\n", (unsigned long long)off);
            exit(2);
        }
        if ((read_le32_raw(nvme_bar + 0x14) & 1u) != 0) {
            printf("ORDER_ERR|queue_addr_written_while_enabled off=%#llx\n", (unsigned long long)off);
        }
        write_le64_raw(nvme_bar + off, value);
        mmio_order++;
        printf("MMIO|nvme|off=%#llx|w=64|dir=W|val=%#llx|seq=%llu\n",
               (unsigned long long)off, (unsigned long long)value, (unsigned long long)mmio_order);
        return;
    }
    uint8_t *p = resolve_dma_addr(a, 8);
    if (!p) {
        printf("FATAL|bad_write64 addr=%p\n", (void *)addr);
        exit(2);
    }
    write_le64_raw(p, value);
    fence_since_last_sqe_write = false;
    dma_writes++;
    dma_hash_mix(((uint64_t)0x18 << 56) ^ value);
}

void sexos_compiler_fence_seq_cst(void)
{
    fence_since_last_sqe_write = true;
    printf("FENCE|seq_cst\n");
}

void sexos_sfence(void)
{
    printf("FENCE|sfence\n");
}

uint64_t sexos_map_pci_bar(uint64_t cap_slot, uint64_t bar_index, uint64_t map_size)
{
    printf("SYSCALL|map_pci_bar|slot=%llu|bar=%llu|size=%#llx\n",
           (unsigned long long)cap_slot, (unsigned long long)bar_index, (unsigned long long)map_size);
    if (cap_slot == SLOT_NVME_HOST) {
        return cfg_nvme_bar_fail ? UINT64_MAX : FAKE_NVME_BAR_VA;
    }
    if (cap_slot == SLOT_USB_HOST) {
        return cfg_xhci_bar_fail ? UINT64_MAX : FAKE_XHCI_BAR_VA;
    }
    return UINT64_MAX;
}

uint64_t sexos_alloc_shared_buffer(uint64_t size, uint64_t consumer_domain)
{
    printf("SYSCALL|alloc_shared_buffer|size=%llu|domain=%llu\n",
           (unsigned long long)size, (unsigned long long)consumer_domain);
    return (uint64_t)(uintptr_t)shared_fb;
}

uint64_t sexos_allocate_memory(uint64_t size)
{
    if (dma_page_count >= MAX_DMA_PAGES || size != 4096ULL) {
        return UINT64_MAX;
    }
    DmaPage *pg = &dma_pages[dma_page_count];
    pg->phys = FAKE_PHYS_BASE + (uint64_t)dma_page_count * 0x1000ULL;
    pg->mem = (uint8_t *)calloc(1, 4096);
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
        printf("SYSCALL|map_phys|phys=%#llx|size=%#llx->FAIL\n",
               (unsigned long long)phys, (unsigned long long)size);
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
    printf("SYSCALL|map_mem_lend|slot=%llu\n", (unsigned long long)cap_slot);
    if (cfg_memlend_fail_once) {
        cfg_memlend_fail_once = false;
        return UINT64_MAX;
    }
    return (uint64_t)(uintptr_t)lend_buf;
}

MemoryRegionReply sexos_memory_region_info(uint64_t slot)
{
    if (slot != SLOT_BUF_LEND) return (MemoryRegionReply){.status=UINT64_MAX};
    return (MemoryRegionReply){0,4096,REGION_QUERY|REGION_READ|REGION_WRITE,writer_state.home,4};
}

MemoryRegionReply sexos_memory_region_view(uint64_t slot,uint64_t offset,uint64_t length,uint64_t rights)
{
    printf("SYSCALL|map_mem_lend|slot=%llu\n", (unsigned long long)slot);
    if (slot != SLOT_BUF_LEND || offset > 4096 || length > 4096-offset ||
        (rights & ~(REGION_QUERY|REGION_READ|REGION_WRITE)))
        return (MemoryRegionReply){.status=UINT64_MAX};
    if (cfg_memlend_fail_once) { cfg_memlend_fail_once=false; return (MemoryRegionReply){.status=UINT64_MAX}; }
    return (MemoryRegionReply){0,(uint64_t)(uintptr_t)(lend_buf+offset),length,0,4};
}

RegionWriterReceipt sexos_region_writer_query(uint64_t slot)
{
    if (slot != SLOT_BUF_LEND) return (RegionWriterReceipt){.status=REGION_WRITER_INVALID};
    return (RegionWriterReceipt){REGION_WRITER_OK,writer_state.revision,writer_state.current,
        writer_state.job,writer_state.peer};
}

RegionWriterReceipt sexos_region_writer_change(uint64_t slot,RegionWriterAction action,
                                                uint64_t expected,uint64_t job)
{
    if (slot != SLOT_BUF_LEND || expected != writer_state.revision || !job)
        return (RegionWriterReceipt){.status=REGION_WRITER_STALE,.revision=writer_state.revision,
            .current=writer_state.current,.job=writer_state.job,.peer=writer_state.peer};
    if (action == REGION_WRITER_CLAIM && writer_state.current == writer_state.home && writer_state.job == job)
        writer_state.current=writer_state.peer;
    else if (action == REGION_WRITER_RELEASE && writer_state.current == writer_state.peer && writer_state.job == job) {
        writer_state.current=writer_state.home;writer_state.job=0;
    } else return (RegionWriterReceipt){.status=REGION_WRITER_BUSY,.revision=writer_state.revision,
        .current=writer_state.current,.job=writer_state.job,.peer=writer_state.peer};
    ++writer_state.revision;
    return (RegionWriterReceipt){REGION_WRITER_OK,writer_state.revision,writer_state.current,
        writer_state.job,writer_state.peer};
}

uint64_t sexos_set_state(uint64_t state)
{
    printf("SYSCALL|set_state|state=%llu\n", (unsigned long long)state);
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
    {
        char pat[32];
        snprintf(pat, sizeof(pat), "0x%llx", (unsigned long long)(uintptr_t)lend_buf);
        char *pos;
        while ((pos = strstr(out, pat)) != NULL) {
            char tmp[1024];
            size_t prefix = (size_t)(pos - out);
            snprintf(tmp, sizeof(tmp), "%.*s0x8f000000%s", (int)prefix, out, pos + strlen(pat));
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
    uint64_t norm_a0 = (arg0 == (uint64_t)(uintptr_t)shared_fb) ? FAKE_SHARED_BUF_TOKEN : arg0;
    printf("PDX_CALL|slot=%llu|op=%#llx|a0=%#llx|a1=%#llx|a2=%#llx\n",
           (unsigned long long)slot, (unsigned long long)opcode,
           (unsigned long long)norm_a0, (unsigned long long)arg1, (unsigned long long)arg2);
    SexosReply r = {0, 0};
    return r;
}

SexosMessage sexos_listen_once(uint64_t slot)
{
    if (msg_index >= msg_count) {
        printf("FINAL|mmio_order=%llu|dma_reads=%llu|dma_writes=%llu|spin_hints=%llu|fb0=%#x|dma_fnv=%#018llx\n",
               (unsigned long long)mmio_order,
               (unsigned long long)dma_reads,
               (unsigned long long)dma_writes,
               (unsigned long long)spin_hints,
               (unsigned)shared_fb[0],
               (unsigned long long)dma_fnv);
        exit(0);
    }
    ScriptMsg *m = &msg_queue[msg_index++];
    if ((m->type_id == BLOCK_READ || m->type_id == BLOCK_WRITE) && m->arg2 == SLOT_BUF_LEND) {
        writer_state.home=m->caller_pd;writer_state.current=m->caller_pd;
        writer_state.peer=2;writer_state.job=m->type_id==BLOCK_READ?msg_index:0;
        writer_state.revision=msg_index*4+1;
    }
    if (m->fill_lend) {
        for (size_t i = 0; i < 512; ++i) {
            lend_buf[i] = (uint8_t)((i & 0xFFu) ^ m->lend_seed);
        }
    }
    printf("LISTEN|slot=%llu|idx=%zu|type=%#llx|caller=%u|a0=%#llx|a1=%#llx|a2=%#llx\n",
           (unsigned long long)slot, msg_index - 1,
           (unsigned long long)m->type_id, (unsigned)m->caller_pd,
           (unsigned long long)m->arg0, (unsigned long long)m->arg1, (unsigned long long)m->arg2);
    SexosMessage out = {m->type_id, m->arg0, m->arg1, m->arg2, m->caller_pd, 0};
    return out;
}

uint64_t sexos_reply(uint32_t target_pd, uint64_t value)
{
    printf("REPLY|target=%u|val=%#llx\n", (unsigned)target_pd, (unsigned long long)value);
    if (msg_index > 0 && msg_queue[msg_index - 1].type_id == BLOCK_READ &&
        msg_queue[msg_index - 1].arg2 == SLOT_BUF_LEND && value == 0) {
        uint64_t w0 = read_le64_raw(lend_buf);
        printf("LEND_READ_HEAD|w0=%#llx\n", (unsigned long long)w0);
    }
    return 0;
}

void sexos_yield(void)
{
    yield_count++;
    printf("YIELD|count=%u\n", (unsigned)yield_count);
    if (yield_count >= 2) {
        printf("FINAL|mmio_order=%llu|dma_reads=%llu|dma_writes=%llu|spin_hints=%llu|fb0=%#x|dma_fnv=%#018llx\n",
               (unsigned long long)mmio_order,
               (unsigned long long)dma_reads,
               (unsigned long long)dma_writes,
               (unsigned long long)spin_hints,
               (unsigned)shared_fb[0],
               (unsigned long long)dma_fnv);
        exit(0);
    }
}

void sexos_spin_loop_hint(void)
{
    spin_hints++;
}

_Noreturn void sexos_spin_forever(void)
{
    printf("SPIN_FOREVER\n");
    exit(0);
}

static void push_msg(uint64_t type_id, uint32_t caller, uint64_t a0, uint64_t a1, uint64_t a2, bool fill, uint8_t seed)
{
    if (msg_count < sizeof(msg_queue) / sizeof(msg_queue[0])) {
        msg_queue[msg_count++] = (ScriptMsg){type_id, caller, a0, a1, a2, fill, seed};
    }
}

static void setup_case(int c)
{
    current_case = c;
    memset(nvme_bar, 0, sizeof(nvme_bar));
    memset(xhci_bar, 0, sizeof(xhci_bar));
    memset(lend_buf, 0, sizeof(lend_buf));
    memset(shared_fb, 0, sizeof(shared_fb));
    memset(disk_data, 0, sizeof(disk_data));

    /* Populate LBA 0 and LBA 1 (superblock) and pre-populate AP5A blocks 256..259 */
    write_le64_raw(disk_data[0] + 0, UINT64_C(0x1122334455667788));
    write_le64_raw(disk_data[0] + 8, UINT64_C(0x99aabbccddeeff00));
    write_le64_raw(disk_data[1] + 0, UINT64_C(0x53455846535f5630));
    write_le64_raw(disk_data[1] + 8, UINT64_C(0x3030303153584553));
    write_le32_raw(disk_data[1] + 16, 4u);
    write_le32_raw(disk_data[1] + 20, 512u);
    write_le64_raw(disk_data[1] + 24, 2048ULL);
    write_le64_raw(disk_data[1] + 40, 64ULL);
    write_le64_raw(disk_data[1] + 48, 1ULL);
    for (uint64_t b = 0; b < 4; ++b) {
        for (size_t i = 0; i < 512; ++i) {
            disk_data[256 + b][i] = (uint8_t)((0x5Au ^ (uint8_t)i ^ (uint8_t)((uint8_t)b * 0x21u)) ^ 0xC3u);
        }
    }

    write_le32_raw(xhci_bar + 0x00, 0x01000020u);
    write_le32_raw(xhci_bar + 0x04, 0x04000801u);
    write_le32_raw(xhci_bar + 0x10, 0x00080001u);

    if (c == 0) {
        /* Preconfigured Admin queues + full block service coverage + queue wraparound */
        cfg_preconfigured = true;
        cfg_dstrd = 0;
        uint64_t asq_phys = sexos_allocate_memory(4096);
        uint64_t acq_phys = sexos_allocate_memory(4096);
        uint64_t cap = ((uint64_t)cfg_dstrd << 32) | (10ULL << 24) | 0x3FFULL;
        write_le64_raw(nvme_bar + 0x00, cap);
        write_le32_raw(nvme_bar + 0x08, 0x00010400u);
        write_le32_raw(nvme_bar + 0x14, (4u << 20) | (6u << 16) | 1u);
        write_le32_raw(nvme_bar + 0x1C, 1u);
        write_le32_raw(nvme_bar + 0x24, 0x000F000Fu);
        write_le64_raw(nvme_bar + 0x28, asq_phys);
        write_le64_raw(nvme_bar + 0x30, acq_phys);

        push_msg(BLOCK_READ, 3, 512ULL, 512ULL, SLOT_BUF_LEND, false, 0);
        push_msg(BLOCK_WRITE, 3, 2046ULL * 512ULL, 512ULL, SLOT_BUF_LEND, true, 0x3Cu);
        push_msg(BLOCK_READ, 3, 2046ULL * 512ULL, 512ULL, SLOT_BUF_LEND, false, 0);
        push_msg(BLOCK_WRITE, 3, 2047ULL * 512ULL, 512ULL, SLOT_BUF_LEND, true, 0x21u);
        push_msg(BLOCK_WRITE, 3, 128ULL * 512ULL, 512ULL, SLOT_BUF_LEND, true, 0x55u);
        push_msg(BLOCK_WRITE, 3, 60ULL * 512ULL, 512ULL, SLOT_BUF_LEND, true, 0x11u);
        push_msg(BLOCK_READ, 4, 0ULL, 512ULL, 99ULL, false, 0);
        push_msg(BLOCK_READ, 4, 0ULL, 512ULL, 0ULL, false, 0);
        push_msg(BLOCK_READ, 4, 0ULL, 256ULL, 0ULL, false, 0);
        push_msg(BLOCK_READ, 4, 0ULL, 0ULL, SLOT_BUF_LEND, false, 0);
        push_msg(BLOCK_READ, 4, 0ULL, 8192ULL, 0ULL, false, 0);
        push_msg(BLOCK_READ, 4, 123ULL, 512ULL, SLOT_BUF_LEND, false, 0);
        push_msg(BLOCK_READ, 4, 123ULL, 512ULL, 0ULL, false, 0);
        push_msg(BLOCK_READ, 4, 512ULL, 1024ULL, SLOT_BUF_LEND, false, 0);
        push_msg(BLOCK_SYNC, 3, 0ULL, 0ULL, 0ULL, false, 0);
        push_msg(0x99ULL, 3, 0ULL, 0ULL, 0ULL, false, 0);
        /* Wrap IO queue (> 16 submissions) */
        for (int i = 0; i < 16; ++i) {
            push_msg(BLOCK_READ, 5, (uint64_t)((128 + (i % 8)) * 512), 512ULL, SLOT_BUF_LEND, false, 0);
        }
    } else if (c == 1) {
        /* Unconfigured Admin queues (cc_en=0, csts_rdy=0) -> program + reprovision + delayed CQE + dstrd=1 */
        cfg_preconfigured = false;
        cfg_dstrd = 1;
        cfg_disable_rdy_delay = 2;
        cfg_enable_rdy_delay = 3;
        cfg_admin_cqe_delay = 2;
        cfg_io_cqe_delay = 3;
        uint64_t cap = ((uint64_t)cfg_dstrd << 32) | (4ULL << 24) | 0x3FFULL;
        write_le64_raw(nvme_bar + 0x00, cap);
        write_le32_raw(nvme_bar + 0x08, 0x00010400u);
        write_le32_raw(nvme_bar + 0x14, (4u << 20) | (6u << 16));
        write_le32_raw(nvme_bar + 0x1C, 0u);
        write_le32_raw(nvme_bar + 0x24, 0u);
        write_le64_raw(nvme_bar + 0x28, 0ULL);
        write_le64_raw(nvme_bar + 0x30, 0ULL);

        push_msg(BLOCK_READ, 3, 512ULL, 512ULL, SLOT_BUF_LEND, false, 0);
        push_msg(BLOCK_WRITE, 3, 200ULL * 512ULL, 512ULL, SLOT_BUF_LEND, true, 0x77u);
        push_msg(BLOCK_SYNC, 3, 0ULL, 0ULL, 0ULL, false, 0);
    } else if (c == 2) {
        /* NVMe BAR0 map fails -> xHCI MMIO probe succeeds -> not-ready replies */
        cfg_nvme_bar_fail = true;
        cfg_xhci_bar_fail = false;
        push_msg(BLOCK_READ, 3, 512ULL, 512ULL, SLOT_BUF_LEND, false, 0);
        push_msg(BLOCK_WRITE, 3, 128ULL * 512ULL, 512ULL, SLOT_BUF_LEND, true, 0x22u);
        push_msg(BLOCK_SYNC, 3, 0ULL, 0ULL, 0ULL, false, 0);
    } else if (c == 3) {
        /* Both NVMe and xHCI probe fail -> empty message queue */
        cfg_nvme_bar_fail = true;
        cfg_xhci_bar_fail = true;
    } else {
        /* Deterministic PRNG fuzz cases */
        uint32_t rng = (uint32_t)(c * 2654435761u + 17u);
        cfg_preconfigured = (rng & 1u) != 0;
        cfg_dstrd = (rng >> 1) & 3u;
        cfg_disable_rdy_delay = (rng >> 3) % 3u;
        cfg_enable_rdy_delay = (rng >> 5) % 3u;
        cfg_admin_cqe_delay = (rng >> 7) % 3u;
        cfg_io_cqe_delay = (rng >> 9) % 4u;
        cfg_memlend_fail_once = ((rng >> 11) & 3u) == 1u;
        cfg_io_status_fail_once = ((rng >> 13) & 3u) == 2u;

        uint64_t cap = ((uint64_t)cfg_dstrd << 32) | (2ULL << 24) | 0x3FFULL;
        write_le64_raw(nvme_bar + 0x00, cap);
        write_le32_raw(nvme_bar + 0x08, 0x00010400u);
        if (cfg_preconfigured) {
            uint64_t asq_phys = sexos_allocate_memory(4096);
            uint64_t acq_phys = sexos_allocate_memory(4096);
            write_le32_raw(nvme_bar + 0x14, (4u << 20) | (6u << 16) | 1u);
            write_le32_raw(nvme_bar + 0x1C, 1u);
            write_le32_raw(nvme_bar + 0x24, 0x000F000Fu);
            write_le64_raw(nvme_bar + 0x28, asq_phys);
            write_le64_raw(nvme_bar + 0x30, acq_phys);
        } else {
            write_le32_raw(nvme_bar + 0x14, (4u << 20) | (6u << 16));
            write_le32_raw(nvme_bar + 0x1C, 0u);
        }

        int n_ops = 4 + (int)(rng % 6u);
        for (int i = 0; i < n_ops; ++i) {
            rng = rng * 1664525u + 1013904223u;
            uint32_t kind = rng % 5u;
            if (kind == 0) {
                uint64_t lba = 128ULL + (rng >> 8) % 100ULL;
                push_msg(BLOCK_WRITE, 3, lba * 512ULL, 512ULL, SLOT_BUF_LEND, true, (uint8_t)(rng & 0xFFu));
            } else if (kind == 1) {
                uint64_t lba = (rng >> 8) % 128ULL;
                push_msg(BLOCK_READ, 3, lba * 512ULL, 512ULL, SLOT_BUF_LEND, false, 0);
            } else if (kind == 2) {
                push_msg(BLOCK_READ, 4, 0ULL, 512ULL, 77ULL, false, 0);
            } else if (kind == 3) {
                push_msg(BLOCK_SYNC, 3, 0ULL, 0ULL, 0ULL, false, 0);
            } else {
                uint64_t lba = 48ULL + (rng >> 8) % 60ULL;
                push_msg(BLOCK_WRITE, 3, lba * 512ULL, 512ULL, SLOT_BUF_LEND, true, 0x42u);
            }
        }
    }
}

int main(int argc, char **argv)
{
    int c = argc > 1 ? atoi(argv[1]) : 0;
    setup_case(c);
    sexdrive_main();
    return 0;
}
"""

RUST_WRAPPER_PREFIX = r"""
#![allow(dead_code, unused_unsafe, unused_variables, non_snake_case, unused_imports)]

extern "C" {
    fn sexos_volatile_read8(addr: *const u8) -> u8;
    fn sexos_volatile_read16(addr: *const u16) -> u16;
    fn sexos_volatile_read32(addr: *const u32) -> u32;
    fn sexos_volatile_read64(addr: *const u64) -> u64;
    fn sexos_volatile_write8(addr: *mut u8, val: u8);
    fn sexos_volatile_write16(addr: *mut u16, val: u16);
    fn sexos_volatile_write32(addr: *mut u32, val: u32);
    fn sexos_volatile_write64(addr: *mut u64, val: u64);
    fn sexos_compiler_fence_seq_cst();
    fn sexos_map_pci_bar(cap_slot: u64, bar_index: u64, map_size: u64) -> u64;
    fn sexos_alloc_shared_buffer(size: u64, consumer_domain: u64) -> u64;
    fn sexos_allocate_memory(size: u64) -> u64;
    fn sexos_map_memory(phys: u64, size: u64) -> u64;
    fn sexos_map_mem_lend(cap_slot: u64) -> u64;
    fn sexos_set_state(state: u64) -> u64;
    fn sexos_pdx_call(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> SexosReply;
    fn sexos_listen_once(slot: u64) -> SexosMessage;
    fn sexos_reply(target_pd: u32, value: u64) -> u64;
    fn sexos_yield();
    fn sexos_spin_loop_hint();
    fn sexos_spin_forever() -> !;
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct SexosReply {
    pub status: u64,
    pub value: u64,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct SexosMessage {
    pub type_id: u64,
    pub arg0: u64,
    pub arg1: u64,
    pub arg2: u64,
    pub caller_pd: u32,
    pub padding: u32,
}

pub trait HostedVolatile: Copy {
    unsafe fn vread(addr: *const Self) -> Self;
    unsafe fn vwrite(addr: *mut Self, val: Self);
}

impl HostedVolatile for u8 {
    unsafe fn vread(addr: *const Self) -> Self { sexos_volatile_read8(addr) }
    unsafe fn vwrite(addr: *mut Self, val: Self) { sexos_volatile_write8(addr, val); }
}

impl HostedVolatile for u16 {
    unsafe fn vread(addr: *const Self) -> Self { sexos_volatile_read16(addr) }
    unsafe fn vwrite(addr: *mut Self, val: Self) { sexos_volatile_write16(addr, val); }
}

impl HostedVolatile for u32 {
    unsafe fn vread(addr: *const Self) -> Self { sexos_volatile_read32(addr) }
    unsafe fn vwrite(addr: *mut Self, val: Self) { sexos_volatile_write32(addr, val); }
}

impl HostedVolatile for u64 {
    unsafe fn vread(addr: *const Self) -> Self { sexos_volatile_read64(addr) }
    unsafe fn vwrite(addr: *mut Self, val: Self) { sexos_volatile_write64(addr, val); }
}

#[inline(always)]
pub unsafe fn hosted_read_volatile<T: HostedVolatile>(addr: *const T) -> T {
    T::vread(addr)
}

#[inline(always)]
pub unsafe fn hosted_write_volatile<T: HostedVolatile>(addr: *mut T, val: T) {
    T::vwrite(addr, val);
}

pub mod sex_pdx {
    use super::*;

    pub const SLOT_KERNEL: u64 = 0;
    pub const SLOT_SHELL: u64 = 6;
    pub const SLOT_USB_HOST: u64 = 8;
    pub const SLOT_BUF_LEND: u64 = 17;

    pub const BLOCK_READ: u64 = 1;
    pub const BLOCK_WRITE: u64 = 2;
    pub const BLOCK_SYNC: u64 = 3;
    pub const BLOCK_ERR_BAD_CMD: u64 = 1;
    pub const BLOCK_ERR_BAD_LEN: u64 = 2;
    pub const BLOCK_ERR_NO_DEVICE: u64 = 4;
    pub const BLOCK_SECTOR_SIZE: u64 = 512;
    pub const BLOCK_MAX_XFER: u64 = 4096;

    pub const SEXDRIVE_AP3_WRITE_PROOF_LBA: u64 = 2047;
    pub const SEXDRIVE_AP4_MULTI_BASE_LBA: u64 = 128;
    pub const SEXDRIVE_AP4_MULTI_SECTORS: u64 = 4;
    pub const SEXDRIVE_AP5A_PERSIST_BASE_LBA: u64 = 256;
    pub const SEXDRIVE_AP5A_PERSIST_SECTORS: u64 = 4;
    pub const SEXDRIVE_AP6_NEG_MISMATCH_LBA: u64 = 384;
    pub const DISKFS_MANIFEST_LBA: u64 = 2046;
    pub const DISKFS_SEXFILES_PROOF_START_LBA: u64 = 2038;
    pub const DISKFS_SEXFILES_PROOF_SECTORS: u64 = 8;
    pub const DISKFS_LINEN_OBJECT_START_LBA: u64 = 2030;
    pub const DISKFS_LINEN_OBJECT_SECTORS: u64 = 8;
    pub const DISKFS_QUIL_OBJECT_START_LBA: u64 = 2022;
    pub const DISKFS_QUIL_OBJECT_SECTORS: u64 = 8;
    pub const SEXFS_V0_META_START_LBA: u64 = 0;
    pub const SEXFS_V0_META_END_LBA: u64 = 47;
    pub const SEXFS_V0_OBJECT_START_LBA: u64 = 128;
    pub const SEXFS_V0_OBJECT_END_LBA: u64 = 2019;

    pub type PdxMessage = super::SexosMessage;

    pub fn raw_print(s: &str) {
        unsafe {
            let _ = sexos_pdx_call(SLOT_KERNEL, 0x45, s.as_ptr() as u64, s.len() as u64, 0);
        }
    }

    pub fn pdx_call(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> u64 {
        let r = unsafe { sexos_pdx_call(slot, opcode, arg0, arg1, arg2) };
        r.value
    }

    pub fn pdx_try_listen_raw(slot: u64) -> Option<PdxMessage> {
        let msg = unsafe { sexos_listen_once(slot) };
        if msg.type_id == 0 {
            sys_yield();
            None
        } else {
            Some(msg)
        }
    }

    pub fn pdx_reply(caller_pd: u32, val: u64) {
        unsafe { let _ = sexos_reply(caller_pd, val); }
    }

    pub fn sys_yield() {
        unsafe { sexos_yield(); }
    }

    pub fn sys_map_mem_lend(slot: u64) -> u64 {
        unsafe { sexos_map_mem_lend(slot) }
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
        "wrong_nvme_cc_offset",
        "(var uint32_t nvme_cc (call sexos_volatile_read32 (cast ((const uint32_t) *) (cast uintptr_t (+ map_va 0x0014ULL)))))",
        "(var uint32_t nvme_cc (call sexos_volatile_read32 (cast ((const uint32_t) *) (cast uintptr_t (+ map_va 0x0018ULL)))))",
        0,
    ),
    (
        "wrong_nvme_csts_offset",
        "(var uint32_t nvme_csts (call sexos_volatile_read32 (cast ((const uint32_t) *) (cast uintptr_t (+ map_va 0x001CULL)))))",
        "(var uint32_t nvme_csts (call sexos_volatile_read32 (cast ((const uint32_t) *) (cast uintptr_t (+ map_va 0x0020ULL)))))",
        0,
    ),
    (
        "wrong_nvme_cap_width_32",
        "(var uint64_t nvme_cap (call sexos_volatile_read64 (cast ((const uint64_t) *) (cast uintptr_t (+ map_va 0x0000ULL)))))",
        "(var uint64_t nvme_cap (cast uint64_t (call sexos_volatile_read32 (cast ((const uint32_t) *) (cast uintptr_t (+ map_va 0x0000ULL))))))",
        0,
    ),
    (
        "missing_pre_doorbell_fence_typed_read",
        "(call sexos_compiler_fence_seq_cst)\n      (= sq_tail (% (+ sq_tail 1u) 16u))",
        "(= sq_tail (% (+ sq_tail 1u) 16u))",
        0,
    ),
    (
        "swapped_reprovision_aqa_after_asq",
        "(call sexos_volatile_write32 (cast (uint32_t *) (cast uintptr_t (+ map_va 0x0024ULL))) new_aqa)\n      (call sexos_volatile_write64 (cast (uint64_t *) (cast uintptr_t (+ map_va 0x0028ULL))) asq_phys)\n      (call sexos_volatile_write64 (cast (uint64_t *) (cast uintptr_t (+ map_va 0x0030ULL))) acq_phys)",
        "(call sexos_volatile_write64 (cast (uint64_t *) (cast uintptr_t (+ map_va 0x0028ULL))) asq_phys)\n      (call sexos_volatile_write32 (cast (uint32_t *) (cast uintptr_t (+ map_va 0x0024ULL))) new_aqa)\n      (call sexos_volatile_write64 (cast (uint64_t *) (cast uintptr_t (+ map_va 0x0030ULL))) acq_phys)",
        0,
    ),
    (
        "duplicated_doorbell_submission",
        '(call sexos_volatile_write32 (cast (uint32_t *) (cast uintptr_t (+ map_va sq1tdbl))) sq_tail)\n      (if (call hot_log)\n        (call sexos_line "[sexdrive.nvme.doorbell] path=typed qid=1 tail=%llu"',
        '(call sexos_volatile_write32 (cast (uint32_t *) (cast uintptr_t (+ map_va sq1tdbl))) sq_tail)\n      (call sexos_volatile_write32 (cast (uint32_t *) (cast uintptr_t (+ map_va sq1tdbl))) sq_tail)\n      (if (call hot_log)\n        (call sexos_line "[sexdrive.nvme.doorbell] path=typed qid=1 tail=%llu"',
        0,
    ),
    (
        "broken_cq_phase_toggle_on_wraparound",
        "(= cq_head (% (+ cq_head 1u) 16u))\n      (if (== cq_head 0u)\n        (^= cq_phase 1u))",
        "(= cq_head (% (+ cq_head 1u) 16u))\n      (if (== cq_head 0u)\n        (= cq_phase 1u))",
        0,
    ),
    (
        "wrong_read_sqe_opcode",
        "(call sexos_volatile_write32 (+ sqe_ptr 0) (| 0x02u (<< (cast uint32_t cid) 16)))",
        "(call sexos_volatile_write32 (+ sqe_ptr 0) (| 0x01u (<< (cast uint32_t cid) 16)))",
        0,
    ),
    (
        "wrong_write_sqe_opcode",
        "(call sexos_volatile_write32 (+ sqe_ptr 0) (| 0x01u (<< (cast uint32_t write_cid) 16)))",
        "(call sexos_volatile_write32 (+ sqe_ptr 0) (| 0x02u (<< (cast uint32_t write_cid) 16)))",
        0,
    ),
    (
        "wrong_admin_identify_cns",
        "(call sexos_volatile_write32 (+ sqe_ptr 0) (| 0x06u (<< (cast uint32_t cid) 16)))\n      (call sexos_volatile_write32 (+ sqe_ptr 6) (cast uint32_t (& id_phys 0xFFFFFFFFULL)))\n      (call sexos_volatile_write32 (+ sqe_ptr 7) (cast uint32_t (>> id_phys 32)))\n      (call sexos_volatile_write32 (+ sqe_ptr 10) 1u)",
        "(call sexos_volatile_write32 (+ sqe_ptr 0) (| 0x06u (<< (cast uint32_t cid) 16)))\n      (call sexos_volatile_write32 (+ sqe_ptr 6) (cast uint32_t (& id_phys 0xFFFFFFFFULL)))\n      (call sexos_volatile_write32 (+ sqe_ptr 7) (cast uint32_t (>> id_phys 32)))\n      (call sexos_volatile_write32 (+ sqe_ptr 10) 0u)",
        0,
    ),
    (
        "wrong_io_queue_entries_encoding",
        "(call sexos_volatile_write32 (+ cq_sqe_ptr 10) (| 1u (<< 15u 16)))",
        "(call sexos_volatile_write32 (+ cq_sqe_ptr 10) (| 1u (<< 16u 16)))",
        0,
    ),
    (
        "wrong_doorbell_stride_shift",
        "(var uint64_t sq1tdbl (+ 0x1000ULL (<< (* 2ULL 1ULL) (+ 2u (cast uint32_t cap_dstrd)))))",
        "(var uint64_t sq1tdbl (+ 0x1000ULL (<< (* 2ULL 1ULL) (+ 3u (cast uint32_t cap_dstrd)))))",
        1,
    ),
    (
        "wrong_sexfs_write_min_lba",
        '(define SEXFS_V0_OBJECT_START_LBA "UINT64_C(128)")',
        '(define SEXFS_V0_OBJECT_START_LBA "UINT64_C(256)")',
        0,
    ),
    (
        "wrong_cqe_status_shift",
        "(var uint32_t sf (& (>> cqe_dw3 17) 0x7FFFu))",
        "(var uint32_t sf (& (>> cqe_dw3 16) 0x7FFFu))",
        0,
    ),
    (
        "missing_cq1hdbl_write_in_bounce_read",
        "(call sexos_volatile_write32 (cast (uint32_t *) (cast uintptr_t (+ map_va cq1hdbl))) cq_head)\n      (= (. (. sexdrive_state nvme_io_state) sq_tail) sq_tail)",
        "(= (. (. sexdrive_state nvme_io_state) sq_tail) sq_tail)",
        0,
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
    raw = run_cmd(["git", "-C", str(REPO), "show", f"{UPSTREAM_REV}:apps/sexdrive/src/main.rs"]).stdout
    sha = hashlib.sha256(raw.encode("utf-8")).hexdigest()
    if EXPECTED_RUST_SHA256 is not None and sha != EXPECTED_RUST_SHA256:
        raise AssertionError(f"unexpected Rust SHA256: {sha} != {EXPECTED_RUST_SHA256}")

    src = raw
    src = src.replace("#![no_std]", "")
    src = src.replace("#![no_main]", "")
    src = re.sub(
        r"struct DummyAllocator;.*?pub unsafe extern \"C\" fn memmove\(.*?\n\}",
        "",
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
    src = re.sub(
        r"let map_va: u64;\s*unsafe \{\s*// syscall 43 = MAP_PCI_BAR\(cap_slot, bar_index, map_size\)\s*core::arch::asm!\(\s*\"syscall\",\s*in\(\"rax\"\) 43u64,\s*in\(\"rdi\"\) SLOT_NVME_HOST,.*?\);\s*\}",
        "let map_va: u64 = unsafe { sexos_map_pci_bar(SLOT_NVME_HOST, 0, 0x4000) };",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = re.sub(
        r"let map_va: u64;\s*unsafe \{\s*// syscall 43 = MAP_PCI_BAR\(cap_slot, bar_index, map_size\)\s*core::arch::asm!\(\s*\"syscall\",\s*in\(\"rax\"\) 43u64,\s*in\(\"rdi\"\) SLOT_USB_HOST,.*?\);\s*\}",
        "let map_va: u64 = unsafe { sexos_map_pci_bar(SLOT_USB_HOST, 0, 0x1000) };",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = re.sub(
        r"let shared_addr: u64;\s*unsafe \{\s*core::arch::asm!\(\s*\"syscall\",\s*in\(\"rax\"\) 40u64,.*?\);\s*\}",
        "let shared_addr: u64 = unsafe { sexos_alloc_shared_buffer(fb_size as u64, 1) };",
        src,
        count=1,
        flags=re.DOTALL,
    )
    src = src.replace("core::ptr::read_volatile(", "hosted_read_volatile(")
    src = src.replace("core::ptr::write_volatile(", "hosted_write_volatile(")
    src = src.replace("compiler_fence(Ordering::SeqCst)", "unsafe { sexos_compiler_fence_seq_cst() }")
    src = src.replace("loop { core::hint::spin_loop(); }", "unsafe { sexos_spin_forever() }")
    src = src.replace("core::hint::spin_loop()", "unsafe { sexos_spin_loop_hint() }")
    src = src.replace('pub extern "C" fn _start() -> !', 'pub extern "C" fn sexdrive_main()')
    return RUST_WRAPPER_PREFIX + "\n" + src, sha


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cases", type=int, default=8)
    parser.add_argument("--all-variants", action="store_true")
    args = parser.parse_args()

    cesta_bin = Path(shutil.which(str(args.cesta)) or args.cesta).resolve()
    cesta_src = HERE / "src/sexdrive.cesta"
    exported_c = HERE / "c/src/sexdrive.c"

    # Verify no preprocessor conditionals inside sexdrive function bodies
    cesta_text = LayoutText(cesta_src.read_text())
    fn_split = cesta_text.split("(fn ", 1)[1]
    if "cpp-if" in fn_split:
        raise AssertionError("cpp-if found inside function bodies of src/sexdrive.cesta")

    variants = ALL_VARIANTS if args.all_variants else CORE_VARIANTS

    with tempfile.TemporaryDirectory() as tmp:
        tmpdir = Path(tmp)
        provider_c = tmpdir / "provider.c"
        provider_c.write_text(PROVIDER_C)
        rust_rs = tmpdir / "rust_sexdrive.rs"
        rust_code, rust_sha = build_hosted_rust_source()
        rust_rs.write_text(rust_code)

        # Export authored Cesta source to C
        authored_c = tmpdir / "sexdrive_authored.c"
        exported = run_cmd([str(cesta_bin), "export-c", str(cesta_src), "--"]).stdout
        frozen = run_cmd([str(cesta_bin), "export-c", str(cesta_src), "--", "-DSEXOS_SEXDRIVE_CURRENT_DURABILITY=0"]).stdout
        authored_c.write_text(frozen)
        if exported_c.exists() and exported_c.read_text().strip() != exported.strip():
            raise AssertionError("c/src/sexdrive.c is out of date with src/sexdrive.cesta")

        common_c_sources = [
            str(HERE / "runtime/client_runtime.c"),
            str(HERE / "runtime/server_runtime.c"),
            str(HERE / "generated/runtime/format_bytes.c"),
            str(HERE / "generated/runtime/packed_words.c"),
        ]
        inc_flags = ["-I", str(HERE / "generated/include"), "-DSEXOS_HOST=1",
                     "-DSEXOS_SEXDRIVE_CURRENT_DURABILITY=0"]

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
                            sys.stderr.write(f"First diff at line {idx + 1}:\n  RUST: {rl}\n  CESTA: {gl}\n")
                            break
                    else:
                        sys.stderr.write(f"Length diff: rust={len(r_lines)} cesta={len(g_lines)}\n")
                    raise AssertionError(f"trace mismatch in variant={vname} case={c}")
                total_lines += len(r_out.splitlines())

        # Run 15 protocol & register mutations against default reference trace
        ref_bin = tmpdir / "rust_default"
        ref_traces = {c: run_cmd([str(ref_bin), str(c)]).stdout for c in range(2)}
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
            f"sexdrive differential: ok ({len(variants)} variants x {args.cases} cases, "
            f"{total_lines} trace lines, {killed}/{len(MUTATIONS)} mutations killed, "
            f"rust_sha256={rust_sha[:16]})"
        )


if __name__ == "__main__":
    main()
