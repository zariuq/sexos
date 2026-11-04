
#ifndef SEXOS_NETWORK_SERVICE_CORE_H
#define SEXOS_NETWORK_SERVICE_CORE_H
#include "network_job_engine.h"
#include "network_dns_job_engine.h"
typedef enum NetworkServiceFrameKind { NETWORK_SERVICE_NO_FRAME=0,NETWORK_SERVICE_SHARED_FRAME=1,NETWORK_SERVICE_CLIENT_FRAME=2,NETWORK_SERVICE_DNS_FRAME=3,NETWORK_SERVICE_STALE_FRAME=4 } NetworkServiceFrameKind;
typedef struct NetworkServiceFrameResult { NetworkServiceFrameKind kind; bool present; uint32_t ticket; NetworkEngineFrame frame; } NetworkServiceFrameResult;
typedef struct NetworkServiceCore { NetworkEngineTransport transport; NetworkJobMachine jobs; NetworkDnsJobMachine dns_jobs; } NetworkServiceCore;
typedef struct NetworkServiceSnapshot { NetworkEngineTransportSnapshot transport; NetworkJobSnapshot jobs; NetworkDnsJobSnapshot dns_jobs; bool owner_present; NetworkJobIdentity owner; bool reply_pending,dns_reply_pending; } NetworkServiceSnapshot;
bool network_service_native_job(NetworkJobMachine *jobs,NetworkEngineTransport *transport,NetworkJobIdentity owner,NetworkJobRequest request,uint64_t now,bool other_held,const NetworkJobResolution *resolution,uint32_t admission_error);
bool network_service_native_dns_job(NetworkDnsJobMachine *jobs,NetworkEngineTransport *transport,NetworkJobIdentity owner,NetworkDnsJobRequest request,uint64_t now,bool other_held);
void network_service_native_advance(NetworkJobMachine *jobs,NetworkDnsJobMachine *dns_jobs,NetworkEngineTransport *transport,uint64_t now);
bool network_service_native_retire(NetworkJobMachine *jobs,NetworkDnsJobMachine *dns_jobs,NetworkEngineTransport *transport,NetworkJobIdentity owner);
NetworkEngineError network_service_core_init(NetworkServiceCore *core,NetConfig config,NetworkEngineMac mac,uint64_t now);
bool network_service_core_job(NetworkServiceCore *core,NetworkJobIdentity owner,NetworkJobRequest request,uint64_t now,bool other_held,const NetworkJobResolution *resolution,uint32_t admission_error);
bool network_service_core_dns_job(NetworkServiceCore *core,NetworkJobIdentity owner,NetworkDnsJobRequest request,uint64_t now,bool other_held);
void network_service_core_advance(NetworkServiceCore *core,uint64_t now);
NetworkEngineError network_service_core_submit(NetworkServiceCore *core,const NetworkEngineFrame *frame);
NetworkServiceFrameResult network_service_core_take_frame(NetworkServiceCore *core,bool live,NetworkJobIdentity live_owner);
NetworkJobPendingReply network_service_core_pending_reply(const NetworkServiceCore *core);
void network_service_core_reply_accepted(NetworkServiceCore *core);
void network_service_core_reply_deferred(NetworkServiceCore *core);
NetworkDnsJobPendingReply network_service_core_dns_pending_reply(const NetworkServiceCore *core);
void network_service_core_dns_reply_accepted(NetworkServiceCore *core);
void network_service_core_dns_reply_deferred(NetworkServiceCore *core);
bool network_service_core_retire(NetworkServiceCore *core,NetworkJobIdentity owner);
NetworkServiceSnapshot network_service_core_snapshot(const NetworkServiceCore *core);
#endif
