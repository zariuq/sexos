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
#include "epoch_journal.h"

#define EPOCH_JOURNAL_CHECKSUM_OFFSET 504u
#define EPOCH_JOURNAL_COMMIT_NEXT 24u
#define EPOCH_JOURNAL_COMMIT_SESSIONS 32u
#define EPOCH_JOURNAL_COMMIT_EVENT 40u

_Static_assert(sizeof(KernelEpochEvent) <= EPOCH_JOURNAL_CHECKSUM_OFFSET - EPOCH_JOURNAL_HEADER_SIZE,
               "epoch event fits the sector payload");
_Static_assert(EPOCH_JOURNAL_SESSION_KIND > KERNEL_EPOCH_DEVICE_RESET,
               "session marker is distinct from kernel effects");

static const uint8_t epoch_journal_magic[8] = {'S','E','X','E','P','O','C','H'};
static const uint8_t epoch_commit_magic[8] = {'S','E','X','J','H','E','A','D'};

static void epoch_journal_copy(uint8_t *destination, const uint8_t *source, size_t count) {
    for (size_t i = 0; i < count; ++i) destination[i] = source[i];
}

static void epoch_journal_put32(uint8_t *destination, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) destination[i] = (uint8_t)(value >> (8 * i));
}

static void epoch_journal_put64(uint8_t *destination, uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) destination[i] = (uint8_t)(value >> (8 * i));
}

static uint32_t epoch_journal_get32(const uint8_t *source) {
    uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i) value |= (uint32_t)source[i] << (8 * i);
    return value;
}

static uint64_t epoch_journal_get64(const uint8_t *source) {
    uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)source[i] << (8 * i);
    return value;
}

uint64_t epoch_journal_checksum(const uint8_t *sector) {
    uint64_t hash = UINT64_C(14695981039346656037);
    for (size_t i = 0; i < EPOCH_JOURNAL_CHECKSUM_OFFSET; ++i) {
        hash ^= sector[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static bool epoch_journal_blank(const uint8_t *sector) {
    for (size_t i = 0; i < EPOCH_JOURNAL_SECTOR_SIZE; ++i)
        if (sector[i]) return false;
    return true;
}

bool epoch_journal_sector_valid(const uint8_t *sector, uint64_t expected_sequence) {
    if (!sector || !expected_sequence) return false;
    for (size_t i = 0; i < 8; ++i)
        if (sector[i] != epoch_journal_magic[i]) return false;
    if (epoch_journal_get32(sector + 8) != EPOCH_JOURNAL_VERSION ||
        epoch_journal_get32(sector + 12) != 0 ||
        epoch_journal_get64(sector + 16) != expected_sequence ||
        epoch_journal_get64(sector + EPOCH_JOURNAL_CHECKSUM_OFFSET) !=
            epoch_journal_checksum(sector))
        return false;
    for (size_t i = EPOCH_JOURNAL_HEADER_SIZE + sizeof(KernelEpochEvent);
         i < EPOCH_JOURNAL_CHECKSUM_OFFSET; ++i)
        if (sector[i]) return false;
    return true;
}

bool epoch_journal_event_sector_valid(const uint8_t *sector, uint64_t expected_epoch) {
    return epoch_journal_sector_valid(sector, expected_epoch) &&
           epoch_journal_get64(sector + EPOCH_JOURNAL_HEADER_SIZE) == expected_epoch &&
           sector[EPOCH_JOURNAL_HEADER_SIZE + offsetof(KernelEpochEvent, kind)] !=
               EPOCH_JOURNAL_SESSION_KIND;
}

static int64_t epoch_journal_encode(uint8_t *sector, uint64_t sequence,
                                    const KernelEpochEvent *event) {
    if (!sector || !event || !sequence) return EPOCH_JOURNAL_INVALID;
    for (size_t i = 0; i < EPOCH_JOURNAL_SECTOR_SIZE; ++i) sector[i] = 0;
    epoch_journal_copy(sector, epoch_journal_magic, 8);
    epoch_journal_put32(sector + 8, EPOCH_JOURNAL_VERSION);
    epoch_journal_put64(sector + 16, sequence);
    epoch_journal_copy(sector + EPOCH_JOURNAL_HEADER_SIZE, (const uint8_t *)event,
                       sizeof(KernelEpochEvent));
    epoch_journal_put64(sector + EPOCH_JOURNAL_CHECKSUM_OFFSET, epoch_journal_checksum(sector));
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_journal_event_encode(uint8_t *sector, const KernelEpochEvent *event) {
    if (!event || !event->epoch || event->kind == EPOCH_JOURNAL_SESSION_KIND)
        return EPOCH_JOURNAL_INVALID;
    return epoch_journal_encode(sector, event->epoch, event);
}

static void epoch_journal_decode(const uint8_t *sector, KernelEpochEvent *event) {
    epoch_journal_copy((uint8_t *)event, sector + EPOCH_JOURNAL_HEADER_SIZE,
                       sizeof(KernelEpochEvent));
}

void epoch_journal_init(EpochJournal *journal) {
    if (!journal) return;
    journal->read = NULL;
    journal->write = NULL;
    journal->sync = NULL;
    journal->read_span = NULL;
    journal->write_span = NULL;
    journal->base = 0;
    journal->sectors = 0;
    journal->next_sequence = 1;
    journal->sessions = 0;
    journal->next_event = 1;
    journal->commit_generation = 0;
    journal->uncertain = false;
}

static bool epoch_journal_session_valid(const KernelEpochEvent *event, uint64_t expected_session) {
    return event->epoch == 0 && event->subject == 0 && event->peer == 0 &&
           event->kind == EPOCH_JOURNAL_SESSION_KIND && event->scancode == 0 &&
           event->rights == 0 && event->key == 0 && event->source_key == 0 &&
           event->revision == expected_session && event->address == 0 && event->token == 0 &&
           event->frame == 0 && event->bitmap == 0 && event->pages == 0 && event->device == 0 &&
           event->device_identity == 0 && event->device_expected_identity == 0 &&
           event->device_before == 0 && event->device_expected == 0 && event->device_after == 0 &&
           !event->device_attempted && event->device_status == 0 && event->message.type_id == 0 &&
           event->message.caller == 0 && event->message.arg0 == 0 && event->message.arg1 == 0 &&
           event->message.arg2 == 0 && event->message.sender == 0 &&
           event->message.caller_task == 0 && !event->message.from_call &&
           event->message.request_id == 0;
}

static int64_t epoch_journal_accept(EpochJournal *journal, const KernelEpochEvent *event) {
    if (event->kind == EPOCH_JOURNAL_SESSION_KIND) {
        if (journal->sessions == UINT64_MAX) return EPOCH_JOURNAL_FULL;
        if (!epoch_journal_session_valid(event, journal->sessions + 1)) return EPOCH_JOURNAL_CORRUPT;
        journal->sessions = event->revision;
        journal->next_event = 1;
        return EPOCH_JOURNAL_OK;
    }
    if (!journal->sessions || !journal->next_event) return EPOCH_JOURNAL_CORRUPT;
    if (event->epoch != journal->next_event) return EPOCH_JOURNAL_CORRUPT;
    journal->next_event = journal->next_event == UINT64_MAX ? 0 : journal->next_event + 1;
    return EPOCH_JOURNAL_OK;
}

static bool epoch_journal_commit_valid(const uint8_t *sector) {
    if (!sector) return false;
    for (size_t i = 0; i < 8; ++i)
        if (sector[i] != epoch_commit_magic[i]) return false;
    if (epoch_journal_get32(sector + 8) != EPOCH_JOURNAL_VERSION ||
        epoch_journal_get32(sector + 12) != 0 || !epoch_journal_get64(sector + 16) ||
        epoch_journal_get64(sector + EPOCH_JOURNAL_CHECKSUM_OFFSET) !=
            epoch_journal_checksum(sector))
        return false;
    for (size_t i = 48; i < EPOCH_JOURNAL_CHECKSUM_OFFSET; ++i)
        if (sector[i]) return false;
    uint64_t next = epoch_journal_get64(sector + EPOCH_JOURNAL_COMMIT_NEXT);
    uint64_t sessions = epoch_journal_get64(sector + EPOCH_JOURNAL_COMMIT_SESSIONS);
    return next && ((sessions && next >= 2) || (!sessions && next == 1 &&
                    epoch_journal_get64(sector + EPOCH_JOURNAL_COMMIT_EVENT) == 1));
}

static void epoch_journal_commit_encode(uint8_t *sector, uint64_t generation,
                                        uint64_t next_sequence, uint64_t sessions,
                                        uint64_t next_event) {
    for (size_t i = 0; i < EPOCH_JOURNAL_SECTOR_SIZE; ++i) sector[i] = 0;
    epoch_journal_copy(sector, epoch_commit_magic, 8);
    epoch_journal_put32(sector + 8, EPOCH_JOURNAL_VERSION);
    epoch_journal_put64(sector + 16, generation);
    epoch_journal_put64(sector + EPOCH_JOURNAL_COMMIT_NEXT, next_sequence);
    epoch_journal_put64(sector + EPOCH_JOURNAL_COMMIT_SESSIONS, sessions);
    epoch_journal_put64(sector + EPOCH_JOURNAL_COMMIT_EVENT, next_event);
    epoch_journal_put64(sector + EPOCH_JOURNAL_CHECKSUM_OFFSET, epoch_journal_checksum(sector));
}

static int64_t epoch_journal_open_committed(EpochJournal *journal) {
    uint8_t metadata[EPOCH_JOURNAL_META_SECTORS][EPOCH_JOURNAL_SECTOR_SIZE];
    bool valid[EPOCH_JOURNAL_META_SECTORS];
    for (uint64_t slot = 0; slot < EPOCH_JOURNAL_META_SECTORS; ++slot) {
        int64_t status = journal->read(journal->base + journal->sectors + slot, metadata[slot]);
        if (status != EPOCH_JOURNAL_OK) return status;
        valid[slot] = epoch_journal_commit_valid(metadata[slot]) &&
                      epoch_journal_get64(metadata[slot] + EPOCH_JOURNAL_COMMIT_NEXT) <=
                          journal->sectors + 1;
    }
    int selected = -1;
    for (uint64_t slot = 0; slot < EPOCH_JOURNAL_META_SECTORS; ++slot)
        if (valid[slot] && (selected < 0 || epoch_journal_get64(metadata[slot] + 16) >
                                             epoch_journal_get64(metadata[selected] + 16)))
            selected = (int)slot;
    if (selected < 0) return EPOCH_JOURNAL_MISSING;
    journal->commit_generation = epoch_journal_get64(metadata[selected] + 16);
    journal->next_sequence = epoch_journal_get64(metadata[selected] + EPOCH_JOURNAL_COMMIT_NEXT);
    journal->sessions = epoch_journal_get64(metadata[selected] + EPOCH_JOURNAL_COMMIT_SESSIONS);
    journal->next_event = epoch_journal_get64(metadata[selected] + EPOCH_JOURNAL_COMMIT_EVENT);
    while (journal->next_sequence <= journal->sectors) {
        uint8_t sector[EPOCH_JOURNAL_SECTOR_SIZE];
        int64_t status = journal->read(journal->base + journal->next_sequence - 1, sector);
        if (status != EPOCH_JOURNAL_OK) return status;
        if (epoch_journal_blank(sector)) {
            if (journal->next_sequence < journal->sectors) {
                status = journal->read(journal->base + journal->next_sequence, sector);
                if (status != EPOCH_JOURNAL_OK) return status;
                if (!epoch_journal_blank(sector)) return EPOCH_JOURNAL_CORRUPT;
            }
            break;
        }
        if (!epoch_journal_sector_valid(sector, journal->next_sequence)) return EPOCH_JOURNAL_CORRUPT;
        KernelEpochEvent event;
        epoch_journal_decode(sector, &event);
        status = epoch_journal_accept(journal, &event);
        if (status != EPOCH_JOURNAL_OK) return status;
        ++journal->next_sequence;
    }
    journal->uncertain = false;
    return EPOCH_JOURNAL_OK;
}

static int64_t epoch_journal_open_scan(EpochJournal *journal) {
    bool blank_seen = false;
    uint64_t first_blank = 0;
    for (uint64_t index = 0; index < journal->sectors; ++index) {
        uint8_t sector[EPOCH_JOURNAL_SECTOR_SIZE];
        int64_t status = journal->read(journal->base + index, sector);
        if (status != EPOCH_JOURNAL_OK) return status;
        if (epoch_journal_blank(sector)) {
            blank_seen = true;
            if (!first_blank) first_blank = index + 1;
            continue;
        }
        if (blank_seen || !epoch_journal_sector_valid(sector, index + 1))
            return EPOCH_JOURNAL_CORRUPT;
        KernelEpochEvent event;
        epoch_journal_decode(sector, &event);
        status = epoch_journal_accept(journal, &event);
        if (status != EPOCH_JOURNAL_OK) return status;
    }
    journal->next_sequence = first_blank ? first_blank : journal->sectors + 1;
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_journal_open(EpochJournal *journal, uint64_t base, uint64_t sectors,
                           EpochJournalRead read, EpochJournalWrite write, EpochJournalSync sync) {
    if (!journal || !sectors || sectors > UINT64_MAX - EPOCH_JOURNAL_META_SECTORS - 1 ||
        base > UINT64_MAX - sectors - EPOCH_JOURNAL_META_SECTORS || !read || !write || !sync)
        return EPOCH_JOURNAL_INVALID;
    epoch_journal_init(journal);
    journal->read = read;
    journal->write = write;
    journal->sync = sync;
    journal->base = base;
    journal->sectors = sectors;
    int64_t status = epoch_journal_open_committed(journal);
    if (status != EPOCH_JOURNAL_MISSING) return status;
    epoch_journal_init(journal);
    journal->read = read;
    journal->write = write;
    journal->sync = sync;
    journal->base = base;
    journal->sectors = sectors;
    return epoch_journal_open_scan(journal);
}

int64_t epoch_journal_commit(EpochJournal *journal) {
    if (!journal || !journal->read || !journal->write || !journal->sync ||
        !journal->next_sequence || journal->next_sequence > journal->sectors + 1)
        return EPOCH_JOURNAL_INVALID;
    if (journal->commit_generation == UINT64_MAX) return EPOCH_JOURNAL_FULL;
    uint64_t generation = journal->commit_generation + 1;
    uint64_t lba = journal->base + journal->sectors + generation % EPOCH_JOURNAL_META_SECTORS;
    uint8_t sector[EPOCH_JOURNAL_SECTOR_SIZE];
    int64_t status = journal->sync();
    if (status != EPOCH_JOURNAL_OK) {
        journal->uncertain = true;
        return status;
    }
    epoch_journal_commit_encode(sector, generation, journal->next_sequence,
                                journal->sessions, journal->next_event);
    status = journal->write(lba, sector);
    if (status == EPOCH_JOURNAL_OK) status = journal->sync();
    if (status != EPOCH_JOURNAL_OK) {
        journal->uncertain = true;
        return status;
    }
    status = journal->read(lba, sector);
    if (status != EPOCH_JOURNAL_OK) {
        journal->uncertain = true;
        return status;
    }
    if (!epoch_journal_commit_valid(sector) || epoch_journal_get64(sector + 16) != generation ||
        epoch_journal_get64(sector + EPOCH_JOURNAL_COMMIT_NEXT) != journal->next_sequence ||
        epoch_journal_get64(sector + EPOCH_JOURNAL_COMMIT_SESSIONS) != journal->sessions ||
        epoch_journal_get64(sector + EPOCH_JOURNAL_COMMIT_EVENT) != journal->next_event) {
        journal->uncertain = true;
        return EPOCH_JOURNAL_CORRUPT;
    }
    journal->commit_generation = generation;
    journal->uncertain = false;
    return EPOCH_JOURNAL_OK;
}

static int64_t epoch_journal_write(EpochJournal *journal, const KernelEpochEvent *event,
                                   uint64_t sessions, uint64_t next_event, bool commit) {
    if (journal->next_sequence > journal->sectors) return EPOCH_JOURNAL_FULL;
    uint8_t expected[EPOCH_JOURNAL_SECTOR_SIZE];
    int64_t status = epoch_journal_encode(expected, journal->next_sequence, event);
    if (status != EPOCH_JOURNAL_OK) return status;
    uint64_t lba = journal->base + journal->next_sequence - 1;
    bool write_data = true;
    if (journal->uncertain) {
        uint8_t current[EPOCH_JOURNAL_SECTOR_SIZE];
        status = journal->read(lba, current);
        if (status != EPOCH_JOURNAL_OK) {
            journal->uncertain = true;
            return status;
        }
        if (!epoch_journal_blank(current)) {
            for (size_t i = 0; i < EPOCH_JOURNAL_SECTOR_SIZE; ++i)
                if (current[i] != expected[i]) {
                    journal->uncertain = true;
                    return EPOCH_JOURNAL_CORRUPT;
                }
            write_data = false;
        }
    }
    if (write_data) status = journal->write(lba, expected);
    if (status != EPOCH_JOURNAL_OK) {
        journal->uncertain = true;
        return status;
    }
    if (commit) status = journal->sync();
    if (status == EPOCH_JOURNAL_OK) status = journal->read(lba, expected);
    if (status != EPOCH_JOURNAL_OK) {
        journal->uncertain = true;
        return status;
    }
    if (!epoch_journal_sector_valid(expected, journal->next_sequence)) {
        journal->uncertain = true;
        return EPOCH_JOURNAL_CORRUPT;
    }
    if (commit) {
        uint64_t saved_sequence = journal->next_sequence;
        uint64_t saved_sessions = journal->sessions, saved_event = journal->next_event;
        journal->next_sequence = saved_sequence + 1;
        journal->sessions = sessions;
        journal->next_event = next_event;
        status = epoch_journal_commit(journal);
        if (status != EPOCH_JOURNAL_OK) {
            journal->next_sequence = saved_sequence;
            journal->sessions = saved_sessions;
            journal->next_event = saved_event;
            journal->uncertain = true;
            return status;
        }
        journal->next_sequence = saved_sequence;
        journal->sessions = saved_sessions;
        journal->next_event = saved_event;
    }
    ++journal->next_sequence;
    journal->uncertain = false;
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_journal_begin_session(EpochJournal *journal) {
    if (!journal || !journal->read || !journal->write || !journal->sync ||
        !journal->next_sequence || journal->sessions == UINT64_MAX)
        return EPOCH_JOURNAL_INVALID;
    KernelEpochEvent event = {0};
    event.kind = EPOCH_JOURNAL_SESSION_KIND;
    event.revision = journal->sessions + 1;
    int64_t status = epoch_journal_write(journal, &event, event.revision, 1, true);
    if (status != EPOCH_JOURNAL_OK) return status;
    journal->sessions = event.revision;
    journal->next_event = 1;
    return EPOCH_JOURNAL_OK;
}

static int64_t epoch_journal_append_mode(EpochJournal *journal, const KernelEpochEvent *event,
                                         bool commit) {
    if (!journal || !event || !journal->read || !journal->write || !journal->sync ||
        !journal->next_sequence)
        return EPOCH_JOURNAL_INVALID;
    if (event->kind == EPOCH_JOURNAL_SESSION_KIND || !journal->sessions ||
        event->epoch != journal->next_event)
        return EPOCH_JOURNAL_DENIED;
    if (!journal->next_event) return EPOCH_JOURNAL_FULL;
    uint64_t next_event = journal->next_event == UINT64_MAX ? 0 : journal->next_event + 1;
    int64_t status = epoch_journal_write(journal, event, journal->sessions, next_event, commit);
    if (status != EPOCH_JOURNAL_OK) return status;
    journal->next_event = next_event;
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_journal_append(EpochJournal *journal, const KernelEpochEvent *event) {
    return epoch_journal_append_mode(journal, event, true);
}

int64_t epoch_journal_append_deferred(EpochJournal *journal, const KernelEpochEvent *event) {
    return epoch_journal_append_mode(journal, event, false);
}

int64_t epoch_journal_span_io(EpochJournal *journal, EpochJournalReadSpan read_span,
                              EpochJournalWriteSpan write_span) {
    if (!journal || !read_span || !write_span) return EPOCH_JOURNAL_INVALID;
    journal->read_span = read_span;
    journal->write_span = write_span;
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_journal_append_span_deferred(EpochJournal *journal, const KernelEpochEvent *events,
                                           uint64_t count) {
    if (!journal || !events || !count || count > EPOCH_JOURNAL_SPAN_SECTORS ||
        !journal->read || !journal->write || !journal->sync || !journal->next_sequence)
        return EPOCH_JOURNAL_INVALID;
    if (journal->next_sequence > journal->sectors ||
        count > journal->sectors - journal->next_sequence + 1)
        return EPOCH_JOURNAL_FULL;
    uint64_t next_event = journal->next_event;
    uint8_t encoded[EPOCH_JOURNAL_SPAN_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE];
    for (uint64_t index = 0; index < count; ++index) {
        if (events[index].kind == EPOCH_JOURNAL_SESSION_KIND || !journal->sessions ||
            events[index].epoch != next_event)
            return EPOCH_JOURNAL_DENIED;
        if (!next_event) return EPOCH_JOURNAL_FULL;
        int64_t status = epoch_journal_encode(encoded + index * EPOCH_JOURNAL_SECTOR_SIZE,
                                              journal->next_sequence + index, events + index);
        if (status != EPOCH_JOURNAL_OK) return status;
        next_event = next_event == UINT64_MAX ? 0 : next_event + 1;
    }
    if (!journal->read_span || !journal->write_span) {
        for (uint64_t index = 0; index < count; ++index) {
            int64_t status = epoch_journal_append_deferred(journal, events + index);
            if (status != EPOCH_JOURNAL_OK) return status;
        }
        return EPOCH_JOURNAL_OK;
    }
    uint64_t lba = journal->base + journal->next_sequence - 1;
    bool write_data = true;
    if (journal->uncertain) {
        uint8_t current[EPOCH_JOURNAL_SPAN_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE];
        if (journal->read_span(lba, count, current) != EPOCH_JOURNAL_OK) {
            journal->uncertain = true;
            return EPOCH_JOURNAL_IO;
        }
        write_data = false;
        for (uint64_t index = 0; index < count; ++index) {
            const uint8_t *sector = current + index * EPOCH_JOURNAL_SECTOR_SIZE;
            if (epoch_journal_blank(sector)) {
                write_data = true;
                continue;
            }
            for (size_t byte = 0; byte < EPOCH_JOURNAL_SECTOR_SIZE; ++byte)
                if (sector[byte] != encoded[index * EPOCH_JOURNAL_SECTOR_SIZE + byte]) {
                    journal->uncertain = true;
                    return EPOCH_JOURNAL_CORRUPT;
                }
        }
    }
    if (write_data) {
        int64_t status = journal->write_span(lba, count, encoded);
        if (status == EPOCH_JOURNAL_RETRY) return status;
        if (status != EPOCH_JOURNAL_OK) {
            journal->uncertain = true;
            return EPOCH_JOURNAL_IO;
        }
    }
    {
        int64_t status = journal->read_span(lba, count, encoded);
        if (status == EPOCH_JOURNAL_RETRY) return status;
        if (status != EPOCH_JOURNAL_OK) {
            journal->uncertain = true;
            return EPOCH_JOURNAL_IO;
        }
    }
    for (uint64_t index = 0; index < count; ++index)
        if (!epoch_journal_sector_valid(encoded + index * EPOCH_JOURNAL_SECTOR_SIZE,
                                        journal->next_sequence + index)) {
            journal->uncertain = true;
            return EPOCH_JOURNAL_CORRUPT;
        }
    journal->next_sequence += count;
    journal->next_event = next_event;
    journal->uncertain = false;
    return EPOCH_JOURNAL_OK;
}

int64_t epoch_journal_read(EpochJournal *journal, uint64_t sequence, KernelEpochEvent *out) {
    if (!journal || !out || !sequence || !journal->read) return EPOCH_JOURNAL_INVALID;
    if (sequence >= journal->next_sequence) return EPOCH_JOURNAL_MISSING;
    uint8_t sector[EPOCH_JOURNAL_SECTOR_SIZE];
    int64_t status = journal->read(journal->base + sequence - 1, sector);
    if (status != EPOCH_JOURNAL_OK) return status;
    if (!epoch_journal_sector_valid(sector, sequence)) return EPOCH_JOURNAL_CORRUPT;
    epoch_journal_decode(sector, out);
    return EPOCH_JOURNAL_OK;
}

static int64_t epoch_journal_replay_accept(EpochJournal *model, const KernelEpochEvent *event,
                                            EpochJournalReplay replay, void *context) {
    int64_t status = epoch_journal_accept(model, event);
    if (status != EPOCH_JOURNAL_OK) return status;
    return replay(event, context);
}

int64_t epoch_journal_replay(EpochJournal *journal, EpochJournalReplay replay, void *context) {
    if (!journal || !replay) return EPOCH_JOURNAL_INVALID;
    EpochJournal model;
    epoch_journal_init(&model);
    uint64_t sequence = 1;
    while (sequence < journal->next_sequence) {
        uint64_t first = sequence;
        uint64_t count = journal->next_sequence - first;
        if (count > EPOCH_JOURNAL_SPAN_SECTORS) count = EPOCH_JOURNAL_SPAN_SECTORS;
        uint8_t sectors[EPOCH_JOURNAL_SPAN_SECTORS * EPOCH_JOURNAL_SECTOR_SIZE];
        if (journal->read_span) {
            int64_t status = journal->read_span(journal->base + first - 1, count, sectors);
            if (status != EPOCH_JOURNAL_OK) return status;
        }
        for (uint64_t index = 0; index < count; ++index) {
            KernelEpochEvent event;
            if (journal->read_span) {
                const uint8_t *sector = sectors + index * EPOCH_JOURNAL_SECTOR_SIZE;
                if (!epoch_journal_sector_valid(sector, first + index)) return EPOCH_JOURNAL_CORRUPT;
                epoch_journal_decode(sector, &event);
            } else {
                int64_t status = epoch_journal_read(journal, first + index, &event);
                if (status != EPOCH_JOURNAL_OK) return status;
            }
            int64_t status = epoch_journal_replay_accept(&model, &event, replay, context);
            if (status != EPOCH_JOURNAL_OK) return status;
        }
        sequence += count;
    }
    if (model.sessions != journal->sessions || model.next_event != journal->next_event)
        return EPOCH_JOURNAL_CORRUPT;
    return EPOCH_JOURNAL_OK;
}
