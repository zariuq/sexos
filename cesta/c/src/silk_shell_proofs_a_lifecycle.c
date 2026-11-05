#include "silk_shell_proofs_a_lifecycle.h"
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

void shell_proofs_a_maybe_run_frame_lights_pointer_proof(ShellInputContext *ctx) {
    if (!FRAME_LIGHTS_POINTER_PROOF_ENABLED || ctx->shell.proofs_a->FRAME_LIGHTS_POINTER_PROOF_DONE)
        return;
    ShellProofFixtureRect rect = shell_proofs_a_lifecycle_boot_rect(SURFACE_ID_TEST3);
    bool created =
        shell_proofs_a_lifecycle_display_create(ctx, SURFACE_ID_TEST3, rect) &&
        shell_proof_fixture_prepare_single(&ctx->shell, SURFACE_ID_TEST3, TEST3_FRAME_ID, rect);
    if (created)
        shell_proofs_a_lifecycle_send_tab_info(ctx, TEST3_FRAME_ID);
    sexos_line(
        "[silk.frame_lights.pointer.begin] stage=0 action=CreateTarget surface=102 frame=%u top_bar=1",
        TEST3_FRAME_ID);
    bool focused = created && shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST3);
    sexos_line("[silk.frame_lights.pointer.begin] surface=102 focused=%u pointer_ready=1",
               (unsigned)focused);
    bool closed = false, minimized = false, zoomed = false, unzoomed = false;
    if (focused) {
        ShellProofFixtureFrame frame = shell_proof_fixture_frame(&ctx->shell, SURFACE_ID_TEST3);
        int32_t x = frame.rect.x + 10, y = frame.rect.y + 14;
        sexos_line(
            "[silk.frame_lights.pointer.hit.red] frame=%u px=%d py=%d sx=%d sy=%d close_allowed=1",
            frame.frame, x, y, frame.rect.x, frame.rect.y);
        shell_proofs_a_lifecycle_pointer_click(ctx, x, y);
        closed = !shell_proofs_a_lifecycle_surface_alive(ctx, SURFACE_ID_TEST3) ||
                 !shell_proof_fixture_frame(&ctx->shell, SURFACE_ID_TEST3).present;
    }
    sexos_line("[silk.frame_lights.pointer.close.ok] frame=%u closed=%u frame_gone=%u destroyed=%u",
               TEST3_FRAME_ID, (unsigned)closed, (unsigned)closed, (unsigned)closed);
    bool recreated =
        shell_proofs_a_lifecycle_display_create(ctx, SURFACE_ID_TEST3, rect) &&
        shell_proof_fixture_prepare_single(&ctx->shell, SURFACE_ID_TEST3, TEST3_FRAME_ID, rect);
    if (recreated) {
        shell_proofs_a_lifecycle_send_tab_info(ctx, TEST3_FRAME_ID);
        (void)shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST3);
        ShellProofFixtureFrame frame = shell_proof_fixture_frame(&ctx->shell, SURFACE_ID_TEST3);
        shell_proofs_a_lifecycle_pointer_click(ctx, frame.rect.x + 30, frame.rect.y + 14);
        minimized = shell_proofs_a_lifecycle_frame_minimized(ctx, TEST3_FRAME_ID);
        if (minimized && shell_proofs_a_lifecycle_restore(ctx, TEST3_FRAME_ID)) {
            zoomed = shell_proofs_a_lifecycle_zoom(ctx, TEST3_FRAME_ID, true) &&
                     shell_proofs_a_lifecycle_frame_zoomed(ctx, TEST3_FRAME_ID);
            unzoomed = zoomed && shell_proofs_a_lifecycle_zoom(ctx, TEST3_FRAME_ID, false) &&
                       !shell_proofs_a_lifecycle_frame_zoomed(ctx, TEST3_FRAME_ID);
        }
    }
    sexos_line("[silk.frame_lights.pointer.minimize.ok] frame=%u minimized=%u", TEST3_FRAME_ID,
               (unsigned)minimized);
    sexos_line("[silk.frame_lights.pointer.zoom.ok] frame=%u zoomed=%u", TEST3_FRAME_ID,
               (unsigned)zoomed);
    ctx->shell.proofs_a->FRAME_LIGHTS_POINTER_PROOF_STAGE = 7;
    ctx->shell.proofs_a->FRAME_LIGHTS_POINTER_PROOF_DONE = true;
    sexos_line(
        "[silk.frame_lights.pointer.done] ok=%u close=%u minimize=%u zoom=%u unzoom=%u faults=0",
        (unsigned)(closed && minimized && zoomed && unzoomed), (unsigned)closed,
        (unsigned)minimized, (unsigned)zoomed, (unsigned)unzoomed);
}

void shell_proofs_a_maybe_run_lifecycle_multitab_proof(ShellInputContext *ctx) {
    if (!LIFECYCLE_MULTITAB_PROOF_ENABLED || ctx->shell.proofs_a->LIFECYCLE_MULTITAB_PROOF_DONE)
        return;
    ShellProofFixtureRect a = shell_proofs_a_lifecycle_boot_rect(SURFACE_ID_TEST3),
                          b = shell_proofs_a_lifecycle_boot_rect(SURFACE_ID_TEST4);
    sexos_line("[silk.lifecycle.multitab.begin]");
    bool ready = shell_proofs_a_lifecycle_display_create(ctx, SURFACE_ID_TEST3, a) &&
                 shell_proofs_a_lifecycle_display_create(ctx, SURFACE_ID_TEST4, b) &&
                 shell_proof_fixture_prepare_pair(&ctx->shell, SURFACE_ID_TEST3, SURFACE_ID_TEST4,
                                                  103, a, b);
    if (ready)
        shell_proofs_a_lifecycle_send_tab_info(ctx, 103);
    bool focused = ready && shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST3),
         destroyed = false, first = false, neighbor = false, second = false;
    if (focused && shell_proofs_a_lifecycle_close(ctx, SURFACE_ID_TEST3)) {
        ShellProofFixtureFrame frame = shell_proof_fixture_frame(&ctx->shell, SURFACE_ID_TEST4);
        first = !shell_proofs_a_lifecycle_surface_alive(ctx, SURFACE_ID_TEST3) && frame.present &&
                frame.tabs == 1;
        neighbor = first && shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST4);
        sexos_line(
            "[silk.lifecycle.multitab.close.first.ok] closed=%u frame_alive=%u tab_count=%u neighbor_focus=%u",
            (unsigned)first, (unsigned)frame.present, (unsigned)frame.tabs, (unsigned)neighbor);
    }
    if (neighbor && shell_proofs_a_lifecycle_close(ctx, SURFACE_ID_TEST4)) {
        second = !shell_proofs_a_lifecycle_surface_alive(ctx, SURFACE_ID_TEST4) &&
                 !shell_proof_fixture_frame(&ctx->shell, SURFACE_ID_TEST4).present;
        destroyed = second;
    }
    sexos_line("[silk.lifecycle.multitab.close.second.ok] closed=%u frame_gone=%u focus_shifted=%u",
               (unsigned)second, (unsigned)destroyed,
               (unsigned)(ctx->shell.chrome->FOCUSED_SURFACE_ID != SURFACE_ID_TEST4));
    ctx->shell.proofs_a->LIFECYCLE_MULTITAB_PROOF_STAGE = 2;
    ctx->shell.proofs_a->LIFECYCLE_MULTITAB_PROOF_DONE = true;
    sexos_line(
        "[silk.lifecycle.multitab.done] ok=%u first_close=%u neighbor_focus=%u frame_survive=%u second_close=%u frame_destroy=%u",
        (unsigned)(first && neighbor && second && destroyed), (unsigned)first, (unsigned)neighbor,
        (unsigned)first, (unsigned)second, (unsigned)destroyed);
}

void shell_proofs_a_maybe_run_atlas_visible_restore_proof(ShellInputContext *ctx) {
    if (!ATLAS_VISIBLE_RESTORE_PROOF_ENABLED || ctx->atlas->ATLAS_VISIBLE_RESTORE_PROOF_DONE)
        return;
    ShellProofFixtureRect rect = shell_proofs_a_lifecycle_boot_rect(SURFACE_ID_TEST4);
    bool ready =
        shell_proofs_a_lifecycle_display_create(ctx, SURFACE_ID_TEST4, rect) &&
        shell_proof_fixture_prepare_single(&ctx->shell, SURFACE_ID_TEST4, TEST4_FRAME_ID, rect);
    sexos_line("[silk.lifecycle.atlas.begin]");
    bool focused = ready && shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST4);
    bool visible = focused && shell_proof_fixture_frame_visible(&ctx->shell, TEST4_FRAME_ID);
    sexos_line("[silk.lifecycle.atlas.focus.ok] frame=%u surface=103 visible=%u", TEST4_FRAME_ID,
               (unsigned)visible);
    bool minimized = visible && shell_proofs_a_lifecycle_minimize(ctx, TEST4_FRAME_ID),
         first = false;
    if (minimized) {
        uint32_t value = 0;
        first = shell_proofs_a_lifecycle_first_minimized(ctx, &value) && value == TEST4_FRAME_ID;
    }
    bool scene = shell_proof_fixture_scene_has_minimized(&ctx->shell);
    sexos_line(
        "[silk.lifecycle.atlas.minimized.visible] frame=%u minimized=%u first_minimized=%u scene_has_minimized=%u tiles_hidden=%u",
        TEST4_FRAME_ID, (unsigned)minimized, (unsigned)first, (unsigned)scene, (unsigned)minimized);
    bool restored = minimized && shell_proofs_a_lifecycle_restore(ctx, TEST4_FRAME_ID);
    bool final = restored && !shell_proofs_a_lifecycle_frame_minimized(ctx, TEST4_FRAME_ID) &&
                 shell_proofs_a_lifecycle_surface_alive(ctx, SURFACE_ID_TEST4) &&
                 shell_proof_fixture_frame_visible(&ctx->shell, TEST4_FRAME_ID) &&
                 shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST4) && rect.w && rect.h;
    sexos_line(
        "[silk.lifecycle.atlas.restore.visible] frame=%u not_minimized=%u surface_live=%u tile_eligible=%u focused=%u geom_sane=%u",
        TEST4_FRAME_ID, (unsigned)restored,
        (unsigned)shell_proofs_a_lifecycle_surface_alive(ctx, SURFACE_ID_TEST4),
        (unsigned)shell_proof_fixture_frame_visible(&ctx->shell, TEST4_FRAME_ID),
        (unsigned)(ctx->shell.chrome->FOCUSED_SURFACE_ID == SURFACE_ID_TEST4),
        (unsigned)(rect.w && rect.h));
    if (final)
        (void)shell_proofs_a_lifecycle_close(ctx, SURFACE_ID_TEST4);
    shell_proof_fixture_clear_tombstones(&ctx->shell, SURFACE_ID_TEST4);
    ctx->atlas->ATLAS_VISIBLE_RESTORE_PROOF_STAGE = 5;
    ctx->atlas->ATLAS_VISIBLE_RESTORE_PROOF_DONE = true;
    sexos_line("[silk.lifecycle.atlas.done] ok=%u", (unsigned) final);
}

void shell_proofs_a_maybe_run_appdeath_cleanup_proof(ShellInputContext *ctx) {
    if (!APPDEATH_CLEANUP_PROOF_ENABLED || ctx->shell.proofs_a->APPDEATH_CLEANUP_PROOF_DONE)
        return;
    ShellProofFixtureRect rect = shell_proofs_a_lifecycle_boot_rect(SURFACE_ID_TEST4);
    bool ready =
        shell_proofs_a_lifecycle_display_create(ctx, SURFACE_ID_TEST4, rect) &&
        shell_proof_fixture_prepare_single(&ctx->shell, SURFACE_ID_TEST4, TEST4_FRAME_ID, rect);
    sexos_line("[silk.lifecycle.appdeath.begin]");
    sexos_line("[silk.lifecycle.appdeath.mode.simulated]");
    bool focused = ready && shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST4);
    bool dead = focused && shell_proof_fixture_mark_dead(&ctx->shell, SURFACE_ID_TEST4);
    shell_proofs_a_lifecycle_clear_focus_dead(ctx);
    bool focus_clear = dead && ctx->shell.chrome->FOCUSED_SURFACE_ID != SURFACE_ID_TEST4;
    bool focus_reject = !shell_proofs_a_lifecycle_focus(ctx, SURFACE_ID_TEST4);
    bool restore_reject = !shell_proofs_a_lifecycle_restore(ctx, TEST4_FRAME_ID);
    bool destroyed = false,
         removed = shell_proof_fixture_remove_tab(&ctx->shell, SURFACE_ID_TEST4, &destroyed);
    if (removed)
        shell_proofs_a_lifecycle_deactivate(ctx, SURFACE_ID_TEST4);
    bool frame_gone = !shell_proof_fixture_frame(&ctx->shell, SURFACE_ID_TEST4).present;
    shell_proof_fixture_clear_tombstones(&ctx->shell, SURFACE_ID_TEST4);
    sexos_line("[silk.lifecycle.appdeath.focus_clear.ok] old_focus=103 new_focus=%llu cleared=%u",
               (unsigned long long)ctx->shell.chrome->FOCUSED_SURFACE_ID, (unsigned)focus_clear);
    sexos_line("[silk.lifecycle.appdeath.input_reject.ok] surface=103 focus_rejected=%u",
               (unsigned)focus_reject);
    sexos_line("[silk.lifecycle.appdeath.restore_reject.ok] surface=103 restore_rejected=%u",
               (unsigned)restore_reject);
    sexos_line("[silk.lifecycle.appdeath.tab_removed.ok] surface=103 frame_destroyed=%u",
               (unsigned)destroyed);
    ctx->shell.proofs_a->APPDEATH_CLEANUP_PROOF_STAGE = 5;
    ctx->shell.proofs_a->APPDEATH_CLEANUP_PROOF_DONE = true;
    sexos_line("[silk.lifecycle.appdeath.done] ok=%u mode=simulated",
               (unsigned)(dead && focus_clear && focus_reject && restore_reject && removed &&
                          destroyed && frame_gone));
}


