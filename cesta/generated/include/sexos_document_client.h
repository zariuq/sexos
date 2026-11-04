#ifndef SEXOS_DOCUMENT_CLIENT_H
#define SEXOS_DOCUMENT_CLIENT_H
#include "sexos_document_snapshot.h"
#define SEXOS_DOCUMENT_SERVER_PD UINT32_C(11)
#define SEXOS_DOCUMENT_STALL_LIMIT UINT32_C(1048576)
#define SEXOS_DOCUMENT_TOTAL_LIMIT UINT32_C(16777216)
enum {DOCUMENT_PHASE_INFO=1,DOCUMENT_PHASE_LOAD=2,DOCUMENT_PHASE_NAME=3,DOCUMENT_PHASE_READ=4,DOCUMENT_PHASE_END=5,DOCUMENT_PHASE_ABORT=6,DOCUMENT_PHASE_CREATE=7,DOCUMENT_PHASE_BEGIN_SAVE=8,DOCUMENT_PHASE_WRITE=9,DOCUMENT_PHASE_COMMIT=10,DOCUMENT_PHASE_DELETE=11};
enum {DOCUMENT_OUTCOME_NONE=0,DOCUMENT_OUTCOME_LOADED=1,DOCUMENT_OUTCOME_SAVED=2,DOCUMENT_OUTCOME_DELETED=3,DOCUMENT_OUTCOME_FAILED=4,DOCUMENT_OUTCOME_UNCERTAIN=5,DOCUMENT_OUTCOME_CANCELLED=6};
enum {DOCUMENT_STATUS_READY=0,DOCUMENT_STATUS_LOADING=1,DOCUMENT_STATUS_SAVING=2,DOCUMENT_STATUS_ERROR=3,DOCUMENT_STATUS_UNCERTAIN=4,DOCUMENT_STATUS_REOPEN=5};
typedef struct DocumentDataRequest {bool present,enqueued;SexosDocumentRequest request;size_t count;} DocumentDataRequest;
typedef struct DocumentClientEvent {bool present;uint8_t outcome,object;uint64_t revision;uint32_t error;} DocumentClientEvent;
typedef struct DocumentClientAction {bool present;SexosDocumentRequest request;} DocumentClientAction;
typedef struct DocumentClient {
 SexosDocumentPayload payload;DocumentName name;bool known_valid[15];uint32_t known[15];
 DocumentDataRequest window[SEXOS_DOCUMENT_WINDOW];SexosDocumentRequest request;
 size_t offset,issued,completed;uint64_t revision;uint32_t generation,sequence,stall,total,error;
 uint8_t object,phase;bool request_present,awaiting,busy,fenced,dead,saving,creating,quiescent,snapshot_present;DocumentSnapshotRegion snapshot;
} DocumentClient;
void document_client_init(DocumentClient *state);
bool document_client_busy(const DocumentClient *state);
bool document_client_loading(const DocumentClient *state);
uint8_t document_client_status(const DocumentClient *state);
bool document_client_set_snapshot(DocumentClient *state,DocumentSnapshotRegion snapshot);
bool document_client_load(DocumentClient *state,uint8_t object,uint64_t revision);
bool document_client_save(DocumentClient *state,bool has_object,uint8_t object,const uint8_t name[16],const uint8_t *bytes,size_t len,uint64_t revision);
bool document_client_delete(DocumentClient *state,uint8_t object,uint64_t revision);
bool document_client_cancel(DocumentClient *state);
void document_client_close(DocumentClient *state);
DocumentClientAction document_client_action(DocumentClient *state,DocumentClientEvent *event);
bool document_client_sent(DocumentClient *state,uint32_t sequence,bool accepted,bool retryable,DocumentClientEvent *event);
bool document_client_reply(DocumentClient *state,uint32_t caller,uint64_t value,DocumentClientEvent *event);
bool document_client_transport_lost(DocumentClient *state,uint32_t sequence,uint32_t error,DocumentClientEvent *event);

void *memcpy(void *destination, const void *source, size_t count);
void *memset(void *destination, int value, size_t count);
#endif
