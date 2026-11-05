
#ifndef SEXOS_DOCUMENT_SERVICE_TRANSPORT_H
#define SEXOS_DOCUMENT_SERVICE_TRANSPORT_H
#include "sexos_document_service.h"
#include "sexos_rpc.h"
#define DOCUMENT_SERVICE_RECEIPTS ((size_t)76)

typedef struct DocumentServiceEnvelope {
    bool present, ready;
    SexosRequest transport;
    SexosDocumentRequest document;
    uint64_t value;
} DocumentServiceEnvelope;

typedef struct DocumentServiceTransport {
    DocumentService service;
    DocumentServiceEnvelope envelopes[DOCUMENT_SERVICE_RECEIPTS];
    bool correlated_required;
} DocumentServiceTransport;

void document_service_transport_init(DocumentServiceTransport *state, bool correlated_required);
bool document_service_transport_prepare(DocumentServiceTransport *state,
                                        const SexosRequest *request, bool live);
void document_service_transport_progress(DocumentServiceTransport *state);
bool document_service_transport_poll(DocumentServiceTransport *state);
bool document_service_transport_pending(const DocumentServiceTransport *state);
bool document_service_transport_retire(DocumentServiceTransport *state, uint32_t caller);
#endif
