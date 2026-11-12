/* Current typed PDX transport, using the actual exported kernel units.
 * The inherited providers stand in for memory, devices and CPU instructions. */
#define main historical_trace_main
#include "epoch_journal.c"
#include "kernel_epoch_log.c"
#include "program_memory.c"
#include "../kernel_model/host.c"
#undef main

void host_cpu_wrmsr(uint32_t msr, uint64_t value) { (void)msr; (void)value; }
void host_cpu_wrpkru(uint32_t mask) { (void)mask; }

uint64_t program_frame_pressure(void) { return 0; }
FrameLeaseResult program_frame_take(uint32_t holder)
{
    (void)holder;
    return (FrameLeaseResult){.status = FRAME_LEASE_FULL};
}
bool program_frame_map_run(FrameLease lease, uint32_t run, uint64_t va, uint64_t flags, uint8_t key)
{
    (void)lease; (void)run; (void)va; (void)flags; (void)key;
    return false;
}
bool program_frame_release(FrameLease lease) { (void)lease; return false; }

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
    kernel.epoch = kernel.epoch_event_count = UINT64_MAX;
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

static void epoch_window_controls(void)
{
    memset(&kernel, 0, sizeof kernel);
    kernel.genesis_sealed = true;
    for (uint64_t expected = 0; expected < KERNEL_EPOCH_EVENT_CAPACITY + 64; ++expected)
        kernel_epoch_record(expected, KERNEL_EPOCH_RING_PUSH, 9, expected + 1u, NULL, 0);
    assert(kernel.epoch == KERNEL_EPOCH_EVENT_CAPACITY + 64);
    assert(kernel.epoch_event_count == kernel.epoch);
    assert(kernel.epoch_compacted_through == 64);
    assert(kernel.epoch_checkpoint.through == 64 &&
           kernel.epoch_checkpoint.boundary.epoch == 64 &&
           kernel.epoch_checkpoint.boundary.revision == 64);
    assert(kernel_epoch_current(kernel.epoch));
    KernelEpochEvent event = {.epoch = UINT64_MAX};
    assert(!kernel_epoch_event_read(0, &event) && event.epoch == UINT64_MAX);
    assert(!kernel_epoch_event_read(64, &event) && event.epoch == UINT64_MAX);
    assert(!kernel_epoch_event_read(KERNEL_EPOCH_EVENT_CAPACITY + 65, &event) &&
           event.epoch == UINT64_MAX);
    assert(!kernel_epoch_event_read(KERNEL_EPOCH_EVENT_CAPACITY + 64, NULL));
    assert(kernel_epoch_event_read(65, &event) && event.epoch == 65 && event.revision == 65 &&
           event.subject == 9 && event.kind == KERNEL_EPOCH_RING_PUSH);
    assert(kernel_epoch_event_read(KERNEL_EPOCH_EVENT_CAPACITY + 64, &event) &&
           event.epoch == KERNEL_EPOCH_EVENT_CAPACITY + 64 &&
           event.revision == KERNEL_EPOCH_EVENT_CAPACITY + 64);
}

static EpochLogBatch durable_batch;

static void durable_epoch_controls(void)
{
    memset(&kernel, 0, sizeof kernel);
    memset(&durable_batch, 0, sizeof durable_batch);
    kernel.scheduler_running = true;
    durable_batch.producer = 1;
    kernel_epoch_batch = &durable_batch;
    kernel_epoch_flush_active = false;
    kernel_epoch_flush_task = 0;
    for (uint32_t id = 1; id <= 3; ++id) {
        Domain *d = domain_boot_record(id);
        assert(d);
        d->live = true;
        d->id = id;
        d->key = (uint8_t)id;
        d->task_id = id * 101;
        d->program.run = d->task_id;
        d->task_state = TASK_READY;
    }
    Domain *files = domain_boot_record(CURRENT_PD_sexfiles);
    assert(files);
    files->live = true;
    files->id = CURRENT_PD_sexfiles;
    files->key = CURRENT_PD_sexfiles;
    files->task_id = 1101;
    files->program.run = files->task_id;
    files->task_state = TASK_READY;
    kernel.genesis_sealed = true;

    for (uint64_t expected = 0; expected < EPOCH_LOG_BATCH_EVENTS; ++expected)
        kernel_epoch_record(expected, KERNEL_EPOCH_RING_PUSH, 9, expected + 1, NULL, 0);
    assert(kernel.epoch == EPOCH_LOG_BATCH_EVENTS && durable_batch.producer == EPOCH_LOG_BATCH_EVENTS + 1);
    for (uint64_t epoch = 1; epoch <= EPOCH_LOG_BATCH_EVENTS; ++epoch)
        assert(epoch_journal_event_sector_valid(durable_batch.sectors[epoch - 1], epoch));
    files->task_state = TASK_WAIT_IO;
    assert(!kernel_epoch_current(kernel.epoch));
    assert(files->wake_pending && files->task_state == TASK_READY);
    assert(!files->reply_ready && !files->reply_value && !files->reply_caller && !files->reply_task);
    assert(kernel.epoch == EPOCH_LOG_BATCH_EVENTS && durable_batch.producer == EPOCH_LOG_BATCH_EVENTS + 1);

    Domain *drive = domain_get(CURRENT_PD_sexdrive);
    drive->reply_rights[files->id] = 1;
    files->task_state = TASK_WAIT_IO;
    files->wake_pending = false;
    assert(!kernel_epoch_current(kernel.epoch));
    assert(!files->reply_ready && !files->reply_value && files->wake_pending &&
           files->task_state == TASK_READY && drive->reply_rights[files->id] == 1);
    drive->reply_rights[files->id] = 0;

    drive->ring.entries[0] = (RingEntry){.type_id = 0x123, .caller = files->id};
    drive->ring.head = 0;
    drive->ring.count = 1;
    drive->ring.revision = 7;
    kernel.current = CURRENT_PD_sexdrive;
    RingEntry drained;
    assert(domain_dequeue(drive, &drained));
    assert(drained.type_id == 0x123 && !drive->ring.count &&
           kernel.epoch == EPOCH_LOG_BATCH_EVENTS &&
           durable_batch.producer == EPOCH_LOG_BATCH_EVENTS + 1);
    Domain *other = domain_get(1);
    other->ring.entries[0] = (RingEntry){.type_id = 0x124, .caller = 3};
    other->ring.head = 0;
    other->ring.count = 1;
    other->ring.revision = 9;
    kernel.current = 1;
    RingEntry refused;
    assert(!domain_dequeue(other, &refused) && other->ring.count == 1 &&
           kernel.epoch == EPOCH_LOG_BATCH_EVENTS &&
           durable_batch.producer == EPOCH_LOG_BATCH_EVENTS + 1);
    other->ring.count = 0;
    /* A physical completion during a flush can arrive while another domain is
     * current. Publication is deferred, but the owner must make progress on
     * LISTEN without requiring a second interrupt or a scheduler-poll accident. */
    const uint32_t vector = 0x22;
    kernel.vector_owner[vector] = drive->id;
    kernel.current = other->id;
    other->task_state = TASK_RUNNING;
    uint64_t epoch_before = kernel.epoch, producer_before = durable_batch.producer;
    kernel_device_interrupt_publish(vector);
    assert(kernel.vector_pending[vector] == 1 && kernel.vector_revision[vector] == 1);
    assert(!drive->ring.count && !kernel.vector_heard_revision[vector] &&
           !kernel.vector_ack_revision[vector]);
    kernel_device_interrupt_drain(drive);
    assert(kernel.vector_pending[vector] == 1 && !drive->ring.count);
    kernel.current = drive->id;
    drive->task_state = TASK_RUNNING;
    ProgramWorker stale = {.id = PROGRAM_WORKER_TASK_FIRST, .target = drive->id,
        .run = drive->task_id + 1, .phase = PROGRAM_WORKER_ATTACHED, .owner_core = 0};
    kernel_current_worker = &stale;
    kernel_device_interrupt_drain(drive);
    assert(kernel.vector_pending[vector] == 1 && !drive->ring.count);
    kernel_current_worker = NULL;
    RingEntry irq = kernel_listen_message(drive, 0);
    assert(irq.type_id == 0xff && irq.caller == 1 && !irq.caller_task && !irq.from_call &&
           irq.arg0 == vector && irq.arg1 == 1 && irq.arg2 == 1);
    assert(!kernel.vector_pending[vector] && kernel.vector_heard_revision[vector] == 1 &&
           !kernel.vector_ack_revision[vector]);
    assert(!kernel_listen_message(drive, 0).type_id);
    assert(kernel.vector_revision[vector] == 1 && kernel.epoch == epoch_before &&
           durable_batch.producer == producer_before);

    /* Freeing one full-ring slot must publish the retained receipt before the
     * listener returns, keeping it from being starved by ordinary new messages. */
    for (size_t i = 0; i < KERNEL_RING_CAPACITY; ++i)
        drive->ring.entries[i] = (RingEntry){.type_id = 0x125, .arg0 = i};
    drive->ring.head = 0;
    drive->ring.count = KERNEL_RING_CAPACITY;
    kernel.current = other->id;
    kernel_device_interrupt_publish(vector);
    assert(kernel.vector_pending[vector] == 2 && kernel.vector_revision[vector] == 2);
    kernel.current = drive->id;
    assert(kernel_listen_message(drive, 0).type_id == 0x125);
    assert(!kernel.vector_pending[vector] && drive->ring.count == KERNEL_RING_CAPACITY);
    assert(kernel.vector_heard_revision[vector] == 1 && !kernel.vector_ack_revision[vector]);
    for (size_t i = 1; i < KERNEL_RING_CAPACITY; ++i)
        assert(kernel_listen_message(drive, 0).type_id == 0x125);
    irq = kernel_listen_message(drive, 0);
    assert(irq.type_id == 0xff && irq.arg1 == 2 && kernel.vector_heard_revision[vector] == 2);
    assert(!kernel.vector_ack_revision[vector] && !drive->ring.count &&
           kernel.epoch == epoch_before && durable_batch.producer == producer_before);
    kernel.current = 0;

    kernel.run_queue.head = kernel.run_queue.count = 0;
    for (uint32_t id = 1; id <= 3; ++id) {
        domain_get(id)->queued = false;
        domain_get(id)->task_state = TASK_READY;
        run_push(id);
    }
    files->queued = false;
    files->task_state = TASK_READY;
    run_push(files->id);
    assert(run_pop() == CURRENT_PD_sexdrive);
    assert(run_pop() == CURRENT_PD_sexfiles);
    assert(run_pop() == 0 && domain_get(1)->queued && domain_get(3)->queued && kernel.run_queue.count == 2);
    durable_batch.flushed_through = EPOCH_LOG_BATCH_EVENTS;
    kernel_epoch_record(EPOCH_LOG_BATCH_EVENTS, KERNEL_EPOCH_RING_POP, 9, EPOCH_LOG_BATCH_EVENTS + 1, NULL, 0);
    assert(kernel.epoch == EPOCH_LOG_BATCH_EVENTS + 1 && durable_batch.producer == EPOCH_LOG_BATCH_EVENTS + 2);

    for (uint64_t expected = EPOCH_LOG_BATCH_EVENTS + 1; expected < KERNEL_EPOCH_EVENT_CAPACITY + 64; ++expected) {
        durable_batch.flushed_through = expected;
        kernel_epoch_record(expected, KERNEL_EPOCH_RING_PUSH, 9, expected + 1, NULL, 0);
    }
    assert(kernel.epoch == KERNEL_EPOCH_EVENT_CAPACITY + 64 && kernel.epoch_compacted_through == 64);
    assert(epoch_journal_event_sector_valid(
        durable_batch.sectors[(kernel.epoch - 1) % EPOCH_LOG_BATCH_EVENTS], kernel.epoch));
    durable_batch.failed = EPOCH_JOURNAL_IO;
    assert(!kernel_epoch_current(kernel.epoch));
    durable_batch.failed = 0;
    durable_batch.flushed_through = kernel.epoch;
    assert(kernel_epoch_current(kernel.epoch));

    kernel.current = 1;
    assert(!kernel_epoch_log_flush(domain_get(1), true));
    kernel.current = CURRENT_PD_sexfiles;
    files->task_state = TASK_RUNNING;
    TrapFrame begin = {.rax = SEXOS_SYSCALL_epoch_flush, .rdi = 1};
    assert(kernel_syscall(&begin) == &begin && begin.rax == 1 && kernel_epoch_flush_active);
    uint64_t before = kernel.epoch, producer = durable_batch.producer;
    kernel_epoch_record(before, KERNEL_EPOCH_RING_PUSH, 9, before + 1, NULL, 0);
    assert(kernel.epoch == before && durable_batch.producer == producer);
    kernel.current = CURRENT_PD_sexdrive;
    kernel_epoch_record(before, KERNEL_EPOCH_RING_POP, 9, before + 1, NULL, 0);
    assert(kernel.epoch == before && durable_batch.producer == producer);
    /* The same progress obligation holds during an explicit flush even with
     * a healthy, empty producer batch, matching the early-boot stall. */
    uint64_t next_irq = kernel.vector_revision[vector] + 1;
    kernel.current = other->id;
    kernel_device_interrupt_publish(vector);
    assert(kernel.vector_pending[vector] == next_irq && !drive->ring.count);
    kernel.current = drive->id;
    drive->task_state = TASK_RUNNING;
    irq = kernel_listen_message(drive, 0);
    assert(irq.type_id == 0xff && irq.arg1 == next_irq &&
           kernel.vector_heard_revision[vector] == next_irq &&
           !kernel.vector_pending[vector] && !kernel.vector_ack_revision[vector]);
    assert(kernel.epoch == before && durable_batch.producer == producer);
    assert(!kernel_epoch_log_eligible(1) && kernel_epoch_log_eligible(CURRENT_PD_sexdrive) &&
           kernel_epoch_log_eligible(CURRENT_PD_sexfiles));
    kernel.run_queue.head = kernel.run_queue.count = 0;
    for (uint32_t id = 1; id <= 3; ++id) {
        domain_get(id)->queued = false;
        domain_get(id)->task_state = TASK_READY;
        run_push(id);
    }
    files->queued = false;
    files->task_state = TASK_READY;
    run_push(files->id);
    assert(run_pop() == CURRENT_PD_sexdrive);
    assert(run_pop() == CURRENT_PD_sexfiles);
    assert(run_pop() == 0 && domain_get(1)->queued && domain_get(3)->queued && kernel.run_queue.count == 2);

    kernel.current = CURRENT_PD_sexfiles;
    files->task_state = TASK_RUNNING;
    TrapFrame end = {.rax = SEXOS_SYSCALL_epoch_flush, .rdi = 0};
    assert(kernel_syscall(&end) == &end && end.rax == 1 && !kernel_epoch_flush_active);
    assert(run_pop() == 1);
    kernel.current = CURRENT_PD_sexfiles;
    files->task_state = TASK_RUNNING;
    assert(kernel_epoch_log_flush(files, true));
    files->task_state = TASK_EXITED;
    kernel_epoch_log_poll();
    assert(!kernel_epoch_flush_active && durable_batch.failed == EPOCH_JOURNAL_IO);
    kernel_epoch_batch = NULL;
    kernel_epoch_flush_task = 0;
}

static void handoff_controls(void)
{
    reset_transport();
    Domain *a = domain_get(1), *b = domain_get(2), *other = domain_get(3);
    TrapFrame a_frame = {0}, b_frame = {0};
    a->frame = &a_frame;
    b->frame = &b_frame;
    a->task_state = TASK_RUNNING;
    b->task_state = TASK_WAIT_IO;
    other->task_state = TASK_READY;
    run_push(other->id);
    kernel.current = a->id;
    TrapFrame call = {.rax = 100, .rdi = 7, .rsi = CALL_TYPE, .rdx = 55};
    TrapFrame *call_next = kernel_syscall(&call);
    if (call_next != &b_frame)
        fprintf(stderr, "call-wait: next=%p expected=%p status=%llu current=%u ring=%zu handoff=%lu queued=%zu\n",
                (void *)call_next, (void *)&b_frame, (unsigned long long)call.rax,
                kernel.current, (size_t)b->ring.count, (unsigned long)kernel.run_queue.handoff,
                kernel.run_queue.count);
    assert(call_next == &b_frame);
    assert(kernel.current == b->id && b->task_state == TASK_RUNNING);
    assert(b->ring.count == 1 && b->reply_rights[a->id] == 1);
    assert(!a->queued && !kernel.run_queue.handoff && kernel.run_queue.count == 1);
    assert(other->queued);

    b->wake_pending = false;
    TrapFrame heard = {.rax = SEXOS_SYSCALL_listen, .rdi = 2};
    assert(kernel_syscall(&heard) == &heard);
    assert(heard.rax == CALL_TYPE && heard.rdx == 55 && !b->ring.count);
    a->task_state = TASK_WAIT_IO;
    b->task_state = TASK_RUNNING;
    TrapFrame reply = {.rax = 101, .rdi = a->id, .rsi = 777};
    TrapFrame *reply_next = kernel_syscall(&reply);
    assert(a->frame == &call && reply_next == a->frame);
    assert(kernel.current == a->id && a->task_state == TASK_RUNNING);
    assert(a->reply_ready && a->reply_value == 777);
    assert(a->reply_caller == b->id && a->reply_task == b->task_id);
    assert(!b->queued && !kernel.run_queue.handoff && kernel.run_queue.count == 1);
    assert(other->queued && run_pop() == other->id);
}

static void controls(void)
{
    handoff_controls();
    ring_epoch_controls();
    input_epoch_controls();
    epoch_window_controls();
    durable_epoch_controls();
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
    next_task_id = PROGRAM_WORKER_TASK_FIRST - 1;
    assert(task_attach(&last) && last.task_id == PROGRAM_WORKER_TASK_FIRST - 1 &&
           next_task_id == PROGRAM_WORKER_TASK_FIRST);
    assert(!task_attach(&exhausted) && exhausted.task_id == 0);
    assert(!task_attach(&exhausted) && next_task_id == PROGRAM_WORKER_TASK_FIRST);

    reset_transport();
    kernel.current = a->id;
    a->task_state = TASK_RUNNING;
    b->rpc_enabled = true;
    ProgramWorker worker = {.id = PROGRAM_WORKER_TASK_FIRST, .target = a->id,
        .run = a->task_id + 1, .phase = PROGRAM_WORKER_ATTACHED, .owner_core = 0};
    kernel_current_worker = &worker;
    uint64_t next_request = a->rpc_next;
    bad = (TrapFrame){.rdi = 7, .rsi = CALL_TYPE};
    pdx_call(a, &bad);
    assert(bad.rax && !b->ring.count && !b->reply_rights[a->id]);
    /* This fixture has main-only CPU/FPU providers. Exercise the real RPC
     * admission boundary directly; worker execution has its own native suite. */
    bad = (TrapFrame){.rdi = 7, .rsi = CALL_TYPE, .rdx = 19};
    kernel_rpc_syscall(a, &bad, SEXOS_RPC_call);
    assert(bad.rax && !b->ring.count && a->rpc_next == next_request);
    for (size_t i = 0; i < KERNEL_RPC_CAPACITY; ++i)
        assert(a->rpc[i].state == RPC_FREE);
    worker.run = a->task_id;
    bad = (TrapFrame){.rdi = 7, .rsi = CALL_TYPE};
    pdx_call(a, &bad);
    assert(!bad.rax && b->ring.count == 1 && b->reply_rights[a->id] == 1);
    assert(b->ring.entries[b->ring.head].caller_task == worker.id);
    kernel_current_worker = NULL;

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
