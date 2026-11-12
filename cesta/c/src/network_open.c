#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif
#include "network_open.h"
#include "sexos_server.h"

static void network_open_zero(void *pointer, size_t bytes) {
    uint8_t *out = pointer;
    for (size_t i = 0; i < bytes; ++i)
        out[i] = 0;
}

NetworkEngineError network_open_init(NetworkOpenEngine *engine, NetConfig config,
                                     NetworkEngineMac mac, uint64_t now) {
    engine->flows = NULL;
    engine->count = engine->capacity = engine->next_egress = 0;
    return network_engine_dns_init(&engine->base, config, mac, now);
}

void network_open_dispose(NetworkOpenEngine *engine) {
    if (engine->flows)
        sexos_free(engine->flows, engine->capacity * sizeof *engine->flows,
                   _Alignof(NetworkOpenFlow));
    engine->flows = NULL;
    engine->count = engine->capacity = engine->next_egress = 0;
}

size_t network_open_add_flow(NetworkOpenEngine *engine) {
    if (engine->base.stopped || engine->count >= NETWORK_OPEN_MAX_FLOWS)
        return NETWORK_OPEN_NO_INDEX;
    if (engine->count == engine->capacity) {
        size_t capacity = engine->capacity ? engine->capacity * 2 : 4;
        if (capacity > NETWORK_OPEN_MAX_FLOWS)
            capacity = NETWORK_OPEN_MAX_FLOWS;
        if (capacity > SIZE_MAX / sizeof *engine->flows)
            return NETWORK_OPEN_NO_INDEX;
        NetworkOpenFlow *flows = sexos_realloc(engine->flows,
            engine->capacity * sizeof *engine->flows, capacity * sizeof *flows,
            _Alignof(NetworkOpenFlow));
        if (!flows)
            return NETWORK_OPEN_NO_INDEX;
        engine->flows = flows;
        engine->capacity = capacity;
    }
    size_t index = engine->count++;
    network_open_zero(&engine->flows[index], sizeof engine->flows[index]);
    return index;
}

NetworkEngineError network_open_connect(NetworkOpenEngine *engine, size_t index,
                                        uint32_t address, uint16_t port) {
    if (engine->base.stopped)
        return NETWORK_ENGINE_STOPPED;
    if (index >= engine->count || !port || !network_route_decide(engine->base.config, address).valid)
        return NETWORK_ENGINE_LENGTH;
    NetworkOpenFlow *flow = &engine->flows[index];
    if (flow->client.state != NETWORK_TCP_CLOSED)
        network_tcp_client_abort(&flow->client);
    flow->pending = true;
    flow->remote_ip = address;
    flow->remote_port = port;
    return NETWORK_ENGINE_OK;
}

NetworkOpenState network_open_state(const NetworkOpenEngine *engine, size_t index) {
    if (index >= engine->count)
        return NETWORK_OPEN_CLOSED;
    const NetworkOpenFlow *flow = &engine->flows[index];
    if (flow->pending || flow->client.state == NETWORK_TCP_SYN_SENT)
        return NETWORK_OPEN_OPENING;
    switch (flow->client.state) {
    case NETWORK_TCP_ESTABLISHED:
    case NETWORK_TCP_FIN_WAIT1:
    case NETWORK_TCP_FIN_WAIT2:
        return NETWORK_OPEN_OPEN;
    case NETWORK_TCP_CLOSE_WAIT:
    case NETWORK_TCP_CLOSING:
    case NETWORK_TCP_LAST_ACK:
    case NETWORK_TCP_TIME_WAIT:
        return NETWORK_OPEN_PEER_CLOSED;
    default:
        return NETWORK_OPEN_CLOSED;
    }
}

bool network_open_idle(const NetworkOpenEngine *engine, size_t index) {
    return index < engine->count && !engine->flows[index].pending &&
           engine->flows[index].client.state == NETWORK_TCP_CLOSED;
}

NetworkTcpClientCount network_open_send(NetworkOpenEngine *engine, size_t index,
                                        const NetworkEnginePayload *payload) {
    if (index >= engine->count || engine->flows[index].pending)
        return (NetworkTcpClientCount){false, 0, NETWORK_TCP_CLIENT_STATE};
    return network_tcp_client_send(&engine->flows[index].client, payload);
}

NetworkTcpClientReceive network_open_receive(NetworkOpenEngine *engine, size_t index,
                                             size_t maximum) {
    if (index >= engine->count || engine->flows[index].pending)
        return (NetworkTcpClientReceive){false, 0, NETWORK_TCP_CLIENT_STATE, {0}};
    return network_tcp_client_receive(&engine->flows[index].client, maximum);
}

void network_open_shutdown(NetworkOpenEngine *engine, size_t index) {
    if (index < engine->count && !engine->flows[index].pending)
        (void)network_tcp_client_finish(&engine->flows[index].client);
}

void network_open_close(NetworkOpenEngine *engine, size_t index) {
    if (index >= engine->count)
        return;
    NetworkOpenFlow *flow = &engine->flows[index];
    if (flow->pending) {
        flow->pending = false;
        flow->remote_ip = 0;
        flow->remote_port = 0;
        network_tcp_client_init(&flow->client);
        return;
    }
    if (!flow->client.finish_requested)
        network_tcp_client_abort(&flow->client);
}

static bool network_open_start(NetworkOpenEngine *engine, size_t index, uint64_t now) {
    NetworkOpenFlow *flow = &engine->flows[index];
    if (!flow->pending)
        return false;
    NetworkNeighborResolution neighbor =
        network_neighbor_resolve(&engine->base.neighbors, engine->base.config, flow->remote_ip, now);
    if (neighbor.kind == NETWORK_NEIGHBOR_DISCOVER) {
        if (network_engine_arp_request(&engine->base.tx, engine->base.config, engine->base.mac,
                                       neighbor.next_hop)) {
            engine->base.tx_present = true;
            engine->base.tx_kind = NETWORK_ENGINE_TX_ARP;
            engine->base.tx_ticket = 0;
            return true;
        }
        return false;
    }
    if (neighbor.kind != NETWORK_NEIGHBOR_HIT)
        return false;
    NetConfig config = engine->base.config;
    config.http = flow->remote_ip;
    config.http_port = flow->remote_port;
    uint64_t seed = UINT64_C(0x7365786f735f3736) ^ ((uint64_t)(index + 1) << 32);
    NetworkTcpClientError error = network_tcp_client_begin(
        &flow->client, config, engine->base.mac, neighbor.mac, (uint32_t)(index + 1), false,
        seed, now);
    flow->pending = false;
    if (error) {
        flow->client.state = NETWORK_TCP_FAILED;
        flow->client.error = error;
    }
    return false;
}

static void network_open_ingress(NetworkOpenEngine *engine, uint64_t now) {
    NetworkEngineFrame input = engine->base.rx;
    for (size_t i = 0; i < engine->count; ++i) {
        NetworkOpenFlow *flow = &engine->flows[i];
        if (!flow->pending && flow->client.state != NETWORK_TCP_CLOSED &&
            network_tcp_client_ingest(&flow->client, &input, now))
            break;
    }
    engine->base.rx_present = false;
    for (size_t i = 0; i < NETWORK_ENGINE_MAX_FRAME; ++i)
        engine->base.rx.bytes[i] = 0;
    engine->base.rx.length = 0;
    if (engine->base.packets != UINT64_MAX)
        engine->base.packets++;
}

static bool network_open_egress(NetworkOpenEngine *engine, uint64_t now) {
    if (!engine->count)
        return false;
    for (size_t n = 0; n < engine->count; ++n) {
        size_t i = (engine->next_egress + n) % engine->count;
        if (network_open_start(engine, i, now))
            return true;
        NetworkOpenFlow *flow = &engine->flows[i];
        if (flow->pending)
            continue;
        NetworkTcpClientOutput output = network_tcp_client_advance(&flow->client, now);
        if (!output.present)
            continue;
        engine->base.tx = output.frame;
        engine->base.tx_present = true;
        engine->base.tx_kind = NETWORK_ENGINE_TX_TCP;
        engine->base.tx_ticket = flow->client.ticket;
        engine->next_egress = (i + 1) % engine->count;
        return true;
    }
    return false;
}

void network_open_advance(NetworkOpenEngine *engine, uint64_t now) {
    if (engine->base.stopped)
        return;
    if (now < engine->base.last || now > (uint64_t)INT64_MAX) {
        network_open_stop(engine, NETWORK_ENGINE_CLOCK);
        return;
    }
    engine->base.last = now;
    if (engine->base.tx_present)
        return;
    if (engine->base.rx_present) {
        NetworkEngineFrame input = engine->base.rx;
        NetworkTcpPacket tcp = network_engine_tcp_decode(&input);
        if (tcp.valid)
            network_open_ingress(engine, now);
        else
            network_engine_dns_advance(&engine->base, now);
    } else
        network_engine_dns_advance(&engine->base, now);
    if (engine->base.stopped || engine->base.tx_present)
        return;
    (void)network_open_egress(engine, now);
}

NetworkEngineError network_open_submit(NetworkOpenEngine *engine,
                                       const NetworkEngineFrame *frame) {
    return network_engine_dns_submit(&engine->base, frame);
}

NetworkEngineFrameResult network_open_take_frame(NetworkOpenEngine *engine) {
    return network_engine_dns_take_frame(&engine->base);
}

bool network_open_can_receive(const NetworkOpenEngine *engine) {
    return network_engine_dns_can_receive(&engine->base);
}

bool network_open_has_frame(const NetworkOpenEngine *engine) {
    return network_engine_dns_has_frame(&engine->base);
}

void network_open_stop(NetworkOpenEngine *engine, NetworkEngineError reason) {
    network_engine_dns_stop(&engine->base, reason);
    for (size_t i = 0; i < engine->count; ++i) {
        engine->flows[i].pending = false;
        network_tcp_client_abort(&engine->flows[i].client);
    }
}
