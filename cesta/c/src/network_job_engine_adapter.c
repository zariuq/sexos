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
#include "network_job_engine.h"

static bool job_engine_begin(void *opaque, uint32_t ticket, uint32_t *error) {
    NetworkJobEngineContext *context = opaque;
    if (!context || !context->engine) {
        *error = NETWORK_JOB_OFFLINE;
        return false;
    }
    NetworkEngineError result =
        network_engine_transport_begin_client(context->engine, ticket, context->tls);
    *error = network_job_engine_error_policy(result);
    return result == NETWORK_ENGINE_OK;
}

static bool job_engine_receive(void *opaque, uint8_t *out, size_t capacity, size_t *count,
                               uint32_t *error) {
    NetworkJobEngineContext *context = opaque;
    if (!context || !context->engine || !out || capacity > NETWORK_TCP_TURN_BYTES) {
        *error = NETWORK_JOB_IO;
        return false;
    }
    NetworkTcpClientReceive result =
        network_engine_transport_client_receive(context->engine, capacity);
    if (!result.ok) {
        *error = NETWORK_JOB_IO;
        return false;
    }
    for (size_t i = 0; i < result.count; i++) {
        out[i] = result.bytes[i];
    }
    *count = result.count;
    *error = 0;
    return true;
}

static bool job_engine_send(void *opaque, const uint8_t *data, size_t length, size_t *count,
                            uint32_t *error) {
    NetworkJobEngineContext *context = opaque;
    if (!context || !context->engine || (!data && length) || length > NETWORK_TCP_TURN_BYTES) {
        *error = NETWORK_JOB_IO;
        return false;
    }
    NetworkEnginePayload payload = {0};
    payload.len = length;
    for (size_t i = 0; i < length; i++)
        payload.bytes[i] = data[i];
    NetworkTcpClientCount result = network_engine_transport_client_send(context->engine, &payload);
    if (!result.ok) {
        *error = NETWORK_JOB_IO;
        return false;
    }
    *count = result.count;
    *error = 0;
    return true;
}

static size_t job_engine_queued(void *opaque) {
    NetworkJobEngineContext *context = opaque;
    return context && context->engine ? network_engine_transport_client_queued(context->engine) : 0;
}

static NetworkJobLink job_engine_link(void *opaque) {
    NetworkJobEngineContext *context = opaque;
    if (!context || !context->engine)
        return NETWORK_JOB_LINK_CLOSED;
    NetworkEngineTransportSnapshot s = network_engine_transport_snapshot(context->engine);
    return network_job_engine_link_policy(s.client_pending, s.base.stopped, s.client.state,
                                          s.client.peer_fin);
}

static void job_engine_finish(void *opaque) {
    NetworkJobEngineContext *context = opaque;
    if (context && context->engine)
        network_engine_transport_client_finish(context->engine);
}

static void job_engine_abort(void *opaque) {
    NetworkJobEngineContext *context = opaque;
    if (context && context->engine)
        network_engine_transport_client_abort(context->engine);
}

NetworkJobWire network_job_engine_wire(NetworkJobEngineContext *context) {
    return (NetworkJobWire){context,           job_engine_begin,  job_engine_receive,
                            job_engine_send,   job_engine_queued, job_engine_link,
                            job_engine_finish, job_engine_abort};
}


