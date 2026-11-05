#include "silk_shell_input.h"
#include "silk_shell_surfaces.h"
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

bool shell_input_peer_quil_has_focus(ShellInputContext *ctx) {
    if (ctx->shell.chrome->FOCUSED_SURFACE_ID == SURFACE_ID_QUIL)
        return true;
    ShellActivationResult result = {0};
    {
        ShellSurfacesContext view = {.shell = ctx->shell, .surfaces = ctx->surfaces};
        result = shell_surfaces_app_activation_info(&view, SHELL_APP_QUIL, false);
    }
    if (result.status || result.info.state != 2)
        return false;
    uint64_t focused = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    return focused == result.info.surface;
}

bool shell_input_peer_spindle_has_focus(ShellInputContext *ctx) {
    if (ctx->shell.chrome->FOCUSED_SURFACE_ID == SURFACE_ID_SPINDLE)
        return true;
    ShellActivationResult result = {0};
    {
        ShellSurfacesContext view = {.shell = ctx->shell, .surfaces = ctx->surfaces};
        result = shell_surfaces_app_activation_info(&view, SHELL_APP_SPINDLE, false);
    }
    if (result.status || result.info.state != 2)
        return false;
    uint64_t focused = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    ShellAppCatalogRow row = shell_custody_catalog_find(result.info.successor, result.info.surface);
    return row.present && focused == (row.shell_surface ? row.shell_surface : row.surface);
}


