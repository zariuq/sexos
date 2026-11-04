#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_document_client.h"

#define MUST(condition) do { if (!(condition)) __builtin_trap(); } while (0)

static bool snapshot_copy_enabled;
DocumentSnapshotCopyResult document_snapshot_copy_value(DocumentSnapshotRegion region, uint8_t object,
                                                         uint32_t generation, uint32_t sequence, size_t length) {
    (void)region; (void)object; (void)generation; (void)sequence;
    DocumentSnapshotCopyResult result = {0};
    if (!snapshot_copy_enabled || length != 5) { result.error = SEXOS_DOCUMENT_STALE; return result; }
    memcpy(result.name.bytes, "snap", 4); memcpy(result.payload.bytes, "hello", 5);
    result.payload.len = 5; result.ok = true; return result;
}

static DocumentClientAction send_kind(DocumentClient *client, uint8_t kind) {
    DocumentClientEvent event = {0};
    DocumentClientAction action = document_client_action(client, &event);
    MUST(!event.present && action.present && action.request.kind == kind);
    MUST(document_client_sent(client, action.request.seq, true, false, &event));
    MUST(!event.present);
    return action;
}

static DocumentClientEvent answer(DocumentClient *client, DocumentClientAction action,
                                  bool success, uint32_t value) {
    DocumentClientEvent event = {0};
    MUST(document_client_reply(client, SEXOS_DOCUMENT_SERVER_PD,
                               sexos_document_reply(action.request.seq, success, value), &event));
    return event;
}

static uint32_t word4(const char *bytes) {
    return (uint32_t)(uint8_t)bytes[0] | ((uint32_t)(uint8_t)bytes[1] << 8) |
           ((uint32_t)(uint8_t)bytes[2] << 16) | ((uint32_t)(uint8_t)bytes[3] << 24);
}

static void load_name(DocumentClient *client, const char name[16]) {
    for (size_t offset = 0; offset < 16; offset += 4) {
        DocumentClientAction action = send_kind(client, SEXOS_DOCUMENT_NAME);
        MUST(action.request.offset == offset);
        MUST(!answer(client, action, true, word4(name + offset)).present);
    }
}

static void establish_loaded(DocumentClient *client) {
    static const char name[16] = "notes.txt";
    MUST(document_client_load(client, 3, 9));
    DocumentClientAction info = send_kind(client, SEXOS_DOCUMENT_INFO);
    DocumentClientEvent ignored = {0};
    MUST(!document_client_reply(client, 10, sexos_document_reply(info.request.seq, true, 42), &ignored));
    MUST(!document_client_reply(client, SEXOS_DOCUMENT_SERVER_PD,
                                sexos_document_reply(info.request.seq + 1, true, 42), &ignored));
    MUST(!answer(client, info, true, 42).present);
    DocumentClientAction load = send_kind(client, SEXOS_DOCUMENT_LOAD);
    MUST(load.request.a == 42 && load.request.object == 3);
    MUST(!answer(client, load, true, 5).present);
    load_name(client, name);

    DocumentClientAction first = send_kind(client, SEXOS_DOCUMENT_READ);
    DocumentClientAction second = send_kind(client, SEXOS_DOCUMENT_READ);
    MUST(first.request.offset == 0 && first.request.a == 4);
    MUST(second.request.offset == 4 && second.request.a == 1);
    MUST(!answer(client, second, true, (uint32_t)'o').present);
    MUST(!answer(client, first, true, word4("hell")).present);
    DocumentClientAction end = send_kind(client, SEXOS_DOCUMENT_END);
    DocumentClientEvent event = answer(client, end, true, 0);
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_LOADED && event.revision == 9);
    MUST(client->payload.len == 5 && !memcmp(client->payload.bytes, "hello", 5));
    MUST(!memcmp(client->name.bytes, name, 16) && client->known_valid[3] && client->known[3] == 42);
}

int main(void) {
    DocumentClient client;
    document_client_init(&client);
    MUST(document_client_status(&client) == DOCUMENT_STATUS_READY);
    MUST(!document_client_load(&client, 1, 0));
    MUST(!document_client_load(&client, 15, 0));
    establish_loaded(&client);

    document_client_init(&client); snapshot_copy_enabled = true;
    DocumentSnapshotWords words = {0};
    MUST(document_client_set_snapshot(&client, (DocumentSnapshotRegion){(uint64_t)(uintptr_t)&words, 9, false}));
    MUST(!document_client_set_snapshot(&client, (DocumentSnapshotRegion){(uint64_t)(uintptr_t)&words, 9, false}));
    MUST(document_client_load(&client, 3, 9));
    DocumentClientAction snap_info = send_kind(&client, SEXOS_DOCUMENT_INFO);
    MUST(!answer(&client, snap_info, true, 42).present);
    DocumentClientAction snap_load = send_kind(&client, SEXOS_DOCUMENT_LOAD);
    MUST(snap_load.request.b == 1);
    MUST(!answer(&client, snap_load, true, 5).present);
    DocumentClientAction snap_end = send_kind(&client, SEXOS_DOCUMENT_END);
    DocumentClientEvent snap_event = answer(&client, snap_end, true, 0);
    MUST(snap_event.present && snap_event.outcome == DOCUMENT_OUTCOME_LOADED);
    MUST(client.payload.len == 5 && !memcmp(client.payload.bytes, "hello", 5));
    snapshot_copy_enabled = false;

    static const uint8_t name[16] = "notes.txt";
    static const uint8_t bytes[5] = { 'w', 'o', 'r', 'l', 'd' };
    MUST(document_client_save(&client, true, 3, name, bytes, sizeof bytes, 10));
    DocumentClientAction info = send_kind(&client, SEXOS_DOCUMENT_INFO);
    DocumentClientEvent event = answer(&client, info, true, 42);
    MUST(!event.present);
    DocumentClientAction begin = send_kind(&client, SEXOS_DOCUMENT_BEGIN_SAVE);
    MUST(begin.request.a == 42 && begin.request.b == 5);
    MUST(!answer(&client, begin, true, 0).present);
    DocumentClientAction write = send_kind(&client, SEXOS_DOCUMENT_WRITE);
    MUST(write.request.offset == 0 && write.request.a == UINT64_C(0x646c726f77));
    MUST(!answer(&client, write, true, 5).present);
    DocumentClientAction commit = send_kind(&client, SEXOS_DOCUMENT_COMMIT);
    MUST(commit.request.a == sexos_document_hash(&client.payload));
    MUST(!answer(&client, commit, true, 5).present);
    DocumentClientAction end = send_kind(&client, SEXOS_DOCUMENT_END);
    event = answer(&client, end, true, 0);
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_SAVED && event.revision == 10);

    MUST(document_client_delete(&client, 3, 11));
    DocumentClientAction deletion = send_kind(&client, SEXOS_DOCUMENT_DELETE);
    MUST(deletion.request.a == 42);
    event = answer(&client, deletion, true, 0);
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_DELETED && !client.known_valid[3]);
    MUST(!document_client_delete(&client, 3, 12));

    document_client_init(&client);
    MUST(document_client_save(&client, false, 0, name, NULL, 0, 20));
    DocumentClientAction create = send_kind(&client, SEXOS_DOCUMENT_CREATE);
    MUST(!answer(&client, create, true, 4).present);
    info = send_kind(&client, SEXOS_DOCUMENT_INFO);
    MUST(!answer(&client, info, true, 51).present);
    begin = send_kind(&client, SEXOS_DOCUMENT_BEGIN_SAVE);
    MUST(!answer(&client, begin, true, 0).present);
    commit = send_kind(&client, SEXOS_DOCUMENT_COMMIT);
    MUST(commit.request.a == UINT64_C(0xcbf29ce484222325));
    MUST(!answer(&client, commit, true, 0).present);
    end = send_kind(&client, SEXOS_DOCUMENT_END);
    event = answer(&client, end, true, 0);
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_SAVED && event.object == 4);

    MUST(document_client_load(&client, 4, 21));
    info = send_kind(&client, SEXOS_DOCUMENT_INFO);
    MUST(document_client_cancel(&client));
    DocumentClientAction abort = send_kind(&client, SEXOS_DOCUMENT_ABORT);
    MUST(abort.request.object == 0 && abort.request.seq == info.request.seq + 1);
    MUST(!document_client_reply(&client, SEXOS_DOCUMENT_SERVER_PD,
                                sexos_document_reply(info.request.seq, true, 51), &event));
    event = answer(&client, abort, true, 0);
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_CANCELLED);
    MUST(document_client_status(&client) == DOCUMENT_STATUS_REOPEN);

    document_client_init(&client);
    MUST(document_client_load(&client, 3, 30));
    info = document_client_action(&client, &event);
    MUST(info.present);
    MUST(document_client_sent(&client, info.request.seq, false, true, &event));
    DocumentClientAction retry = document_client_action(&client, &event);
    MUST(retry.present && retry.request.seq == info.request.seq);
    MUST(document_client_sent(&client, retry.request.seq, false, false, &event));
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_UNCERTAIN);

    document_client_init(&client);
    establish_loaded(&client);
    MUST(document_client_save(&client, true, 3, name, bytes, sizeof bytes, 31));
    info = send_kind(&client, SEXOS_DOCUMENT_INFO);
    event = answer(&client, info, true, 43);
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_UNCERTAIN && event.error == SEXOS_DOCUMENT_STALE);

    document_client_init(&client);
    client.known_valid[3] = true;
    client.known[3] = 42;
    MUST(document_client_save(&client, true, 3, name, bytes, sizeof bytes, 32));
    info = send_kind(&client, SEXOS_DOCUMENT_INFO);
    MUST(!answer(&client, info, true, 42).present);
    begin = send_kind(&client, SEXOS_DOCUMENT_BEGIN_SAVE);
    MUST(!answer(&client, begin, true, 0).present);
    write = send_kind(&client, SEXOS_DOCUMENT_WRITE);
    event = answer(&client, write, true, 4);
    MUST(event.present && event.outcome == DOCUMENT_OUTCOME_UNCERTAIN && event.error == SEXOS_DOCUMENT_INVALID);

    document_client_init(&client);
    client.sequence = SEXOS_DOCUMENT_SEQ_MAX - 1;
    MUST(document_client_load(&client, 3, 40));
    DocumentClientAction none = document_client_action(&client, &event);
    MUST(!none.present && event.present && event.outcome == DOCUMENT_OUTCOME_UNCERTAIN);

    document_client_init(&client);
    MUST(document_client_load(&client, 3, 41));
    client.stall = SEXOS_DOCUMENT_STALL_LIMIT - 1;
    none = document_client_action(&client, &event);
    MUST(!none.present && event.present && event.error == SEXOS_DOCUMENT_TIMEOUT);

    document_client_init(&client);
    document_client_close(&client);
    MUST(!document_client_load(&client, 3, 1) && !document_client_cancel(&client));
    puts("document client controls passed");
    return 0;
}
