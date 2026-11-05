#!/usr/bin/env python3
"""Compare servers/sexfiles with its Cesta translation.

The Rust server is compiled from the pinned revision against the pinned
crates/sex-object-model, with its 10 internal modules (vfs, messages, pdx,
trampoline, proof, sexobject, backends/mod, backends/ramfs, backends/tmpfs,
backends/diskfs) extracted verbatim from git. Only main.rs has its no_std
scaffolding removed, and crates/sex-pdx and crates/sex-rt are replaced by
wrappers that keep the pinned pdx_listen_raw, diskfs_v2_encode_*,
ranges_overlap, heap_init and expand_heap functions while routing syscalls
to a single C provider shared with the Cesta build.

The C provider serves:
  - MemLend buffer mapping at a fixed host address (0x600000000000) via
    sexos_grant_mem_lend and sexos_map_mem_lend (including negative capability
    checks and simulated grant failures).
  - A 2048-sector (1 MiB) in-memory block device on SLOT_BLOCK (slot 15)
    implementing BLOCK_READ, BLOCK_WRITE (with SexDrive's write_guard_allows),
    BLOCK_SYNC, and unknown-opcode rejection, plus optional pre-seeded V1/V2
    manifests and AP4 payloads, missing-image faults, and interleaved client
    requests arriving during block roundtrips (exercising defer_stash and
    defer_pop).
  - Client VFS requests across all 24 opcodes (RamFS 0x30-0x37, 0x3D, 0x3F;
    DiskFS V1-V4 0x38-0x3C, 0x3E, 0x42, 0x43, 0x47-0x4A; SexObject 0x40, 0x41)
    plus stray replies and unknown opcodes from multiple protection domains.
"""

import argparse
import os
from pathlib import Path
import random
import re
import shutil
from layout_text import LayoutText

from differential_support import (
    C_MAIN,
    LISTEN,
    PROVIDER_PRELUDE,
    RUST_MAIN,
    first_difference,
    pinned_function,
    run,
)

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
MEM_LEND_VA = 0x600000000000

NON_ISOLATED_ENV_FLAGS = [
    "SEXFILES_RAMFS_PROOF",
    "SEXOS_DISKFS_OBJECT_TABLE_PROOF",
    "SEXOS_SEXFILES_JOURNAL_PROOF",
    "SEXOS_SEXFILES_REPLAY_PROOF",
    "SEXOS_SEXFILES_CAP_RECORD_PROOF",
    "SEXOS_LINEN_SEXFILES_METADATA_PROOF",
    "SEXOS_SEXFILES_FAULT_INJECTION_PROOF",
    "SEXOS_SEXFILES_REAL_BLOCK_PROOF",
    "SEXOS_SEXFILES_REBOOT_PROOF",
    "SEXOS_SEXFILES_EXTENT_PROOF",
    "SEXOS_SEXFILES_CHECKPOINT_PROOF",
    "SEXOS_SEXOBJECT_VIEW_PROOF",
    "SEXOS_LINEN_DISK_OBJECT_PROOF",
    "SEXFS_V0_SUPERBLOCK_FORMAT_MOUNT_PROOF",
    "SEXOBJECT_TABLE_PERSIST_PROOF",
    "SEXOBJECT_TABLE_EXTENT_ALLOC_PROOF",
    "SEXOBJECT_EXTENT_WRITE_FULL_BLOCK_PROOF",
    "SEXOBJECT_WRITE_READ_PERSIST_PROOF",
    "SEXOBJECT_MULTI_OBJECT_PROOF",
]

ENV_FLAGS = set(NON_ISOLATED_ENV_FLAGS) | {"SEXFILES_ROUTE_AUDIT_ONLY", "SEXFILES_ISOLATE_PROOF"}

CFG_TO_C_DEFINE = {
    "sexfiles_diskfs100_ap2_proof": "SEXFILES_DISKFS_100_PROOF",
    "sexfiles_diskfs100_ap3_proof": "SEXFILES_DISKFS_100_AP3_PROOF",
    "sexfiles_diskfs_multi_object_proof": "sexfiles_diskfs_multi_object_proof",
    "sexfiles_diskfs100_ap4_write": "SEXFILES_DISKFS_100_AP4_WRITE",
    "sexfiles_diskfs100_ap4_read": "SEXFILES_DISKFS_100_AP4_READ",
    "sexfiles_diskfs100_ap5_neg_mismatch": "SEXFILES_DISKFS_100_AP5_NEG_MISMATCH",
    "sexfiles_diskfs100_ap5_neg_missing_image": "SEXFILES_DISKFS_100_AP5_NEG_MISSING_IMAGE",
    "sexfiles_diskfs100_ap5_neg_read_no_write": "SEXFILES_DISKFS_100_AP5_NEG_READ_NO_WRITE",
    "sexfiles_diskfs100_ap5_neg_flush_skip": "SEXFILES_DISKFS_100_AP5_NEG_FLUSH_SKIP",
    "sexfiles_diskfs100_ap6_flush_fsync": "SEXFILES_DISKFS_100_AP6_FLUSH_FSYNC",
    "sexfiles_diskfs_bridge_strict_proof": "SEXFILES_DISKFS_BRIDGE_STRICT_PROOF",
    "sexfiles_diskfs_negative_bounds_auth_proof": "SEXFILES_DISKFS_NEGATIVE_BOUNDS_AUTH_PROOF",
}

DEFAULT_VARIANTS = [
    (),
    (
        "SEXFILES_RAMFS_PROOF",
        "SEXOS_DISKFS_OBJECT_TABLE_PROOF",
        "SEXOS_SEXFILES_JOURNAL_PROOF",
        "SEXOS_SEXFILES_REPLAY_PROOF",
        "SEXOS_SEXFILES_CAP_RECORD_PROOF",
        "SEXOS_LINEN_SEXFILES_METADATA_PROOF",
        "SEXOS_SEXFILES_FAULT_INJECTION_PROOF",
        "SEXOS_SEXFILES_REBOOT_PROOF",
        "SEXOS_SEXFILES_EXTENT_PROOF",
        "SEXOS_SEXFILES_CHECKPOINT_PROOF",
        "SEXOS_SEXOBJECT_VIEW_PROOF",
    ),
    (
        "SEXOS_SEXFILES_REAL_BLOCK_PROOF",
        "SEXOS_LINEN_DISK_OBJECT_PROOF",
    ),
    (
        "SEXFS_V0_SUPERBLOCK_FORMAT_MOUNT_PROOF",
        "SEXOBJECT_TABLE_PERSIST_PROOF",
        "SEXOBJECT_TABLE_EXTENT_ALLOC_PROOF",
    ),
    (
        "SEXOBJECT_EXTENT_WRITE_FULL_BLOCK_PROOF",
        "SEXOBJECT_WRITE_READ_PERSIST_PROOF",
        "SEXOBJECT_MULTI_OBJECT_PROOF",
    ),
    ("cfg:sexfiles_diskfs_multi_object_proof", "SEXFILES_ROUTE_AUDIT_ONLY"),
    ("cfg:sexfiles_diskfs100_ap2_proof",),
    ("cfg:sexfiles_diskfs100_ap3_proof", "cfg:sexfiles_diskfs_multi_object_proof"),
    ("cfg:sexfiles_diskfs100_ap4_write",),
    ("cfg:sexfiles_diskfs100_ap4_read", "cfg:sexfiles_diskfs100_ap5_neg_read_no_write"),
    ("cfg:sexfiles_diskfs100_ap5_neg_mismatch",),
    ("cfg:sexfiles_diskfs100_ap5_neg_missing_image",),
    ("cfg:sexfiles_diskfs100_ap5_neg_flush_skip",),
    ("cfg:sexfiles_diskfs100_ap6_flush_fsync",),
    ("cfg:sexfiles_diskfs_negative_bounds_auth_proof",),
    ("cfg:sexfiles_diskfs_bridge_strict_proof",),
]

ALL_VARIANTS = DEFAULT_VARIANTS + [(flag,) for flag in NON_ISOLATED_ENV_FLAGS] + [
    ("cfg:sexfiles_diskfs100_ap4_read",),
]

PROVIDER = (
    PROVIDER_PRELUDE.replace("if (slot == 1 &&", "if (slot == 15 &&")
    .replace(
        'uint64_t sexos_allocate_memory(uint64_t size) { (void)size; printf("UNEXPECTED allocate\\n"); exit(3); }\n'
        'uint64_t sexos_map_memory(uint64_t pfn, uint64_t size) { (void)pfn; (void)size; printf("UNEXPECTED map\\n"); exit(3); }',
        r'''uint64_t sexos_allocate_memory(uint64_t size) {
    uint64_t pfn = 0x1234;
    printf("A %" PRIu64 " -> %" PRIu64 "\n", size, pfn);
    return pfn;
}
uint64_t sexos_map_memory(uint64_t pfn, uint64_t size) {
    uint64_t vaddr = 0x400000000000ull;
    printf("M %" PRIu64 " %" PRIu64 " -> %" PRIu64 "\n", pfn, size, vaddr);
    return vaddr;
}''',
    )
    + f"\n#define MEM_LEND_VA UINT64_C({MEM_LEND_VA:#x})\n"
    + r'''
#include <sys/mman.h>

static uint8_t disk[2048 * 512];
static bool mem_mapped = false;
static uint64_t req_step = 0;

static void ensure_mem_mapped(void) {
    if (!mem_mapped) {
        void *p = mmap((void *)(uintptr_t)MEM_LEND_VA, 4096, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | 0x100000, -1, 0);
        if ((uint64_t)(uintptr_t)p != MEM_LEND_VA) {
            printf("MAP FAILED\n");
            exit(4);
        }
        mem_mapped = true;
    }
}

uint64_t sexos_grant_mem_lend(uint64_t domain_slot, uint64_t length, uint64_t lend_slot) {
    flush_spins();
    uint64_t ret = (scenario.faults == 2 && scenario.mix == 2 && chance(50)) ? UINT64_MAX : MEM_LEND_VA;
    printf("G %" PRIu64 " %" PRIu64 " %" PRIu64 " -> %#" PRIx64 "\n", domain_slot, length, lend_slot, ret);
    return ret;
}

uint64_t sexos_map_mem_lend(uint64_t cap_slot) {
    flush_spins();
    uint64_t ret = (cap_slot == 17) ? MEM_LEND_VA : UINT64_MAX;
    printf("P %" PRIu64 " -> %#" PRIx64 "\n", cap_slot, ret);
    return ret;
}

static RegionWriterState writer_state = {true, 4, 11, 2, 11, 1, 0};

MemoryRegionReply sexos_memory_region_info(uint64_t slot) {
    if (slot != 17) return (MemoryRegionReply){.status=UINT64_MAX};
    return (MemoryRegionReply){0,4096,REGION_QUERY|REGION_READ|REGION_WRITE,2,4};
}

MemoryRegionReply sexos_memory_region_view(uint64_t slot,uint64_t offset,uint64_t length,uint64_t rights) {
    flush_spins();
    uint64_t ret = (scenario.faults == 2 && scenario.mix == 2 && chance(50)) ? UINT64_MAX : MEM_LEND_VA;
    printf("G %" PRIu64 " %" PRIu64 " %" PRIu64 " -> %#" PRIx64 "\n", UINT64_C(15), length, slot, ret);
    if (slot != 17 || offset || length != 4096 ||
        rights != (REGION_QUERY|REGION_READ|REGION_WRITE) || ret == UINT64_MAX)
        return (MemoryRegionReply){.status=UINT64_MAX};
    return (MemoryRegionReply){0,ret,length,0,4};
}

RegionWriterReceipt sexos_region_writer_query(uint64_t slot) {
    if (slot != 17) return (RegionWriterReceipt){.status=REGION_WRITER_INVALID};
    return (RegionWriterReceipt){REGION_WRITER_OK,writer_state.revision,writer_state.current,
        writer_state.job,writer_state.peer};
}

RegionWriterReceipt sexos_region_writer_change(uint64_t slot,RegionWriterAction action,
                                                uint64_t expected,uint64_t job) {
    if (slot != 17 || expected != writer_state.revision || !job)
        return (RegionWriterReceipt){.status=REGION_WRITER_STALE,.revision=writer_state.revision,
            .current=writer_state.current,.job=writer_state.job,.peer=writer_state.peer};
    if (action == REGION_WRITER_OFFER && writer_state.current == writer_state.home && !writer_state.job)
        writer_state.job=job;
    else if (action == REGION_WRITER_CANCEL && writer_state.current == writer_state.home && writer_state.job == job)
        writer_state.job=0;
    else return (RegionWriterReceipt){.status=REGION_WRITER_BUSY,.revision=writer_state.revision,
        .current=writer_state.current,.job=writer_state.job,.peer=writer_state.peer};
    ++writer_state.revision;
    return (RegionWriterReceipt){REGION_WRITER_OK,writer_state.revision,writer_state.current,
        writer_state.job,writer_state.peer};
}

static uint64_t fnv1a(const char *s) {
    uint64_t h = 0xcbf29ce484222325ull;
    while (*s) {
        h ^= (uint8_t)*s++;
        h *= 0x100000001b3ull;
    }
    return h;
}

static void put_le16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put_le32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (8 * i)); }
static void put_le64(uint8_t *p, uint64_t v) { for (int i = 0; i < 8; ++i) p[i] = (uint8_t)(v >> (8 * i)); }

static bool is_all_zero(const uint8_t *p, size_t n) {
    for (size_t i = 0; i < n; ++i) if (p[i] != 0) return false;
    return true;
}

static uint64_t storage_reply(void) {
    uint64_t cmd = call.opcode, offset = call.a0, size = call.a1, buf_cap = call.a2;
    if (cmd == 1) { /* BLOCK_READ */
        if (size == 0 || size > 4096 || (offset % 512) != 0 || offset + size > sizeof disk)
            return 2; /* BLOCK_ERR_BAD_LEN */
        if (buf_cap == 17) {
            if (size != 512) return 2;
            if (scenario.faults == 2 && scenario.mix == 2) return 4; /* BLOCK_ERR_NO_DEVICE */
            if (scenario.faults == 1 && chance(4)) return 4;
            memcpy((void *)(uintptr_t)MEM_LEND_VA, disk + offset, 512);
            return 0;
        }
        return 0;
    }
    if (cmd == 2) { /* BLOCK_WRITE */
        if (size != 512 || (offset % 512) != 0 || offset + size > sizeof disk || buf_cap != 17)
            return 4;
        uint64_t lba = offset / 512;
        const uint8_t *buf = (const uint8_t *)(uintptr_t)MEM_LEND_VA;
        if (lba == 0 && is_all_zero(buf, 512))
            return 4; /* Reject unformatted zero-write at LBA 0 for real_block negative proof */
        bool allowed = (lba <= 47) || (lba >= 128 && lba <= 2047);
        if (!allowed) return 4;
        if (scenario.faults == 2 && scenario.mix == 2) return 4;
        if (scenario.faults == 1 && chance(4)) return 4;
        memcpy(disk + offset, buf, 512);
        return 0;
    }
    if (cmd == 3) { /* BLOCK_SYNC */
        return 4; /* BLOCK_ERR_NO_DEVICE on QEMU NVMe */
    }
    return 1; /* BLOCK_ERR_BAD_CMD */
}

static void request(SexosMessage *m) {
    static const uint32_t callers[6] = {0, 3, 6, 11, 12, 13};
    m->caller_pd = callers[next() % 6];
    uint64_t step = req_step++;
    /* Scripted sequence first to exercise deterministic RamFS + DiskFS V4 + SexObject workflows. */
    switch (step) {
    case 0: /* OP_RAMFS_OPEN create "alpha" */
        m->type_id = 0x30; m->caller_pd = 3;
        m->arg0 = word("alpha"); m->arg1 = 0; m->arg2 = 1ull << 24;
        return;
    case 1: /* OP_RAMFS_WRITE handle 1 offset 0 */
        m->type_id = 0x32; m->caller_pd = 3;
        m->arg0 = 1; m->arg1 = 0; m->arg2 = word("hello123");
        return;
    case 2: /* OP_RAMFS_READ handle 1 offset 0 len 8 */
        m->type_id = 0x31; m->caller_pd = 3;
        m->arg0 = 1; m->arg1 = 0; m->arg2 = 8;
        return;
    case 3: /* OP_RAMFS_STAT handle 1 */
        m->type_id = 0x35; m->caller_pd = 3;
        m->arg0 = 1;
        return;
    case 4: /* OP_RAMFS_OBJECT_ID handle 1 */
        m->type_id = 0x37; m->caller_pd = 3;
        m->arg0 = 1;
        return;
    case 5: /* OP_RAMFS_READNAME handle 1 offset 0 len 8 */
        m->type_id = 0x3D; m->caller_pd = 3;
        m->arg0 = 1; m->arg1 = 0; m->arg2 = 8;
        return;
    case 6: /* OP_RAMFS_LIST index 0 */
        m->type_id = 0x34; m->caller_pd = 3;
        m->arg0 = 0;
        return;
    case 7: /* OP_RAMFS_CREATE_OWNER "owned" for pd 6 */
        m->type_id = 0x36; m->caller_pd = 6;
        m->arg0 = word("owned"); m->arg1 = 0; m->arg2 = 6ull << 32;
        return;
    case 8: /* OP_RAMFS_STATUS object 1 */
        m->type_id = 0x3F; m->caller_pd = 3;
        m->arg0 = 1;
        return;
    case 9: /* OP_RAMFS_CLOSE handle 1 */
        m->type_id = 0x33; m->caller_pd = 3;
        m->arg0 = 1;
        return;
    case 10: /* OP_DISKFS_SELECT path_id 0 */
        m->type_id = 0x3E; m->caller_pd = 12;
        m->arg0 = 0;
        return;
    case 11: /* OP_DISKFS_STAT */
        m->type_id = 0x3B; m->caller_pd = 12;
        return;
    case 12: /* OP_DISKFS_MANIFEST_HASH */
        m->type_id = 0x3C; m->caller_pd = 12;
        return;
    case 13: /* OP_DISKFS_WRITE offset 0 */
        m->type_id = 0x38; m->caller_pd = 12;
        m->arg0 = 0; m->arg1 = 0x1122334455667788ull; m->arg2 = 0x99AABBCCDDEEFF00ull;
        return;
    case 14: /* OP_DISKFS_READ offset 0 len 8 */
        m->type_id = 0x39; m->caller_pd = 12;
        m->arg0 = 0; m->arg1 = 8;
        return;
    case 15: /* OP_DISKFS_READ_V2 offset 0 len 6 */
        m->type_id = 0x4A; m->caller_pd = 12;
        m->arg0 = 0; m->arg1 = 6;
        return;
    case 16: /* OP_DISKFS_CREATE "notes" */
        m->type_id = 0x42; m->caller_pd = 12;
        m->arg0 = 0; m->arg1 = word("notes"); m->arg2 = 0;
        return;
    case 17: /* OP_DISKFS_LIST slot 3 word 0xFF (meta) */
        m->type_id = 0x43; m->caller_pd = 12;
        m->arg0 = 3; m->arg1 = 0xFF;
        return;
    case 18: /* OP_DISKFS_LIST slot 3 word 0 (name lo) */
        m->type_id = 0x43; m->caller_pd = 12;
        m->arg0 = 3; m->arg1 = 0;
        return;
    case 19: /* OP_DISKFS_SELECT path_id 3 ("notes") */
        m->type_id = 0x3E; m->caller_pd = 12;
        m->arg0 = 3;
        return;
    case 20: /* OP_DISKFS_WRITE offset 0 on newly created V4 object (allocates pool block!) */
        m->type_id = 0x38; m->caller_pd = 12;
        m->arg0 = 0; m->arg1 = word("v4pool!!"); m->arg2 = word("content!");
        return;
    case 21: /* OP_DISKFS_READ_V2 offset 0 len 6 on V4 object */
        m->type_id = 0x4A; m->caller_pd = 12;
        m->arg0 = 0; m->arg1 = 6;
        return;
    case 22: /* OP_DISKFS_TRUNCATE to 8 bytes */
        m->type_id = 0x49; m->caller_pd = 12;
        m->arg0 = 8;
        return;
    case 23: /* OP_DISKFS_READ_V2 at EOF (offset 8) */
        m->type_id = 0x4A; m->caller_pd = 12;
        m->arg0 = 8; m->arg1 = 6;
        return;
    case 24: /* OP_DISKFS_RENAME slot 3 to "draft" */
        m->type_id = 0x48; m->caller_pd = 12;
        m->arg0 = 3; m->arg1 = word("draft"); m->arg2 = 0;
        return;
    case 25: /* OP_DISKFS_DELETE slot 3 */
        m->type_id = 0x47; m->caller_pd = 12;
        m->arg0 = 3;
        return;
    case 26: /* OP_DISKFS_FLUSH */
        m->type_id = 0x3A; m->caller_pd = 12;
        return;
    case 27: /* OP_SEXOBJECT_NATIVE_PERSIST_PROOF */
        m->type_id = 0x40; m->caller_pd = 13;
        return;
    case 28: /* OP_SEXOBJECT_READ_BACK object_id 2 */
        m->type_id = 0x41; m->caller_pd = 11;
        m->arg0 = 2;
        return;
    default:
        break;
    }

    static const uint64_t ops[] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x42, 0x43, 0x47, 0x48, 0x49, 0x4A, 0x99
    };
    m->type_id = ops[next() % (sizeof ops / sizeof ops[0])];
    switch (m->type_id) {
    case 0x30: /* OP_RAMFS_OPEN */
        m->arg0 = PICK(word("alpha"), word("beta"), word("gamma"), word("delta"), 0);
        m->arg1 = PICK(0, word("_suffix"));
        m->arg2 = PICK(0, 1ull << 24, 3ull << 24);
        break;
    case 0x31: /* OP_RAMFS_READ */
    case 0x3D: /* OP_RAMFS_READNAME */
        m->arg0 = PICK(1, 2, 3, 99);
        m->arg1 = PICK(0, 4, 8, 4096);
        m->arg2 = PICK(0, 4, 8, 16);
        break;
    case 0x32: /* OP_RAMFS_WRITE */
        m->arg0 = PICK(1, 2, 3, 99);
        m->arg1 = PICK(0, 8, 16, 4090, 4096);
        m->arg2 = next();
        break;
    case 0x33: /* OP_RAMFS_CLOSE */
    case 0x35: /* OP_RAMFS_STAT */
    case 0x37: /* OP_RAMFS_OBJECT_ID */
    case 0x3F: /* OP_RAMFS_STATUS */
        m->arg0 = PICK(1, 2, 3, 99);
        break;
    case 0x34: /* OP_RAMFS_LIST */
        m->arg0 = PICK(0, 1, 2, 10);
        break;
    case 0x36: /* OP_RAMFS_CREATE_OWNER */
        m->arg0 = PICK(word("own1"), word("own2"), word("alpha"));
        m->arg1 = 0;
        m->arg2 = PICK(0, 3ull << 32, 6ull << 32);
        break;
    case 0x38: /* OP_DISKFS_WRITE */
        m->arg0 = PICK(0, 16, 32, 4088, 65536);
        m->arg1 = next();
        m->arg2 = next();
        break;
    case 0x39: /* OP_DISKFS_READ */
    case 0x4A: /* OP_DISKFS_READ_V2 */
        m->arg0 = PICK(0, 6, 8, 16, 4096);
        m->arg1 = PICK(0, 4, 6, 8, 12);
        break;
    case 0x3E: /* OP_DISKFS_SELECT */
    case 0x47: /* OP_DISKFS_DELETE */
        m->arg0 = PICK(0, 1, 2, 3, 4, 15, 99);
        break;
    case 0x42: /* OP_DISKFS_CREATE */
    case 0x48: /* OP_DISKFS_RENAME */
        m->arg0 = PICK(0, 2, 3, 4, 15);
        m->arg1 = PICK(word("doc1"), word("doc2"), word("notes"), 0);
        m->arg2 = PICK(0, word("part2"));
        break;
    case 0x43: /* OP_DISKFS_LIST */
        m->arg0 = PICK(0, 1, 2, 3, 15);
        m->arg1 = PICK(0, 1, 2, 0xFF);
        break;
    case 0x49: /* OP_DISKFS_TRUNCATE */
        m->arg0 = PICK(0, 8, 16, 64, 4096, 70000);
        break;
    default:
        m->arg0 = next();
        break;
    }
}

static uint64_t reply_share(void) {
    return (scenario.mix == 2 && scenario.faults == 0) ? 20 : 92;
}

static bool answered(uint64_t opcode) {
    (void)opcode;
    return true;
}

static void server_start(void) {
    ensure_mem_mapped();
    memset(disk, 0, sizeof disk);
    memset((void *)(uintptr_t)MEM_LEND_VA, 0, 4096);
    req_step = 0;

    if (scenario.mix == 1) {
        /* Pre-seed V2 3-entry manifest at LBA 2046 and AP4 payload at LBA 2038. */
        uint8_t *mf = disk + 2046 * 512;
        put_le64(mf + 0, 0x31564D4B53494453ull);
        put_le16(mf + 8, 2);
        put_le16(mf + 10, 3);
        const char *paths[3] = {"/disk/sexfiles-proof-v1", "/disk/linen-object-v1", "/disk/quil-object-v1"};
        const uint64_t lbas[3] = {2038, 2030, 2022};
        for (int i = 0; i < 3; ++i) {
            uint8_t *e = mf + 32 + i * 32;
            put_le64(e + 0, fnv1a(paths[i]));
            put_le64(e + 8, lbas[i]);
            put_le32(e + 16, 4096);
            put_le16(e + 20, 3);
        }
        uint8_t *obj0 = disk + 2038 * 512;
        for (int i = 0; i < 128; ++i) {
            obj0[i] = (uint8_t)((0x9Du ^ (uint8_t)i ^ 0x42u) & 0xFFu);
        }
    } else if (scenario.mix == 2 && scenario.faults == 0) {
        /* Pre-seed V1 single-entry manifest at LBA 2046 to exercise V1->V2->V4 upgrade. */
        uint8_t *mf = disk + 2046 * 512;
        put_le64(mf + 0, 0x31564D4B53494453ull);
        put_le16(mf + 8, 1);
        put_le16(mf + 10, 1);
        uint8_t *e = mf + 32;
        put_le64(e + 0, fnv1a("/disk/sexfiles-proof-v1"));
        put_le64(e + 8, 2038);
        put_le32(e + 16, 4096);
        put_le16(e + 20, 3);
    }
}
'''
)

RUST_SPIN = r'''
#![no_std]
use core::cell::UnsafeCell;
use core::ops::{Deref, DerefMut};

pub struct RwLock<T> {
    data: UnsafeCell<T>,
}
unsafe impl<T> Sync for RwLock<T> {}
unsafe impl<T> Send for RwLock<T> {}

pub struct RwLockReadGuard<'a, T> {
    data: &'a T,
}
impl<'a, T> Deref for RwLockReadGuard<'a, T> {
    type Target = T;
    fn deref(&self) -> &T { self.data }
}

pub struct RwLockWriteGuard<'a, T> {
    data: &'a mut T,
}
impl<'a, T> Deref for RwLockWriteGuard<'a, T> {
    type Target = T;
    fn deref(&self) -> &T { self.data }
}
impl<'a, T> DerefMut for RwLockWriteGuard<'a, T> {
    fn deref_mut(&mut self) -> &mut T { self.data }
}

impl<T> RwLock<T> {
    pub const fn new(val: T) -> Self {
        Self { data: UnsafeCell::new(val) }
    }
    pub fn read(&self) -> RwLockReadGuard<'_, T> {
        RwLockReadGuard { data: unsafe { &*self.data.get() } }
    }
    pub fn write(&self) -> RwLockWriteGuard<'_, T> {
        RwLockWriteGuard { data: unsafe { &mut *self.data.get() } }
    }
}
'''


def build_sex_pdx_source(pdx_lib):
    listen = pinned_function(pdx_lib, "pub fn pdx_listen_raw(slot: u64) -> PdxMessage {", LISTEN)
    listen = listen.replace("crate::listen_once(slot)", "listen_once(slot)")
    v2_ok = pinned_function(pdx_lib, "pub fn diskfs_v2_encode_ok(n: usize, payload: &[u8]) -> u64 {", {})
    v2_eof = pinned_function(pdx_lib, "pub fn diskfs_v2_encode_eof() -> u64 {", {})
    v2_err = pinned_function(pdx_lib, "pub fn diskfs_v2_encode_err(magnitude: u64) -> u64 {", {})
    overlap = pinned_function(
        pdx_lib,
        "pub const fn ranges_overlap(a_start: u64, a_len: u64, b_start: u64, b_len: u64) -> bool {",
        {},
    )
    layout_start = pdx_lib.index("pub const DISK_TOTAL_SECTORS: u64 = 2048;")
    layout_end = pdx_lib.index("/// Server side: encode a successful read")
    layout_consts = pdx_lib[layout_start:layout_end]
    meta_start = pdx_lib.index("pub const SEXFS_V0_META_START_LBA: u64 = 0;")
    meta_end = pdx_lib.index("pub const fn ranges_overlap")
    meta_consts = pdx_lib[meta_start:meta_end]

    return (
        r'''
#[repr(C)] pub struct SexosReply { pub status: u64, pub value: u64 }
#[repr(C)] pub struct SexosMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, pub padding: u32 }
extern "C" {
    pub fn sexos_pdx_call(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> SexosReply;
    pub fn sexos_listen_once(slot: u64) -> SexosMessage;
    pub fn sexos_reply(target_pd: u32, value: u64) -> u64;
    pub fn sexos_yield();
    pub fn sexos_grant_mem_lend(domain_slot: u64, length: u64, lend_slot: u64) -> u64;
    pub fn sexos_map_mem_lend(cap_slot: u64) -> u64;
}
fn listen_once(slot: u64) -> (u64, u64, u64, u64, u64) {
    let m = unsafe { sexos_listen_once(slot) };
    (m.type_id, m.caller_pd as u64, m.arg0, m.arg1, m.arg2)
}
pub fn log_line(text: &str) {
    let line = format!("{text}\n");
    unsafe { sexos_pdx_call(0, 69, line.as_ptr() as u64, line.len() as u64, 0); }
}
#[macro_export]
macro_rules! serial_println {
    ($($argument:tt)*) => { $crate::log_line(&format!($($argument)*)) };
}
pub const SLOT_STORAGE: u64 = 1;
pub const SLOT_BLOCK: u64 = 15;
pub const SLOT_BUF_LEND: u64 = 17;
pub const BLOCK_READ: u64 = 1;
pub const BLOCK_WRITE: u64 = 2;
pub const BLOCK_SYNC: u64 = 3;
pub const BLOCK_OK: u64 = 0;
pub const BLOCK_ERR_BAD_CMD: u64 = 1;
pub const BLOCK_ERR_BAD_LEN: u64 = 2;
pub const BLOCK_ERR_BAD_CAP: u64 = 3;
pub const BLOCK_ERR_NO_DEVICE: u64 = 4;
pub const BLOCK_ERR_TIMEOUT: u64 = 5;
pub const BLOCK_SECTOR_SIZE: u64 = 512;
pub const BLOCK_MAX_XFER: u64 = 4096;
pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, _pad: u32 }
pub fn pdx_call(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> (u64, u64) {
    let reply = unsafe { sexos_pdx_call(slot, opcode, arg0, arg1, arg2) };
    (reply.status, reply.value)
}
pub fn pdx_reply(target_pd: u32, value: u64) -> u64 { unsafe { sexos_reply(target_pd, value) } }
pub fn sys_yield() { unsafe { sexos_yield() } }
pub fn sys_grant_mem_lend(domain_slot: u64, length: u64, lend_slot: u64) -> u64 {
    unsafe { sexos_grant_mem_lend(domain_slot, length, lend_slot) }
}
pub fn sys_map_mem_lend(cap_slot: u64) -> u64 {
    unsafe { sexos_map_mem_lend(cap_slot) }
}
'''
        + layout_consts
        + v2_ok
        + v2_eof
        + v2_err
        + meta_consts
        + overlap
        + listen
    )


def build_sex_rt_source(rt_lib):
    head = rt_lib[rt_lib.index("const HEAP_START_VADDR") : rt_lib.index("#[global_allocator]")]
    expand = pinned_function(
        rt_lib,
        "fn expand_heap(needed_size: usize) -> Result<usize, ()> {",
        {
            "31": "pfn = unsafe { sexos_allocate_memory(size_aligned as u64) };",
            "30": "vaddr = unsafe { sexos_map_memory(pfn, size_aligned as u64) };",
        },
    )
    init = pinned_function(rt_lib, "pub fn heap_init() {", {})
    return (
        "#![no_std]\nuse core::sync::atomic::{AtomicUsize, Ordering};\n"
        'extern "C" { fn sexos_allocate_memory(size: u64) -> u64; fn sexos_map_memory(pfn: u64, size: u64) -> u64; }\n'
        + head
        + expand
        + init
    )


def rust_main_source(source):
    for item in (
        "#![no_std]\n",
        "#![no_main]\n",
        "#[panic_handler]\nfn panic(_info: &core::panic::PanicInfo) -> ! {\n    loop {\n        unsafe {\n            core::arch::asm!(\"syscall\", in(\"rax\") 24, lateout(\"rcx\") _, lateout(\"r11\") _);\n        }\n    }\n}\n",
    ):
        assert source.count(item) == 1, item
        source = source.replace(item, "")
    fallback_asm = 'unsafe {\n            core::arch::asm!("syscall", in("rax") 24, lateout("rcx") _, lateout("r11") _);\n        }'
    assert source.count(fallback_asm) == 1
    source = source.replace(fallback_asm, "sex_pdx::sys_yield();")
    entry = '#[no_mangle]\npub extern "C" fn _start() -> ! {'
    assert source.count(entry) == 1
    source = source.replace(entry, "fn sexfiles_start() -> ! {")
    ffi = r'''
extern crate sex_pdx;
extern crate sex_object_model;
extern "C" {
    fn provider_start(argc: i32, argv: *const *const std::os::raw::c_char);
    fn provider_panic() -> !;
}
'''
    return ffi + source + (RUST_MAIN % "sexfiles_start")


def scenarios(count):
    r = random.Random(0x5E4F11E5)
    table = [
        (120, 88, 0, 0),
        (140, 92, 0, 1),
        (110, 85, 0, 2),
        (100, 85, 1, 0),
        (90, 80, 2, 1),
        (60, 75, 2, 2),
    ]
    return [[r.getrandbits(64), *table[i % len(table)]] for i in range(count)]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=6)
    parser.add_argument("--all-variants", action="store_true")
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    variants = ALL_VARIANTS if args.all_variants else DEFAULT_VARIANTS

    work = HERE / "build/sexfiles-differential"
    if work.exists():
        shutil.rmtree(work)
    (work / "src/backends").mkdir(parents=True)

    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()

    for rel in (
        "vfs.rs",
        "messages.rs",
        "pdx.rs",
        "trampoline.rs",
        "proof.rs",
        "sexobject.rs",
        "backends/mod.rs",
        "backends/ramfs.rs",
        "backends/tmpfs.rs",
        "backends/diskfs.rs",
    ):
        (work / "src" / rel).write_text(show(f"servers/sexfiles/src/{rel}"))

    (work / "src/main.rs").write_text(rust_main_source(show("servers/sexfiles/src/main.rs")))
    (work / "spin.rs").write_text(RUST_SPIN)
    (work / "sex_object_model.rs").write_text(show("crates/sex-object-model/src/lib.rs"))
    (work / "sex_rt.rs").write_text(build_sex_rt_source(show("sex-rt/src/lib.rs")))
    (work / "sex_pdx.rs").write_text(build_sex_pdx_source(show("crates/sex-pdx/src/lib.rs")))

    for crate_name in ("spin", "sex_object_model", "sex_rt", "sex_pdx"):
        run(
            [
                "rustc",
                "--edition",
                "2021",
                "--crate-type",
                "rlib",
                "--crate-name",
                crate_name,
                "-O",
                "-A",
                "warnings",
                work / f"{crate_name}.rs",
                "-o",
                work / f"lib{crate_name}.rlib",
            ]
        )

    include = ["-I", HERE / "generated/include"]
    runtime = [
        HERE / "runtime/client_runtime.c",
        HERE / "runtime/server_runtime.c",
        *sorted((HERE / "generated/runtime").glob("*.c")),
    ]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]

    provider_c = work / "provider.c"
    provider_c.write_text(PROVIDER)
    provider_o = work / "provider.o"
    run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include, "-c", provider_c, "-o", provider_o])
    (work / "harness.c").write_text(PROVIDER + (C_MAIN % "sexfiles_main"))

    cesta_units = ("sexfiles", "sexfiles_diskfs", "sexfiles_proof")

    assert "#if" not in (HERE / "generated/include/sexfiles.h").read_text().replace("#if !defined(SEXOS_SEXFILES_H)", "").replace("#ifndef SEXOS_SEXFILES_H", "").replace("#endif", "")
    shared_objs = {}
    for compiler in ("gcc", "clang"):
        shared_objs[compiler] = []
        for unit in ("sexfiles_diskfs", "sexfiles_proof"):
            conditions = set(re.findall(r'\(cpp-if "([^"]+)"',
                                        (HERE / "src" / f"{unit}.cesta").read_text()))
            assert conditions <= {"SEXOS_SEXFILES_CURRENT_DOCUMENT"}, conditions
            obj = work / f"{unit}-{compiler}-shared.o"
            run(
                [
                    cesta,
                    "cc",
                    "--cc",
                    compiler,
                    "-std=c11",
                    "-O2",
                    *warnings,
                    *sanitizers,
                    *include,
                    "-DSEXOS_SEXFILES_CURRENT_DOCUMENT=0",
                    "-c",
                    HERE / "src" / f"{unit}.cesta",
                    "-o",
                    obj,
                ]
            )
            shared_objs[compiler].append(obj)

    def compile_cesta_objects(compiler, defines, tag):
        obj = work / f"sexfiles-{compiler}-{tag}.o"
        run(
            [
                cesta,
                "cc",
                "--cc",
                compiler,
                "-std=c11",
                "-O2",
                *warnings,
                "-DSEXOS_SEXFILES_CURRENT_DOCUMENT=0",
                *defines,
                *sanitizers,
                *include,
                "-c",
                HERE / "src/sexfiles.cesta",
                "-o",
                obj,
            ]
        )
        return [obj, *shared_objs[compiler]]

    cases = scenarios(args.cases)
    compared = 0
    references = {}

    for v_idx, variant in enumerate(variants):
        tag = f"v{v_idx}"
        env = {name: value for name, value in os.environ.items() if name not in ENV_FLAGS}
        rust_cfgs = []
        c_defines = []
        for item in variant:
            if item.startswith("cfg:"):
                cfg_name = item[4:]
                rust_cfgs.extend(["--cfg", cfg_name])
                c_defines.append(f"-D{CFG_TO_C_DEFINE[cfg_name]}")
            else:
                env[item] = "1"
                c_defines.append(f"-D{item}")

        reference = work / f"reference-{tag}"
        run(
            [
                "rustc",
                "--edition",
                "2021",
                "-O",
                "-A",
                "warnings",
                *rust_cfgs,
                "-L",
                work,
                "--extern",
                f"spin={work / 'libspin.rlib'}",
                "--extern",
                f"sex_object_model={work / 'libsex_object_model.rlib'}",
                "--extern",
                f"sex_rt={work / 'libsex_rt.rlib'}",
                "--extern",
                f"sex_pdx={work / 'libsex_pdx.rlib'}",
                work / "src/main.rs",
                "-Clink-arg=" + str(provider_o),
                "-o",
                reference,
            ],
            env=env,
        )

        harnesses = []
        for compiler in ("gcc", "clang"):
            objs = compile_cesta_objects(compiler, c_defines, tag)
            harness = work / f"harness-{compiler}-{tag}"
            run(
                [
                    compiler,
                    "-std=c11",
                    "-O2",
                    "-DSEXOS_HOST",
                    *sanitizers,
                    *include,
                    work / "harness.c",
                    *objs,
                    *runtime,
                    "-o",
                    harness,
                ]
            )
            harnesses.append((compiler, harness))

        for number, case in enumerate(cases):
            expected = run([reference, *case]).stdout
            references[v_idx, number] = expected
            for compiler, harness in harnesses:
                actual = run([harness, *case]).stdout
                if actual != expected:
                    raise SystemExit(
                        first_difference(
                            actual.decode(),
                            expected.decode(),
                            f"{tag} ({variant}) scenario {number} {compiler}",
                        )
                    )
            compared += len(expected.splitlines())

    # Controls: each mutation across sexfiles, sexfiles_diskfs, and sexfiles_proof must be killed.
    mutations = [
        # sexfiles.cesta: defer ring, RamFS, VFS / DiskFS V4
        ("sexfiles", 0, "(>= (. sexfiles_state defer_len) DEFER_CAP)", "(>= (. sexfiles_state defer_len) (- DEFER_CAP 1))"),
        ("sexfiles", 1, "(== (& (. cap rights) required) required)", "(!= (& (. cap rights) required) required)"),
        ("sexfiles", 0, "(& flags RAMFS_O_CREATE)", "(& flags RAMFS_O_EXCL)"),
        ("sexfiles", 1, "(?: (>= start (-> entry data_len)) CAP_RIGHT_APPEND CAP_RIGHT_WRITE)", "(?: (>= start (-> entry data_len)) CAP_RIGHT_DELETE CAP_RIGHT_DELETE)"),
        ("sexfiles", 0, "(== type_id OP_DISKFS_READ)", "(== type_id OP_DISKFS_WRITE)"),
        ("sexfiles", 0, "(+ DISKFS_V4_POOL_BASE_LBA (* run_start DISKFS_V4_BLOCK_SECTORS))", "(+ (+ DISKFS_V4_POOL_BASE_LBA 8) (* run_start DISKFS_V4_BLOCK_SECTORS))"),
        ("sexfiles", 1, "(if (< (-> entry cap_generation) UINT64_MAX)\n        (+= (-> entry cap_generation) 1))", "(if (< (-> entry cap_generation) UINT64_MAX)\n        (= (-> entry cap_generation) 1))"),
        ("sexfiles", 0, "(if (!= (& flags RAMFS_O_EXCL) 0)", "(if (== (& flags RAMFS_O_EXCL) 0)"),
        # sexfiles_diskfs.cesta: checksums, journal, checkpoints, extent allocator, SexFS v0
        ("sexfiles_diskfs", 1, "(== (. rec kind) JOURNAL_KIND_TX_COMMIT)", "(== (. rec kind) JOURNAL_KIND_TX_BEGIN)"),
        ("sexfiles_diskfs", 1, "(== (. cp magic) DISKFS_CHECKPOINT_MAGIC)", "(!= (. cp magic) DISKFS_CHECKPOINT_MAGIC)"),
        ("sexfiles_diskfs", 3, "(call put_u64_le sb 0 SEXFS_V0_SUPERBLOCK_MAGIC)", "(call put_u64_le sb 0 0)"),
        ("sexfiles_diskfs", 3, "(define SEXFS_V0_DATA_REGION_MIN_BLOCK \"((size_t)16)\")", "(define SEXFS_V0_DATA_REGION_MIN_BLOCK \"((size_t)17)\")"),
        ("sexfiles_diskfs", 3, "(call put_u64_le fm 0 SEXFS_V0_FREEMAP_MAGIC)", "(call put_u64_le fm 0 0)"),
        # sexfiles_proof.cesta: proof harnesses
        ("sexfiles_proof", 10, "(= ([] wrong_expected i) (cast uint8_t (& (^ (^ 0xC7u (cast uint8_t i)) 0x55u) 0xFFu)))", "(= ([] wrong_expected i) (cast uint8_t (& (^ (^ 0xC7u (cast uint8_t i)) 0x54u) 0xFFu)))"),
        ("sexfiles_proof", 9, "(= ([] expected i) (cast uint8_t (& (^ (^ 0x9Du (cast uint8_t i)) 0x42u) 0xFFu)))", "(= ([] expected i) (cast uint8_t (& (^ (^ 0x9Du (cast uint8_t i)) 0x43u) 0xFFu)))"),
    ]

    base_objs = {}
    for v_idx in {0, 1, 3, 9, 10}:
        c_defines = []
        for item in variants[v_idx]:
            if item.startswith("cfg:"):
                c_defines.append(f"-D{CFG_TO_C_DEFINE[item[4:]]}")
            else:
                c_defines.append(f"-D{item}")
        base_objs[v_idx] = (
            c_defines,
            [work / f"sexfiles-gcc-v{v_idx}.o", *shared_objs["gcc"]],
        )

    for number, (unit, v_idx, old_text, new_text) in enumerate(mutations):
        original = LayoutText((HERE / "src" / f"{unit}.cesta").read_text())
        assert original.count(old_text) == 1, (unit, old_text)
        mutant = work / f"mutant-{number}.cesta"
        mutant.write_text(original.replace(old_text, new_text))
        c_defines, objs = base_objs[v_idx]
        mutant_obj = work / f"mutant-{number}.o"
        run(
            [
                cesta,
                "cc",
                "--cc",
                "gcc",
                "-std=c11",
                "-O2",
                "-DSEXOS_SEXFILES_CURRENT_DOCUMENT=0",
                *c_defines,
                *sanitizers,
                *include,
                "-c",
                mutant,
                "-o",
                mutant_obj,
            ]
        )
        link_objs = [mutant_obj if u == unit else objs[i] for i, u in enumerate(cesta_units)]
        harness = work / f"mutant-{number}"
        run(
            [
                "gcc",
                "-std=c11",
                "-O2",
                "-DSEXOS_HOST",
                *sanitizers,
                *include,
                work / "harness.c",
                *link_objs,
                *runtime,
                "-o",
                harness,
            ]
        )
        if not any(run([harness, *case]).stdout != references[v_idx, idx] for idx, case in enumerate(cases)):
            raise SystemExit(f"mutation not detected ({unit}): {old_text} -> {new_text}")

    print(
        f"sexfiles: Cesta translation equals the Rust server on {compared} trace lines "
        f"({args.cases} scenarios x {len(variants)} build variants), GCC and Clang, "
        f"address and UB sanitizers; {len(mutations)} mutations detected"
    )


if __name__ == "__main__":
    main()
