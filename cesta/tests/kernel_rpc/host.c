/* Exercise the real correlated-call kernel alongside the legacy trace boundary. */
#define main legacy_model_main
#include "../kernel_model/host.c"
#undef main

static TrapFrame request(Domain *actor, uint64_t op, uint64_t a, uint64_t b, uint64_t c)
{
    TrapFrame f = {.rax = op, .rdi = a, .rsi = b, .rdx = c};
    kernel.current = actor->id;
    kernel_syscall(&f);
    return f;
}

static void cursor_certificates(void)
{
    Ring ring = {0};
    RingEntry out = {.arg0 = 991};
    assert(ring_push(&ring, 9, (RingEntry){.arg0 = 17}));
    KernelRingContext stale = {.ring = &ring, .subject = 9, .out = &out};
    assert(kernel_ring_inspect(&stale));
    /* Restoring the same head/count must not restore the old certificate. */
    for (unsigned i = 0; i < KERNEL_RING_CAPACITY; ++i) {
        assert(ring_pop(&ring, 9, &out));
        assert(ring_push(&ring, 9, (RingEntry){.arg0 = 100 + i}));
    }
    assert(ring.head == stale.head && ring.count == stale.count);
    assert(!kernel_ring_is_current(&stale));
    out.arg0 = 991;
    assert(!kernel_ring_copy(&stale) && out.arg0 == 991);
    KernelRingContext fresh = {.ring = &ring, .subject = 9, .out = &out};
    assert(kernel_ring_inspect(&fresh) && kernel_ring_copy(&fresh) && kernel_ring_swap(&fresh));
    assert(kernel_ring_advance(&fresh) && !kernel_ring_advance(&fresh));
    assert(!kernel_ring_is_current(&fresh));
    ring.revision = UINT64_MAX - 1;
    assert(ring_push(&ring, 9, (RingEntry){.arg0 = 73}));
    assert(ring.revision == UINT64_MAX && ring.count == 1);
    assert(!ring_push(&ring, 9, (RingEntry){.arg0 = 74}));
    assert(!ring_pop(&ring, 9, &out));
    assert(ring.revision == UINT64_MAX && ring.count == 1);
}

static void reply_certificates(Domain *caller, Domain *peer)
{
    RpcSlot *slot = &caller->rpc[0];
    *slot = (RpcSlot){.id = 9101, .peer = peer->id, .state = RPC_HEARD};
    TrapFrame frame = {.rdi = caller->id, .rsi = 9101, .rdx = 1234};
    KernelReplyContext reply = {.peer = peer, .frame = &frame};
    assert(kernel_reply_inspect(&reply) && kernel_reply_complete(&reply));
    assert(!kernel_reply_is_heard(&reply) && !kernel_reply_complete(&reply));
    assert(kernel_reply_is_ready(&reply) && kernel_reply_publish(&reply));
    frame = (TrapFrame){.rdi = 9101};
    KernelPollContext poll = {.caller = caller, .frame = &frame};
    assert(kernel_poll_inspect(&poll) && kernel_poll_consume(&poll));
    assert(!kernel_poll_is_ready(&poll) && !kernel_poll_consume(&poll));
    /* Publication uses its saved value even if another call reuses the slot. */
    *slot = (RpcSlot){.id = 9102, .peer = peer->id, .state = RPC_HEARD, .value = 888};
    assert(kernel_poll_publish(&poll) && frame.rsi == 1234);
    assert(slot->id == 9102 && slot->state == RPC_HEARD && slot->value == 888);
    *slot = (RpcSlot){0};
}

static void full_width_selectors(void)
{
    memset(&kernel, 0, sizeof kernel);
    memset(pd_of_key, 0, sizeof pd_of_key);
    initialize();
    Domain *caller = domain_of_key(3);
    Domain *peer = domain_get(cap_get(caller, 5)->target);
    const uint64_t upper[] = {UINT64_C(1) << 32, UINT64_C(1) << 40,
                              UINT64_C(1) << 63, UINT64_C(0xffffffff00000000)};
    caller->init_arg = 997;
    caller->init_arg_ready = true;
    for (size_t i = 0; i < sizeof upper / sizeof *upper; ++i) {
        assert(request(caller, SEXOS_SYSCALL_call, upper[i] | 5, CALL_TYPE, 17).rax == ERR_CAP_INVALID);
        assert(request(caller, SEXOS_RPC_call, upper[i] | 5, CALL_TYPE, 17).rax == ERR_CAP_INVALID);
        assert(peer->ring.count == 0 && caller->rpc_next == 0);
        assert(request(caller, SEXOS_SYSCALL_call, upper[i], 0x12, 0).rax == ERR_CAP_INVALID);
        assert(caller->init_arg_ready && caller->init_arg == 997);
    }
    TrapFrame init = request(caller, SEXOS_SYSCALL_call, 0, 0x12, 0);
    assert(init.rax == 0 && init.rsi == 997 && !caller->init_arg_ready);

    assert(request(caller, SEXOS_SYSCALL_call, 5, CALL_TYPE, 17).rax == 0);
    for (size_t i = 0; i < sizeof upper / sizeof *upper; ++i) {
        assert(request(peer, SEXOS_SYSCALL_listen, upper[i], 0, 0).rax == 0);
        assert(request(peer, SEXOS_RPC_listen, upper[i], 0, 0).rax == 0);
        assert(peer->ring.count == 1 && peer->reply_rights[caller->id] == 0);
    }
    TrapFrame heard = request(peer, SEXOS_SYSCALL_listen, 0, 0, 0);
    assert(heard.rax == CALL_TYPE && heard.rsi == caller->id && heard.rdx == 17);
    for (size_t i = 0; i < sizeof upper / sizeof *upper; ++i) {
        assert(request(peer, SEXOS_SYSCALL_reply, upper[i] | caller->id, 71, 0).rax != 0);
        assert(peer->reply_rights[caller->id] == 1 && !caller->reply_ready);
    }
    assert(request(peer, SEXOS_SYSCALL_reply, caller->id, 71, 0).rax == 0);
    for (size_t i = 0; i < sizeof upper / sizeof *upper; ++i) {
        assert(request(caller, SEXOS_SYSCALL_listen, upper[i], 0, 0).rax == 0);
        assert(request(caller, SEXOS_RPC_listen, upper[i], 0, 0).rax == 0);
        assert(caller->reply_ready && caller->reply_value == 71);
    }
    assert(request(caller, SEXOS_SYSCALL_listen, 0, 0, 0).rdx == 71);
    assert(!caller->reply_ready);

    assert(request(peer, SEXOS_RPC_register, 0, 0, 0).rax == 1);
    TrapFrame call = request(caller, SEXOS_RPC_call, 5, CALL_TYPE, 23);
    assert(call.rax == 0 && call.rsi != 0);
    assert(request(peer, SEXOS_RPC_listen, 0, 0, 0).r9 == call.rsi);
    for (size_t i = 0; i < sizeof upper / sizeof *upper; ++i) {
        assert(request(peer, SEXOS_RPC_reply, upper[i] | caller->id, call.rsi, 81).rax == ERR_CAP_INVALID);
        assert(rpc_find(caller, call.rsi)->state == RPC_HEARD);
    }
    assert(request(peer, SEXOS_RPC_reply, caller->id, call.rsi, 81).rax == 0);
    TrapFrame result = request(caller, SEXOS_RPC_poll, call.rsi, 0, 0);
    assert(result.rax == 0 && result.rsi == 81);
}

int main(void)
{
    initialize();
    Domain *caller = domain_of_key(3);
    const Cap *cap = cap_get(caller, 5);
    assert(cap && cap->kind == CAP_DOMAIN);
    Domain *peer = domain_get(cap->target);
    Domain *other = domain_of_key(2);
    assert(other != peer && caller != peer);
    assert(kernel.genesis_sealed);
    Cap original_cap = *cap;
    assert(!cap_set(caller, 5, domain_cap(other->id)));
    assert(!cap_set(caller, KERNEL_CAP_SLOTS - 1, domain_cap(other->id)));
    assert(cap_get(caller, 5)->target == original_cap.target);
    assert(cap_get(caller, KERNEL_CAP_SLOTS - 1) == NULL);
    assert(request(caller, SEXOS_RPC_call, 5, CALL_TYPE, 7).rax == NO_ADDRESS);
    assert(caller->rpc_next == 0 && peer->ring.count == 0);
    assert(request(peer, SEXOS_RPC_register, 0, 0, 0).rax == 1);
    assert(request(caller, SEXOS_RPC_call, KERNEL_CAP_SLOTS, CALL_TYPE, 7).rax == ERR_CAP_INVALID);

    uint64_t ids[KERNEL_RPC_CAPACITY];
    for (size_t i = 0; i < KERNEL_RPC_CAPACITY; ++i) {
        TrapFrame f = request(caller, SEXOS_RPC_call, 5, CALL_TYPE, 100 + i);
        assert(f.rax == 0 && f.rsi != 0);
        ids[i] = f.rsi;
        assert(request(caller, SEXOS_RPC_poll, ids[i], 0, 0).rax == ERR_SERVICE_NOT_READY);
        assert(request(peer, SEXOS_RPC_reply, caller->id, ids[i], 900 + i).rax == ERR_CAP_INVALID);
    }
    assert(request(caller, SEXOS_RPC_call, 5, CALL_TYPE, 1).rax == ERR_SERVICE_NOT_READY);
    assert(caller->rpc_next == KERNEL_RPC_CAPACITY && peer->ring.count == KERNEL_RPC_CAPACITY);

    for (size_t i = 0; i < KERNEL_RPC_CAPACITY; ++i) {
        TrapFrame f = request(peer, SEXOS_RPC_listen, 0, 0, 0);
        assert(f.rax == CALL_TYPE && f.rsi == caller->id && f.rdx == 100 + i && f.r9 == ids[i]);
        assert(request(other, SEXOS_RPC_reply, caller->id, ids[i], 999).rax == ERR_CAP_INVALID);
    }
    /* Completion may arrive out of order without changing the legacy reply slot. */
    caller->reply_ready = true;
    caller->reply_value = 12345;
    assert(domain_enqueue(caller,(RingEntry){.type_id=0x799,.arg0=321,.sender=other->id}));
    for (size_t i = KERNEL_RPC_CAPACITY; i-- > 0;) {
        assert(request(peer, SEXOS_RPC_reply, caller->id, ids[i], 900 + i).rax == 0);
        assert(request(peer, SEXOS_RPC_reply, caller->id, ids[i], 10).rax == ERR_CAP_INVALID);
        assert(caller->reply_ready && caller->reply_value == 12345);
    }
    for (size_t i = 0; i < KERNEL_RPC_CAPACITY; ++i) {
        TrapFrame f = request(caller, SEXOS_RPC_poll, ids[i], 0, 0);
        assert(f.rax == 0 && f.rsi == 900 + i);
        assert(request(caller, SEXOS_RPC_poll, ids[i], 0, 0).rax == ERR_CAP_INVALID);
        assert(caller->reply_ready && caller->reply_value==12345 && caller->ring.count==1);
    }

    TrapFrame full={.rax=SEXOS_RPC_call,.rdi=5,.rsi=CALL_TYPE,
                    .rdx=UINT64_C(0xFEDCBA9876543210),.r10=UINT64_C(0xC001D00DFACE1234),.r8=UINT64_MAX};
    kernel.current=caller->id;kernel_syscall(&full);assert(full.rax==0);
    uint64_t full_id=full.rsi;
    TrapFrame received=request(peer,SEXOS_RPC_listen,0,0,0);
    assert(received.r9==full_id && received.rdx==UINT64_C(0xFEDCBA9876543210) &&
           received.r10==UINT64_C(0xC001D00DFACE1234) && received.r8==UINT64_MAX);
    assert(request(peer,SEXOS_RPC_reply,caller->id,full_id,UINT64_MAX).rax==0);
    received=request(caller,SEXOS_RPC_poll,full_id,0,0);
    assert(received.rax==0 && received.rsi==UINT64_MAX);

    /* Cancelled queued calls are not delivered; fresh identities do not alias them. */
    TrapFrame f = request(caller, SEXOS_RPC_call, 5, CALL_TYPE, 20);
    uint64_t cancelled = f.rsi;
    assert(f.rax == 0);
    assert(request(caller, SEXOS_RPC_cancel, cancelled, 0, 0).rax == 0);
    assert(request(peer, SEXOS_RPC_listen, 0, 0, 0).rax == 0);
    assert(request(peer, SEXOS_RPC_reply, caller->id, cancelled, 40).rax == ERR_CAP_INVALID);
    f = request(caller, SEXOS_RPC_call, 5, CALL_TYPE, 21);
    assert(f.rax == 0 && f.rsi > cancelled);
    uint64_t active = f.rsi;
    assert(request(peer, SEXOS_RPC_listen, 0, 0, 0).r9 == active);
    assert(request(caller, SEXOS_RPC_cancel, active, 0, 0).rax == 0);
    assert(request(peer, SEXOS_RPC_reply, caller->id, active, 42).rax == ERR_CAP_INVALID);

    /* Exhausting identities refuses admission without damaging existing state. */
    caller->rpc_next = UINT64_MAX;
    uint32_t count = peer->ring.count;
    assert(request(caller, SEXOS_RPC_call, 5, CALL_TYPE, 1).rax == ERR_SERVICE_NOT_READY);
    assert(peer->ring.count == count);
    assert(request(caller, SEXOS_RPC_cancel, active, 0, 0).rax == ERR_CAP_INVALID);
    assert(caller->reply_ready && caller->reply_value==12345 && caller->ring.count==1);
    cursor_certificates();
    reply_certificates(caller, peer);
    full_width_selectors();
    puts("kernel RPC: bounded admission, authenticated completion, retention, cancellation and exhaustion passed");
    return 0;
}
