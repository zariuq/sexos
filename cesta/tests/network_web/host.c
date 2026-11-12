#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "network_web.h"
#include "program_memory.h"
#include "sexos_server.h"

__attribute__((aligned(4096))) static uint8_t backing[32 * 1024 * 1024];
static uint64_t held;
static bool deny;
static NetworkWebService service;

SexosProgramMemory sexos_program_memory_grow(uint64_t pages)
{
    if (deny) return (SexosProgramMemory){PROGRAM_MEMORY_DENIED, (uintptr_t)backing, held, 0};
    held += pages;
    assert(held * 4096 <= sizeof backing);
    return (SexosProgramMemory){PROGRAM_MEMORY_OK, (uintptr_t)backing, held, 0};
}

SexosMessage sexos_listen_once(uint64_t slot)
{
    (void)slot;
    return (SexosMessage){0};
}

void sexos_yield(void) {}
void sexos_heap_reset_for_restart(void);

static void reset(void)
{
    held = 0;
    deny = false;
    sexos_heap_reset_for_restart();
    sexos_heap_init();
    network_web_init(&service);
}

static void finish(void)
{
    network_web_dispose(&service);
    assert(!service.flows && !service.sockets && !service.flow_count && !service.socket_count);
}

static NetworkJobIdentity owner_for(size_t n)
{
    return (NetworkJobIdentity){(uint32_t)(n / 255 + 1), (uint32_t)(n / 255 + 100)};
}

static void exact_owner_and_retirement(void)
{
    reset();
    NetworkJobIdentity a = {15, 100}, stale = {15, 101}, b = {30, 100};
    NetworkWebAcquire af = network_web_flow_acquire(&service, a, 7);
    NetworkWebAcquire bf = network_web_flow_acquire(&service, b, 7);
    assert(af.present && af.created && bf.present && bf.created && af.index != bf.index);
    NetworkWebAcquire again = network_web_flow_acquire(&service, a, 7);
    assert(again.present && !again.created && again.index == af.index);
    NetworkWebAcquire as = network_web_socket_acquire(&service, a, 7);
    NetworkWebAcquire bs = network_web_socket_acquire(&service, b, 7);
    assert(as.present && bs.present && as.index != bs.index);
    assert(!network_web_flow_release(&service, stale, 7));
    assert(!network_web_socket_release(&service, stale, 7));
    assert(network_web_flow_find(&service, a, 7) == af.index);
    assert(network_web_socket_find(&service, a, 7) == as.index);
    assert(network_web_resolve_begin(&service, a, 10));
    assert(!network_web_resolve_begin(&service, b, 11));
    assert(!network_web_resolve_finish(&service, stale));
    NetworkWebRetirement retired = network_web_retire(&service, a);
    assert(retired.flows == 1 && retired.sockets == 1 && retired.resolving);
    assert(network_web_flow_find(&service, a, 7) == NETWORK_WEB_NO_INDEX);
    assert(network_web_socket_find(&service, a, 7) == NETWORK_WEB_NO_INDEX);
    assert(network_web_flow_find(&service, b, 7) == bf.index);
    assert(network_web_socket_find(&service, b, 7) == bs.index);
    assert(network_web_resolve_begin(&service, b, 11));
    assert(network_web_resolve_begin(&service, a, 12) == false);
    assert(network_web_resolve_begin(&service, a, 5011));
    assert(network_web_resolve_finish(&service, a));
    assert(!network_web_resolve_finish(&service, a));
    assert(network_web_flow_release(&service, b, 7));
    assert(network_web_socket_release(&service, b, 7));
    assert(!network_web_flow_release(&service, b, 7));
    assert(!network_web_flow_acquire(&service, (NetworkJobIdentity){0, 1}, 0).present);
    assert(!network_web_flow_acquire(&service, a, 255).present);
    assert(!network_web_socket_acquire(&service, a, 255).present);
    finish();
}

static void capacity_preservation_and_reuse(void)
{
    reset();
    for (size_t n = 0; n < NETWORK_WEB_MAX_FLOWS; ++n) {
        NetworkJobIdentity owner = owner_for(n);
        NetworkWebAcquire flow = network_web_flow_acquire(&service, owner, n % 255);
        NetworkWebAcquire socket = network_web_socket_acquire(&service, owner, n % 255);
        assert(flow.present && flow.created && flow.index == n);
        assert(socket.present && socket.created && socket.index == n);
        service.flows[n].listening = (uint16_t)n;
        service.flows[n].out[0] = (uint8_t)n;
    }
    assert(service.flow_count == NETWORK_WEB_MAX_FLOWS && service.flow_capacity == NETWORK_WEB_MAX_FLOWS);
    assert(service.socket_count == NETWORK_WEB_MAX_SOCKETS && service.socket_capacity == NETWORK_WEB_MAX_SOCKETS);
    assert(sexos_heap_usage().used > 128 * 1024);
    NetworkWebService before = service;
    NetworkJobIdentity extra = {9999, 9999};
    assert(!network_web_flow_acquire(&service, extra, 0).present);
    assert(!network_web_socket_acquire(&service, extra, 0).present);
    assert(!memcmp(&before, &service, sizeof before));
    for (size_t n = 0; n < NETWORK_WEB_MAX_FLOWS; n += 61) {
        NetworkJobIdentity owner = owner_for(n);
        assert(network_web_flow_find(&service, owner, n % 255) == n);
        assert(network_web_socket_find(&service, owner, n % 255) == n);
        assert(service.flows[n].listening == (uint16_t)n && service.flows[n].out[0] == (uint8_t)n);
    }
    for (size_t n = 0; n < NETWORK_WEB_MAX_FLOWS; n += 2) {
        NetworkJobIdentity owner = owner_for(n);
        assert(network_web_flow_release(&service, owner, n % 255));
        assert(network_web_socket_release(&service, owner, n % 255));
    }
    size_t flow_count = service.flow_count, socket_count = service.socket_count;
    size_t flow_capacity = service.flow_capacity, socket_capacity = service.socket_capacity;
    for (size_t n = 0; n < NETWORK_WEB_MAX_FLOWS; n += 2) {
        NetworkJobIdentity owner = owner_for(n);
        NetworkWebAcquire flow = network_web_flow_acquire(&service, owner, n % 255);
        NetworkWebAcquire socket = network_web_socket_acquire(&service, owner, n % 255);
        assert(flow.present && !flow.created && flow.index == n);
        assert(socket.present && !socket.created && socket.index == n);
    }
    assert(service.flow_count == flow_count && service.socket_count == socket_count);
    assert(service.flow_capacity == flow_capacity && service.socket_capacity == socket_capacity);
    finish();
}

static void allocation_refusal_is_inert(void)
{
    reset();
    void *fixed = sexos_malloc(128 * 1024, 4096);
    assert(fixed);
    deny = true;
    NetworkWebService before = service;
    assert(!network_web_flow_acquire(&service, (NetworkJobIdentity){15, 100}, 0).present);
    assert(!memcmp(&before, &service, sizeof before));
    assert(!network_web_socket_acquire(&service, (NetworkJobIdentity){15, 100}, 0).present);
    assert(!memcmp(&before, &service, sizeof before));
    sexos_free(fixed, 128 * 1024, 4096);
    finish();
}

int main(void)
{
    exact_owner_and_retirement();
    capacity_preservation_and_reuse();
    allocation_refusal_is_inert();
    puts("network web: exact ownership, 4096-entry bounds, preservation, reuse, refusal and retirement passed");
    return 0;
}
