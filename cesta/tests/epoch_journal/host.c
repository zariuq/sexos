#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "epoch_journal.c"

#define SECTORS 8u
static uint8_t disk[(SECTORS + EPOCH_JOURNAL_META_SECTORS) * EPOCH_JOURNAL_SECTOR_SIZE];
static EpochJournal journal;
static int fail_write = -1, fail_sync = 0, fail_read = -1, corrupt_write = 0;
static int64_t fail_lba = -1;
static uint64_t writes, reads;
static uint64_t io_calls, retry_at=UINT64_MAX;
static bool retry_after_effect;

static bool transient(void) {
    uint64_t at=io_calls++;
    if (at!=retry_at) return false;
    retry_at=UINT64_MAX;
    return true;
}

static int64_t host_read(uint64_t lba, uint8_t *out)
{
    assert(lba < SECTORS + EPOCH_JOURNAL_META_SECTORS);
    bool retry=transient();
    if (retry&&!retry_after_effect) return EPOCH_JOURNAL_RETRY;
    ++reads;
    if (fail_read >= 0 && lba == (uint64_t)fail_read) return EPOCH_JOURNAL_IO;
    memcpy(out, disk + lba * EPOCH_JOURNAL_SECTOR_SIZE, EPOCH_JOURNAL_SECTOR_SIZE);
    return retry ? EPOCH_JOURNAL_RETRY : EPOCH_JOURNAL_OK;
}

static int64_t host_write(uint64_t lba, const uint8_t *data)
{
    assert(lba < SECTORS + EPOCH_JOURNAL_META_SECTORS);
    bool retry=transient();
    if (retry&&!retry_after_effect) return EPOCH_JOURNAL_RETRY;
    uint64_t current = writes++;
    if ((fail_write >= 0 && current == (uint64_t)fail_write) || fail_lba == (int64_t)lba)
        return EPOCH_JOURNAL_IO;
    memcpy(disk + lba * EPOCH_JOURNAL_SECTOR_SIZE, data, EPOCH_JOURNAL_SECTOR_SIZE);
    if (corrupt_write && lba < SECTORS)
        disk[lba * EPOCH_JOURNAL_SECTOR_SIZE + 24 + offsetof(KernelEpochEvent, subject)] ^= 1;
    return retry ? EPOCH_JOURNAL_RETRY : EPOCH_JOURNAL_OK;
}

static uint64_t span_reads, span_writes;

static int64_t host_read_span(uint64_t lba, uint64_t count, uint8_t *out)
{
    ++span_reads;
    for (uint64_t index = 0; index < count; ++index) {
        int64_t status = host_read(lba + index, out + index * EPOCH_JOURNAL_SECTOR_SIZE);
        if (status != EPOCH_JOURNAL_OK) return status;
    }
    return EPOCH_JOURNAL_OK;
}

static int64_t host_write_span(uint64_t lba, uint64_t count, const uint8_t *data)
{
    ++span_writes;
    for (uint64_t index = 0; index < count; ++index) {
        int64_t status = host_write(lba + index, data + index * EPOCH_JOURNAL_SECTOR_SIZE);
        if (status != EPOCH_JOURNAL_OK) return status;
    }
    return EPOCH_JOURNAL_OK;
}

static int64_t host_sync(void) {
    if (transient()) return EPOCH_JOURNAL_RETRY;
    return fail_sync ? EPOCH_JOURNAL_IO : EPOCH_JOURNAL_OK;
}

static KernelEpochEvent sample(uint64_t epoch)
{
    KernelEpochEvent event = {0};
    event.epoch = epoch;
    event.subject = (uint32_t)epoch + 8;
    event.kind = epoch % 2 ? KERNEL_EPOCH_RING_PUSH : KERNEL_EPOCH_RING_POP;
    event.revision = epoch;
    event.message.type_id = UINT64_C(0xabc) + epoch;
    event.message.caller = 7;
    event.message.caller_task = 70;
    return event;
}

static void reset(void)
{
    memset(disk, 0, sizeof disk);
    io_calls=0; retry_at=UINT64_MAX; retry_after_effect=false;
    epoch_journal_init(&journal);
    fail_write = -1;
    fail_sync = 0;
    fail_read = -1;
    corrupt_write = 0;
    fail_lba = -1;
    writes = 0;
    reads = 0;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 1 && !journal.sessions && journal.next_event == 1);
    assert(epoch_journal_begin_session(&journal) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 2 && journal.sessions == 1 && journal.next_event == 1);
}

static void append(uint64_t epoch)
{
    KernelEpochEvent event = sample(epoch);
    assert(epoch_journal_append(&journal, &event) == EPOCH_JOURNAL_OK);
    assert(journal.next_event == epoch + 1);
}

static void expect(uint64_t sequence, uint64_t epoch)
{
    KernelEpochEvent expected = sample(epoch), actual;
    assert(epoch_journal_read(&journal, sequence, &actual) == EPOCH_JOURNAL_OK);
    assert(!memcmp(&expected, &actual, sizeof expected));
}

static void expect_session(uint64_t sequence, uint64_t revision)
{
    KernelEpochEvent event;
    assert(epoch_journal_read(&journal, sequence, &event) == EPOCH_JOURNAL_OK);
    assert(event.kind == EPOCH_JOURNAL_SESSION_KIND && event.epoch == 0 && event.revision == revision);
}

typedef struct {
    uint64_t sessions, next_event, count;
} ReplayState;

static int64_t count_replay(const KernelEpochEvent *event, void *context)
{
    ReplayState *state = context;
    if (event->kind == EPOCH_JOURNAL_SESSION_KIND) {
        assert(event->epoch == 0 && event->revision == state->sessions + 1);
        ++state->sessions;
        state->next_event = 1;
        return EPOCH_JOURNAL_OK;
    }
    assert(state->sessions && event->epoch == state->next_event && event->revision == state->next_event);
    assert(event->subject == state->next_event + 8);
    ++state->next_event;
    ++state->count;
    return EPOCH_JOURNAL_OK;
}

static int64_t fail_replay(const KernelEpochEvent *event, void *context)
{
    ReplayState *state = context;
    if (event->kind != EPOCH_JOURNAL_SESSION_KIND && event->epoch == 2) {
        ++state->count;
        return 42;
    }
    return count_replay(event, context);
}

static int64_t accept_replay(const KernelEpochEvent *event, void *context)
{
    (void)event;
    ++*(uint64_t *)context;
    return EPOCH_JOURNAL_OK;
}

static void basic_and_full(void)
{
    reset();
    expect_session(1, 1);
    append(1);
    expect(2, 1);
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 3 && journal.sessions == 1 && journal.next_event == 2);
    KernelEpochEvent duplicate = sample(1);
    assert(epoch_journal_append(&journal, &duplicate) == EPOCH_JOURNAL_DENIED && journal.next_sequence == 3);
    for (uint64_t epoch = 2; epoch < SECTORS; ++epoch) append(epoch);
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == SECTORS + 1);
    KernelEpochEvent overflow = sample(SECTORS);
    assert(epoch_journal_append(&journal, &overflow) == EPOCH_JOURNAL_FULL && journal.next_sequence == SECTORS + 1);
    for (uint64_t epoch = 1; epoch < SECTORS; ++epoch) expect(epoch + 1, epoch);
    ReplayState state = {.next_event = 1};
    assert(epoch_journal_replay(&journal, count_replay, &state) == EPOCH_JOURNAL_OK);
    assert(state.count == SECTORS - 1 && state.sessions == 1);
}

static void failures_and_recovery(void)
{
    reset();
    append(1); append(2);
    fail_write = 6;
    KernelEpochEvent third = sample(3);
    assert(epoch_journal_append(&journal, &third) == EPOCH_JOURNAL_IO);
    assert(journal.next_sequence == 4 && journal.next_event == 3);
    fail_write = -1;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 4 && journal.next_event == 3);
    append(3);
    fail_sync = 1;
    KernelEpochEvent fourth = sample(4);
    assert(epoch_journal_append(&journal, &fourth) == EPOCH_JOURNAL_IO);
    assert(journal.next_sequence == 5 && journal.next_event == 4);
    fail_sync = 0;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 6 && journal.next_event == 5);
    expect(5, 4);
}

static void corruption_refusals(void)
{
    reset();
    append(1);
    uint8_t saved[EPOCH_JOURNAL_SECTOR_SIZE];
    memcpy(saved, disk + EPOCH_JOURNAL_SECTOR_SIZE, sizeof saved);
    disk[EPOCH_JOURNAL_SECTOR_SIZE + 24 + offsetof(KernelEpochEvent, subject)] ^= 1;
    KernelEpochEvent out;
    assert(epoch_journal_read(&journal, 2, &out) == EPOCH_JOURNAL_CORRUPT);
    memcpy(disk + EPOCH_JOURNAL_SECTOR_SIZE, saved, sizeof saved);
    ++disk[EPOCH_JOURNAL_SECTOR_SIZE + 16];
    epoch_journal_put64(disk + EPOCH_JOURNAL_SECTOR_SIZE + EPOCH_JOURNAL_CHECKSUM_OFFSET,
                        epoch_journal_checksum(disk + EPOCH_JOURNAL_SECTOR_SIZE));
    assert(epoch_journal_read(&journal, 2, &out) == EPOCH_JOURNAL_CORRUPT);
    memcpy(disk + EPOCH_JOURNAL_SECTOR_SIZE, saved, sizeof saved);
    reset();
    corrupt_write = 1;
    KernelEpochEvent initial = sample(1);
    assert(epoch_journal_append(&journal, &initial) == EPOCH_JOURNAL_CORRUPT && journal.next_sequence == 2);

    reset();
    append(1);
    assert(epoch_journal_begin_session(&journal) == EPOCH_JOURNAL_OK);
    append(1);
    memset(disk + EPOCH_JOURNAL_SECTOR_SIZE, 0, EPOCH_JOURNAL_SECTOR_SIZE);
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    ReplayState state = {.next_event = 1};
    assert(epoch_journal_replay(&journal, count_replay, &state) == EPOCH_JOURNAL_CORRUPT);
    memset(disk + SECTORS * EPOCH_JOURNAL_SECTOR_SIZE, 0,
           EPOCH_JOURNAL_META_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE);
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_CORRUPT);
}

static void committed_recovery(void)
{
    reset();
    append(1);
    assert(journal.commit_generation == 2);
    uint64_t before = reads;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(reads - before <= 4 && journal.next_sequence == 3 && journal.next_event == 2);
    uint64_t slot = journal.commit_generation % EPOCH_JOURNAL_META_SECTORS;
    disk[(SECTORS + slot) * EPOCH_JOURNAL_SECTOR_SIZE + 24] ^= 1;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 3 && journal.next_event == 2 && journal.sessions == 1);
    ReplayState state = {.next_event = 1};
    assert(epoch_journal_replay(&journal, count_replay, &state) == EPOCH_JOURNAL_OK);
    assert(state.count == 1 && state.sessions == 1);

    reset();
    fail_lba = SECTORS + (journal.commit_generation + 1) % EPOCH_JOURNAL_META_SECTORS;
    KernelEpochEvent first = sample(1);
    assert(epoch_journal_append(&journal, &first) == EPOCH_JOURNAL_IO);
    assert(journal.next_sequence == 2 && journal.next_event == 1 && journal.uncertain);
    fail_lba = -1;
    assert(epoch_journal_append(&journal, &first) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 3 && journal.next_event == 2 && !journal.uncertain);

    fail_lba = SECTORS + (journal.commit_generation + 1) % EPOCH_JOURNAL_META_SECTORS;
    KernelEpochEvent second = sample(2);
    assert(epoch_journal_append(&journal, &second) == EPOCH_JOURNAL_IO);
    assert(journal.next_sequence == 3 && journal.next_event == 2 && journal.uncertain);
    fail_lba = -1;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 4 && journal.next_event == 3 && journal.sessions == 1);
    expect(3, 2);
    memset(disk + SECTORS * EPOCH_JOURNAL_SECTOR_SIZE, 0,
           EPOCH_JOURNAL_META_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE);
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.next_sequence == 4 && journal.next_event == 3);
    journal.commit_generation = UINT64_MAX;
    assert(epoch_journal_commit(&journal) == EPOCH_JOURNAL_FULL);
}

static void sessions_and_encoding(void)
{
    reset();
    append(1); append(2);
    assert(epoch_journal_begin_session(&journal) == EPOCH_JOURNAL_OK);
    assert(journal.sessions == 2 && journal.next_event == 1 && journal.next_sequence == 5);
    append(1);
    expect_session(4, 2);
    expect(5, 1);
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(journal.sessions == 2 && journal.next_event == 2 && journal.next_sequence == 6);
    ReplayState state = {.next_event = 1};
    assert(epoch_journal_replay(&journal, count_replay, &state) == EPOCH_JOURNAL_OK);
    assert(state.sessions == 2 && state.count == 3);

    uint8_t sector[EPOCH_JOURNAL_SECTOR_SIZE];
    KernelEpochEvent event = sample(9);
    assert(epoch_journal_event_encode(sector, &event) == EPOCH_JOURNAL_OK);
    assert(epoch_journal_event_sector_valid(sector, 9));
    assert(!epoch_journal_event_sector_valid(sector, 8));
    event.kind = EPOCH_JOURNAL_SESSION_KIND;
    assert(epoch_journal_event_encode(sector, &event) == EPOCH_JOURNAL_INVALID);

    reset();
    assert(epoch_journal_span_io(&journal, host_read_span, host_write_span) == EPOCH_JOURNAL_OK);
    KernelEpochEvent events[3] = {sample(1), sample(2), sample(3)};
    span_reads = span_writes = 0;
    assert(epoch_journal_append_span_deferred(&journal, events, 3) == EPOCH_JOURNAL_OK);
    assert(span_reads == 1 && span_writes == 1 && journal.next_sequence == 5 && journal.next_event == 4);
    assert(epoch_journal_commit(&journal) == EPOCH_JOURNAL_OK);
    for (uint64_t epoch = 1; epoch <= 3; ++epoch) expect(epoch + 1, epoch);
    ReplayState span_state = {.next_event = 1};
    span_reads = 0;
    assert(epoch_journal_replay(&journal, count_replay, &span_state) == EPOCH_JOURNAL_OK);
    assert(span_reads == 1 && span_state.sessions == 1 && span_state.count == 3);

    reset();
    assert(epoch_journal_span_io(&journal, host_read_span, host_write_span) == EPOCH_JOURNAL_OK);
    corrupt_write = 1;
    assert(epoch_journal_append_span_deferred(&journal, events, 3) == EPOCH_JOURNAL_CORRUPT);
}

static void callback_and_read_refusals(void)
{
    reset();
    append(1); append(2);
    ReplayState state = {.next_event = 1};
    assert(epoch_journal_replay(&journal, fail_replay, &state) == 42 && state.count == 2);
    uint8_t altered[EPOCH_JOURNAL_SECTOR_SIZE];
    memcpy(altered, disk + 2 * EPOCH_JOURNAL_SECTOR_SIZE, sizeof altered);
    epoch_journal_put64(disk + 2 * EPOCH_JOURNAL_SECTOR_SIZE + EPOCH_JOURNAL_HEADER_SIZE, 3);
    epoch_journal_put64(disk + 2 * EPOCH_JOURNAL_SECTOR_SIZE + EPOCH_JOURNAL_CHECKSUM_OFFSET,
                        epoch_journal_checksum(disk + 2 * EPOCH_JOURNAL_SECTOR_SIZE));
    uint64_t accepted = 0;
    assert(epoch_journal_replay(&journal, accept_replay, &accepted) == EPOCH_JOURNAL_CORRUPT &&
           accepted == 2);
    memcpy(disk + 2 * EPOCH_JOURNAL_SECTOR_SIZE, altered, sizeof altered);
    ++journal.next_event;
    accepted = 0;
    assert(epoch_journal_replay(&journal, accept_replay, &accepted) == EPOCH_JOURNAL_CORRUPT &&
           accepted == 3);
    --journal.next_event;
    KernelEpochEvent out = {.epoch = 99};
    assert(epoch_journal_read(&journal, 0, &out) == EPOCH_JOURNAL_INVALID && out.epoch == 99);
    assert(epoch_journal_read(&journal, 4, &out) == EPOCH_JOURNAL_MISSING && out.epoch == 99);
    assert(epoch_journal_append(&journal, NULL) == EPOCH_JOURNAL_INVALID);
    assert(epoch_journal_replay(&journal, NULL, NULL) == EPOCH_JOURNAL_INVALID);
    fail_read = SECTORS;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_IO);
    fail_read = -1;
    assert(epoch_journal_open(&journal, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    fail_read = 1;
    assert(epoch_journal_read(&journal, 2, &out) == EPOCH_JOURNAL_IO);
    fail_read = -1;
    memset(disk, 0, sizeof disk);
    EpochJournal raw;
    KernelEpochEvent ordinary = sample(1);
    assert(epoch_journal_open(&raw, 0, SECTORS, host_read, host_write, host_sync) == EPOCH_JOURNAL_OK);
    assert(epoch_journal_append(&raw, &ordinary) == EPOCH_JOURNAL_DENIED && raw.next_sequence == 1);
}

static int64_t retry_operation(unsigned operation) {
    if (operation==0) return epoch_journal_open(&journal,0,SECTORS,host_read,host_write,host_sync);
    if (operation==1) {
        KernelEpochEvent event=sample(1);
        return epoch_journal_append(&journal,&event);
    }
    return epoch_journal_begin_session(&journal);
}

static void retry_boundaries(void) {
    unsigned checked=0;
    for (unsigned operation=0;operation<3;++operation) {
        reset(); io_calls=0;
        assert(retry_operation(operation)==EPOCH_JOURNAL_OK);
        uint64_t count=io_calls;
        assert(count);
        for (unsigned after=0;after<2;++after) for (uint64_t boundary=0;boundary<count;++boundary) {
            reset(); io_calls=0; retry_at=boundary; retry_after_effect=after;
            assert(retry_operation(operation)==EPOCH_JOURNAL_RETRY);
            if (operation) assert(journal.next_sequence==2&&journal.sessions==1&&journal.next_event==1&&journal.uncertain);
            assert(retry_operation(operation)==EPOCH_JOURNAL_OK);
            if (operation==1) { assert(journal.next_sequence==3&&journal.next_event==2);expect(2,1); }
            if (operation==2) assert(journal.next_sequence==3&&journal.sessions==2&&journal.next_event==1);
            ++checked;
        }
    }
    reset(); io_calls=0; retry_at=0; retry_after_effect=true;
    KernelEpochEvent accepted=sample(1), changed=sample(1);
    assert(epoch_journal_append(&journal,&accepted)==EPOCH_JOURNAL_RETRY && journal.uncertain);
    ++changed.subject;
    assert(epoch_journal_append(&journal,&changed)==EPOCH_JOURNAL_CORRUPT && journal.next_sequence==2);
    assert(epoch_journal_append(&journal,&accepted)==EPOCH_JOURNAL_OK);
    printf("epoch journal transient boundaries: %u before/after-effect refusals recovered\n",checked);
}

int main(void)
{
    basic_and_full();
    failures_and_recovery();
    corruption_refusals();
    committed_recovery();
    sessions_and_encoding();
    callback_and_read_refusals();
    retry_boundaries();
    puts("epoch journal: ordered records, scoped epochs, sessions, recovery, corruption and capacity controls passed");
    return 0;
}
