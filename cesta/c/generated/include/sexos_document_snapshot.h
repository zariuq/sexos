#ifndef SEXOS_DOCUMENT_SNAPSHOT_H
#define SEXOS_DOCUMENT_SNAPSHOT_H
#include "sexos_document_service.h"
#include "sexos_memory_regions.h"
#define DOCUMENT_SNAPSHOT_BYTES UINT64_C(16384)
#define DOCUMENT_SNAPSHOT_WORDS ((size_t)2048)
#define DOCUMENT_SNAPSHOT_DATA_WORD ((size_t)64)
#define DOCUMENT_SNAPSHOT_MAGIC UINT64_C(0x3150414e53514553)
typedef struct DocumentSnapshotWords {uint64_t words[DOCUMENT_SNAPSHOT_WORDS];} DocumentSnapshotWords;
typedef struct DocumentSnapshotRegion {uint64_t address;uint32_t subject;bool writer;} DocumentSnapshotRegion;
typedef struct DocumentSnapshotCopyResult {bool ok;uint32_t error;DocumentName name;SexosDocumentPayload payload;} DocumentSnapshotCopyResult;
bool document_snapshot_resolve(uint32_t subject,bool writer,DocumentSnapshotRegion *out);
bool document_snapshot_resolve_client(DocumentSnapshotRegion *out);
bool document_snapshot_clear(DocumentSnapshotRegion *region,uint32_t *error);
bool document_snapshot_publish(DocumentSnapshotRegion *region,SexosDocumentRequest request,uint32_t generation,const DocumentName *name,const SexosDocumentPayload *payload,uint32_t *error);
bool document_snapshot_copy(const DocumentSnapshotRegion *region,uint8_t object,uint32_t generation,uint32_t sequence,size_t length,DocumentName *name,SexosDocumentPayload *payload,uint32_t *error);
DocumentSnapshotCopyResult document_snapshot_copy_value(DocumentSnapshotRegion region,uint8_t object,uint32_t generation,uint32_t sequence,size_t length);
#endif
