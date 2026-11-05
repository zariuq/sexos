#!/usr/bin/env python3
"""Compare apps/spindle with its Cesta translation."""

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
    RUST_FFI,
    RUST_MAIN,
    first_difference,
    pinned_function,
    run,
)

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"

ENV_FLAGS = [
    "SEXOS_SPINDLE_ALIASES_PROOF",
    "SEXOS_SPINDLE_APPS_REGISTRY_PROOF",
    "SEXOS_SPINDLE_COMMAND_HISTORY_PROOF",
    "SEXOS_SPINDLE_PERSIST_HISTORY_PROOF",
    "SEXOS_SPINDLE_FILES_COMMANDS_PROOF",
    "SEXOS_SPINDLE_BELL_BRIDGE_PROOF",
    "SEXOS_SPINDLE_LINEN_BRIDGE_PROOF",
    "SEXOS_SPINDLE_STATUS_PANEL_PROOF",
    "SEXOS_SPINDLE_DAILY_SUMMARY_PROOF",
    "SEXOS_SPINDLE_HELP_POLISH_PROOF",
    "SEXOS_APP_LAUNCH_COMMANDS_PROOF",
    "SEXOS_SPINDLE_APP_LAUNCH_EXEC_PROOF",
    "SEXOS_APP_REGISTRY_STATIC_V2_PROOF",
    "SEXOS_SPINDLE_LINEN_WORKFLOW_PROOF",
    "SEXOS_SPINDLE_QUIL_WORKFLOW_PROOF",
    "SEXOS_SPINDLE_EDITOR_V2_PROOF",
    "SEXOS_SPINDLE_APP_LIFECYCLE_PROOF",
    "SEXOS_SPINDLE_LIFECYCLE_HELP_V2_PROOF",
    "SEXOS_SPINDLE_EDITOR_STATUS_PROOF",
    "SEXOS_SPINDLE_SEARCH_HELP_PROOF",
    "SEXOS_SPINDLE_WINDOW_WORKFLOW_PROOF",
    "SEXOS_SPINDLE_BROWSER_STUB_PROOF",
    "SEXOS_SPINDLE_FRAME_CHROME_PROOF",
    "SEXOS_SPINDLE_FRAME_RIM_PROOF",
    "SEXOS_SPINDLE_FRAME_LIGHTS_PROOF",
    "SEXOS_SPINDLE_ATLAS_PROOF",
    "SEXOS_SPINDLE_EDITOR_QUALITY_PROOF",
    "SEXOS_SPINDLE_EDITOR_POLISH_PROOF",
    "SEXOS_SPINDLE_EDITOR_V3_PROOF",
    "SEXOS_SPINDLE_EDITOR_FINISH_PROOF",
]

DEFAULT_VARIANTS = [
    (),
    (
        "SEXOS_SPINDLE_COMMAND_HISTORY_PROOF",
        "SEXOS_SPINDLE_PERSIST_HISTORY_PROOF",
        "SEXOS_SPINDLE_FILES_COMMANDS_PROOF",
    ),
    (
        "SEXOS_SPINDLE_BELL_BRIDGE_PROOF",
        "SEXOS_SPINDLE_LINEN_BRIDGE_PROOF",
        "SEXOS_SPINDLE_STATUS_PANEL_PROOF",
        "SEXOS_SPINDLE_DAILY_SUMMARY_PROOF",
        "SEXOS_SPINDLE_HELP_POLISH_PROOF",
    ),
    (
        "SEXOS_SPINDLE_ALIASES_PROOF",
        "SEXOS_SPINDLE_APPS_REGISTRY_PROOF",
        "SEXOS_APP_LAUNCH_COMMANDS_PROOF",
        "SEXOS_SPINDLE_APP_LAUNCH_EXEC_PROOF",
        "SEXOS_APP_REGISTRY_STATIC_V2_PROOF",
    ),
    (
        "SEXOS_SPINDLE_LINEN_WORKFLOW_PROOF",
        "SEXOS_SPINDLE_QUIL_WORKFLOW_PROOF",
        "SEXOS_SPINDLE_EDITOR_V2_PROOF",
        "SEXOS_SPINDLE_APP_LIFECYCLE_PROOF",
        "SEXOS_SPINDLE_LIFECYCLE_HELP_V2_PROOF",
        "SEXOS_SPINDLE_EDITOR_STATUS_PROOF",
    ),
    (
        "SEXOS_SPINDLE_SEARCH_HELP_PROOF",
        "SEXOS_SPINDLE_WINDOW_WORKFLOW_PROOF",
        "SEXOS_SPINDLE_BROWSER_STUB_PROOF",
        "SEXOS_SPINDLE_FRAME_CHROME_PROOF",
        "SEXOS_SPINDLE_FRAME_RIM_PROOF",
        "SEXOS_SPINDLE_FRAME_LIGHTS_PROOF",
        "SEXOS_SPINDLE_ATLAS_PROOF",
        "SEXOS_SPINDLE_EDITOR_QUALITY_PROOF",
        "SEXOS_SPINDLE_EDITOR_POLISH_PROOF",
        "SEXOS_SPINDLE_EDITOR_V3_PROOF",
        "SEXOS_SPINDLE_EDITOR_FINISH_PROOF",
    ),
    tuple(ENV_FLAGS),
]

ALL_VARIANTS = DEFAULT_VARIANTS + [(flag,) for flag in ENV_FLAGS]

PROVIDER = PROVIDER_PRELUDE + r'''
#define OBJECTS 15
#define CAPACITY 512
static struct { bool used; uint8_t name[16]; uint8_t data[CAPACITY]; size_t size; } objects[OBJECTS];
static int selected = -1;
static uint8_t queue_code[256], queue_val[256];
static size_t queue_len, queue_pos;
static size_t script_step;
static uint64_t settle_until;
static bool cmd_needs_settle;

static uint8_t ascii_to_scancode(char c) {
    switch (c) {
    case '1': return 0x02; case '2': return 0x03; case '3': return 0x04;
    case '4': return 0x05; case '5': return 0x06; case '6': return 0x07;
    case '7': return 0x08; case '8': return 0x09; case '9': return 0x0A;
    case '0': return 0x0B;
    case 'q': return 0x10; case 'w': return 0x11; case 'e': return 0x12;
    case 'r': return 0x13; case 't': return 0x14; case 'y': return 0x15;
    case 'u': return 0x16; case 'i': return 0x17; case 'o': return 0x18;
    case 'p': return 0x19;
    case 'a': return 0x1E; case 's': return 0x1F; case 'd': return 0x20;
    case 'f': return 0x21; case 'g': return 0x22; case 'h': return 0x23;
    case 'j': return 0x24; case 'k': return 0x25; case 'l': return 0x26;
    case 'z': return 0x2C; case 'x': return 0x2D; case 'c': return 0x2E;
    case 'v': return 0x2F; case 'b': return 0x30; case 'n': return 0x31;
    case 'm': return 0x32;
    case ' ': return 0x39; case '-': return 0x0C; case '=': return 0x0D;
    case '[': return 0x1A; case ']': return 0x1B; case ';': return 0x27;
    case '\'': return 0x28; case '`': return 0x29; case '\\': return 0x2B;
    case ',': return 0x33; case '.': return 0x34; case '/': return 0x35;
    default: return 0x39;
    }
}

static void enqueue_key(uint8_t sc, uint8_t val) {
    if (queue_len < sizeof queue_code) {
        queue_code[queue_len] = sc;
        queue_val[queue_len] = val;
        queue_len++;
    }
}

static bool is_settle_cmd(const char *s) {
    return strcmp(s, "disk") == 0 ||
           strcmp(s, "mkdoc notes") == 0 ||
           strcmp(s, "filldoc 2 24") == 0 ||
           strcmp(s, "catdoc 2") == 0 ||
           strcmp(s, "truncdoc 2 12") == 0 ||
           strcmp(s, "filldocx 2 36") == 0 ||
           strcmp(s, "catdocx 2") == 0 ||
           strcmp(s, "mvdoc 2 draft") == 0 ||
           strncmp(s, "rmdoc ", 6) == 0;
}

static void enqueue_cmd(const char *s) {
    queue_len = queue_pos = 0;
    cmd_needs_settle = is_settle_cmd(s);
    for (size_t i = 0; s[i]; ++i) enqueue_key(ascii_to_scancode(s[i]), 1);
    enqueue_key(0x1C, 1);
}

static uint64_t v2(uint64_t status, uint64_t field, uint64_t payload) {
    return (status << 56) | (field << 48) | payload;
}

static uint64_t storage_reply(void) {
    uint64_t a0 = call.a0, a1 = call.a1, a2 = call.a2;
    switch (call.opcode) {
    case 0x30:
    case 0x32:
    case 0x33:
    case 0x34:
        return fault() ? error() : 0;
    case 0x3E: /* DISKFS_SELECT */
        if (fault()) return error();
        if (a0 >= OBJECTS || !objects[a0].used) return (uint64_t)(int64_t)-3;
        selected = (int)a0;
        return 0;
    case 0x3B: /* DISKFS_STAT */
        if (fault() || selected < 0 || !objects[selected].used) return (uint64_t)(int64_t)-3;
        return (uint64_t)objects[selected].size;
    case 0x42: { /* DISKFS_CREATE */
        if (fault()) return (uint64_t)(int64_t)PICK(-8, -5, -1);
        uint8_t name[16];
        for (int i = 0; i < 8; ++i) {
            name[i] = (uint8_t)(a1 >> (8 * i));
            name[8 + i] = (uint8_t)(a2 >> (8 * i));
        }
        for (int i = 0; i < OBJECTS; ++i)
            if (objects[i].used && memcmp(objects[i].name, name, 16) == 0)
                return (uint64_t)(int64_t)-8;
        for (int i = 2; i < OBJECTS; ++i) {
            if (!objects[i].used) {
                objects[i].used = true;
                memcpy(objects[i].name, name, 16);
                objects[i].size = 0;
                memset(objects[i].data, 0, CAPACITY);
                return (uint64_t)i;
            }
        }
        return (uint64_t)(int64_t)-5;
    }
    case 0x43: { /* DISKFS_LIST */
        if (fault()) return (uint64_t)(int64_t)-3;
        if (a0 >= OBJECTS) return 0;
        if (a1 == 0xFF)
            return objects[a0].used ? ((1ull << 62) | (uint64_t)objects[a0].size) : 0;
        if (a1 >= 2) return 0;
        uint64_t w = 0;
        for (int i = 0; i < 8; ++i)
            w |= (uint64_t)objects[a0].name[a1 * 8 + i] << (8 * i);
        return w;
    }
    case 0x47: /* DISKFS_DELETE */
        if (fault()) return (uint64_t)(int64_t)PICK(-6, -3, -1);
        if (a0 < 2) return (uint64_t)(int64_t)-6;
        if (a0 >= OBJECTS || !objects[a0].used) return (uint64_t)(int64_t)-3;
        objects[a0].used = false;
        return 0;
    case 0x48: { /* DISKFS_RENAME */
        if (fault()) return (uint64_t)(int64_t)PICK(-8, -6, -3, -1);
        if (a0 < 2) return (uint64_t)(int64_t)-6;
        if (a0 >= OBJECTS || !objects[a0].used) return (uint64_t)(int64_t)-3;
        uint8_t name[16];
        for (int i = 0; i < 8; ++i) {
            name[i] = (uint8_t)(a1 >> (8 * i));
            name[8 + i] = (uint8_t)(a2 >> (8 * i));
        }
        for (int i = 0; i < OBJECTS; ++i)
            if (i != (int)a0 && objects[i].used && memcmp(objects[i].name, name, 16) == 0)
                return (uint64_t)(int64_t)-8;
        memcpy(objects[a0].name, name, 16);
        return 0;
    }
    case 0x38: /* DISKFS_WRITE */
        if (fault() || selected < 0 || !objects[selected].used || a0 + 16 > CAPACITY)
            return (uint64_t)(int64_t)-4;
        for (int i = 0; i < 8; ++i) {
            objects[selected].data[a0 + i] = (uint8_t)(a1 >> (8 * i));
            objects[selected].data[a0 + 8 + i] = (uint8_t)(a2 >> (8 * i));
        }
        if (a0 + 16 > objects[selected].size) objects[selected].size = (size_t)(a0 + 16);
        return 16;
    case 0x49: /* DISKFS_TRUNCATE */
        if (fault() || selected < 0 || !objects[selected].used || a0 > CAPACITY)
            return (uint64_t)(int64_t)-4;
        objects[selected].size = (size_t)a0;
        return a0;
    case 0x39: { /* DISKFS_READ */
        if (fault() || selected < 0 || !objects[selected].used)
            return (uint64_t)(int64_t)-4;
        uint64_t w = 0;
        for (uint64_t i = 0; i < 8 && a0 + i < objects[selected].size; ++i)
            w |= (uint64_t)objects[selected].data[a0 + i] << (8 * i);
        if (chance(10)) w ^= 0x04;
        return w;
    }
    case 0x4A: { /* DISKFS_READ_V2 */
        if (fault() || selected < 0 || !objects[selected].used)
            return v2(0x80, 4, 0);
        size_t size = objects[selected].size;
        if (a0 >= size) return v2(1, 0, 0);
        size_t n = size - (size_t)a0;
        if (n > 6) n = 6;
        if (n > a1) n = (size_t)a1;
        uint64_t payload = 0;
        for (size_t i = 0; i < n; ++i)
            payload |= (uint64_t)objects[selected].data[a0 + i] << (8 * i);
        if (chance(10)) payload ^= 0x02;
        if (chance(8) && n > 1) n -= 1;
        if (chance(8)) return v2(1, 0, 0);
        return v2(0, n, payload);
    }
    default:
        return 0;
    }
}

static const char *const SCRIPT_CMDS[] = {
#ifdef SEXOS_SPINDLE_CALC_CURRENT_GATE
    "calc x = 2",
    "calc x",
    "calc ans",
    "calc 255 in hex",
    "calc missing",
#endif
#ifdef SEXOS_SPINDLE_CALC_GATE
    "calc 0.1",
    "calc ans-0.05",
    "calc 1/3",
    "calc ans",
    "calc 1/0",
    "sextime 0",
    "sextime 1356048000",
    "sextime nope",
#endif
    "help",
    "mkdoc notes",
    "mkdoc notes",
    "mkdoc",
    "disk",
    "filldoc 2 24",
    "catdoc 2",
    "truncdoc 2 12",
    "catdoc 2",
    "filldocx 2 36",
    "catdocx 2",
    "mvdoc 2 draft",
    "mvdoc 2",
    "mvdoc",
    "rmdoc 0",
    "rmdoc 2",
    "rmdoc 9",
    "rmdoc",
    "winreset",
    "notify test msg",
    "notify",
    "bell-test",
    "bell-status",
    "linen-status",
    "linen-list",
    "linen-open 2",
    "linen-open 0",
    "linen-open",
    "linen-search work",
    "linen-search",
    "launch quil",
    "launch webstub",
    "launch unknown",
    "launch",
    "app-info spindle",
    "app-info quil",
    "app-info linen",
    "app-info bell",
    "app-info atlas",
    "app-info collar",
    "app-info mesh",
    "app-info unknown",
    "app-info",
    "app-status",
    "d",
    "b",
    "k",
    "a",
    "q",
    "n alias-notify",
    "echo hello world",
    "echo",
    "status",
    "pd",
    "servers",
    "bell",
    "files",
    "apps",
    "blockers",
    "keys",
    "proof boot",
    "proof input",
    "proof display",
    "proof storage",
    "proof other",
    "about",
    "route",
    "input",
    "close",
    "faults",
    "save",
    "load",
    "ls",
    "filldoc",
    "filldoc 2",
    "truncdoc",
    "truncdoc 2",
    "catdoc",
    "filldocx",
    "filldocx 2",
    "catdocx",
    "launch-outcomes",
    "bell-launch-events",
    "project-scene-link",
    "project-scene-status",
    "mesh-graph",
    "mesh-graph-status",
    "collar-grants",
    "collar-status",
    "authority-status",
    "session",
    "daily",
    "events",
    "events clear",
    "events",
    "history",
    "history clear",
    "history",
    "object-new",
    "object-tag",
    "object-search",
    "editor keys",
    "editor search",
    "editor selection",
    "editor save",
    "editor undo",
    "editor",
    "quil",
    "edit",
    "edit-help",
    "edit-status",
    "lifecycle",
    "app-state",
    "browser",
    "web",
    "browser-status",
    "url sexos.org",
    "url",
    "browser-localdoc",
    "browser-localdoc-status",
    "browser-surface",
    "browser-roadmap",
    "url-status",
    "atlas",
    "frame-chrome",
    "frame-lights",
    "frame-lights-keys",
    "frame-rim",
    "scene-status",
    "scene-lifecycle",
    "scene-lifecycle-status",
    "scene-keys",
    "scene-switch-status",
    "windows",
    "focus-help",
    "window-keys",
    "search",
    "unknowncmd",
    "clear",
};

static void set_msg(SexosMessage *m, uint64_t type_id, uint32_t caller_pd, uint64_t a0, uint64_t a1, uint64_t a2) {
    m->type_id = type_id;
    m->caller_pd = caller_pd;
    m->arg0 = a0;
    m->arg1 = a1;
    m->arg2 = a2;
}

static void request(SexosMessage *m) {
    if (call.pending) {
        if (chance(10)) {
            if (chance(50)) {
                set_msg(m, 0x202, 7, 0x1E, 1, 0);
                return;
            }
            set_msg(m, 0x999, 7, 1, 2, 3);
            return;
        }
        return;
    }
    if (yields < settle_until) {
        return;
    }
    if (queue_pos < queue_len) {
        uint8_t sc = queue_code[queue_pos];
        uint8_t val = queue_val[queue_pos];
        queue_pos++;
        if (sc == 0x1C && val == 1 && cmd_needs_settle) {
            settle_until = yields + 8000;
            cmd_needs_settle = false;
        }
        set_msg(m, 0x202, 3, sc, val, 0);
        return;
    }
    if (script_step == 1) {
        script_step++;
        /* Exercise Vi mode, history nav, ghost accept, paging, keyup right after "help". */
        queue_len = queue_pos = 0;
        enqueue_key(0x23, 1); /* h */
        enqueue_key(0x12, 1); /* e */
        enqueue_key(0x4D, 1); /* Right: ghost accept "help" */
        enqueue_key(0x0E, 1); /* Backspace */
        enqueue_key(0x0F, 1); /* Tab: ghost accept */
        enqueue_key(0x48, 1); /* Up: history nav */
        enqueue_key(0x50, 1); /* Down: history nav */
        enqueue_key(0x49, 1); /* PageUp */
        enqueue_key(0x51, 1); /* PageDown */
        enqueue_key(0x10, 1); /* q */
        enqueue_key(0x39, 1); /* space */
        enqueue_key(0x11, 1); /* w */
        enqueue_key(0x12, 1); /* e */
        enqueue_key(0x01, 1); /* Esc -> normal mode */
        enqueue_key(0x01, 1); /* Esc -> normal already */
        enqueue_key(0x23, 1); /* h: left */
        enqueue_key(0x0E, 1); /* Backspace in normal: left */
        enqueue_key(0x26, 1); /* l: right */
        enqueue_key(0x0B, 1); /* 0: home */
        enqueue_key(0x11, 1); /* w: word_fwd */
        enqueue_key(0x30, 1); /* b: word_back */
        enqueue_key(0x12, 1); /* e: word_end */
        enqueue_key(0x20, 1); /* d */
        enqueue_key(0x20, 1); /* d -> dd */
        enqueue_key(0x16, 1); /* u -> undo */
        enqueue_key(0x1E, 1); /* a -> append insert */
        enqueue_key(0x01, 1); /* Esc -> normal */
        enqueue_key(0x17, 1); /* i -> insert */
        enqueue_key(0x4D, 1); /* Right: no ghost */
        enqueue_key(0x0F, 1); /* Tab: no ghost */
        enqueue_key(0x01, 1); /* Esc -> normal */
        enqueue_key(0x1C, 1); /* Enter in normal mode */
        enqueue_key(0x1E, 0); /* keyup ignored */
        uint8_t sc = queue_code[queue_pos];
        uint8_t val = queue_val[queue_pos];
        queue_pos++;
        set_msg(m, 0x202, 3, sc, val, 0);
        return;
    }
    size_t ncmds = sizeof(SCRIPT_CMDS) / sizeof(SCRIPT_CMDS[0]);
    size_t cmd_idx = script_step == 0 ? 0 : script_step - 1;
    if (cmd_idx < ncmds) {
        script_step++;
        enqueue_cmd(SCRIPT_CMDS[cmd_idx]);
        uint8_t sc = queue_code[queue_pos];
        uint8_t val = queue_val[queue_pos];
        queue_pos++;
        if (sc == 0x1C && val == 1 && cmd_needs_settle) {
            settle_until = yields + 8000;
            cmd_needs_settle = false;
        }
        set_msg(m, 0x202, 3, sc, val, 0);
        return;
    }
    uint64_t r = next() % 10;
    if (r < 7) {
        static const uint8_t keys[] = {
            0x1E, 0x30, 0x2E, 0x20, 0x12, 0x23, 0x17, 0x26, 0x39,
            0x0E, 0x01, 0x1C, 0x48, 0x50, 0x49, 0x51, 0x4D, 0x0F
        };
        uint8_t sc = keys[next() % sizeof(keys)];
        set_msg(m, 0x202, 3, sc, chance(90) ? 1 : 0, 0);
        return;
    }
    set_msg(m, 0x300, 3, 0, 0, 0);
}

static uint64_t reply_share(void) { return scenario.mix == 2 ? 95 : 80; }
static bool answered(uint64_t opcode) { return opcode >= 0x38; }

static void server_start(void) {
    memset(objects, 0, sizeof(objects));
    selected = -1;
    queue_len = queue_pos = 0;
    script_step = 0;
    settle_until = 0;
    cmd_needs_settle = false;
    objects[0].used = objects[1].used = true;
    memcpy(objects[0].name, "sexfiles-proof-v", 16);
    memcpy(objects[1].name, "linen-object-v1", 15);
    objects[0].size = objects[1].size = 64;
}
'''


def pinned_line(source, prefix):
    lines = [line for line in source.splitlines() if line.startswith(prefix)]
    assert len(lines) == 1, prefix
    return "    " + lines[0] + "\n"


def rust_reference(source):
    source = re.sub(r"^//!", "//", source, flags=re.M)
    for name in ("memcpy", "memset", "memmove", "memcmp"):
        source, count = re.subn(
            r'#\[no_mangle\]\npub unsafe extern "C" fn %s\(.*?\n}\n' % name,
            "",
            source,
            flags=re.S,
        )
        assert count == 1, name
    source, count = re.subn(
        r"#\[alloc_error_handler\]\nfn alloc_error_handler\(.*?\n}\n",
        "",
        source,
        flags=re.S,
    )
    assert count == 1
    for old, new in (
        ("#![no_std]\n", ""),
        ("#![no_main]\n", ""),
        ("#![feature(alloc_error_handler)]\n", ""),
        ("#[global_allocator]\nstatic ALLOCATOR: DummyAllocator = DummyAllocator;\n", ""),
        ("#[panic_handler]\nfn panic(_info: &core::panic::PanicInfo) -> ! {\n    loop {}\n}\n", ""),
        ('#[no_mangle]\npub extern "C" fn _start() -> ! {', "fn spindle_start() -> ! {"),
        ("Limitations: no shift, no visual cursor indicator.", "Limitations: no visual cursor indicator."),
        ("[spindle.help.section] name=basics commands=6", "[spindle.help.section] name=basics commands=8"),
        ('            sb.push(b"  echo <msg>   print message");',
         '            sb.push(b"  echo <msg>   print message");\n            sb.push(b"  sextime <n>  Unix seconds as SexTime/UTC/Long Count");\n            sb.push(b"  calc <expr>  exact decimal + - * / % ( ), ans");'),
        ('            serial_println!("[spindle.help.command] name=echo ok=1");',
         '            serial_println!("[spindle.help.command] name=echo ok=1");\n            serial_println!("[spindle.help.command] name=sextime ok=1");\n            serial_println!("[spindle.help.command] name=calc ok=1");'),
    ):
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    return source


def scenarios(count):
    r = random.Random(0x5A1D)
    return [
        [
            r.getrandbits(64),
            [106000, 18000, 4000, 2000, 1000, 800][k % 6],
            r.choice([60, 80, 95]),
            k % 3,
            (k // 3) % 3,
        ]
        for k in range(count)
    ]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=6)
    parser.add_argument("--all-variants", action="store_true")
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    variants = ALL_VARIANTS if args.all_variants else DEFAULT_VARIANTS
    work = HERE / "build/spindle-differential"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    main_rs = show("apps/spindle/src/main.rs")
    translation = LayoutText((HERE / "src/spindle.cesta").read_text())

    rust_gates = set(re.findall(r'option_env!\("(\w+)"\)', main_rs)) | set(
        re.findall(r"cfg!\((\w+)\)", main_rs)
    )
    c_gates = set(re.findall(r'cpp-if "defined\((\w+)\)"', translation))
    current_selectors = {"SEXOS_SPINDLE_CURRENT"}
    assert current_selectors <= c_gates
    c_gates -= current_selectors
    assert rust_gates == c_gates == set(ENV_FLAGS), (
        rust_gates ^ c_gates,
        c_gates ^ set(ENV_FLAGS),
    )

    pdx = show("crates/sex-pdx/src/lib.rs")
    pinned = [
        pinned_function(pdx, "pub fn pdx_listen_raw(slot: u64) -> PdxMessage {", LISTEN),
        pinned_function(
            pdx, "pub fn pdx_try_listen_raw(slot: u64) -> Option<PdxMessage> {", LISTEN
        ),
        pinned_function(
            pdx, "pub fn diskfs_v2_payload(reply: u64) -> [u8; DISKFS_V2_MAX_READ] {", {}
        ),
        "    pub type Color = u32;\n",
        "    #[repr(C)] #[derive(Clone, Copy)] pub struct Rect { pub x: u32, pub y: u32, pub width: u32, pub height: u32 }\n",
        *(
            pinned_line(pdx, prefix)
            for prefix in (
                "pub const OP_WINDOW_CREATE",
                "pub const SLOT_BELL",
                "pub const SLOT_LINEN",
                "pub const SLOT_SHELL",
                "pub const OP_BELL_NOTIFY",
                "pub const DISKFS_V2_STATUS_OK",
                "pub const DISKFS_V2_STATUS_EOF",
                "pub const DISKFS_V2_MAX_READ",
                "pub fn diskfs_v2_status",
                "pub fn diskfs_v2_len_field",
            )
        ),
    ]
    sex_graphics_stub = """
mod sex_graphics {
    use super::sex_pdx::{Color, Rect};
    pub struct WindowBuffer;
    impl WindowBuffer {
        pub fn draw_rect(&mut self, _r: Rect, _c: Color) {}
    }
    pub mod font {
        use super::{Color, WindowBuffer};
        pub fn draw_str(_fb: &mut WindowBuffer, _x: u32, _y: u32, _s: &[u8], _fg: Color, _bg: Option<Color>) {}
        pub fn draw_char(_fb: &mut WindowBuffer, _x: u32, _y: u32, _c: u8, _fg: Color, _bg: Option<Color>) {}
        pub const fn str_width(s: &[u8]) -> u32 { (s.len() as u32) * 8 }
    }
}
"""
    rust = work / "reference.rs"
    rust.write_text(
        RUST_FFI
        + "".join(pinned)
        + "}\n"
        + sex_graphics_stub
        + rust_reference(main_rs)
        + RUST_MAIN % "spindle_start"
    )
    (work / "provider.c").write_text(PROVIDER)
    (work / "main.c").write_text(C_MAIN % "spindle_main")

    include = ["-I", HERE / "generated/include"]
    runtime = [
        HERE / "runtime/client_runtime.c",
        HERE / "runtime/server_runtime.c",
        *sorted((HERE / "generated/runtime").glob("*.c")),
    ]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]
    plain_provider = work / "provider-plain.o"
    run(
        [
            "gcc",
            "-std=c11",
            "-O2",
            "-DSEXOS_HOST",
            *warnings,
            *include,
            "-c",
            work / "provider.c",
            "-o",
            plain_provider,
        ]
    )

    mutations = [
        ('(define SURF_COLS "((size_t)20)")', '(define SURF_COLS "((size_t)19)")', ()),
        ("(= (-> sp frame_budget) 8u)", "(= (-> sp frame_budget) 7u)", ()),
        ("(= (-> sp raw_budget) 32u)", "(= (-> sp raw_budget) 31u)", ()),
        ("(= (-> sp key_recv_budget) 64u)", "(= (-> sp key_recv_budget) 63u)", ()),
        ("(= (-> sp echo_budget) 16u)", "(= (-> sp echo_budget) 15u)", ()),
        ("(var uint32_t count (call min_u32 (-> hist total) 128u))", "(var uint32_t count (call min_u32 (-> hist total) 64u))", ()),
        ("(call sexos_pdx_call SEXOS_SLOT_display 0xFAu sid 0 0)", "(call sexos_pdx_call SEXOS_SLOT_display 0xFAu sid 1 0)", ()),
        ('(= ([] grid prow) 62u)', '(= ([] grid prow) 63u)', ()),
        ('(if (call slice_eq raw_cmd "d")', '(if (call slice_eq raw_cmd "D")', ()),
        ('(call sexos_pdx_call SEXOS_SLOT_shell 0x15u (cast uint64_t app_id) 0 0)', '(call sexos_pdx_call SEXOS_SLOT_shell 0x16u (cast uint64_t app_id) 0 0)', ()),
        ('(var uint8_t expect (cast uint8_t (& (+ (* idx 37u) 11u) 0x7Fu)))', '(var uint8_t expect (cast uint8_t (& (+ (* idx 37u) 13u) 0x7Fu)))', ()),
        ('(var uint8_t expect (cast uint8_t (& (+ (* idx 37u) 11u) 0xFFu)))', '(var uint8_t expect (cast uint8_t (& (+ (* idx 37u) 13u) 0xFFu)))', ()),
        ('(case 0x02u (block (= (* ch_out) 49u) (return true)))', '(case 0x02u (block (= (* ch_out) 50u) (return true)))', ()),
        ('(var uint32_t output_bytes (call sat_mul_u32 output_lines 84u))', '(var uint32_t output_bytes (call sat_mul_u32 output_lines 80u))', ()),
        ('(call sexos_pdx_call SEXOS_SLOT_linen 0x47u a0 a1 0)', '(call sexos_pdx_call SEXOS_SLOT_linen 0x47u a0 a1 1)', ()),
    ]
    mutation_variants = {variant for _, _, variant in mutations}
    assert mutation_variants <= set(variants)

    markers = [
        rb"\[spindle\.init\.start\]",
        rb"\[spindle\.ready\]",
        rb"\[spindle\.grid\.render\.ok\]",
        rb"\[spindle\.help\.section\] name=basics",
        rb"\[spindle\.status\.panel\] command=status",
        rb"\[spindle\.launch\.request\] app=quil",
        rb"\[spindle\.app\.command\] name=app-info ok=1",
        rb"\[spindle\.mkdoc\] id=2 ok=1",
        rb"\[spindle\.filldoc\] id=2 bytes=24 ok=1",
        rb"\[spindle\.catdoc\] id=2 size=24",
        rb"\[spindle\.truncdoc\] id=2 new_len=12 ok=1",
        rb"\[spindle\.filldocx\] id=2 bytes=36 ok=1",
        rb"\[spindle\.catdocx\] id=2 size=36",
        rb"\[spindle\.mvdoc\] id=2 ok=1",
        rb"\[spindle\.rmdoc\] id=2 ok=1",
        rb"\[spindle\.winreset\] ok=1",
        rb"\[spindle\.bell\.send\] command=notify",
        rb"\[spindle\.linen\.send\] op=open id=2",
        rb"\[spindle\.linen\.search\.send\] token=work",
        rb"\[spindle\.alias\.exec\] alias=d target=daily ok=1",
        rb"\[spindle\.vi\.mode\] mode=normal",
        rb"\[spindle\.line\.edit\.ok\] op=dd",
        rb"\[spindle\.line\.edit\.ok\] op=undo",
        rb"\[spindle\.ghost\.accept\]",
        rb"\[spindle\.page\.nav\] dir=up",
        rb"\[spindle\.command\.history\.proof\.done\] ok=1",
        rb"\[spindle\.persist\.proof\.done\] ok=1",
        rb"\[spindle\.files\.proof\.done\] ok=1",
        rb"\[spindle\.bell\.proof\.done\] ok=1",
        rb"\[spindle\.linen\.proof\.done\] ok=1",
        rb"\[spindle\.status\.proof\.done\] ok=1",
        rb"\[spindle\.daily\.proof\.done\] ok=1",
        rb"\[spindle\.help\.proof\.done\] ok=1",
    ]
    unreached = set(markers)
    cases = scenarios(args.cases)
    compared = 0
    references = {}

    for variant in variants:
        tag = f"v{variants.index(variant)}"
        env = {name: value for name, value in os.environ.items() if name not in ENV_FLAGS}
        for name in variant:
            env[name] = "1"
        reference = work / f"reference-{tag}"
        run(
            [
                "rustc",
                "--edition",
                "2021",
                "-O",
                "-A",
                "warnings",
                rust,
                "-o",
                reference,
                "-C",
                f"link-arg={plain_provider}",
            ],
            env=env,
        )
        harnesses = []
        for compiler in ("gcc", "clang"):
            obj = work / f"spindle-{compiler}-{tag}.o"
            calc_obj = work / f"spindle-calc-{compiler}-{tag}.o"
            sextime_obj = work / f"spindle-sextime-{compiler}-{tag}.o"
            run(
                [
                    cesta,
                    "cc",
                    "--cc",
                    compiler,
                    "-std=c11",
                    "-O2",
                    *warnings,
                    *(f"-D{n}" for n in variant),
                    *sanitizers,
                    *include,
                    "-c",
                    HERE / "src/spindle.cesta",
                    "-o",
                    obj,
                ]
            )
            run(
                [
                    cesta,"cc","--cc",compiler,"-std=c11","-O2",*warnings,
                    *sanitizers,*include,"-c",HERE/"src/spindle_sextime.cesta",
                    "-o",sextime_obj,
                ]
            )
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
                    "-c",
                    HERE / "src/spindle_calc.cesta",
                    "-o",
                    calc_obj,
                ]
            )
            harness = work / f"harness-{compiler}-{tag}"
            run(
                [
                    compiler,
                    "-std=c11",
                    "-O2",
                    "-DSEXOS_HOST",
                    *sanitizers,
                    *include,
                    work / "provider.c",
                    work / "main.c",
                    obj,
                    calc_obj,
                    sextime_obj,
                    *runtime,
                    "-o",
                    harness,
                ]
            )
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
                    raise SystemExit(
                        first_difference(
                            actual.decode(errors="replace"),
                            expected.decode(errors="replace"),
                            f"variant {tag} {variant} scenario {number} {compiler}",
                        )
                    )
            compared += len(expected.splitlines())
        print(f"spindle: variant {tag} equal", flush=True)
    assert not unreached, f"no scenario reached {sorted(unreached)}"

    calc_mutant_obj=work/"spindle-calc-mutant.o"
    run([
        cesta,"cc","--cc","gcc","-std=c11","-O2",*include,"-c",
        HERE/"src/spindle_calc.cesta","-o",calc_mutant_obj,
    ])
    sextime_mutant_obj=work/"spindle-sextime-mutant.o"
    run([
        cesta,"cc","--cc","gcc","-std=c11","-O2",*include,"-c",
        HERE/"src/spindle_sextime.cesta","-o",sextime_mutant_obj,
    ])
    complete_obj=work/"spindle-complete.o"
    run([
        cesta,"cc","--cc","gcc","-std=c11","-O2",*include,"-c",
        HERE/"src/spindle_complete.cesta","-o",complete_obj,
    ])
    for number, (old_text, new_text, variant) in enumerate(mutations):
        assert translation.count(old_text) == 1, old_text
        mutant = work / f"spindle-mutant-{number}.cesta"
        mutant.write_text(translation.replace(old_text, new_text))
        run(
            [
                cesta,
                "cc",
                "--cc",
                "gcc",
                "-std=c11",
                "-O2",
                *(f"-D{n}" for n in variant),
                *include,
                "-c",
                mutant,
                "-o",
                work / "mutant.o",
            ]
        )
        run(
            [
                "gcc",
                "-std=c11",
                "-O2",
                "-DSEXOS_HOST",
                *include,
                work / "provider.c",
                work / "main.c",
                work / "mutant.o",
                calc_mutant_obj,
                sextime_mutant_obj,
                *runtime,
                "-o",
                work / "mutant-harness",
            ]
        )
        if not any(
            run([work / "mutant-harness", *case]).stdout != references[variant, index]
            for index, case in enumerate(cases)
        ):
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")

    calc_spindle_obj=work/"spindle-calc-gate.o"
    run([
        cesta,"cc","--cc","gcc","-std=c11","-O2",*include,"-c",
        HERE/"src/spindle.cesta","-o",calc_spindle_obj,
    ])
    calc_harness=work/"calc-integration-harness"
    run([
        "gcc","-std=c11","-O2","-DSEXOS_HOST","-DSEXOS_SPINDLE_CALC_GATE",
        *include,work/"provider.c",work/"main.c",calc_spindle_obj,calc_mutant_obj,sextime_mutant_obj,
        *runtime,"-o",calc_harness,
    ])
    calc_trace=run([calc_harness,*cases[0]]).stdout
    assert b"[spindle.calc] expr_bytes=3 result== 0.1 ok=1" in calc_trace
    assert b"[spindle.calc] expr_bytes=8 result== 0.05 ok=1" in calc_trace
    assert b"[spindle.calc] expr_bytes=3 result== 0.333333333 ok=1" in calc_trace
    assert b"[spindle.calc] expr_bytes=3 result=~ 0.333333333 ok=1" in calc_trace
    assert b"[spindle.calc] expr_bytes=3 error=DivideByZero ok=0" in calc_trace
    assert b"[spindle.sextime] SexTime -1356048000 s | UTC 1970-01-01T00:00:00Z | Long Count 12.17.16.7.5 (GMT 584283) ok=1" in calc_trace
    assert b"[spindle.sextime] SexTime 0 s | UTC 2012-12-21T00:00:00Z | Long Count 13.0.0.0.0 (GMT 584283) ok=1" in calc_trace
    assert b"[spindle.sextime] ok=0 reason=usage" in calc_trace
    current_spindle_obj=work/"spindle-calc-current-gate.o"
    run([
        cesta,"cc","--cc","gcc","-std=c11","-O2","-DSEXOS_SPINDLE_CURRENT=1",*include,"-c",
        HERE/"src/spindle.cesta","-o",current_spindle_obj,
    ])
    current_harness=work/"calc-current-integration-harness"
    run([
        "gcc","-std=c11","-O2","-DSEXOS_HOST","-DSEXOS_SPINDLE_CALC_CURRENT_GATE",
        *include,work/"provider.c",work/"main.c",current_spindle_obj,calc_mutant_obj,
        complete_obj,sextime_mutant_obj,*runtime,"-o",current_harness,
    ])
    current_trace=run([current_harness,*cases[0]]).stdout
    assert b"[spindle.calc] expr_bytes=5 result== 2 ok=1" in current_trace
    assert b"[spindle.calc] expr_bytes=1 result== 2 ok=1" in current_trace
    assert b"[spindle.calc] expr_bytes=3 result== 2 ok=1" in current_trace
    assert b"[spindle.calc] expr_bytes=10 result== 0xff ok=1" in current_trace
    assert b"[spindle.calc] expr_bytes=7 error=Unknown ok=0" in current_trace
    assert b"[spindle.complete] matches=5 added=0 len=1" in current_trace
    print(
        f"spindle: Cesta translation equals the Rust app on {compared} trace lines "
        f"({args.cases} scenarios x {len(variants)} build variants, GCC and Clang, "
        f"address and UB sanitizers); {len(mutations)} mutations detected"
    )


if __name__ == "__main__":
    main()
