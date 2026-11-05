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
/*Bounded value extraction from the published static-label record array.*/

#include "silk_shell_models_render.h"

ShellLinenItems shell_models_peer_list_items(ShellLinenRenderContext *ctx) {
    ShellLinenItems items = {0};
    for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
        const LinenObject *object = &ctx->shell.models->LINEN_OBJECTS[i];
        if (!object->present)
            continue;
        if (items.count >= SHELL_LINEN_LIST_CAPACITY) {
            sexos_fail();
            for (;;) {
            }
        }
        ShellLinenListItem *out = &items.items[items.count++];
        out->id = object->object_id;
        if (object->name_len) {
            out->len = object->name_len < 24 ? object->name_len : 24;
            for (size_t j = 0; j < out->len; ++j)
                out->name[j] = object->name[j];
        } else {
            if (!object->display_name) {
                sexos_fail();
                for (;;) {
                }
            }
            while (out->len < SHELL_LINEN_LIST_NAME_CAPACITY && object->display_name[out->len]) {
                out->name[out->len] = (uint8_t)object->display_name[out->len];
                ++out->len;
            }
        }
    }
    return items;
}


