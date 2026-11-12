
#ifndef SEXOS_NETWORK_WEB_H
#define SEXOS_NETWORK_WEB_H
#include "network_job_machine.h"

enum {
    NETWORK_WEB_MAX_FLOWS = 4096,
    NETWORK_WEB_MAX_SOCKETS = 4096,
    NETWORK_WEB_CLIENT_FLOWS = 255,
    NETWORK_WEB_WIRE_BYTES = 1400,
    NETWORK_WEB_RESOLVE_HOLD_TICKS = 5000
};
#define NETWORK_WEB_NO_INDEX ((size_t)-1)

typedef struct NetworkWebFlow {
    bool present;
    NetworkJobIdentity owner;
    uint8_t number;
    uint16_t listening;
    uint8_t out[NETWORK_WEB_WIRE_BYTES];
    size_t out_at, out_len;
} NetworkWebFlow;

typedef struct NetworkWebSocket {
    bool present;
    NetworkJobIdentity owner;
    uint8_t number;
} NetworkWebSocket;

typedef struct NetworkWebService {
    NetworkWebFlow *flows;
    size_t flow_count, flow_capacity;
    NetworkWebSocket *sockets;
    size_t socket_count, socket_capacity;
    bool resolving_present;
    NetworkJobIdentity resolving_owner;
    uint64_t resolving_since;
} NetworkWebService;

typedef struct NetworkWebAcquire {
    size_t index;
    bool present, created;
} NetworkWebAcquire;

typedef struct NetworkWebRetirement {
    size_t flows, sockets;
    bool resolving;
} NetworkWebRetirement;

void network_web_init(NetworkWebService *service);
void network_web_dispose(NetworkWebService *service);
size_t network_web_flow_find(const NetworkWebService *service, NetworkJobIdentity owner,
                             uint8_t number);
size_t network_web_socket_find(const NetworkWebService *service, NetworkJobIdentity owner,
                               uint8_t number);
NetworkWebAcquire network_web_flow_acquire(NetworkWebService *service, NetworkJobIdentity owner,
                                           uint8_t number);
NetworkWebAcquire network_web_socket_acquire(NetworkWebService *service, NetworkJobIdentity owner,
                                             uint8_t number);
bool network_web_flow_release(NetworkWebService *service, NetworkJobIdentity owner, uint8_t number);
bool network_web_socket_release(NetworkWebService *service, NetworkJobIdentity owner, uint8_t number);
NetworkWebRetirement network_web_retire(NetworkWebService *service, NetworkJobIdentity owner);
bool network_web_resolve_begin(NetworkWebService *service, NetworkJobIdentity owner, uint64_t now);
bool network_web_resolve_finish(NetworkWebService *service, NetworkJobIdentity owner);
#endif
