#include "network_web_open.h"
#include "program_memory.h"
#include "sexos_server.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CHECK(x) check((x), __LINE__)
static void check(bool ok, int line) { if (!ok) { fprintf(stderr, "open check failed at %d\n", line); assert(ok); } }

static const NetworkEngineMac LOCAL = {{0x52, 0x54, 0, 0x12, 0x34, 0x56}};
static const NetworkEngineMac PEER = {{2, 0, 0, 0, 0, 1}};
__attribute__((aligned(4096))) static uint8_t backing[16 * 1024 * 1024];
static uint64_t held;
static bool deny;
static NetworkWebOpen service;

SexosProgramMemory sexos_program_memory_grow(uint64_t pages)
{
    if (deny) return (SexosProgramMemory){PROGRAM_MEMORY_DENIED, (uintptr_t)backing, held, 0};
    held += pages;
    assert(held * 4096 <= sizeof backing);
    return (SexosProgramMemory){PROGRAM_MEMORY_OK, (uintptr_t)backing, held, 0};
}
SexosMessage sexos_listen_once(uint64_t slot) { (void)slot; return (SexosMessage){0}; }
void sexos_yield(void) {}
void sexos_heap_reset_for_restart(void);

static void p16(NetworkEngineFrame *f, size_t n, uint16_t v)
{
    f->bytes[n] = (uint8_t)(v >> 8); f->bytes[n + 1] = (uint8_t)v;
}
static void p32(NetworkEngineFrame *f, size_t n, uint32_t v)
{
    f->bytes[n] = (uint8_t)(v >> 24); f->bytes[n + 1] = (uint8_t)(v >> 16);
    f->bytes[n + 2] = (uint8_t)(v >> 8); f->bytes[n + 3] = (uint8_t)v;
}
static NetworkEngineFrame arp(uint32_t sender, uint32_t target, NetworkEngineMac mac, bool reply)
{
    NetworkEngineFrame f = {0}; f.length = 60;
    for (size_t i = 0; i < 6; ++i) {
        f.bytes[i] = reply ? LOCAL.bytes[i] : 255;
        f.bytes[6 + i] = mac.bytes[i]; f.bytes[22 + i] = mac.bytes[i];
        f.bytes[32 + i] = reply ? LOCAL.bytes[i] : 255;
    }
    p16(&f, 12, 0x0806); p16(&f, 14, 1); p16(&f, 16, 0x0800);
    f.bytes[18] = 6; f.bytes[19] = 4; p16(&f, 20, reply ? 2 : 1);
    p32(&f, 28, sender); p32(&f, 38, target); return f;
}
static NetworkEnginePayload payload(size_t n, uint8_t salt)
{
    NetworkEnginePayload p = {0}; p.len = n;
    for (size_t i = 0; i < n; ++i) p.bytes[i] = (uint8_t)(i * 13 + salt);
    return p;
}
static NetworkTcpPacket take_tcp(NetworkWebOpen *s)
{
    NetworkEngineFrameResult r = network_web_open_take_frame(s); CHECK(r.present);
    NetworkTcpPacket t = network_engine_tcp_decode(&r.frame); CHECK(t.valid); return t;
}
static void reset(void)
{
    held = 0; deny = false; sexos_heap_reset_for_restart(); sexos_heap_init();
    CHECK(network_web_open_init(&service, net_config_controlled(), LOCAL, 0) == NETWORK_ENGINE_OK);
}
static void finish(void)
{
    network_web_open_dispose(&service);
    CHECK(!service.owners.flows && !service.engine.flows && !service.owners.flow_count && !service.engine.count);
}

static void two_connections_and_close(void)
{
    reset();
    NetworkJobIdentity a = {15, 100}, b = {30, 200}, wrong = {15, 101};
    NetConfig c = net_config_controlled();
    CHECK(network_web_open_connect(&service, a, 0, c.http, c.http_port) == NETWORK_ENGINE_OK);
    CHECK(network_web_open_connect(&service, b, 0, c.http, c.http_port + 1) == NETWORK_ENGINE_OK);
    CHECK(network_web_open_state(&service, a, 0) == NETWORK_OPEN_OPENING);
    CHECK(service.owners.flow_count == 2 && service.engine.count == 2);
    network_web_open_advance(&service, 0);
    NetworkEngineFrameResult query = network_web_open_take_frame(&service); CHECK(query.present);
    NetworkArpPacket arp_query = network_engine_arp_decode(&query.frame);
    CHECK(arp_query.valid && arp_query.operation == 1 && arp_query.target_ip == c.http);
    NetworkEngineFrame reply = arp(c.http, c.local, PEER, true);
    CHECK(network_web_open_submit(&service, &reply) == NETWORK_ENGINE_OK);
    network_web_open_advance(&service, 1);
    NetworkTcpPacket syn0 = take_tcp(&service);
    CHECK((syn0.flags & NETWORK_TCP_SYN) && syn0.source_port == c.source_base + 1);
    network_web_open_advance(&service, 2);
    NetworkTcpPacket syn1 = take_tcp(&service);
    CHECK((syn1.flags & NETWORK_TCP_SYN) && syn1.source_port == c.source_base + 2 &&
          syn1.destination_port == c.http_port + 1);

    NetworkEnginePayload empty = {0};
    NetworkEngineFrame synack0 = {0};
    CHECK(network_engine_tcp_build(&synack0, LOCAL, PEER, c.http, c.local, c.http_port,
          c.source_base + 1, 900, syn0.sequence + 1, NETWORK_TCP_SYN | NETWORK_TCP_ACK, 8192, &empty));
    CHECK(network_web_open_submit(&service, &synack0) == NETWORK_ENGINE_OK);
    network_web_open_advance(&service, 3);
    CHECK(take_tcp(&service).flags == NETWORK_TCP_ACK);
    CHECK(network_web_open_state(&service, a, 0) == NETWORK_OPEN_OPEN);

    NetworkEngineFrame synack1 = {0};
    CHECK(network_engine_tcp_build(&synack1, LOCAL, PEER, c.http, c.local, c.http_port + 1,
          c.source_base + 2, 1000, syn1.sequence + 1, NETWORK_TCP_SYN | NETWORK_TCP_ACK, 8192, &empty));
    CHECK(network_web_open_submit(&service, &synack1) == NETWORK_ENGINE_OK);
    network_web_open_advance(&service, 4);
    CHECK(take_tcp(&service).flags == NETWORK_TCP_ACK);
    CHECK(network_web_open_state(&service, b, 0) == NETWORK_OPEN_OPEN);

    NetworkEnginePayload out0 = payload(200, 3), out1 = payload(100, 7);
    CHECK(network_web_open_send(&service, a, 0, &out0).count == 200);
    CHECK(network_web_open_send(&service, b, 0, &out1).count == 100);
    network_web_open_advance(&service, 5);
    NetworkTcpPacket data0 = take_tcp(&service);
    CHECK(data0.payload_length == 200 && data0.source_port == c.source_base + 1);
    network_web_open_advance(&service, 6);
    NetworkTcpPacket data1 = take_tcp(&service);
    CHECK(data1.payload_length == 100 && data1.source_port == c.source_base + 2);

    NetworkEngineFrame ack0 = {0};
    CHECK(network_engine_tcp_build(&ack0, LOCAL, PEER, c.http, c.local, c.http_port,
          c.source_base + 1, service.engine.flows[0].client.rcv_nxt,
          service.engine.flows[0].client.pending_end, NETWORK_TCP_ACK, 8192, &empty));
    CHECK(network_web_open_submit(&service, &ack0) == NETWORK_ENGINE_OK);
    network_web_open_advance(&service, 7);
    CHECK(service.engine.flows[0].client.send_len == 0);
    CHECK(!network_open_has_frame(&service.engine));

    NetworkEnginePayload incoming = payload(64, 9);
    NetworkEngineFrame peerdata = {0};
    CHECK(network_engine_tcp_build(&peerdata, LOCAL, PEER, c.http, c.local, c.http_port + 1,
          c.source_base + 2, service.engine.flows[1].client.rcv_nxt,
          service.engine.flows[1].client.snd_nxt,
          NETWORK_TCP_PSH | NETWORK_TCP_ACK | NETWORK_TCP_FIN, 8192, &incoming));
    CHECK(network_web_open_submit(&service, &peerdata) == NETWORK_ENGINE_OK);
    network_web_open_advance(&service, 8);
    CHECK(take_tcp(&service).flags == NETWORK_TCP_ACK);
    CHECK(network_web_open_state(&service, b, 0) == NETWORK_OPEN_PEER_CLOSED);
    NetworkTcpClientReceive got = network_web_open_receive(&service, b, 0, 256);
    CHECK(got.ok && got.count == 64 && !memcmp(got.bytes, incoming.bytes, 64));
    CHECK(!network_web_open_receive(&service, wrong, 0, 256).ok);
    CHECK(!network_web_open_send(&service, wrong, 0, &out0).ok);

    network_web_open_close(&service, a, 0);
    CHECK(service.engine.flows[0].client.state == NETWORK_TCP_CLOSED);
    network_web_open_advance(&service, 9);
    NetworkTcpPacket reset = take_tcp(&service);
    CHECK((reset.flags & NETWORK_TCP_RST) && reset.source_port == c.source_base + 1);
    CHECK(network_web_open_state(&service, b, 0) == NETWORK_OPEN_PEER_CLOSED);
    CHECK(!network_web_open_close(&service, a, 0));

    network_web_open_shutdown(&service, b, 0);
    network_web_open_advance(&service, 10);
    CHECK((take_tcp(&service).flags & NETWORK_TCP_FIN) != 0);
    NetworkWebRetirement retired = network_web_open_retire(&service, b);
    CHECK(retired.flows == 1 && service.engine.flows[1].client.finish_requested);

    NetworkJobIdentity next = {31, 300};
    CHECK(network_web_open_connect(&service, next, 0, c.http, c.http_port) == NETWORK_ENGINE_OK);
    network_web_open_advance(&service, 11);
    NetworkTcpPacket reused = take_tcp(&service);
    CHECK((reused.flags & NETWORK_TCP_SYN) && reused.source_port == c.source_base + 1);
    retired = network_web_open_retire(&service, next);
    CHECK(retired.flows == 1);
    finish();
}

static void failures_preserve_joined_state(void)
{
    reset();
    NetworkJobIdentity a = {15, 100};
    NetConfig c = net_config_controlled();
    CHECK(network_web_open_connect(&service, a, 255, c.http, c.http_port) == NETWORK_ENGINE_BUSY);
    CHECK(network_web_open_connect(&service, a, 0, c.http, 0) == NETWORK_ENGINE_LENGTH);
    CHECK(service.owners.flow_count == 1 && !service.owners.flows[0].present && service.engine.count == 1);
    network_web_open_dispose(&service);
    reset();

    void *fixed = sexos_malloc(100000, 16); CHECK(fixed);
    deny = true;
    CHECK(network_web_open_connect(&service, a, 0, c.http, c.http_port) == NETWORK_ENGINE_BUSY);
    CHECK(service.owners.flow_count == 1 && !service.owners.flows[0].present && service.owners.flow_capacity);
    CHECK(service.engine.count == 0 && service.engine.capacity == 0);
    sexos_free(fixed, 100000, 16);
    finish();
}

int main(void)
{
    two_connections_and_close();
    failures_preserve_joined_state();
    puts("network open: two connections, exact ownership, partial I/O, final read, close, reuse and rollback passed");
    return 0;
}
