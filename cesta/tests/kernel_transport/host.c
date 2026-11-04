/* Current typed PDX transport, using the actual exported kernel units.
 * The inherited providers stand in for memory, devices and CPU instructions. */
#define main historical_trace_main
#include "../kernel_model/host.c"
#undef main

void kernel_region_writer_syscall(Domain *d,TrapFrame *f){(void)d;f->rax=UINT64_MAX;}
bool kernel_region_writer_recover_dead(Domain *d){(void)d;return true;}
void kernel_device_reset_syscall(Domain *d,TrapFrame *f){(void)d;f->rax=DEVICE_RESET_INVALID;}
bool kernel_device_reset_owner_dead(Domain *d){(void)d;return true;}

static void reset_transport(void)
{
    memset(&kernel, 0, sizeof kernel);
    for (unsigned i = 1; i <= 3; ++i) {
        Domain *d = domain_boot_record(i);
        assert(d);
        d->live = true;
        d->id = i;
        d->key = (uint8_t)i;
        d->task_id = i * 101;
        d->task_state = TASK_READY;
        assert(cap_set(d, 2, (Cap){.kind = CAP_MESSAGE_QUEUE}));
        for (unsigned t = 1; t <= 3; ++t)
            assert(cap_set(d, 5 + t, (Cap){.kind = CAP_DOMAIN, .target = t}));
    }
    kernel_seal_genesis();
}

static TrapFrame invoke(unsigned actor, uint64_t number, uint64_t a,
                        uint64_t b, uint64_t c)
{
    assert(actor >= 1 && actor <= 3);
    for (unsigned i = 1; i <= 3; ++i) domain_get(i)->task_state = TASK_READY;
    kernel.current = actor;
    domain_get(actor)->task_state = TASK_RUNNING;
    TrapFrame f = {.rax = number, .rdi = a, .rsi = b, .rdx = c,
                   .r10 = c ^ UINT64_C(0xabcdef), .r8 = ~c};
    assert(kernel_syscall(&f) == &f);
    return f;
}

static void ring_epoch_controls(void)
{
    memset(&kernel, 0, sizeof kernel);
    Ring ring = {0};
    RingEntry entry = {.type_id = UINT64_C(0xabc), .caller = 7, .caller_task = 70,
                       .arg0 = 11, .arg1 = 12, .arg2 = 13};
    assert(!ring_push(&ring, 9, entry));
    kernel.genesis_sealed = true;
    KernelRingPushContext aborted = {.ring = &ring, .subject = 9, .entry = entry};
    assert(kernel_ring_push_snapshot(&aborted));
    assert(kernel_ring_push_transform(&aborted));
    assert(kernel_ring_push_swap(&aborted));
    /* The inactive tail changed, but no message or revision is published. */
    assert(ring.count == 0 && ring.head == 0 && ring.revision == 0);
    RingEntry absent = {0};
    assert(!ring_pop(&ring, 9, &absent));

    KernelRingPushContext stale = {.ring = &ring, .subject = 9, .entry = entry};
    assert(kernel_ring_push_snapshot(&stale));
    assert(kernel_ring_push_transform(&stale));
    ++ring.revision;
    assert(!kernel_ring_push_swap(&stale));
    assert(ring.count == 0 && ring.revision == 1);

    ring.revision = 0;
    KernelRingPushContext committed = {.ring = &ring, .subject = 9, .entry = entry};
    assert(kernel_ring_push_snapshot(&committed));
    assert(kernel_ring_push_transform(&committed));
    assert(kernel_ring_push_swap(&committed));
    assert(kernel_ring_push_commit(&committed));
    assert(ring.count == 1 && ring.revision == 1);
    assert(kernel.epoch == 1 && kernel.epoch_event_count == 1);
    KernelEpochEvent first = kernel.epoch_events[0];
    assert(first.epoch == 1 && first.kind == KERNEL_EPOCH_RING_PUSH &&
           first.subject == 9 && first.revision == 1 &&
           first.message.type_id == entry.type_id && first.message.arg2 == entry.arg2);

    RingEntry privately_copied = {0};
    KernelRingContext pop_aborted = {.ring = &ring, .subject = 9, .out = &privately_copied};
    assert(kernel_ring_inspect(&pop_aborted));
    assert(kernel_ring_copy(&pop_aborted));
    assert(kernel_ring_swap(&pop_aborted));
    assert(privately_copied.type_id == entry.type_id);
    assert(ring.head == 0 && ring.count == 1 && ring.revision == 1);

    KernelRingContext pop_stale = {.ring = &ring, .subject = 9, .out = &privately_copied};
    assert(kernel_ring_inspect(&pop_stale));
    assert(kernel_ring_copy(&pop_stale));
    ++ring.revision;
    assert(!kernel_ring_swap(&pop_stale));
    assert(ring.head == 0 && ring.count == 1 && ring.revision == 2);
    ring.revision = 1;

    RingEntry observed = {0};
    assert(ring_pop(&ring, 9, &observed));
    assert(observed.type_id == entry.type_id && observed.caller == entry.caller &&
           observed.caller_task == entry.caller_task && observed.arg0 == entry.arg0 &&
           observed.arg1 == entry.arg1 && observed.arg2 == entry.arg2);
    assert(ring.head == 1 && ring.count == 0 && ring.revision == 2);
    assert(kernel.epoch == 2 && kernel.epoch_event_count == 2);
    KernelEpochEvent second = kernel.epoch_events[1];
    assert(second.epoch == 2 && second.kind == KERNEL_EPOCH_RING_POP &&
           second.subject == 9 && second.revision == 2 &&
           second.message.type_id == entry.type_id && second.message.arg2 == entry.arg2);
    kernel.epoch_event_count = KERNEL_EPOCH_EVENT_CAPACITY;
    assert(!ring_push(&ring, 9, entry) && ring.count == 0 && ring.revision == 2);
}

static void input_epoch_controls(void)
{
    memset(&kernel, 0, sizeof kernel);
    kernel.input_pd = 4;
    assert(!input_push(0x1d));
    kernel.genesis_sealed = true;
    KernelInputPushContext aborted = {.state = &kernel, .scancode = 0x1e};
    assert(kernel_input_push_snapshot(&aborted));
    assert(kernel_input_push_transform(&aborted));
    assert(kernel_input_push_swap(&aborted));
    assert(kernel.input_count == 0 && kernel.input_head == 0 && kernel.input_revision == 0);
    uint8_t absent = 0;
    assert(!input_pop(&absent));

    KernelInputPushContext stale = {.state = &kernel, .scancode = 0x1f};
    assert(kernel_input_push_snapshot(&stale));
    assert(kernel_input_push_transform(&stale));
    ++kernel.input_revision;
    assert(!kernel_input_push_swap(&stale));
    assert(kernel.input_count == 0 && kernel.input_revision == 1);

    kernel.input_revision = 0;
    assert(input_push(0x20));
    assert(kernel.input_count == 1 && kernel.input_revision == 1);
    assert(kernel.epoch == 1 && kernel.epoch_event_count == 1);
    assert(kernel.epoch_events[0].kind == KERNEL_EPOCH_INPUT_PUSH &&
           kernel.epoch_events[0].subject == 4 &&
           kernel.epoch_events[0].revision == 1 &&
           kernel.epoch_events[0].scancode == 0x20);

    uint8_t private_out = 0;
    KernelInputPopContext pop_aborted = {.state = &kernel, .out = &private_out};
    assert(kernel_input_pop_snapshot(&pop_aborted));
    assert(kernel_input_pop_transform(&pop_aborted));
    assert(kernel_input_pop_swap(&pop_aborted));
    assert(private_out == 0x20 && kernel.input_head == 0 &&
           kernel.input_count == 1 && kernel.input_revision == 1);

    KernelInputPopContext pop_stale = {.state = &kernel, .out = &private_out};
    assert(kernel_input_pop_snapshot(&pop_stale));
    assert(kernel_input_pop_transform(&pop_stale));
    ++kernel.input_revision;
    assert(!kernel_input_pop_swap(&pop_stale));
    assert(kernel.input_head == 0 && kernel.input_count == 1 && kernel.input_revision == 2);

    kernel.input_revision = 1;
    uint8_t observed = 0;
    assert(input_pop(&observed) && observed == 0x20);
    assert(kernel.input_head == 1 && kernel.input_count == 0 && kernel.input_revision == 2);
    assert(kernel.epoch == 2 && kernel.epoch_event_count == 2);
    assert(kernel.epoch_events[1].kind == KERNEL_EPOCH_INPUT_POP &&
           kernel.epoch_events[1].subject == 4 &&
           kernel.epoch_events[1].revision == 2 &&
           kernel.epoch_events[1].scancode == 0x20);

    kernel.input_count = KERNEL_INPUT_CAPACITY;
    assert(!input_push(0x21) && kernel.input_count == KERNEL_INPUT_CAPACITY);
    kernel.input_count = 0;
    kernel.input_revision = UINT64_MAX;
    assert(!input_push(0x22) && !input_pop(&observed));
}

static void controls(void)
{
    ring_epoch_controls();
    input_epoch_controls();
    reset_transport();
    Domain *a = domain_get(1), *b = domain_get(2);
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 123).rax == 0);
    assert(b->reply_rights[1] == 1); /* Before anybody listens. */
    assert(invoke(2, SEXOS_SYSCALL_reply, 1, 701, 0).rax == 0);
    assert(b->reply_rights[1] == 0 && a->reply_value == 701);
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 124).rax == 0);
    assert(invoke(2, SEXOS_SYSCALL_reply, 1, 702, 0).rax == 1);
    assert(b->reply_rights[1] == 1 && a->reply_value == 701);
    TrapFrame reply = invoke(1, SEXOS_SYSCALL_listen, 0, 0, 0);
    assert(reply.rax == 1 && reply.rsi == 2 && reply.rdi == 202 && reply.rdx == 701);
    assert(!a->reply_ready && a->reply_value == 0 && a->reply_caller == 0 && a->reply_task == 0);
    assert(invoke(2, SEXOS_SYSCALL_reply, 1, 702, 0).rax == 0);
    assert(invoke(2, SEXOS_SYSCALL_reply, 1, 703, 0).rax == 1);
    assert(invoke(3, SEXOS_SYSCALL_reply, 1, 704, 0).rax == 1);
    TrapFrame call = invoke(2, SEXOS_SYSCALL_listen, 2, 0, 0);
    assert(call.rax == CALL_TYPE && call.rdi == 101 && call.rsi == 1 && call.rdx == 123);
    assert(call.r10 == (123 ^ UINT64_C(0xabcdef)) && call.r8 == ~UINT64_C(123));
    assert(b->reply_rights[1] == 0); /* Hearing cannot mint a second obligation. */
    assert(invoke(2, SEXOS_SYSCALL_listen, 0, 0, 0).rdx == 124);
    TrapFrame empty = invoke(2, SEXOS_SYSCALL_listen, 0, 0, 0);
    assert((empty.rax | empty.rdi | empty.rsi | empty.rdx | empty.r10 | empty.r8) == 0);

    reset_transport();
    for (unsigned i = 0; i < KERNEL_RING_CAPACITY; ++i)
        assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, i).rax == 0);
    uint64_t rev = b->ring.revision;
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 999).rax == ERR_SERVICE_NOT_READY);
    assert(b->reply_rights[1] == KERNEL_RING_CAPACITY && b->ring.count == KERNEL_RING_CAPACITY);
    assert(b->ring.revision == rev);
    b->reply_rights[1] = UINT64_MAX;
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 999).rax == ERR_SERVICE_NOT_READY);
    assert(b->reply_rights[1] == UINT64_MAX && b->ring.revision == rev);
    assert(invoke(2, SEXOS_SYSCALL_listen, 0, 0, 0).rax == CALL_TYPE);
    assert(b->reply_rights[1] == UINT64_MAX);
    /* Every accepted call retains one right: a full counter refuses before enqueue. */
    rev=b->ring.revision;
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 888).rax == ERR_SERVICE_NOT_READY);
    assert(b->reply_rights[1] == UINT64_MAX && b->ring.count == KERNEL_RING_CAPACITY-1 && b->ring.revision==rev);
    assert(invoke(2, SEXOS_SYSCALL_reply, 1, 71, 0).rax == 0);
    assert(b->reply_rights[1] == UINT64_MAX-1);
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 889).rax == 0);
    assert(b->reply_rights[1] == UINT64_MAX && b->ring.count == KERNEL_RING_CAPACITY);

    reset_transport();
    b->ring.revision = UINT64_MAX;
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 1).rax == ERR_SERVICE_NOT_READY);
    assert(b->reply_rights[1] == 0 && b->ring.count == 0);
    b->reply_rights[1]=UINT64_MAX;
    assert(invoke(1, SEXOS_SYSCALL_call, 7, CALL_TYPE, 2).rax == ERR_SERVICE_NOT_READY);
    assert(b->reply_rights[1] == UINT64_MAX && b->ring.count == 0);
    const uint64_t high[] = {UINT64_C(1) << 32, UINT64_C(1) << 63};
    for (unsigned i = 0; i < 2; ++i) {
        assert(invoke(1, SEXOS_SYSCALL_call, high[i] | 7, CALL_TYPE, 1).rax == ERR_CAP_INVALID);
        assert(invoke(2, SEXOS_SYSCALL_reply, high[i] | 1, 5, 0).rax == 1);
        TrapFrame absent = invoke(2, SEXOS_SYSCALL_listen, high[i] | 2, 0, 0);
        assert((absent.rax | absent.rdi | absent.rsi | absent.rdx | absent.r10 | absent.r8) == 0);
    }

    reset_transport();
    kernel.current = 1;
    a->task_state = TASK_RUNNING;
    assert(kernel_current_task(a) == 101 && kernel_current_task(b) == 0);
    Domain copy = *a;
    assert(kernel_current_task(&copy) == 0 && kernel_current_task(NULL) == 0);
    a->id = 2;
    assert(domain_get(1) == NULL && domain_get(0) == NULL);
    assert(domain_get(KERNEL_MAX_DOMAINS) == NULL && domain_get(UINT32_MAX) == NULL);
    assert(kernel_current_task(a) == 0);
    a->id = 1;
    a->task_state = TASK_EXITED;
    assert(kernel_current_task(a) == 0);
    a->task_state = TASK_READY;
    assert(kernel_current_task(a) == 0);
    a->task_state = TASK_RUNNING;
    a->task_id = 0;
    TrapFrame bad = {.rdi = 7, .rsi = CALL_TYPE};
    pdx_call(a, &bad);
    assert(bad.rax == ERR_CAP_INVALID && b->ring.count == 0 && b->reply_rights[1] == 0);
    b->reply_rights[1] = 1;
    kernel.current = 2;
    b->task_id = 0;
    b->task_state = TASK_RUNNING;
    bad = (TrapFrame){.rdi = 1, .rsi = 9};
    pdx_reply(b, &bad);
    assert(bad.rax == 1 && b->reply_rights[1] == 1 && !a->reply_ready);

    reset_transport();
    kernel.current = 1;
    a->task_state = TASK_RUNNING;
    b->task_state = TASK_EXITED;
    bad = (TrapFrame){.rdi = 7, .rsi = CALL_TYPE};
    pdx_call(a, &bad);
    assert(bad.rax == ERR_SERVICE_NOT_READY && b->reply_rights[1] == 0 && b->ring.count == 0);
    a->reply_rights[2] = 1;
    bad = (TrapFrame){.rdi = 2, .rsi = 99};
    pdx_reply(a, &bad);
    assert(bad.rax == 1 && a->reply_rights[2] == 1 && !b->reply_ready);

    reset_transport();
    kernel.current=1;a->task_state=TASK_RUNNING;b->task_state=TASK_DORMANT;
    bad=(TrapFrame){.rdi=7,.rsi=CALL_TYPE};pdx_call(a,&bad);
    assert(bad.rax==ERR_SERVICE_NOT_READY && b->ring.count==0 && b->reply_rights[1]==0);
    a->reply_rights[2]=1;bad=(TrapFrame){.rdi=2,.rsi=9};pdx_reply(a,&bad);
    assert(bad.rax==1 && a->reply_rights[2]==1 && !b->reply_ready);

    Domain detached = {0}, last = {0}, exhausted = {0};
    next_task_id = 711;
    assert(task_attach(&detached) && detached.task_id == 711);
    assert(!task_attach(&detached) && next_task_id == 712);
    next_task_id = UINT32_MAX;
    assert(task_attach(&last) && last.task_id == UINT32_MAX && next_task_id == 0);
    assert(!task_attach(&exhausted) && exhausted.task_id == 0);

    reset_transport();
    assert(invoke(2, SEXOS_RPC_register, 0, 0, 0).rax == 1);
    TrapFrame rpc = invoke(1, SEXOS_RPC_call, 7, CALL_TYPE, 19);
    assert(rpc.rax == 0 && rpc.rsi != 0);
    TrapFrame heard = invoke(2, SEXOS_RPC_listen, 0, 0, 0);
    assert(heard.rdi == 101 && heard.rsi == 1 && heard.r9 == rpc.rsi && heard.rdx == 19);
    assert(b->reply_rights[1] == 0); /* Correlated calls have their own authority. */

    reset_transport();
    assert(domain_enqueue(b, (RingEntry){.type_id = 0x201, .caller = 1, .arg0 = 72}));
    TrapFrame kernel_event = invoke(2, SEXOS_SYSCALL_listen, 0, 0, 0);
    assert(kernel_event.rax == 0x201 && kernel_event.rdi == 0);
    assert(b->reply_rights[1] == 0);

    reset_transport();
    b=domain_get(2);kernel.vector_owner[0x22]=2;kernel.vector_revision[0x22]=1;
    assert(domain_enqueue(b,(RingEntry){.type_id=0xFF,.caller=1,.arg0=0x22,
        .arg1=1,.arg2=4,.sender=0,.from_call=false}));
    TrapFrame interrupt=invoke(2,SEXOS_SYSCALL_listen,0,0,0);
    assert(interrupt.rax==0xFF&&interrupt.rsi==1&&interrupt.rdx==0x22&&
        interrupt.r10==1&&interrupt.r8==4);
    assert(kernel.vector_heard_revision[0x22]==1&&kernel.vector_ack_revision[0x22]==0);
    assert(domain_enqueue(b,(RingEntry){.type_id=0xFF,.caller=3,.arg0=0x22,
        .arg1=2,.arg2=4,.sender=3,.from_call=false}));
    interrupt=invoke(2,SEXOS_SYSCALL_listen,0,0,0);
    assert(interrupt.rax==0xFF&&kernel.vector_heard_revision[0x22]==1);
}

static void emit(TrapFrame f, bool listened)
{
    printf("[%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64,
           f.rax, listened ? f.rsi : 0, listened ? f.rdi : 0,
           listened ? f.rdx : 0, listened ? f.r10 : 0, listened ? f.r8 : 0);
    for (unsigned i = 1; i <= 3; ++i) {
        const Domain *d = domain_get(i);
        assert(d);
        printf(",%u,%" PRIu64 ",%u,%u,%u", d->reply_ready, d->reply_value,
               d->reply_caller, d->reply_task, d->ring.count);
        for (unsigned j = 1; j <= 3; ++j) printf(",%" PRIu64, d->reply_rights[j]);
    }
    puts("]");
}

int main(void)
{
    controls();
    reset_transport();
    char op;
    unsigned actor;
    uint64_t target, value;
    while (scanf(" %c %u %" SCNu64 " %" SCNu64, &op, &actor, &target, &value) == 4) {
        TrapFrame f;
        if (op == 'C') f = invoke(actor, SEXOS_SYSCALL_call, target, CALL_TYPE, value);
        else if (op == 'R') f = invoke(actor, SEXOS_SYSCALL_reply, target, value, 0);
        else { assert(op == 'L'); f = invoke(actor, SEXOS_SYSCALL_listen, target, 0, 0); }
        emit(f, op == 'L');
    }
    return 0;
}
