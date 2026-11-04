#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_document_journal.h"

#define MUST(condition) do { if (!(condition)) __builtin_trap(); } while (0)

static uint8_t media[DOCUMENT_JOURNAL_CAPACITY];
static unsigned syncs;
static unsigned fail_sync;
static bool fail_read;
static bool corrupt_payload_write;

bool document_journal_io_read(uint8_t object, uint64_t offset,
                              DocumentJournalBlock *block, uint32_t *error) {
    if (object != 3 || fail_read || offset > sizeof media || block->len > sizeof media - offset) {
        *error = SEXOS_DOCUMENT_IO; return false;
    }
    memcpy(block->bytes, media + offset, block->len); return true;
}
bool document_journal_io_write(uint8_t object, uint64_t offset,
                               const DocumentJournalBlock *block, uint32_t *error) {
    if (object != 3 || offset > sizeof media || block->len > sizeof media - offset) {
        *error = SEXOS_DOCUMENT_IO; return false;
    }
    memcpy(media + offset, block->bytes, block->len);
    if (corrupt_payload_write && (offset % DOCUMENT_JOURNAL_STRIDE) >= DOCUMENT_JOURNAL_PAYLOAD && block->len) {
        media[offset] ^= 1; corrupt_payload_write = false;
    }
    return true;
}
bool document_journal_io_sync(uint8_t object, uint32_t *error) {
    MUST(object == 3); syncs++;
    if (fail_sync && syncs == fail_sync) { *error = SEXOS_DOCUMENT_IO; return false; }
    return true;
}

static void put32(size_t offset, uint32_t value) {
    for (size_t i = 0; i < 4; i++) media[offset + i] = (uint8_t)(value >> (i * 8));
}
static void put64(size_t offset, uint64_t value) {
    for (size_t i = 0; i < 8; i++) media[offset + i] = (uint8_t)(value >> (i * 8));
}
static uint64_t hash_bytes(const uint8_t *bytes, size_t len) {
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (size_t i = 0; i < len; i++) hash = (hash ^ bytes[i]) * UINT64_C(0x100000001b3);
    return hash;
}
static void refresh_header_checksum(size_t base) {
    put64(base + 504, hash_bytes(media + base, 504));
}
static SexosDocumentPayload payload(const char *text) {
    SexosDocumentPayload value = {0}; value.len = strlen(text);
    memcpy(value.bytes, text, value.len); return value;
}
static DocumentJournalProgress load_all(bool journal, SexosDocumentPayload *out) {
    DocumentJournalLoad state; document_journal_load_init(&state, 3, UINT32_C(0x40007), journal);
    DocumentJournalProgress progress = {0};
    for (unsigned i = 0; i < 100 && progress.status == DOCUMENT_JOURNAL_PENDING; i++)
        progress = document_journal_load_step(&state, out);
    MUST(progress.status != DOCUMENT_JOURNAL_PENDING); return progress;
}
static DocumentJournalProgress save_all(const SexosDocumentPayload *input) {
    DocumentJournalSave state; document_journal_save_init(&state, 3, UINT32_C(0x40007));
    DocumentJournalProgress progress = {0};
    for (unsigned i = 0; i < 100 && progress.status == DOCUMENT_JOURNAL_PENDING; i++)
        progress = document_journal_save_step(&state, input);
    MUST(progress.status != DOCUMENT_JOURNAL_PENDING); return progress;
}

int main(void) {
    memset(media, 0, sizeof media); memcpy(media, "QP01", 4); put32(4, 5);
    memcpy(media + 16, "hello", 5);
    SexosDocumentPayload output = {0};
    DocumentJournalProgress progress = load_all(false, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && !progress.journal && output.len == 5);
    MUST(!memcmp(output.bytes, "hello", 5));

    SexosDocumentPayload alpha = payload("alpha"); syncs = 0;
    progress = save_all(&alpha);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.receipt.bank == 1);
    MUST(progress.receipt.generation == 1 && progress.receipt.len == 5 && syncs == 3);
    memset(&output, 0xaa, sizeof output); progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.journal && progress.receipt.bank == 1);
    MUST(output.len == 5 && !memcmp(output.bytes, "alpha", 5) && output.bytes[5] == 0);

    SexosDocumentPayload beta = payload("beta-data"); syncs = 0;
    progress = save_all(&beta);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.receipt.bank == 0);
    MUST(progress.receipt.generation == 2 && syncs == 3);
    progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.receipt.generation == 2);
    MUST(output.len == beta.len && !memcmp(output.bytes, beta.bytes, beta.len));

    media[3] = 'X'; refresh_header_checksum(0);
    progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.receipt.bank == 1);
    media[3] = '2'; refresh_header_checksum(0);
    put32(8, UINT32_C(0x50007)); refresh_header_checksum(0);
    progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.receipt.bank == 1);
    put32(8, UINT32_C(0x40007)); refresh_header_checksum(0);
    media[504] ^= 1;
    progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.receipt.bank == 1);
    media[504] ^= 1;

    media[DOCUMENT_JOURNAL_PAYLOAD] ^= 0x80;
    progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && progress.receipt.bank == 1);
    MUST(output.len == 5 && !memcmp(output.bytes, "alpha", 5));
    media[DOCUMENT_JOURNAL_PAYLOAD] ^= 0x80;

    memcpy(media + DOCUMENT_JOURNAL_STRIDE, media, 512 + beta.len);
    progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_ERROR && progress.error == SEXOS_DOCUMENT_STALE);

    memset(media, 0, sizeof media); memcpy(media, "QP01", 4); put32(4, 5); memcpy(media + 16, "hello", 5);
    progress = save_all(&alpha); MUST(progress.status == DOCUMENT_JOURNAL_DONE);
    SexosDocumentPayload gamma = payload("gamma"); syncs = 0; fail_sync = 1;
    progress = save_all(&gamma); MUST(progress.status == DOCUMENT_JOURNAL_ERROR);
    fail_sync = 0; progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && !memcmp(output.bytes, "alpha", 5));

    syncs = 0; corrupt_payload_write = true; progress = save_all(&gamma);
    MUST(progress.status == DOCUMENT_JOURNAL_ERROR);
    progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_DONE && !memcmp(output.bytes, "alpha", 5));

    put64(DOCUMENT_JOURNAL_STRIDE + 16, UINT64_MAX);
    refresh_header_checksum(DOCUMENT_JOURNAL_STRIDE);
    progress = save_all(&gamma);
    MUST(progress.status == DOCUMENT_JOURNAL_ERROR && progress.error == SEXOS_DOCUMENT_STALE);

    fail_read = true; memset(&output, 0xaa, sizeof output); progress = load_all(true, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_ERROR && output.len == 0 && output.bytes[0] == 0);
    fail_read = false; memset(media, 0, sizeof media); memcpy(media, "QP0X", 4); put32(4, 1); media[16] = 'x';
    progress = load_all(false, &output);
    MUST(progress.status == DOCUMENT_JOURNAL_ERROR && progress.error == SEXOS_DOCUMENT_INVALID);
    puts("document journal controls passed"); return 0;
}
