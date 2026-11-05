#include "network_engine_tcp_packet.h"
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

static uint16_t tcp_be16(const NetworkEngineFrame *frame, size_t offset) {
    return (uint16_t)((uint16_t)frame->bytes[offset] << 8 | frame->bytes[offset + 1]);
}

static uint32_t tcp_be32(const NetworkEngineFrame *frame, size_t offset) {
    return (uint32_t)frame->bytes[offset] << 24 | (uint32_t)frame->bytes[offset + 1] << 16 |
           (uint32_t)frame->bytes[offset + 2] << 8 | frame->bytes[offset + 3];
}

static void tcp_put16(NetworkEngineFrame *frame, size_t offset, uint16_t value) {
    frame->bytes[offset] = (uint8_t)(value >> 8);
    frame->bytes[offset + 1] = (uint8_t)value;
}

static void tcp_put32(NetworkEngineFrame *frame, size_t offset, uint32_t value) {
    frame->bytes[offset] = (uint8_t)(value >> 24);
    frame->bytes[offset + 1] = (uint8_t)(value >> 16);
    frame->bytes[offset + 2] = (uint8_t)(value >> 8);
    frame->bytes[offset + 3] = (uint8_t)value;
}

static uint16_t tcp_checksum(const NetworkEngineFrame *frame, size_t offset, size_t length,
                             uint32_t sum) {
    for (size_t i = 0; i < length; i += 2) {
        uint16_t word = (uint16_t)frame->bytes[offset + i] << 8;
        if (i + 1 < length)
            word |= frame->bytes[offset + i + 1];
        sum += word;
    }
    while (sum >> 16) {
        sum = (sum & UINT32_C(0xffff)) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

static uint32_t tcp_pseudo(uint32_t source, uint32_t destination, uint16_t length) {
    return (source >> 16) + (source & 0xffff) + (destination >> 16) + (destination & 0xffff) + 6 +
           length;
}

static void tcp_clear(NetworkEngineFrame *frame) {
    for (size_t i = 0; i < NETWORK_ENGINE_MAX_FRAME; i++)
        frame->bytes[i] = 0;
    frame->length = 0;
}

NetworkTcpPacket network_engine_tcp_decode(const NetworkEngineFrame *frame) {
    NetworkTcpPacket out = {0};
    NetworkIpv4Packet ip = network_engine_ipv4_decode(frame);
    if (!ip.valid || ip.protocol != 6 || ip.transport_length < 20)
        return out;
    size_t offset = ip.transport_offset;
    size_t header = (size_t)(frame->bytes[offset + 12] >> 4) * 4;
    if (header < 20 || header > ip.transport_length)
        return out;
    if (tcp_checksum(frame, offset, ip.transport_length,
                     tcp_pseudo(ip.source, ip.destination, (uint16_t)ip.transport_length)) != 0)
        return out;
    out.valid = true;
    out.source_port = tcp_be16(frame, offset);
    out.destination_port = tcp_be16(frame, offset + 2);
    out.sequence = tcp_be32(frame, offset + 4);
    out.acknowledgment = tcp_be32(frame, offset + 8);
    out.flags = frame->bytes[offset + 13];
    out.window = tcp_be16(frame, offset + 14);
    out.header_length = header;
    out.payload_offset = offset + header;
    out.payload_length = ip.transport_length - header;
    return out;
}

bool network_engine_tcp_build(NetworkEngineFrame *frame, NetworkEngineMac destination_mac,
                              NetworkEngineMac source_mac, uint32_t source_ip,
                              uint32_t destination_ip, uint16_t source_port,
                              uint16_t destination_port, uint32_t sequence, uint32_t acknowledgment,
                              uint8_t flags, uint16_t window, const NetworkEnginePayload *payload) {
    if (!frame || !payload || payload->len > 1460 || !source_ip || !destination_ip ||
        !source_port || !destination_port)
        return false;
    size_t tcp_length = 20 + payload->len, total = 20 + tcp_length;
    if (14 + total > NETWORK_ENGINE_MAX_FRAME)
        return false;
    tcp_clear(frame);
    for (size_t i = 0; i < 6; i++) {
        frame->bytes[i] = destination_mac.bytes[i];
        frame->bytes[6 + i] = source_mac.bytes[i];
    }
    tcp_put16(frame, 12, 0x0800);
    frame->bytes[14] = 0x45;
    tcp_put16(frame, 16, (uint16_t)total);
    tcp_put16(frame, 20, 0x4000);
    frame->bytes[22] = 64;
    frame->bytes[23] = 6;
    tcp_put32(frame, 26, source_ip);
    tcp_put32(frame, 30, destination_ip);
    tcp_put16(frame, 24, tcp_checksum(frame, 14, 20, 0));
    size_t tcp = 34;
    tcp_put16(frame, tcp, source_port);
    tcp_put16(frame, tcp + 2, destination_port);
    tcp_put32(frame, tcp + 4, sequence);
    tcp_put32(frame, tcp + 8, acknowledgment);
    frame->bytes[tcp + 12] = 0x50;
    frame->bytes[tcp + 13] = flags;
    tcp_put16(frame, tcp + 14, window);
    for (size_t i = 0; i < payload->len; i++) {
        frame->bytes[tcp + 20 + i] = payload->bytes[i];
    }
    tcp_put16(frame, tcp + 16,
              tcp_checksum(frame, tcp, tcp_length,
                           tcp_pseudo(source_ip, destination_ip, (uint16_t)tcp_length)));
    frame->length = 14 + total;
    return true;
}


