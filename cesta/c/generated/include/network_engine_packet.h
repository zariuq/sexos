
#ifndef SEXOS_NETWORK_ENGINE_PACKET_H
#define SEXOS_NETWORK_ENGINE_PACKET_H
#include "network_engine_frame.h"
enum { NETWORK_ENGINE_MAX_PAYLOAD=1472 };
typedef struct NetworkEnginePayload { uint8_t bytes[NETWORK_ENGINE_MAX_PAYLOAD]; size_t len; } NetworkEnginePayload;
typedef struct NetworkEngineMac { uint8_t bytes[6]; } NetworkEngineMac;
typedef struct NetworkArpPacket {
    bool valid; uint16_t operation; NetworkEngineMac sender_mac,target_mac; uint32_t sender_ip,target_ip;
} NetworkArpPacket;
typedef struct NetworkIpv4Packet {
    bool valid; uint8_t protocol; uint32_t source,destination; size_t header_length,transport_offset,transport_length;
} NetworkIpv4Packet;
typedef struct NetworkUdpPacket {
    bool valid; uint16_t source_port,destination_port; size_t payload_offset,payload_length;
} NetworkUdpPacket;
NetworkArpPacket network_engine_arp_decode(const NetworkEngineFrame *frame);
bool network_engine_arp_request(NetworkEngineFrame *frame, NetConfig config, NetworkEngineMac mac, uint32_t target);
bool network_engine_arp_reply(NetworkEngineFrame *frame, NetConfig config, NetworkEngineMac mac, NetworkEngineMac peer_mac, uint32_t peer_ip);
NetworkIpv4Packet network_engine_ipv4_decode(const NetworkEngineFrame *frame);
NetworkUdpPacket network_engine_udp_decode(const NetworkEngineFrame *frame);
bool network_engine_udp_build(NetworkEngineFrame *frame, NetworkEngineMac destination_mac, NetworkEngineMac source_mac,
    uint32_t source_ip, uint32_t destination_ip, uint16_t source_port, uint16_t destination_port,
    const NetworkEnginePayload *payload);
#endif
