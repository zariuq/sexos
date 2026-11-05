
#ifndef SEXOS_NETWORK_ENGINE_FRAME_H
#define SEXOS_NETWORK_ENGINE_FRAME_H
#include "sexos_network_config.h"

enum {
    NETWORK_ENGINE_MAX_FRAME = 1514
};

typedef struct NetworkEngineFrame {
    size_t length;
    uint8_t bytes[NETWORK_ENGINE_MAX_FRAME];
} NetworkEngineFrame;

typedef struct NetworkEngineTicket {
    bool present;
    uint32_t value;
} NetworkEngineTicket;

NetworkEngineTicket network_engine_client_ticket(const NetworkEngineFrame *frame, NetConfig config);
NetworkEngineTicket network_engine_dns_ticket(const NetworkEngineFrame *frame, NetConfig config);
#endif
