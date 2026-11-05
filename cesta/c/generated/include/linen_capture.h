
#ifndef SEXOS_LINEN_CAPTURE_H
#define SEXOS_LINEN_CAPTURE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "linen_capture_layout.h"

typedef struct LinenCaptureRow {
    bool present;
    uint64_t id;
    uint8_t kind, disk, len, name[LINEN_CAPTURE_NAME_CAPACITY];
} LinenCaptureRow;

typedef struct LinenCaptureTable {
    LinenCaptureRow rows[LINEN_CAPTURE_SLOTS];
} LinenCaptureTable;

typedef struct LinenCaptureFrame {
    uint8_t bytes[LINEN_CAPTURE_MAX_BYTES];
    size_t len;
} LinenCaptureFrame;

typedef struct LinenCaptureServer {
    LinenCaptureFrame frame;
    bool active;
    uint32_t owner, task;
    uint64_t job, retired, revision;
} LinenCaptureServer;

enum {
    LINEN_CAPTURE_FULL = 1,
    LINEN_CAPTURE_STALE = 2,
    LINEN_CAPTURE_EXHAUSTED = 3,
    LINEN_CAPTURE_UNSUPPORTED = 4,
    LINEN_CAPTURE_DENIED = 5,
    LINEN_CAPTURE_MALFORMED = 6,
    LINEN_CAPTURE_RANGE = 7
};

bool linen_capture_encode(const LinenCaptureTable *table, uint64_t revision,
                          LinenCaptureFrame *out);
bool linen_capture_decode(const LinenCaptureFrame *frame, LinenCaptureTable *out);
uint64_t linen_capture_error(uint8_t error);
LinenCaptureServer linen_capture_new(uint32_t owner, uint32_t task);
uint64_t linen_capture_begin(LinenCaptureServer *state, uint32_t caller, uint32_t task,
                             uint64_t job, uint64_t version, uint64_t reserved,
                             const LinenCaptureTable *table);
uint64_t linen_capture_read(const LinenCaptureServer *state, uint32_t caller, uint32_t task,
                            uint64_t job, uint64_t offset, uint64_t reserved);
uint64_t linen_capture_end(LinenCaptureServer *state, uint32_t caller, uint32_t task, uint64_t job,
                           uint64_t reserved0, uint64_t reserved1);
bool linen_capture_append(LinenCaptureFrame *frame, uint64_t word);
#endif
