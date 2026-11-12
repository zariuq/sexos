
#ifndef SEXOS_EPOCH_JOURNAL_H
#define SEXOS_EPOCH_JOURNAL_H
#include "epoch_types.h"

#define EPOCH_JOURNAL_SECTOR_SIZE 512u
#define EPOCH_JOURNAL_VERSION 1u
#define EPOCH_JOURNAL_HEADER_SIZE 24u
#define EPOCH_JOURNAL_SESSION_KIND 255u
#define EPOCH_JOURNAL_SPAN_SECTORS 8u
#define EPOCH_LOG_BATCH_EVENTS 64u

typedef struct {
    uint8_t sectors[EPOCH_LOG_BATCH_EVENTS][EPOCH_JOURNAL_SECTOR_SIZE];
    volatile uint64_t producer, flushed_through, failed, retries;
} EpochLogBatch;

enum {
    EPOCH_JOURNAL_OK = 0,
    EPOCH_JOURNAL_INVALID = 1,
    EPOCH_JOURNAL_RETRY = 2,
    EPOCH_JOURNAL_DENIED = 3,
    EPOCH_JOURNAL_FULL = 6,
    EPOCH_JOURNAL_CORRUPT = 7,
    EPOCH_JOURNAL_IO = 8,
    EPOCH_JOURNAL_MISSING = 9
};

typedef int64_t (*EpochJournalRead)(uint64_t lba, uint8_t *out);
typedef int64_t (*EpochJournalWrite)(uint64_t lba, const uint8_t *data);
typedef int64_t (*EpochJournalSync)(void);
typedef int64_t (*EpochJournalReplay)(const KernelEpochEvent *event, void *context);
typedef int64_t (*EpochJournalReadSpan)(uint64_t lba, uint64_t sectors, uint8_t *out);
typedef int64_t (*EpochJournalWriteSpan)(uint64_t lba, uint64_t sectors, const uint8_t *data);

typedef struct {
    EpochJournalRead read;
    EpochJournalWrite write;
    EpochJournalSync sync;
    EpochJournalReadSpan read_span;
    EpochJournalWriteSpan write_span;
    uint64_t base, sectors, next_sequence, sessions, next_event, commit_generation;
    bool uncertain;
} EpochJournal;

void epoch_journal_init(EpochJournal *journal);
int64_t epoch_journal_open(EpochJournal *journal, uint64_t base, uint64_t sectors,
                           EpochJournalRead read, EpochJournalWrite write, EpochJournalSync sync);
int64_t epoch_journal_span_io(EpochJournal *journal, EpochJournalReadSpan read_span,
                              EpochJournalWriteSpan write_span);
int64_t epoch_journal_begin_session(EpochJournal *journal);
int64_t epoch_journal_append(EpochJournal *journal, const KernelEpochEvent *event);
int64_t epoch_journal_append_deferred(EpochJournal *journal, const KernelEpochEvent *event);
int64_t epoch_journal_append_span_deferred(EpochJournal *journal, const KernelEpochEvent *events,
                                           uint64_t count);
int64_t epoch_journal_commit(EpochJournal *journal);
int64_t epoch_journal_read(EpochJournal *journal, uint64_t sequence, KernelEpochEvent *out);
int64_t epoch_journal_replay(EpochJournal *journal, EpochJournalReplay replay, void *context);
uint64_t epoch_journal_checksum(const uint8_t *sector);
int64_t epoch_journal_event_encode(uint8_t *sector, const KernelEpochEvent *event);
bool epoch_journal_sector_valid(const uint8_t *sector, uint64_t expected_sequence);
bool epoch_journal_event_sector_valid(const uint8_t *sector, uint64_t expected_epoch);
#endif
