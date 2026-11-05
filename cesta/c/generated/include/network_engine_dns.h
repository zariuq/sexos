
#ifndef SEXOS_NETWORK_ENGINE_DNS_H
#define SEXOS_NETWORK_ENGINE_DNS_H
#include "network_engine_neighbor.h"
#include "network_dns_codec.h"

typedef enum NetworkEngineError {
    NETWORK_ENGINE_OK = 0,
    NETWORK_ENGINE_CONFIG = 1,
    NETWORK_ENGINE_MAC = 2,
    NETWORK_ENGINE_CLOCK = 3,
    NETWORK_ENGINE_BUSY = 4,
    NETWORK_ENGINE_LENGTH = 5,
    NETWORK_ENGINE_STOPPED = 6,
    NETWORK_ENGINE_PROTOCOL = 7
} NetworkEngineError;

typedef enum NetworkEngineTxKind {
    NETWORK_ENGINE_TX_NONE = 0,
    NETWORK_ENGINE_TX_ARP = 1,
    NETWORK_ENGINE_TX_DNS = 2,
    NETWORK_ENGINE_TX_TCP = 3,
    NETWORK_ENGINE_TX_LISTENER = 4
} NetworkEngineTxKind;

typedef struct NetworkEngineFrameResult {
    bool present;
    NetworkEngineFrame frame;
} NetworkEngineFrameResult;

typedef struct NetworkEngineDnsResult {
    bool ok, present;
    NetworkEngineError error;
    NetworkDnsFrame frame;
} NetworkEngineDnsResult;

typedef struct NetworkEngineDnsSnapshot {
    bool stopped, rx_present, tx_present, dns_active, query_pending, response_present;
    NetworkEngineError failure;
    uint32_t ticket;
    uint64_t last, packets;
    size_t neighbors;
} NetworkEngineDnsSnapshot;

typedef struct NetworkEngineDns {
    NetConfig config;
    NetworkEngineMac mac;
    NetworkNeighborTable neighbors;
    bool stopped;
    NetworkEngineError failure;
    uint64_t last, packets;
    bool rx_present, tx_present;
    NetworkEngineFrame rx, tx;
    NetworkEngineTxKind tx_kind;
    uint32_t tx_ticket;
    bool dns_active, query_pending, response_present;
    uint32_t dns_ticket;
    NetworkDnsFrame response;
} NetworkEngineDns;

NetworkEngineError network_engine_dns_init(NetworkEngineDns *engine, NetConfig config,
                                           NetworkEngineMac mac, uint64_t now);
NetworkEngineError network_engine_dns_submit(NetworkEngineDns *engine,
                                             const NetworkEngineFrame *frame);
bool network_engine_dns_can_receive(const NetworkEngineDns *engine);
bool network_engine_dns_has_frame(const NetworkEngineDns *engine);
NetworkEngineFrameResult network_engine_dns_take_frame(NetworkEngineDns *engine);
void network_engine_dns_stop(NetworkEngineDns *engine, NetworkEngineError reason);
NetworkEngineError network_engine_dns_begin(NetworkEngineDns *engine, uint32_t ticket);
NetworkEngineDnsResult network_engine_dns_receive(NetworkEngineDns *engine);
void network_engine_dns_abort(NetworkEngineDns *engine);
void network_engine_dns_advance(NetworkEngineDns *engine, uint64_t now);
NetworkEngineDnsSnapshot network_engine_dns_snapshot(const NetworkEngineDns *engine);
#endif
