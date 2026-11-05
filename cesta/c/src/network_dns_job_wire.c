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
#include "network_dns_job_machine.h"

bool network_dns_job_wire_valid(NetworkDnsJobWire wire) {
    return wire.begin && wire.receive && wire.abort;
}

NetworkDnsJobWireResult network_dns_job_wire_begin(NetworkDnsJobWire wire, uint32_t ticket) {
    uint32_t error = NETWORK_JOB_OFFLINE;
    if (!network_dns_job_wire_valid(wire))
        return (NetworkDnsJobWireResult){false, false, 0, error};
    bool ok = wire.begin(wire.context, ticket, &error);
    return (NetworkDnsJobWireResult){ok, false, 0, error};
}

NetworkDnsJobWireResult network_dns_job_wire_receive(NetworkDnsJobWire wire,
                                                     NetworkDnsJobWriteView out, size_t offset,
                                                     size_t length) {
    uint32_t error = NETWORK_JOB_OFFLINE;
    bool present = false;
    size_t count = 0;
    if (!network_dns_job_wire_valid(wire) || offset > out.len || length > out.len - offset ||
        length > NETWORK_DNS_MAX_BYTES)
        return (NetworkDnsJobWireResult){false, false, 0, error};
    bool ok = wire.receive(wire.context, out.data + offset, length, &present, &count, &error);
    return (NetworkDnsJobWireResult){ok, present, count, error};
}

void network_dns_job_wire_abort(NetworkDnsJobWire wire) {
    if (network_dns_job_wire_valid(wire))
        wire.abort(wire.context);
}


