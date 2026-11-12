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
#include "network_web.h"
#include "sexos_server.h"

static void network_web_zero(void *pointer, uint8_t value, size_t bytes) {
    uint8_t *out = pointer;
    for (size_t i = 0; i < bytes; ++i)
        out[i] = value;
}

static bool network_web_owner_valid(NetworkJobIdentity owner, uint8_t number) {
    return owner.pd && owner.task && number < NETWORK_WEB_CLIENT_FLOWS;
}

void network_web_init(NetworkWebService *service) {
    *service = (NetworkWebService){0};
}

void network_web_dispose(NetworkWebService *service) {
    if (service->flows)
        sexos_free(service->flows, service->flow_capacity * sizeof *service->flows,
                   _Alignof(NetworkWebFlow));
    if (service->sockets)
        sexos_free(service->sockets, service->socket_capacity * sizeof *service->sockets,
                   _Alignof(NetworkWebSocket));
    *service = (NetworkWebService){0};
}

size_t network_web_flow_find(const NetworkWebService *service, NetworkJobIdentity owner,
                             uint8_t number) {
    for (size_t i = 0; i < service->flow_count; ++i) {
        const NetworkWebFlow *flow = &service->flows[i];
        if (flow->present && flow->number == number && flow->owner.pd == owner.pd &&
            flow->owner.task == owner.task)
            return i;
    }
    return NETWORK_WEB_NO_INDEX;
}

size_t network_web_socket_find(const NetworkWebService *service, NetworkJobIdentity owner,
                               uint8_t number) {
    for (size_t i = 0; i < service->socket_count; ++i) {
        const NetworkWebSocket *socket = &service->sockets[i];
        if (socket->present && socket->number == number && socket->owner.pd == owner.pd &&
            socket->owner.task == owner.task)
            return i;
    }
    return NETWORK_WEB_NO_INDEX;
}

static bool network_web_grow_flows(NetworkWebService *service) {
    if (service->flow_capacity >= NETWORK_WEB_MAX_FLOWS)
        return false;
    size_t capacity = service->flow_capacity ? service->flow_capacity * 2 : 16;
    if (capacity > NETWORK_WEB_MAX_FLOWS)
        capacity = NETWORK_WEB_MAX_FLOWS;
    if (capacity > SIZE_MAX / sizeof *service->flows)
        return false;
    NetworkWebFlow *flows = sexos_realloc(service->flows,
        service->flow_capacity * sizeof *service->flows, capacity * sizeof *flows,
        _Alignof(NetworkWebFlow));
    if (!flows)
        return false;
    network_web_zero(flows + service->flow_capacity, 0,
           (capacity - service->flow_capacity) * sizeof *flows);
    service->flows = flows;
    service->flow_capacity = capacity;
    return true;
}

static bool network_web_grow_sockets(NetworkWebService *service) {
    if (service->socket_capacity >= NETWORK_WEB_MAX_SOCKETS)
        return false;
    size_t capacity = service->socket_capacity ? service->socket_capacity * 2 : 16;
    if (capacity > NETWORK_WEB_MAX_SOCKETS)
        capacity = NETWORK_WEB_MAX_SOCKETS;
    if (capacity > SIZE_MAX / sizeof *service->sockets)
        return false;
    NetworkWebSocket *sockets = sexos_realloc(service->sockets,
        service->socket_capacity * sizeof *service->sockets, capacity * sizeof *sockets,
        _Alignof(NetworkWebSocket));
    if (!sockets)
        return false;
    network_web_zero(sockets + service->socket_capacity, 0,
           (capacity - service->socket_capacity) * sizeof *sockets);
    service->sockets = sockets;
    service->socket_capacity = capacity;
    return true;
}

NetworkWebAcquire network_web_flow_acquire(NetworkWebService *service, NetworkJobIdentity owner,
                                           uint8_t number) {
    if (!network_web_owner_valid(owner, number))
        return (NetworkWebAcquire){NETWORK_WEB_NO_INDEX, false, false};
    size_t found = network_web_flow_find(service, owner, number);
    if (found != NETWORK_WEB_NO_INDEX)
        return (NetworkWebAcquire){found, true, false};
    for (size_t i = 0; i < service->flow_count; ++i)
        if (!service->flows[i].present) {
            service->flows[i].present = true;
            service->flows[i].owner = owner;
            service->flows[i].number = number;
            return (NetworkWebAcquire){i, true, false};
        }
    if (service->flow_count >= NETWORK_WEB_MAX_FLOWS ||
        (service->flow_count == service->flow_capacity && !network_web_grow_flows(service)))
        return (NetworkWebAcquire){NETWORK_WEB_NO_INDEX, false, false};
    size_t i = service->flow_count++;
    service->flows[i].present = true;
    service->flows[i].owner = owner;
    service->flows[i].number = number;
    return (NetworkWebAcquire){i, true, true};
}

NetworkWebAcquire network_web_socket_acquire(NetworkWebService *service, NetworkJobIdentity owner,
                                             uint8_t number) {
    if (!network_web_owner_valid(owner, number))
        return (NetworkWebAcquire){NETWORK_WEB_NO_INDEX, false, false};
    size_t found = network_web_socket_find(service, owner, number);
    if (found != NETWORK_WEB_NO_INDEX)
        return (NetworkWebAcquire){found, true, false};
    for (size_t i = 0; i < service->socket_count; ++i)
        if (!service->sockets[i].present) {
            service->sockets[i].present = true;
            service->sockets[i].owner = owner;
            service->sockets[i].number = number;
            return (NetworkWebAcquire){i, true, false};
        }
    if (service->socket_count >= NETWORK_WEB_MAX_SOCKETS ||
        (service->socket_count == service->socket_capacity && !network_web_grow_sockets(service)))
        return (NetworkWebAcquire){NETWORK_WEB_NO_INDEX, false, false};
    size_t i = service->socket_count++;
    service->sockets[i].present = true;
    service->sockets[i].owner = owner;
    service->sockets[i].number = number;
    return (NetworkWebAcquire){i, true, true};
}

bool network_web_flow_release(NetworkWebService *service, NetworkJobIdentity owner, uint8_t number) {
    size_t i = network_web_flow_find(service, owner, number);
    if (i == NETWORK_WEB_NO_INDEX)
        return false;
    NetworkWebFlow *flow = &service->flows[i];
    flow->present = false;
    flow->owner = (NetworkJobIdentity){0, 0};
    flow->number = 0;
    flow->listening = 0;
    flow->out_at = flow->out_len = 0;
    return true;
}

bool network_web_socket_release(NetworkWebService *service, NetworkJobIdentity owner,
                                uint8_t number) {
    size_t i = network_web_socket_find(service, owner, number);
    if (i == NETWORK_WEB_NO_INDEX)
        return false;
    service->sockets[i] = (NetworkWebSocket){0};
    return true;
}

NetworkWebRetirement network_web_retire(NetworkWebService *service, NetworkJobIdentity owner) {
    NetworkWebRetirement retired = {0};
    for (size_t i = 0; i < service->flow_count; ++i) {
        NetworkWebFlow *flow = &service->flows[i];
        if (flow->present && flow->owner.pd == owner.pd && flow->owner.task == owner.task) {
            flow->present = false;
            flow->owner = (NetworkJobIdentity){0, 0};
            flow->number = 0;
            flow->listening = 0;
            flow->out_at = flow->out_len = 0;
            ++retired.flows;
        }
    }
    for (size_t i = 0; i < service->socket_count; ++i)
        if (service->sockets[i].present && service->sockets[i].owner.pd == owner.pd &&
            service->sockets[i].owner.task == owner.task) {
            service->sockets[i] = (NetworkWebSocket){0};
            ++retired.sockets;
        }
    if (service->resolving_present && service->resolving_owner.pd == owner.pd &&
        service->resolving_owner.task == owner.task) {
        service->resolving_present = false;
        service->resolving_owner = (NetworkJobIdentity){0, 0};
        service->resolving_since = 0;
        retired.resolving = true;
    }
    return retired;
}

bool network_web_resolve_begin(NetworkWebService *service, NetworkJobIdentity owner, uint64_t now) {
    if (!owner.pd || !owner.task)
        return false;
    if (service->resolving_present &&
        (service->resolving_owner.pd != owner.pd || service->resolving_owner.task != owner.task)) {
        uint64_t held = now >= service->resolving_since ? now - service->resolving_since : 0;
        if (held < NETWORK_WEB_RESOLVE_HOLD_TICKS)
            return false;
    }
    service->resolving_present = true;
    service->resolving_owner = owner;
    service->resolving_since = now;
    return true;
}

bool network_web_resolve_finish(NetworkWebService *service, NetworkJobIdentity owner) {
    if (!service->resolving_present || service->resolving_owner.pd != owner.pd ||
        service->resolving_owner.task != owner.task)
        return false;
    service->resolving_present = false;
    service->resolving_owner = (NetworkJobIdentity){0, 0};
    service->resolving_since = 0;
    return true;
}
