#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "epoch_journal.c"
#include "epoch_files.c"

#define EVENTS 5u
static uint8_t disk[(EPOCH_FILES_SECTORS + EPOCH_JOURNAL_META_SECTORS) * EPOCH_JOURNAL_SECTOR_SIZE];
static uint8_t io_buffer[EPOCH_JOURNAL_SPAN_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE];
static EpochLogBatch batch;
static uint64_t bridge_va, writes, reads, syncs, corrupt_read = UINT64_MAX, retry_read_at = UINT64_MAX,
               retry_sync_at = UINT64_MAX,
               flush_entries, flush_exits;
static int fail_write = -1, fail_sync = 0, fail_read = -1, retry_once, retry_sync_once;
static unsigned retry_meta_write, retry_meta_read;
static bool flush_active, region_valid = true;

uint64_t diskfs_bridge_get_buf_va(void) { return bridge_va; }
void sexos_line(const char *format, ...) { (void)format; }

uint64_t sexos_epoch_flush(uint64_t enter)
{
    if (enter) {
        assert(!flush_active);
        flush_active = true;
        ++flush_entries;
    } else {
        assert(flush_active);
        flush_active = false;
        ++flush_exits;
    }
    return 1;
}

MemoryRegionReply sexos_memory_region_info(uint64_t slot)
{
    assert(slot == SEXOS_SLOT_epoch_log);
    const uint64_t rights = REGION_QUERY | REGION_READ | REGION_WRITE;
    return region_valid ? (MemoryRegionReply){0, EPOCH_LOG_REGION_BYTES, rights, 0, EPOCH_LOG_REGION_ID} :
                          (MemoryRegionReply){UINT64_MAX, 0, 0, 0, 0};
}

MemoryRegionReply sexos_memory_region_view(uint64_t slot, uint64_t offset, uint64_t length,
                                            uint64_t rights)
{
    assert(region_valid && slot == SEXOS_SLOT_epoch_log && !offset && length == sizeof batch);
    assert(rights == (REGION_QUERY | REGION_READ | REGION_WRITE));
    return (MemoryRegionReply){0, (uint64_t)(uintptr_t)&batch, sizeof batch, 0, 0};
}

static int64_t host_read(uint64_t lba, uint8_t *out)
{
    assert(lba >= EPOCH_FILES_BASE_LBA && lba < EPOCH_FILES_BASE_LBA + EPOCH_FILES_SECTORS + EPOCH_JOURNAL_META_SECTORS);
    if (fail_read >= 0 && lba == EPOCH_FILES_BASE_LBA + (uint64_t)fail_read)
        return EPOCH_JOURNAL_IO;
    memcpy(out, disk + (lba - EPOCH_FILES_BASE_LBA) * EPOCH_JOURNAL_SECTOR_SIZE,
           EPOCH_JOURNAL_SECTOR_SIZE);
    if (reads++ == corrupt_read)
        out[EPOCH_JOURNAL_HEADER_SIZE + offsetof(KernelEpochEvent, subject)] ^= 1;
    return EPOCH_JOURNAL_OK;
}

uint64_t diskfs_block_read(uint64_t offset, uint64_t size, uint64_t cap)
{
    assert(!(offset % 512) && size && !(size % 512) && size <= 4096 && cap == SEXOS_SLOT_buf_lend);
    bool metadata = offset / 512 >= EPOCH_FILES_BASE_LBA + EPOCH_FILES_SECTORS;
    if (metadata && retry_meta_read == 1) {
        retry_meta_read = 0;
        return BLOCK_ERR_TIMEOUT;
    }
    if (reads == retry_read_at) {
        retry_read_at = UINT64_MAX;
        return BLOCK_ERR_TIMEOUT;
    }
    for (uint64_t sector = 0; sector < size / 512; ++sector) {
        int64_t status = host_read(offset / 512 + sector, io_buffer + sector * 512);
        if (status != EPOCH_JOURNAL_OK) return status;
    }
    if (metadata && retry_meta_read == 2) {
        retry_meta_read = 0;
        return BLOCK_ERR_TIMEOUT;
    }
    return EPOCH_JOURNAL_OK;
}

uint64_t diskfs_block_write(uint64_t offset, uint64_t size, uint64_t cap)
{
    assert(!(offset % 512) && size && !(size % 512) && size <= 4096 && cap == SEXOS_SLOT_buf_lend);
    bool metadata = offset / 512 >= EPOCH_FILES_BASE_LBA + EPOCH_FILES_SECTORS;
    if (metadata && retry_meta_write == 1) {
        retry_meta_write = 0;
        return BLOCK_ERR_TIMEOUT;
    }
    if (retry_once) {
        --retry_once;
        return BLOCK_ERR_TIMEOUT;
    }
    for (uint64_t sector = 0; sector < size / 512; ++sector) {
        uint64_t lba = offset / 512 + sector;
        assert(lba >= EPOCH_FILES_BASE_LBA &&
               lba < EPOCH_FILES_BASE_LBA + EPOCH_FILES_SECTORS + EPOCH_JOURNAL_META_SECTORS);
        uint64_t current = writes++;
        if (fail_write >= 0 && current == (uint64_t)fail_write) return EPOCH_JOURNAL_IO;
        memcpy(disk + (lba - EPOCH_FILES_BASE_LBA) * EPOCH_JOURNAL_SECTOR_SIZE,
               io_buffer + sector * 512, EPOCH_JOURNAL_SECTOR_SIZE);
    }
    if (metadata && retry_meta_write == 2) {
        retry_meta_write = 0;
        return BLOCK_ERR_TIMEOUT;
    }
    return EPOCH_JOURNAL_OK;
}

uint64_t diskfs_block_sync(void) {
    if (syncs++ == retry_sync_at) {
        retry_sync_at = UINT64_MAX;
        return BLOCK_ERR_TIMEOUT;
    }
    if (retry_sync_once) { --retry_sync_once; return BLOCK_ERR_TIMEOUT; }
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
    memset(io_buffer, 0, sizeof io_buffer);
    memset(&batch, 0, sizeof batch);
    bridge_va = (uint64_t)(uintptr_t)io_buffer;
    writes = 0;
    reads = 0;
    syncs = 0;
    corrupt_read = UINT64_MAX;
    retry_read_at = UINT64_MAX;
    retry_sync_at = UINT64_MAX;
    retry_meta_write = retry_meta_read = 0;
    fail_write = -1;
    fail_sync = 0;
    fail_read = -1;
    retry_once = 0;
    retry_sync_once = 0;
    flush_active = false;
    region_valid = true;
    epoch_files_init();
}

static void batch_put(uint64_t epoch)
{
    uint8_t *sector = batch.sectors[(epoch - 1) % EPOCH_LOG_BATCH_EVENTS];
    KernelEpochEvent event = sample(epoch);
    assert(epoch_journal_event_encode(sector, &event) == EPOCH_JOURNAL_OK);
    batch.producer = epoch + 1;
}

static void expect_at(uint64_t sequence, uint64_t epoch)
{
    KernelEpochEvent expected = sample(epoch), actual;
    assert(epoch_journal_read(&epoch_files_journal, sequence, &actual) == EPOCH_JOURNAL_OK);
    assert(!memcmp(&expected, &actual, sizeof expected));
}

static void expect(uint64_t epoch) { expect_at(epoch + 1, epoch); }

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

static void basic_and_capacity(void)
{
    reset();
    for (uint64_t epoch = 1; epoch <= EVENTS; ++epoch) batch_put(epoch);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK);
    assert(batch.flushed_through == EVENTS && epoch_files_journal.next_sequence == EVENTS + 2 &&
           epoch_files_journal.commit_generation == 2);
    for (uint64_t epoch = 1; epoch <= EVENTS; ++epoch) expect(epoch);
    assert(epoch_files_open() == EPOCH_JOURNAL_OK && epoch_files_journal.next_sequence == EVENTS + 2);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK && batch.flushed_through == EVENTS);
    ReplayState state = {.next_event = 1};
    assert(epoch_journal_replay(&epoch_files_journal, count_replay, &state) == EPOCH_JOURNAL_OK);
    assert(state.count == EVENTS && state.sessions == 1);
    batch_put(EVENTS + 1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_FULL);
    assert(batch.failed == EPOCH_JOURNAL_FULL && batch.flushed_through == EVENTS);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_FULL);
}

static void failure_and_gap(void)
{
    reset();
    for (uint64_t epoch = 1; epoch <= EVENTS; ++epoch) batch_put(epoch);
    fail_write = 4;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_IO);
    assert(batch.failed == EPOCH_JOURNAL_IO && !batch.flushed_through &&
           epoch_files_journal.next_sequence == 2);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_IO);
    fail_write = -1;
    batch.failed = 0;
    int64_t resumed = epoch_files_flush(&batch);
    if (resumed != EPOCH_JOURNAL_OK)
        fprintf(stderr, "resumed=%lld next=%llu event=%llu uncertain=%u failed=%llu\n",
                (long long)resumed, (unsigned long long)epoch_files_journal.next_sequence,
                (unsigned long long)epoch_files_journal.next_event,
                (unsigned)epoch_files_journal.uncertain, (unsigned long long)batch.failed);
    assert(resumed == EPOCH_JOURNAL_OK && batch.flushed_through == EVENTS);
    expect(EVENTS);

    reset();
    batch_put(1);
    batch_put(3);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_DENIED);
    assert(batch.failed == EPOCH_JOURNAL_DENIED && !batch.flushed_through &&
           epoch_files_journal.next_sequence == 2);
    KernelEpochEvent boundary;
    assert(epoch_journal_read(&epoch_files_journal, 2, &boundary) == EPOCH_JOURNAL_MISSING);
}

static void recovery_and_readback(void)
{
    reset();
    batch_put(1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK);
    epoch_files_init();
    memset(&batch, 0, sizeof batch);
    batch_put(1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK);
    memset(disk + EPOCH_JOURNAL_SECTOR_SIZE, 0, EPOCH_JOURNAL_SECTOR_SIZE);
    epoch_files_init();
    assert(epoch_files_open() == EPOCH_JOURNAL_CORRUPT && !epoch_files_ready);
    ReplayState damaged = {.next_event = 1};
    assert(epoch_journal_replay(&epoch_files_journal, count_replay, &damaged) == EPOCH_JOURNAL_CORRUPT);
    memset(disk + EPOCH_FILES_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE, 0,
           EPOCH_JOURNAL_META_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE);
    epoch_files_init();
    assert(epoch_files_open() == EPOCH_JOURNAL_CORRUPT && !epoch_files_ready);

    reset();
    batch_put(1);
    assert(epoch_files_open() == EPOCH_JOURNAL_OK);
    corrupt_read = reads;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_CORRUPT);
    assert(batch.failed == EPOCH_JOURNAL_CORRUPT && !batch.flushed_through &&
           epoch_files_journal.next_sequence == 2);
    corrupt_read = UINT64_MAX;
    batch.failed = 0;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK && batch.flushed_through == 1);
    expect(1);

    reset();
    assert(epoch_files_open() == EPOCH_JOURNAL_OK);
    batch_put(1);
    fail_sync = 1;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_IO);
    assert(batch.failed == EPOCH_JOURNAL_IO && !batch.flushed_through &&
           epoch_files_journal.next_sequence == 3 && epoch_files_journal.commit_generation == 1);
    fail_sync = 0;
    batch.failed = 0;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK && batch.flushed_through == 1);
    expect(1);
}

static void corruption_and_buffers(void)
{
    reset();
    batch_put(1);
    batch.sectors[0][EPOCH_JOURNAL_HEADER_SIZE + offsetof(KernelEpochEvent, subject)] ^= 1;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_CORRUPT);
    assert(batch.failed == EPOCH_JOURNAL_CORRUPT && !batch.flushed_through &&
           epoch_files_journal.next_sequence == 2);

    reset();
    bridge_va = 0;
    batch_put(1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_IO);
    assert(batch.failed == EPOCH_JOURNAL_IO && !batch.flushed_through);

    reset();
    assert(epoch_files_open() == EPOCH_JOURNAL_OK);
    bridge_va = 0;
    batch_put(1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_IO);
    assert(batch.failed == EPOCH_JOURNAL_IO && !batch.flushed_through && writes == 2);

    reset();
    fail_read = 0;
    assert(epoch_files_open() == EPOCH_JOURNAL_IO && !epoch_files_ready);
    fail_read = -1;
    assert(epoch_files_open() == EPOCH_JOURNAL_OK);
    batch_put(1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK && batch.flushed_through == 1);
}

static void two_boots(void)
{
    reset();
    batch_put(1);
    batch_put(2);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK);
    epoch_files_init();
    memset(&batch, 0, sizeof batch);
    batch_put(1);
    batch_put(2);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK);
    assert(epoch_files_journal.sessions == 2 && epoch_files_journal.next_event == 3 &&
           epoch_files_journal.next_sequence == 7);
    KernelEpochEvent boundary;
    assert(epoch_journal_read(&epoch_files_journal, 4, &boundary) == EPOCH_JOURNAL_OK);
    assert(boundary.kind == EPOCH_JOURNAL_SESSION_KIND && boundary.revision == 2);
    expect_at(5, 1);
    expect_at(6, 2);
    ReplayState state = {.next_event = 1};
    assert(epoch_journal_replay(&epoch_files_journal, count_replay, &state) == EPOCH_JOURNAL_OK);
    assert(state.sessions == 2 && state.count == 4);
}

static void retry_backpressure(void)
{
    for (unsigned metadata=0;metadata<2;++metadata) {
        reset(); batch_put(1); retry_read_at=metadata;
        assert(epoch_files_flush(&batch)==EPOCH_JOURNAL_RETRY && !batch.failed && !batch.flushed_through);
        assert(epoch_files_flush(&batch)==EPOCH_JOURNAL_OK && batch.flushed_through==1 && !batch.failed);
    }
    reset();
    uint8_t sector[512]; memset(sector,0xa5,sizeof sector);
    retry_read_at=reads;
    assert(epoch_files_read_block(EPOCH_FILES_BASE_LBA,sector)==EPOCH_JOURNAL_RETRY);
    for (size_t i=0;i<sizeof sector;++i) assert(sector[i]==0xa5);
    retry_once=1;
    assert(epoch_files_write_block(EPOCH_FILES_BASE_LBA,sector)==EPOCH_JOURNAL_RETRY);
    assert(!writes && !batch.failed);
    retry_sync_once=1;
    assert(epoch_files_sync_blocks()==EPOCH_JOURNAL_RETRY && !batch.failed);
    assert(epoch_files_sync_blocks()==EPOCH_JOURNAL_OK);
    reset();
    assert(epoch_files_open() == EPOCH_JOURNAL_OK);
    batch_put(1);
    retry_once = 1;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_RETRY && !batch.failed &&
           !batch.flushed_through && !retry_once && batch.retries == 1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK && batch.flushed_through == 1 &&
           !batch.retries);
    expect(1);

    batch_put(2);
    retry_once = 1;
    batch.retries = 4;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_IO && batch.failed == EPOCH_JOURNAL_IO &&
           batch.flushed_through == 1 && !retry_once && batch.retries == 4);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_IO);
    assert(flush_entries == flush_exits && !flush_active);

    reset();
    batch_put(1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK);
    epoch_files_init();
    memset(&batch, 0, sizeof batch);
    batch_put(1);
    retry_read_at = reads + 4;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_RETRY && !batch.failed &&
           !batch.flushed_through && batch.retries == 1);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK && batch.flushed_through == 1 &&
           !batch.retries);
    expect_at(4, 1);
}

static void commit_retries(void)
{
    for (unsigned fault = 0; fault < 6; ++fault) {
        reset();
        assert(epoch_files_open() == EPOCH_JOURNAL_OK);
        uint64_t generation = epoch_files_journal.commit_generation;
        batch_put(1);
        switch (fault) {
        case 0: retry_sync_at = syncs; break; /* commit's first sync */
        case 1: retry_meta_write = 1; break;
        case 2: retry_meta_write = 2; break;
        case 3: retry_sync_at = syncs + 1; break; /* metadata sync */
        case 4: retry_meta_read = 1; break;
        case 5: retry_meta_read = 2; break;
        }
        assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !batch.failed);
        if (!epoch_files_journal.uncertain || batch.flushed_through != 0)
            fprintf(stderr, "commit retry fault=%u uncertain=%u flushed=%llu syncs=%llu reads=%llu writes=%llu pending_sync=%llu\n",
                    fault, epoch_files_journal.uncertain, (unsigned long long)batch.flushed_through,
                    (unsigned long long)syncs, (unsigned long long)reads, (unsigned long long)writes,
                    (unsigned long long)retry_sync_at);
        assert(epoch_files_journal.uncertain && !batch.flushed_through);
        assert(epoch_files_journal.commit_generation == generation && batch.retries == 1);
        uint64_t sequence = epoch_files_journal.next_sequence, entries = flush_entries;
        assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !batch.failed);
        assert(!epoch_files_journal.uncertain && batch.flushed_through == 1 &&
               epoch_files_journal.next_sequence == sequence &&
               epoch_files_journal.commit_generation == generation + 1);
        assert(flush_entries == entries + 1 && flush_entries == flush_exits);
        expect(1);
        entries = flush_entries;
        assert(epoch_files_poll() == EPOCH_JOURNAL_OK && flush_entries == entries);
    }

    /* New producer work while a metadata receipt is uncertain still appends
     * each event once and commits the complete, ordered prefix. */
    reset();
    assert(epoch_files_open() == EPOCH_JOURNAL_OK);
    batch_put(1);
    retry_meta_write = 2;
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !batch.failed && epoch_files_journal.uncertain);
    batch_put(2);
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !batch.failed && !epoch_files_journal.uncertain);
    assert(batch.flushed_through == 1 && epoch_files_journal.next_event == 2);
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !batch.failed);
    assert(batch.flushed_through == 2 && epoch_files_journal.next_event == 3);
    expect(1); expect(2);

    /* Quiet metadata retries retain the existing bounded no-progress policy. */
    reset();
    assert(epoch_files_open() == EPOCH_JOURNAL_OK);
    batch_put(1);
    retry_meta_read = 1;
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !batch.failed && epoch_files_journal.uncertain);
    for (unsigned attempt = 2; attempt <= 4; ++attempt) {
        retry_sync_once = 1;
        assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !batch.failed && batch.retries == attempt);
    }
    retry_sync_once = 1;
    assert(epoch_files_poll() == EPOCH_JOURNAL_IO && batch.failed == EPOCH_JOURNAL_IO);
    assert(!flush_active && flush_entries == flush_exits);

    /* A metadata-only retry must also be polled, even with no new records. */
    reset();
    batch_put(1);
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && batch.flushed_through == 1);
    uint64_t sequence = epoch_files_journal.next_sequence, generation = epoch_files_journal.commit_generation;
    retry_meta_read = 1;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_RETRY && !batch.failed &&
           batch.flushed_through == 1 && epoch_files_journal.uncertain);
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && !epoch_files_journal.uncertain);
    assert(epoch_files_journal.next_sequence == sequence &&
           epoch_files_journal.commit_generation == generation + 1);

    for (unsigned malformed = 0; malformed < 4; ++malformed) {
        reset();
        assert(epoch_files_open() == EPOCH_JOURNAL_OK);
        batch_put(1);
        switch (malformed) {
        case 0: epoch_files_journal.next_event = 0; break;
        case 1: epoch_files_journal.next_event = batch.producer + 1; break;
        case 2: batch.flushed_through = 1; break;
        case 3: batch.flushed_through = UINT64_MAX; break;
        }
        uint64_t before_reads = reads, before_writes = writes, before_syncs = syncs;
        assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_CORRUPT &&
               batch.failed == EPOCH_JOURNAL_CORRUPT);
        assert(reads == before_reads && writes == before_writes && syncs == before_syncs);
        assert(!flush_active && flush_entries == flush_exits);
    }
}

static void polling(void)
{
    reset();
    region_valid = false;
    assert(epoch_files_poll() == EPOCH_JOURNAL_DENIED && !epoch_files_batch);
    reset();
    batch_put(1);
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && epoch_files_batch == &batch);
    assert(batch.flushed_through == 1 && flush_entries == flush_exits);
    uint64_t entries = flush_entries;
    assert(epoch_files_poll() == EPOCH_JOURNAL_OK && flush_entries == entries);
}

int main(void)
{
    basic_and_capacity();
    failure_and_gap();
    recovery_and_readback();
    corruption_and_buffers();
    two_boots();
    retry_backpressure();
    commit_retries();
    polling();
    assert(!flush_active && flush_entries == flush_exits && flush_entries);
    puts("epoch files: disk ordering, sessions, recovery, gaps, corruption, capacity and I/O controls passed");
    return 0;
}
