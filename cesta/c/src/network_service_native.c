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
#include "network_service_core.h"

bool network_service_native_job(NetworkJobMachine *jobs, NetworkEngineTransport *transport,
                                NetworkJobIdentity owner, NetworkJobRequest request, uint64_t now,
                                bool other_held, const NetworkJobResolution *resolution,
                                uint32_t admission_error) {
    NetworkJobEngineContext context = {
        transport, request.kind == NETWORK_JOB_BEGIN_DUPLEX_RESOLVED || jobs->duplex};
    NetworkJobWire wire = network_job_engine_wire(&context);
    return network_job_machine_handle_resolved(jobs, owner, request, now, other_held, resolution,
                                               admission_error, wire);
}

bool network_service_native_dns_job(NetworkDnsJobMachine *jobs, NetworkEngineTransport *transport,
                                    NetworkJobIdentity owner, NetworkDnsJobRequest request,
                                    uint64_t now, bool other_held) {
    NetworkDnsJobEngineContext context = {transport};
    NetworkDnsJobWire wire = network_dns_job_engine_wire(&context);
    return network_dns_job_machine_handle(jobs, owner, request, now, other_held, wire);
}

void network_service_native_advance(NetworkJobMachine *jobs, NetworkDnsJobMachine *dns_jobs,
                                    NetworkEngineTransport *transport, uint64_t now) {
    network_engine_transport_advance(transport, now);
    NetworkJobEngineContext context = {transport, jobs->duplex};
    NetworkJobWire wire = network_job_engine_wire(&context);
    network_job_machine_advance(jobs, now, wire);
    NetworkDnsJobEngineContext dns_context = {transport};
    NetworkDnsJobWire dns_wire = network_dns_job_engine_wire(&dns_context);
    network_dns_job_machine_advance(dns_jobs, now, dns_wire);
}

bool network_service_native_retire(NetworkJobMachine *jobs, NetworkDnsJobMachine *dns_jobs,
                                   NetworkEngineTransport *transport, NetworkJobIdentity owner) {
    NetworkJobEngineContext context = {transport, jobs->duplex};
    NetworkJobWire wire = network_job_engine_wire(&context);
    bool client = network_job_machine_retire(jobs, owner, wire);
    NetworkDnsJobEngineContext dns_context = {transport};
    NetworkDnsJobWire dns_wire = network_dns_job_engine_wire(&dns_context);
    bool dns = network_dns_job_machine_retire(dns_jobs, owner, dns_wire);
    return client || dns;
}


