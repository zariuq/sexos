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
#include "sexos_rpc.h"
#include "sexos_memory_regions.h"

static uint8_t network_web_probe_byte(size_t offset) {
    return (uint8_t)(offset * 17 + 78);
}

static uint64_t network_web_probe_call(uint32_t seq, uint8_t kind, uint8_t flow, uint64_t value) {
    uint64_t a = (seq & 0xffffff) | ((uint64_t)flow << 24) | ((uint64_t)kind << 32);
    SexosReply admitted = {UINT64_MAX, 0};
    for (uint64_t remaining = UINT64_C(20000); remaining; --remaining) {
        admitted = sexos_rpc_begin(SEXOS_SLOT_net, UINT64_C(0x211), a, value, 0);
        if (!admitted.status)
            break;
        if (admitted.status != UINT64_MAX && admitted.status != SEXOS_RPC_BUSY)
            return UINT64_MAX;
        sexos_yield();
    }
    if (admitted.status)
        return UINT64_MAX;
    for (uint64_t remaining = UINT64_C(20000); remaining; --remaining) {
        SexosReply polled = sexos_rpc_poll(admitted.value);
        if (!polled.status)
            return polled.value;
        if (polled.status != SEXOS_RPC_BUSY)
            return UINT64_MAX;
        sexos_yield();
    }
    (void)sexos_rpc_cancel(admitted.value);
    return UINT64_MAX;
}

static uint32_t network_web_probe_expect(uint32_t seq, uint8_t kind, uint8_t flow, uint64_t value,
                                         uint8_t wanted_status) {
    uint64_t word = network_web_probe_call(seq, kind, flow, value);
    if (word >> 56 != 0xf6 || ((word >> 32) & 0xffffff) != seq ||
        ((word >> 24) & 0xff) != wanted_status) {
        sexos_line("[network.web.protocol.probe.failed] seq=%u kind=%u status=%u value=%u", seq,
                   (unsigned)kind, (unsigned)((word >> 24) & 0xff), (unsigned)(word & 0xffffff));
        sexos_fail();
    }
    return (uint32_t)(word & 0xffffff);
}

static uint8_t *network_web_probe_region(void) {
    MemoryRegionReply view = sexos_memory_region_view(259, 0, 64 * 1024,
                                                      REGION_QUERY | REGION_READ | REGION_WRITE);
    if (view.status || view.value1 != 64 * 1024 || view.value3 != 0x5100)
        sexos_fail();
    return (uint8_t *)(uintptr_t)view.value0;
}

void network_web_protocol_probe_main(void) {
    sexos_line("[network.web.protocol.probe.begin] bytes=1025 partial=256");
    uint32_t seq = 1;
    (void)network_web_probe_expect(seq++, 1, 0, 0, 0);
    if (network_web_probe_expect(seq++, 4, 1, 0, 0) != 3)
        sexos_fail();
    uint64_t endpoint = ((uint64_t)UINT32_C(0x0a000202) << 16) | 46078;
    (void)network_web_probe_expect(seq++, 3, 0, endpoint, 0);
    for (size_t turn = 0; turn < 4096; ++turn) {
        uint32_t state = network_web_probe_expect(seq++, 4, 0, 0, 0);
        if (state == 1)
            break;
        if (state != 0)
            sexos_fail();
        sexos_yield();
    }
    sexos_line("[network.web.protocol.probe.connected] flow=0 open=1");
    uint8_t *region = network_web_probe_region();
    size_t sent = 0;
    while (sent < 1025) {
        size_t count = 1025 - sent < 256 ? 1025 - sent : 256;
        for (size_t i = 0; i < count; ++i)
            region[i] = network_web_probe_byte(sent + i);
        uint32_t accepted = network_web_probe_expect(seq++, 5, 0, count, 0);
        if (!accepted || accepted > count)
            sexos_fail();
        sent += accepted;
        sexos_yield();
    }
    size_t received = 0;
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    while (received < 1025) {
        uint32_t count = network_web_probe_expect(seq++, 6, 0, 256, 0);
        if (count > 256 || count > 1025 - received)
            sexos_fail();
        if (!count) {
            sexos_yield();
            continue;
        }
        for (size_t i = 0; i < count; ++i) {
            uint8_t byte = region[i];
            if (byte != network_web_probe_byte(received + i))
                sexos_fail();
            hash = (hash ^ byte) * UINT64_C(0x100000001b3);
        }
        received += count;
    }
    sexos_line("[network.web.protocol.probe.bytes] length=%zu hash=%#llx exact=1", received,
               (unsigned long long)hash);
    for (size_t turn = 0; turn < 4096; ++turn) {
        uint32_t state = network_web_probe_expect(seq++, 4, 0, 0, 0);
        if (state == 2)
            break;
        if (state == 3)
            sexos_fail();
        sexos_yield();
    }
    uint32_t final = network_web_probe_expect(seq++, 6, 0, 256, 0);
    if (final > 256)
        sexos_fail();
    (void)network_web_probe_expect(seq++, 7, 0, 0, 0);
    (void)network_web_probe_expect(seq++, 8, 0, 0, 6);
    sexos_line("[network.web.protocol.probe.done] flow=0 bytes=%zu final_read=%u tls_refused=1",
               received, final);
    for (;;)
        sexos_yield();
}
