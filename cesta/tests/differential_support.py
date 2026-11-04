"""Shared pieces of the Rust-reference differential tests."""

import re
import subprocess
import multiprocessing
from concurrent.futures import ProcessPoolExecutor,ThreadPoolExecutor


def run(args, env=None, **kwargs):
    return subprocess.run([str(a) for a in args], check=True, capture_output=True, env=env, **kwargs)


def parallel_map(function, items, jobs=4):
    """Run independent subprocess-oriented cases concurrently, preserving order."""
    items = list(items)
    if jobs <= 1 or len(items) <= 1:
        return [function(item) for item in items]
    with ThreadPoolExecutor(max_workers=min(jobs,len(items))) as pool:
        return list(pool.map(function,items))


def _run_command(spec):
    args,env=spec
    command=[str(a)for a in args]
    result=subprocess.run(command,capture_output=True,env=env)
    if result.returncode:
        raise RuntimeError((command,result.returncode,result.stdout[-1000:],result.stderr[-3000:]))
    return result.stdout


def run_commands(commands,jobs=4):
    """Run independent commands in a bounded process pool, preserving order."""
    commands=[(args,env)for args,env in commands]
    if jobs<=1 or len(commands)<=1:return [_run_command(command)for command in commands]
    with ProcessPoolExecutor(max_workers=min(jobs,len(commands)),
                             mp_context=multiprocessing.get_context("spawn"))as pool:
        return list(pool.map(_run_command,commands))


def pinned_function(source, signature, syscalls):
    """The function of the pinned SOURCE that starts with SIGNATURE, each inline
    syscall (named by the RAX operand as written) replaced by provider code."""
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    function = source[start:source.index("\n}\n", start) + 3]
    for rax, replacement in syscalls.items():
        pattern = r'unsafe \{\s*core::arch::asm!\(\s*"syscall",\s*in\("rax"\) %s,.*?\);\s*\}' % re.escape(rax)
        function, count = re.subn(pattern, lambda _: replacement, function, flags=re.S)
        assert count == 1, (signature, rax)
    return function


# The listen syscall (28) as a provider call returning (type, caller, arg0, arg1, arg2).
LISTEN = {"28u64": "let (t, c, a, b, d) = crate::listen_once(slot); "
                   "type_id = t; caller_pd = c; arg0 = a; arg1 = b; arg2 = d;"}


def first_difference(actual, expected, label):
    a, e = actual.splitlines(), expected.splitlines()
    i = next(i for i in range(max(len(a), len(e))) if i >= len(a) or i >= len(e) or a[i] != e[i])
    return (f"{label}: first difference at line {i + 1}\n"
            f"  rust: {e[i][:300] if i < len(e) else '<end>'}\n  c:    {a[i][:300] if i < len(a) else '<end>'}\n"
            f"  context: {[line[:200] for line in e[max(0, i - 3):i]]}")


# A provider written once in C serves both servers: the Rust reference links
# it through FFI and the Cesta harness links it directly, so both meet the
# same storage replies, requests and faults, and both log through the same
# raw-print call. The server's part defines storage_reply (the answer to the
# pending storage call), request (any other message), reply_share (percent of
# non-empty listens that answer a pending call), answered (whether SexFiles
# answers an opcode at all) and server_start. Every random draw sits in its
# own statement, so GCC and Clang builds draw in the same order.
PROVIDER_PRELUDE = r'''
#include "sexos_server.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A scenario: seed, limit (yields), listen (percent of non-empty listens),
   faults (0 none, 1 light, 2 heavy) and mix (0-2, the server's choice). */
static struct { uint64_t seed, limit, listen, faults, mix; } scenario;
static uint64_t rng, yields, listens, spins;
static struct { uint64_t opcode, a0, a1, a2; bool pending; } call;

static uint64_t next(void) {
    rng += 0x9E3779B97F4A7C15ull;
    uint64_t z = rng;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
#define PICK(...) pick((const uint64_t[]){__VA_ARGS__}, sizeof((const uint64_t[]){__VA_ARGS__}) / sizeof(uint64_t))
static uint64_t pick(const uint64_t *values, size_t count) { return values[next() % count]; }
static bool chance(uint64_t percent) { return next() % 100 < percent; }
static bool fault(void) { return scenario.faults && chance(scenario.faults == 1 ? 6 : 25); }
/* -2, -3, -4, -5 and -19 (ERR_NO_DEVICE). */
static uint64_t error(void) { return PICK(UINT64_MAX - 1, UINT64_MAX - 2, UINT64_MAX - 3, UINT64_MAX - 4, UINT64_MAX - 18); }

/* The bytes [offset, end) of BYTES as a little-endian word (at most eight). */
static uint64_t word_at(const uint8_t *bytes, size_t end, size_t offset) {
    uint64_t w = 0;
    for (size_t i = 0; i < 8 && offset + i < end; ++i) w |= (uint64_t)bytes[offset + i] << (8 * i);
    return w;
}
static inline uint64_t word(const char *text) { return word_at((const uint8_t *)text, strlen(text), 0); }

static void flush_spins(void) {
    if (spins) printf("S %" PRIu64 "\n", spins);
    spins = 0;
}
void sexos_spin_loop_hint(void) { ++spins; }
_Noreturn void provider_panic(void) { flush_spins(); printf("PANIC\n"); exit(0); }
_Noreturn void sexos_spin_forever(void) { provider_panic(); }
uint64_t sexos_allocate_memory(uint64_t size) { (void)size; printf("UNEXPECTED allocate\n"); exit(3); }
uint64_t sexos_map_memory(uint64_t pfn, uint64_t size) { (void)pfn; (void)size; printf("UNEXPECTED map\n"); exit(3); }

static uint64_t storage_reply(void);
static void request(SexosMessage *m);
static uint64_t reply_share(void);
static bool answered(uint64_t opcode);
static void server_start(void);

SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t a0, uint64_t a1, uint64_t a2) {
    flush_spins();
    if (slot == 0 && opcode == 69) {
        const char *text = (const char *)(uintptr_t)a0;
        size_t length = (size_t)a1;
        if (length && text[length - 1] == '\n') --length;
        fputs("L ", stdout);
        fwrite(text, 1, length, stdout);
        fputc('\n', stdout);
        return (SexosReply){0, 0};
    }
    uint64_t status = 0;
    /* Heavy faults refuse some enqueues, of requests SexFiles would take. */
    if (slot == 1 && scenario.faults == 2 && answered(opcode) && chance(3)) status = PICK(1, UINT64_MAX - 5);
    printf("C %" PRIu64 " %#" PRIx64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " -> %" PRIu64 "\n", slot, opcode, a0, a1, a2, status);
    if (slot == 1 && status == 0 && answered(opcode)) {
        call.opcode = opcode;
        call.a0 = a0;
        call.a1 = a1;
        call.a2 = a2;
        call.pending = true;
    }
    return (SexosReply){status, 0};
}

SexosMessage sexos_listen_once(uint64_t slot) {
    flush_spins();
    if (++listens > 40 * scenario.limit) { printf("END listens\n"); exit(0); }
    SexosMessage m = {0, 0, 0, 0, 0, 0};
    if (chance(scenario.listen)) {
        if (call.pending && chance(reply_share())) {
            m.type_id = 1;
            m.arg0 = storage_reply();
            call.pending = false;
        } else {
            request(&m);
        }
    }
    printf("T %" PRIu64 " %#" PRIx64 " %" PRIu32 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", slot, m.type_id, m.caller_pd, m.arg0, m.arg1, m.arg2);
    return m;
}

uint64_t sexos_reply(uint32_t target_pd, uint64_t value) {
    flush_spins();
    printf("R %" PRIu32 " %" PRIu64 "\n", target_pd, value);
    return 0;
}

void sexos_yield(void) {
    flush_spins();
    printf("Y\n");
    if (++yields >= scenario.limit) { printf("END\n"); exit(0); }
}

void provider_start(int argc, char **argv) {
    if (argc != 6) exit(2);
    uint64_t *fields[5] = {&scenario.seed, &scenario.limit, &scenario.listen, &scenario.faults, &scenario.mix};
    for (int i = 0; i < 5; ++i) *fields[i] = strtoull(argv[i + 1], NULL, 10);
    rng = scenario.seed;
    server_start();
}
'''

# The C harness's entry: the scenario, then the translated server.
C_MAIN = r'''
#include "sexos_server.h"
void provider_start(int argc, char **argv);
int main(int argc, char **argv) {
    provider_start(argc, argv);
    %s();
    return 0;
}
'''

# The Rust side of the provider: its calls, the kernel's raw print as the C
# runtime's line writer makes it, and the opening of a sex_pdx replacement
# module (a server adds its constants and the pinned listen functions, then
# closes it).
RUST_FFI = r'''
#[repr(C)] pub struct SexosReply { pub status: u64, pub value: u64 }
#[repr(C)] pub struct SexosMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, pub padding: u32 }
extern "C" {
    pub fn sexos_pdx_call(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> SexosReply;
    pub fn sexos_listen_once(slot: u64) -> SexosMessage;
    pub fn sexos_reply(target_pd: u32, value: u64) -> u64;
    pub fn sexos_yield();
    pub fn sexos_spin_loop_hint();
    fn provider_start(argc: i32, argv: *const *const std::os::raw::c_char);
    fn provider_panic() -> !;
}
fn listen_once(slot: u64) -> (u64, u64, u64, u64, u64) {
    let m = unsafe { sexos_listen_once(slot) };
    (m.type_id, m.caller_pd as u64, m.arg0, m.arg1, m.arg2)
}
fn log_line(text: &str) {
    let line = format!("{text}\n");
    unsafe { sexos_pdx_call(0, 69, line.as_ptr() as u64, line.len() as u64, 0); }
}
mod sex_pdx {
    macro_rules! serial_println { ($($argument:tt)*) => { crate::log_line(&format!($($argument)*)) }; }
    pub(crate) use serial_println;
    pub const SLOT_STORAGE: u64 = 1;
    pub const SLOT_DISPLAY: u64 = 5;
    pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, _pad: u32 }
    pub fn pdx_call(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> (u64, u64) {
        let reply = unsafe { crate::sexos_pdx_call(slot, opcode, arg0, arg1, arg2) };
        (reply.status, reply.value)
    }
    pub fn pdx_reply(target_pd: u32, value: u64) -> u64 { unsafe { crate::sexos_reply(target_pd, value) } }
    pub fn sys_yield() { unsafe { crate::sexos_yield() } }
    pub fn sched_yield() { sys_yield(); }
'''

# The Rust reference's entry: a panic is the provider's (the server's own
# handler spins), then the scenario, then the server's renamed _start.
RUST_MAIN = r'''
fn main() {
    std::panic::set_hook(Box::new(|_| unsafe { provider_panic() }));
    let args: Vec<std::ffi::CString> = std::env::args().map(|a| std::ffi::CString::new(a).unwrap()).collect();
    let argv: Vec<*const std::os::raw::c_char> = args.iter().map(|a| a.as_ptr()).collect();
    unsafe { provider_start(argv.len() as i32, argv.as_ptr()) };
    %s()
}
'''
