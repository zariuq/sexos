/* SPDX-License-Identifier: GPL-3.0-only */
/* Program-image interface adapted from upstream 845de950 (2026-11-09).
   Validation is separate from mapping and relocation effects. */
#ifndef SEXOS_PROGRAM_IMAGE_H
#define SEXOS_PROGRAM_IMAGE_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "program_image_bytes.h"

enum {
    PROGRAM_IMAGE_SEGMENTS = 8,
    PROGRAM_IMAGE_OK = 0, PROGRAM_IMAGE_HEADER = 1, PROGRAM_IMAGE_LAYOUT = 2,
    PROGRAM_IMAGE_PERMISSIONS = 3, PROGRAM_IMAGE_SPAN = 4,
    PROGRAM_IMAGE_ENTRY = 5, PROGRAM_IMAGE_RELOCATION = 6
};
typedef struct ProgramSegment {
    uint64_t offset, file, window, memory;
    bool writable, executable;
} ProgramSegment;
typedef struct ProgramImage {
    cesta_read_bytes source;
    uint64_t entry, bias, rela, rela_count, stack, window;
    size_t count;
    uint32_t error;
    ProgramSegment segments[PROGRAM_IMAGE_SEGMENTS];
} ProgramImage;
typedef struct ProgramRelocation {
    uint64_t offset, value;
    bool valid;
} ProgramRelocation;

ProgramImage program_image_parse(cesta_read_bytes source, uint64_t window);
ProgramRelocation program_image_relocation(const ProgramImage *image, uint64_t index, uint64_t base);
bool program_image_kernel_allowed(const ProgramImage *image);
#endif
