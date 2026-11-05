
#ifndef SEXOS_NETWORK_DNS_JOB_MACHINE_H
#define SEXOS_NETWORK_DNS_JOB_MACHINE_H
#include "network_dns_job_abi.h"
#include "network_job_machine.h"
#include "network_dns_codec.h"

enum {
    NETWORK_DNS_JOB_SLOTS = 6
};

#define NETWORK_DNS_JOB_TICKS_PER_SECOND UINT64_C(1000)
#define NETWORK_DNS_JOB_MAX_TTL_SECONDS UINT32_C(60)

typedef struct NetworkDnsJobResolved {
    NetworkJobIdentity owner;
    uint32_t generation;
    uint8_t address[4];
    uint64_t observed_tick, expires_tick;
} NetworkDnsJobResolved;

typedef struct NetworkDnsJobResolvedResult {
    bool present;
    NetworkDnsJobResolved value;
} NetworkDnsJobResolvedResult;

typedef struct NetworkDnsJobWire {
    void *context;
    bool (*begin)(void *context, uint32_t ticket, uint32_t *error);
    bool (*receive)(void *context, uint8_t *out, size_t capacity, bool *present, size_t *count,
                    uint32_t *error);
    void (*abort)(void *context);
} NetworkDnsJobWire;

typedef struct NetworkDnsJobWireResult {
    bool ok, present;
    size_t count;
    uint32_t error;
} NetworkDnsJobWireResult;

typedef struct NetworkDnsJobWriteView {
    uint8_t *data;
    size_t len;
} NetworkDnsJobWriteView;

typedef struct NetworkDnsJobReceipt {
    bool present;
    NetworkJobIdentity owner;
    NetworkDnsJobRequest request;
    uint64_t word;
} NetworkDnsJobReceipt;

typedef struct NetworkDnsJobPendingReply {
    bool present;
    NetworkJobIdentity owner;
    uint64_t word;
} NetworkDnsJobPendingReply;

typedef struct NetworkDnsJobRecord {
    bool present;
    NetworkDnsJobResolved value;
} NetworkDnsJobRecord;

typedef struct NetworkDnsJobMachine {
    bool owner_present;
    NetworkJobIdentity owner;
    uint32_t ticket, next_ticket;
    NetworkDnsJobState state;
    uint64_t deadline, last;
    bool answer_present;
    NetworkDnsAnswer answer;
    bool candidate_present;
    NetworkDnsJobResolved candidate;
    NetworkDnsJobRecord records[NETWORK_DNS_JOB_SLOTS];
    uint64_t records_clock;
    uint32_t failure;
    uint8_t receive[NETWORK_DNS_MAX_BYTES];
    NetworkDnsJobReceipt pending[NETWORK_DNS_JOB_SLOTS], cached[NETWORK_DNS_JOB_SLOTS];
    size_t cursor;
} NetworkDnsJobMachine;

typedef struct NetworkDnsJobSnapshot {
    NetworkDnsJobState state;
    bool owner_present;
    NetworkJobIdentity owner;
    uint32_t ticket, next_ticket, failure;
    size_t records;
    bool candidate_present;
} NetworkDnsJobSnapshot;

typedef bool (*NetworkDnsJobLive)(void *context, NetworkJobIdentity owner);
bool network_dns_job_wire_valid(NetworkDnsJobWire wire);
NetworkDnsJobWireResult network_dns_job_wire_begin(NetworkDnsJobWire wire, uint32_t ticket);
NetworkDnsJobWireResult network_dns_job_wire_receive(NetworkDnsJobWire wire,
                                                     NetworkDnsJobWriteView out, size_t offset,
                                                     size_t length);
void network_dns_job_wire_abort(NetworkDnsJobWire wire);
void network_dns_job_machine_init(NetworkDnsJobMachine *machine);
NetworkDnsJobSnapshot network_dns_job_machine_snapshot(const NetworkDnsJobMachine *machine);
bool network_dns_job_machine_can_transmit(const NetworkDnsJobMachine *machine, uint32_t ticket);
NetworkDnsJobResolvedResult network_dns_job_machine_resolved(const NetworkDnsJobMachine *machine,
                                                             NetworkJobIdentity owner,
                                                             uint32_t generation, uint64_t now);
size_t network_dns_job_machine_owned_records(const NetworkDnsJobMachine *machine,
                                             NetworkJobIdentity owner);
void network_dns_job_machine_maintain_records(NetworkDnsJobMachine *machine, uint64_t now,
                                              void *context, NetworkDnsJobLive live);
NetworkDnsJobPendingReply
network_dns_job_machine_pending_reply(const NetworkDnsJobMachine *machine);
void network_dns_job_machine_reply_accepted(NetworkDnsJobMachine *machine);
void network_dns_job_machine_reply_deferred(NetworkDnsJobMachine *machine);
bool network_dns_job_machine_retire(NetworkDnsJobMachine *machine, NetworkJobIdentity owner,
                                    NetworkDnsJobWire wire);
bool network_dns_job_machine_handle(NetworkDnsJobMachine *machine, NetworkJobIdentity owner,
                                    NetworkDnsJobRequest request, uint64_t now, bool other_held,
                                    NetworkDnsJobWire wire);
void network_dns_job_machine_advance(NetworkDnsJobMachine *machine, uint64_t now,
                                     NetworkDnsJobWire wire);
#endif
