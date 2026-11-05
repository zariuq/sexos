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

#include "silk_shell_atlas.h"
void launcher_focus_discard(void);
bool shell_custody_has_surface(uint64_t sid, bool terminal_only);
int32_t desktop_height(void);
int32_t desktop_width(void);

static ShellAtlasContext atlas_context(void) {
    return (ShellAtlasContext){&SILK_SHELL_STATE.atlas, &SILK_SHELL_STATE.chrome,
                               &SILK_SHELL_STATE.models};
}

static void atlas_binding(const ShellAtlasContext *ctx) {
    if (ctx && ctx->atlas == &SILK_SHELL_STATE.atlas && ctx->chrome == &SILK_SHELL_STATE.chrome &&
        ctx->models == &SILK_SHELL_STATE.models)
        return;
    sexos_fail();
    for (;;) {
    }
}

void collect_atlas_snapshot(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_collect_atlas_snapshot(&ctx);
}

bool surface_in_active_scene(uint64_t sid) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_surface_in_active_scene(&ctx, sid);
}

bool surface_scene_id(uint64_t sid, uint8_t *out_scene_id) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_surface_scene_id(&ctx, sid, out_scene_id);
}

void clear_focus_if_wrong_scene(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_clear_focus_if_wrong_scene(&ctx);
}

void sync_lifecycle_scene_visibility(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_sync_lifecycle_scene_visibility(&ctx);
}

void sync_scene_visibility(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_sync_scene_visibility(&ctx);
}

void atlas_default_label(uint32_t scene_id, uint8_t *out_label) {
    ShellAtlasContext ctx = atlas_context();
    ShellAtlasLabel value = shell_atlas_atlas_default_label(&ctx, scene_id);
    if (out_label)
        for (size_t i = 0; i < ATLAS_LABEL_LEN; ++i)
            out_label[i] = value.bytes[i];
}

void scene_init_all(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_scene_init_all(&ctx);
}

void scene_update_flags(uint8_t scene_idx) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_scene_update_flags(&ctx, scene_idx);
}

bool validate_scene_id(uint8_t scene_id) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_validate_scene_id(&ctx, scene_id);
}

uint8_t scene_accent_token(uint8_t scene_id) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_scene_accent_token(&ctx, scene_id);
}

bool scene_is_pinned(uint8_t scene_id) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_scene_is_pinned(&ctx, scene_id);
}

void scene_label_token(uint8_t scene_id, uint8_t *out_label) {
    ShellAtlasContext ctx = atlas_context();
    ShellAtlasLabel value = shell_atlas_scene_label_token(&ctx, scene_id);
    if (out_label)
        for (size_t i = 0; i < ATLAS_LABEL_LEN; ++i)
            out_label[i] = value.bytes[i];
}

void atlas_capture_snapshot(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_atlas_capture_snapshot(&ctx);
}

bool atlas_is_enabled(void) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_atlas_is_enabled(&ctx);
}

void atlas_toggle(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_atlas_toggle(&ctx);
}

void atlas_exit(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_atlas_exit(&ctx);
}

void atlas_card_pos(size_t scene_idx, uint32_t cw, int32_t *out_x, int32_t *out_y, uint32_t *out_w,
                    uint32_t *out_h) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_atlas_card_pos(&ctx, scene_idx, cw, out_x, out_y, out_w, out_h);
}

bool atlas_scene_at_point(int32_t px, int32_t py, uint8_t *out_scene_id) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_atlas_scene_at_point(&ctx, px, py, out_scene_id);
}

bool handle_atlas_keyboard(uint8_t scancode) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_handle_atlas_keyboard(&ctx, scancode);
}

void atlas_render_stub(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_atlas_render_stub(&ctx);
}

void atlas_clear_stub(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_atlas_clear_stub(&ctx);
}

void atlas_apply_scene_accent_to_chrome(uint8_t scene_idx) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_atlas_apply_scene_accent_to_chrome(&ctx, scene_idx);
}

void switch_scene(uint8_t scene_idx) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_switch_scene(&ctx, scene_idx);
}

void next_scene(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_next_scene(&ctx);
}

void prev_scene(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_prev_scene(&ctx);
}

bool atlas_cycle_scene(int32_t delta, const char *reason) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_atlas_cycle_scene(&ctx, delta, reason);
}

bool next_frame_in_scene(uint32_t start_frame_id, bool forward, uint32_t *out_frame_id) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_next_frame_in_scene(&ctx, start_frame_id, forward, out_frame_id);
}

void focus_next_frame(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_focus_next_frame(&ctx);
}

void focus_prev_frame(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_focus_prev_frame(&ctx);
}

void focus_next_tab(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_focus_next_tab(&ctx);
}

void focus_prev_tab(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_focus_prev_tab(&ctx);
}

bool toggle_minimize_focused_frame(void) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_toggle_minimize_focused_frame(&ctx);
}

bool toggle_zoom_focused_frame(void) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_toggle_zoom_focused_frame(&ctx);
}

bool close_focused_tab_or_frame_safe(void) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_close_focused_tab_or_frame_safe(&ctx);
}

bool atlas_same_scene_drop_noop(uint32_t frame_id, uint8_t source_scene, uint8_t target_scene) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_atlas_same_scene_drop_noop(&ctx, frame_id, source_scene, target_scene);
}

bool reparent_frame_to_scene(uint32_t frame_id, uint8_t target_scene, const char *reason) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_reparent_frame_to_scene(&ctx, frame_id, target_scene, reason);
}

bool atlas_pointer_drag_begin_at(int32_t px, int32_t py) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_atlas_pointer_drag_begin_at(&ctx, px, py);
}

bool atlas_pointer_drop_at(int32_t px, int32_t py) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_atlas_pointer_drop_at(&ctx, px, py);
}

bool atlas_pointer_drag_cancel(const char *reason) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_atlas_pointer_drag_cancel(&ctx, reason);
}

uint8_t snap_compute_checksum(const SceneLayoutSnapshot *snap) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_snap_compute_checksum(&ctx, snap);
}

void snap_capture_layout(void) {
    ShellAtlasContext ctx = atlas_context();
    shell_atlas_snap_capture_layout(&ctx);
}

bool snap_validate(const SceneLayoutSnapshot *snap) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_snap_validate(&ctx, snap);
}

bool snap_restore_layout(void) {
    ShellAtlasContext ctx = atlas_context();
    return shell_atlas_snap_restore_layout(&ctx);
}

bool shell_atlas_peer_active_surface_for_frame(ShellAtlasContext *ctx, uint32_t frame,
                                               uint64_t *out) {
    atlas_binding(ctx);
    return active_surface_for_frame(frame, out);
}

bool shell_atlas_peer_surface_is_alive(ShellAtlasContext *ctx, uint64_t sid) {
    atlas_binding(ctx);
    return surface_is_alive(sid);
}

bool shell_atlas_peer_surface_is_lifecycle_focusable(ShellAtlasContext *ctx, uint64_t sid) {
    atlas_binding(ctx);
    return surface_is_lifecycle_focusable(sid);
}

bool shell_atlas_peer_is_tombstoned(ShellAtlasContext *ctx, uint64_t sid) {
    atlas_binding(ctx);
    return is_tombstoned(sid);
}

bool shell_atlas_peer_try_set_focus(ShellAtlasContext *ctx, uint64_t sid) {
    atlas_binding(ctx);
    return try_set_focus(sid);
}

bool shell_atlas_peer_lifecycle_state(ShellAtlasContext *ctx, uint64_t sid, uint8_t *out) {
    atlas_binding(ctx);
    return lifecycle_state(sid, out);
}

bool shell_atlas_peer_set_lifecycle_state(ShellAtlasContext *ctx, uint64_t sid, uint8_t state) {
    atlas_binding(ctx);
    return set_lifecycle_state(sid, state);
}

bool shell_atlas_peer_get_surface_bounds(ShellAtlasContext *ctx, uint64_t sid, int32_t *x,
                                         int32_t *y, uint32_t *w, uint32_t *h) {
    atlas_binding(ctx);
    return get_surface_bounds(sid, x, y, w, h);
}

bool shell_atlas_peer_make_focus_ref(ShellAtlasContext *ctx, uint64_t sid, FocusRef *out) {
    atlas_binding(ctx);
    return make_focus_ref(sid, out);
}

bool shell_atlas_peer_focus_ref_is_current(ShellAtlasContext *ctx, const FocusRef *ref) {
    atlas_binding(ctx);
    return focus_ref_is_current(ref);
}

bool shell_atlas_peer_selected_frame_id(ShellAtlasContext *ctx, uint32_t *out) {
    atlas_binding(ctx);
    return selected_frame_id(out);
}

void shell_atlas_peer_clear_focus_if_dead(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    clear_focus_if_dead();
}

void shell_atlas_peer_clear_drag_if_dead(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    clear_drag_if_dead();
}

void shell_atlas_peer_clear_drag_if_wrong_scene(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    clear_drag_if_wrong_scene();
}

void shell_atlas_peer_clear_hover_if_dead(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    clear_hover_if_dead();
}

void shell_atlas_peer_clear_hover_if_wrong_scene(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    clear_hover_if_wrong_scene();
}

void shell_atlas_peer_launcher_focus_discard(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    launcher_focus_discard();
}

void shell_atlas_peer_tile_active_scene_frames(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    tile_active_scene_frames();
}

void shell_atlas_peer_cycle_scene_render_token_preset(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    cycle_scene_render_token_preset();
}

void shell_atlas_peer_cycle_prev_scene_render_token_preset(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    cycle_prev_scene_render_token_preset();
}

const char *shell_atlas_peer_get_preset_name(ShellAtlasContext *ctx, uint8_t idx) {
    atlas_binding(ctx);
    return get_preset_name(idx);
}

void shell_atlas_peer_apply_custom_tint_bundle(ShellAtlasContext *ctx, size_t idx) {
    atlas_binding(ctx);
    apply_custom_tint_bundle(idx);
}

void shell_atlas_peer_resolve_scene_render_tokens(ShellAtlasContext *ctx, uint32_t *out) {
    atlas_binding(ctx);
    resolve_scene_render_tokens(out);
}

void shell_atlas_peer_push_token_preset(ShellAtlasContext *ctx, const uint32_t *tokens) {
    atlas_binding(ctx);
    push_token_preset(tokens);
}

bool shell_atlas_peer_send_silkbar_phase2_update(ShellAtlasContext *ctx, uint32_t kind, uint64_t a,
                                                 uint64_t b) {
    atlas_binding(ctx);
    return send_silkbar_phase2_update(kind, a, b);
}

size_t shell_atlas_peer_bell_ring_count(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    return bell_ring_count();
}

bool shell_atlas_peer_frame_is_minimized(ShellAtlasContext *ctx, uint32_t id) {
    atlas_binding(ctx);
    return frame_is_minimized(id);
}

bool shell_atlas_peer_minimize_frame(ShellAtlasContext *ctx, uint32_t id) {
    atlas_binding(ctx);
    return minimize_frame(id);
}

bool shell_atlas_peer_restore_minimized_frame(ShellAtlasContext *ctx, uint32_t id) {
    atlas_binding(ctx);
    return restore_minimized_frame(id);
}

bool shell_atlas_peer_toggle_zoom_frame(ShellAtlasContext *ctx, uint32_t id) {
    atlas_binding(ctx);
    return toggle_zoom_frame(id);
}

bool shell_atlas_peer_switch_to_tab(ShellAtlasContext *ctx, uint32_t frame, uint32_t tab) {
    atlas_binding(ctx);
    return switch_to_tab(frame, tab);
}

bool shell_atlas_peer_is_closeable_surface(ShellAtlasContext *ctx, uint64_t sid) {
    atlas_binding(ctx);
    return is_closeable_surface(sid);
}

bool shell_atlas_peer_close_surface_from_frame_light(ShellAtlasContext *ctx, uint64_t sid) {
    atlas_binding(ctx);
    return close_surface_from_frame_light(sid);
}

void shell_atlas_peer_clamp_position(ShellAtlasContext *ctx, int32_t x, int32_t y, uint32_t w,
                                     uint32_t h, int32_t *out_x, int32_t *out_y) {
    atlas_binding(ctx);
    clamp_position(x, y, w, h, out_x, out_y);
}

void shell_atlas_peer_clamp_surface_size(ShellAtlasContext *ctx, int32_t x, int32_t y, uint32_t w,
                                         uint32_t h, uint32_t *out_w, uint32_t *out_h) {
    atlas_binding(ctx);
    clamp_surface_size(x, y, w, h, out_w, out_h);
}

int32_t shell_atlas_peer_desktop_width(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    return desktop_width();
}

int32_t shell_atlas_peer_desktop_height(ShellAtlasContext *ctx) {
    atlas_binding(ctx);
    return desktop_height();
}

bool shell_atlas_peer_shell_custody_has_surface(ShellAtlasContext *ctx, uint64_t sid,
                                                bool terminal_only) {
    atlas_binding(ctx);
    return shell_custody_has_surface(sid, terminal_only);
}


