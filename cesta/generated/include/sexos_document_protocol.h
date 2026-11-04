#ifndef SEXOS_DOCUMENT_PROTOCOL_H
#define SEXOS_DOCUMENT_PROTOCOL_H
#include "sexos_client.h"
#define SEXOS_DOCUMENT_OP UINT64_C(0x4d)
#define SEXOS_DOCUMENT_MAX ((size_t)12288)
#define SEXOS_DOCUMENT_WINDOW ((size_t)4)
#define SEXOS_DOCUMENT_SEQ_MAX UINT32_C(0xffffff)
enum {SEXOS_DOCUMENT_INFO=1,SEXOS_DOCUMENT_LOAD=2,SEXOS_DOCUMENT_READ=3,SEXOS_DOCUMENT_NAME=4,SEXOS_DOCUMENT_END=5,SEXOS_DOCUMENT_BEGIN_SAVE=6,SEXOS_DOCUMENT_WRITE=7,SEXOS_DOCUMENT_COMMIT=8,SEXOS_DOCUMENT_CREATE=9,SEXOS_DOCUMENT_ABORT=10,SEXOS_DOCUMENT_DELETE=11};
enum {SEXOS_DOCUMENT_INVALID=1,SEXOS_DOCUMENT_DENIED=2,SEXOS_DOCUMENT_STALE=3,SEXOS_DOCUMENT_BUSY=5,SEXOS_DOCUMENT_TIMEOUT=8,SEXOS_DOCUMENT_IO=9,SEXOS_DOCUMENT_EXISTS=10};
typedef struct SexosDocumentRequest {uint32_t seq;uint8_t kind,object;uint32_t offset;uint64_t a,b;} SexosDocumentRequest;
typedef struct SexosDocumentDecode {bool ok;SexosDocumentRequest request;} SexosDocumentDecode;
typedef struct SexosDocumentReply {bool ok,success;uint32_t value;} SexosDocumentReply;
typedef struct SexosDocumentPayload {uint8_t bytes[SEXOS_DOCUMENT_MAX];size_t len;} SexosDocumentPayload;
uint64_t sexos_document_header(SexosDocumentRequest request);
SexosDocumentDecode sexos_document_decode(uint64_t header,uint64_t a,uint64_t b);
uint64_t sexos_document_reply(uint32_t seq,bool success,uint32_t value);
SexosDocumentReply sexos_document_decode_reply(uint64_t value,uint32_t seq);
uint64_t sexos_document_hash(const SexosDocumentPayload *payload);
#endif
