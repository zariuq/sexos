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

#include "silk_shell_proofs_b_input.h"
#include "silk_shell_input.h"
#include "silk_shell_chrome.h"

static void proofs_b_input_binding(const ShellProofsBInputContext *ctx) {
    if (ctx && ctx->input.shell.chrome == &SILK_SHELL_STATE.chrome &&
        ctx->input.shell.models == &SILK_SHELL_STATE.models &&
        ctx->input.shell.apps == &SILK_SHELL_STATE.apps &&
        ctx->input.shell.input == &SILK_SHELL_STATE.input &&
        ctx->input.shell.proofs_a == &SILK_SHELL_STATE.proofs_a &&
        ctx->input.atlas == &SILK_SHELL_STATE.atlas &&
        ctx->input.surfaces == &SILK_SHELL_STATE.surfaces &&
        ctx->proofs_b == &SILK_SHELL_STATE.proofs_b)
        return;
    sexos_fail();
    for (;;) {
    }
}

ShellProofsBDragTarget shell_proofs_b_input_drag_target(ShellProofsBInputContext *ctx) {
    proofs_b_input_binding(ctx);
    ShellChromeDragTarget target = {0};
    bool present = shell_chrome_synthetic_window_drag_target(&ctx->input.shell, &target);
    return (ShellProofsBDragTarget){present,  target.frame, target.sid, target.x, target.y,
                                    target.w, target.h,     target.px,  target.py};
}

ShellProofsBBounds shell_proofs_b_input_bounds(ShellProofsBInputContext *ctx, uint64_t surface) {
    proofs_b_input_binding(ctx);
    ShellProofsBBounds out = {0};
    out.present =
        shell_input_get_surface_bounds(&ctx->input, surface, &out.x, &out.y, &out.w, &out.h);
    return out;
}

ShellProofsBFrame shell_proofs_b_input_frame_for_surface(ShellProofsBInputContext *ctx,
                                                         uint64_t surface) {
    proofs_b_input_binding(ctx);
    uint32_t frame = 0;
    bool present = shell_chrome_frame_for_surface(&ctx->input.shell, surface, &frame);
    return (ShellProofsBFrame){present, frame};
}

ShellProofsBFrame shell_proofs_b_input_first_minimized(ShellProofsBInputContext *ctx) {
    proofs_b_input_binding(ctx);
    uint32_t frame = 0;
    bool present = shell_chrome_first_minimized_frame_id(&ctx->input.shell, &frame);
    return (ShellProofsBFrame){present, frame};
}

ShellProofsBId shell_proofs_b_input_active_surface(ShellProofsBInputContext *ctx, uint32_t frame) {
    proofs_b_input_binding(ctx);
    uint64_t surface = 0;
    bool present = shell_chrome_active_surface_for_frame(&ctx->input.shell, frame, &surface);
    return (ShellProofsBId){present, surface};
}

bool shell_proofs_b_input_focus(ShellProofsBInputContext *ctx, uint64_t surface) {
    proofs_b_input_binding(ctx);
    return shell_chrome_try_set_focus(&ctx->input.shell, surface);
}

bool shell_proofs_b_input_action(ShellProofsBInputContext *ctx, uint8_t action) {
    proofs_b_input_binding(ctx);
    return shell_chrome_access_handle_keyboard_action(&ctx->input.shell, action);
}

bool shell_proofs_b_input_surface_alive(ShellProofsBInputContext *ctx, uint64_t surface) {
    proofs_b_input_binding(ctx);
    return shell_input_surface_is_alive(&ctx->input, surface);
}

bool shell_proofs_b_input_tombstoned(ShellProofsBInputContext *ctx, uint64_t surface) {
    proofs_b_input_binding(ctx);
    return shell_input_is_tombstoned(&ctx->input, surface);
}

bool shell_proofs_b_input_quil_has_focus(ShellProofsBInputContext *ctx) {
    proofs_b_input_binding(ctx);
    return shell_input_peer_quil_has_focus(&ctx->input);
}

void shell_proofs_b_input_hid(ShellProofsBInputContext *ctx, uint64_t event_class, uint64_t arg0,
                              uint64_t arg1) {
    proofs_b_input_binding(ctx);
    shell_input_handle_hid_event(&ctx->input, event_class, arg0, arg1);
}

void shell_proofs_b_input_cursor(ShellProofsBInputContext *ctx, int32_t x, int32_t y,
                                 const char *source) {
    proofs_b_input_binding(ctx);
    shell_input_send_cursor_checked(&ctx->input, x, y, source);
}

void shell_proofs_b_input_transition(ShellProofsBInputContext *ctx, uint8_t tag) {
    proofs_b_input_binding(ctx);
    shell_chrome_try_transition(&ctx->input.shell, (InteractionState){.tag = tag});
}

bool shell_proofs_b_input_zoom_click(ShellProofsBInputContext *ctx, uint32_t frame) {
    proofs_b_input_binding(ctx);
    return synthetic_prove_frame_light_zoom_click(frame);
}

void shell_proofs_b_input_display_create(ShellProofsBInputContext *ctx, uint64_t surface, int32_t x,
                                         int32_t y, uint32_t w, uint32_t h) {
    proofs_b_input_binding(ctx);
    (void)sexos_pdx_call(SLOT_DISPLAY, UINT64_C(0xec), surface,
                         ((uint64_t)(uint32_t)y << 32) | (uint32_t)x, ((uint64_t)h << 32) | w);
}

bool shell_proofs_b_input_lifecycle_visible(ShellProofsBInputContext *ctx, uint64_t surface) {
    proofs_b_input_binding(ctx);
    uint8_t state = 0;
    if (!shell_chrome_lifecycle_state(&ctx->input.shell, surface, &state))
        return shell_chrome_lifecycle_register(&ctx->input.shell, surface, LIFECYCLE_VISIBLE);
    if (state == LIFECYCLE_VISIBLE)
        return true;
    return shell_chrome_set_lifecycle_state(&ctx->input.shell, surface, LIFECYCLE_VISIBLE);
}


