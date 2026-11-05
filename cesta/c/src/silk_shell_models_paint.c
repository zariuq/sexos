#include "silk_shell_models_paint.h"
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

void shell_models_paint_surface(ShellLinenRenderContext *ctx, bool startup_pending) {
    if (!startup_pending &&
        (!ctx->shell.models->LINEN_REMOTE_FETCHED || !ctx->shell.models->LINEN_REMOTE_REAL)) {
        ctx->shell.models->LINEN_REMOTE_FETCHED = true;
        ShellLinenModelsContext remote = {ctx->shell.models, ctx->linen};
        shell_models_fetch_remote_snapshot(&remote);
    }
    if (shell_models_peer_object_count(ctx) == 0)
        shell_models_render_static_ui(ctx);
    else
        shell_models_render_object_list(ctx);
}

void shell_models_paint_surface_fast(ShellLinenRenderContext *ctx) {
    bool remote = ctx->shell.models->LINEN_REMOTE_FETCHED;
    size_t count = shell_models_peer_object_count(ctx);
    sexos_line("[linen.fast_paint] sid=200 objects=%zu ok=1 reason=%s", count,
               remote ? "remote_ready" : "seeds_only");
    if (count == 0)
        shell_models_render_static_ui(ctx);
    else
        shell_models_render_object_list(ctx);
}


