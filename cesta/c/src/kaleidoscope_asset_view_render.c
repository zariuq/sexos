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
#include "kaleidoscope_asset_view.h"

static uint32_t asset_render_scale(uint32_t width) {
    return width >= 256 ? 2 : 1;
}

static size_t asset_render_columns(uint32_t width) {
    uint32_t inset = 16, advance = 6 * asset_render_scale(width);
    if (width <= inset)
        return 0;
    size_t columns = (width - inset) / advance;
    if (!columns)
        return 0;
    columns--;
    return columns < 79 ? columns : 79;
}

static const uint8_t asset_default_title[] = "QOI / controlled HTTPS lab root",
                     asset_pending[] = "Dimensions pending", asset_none[] = "No retained image",
                     asset_no_fit[] = "Image does not fit",
                     asset_controls_wide[] = "F Fit  A 1:1  G fetch  Q clear",
                     asset_controls_fit[] = "F Fit  A 1:1",
                     asset_controls_clear[] = "G fetch  Q clear";

static void asset_put(KaleidoscopeAssetFrame *frame, size_t row, const uint8_t *text,
                      size_t length) {
    if (row >= frame->rows)
        return;
    size_t at = row * (frame->cols + 1), count = length < frame->cols ? length : frame->cols;
    for (size_t i = 0; i < count; i++)
        frame->bytes[at + i] = text[i];
}

static void asset_put_label(KaleidoscopeAssetFrame *frame, uint8_t status) {
    static const uint8_t a[] = "Loading image", b[] = "Decoding image", c[] = "Publishing image",
                         d[] = "Image ready", e[] = "Image unavailable", f[] = "Invalid image",
                         g[] = "Clearing image", h[] = "Image cleared", i[] = "Waiting for receipt",
                         j[] = "Image does not fit";
    if (status == 0)
        asset_put(frame, 1, a, sizeof(a) - 1);
    else if (status == 1)
        asset_put(frame, 1, b, sizeof(b) - 1);
    else if (status == 2)
        asset_put(frame, 1, c, sizeof(c) - 1);
    else if (status == 3)
        asset_put(frame, 1, d, sizeof(d) - 1);
    else if (status == 4)
        asset_put(frame, 1, e, sizeof(e) - 1);
    else if (status == 6)
        asset_put(frame, 1, g, sizeof(g) - 1);
    else if (status == 7)
        asset_put(frame, 1, h, sizeof(h) - 1);
    else if (status == 8)
        asset_put(frame, 1, i, sizeof(i) - 1);
    else if (status == 9)
        asset_put(frame, 1, j, sizeof(j) - 1);
    else
        asset_put(frame, 1, f, sizeof(f) - 1);
}

static size_t asset_decimal(uint8_t out[32], size_t at, size_t value) {
    if (value >= 10)
        out[at++] = (uint8_t)('0' + value / 10);
    out[at++] = (uint8_t)('0' + value % 10);
    return at;
}

static size_t asset_append(uint8_t out[32], size_t at, const char *text) {
    for (size_t i = 0; text[i]; i++)
        out[at++] = (uint8_t)text[i];
    return at;
}

void kaleidoscope_asset_render(uint32_t surface_width, uint32_t surface_height, uint8_t status,
                               uint8_t mode, size_t width, size_t height, const uint8_t *title,
                               size_t title_len, KaleidoscopeAssetFrame *frame) {
    if (!frame) {
        return;
    }
    *frame = (KaleidoscopeAssetFrame){0};
    uint32_t factor = asset_render_scale(surface_width);
    size_t columns = asset_render_columns(surface_width);
    uint32_t top = 36;
    size_t rows = columns && surface_height >= top + 7 * factor
                      ? 1 + (surface_height - top - 7 * factor) / (9 * factor)
                      : 0;
    if (rows > 24)
        rows = 24;
    if (columns && rows > 512 / (columns + 1))
        rows = 512 / (columns + 1);
    frame->rows = rows;
    frame->cols = columns;
    if (!rows)
        return;
    frame->len = rows * (columns + 1) - 1;
    for (size_t row = 0; row < rows; row++) {
        size_t at = row * (columns + 1);
        for (size_t i = 0; i < columns; i++)
            frame->bytes[at + i] = ' ';
        if (row + 1 < rows)
            frame->bytes[at + columns] = '\n';
    }
    if (title)
        asset_put(frame, 0, title, title_len);
    else
        asset_put(frame, 0, asset_default_title, sizeof(asset_default_title) - 1);
    asset_put_label(frame, status);
    if (!width || !height) {
        if (status == KALEIDOSCOPE_ASSET_LOADING || status == KALEIDOSCOPE_ASSET_DECODING)
            asset_put(frame, 2, asset_pending, sizeof(asset_pending) - 1);
        else
            asset_put(frame, 2, asset_none, sizeof(asset_none) - 1);
    } else {
        KaleidoscopeBitmapGeometry geometry = {0};
        if (kaleidoscope_asset_geometry(surface_width, surface_height, width, height, mode,
                                        &geometry)) {
            uint8_t metrics[32] = {0};
            size_t n = 0;
            n = asset_append(metrics, n, mode == KALEIDOSCOPE_ASSET_FIT ? "Fit " : "1:1 ");
            n = asset_decimal(metrics, n, width);
            metrics[n++] = 'x';
            n = asset_decimal(metrics, n, height);
            n = asset_append(metrics, n, " / ");
            n = asset_decimal(metrics, n, geometry.scale);
            metrics[n++] = 'x';
            asset_put(frame, 2, metrics, n);
        } else
            asset_put(frame, 2, asset_no_fit, sizeof(asset_no_fit) - 1);
    }
    if (kaleidoscope_asset_chrome_rows(surface_width) == 4)
        asset_put(frame, 3, asset_controls_wide, sizeof(asset_controls_wide) - 1);
    else {
        asset_put(frame, 3, asset_controls_fit, sizeof(asset_controls_fit) - 1);
        asset_put(frame, 4, asset_controls_clear, sizeof(asset_controls_clear) - 1);
    }
}


