#!/usr/bin/env python3
"""Compare servers/quil with its Cesta translation.

The Rust server is compiled from the pinned revision with only its no_std
scaffolding removed: the failing allocator, the panic handler, its own
memcpy, memset, memmove and memcmp, and the _start name. crates/sex-pdx is
replaced by a module that keeps the pinned pdx_listen_raw and
pdx_try_listen_raw (their listen syscall routed to the provider), the pinned
READ_V2 decoders and the pinned constants Quil uses. One scripted provider,
written in C, serves both servers (see differential_support).

The provider plays SexFiles: RamFS holds the document file quil_doc_01, read
back as written; DiskFS V3 holds 16 objects by path id, each a name and up
to 16 KiB of content, some starting as valid Quil documents, with exact
READ_V2 replies (OK with up to six bytes, EOF at the end, encoded errors
past it). Its faults are errors, short writes, short reads, encoded errors,
a status with no magnitude (an Err of 0), corrupted words, refused enqueues
and, in quiet scenarios, replies longer than asked for, which panic the Rust
(slices out of range) and so must spin the translation. A busy SexFiles
never answers the native SexObject calls. Requests are HID keys (single keys
and sequences that work the palette's save, load and New Buffer, name entry,
typing, shift, cursor moves and bursts of long and many lines), pings,
disk-document opens, unknown types and stray replies, also while a storage
reply is awaited. The bounded persist call times out only after 300000
empty polls, beyond every scenario's yield limit, and the buffer fills only
after 12288 keys, so neither path is reached; nor is the overflow marker of
draw_text_lines, whose line count is already capped at 24.
"""

import argparse
import os
from pathlib import Path
import random
import re
import shutil
from layout_text import LayoutText

from differential_support import (C_MAIN, LISTEN, PROVIDER_PRELUDE, RUST_FFI, RUST_MAIN, first_difference,
                                  pinned_function, run)

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
Q = "SEXOS_QUIL_"
EDITOR = [Q + name for name in ("TEXT_COMMANDS_PROOF", "CURSOR_NAV_PROOF", "TEXT_SELECTION_PROOF", "TEXT_DELETE_PROOF",
                                "EDITOR_KEYBINDINGS_PROOF", "UNDO_REDO_PROOF", "UNDO_REDO_KEY_PROOF", "VISUAL_CURSOR_PROOF",
                                "FIND_PROOF", "MOD_LOWERCASE_PROOF", "WORD_NAV_PROOF", "LINE_STATS_PROOF", "FIND_NAV_PROOF",
                                "SEL_COPY_DELETE_PROOF", "DIRTY_PROOF", "CMD_SURFACE_PROOF", "CLIPBOARD_STATUS_PROOF",
                                "PASTE_PROOF", "REPLACE_PROOF", "GOTO_LINE_PROOF", "STORAGE_PHASEA_PROOF")]
SLOT, HIGHBIT, NAV, KBUF, TBUF, ASYNC, SAVE_OPEN, INPUT, LIVE_USB, PHYSICAL = (
    Q + "DISKFS_SLOT_PROOF", Q + "READ_V2_HIGHBIT_PROOF", Q + "KEYBOARD_NAV_PROOF", Q + "KEYBOARD_BUFFER_PROOF",
    Q + "TEXT_BUFFER_PROOF", Q + "TEXT_SAVE_ASYNC_PROOF", Q + "SAVE_OPEN_SEXOBJECT_PROOF", Q + "TEXT_INPUT_PIPELINE_PROOF",
    Q + "LIVE_USB_CREATE_SAVE_REOPEN_PROOF", Q + "PHYSICAL_KEYBOARD_PROOF")
CAP = "SEXOS_STORAGE_CAP_PROOF"
ENV_FLAGS = [CAP, SLOT, HIGHBIT, NAV, KBUF, TBUF, ASYNC, *EDITOR, SAVE_OPEN, INPUT, LIVE_USB, PHYSICAL]
CFG_FLAGS = ["sexfiles_quil_persistence_proof"]
SINGLES = [SLOT, HIGHBIT, NAV, KBUF, TBUF, ASYNC, SAVE_OPEN, INPUT, LIVE_USB, PHYSICAL, CAP, CFG_FLAGS[0]]
DEFAULT_VARIANTS = [(), tuple(EDITOR), *((flag,) for flag in SINGLES), (SAVE_OPEN, LIVE_USB, PHYSICAL, INPUT),
                    (NAV, KBUF, TBUF, PHYSICAL), (SLOT, HIGHBIT, CFG_FLAGS[0]), tuple(ENV_FLAGS + CFG_FLAGS)]
ALL_VARIANTS = DEFAULT_VARIANTS + [(flag,) for flag in EDITOR]

# SexFiles for Quil: the server part of the shared provider.
PROVIDER = PROVIDER_PRELUDE + r'''
/* RamFS: the one document file. DiskFS V3: objects by path id; a select
   picks the object later writes, truncates and reads use. */
#define OBJECTS 16
#define CAPACITY 16400
static uint8_t ram_file[16400];
static bool ram_exists;
static uint64_t handles;
static struct { bool used; uint8_t name[16]; uint8_t data[CAPACITY]; size_t size; } objects[OBJECTS];
static int selected = -1;
/* SexFiles still running its startup proofs (heavy faults, balanced mix): it never answers the native SexObject calls. */
static bool busy;
/* Queued keys of a sequence. */
static uint64_t queue_code[48], queue_value[48];
static size_t queue_len, queue_pos;

/* A READ_V2 reply: status in bits 56-63, a length or magnitude in 48-55, payload below. */
static uint64_t v2(uint64_t status, uint64_t field, uint64_t payload) { return status << 56 | field << 48 | payload; }
static uint64_t v2_bytes(const uint8_t *data, size_t from, size_t n) {
    uint64_t w = 0;
    for (size_t i = 0; i < n; ++i) w |= (uint64_t)data[from + i] << (8 * i);
    return w;
}

static uint64_t storage_reply(void) {
    uint64_t a0 = call.a0, a1 = call.a1, a2 = call.a2;
    switch (call.opcode) {
    case 0x30: /* RAMFS_OPEN: name in a0 and a1, O_CREATE in bit 24 of a2 */
        if (fault()) return error();
        if (!(a2 >> 24 & 1) && !ram_exists) return UINT64_MAX - 1;
        ram_exists = true;
        return ++handles;
    case 0x32: /* RAMFS_WRITE: handle, offset, data */
        if (fault()) return error();
        if (a1 <= sizeof ram_file - 8)
            for (int i = 0; i < 8; ++i) ram_file[a1 + i] = (uint8_t)(a2 >> 8 * i);
        return 8;
    case 0x31: /* RAMFS_READ: handle, offset, length; 0xDEADBEEF is no handle (-1) */
        if (a0 == 0xDEADBEEF) return PICK(UINT64_MAX, UINT64_MAX, UINT64_MAX - 2, 7);
        if (fault()) {
            uint64_t k = next() % 3;
            return k == 0 ? error() : k == 1 ? 0 : word_at(ram_file, sizeof ram_file, a1) ^ 0x0100;
        }
        return a1 < sizeof ram_file ? word_at(ram_file, sizeof ram_file, a1) : 0;
    case 0x33: /* RAMFS_CLOSE */
        return fault() ? error() : 0;
    case 0x3E: /* DISKFS_SELECT */
        if (fault()) return error();
        if (a0 >= OBJECTS || !objects[a0].used) return UINT64_MAX - 2;
        selected = (int)a0;
        return 0;
    case 0x3B: /* DISKFS_STAT */
        return fault() ? error() : 4096;
    case 0x42: { /* DISKFS_CREATE: the name in a1 and a2; -8 when it exists */
        if (fault()) return error();
        uint8_t name[16];
        for (int i = 0; i < 8; ++i) {
            name[i] = (uint8_t)(a1 >> 8 * i);
            name[8 + i] = (uint8_t)(a2 >> 8 * i);
        }
        for (int i = 0; i < OBJECTS; ++i)
            if (objects[i].used && !memcmp(objects[i].name, name, 16)) return UINT64_MAX - 7;
        for (int i = 3; i < OBJECTS; ++i)
            if (!objects[i].used) {
                objects[i].used = true;
                memcpy(objects[i].name, name, 16);
                objects[i].size = 0;
                return (uint64_t)i;
            }
        return UINT64_MAX - 11;
    }
    case 0x38: /* DISKFS_WRITE: offset, low word, high word */
        if (fault()) return PICK(UINT64_MAX - 3, 4, 8);
        if (selected < 0) return UINT64_MAX - 2;
        if (a0 > CAPACITY - 16) return UINT64_MAX - 3;
        for (int i = 0; i < 8; ++i) {
            objects[selected].data[a0 + i] = (uint8_t)(a1 >> 8 * i);
            objects[selected].data[a0 + 8 + i] = (uint8_t)(a2 >> 8 * i);
        }
        if (a0 + 16 > objects[selected].size) objects[selected].size = a0 + 16;
        return 16;
    case 0x49: /* DISKFS_TRUNCATE to a0 */
        if (fault()) return error();
        if (selected < 0) return UINT64_MAX - 2;
        if (a0 > CAPACITY) return UINT64_MAX - 3;
        objects[selected].size = a0;
        return 0;
    case 0x43: /* DISKFS_LIST: 8 bytes (a1) of object a0's name */
        if (fault()) return error();
        if (a0 >= OBJECTS || !objects[a0].used) return UINT64_MAX - 2;
        return a1 < 2 ? word_at(objects[a0].name, 16, a1 * 8) : 0;
    case 0x4A: { /* DISKFS_READ_V2: offset, length */
        if (selected < 0) return v2(0xFF, 3, 0);
        size_t size = objects[selected].size;
        size_t n = a0 < size ? size - a0 : 0;
        if (n > a1) n = a1;
        if (n > 6) n = 6;
        if (fault()) {
            uint64_t k = next() % 6;
            if (k == 0) return error();
            if (k == 1) return v2(0xFF, 4, 0);
            if (k == 2) return v2(0x02, 0, 0);
            if (k == 3 && n > 0) return v2(0, n - 1, v2_bytes(objects[selected].data, a0, n - 1));
            if (k == 4 && scenario.mix == 2 && n > 0 && n < 6 && a0 + n < size) return v2(0, n + 1, v2_bytes(objects[selected].data, a0, n + 1));
            return v2(0x01, 0, 0);
        }
        if (a0 == size) return v2(0x01, 0, 0);
        if (a0 > size) return v2(0xFF, 4, 0);
        return v2(0, n, v2_bytes(objects[selected].data, a0, n));
    }
    case 0x40: /* native SexObject save: the object id */
        if (fault()) return 0;
        return PICK(1, 1, 2, 5);
    case 0x41: /* native SexObject open: the length, 4 for test */
        if (fault()) return PICK(3, 0);
        return 4;
    default:
        return next();
    }
}

/* Scancodes: letters, digits, punctuation, space, the editing and palette keys, their variants, shift press and release, and unmapped ones. */
static const uint64_t KEYS[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1E, 0x1F, 0x20, 0x21, 0x22,
    0x23, 0x24, 0x25, 0x26, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x02, 0x03, 0x04, 0x0B, 0x27, 0x28, 0x33, 0x34, 0x35,
    0x39, 0x39, 0x0E, 0x0E, 0x1C, 0x1C, 0x01, 0x48, 0x50, 0x4B, 0x4D, 0x47, 0x4F, 0x53, 0x2A, 0xAA, 0x67, 0x6C, 0x0D, 0x1B,
    0x3B, 0x110, 0x0F};
#define KEY_COUNT (sizeof KEYS / sizeof KEYS[0])
static const uint64_t PRINTABLE[] = {0x10, 0x12, 0x14, 0x17, 0x18, 0x1E, 0x1F, 0x20, 0x26, 0x31, 0x32, 0x39, 0x39, 0x03, 0x34};
#define PRINTABLE_COUNT (sizeof PRINTABLE / sizeof PRINTABLE[0])
/* Sequences as code-value pairs ending at 0: save (row 1), load (row 2), New Buffer (row 3), Esc, test, a name and
   Enter, shifted A then a, cursor edits, ups, Esc-Down-Enter and three Enters. */
static const uint64_t SEQUENCES[][16] = {
    {0x50, 1, 0x1C, 1, 0}, {0x50, 1, 0x50, 1, 0x1C, 1, 0}, {0x50, 1, 0x50, 1, 0x50, 1, 0x1C, 1, 0}, {0x01, 1, 0},
    {0x14, 1, 0x12, 1, 0x1F, 1, 0x14, 1, 0}, {0x31, 1, 0x18, 1, 0x14, 1, 0x12, 1, 0x1C, 1, 0},
    {0x2A, 1, 0x1E, 1, 0x2A, 0, 0x1E, 1, 0}, {0x4B, 1, 0x4B, 1, 0x0E, 1, 0}, {0x47, 1, 0x53, 1, 0x4F, 1, 0},
    {0x48, 1, 0x48, 1, 0x50, 1, 0}, {0x01, 1, 0x50, 1, 0x1C, 1, 0}, {0x1C, 1, 0x1C, 1, 0x1C, 1, 0}};
#define SEQUENCE_COUNT (sizeof SEQUENCES / sizeof SEQUENCES[0])

static void enqueue(uint64_t code, uint64_t value) {
    if (queue_len < 48) {
        queue_code[queue_len] = code;
        queue_value[queue_len] = value;
        ++queue_len;
    }
}

static void request(SexosMessage *m) {
    uint64_t kind = next() % 100;
    m->caller_pd = (uint32_t)PICK(6, 6, 3, 9);
    if (kind < 72) {
        m->type_id = 0x202;
        if (queue_pos == queue_len && chance(35)) {
            queue_len = queue_pos = 0;
            uint64_t which = next() % (SEQUENCE_COUNT + 2);
            if (which < SEQUENCE_COUNT) {
                for (size_t i = 0; SEQUENCES[which][i]; i += 2) enqueue(SEQUENCES[which][i], SEQUENCES[which][i + 1]);
            } else {
                /* A burst: a long line (wrapping at 80 columns), or many short ones (scrolling the 22-row view). */
                bool lines = which == SEQUENCE_COUNT + 1;
                for (int i = 0; i < 46; ++i) {
                    uint64_t key = lines && i % 2 ? 0x1C : PRINTABLE[next() % PRINTABLE_COUNT];
                    enqueue(key, 1);
                }
            }
        }
        if (queue_pos < queue_len) {
            m->arg0 = queue_code[queue_pos];
            m->arg1 = queue_value[queue_pos];
            ++queue_pos;
        } else {
            m->arg0 = KEYS[next() % KEY_COUNT];
            m->arg1 = PICK(1, 1, 1, 0, 2);
        }
        m->arg2 = PICK(0, 0, 1);
    } else if (kind < 80) {
        m->type_id = 0xD0;
    } else if (kind < 86) {
        m->type_id = 0x4A;
        m->arg0 = next() % 40;
        m->arg1 = PICK(0, 2, 3, 4, 5, 15, 9999);
    } else if (kind < 94 || busy || !scenario.faults) {
        /* Stray replies come only with faults, and never from a busy SexFiles. */
        m->type_id = PICK(0x41, 0x100, 0x42, 0xC0);
        m->arg0 = next();
    } else {
        m->type_id = 1;
        m->arg0 = next();
    }
}

static uint64_t reply_share(void) { return scenario.mix == 2 ? 90 : 50; }
static bool answered(uint64_t opcode) { return !(busy && (opcode == 0x40 || opcode == 0x41)); }

/* A Quil document: the header (QP01 and the length) at 0, the content at 16. */
static void put_document(int id, const char *name, const uint8_t *text, size_t len) {
    objects[id].used = true;
    memset(objects[id].name, 0, 16);
    memcpy(objects[id].name, name, strlen(name));
    uint64_t header = 0x31305051ull | (uint64_t)len << 32;
    for (int i = 0; i < 8; ++i) objects[id].data[i] = (uint8_t)(header >> 8 * i);
    memcpy(objects[id].data + 16, text, len);
    objects[id].size = 16 + len;
}

static void server_start(void) {
    busy = scenario.faults == 2 && scenario.mix == 0;
    static const uint8_t text[] = "hello from disk\nline two \xc3\xa9\x80\xff end\n";
    objects[0].used = objects[1].used = true;
    memcpy(objects[0].name, "sexfiles-proof-v", 16);
    memcpy(objects[1].name, "linen-object-v1", 15);
    objects[0].size = objects[1].size = 4096;
    for (int i = 0; i < 4096; ++i) objects[1].data[i] = (uint8_t)next();
    if (chance(70)) put_document(2, "n-v1", text, sizeof text - 1);
    else {
        objects[2].used = true;
        memcpy(objects[2].name, "n-v1", 4);
        objects[2].size = (size_t)(next() % 24);
    }
    if (chance(60)) put_document(3, "notes", text + 6, 9);
    if (chance(40)) put_document(4, "draft", text, 30);
}
'''

def pinned_line(source, prefix):
    lines = [line for line in source.splitlines() if line.startswith(prefix)]
    assert len(lines) == 1, prefix
    return "    " + lines[0] + "\n"


def rust_reference(source):
    for name in ("memcpy", "memset", "memmove", "memcmp"):
        source, count = re.subn(r'#\[no_mangle\]\npub unsafe extern "C" fn %s\(.*?\n}\n' % name, "", source, flags=re.S)
        assert count == 1, name
    for old, new in (("#![no_std]\n", ""), ("#![no_main]\n", ""),
                     ("#[global_allocator]\nstatic ALLOCATOR: DummyAllocator = DummyAllocator;\n", ""),
                     ("#[panic_handler]\nfn panic(_: &core::panic::PanicInfo) -> ! {\n    loop {}\n}\n", ""),
                     ('#[no_mangle]\npub extern "C" fn _start() -> ! {', "fn quil_start() -> ! {")):
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    return source


# A scenario is seed, limit (yields), listen (percent), faults (0-2) and mix (0-2).
def scenarios(count):
    r = random.Random(0x0B11)
    return [[r.getrandbits(64), [9000, 2500, 300][(k // 9) % 3], r.choice([25, 70, 95]), k % 3, (k // 3) % 3]
            for k in range(count)]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=12)
    parser.add_argument("--all-variants", action="store_true", help="also each editor proof alone")
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    variants = ALL_VARIANTS if args.all_variants else DEFAULT_VARIANTS
    work = HERE / "build/quil-differential"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    main_rs = show("servers/quil/src/main.rs")
    translation = LayoutText((HERE / "src/quil.cesta").read_text())
    # Every build gate of the Rust is a flag of the translation, and no other.
    rust_gates = set(re.findall(r'option_env!\("(\w+)"\)', main_rs)) | set(re.findall(r"cfg!\((\w+)\)", main_rs))
    c_gates = set(re.findall(r'cpp-if "defined\((\w+)\)"', translation))
    assert rust_gates == c_gates == set(ENV_FLAGS + CFG_FLAGS), (rust_gates ^ c_gates, c_gates ^ set(ENV_FLAGS + CFG_FLAGS))

    pdx = show("crates/sex-pdx/src/lib.rs")
    pinned = [pinned_function(pdx, "pub fn pdx_listen_raw(slot: u64) -> PdxMessage {", LISTEN),
              pinned_function(pdx, "pub fn pdx_try_listen_raw(slot: u64) -> Option<PdxMessage> {", LISTEN),
              pinned_function(pdx, "pub fn diskfs_v2_payload(reply: u64) -> [u8; DISKFS_V2_MAX_READ] {", {}),
              *(pinned_line(pdx, prefix) for prefix in (
                  "pub const OP_QUIL_PING", "pub const OP_QUIL_OPEN_DISK_DOC", "pub const OP_TEXT_DRAW", "pub const OP_TEXT_CLEAR",
                  "pub const ERR_CAP_INVALID", "pub const DISKFS_V2_STATUS_OK", "pub const DISKFS_V2_STATUS_EOF",
                  "pub const DISKFS_V2_MAX_READ", "pub fn diskfs_v2_status", "pub fn diskfs_v2_len_field"))]
    rust = work / "reference.rs"
    rust.write_text(RUST_FFI + "".join(pinned) + "}\n" + rust_reference(main_rs) + RUST_MAIN % "quil_start")
    (work / "provider.c").write_text(PROVIDER)
    (work / "main.c").write_text(C_MAIN % "quil_main")
    include = ["-I", HERE / "generated/include"]
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]
    plain_provider = work / "provider-plain.o"
    run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *warnings, *include, "-c", work / "provider.c", "-o", plain_provider])

    # Controls: each mutation must change some trace of the variant named with it.
    mutations = [
        ("(if (&& (>= col QUIL_VIEW_COLS) (< i len))", "(if (&& (> col QUIL_VIEW_COLS) (< i len))", ()),
        ("(= (-> q view_top) (- (+ cline 1) QUIL_VIEW_ROWS))", "(= (-> q view_top) (- cline QUIL_VIEW_ROWS))", ()),
        ("(if (&& (== i cursor) (< total 512)) (= ([] line_buf (post++ total)) 0x23))",
         "(if (&& (== i cursor) (< total 512)) (= ([] line_buf (post++ total)) 0x7C))", ()),
        ("(while (< total 40) (= ([] line_buf (post++ total)) 0x20))", "(while (< total 39) (= ([] line_buf (post++ total)) 0x20))", ()),
        ("(var size_t offset (* (- k 1) 8))", "(var size_t offset (* (- chunks k) 8))", ()),
        ("(call sexos_line \"[quil.undo.push] idx=%zu len=%zu ok=1\" (?: (== (-> q undo_head) 0) (- UNDO_DEPTH 1) (- (-> q undo_head) 1))",
         "(call sexos_line \"[quil.undo.push] idx=%zu len=%zu ok=1\" (?: (== (-> q undo_head) 0) 0 (- (-> q undo_head) 1))", ()),
        ("(init 0x1E 'a' 'A')", "(init 0x1E 'a' 'a')", ()),
        ("(case 0x1c (case 0x0d (return 3)))", "(case 0x1c (return 3))", ()),
        ("(| QUIL_PERSIST_MAGIC (<< (cast uint64_t len) 32))", "(| QUIL_PERSIST_MAGIC (<< (cast uint64_t len) 31))", ()),
        ("(&& (== total 0) (== (& raw 0xFF) 0))", "(== (& raw 0xFF) 0)", ()),
        ("(define HID_STASH_CAPACITY \"((size_t)8)\")", "(define HID_STASH_CAPACITY \"((size_t)7)\")", ()),
        ("(= (-> q key_log) 16)", "(= (-> q key_log) 15)", ()),
        ("(= (-> q selected_row) (?: (== old 0) (- QUIL_ROWS 1) (- old 1)))", "(= (-> q selected_row) (?: (== old 0) 0 (- old 1)))", ()),
        ("(return (call min_size lines QUIL_MAX_VISIBLE_LINES)))", "(return lines))", ()),
        ("(= (-> q shift_held) (== value 1))", "(= (-> q shift_held) true)", ()),
        ("(var (static const size_t) ([] probe_at 4) (init 16 20 24 28))", "(var (static const size_t) ([] probe_at 4) (init 16 20 24 27))", (HIGHBIT,)),
        ("(= ([] payload 15) 0x02)", "(= ([] payload 15) 0x03)", (SLOT,)),
        ("(if (< pos (-> q buffer_len)) (= (-> q cursor_pos) pos))", "(= (-> q cursor_pos) pos)", tuple(EDITOR)),
        ("(if (> (+ hoff n) 8) (call sexos_spin_forever))", "(if (> (+ hoff n) 9) (call sexos_spin_forever))", ()),
        ("(if (!= (call v2_status reply) DISKFS_V2_STATUS_OK) (return (call fail err (- (cast int64_t (call v2_len_field reply))))))",
         "(if (!= (call v2_status reply) DISKFS_V2_STATUS_OK) (return (call fail err (- 2))))", ()),
        ("(if (! (call await_reply q 32 (& object_id)))\n        (block\n          (call sexos_line \"[quil.sexobject.save.reply.timeout] attempts=32\")",
         "(if (! (call await_reply q 31 (& object_id)))\n        (block\n          (call sexos_line \"[quil.sexobject.save.reply.timeout] attempts=32\")", (SAVE_OPEN,)),
        ("(if (&& (-> q physical_keyboard_active) (-> q palette_active)) (= (-> q palette_active) false))", "(cast void 0)", (PHYSICAL,)),
    ]
    mutation_variants = {variant for _, _, variant in mutations}
    assert mutation_variants <= set(variants)
    markers = [rb"\[quil\.persist\.save\.ok\]", rb"\[quil\.persist\.load\.ok\]", rb"\[quil\.name\.mode\] on=1", rb"\[quil\.doc\.create\.ok\]",
               rb"\[quil\.doc\.create\.fail\] error=-8", rb"\[quil\.new\.ok\]", rb"\[quil\.save\.ok\]", rb"\[quil\.load\.ok\]",
               rb"\[quil\.persist\.load\.miss\]", rb"\[quil\.persist\.load\.err\] stage=read", rb"\[quil\.persist\.load\.err\] stage=header err=0",
               rb"\[quil\.persist\.load\.miss\.fallback\]", rb"\[quil\.hid\.stash\] idx=", rb"\[quil\.hid\.stash\.drop\]", rb"\[quil\.sync\.skip\]",
               rb"dir=up ok=0 reason=first_line", rb"dir=down ok=0 reason=last_line", rb"\[quil\.cursor\.move\] old=\d+ new=\d+ len=\d+ dir=up ok=1",
               rb"\[quil\.view\] top=[1-9]", rb"\[quil\.open\.disk_doc\.recv\]", rb"\[quil\.open\.disk_doc\.load\.fail\]",
               rb"\[quil\.route\.recv\]", rb"\[quil\.unknown\.yield\]", rb"ch=0 reason=unmapped", rb"\[quil\.palette\.reject\] action=key",
               rb"\[quil\.text\.recv\] code=30 ch=65", rb"\[quil\.text\.backspace\] old=\d+ new=\d+ at=\d+ ok=1", rb"\[quil\.text\.delete\] mode=char",
               rb"\[quil\.sexobject\.save\.open\.done\] ok=1", rb"\[quil\.sexobject\.save\.reply\.timeout\]",
               rb"\[live_usb\.quil_create_save_reopen\.done\] ok=1", rb"\[physical_keyboard\.quil\.done\] ok=1",
               rb"\[quil\.read_v2\.highbit\.done\] ok=1", rb"reported_failure=1 buffer_intact=1 dirty_preserved=1 ok=1",
               rb"\[quil\.diskfs\.slot\.min\.done\] ok=1", rb"\[quil\.diskfs\.slot\.min\.read\.err\] status=",
               rb"\[quil\.sexfiles\.proof\.match\]", rb"\[quil\.sexfiles\.proof\.deny\] invalid_handle error=-1",
               rb"\[quil\.save_load\.proof\.ok\]", rb"\[quil\.storage\.cap\.ok\]", rb"\[quil\.text\.buffer\.proof\.done\] ok=1",
               rb"\[quil\.keyboard\.buffer\.proof\.done\] ok=1", rb"\[quil\.goto\.line\] line=9", rb"\[quil\.replace\.result\] count=2",
               rb"\nPANIC\n"]
    unreached = set(markers)
    cases = scenarios(args.cases)
    compared = 0
    references = {}
    for variant in variants:
        tag = f"v{variants.index(variant)}"
        env = {name: value for name, value in os.environ.items() if name not in ENV_FLAGS}
        for name in variant:
            if name in ENV_FLAGS:
                env[name] = "1"
        cfgs = [part for name in variant if name in CFG_FLAGS for part in ("--cfg", name)]
        reference = work / f"reference-{tag}"
        run(["rustc", "--edition", "2021", "-O", "-A", "warnings", *cfgs, rust, "-o", reference, "-C", f"link-arg={plain_provider}"], env=env)
        harnesses = []
        for compiler in ("gcc", "clang"):
            obj = work / f"quil-{compiler}-{tag}.o"
            run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", *warnings, "-DSEXOS_QUIL_CURRENT_KEYS=0", *(f"-D{n}" for n in variant), *sanitizers, *include,
                 "-c", HERE / "src/quil.cesta", "-o", obj])
            harness = work / f"harness-{compiler}-{tag}"
            run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *sanitizers, *include, work / "provider.c", work / "main.c", obj,
                 *runtime, "-o", harness])
            harnesses.append((compiler, harness))
        for number, case in enumerate(cases):
            expected = run([reference, *case]).stdout
            assert b"UNEXPECTED" not in expected, (tag, number)
            unreached -= {m for m in unreached if re.search(m, expected)}
            if variant in mutation_variants:
                references[variant, number] = expected
            for compiler, harness in harnesses:
                actual = run([harness, *case]).stdout
                if actual != expected:
                    raise SystemExit(first_difference(actual.decode(errors="replace"), expected.decode(errors="replace"),
                                                      f"variant {tag} {variant} scenario {number} {compiler}"))
            compared += len(expected.splitlines())
        print(f"quil: variant {tag} equal", flush=True)
    assert not unreached, f"no scenario reached {sorted(unreached)}"

    for number, (old_text, new_text, variant) in enumerate(mutations):
        assert translation.count(old_text) == 1, old_text
        mutant = work / f"quil-mutant-{number}.cesta"
        mutant.write_text(translation.replace(old_text, new_text))
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", "-DSEXOS_QUIL_CURRENT_KEYS=0", *(f"-D{n}" for n in variant), *include, "-c", mutant, "-o", work / "mutant.o"])
        run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include, work / "provider.c", work / "main.c", work / "mutant.o", *runtime,
             "-o", work / "mutant-harness"])
        if not any(run([work / "mutant-harness", *case]).stdout != references[variant, index] for index, case in enumerate(cases)):
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")
    # Current upstream control chords are intentionally newer than the pinned
    # Rust cohort. Exercise the actual modifier dispatcher independently.
    current_case = [1, 10, 0, 0, 0]
    current_outputs = []
    for compiler in ("gcc", "clang"):
        obj = work / f"quil-current-{compiler}.o"
        harness = work / f"current-{compiler}"
        run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", *warnings,
             "-DSEXOS_QUIL_CURRENT_KEYS_PROOF=1", "-DSEXOS_QUIL_CURRENT_DOCUMENT=0", *sanitizers, *include,
             "-c", HERE / "src/quil.cesta", "-o", obj])
        run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", "-Wno-unused-function",
             *sanitizers, *include, work / "provider.c", work / "main.c", obj,
             *runtime, "-o", harness])
        output = run([harness, *current_case]).stdout
        assert b"[quil.current.keys.proof.done] ok=1 len=2 cursor=2 control=0 goto=0 find=0" in output
        assert b"[quil.text.stats] bytes=20 lines=2 words=4 chars=20 cursor=20 ok=1" in output
        assert b"[quil.goto.line] line=2 old=20 new=8 ok=1 reason=found" in output
        assert b"[quil.find.nav] dir=prev old=19 new=13 count=2 ok=1" in output
        assert b"[quil.replace] start=14 old_bytes=5 new_bytes=1 applied=1" in output
        assert b"[quil.replace.all] count=1 applied=1 len=12" in output
        current_outputs.append(output)
    assert current_outputs[0] == current_outputs[1]
    current_mutations = [
        ("(if (== code 0x1D)", "(if (== code 0x1C)"),
        ("(= next (call word_right_at q old))", "(= next (call word_left_at q old))"),
        ("(call remove_range q start bytes)", "(call remove_range q start (- bytes 1))"),
        ("(call count_words q) (call count_chars q)", "(call count_words q) (call count_words q)"),
        ("(if (== scancode 0x22)", "(if (== scancode 0x23)"),
        ("(- (cast uint32_t code) 1)", "(- (cast uint32_t code) 2)"),
        ("(if (== scancode 0x21)", "(if (== scancode 0x20)"),
        ("(call replace_span q start (-> q sel_end)", "(call replace_span q (+ start 1) (-> q sel_end)"),
        ("(if event_shift (cast void (call find_prev q)) (cast void (call find_next q)))",
         "(if event_shift (cast void (call find_next q)) (cast void (call find_next q)))"),
    ]
    for number, (old_text, new_text) in enumerate(current_mutations):
        assert translation.count(old_text) == 1, old_text
        mutant = work / f"quil-current-mutant-{number}.cesta"
        mutant.write_text(translation.replace(old_text, new_text))
        obj = work / f"quil-current-mutant-{number}.o"
        harness = work / f"current-mutant-{number}"
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2",
             "-DSEXOS_QUIL_CURRENT_KEYS_PROOF=1", "-DSEXOS_QUIL_CURRENT_DOCUMENT=0", *include, "-c", mutant, "-o", obj])
        run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include,
             work / "provider.c", work / "main.c", obj, *runtime, "-o", harness])
        assert run([harness, *current_case]).stdout != current_outputs[0], ("current mutation survived", number)
    print(f"quil: Cesta translation equals the Rust server on {compared} trace lines ({args.cases} scenarios x {len(variants)} "
          f"build variants, GCC and Clang, address and UB sanitizers); {len(mutations)} frozen and "
          f"{len(current_mutations)} current-key mutations detected")


if __name__ == "__main__":
    main()
