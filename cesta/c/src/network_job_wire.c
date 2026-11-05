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
#include "network_job_machine.h"

bool network_job_wire_valid(NetworkJobWire wire) {
    return wire.begin && wire.receive && wire.send && wire.queued && wire.link &&
           wire.finish_send && wire.abort;
}

NetworkJobWireResult network_job_wire_begin(NetworkJobWire wire, uint32_t ticket) {
    uint32_t error = NETWORK_JOB_IO;
    bool ok = wire.begin(wire.context, ticket, &error);
    return (NetworkJobWireResult){ok, 0, error};
}

NetworkJobWireResult network_job_wire_receive(NetworkJobWire wire, NetworkJobWriteView out,
                                              size_t offset, size_t length) {
    if (offset > out.len || length > out.len - offset)
        return (NetworkJobWireResult){false, 0, NETWORK_JOB_INVALID};
    size_t count = 0;
    uint32_t error = NETWORK_JOB_IO;
    bool ok = wire.receive(wire.context, out.data + offset, length, &count, &error);
    return (NetworkJobWireResult){ok, count, error};
}

NetworkJobWireResult network_job_wire_send(NetworkJobWire wire, NetworkJobReadView data,
                                           size_t offset, size_t length) {
    if (offset > data.len || length > data.len - offset)
        return (NetworkJobWireResult){false, 0, NETWORK_JOB_INVALID};
    size_t count = 0;
    uint32_t error = NETWORK_JOB_IO;
    bool ok = wire.send(wire.context, data.data + offset, length, &count, &error);
    return (NetworkJobWireResult){ok, count, error};
}

size_t network_job_wire_queued(NetworkJobWire wire) {
    return wire.queued(wire.context);
}

NetworkJobLink network_job_wire_link(NetworkJobWire wire) {
    return wire.link(wire.context);
}

void network_job_wire_finish_send(NetworkJobWire wire) {
    wire.finish_send(wire.context);
}

void network_job_wire_abort(NetworkJobWire wire) {
    wire.abort(wire.context);
}


