#ifndef SEXOS_DOCUMENT_SERVICE_H
#define SEXOS_DOCUMENT_SERVICE_H
#include "sexos_document_protocol.h"
#define SEXOS_DOCUMENT_SESSION_COUNT ((size_t)19)
enum {DOCUMENT_BACKEND_PENDING=0,DOCUMENT_BACKEND_DONE=1,DOCUMENT_BACKEND_ERROR=2};
typedef struct DocumentServiceResult {bool answered;uint64_t value;} DocumentServiceResult;
typedef struct DocumentServiceCompletion {bool present;uint32_t caller;SexosDocumentRequest request;uint64_t value;} DocumentServiceCompletion;
typedef struct DocumentName {uint8_t bytes[16];} DocumentName;
typedef struct DocumentSession {
 SexosDocumentPayload payload;DocumentName name;SexosDocumentRequest last;uint64_t receipt;
 size_t written;uint32_t generation;uint8_t object;
 bool bound,saving,last_present,dead,created_present,load_pending,commit_pending;uint8_t created_object;uint32_t created_generation;SexosDocumentRequest pending_request;
} DocumentSession;
typedef struct DocumentService {DocumentSession sessions[SEXOS_DOCUMENT_SESSION_COUNT];} DocumentService;
void document_service_init(DocumentService *state);
bool document_service_retire(DocumentService *state,uint32_t caller);
bool document_service_browser_scrubbed(const DocumentService *state);
DocumentServiceResult document_service_handle(DocumentService *state,uint32_t caller,bool live,SexosDocumentRequest request);
DocumentServiceCompletion document_service_progress(DocumentService *state);
bool document_backend_identity(uint8_t object,uint32_t *generation,uint32_t *error);
uint8_t document_backend_load_step(uint32_t caller,uint8_t object,SexosDocumentPayload *payload,DocumentName *name,uint32_t *error);
bool document_backend_publish_load(uint32_t caller,SexosDocumentRequest request,uint32_t generation,const SexosDocumentPayload *payload,const DocumentName *name,uint32_t *error);
bool document_backend_name(uint8_t object,DocumentName *name,uint32_t *error);
bool document_backend_create(const DocumentName *name,bool fresh_only,uint8_t *object,uint32_t *generation,uint32_t *error);
uint8_t document_backend_save_step(uint32_t caller,uint8_t object,const SexosDocumentPayload *payload,uint32_t *error);
bool document_backend_delete(uint8_t object,uint32_t *error);
void document_backend_cancel_work(uint32_t caller);
bool document_backend_clear_load(uint32_t caller,uint32_t *error);

void *memcpy(void *destination, const void *source, size_t count);
void *memset(void *destination, int value, size_t count);
#endif
