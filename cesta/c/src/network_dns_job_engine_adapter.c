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
#include "network_dns_job_engine.h"

static uint32_t dns_engine_error(NetworkEngineError error) {
    return error == NETWORK_ENGINE_STOPPED ? NETWORK_JOB_OFFLINE : NETWORK_JOB_IO;
}

static bool dns_engine_begin(void *opaque, uint32_t ticket, uint32_t *error) {
    NetworkDnsJobEngineContext *context = opaque;
    if (!context || !context->engine) {
        *error = NETWORK_JOB_OFFLINE;
        return false;
    }
    NetworkEngineError result = network_engine_transport_begin_dns(context->engine, ticket);
    *error = dns_engine_error(result);
    return result == NETWORK_ENGINE_OK;
}

static bool dns_engine_receive(void *opaque, uint8_t *out, size_t capacity, bool *present,
                               size_t *count, uint32_t *error) {
    NetworkDnsJobEngineContext *context = opaque;
    if (!context || !context->engine || !out || capacity < NETWORK_DNS_MAX_BYTES) {
        *error = NETWORK_JOB_IO;
        return false;
    }
    NetworkEngineDnsResult result = network_engine_transport_dns_receive(context->engine);
    if (!result.ok) {
        *error = dns_engine_error(result.error);
        return false;
    }
    *present = result.present;
    *count = result.present ? result.frame.len : 0;
    if (*count > capacity) {
        *error = NETWORK_JOB_IO;
        return false;
    }
    for (size_t i = 0; i < *count; i++)
        out[i] = result.frame.bytes[i];
    *error = 0;
    return true;
}

static void dns_engine_abort(void *opaque) {
    NetworkDnsJobEngineContext *context = opaque;
    if (context && context->engine)
        network_engine_transport_dns_abort(context->engine);
}

NetworkDnsJobWire network_dns_job_engine_wire(NetworkDnsJobEngineContext *context) {
    return (NetworkDnsJobWire){context, dns_engine_begin, dns_engine_receive, dns_engine_abort};
}


