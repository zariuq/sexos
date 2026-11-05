#include "network_engine_dns.h"
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

static bool engine_mac_valid(NetworkEngineMac mac) {
    bool nonzero = false;
    for (size_t i = 0; i < 6; i++)
        if (mac.bytes[i])
            nonzero = true;
    return nonzero && (mac.bytes[0] & 1) == 0;
}

static void engine_frame_clear(NetworkEngineFrame *frame) {
    for (size_t i = 0; i < NETWORK_ENGINE_MAX_FRAME; i++)
        frame->bytes[i] = 0;
    frame->length = 0;
}

static void engine_dns_frame_clear(NetworkDnsFrame *frame) {
    for (size_t i = 0; i < NETWORK_DNS_MAX_BYTES; i++)
        frame->bytes[i] = 0;
    frame->len = 0;
}

NetworkEngineError network_engine_dns_init(NetworkEngineDns *engine, NetConfig config,
                                           NetworkEngineMac mac, uint64_t now) {
    if (!net_config_valid(config))
        return NETWORK_ENGINE_CONFIG;
    if (!engine_mac_valid(mac))
        return NETWORK_ENGINE_MAC;
    if (now > (uint64_t)INT64_MAX)
        return NETWORK_ENGINE_CLOCK;
    NetworkEngineDns next = {0};
    next.config = config;
    next.mac = mac;
    next.last = now;
    network_neighbor_init(&next.neighbors);
    *engine = next;
    return NETWORK_ENGINE_OK;
}

NetworkEngineError network_engine_dns_submit(NetworkEngineDns *engine,
                                             const NetworkEngineFrame *frame) {
    if (engine->stopped)
        return NETWORK_ENGINE_STOPPED;
    if (engine->rx_present)
        return NETWORK_ENGINE_BUSY;
    if (frame->length < 14 || frame->length > NETWORK_ENGINE_MAX_FRAME)
        return NETWORK_ENGINE_LENGTH;
    engine->rx = *frame;
    engine->rx_present = true;
    return NETWORK_ENGINE_OK;
}

bool network_engine_dns_can_receive(const NetworkEngineDns *engine) {
    return !engine->stopped && !engine->rx_present;
}

bool network_engine_dns_has_frame(const NetworkEngineDns *engine) {
    return engine->tx_present;
}

NetworkEngineFrameResult network_engine_dns_take_frame(NetworkEngineDns *engine) {
    if (!engine->tx_present)
        return (NetworkEngineFrameResult){0};
    NetworkEngineFrameResult result = {true, engine->tx};
    engine->tx_present = false;
    engine->tx_kind = NETWORK_ENGINE_TX_NONE;
    engine->tx_ticket = 0;
    engine_frame_clear(&engine->tx);
    return result;
}

void network_engine_dns_stop(NetworkEngineDns *engine, NetworkEngineError reason) {
    engine->stopped = true;
    engine->failure = reason;
    engine->rx_present = false;
    engine->tx_present = false;
    engine->dns_active = false;
    engine->query_pending = false;
    engine->response_present = false;
    engine_frame_clear(&engine->rx);
    engine_frame_clear(&engine->tx);
    engine_dns_frame_clear(&engine->response);
    network_neighbor_init(&engine->neighbors);
    engine->tx_kind = NETWORK_ENGINE_TX_NONE;
    engine->tx_ticket = 0;
    engine->dns_ticket = 0;
}

void network_engine_dns_abort(NetworkEngineDns *engine) {
    uint32_t ticket = engine->dns_ticket;
    if (engine->tx_present && engine->tx_kind == NETWORK_ENGINE_TX_DNS &&
        engine->tx_ticket == ticket) {
        engine->tx_present = false;
        engine->tx_kind = NETWORK_ENGINE_TX_NONE;
        engine->tx_ticket = 0;
        engine_frame_clear(&engine->tx);
    }
    engine->dns_active = false;
    engine->query_pending = false;
    engine->response_present = false;
    engine->dns_ticket = 0;
    engine_dns_frame_clear(&engine->response);
}

NetworkEngineError network_engine_dns_begin(NetworkEngineDns *engine, uint32_t ticket) {
    if (engine->stopped)
        return NETWORK_ENGINE_STOPPED;
    if (!ticket || ticket > NET_CONFIG_MAX_TICKET)
        return NETWORK_ENGINE_LENGTH;
    network_engine_dns_abort(engine);
    engine->dns_active = true;
    engine->query_pending = true;
    engine->dns_ticket = ticket;
    return NETWORK_ENGINE_OK;
}

NetworkEngineDnsResult network_engine_dns_receive(NetworkEngineDns *engine) {
    if (engine->stopped)
        return (NetworkEngineDnsResult){false, false, NETWORK_ENGINE_STOPPED, {{0}, 0}};
    if (!engine->response_present)
        return (NetworkEngineDnsResult){true, false, NETWORK_ENGINE_OK, {{0}, 0}};
    NetworkEngineDnsResult result = {true, true, NETWORK_ENGINE_OK, engine->response};
    engine->response_present = false;
    engine_dns_frame_clear(&engine->response);
    return result;
}

static void engine_dns_accept(NetworkEngineDns *engine, NetworkEngineFrame frame) {
    if (!engine->dns_active || engine->response_present)
        return;
    NetworkIpv4Packet ip = network_engine_ipv4_decode(&frame);
    NetworkUdpPacket udp = network_engine_udp_decode(&frame);
    uint32_t port = (uint32_t)engine->config.source_base + engine->dns_ticket;
    if (!ip.valid || !udp.valid || ip.source != engine->config.dns ||
        ip.destination != engine->config.local || udp.source_port != engine->config.dns_port ||
        udp.destination_port != port || udp.payload_length > NETWORK_DNS_MAX_BYTES)
        return;
    for (size_t i = 0; i < udp.payload_length; i++) {
        engine->response.bytes[i] = frame.bytes[udp.payload_offset + i];
    }
    engine->response.len = udp.payload_length;
    engine->response_present = true;
}

static void engine_dns_ingress(NetworkEngineDns *engine, uint64_t now) {
    NetworkEngineFrame input = engine->rx;
    NetworkArpPacket arp = network_engine_arp_decode(&input);
    if (arp.valid) {
        bool observed = network_neighbor_observe(&engine->neighbors, engine->config, arp, now);
        if (observed && arp.operation == 1 && !engine->tx_present &&
            network_engine_arp_reply(&engine->tx, engine->config, engine->mac, arp.sender_mac,
                                     arp.sender_ip)) {
            engine->tx_present = true;
            engine->tx_kind = NETWORK_ENGINE_TX_ARP;
            engine->tx_ticket = 0;
        }
    } else
        engine_dns_accept(engine, input);
    engine->rx_present = false;
    engine_frame_clear(&engine->rx);
    if (engine->packets != UINT64_MAX)
        engine->packets++;
}

static void engine_dns_egress(NetworkEngineDns *engine, uint64_t now) {
    if (!engine->dns_active || !engine->query_pending || engine->tx_present)
        return;
    NetworkNeighborResolution neighbor =
        network_neighbor_resolve(&engine->neighbors, engine->config, engine->config.dns, now);
    if (neighbor.kind == NETWORK_NEIGHBOR_DISCOVER) {
        if (network_engine_arp_request(&engine->tx, engine->config, engine->mac,
                                       neighbor.next_hop)) {
            engine->tx_present = true;
            engine->tx_kind = NETWORK_ENGINE_TX_ARP;
        }
        return;
    }
    if (neighbor.kind != NETWORK_NEIGHBOR_HIT)
        return;
    NetworkDnsFrame query = {0};
    NetworkDnsQueryResult made = network_dns_query((uint16_t)engine->dns_ticket, &query);
    if (!made.ok) {
        network_engine_dns_stop(engine, NETWORK_ENGINE_PROTOCOL);
        return;
    }
    NetworkEnginePayload payload = {0};
    payload.len = query.len;
    for (size_t i = 0; i < query.len; i++)
        payload.bytes[i] = query.bytes[i];
    uint32_t source_port = (uint32_t)engine->config.source_base + engine->dns_ticket;
    if (source_port > UINT16_MAX ||
        !network_engine_udp_build(&engine->tx, neighbor.mac, engine->mac, engine->config.local,
                                  engine->config.dns, (uint16_t)source_port,
                                  engine->config.dns_port, &payload)) {
        network_engine_dns_stop(engine, NETWORK_ENGINE_PROTOCOL);
        return;
    }
    engine->tx_present = true;
    engine->tx_kind = NETWORK_ENGINE_TX_DNS;
    engine->tx_ticket = engine->dns_ticket;
    engine->query_pending = false;
}

void network_engine_dns_advance(NetworkEngineDns *engine, uint64_t now) {
    if (engine->stopped)
        return;
    if (now < engine->last || now > (uint64_t)INT64_MAX) {
        network_engine_dns_stop(engine, NETWORK_ENGINE_CLOCK);
        return;
    }
    engine->last = now;
    if (engine->tx_present)
        return;
    if (engine->rx_present)
        engine_dns_ingress(engine, now);
    if (!engine->tx_present)
        engine_dns_egress(engine, now);
}

NetworkEngineDnsSnapshot network_engine_dns_snapshot(const NetworkEngineDns *engine) {
    return (NetworkEngineDnsSnapshot){engine->stopped,
                                      engine->rx_present,
                                      engine->tx_present,
                                      engine->dns_active,
                                      engine->query_pending,
                                      engine->response_present,
                                      engine->failure,
                                      engine->dns_ticket,
                                      engine->last,
                                      engine->packets,
                                      network_neighbor_count(&engine->neighbors)};
}


