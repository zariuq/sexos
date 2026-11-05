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

#include "silk_shell_proofs_b_atlas.h"
#include "silk_shell_atlas.h"

static void proofs_b_atlas_binding(const ShellInputContext *ctx) {
    if (ctx && ctx->shell.chrome == &SILK_SHELL_STATE.chrome &&
        ctx->shell.models == &SILK_SHELL_STATE.models &&
        ctx->shell.apps == &SILK_SHELL_STATE.apps && ctx->shell.input == &SILK_SHELL_STATE.input &&
        ctx->shell.proofs_a == &SILK_SHELL_STATE.proofs_a &&
        ctx->atlas == &SILK_SHELL_STATE.atlas && ctx->surfaces == &SILK_SHELL_STATE.surfaces)
        return;
    sexos_fail();
    for (;;) {
    }
}

static ShellAtlasContext proofs_b_atlas(ShellInputContext *ctx) {
    return (ShellAtlasContext){ctx->atlas, ctx->shell.chrome, ctx->shell.models};
}

int32_t shell_proofs_b_atlas_desktop_width(ShellInputContext *ctx) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    return shell_atlas_peer_desktop_width(&view);
}

int32_t shell_proofs_b_atlas_bar_height(ShellInputContext *ctx) {
    proofs_b_atlas_binding(ctx);
    return P.bar_height;
}

ShellProofsBCard shell_proofs_b_atlas_card(ShellInputContext *ctx, size_t scene, uint32_t width) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    ShellProofsBCard out = {0};
    shell_atlas_atlas_card_pos(&view, scene, width, &out.x, &out.y, &out.w, &out.h);
    return out;
}

ShellProofsBScene shell_proofs_b_atlas_scene_at(ShellInputContext *ctx, int32_t x, int32_t y) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    uint8_t scene = 0;
    bool present = shell_atlas_atlas_scene_at_point(&view, x, y, &scene);
    return (ShellProofsBScene){present, scene};
}

void shell_proofs_b_atlas_exit(ShellInputContext *ctx) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    shell_atlas_atlas_exit(&view);
}

bool shell_proofs_b_atlas_cycle(ShellInputContext *ctx, int32_t delta, const char *reason) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    return shell_atlas_atlas_cycle_scene(&view, delta, reason);
}

bool shell_proofs_b_atlas_same_scene_noop(ShellInputContext *ctx, uint32_t frame, uint8_t source,
                                          uint8_t target) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    return shell_atlas_atlas_same_scene_drop_noop(&view, frame, source, target);
}

bool shell_proofs_b_atlas_reparent(ShellInputContext *ctx, uint32_t frame, uint8_t target,
                                   const char *reason) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    return shell_atlas_reparent_frame_to_scene(&view, frame, target, reason);
}

bool shell_proofs_b_atlas_surface_in_active_scene(ShellInputContext *ctx, uint64_t surface) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    return shell_atlas_surface_in_active_scene(&view, surface);
}

void shell_proofs_b_atlas_input_hid(ShellInputContext *ctx, uint64_t event_class, uint64_t arg0,
                                    uint64_t arg1) {
    proofs_b_atlas_binding(ctx);
    shell_input_handle_hid_event(ctx, event_class, arg0, arg1);
}

bool shell_proofs_b_atlas_cancel_drag(ShellInputContext *ctx, const char *reason) {
    proofs_b_atlas_binding(ctx);
    ShellAtlasContext view = proofs_b_atlas(ctx);
    return shell_atlas_atlas_pointer_drag_cancel(&view, reason);
}


