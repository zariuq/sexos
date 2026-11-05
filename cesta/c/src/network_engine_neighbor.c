#include "network_engine_neighbor.h"
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

static bool neighbor_mac_valid(NetworkEngineMac mac) {
    bool nonzero = false;
    for (size_t i = 0; i < 6; i++)
        if (mac.bytes[i])
            nonzero = true;
    return nonzero && (mac.bytes[0] & 1) == 0;
}

static bool neighbor_address_valid(uint32_t address) {
    uint8_t first = (uint8_t)(address >> 24);
    return first && first != 127 && first < 224;
}

void network_neighbor_init(NetworkNeighborTable *table) {
    *table = (NetworkNeighborTable){0};
}

NetworkRouteDecision network_route_decide(NetConfig config, uint32_t destination) {
    if (!net_config_valid(config) || !neighbor_address_valid(destination) ||
        destination == config.local)
        return (NetworkRouteDecision){0};
    bool on_link = net_config_contains(config, destination);
    return (NetworkRouteDecision){true, on_link, on_link ? destination : config.gateway};
}

void network_neighbor_expire(NetworkNeighborTable *table, uint64_t now) {
    for (size_t i = 0; i < NETWORK_NEIGHBOR_CAPACITY; i++)
        if (table->entries[i].present && now >= table->entries[i].expires)
            table->entries[i] = (NetworkNeighborEntry){0};
}

static size_t neighbor_slot(NetworkNeighborTable *table, uint32_t address) {
    for (size_t i = 0; i < NETWORK_NEIGHBOR_CAPACITY; i++)
        if (table->entries[i].present && table->entries[i].address == address)
            return i;
    for (size_t i = 0; i < NETWORK_NEIGHBOR_CAPACITY; i++)
        if (!table->entries[i].present)
            return i;
    size_t selected = 0;
    for (size_t i = 1; i < NETWORK_NEIGHBOR_CAPACITY; i++)
        if (table->entries[i].last_used < table->entries[selected].last_used)
            selected = i;
    return selected;
}

bool network_neighbor_observe(NetworkNeighborTable *table, NetConfig config,
                              NetworkArpPacket packet, uint64_t now) {
    if (!net_config_valid(config) || !packet.valid ||
        (packet.operation != 1 && packet.operation != 2) || packet.target_ip != config.local ||
        !net_config_contains(config, packet.sender_ip) ||
        !neighbor_address_valid(packet.sender_ip) || !neighbor_mac_valid(packet.sender_mac) ||
        UINT64_MAX - now < NETWORK_NEIGHBOR_LIFETIME)
        return false;
    network_neighbor_expire(table, now);
    size_t slot = neighbor_slot(table, packet.sender_ip);
    table->entries[slot] = (NetworkNeighborEntry){true, packet.sender_ip, packet.sender_mac,
                                                  now + NETWORK_NEIGHBOR_LIFETIME, now};
    if (table->probe_present && table->probe_address == packet.sender_ip) {
        table->probe_present = false;
        table->probe_address = 0;
        table->next_probe = 0;
    }
    return true;
}

NetworkNeighborResolution network_neighbor_resolve(NetworkNeighborTable *table, NetConfig config,
                                                   uint32_t destination, uint64_t now) {
    NetworkRouteDecision route = network_route_decide(config, destination);
    if (!route.valid)
        return (NetworkNeighborResolution){NETWORK_NEIGHBOR_INVALID, 0, {{0}}};
    network_neighbor_expire(table, now);
    for (size_t i = 0; i < NETWORK_NEIGHBOR_CAPACITY; i++)
        if (table->entries[i].present && table->entries[i].address == route.next_hop) {
            table->entries[i].last_used = now;
            return (NetworkNeighborResolution){NETWORK_NEIGHBOR_HIT, route.next_hop,
                                               table->entries[i].mac};
        }
    if (!table->probe_present || table->probe_address != route.next_hop) {
        table->probe_present = true;
        table->probe_address = route.next_hop;
        table->next_probe = now;
    }
    if (now >= table->next_probe) {
        table->next_probe = UINT64_MAX - now < NETWORK_NEIGHBOR_PROBE_INTERVAL
                                ? UINT64_MAX
                                : now + NETWORK_NEIGHBOR_PROBE_INTERVAL;
        return (NetworkNeighborResolution){NETWORK_NEIGHBOR_DISCOVER, route.next_hop, {{0}}};
    }
    return (NetworkNeighborResolution){NETWORK_NEIGHBOR_WAIT, route.next_hop, {{0}}};
}

size_t network_neighbor_count(const NetworkNeighborTable *table) {
    size_t count = 0;
    for (size_t i = 0; i < NETWORK_NEIGHBOR_CAPACITY; i++)
        if (table->entries[i].present)
            count++;
    return count;
}


