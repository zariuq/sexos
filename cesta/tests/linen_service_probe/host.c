/* Protocol probe joining the actual kernel RPC, Linen dispatcher, shell
 * capture/publication client and display admission adapter. CPU entry and
 * pixels remain host boundaries. */
#define main historical_kernel_trace_main
#include "../kernel_model/host.c"
#undef main
#undef SLOT_USB_HOST
#undef SLOT_NIC
#include "linen_adapter.h"
#include "silk_shell_models_linen.h"
#include "shell_linen_runtime.h"
#include <assert.h>

const LinenObject LINEN_SEED_OBJECTS[6] = {{0}};
static unsigned paints;

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
    TrapFrame frame = trap(3, SEXOS_RPC_call, slot, opcode, a, b, c);
    return (SexosReply){frame.rax, frame.rsi};
}
SexosReply sexos_rpc_poll(uint64_t id)
{
    TrapFrame frame = trap(3, SEXOS_RPC_poll, id, 0, 0, 0, 0);
    return (SexosReply){frame.rax, frame.rsi};
}
uint64_t sexos_rpc_cancel(uint64_t id)
{
    return trap(3, SEXOS_RPC_cancel, id, 0, 0, 0, 0).rax;
}
uint64_t sexos_rpc_reply(uint32_t caller, uint64_t id, uint64_t value)
{
    return trap(7, SEXOS_RPC_reply, caller, id, value, 0, 0).rax;
}
uint64_t sexos_reply(uint32_t caller, uint64_t value)
{
    return trap(7, SEXOS_SYSCALL_reply, caller, value, 0, 0, 0).rax;
}
uint64_t sexos_answer_request(const SexosRequest *request, uint64_t value)
{
    return request->id ? sexos_rpc_reply(request->message.caller_pd, request->id, value) :
        sexos_reply(request->message.caller_pd, value);
}
SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t a, uint64_t b, uint64_t c)
{
    TrapFrame frame = trap(3, SEXOS_SYSCALL_call, slot, opcode, a, b, c);
    return (SexosReply){frame.rax, frame.rsi};
}
uint64_t sexos_rpc_register(void) { return 1; }
SexosMessage sexos_listen(SexosListener *listener, uint64_t slot)
{ (void)listener; (void)slot; return (SexosMessage){0}; }
SexosRequest sexos_receive_request(bool correlated)
{ (void)correlated; return (SexosRequest){0}; }
void sexos_line(const char *format, ...) { (void)format; }
void sexos_spin_loop_hint(void) {}
_Noreturn void sexos_spin_forever(void) { abort(); }
void sexos_yield(void) {}
void sexos_fail(void) { abort(); }

static void setup(void)
{
    memset(&kernel, 0, sizeof kernel);
    const unsigned ids[] = {1, 3, 7};
    for (size_t i = 0; i < 3; ++i) {
        unsigned id = ids[i]; Domain *domain = domain_boot_record(id); assert(domain);
        domain->live = true; domain->id = id; domain->key = (uint8_t)id;
        domain->task_id = id; domain->task_state = TASK_READY;
        assert(cap_set(domain, 2, (Cap){.kind = CAP_MESSAGE_QUEUE}));
    }
    assert(cap_set(domain_get(3), 13, (Cap){.kind = CAP_DOMAIN, .target = 7}));
    assert(cap_set(domain_get(3), 5, (Cap){.kind = CAP_DOMAIN, .target = 1}));
    assert(cap_set(domain_get(1), 6, (Cap){.kind = CAP_DOMAIN, .target = 3}));
    kernel_seal_genesis();
    assert(trap(7, SEXOS_RPC_register, 0, 0, 0, 0, 0).rax == 1);
}

static bool serve_linen(void)
{
    TrapFrame heard = trap(7, SEXOS_RPC_listen, 0, 0, 0, 0, 0);
    if (!heard.rax) return false;
    SexosRequest request = {.message = {.type_id = heard.rax, .caller_pd = (uint32_t)heard.rsi,
        .caller_task = (uint32_t)heard.rdi, .arg0 = heard.rdx, .arg1 = heard.r10,
        .arg2 = heard.r8}, .id = heard.r9};
    probe_linen_dispatch(&request);
    return true;
}

void shell_models_peer_paint_fast(ShellLinenModelsContext *context)
{
    ++paints;
    ShellTextBytes text = {.len = 2, .bytes = {'O', 'K'}};
    ShellTextMetadata metadata = {context->linen->capture_snapshot.count, 0, 1,
        context->models->SELECTED_LINEN_OBJECT_ID};
    assert(shell_linen_text_queue(context->linen, &text, UINT32_C(0x00f0f8ff), metadata));
}

static uint64_t display_ack(uint32_t sequence)
{
    return UINT64_C(0x56) | (UINT64_C(200) << 8) | ((uint64_t)sequence << 24);
}

static void drain_display(uint32_t sequence)
{
    unsigned operations = 0;
    for (;;) {
        TrapFrame heard = trap(1, SEXOS_SYSCALL_listen, 0, 0, 0, 0, 0);
        if (!heard.rax) break;
        assert(heard.rsi == 3 && heard.rdi == 3);
        assert(heard.rax == UINT64_C(0x276) || heard.rax == UINT64_C(0x277) ||
            heard.rax == UINT64_C(0x278));
        if (heard.rax == UINT64_C(0x278)) assert((uint32_t)heard.r10 == sequence);
        ++operations;
    }
    assert(operations == 3);
}

static SexosMessage display_notice(uint64_t value)
{
    assert(trap(1, SEXOS_SYSCALL_call, 6, 1, value, 0, 0).rax == 0);
    TrapFrame heard = trap(3, SEXOS_SYSCALL_listen, 0, 0, 0, 0, 0);
    assert(heard.rax == 1 && heard.rsi == 1 && heard.rdi == 1);
    return (SexosMessage){.type_id = heard.rax, .caller_pd = (uint32_t)heard.rsi,
        .caller_task = (uint32_t)heard.rdi, .arg0 = heard.rdx, .arg1 = heard.r10,
        .arg2 = heard.r8};
}

int main(void)
{
    setup();
    probe_linen_init();

    SilkShellModelsState models = {0};
    ShellLinenRuntime runtime = shell_models_linen_new();
    ShellLinenModelsContext context = {&models, &runtime};
    assert(runtime.correlated_required);
    shell_models_fetch_remote_snapshot(&context);
    bool captured = false;
    for (unsigned turn = 0; linen_capture_client_busy(&runtime.capture); ++turn) {
        assert(turn < 1000);
        shell_models_linen_async_poll(&context);
        if (serve_linen() && probe_linen_capture_active() && !captured) {
            captured = true;
            probe_linen_mutate_live_table();
        }
    }
    assert(captured && paints == 1 && runtime.publication_revision == 2);
    assert(models.LINEN_REMOTE_REAL && models.LINEN_REMOTE_FETCHED);
    assert(models.LINEN_OBJECTS[0].present);
    assert(models.LINEN_OBJECTS[0].object_id == UINT64_C(0xfedcba9876543210));
    assert(models.LINEN_OBJECTS[0].name[0] == 240 && models.LINEN_OBJECTS[0].name_len == 24);

    uint64_t job = runtime.presentation_active.job;
    uint32_t sequence = runtime.presentation_active.sequence;
    uint64_t revision = runtime.presentation_active.revision;
    while (!runtime.text.current.awaiting) {
        ShellTextPoll result = shell_linen_text_poll(&runtime, 1);
        assert(result.kind == SHELL_TEXT_PENDING);
    }
    assert(runtime.presentation_active.submitted);
    drain_display(sequence);

    ShellTextReceipt receipt = {0};
    SexosMessage stale = display_notice(display_ack(sequence + 1));
    assert(!shell_linen_text_receive(&runtime, stale.caller_pd, stale.caller_task, stale.arg0, &receipt));
    assert(runtime.presentation_active.job == job);
    SexosMessage applied = display_notice(display_ack(sequence));
    assert(shell_linen_text_receive(&runtime, applied.caller_pd, applied.caller_task, applied.arg0, &receipt));
    assert(receipt.bytes == 2 && receipt.metadata.count == 1);
    assert(runtime.presentation_last.outcome == SHELL_PRESENTATION_COMPLETE);
    assert(runtime.presentation_last.job == job && runtime.presentation_last.revision == revision);
    assert(!runtime.presentation_active.present && !probe_linen_capture_active());
    puts("PASS: correlated Linen protocol probe joined real client, server, kernel and display adapters");
}
