#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_document_snapshot.h"

#define MUST(condition) do { if (!(condition)) __builtin_trap(); } while (0)

static _Alignas(4096) DocumentSnapshotWords regions[16];
static const uint32_t targets[16] = {9,16,18,19,20,21,22,23,32,33,34,35,36,37,38,39};
static size_t client_index;
static bool bad_peer;
static bool bad_region;

static uint32_t region_id(size_t index) { return index < 3 ? (uint32_t)(6 + index) : (uint32_t)(0x2000 + index - 3); }
MemoryRegionReply sexos_memory_region_info(uint64_t slot) {
    size_t index = slot == 64 ? client_index : (size_t)(slot - 64);
    if (index >= 16) return (MemoryRegionReply){.status = UINT64_MAX};
    bool writer = slot != 64;
    return (MemoryRegionReply){0, DOCUMENT_SNAPSHOT_BYTES,
        REGION_QUERY | REGION_READ | (writer ? REGION_WRITE : 0),
        bad_peer ? 99 : (writer ? targets[index] : 11), bad_region ? 99 : region_id(index)};
}
MemoryRegionReply sexos_memory_region_view(uint64_t slot, uint64_t offset,
                                           uint64_t length, uint64_t rights) {
    size_t index = slot == 64 ? client_index : (size_t)(slot - 64);
    if (index >= 16 || offset || length != DOCUMENT_SNAPSHOT_BYTES) return (MemoryRegionReply){.status = UINT64_MAX};
    uint64_t expected = REGION_QUERY | REGION_READ | (slot == 64 ? 0 : REGION_WRITE);
    if (rights != expected) return (MemoryRegionReply){.status = UINT64_MAX};
    return (MemoryRegionReply){0, (uint64_t)(uintptr_t)&regions[index], length, 0, region_id(index)};
}

int main(void) {
    memset(regions, 0, sizeof regions); client_index = 2;
    DocumentSnapshotRegion writer, reader;
    MUST(document_snapshot_resolve(18, true, &writer));
    MUST(document_snapshot_resolve(18, false, &reader));
    DocumentSnapshotRegion discovered;
    MUST(document_snapshot_resolve_client(&discovered));
    MUST(discovered.subject == 18 && discovered.address == reader.address && !discovered.writer);
    MUST(writer.writer && !reader.writer && writer.address == reader.address);
    bad_peer = true; MUST(!document_snapshot_resolve(18, true, &writer)); bad_peer = false;
    bad_region = true; MUST(!document_snapshot_resolve(18, true, &writer)); bad_region = false;
    MUST(document_snapshot_resolve(18, true, &writer));

    DocumentName name = {0}; memcpy(name.bytes, "notes.bin", 9);
    SexosDocumentPayload payload = {0};
    for (size_t i = 0; i < 513; i++) payload.bytes[i] = (uint8_t)(i * 37 + 0x80);
    payload.len = 513; uint32_t error = 0;
    SexosDocumentRequest request = {7, SEXOS_DOCUMENT_LOAD, 3, 0, 9, 1};
    regions[2].words[0] = 1;
    MUST(!document_snapshot_publish(&writer, request, 9, &name, &payload, &error));
    MUST(error == SEXOS_DOCUMENT_BUSY); regions[2].words[0] = 0;
    MUST(document_snapshot_publish(&writer, request, 9, &name, &payload, &error));
    MUST(regions[2].words[0] == 14 && regions[2].words[1] == DOCUMENT_SNAPSHOT_MAGIC);
    DocumentName copied_name = {0}; SexosDocumentPayload copied = {0};
    MUST(document_snapshot_copy(&reader, 3, 9, 7, 513, &copied_name, &copied, &error));
    MUST(copied.len == payload.len && !memcmp(copied.bytes, payload.bytes, payload.len));
    MUST(!memcmp(copied_name.bytes, name.bytes, 16));
    MUST(!document_snapshot_copy(&reader, 3, 8, 7, 513, &copied_name, &copied, &error));
    MUST(copied.len == 0 && copied.bytes[0] == 0);

    MUST(!document_snapshot_publish(&writer, request, 9, &name, &payload, &error));
    MUST(error == SEXOS_DOCUMENT_STALE);
    regions[2].words[DOCUMENT_SNAPSHOT_DATA_WORD] ^= 1;
    memset(&copied, 0xaa, sizeof copied); memset(&copied_name, 0xaa, sizeof copied_name);
    MUST(!document_snapshot_copy(&reader, 3, 9, 7, 513, &copied_name, &copied, &error));
    MUST(copied.len == 0 && copied.bytes[0] == 0 && copied_name.bytes[0] == 0);
    MUST(document_snapshot_clear(&writer, &error));
    for (size_t i = 0; i < DOCUMENT_SNAPSHOT_WORDS; i++) MUST(regions[2].words[i] == 0);
    MUST(!document_snapshot_clear(&reader, &error) && error == SEXOS_DOCUMENT_DENIED);
    puts("document snapshot controls passed"); return 0;
}
