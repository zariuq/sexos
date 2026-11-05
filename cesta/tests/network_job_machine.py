"""Qualify the bounded current-upstream Sexnet jobs machine in Cesta."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import subprocess
import sys
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(command, *, cwd=HERE, env=None):
    result = subprocess.run(list(map(str, command)), cwd=cwd, capture_output=True, env=env)
    assert result.returncode == 0, (command, result.returncode, result.stdout[-2000:], result.stderr[-8000:])
    return result.stdout


HOST = r'''
#include "network_job_machine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) check((condition), __LINE__)
static void check(bool condition, int line) {
    if (!condition) { fprintf(stderr, "network job check failed at %d\n", line); exit(1); }
}

typedef struct TestWire {
    NetworkJobLink link;
    uint8_t incoming[NETWORK_JOB_MAX_DUPLEX_BYTES + 1];
    uint8_t sent[NETWORK_JOB_MAX_DUPLEX_BYTES];
    size_t incoming_len, receive_at, sent_len, send_limit, receive_limit, queued;
    size_t begins, aborts, finishes, receive_calls;
    bool begin_fails;
} TestWire;

static bool wire_begin(void *context, uint32_t ticket, uint32_t *error) {
    TestWire *wire = context; (void)ticket; wire->begins++;
    if (wire->begin_fails) { *error = NETWORK_JOB_OFFLINE; return false; }
    return true;
}
static bool wire_receive(void *context, uint8_t *out, size_t capacity, size_t *count, uint32_t *error) {
    TestWire *wire = context; (void)error; wire->receive_calls++;
    size_t remaining = wire->incoming_len - wire->receive_at;
    size_t n = capacity < wire->receive_limit ? capacity : wire->receive_limit;
    if (n > remaining) n = remaining;
    memcpy(out, wire->incoming + wire->receive_at, n); wire->receive_at += n; *count = n; return true;
}
static bool wire_send(void *context, const uint8_t *data, size_t length, size_t *count, uint32_t *error) {
    TestWire *wire = context; (void)error;
    size_t n = length < wire->send_limit ? length : wire->send_limit;
    CHECK(wire->sent_len + n <= sizeof(wire->sent)); memcpy(wire->sent + wire->sent_len, data, n);
    wire->sent_len += n; *count = n; return true;
}
static size_t wire_queued(void *context) { return ((TestWire *)context)->queued; }
static NetworkJobLink wire_link(void *context) { return ((TestWire *)context)->link; }
static void wire_finish(void *context) { ((TestWire *)context)->finishes++; }
static void wire_abort(void *context) { ((TestWire *)context)->aborts++; }
static NetworkJobWire make_wire(TestWire *test) {
    return (NetworkJobWire){ test, wire_begin, wire_receive, wire_send, wire_queued, wire_link, wire_finish, wire_abort };
}
static void init_wire(TestWire *wire) {
    memset(wire, 0, sizeof(*wire)); wire->link = NETWORK_JOB_LINK_OPEN;
    wire->send_limit = NETWORK_JOB_TURN; wire->receive_limit = NETWORK_JOB_TURN;
}

static const NetworkJobIdentity A = {7, 70};
static const NetworkJobIdentity B = {7, 71};
static NetworkJobRequest request(uint32_t seq, uint8_t kind, uint32_t ticket) {
    return (NetworkJobRequest){seq, kind, 0, 0, ticket, 0};
}
static NetworkJobReplyDecode call_at(NetworkJobMachine *machine, TestWire *test, NetworkJobIdentity owner,
                                     NetworkJobRequest operation, uint64_t now) {
    NetworkJobWire wire = make_wire(test);
    CHECK(network_job_machine_handle(machine, owner, operation, now, wire));
    NetworkJobPendingReply pending = network_job_machine_pending_reply(machine);
    CHECK(pending.present && network_job_identity_equal(pending.owner, owner));
    NetworkJobReplyDecode result = network_job_decode_reply(pending.word, operation.seq);
    CHECK(result.valid); network_job_machine_reply_accepted(machine); return result;
}
static NetworkJobReplyDecode resolved_call(NetworkJobMachine *machine, TestWire *test, NetworkJobRequest operation,
                                            uint64_t now, const NetworkJobResolution *resolution, uint32_t error) {
    NetworkJobWire wire = make_wire(test);
    CHECK(network_job_machine_handle_resolved(machine, A, operation, now, false, resolution, error, wire));
    NetworkJobPendingReply pending = network_job_machine_pending_reply(machine);
    CHECK(pending.present && network_job_identity_equal(pending.owner, A));
    NetworkJobReplyDecode result = network_job_decode_reply(pending.word, operation.seq);
    CHECK(result.valid); network_job_machine_reply_accepted(machine); return result;
}
static void expect(NetworkJobReplyDecode result, bool success, uint32_t value) {
    CHECK(result.success == success && result.value == value);
}
static uint32_t stage(NetworkJobMachine *machine, TestWire *wire, size_t bytes) {
    NetworkJobRequest begin = request(1, NETWORK_JOB_BEGIN, 0); begin.length = (uint16_t)bytes;
    NetworkJobReplyDecode result = call_at(machine, wire, A, begin, 10); CHECK(result.success); uint32_t ticket = result.value;
    for (size_t offset = 0; offset < bytes; offset += 8) {
        NetworkJobRequest write = request(2 + (uint32_t)(offset / 8), NETWORK_JOB_WRITE, ticket);
        write.offset = (uint16_t)offset; write.length = (uint16_t)((bytes - offset) < 8 ? bytes - offset : 8);
        write.a = UINT64_MAX >> ((8 - write.length) * 8);
        expect(call_at(machine, wire, A, write, 10), true, (uint32_t)(offset + write.length));
    }
    return ticket;
}
static uint32_t start(NetworkJobMachine *machine, TestWire *wire, size_t bytes) {
    uint32_t ticket = stage(machine, wire, bytes);
    expect(call_at(machine, wire, A, request(1000, NETWORK_JOB_START, ticket), 10), true, 0); return ticket;
}
static NetworkJobResolution observation(void) {
    NetworkJobResolution result = {A, 1, {10, 0, 2, 2}, 10, 1010}; return result;
}
static uint32_t resolved_start(NetworkJobMachine *machine, TestWire *wire, uint8_t kind, size_t bytes) {
    NetworkJobRequest begin = request(1, kind, 0); begin.length = (uint16_t)bytes; begin.a = 1;
    NetworkJobResolution resolution = observation(); NetworkJobReplyDecode opened = resolved_call(machine, wire, begin, 10, &resolution, 0);
    CHECK(opened.success); uint32_t ticket = opened.value;
    for (size_t offset = 0; offset < bytes; offset += 8) {
        NetworkJobRequest write = request(2 + (uint32_t)(offset / 8), NETWORK_JOB_WRITE, ticket);
        write.offset = (uint16_t)offset; write.length = (uint16_t)((bytes - offset) < 8 ? bytes - offset : 8);
        write.a = UINT64_MAX >> ((8 - write.length) * 8);
        expect(call_at(machine, wire, A, write, 10), true, (uint32_t)(offset + write.length));
    }
    expect(call_at(machine, wire, A, request(1000, NETWORK_JOB_START, ticket), 10), true, 0); return ticket;
}
static void expect_scrubbed(const NetworkJobMachine *machine) {
    for (size_t i = 0; i < NETWORK_JOB_MAX_BYTES; i++) CHECK(machine->request[i] == 0 && machine->response[i] == 0);
}

static void identity_and_replay(void) {
    NetworkJobMachine machine; TestWire test; network_job_machine_init(&machine); init_wire(&test);
    uint32_t ticket = stage(&machine, &test, 1);
    expect(call_at(&machine, &test, B, request(5, NETWORK_JOB_CANCEL, ticket), 10), false, NETWORK_JOB_DENIED);
    expect(call_at(&machine, &test, A, request(5, NETWORK_JOB_CANCEL, ticket + 1), 10), false, NETWORK_JOB_STALE);
    expect(call_at(&machine, &test, A, request(4, NETWORK_JOB_CANCEL, ticket), 10), false, NETWORK_JOB_STALE);
    expect(call_at(&machine, &test, A, request(6, NETWORK_JOB_STATUS, ticket), 10), true, network_job_status(NETWORK_JOB_STAGING, 0));
    expect(call_at(&machine, &test, A, request(6, NETWORK_JOB_CANCEL, ticket), 10), false, NETWORK_JOB_STALE);
    NetworkJobRequest start_request = request(7, NETWORK_JOB_START, ticket);
    CHECK(network_job_machine_handle(&machine, A, start_request, 10, make_wire(&test)));
    CHECK(test.begins == 1); NetworkJobPendingReply receipt = network_job_machine_pending_reply(&machine); CHECK(receipt.present);
    CHECK(network_job_machine_handle(&machine, A, start_request, 11, make_wire(&test))); CHECK(test.begins == 1);
    network_job_machine_reply_accepted(&machine);
    CHECK(network_job_machine_handle(&machine, A, start_request, 11, make_wire(&test))); CHECK(test.begins == 1);
    CHECK(network_job_machine_pending_reply(&machine).word == receipt.word); network_job_machine_reply_accepted(&machine);
}

static void opening_waits_without_io(void) {
    NetworkJobMachine machine; TestWire test; network_job_machine_init(&machine); init_wire(&test);
    uint32_t ticket = start(&machine, &test, 1); (void)ticket;
    test.link = NETWORK_JOB_LINK_OPENING;
    network_job_machine_advance(&machine, 11, make_wire(&test));
    CHECK(machine.state == NETWORK_JOB_CONNECTING && machine.sent == 0 && test.receive_calls == 0 && test.sent_len == 0);
    test.link = NETWORK_JOB_LINK_OPEN;
    network_job_machine_advance(&machine, 12, make_wire(&test));
    CHECK(machine.sent == 1 && test.receive_calls == 1);
}

static void legacy_close(void) {
    NetworkJobMachine machine; TestWire test; network_job_machine_init(&machine); init_wire(&test);
    uint32_t ticket = start(&machine, &test, 600); test.send_limit = 99;
    for (uint64_t now = 11; now < 20; now++) network_job_machine_advance(&machine, now, make_wire(&test));
    CHECK(machine.sent == 600 && test.finishes == 1 && machine.state == NETWORK_JOB_RECEIVING);
    memset(test.incoming, 0x41, 600); test.incoming_len = 600; test.receive_limit = 101;
    for (uint64_t now = 20; now < 27; now++) network_job_machine_advance(&machine, now, make_wire(&test));
    CHECK(machine.response_len == 600 && machine.state != NETWORK_JOB_COMPLETE);
    test.link = NETWORK_JOB_LINK_PEER_FIN; network_job_machine_advance(&machine, 28, make_wire(&test));
    CHECK(machine.state == NETWORK_JOB_COMPLETE);
    NetworkJobRequest read = request(1001, NETWORK_JOB_READ, ticket); read.offset = 598; read.length = 2;
    expect(call_at(&machine, &test, A, read, 28), true, 0x4141);

    network_job_machine_init(&machine); init_wire(&test); start(&machine, &test, 8);
    machine.response[NETWORK_JOB_MAX_BYTES - 1] = 7; test.link = NETWORK_JOB_LINK_PEER_FIN;
    network_job_machine_advance(&machine, 11, make_wire(&test));
    CHECK(machine.state == NETWORK_JOB_FAILED); expect_scrubbed(&machine);
}

static void stream_custody(void) {
    NetworkJobMachine machine; TestWire test; network_job_machine_init(&machine); init_wire(&test);
    uint32_t ticket = resolved_start(&machine, &test, NETWORK_JOB_BEGIN_STREAM_RESOLVED, 1);
    network_job_machine_advance(&machine, 11, make_wire(&test)); CHECK(machine.sent == 1 && test.finishes == 1);
    for (size_t i = 0; i < 2049; i++) test.incoming[i] = (uint8_t)(i * 19 + 7);
    test.incoming_len = 2049; test.link = NETWORK_JOB_LINK_PEER_FIN;
    size_t actual = 0; uint32_t seq = 1001; uint64_t now = 12;
    for (size_t turn = 0; turn < 2000 && (actual < 2049 || machine.state != NETWORK_JOB_COMPLETE); turn++, now++) {
        network_job_machine_advance(&machine, now, make_wire(&test)); CHECK(machine.state != NETWORK_JOB_FAILED);
        NetworkJobStreamSnapshot snapshot = network_job_machine_stream_snapshot(&machine);
        CHECK(snapshot.present && snapshot.buffered <= NETWORK_JOB_STREAM_RING_BYTES);
        if (snapshot.buffered) {
            size_t count = snapshot.buffered < 4 ? snapshot.buffered : 4;
            NetworkJobRequest read = request(seq++, NETWORK_JOB_READ, ticket); read.offset = (uint16_t)actual; read.length = (uint16_t)count;
            NetworkJobReplyDecode word = call_at(&machine, &test, A, read, now); CHECK(word.success);
            for (size_t i = 0; i < count; i++) CHECK((uint8_t)(word.value >> (8 * i)) == (uint8_t)((actual + i) * 19 + 7));
            actual += count;
        }
    }
    CHECK(actual == 2049 && machine.state == NETWORK_JOB_COMPLETE && machine.consumed == machine.response_len);
}

static void duplex_graceful_close(void) {
    NetworkJobMachine machine; TestWire test; network_job_machine_init(&machine); init_wire(&test);
    uint32_t ticket = resolved_start(&machine, &test, NETWORK_JOB_BEGIN_DUPLEX_RESOLVED, 1);
    network_job_machine_advance(&machine, 11, make_wire(&test)); CHECK(machine.sent == 1 && test.finishes == 0);
    memcpy(test.incoming, "abc", 3); test.incoming_len = 3; test.link = NETWORK_JOB_LINK_PEER_FIN;
    network_job_machine_advance(&machine, 12, make_wire(&test)); CHECK(machine.response_len == 3 && test.finishes == 0);
    expect(call_at(&machine, &test, A, request(1001, NETWORK_JOB_FINISH, ticket), 12), true, 0);
    network_job_machine_advance(&machine, 13, make_wire(&test));
    CHECK(test.finishes == 1 && machine.finished_send && machine.state == NETWORK_JOB_RECEIVING);
    NetworkJobRequest read = request(1002, NETWORK_JOB_READ, ticket); read.length = 3;
    expect(call_at(&machine, &test, A, read, 13), true, UINT32_C(0x00636261));
    network_job_machine_advance(&machine, 14, make_wire(&test)); CHECK(machine.state == NETWORK_JOB_COMPLETE);

    network_job_machine_init(&machine); init_wire(&test);
    ticket = resolved_start(&machine, &test, NETWORK_JOB_BEGIN_DUPLEX_RESOLVED, 1);
    network_job_machine_advance(&machine, 11, make_wire(&test));
    expect(call_at(&machine, &test, A, request(1001, NETWORK_JOB_FINISH, ticket), 11), true, 0);
    network_job_machine_advance(&machine, 12, make_wire(&test));
    CHECK(machine.finished_send && machine.state != NETWORK_JOB_COMPLETE);
    test.link = NETWORK_JOB_LINK_CLOSED; network_job_machine_advance(&machine, 13, make_wire(&test));
    CHECK(machine.state == NETWORK_JOB_FAILED); expect_scrubbed(&machine);
}

static void exhaustion_cleanup_and_capacity(void) {
    NetworkJobMachine machine; TestWire test; network_job_machine_init(&machine); init_wire(&test);
    machine.next_ticket = NETWORK_JOB_MAX_TICKET; NetworkJobRequest begin = request(1, NETWORK_JOB_BEGIN, 0); begin.length = 1;
    expect(call_at(&machine, &test, A, begin, 10), true, NETWORK_JOB_MAX_TICKET);
    machine.response[NETWORK_JOB_MAX_BYTES - 1] = 9;
    expect(call_at(&machine, &test, A, request(2, NETWORK_JOB_RELEASE, NETWORK_JOB_MAX_TICKET), 10), true, 0); expect_scrubbed(&machine);
    begin.seq = 3; expect(call_at(&machine, &test, A, begin, 10), false, NETWORK_JOB_EXHAUSTED);

    network_job_machine_init(&machine); init_wire(&test);
    for (uint32_t i = 0; i < NETWORK_JOB_OWNER_SLOTS; i++) {
        NetworkJobIdentity owner = {100 + i, 1000 + i};
        expect(call_at(&machine, &test, owner, request(1, NETWORK_JOB_STATUS, 1), 10), false, NETWORK_JOB_DENIED);
    }
    CHECK(!network_job_machine_handle(&machine, A, begin, 10, make_wire(&test)));
    NetworkJobIdentity retired = {100, 1000}; CHECK(!network_job_machine_retire(&machine, retired, make_wire(&test)));
    begin.seq = 4; expect(call_at(&machine, &test, A, begin, 10), true, 1);

    network_job_machine_init(&machine); init_wire(&test);
    uint32_t ticket = resolved_start(&machine, &test, NETWORK_JOB_BEGIN_STREAM_RESOLVED, 1);
    machine.request[NETWORK_JOB_MAX_BYTES - 1] = 8; machine.response[NETWORK_JOB_MAX_BYTES - 1] = 7;
    network_job_machine_advance(&machine, machine.idle_deadline, make_wire(&test));
    CHECK(machine.state == NETWORK_JOB_FAILED && !network_job_machine_can_transmit(&machine, ticket)); expect_scrubbed(&machine);
}

int main(void) {
    identity_and_replay(); opening_waits_without_io(); legacy_close(); stream_custody(); duplex_graceful_close(); exhaustion_cleanup_and_capacity();
    puts("PASS network-job-machine current state, replay, custody, timeout, and graceful-close controls"); return 0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    parser.add_argument("--output", type=Path, default=HERE / "build/network-job-machine-tests")
    args = parser.parse_args()
    output = args.output.resolve(); output.mkdir(parents=True, exist_ok=True)

    inventory = json.loads((HERE / "interfaces/network_job_machine.source.json").read_text())
    jobs = run(["git", "show", f"{inventory['pin']}:{inventory['path']}"], cwd=HERE.parent)
    abi = run(["git", "show", f"{inventory['pin']}:{inventory['abi_path']}"], cwd=HERE.parent)
    assert hashlib.sha256(jobs).hexdigest() == inventory["sha256"]
    assert hashlib.sha256(abi).hexdigest() == inventory["abi_sha256"]
    rust_source = "mod network_job {\n" + abi.decode().replace("//!", "//") + "\n}\nmod jobs {\n"
    rust_source += jobs.decode().replace("use sex_pdx::network_job", "use crate::network_job") + "\n}\n"
    rust = output / "reference.rs"; rust.write_text(rust_source)
    rust_tests = output / "reference-tests"
    run(["rustc", "--edition=2021", "--test", "-O", rust, "-o", rust_tests])
    rust_output = run([rust_tests, "--test-threads=1"])
    assert b"47 passed" in rust_output, rust_output[-2000:]

    host = output / "host.c"; host.write_text(HOST)
    configurations = []; plans = {}; machine_exports = {}
    for compiler in ("gcc", "clang"):
        sources = []
        for name in ("network_job_abi", "network_job_wire", "network_job_machine"):
            source = output / f"{name}-{compiler}.c"; command = [args.cesta, "export-c", f"src/{name}.cesta", "--cc", compiler, "--std=c11", "-o", source]
            if name == "network_job_machine":
                plan = output / f"{name}-{compiler}.plan"; command.extend(["--plan", plan]); plans[compiler] = plan
            else:
                run(command + ["--", "-Igenerated/include"])
            if name == "network_job_machine":
                run(command + ["--", "-Igenerated/include"])
                machine_exports[compiler] = source
            sources.append(source)
        for mode, flags in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = output / f"{compiler}-{mode}"
            run([compiler, "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror", *flags,
                 "-Igenerated/include", *sources, host, "-o", binary])
            actual = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            configurations.append({"compiler": compiler, "mode": mode, "sha256": hashlib.sha256(actual).hexdigest()})

    base = LayoutText(machine_exports["gcc"].read_text())
    mutations = [
        ("opening-does-no-io", "if(initial_link==NETWORK_JOB_LINK_OPENING){m->state=NETWORK_JOB_CONNECTING;return;}", "if(initial_link==NETWORK_JOB_LINK_OPENING){m->state=NETWORK_JOB_CONNECTING;}"),
        ("duplex-needs-peer-fin", "m->finished_send&&link==NETWORK_JOB_LINK_PEER_FIN&&drained", "m->finished_send&&link!=NETWORK_JOB_LINK_CLOSED&&drained"),
        ("duplex-preserves-unread", "&&m->consumed==m->response_len&&network_job_wire_queued", "&&network_job_wire_queued"),
        ("closed-is-failure", "==NETWORK_JOB_LINK_CLOSED", "==((NetworkJobLink)99)"),
        ("ticket-exhaustion", "if(m->next_ticket>NETWORK_JOB_MAX_TICKET)", "if(false)"),
        ("owner-enforcement", "if(!m->owner_present||!network_job_identity_equal(m->owner,owner))", "if(!m->owner_present&&!network_job_identity_equal(m->owner,owner))"),
        ("response-scrub", "for(size_t i=0;i<sizeof(m->response);i++)m->response[i]=0;", "for(size_t i=0;i<sizeof(m->response)-1;i++)m->response[i]=0;"),
    ]
    caught = []
    abi_c = output / "network_job_abi-gcc.c"; wire_c = output / "network_job_wire-gcc.c"
    for name, old, new in mutations:
        assert base.count(old) >= 1, (name, base.count(old))
        mutant = output / f"mutant-{name}.c"; mutant.write_text(base.replace(old, new))
        binary = output / f"mutant-{name}"
        run(["gcc", "-std=c11", "-O2", "-Igenerated/include", abi_c, wire_c, mutant, host, "-o", binary])
        result = subprocess.run([binary], cwd=HERE, capture_output=True)
        assert result.returncode != 0, ("survived", name, result.stdout, result.stderr); caught.append(name)

    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source.resolve() / "tests"))
        helper = importlib.import_module("ownership_admission")
        unique = {}
        for plan in plans.values(): unique[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for index, plan in enumerate(unique.values()):
            (record,) = helper.parse(plan.read_bytes())
            admission.append(helper.admit_record(record, output / f"Machine{index}.lean", args.lean_root.resolve()))

    summary = {
        "pin": inventory["pin"], "upstream_rust_tests": 47,
        "configurations": configurations, "mutants_caught": caught, "admission": admission,
    }
    (output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("network job machine: 47 upstream tests;", len(configurations), "C configurations;", len(caught), "mutants;", sum(x["functions"] for x in admission), "plans admitted")


if __name__ == "__main__":
    main()
