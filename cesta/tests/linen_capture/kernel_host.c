/* Actual kernel and service state machines; CPU traps and device/heap access
 * are replaced by the existing host boundary. This is not a boot test. */
#define main historical_kernel_trace_main
#include "../kernel_model/host.c"
#undef main
#include "linen_capture_transport.h"
#include "linen_capture_service.h"

static TrapFrame trap(uint32_t actor, uint64_t op, uint64_t a, uint64_t b,
                      uint64_t c, uint64_t d, uint64_t e)
{
    kernel.current = actor;
    domain_get(actor)->task_state = TASK_RUNNING;
    TrapFrame frame = {.rax = op, .rdi = a, .rsi = b, .rdx = c, .r10 = d, .r8 = e};
    assert(kernel_syscall(&frame) == &frame);
    return frame;
}

SexosReply sexos_rpc_begin(uint64_t slot, uint64_t opcode, uint64_t a, uint64_t b, uint64_t c)
{
    TrapFrame f = trap(3, SEXOS_RPC_call, slot, opcode, a, b, c);
    return (SexosReply){f.rax, f.rsi};
}
SexosReply sexos_rpc_poll(uint64_t id)
{
    TrapFrame f = trap(3, SEXOS_RPC_poll, id, 0, 0, 0, 0);
    return (SexosReply){f.rax, f.rsi};
}
uint64_t sexos_rpc_cancel(uint64_t id) {return trap(3, SEXOS_RPC_cancel, id, 0, 0, 0, 0).rax;}
uint64_t sexos_rpc_reply(uint32_t caller, uint64_t id, uint64_t value)
{
    return trap(7, SEXOS_RPC_reply, caller, id, value, 0, 0).rax;
}
uint64_t sexos_reply(uint32_t caller, uint64_t value)
{
    return trap(7, SEXOS_SYSCALL_reply, caller, value, 0, 0, 0).rax;
}

static void setup(void)
{
    memset(&kernel, 0, sizeof kernel);
    const unsigned ids[] = {1, 3, 7};
    for (size_t i = 0; i < 3; ++i) {
        unsigned id = ids[i]; Domain *d = domain_boot_record(id); assert(d);
        d->live = true; d->id = id; d->key = (uint8_t)id;
        d->task_id = id * 101; d->task_state = TASK_READY;
        assert(cap_set(d, 2, (Cap){.kind = CAP_MESSAGE_QUEUE}));
    }
    assert(cap_set(domain_get(3), 13, (Cap){.kind = CAP_DOMAIN, .target = 7}));
    assert(cap_set(domain_get(3), 5, (Cap){.kind = CAP_DOMAIN, .target = 1}));
    assert(cap_set(domain_get(1), 6, (Cap){.kind = CAP_DOMAIN, .target = 3}));
    kernel_seal_genesis();
}

static bool serve(LinenCaptureServer *server, const LinenCaptureTable *table,
                  LinenCaptureAnswer *answer, bool send)
{
    if (answer->present) return send && linen_capture_answer_poll(answer);
    TrapFrame f = trap(7, SEXOS_RPC_listen, 0, 0, 0, 0, 0);
    if (!f.rax) return false;
    SexosRequest request = {.message = {.type_id = f.rax, .caller_pd = (uint32_t)f.rsi,
        .caller_task = (uint32_t)f.rdi, .arg0 = f.rdx, .arg1 = f.r10, .arg2 = f.r8}, .id = f.r9};
    assert(linen_capture_service_prepare(server, table, &request, answer));
    assert(answer->request.id == f.r9 && answer->request.message.caller_task == f.rdi);
    return send && linen_capture_answer_poll(answer);
}

static void drive(LinenCaptureClient *client, LinenCaptureServer *server,
                  LinenCaptureTable *table, LinenCaptureAnswer *answer)
{
    unsigned publications = 0;
    for (unsigned turn = 0; linen_capture_client_busy(client); ++turn) {
        assert(turn < 1000);
        LinenCaptureStep step = linen_capture_transport_step(client, 13);
        if (step.event == LINEN_CLIENT_PUBLISHED) ++publications;
        else assert(step.event == LINEN_CLIENT_IGNORED || step.event == LINEN_CLIENT_PROGRESS);
        (void)serve(server, table, answer, true);
    }
    assert(publications == 1 && !server->active && !answer->present);
}

int main(void)
{
    setup();
    LinenCaptureClient client = linen_capture_client_new();
    LinenCaptureServer server = linen_capture_new(3, 303);
    LinenCaptureAnswer answer = {0}; LinenCaptureTable table = {0};
    table.rows[0] = (LinenCaptureRow){.present = true, .id = UINT64_C(0xfedcba9876543210), .len = 24};
    for (size_t i = 0; i < 24; ++i) table.rows[0].name[i] = (uint8_t)(240 + i);

    /* Strong-required admission never silently selects the legacy path. */
    assert(linen_capture_client_refresh(&client));
    LinenCaptureStep unsupported = linen_capture_transport_step(&client, 13);
    assert(unsupported.status == SEXOS_RPC_UNSUPPORTED && client.request == 0 && client.job == 1);
    assert(trap(7, SEXOS_RPC_register, 0, 0, 0, 0, 0).rax == 1);
    assert(trap(1, SEXOS_RPC_register, 0, 0, 0, 0, 0).rax == 1);

    uint64_t unrelated[KERNEL_RPC_CAPACITY] = {0};
    for (unsigned i = 0; i + 1 < KERNEL_RPC_CAPACITY; ++i) {
        SexosReply call = sexos_rpc_begin(5, UINT64_C(0x600), i, 0, 0);
        assert(call.status == 0); unrelated[i] = call.value;
        TrapFrame heard = trap(1, SEXOS_RPC_listen, 0, 0, 0, 0, 0);
        assert(heard.r9 == call.value && heard.rdi == 303);
        assert(trap(1, SEXOS_RPC_reply, 3, call.value, UINT64_C(0x8000000000000000) + i, 0, 0).rax == 0);
    }
    assert(trap(1, SEXOS_SYSCALL_call, 6, UINT64_C(0x999), 17, 18, 19).rax == 0);
    assert(linen_capture_transport_step(&client, 13).admitted);
    assert(serve(&server, &table, &answer, false) == false && answer.present && server.active);
    uint64_t abandoned = client.request;
    assert(linen_capture_transport_retry(&client));
    assert(!rpc_find(domain_get(3), abandoned));
    assert(linen_capture_answer_poll(&answer) && !answer.present);
    assert(client.job == 1 && server.job == 1);
    LinenCaptureFrame retained = server.frame;
    table.rows[0].name[0] ^= UINT8_C(0xff);
    drive(&client, &server, &table, &answer);
    assert(client.frame.len == retained.len && !memcmp(client.frame.bytes, retained.bytes, retained.len));
    assert(client.table.rows[0].id == UINT64_C(0xfedcba9876543210));
    assert(domain_get(3)->ring.count == 1); /* Notification never consumed by RPC polling. */
    for (unsigned i = 0; i + 1 < KERNEL_RPC_CAPACITY; ++i) {
        SexosReply ready = sexos_rpc_poll(unrelated[i]);
        assert(ready.status == 0 && ready.value == UINT64_C(0x8000000000000000) + i);
        assert(sexos_rpc_poll(unrelated[i]).status == SEXOS_RPC_INVALID);
    }

    assert(linen_capture_client_refresh(&client));
    assert(linen_capture_transport_step(&client, 13).admitted);
    assert(linen_capture_transport_cancel(&client)); /* Cancel queued begin; retain cleanup. */
    assert(client.job == 2 && client.phase == LINEN_CLIENT_END);
    (void)serve(&server, &table, &answer, true);
    for (unsigned turn = 0; linen_capture_client_busy(&client); ++turn) {
        assert(turn < 10);
        LinenCaptureStep step = linen_capture_transport_step(&client, 13);
        if (step.event) assert(step.event == LINEN_CLIENT_CANCELLED);
        (void)serve(&server, &table, &answer, true);
    }
    assert(!server.active && !answer.present);
    puts("PASS: real current kernel RPC, capture client/service adapters, retained envelopes, cancellation and unrelated replies");
}
