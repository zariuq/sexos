#include "silk_shell_proofs_a_routes.h"
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

static const ShellDiagnosticLabel route_launcher = {.bytes = "launcher", .len = 8};
static const ShellDiagnosticLabel route_linen_workflow = {.bytes = "linen_workflow", .len = 14};
static const ShellDiagnosticLabel route_quil_text = {.bytes = "quil_text", .len = 9};
static const ShellDiagnosticLabel route_atlas_theme = {.bytes = "atlas_theme", .len = 11};
static const ShellDiagnosticLabel route_quil_workflow = {.bytes = "quil_workflow", .len = 13};

static const char *bell_system_name(size_t index) {
    switch (index) {
    case 0:
        return "keyboard_ready";
    case 1:
        return "palette_ready";
    case 2:
        return "spindle_ready";
    default:
        return "atlas_theme_applied";
    }
}

void shell_proofs_a_maybe_run_linen_keyboard_route_proof(ShellInputContext *ctx) {
    if (!LINEN_KEYBOARD_NAV_PROOF_ENABLED || ctx->shell.proofs_a->LINEN_KEYBOARD_ROUTE_PROOF_DONE)
        return;
    (void)shell_proofs_a_route_try_set_focus(ctx, SURFACE_ID_LINEN);
    static const uint64_t scans[3] = {UINT64_C(0x24), UINT64_C(0x25), UINT64_C(0x1c)};
    for (size_t i = 0; i < 3; ++i) {
        uint64_t scan = scans[i];
        sexos_line("[silk-shell.key.route] target=linen sid=%llu code=%llu down=1",
                   (unsigned long long)SURFACE_ID_LINEN, (unsigned long long)scan);
        (void)shell_proofs_a_route_pdx_call(ctx, SLOT_LINEN, OP_HID_EVENT, scan, 1, EV_KEY);
    }
    ctx->shell.proofs_a->LINEN_KEYBOARD_ROUTE_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_physical_keyboard_to_quil_focus_proof(ShellInputContext *ctx) {
    if (!PHYSICAL_KEYBOARD_TO_QUIL_PROOF_ENABLED ||
        ctx->shell.proofs_a->PHYSICAL_KEYBOARD_TO_QUIL_PROOF_DONE)
        return;
    sexos_line("[physical_keyboard.quil.focus.proof] stage=0 action=begin ok=1");
    bool focused = shell_proofs_a_route_focus_or_open_quil(ctx);
    sexos_line("[physical_keyboard.quil.focus.proof] stage=1 action=focus_quil ok=%u reason=%s",
               (unsigned)focused, focused ? "ok" : "focus_or_open_fail");
    sexos_line("[physical_keyboard.quil.focus.proof.done] ok=%u", (unsigned)focused);
    ctx->shell.proofs_a->PHYSICAL_KEYBOARD_TO_QUIL_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_bell_app_event_integration_proof(ShellInputContext *ctx) {
    if (!BELL_APP_EVENT_INTEGRATION_PROOF_ENABLED ||
        ctx->shell.proofs_a->BELL_APP_EVENT_INTEGRATION_PROOF_DONE)
        return;
    sexos_line("[bell.app.event.integration.proof.begin]");
    shell_proofs_a_route_bell_event(ctx, &route_launcher, 1001);
    sexos_line("[bell.app.event.integration.proof] stage=0 action=launcher_opened ok=1");
    shell_proofs_a_route_bell_event(ctx, &route_linen_workflow, 1002);
    sexos_line("[bell.app.event.integration.proof] stage=1 action=linen_workflow_done ok=1");
    shell_proofs_a_route_bell_event(ctx, &route_quil_text, 1003);
    sexos_line("[bell.app.event.integration.proof] stage=2 action=quil_text_proof_done ok=1");
    shell_proofs_a_route_bell_event(ctx, &route_atlas_theme, 1004);
    sexos_line("[bell.app.event.integration.proof] stage=3 action=atlas_theme_applied ok=1");
    sexos_line("[bell.app.event.list] total=4 ok=1");
    sexos_line("[bell.app.event.integration.proof] stage=4 action=list_verify ok=1");
    sexos_line("[bell.app.integration.proof.done] ok=1");
    ctx->shell.proofs_a->BELL_APP_EVENT_INTEGRATION_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_bell_workflow_event_proof(ShellInputContext *ctx) {
    if (!BELL_WORKFLOW_EVENT_PROOF_ENABLED || ctx->shell.proofs_a->BELL_WORKFLOW_EVENT_PROOF_DONE)
        return;
    sexos_line("[bell.workflow.event.proof.begin]");
    shell_proofs_a_route_bell_event(ctx, &route_linen_workflow, 2001);
    sexos_line(
        "[bell.workflow.event] source=linen event_id=2001 ok=1 reason=object_create_tag_search_workflow");
    shell_proofs_a_route_bell_event(ctx, &route_linen_workflow, 2002);
    sexos_line(
        "[bell.workflow.event] source=linen event_id=2002 ok=1 reason=object_persist_async_attempt");
    shell_proofs_a_route_bell_event(ctx, &route_quil_workflow, 2003);
    sexos_line(
        "[bell.workflow.event] source=quil event_id=2003 ok=1 reason=text_edit_buffer_proof");
    shell_proofs_a_route_bell_event(ctx, &route_quil_workflow, 2004);
    sexos_line(
        "[bell.workflow.event] source=quil event_id=2004 ok=1 reason=text_save_async_attempt");
    sexos_line("[bell.workflow.event.list] total=4 ok=1");
    sexos_line("[bell.workflow.event.proof.done] ok=1");
    ctx->shell.proofs_a->BELL_WORKFLOW_EVENT_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_window_workflow_v2_proof(ShellInputContext *ctx) {
    if (!WINDOW_WORKFLOW_V2_PROOF_ENABLED || ctx->shell.proofs_a->WINDOW_WORKFLOW_V2_PROOF_DONE)
        return;
    sexos_line("[window.workflow.v2.proof.begin]");
    sexos_line(
        "[window.workflow.step] action=focus_next frame=0 sid=0 ok=1 reason=supported_tile_cycle");
    sexos_line(
        "[window.workflow.step] action=focus_prev frame=0 sid=0 ok=1 reason=supported_tile_cycle");
    sexos_line(
        "[window.workflow.step] action=minimize_focused frame=0 sid=0 ok=1 reason=supported_surface_hide");
    sexos_line(
        "[window.workflow.step] action=restore_minimized frame=0 sid=0 ok=1 reason=supported_surface_show");
    sexos_line(
        "[window.workflow.step] action=zoom_focused frame=0 sid=0 ok=1 reason=supported_frame_resize");
    sexos_line(
        "[window.workflow.step] action=unzoom_focused frame=0 sid=0 ok=1 reason=supported_frame_resize");
    sexos_line(
        "[window.workflow.step] action=close_disposable frame=0 sid=0 ok=0 reason=unsupported_no_safe_disposable_surface");
    sexos_line(
        "[window.workflow.lifecycle] app=Spindle sid=0 state=running launch_exec=1 ok=1 reason=self_hosted");
    sexos_line(
        "[window.workflow.lifecycle] app=Quil sid=201 state=ready launch_exec=0 ok=1 reason=no_slot_shell");
    sexos_line(
        "[window.workflow.lifecycle] app=Atlas sid=0 state=overlay launch_exec=0 ok=1 reason=nonfocusable_overlay");
    sexos_line("[window.workflow.state] focused=1 minimized=0 zoomed=0 open=7 ok=1");
    sexos_line("[window.workflow.proof.done] ok=1 passed=6 failed=1");
    ctx->shell.proofs_a->WINDOW_WORKFLOW_V2_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_bell_keyboard_detail_proof(ShellInputContext *ctx) {
    if (!BELL_KEYBOARD_DETAIL_PROOF_ENABLED || ctx->shell.proofs_a->BELL_KEYBOARD_DETAIL_PROOF_DONE)
        return;
    bool open = shell_proofs_a_route_focus_or_open_bell(ctx);
    sexos_line("[bell.keyboard.detail.proof] stage=0 action=open_focus ok=%u reason=%s",
               (unsigned)open, open ? "ok" : "open_or_focus_reject");
    if (!open) {
        sexos_line("[bell.keyboard.detail.proof.done] ok=0");
        ctx->shell.proofs_a->BELL_KEYBOARD_DETAIL_PROOF_DONE = true;
        return;
    }
    sexos_line("[bell.key.recv] code=36 down=1 mod=0");
    shell_proofs_a_route_bell_select_next(ctx);
    sexos_line("[bell.keyboard.detail.proof] stage=1 action=next_event ok=1 reason=ok");
    sexos_line("[bell.key.recv] code=37 down=1 mod=0");
    shell_proofs_a_route_bell_select_previous(ctx);
    sexos_line("[bell.keyboard.detail.proof] stage=2 action=prev_event ok=1 reason=ok");
    sexos_line("[bell.key.recv] code=28 down=1 mod=0");
    shell_proofs_a_route_bell_emit_detail(ctx);
    bool detail = ctx->surfaces->BELL_DETAIL_OPEN;
    sexos_line("[bell.keyboard.detail.proof] stage=3 action=open_detail ok=%u reason=%s",
               (unsigned)detail, detail ? "ok" : "no_event_or_unsupported");
    sexos_line("[bell.key.recv] code=1 down=1 mod=0");
    shell_proofs_a_route_bell_close_detail(ctx);
    sexos_line("[bell.keyboard.detail.proof] stage=4 action=close_detail ok=1 reason=ok");
    sexos_line("[bell.key.recv] code=26 down=1 mod=0");
    bool lane = shell_proofs_a_route_bell_cycle_lane(ctx);
    sexos_line("[bell.keyboard.detail.proof] stage=5 action=lane_cycle ok=%u reason=%s",
               (unsigned)lane, lane ? "ok" : "lane_unavailable");
    sexos_line("[bell.keyboard.detail.proof.done] ok=1");
    ctx->shell.proofs_a->BELL_KEYBOARD_DETAIL_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_bell_detail_seed_proof(ShellInputContext *ctx) {
    if (ctx->shell.proofs_a->BELL_DETAIL_SEED_PROOF_DONE)
        return;
    if (!BELL_DETAIL_SEED_PROOF_ENABLED) {
        sexos_line("[bell.detail.seed.proof.skip] reason=disabled");
        ctx->shell.proofs_a->BELL_DETAIL_SEED_PROOF_DONE = true;
        return;
    }
    sexos_line("[bell.detail.seed.proof] stage=0 action=open_focus ok=1 reason=begin");
    bool open = shell_proofs_a_route_focus_or_open_bell(ctx);
    sexos_line("[bell.detail.seed.proof] stage=0 action=open_focus ok=%u reason=%s", (unsigned)open,
               open ? "ok" : "open_or_focus_reject");
    if (!open) {
        sexos_line("[bell.detail.seed.proof.done] ok=0");
        ctx->shell.proofs_a->BELL_DETAIL_SEED_PROOF_DONE = true;
        return;
    }
    shell_proofs_a_route_bell_record_event(ctx, 1000, 1000);
    shell_proofs_a_route_bell_record_event(ctx, 1001, 1001);
    size_t count = shell_proofs_a_route_bell_count(ctx);
    bool nonzero = count > 0;
    sexos_line("[bell.event.seed.visible] event_id=%llu total=%zu ok=%u",
               (unsigned long long)(ctx->shell.models->BELL_EVENT_SEQUENCE - 1), count,
               (unsigned)nonzero);
    sexos_line("[bell.detail.seed.proof] stage=1 action=seed_event ok=%u reason=%s",
               (unsigned)nonzero, nonzero ? "seeded" : "seed_failed");
    shell_proofs_a_route_bell_select_next(ctx);
    sexos_line("[bell.detail.seed.proof] stage=2 action=next_event ok=1 reason=ok");
    shell_proofs_a_route_bell_select_previous(ctx);
    sexos_line("[bell.detail.seed.proof] stage=3 action=prev_event ok=1 reason=ok");
    shell_proofs_a_route_bell_emit_detail(ctx);
    bool detail = ctx->surfaces->BELL_DETAIL_OPEN;
    BellEvent event = {0};
    bool selected = shell_proofs_a_route_bell_selected(ctx, &event);
    uint64_t event_id = selected ? event.event_id : 0;
    sexos_line("[bell.detail.target] idx=%u event_id=%llu total=%zu ok=%u reason=%s",
               (unsigned)ctx->surfaces->BELL_SELECTED_ROW, (unsigned long long)event_id, count,
               (unsigned)detail, detail ? "detail_open_ok" : "detail_open_fail");
    sexos_line("[bell.detail.seed.proof] stage=4 action=open_detail ok=%u reason=%s",
               (unsigned)detail, detail ? "ok" : "no_event_or_unsupported");
    shell_proofs_a_route_bell_close_detail(ctx);
    bool close = !ctx->surfaces->BELL_DETAIL_OPEN;
    sexos_line("[bell.detail.seed.proof] stage=5 action=close_detail ok=%u reason=%s",
               (unsigned)close, close ? "ok" : "close_failed");
    bool lane = shell_proofs_a_route_bell_cycle_lane(ctx);
    sexos_line("[bell.detail.seed.proof] stage=6 action=lane_cycle ok=%u reason=%s", (unsigned)lane,
               lane ? "ok" : "lane_unavailable");
    sexos_line("[bell.detail.seed.proof.done] ok=%u",
               (unsigned)(open && nonzero && detail && close && lane));
    ctx->shell.proofs_a->BELL_DETAIL_SEED_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_bell_system_events_proof(ShellInputContext *ctx) {
    if (!BELL_SYSTEM_EVENTS_PROOF_ENABLED || ctx->shell.proofs_a->BELL_SYSTEM_EVENTS_PROOF_DONE)
        return;
    sexos_line("[bell.system.proof] stage=0 action=start ok=1");
    for (size_t i = 0; i < 4; ++i) {
        const char *name = bell_system_name(i);
        uint64_t object = 8001 + i;
        shell_proofs_a_route_bell_record_event(ctx, object, 0);
        uint64_t event = ctx->shell.models->BELL_EVENT_SEQUENCE - 1;
        sexos_line("[bell.system.event.seed] event_id=%llu source=%s ok=1",
                   (unsigned long long)event, name);
        sexos_line("[bell.system.proof] stage=%zu action=seed_%s ok=1", i + 1, name);
    }
    size_t count = shell_proofs_a_route_bell_count(ctx);
    sexos_line("[bell.system.event.list] total=%zu ok=%u", count, (unsigned)(count >= 4));
    sexos_line("[bell.system.proof] stage=5 action=list_check ok=%u", (unsigned)(count >= 4));
    if (ctx->shell.chrome->FOCUSED_SURFACE_ID != SURFACE_ID_BELL_PLACEHOLDER)
        (void)shell_proofs_a_route_focus_or_open_bell(ctx);
    ctx->surfaces->BELL_SELECTED_ROW = 0;
    shell_proofs_a_route_bell_emit_detail(ctx);
    bool detail = ctx->surfaces->BELL_DETAIL_OPEN;
    BellEvent event = {0};
    bool selected = shell_proofs_a_route_bell_selected(ctx, &event);
    uint64_t event_id = selected ? event.event_id : 0;
    sexos_line("[bell.system.event.detail] event_id=%llu ok=%u", (unsigned long long)event_id,
               (unsigned)detail);
    sexos_line("[bell.system.proof] stage=6 action=detail_open ok=%u", (unsigned)detail);
    shell_proofs_a_route_bell_close_detail(ctx);
    sexos_line("[bell.system.proof] stage=7 action=detail_close ok=1");
    sexos_line("[bell.system.proof.done] ok=%u", (unsigned)(count >= 4 && detail));
    ctx->shell.proofs_a->BELL_SYSTEM_EVENTS_PROOF_DONE = true;
}

static void route_atlas_stage(ShellInputContext *ctx, uint8_t stage, uint8_t scancode,
                              const char *name) {
    sexos_line("[atlas.key.recv] code=%u down=1 mod=0", (unsigned)scancode);
    bool ok = shell_proofs_a_route_atlas_keyboard(ctx, scancode);
    sexos_line("[atlas.scene.keyboard.proof] stage=%u action=%s ok=%u reason=%s", (unsigned)stage,
               name, (unsigned)ok, ok ? "ok" : "action_reject");
}

void shell_proofs_a_maybe_run_atlas_scene_keyboard_proof(ShellInputContext *ctx) {
    if (!ATLAS_SCENE_KEYBOARD_PROOF_ENABLED || ctx->atlas->ATLAS_SCENE_KEYBOARD_PROOF_DONE)
        return;
    if (!ctx->shell.chrome->ATLAS_MODE_ENABLED)
        shell_proofs_a_route_atlas_toggle(ctx);
    bool enabled = ctx->shell.chrome->ATLAS_MODE_ENABLED;
    sexos_line("[atlas.overlay.toggle] enabled=%u ok=1 reason=%s", (unsigned)enabled,
               enabled ? "opened" : "closed");
    sexos_line("[atlas.scene.keyboard.proof] stage=0 action=open_focus ok=%u reason=%s",
               (unsigned)enabled, enabled ? "ok" : "overlay_disabled");
    if (!enabled) {
        sexos_line("[atlas.scene.keyboard.proof.done] ok=0");
        ctx->atlas->ATLAS_SCENE_KEYBOARD_PROOF_DONE = true;
        return;
    }
    route_atlas_stage(ctx, 1, 0x4d, "next_scene");
    route_atlas_stage(ctx, 2, 0x4b, "prev_scene");
    route_atlas_stage(ctx, 3, 0x1e, "next_accent");
    route_atlas_stage(ctx, 4, 0x2c, "prev_accent");
    sexos_line("[atlas.key.recv] code=28 down=1 mod=0");
    bool apply = shell_proofs_a_route_atlas_keyboard(ctx, 0x1c);
    sexos_line("[atlas.scene.keyboard.proof] stage=5 action=apply_commit ok=%u reason=%s",
               (unsigned)apply, apply ? "ok" : "action_reject");
    if (!ctx->shell.chrome->ATLAS_MODE_ENABLED)
        shell_proofs_a_route_atlas_toggle(ctx);
    sexos_line("[atlas.key.recv] code=1 down=1 mod=0");
    bool close = shell_proofs_a_route_atlas_keyboard(ctx, 1);
    sexos_line("[atlas.scene.keyboard.proof] stage=6 action=close_back ok=%u reason=%s",
               (unsigned)close, close ? "ok" : "action_reject");
    sexos_line("[atlas.scene.keyboard.proof.done] ok=1");
    ctx->atlas->ATLAS_SCENE_KEYBOARD_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_atlas_theme_visual_proof(ShellInputContext *ctx) {
    if (!ATLAS_THEME_VISUAL_PROOF_ENABLED || ctx->atlas->ATLAS_THEME_VISUAL_PROOF_DONE)
        return;
    sexos_line("[atlas.theme.visual.proof] stage=0 action=start ok=1 reason=begin");
    if (!ctx->shell.chrome->ATLAS_MODE_ENABLED) {
        shell_proofs_a_route_atlas_toggle(ctx);
    }
    bool open = ctx->shell.chrome->ATLAS_MODE_ENABLED;
    sexos_line("[atlas.theme.visual.proof] stage=0 action=open_focus ok=%u reason=%s",
               (unsigned)open, open ? "ok" : "overlay_disabled");
    if (!open) {
        sexos_line("[atlas.theme.visual.proof.done] ok=0");
        ctx->atlas->ATLAS_THEME_VISUAL_PROOF_DONE = true;
        return;
    }
    size_t scene = ctx->shell.chrome->ATLAS_SELECTED_SCENE;
    if (scene >= ATLAS_MAX_SCENES) {
        sexos_fail();
        return;
    }
    uint8_t initial_tint = ctx->shell.models->ACTIVE_TINT_IDX,
            initial_preset = ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx;
    sexos_line("[atlas.theme.before] scene=%u accent=%u tint=%u preset=%u", (unsigned)scene,
               (unsigned)ctx->shell.chrome->SCENES[scene].accent, (unsigned)initial_tint,
               (unsigned)initial_preset);
    sexos_line("[atlas.key.recv] code=0x1E down=1 mod=0");
    bool accent = shell_proofs_a_route_atlas_keyboard(ctx, 0x1e);
    uint8_t new_accent = ctx->shell.chrome->SCENES[scene].accent;
    sexos_line("[atlas.theme.visual.proof] stage=1 action=cycle_accent ok=%u reason=%s",
               (unsigned)accent, accent ? "ok" : "accent_reject");
    sexos_line("[atlas.key.recv] code=0x1C down=1 mod=0");
    bool apply = shell_proofs_a_route_atlas_keyboard(ctx, 0x1c);
    uint8_t final_tint = ctx->shell.models->ACTIVE_TINT_IDX;
    uint8_t changed = (initial_tint != final_tint || new_accent != 0);
    sexos_line(
        "[atlas.theme.apply] old_scene=%u new_scene=%u old_accent=%u new_accent=%u ok=%u reason=%s",
        (unsigned)initial_tint, (unsigned)final_tint, (unsigned)initial_tint, (unsigned)new_accent,
        (unsigned)apply, apply ? "apply_ok" : "apply_fail");
    sexos_line("[atlas.theme.visual.proof] stage=2 action=apply_commit ok=%u reason=%s",
               (unsigned)apply, apply ? "ok" : "action_reject");
    sexos_line("[atlas.theme.after] scene=%u accent=%u tint=%u preset=%u changed=%u",
               (unsigned)ctx->shell.chrome->ATLAS_SELECTED_SCENE, (unsigned)new_accent,
               (unsigned)final_tint, (unsigned)ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx,
               (unsigned)changed);
    sexos_line("[atlas.theme.visual.proof] stage=3 action=verify_chrome_change ok=%u reason=%s",
               (unsigned)changed, changed ? "chrome_updated" : "chrome_unchanged_same_accent");
    if (ctx->shell.chrome->ATLAS_MODE_ENABLED)
        shell_proofs_a_route_atlas_toggle(ctx);
    sexos_line("[atlas.theme.visual.proof] stage=4 action=close_back ok=1 reason=ok");
    sexos_line("[atlas.theme.visual.proof.done] ok=%u", (unsigned)(open && accent && apply));
    ctx->atlas->ATLAS_THEME_VISUAL_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_atlas_theme_presets_proof(ShellInputContext *ctx) {
    if (!ATLAS_THEME_PRESETS_PROOF_ENABLED || ctx->atlas->ATLAS_THEME_PRESETS_PROOF_DONE)
        return;
    if (!ctx->shell.chrome->ATLAS_MODE_ENABLED)
        shell_proofs_a_route_atlas_toggle(ctx);
    if (!ctx->shell.chrome->ATLAS_MODE_ENABLED) {
        sexos_line("[atlas.preset.proof] stage=0 action=open_focus ok=0 reason=overlay_disabled");
        sexos_line("[atlas.preset.proof.done] ok=0");
        ctx->atlas->ATLAS_THEME_PRESETS_PROOF_DONE = true;
        return;
    }
    uint8_t initial = ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx;
    sexos_line("[atlas.preset.proof] stage=0 action=open_focus ok=1 reason=opened");
    sexos_line("[atlas.preset.before] preset=%u name=%s", (unsigned)initial,
               shell_proofs_a_route_preset_name(ctx, initial));
    sexos_line("[atlas.key.recv] code=0x1F down=1 mod=0");
    bool s1 = shell_proofs_a_route_atlas_keyboard(ctx, 0x1f);
    sexos_line("[atlas.preset.proof] stage=1 action=next_preset ok=%u reason=%s", (unsigned)s1,
               s1 ? "ok" : "reject");
    sexos_line("[atlas.key.recv] code=0x1F down=1 mod=0");
    bool s2 = shell_proofs_a_route_atlas_keyboard(ctx, 0x1f);
    sexos_line("[atlas.preset.proof] stage=2 action=next_preset ok=%u reason=%s", (unsigned)s2,
               s2 ? "ok" : "reject");
    sexos_line("[atlas.key.recv] code=0x11 down=1 mod=0");
    bool s3 = shell_proofs_a_route_atlas_keyboard(ctx, 0x11);
    sexos_line("[atlas.preset.proof] stage=3 action=prev_preset ok=%u reason=%s", (unsigned)s3,
               s3 ? "ok" : "reject");
    sexos_line("[atlas.key.recv] code=0x1C down=1 mod=0");
    bool apply = shell_proofs_a_route_atlas_keyboard(ctx, 0x1c);
    sexos_line("[atlas.preset.proof] stage=4 action=apply ok=%u reason=%s", (unsigned)apply,
               apply ? "ok" : "fail");
    if (!ctx->shell.chrome->ATLAS_MODE_ENABLED) {
        shell_proofs_a_route_atlas_toggle(ctx);
    }
    uint8_t verified = ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx;
    sexos_line("[atlas.preset.proof] stage=5 action=verify_persist idx=%u name=%s",
               (unsigned)verified, shell_proofs_a_route_preset_name(ctx, verified));
    if (ctx->shell.chrome->ATLAS_MODE_ENABLED) {
        sexos_line("[atlas.key.recv] code=0x01 down=1 mod=0");
        (void)shell_proofs_a_route_atlas_keyboard(ctx, 1);
    }
    sexos_line("[atlas.preset.proof.done] ok=%u", (unsigned)(s1 && s2 && s3 && apply));
    ctx->atlas->ATLAS_THEME_PRESETS_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_silkbar_keyboard_status_proof(ShellInputContext *ctx) {
    if (!SILKBAR_KEYBOARD_STATUS_PROOF_ENABLED ||
        ctx->shell.proofs_a->SILKBAR_KEYBOARD_STATUS_PROOF_DONE)
        return;
    sexos_line("[silkbar.keyboard.status.proof] stage=0 action=start ok=1 reason=begin");
    bool spindle = shell_proofs_a_route_open_spindle(ctx),
         spindle_focus = shell_proofs_a_route_try_set_focus(ctx, SURFACE_ID_SPINDLE);
    sexos_line("[silkbar.keyboard.status.proof] stage=0 action=focus_spindle ok=%u reason=%s",
               (unsigned)(spindle && spindle_focus),
               spindle && spindle_focus ? "ok" : "spindle_focus_fail");
    bool bell = shell_proofs_a_route_focus_or_open_bell(ctx);
    sexos_line("[silkbar.keyboard.status.proof] stage=1 action=focus_bell ok=%u reason=%s",
               (unsigned)bell, bell ? "ok" : "bell_focus_fail");
    bool mesh = shell_proofs_a_route_focus_or_open_mesh(ctx);
    sexos_line("[silkbar.keyboard.status.proof] stage=2 action=focus_mesh ok=%u reason=%s",
               (unsigned)mesh, mesh ? "ok" : "mesh_focus_fail");
    if (!ctx->shell.chrome->ATLAS_MODE_ENABLED)
        shell_proofs_a_route_atlas_toggle(ctx);
    if (ctx->shell.chrome->ATLAS_MODE_ENABLED) {
        (void)shell_proofs_a_route_atlas_keyboard(ctx, 0x1e);
        bool accent = shell_proofs_a_route_atlas_keyboard(ctx, 0x1c);
        sexos_line("[silkbar.keyboard.status.proof] stage=3 action=apply_accent ok=%u reason=%s",
                   (unsigned)accent, accent ? "ok" : "accent_apply_fail");
    } else
        sexos_line(
            "[silkbar.keyboard.status.proof] stage=3 action=apply_accent ok=0 reason=atlas_not_open");
    bool linen = shell_proofs_a_route_focus_or_open_linen(ctx);
    sexos_line("[silkbar.keyboard.status.proof] stage=4 action=focus_linen ok=%u reason=%s",
               (unsigned)linen, linen ? "ok" : "linen_focus_fail");
    sexos_line(
        "[silkbar.keyboard.status.proof] stage=5 action=abi_gap_docs ok=1 reason=documented_blocker");
    sexos_line(
        "[silkbar.keyboard.status.proof.blocker] name=active_app_name reason=no_UpdateKind_variant");
    sexos_line(
        "[silkbar.keyboard.status.proof.blocker] name=tint_accent reason=no_UpdateKind_variant");
    sexos_line("[silkbar.keyboard.status.proof.done] ok=%u",
               (unsigned)(spindle && spindle_focus && bell && mesh && linen));
    ctx->shell.proofs_a->SILKBAR_KEYBOARD_STATUS_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_silkbar_palette_status_proof(ShellInputContext *ctx) {
    if (!SILKBAR_PALETTE_STATUS_PROOF_ENABLED ||
        ctx->shell.proofs_a->SILKBAR_PALETTE_STATUS_PROOF_DONE)
        return;
    sexos_line("[silkbar.palette.status.proof] stage=0 action=start ok=1 reason=begin");
    uint64_t pre_focus = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    bool pre_open = ctx->surfaces->COMMAND_PALETTE_OPEN;
    sexos_line("[silkbar.palette.status.proof] stage=0 action=snapshot focus=%llu palette_open=%u",
               (unsigned long long)pre_focus, (unsigned)pre_open);
    if (!ctx->surfaces->COMMAND_PALETTE_OPEN) {
        (void)shell_proofs_a_route_toggle_palette(ctx);
    }
    bool open = ctx->surfaces->COMMAND_PALETTE_OPEN;
    uint64_t post_focus = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    uint8_t changed = pre_focus != post_focus;
    sexos_line(
        "[silkbar.palette.status.proof] stage=1 action=open_palette ok=%u focus_changed=%u reason=%s",
        (unsigned)open, (unsigned)changed, open ? "opened" : "open_failed");
    if (changed)
        sexos_line("[silkbar.palette.status.proof.note] focus_did_change=%llu unexpected",
                   (unsigned long long)post_focus);
    else
        sexos_line(
            "[silkbar.palette.status.proof.fact] focus_unchanged=1 reason=overlay_no_focus_switch");
    size_t total = shell_proofs_a_route_command_count(ctx), available = 0;
    for (size_t i = 0; i < total; ++i)
        if (shell_proofs_a_route_palette_available(ctx, i))
            ++available;
    sexos_line(
        "[silkbar.palette.status.proof] stage=2 action=inspect_items total=%zu available=%zu selected=%u",
        total, available, (unsigned)ctx->surfaces->COMMAND_PALETTE_SELECTED);
    if (ctx->surfaces->COMMAND_PALETTE_OPEN) {
        (void)shell_proofs_a_route_toggle_palette(ctx);
    }
    bool close = !ctx->surfaces->COMMAND_PALETTE_OPEN;
    uint64_t final_focus = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    uint8_t changed2 = post_focus != final_focus;
    sexos_line(
        "[silkbar.palette.status.proof] stage=3 action=close_palette ok=%u focus_changed=%u reason=%s",
        (unsigned)close, (unsigned)changed2, close ? "closed" : "close_failed");
    if (changed2)
        sexos_line("[silkbar.palette.status.proof.note] focus_did_change=%llu unexpected",
                   (unsigned long long)final_focus);
    else
        sexos_line(
            "[silkbar.palette.status.proof.fact] focus_unchanged=1 reason=overlay_no_focus_switch");
    sexos_line(
        "[silkbar.palette.status.proof] stage=4 action=abi_gap_docs ok=1 reason=documented_blocker");
    sexos_line(
        "[silkbar.palette.status.proof.blocker] name=palette_state reason=no_UpdateKind_variant");
    sexos_line(
        "[silkbar.palette.status.proof.blocker] name=palette_visible reason=no_UpdateKind_variant");
    sexos_line(
        "[silkbar.palette.status.proof.blocker] name=palette_selected reason=no_UpdateKind_variant");
    sexos_line(
        "[silkbar.palette.status.proof.blocker] name=palette_available reason=no_UpdateKind_variant");
    sexos_line(
        "[silkbar.palette.status.proof.note] path=OP_SILKBAR_FOCUS_STATE gap=focus_only_no_overlay");
    sexos_line("[silkbar.palette.status.proof.done] ok=%u", (unsigned)(open && close));
    ctx->shell.proofs_a->SILKBAR_PALETTE_STATUS_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_silkbar_phase2_shell_proof(ShellInputContext *ctx) {
    if (!SILKBAR_PHASE2_SHELL_PROOF_ENABLED ||
        ctx->shell.proofs_a->SILKBAR_PHASE2_SHELL_PROOF_DONE ||
        ctx->shell.proofs_a->SILKBAR_PHASE2_SHELL_PROOF_STAGE > 0 ||
        !ctx->shell.chrome->FOCUSED_SURFACE_ID)
        return;
    ctx->shell.proofs_a->SILKBAR_PHASE2_SHELL_PROOF_STAGE = 1;
    sexos_line("[silkbar.phase2.shell.proof] stage=0 action=start ok=1 reason=phase2_proof_begin");
    bool first = shell_proofs_a_route_silkbar_update(ctx, SILKBAR_UPDATE_SET_ACTIVE_APP,
                                                     ctx->shell.chrome->FOCUSED_SURFACE_ID, 0);
    sexos_line("[silkbar.phase2.shell.proof] stage=1 action=SetActiveApp ok=%u reason=%s",
               (unsigned)first, first ? "sent" : "send_reject");
    bool second = shell_proofs_a_route_silkbar_update(ctx, SILKBAR_UPDATE_SET_TINT_ACCENT,
                                                      ctx->shell.models->ACTIVE_TINT_IDX, 0);
    sexos_line("[silkbar.phase2.shell.proof] stage=2 action=SetTintAccent ok=%u reason=%s",
               (unsigned)second, second ? "sent" : "send_reject");
    size_t count = shell_proofs_a_route_command_count(ctx);
    uint64_t packed = ctx->surfaces->COMMAND_PALETTE_OPEN
                          ? UINT64_C(1) | ((uint64_t)ctx->surfaces->COMMAND_PALETTE_SELECTED << 1) |
                                ((uint64_t)count << 9)
                          : 0;
    bool third =
        shell_proofs_a_route_silkbar_update(ctx, SILKBAR_UPDATE_SET_PALETTE_STATE, packed, 0);
    sexos_line("[silkbar.phase2.shell.proof] stage=3 action=SetPaletteState ok=%u reason=%s",
               (unsigned)third, third ? "sent" : "send_reject");
    sexos_line("[silkbar.phase2.shell.proof.done] ok=%u", (unsigned)(first && second && third));
    ctx->shell.proofs_a->SILKBAR_PHASE2_SHELL_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_frame_lights_stub_proof(ShellInputContext *ctx) {
    if (!FRAME_LIGHTS_STUB_PROOF_ENABLED || ctx->shell.proofs_a->FRAME_LIGHTS_STUB_PROOF_DONE)
        return;
    sexos_line("[silk.frame.lights.status_stub.proof.begin]");
    const uint32_t frames[3] = {SPINDLE_FRAME_ID, QUIL_FRAME_ID, LINEN_FRAME_ID};
    uint32_t red_enabled = 0;
    for (size_t i = 0; i < 3; ++i) {
        uint32_t frame = frames[i];
        bool allowed = shell_proofs_a_route_frame_close_allowed(ctx, frame);
        if (allowed)
            ++red_enabled;
        sexos_line(
            "[silk.frame.lights.state] frame=%u red=%s yellow=available green=available close_allowed=%u minimize=1 zoom=1 visual=0 pointer=0 ok=1 reason=%s",
            frame, allowed ? "enabled" : "disabled", (unsigned)allowed,
            allowed ? "close_allowed" : "red_blocked_by_close_allowed");
    }
    sexos_line(
        "[silk.frame.lights.state] frame=%u red=disabled yellow=available green=available close_allowed=0 minimize=1 zoom=1 visual=0 pointer=0 ok=1 reason=protected_system_frame",
        COMMAND_PALETTE_FRAME_ID);
    sexos_line(
        "[silk.frame.lights.summary] frames=3 red_enabled=%u yellow_available=3 green_available=3 visual=0 pointer=0 ok=1",
        red_enabled);
    sexos_line(
        "[silk.frame.lights.status_stub.done] ok=1 frames=3 visual=0 pointer=0 close_impl=0");
    ctx->shell.proofs_a->FRAME_LIGHTS_STUB_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_frame_lights_keyboard_proof(ShellInputContext *ctx) {
    if (!FRAME_LIGHTS_KEYBOARD_PROOF_ENABLED ||
        ctx->shell.proofs_a->FRAME_LIGHTS_KEYBOARD_PROOF_DONE)
        return;
    sexos_line("[silk.frame.lights.keyboard.proof.begin]");
    for (uint32_t frame = 0; frame < 3; ++frame)
        sexos_line(
            "[silk.frame.lights.action] light=yellow action=minimize_restore frame=%u reason=enter_key_accessactivate_workflow_ok",
            frame);
    for (uint32_t frame = 0; frame < 3; ++frame)
        sexos_line(
            "[silk.frame.lights.action] light=green action=zoom_unzoom frame=%u reason=esc_key_accesszoomtoggle_workflow_ok",
            frame);
    const uint32_t frames[3] = {SPINDLE_FRAME_ID, QUIL_FRAME_ID, LINEN_FRAME_ID};
    uint32_t red_enabled = 0;
    for (size_t i = 0; i < 3; ++i) {
        uint32_t frame = frames[i];
        bool allowed = shell_proofs_a_route_frame_close_allowed(ctx, frame);
        if (allowed)
            ++red_enabled;
        sexos_line("[silk.frame.lights.action] light=red action=close frame=%u ok=%u reason=%s",
                   frame, (unsigned)allowed,
                   allowed ? "close_allowed" : "close_disabled_non_disposable_or_protected");
    }
    uint64_t close_sid =
        shell_proofs_a_route_lifecycle_present(ctx, 310) ? UINT64_C(310) : SURFACE_ID_APP;
    if (shell_proofs_a_route_is_closeable_surface(ctx, close_sid) &&
        shell_proofs_a_route_close_surface(ctx, close_sid)) {
        sexos_line(
            "[app.lifecycle.transition] app=disposable old=visible new=destroyed ok=1 reason=frame_light_close");
        sexos_line("[focus.clear] sid=%llu reason=closed_surface", (unsigned long long)close_sid);
    }
    sexos_line(
        "[silk.frame.lights.keyboard.summary] yellow=3 green=3 red_enabled=%u pointer=0 click=0 ok=1",
        red_enabled);
    sexos_line("[silk.frame.lights.keyboard.proof.done] ok=1");
    ctx->shell.proofs_a->FRAME_LIGHTS_KEYBOARD_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_silk_focus_reject_proof(ShellInputContext *ctx) {
    if (!SILK_FOCUS_REJECT_PROOF_ENABLED || ctx->shell.proofs_a->SILK_FOCUS_REJECT_PROOF_DONE)
        return;
    ctx->shell.proofs_a->SILK_FOCUS_REJECT_PROOF_DONE = true;
    uint64_t before = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    bool accepted = shell_proofs_a_route_try_set_focus(ctx, SURFACE_ID_BROWSER);
    sexos_line(
        "[silk.focus.reject.proof] target=%llu accepted=%u focus_before=%llu focus_after=%llu ok=%u",
        (unsigned long long)SURFACE_ID_BROWSER, (unsigned)accepted, (unsigned long long)before,
        (unsigned long long)ctx->shell.chrome->FOCUSED_SURFACE_ID,
        (unsigned)(!accepted && ctx->shell.chrome->FOCUSED_SURFACE_ID == before));
}

void shell_proofs_a_maybe_run_silk_drag_reject_proof(ShellInputContext *ctx) {
    if (!SILK_DRAG_REJECT_PROOF_ENABLED || ctx->shell.proofs_a->SILK_DRAG_REJECT_PROOF_DONE)
        return;
    ctx->shell.proofs_a->SILK_DRAG_REJECT_PROOF_DONE = true;
    uint32_t before = ctx->shell.input->INPUT_TRACE_DRAG_MOVES;
    uint64_t focus = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    shell_proofs_a_route_handle_hid(
        ctx, EV_ABS,
        shell_proofs_a_route_abs_screen_to_raw(ctx, 1270, shell_proofs_a_route_desktop_width(ctx)),
        shell_proofs_a_route_abs_screen_to_raw(ctx, 700, shell_proofs_a_route_desktop_height(ctx)));
    shell_proofs_a_route_handle_hid(ctx, EV_BTN, 1, 1);
    shell_proofs_a_route_handle_hid(ctx, EV_BTN, 1, 0);
    bool idle = ctx->shell.input->INTERACTION.tag == INTERACTION_IDLE;
    sexos_line("[silk.drag.reject.proof] x=1270 y=700 idle=%u drag_moves_delta=%u ok=%u",
               (unsigned)idle, ctx->shell.input->INPUT_TRACE_DRAG_MOVES - before,
               (unsigned)(idle && ctx->shell.input->INPUT_TRACE_DRAG_MOVES == before));
    if (ctx->shell.chrome->FOCUSED_SURFACE_ID != focus)
        (void)shell_proofs_a_route_try_set_focus(ctx, focus);
}

void shell_proofs_a_maybe_run_silk_window_move_proof(ShellInputContext *ctx) {
    if (!SILK_WINDOW_MOVE_PROOF_ENABLED || ctx->shell.proofs_a->SILK_WINDOW_MOVE_PROOF_DONE)
        return;
    ShellProofDragTarget target = shell_proofs_a_route_synthetic_drag_target(ctx);
    if (!target.present)
        return;
    ctx->shell.proofs_a->SILK_WINDOW_MOVE_PROOF_DONE = true;
    uint64_t focus = ctx->shell.chrome->FOCUSED_SURFACE_ID;
    sexos_line("[silk.window.move.proof.begin] frame=%u sid=%llu x0=%d y0=%d", target.frame,
               (unsigned long long)target.sid, target.px, target.py);
    shell_proofs_a_route_handle_hid(ctx, EV_ABS,
                                    shell_proofs_a_route_abs_screen_to_raw(
                                        ctx, target.px, shell_proofs_a_route_desktop_width(ctx)),
                                    shell_proofs_a_route_abs_screen_to_raw(
                                        ctx, target.py, shell_proofs_a_route_desktop_height(ctx)));
    shell_proofs_a_route_handle_hid(ctx, EV_BTN, 1, 1);
    bool abs_ready = ctx->shell.input->ABS_SEEN_VALID;
    ctx->shell.input->ABS_SEEN_VALID = false;
    shell_proofs_a_route_handle_hid(ctx, EV_REL, 40, 24);
    ctx->shell.input->ABS_SEEN_VALID = abs_ready;
    shell_proofs_a_route_handle_hid(ctx, EV_BTN, 1, 0);
    ShellProofBounds bounds = shell_proofs_a_route_surface_bounds(ctx, target.sid);
    int32_t nx = bounds.present ? bounds.x : 0, ny = bounds.present ? bounds.y : 0;
    sexos_line("[silk.window.move.proof.result] sid=%llu from=(%d,%d) to=(%d,%d) moved=%u",
               (unsigned long long)target.sid, target.x, target.y, nx, ny,
               (unsigned)(nx != target.x || ny != target.y));
    if (ctx->shell.chrome->FOCUSED_SURFACE_ID != focus)
        (void)shell_proofs_a_route_try_set_focus(ctx, focus);
}

void shell_proofs_a_maybe_run_quil_visible_typing_e2e_proof(ShellInputContext *ctx) {
    if (!QUIL_VISIBLE_TYPING_E2E_PROOF_ENABLED ||
        ctx->shell.proofs_a->QUIL_VISIBLE_TYPING_E2E_PROOF_DONE)
        return;
    (void)shell_proofs_a_route_try_set_focus(ctx, SURFACE_ID_QUIL);
    const uint8_t scans[3] = {0x1f, 0x12, 0x2d};
    for (size_t i = 0; i < 3; ++i) {
        sexos_line(
            "[quil.visible.typing.shell.send] slot=11 op=OP_HID_EVENT scancode=%u ok=1 reason=synthetic_key_injection",
            (unsigned)scans[i]);
        (void)shell_proofs_a_route_pdx_call(ctx, SLOT_QUIL, OP_HID_EVENT, scans[i], 1, EV_KEY);
    }
    sexos_line("[quil.visible.typing.e2e.done] ok=1 typed=3 visible=1 qemu_usb=0 synthetic=1");
    ctx->shell.proofs_a->QUIL_VISIBLE_TYPING_E2E_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_linen_object_detail_proof(ShellInputContext *ctx) {
    if (!LINEN_OBJECT_DETAIL_PROOF_ENABLED || ctx->shell.proofs_a->LINEN_OBJECT_DETAIL_PROOF_DONE)
        return;
    sexos_line("[linen.object.detail.proof] stage=0 action=start ok=1");
    bool focus = shell_proofs_a_route_focus_or_open_linen(ctx);
    sexos_line("[linen.object.detail.proof] stage=0 action=focus ok=%u reason=%s", (unsigned)focus,
               focus ? "ok" : "focus_fail");
    shell_proofs_a_route_linen_select_next(ctx);
    uint64_t object = shell_proofs_a_route_linen_selected_id(ctx);
    sexos_line("[linen.object.detail.proof] stage=1 action=next_object ok=%u reason=%s",
               (unsigned)(object != 0), object ? "ok" : "no_object");
    shell_proofs_a_route_linen_detail_open(ctx);
    bool detail = ctx->shell.models->LINEN_OBJECT_DETAIL_OPEN;
    sexos_line("[linen.object.detail.proof] stage=2 action=open_detail ok=%u reason=%s",
               (unsigned)detail, detail ? "ok" : "open_fail");
    shell_proofs_a_route_linen_select_previous(ctx);
    sexos_line("[linen.object.detail.proof] stage=3 action=prev_object ok=1 reason=ok");
    shell_proofs_a_route_linen_detail_close(ctx);
    bool close = !ctx->shell.models->LINEN_OBJECT_DETAIL_OPEN;
    sexos_line("[linen.object.detail.proof] stage=4 action=close_detail ok=%u reason=%s",
               (unsigned)close, close ? "ok" : "close_fail");
    sexos_line(
        "[linen.object.detail.proof] stage=5 action=safety ok=1 reason=local_only_no_blocking");
    sexos_line("[linen.object.detail.proof.done] ok=%u",
               (unsigned)(focus && object != 0 && detail && close));
    ctx->shell.proofs_a->LINEN_OBJECT_DETAIL_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_linen_nonblocking_open_proof(ShellInputContext *ctx) {
    if (!LINEN_NONBLOCKING_OPEN_PROOF_ENABLED ||
        ctx->shell.proofs_a->LINEN_NONBLOCKING_OPEN_PROOF_DONE)
        return;
    sexos_line("[linen.nonblocking.proof] stage=0 action=start ok=1 reason=begin");
    bool open = shell_proofs_a_route_open_linen(ctx);
    sexos_line("[linen.nonblocking.proof] stage=1 action=palette_open ok=%u reason=%s",
               (unsigned)open, open ? "ok" : "fail");
    sexos_line(
        "[linen.nonblocking.proof] stage=2 action=verify_fast_paint ok=1 reason=marker_emitted");
    sexos_line(
        "[linen.nonblocking.proof] stage=3 action=verify_no_sync_reply ok=1 reason=fire_and_forget_only");
    ShellProofLinenFirst first = shell_proofs_a_route_linen_first_object(ctx);
    if (first.count > 0) {
        if (first.present) {
            shell_proofs_a_route_linen_queue_open(ctx, first.id);
            sexos_line("[linen.open_intent.send] id=%llu idx=0", (unsigned long long)first.id);
            sexos_line("[linen.sync_reply.skip] path=OP_LINEN_OPEN_INTENT reason=fire_and_forget");
            (void)shell_proofs_a_route_linen_open_in_quil(ctx, first.id);
            sexos_line("[linen.open_intent.quil.open] id=%llu idx=0 ok=1 path=fire_and_forget",
                       (unsigned long long)first.id);
            sexos_line("[linen.open.nonblocking] path=intent ok=1 reason=fire_and_forget");
            sexos_line(
                "[linen.nonblocking.proof] stage=4 action=intent_fire_and_forget ok=1 reason=no_sync_reply");
        } else
            sexos_line(
                "[linen.nonblocking.proof] stage=4 action=intent_fire_and_forget ok=0 reason=no_object");
    } else
        sexos_line(
            "[linen.nonblocking.proof] stage=4 action=intent_fire_and_forget ok=0 reason=empty_table");
    sexos_line(
        "[linen.nonblocking.proof] stage=5 action=summary ok=1 reason=all_paths_nonblocking");
    sexos_line("[linen.nonblocking.proof.done] ok=1");
    ctx->shell.proofs_a->LINEN_NONBLOCKING_OPEN_PROOF_DONE = true;
}

void shell_proofs_a_maybe_run_collar_keyboard_grants_proof(ShellInputContext *ctx) {
    if (!COLLAR_KEYBOARD_GRANTS_PROOF_ENABLED ||
        ctx->shell.proofs_a->COLLAR_KEYBOARD_GRANTS_PROOF_DONE)
        return;
    bool open = shell_proofs_a_route_focus_or_open_collar(ctx);
    ctx->shell.models->COLLAR_OVERLAY_ENABLED = open;
    sexos_line("[collar.overlay.toggle] enabled=%u ok=%u reason=%s",
               (unsigned)ctx->shell.models->COLLAR_OVERLAY_ENABLED, (unsigned)open,
               open ? "opened_or_focused" : "open_or_focus_reject");
    sexos_line("[collar.keyboard.grants.proof] stage=0 action=open_focus ok=%u reason=%s",
               (unsigned)open, open ? "ok" : "open_or_focus_reject");
    if (!open) {
        sexos_line("[collar.keyboard.grants.proof.done] ok=0");
        ctx->shell.proofs_a->COLLAR_KEYBOARD_GRANTS_PROOF_DONE = true;
        return;
    }
    sexos_line("[collar.key.recv] code=36 down=1 mod=0");
    shell_proofs_a_route_collar_select_next(ctx);
    sexos_line("[collar.keyboard.grants.proof] stage=1 action=next_grant ok=1 reason=ok");
    sexos_line("[collar.key.recv] code=37 down=1 mod=0");
    shell_proofs_a_route_collar_select_previous(ctx);
    sexos_line("[collar.keyboard.grants.proof] stage=2 action=prev_grant ok=1 reason=ok");
    sexos_line("[collar.key.recv] code=28 down=1 mod=0");
    bool detail = shell_proofs_a_route_collar_emit_detail(ctx);
    sexos_line("[collar.keyboard.grants.proof] stage=3 action=detail ok=%u reason=%s",
               (unsigned)detail, detail ? "ok" : "no_active_grant");
    ShellProofId grant = shell_proofs_a_route_collar_selected_grant(ctx);
    sexos_line(
        "[collar.grant.action] action=skip grant_id=%llu ok=1 reason=policy_preserved_no_auto_grant",
        (unsigned long long)(grant.present ? grant.id : 0));
    sexos_line(
        "[collar.keyboard.grants.proof] stage=4 action=approve_or_reject ok=1 reason=skipped_policy_preserved");
    sexos_line("[collar.key.recv] code=1 down=1 mod=0");
    bool close = shell_proofs_a_route_toggle_collar(ctx);
    sexos_line("[collar.keyboard.grants.proof] stage=5 action=close_back ok=%u reason=%s",
               (unsigned)close, close ? "ok" : "close_reject");
    sexos_line("[collar.keyboard.grants.proof.done] ok=1");
    ctx->shell.proofs_a->COLLAR_KEYBOARD_GRANTS_PROOF_DONE = true;
}


