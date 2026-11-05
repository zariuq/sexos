
#ifndef SEXOS_NETWORK_TCP_LISTENER_H
#define SEXOS_NETWORK_TCP_LISTENER_H
#include "network_engine_tcp_packet.h"

enum {
    NETWORK_TCP_LISTENER_BUFFER = 4096,
    NETWORK_TCP_LISTENER_TURN = 256,
    NETWORK_TCP_LISTENER_RETRY_LIMIT = 5
};

#define NETWORK_TCP_LISTENER_RETRY_TICKS UINT64_C(1000)

typedef enum NetworkTcpListenerState {
    NETWORK_LISTENER_LISTEN = 0,
    NETWORK_LISTENER_SYN_RECEIVED = 1,
    NETWORK_LISTENER_ESTABLISHED = 2,
    NETWORK_LISTENER_LAST_ACK = 3,
    NETWORK_LISTENER_FAILED = 4
} NetworkTcpListenerState;

typedef struct NetworkTcpListenerStats {
    uint64_t received, queued, receive_hash, queue_hash, flow_received, flow_queued,
        flow_receive_hash, flow_queue_hash, flow;
} NetworkTcpListenerStats;

typedef struct NetworkTcpListenerOutput {
    bool present;
    NetworkEngineFrame frame;
} NetworkTcpListenerOutput;

typedef struct NetworkTcpListenerSnapshot {
    NetworkTcpListenerState state;
    uint32_t snd_una, snd_nxt, rcv_nxt;
    size_t buffered;
    bool pending, immediate, close_requested;
    unsigned retries;
    uint64_t last, retransmit_at;
} NetworkTcpListenerSnapshot;

typedef struct NetworkTcpListener {
    NetworkTcpListenerState state;
    NetConfig config;
    NetworkEngineMac local_mac, peer_mac;
    uint64_t seed, last;
    uint32_t peer_ip;
    uint16_t peer_port;
    uint32_t snd_una, snd_nxt, rcv_nxt;
    uint8_t receive[NETWORK_TCP_LISTENER_BUFFER];
    size_t receive_len;
    bool pending_present, pending_sent, pending_fin, immediate_present, close_requested;
    NetworkEngineFrame pending, immediate;
    uint32_t pending_end;
    size_t pending_payload;
    unsigned retries;
    uint64_t retransmit_at;
    NetworkTcpListenerStats stats;
} NetworkTcpListener;

bool network_tcp_listener_init(NetworkTcpListener *listener, NetConfig config,
                               NetworkEngineMac local_mac, uint64_t seed, uint64_t now);
bool network_tcp_listener_ingest(NetworkTcpListener *listener, const NetworkEngineFrame *frame,
                                 uint64_t now);
NetworkTcpListenerOutput network_tcp_listener_advance(NetworkTcpListener *listener, uint64_t now);
NetworkTcpListenerState network_tcp_listener_state(const NetworkTcpListener *listener);
NetworkTcpListenerStats network_tcp_listener_statistics(const NetworkTcpListener *listener);
NetworkTcpListenerSnapshot network_tcp_listener_snapshot(const NetworkTcpListener *listener);
#endif
