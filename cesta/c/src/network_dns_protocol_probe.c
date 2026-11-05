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
#include "network_dns_job_abi.h"

static NetworkDnsJobReplyDecode dns_probe_call(NetworkDnsJobRequest request) {
    NetworkDnsJobWords words = network_dns_job_encode(request);
    SexosReply admitted = {UINT64_MAX, 0};
    for (uint64_t remaining = UINT64_C(20000); remaining; --remaining) {
        admitted = sexos_rpc_begin(SEXOS_SLOT_net, NETWORK_DNS_JOB_OP, words.words[0],
                                   words.words[1], words.words[2]);
        if (!admitted.status)
            break;
        if (admitted.status != UINT64_MAX && admitted.status != SEXOS_RPC_BUSY)
            return (NetworkDnsJobReplyDecode){false, false, (uint32_t)admitted.status};
        sexos_yield();
    }
    if (admitted.status)
        return (NetworkDnsJobReplyDecode){false, false, NETWORK_JOB_TIMEOUT};
    for (uint64_t remaining = UINT64_C(20000); remaining; --remaining) {
        SexosReply polled = sexos_rpc_poll(admitted.value);
        if (!polled.status)
            return network_dns_job_decode_reply(polled.value, request.seq);
        if (polled.status != SEXOS_RPC_BUSY)
            return (NetworkDnsJobReplyDecode){false, false, (uint32_t)polled.status};
        sexos_yield();
    }
    (void)sexos_rpc_cancel(admitted.value);
    return (NetworkDnsJobReplyDecode){false, false, NETWORK_JOB_TIMEOUT};
}

static uint32_t dns_probe_expect(NetworkDnsJobRequest request) {
    NetworkDnsJobReplyDecode reply = dns_probe_call(request);
    if (!reply.valid || !reply.success) {
        sexos_line("[network.dns.protocol.probe.failed] seq=%u kind=%u detail=%u", request.seq,
                   (unsigned)request.kind, reply.value);
        sexos_fail();
    }
    return reply.value;
}

void network_dns_protocol_probe_main(void) {
    sexos_line("[network.dns.protocol.probe.begin] mode=correlated-required name=probe.sexos.test");
    uint32_t seq = 1;
    uint32_t ticket = dns_probe_expect((NetworkDnsJobRequest){seq++, NETWORK_DNS_JOB_BEGIN, 0});
    if (!ticket || ticket > NETWORK_JOB_MAX_TICKET)
        sexos_fail();
    sexos_line("[network.dns.protocol.probe.admitted] ticket=%u", ticket);
    NetworkDnsJobState state = NETWORK_DNS_JOB_WAITING;
    for (size_t turn = 0; turn < 4096; ++turn) {
        NetworkDnsJobStateDecode decoded = network_dns_job_state_decode(
            dns_probe_expect((NetworkDnsJobRequest){seq++, NETWORK_DNS_JOB_STATUS, ticket}));
        if (!decoded.ok)
            sexos_fail();
        state = decoded.state;
        if (state == NETWORK_DNS_JOB_COMPLETE)
            break;
        if (state == NETWORK_DNS_JOB_FAILED || state == NETWORK_DNS_JOB_CANCELLED ||
            state == NETWORK_DNS_JOB_IDLE)
            sexos_fail();
        sexos_yield();
    }
    if (state != NETWORK_DNS_JOB_COMPLETE)
        sexos_fail();
    uint32_t address =
        dns_probe_expect((NetworkDnsJobRequest){seq++, NETWORK_DNS_JOB_RESULT, ticket});
    if (address != UINT32_C(0x0a000202))
        sexos_fail();
    sexos_line("[network.dns.protocol.probe.result] ticket=%u address=10.0.2.2", ticket);
    if (dns_probe_expect((NetworkDnsJobRequest){seq++, NETWORK_DNS_JOB_RELEASE, ticket}) != 0)
        sexos_fail();
    sexos_line("[network.dns.protocol.probe.done] ticket=%u positive_release=1", ticket);
    for (;;)
        sexos_yield();
}


