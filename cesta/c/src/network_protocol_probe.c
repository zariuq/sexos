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
#include "network_job_abi.h"

static uint8_t network_probe_byte(size_t offset) {
    return (uint8_t)(offset * 17 + 78);
}

static NetworkJobReplyDecode network_probe_call(NetworkJobRequest request) {
    NetworkJobWords words = network_job_encode(request);
    SexosReply admitted = {UINT64_MAX, 0};
    uint64_t attempts = 0;
    for (uint64_t remaining = UINT64_C(20000); remaining; remaining--) {
        attempts++;
        admitted = sexos_rpc_begin(SEXOS_SLOT_net, NETWORK_JOB_OP, words.words[0], words.words[1],
                                   words.words[2]);
        if (!admitted.status) {
            if (request.seq == 1)
                sexos_line("[network.protocol.probe.admitted] seq=1 id=%llu attempts=%llu",
                           (unsigned long long)admitted.value, (unsigned long long)attempts);
            break;
        }
        if (request.seq == 1 && (attempts <= 4 || (attempts & (attempts - 1)) == 0))
            sexos_line("[network.protocol.probe.admission.wait] attempt=%llu status=%#llx",
                       (unsigned long long)attempts, (unsigned long long)admitted.status);
        if (admitted.status != UINT64_MAX && admitted.status != SEXOS_RPC_BUSY)
            return (NetworkJobReplyDecode){false, false, (uint32_t)admitted.status};
        sexos_yield();
    }
    if (admitted.status)
        return (NetworkJobReplyDecode){false, false, NETWORK_JOB_TIMEOUT};
    for (uint64_t remaining = UINT64_C(20000); remaining; remaining--) {
        SexosReply polled = sexos_rpc_poll(admitted.value);
        if (!polled.status)
            return network_job_decode_reply(polled.value, request.seq);
        if (polled.status != SEXOS_RPC_BUSY)
            return (NetworkJobReplyDecode){false, false, (uint32_t)polled.status};
        sexos_yield();
    }
    (void)sexos_rpc_cancel(admitted.value);
    return (NetworkJobReplyDecode){false, false, NETWORK_JOB_TIMEOUT};
}

static uint32_t network_probe_expect(NetworkJobRequest request) {
    NetworkJobReplyDecode reply = network_probe_call(request);
    if (!reply.valid || !reply.success) {
        sexos_line("[network.protocol.probe.failed] seq=%u kind=%u detail=%u", request.seq,
                   (unsigned)request.kind, reply.value);
        sexos_fail();
    }
    return reply.value;
}

void network_protocol_probe_main(void) {
    sexos_line("[network.protocol.probe.begin] mode=correlated-required legacy=0 bytes=1025");
    uint32_t seq = 1;
    NetworkJobRequest begin = {seq++, NETWORK_JOB_BEGIN, 0, 1025, 0, 0};
    uint32_t ticket = network_probe_expect(begin);
    if (!ticket || ticket > NETWORK_JOB_MAX_TICKET)
        sexos_fail();
    for (size_t offset = 0; offset < 1025; offset += 8) {
        size_t count = 1025 - offset < 8 ? 1025 - offset : 8;
        uint64_t word = 0;
        for (size_t i = 0; i < count; i++)
            word |= (uint64_t)network_probe_byte(offset + i) << (8 * i);
        NetworkJobRequest write = {
            seq++, NETWORK_JOB_WRITE, (uint16_t)offset, (uint16_t)count, ticket, word};
        if (network_probe_expect(write) != offset + count)
            sexos_fail();
    }
    NetworkJobRequest start = {seq++, NETWORK_JOB_START, 0, 0, ticket, 0};
    if (network_probe_expect(start) != 0)
        sexos_fail();
    size_t length = 0;
    for (size_t turn = 0; turn < 4096; turn++) {
        NetworkJobRequest status = {seq++, NETWORK_JOB_STATUS, 0, 0, ticket, 0};
        NetworkJobStatusDecode decoded = network_job_decode_status(network_probe_expect(status));
        if (!decoded.ok)
            sexos_fail();
        if (decoded.state == NETWORK_JOB_COMPLETE) {
            length = decoded.response_length;
            break;
        }
        if (decoded.state == NETWORK_JOB_FAILED || decoded.state == NETWORK_JOB_CANCELLED ||
            decoded.state == NETWORK_JOB_IDLE)
            sexos_fail();
        sexos_yield();
    }
    if (length != 1025)
        sexos_fail();
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (size_t offset = 0; offset < length; offset += 4) {
        size_t count = length - offset < 4 ? length - offset : 4;
        NetworkJobRequest read = {seq++,           NETWORK_JOB_READ, (uint16_t)offset,
                                  (uint16_t)count, ticket,           0};
        uint32_t word = network_probe_expect(read);
        for (size_t i = 0; i < count; i++) {
            uint8_t byte = (uint8_t)(word >> (8 * i));
            if (byte != network_probe_byte(offset + i))
                sexos_fail();
            hash = (hash ^ byte) * UINT64_C(0x100000001b3);
        }
    }
    sexos_line("[network.protocol.probe.bytes] ticket=%u length=%zu hash=%#llx exact=1", ticket,
               length, (unsigned long long)hash);
    NetworkJobRequest release = {seq++, NETWORK_JOB_RELEASE, 0, 0, ticket, 0};
    if (network_probe_expect(release) != 0)
        sexos_fail();
    sexos_line(
        "[network.protocol.probe.done] mode=correlated ticket=%u bytes=%zu positive_release=1",
        ticket, length);
    for (;;)
        sexos_yield();
}


