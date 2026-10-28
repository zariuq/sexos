#!/usr/bin/env python3
"""Compare servers/linen with its Cesta translation.

The Rust server (main.rs, session.rs and sexobject.rs at the pinned revision)
is compiled with only its no_std scaffolding removed: the failing allocator,
the panic handler and the _start name. crates/sex-object-model is compiled
from the pinned revision; crates/sex-pdx is replaced by a module that keeps
the pinned pdx_listen_raw and pdx_try_listen_raw with their listen syscall
routed to a provider. One scripted provider, written in C, serves both
servers: the Rust links it, so both see the same storage replies, requests
and faults and both log through the same raw-print call. Spin-loop hints are
counted, so the startup delay must be exactly five million of them.

The provider plays SexFiles: a DiskFS V3 manifest of 15 slots whose
generation, use bits and names change between scans; RamFS files whose names
and contents read back as written; the DiskFS fixed object, which starts with
the AP3 pattern; and at two fault levels, errors, short writes, small block
statuses, corrupted words, valid words with the top bit set and refused
enqueues. Client requests (creates with good and bad kinds and lengths, lists,
gets, snapshots, name reads, open intents, searches with valid and invalid
UTF-8 tokens, rescans), HID events, stray replies and unknown types arrive at
any time, also while a storage reply is awaited, so the defer stash fills,
drops and replays. The variants cover every build flag and the exclusions
between the DiskFS lanes. A second differential compares the SexObject model
header and the Linen adapters with the Rust crate and sexobject.rs. The AP5
read-no-write failure needs AP3_WRITE, which excludes the AP5 lane, so no
build can print it.
"""

import argparse
import os
from pathlib import Path
import random
import re
import shutil

from differential_support import (C_MAIN, LISTEN, PROVIDER_PRELUDE, RUST_FFI, RUST_MAIN, first_difference,
                                  pinned_function, run)

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
AP = "SEXOS_LINEN_DISKFS_PERSISTENCE_100_"
ENV_FLAGS = ["SEXOS_LINEN_SESSION_PROOF", "SEXOS_LINEN_SEXFILES_METADATA_PROOF", "SEXOS_LINEN_SEXFILES100_PROOF",
             "SEXOS_SEXOBJECT_OQ5_PROOF", "SEXOS_LINEN_DISK_OBJECT_PROOF", "SEXOS_LINEN_DISKFS_DIRECT_PROOF",
             "LINEN_SEXOBJECT_NATIVE_PERSIST_PROOF", AP + "AP2", AP + "AP3_WRITE", AP + "AP3_READ",
             AP + "AP4_META_WRITE", AP + "AP4_META_READ", AP + "AP4_META_AUDIT", AP + "AP5_NEGATIVE",
             AP + "AP5_NEG_MISMATCH", AP + "AP5_NEG_MISSING", AP + "AP5_NEG_READ_NO_WRITE",
             AP + "AP5_NEG_METADATA_FALSE_CLAIM", AP + "AP5_NEG_FLUSH_SKIP", "SEXOS_LINEN_KEYBOARD_NAV_PROOF",
             "SEXOS_LINEN_OBJECT_WORKFLOW_PROOF", "SEXOS_LINEN_OBJECT_PERSIST_PROOF", "SEXOS_LINEN_OBJECT_SCHEMA_PROOF",
             "SEXOS_LINEN_PERSIST_READBACK_PROOF"]
CFG_FLAGS = ["linen_diskfs_slot_proof", "linen_reboot_restore_current_tier_proof", "linen_object_ux_current_tier_proof"]
AP5_ALL = [AP + "AP5_NEGATIVE", AP + "AP5_NEG_MISMATCH", AP + "AP5_NEG_MISSING", AP + "AP5_NEG_READ_NO_WRITE",
           AP + "AP5_NEG_METADATA_FALSE_CLAIM", AP + "AP5_NEG_FLUSH_SKIP"]
UX = ["linen_reboot_restore_current_tier_proof", "linen_object_ux_current_tier_proof"]
LOCAL = ["SEXOS_LINEN_OBJECT_WORKFLOW_PROOF", "SEXOS_LINEN_OBJECT_PERSIST_PROOF", "SEXOS_LINEN_KEYBOARD_NAV_PROOF",
         "SEXOS_LINEN_OBJECT_SCHEMA_PROOF", "SEXOS_LINEN_PERSIST_READBACK_PROOF"]
VARIANTS = [
    (),
    tuple(LOCAL),
    ("SEXOS_LINEN_OBJECT_PERSIST_PROOF",),
    ("SEXOS_LINEN_KEYBOARD_NAV_PROOF",),
    ("SEXOS_LINEN_SEXFILES100_PROOF",),
    ("SEXOS_LINEN_SEXFILES100_PROOF", "SEXOS_LINEN_KEYBOARD_NAV_PROOF"),
    ("SEXOS_LINEN_SESSION_PROOF", "SEXOS_LINEN_SEXFILES_METADATA_PROOF", "SEXOS_SEXOBJECT_OQ5_PROOF"),
    ("SEXOS_LINEN_SEXFILES_METADATA_PROOF",),
    ("SEXOS_LINEN_DISK_OBJECT_PROOF",),
    ("LINEN_SEXOBJECT_NATIVE_PERSIST_PROOF",),
    ("linen_diskfs_slot_proof", *UX),
    ("SEXOS_LINEN_DISKFS_DIRECT_PROOF", "linen_diskfs_slot_proof", "LINEN_SEXOBJECT_NATIVE_PERSIST_PROOF",
     "SEXOS_LINEN_SESSION_PROOF", *UX),
    (AP + "AP2",),
    (AP + "AP2", AP + "AP3_WRITE", AP + "AP3_READ", AP + "AP4_META_AUDIT", *AP5_ALL, "linen_diskfs_slot_proof",
     "SEXOS_LINEN_SEXFILES100_PROOF", *UX),
    (AP + "AP3_WRITE",),
    (AP + "AP3_READ",),
    (AP + "AP3_WRITE", AP + "AP3_READ", *AP5_ALL, *UX),
    (AP + "AP4_META_WRITE", AP + "AP5_NEGATIVE", AP + "AP5_NEG_MISMATCH", "SEXOS_LINEN_SESSION_PROOF", *UX),
    (AP + "AP4_META_READ",),
    (AP + "AP4_META_AUDIT", "SEXOS_LINEN_SEXFILES100_PROOF"),
    tuple(AP5_ALL),
    (AP + "AP5_NEGATIVE",),
    tuple(ENV_FLAGS + CFG_FLAGS),
    tuple(name for name in ENV_FLAGS + CFG_FLAGS if not name.startswith(AP)),
]

# SexFiles for Linen: the server part of the shared provider (see
# differential_support). Mix 2 is quiet while a storage reply is awaited.
PROVIDER = PROVIDER_PRELUDE + r'''
static uint64_t handles, generation;
/* SexFiles still running its startup proofs (heavy faults, balanced mix): it never answers the native persist call. */
static bool busy;
static uint8_t open_name[24], ram_file[256], disk[4096];
static struct { bool used; unsigned name; } manifest[15];

static const char *const NAMES[] = {"notes", "linen-object-v1", "sexfiles-proof-v1", "a", "abcdefghijklmnopqrstuvwx",
                                    "", "work-plan", "beta-report.txt", "caf\xc3\xa9"};
#define NAME_COUNT (sizeof NAMES / sizeof NAMES[0])
static const char *const TOKENS[] = {"work", "beta", "doc", "", "notes", "abcdefgh", "\xc3\xa9t\xc3\xa9", "\xc3(",
    "\xed\xa0\x80", "\xf0\x9f\x98\x80", "\xf4\x90\x80\x80", "\xe0\x80\xaf", "\xc0\xaf", "a\nb", "\xef\xbf\xbf",
    "\xf0\x90\x80\x80", "\xf4\x8f\xbf\xbf", "\xe0\xa0\x80", "\xc2\x80", "\xe2\x82", "\x80", "-alpha", "gamm", "ijklmnop"};
#define TOKEN_COUNT (sizeof TOKENS / sizeof TOKENS[0])

static void mutate_manifest(void) {
    uint64_t changes = 1 + next() % 3;
    for (uint64_t n = 0; n < changes; ++n) {
        unsigned slot = (unsigned)(next() % 15);
        if (chance(50)) manifest[slot].used = !manifest[slot].used;
        else manifest[slot].name = (unsigned)(next() % NAME_COUNT);
    }
    generation += 1;
}

/* SexFiles' answer to the storage call awaiting one. */
static uint64_t storage_reply(void) {
    uint64_t a0 = call.a0, a1 = call.a1, a2 = call.a2;
    switch (call.opcode) {
    case 0x43: /* DISKFS_LIST: the generation (0xFE), a slot's info word (0xFF) or 8 bytes of its name */
        if (a1 == 0xFE) {
            if (fault()) return error();
            if (chance(35)) mutate_manifest();
            return generation;
        }
        if (a0 >= 15) return error();
        if (a1 == 0xFF) {
            if (fault()) return chance(50) ? error() : 3ull << 62;
            uint64_t low = next() & 0xFFF;
            return (uint64_t)manifest[a0].used << 62 | low;
        }
        if (a1 < 3) {
            const char *name = NAMES[manifest[a0].name];
            uint64_t w = word_at((const uint8_t *)name, strlen(name), (size_t)a1 * 8);
            if (fault()) return chance(50) ? error() : w ^ 0x01;
            return w;
        }
        return error();
    case 0x36: /* RAMFS_CREATE_OWNER: a new handle */
        if (fault()) return error();
        return ++handles;
    case 0x37: /* RAMFS_OBJECT_ID: the global id */
        if (fault()) return error();
        return 100 + a0;
    case 0x30: /* RAMFS_OPEN: name bytes 0-15 in a0 and a1, 16-23 in a2 */
        for (int i = 0; i < 8; ++i) {
            open_name[i] = (uint8_t)(a0 >> 8 * i);
            open_name[8 + i] = (uint8_t)(a1 >> 8 * i);
            open_name[16 + i] = (uint8_t)(a2 >> 8 * i);
        }
        if (fault()) return error();
        return ++handles;
    case 0x3D: /* RAMFS_READNAME: handle, offset, length */
        if (fault()) {
            uint64_t k = next() % 3;
            return k == 0 ? error() : k == 1 ? 0 : word_at(open_name, sizeof open_name, a1) ^ 0x0100;
        }
        if (a1 >= sizeof open_name) return 0;
        return word_at(open_name, a2 < sizeof open_name - a1 ? a1 + a2 : sizeof open_name, a1);
    case 0x32: /* RAMFS_WRITE: handle, offset, data */
        if (fault()) return chance(50) ? error() : 4;
        if (a1 <= sizeof ram_file - 8)
            for (int i = 0; i < 8; ++i) ram_file[a1 + i] = (uint8_t)(a2 >> 8 * i);
        return 8;
    case 0x31: /* RAMFS_READ: handle, offset, length */
        if (a1 >= 128 && chance(75)) return error();
        if (fault()) return chance(50) ? error() : word_at(ram_file, sizeof ram_file, a1) ^ 0x10000;
        return a1 < sizeof ram_file ? word_at(ram_file, sizeof ram_file, a1) : 0;
    case 0x33: /* RAMFS_CLOSE */
    case 0x3E: /* DISKFS_SELECT */
        return fault() ? error() : 0;
    case 0x3B: /* DISKFS_STAT */
        return fault() ? error() : 4096;
    case 0x38: /* DISKFS_WRITE: offset, low word, high word */
        if (fault()) return PICK(UINT64_MAX - 3, 4, 8);
        if (a0 <= sizeof disk - 16)
            for (int i = 0; i < 8; ++i) {
                disk[a0 + i] = (uint8_t)(a1 >> 8 * i);
                disk[a0 + 8 + i] = (uint8_t)(a2 >> 8 * i);
            }
        return 16;
    case 0x3A: /* DISKFS_FLUSH: ERR_NO_DEVICE under QEMU */
        return PICK(UINT64_MAX - 18, UINT64_MAX - 18, 0);
    case 0x39: { /* DISKFS_READ: offset, length; the reply is the data */
        uint64_t w = a0 < sizeof disk ? word_at(disk, sizeof disk, a0) : 0;
        if (fault()) {
            uint64_t k = next() % 4;
            if (k == 0) return UINT64_MAX - 3;
            if (k == 1) return PICK(4, 255, 256);
            return k == 2 ? w ^ 0x100 : w ^ 1ull << 63;
        }
        return w;
    }
    case 0x40: /* SexObject native persist proof: the object id */
        if (fault()) return 0;
        return PICK(1, 1, 2, 0);
    default:
        return next();
    }
}

static void request(SexosMessage *m) {
    uint64_t kind = next() % 32;
    if (scenario.mix == 1 && kind >= 12 && chance(50)) kind = next() % 6;
    uint64_t caller = PICK(3, 6, 7, 9, 12, 0);
    m->caller_pd = (uint32_t)caller;
    if (kind < 6) {
        uint64_t object_kind = PICK(0, 0, 1, 2, 3, 0xFF);
        uint64_t length = PICK(0, 1, 4, 8, 9, 14, 16, 16, 17, 24, 25);
        m->type_id = 0x41;
        m->arg0 = object_kind | length << 8;
        m->arg1 = PICK(word("work-doc"), word("beta-tag"), word("notes"), word("zz"), word("caf\xc3\xa9!!!"), UINT64_MAX);
        m->arg2 = PICK(0, word("-alpha"), word("gamma123"), word("\xff\xfe"));
        caller = PICK(3, 6, 7, 9, 12);
        m->caller_pd = (uint32_t)caller;
    } else if (kind < 9) {
        uint64_t start = next() % 20;
        uint64_t high = PICK(0, 0, 0x100);
        m->type_id = 0x42;
        m->arg0 = start | high;
    } else if (kind < 12) {
        m->type_id = 0x43;
        m->arg0 = chance(85) ? next() % 24 : PICK(0xFFFF, UINT64_MAX);
    } else if (kind < 15) {
        uint64_t slot = next() % 18;
        uint64_t high = PICK(0, 0, 0x100);
        m->type_id = 0x44;
        m->arg0 = slot | high;
    } else if (kind < 18) {
        m->type_id = 0x45;
        m->arg0 = next() % 22;
        m->arg1 = PICK(0, 0, 3, 8, 15, 16, 23, 24, 40, 1ull << 40);
        m->arg2 = PICK(0, 1, 3, 8, 8, 9, 64);
    } else if (kind < 20) {
        m->type_id = 0x46;
        m->arg0 = next() % 24;
    } else if (kind < 23) {
        m->type_id = 0x47;
        m->arg0 = word(TOKENS[next() % TOKEN_COUNT]);
        m->arg1 = word(TOKENS[next() % TOKEN_COUNT]);
    } else if (kind < 25) {
        m->type_id = 0x49;
    } else if (kind < 30) {
        m->type_id = 0x202;
        m->arg0 = PICK(0x24, 0x50, 0x25, 0x48, 0x1C, 0x39, 0x0E, 0x53, 0x10, 0x124);
        m->arg1 = PICK(1, 1, 0, 2);
    } else {
        /* Stray replies come only with faults, and never from a busy SexFiles. */
        m->type_id = scenario.faults && !busy ? PICK(1, 0x48, 0x02, 0x100, 0x4A) : PICK(0x48, 0x02, 0x100, 0x4A);
        m->arg0 = next();
    }
}

static uint64_t reply_share(void) { return scenario.mix == 2 ? 90 : 50; }
static bool answered(uint64_t opcode) { return !(busy && opcode == 0x40); }
static void server_start(void) {
    generation = PICK(0, 1, 7);
    for (int i = 0; i < 15; ++i) {
        manifest[i].used = chance(40);
        manifest[i].name = (unsigned)(next() % NAME_COUNT);
    }
    for (int i = 0; i < 128; ++i) disk[i] = (uint8_t)(0xB6 ^ i ^ 0x2D);
    busy = scenario.faults == 2 && scenario.mix == 0;
}
'''

# The SexObject model and the Linen adapters over the same inputs.
MODEL_RUST = r'''
mod session;
mod sexobject;
const LINEN_MAX_NAME: usize = 24;
const LINEN_MAX_OBJECTS: usize = 16;
use sex_object_model::{SexObjectHeader, SexObjectKind, SexObjectRef};
fn kind_text(kind: Option<SexObjectKind>) -> String { kind.map_or("none".to_string(), |k| format!("{}", k as u32)) }
fn main() {
    for v in [0u16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 255, 0xFFFF] {
        let kind = SexObjectKind::from_u16(v);
        println!("kind {} {} {}", v, kind_text(kind), kind.map_or(0, |k| k.as_u16()));
    }
    for id in [0u64, 1, 0x1234, u64::MAX] {
        for generation in [0u64, 5] {
            let r = SexObjectRef::new(id, generation);
            println!("ref {} {} {} {}", r.object_id, r.generation, r.is_null() as u8, SexObjectRef::NULL.is_null() as u8);
        }
    }
    let ids = [0u64, 1, 0x1234, u64::MAX];
    let mut i = 0u64;
    for kind in [0u32, 4, 5, 7, 11, 12, 0xFFFF, 0x10000, 0x10004, 0xFFFF_FFFF] {
        for flags in [0u32, 1, 2, 3, 4, 7, 8, 0xF, 0xFFFF_FFFF] {
            let h = SexObjectHeader { object_id: ids[(i % 4) as usize], content_generation: 3 * i + 1, rights_generation: 5 * i,
                metadata_generation: 7 * i + 2, object_size_bytes: i, first_block: 2 * i, owner_pd: i as u32, kind, checksum: 9,
                flags, reserved0: 0, reserved1: 0 };
            let (c, o) = (h.cap_ref(), h.object_ref());
            println!("header {} {} {} {} {} {} {} {} {}", kind, flags, c.object_id, c.generation, o.object_id, o.generation,
                h.is_tombstoned() as u8, h.is_sealed() as u8, h.is_redacted() as u8);
            println!("header.kind {}", kind_text(h.kind()));
            i += 1;
        }
    }
    println!("layout {} {} {} {}", std::mem::size_of::<SexObjectHeader>(), std::mem::offset_of!(SexObjectHeader, owner_pd),
        std::mem::offset_of!(SexObjectHeader, reserved0), std::mem::size_of::<SexObjectRef>());
    for (k, kind) in [session::ObjectKind::Document, session::ObjectKind::Session, session::ObjectKind::Unknown].into_iter().enumerate() {
        println!("map {} {}", k, sexobject::linen_kind_to_sex(kind) as u32);
        for sfid in [0u64, 1, 42, u64::MAX] {
            for generation in [1u64, 2, u64::MAX] {
                for owner in [0u32, 7, 42, u32::MAX] {
                    let obj = session::LinenObject { object_id: 9, kind, owner_pd: owner, name: [0x61; LINEN_MAX_NAME], name_len: 3,
                        ramfs_handle: 77, sexfiles_object_id: sfid, generation, flags: 1 };
                    let h = sexobject::sexobject_header_from_linen(&obj);
                    let r = sexobject::linen_object_ref(&obj);
                    println!("adapter {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}", k, h.object_id, h.content_generation,
                        h.rights_generation, h.metadata_generation, h.object_size_bytes, h.first_block, h.owner_pd, h.kind,
                        h.checksum, h.flags, h.reserved0, h.reserved1, r.object_id, r.generation);
                }
            }
        }
    }
}
'''

MODEL_C = r'''
#include "linen.c"
#include <inttypes.h>
#include <stdio.h>
static const char *kind_text(bool some, SexObjectKind kind) {
    static char text[16];
    if (!some) return "none";
    snprintf(text, sizeof text, "%u", (unsigned)kind);
    return text;
}
int main(void) {
    const uint16_t values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 255, 0xFFFF};
    for (size_t n = 0; n < sizeof values / sizeof values[0]; ++n) {
        SexObjectKind kind = SEX_OBJECT_KIND_RAW_BLOB;
        bool some = sex_object_kind_from_u16(values[n], &kind);
        printf("kind %u %s %u\n", (unsigned)values[n], kind_text(some, kind), some ? (unsigned)sex_object_kind_as_u16(kind) : 0u);
    }
    const uint64_t ids[] = {0, 1, 0x1234, UINT64_MAX};
    for (size_t n = 0; n < 4; ++n) {
        const uint64_t generations[] = {0, 5};
        for (size_t g = 0; g < 2; ++g) {
            SexObjectRef r = sex_object_ref_new(ids[n], generations[g]);
            printf("ref %" PRIu64 " %" PRIu64 " %u %u\n", r.object_id, r.generation, (unsigned)sex_object_ref_is_null(r),
                   (unsigned)sex_object_ref_is_null(sex_object_ref_new(0, 0)));
        }
    }
    const uint32_t kinds[] = {0, 4, 5, 7, 11, 12, 0xFFFF, 0x10000, 0x10004, 0xFFFFFFFF};
    const uint32_t flag_words[] = {0, 1, 2, 3, 4, 7, 8, 0xF, 0xFFFFFFFF};
    uint64_t i = 0;
    for (size_t k = 0; k < 10; ++k)
        for (size_t f = 0; f < 9; ++f) {
            SexObjectHeader h = {ids[i % 4], 3 * i + 1, 5 * i, 7 * i + 2, i, 2 * i, (uint32_t)i, kinds[k], 9, flag_words[f], 0, 0};
            SexObjectRef c = sex_object_header_cap_ref(&h), o = sex_object_header_object_ref(&h);
            printf("header %u %u %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %u %u %u\n", kinds[k], flag_words[f], c.object_id,
                   c.generation, o.object_id, o.generation, (unsigned)sex_object_header_is_tombstoned(&h),
                   (unsigned)sex_object_header_is_sealed(&h), (unsigned)sex_object_header_is_redacted(&h));
            SexObjectKind kind = SEX_OBJECT_KIND_RAW_BLOB;
            bool some = sex_object_header_kind(&h, &kind);
            printf("header.kind %s\n", kind_text(some, kind));
            ++i;
        }
    printf("layout %zu %zu %zu %zu\n", sizeof(SexObjectHeader), offsetof(SexObjectHeader, owner_pd),
           offsetof(SexObjectHeader, reserved0), sizeof(SexObjectRef));
    const uint64_t sfids[] = {0, 1, 42, UINT64_MAX}, gens[] = {1, 2, UINT64_MAX};
    const uint32_t owners[] = {0, 7, 42, UINT32_MAX};
    for (unsigned k = 0; k < 3; ++k) {
        printf("map %u %u\n", k, (unsigned)linen_kind_to_sex((uint8_t)k));
        for (size_t s = 0; s < 4; ++s)
            for (size_t g = 0; g < 3; ++g)
                for (size_t w = 0; w < 4; ++w) {
                    LinenObject obj = {.object_id = 9, .kind = (uint8_t)k, .owner_pd = owners[w], .name_len = 3,
                                       .ramfs_handle = 77, .sexfiles_object_id = sfids[s], .generation = gens[g], .flags = 1};
                    for (size_t b = 0; b < sizeof obj.name; ++b) obj.name[b] = 0x61;
                    SexObjectHeader h = sexobject_header_from_linen(&obj);
                    SexObjectRef r = linen_object_ref(&obj);
                    printf("adapter %u %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %u %u %u %u %" PRIu64
                           " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", k, h.object_id, h.content_generation, h.rights_generation,
                           h.metadata_generation, h.object_size_bytes, h.first_block, h.owner_pd, h.kind, h.checksum, h.flags,
                           h.reserved0, h.reserved1, r.object_id, r.generation);
                }
    }
    return 0;
}
'''


def rust_reference(source):
    for old, new in (("#![no_std]\n", ""), ("#![no_main]\n", ""), ("#![allow(static_mut_refs)]\n", ""),
                     ("#[global_allocator]\nstatic ALLOCATOR: DummyAllocator = DummyAllocator;\n", ""),
                     ("#[panic_handler]\nfn panic(_: &core::panic::PanicInfo) -> ! { loop {} }\n", ""),
                     ('#[no_mangle]\npub extern "C" fn _start() -> ! {', "fn linen_start() -> ! {"),
                     ("core::hint::spin_loop();", "unsafe { crate::sexos_spin_loop_hint() };")):
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    return source


# A scenario is seed, limit (yields), listen (percent), faults (0-2) and mix (0-2).
def scenarios(count):
    r = random.Random(0x11E7)
    return [[r.getrandbits(64), [9000, 2500, 300][(k // 9) % 3], r.choice([25, 70, 95]), k % 3, (k // 3) % 3]
            for k in range(count)]


def tag_of(variant):
    if not variant:
        return "default"
    return f"v{VARIANTS.index(variant)}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=18)
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    work = HERE / "build/linen-differential"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    main_rs = show("servers/linen/src/main.rs")
    translation = (HERE / "src/linen.cesta").read_text()
    # Every build gate of the Rust is a flag of the translation, and no other.
    rust_gates = set(re.findall(r'option_env!\("(\w+)"\)', main_rs)) | set(re.findall(r"cfg!\((\w+)\)", main_rs))
    c_gates = set(re.findall(r'cpp-if "defined\((\w+)\)"', translation))
    assert rust_gates == c_gates == set(ENV_FLAGS + CFG_FLAGS), (rust_gates ^ c_gates, c_gates ^ set(ENV_FLAGS + CFG_FLAGS))

    pdx = show("crates/sex-pdx/src/lib.rs")
    listen = pinned_function(pdx, "pub fn pdx_listen_raw(slot: u64) -> PdxMessage {", LISTEN)
    try_listen = pinned_function(pdx, "pub fn pdx_try_listen_raw(slot: u64) -> Option<PdxMessage> {", LISTEN)
    for name in ("session.rs", "sexobject.rs"):
        (work / name).write_text(show(f"servers/linen/src/{name}"))
    (work / "sex_object_model.rs").write_text(show("crates/sex-object-model/src/lib.rs"))
    run(["rustc", "--edition", "2021", "-O", "--crate-type", "rlib", "--crate-name", "sex_object_model",
         work / "sex_object_model.rs", "--out-dir", work])
    extern = ["--extern", f"sex_object_model={work / 'libsex_object_model.rlib'}"]
    rust = work / "reference.rs"
    rust.write_text(RUST_FFI + listen + try_listen + "}\n" + rust_reference(main_rs) + RUST_MAIN % "linen_start")
    (work / "provider.c").write_text(PROVIDER)
    (work / "main.c").write_text(C_MAIN % "linen_main")
    include = ["-I", HERE / "generated/include"]
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]
    plain_provider = work / "provider-plain.o"
    run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *warnings, *include, "-c", work / "provider.c", "-o", plain_provider])

    # The SexObject model and the adapters.
    model_include = work / "model-include"
    model_include.mkdir()

    def model_trace(model_source, linen_source, compiler="gcc", flags=()):
        (work / "model-interface.cesta").write_text(model_source)
        run([cesta, "export-c", "-o", model_include / "sex_object_model.h", work / "model-interface.cesta", "--"])
        (work / "linen-model.cesta").write_text(linen_source)
        run([cesta, "export-c", "-o", work / "linen.c", work / "linen-model.cesta", "--", *include])
        (work / "model.c").write_text(MODEL_C)
        harness = work / "model-harness"
        run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *flags, "-I", work, "-I", model_include, *include,
             work / "model.c", work / "provider.c", *runtime, "-o", harness])
        return run([harness]).stdout

    (work / "model.rs").write_text(MODEL_RUST)
    run(["rustc", "--edition", "2021", "-O", "-A", "warnings", *extern, work / "model.rs", "-o", work / "model-reference"])
    model_expected = run([work / "model-reference"]).stdout
    model_source = (HERE / "src/sex_object_model_interface.cesta").read_text()
    for compiler in ("gcc", "clang"):
        actual = model_trace(model_source, translation, compiler, [*warnings, "-Wno-unused-function", *sanitizers])
        if actual != model_expected:
            raise SystemExit(first_difference(actual.decode(), model_expected.decode(), f"SexObject model {compiler}"))
    model_mutations = [
        (("(if (|| (== v 7) (> v 11)) (return false))", "(if (> v 11) (return false))"), None),
        (("(return (call sex_object_ref_new (-> h object_id) (-> h rights_generation))))",
          "(return (call sex_object_ref_new (-> h object_id) (-> h content_generation))))"), None),
        (("(if (> (-> h kind) UINT16_MAX) (return false))", "(if (> (-> h kind) UINT32_MAX) (return false))"), None),
        (None, ("(= (. object_id) (-> obj sexfiles_object_id))", "(= (. object_id) (-> obj object_id))")),
        (None, ("(?: (== kind OBJECT_KIND_SESSION) SEX_OBJECT_KIND_SPINDLE_SESSION SEX_OBJECT_KIND_RAW_BLOB)",
                "(?: (== kind OBJECT_KIND_SESSION) SEX_OBJECT_KIND_SPINDLE_SESSION SEX_OBJECT_KIND_LINEN_PROJECT)")),
    ]
    for model_change, linen_change in model_mutations:
        mutant_model, mutant_linen = model_source, translation
        for change, name in ((model_change, "model"), (linen_change, "linen")):
            if change:
                text = mutant_model if name == "model" else mutant_linen
                assert text.count(change[0]) == 1, change[0]
                if name == "model":
                    mutant_model = text.replace(*change)
                else:
                    mutant_linen = text.replace(*change)
        if model_trace(mutant_model, mutant_linen, flags=["-Wno-unused-function"]) == model_expected:
            raise SystemExit(f"model mutation not detected: {model_change or linen_change}")
    model_lines = len(model_expected.splitlines())

    # Controls: each mutation must change some trace of the variant named with it.
    mutations = [
        ("(= stale (!= (. obj name_len) nlen))", "(= stale false)", ()),
        ("(= (* out) ([] (-> l defer) 0))", "(= (* out) ([] (-> l defer) (- (-> l defer_len) 1)))", ()),
        ("(if (<= n 4)", "(if (<= n 3)", ()),
        ("(<< (& (. obj sexfiles_object_id) 0xFF) 48)", "(<< (& (. obj sexfiles_object_id) 0xFF) 40)", ()),
        ("(var uint64_t max_len (?: (< (- 19 off) 8) (- 19 off) 8))", "(var uint64_t max_len 8)", ("SEXOS_LINEN_SEXFILES100_PROOF",)),
        ("(fn (static bool) diskfs_read_ok ((int64_t reply)) (return (> (cast uint64_t reply) 255)))",
         "(fn (static bool) diskfs_read_ok ((int64_t reply)) (return (> reply 255)))", (AP + "AP2",)),
        ("(if (== b 0xED) (= high 0x9F))", "(cast void 0)", ()),
        ("(= (. key_log) 16)", "(= (. key_log) 17)", ()),
        ("(?: (== old 0) (cast uint8_t (- count 1)) (cast uint8_t (- old 1)))", "(?: (== old 0) 0 (cast uint8_t (- old 1)))", ()),
        ("(var size_t max_len (?: (< arg2 8) arg2 8))", "(var size_t max_len (?: (< arg2 8) arg2 7))", ()),
        ("(= ([] meta 23) name_len)", "(= ([] meta 22) name_len)", ()),
        ("(for (var uint64_t w 0) (< w 128) (++ w)", "(for (var uint64_t w 0) (< w 127) (++ w)", ("LINEN_SEXOBJECT_NATIVE_PERSIST_PROOF",)),
        ("(if (!= (. msg type_id) 0) (call linen_defer_stash l (& msg)))", "(cast void 0)", ()),
        ("(= (. (-> slot object) generation) 1)", "(= (. (-> slot object) generation) 2)", ("SEXOS_LINEN_SEXFILES_METADATA_PROOF",)),
        ("(for (var uint32_t i 0) (< i 5000000) (++ i) (call sexos_spin_loop_hint))",
         "(for (var uint32_t i 0) (< i 4999999) (++ i) (call sexos_spin_loop_hint))", ()),
        ("(if (== slot_idx LINEN_MAX_OBJECTS) (return ERR_TABLE_FULL))", "(if (== slot_idx 15) (return ERR_TABLE_FULL))", ()),
        ("(if (call take (& (-> l key_log)))", "(if true", ()),
        ("(if (!= (* id) 0)\n            (block\n              (var LinenObject obj)",
         "(if false\n            (block\n              (var LinenObject obj)", ()),
    ]
    mutation_variants = {variant for _, _, variant in mutations}
    cases = scenarios(args.cases)
    compared = 0
    references = {}
    markers = [rb"S 5000000\n", rb"\[linen\.defer\.stash\]", rb"\[linen\.defer\.drop\]", rb"\[linen\.defer\.replay\]",
               rb"\[linen\.disk\.rescan\.skip\]", rb"\[linen\.disk\.rescan\.err\] stage=info", rb"renamed=[1-9]", rb"removed=[1-9]",
               rb"\[perf\.noise\.summary\]", rb"token=\? ", rb"\[linen\.search\.bridge\.result\] token=work count=[1-9]\d* selected=[1-9]",
               rb"reason=create_failed err=-1", rb"\[linen\.ramfs\.read\.match\] id=\d+ len=19 ok=1", rb"reason=name_mismatch",
               rb"\[linen\.disk\.object\.load\.match\] ok=1", rb"\[linen\.disk\.object\.save\.write\] chunk=",
               rb"\[linen\.disk\.object\.load\.bounds_negative\] ok=1", rb"\[linen\.sexobject\.native\.done\] ok=1",
               rb"\[linen\.sexobject\.native\.timeout\]", rb"reason=bad_object_id", rb"\[linen\.diskfs\.slot\.min\.match\] ok=1",
               rb"\[linen\.diskfs\.slot\.min\.read\.err\]", rb"\[linen\.diskfs100\.ap2\.done\] ok=1",
               rb"\[linen\.diskfs100\.ap2\.flush\.honest\]", rb"\[linen\.diskfs100\.ap3\.write\.all_done\] ok=1",
               rb"\[linen\.diskfs100\.ap3\.read\.match\] bytes=128 ok=1", rb"\[linen\.diskfs100\.ap3\.fail\] phase=read",
               rb"\[linen\.keyboard\.nav\.proof\.done\] ok=1", rb"\[linen\.object\.workflow\.proof\.done\] ok=1",
               rb"\[linen\.sexfiles100\.audit\.done\] ok=1 count=5", rb"\[linen\.sexfiles\.persist\.fail\]",
               rb"\[linen\.session\.proof\.bounds\] table_full_err=-1", rb"\[linen\.sexfiles\.proof\.generation\] id=\d+ gen=2",
               rb"\[linen\.diskfs100\.ap5\.neg\.done\] case=flush_skip ok=1", rb"\[linen\.object\.persist\.send\]",
               rb"\[linen\.snapshot\.name\] id=\d+ off=\d+ len=[1-8]"]
    unreached = set(markers)
    for variant in VARIANTS:
        tag = tag_of(variant)
        env = {name: value for name, value in os.environ.items() if name not in ENV_FLAGS}
        for name in variant:
            if name in ENV_FLAGS:
                env[name] = "1"
        cfgs = [part for name in variant if name in CFG_FLAGS for part in ("--cfg", name)]
        reference = work / f"reference-{tag}"
        run(["rustc", "--edition", "2021", "-O", "-A", "warnings", *extern, *cfgs, rust, "-o", reference,
             "-C", f"link-arg={plain_provider}"], env=env)
        harnesses = []
        for compiler in ("gcc", "clang"):
            obj = work / f"linen-{compiler}-{tag}.o"
            run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", *warnings, *(f"-D{n}" for n in variant), *sanitizers, *include,
                 "-c", HERE / "src/linen.cesta", "-o", obj])
            harness = work / f"harness-{compiler}-{tag}"
            run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *sanitizers, *include, work / "provider.c", work / "main.c", obj,
                 *runtime, "-o", harness])
            harnesses.append((compiler, harness))
        for number, case in enumerate(cases):
            expected = run([reference, *case]).stdout
            assert b"PANIC" not in expected and b"UNEXPECTED" not in expected, (tag, number)
            unreached -= {m for m in unreached if re.search(m, expected)}
            if variant in mutation_variants:
                references[variant, number] = expected
            for compiler, harness in harnesses:
                actual = run([harness, *case]).stdout
                if actual != expected:
                    raise SystemExit(first_difference(actual.decode(errors="replace"), expected.decode(errors="replace"),
                                                      f"variant {tag} {variant} scenario {number} {compiler}"))
            compared += len(expected.splitlines())
        print(f"linen: variant {tag} equal", flush=True)

    assert not unreached, f"no scenario reached {sorted(unreached)}"

    for number, (old_text, new_text, variant) in enumerate(mutations):
        assert translation.count(old_text) == 1, old_text
        mutant = work / f"linen-mutant-{number}.cesta"
        mutant.write_text(translation.replace(old_text, new_text))
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *(f"-D{n}" for n in variant), *include, "-c", mutant, "-o", work / "mutant.o"])
        run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include, work / "provider.c", work / "main.c", work / "mutant.o", *runtime,
             "-o", work / "mutant-harness"])
        if not any(run([work / "mutant-harness", *case]).stdout != references[variant, index] for index, case in enumerate(cases)):
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")
    print(f"linen: Cesta translation equals the Rust server on {compared} trace lines ({args.cases} scenarios x {len(VARIANTS)} "
          f"build variants, GCC and Clang, address and UB sanitizers); SexObject model and adapters equal on {model_lines} "
          f"lines; {len(mutations)} server and {len(model_mutations)} model mutations detected")


if __name__ == "__main__":
    main()
