#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_document_service.h"

#define MUST(condition) do { if (!(condition)) __builtin_trap(); } while (0)

typedef struct Object {
    bool present;
    uint32_t generation;
    DocumentName name;
    SexosDocumentPayload payload;
} Object;

static Object objects[15];
static unsigned identity_calls, create_calls, save_calls, delete_calls, publish_calls, cancel_calls, clear_calls;
static bool mutate_during_load;

static void backend_init(void) {
    memset(objects, 0, sizeof objects);
    objects[2].present = true; objects[2].generation = 1;
    memcpy(objects[2].name.bytes, "system", 6);
    objects[3].present = true; objects[3].generation = 7;
    memcpy(objects[3].name.bytes, "notes.txt", 9);
    memcpy(objects[3].payload.bytes, "hello", 5); objects[3].payload.len = 5;
    identity_calls = create_calls = save_calls = delete_calls = publish_calls = cancel_calls = clear_calls = 0;
    mutate_during_load = false;
}

bool document_backend_identity(uint8_t object, uint32_t *generation, uint32_t *error) {
    identity_calls++;
    if (object >= 15 || !objects[object].present) { *error = SEXOS_DOCUMENT_STALE; return false; }
    *generation = objects[object].generation; return true;
}
uint8_t document_backend_load_step(uint32_t caller, uint8_t object, SexosDocumentPayload *payload,
                                   DocumentName *name, uint32_t *error) {
    (void)caller; (void)error;
    if (object >= 15 || !objects[object].present) return DOCUMENT_BACKEND_ERROR;
    *payload = objects[object].payload; *name = objects[object].name;
    if (mutate_during_load) objects[object].generation++;
    return DOCUMENT_BACKEND_DONE;
}
bool document_backend_publish_load(uint32_t caller, SexosDocumentRequest request,
                                   uint32_t generation, const SexosDocumentPayload *payload,
                                   const DocumentName *name, uint32_t *error) {
    (void)caller; (void)request; (void)generation; (void)payload; (void)name; (void)error;
    publish_calls++; return true;
}
bool document_backend_name(uint8_t object, DocumentName *name, uint32_t *error) {
    if (object >= 15 || !objects[object].present) { *error = SEXOS_DOCUMENT_STALE; return false; }
    *name = objects[object].name; return true;
}
bool document_backend_create(const DocumentName *name, bool fresh_only, uint8_t *object,
                             uint32_t *generation, uint32_t *error) {
    (void)fresh_only; create_calls++;
    for (size_t i = 3; i < 15; i++) if (objects[i].present &&
        !memcmp(objects[i].name.bytes, name->bytes, 16)) { *error = SEXOS_DOCUMENT_EXISTS; return false; }
    for (size_t i = 3; i < 15; i++) if (!objects[i].present) {
        objects[i].present = true; objects[i].generation++;
        if (!objects[i].generation) objects[i].generation = 1;
        objects[i].name = *name; objects[i].payload.len = 0;
        *object = (uint8_t)i; *generation = objects[i].generation; return true;
    }
    *error = SEXOS_DOCUMENT_BUSY; return false;
}
uint8_t document_backend_save_step(uint32_t caller, uint8_t object,
                                   const SexosDocumentPayload *payload, uint32_t *error) {
    (void)caller;
    if (object >= 15 || !objects[object].present) { *error = SEXOS_DOCUMENT_STALE; return DOCUMENT_BACKEND_ERROR; }
    save_calls++; objects[object].payload = *payload; return DOCUMENT_BACKEND_DONE;
}
bool document_backend_delete(uint8_t object, uint32_t *error) {
    if (object < 3 || object >= 15 || !objects[object].present) { *error = SEXOS_DOCUMENT_DENIED; return false; }
    delete_calls++; objects[object].present = false; objects[object].generation++; return true;
}
void document_backend_cancel_work(uint32_t caller) { (void)caller; cancel_calls++; }
bool document_backend_clear_load(uint32_t caller, uint32_t *error) { (void)caller; (void)error; clear_calls++; return true; }

static SexosDocumentReply call(DocumentService *service, uint32_t caller, bool live,
                               SexosDocumentRequest request) {
    DocumentServiceResult result = document_service_handle(service, caller, live, request);
    if (!result.answered) {
        DocumentServiceCompletion completion = {0};
        for (unsigned i = 0; i < 100 && !completion.present; i++) completion = document_service_progress(service);
        MUST(completion.present && completion.caller == caller &&
             completion.request.seq == request.seq);
        result = (DocumentServiceResult){true, completion.value};
    }
    SexosDocumentReply reply = sexos_document_decode_reply(result.value, request.seq);
    MUST(reply.ok); return reply;
}

static SexosDocumentRequest request(uint32_t seq, uint8_t kind, uint8_t object,
                                    uint32_t offset, uint64_t a, uint64_t b) {
    return (SexosDocumentRequest){seq, kind, object, offset, a, b};
}

static void pack_name(const char *text, uint64_t *lo, uint64_t *hi) {
    uint8_t bytes[16] = {0}; size_t len = strlen(text); if (len > 16) len = 16;
    memcpy(bytes, text, len); *lo = *hi = 0;
    for (size_t i = 0; i < 8; i++) { *lo |= (uint64_t)bytes[i] << (i * 8); *hi |= (uint64_t)bytes[i + 8] << (i * 8); }
}

int main(void) {
    backend_init(); DocumentService service; document_service_init(&service);
    SexosDocumentReply reply = call(&service, 99, true, request(1, SEXOS_DOCUMENT_INFO, 3, 0, 0, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_DENIED);
    reply = call(&service, 9, false, request(1, SEXOS_DOCUMENT_INFO, 3, 0, 0, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_DENIED);

    SexosDocumentRequest info = request(1, SEXOS_DOCUMENT_INFO, 3, 0, 0, 0);
    reply = call(&service, 9, true, info); MUST(reply.success && reply.value == 7);
    unsigned identities = identity_calls;
    reply = call(&service, 9, true, info); MUST(reply.success && reply.value == 7 && identity_calls == identities);
    reply = call(&service, 9, true, request(1, SEXOS_DOCUMENT_INFO, 2, 0, 0, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_STALE);

    SexosDocumentRequest load = request(2, SEXOS_DOCUMENT_LOAD, 3, 0, 7, 1);
    reply = call(&service, 9, true, load); MUST(reply.success && reply.value == 5 && publish_calls == 1);
    reply = call(&service, 9, true, request(3, SEXOS_DOCUMENT_NAME, 3, 0, 0, 0));
    MUST(reply.success && reply.value == UINT32_C(0x65746f6e));
    reply = call(&service, 9, true, request(4, SEXOS_DOCUMENT_READ, 3, 1, 4, 0));
    MUST(reply.success && reply.value == UINT32_C(0x6f6c6c65));
    reply = call(&service, 9, true, request(5, SEXOS_DOCUMENT_END, 3, 0, 0, 0));
    MUST(reply.success && reply.value == 0 && cancel_calls == 2 && clear_calls == 1);

    backend_init(); document_service_init(&service);
    reply = call(&service, 9, true, request(1, SEXOS_DOCUMENT_BEGIN_SAVE, 3, 0, 6, 5));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_STALE);
    reply = call(&service, 9, true, request(2, SEXOS_DOCUMENT_BEGIN_SAVE, 3, 0, 7, 5));
    MUST(reply.success);
    uint64_t lo = UINT64_C(0x646c726f77);
    reply = call(&service, 9, true, request(3, SEXOS_DOCUMENT_WRITE, 3, 0, lo, 0));
    MUST(reply.success && reply.value == 5);
    SexosDocumentPayload payload = { .bytes = { 'w','o','r','l','d' }, .len = 5 };
    reply = call(&service, 9, true, request(4, SEXOS_DOCUMENT_COMMIT, 3, 0, 1, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_INVALID && save_calls == 0);
    SexosDocumentRequest commit = request(5, SEXOS_DOCUMENT_COMMIT, 3, 0,
                                          sexos_document_hash(&payload), 0);
    reply = call(&service, 9, true, commit); MUST(reply.success && reply.value == 5 && save_calls == 1);
    reply = call(&service, 9, true, commit); MUST(reply.success && save_calls == 1);
    reply = call(&service, 9, true, request(6, SEXOS_DOCUMENT_END, 3, 0, 0, 0)); MUST(reply.success);
    MUST(objects[3].payload.len == 5 && !memcmp(objects[3].payload.bytes, "world", 5));

    backend_init(); document_service_init(&service);
    reply = call(&service, 9, true, request(1, SEXOS_DOCUMENT_DELETE, 3, 0, 6, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_STALE && delete_calls == 0);
    SexosDocumentRequest deletion = request(2, SEXOS_DOCUMENT_DELETE, 3, 0, 7, 0);
    reply = call(&service, 9, true, deletion); MUST(reply.success && delete_calls == 1);
    reply = call(&service, 9, true, deletion); MUST(reply.success && delete_calls == 1);
    reply = call(&service, 9, true, request(2, SEXOS_DOCUMENT_DELETE, 4, 0, 7, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_STALE);

    backend_init(); document_service_init(&service);
    uint64_t name_lo, name_hi; pack_name("bad name", &name_lo, &name_hi);
    reply = call(&service, 15, true, request(1, SEXOS_DOCUMENT_CREATE, 0, 0, name_lo, name_hi));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_INVALID && create_calls == 0);
    pack_name("import.txt", &name_lo, &name_hi);
    SexosDocumentRequest create = request(2, SEXOS_DOCUMENT_CREATE, 0, 0, name_lo, name_hi);
    reply = call(&service, 15, true, create); MUST(reply.success && reply.value == 4 && create_calls == 1);
    reply = call(&service, 15, true, create); MUST(reply.success && reply.value == 4 && create_calls == 1);
    reply = call(&service, 15, true, request(3, SEXOS_DOCUMENT_LOAD, 4, 0, 1, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_DENIED);
    reply = call(&service, 15, true, request(4, SEXOS_DOCUMENT_INFO, 4, 0, 0, 0)); MUST(reply.success && reply.value == 1);
    reply = call(&service, 15, true, request(5, SEXOS_DOCUMENT_BEGIN_SAVE, 4, 0, 1, 0)); MUST(reply.success);
    reply = call(&service, 15, true, request(6, SEXOS_DOCUMENT_COMMIT, 4, 0,
                                            UINT64_C(0xcbf29ce484222325), 0)); MUST(reply.success);
    reply = call(&service, 15, true, request(7, SEXOS_DOCUMENT_END, 4, 0, 0, 0)); MUST(reply.success);
    MUST(document_service_browser_scrubbed(&service));

    backend_init(); document_service_init(&service); mutate_during_load = true;
    reply = call(&service, 9, true, request(1, SEXOS_DOCUMENT_LOAD, 3, 0, 7, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_STALE);

    backend_init(); document_service_init(&service);
    MUST(document_service_retire(&service, 9));
    reply = call(&service, 9, true, request(1, SEXOS_DOCUMENT_INFO, 3, 0, 0, 0));
    MUST(!reply.success && reply.value == SEXOS_DOCUMENT_DENIED);
    puts("document service controls passed"); return 0;
}
