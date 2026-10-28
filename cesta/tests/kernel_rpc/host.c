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

int main(void)
{
    initialize();
    Domain *caller = domain_of_key(3);
    const Cap *cap = cap_get(caller, 5);
    assert(cap && cap->kind == CAP_DOMAIN);
    Domain *peer = domain_get(cap->target);
    Domain *other = domain_of_key(2);
    assert(other != peer && caller != peer);
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
    puts("kernel RPC: bounded admission, authenticated completion, retention, cancellation and exhaustion passed");
    return 0;
}
