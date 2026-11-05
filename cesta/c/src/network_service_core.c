#include "network_service_core.h"
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

NetworkEngineError network_service_core_init(NetworkServiceCore *core, NetConfig config,
                                             NetworkEngineMac mac, uint64_t now) {
    NetworkEngineError error = network_engine_transport_init(&core->transport, config, mac, now);
    if (error)
        return error;
    network_job_machine_init(&core->jobs);
    network_dns_job_machine_init(&core->dns_jobs);
    return NETWORK_ENGINE_OK;
}

bool network_service_core_job(NetworkServiceCore *core, NetworkJobIdentity owner,
                              NetworkJobRequest request, uint64_t now, bool other_held,
                              const NetworkJobResolution *resolution, uint32_t admission_error) {
    return network_service_native_job(&core->jobs, &core->transport, owner, request, now,
                                      other_held || core->dns_jobs.owner_present, resolution,
                                      admission_error);
}

bool network_service_core_dns_job(NetworkServiceCore *core, NetworkJobIdentity owner,
                                  NetworkDnsJobRequest request, uint64_t now, bool other_held) {
    return network_service_native_dns_job(&core->dns_jobs, &core->transport, owner, request, now,
                                          other_held || core->jobs.owner_present);
}

void network_service_core_advance(NetworkServiceCore *core, uint64_t now) {
    network_service_native_advance(&core->jobs, &core->dns_jobs, &core->transport, now);
}

NetworkEngineError network_service_core_submit(NetworkServiceCore *core,
                                               const NetworkEngineFrame *frame) {
    return network_engine_transport_submit(&core->transport, frame);
}

NetworkServiceFrameResult network_service_core_take_frame(NetworkServiceCore *core, bool live,
                                                          NetworkJobIdentity live_owner) {
    NetworkEngineFrameResult taken = network_engine_transport_take_frame(&core->transport);
    if (!taken.present)
        return (NetworkServiceFrameResult){NETWORK_SERVICE_NO_FRAME, false, 0, {0}};
    NetworkEngineTicket client =
        network_engine_client_ticket(&taken.frame, core->transport.base.config);
    if (client.present) {
        bool owned = live && core->jobs.owner_present &&
                     network_job_identity_equal(core->jobs.owner, live_owner) &&
                     network_job_machine_can_transmit(&core->jobs, client.value);
        return (NetworkServiceFrameResult){
            owned ? NETWORK_SERVICE_CLIENT_FRAME : NETWORK_SERVICE_STALE_FRAME, owned, client.value,
            owned ? taken.frame : (NetworkEngineFrame){0}};
    }
    NetworkEngineTicket dns = network_engine_dns_ticket(&taken.frame, core->transport.base.config);
    if (dns.present) {
        bool owned = live && core->dns_jobs.owner_present &&
                     network_job_identity_equal(core->dns_jobs.owner, live_owner) &&
                     network_dns_job_machine_can_transmit(&core->dns_jobs, dns.value);
        return (NetworkServiceFrameResult){
            owned ? NETWORK_SERVICE_DNS_FRAME : NETWORK_SERVICE_STALE_FRAME, owned, dns.value,
            owned ? taken.frame : (NetworkEngineFrame){0}};
    }
    return (NetworkServiceFrameResult){NETWORK_SERVICE_SHARED_FRAME, true, 0, taken.frame};
}

NetworkJobPendingReply network_service_core_pending_reply(const NetworkServiceCore *core) {
    return network_job_machine_pending_reply(&core->jobs);
}

void network_service_core_reply_accepted(NetworkServiceCore *core) {
    network_job_machine_reply_accepted(&core->jobs);
}

void network_service_core_reply_deferred(NetworkServiceCore *core) {
    network_job_machine_reply_deferred(&core->jobs);
}

NetworkDnsJobPendingReply network_service_core_dns_pending_reply(const NetworkServiceCore *core) {
    return network_dns_job_machine_pending_reply(&core->dns_jobs);
}

void network_service_core_dns_reply_accepted(NetworkServiceCore *core) {
    network_dns_job_machine_reply_accepted(&core->dns_jobs);
}

void network_service_core_dns_reply_deferred(NetworkServiceCore *core) {
    network_dns_job_machine_reply_deferred(&core->dns_jobs);
}

bool network_service_core_retire(NetworkServiceCore *core, NetworkJobIdentity owner) {
    return network_service_native_retire(&core->jobs, &core->dns_jobs, &core->transport, owner);
}

NetworkServiceSnapshot network_service_core_snapshot(const NetworkServiceCore *core) {
    NetworkJobPendingReply reply = network_job_machine_pending_reply(&core->jobs);
    NetworkDnsJobPendingReply dns_reply = network_dns_job_machine_pending_reply(&core->dns_jobs);
    bool owner_present = core->jobs.owner_present || core->dns_jobs.owner_present;
    NetworkJobIdentity owner = core->jobs.owner_present ? core->jobs.owner : core->dns_jobs.owner;
    return (NetworkServiceSnapshot){network_engine_transport_snapshot(&core->transport),
                                    network_job_machine_snapshot(&core->jobs),
                                    network_dns_job_machine_snapshot(&core->dns_jobs),
                                    owner_present,
                                    owner,
                                    reply.present,
                                    dns_reply.present};
}


