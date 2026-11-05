
#ifndef KALEIDOSCOPE_PNG_H
#define KALEIDOSCOPE_PNG_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kaleidoscope_qoi.h"
#define KALEIDOSCOPE_PNG_SCRATCH_BYTES ((size_t)18816)

enum {
    KALEIDOSCOPE_PNG_OK = 0,
    KALEIDOSCOPE_PNG_SIGNATURE = 1,
    KALEIDOSCOPE_PNG_INPUT_TOO_LARGE = 2,
    KALEIDOSCOPE_PNG_TRUNCATED = 3,
    KALEIDOSCOPE_PNG_CRC = 4,
    KALEIDOSCOPE_PNG_HEADER = 5,
    KALEIDOSCOPE_PNG_UNSUPPORTED = 6,
    KALEIDOSCOPE_PNG_PALETTE = 7,
    KALEIDOSCOPE_PNG_ZLIB = 8,
    KALEIDOSCOPE_PNG_DEFLATE = 9,
    KALEIDOSCOPE_PNG_ADLER = 10,
    KALEIDOSCOPE_PNG_SIZE = 11,
    KALEIDOSCOPE_PNG_FILTER = 12
};

typedef struct KaleidoscopePngScratch {
    uint8_t bytes[KALEIDOSCOPE_PNG_SCRATCH_BYTES];
} KaleidoscopePngScratch;

typedef struct KaleidoscopePngResult {
    uint8_t error;
    size_t width, height;
} KaleidoscopePngResult;

bool kaleidoscope_png_is_png(const KaleidoscopeQoiSource *source);
KaleidoscopePngResult kaleidoscope_png_decode(const KaleidoscopeQoiSource *source,
                                              KaleidoscopePngScratch *scratch,
                                              KaleidoscopeQoiPixels *pixels);
#endif
