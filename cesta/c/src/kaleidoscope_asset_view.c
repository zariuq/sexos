#include "kaleidoscope_asset_view.h"
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

static uint32_t asset_scale(uint32_t width) {
    return width >= 256 ? 2 : 1;
}

static size_t asset_columns(uint32_t width) {
    uint32_t inset = 16, advance = 6 * asset_scale(width);
    if (width <= inset)
        return 0;
    size_t columns = (width - inset) / advance;
    if (!columns)
        return 0;
    columns--;
    return columns < 79 ? columns : 79;
}

size_t kaleidoscope_asset_chrome_rows(uint32_t width) {
    return asset_columns(width) >= 30 ? 4 : 5;
}

bool kaleidoscope_asset_geometry(uint32_t surface_width, uint32_t surface_height, size_t width,
                                 size_t height, uint8_t mode,
                                 KaleidoscopeBitmapGeometry *geometry) {
    if (!geometry || width < 1 || width > 48 || height < 1 || height > 48 ||
        !asset_columns(surface_width) ||
        (mode != KALEIDOSCOPE_ASSET_FIT && mode != KALEIDOSCOPE_ASSET_ACTUAL)) {
        return false;
    }
    uint32_t scale = asset_scale(surface_width),
             top = 28 + 8 + (uint32_t)kaleidoscope_asset_chrome_rows(surface_width) * 9 * scale + 8;
    if (surface_width < 16 || surface_height < top + 8)
        return false;
    uint32_t available_width = surface_width - 16, available_height = surface_height - top - 8,
             factor = 1;
    if (mode == KALEIDOSCOPE_ASSET_FIT) {
        uint32_t a = available_width / (uint32_t)width, b = available_height / (uint32_t)height;
        factor = a < b ? a : b;
        if (factor > 4)
            factor = 4;
    }
    if (!factor)
        return false;
    uint32_t image_width = (uint32_t)width * factor, image_height = (uint32_t)height * factor;
    if (image_width > available_width || image_height > available_height)
        return false;
    uint32_t x = 8 + (available_width - image_width) / 2,
             y = top + (available_height - image_height) / 2;
    if (x > UINT16_MAX || y > UINT16_MAX)
        return false;
    *geometry = (KaleidoscopeBitmapGeometry){(uint16_t)x, (uint16_t)y, (uint8_t)width,
                                             (uint8_t)height, (uint8_t)factor};
    return true;
}


