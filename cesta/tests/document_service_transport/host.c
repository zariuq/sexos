#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_document_service_transport.h"

#define MUST(condition) do { if (!(condition)) __builtin_trap(); } while (0)

static uint64_t answered_id[32], answered_value[32];
static size_t answered_count;
static uint64_t busy_id;
static unsigned load_steps;

uint64_t sexos_answer_request(const SexosRequest *request, uint64_t value) {
    if (busy_id && request->id == busy_id) { busy_id = 0; return SEXOS_RPC_BUSY; }
    MUST(answered_count < 32); answered_id[answered_count] = request->id;
    answered_value[answered_count++] = value; return 0;
}
bool document_backend_identity(uint8_t object, uint32_t *generation, uint32_t *error) {
    if (object != 3) { *error = SEXOS_DOCUMENT_STALE; return false; }
    *generation = 7; return true;
}
uint8_t document_backend_load_step(uint32_t caller, uint8_t object, SexosDocumentPayload *payload,
                                   DocumentName *name, uint32_t *error) {
    (void)caller; (void)object; (void)error; load_steps++;
    if (load_steps == 1) return DOCUMENT_BACKEND_PENDING;
    memcpy(payload->bytes, "hello", 5); payload->len = 5; memcpy(name->bytes, "notes", 5);
    return DOCUMENT_BACKEND_DONE;
}
bool document_backend_publish_load(uint32_t caller, SexosDocumentRequest request,
                                   uint32_t generation, const SexosDocumentPayload *payload,
                                   const DocumentName *name, uint32_t *error) {
    (void)caller; (void)request; (void)generation; (void)payload; (void)name; (void)error; return true;
}
bool document_backend_name(uint8_t object, DocumentName *name, uint32_t *error) {
    (void)object; (void)error; memcpy(name->bytes, "notes", 5); return true;
}
bool document_backend_create(const DocumentName *name, bool fresh_only, uint8_t *object,
                             uint32_t *generation, uint32_t *error) {
    (void)name; (void)fresh_only; (void)object; (void)generation; *error = SEXOS_DOCUMENT_DENIED; return false;
}
uint8_t document_backend_save_step(uint32_t caller, uint8_t object,
                                   const SexosDocumentPayload *payload, uint32_t *error) {
    (void)caller; (void)object; (void)payload; *error = SEXOS_DOCUMENT_DENIED; return DOCUMENT_BACKEND_ERROR;
}
bool document_backend_delete(uint8_t object, uint32_t *error) {
    (void)object; *error = SEXOS_DOCUMENT_DENIED; return false;
}
void document_backend_cancel_work(uint32_t caller) { (void)caller; }
bool document_backend_clear_load(uint32_t caller, uint32_t *error) { (void)caller; (void)error; return true; }

static SexosRequest request(uint64_t id, uint32_t caller, uint32_t task,
                            SexosDocumentRequest document) {
    return (SexosRequest){{SEXOS_DOCUMENT_OP, sexos_document_header(document),
                           document.a, document.b, caller, task}, id};
}
static SexosDocumentReply answer_at(size_t index, uint32_t seq) {
    SexosDocumentReply decoded = sexos_document_decode_reply(answered_value[index], seq);
    MUST(decoded.ok); return decoded;
}

int main(void) {
    DocumentServiceTransport transport; document_service_transport_init(&transport, true);
    SexosRequest other = {0}; other.message.type_id = 99;
    MUST(!document_service_transport_prepare(&transport, &other, true));

    SexosRequest malformed = request(1, 9, 9, (SexosDocumentRequest){0});
    MUST(document_service_transport_prepare(&transport, &malformed, true));
    MUST(document_service_transport_poll(&transport));
    MUST(!answer_at(0, 0).success && answer_at(0, 0).value == SEXOS_DOCUMENT_INVALID);

    answered_count = 0; SexosDocumentRequest info = {1, SEXOS_DOCUMENT_INFO, 3, 0, 0, 0};
    SexosRequest legacy = request(0, 9, 0, info);
    MUST(document_service_transport_prepare(&transport, &legacy, true));
    document_service_transport_poll(&transport);
    MUST(!answer_at(0, 1).success && answer_at(0, 1).value == SEXOS_DOCUMENT_DENIED);

    answered_count = 0; SexosRequest wrong_task = request(2, 9, 8, info);
    MUST(document_service_transport_prepare(&transport, &wrong_task, true));
    document_service_transport_poll(&transport);
    MUST(!answer_at(0, 1).success && answer_at(0, 1).value == SEXOS_DOCUMENT_DENIED);

    document_service_transport_init(&transport, true); answered_count = 0;
    SexosRequest first = request(10, 9, 9, info); busy_id = 10;
    MUST(document_service_transport_prepare(&transport, &first, true));
    MUST(!document_service_transport_poll(&transport));
    MUST(document_service_transport_pending(&transport) && answered_count == 0);
    MUST(document_service_transport_poll(&transport));
    MUST(!document_service_transport_pending(&transport) && answered_count == 1);
    MUST(answer_at(0, 1).success && answer_at(0, 1).value == 7);

    document_service_transport_init(&transport, true); answered_count = 0; load_steps = 0;
    SexosDocumentRequest load = {1, SEXOS_DOCUMENT_LOAD, 3, 0, 7, 0};
    SexosRequest loading = request(20, 9, 9, load);
    MUST(document_service_transport_prepare(&transport, &loading, true));
    MUST(document_service_transport_pending(&transport));
    document_service_transport_progress(&transport);
    document_service_transport_poll(&transport); MUST(answered_count == 0);
    document_service_transport_progress(&transport);
    document_service_transport_poll(&transport); MUST(answered_count == 1);
    MUST(answer_at(0, 1).success && answer_at(0, 1).value == 5);

    document_service_transport_init(&transport, true); answered_count = 0; load_steps = 0;
    SexosRequest load16 = request(41, 16, 16, load);
    SexosRequest load9 = request(42, 9, 9, load);
    MUST(document_service_transport_prepare(&transport, &load16, true));
    MUST(document_service_transport_prepare(&transport, &load9, true));
    document_service_transport_progress(&transport);
    document_service_transport_progress(&transport);
    document_service_transport_poll(&transport);
    MUST(answered_count == 1 && answered_id[0] == 42);
    document_service_transport_progress(&transport);
    document_service_transport_poll(&transport);
    MUST(answered_count == 2 && answered_id[1] == 41);

    document_service_transport_init(&transport, true); answered_count = 0; load_steps = 0;
    loading = request(50, 9, 9, load); MUST(document_service_transport_prepare(&transport, &loading, true));
    transport.envelopes[1] = transport.envelopes[0];
    transport.envelopes[0].document.seq = 99;
    transport.envelopes[0].transport.id = 49;
    document_service_transport_progress(&transport);
    document_service_transport_progress(&transport);
    document_service_transport_poll(&transport);
    MUST(answered_count == 1 && answered_id[0] == 50);

    document_service_transport_init(&transport, true); answered_count = 0; load_steps = 0;
    loading = request(30, 9, 9, load); MUST(document_service_transport_prepare(&transport, &loading, true));
    MUST(document_service_transport_retire(&transport, 9));
    document_service_transport_poll(&transport); MUST(answered_count == 1);
    MUST(!answer_at(0, 1).success && answer_at(0, 1).value == SEXOS_DOCUMENT_DENIED);

    document_service_transport_init(&transport, false); answered_count = 0;
    legacy = request(0, 9, 0, info); MUST(document_service_transport_prepare(&transport, &legacy, true));
    document_service_transport_poll(&transport); MUST(answered_count == 1 && answer_at(0, 1).success);
    puts("document service transport controls passed"); return 0;
}
