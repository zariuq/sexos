#include "network_dns_job_machine.h"
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

static bool dns_job_request_equal(NetworkDnsJobRequest a, NetworkDnsJobRequest b) {
    return a.seq == b.seq && a.kind == b.kind && a.ticket == b.ticket;
}

static bool dns_job_valid_time(const NetworkDnsJobMachine *m, const NetworkDnsJobResolved *record,
                               uint64_t now) {
    return now >= m->records_clock && now >= record->observed_tick && now < record->expires_tick;
}

static size_t dns_job_pending_slot(const NetworkDnsJobMachine *m) {
    for (size_t offset = 0; offset < NETWORK_DNS_JOB_SLOTS; offset++) {
        size_t slot = (m->cursor + offset) % NETWORK_DNS_JOB_SLOTS;
        if (m->pending[slot].present)
            return slot;
    }
    return NETWORK_DNS_JOB_SLOTS;
}

static void dns_job_clear_receive(NetworkDnsJobMachine *m) {
    for (size_t i = 0; i < sizeof(m->receive); i++)
        m->receive[i] = 0;
}

void network_dns_job_machine_init(NetworkDnsJobMachine *m) {
    *m = (NetworkDnsJobMachine){0};
    m->next_ticket = 1;
    m->state = NETWORK_DNS_JOB_IDLE;
}

NetworkDnsJobSnapshot network_dns_job_machine_snapshot(const NetworkDnsJobMachine *m) {
    size_t records = 0;
    for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++)
        if (m->records[i].present)
            records++;
    return (NetworkDnsJobSnapshot){m->state,  m->owner_present,    m->owner,
                                   m->ticket, m->next_ticket,      m->failure,
                                   records,   m->candidate_present};
}

bool network_dns_job_machine_can_transmit(const NetworkDnsJobMachine *m, uint32_t ticket) {
    return m->owner_present && m->ticket == ticket && m->state == NETWORK_DNS_JOB_WAITING;
}

NetworkDnsJobResolvedResult network_dns_job_machine_resolved(const NetworkDnsJobMachine *m,
                                                             NetworkJobIdentity owner,
                                                             uint32_t generation, uint64_t now) {
    for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++) {
        const NetworkDnsJobRecord *r = &m->records[i];
        if (r->present && network_job_identity_equal(r->value.owner, owner) &&
            r->value.generation == generation && dns_job_valid_time(m, &r->value, now))
            return (NetworkDnsJobResolvedResult){true, r->value};
    }
    return (NetworkDnsJobResolvedResult){0};
}

size_t network_dns_job_machine_owned_records(const NetworkDnsJobMachine *m,
                                             NetworkJobIdentity owner) {
    size_t count = 0;
    for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++)
        if (m->records[i].present && network_job_identity_equal(m->records[i].value.owner, owner))
            count++;
    return count;
}

NetworkDnsJobPendingReply network_dns_job_machine_pending_reply(const NetworkDnsJobMachine *m) {
    size_t slot = dns_job_pending_slot(m);
    if (slot == NETWORK_DNS_JOB_SLOTS)
        return (NetworkDnsJobPendingReply){0};
    return (NetworkDnsJobPendingReply){true, m->pending[slot].owner, m->pending[slot].word};
}

void network_dns_job_machine_reply_accepted(NetworkDnsJobMachine *m) {
    size_t slot = dns_job_pending_slot(m);
    if (slot != NETWORK_DNS_JOB_SLOTS) {
        m->pending[slot].present = false;
        m->cursor = (slot + 1) % NETWORK_DNS_JOB_SLOTS;
    }
}

void network_dns_job_machine_reply_deferred(NetworkDnsJobMachine *m) {
    size_t slot = dns_job_pending_slot(m);
    if (slot != NETWORK_DNS_JOB_SLOTS)
        m->cursor = (slot + 1) % NETWORK_DNS_JOB_SLOTS;
}

static void dns_job_scrub(NetworkDnsJobMachine *m, NetworkDnsJobWire wire) {
    dns_job_clear_receive(m);
    m->answer_present = false;
    m->answer = (NetworkDnsAnswer){0};
    m->candidate_present = false;
    m->candidate = (NetworkDnsJobResolved){0};
    m->deadline = 0;
    m->last = 0;
    network_dns_job_wire_abort(wire);
}

static void dns_job_fail(NetworkDnsJobMachine *m, uint32_t why, NetworkDnsJobWire wire) {
    dns_job_scrub(m, wire);
    m->failure = why;
    m->state = NETWORK_DNS_JOB_FAILED;
}

bool network_dns_job_machine_retire(NetworkDnsJobMachine *m, NetworkJobIdentity owner,
                                    NetworkDnsJobWire wire) {
    if (!network_dns_job_wire_valid(wire))
        return false;
    bool held = m->owner_present && network_job_identity_equal(m->owner, owner);
    if (held) {
        dns_job_scrub(m, wire);
        m->owner_present = false;
        m->ticket = 0;
        m->failure = 0;
        m->state = NETWORK_DNS_JOB_IDLE;
    }
    if (m->candidate_present && network_job_identity_equal(m->candidate.owner, owner))
        m->candidate_present = false;
    for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++) {
        if (m->records[i].present && network_job_identity_equal(m->records[i].value.owner, owner))
            m->records[i].present = false;
        if (m->cached[i].present && network_job_identity_equal(m->cached[i].owner, owner)) {
            m->cached[i].present = false;
            m->pending[i].present = false;
        }
    }
    return held;
}

static bool dns_job_respond(NetworkDnsJobMachine *m, size_t slot, NetworkJobIdentity owner,
                            NetworkDnsJobRequest request, bool success, uint32_t value) {
    NetworkDnsJobReceipt receipt = {true, owner, request,
                                    success ? network_dns_job_reply(request.seq, value)
                                            : network_dns_job_error(request.seq, value)};
    m->pending[slot] = receipt;
    if (!(m->cached[slot].present && request.seq <= m->cached[slot].request.seq))
        m->cached[slot] = receipt;
    return true;
}

static NetworkDnsJobRecord dns_job_observed(const NetworkDnsJobMachine *m, NetworkDnsAnswer answer,
                                            uint64_t now) {
    if (!answer.ttl || now < m->records_clock || !m->owner_present)
        return (NetworkDnsJobRecord){0};
    uint32_t seconds =
        answer.ttl < NETWORK_DNS_JOB_MAX_TTL_SECONDS ? answer.ttl : NETWORK_DNS_JOB_MAX_TTL_SECONDS;
    uint64_t lifetime = (uint64_t)seconds * NETWORK_DNS_JOB_TICKS_PER_SECOND;
    if (UINT64_MAX - now < lifetime)
        return (NetworkDnsJobRecord){0};
    NetworkDnsJobResolved result = {m->owner, m->ticket, {0}, now, now + lifetime};
    for (size_t i = 0; i < 4; i++)
        result.address[i] = answer.address[i];
    return (NetworkDnsJobRecord){true, result};
}

static bool dns_job_apply(NetworkDnsJobMachine *m, NetworkJobIdentity owner,
                          NetworkDnsJobRequest request, uint64_t now, bool other_held,
                          NetworkDnsJobWire wire, uint32_t *value, uint32_t *error) {
    if (!owner.pd || !owner.task || !network_dns_job_request_valid(request)) {
        *error = NETWORK_JOB_INVALID;
        return false;
    }
    if (request.kind == NETWORK_DNS_JOB_BEGIN) {
        if (m->owner_present || other_held) {
            *error = NETWORK_JOB_BUSY;
            return false;
        }
        if (m->next_ticket > NETWORK_JOB_MAX_TICKET) {
            *error = NETWORK_JOB_EXHAUSTED;
            return false;
        }
        if (UINT64_MAX - now < UINT64_C(5000)) {
            *error = NETWORK_JOB_TIMEOUT;
            return false;
        }
        for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++) {
            if (m->cached[i].present && network_job_identity_equal(m->cached[i].owner, owner))
                m->records[i].present = false;
        }
        m->candidate_present = false;
        m->ticket = m->next_ticket++;
        m->owner_present = true;
        m->owner = owner;
        m->state = NETWORK_DNS_JOB_WAITING;
        m->deadline = now + 5000;
        m->last = now;
        m->answer_present = false;
        m->failure = 0;
        dns_job_clear_receive(m);
        NetworkDnsJobWireResult started = network_dns_job_wire_begin(wire, m->ticket);
        if (!started.ok)
            dns_job_fail(m, started.error, wire);
        *value = m->ticket;
        return true;
    }
    if (!m->owner_present || !network_job_identity_equal(m->owner, owner)) {
        *error = NETWORK_JOB_DENIED;
        return false;
    }
    if (m->ticket != request.ticket) {
        *error = NETWORK_JOB_STALE;
        return false;
    }
    if (request.kind == NETWORK_DNS_JOB_STATUS) {
        *value = (uint32_t)m->state;
        return true;
    }
    if (request.kind == NETWORK_DNS_JOB_RESULT) {
        if (m->answer_present && m->state == NETWORK_DNS_JOB_COMPLETE) {
            *value = (uint32_t)m->answer.address[0] << 24 | (uint32_t)m->answer.address[1] << 16 |
                     (uint32_t)m->answer.address[2] << 8 | m->answer.address[3];
            return true;
        }
        *error = m->state == NETWORK_DNS_JOB_FAILED ? m->failure : NETWORK_JOB_BUSY;
        return false;
    }
    if (request.kind == NETWORK_DNS_JOB_CANCEL) {
        dns_job_scrub(m, wire);
        m->failure = 0;
        m->state = NETWORK_DNS_JOB_CANCELLED;
        *value = 0;
        return true;
    }
    if (request.kind == NETWORK_DNS_JOB_RELEASE) {
        NetworkDnsJobRecord promoted = {0};
        if (m->candidate_present && m->state == NETWORK_DNS_JOB_COMPLETE &&
            network_job_identity_equal(m->candidate.owner, owner) &&
            m->candidate.generation == m->ticket && dns_job_valid_time(m, &m->candidate, now))
            promoted = (NetworkDnsJobRecord){true, m->candidate};
        for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++) {
            if (m->cached[i].present && network_job_identity_equal(m->cached[i].owner, owner)) {
                m->records[i] = promoted;
                break;
            }
        }
        if (now > m->records_clock)
            m->records_clock = now;
        dns_job_scrub(m, wire);
        m->owner_present = false;
        m->ticket = 0;
        m->failure = 0;
        m->state = NETWORK_DNS_JOB_IDLE;
        *value = 0;
        return true;
    }
    *error = NETWORK_JOB_INVALID;
    return false;
}

bool network_dns_job_machine_handle(NetworkDnsJobMachine *m, NetworkJobIdentity owner,
                                    NetworkDnsJobRequest request, uint64_t now, bool other_held,
                                    NetworkDnsJobWire wire) {
    if (!network_dns_job_wire_valid(wire))
        return false;
    size_t slot = NETWORK_DNS_JOB_SLOTS;
    for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++)
        if (m->cached[i].present && network_job_identity_equal(m->cached[i].owner, owner)) {
            slot = i;
            break;
        }
    if (slot == NETWORK_DNS_JOB_SLOTS) {
        for (size_t i = 0; i < NETWORK_DNS_JOB_SLOTS; i++) {
            if (!m->cached[i].present) {
                slot = i;
                break;
            }
        }
    }
    if (slot == NETWORK_DNS_JOB_SLOTS)
        return false;
    if (m->pending[slot].present)
        return dns_job_request_equal(m->pending[slot].request, request);
    if (m->cached[slot].present) {
        if (dns_job_request_equal(m->cached[slot].request, request)) {
            m->pending[slot] = m->cached[slot];
            return true;
        }
        if (request.seq <= m->cached[slot].request.seq)
            return dns_job_respond(m, slot, owner, request, false, NETWORK_JOB_STALE);
    }
    uint32_t value = 0, error = 0;
    bool success;
    if (other_held && request.kind == NETWORK_DNS_JOB_BEGIN) {
        success = false;
        error = NETWORK_JOB_BUSY;
    } else
        success = dns_job_apply(m, owner, request, now, other_held, wire, &value, &error);
    return dns_job_respond(m, slot, owner, request, success, success ? value : error);
}

void network_dns_job_machine_advance(NetworkDnsJobMachine *m, uint64_t now,
                                     NetworkDnsJobWire wire) {
    if (!network_dns_job_wire_valid(wire) || m->state != NETWORK_DNS_JOB_WAITING)
        return;
    if (now < m->last || now >= m->deadline) {
        dns_job_fail(m, NETWORK_JOB_TIMEOUT, wire);
        return;
    }
    m->last = now;
    dns_job_clear_receive(m);
    NetworkDnsJobWireResult received;
    {
        NetworkDnsJobWriteView out = {m->receive, sizeof(m->receive)};
        received = network_dns_job_wire_receive(wire, out, 0, sizeof(m->receive));
    }
    if (!received.ok || received.count > sizeof(m->receive)) {
        uint32_t error = received.ok ? NETWORK_JOB_IO : received.error;
        dns_job_clear_receive(m);
        dns_job_fail(m, error, wire);
        return;
    }
    if (!received.present) {
        dns_job_clear_receive(m);
        return;
    }
    NetworkDnsFrame frame = {0};
    frame.len = received.count;
    for (size_t i = 0; i < received.count; i++)
        frame.bytes[i] = m->receive[i];
    NetworkDnsParseResult parsed = network_dns_parse((uint16_t)m->ticket, &frame);
    dns_job_clear_receive(m);
    if (parsed.kind == NETWORK_DNS_RESULT_UNRELATED)
        return;
    if (parsed.kind != NETWORK_DNS_RESULT_ANSWER) {
        dns_job_fail(m, NETWORK_JOB_IO, wire);
        return;
    }
    NetworkDnsJobRecord observed = dns_job_observed(m, parsed.answer, now);
    m->candidate_present = observed.present;
    m->candidate = observed.value;
    if (now > m->records_clock)
        m->records_clock = now;
    m->answer = parsed.answer;
    m->answer_present = true;
    m->state = NETWORK_DNS_JOB_COMPLETE;
    m->deadline = 0;
    network_dns_job_wire_abort(wire);
}


