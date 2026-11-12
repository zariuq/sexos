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
#include "epoch_files.h"

static EpochJournal epoch_files_journal;
static EpochLogBatch *epoch_files_batch;
static bool epoch_files_ready;
static uint64_t epoch_files_pending_report;
static uint64_t epoch_files_poll_report;

static int64_t epoch_files_block_status(uint64_t status) {
    return status == 0 ? EPOCH_JOURNAL_OK :
           status == BLOCK_ERR_TIMEOUT ? EPOCH_JOURNAL_RETRY : EPOCH_JOURNAL_IO;
}

void epoch_files_init(void) {
    epoch_journal_init(&epoch_files_journal);
    epoch_files_batch = NULL;
    epoch_files_ready = false;
}

static int64_t epoch_files_read_block(uint64_t lba, uint8_t *out) {
    uint64_t va = diskfs_bridge_get_buf_va();
    if (!va || va == UINT64_MAX || lba > (UINT64_MAX - 511) / 512)
        return EPOCH_JOURNAL_IO;
    int64_t status = epoch_files_block_status(
        diskfs_block_read(lba * 512, EPOCH_JOURNAL_SECTOR_SIZE, SEXOS_SLOT_buf_lend));
    if (status != EPOCH_JOURNAL_OK) return status;
    for (size_t i = 0; i < EPOCH_JOURNAL_SECTOR_SIZE; ++i)
        out[i] = *(volatile uint8_t *)(uintptr_t)(va + i);
    return EPOCH_JOURNAL_OK;
}

static int64_t epoch_files_write_block(uint64_t lba, const uint8_t *data) {
    uint64_t va = diskfs_bridge_get_buf_va();
    if (!va || va == UINT64_MAX || lba > (UINT64_MAX - 511) / 512)
        return EPOCH_JOURNAL_IO;
    for (size_t i = 0; i < EPOCH_JOURNAL_SECTOR_SIZE; ++i)
        *(volatile uint8_t *)(uintptr_t)(va + i) = data[i];
    return epoch_files_block_status(
        diskfs_block_write(lba * 512, EPOCH_JOURNAL_SECTOR_SIZE, SEXOS_SLOT_buf_lend));
}

static int64_t epoch_files_read_blocks(uint64_t lba, uint64_t sectors, uint8_t *out) {
    if (!sectors || sectors > EPOCH_JOURNAL_SPAN_SECTORS) return EPOCH_JOURNAL_INVALID;
    uint64_t va = diskfs_bridge_get_buf_va();
    if (!va || va == UINT64_MAX || lba > (UINT64_MAX - 511) / 512)
        return EPOCH_JOURNAL_IO;
    uint64_t bytes = sectors * EPOCH_JOURNAL_SECTOR_SIZE;
    int64_t status = epoch_files_block_status(diskfs_block_read(lba * 512, bytes, SEXOS_SLOT_buf_lend));
    if (status != EPOCH_JOURNAL_OK) return status;
    for (size_t i = 0; i < bytes; ++i) out[i] = *(volatile uint8_t *)(uintptr_t)(va + i);
    return EPOCH_JOURNAL_OK;
}

static int64_t epoch_files_write_blocks(uint64_t lba, uint64_t sectors, const uint8_t *data) {
    if (!sectors || sectors > EPOCH_JOURNAL_SPAN_SECTORS) return EPOCH_JOURNAL_INVALID;
    uint64_t va = diskfs_bridge_get_buf_va();
    if (!va || va == UINT64_MAX || lba > (UINT64_MAX - 511) / 512)
        return EPOCH_JOURNAL_IO;
    uint64_t bytes = sectors * EPOCH_JOURNAL_SECTOR_SIZE;
    for (size_t i = 0; i < bytes; ++i) *(volatile uint8_t *)(uintptr_t)(va + i) = data[i];
    return epoch_files_block_status(diskfs_block_write(lba * 512, bytes, SEXOS_SLOT_buf_lend));
}

static int64_t epoch_files_sync_blocks(void) {
    return epoch_files_block_status(diskfs_block_sync());
}

static int64_t epoch_files_count_replay(const KernelEpochEvent *event, void *context) {
    (void)event;
    ++*(uint64_t *)context;
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_files_open(void) {
    if (epoch_files_ready) return EPOCH_JOURNAL_OK;
    int64_t status = epoch_journal_open(&epoch_files_journal, EPOCH_FILES_BASE_LBA,
                                       EPOCH_FILES_SECTORS, epoch_files_read_block,
                                       epoch_files_write_block, epoch_files_sync_blocks);
    if (status == EPOCH_JOURNAL_OK)
        status = epoch_journal_span_io(&epoch_files_journal, epoch_files_read_blocks,
                                       epoch_files_write_blocks);
    uint64_t replayed = 0;
    if (status == EPOCH_JOURNAL_OK)
        status = epoch_journal_replay(&epoch_files_journal, epoch_files_count_replay, &replayed);
    if (status == EPOCH_JOURNAL_OK && replayed)
        sexos_line("[sexfiles.epoch.replay] sessions=%llu records=%llu next_sequence=%llu next_event=%llu",
                   (unsigned long long)epoch_files_journal.sessions,
                   (unsigned long long)replayed,
                   (unsigned long long)epoch_files_journal.next_sequence,
                   (unsigned long long)epoch_files_journal.next_event);
    if (status == EPOCH_JOURNAL_OK)
        status = epoch_journal_begin_session(&epoch_files_journal);
    epoch_files_ready = status == EPOCH_JOURNAL_OK;
    return status;
}

static void epoch_files_copy(uint8_t *destination, const uint8_t *source, size_t count) {
    for (size_t i = 0; i < count; ++i) destination[i] = source[i];
}

static int64_t epoch_files_commit_prefix(EpochLogBatch *batch, uint64_t next, uint64_t producer) {
    int64_t status = epoch_journal_commit(&epoch_files_journal);
    if (status == EPOCH_JOURNAL_RETRY) return status;
    if (status != EPOCH_JOURNAL_OK) {
        __atomic_store_n(&batch->failed, (uint64_t)status, __ATOMIC_RELEASE);
        return status;
    }
    __atomic_store_n(&batch->flushed_through, next - 1, __ATOMIC_RELEASE);
    return next < producer ? EPOCH_JOURNAL_RETRY : EPOCH_JOURNAL_OK;
}

static int64_t epoch_files_flush_inner(EpochLogBatch *batch) {
    if (!epoch_files_ready) {
        int64_t status = epoch_files_open();
        if (status == EPOCH_JOURNAL_RETRY) return status;
        if (status != EPOCH_JOURNAL_OK) {
            __atomic_store_n(&batch->failed, (uint64_t)status, __ATOMIC_RELEASE);
            return status;
        }
    }
    uint64_t next = batch->flushed_through + 1;
    uint64_t producer = __atomic_load_n(&batch->producer, __ATOMIC_ACQUIRE);
    if (!next || !epoch_files_journal.next_event || epoch_files_journal.next_event < next ||
        epoch_files_journal.next_event > producer) {
        __atomic_store_n(&batch->failed, EPOCH_JOURNAL_CORRUPT, __ATOMIC_RELEASE);
        return EPOCH_JOURNAL_CORRUPT;
    }
    /* The journal's accepted prefix can precede its durable metadata receipt.
     * Settle that exact prefix before consuming any new producer records. */
    if (epoch_files_journal.next_event > next)
        return epoch_files_commit_prefix(batch, epoch_files_journal.next_event, producer);
    KernelEpochEvent events[EPOCH_FILES_FLUSH_LIMIT];
    uint64_t count = 0;
    while (next < producer && count < EPOCH_FILES_FLUSH_LIMIT) {
        const uint8_t *sector = batch->sectors[(next - 1) % EPOCH_LOG_BATCH_EVENTS];
        epoch_files_copy((uint8_t *)&events[count], sector + EPOCH_JOURNAL_HEADER_SIZE,
                         sizeof(KernelEpochEvent));
        if (events[count].epoch != next) {
            __atomic_store_n(&batch->failed, EPOCH_JOURNAL_DENIED, __ATOMIC_RELEASE);
            return EPOCH_JOURNAL_DENIED;
        }
        if (!epoch_journal_event_sector_valid(sector, next)) {
            __atomic_store_n(&batch->failed, EPOCH_JOURNAL_CORRUPT, __ATOMIC_RELEASE);
            return EPOCH_JOURNAL_CORRUPT;
        }
        ++next;
        ++count;
    }
    if (count) {
        int64_t status = epoch_journal_append_span_deferred(&epoch_files_journal, events, count);
        if (status == EPOCH_JOURNAL_RETRY) return status;
        if (status != EPOCH_JOURNAL_OK) {
            __atomic_store_n(&batch->failed, (uint64_t)status, __ATOMIC_RELEASE);
            return status;
        }
    }
    return epoch_files_commit_prefix(batch, next, producer);
}

int64_t epoch_files_flush(EpochLogBatch *batch) {
    if (!batch) return EPOCH_JOURNAL_INVALID;
    uint64_t failed = __atomic_load_n(&batch->failed, __ATOMIC_ACQUIRE);
    if (failed) return (int64_t)failed;
    if (sexos_epoch_flush(1) != 1) {
        __atomic_store_n(&batch->failed, EPOCH_JOURNAL_DENIED, __ATOMIC_RELEASE);
        return EPOCH_JOURNAL_DENIED;
    }
    uint64_t flushed_before = __atomic_load_n(&batch->flushed_through, __ATOMIC_ACQUIRE);
    int64_t status = epoch_files_flush_inner(batch);
    if (status == EPOCH_JOURNAL_RETRY &&
        __atomic_load_n(&batch->flushed_through, __ATOMIC_ACQUIRE) == flushed_before) {
        uint64_t retries = __atomic_load_n(&batch->retries, __ATOMIC_ACQUIRE) + 1;
        if (retries > 4) {
            __atomic_store_n(&batch->failed, EPOCH_JOURNAL_IO, __ATOMIC_RELEASE);
            (void)sexos_epoch_flush(0);
            return EPOCH_JOURNAL_IO;
        }
        __atomic_store_n(&batch->retries, retries, __ATOMIC_RELEASE);
    } else {
        __atomic_store_n(&batch->retries, 0, __ATOMIC_RELEASE);
    }
    if (sexos_epoch_flush(0) != 1) {
        __atomic_store_n(&batch->failed, EPOCH_JOURNAL_IO, __ATOMIC_RELEASE);
        return EPOCH_JOURNAL_IO;
    }
    if (status == EPOCH_JOURNAL_OK || status == EPOCH_JOURNAL_RETRY) {
        uint64_t through = __atomic_load_n(&batch->flushed_through, __ATOMIC_ACQUIRE);
        if (through && (through <= 8 || !(through & (through - 1))))
            sexos_line("[sexfiles.epoch.flush] session=%llu through=%llu next_sequence=%llu failed=%llu",
                       (unsigned long long)epoch_files_journal.sessions,
                       (unsigned long long)through,
                       (unsigned long long)epoch_files_journal.next_sequence,
                       (unsigned long long)__atomic_load_n(&batch->failed, __ATOMIC_ACQUIRE));
    }
    return status;
}

static int64_t epoch_files_attach(void) {
    if (epoch_files_batch) return EPOCH_JOURNAL_OK;
    const uint64_t rights = REGION_QUERY | REGION_READ | REGION_WRITE;
    MemoryRegionReply info = sexos_memory_region_info(SEXOS_SLOT_epoch_log);
    if (info.status != 0 || info.value0 != EPOCH_LOG_REGION_BYTES || info.value1 != rights ||
        info.value2 != 0 || info.value3 != EPOCH_LOG_REGION_ID)
        return EPOCH_JOURNAL_DENIED;
    MemoryRegionReply view = sexos_memory_region_view(SEXOS_SLOT_epoch_log, 0,
                                                       sizeof(EpochLogBatch), rights);
    if (view.status != 0 || !view.value0 || view.value0 == UINT64_MAX ||
        view.value0 > UINTPTR_MAX - sizeof(EpochLogBatch) || view.value1 != sizeof(EpochLogBatch))
        return EPOCH_JOURNAL_DENIED;
    epoch_files_batch = (EpochLogBatch *)(uintptr_t)view.value0;
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_files_poll(void) {
    int64_t status = epoch_files_attach();
    if (status != EPOCH_JOURNAL_OK) return status;
    uint64_t failed = __atomic_load_n(&epoch_files_batch->failed, __ATOMIC_ACQUIRE);
    if (failed) return (int64_t)failed;
    uint64_t flushed = __atomic_load_n(&epoch_files_batch->flushed_through, __ATOMIC_ACQUIRE);
    uint64_t producer = __atomic_load_n(&epoch_files_batch->producer, __ATOMIC_ACQUIRE);
    if (producer && producer == flushed + 1 && !epoch_files_journal.uncertain)
        return EPOCH_JOURNAL_OK;
    if (!producer || producer <= flushed || producer - 1 - flushed > EPOCH_LOG_BATCH_EVENTS)
        return EPOCH_JOURNAL_CORRUPT;
    if (producer != epoch_files_poll_report) {
        epoch_files_poll_report = producer;
        sexos_line("[sexfiles.epoch.poll] producer=%llu flushed=%llu",
                   (unsigned long long)producer, (unsigned long long)flushed);
    }
    if (producer - 1 - flushed >= 32 && producer != epoch_files_pending_report) {
        epoch_files_pending_report = producer;
        sexos_line("[sexfiles.epoch.pending] producer=%llu flushed=%llu",
                   (unsigned long long)producer, (unsigned long long)flushed);
    }
    status = epoch_files_flush(epoch_files_batch);
    return status == EPOCH_JOURNAL_RETRY ? EPOCH_JOURNAL_OK : status;
}
