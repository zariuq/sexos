
#ifndef SEXOS_DOCUMENT_CLIENT_TRANSPORT_H
#define SEXOS_DOCUMENT_CLIENT_TRANSPORT_H
#include "sexos_document_client.h"
#include "sexos_rpc.h"
#define DOCUMENT_CLIENT_TRANSPORT_SLOTS ((size_t)5)

typedef struct DocumentClientTransportEntry {
    bool present;
    uint32_t sequence;
    uint64_t request_id;
} DocumentClientTransportEntry;

typedef struct DocumentClientTransport {
    DocumentClientTransportEntry entries[DOCUMENT_CLIENT_TRANSPORT_SLOTS];
} DocumentClientTransport;

void document_client_transport_init(DocumentClientTransport *state);
bool document_client_transport_step(DocumentClient *client, DocumentClientTransport *transport,
                                    DocumentClientEvent *event);
bool document_client_transport_pending(const DocumentClientTransport *transport);
void document_client_transport_close(DocumentClient *client, DocumentClientTransport *transport);
#endif
