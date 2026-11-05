#include "sexos_network_config.h"
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

NetConfig net_config_controlled(void) {
    return (NetConfig){.local = 0x0a00020f,
                       .gateway = 0x0a000202,
                       .dns = 0x0a000202,
                       .http = 0x0a000202,
                       .prefix = 24,
                       .profile = 1,
                       .dns_port = 46079,
                       .http_port = 46078,
                       .listen_port = 46076,
                       .source_base = NET_CONFIG_SOURCE_BASE};
}

static uint32_t net_config_mask(uint8_t prefix) {
    if (prefix < 1 || prefix > 30)
        return 0;
    return UINT32_MAX << (32 - prefix);
}

static bool net_config_unicast(uint32_t address) {
    uint32_t first = address >> 24;
    return first && first != 127 && first < 224;
}

static bool net_config_usable(uint32_t address, uint32_t mask) {
    uint32_t host = address & ~mask;
    return net_config_unicast(address) && host && host != ~mask;
}

bool net_config_contains(NetConfig c, uint32_t address) {
    uint32_t mask = net_config_mask(c.prefix);
    return mask && net_config_usable(c.local, mask) && net_config_usable(address, mask) &&
           (c.local & mask) == (address & mask);
}

static bool net_config_endpoint(NetConfig c, uint32_t address) {
    uint32_t mask = net_config_mask(c.prefix);
    return mask && net_config_unicast(address) && address != c.local &&
           ((address & mask) != (c.local & mask) || net_config_contains(c, address));
}

bool net_config_valid(NetConfig c) {
    return c.profile == 1 && net_config_contains(c, c.local) && net_config_contains(c, c.gateway) &&
           net_config_endpoint(c, c.dns) && net_config_endpoint(c, c.http) &&
           c.gateway != c.local && c.dns_port && c.http_port && c.listen_port &&
           c.listen_port < c.source_base && c.source_base == NET_CONFIG_SOURCE_BASE &&
           (uint32_t)c.source_base + NET_CONFIG_MAX_TICKET == UINT16_MAX;
}

const char *net_config_hostname(NetConfig c) {
    return c.profile == 1 ? NET_CONFIG_HOST_NAME : NULL;
}

NetConfigEndpoint net_config_tls_endpoint(NetConfig c) {
    if (!net_config_valid(c))
        return (NetConfigEndpoint){0};
    return (NetConfigEndpoint){true, c.http, NET_CONFIG_TLS_PORT};
}

NetConfigWords net_config_to_words(NetConfig c) {
    return (NetConfigWords){c.local | ((uint64_t)c.prefix << 32) | ((uint64_t)c.profile << 40) |
                                ((uint64_t)NET_CONFIG_VERSION << 56),
                            c.gateway | ((uint64_t)c.dns << 32),
                            (uint64_t)c.dns_port | ((uint64_t)c.http_port << 16) |
                                ((uint64_t)c.listen_port << 32) | ((uint64_t)c.source_base << 48),
                            c.http};
}

NetConfigResult net_config_from_words(NetConfigWords w) {
    if (w.word0 >> 56 != NET_CONFIG_VERSION || ((w.word0 >> 48) & 255) || w.word3 >> 32)
        return (NetConfigResult){0};
    NetConfig c = {.local = (uint32_t)w.word0,
                   .gateway = (uint32_t)w.word1,
                   .dns = (uint32_t)(w.word1 >> 32),
                   .http = (uint32_t)w.word3,
                   .prefix = (uint8_t)(w.word0 >> 32),
                   .profile = (uint8_t)(w.word0 >> 40),
                   .dns_port = (uint16_t)w.word2,
                   .http_port = (uint16_t)(w.word2 >> 16),
                   .listen_port = (uint16_t)(w.word2 >> 32),
                   .source_base = (uint16_t)(w.word2 >> 48)};
    if (!net_config_valid(c))
        return (NetConfigResult){0};
    return (NetConfigResult){true, c};
}


