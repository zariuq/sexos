
#ifndef KALEIDOSCOPE_ASSET_PIPELINE_H
#define KALEIDOSCOPE_ASSET_PIPELINE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kaleidoscope_qoi.h"
#include "kaleidoscope_png.h"

enum {
    KALEIDOSCOPE_ASSET_NONE = 0,
    KALEIDOSCOPE_ASSET_QOI = 1,
    KALEIDOSCOPE_ASSET_PNG = 2
};

typedef struct KaleidoscopeAssetImage {
    KaleidoscopeQoiSource source;
    KaleidoscopeQoiPixels pixels;
    KaleidoscopePngScratch scratch;
    KaleidoscopeQoiDecoder qoi;
    size_t width, height, wire_bytes;
    uint32_t ticket;
    uint64_t wire_hash;
    uint8_t format;
    bool ready, decoded;
} KaleidoscopeAssetImage;

void kaleidoscope_asset_image_init(KaleidoscopeAssetImage *image);
bool kaleidoscope_asset_image_begin(KaleidoscopeAssetImage *image,
                                    const KaleidoscopeQoiSource *source, uint32_t ticket);
bool kaleidoscope_asset_image_advance(KaleidoscopeAssetImage *image, bool *became_ready);
bool kaleidoscope_asset_image_pending(const KaleidoscopeAssetImage *image);
size_t kaleidoscope_asset_image_pixel_count(const KaleidoscopeAssetImage *image);
void kaleidoscope_asset_image_clear(KaleidoscopeAssetImage *image);
#endif
