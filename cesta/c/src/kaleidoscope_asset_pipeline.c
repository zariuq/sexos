#include "kaleidoscope_asset_pipeline.h"
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

static void asset_source_erase(KaleidoscopeQoiSource *source) {
    for (size_t i = 0; i < KALEIDOSCOPE_QOI_MAX_INPUT; i++)
        source->bytes[i] = 0;
    source->len = 0;
}

static void asset_scratch_erase(KaleidoscopePngScratch *scratch) {
    for (size_t i = 0; i < KALEIDOSCOPE_PNG_SCRATCH_BYTES; i++)
        scratch->bytes[i] = 0;
}

static uint64_t asset_wire_hash(const KaleidoscopeQoiSource *source) {
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (size_t i = 0; i < source->len; i++) {
        hash ^= source->bytes[i];
        hash *= UINT64_C(0x100000001b3);
    }
    return hash;
}

void kaleidoscope_asset_image_init(KaleidoscopeAssetImage *image) {
    if (image)
        *image = (KaleidoscopeAssetImage){0};
}

void kaleidoscope_asset_image_clear(KaleidoscopeAssetImage *image) {
    if (!image)
        return;
    if (image->format == KALEIDOSCOPE_ASSET_QOI)
        kaleidoscope_qoi_cancel(&image->qoi, &image->pixels);
    for (size_t i = 0; i < KALEIDOSCOPE_QOI_MAX_PIXELS; i++)
        image->pixels.values[i] = 0;
    asset_source_erase(&image->source);
    asset_scratch_erase(&image->scratch);
    *image = (KaleidoscopeAssetImage){0};
}

bool kaleidoscope_asset_image_begin(KaleidoscopeAssetImage *image,
                                    const KaleidoscopeQoiSource *source, uint32_t ticket) {
    if (!image || !source || !ticket || source->len > KALEIDOSCOPE_QOI_MAX_INPUT) {
        return false;
    }
    kaleidoscope_asset_image_clear(image);
    image->source.len = source->len;
    for (size_t i = 0; i < source->len; i++)
        image->source.bytes[i] = source->bytes[i];
    image->ticket = ticket;
    image->wire_bytes = source->len;
    image->wire_hash = asset_wire_hash(source);
    if (kaleidoscope_png_is_png(&image->source)) {
        KaleidoscopePngResult result =
            kaleidoscope_png_decode(&image->source, &image->scratch, &image->pixels);
        asset_scratch_erase(&image->scratch);
        asset_source_erase(&image->source);
        if (result.error) {
            kaleidoscope_asset_image_clear(image);
            return false;
        }
        image->format = KALEIDOSCOPE_ASSET_PNG;
        image->width = result.width;
        image->height = result.height;
        image->decoded = true;
        return true;
    }
    if (kaleidoscope_qoi_init(&image->source, &image->qoi)) {
        kaleidoscope_asset_image_clear(image);
        return false;
    }
    image->format = KALEIDOSCOPE_ASSET_QOI;
    image->width = image->qoi.width;
    image->height = image->qoi.height;
    return true;
}

bool kaleidoscope_asset_image_advance(KaleidoscopeAssetImage *image, bool *became_ready) {
    if (!image || !became_ready)
        return false;
    *became_ready = false;
    if (image->ready)
        return true;
    if (image->decoded) {
        image->decoded = false;
        image->ready = true;
        *became_ready = true;
        return true;
    }
    if (image->format != KALEIDOSCOPE_ASSET_QOI)
        return true;
    KaleidoscopeQoiStep step =
        kaleidoscope_qoi_advance(&image->qoi, &image->source, &image->pixels);
    if (step.error) {
        kaleidoscope_asset_image_clear(image);
        return false;
    }
    if (step.state == KALEIDOSCOPE_QOI_READY) {
        asset_source_erase(&image->source);
        image->ready = true;
        *became_ready = true;
    }
    return true;
}

bool kaleidoscope_asset_image_pending(const KaleidoscopeAssetImage *image) {
    return image && !image->ready && (image->decoded || image->format == KALEIDOSCOPE_ASSET_QOI);
}

size_t kaleidoscope_asset_image_pixel_count(const KaleidoscopeAssetImage *image) {
    return image && image->ready ? image->width * image->height : 0;
}


