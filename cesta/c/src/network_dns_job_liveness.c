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

void network_dns_job_machine_maintain_records(NetworkDnsJobMachine *m, uint64_t now, void *context,
                                              NetworkDnsJobLive live) {
    if (now < m->records_clock) {
        for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++)
            m->records[i].present = false;
        m->candidate_present = false;
        return;
    }
    m->records_clock = now;
    if (m->candidate_present &&
        (now < m->candidate.observed_tick || now >= m->candidate.expires_tick))
        m->candidate_present = false;
    for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++) {
        if (m->records[i].present &&
            (now < m->records[i].value.observed_tick || now >= m->records[i].value.expires_tick))
            m->records[i].present = false;
        bool candidate_here = m->candidate_present && m->cached[i].present &&
                              network_job_identity_equal(m->cached[i].owner, m->candidate.owner);
        bool present = m->records[i].present || candidate_here;
        NetworkJobIdentity owner =
            m->records[i].present ? m->records[i].value.owner : m->candidate.owner;
        if (present && (!live || !live(context, owner))) {
            m->records[i].present = false;
            if (candidate_here)
                m->candidate_present = false;
        }
    }
}


