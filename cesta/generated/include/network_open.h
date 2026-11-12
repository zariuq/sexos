
#ifndef SEXOS_NETWORK_OPEN_H
#define SEXOS_NETWORK_OPEN_H
#include "network_engine_dns.h"
#include "network_tcp_client.h"

enum {
    NETWORK_OPEN_MAX_FLOWS = 4096
};
#define NETWORK_OPEN_NO_INDEX ((size_t)-1)

typedef enum NetworkOpenState {
    NETWORK_OPEN_OPENING = 0,
    NETWORK_OPEN_OPEN = 1,
    NETWORK_OPEN_PEER_CLOSED = 2,
    NETWORK_OPEN_CLOSED = 3
} NetworkOpenState;

typedef struct NetworkOpenFlow {
    bool pending;
    uint32_t remote_ip;
    uint16_t remote_port;
    NetworkTcpClient client;
} NetworkOpenFlow;

typedef struct NetworkOpenEngine {
    NetworkEngineDns base;
    NetworkOpenFlow *flows;
    size_t count, capacity, next_egress;
} NetworkOpenEngine;

NetworkEngineError network_open_init(NetworkOpenEngine *engine, NetConfig config,
                                     NetworkEngineMac mac, uint64_t now);
void network_open_dispose(NetworkOpenEngine *engine);
size_t network_open_add_flow(NetworkOpenEngine *engine);
NetworkEngineError network_open_connect(NetworkOpenEngine *engine, size_t index,
                                        uint32_t address, uint16_t port);
NetworkOpenState network_open_state(const NetworkOpenEngine *engine, size_t index);
bool network_open_idle(const NetworkOpenEngine *engine, size_t index);
NetworkTcpClientCount network_open_send(NetworkOpenEngine *engine, size_t index,
                                        const NetworkEnginePayload *payload);
NetworkTcpClientReceive network_open_receive(NetworkOpenEngine *engine, size_t index,
                                             size_t maximum);
void network_open_shutdown(NetworkOpenEngine *engine, size_t index);
void network_open_close(NetworkOpenEngine *engine, size_t index);
NetworkEngineError network_open_submit(NetworkOpenEngine *engine,
                                       const NetworkEngineFrame *frame);
NetworkEngineFrameResult network_open_take_frame(NetworkOpenEngine *engine);
bool network_open_can_receive(const NetworkOpenEngine *engine);
bool network_open_has_frame(const NetworkOpenEngine *engine);
void network_open_advance(NetworkOpenEngine *engine, uint64_t now);
void network_open_stop(NetworkOpenEngine *engine, NetworkEngineError reason);
#endif
