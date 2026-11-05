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

#include "silk_shell_proofs_a_lifecycle.h"
#include "silk_shell_surfaces.h"

static ShellInputContext lifecycle_context(void) {
    return (ShellInputContext){.shell = {.input = &SILK_SHELL_STATE.input,
                                         .chrome = &SILK_SHELL_STATE.chrome,
                                         .models = &SILK_SHELL_STATE.models,
                                         .apps = &SILK_SHELL_STATE.apps,
                                         .proofs_a = &SILK_SHELL_STATE.proofs_a},
                               .atlas = &SILK_SHELL_STATE.atlas,
                               .surfaces = &SILK_SHELL_STATE.surfaces};
}

ShellProofFixtureRect shell_proofs_a_lifecycle_boot_rect(uint64_t surface) {
    Rect r = surface == SURFACE_ID_TEST3 ? P.boot_rect_102 : P.boot_rect_103;
    return (ShellProofFixtureRect){r.x, r.y, r.w, r.h};
}

bool shell_proofs_a_lifecycle_display_create(ShellInputContext *ctx, uint64_t surface,
                                             ShellProofFixtureRect r) {
    (void)ctx;
    return sexos_pdx_call(SLOT_DISPLAY, 0xec, surface,
                          ((uint64_t)(uint32_t)r.y << 32) | (uint32_t)r.x,
                          ((uint64_t)r.h << 32) | r.w)
               .status == 0;
}

void shell_proofs_a_lifecycle_send_tab_info(ShellInputContext *ctx, uint32_t frame) {
    shell_chrome_send_frame_tab_info(&ctx->shell, frame);
}

bool shell_proofs_a_lifecycle_focus(ShellInputContext *ctx, uint64_t surface) {
    return shell_chrome_try_set_focus(&ctx->shell, surface);
}

bool shell_proofs_a_lifecycle_close(ShellInputContext *ctx, uint64_t surface) {
    return shell_chrome_close_surface_from_frame_light(&ctx->shell, surface);
}

bool shell_proofs_a_lifecycle_minimize(ShellInputContext *ctx, uint32_t frame) {
    return shell_chrome_minimize_frame(&ctx->shell, frame);
}

bool shell_proofs_a_lifecycle_restore(ShellInputContext *ctx, uint32_t frame) {
    return shell_chrome_restore_minimized_frame(&ctx->shell, frame);
}

bool shell_proofs_a_lifecycle_zoom(ShellInputContext *ctx, uint32_t frame, bool enabled) {
    if (enabled == shell_chrome_frame_is_zoomed(&ctx->shell, frame))
        return true;
    return enabled ? shell_chrome_zoom_frame(&ctx->shell, frame)
                   : shell_chrome_unzoom_frame(&ctx->shell, frame);
}

bool shell_proofs_a_lifecycle_frame_minimized(ShellInputContext *ctx, uint32_t frame) {
    return shell_chrome_frame_is_minimized(&ctx->shell, frame);
}

bool shell_proofs_a_lifecycle_frame_zoomed(ShellInputContext *ctx, uint32_t frame) {
    return shell_chrome_frame_is_zoomed(&ctx->shell, frame);
}

bool shell_proofs_a_lifecycle_first_minimized(ShellInputContext *ctx, uint32_t *frame) {
    return shell_chrome_first_minimized_frame_id(&ctx->shell, frame);
}

void shell_proofs_a_lifecycle_pointer_click(ShellInputContext *ctx, int32_t x, int32_t y) {
    (void)shell_chrome_click_hit_test_and_focus(&ctx->shell, x, y, 1);
}

void shell_proofs_a_lifecycle_clear_focus_dead(ShellInputContext *ctx) {
    shell_chrome_peer_clear_focus_if_dead(&ctx->shell);
}

bool shell_proofs_a_lifecycle_surface_alive(ShellInputContext *ctx, uint64_t surface) {
    return shell_chrome_peer_surface_is_alive(&ctx->shell, surface);
}

void shell_proofs_a_lifecycle_deactivate(ShellInputContext *ctx, uint64_t surface) {
    (void)ctx;
    (void)sexos_pdx_call(SLOT_DISPLAY, OP_SURFACE_DEACTIVATE, surface, 0, 0);
}

void maybe_run_frame_lights_pointer_proof(void) {
    ShellInputContext ctx = lifecycle_context();
    shell_proofs_a_maybe_run_frame_lights_pointer_proof(&ctx);
}

void maybe_run_lifecycle_multitab_proof(void) {
    ShellInputContext ctx = lifecycle_context();
    shell_proofs_a_maybe_run_lifecycle_multitab_proof(&ctx);
}

void maybe_run_atlas_visible_restore_proof(void) {
    ShellInputContext ctx = lifecycle_context();
    shell_proofs_a_maybe_run_atlas_visible_restore_proof(&ctx);
}

void maybe_run_appdeath_cleanup_proof(void) {
    ShellInputContext ctx = lifecycle_context();
    shell_proofs_a_maybe_run_appdeath_cleanup_proof(&ctx);
}


