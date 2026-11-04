#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_document_client_transport.h"

#define MUST(condition) do { if (!(condition)) __builtin_trap(); } while (0)

typedef struct FakeRequest {bool active,ready;uint64_t id,value;SexosDocumentRequest document;} FakeRequest;
static FakeRequest requests[16];
static size_t request_count;
static uint64_t next_id = 100;
static uint64_t begin_error;
static unsigned cancellations;

SexosReply sexos_rpc_begin(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    MUST(slot == SEXOS_SLOT_storage && opcode == SEXOS_DOCUMENT_OP);
    if (begin_error) { uint64_t error = begin_error; begin_error = 0; return (SexosReply){error, 0}; }
    MUST(request_count < 16); SexosDocumentDecode decoded = sexos_document_decode(arg0, arg1, arg2); MUST(decoded.ok);
    uint64_t id = next_id++; requests[request_count++] = (FakeRequest){true, false, id, 0, decoded.request};
    return (SexosReply){0, id};
}
SexosReply sexos_rpc_poll(uint64_t id) {
    for (size_t i = 0; i < request_count; i++) if (requests[i].active && requests[i].id == id) {
        if (!requests[i].ready) return (SexosReply){SEXOS_RPC_BUSY, 0};
        requests[i].active = false; return (SexosReply){0, requests[i].value};
    }
    return (SexosReply){SEXOS_RPC_INVALID, 0};
}
uint64_t sexos_rpc_cancel(uint64_t id) {
    for (size_t i = 0; i < request_count; i++) if (requests[i].active && requests[i].id == id) {
        requests[i].active = false; cancellations++; return 0;
    }
    return SEXOS_RPC_INVALID;
}
DocumentSnapshotCopyResult document_snapshot_copy_value(DocumentSnapshotRegion region, uint8_t object,
                                                         uint32_t generation, uint32_t sequence, size_t length) {
    (void)region; (void)object; (void)generation; (void)sequence; (void)length;
    return (DocumentSnapshotCopyResult){.error=SEXOS_DOCUMENT_STALE};
}
static FakeRequest *latest(uint8_t kind) {
    MUST(request_count); FakeRequest *request = &requests[request_count - 1];
    MUST(request->document.kind == kind); return request;
}
static void complete(FakeRequest *request, bool success, uint32_t value) {
    request->value = sexos_document_reply(request->document.seq, success, value); request->ready = true;
}

int main(void) {
    DocumentClient client; DocumentClientTransport transport; DocumentClientEvent event;
    document_client_init(&client); document_client_transport_init(&transport);
    MUST(document_client_load(&client, 3, 9));
    MUST(!document_client_transport_step(&client, &transport, &event));
    FakeRequest *info = latest(SEXOS_DOCUMENT_INFO); uint32_t info_seq = info->document.seq;
    MUST(!document_client_transport_step(&client, &transport, &event));
    MUST(request_count == 1 && document_client_busy(&client));
    complete(info, true, 7);
    MUST(!document_client_transport_step(&client, &transport, &event));
    FakeRequest *load = latest(SEXOS_DOCUMENT_LOAD); MUST(load->document.a == 7);
    complete(load, true, 0);
    MUST(!document_client_transport_step(&client, &transport, &event));
    for (size_t word = 0; word < 4; word++) {
        FakeRequest *name = latest(SEXOS_DOCUMENT_NAME); MUST(name->document.offset == word * 4);
        complete(name, true, word == 0 ? UINT32_C(0x70616e73) : 0);
        MUST(!document_client_transport_step(&client, &transport, &event));
    }
    FakeRequest *end = latest(SEXOS_DOCUMENT_END); complete(end, true, 0);
    MUST(document_client_transport_step(&client, &transport, &event));
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_LOADED && !document_client_busy(&client));
    MUST(client.known_valid[3] && client.known[3] == 7 && info_seq == 1);

    document_client_init(&client); document_client_transport_init(&transport);
    begin_error = SEXOS_RPC_BUSY; MUST(document_client_load(&client, 3, 10));
    MUST(!document_client_transport_step(&client, &transport, &event));
    size_t before = request_count; MUST(!document_client_transport_step(&client, &transport, &event));
    MUST(request_count == before + 1 && latest(SEXOS_DOCUMENT_INFO)->document.seq == 1);

    document_client_init(&client); document_client_transport_init(&transport);
    begin_error = SEXOS_RPC_UNSUPPORTED; MUST(document_client_load(&client, 3, 11));
    MUST(!document_client_transport_step(&client, &transport, &event));
    before = request_count; MUST(!document_client_transport_step(&client, &transport, &event));
    MUST(request_count == before + 1 && latest(SEXOS_DOCUMENT_INFO)->document.seq == 1);

    document_client_init(&client); document_client_transport_init(&transport);
    MUST(document_client_load(&client, 3, 12)); MUST(!document_client_transport_step(&client, &transport, &event));
    FakeRequest *lost = latest(SEXOS_DOCUMENT_INFO); lost->active = false;
    MUST(document_client_transport_step(&client, &transport, &event));
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_UNCERTAIN);

    document_client_init(&client); document_client_transport_init(&transport);
    MUST(document_client_load(&client, 3, 13)); MUST(!document_client_transport_step(&client, &transport, &event));
    document_client_transport_close(&client, &transport); MUST(cancellations == 1 && !document_client_load(&client, 3, 14));
    puts("document client transport controls passed"); return 0;
}
