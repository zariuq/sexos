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

#include "silk_shell_chrome.h"
#define U64(value) ((unsigned long long)(value))
#define FRAMES (ctx->chrome->FRAMES)
#define FOCUSED_SURFACE_ID (ctx->chrome->FOCUSED_SURFACE_ID)
#define ACTIVE_SCENE_IDX (ctx->chrome->ACTIVE_SCENE_IDX)
#define HOVERED_FRAME_ID (ctx->chrome->HOVERED_FRAME_ID)
#define HOVER_KIND (ctx->chrome->HOVER_KIND)
#define HOVERED_FRAME_LIGHT (ctx->chrome->HOVERED_FRAME_LIGHT)
#define SCENES (ctx->chrome->SCENES)
#define ATLAS_MODE_ENABLED (ctx->chrome->ATLAS_MODE_ENABLED)
#define DRAG_MOVED_THIS_DRAG (ctx->chrome->DRAG_MOVED_THIS_DRAG)
#define WINDOW_MOVE_PROOF_DONE_EMITTED (ctx->chrome->WINDOW_MOVE_PROOF_DONE_EMITTED)
#define WINDOWS (ctx->chrome->WINDOWS)
#define WINDOWS_len (ctx->chrome->WINDOWS_len)
#define SURFACE_100_ALIVE (ctx->chrome->SURFACE_100_ALIVE)
#define SURFACE_101_ALIVE (ctx->chrome->SURFACE_101_ALIVE)
#define SURFACE_102_ALIVE (ctx->chrome->SURFACE_102_ALIVE)
#define SURFACE_103_ALIVE (ctx->chrome->SURFACE_103_ALIVE)
#define SURFACE_100_W (ctx->chrome->SURFACE_100_W)
#define SURFACE_100_H (ctx->chrome->SURFACE_100_H)
#define SURFACE_101_X (ctx->chrome->SURFACE_101_X)
#define SURFACE_101_Y (ctx->chrome->SURFACE_101_Y)
#define SURFACE_101_W (ctx->chrome->SURFACE_101_W)
#define SURFACE_101_H (ctx->chrome->SURFACE_101_H)
#define SURFACE_102_X (ctx->chrome->SURFACE_102_X)
#define SURFACE_102_Y (ctx->chrome->SURFACE_102_Y)
#define SURFACE_102_W (ctx->chrome->SURFACE_102_W)
#define SURFACE_102_H (ctx->chrome->SURFACE_102_H)
#define SURFACE_103_X (ctx->chrome->SURFACE_103_X)
#define SURFACE_103_Y (ctx->chrome->SURFACE_103_Y)
#define SURFACE_103_W (ctx->chrome->SURFACE_103_W)
#define SURFACE_103_H (ctx->chrome->SURFACE_103_H)
#define INTERACTION (ctx->input->INTERACTION)
#define POINTER_X (ctx->input->POINTER_X)
#define POINTER_Y (ctx->input->POINTER_Y)
#define INPUT_TRACE_DRAG_MOVES (ctx->input->INPUT_TRACE_DRAG_MOVES)
#define DRAG_PENDING_ACTIVE (ctx->input->DRAG_PENDING_ACTIVE)
#define DRAG_PENDING_START_X (ctx->input->DRAG_PENDING_START_X)
#define DRAG_PENDING_START_Y (ctx->input->DRAG_PENDING_START_Y)
#define DRAG_PENDING_TARGET (ctx->input->DRAG_PENDING_TARGET)
#define DRAG_PENDING_KIND (ctx->input->DRAG_PENDING_KIND)
#define TAB_DRAG_PENDING_FRAME_ID (ctx->input->TAB_DRAG_PENDING_FRAME_ID)
#define TAB_DRAG_PENDING_START_TAB (ctx->input->TAB_DRAG_PENDING_START_TAB)
#define LAUNCHER_ACTIVE (ctx->input->LAUNCHER_ACTIVE)
#define STATUS_ACTIVE (ctx->input->STATUS_ACTIVE)
#define CLOCK_ACTIVE (ctx->input->CLOCK_ACTIVE)
#define SCENE_SETTINGS_ACTIVE (ctx->input->SCENE_SETTINGS_ACTIVE)
#define SURFACE_200_X (ctx->input->SURFACE_200_X)
#define SURFACE_200_Y (ctx->input->SURFACE_200_Y)
#define SURFACE_200_W (ctx->input->SURFACE_200_W)
#define SURFACE_200_H (ctx->input->SURFACE_200_H)
#define SURFACE_201_X (ctx->input->SURFACE_201_X)
#define SURFACE_201_Y (ctx->input->SURFACE_201_Y)
#define SURFACE_201_W (ctx->input->SURFACE_201_W)
#define SURFACE_201_H (ctx->input->SURFACE_201_H)
#define SURFACE_202_X (ctx->input->SURFACE_202_X)
#define SURFACE_202_Y (ctx->input->SURFACE_202_Y)
#define SURFACE_202_W (ctx->input->SURFACE_202_W)
#define SURFACE_202_H (ctx->input->SURFACE_202_H)
#define SURFACE_203_X (ctx->input->SURFACE_203_X)
#define SURFACE_203_Y (ctx->input->SURFACE_203_Y)
#define SURFACE_203_W (ctx->input->SURFACE_203_W)
#define SURFACE_203_H (ctx->input->SURFACE_203_H)
#define SURFACE_204_X (ctx->input->SURFACE_204_X)
#define SURFACE_204_Y (ctx->input->SURFACE_204_Y)
#define SURFACE_204_W (ctx->input->SURFACE_204_W)
#define SURFACE_204_H (ctx->input->SURFACE_204_H)
#define SURFACE_205_X (ctx->input->SURFACE_205_X)
#define SURFACE_205_Y (ctx->input->SURFACE_205_Y)
#define SURFACE_205_W (ctx->input->SURFACE_205_W)
#define SURFACE_205_H (ctx->input->SURFACE_205_H)
#define SURFACE_0x99_X (ctx->input->SURFACE_0x99_X)
#define SURFACE_0x99_Y (ctx->input->SURFACE_0x99_Y)
#define SURFACE_0x99_W (ctx->input->SURFACE_0x99_W)
#define SURFACE_0x99_H (ctx->input->SURFACE_0x99_H)
#define LIFECYCLE_TABLE (ctx->models->LIFECYCLE_TABLE)
#define LIFECYCLE_GENERATION (ctx->models->LIFECYCLE_GENERATION)
#define FOCUSED_SURFACE (ctx->models->FOCUSED_SURFACE)
#define ACTIVE_TINT_IDX (ctx->models->ACTIVE_TINT_IDX)
#define DRAG_CAPTURE_MOVE_SEEN (ctx->proofs_a->DRAG_CAPTURE_MOVE_SEEN)
#define scancode_to_action(scancode, out_action)                                                   \
    shell_chrome_scancode_to_action(ctx, scancode, out_action)
#define action_name(action) shell_chrome_action_name(ctx, action)
#define desktop_width() shell_chrome_desktop_width(ctx)
#define desktop_height() shell_chrome_desktop_height(ctx)
#define layout_left(out_x, out_y, out_w, out_h)                                                    \
    shell_chrome_layout_left(ctx, out_x, out_y, out_w, out_h)
#define layout_right(out_x, out_y, out_w, out_h)                                                   \
    shell_chrome_layout_right(ctx, out_x, out_y, out_w, out_h)
#define layout_maximize(out_x, out_y, out_w, out_h)                                                \
    shell_chrome_layout_maximize(ctx, out_x, out_y, out_w, out_h)
#define clamp_position(x, y, w, h, out_x, out_y)                                                   \
    shell_chrome_clamp_position(ctx, x, y, w, h, out_x, out_y)
#define snap_end_pos(w, h, out_x, out_y) shell_chrome_snap_end_pos(ctx, w, h, out_x, out_y)
#define lifecycle_register(sid, initial_state)                                                     \
    shell_chrome_lifecycle_register(ctx, sid, initial_state)
#define lifecycle_state(sid, out_state) shell_chrome_lifecycle_state(ctx, sid, out_state)
#define set_lifecycle_state(sid, next) shell_chrome_set_lifecycle_state(ctx, sid, next)
#define surface_generation(sid, out_gen) shell_chrome_surface_generation(ctx, sid, out_gen)
#define bump_surface_generation(sid) shell_chrome_bump_surface_generation(ctx, sid)
#define make_focus_ref(sid, out_ref) shell_chrome_make_focus_ref(ctx, sid, out_ref)
#define focus_ref_is_current(r) shell_chrome_focus_ref_is_current(ctx, r)
#define sync_focus_ref() shell_chrome_sync_focus_ref(ctx)
#define surface_is_lifecycle_live(sid) shell_chrome_surface_is_lifecycle_live(ctx, sid)
#define surface_is_lifecycle_focusable(sid) shell_chrome_surface_is_lifecycle_focusable(ctx, sid)
#define lifecycle_init_all() shell_chrome_lifecycle_init_all(ctx)
#define is_shell_surface(sid) shell_chrome_is_shell_surface(ctx, sid)
#define is_focusable_surface(sid) shell_chrome_is_focusable_surface(ctx, sid)
#define frame_for_surface(sid, out_frame_id) shell_chrome_frame_for_surface(ctx, sid, out_frame_id)
#define active_surface_for_frame(id, out_surface_id)                                               \
    shell_chrome_active_surface_for_frame(ctx, id, out_surface_id)
#define selected_frame_id(out_frame_id) shell_chrome_selected_frame_id(ctx, out_frame_id)
#define selected_surface_id(out_sid) shell_chrome_selected_surface_id(ctx, out_sid)
#define selected_window_options_mask() shell_chrome_selected_window_options_mask(ctx)
#define frame_is_minimized(frame_id) shell_chrome_frame_is_minimized(ctx, frame_id)
#define set_frame_minimized(frame_id, minimized)                                                   \
    shell_chrome_set_frame_minimized(ctx, frame_id, minimized)
#define frame_is_zoomed(frame_id) shell_chrome_frame_is_zoomed(ctx, frame_id)
#define set_frame_zoomed(frame_id, zoomed) shell_chrome_set_frame_zoomed(ctx, frame_id, zoomed)
#define frame_has_top_bar(frame_id) shell_chrome_frame_has_top_bar(ctx, frame_id)
#define set_frame_top_bar(frame_id, enabled) shell_chrome_set_frame_top_bar(ctx, frame_id, enabled)
#define first_minimized_frame_id(out_frame_id)                                                     \
    shell_chrome_first_minimized_frame_id(ctx, out_frame_id)
#define frame_accepts_input(frame_id) shell_chrome_frame_accepts_input(ctx, frame_id)
#define frame_chrome_visible(frame_id) shell_chrome_frame_chrome_visible(ctx, frame_id)
#define clear_hover_if_wrong_scene() shell_chrome_clear_hover_if_wrong_scene(ctx)
#define frame_tab_count(frame_id) shell_chrome_frame_tab_count(ctx, frame_id)
#define frame_active_tab_index(frame_id) shell_chrome_frame_active_tab_index(ctx, frame_id)
#define frame_tab_at(frame_id, x, y, out_tab)                                                      \
    shell_chrome_frame_tab_at(ctx, frame_id, x, y, out_tab)
#define swap_frame_tabs(frame_id, idx_a, idx_b)                                                    \
    shell_chrome_swap_frame_tabs(ctx, frame_id, idx_a, idx_b)
#define frame_light_to_option_mask(light) shell_chrome_frame_light_to_option_mask(ctx, light)
#define abs_screen_to_raw(screen, dim) shell_chrome_abs_screen_to_raw(ctx, screen, dim)
#define compute_resize_edge(px, py, sx, sy, sw, sh)                                                \
    shell_chrome_compute_resize_edge(ctx, px, py, sx, sy, sw, sh)
#define compute_resize_rect(cur_x, cur_y, cur_w, cur_h, edge, dx, dy, out_x, out_y, out_w, out_h)  \
    shell_chrome_compute_resize_rect(ctx, cur_x, cur_y, cur_w, cur_h, edge, dx, dy, out_x, out_y,  \
                                     out_w, out_h)
#define native_child_geometry(frame, x, y, w, h)                                                   \
    shell_chrome_native_child_geometry(ctx, frame, x, y, w, h)
#define update_local_geometry(surface_id, x, y, w, h)                                              \
    shell_chrome_update_local_geometry(ctx, surface_id, x, y, w, h)
#define is_closeable_surface(surface_id) shell_chrome_is_closeable_surface(ctx, surface_id)
#define frame_close_allowed(frame_id) shell_chrome_frame_close_allowed(ctx, frame_id)
#define emit_chrome_diagnostics(frame_id, reason)                                                  \
    shell_chrome_emit_chrome_diagnostics(ctx, frame_id, reason)
#define frame_chrome_operation(frame_id, out)                                                      \
    shell_chrome_frame_chrome_operation(ctx, frame_id, out)
#define send_frame_tab_info(frame_id) shell_chrome_send_frame_tab_info(ctx, frame_id)
#define close_surface_from_frame_light(surface_id)                                                 \
    shell_chrome_close_surface_from_frame_light(ctx, surface_id)
#define minimize_frame(frame_id) shell_chrome_minimize_frame(ctx, frame_id)
#define restore_minimized_frame_inner(frame_id, focus)                                             \
    shell_chrome_restore_minimized_frame_inner(ctx, frame_id, focus)
#define restore_minimized_frame(frame_id) shell_chrome_restore_minimized_frame(ctx, frame_id)
#define zoom_frame(frame_id) shell_chrome_zoom_frame(ctx, frame_id)
#define unzoom_frame(frame_id) shell_chrome_unzoom_frame(ctx, frame_id)
#define toggle_zoom_frame(frame_id) shell_chrome_toggle_zoom_frame(ctx, frame_id)
#define tile_active_scene_frames() shell_chrome_tile_active_scene_frames(ctx)
#define tile_visible_frames() shell_chrome_tile_visible_frames(ctx)
#define frame_light_at(frame_id, x, y) shell_chrome_frame_light_at(ctx, frame_id, x, y)
#define toggle_top_bar_for_active_frame() shell_chrome_toggle_top_bar_for_active_frame(ctx)
#define switch_to_tab(frame_id, tab_index) shell_chrome_switch_to_tab(ctx, frame_id, tab_index)
#define try_set_focus_inner(sid, refresh_children)                                                 \
    shell_chrome_try_set_focus_inner(ctx, sid, refresh_children)
#define try_set_focus(sid) shell_chrome_try_set_focus(ctx, sid)
#define update_frame_hover_at(x, y) shell_chrome_update_frame_hover_at(ctx, x, y)
#define access_copy_label(dst, src) shell_chrome_access_copy_label(ctx, dst, src)
#define access_label_token(label) shell_chrome_access_label_token(ctx, label)
#define access_node_is_valid_target(target) shell_chrome_access_node_is_valid_target(ctx, target)
#define access_emit_scene_node(nodes, idx, scene_id)                                               \
    shell_chrome_access_emit_scene_node(ctx, nodes, idx, scene_id)
#define access_emit_frame_node(nodes, idx, frame)                                                  \
    shell_chrome_access_emit_frame_node(ctx, nodes, idx, frame)
#define access_emit_shell_nodes(nodes) shell_chrome_access_emit_shell_nodes(ctx, nodes)
#define access_describe_node(node) shell_chrome_access_describe_node(ctx, node)
#define access_describe_focus() shell_chrome_access_describe_focus(ctx)
#define access_move_focus(nodes, length, forward)                                                  \
    shell_chrome_access_move_focus(ctx, nodes, length, forward)
#define access_handle_keyboard_action(action)                                                      \
    shell_chrome_access_handle_keyboard_action(ctx, action)
#define try_transition(next) shell_chrome_try_transition(ctx, next)
#define move_surface_by(sid, dx, dy, out) shell_chrome_move_surface_by(ctx, sid, dx, dy, out)
#define synthetic_window_drag_target(out) shell_chrome_synthetic_window_drag_target(ctx, out)
#define silk_drag_begin_mark(sid, x, y) shell_chrome_silk_drag_begin_mark(ctx, sid, x, y)
#define silk_drag_end_mark(sid) shell_chrome_silk_drag_end_mark(ctx, sid)
#define drag_move_focused(dx, dy) shell_chrome_drag_move_focused(ctx, dx, dy)
#define resize_accumulate_delta(dx, dy) shell_chrome_resize_accumulate_delta(ctx, dx, dy)
#define clear_resize_if_dead() shell_chrome_clear_resize_if_dead(ctx)
#define try_snap_on_drag_release(sid, px, py)                                                      \
    shell_chrome_try_snap_on_drag_release(ctx, sid, px, py)
#define apply_resize_geometry(dx, dy) shell_chrome_apply_resize_geometry(ctx, dx, dy)
#define hit_test_surface_chrome(x, y, sid, out)                                                    \
    shell_chrome_hit_test_surface_chrome(ctx, x, y, sid, out)
#define hit_test_at(x, y) shell_chrome_hit_test_at(ctx, x, y)
#define hit_target_surface(target, out) shell_chrome_hit_target_surface(ctx, target, out)
#define hit_target_label(target, bar, out) shell_chrome_hit_target_label(ctx, target, bar, out)
#define toggle_os_panel(active, panel, sid, label, x, y, w, h)                                     \
    shell_chrome_toggle_os_panel(ctx, active, panel, sid, label, x, y, w, h)
#define toggle_scene_settings_panel() shell_chrome_toggle_scene_settings_panel(ctx)
#define handle_silkbar_click(px, py) shell_chrome_handle_silkbar_click(ctx, px, py)
#define click_hit_test_and_focus(x, y, buttons)                                                    \
    shell_chrome_click_hit_test_and_focus(ctx, x, y, buttons)
#define point_in_surface(x, y, sid) shell_chrome_peer_point_in_surface(ctx, x, y, sid)
#define surface_input_lifetime_mark_drag_target(sid, live, phase, ok)                              \
    shell_chrome_peer_surface_input_lifetime_mark_drag_target(ctx, sid, live, phase, ok)
#define surface_input_lifetime_mark_click_target(sid, live, committed, ok)                         \
    shell_chrome_peer_surface_input_lifetime_mark_click_target(ctx, sid, live, committed, ok)
#define shell_interaction_contract_mark_stage(stage)                                               \
    shell_chrome_peer_shell_interaction_contract_mark_stage(ctx, stage)
#define shell_interaction_contract_try_done()                                                      \
    shell_chrome_peer_shell_interaction_contract_try_done(ctx)
#define click_drag_proof_begin_drag_once() shell_chrome_peer_click_drag_proof_begin_drag_once(ctx)
#define atlas_scene_at_point(x, y, out) shell_chrome_peer_atlas_scene_at_point(ctx, x, y, out)
#define switch_scene(scene) shell_chrome_peer_switch_scene(ctx, scene)
#define sync_scene_visibility() shell_chrome_peer_sync_scene_visibility(ctx)
#define clear_focus_if_wrong_scene() shell_chrome_peer_clear_focus_if_wrong_scene(ctx)
#define clear_drag_if_wrong_scene() shell_chrome_peer_clear_drag_if_wrong_scene(ctx)
#define atlas_capture_snapshot() shell_chrome_peer_atlas_capture_snapshot(ctx)
#define toggle_bell() shell_chrome_peer_toggle_bell(ctx)
#define validate_scene_id(scene) shell_chrome_peer_validate_scene_id(ctx, scene)
#define next_scene() shell_chrome_peer_next_scene(ctx)
#define prev_scene() shell_chrome_peer_prev_scene(ctx)
#define frame_label(frame) shell_chrome_peer_frame_label(ctx, frame)
#define surface_scene_id(sid, out) shell_chrome_peer_surface_scene_id(ctx, sid, out)
#define surface_in_active_scene(sid) shell_chrome_peer_surface_in_active_scene(ctx, sid)
#define launcher_focus_discard() shell_chrome_peer_launcher_focus_discard(ctx)
#define mesh_is_visible_in_active_scene() shell_chrome_peer_mesh_is_visible_in_active_scene(ctx)
#define mesh_render_pd_graph() shell_chrome_peer_mesh_render_pd_graph(ctx)
#define bell_ring_count() shell_chrome_peer_bell_ring_count(ctx)
#define send_silkbar_phase2_update(kind, a, b)                                                     \
    shell_chrome_peer_send_silkbar_phase2_update(ctx, kind, a, b)
#define surface_input_lifetime_mark_focus_live(sid, alive, ok)                                     \
    shell_chrome_peer_surface_input_lifetime_mark_focus_live(ctx, sid, alive, ok)
#define surface_input_lifetime_try_done() shell_chrome_peer_surface_input_lifetime_try_done(ctx)
#define pdx_call(slot, opcode, a, b, c) shell_chrome_peer_pdx_call(ctx, slot, opcode, a, b, c)
#define shell_request_app_close(sid) shell_chrome_peer_shell_request_app_close(ctx, sid)
#define clear_focus_if_dead() shell_chrome_peer_clear_focus_if_dead(ctx)
#define clear_drag_if_dead() shell_chrome_peer_clear_drag_if_dead(ctx)
#define clear_hover_if_dead() shell_chrome_peer_clear_hover_if_dead(ctx)
#define record_tombstone_event(sid, old, next, reason)                                             \
    shell_chrome_peer_record_tombstone_event(ctx, sid, old, next, reason)
#define snap_capture_layout() shell_chrome_peer_snap_capture_layout(ctx)
#define desktop_ready(sid) shell_chrome_peer_desktop_ready(ctx, sid)
#define linen_object_count() shell_chrome_peer_linen_object_count(ctx)
#define linen_render_object_list() shell_chrome_peer_linen_render_object_list(ctx)
#define surface_is_alive(sid) shell_chrome_peer_surface_is_alive(ctx, sid)
#define is_tombstoned(sid) shell_chrome_peer_is_tombstoned(ctx, sid)
#define get_surface_bounds(sid, x, y, w, h)                                                        \
    shell_chrome_peer_get_surface_bounds(ctx, sid, x, y, w, h)
#define clamp_surface_size(x, y, w, h, cw, ch)                                                     \
    shell_chrome_peer_clamp_surface_size(ctx, x, y, w, h, cw, ch)

static int32_t chrome_add(int32_t a, int32_t b) {
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t chrome_sub(int32_t a, int32_t b) {
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t chrome_max(int32_t a, int32_t b) {
    return a > b ? a : b;
}

static int32_t chrome_min(int32_t a, int32_t b) {
    return a < b ? a : b;
}

static int32_t chrome_clamp(int32_t value, int32_t low, int32_t high) {
    if (low > high) {
        sexos_fail();

        return low;
    }
    return chrome_min(chrome_max(value, low), high);
}

static uint32_t chrome_sat_sub(uint32_t a, uint32_t b) {
    return a > b ? a - b : 0;
}

static const char *chrome_life_name(uint8_t state) {
    switch (state) {
    case LIFECYCLE_ALLOCATED:
        return "Allocated";

    case LIFECYCLE_MAPPED:
        return "Mapped";

    case LIFECYCLE_VISIBLE:
        return "Visible";

    case LIFECYCLE_HIDDEN:
        return "Hidden";

    case LIFECYCLE_MINIMIZED:
        return "Minimized";

    case LIFECYCLE_CLOSING:
        return "Closing";

    case LIFECYCLE_TOMBSTONED:
        return "Tombstoned";

    case LIFECYCLE_DESTROYED:
        return "Destroyed";

    default:
        return "Invalid";
    }
    return "Invalid";
}

static size_t chrome_frame_slot(ShellChromeContext *ctx, uint32_t id) {
    for (size_t i = 0; i < MAX_FRAMES; ++i)
        if (FRAMES[i].present && FRAMES[i].frame_id == id)
            return i;

    return MAX_FRAMES;
}

static bool chrome_frame_flag(ShellChromeContext *ctx, uint32_t id, uint32_t flag) {
    for (size_t i = 0; i < MAX_FRAMES; ++i)
        if (FRAMES[i].present && FRAMES[i].frame_id == id && (FRAMES[i].flags & flag))
            return true;

    return false;
}

static void chrome_set_flag(ShellChromeContext *ctx, uint32_t id, uint32_t flag, bool value) {
    size_t i = chrome_frame_slot(ctx, id);

    if (i == MAX_FRAMES)
        return;

    if (value)
        FRAMES[i].flags |= flag;

    else
        FRAMES[i].flags &= ~flag;
}

bool shell_chrome_scancode_to_action(ShellChromeContext *ctx, uint8_t scancode,
                                     uint8_t *out_action) {
    (void)ctx;

    switch (scancode) {
    case 0x01:
        *out_action = ACTION_ACCESS_ZOOM_TOGGLE;

        return true;

    case 0x0F:
        *out_action = ACTION_ACCESS_FOCUS_NEXT;

        return true;

    case 0x0E:
        *out_action = ACTION_ACCESS_FOCUS_PREV;

        return true;

    case 0x1C:
        *out_action = ACTION_ACCESS_ACTIVATE;

        return true;

    case 0x3C:
        *out_action = ACTION_DESTROY_FOCUSED;

        return true;

    case 0x02:
        *out_action = ACTION_FOCUS_100;

        return true;

    case 0x03:
        *out_action = ACTION_FOCUS_101;

        return true;

    case 0x04:
        *out_action = ACTION_FOCUS_102;

        return true;

    case 0x05:
        *out_action = ACTION_FOCUS_103;

        return true;

    case 0x06:
        *out_action = ACTION_FOCUS_200;

        return true;

    case 0x3D:
        *out_action = ACTION_RECREATE_FOCUSED;

        return true;

    case 0x13:
        *out_action = ACTION_RESET_ALL;

        return true;

    case 0x26:
        *out_action = ACTION_SNAP_LEFT;

        return true;

    case 0x27:
        *out_action = ACTION_SNAP_RIGHT;

        return true;

    case 0x32:
        *out_action = ACTION_MAXIMIZE;

        return true;

    case 0x2E:
        *out_action = ACTION_CENTER;

        return true;

    case 0x1A:
        *out_action = ACTION_SHRINK_WIDTH;

        return true;

    case 0x1B:
        *out_action = ACTION_GROW_WIDTH;

        return true;

    case 0x0C:
        *out_action = ACTION_SHRINK_HEIGHT;

        return true;

    case 0x0D:
        *out_action = ACTION_GROW_HEIGHT;

        return true;

    case 0x49:
        *out_action = ACTION_RESTORE_MINIMIZED;

        return true;

    case 0x3E:
        *out_action = ACTION_TOGGLE_TOP_BAR;

        return true;

    case 0x3F:
        *out_action = ACTION_CYCLE_RENDER_TOKEN_PRESET;

        return true;

    case 0x40:
        *out_action = ACTION_CYCLE_CUSTOM_TINT;

        return true;

    case 0x41:
        *out_action = ACTION_TOGGLE_SCENE_SETTINGS_PANEL;

        return true;

    case 0x3B:
        *out_action = ACTION_LEGACY_FOCUS_TOGGLE;

        return true;

    case 0x42:
        *out_action = ACTION_TOGGLE_LINEN;

        return true;

    case 0x43:
        *out_action = ACTION_TOGGLE_QUIL;

        return true;

    case 0x44:
        *out_action = ACTION_TOGGLE_ATLAS;

        return true;

    case 0x46:
        *out_action = ACTION_TOGGLE_SPINDLE;

        return true;

    case 0x57:
        *out_action = ACTION_ACCESS_CLOSE;

        return true;

    case 0x58:
        *out_action = ACTION_TOGGLE_MESH;

        return true;

    case 0x59:
        *out_action = ACTION_OPEN_OBJECT_IN_QUIL;

        return true;

    case 0x52:
        *out_action = ACTION_TOGGLE_COLLAR;

        return true;

    case 0x51:
        *out_action = ACTION_TOGGLE_BELL;

        return true;

    case 0x47:
        *out_action = ACTION_SNAP_HOME;

        return true;

    case 0x4F:
        *out_action = ACTION_SNAP_END;

        return true;

    case 0x4B:
        *out_action = ACTION_MOVE_LEFT;

        return true;

    case 0x4D:
        *out_action = ACTION_MOVE_RIGHT;

        return true;

    case 0x48:
        *out_action = ACTION_MOVE_UP;

        return true;

    case 0x50:
        *out_action = ACTION_MOVE_DOWN;

        return true;

    case 0x24:
        *out_action = ACTION_SELECT_NEXT_LINEN_OBJECT;

        return true;

    case 0x25:
        *out_action = ACTION_SELECT_PREV_LINEN_OBJECT;

        return true;

    case 0x29:
        *out_action = ACTION_TOGGLE_COMMAND_PALETTE;

        return true;

    default:
        return false;
    }
    return false;
}

const char *shell_chrome_action_name(ShellChromeContext *ctx, uint8_t action) {
    (void)ctx;

    switch (action) {
    case ACTION_ACCESS_FOCUS_NEXT:
        return "AccessFocusNext";

    case ACTION_ACCESS_FOCUS_PREV:
        return "AccessFocusPrev";

    case ACTION_ACCESS_ACTIVATE:
        return "AccessActivate";

    case ACTION_ACCESS_CLOSE:
        return "AccessClose";

    case ACTION_ACCESS_ZOOM_TOGGLE:
        return "AccessZoomToggle";

    case ACTION_TOGGLE_QUIL:
        return "ToggleQuil";

    case ACTION_TOGGLE_LINEN:
        return "ToggleLinen";

    case ACTION_TOGGLE_SPINDLE:
        return "ToggleSpindle";

    case ACTION_TOGGLE_ATLAS:
        return "ToggleAtlas";

    case ACTION_TOGGLE_BELL:
        return "ToggleBell";

    case ACTION_TOGGLE_COLLAR:
        return "ToggleCollar";

    case ACTION_TOGGLE_MESH:
        return "ToggleMesh";

    case ACTION_TOGGLE_COMMAND_PALETTE:
        return "ToggleCommandPalette";

    case ACTION_MAXIMIZE:
        return "Maximize";

    case ACTION_RESTORE_MINIMIZED:
        return "RestoreMinimized";

    default:
        return "Other";
    }
    return "Other";
}

int32_t shell_chrome_desktop_width(ShellChromeContext *ctx) {
    return (int32_t)shell_chrome_peer_load_dimension(&ctx->apps->width);
}

int32_t shell_chrome_desktop_height(ShellChromeContext *ctx) {
    return (int32_t)shell_chrome_peer_load_dimension(&ctx->apps->height);
}

void shell_chrome_layout_left(ShellChromeContext *ctx, int32_t *out_x, int32_t *out_y,
                              uint32_t *out_w, uint32_t *out_h) {
    *out_x = 0;

    *out_y = P.bar_height;

    *out_w = (uint32_t)desktop_width() / 2;

    *out_h = (uint32_t)chrome_sub(desktop_height(), P.bar_height);
}

void shell_chrome_layout_right(ShellChromeContext *ctx, int32_t *out_x, int32_t *out_y,
                               uint32_t *out_w, uint32_t *out_h) {
    *out_x = desktop_width() / 2;

    *out_y = P.bar_height;

    *out_w = (uint32_t)desktop_width() / 2;

    *out_h = (uint32_t)chrome_sub(desktop_height(), P.bar_height);
}

void shell_chrome_layout_maximize(ShellChromeContext *ctx, int32_t *out_x, int32_t *out_y,
                                  uint32_t *out_w, uint32_t *out_h) {
    *out_x = 0;

    *out_y = P.bar_height;

    *out_w = (uint32_t)desktop_width();

    *out_h = (uint32_t)chrome_sub(desktop_height(), P.bar_height);
}

void shell_chrome_clamp_position(ShellChromeContext *ctx, int32_t x, int32_t y, uint32_t w,
                                 uint32_t h, int32_t *out_x, int32_t *out_y) {
    int32_t mx = (int32_t)chrome_sat_sub((uint32_t)desktop_width(), w);

    uint32_t my = chrome_sat_sub((uint32_t)desktop_height(), h);

    if (my < (uint32_t)P.bar_height)
        my = (uint32_t)P.bar_height;

    *out_x = chrome_clamp(x, 0, mx);

    *out_y = chrome_clamp(y, P.bar_height, (int32_t)my);
}

void shell_chrome_snap_end_pos(ShellChromeContext *ctx, uint32_t w, uint32_t h, int32_t *out_x,
                               int32_t *out_y) {
    *out_x = chrome_max((int32_t)chrome_sat_sub((uint32_t)desktop_width(), w), 0);

    *out_y = chrome_max((int32_t)chrome_sat_sub((uint32_t)desktop_height(), h), P.bar_height);
}

bool shell_chrome_lifecycle_register(ShellChromeContext *ctx, uint64_t sid, uint8_t initial_state) {
    if (initial_state > LIFECYCLE_DESTROYED)
        return false;

    for (size_t i = 0; i < LIFECYCLE_MAX_SURFACES; ++i) {
        if (LIFECYCLE_TABLE[i].present && LIFECYCLE_TABLE[i].surface_id == sid) {
            /* Re-registration starts a new incarnation;
    
    
    
     an old focus stamp cannot return. */
            if (LIFECYCLE_GENERATION == UINT64_MAX) {
                sexos_line("[lifecycle.register.reject] sid=%llu reason=generation_exhausted",
                           U64(sid));

                return false;
            }
            ++LIFECYCLE_GENERATION;

            LIFECYCLE_TABLE[i].lc = (SurfaceLifecycle){initial_state, LIFECYCLE_GENERATION};

            return true;
        }
    }
    for (size_t i = 0; i < LIFECYCLE_MAX_SURFACES; ++i) {
        if (!LIFECYCLE_TABLE[i].present) {
            LIFECYCLE_TABLE[i] = (LifecycleTableEntry){true, sid, {initial_state, 1}};

            return true;
        }
    }
    return false;
}

bool shell_chrome_lifecycle_state(ShellChromeContext *ctx, uint64_t sid, uint8_t *out_state) {
    for (size_t i = 0; i < LIFECYCLE_MAX_SURFACES; ++i)
        if (LIFECYCLE_TABLE[i].present && LIFECYCLE_TABLE[i].surface_id == sid) {
            *out_state = LIFECYCLE_TABLE[i].lc.state;

            return true;
        }
    return false;
}

bool shell_chrome_set_lifecycle_state(ShellChromeContext *ctx, uint64_t sid, uint8_t next) {
    if (next > LIFECYCLE_DESTROYED)
        return false;

    for (size_t i = 0; i < LIFECYCLE_MAX_SURFACES; ++i) {
        if (!LIFECYCLE_TABLE[i].present || LIFECYCLE_TABLE[i].surface_id != sid)
            continue;

        uint8_t prev = LIFECYCLE_TABLE[i].lc.state;

        if (prev == next)
            return true;

        bool bump = next == LIFECYCLE_CLOSING || next == LIFECYCLE_DESTROYED ||
                    (prev == LIFECYCLE_CLOSING && next == LIFECYCLE_TOMBSTONED);

        if (bump) {
            if (LIFECYCLE_GENERATION == UINT64_MAX) {
                sexos_line("[lifecycle.generation.bump.wrap] sid=%llu prev=%s next=%s", U64(sid),
                           chrome_life_name(prev), chrome_life_name(next));

                sexos_line("[lifecycle.transition.reject] sid=%llu reason=generation_exhausted",
                           U64(sid));

                return false;
            }
            ++LIFECYCLE_GENERATION;

            LIFECYCLE_TABLE[i].lc.generation = LIFECYCLE_GENERATION;

            sexos_line("[lifecycle.generation.bump] sid=%llu gen=%llu prev=%s next=%s", U64(sid),
                       U64(LIFECYCLE_GENERATION), chrome_life_name(prev), chrome_life_name(next));
        }
        LIFECYCLE_TABLE[i].lc.state = next;

        sexos_line("[lifecycle.transition.allow] sid=%llu from=%s to=%s", U64(sid),
                   chrome_life_name(prev), chrome_life_name(next));

        return true;
    }
    sexos_line("[lifecycle.transition.reject] sid=%llu reason=unknown_surface", U64(sid));

    return false;
}

bool shell_chrome_surface_generation(ShellChromeContext *ctx, uint64_t sid, uint64_t *out_gen) {
    for (size_t i = 0; i < LIFECYCLE_MAX_SURFACES; ++i)
        if (LIFECYCLE_TABLE[i].present && LIFECYCLE_TABLE[i].surface_id == sid) {
            *out_gen = LIFECYCLE_TABLE[i].lc.generation;

            return true;
        }
    return false;
}

bool shell_chrome_bump_surface_generation(ShellChromeContext *ctx, uint64_t sid) {
    for (size_t i = 0; i < LIFECYCLE_MAX_SURFACES; ++i) {
        if (!LIFECYCLE_TABLE[i].present || LIFECYCLE_TABLE[i].surface_id != sid)
            continue;

        if (LIFECYCLE_GENERATION == UINT64_MAX) {
            sexos_line("[lifecycle.generation.bump.wrap] sid=%llu", U64(sid));

            return false;
        }
        ++LIFECYCLE_GENERATION;

        LIFECYCLE_TABLE[i].lc.generation = LIFECYCLE_GENERATION;

        sexos_line("[lifecycle.generation.bump] sid=%llu gen=%llu", U64(sid),
                   U64(LIFECYCLE_GENERATION));

        return true;
    }
    return false;
}

bool shell_chrome_make_focus_ref(ShellChromeContext *ctx, uint64_t sid, FocusRef *out_ref) {
    uint64_t gen = 0;

    if (!sid || !surface_generation(sid, &gen) || !gen)
        return false;

    *out_ref = (FocusRef){true, sid, gen};

    sexos_line("[lifecycle.focusref.make] sid=%llu gen=%llu", U64(sid), U64(gen));

    return true;
}

bool shell_chrome_focus_ref_is_current(ShellChromeContext *ctx, const FocusRef *r) {
    uint64_t gen = 0;

    if (!r->present || !r->generation)
        return false;

    if (surface_generation(r->surface_id, &gen)) {
        if (r->generation == gen)
            return true;

        sexos_line("[lifecycle.focusref.reject] sid=%llu ref_gen=%llu current_gen=%llu",
                   U64(r->surface_id), U64(r->generation), U64(gen));

        return false;
    }
    sexos_line("[lifecycle.focusref.reject] sid=%llu reason=unknown_surface", U64(r->surface_id));

    return false;
}

void shell_chrome_sync_focus_ref(ShellChromeContext *ctx) {
    FocusRef current = {0};

    if (FOCUSED_SURFACE_ID)
        make_focus_ref(FOCUSED_SURFACE_ID, &current);

    FOCUSED_SURFACE = current;
}

bool shell_chrome_surface_is_lifecycle_live(ShellChromeContext *ctx, uint64_t sid) {
    uint8_t state = 0;

    return lifecycle_state(sid, &state) &&
           (state == LIFECYCLE_VISIBLE || state == LIFECYCLE_MAPPED || state == LIFECYCLE_HIDDEN ||
            state == LIFECYCLE_MINIMIZED);
}

bool shell_chrome_surface_is_lifecycle_focusable(ShellChromeContext *ctx, uint64_t sid) {
    uint8_t state = 0;

    return lifecycle_state(sid, &state) &&
           (state == LIFECYCLE_VISIBLE || state == LIFECYCLE_MAPPED);
}

void shell_chrome_lifecycle_init_all(ShellChromeContext *ctx) {
#define CHROME_BOOT_LIFE(sid, state) {sid, state},

    static const struct {
        uint64_t sid;

        uint8_t state;

    } rows[SHELL_BOOT_LIFECYCLE_COUNT] = {SEXOS_SHELL_BOOT_LIFECYCLE_ROWS(CHROME_BOOT_LIFE)};

#undef CHROME_BOOT_LIFE
    for (size_t i = 0; i < SHELL_BOOT_LIFECYCLE_COUNT; ++i)
        if (!lifecycle_register(rows[i].sid, rows[i].state)) {
            sexos_fail();

            return;
        }
    sexos_line("[lifecycle.state.init] lifecycle model initialized");

    sexos_line("[silk.lifecycle.init] ok=1 surfaces=%zu states=Visible|Mapped|Allocated",
               (size_t)SHELL_BOOT_LIFECYCLE_COUNT);

    sexos_line(
        "[silk.lifecycle.invariant.ok] all_surfaces_registered=1 generation_safe=1 static_table_full=0");

    sexos_line("[silk.lifecycle.surface.live] ok=1 surfaces=%zu",
               (size_t)SHELL_BOOT_LIFECYCLE_COUNT);
}

bool shell_chrome_is_shell_surface(ShellChromeContext *ctx, uint64_t sid) {
    (void)ctx;

    return sid == SURFACE_ID_APP || sid == SURFACE_ID_STATIC || sid == SURFACE_ID_TEST3 ||
           sid == SURFACE_ID_TEST4;
}

bool shell_chrome_is_focusable_surface(ShellChromeContext *ctx, uint64_t sid) {
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        if (!ctx->apps->present[i])
            continue;

        ShellAppCustody entry = ctx->apps->entries[i];

        if ((entry.dead || entry.close_pending) && shell_custody_owns_surface(&entry, sid))
            return false;
    }
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        if (!ctx->apps->present[i])
            continue;

        ShellAppCustody entry = ctx->apps->entries[i];

        if (entry.surface == sid || shell_custody_is_successor_frame(&entry, sid))
            return !entry.dead && entry.has_bounds;
    }
    return is_shell_surface(sid) || sid == SURFACE_ID_LINEN ||
           shell_chrome_peer_app_focusable(ctx, sid);
}

bool shell_chrome_frame_for_surface(ShellChromeContext *ctx, uint64_t sid, uint32_t *out_frame_id) {
    for (size_t i = 0; i < MAX_FRAMES; ++i)
        if (FRAMES[i].present) {
            for (size_t j = 0; j < MAX_TABS_PER_FRAME; ++j)
                if (FRAMES[i].tabs[j].present && FRAMES[i].tabs[j].surface_id == sid) {
                    *out_frame_id = FRAMES[i].frame_id;

                    return true;
                }
        }
    return false;
}

bool shell_chrome_active_surface_for_frame(ShellChromeContext *ctx, uint32_t id,
                                           uint64_t *out_surface_id) {
    size_t i = chrome_frame_slot(ctx, id);

    if (i == MAX_FRAMES)
        return false;

    uint8_t active = FRAMES[i].active_tab;

    if (active >= MAX_TABS_PER_FRAME || !FRAMES[i].tabs[active].present)
        return false;

    *out_surface_id = FRAMES[i].tabs[active].surface_id;

    return true;
}

bool shell_chrome_selected_frame_id(ShellChromeContext *ctx, uint32_t *out_frame_id) {
    return frame_for_surface(FOCUSED_SURFACE_ID, out_frame_id);
}

bool shell_chrome_selected_surface_id(ShellChromeContext *ctx, uint64_t *out_sid) {
    uint64_t sid = FOCUSED_SURFACE_ID;

    if (!sid || !surface_is_alive(sid))
        return false;

    *out_sid = sid;

    return true;
}

uint32_t shell_chrome_selected_window_options_mask(ShellChromeContext *ctx) {
    uint32_t id = 0;

    return selected_frame_id(&id) ? OPTION_MOVE : 0;
}

bool shell_chrome_frame_is_minimized(ShellChromeContext *ctx, uint32_t frame_id) {
    return chrome_frame_flag(ctx, frame_id, FRAME_FLAG_MINIMIZED);
}

void shell_chrome_set_frame_minimized(ShellChromeContext *ctx, uint32_t frame_id, bool minimized) {
    chrome_set_flag(ctx, frame_id, FRAME_FLAG_MINIMIZED, minimized);
}

bool shell_chrome_frame_is_zoomed(ShellChromeContext *ctx, uint32_t frame_id) {
    return chrome_frame_flag(ctx, frame_id, FRAME_FLAG_ZOOMED);
}

void shell_chrome_set_frame_zoomed(ShellChromeContext *ctx, uint32_t frame_id, bool zoomed) {
    chrome_set_flag(ctx, frame_id, FRAME_FLAG_ZOOMED, zoomed);
}

bool shell_chrome_frame_has_top_bar(ShellChromeContext *ctx, uint32_t frame_id) {
    return chrome_frame_flag(ctx, frame_id, FRAME_FLAG_TOP_BAR);
}

void shell_chrome_set_frame_top_bar(ShellChromeContext *ctx, uint32_t frame_id, bool enabled) {
    chrome_set_flag(ctx, frame_id, FRAME_FLAG_TOP_BAR, enabled);
}

bool shell_chrome_first_minimized_frame_id(ShellChromeContext *ctx, uint32_t *out_frame_id) {
    for (size_t i = 0; i < MAX_FRAMES; ++i)
        if (FRAMES[i].present && (FRAMES[i].flags & FRAME_FLAG_MINIMIZED)) {
            *out_frame_id = FRAMES[i].frame_id;

            return true;
        }
    return false;
}

bool shell_chrome_frame_accepts_input(ShellChromeContext *ctx, uint32_t frame_id) {
    size_t i = chrome_frame_slot(ctx, frame_id);

    if (i == MAX_FRAMES)
        return false;

    ShellFrame frame = FRAMES[i];

    if (frame.scene_id != ACTIVE_SCENE_IDX || (frame.flags & FRAME_FLAG_MINIMIZED) ||
        frame.active_tab >= MAX_TABS_PER_FRAME)
        return false;

    ShellTab tab = frame.tabs[frame.active_tab];

    if (tab.present) {
        if (!surface_is_alive(tab.surface_id)) {
            sexos_line("[tab.focus.reject.dead] frame=%u tab=%u surface=%llu", frame_id,
                       (unsigned)frame.active_tab, U64(tab.surface_id));

            return false;
        }
        if (is_tombstoned(tab.surface_id)) {
            sexos_line("[tab.focus.reject.dead] frame=%u tab=%u surface=%llu reason=tombstoned",
                       frame_id, (unsigned)frame.active_tab, U64(tab.surface_id));

            return false;
        }
    }
    return true;
}

bool shell_chrome_frame_chrome_visible(ShellChromeContext *ctx, uint32_t frame_id) {
    size_t i = chrome_frame_slot(ctx, frame_id);

    if (i != MAX_FRAMES &&
        (FRAMES[i].scene_id != ACTIVE_SCENE_IDX || (FRAMES[i].flags & FRAME_FLAG_MINIMIZED)))
        return false;

    uint64_t sid = 0;

    if (!active_surface_for_frame(frame_id, &sid) || !surface_is_alive(sid) || is_tombstoned(sid))
        return false;

    return frame_tab_count(frame_id) > 1 ||
           (HOVERED_FRAME_ID == frame_id && HOVER_KIND != HOVER_NONE);
}

void shell_chrome_clear_hover_if_wrong_scene(ShellChromeContext *ctx) {
    if (HOVERED_FRAME_ID && !frame_accepts_input(HOVERED_FRAME_ID)) {
        sexos_line("[shell.hover.clear_dead] frame=%u reason=invalid_target", HOVERED_FRAME_ID);

        HOVERED_FRAME_ID = 0;

        HOVER_KIND = HOVER_NONE;

        HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

        sexos_line("[shell.frame.hover.clear.wrong-scene]");
    }
}

uint32_t shell_chrome_frame_tab_count(ShellChromeContext *ctx, uint32_t frame_id) {
    size_t i = chrome_frame_slot(ctx, frame_id);

    return i == MAX_FRAMES ? 0 : FRAMES[i].tab_count;
}

uint32_t shell_chrome_frame_active_tab_index(ShellChromeContext *ctx, uint32_t frame_id) {
    size_t i = chrome_frame_slot(ctx, frame_id);

    return i == MAX_FRAMES ? 0 : FRAMES[i].active_tab;
}

bool shell_chrome_frame_tab_at(ShellChromeContext *ctx, uint32_t frame_id, int32_t x, int32_t y,
                               uint32_t *out_tab) {
    if (!frame_accepts_input(frame_id))
        return false;

    uint64_t sid = 0;

    int32_t sx = 0, sy = 0;

    uint32_t sw = 0, sh = 0;

    if (!active_surface_for_frame(frame_id, &sid) || !get_surface_bounds(sid, &sx, &sy, &sw, &sh))
        return false;

    bool top = frame_has_top_bar(frame_id);

    int32_t band = top ? FRAME_TOP_BAR_HEIGHT_PX : FRAME_TAB_STRIP_PX;

    int32_t exclusion = top ? FRAME_TOP_BAR_LIGHT_EXCLUSION_PX : FRAME_TAB_LIGHT_EXCLUSION_PX;

    if (y < sy || y >= chrome_add(sy, band))
        return false;

    int32_t start = chrome_add(sx, exclusion);

    if (x < start)
        return false;

    int32_t right = chrome_sub(chrome_add(sx, (int32_t)sw), FRAME_RIM_PX);

    if (x >= right)
        return false;

    uint32_t count = frame_tab_count(frame_id);

    if (!count)
        return false;

    int32_t width = chrome_max(chrome_sub(right, start), 0);

    if (width < FRAME_TAB_MIN_WIDTH_PX) {
        *out_tab = 0;

        return true;
    }
    int32_t slot = width / (int32_t)count;

    *out_tab = (uint32_t)chrome_min(chrome_sub(x, start) / chrome_max(slot, 1), (int32_t)count - 1);

    return true;
}

bool shell_chrome_swap_frame_tabs(ShellChromeContext *ctx, uint32_t frame_id, uint8_t idx_a,
                                  uint8_t idx_b) {
    size_t i = chrome_frame_slot(ctx, frame_id);

    if (i == MAX_FRAMES)
        return false;

    if (idx_a >= FRAMES[i].tab_count || idx_b >= FRAMES[i].tab_count ||
        idx_a >= MAX_TABS_PER_FRAME || idx_b >= MAX_TABS_PER_FRAME)
        return false;

    if (idx_a == idx_b)
        return true;

    if (!FRAMES[i].tabs[idx_a].present || !FRAMES[i].tabs[idx_b].present)
        return false;

    ShellTab temp = FRAMES[i].tabs[idx_a];

    FRAMES[i].tabs[idx_a] = FRAMES[i].tabs[idx_b];

    FRAMES[i].tabs[idx_b] = temp;

    if (FRAMES[i].active_tab == idx_a)
        FRAMES[i].active_tab = idx_b;

    else if (FRAMES[i].active_tab == idx_b)
        FRAMES[i].active_tab = idx_a;

    sexos_line("[silk.tab.reorder.swap] frame=%u a=%u b=%u", frame_id, (unsigned)idx_a,
               (unsigned)idx_b);

    return true;
}

uint32_t shell_chrome_frame_light_to_option_mask(ShellChromeContext *ctx, uint32_t light) {
    (void)ctx;

    switch (light) {
    case FRAME_LIGHT_CLOSE:
        return OPTION_CLOSE;

    case FRAME_LIGHT_MINIMIZE:
        return OPTION_MINIMIZE;

    case FRAME_LIGHT_ZOOM:
        return OPTION_ZOOM;

    default:
        return 0;
    }
    return 0;
}

uint64_t shell_chrome_abs_screen_to_raw(ShellChromeContext *ctx, int32_t screen, int32_t dim) {
    (void)ctx;

    if (dim <= 1)
        return 0;

    int64_t s = chrome_clamp(screen, 0, dim - 1);

    return (uint64_t)((s * (int64_t)TABLET_RAW_MAX) / ((int64_t)dim - 1));
}

uint8_t shell_chrome_compute_resize_edge(ShellChromeContext *ctx, int32_t px, int32_t py,
                                         int32_t sx, int32_t sy, uint32_t sw, uint32_t sh) {
    (void)ctx;

    int32_t right = chrome_sub(chrome_add(sx, (int32_t)sw), 1),
            bottom = chrome_sub(chrome_add(sy, (int32_t)sh), 1);

    bool left =
        px<chrome_add(sx, FRAME_RESIZE_ZONE_PX), r = px> chrome_sub(right, FRAME_RESIZE_ZONE_PX);

    bool top =
        py<chrome_add(sy, FRAME_RESIZE_ZONE_PX), b = py> chrome_sub(bottom, FRAME_RESIZE_ZONE_PX);

    if (left && top)
        return RESIZE_EDGE_TOP_LEFT;

    if (r && top)
        return RESIZE_EDGE_TOP_RIGHT;

    if (left && b)
        return RESIZE_EDGE_BOTTOM_LEFT;

    if (r && b)
        return RESIZE_EDGE_BOTTOM_RIGHT;

    if (left)
        return RESIZE_EDGE_LEFT;

    if (r)
        return RESIZE_EDGE_RIGHT;

    if (top)
        return RESIZE_EDGE_TOP;

    return RESIZE_EDGE_BOTTOM;
}

void shell_chrome_compute_resize_rect(ShellChromeContext *ctx, int32_t cur_x, int32_t cur_y,
                                      uint32_t cur_w, uint32_t cur_h, uint8_t edge, int32_t dx,
                                      int32_t dy, int32_t *out_x, int32_t *out_y, uint32_t *out_w,
                                      uint32_t *out_h) {
    bool left =
        edge == RESIZE_EDGE_LEFT || edge == RESIZE_EDGE_BOTTOM_LEFT || edge == RESIZE_EDGE_TOP_LEFT;

    bool right = edge == RESIZE_EDGE_RIGHT || edge == RESIZE_EDGE_BOTTOM_RIGHT ||
                 edge == RESIZE_EDGE_TOP_RIGHT;

    bool top =
        edge == RESIZE_EDGE_TOP || edge == RESIZE_EDGE_TOP_LEFT || edge == RESIZE_EDGE_TOP_RIGHT;

    bool bottom = edge == RESIZE_EDGE_BOTTOM || edge == RESIZE_EDGE_BOTTOM_LEFT ||
                  edge == RESIZE_EDGE_BOTTOM_RIGHT;

    int32_t x = left ? chrome_add(cur_x, dx) : cur_x, y = top ? chrome_add(cur_y, dy) : cur_y;

    int32_t w = left    ? chrome_sub((int32_t)cur_w, dx)
                : right ? chrome_add((int32_t)cur_w, dx)
                        : (int32_t)cur_w;

    int32_t h = top      ? chrome_sub((int32_t)cur_h, dy)
                : bottom ? chrome_add((int32_t)cur_h, dy)
                         : (int32_t)cur_h;

    uint32_t cw = 0, ch = 0;

    clamp_surface_size(x, y, (uint32_t)chrome_max(w, 0), (uint32_t)chrome_max(h, 0), &cw, &ch);

    int32_t ax = left ? chrome_sub(chrome_add(cur_x, (int32_t)cur_w), (int32_t)cw) : x;

    int32_t ay = top ? chrome_sub(chrome_add(cur_y, (int32_t)cur_h), (int32_t)ch) : y;

    clamp_position(ax, ay, cw, ch, out_x, out_y);

    *out_w = cw;

    *out_h = ch;
}

Rect shell_chrome_rect_new(int32_t x, int32_t y, uint32_t w, uint32_t h) {
    return (Rect){x, y, w, h};
}

bool shell_chrome_rect_contains(const Rect *rect, int32_t x, int32_t y) {
    return x >= rect->x && x < chrome_add(rect->x, (int32_t)rect->w) && y >= rect->y &&
           y < chrome_add(rect->y, (int32_t)rect->h);
}

static bool chrome_budget(uint32_t *budget) {
    if (!*budget)
        return false;

    --*budget;

    return true;
}

static size_t chrome_document_index(uint64_t frame) {
    size_t index = 0;

    for (uint32_t kind = SHELL_APP_QUIL; kind <= SHELL_APP_BROWSER; kind += 2) {
        for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
            ShellAppCatalogRow row = shell_custody_catalog_at(i);

            if (row.app != kind)
                continue;

            if ((row.shell_surface ? row.shell_surface : row.surface) == frame)
                return index;

            ++index;
        }
    }
    return SHELL_DOCUMENT_VIEW_COUNT;
}

static size_t chrome_spindle_index(uint64_t frame) {
    size_t index = 0;

    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        ShellAppCatalogRow row = shell_custody_catalog_at(i);

        if (row.app != SHELL_APP_SPINDLE)
            continue;

        if ((row.shell_surface ? row.shell_surface : row.surface) == frame)
            return index;

        ++index;
    }
    return SHELL_SPINDLE_VIEW_COUNT;
}

static bool chrome_live_custody(ShellChromeContext *ctx, uint64_t frame, ShellAppCustody *out) {
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        if (!ctx->apps->present[i])
            continue;

        ShellAppCustody entry = ctx->apps->entries[i];

        if (!entry.dead && !entry.close_pending && shell_custody_owns_surface(&entry, frame)) {
            *out = entry;

            return true;
        }
    }
    return false;
}

void shell_chrome_native_child_geometry(ShellChromeContext *ctx, uint64_t frame, int32_t x,
                                        int32_t y, uint32_t w, uint32_t h) {
    ShellAppCustody entry = {0};

    if (!chrome_live_custody(ctx, frame, &entry))
        return;

    size_t spindle = chrome_spindle_index(frame);

    if (spindle < SHELL_SPINDLE_VIEW_COUNT)
        shell_spindle_delivery_queue(&ctx->apps->spindle[spindle], entry.target, entry.task, w, h);

    size_t document = chrome_document_index(frame);

    if (document < SHELL_DOCUMENT_VIEW_COUNT) {
        bool inset = false;

        for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
            ShellAppCatalogRow row = shell_custody_catalog_at(i);

            if (row.app != SHELL_APP_SPINDLE && row.shell_surface == frame) {
                inset = true;

                break;
            }
        }
        uint32_t vw = inset ? chrome_sat_sub(w, 16) : w, vh = inset ? chrome_sat_sub(h, 44) : h;

        shell_document_delivery_queue(&ctx->apps->document[document], entry.target, entry.task, vw,
                                      vh);
    }
    ShellViewChildren children = shell_desktop_children(frame, x, y, w, h);

    for (size_t i = 0; i < 4; ++i) {
        ShellViewChild child = children.values[i];

        if (!child.present)
            continue;

        if (child.w && child.h)
            pdx_call(SLOT_DISPLAY, 0xEC, child.sid,
                     ((uint64_t)(uint32_t)child.y << 32) | (uint32_t)child.x,
                     ((uint64_t)child.h << 32) | child.w);

        sexos_line("[shell.app.viewport] target=%u sid=%llu x=%d y=%d w=%u h=%u", entry.target,
                   U64(child.sid), child.x, child.y, child.w, child.h);
    }
}

void shell_chrome_update_local_geometry(ShellChromeContext *ctx, uint64_t surface_id, int32_t x,
                                        int32_t y, uint32_t w, uint32_t h) {
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        if (!ctx->apps->present[i])
            continue;

        ShellAppCustody entry = ctx->apps->entries[i];

        if ((entry.dead || entry.close_pending) && shell_custody_owns_surface(&entry, surface_id))
            return;
    }
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        if (!ctx->apps->present[i])
            continue;

        ShellAppCustody entry = ctx->apps->entries[i];

        if (!entry.dead &&
            (entry.surface == surface_id || shell_custody_is_successor_frame(&entry, surface_id))) {
            ctx->apps->entries[i].has_bounds = true;

            ctx->apps->entries[i].x = x;

            ctx->apps->entries[i].y = y;

            ctx->apps->entries[i].w = w;

            ctx->apps->entries[i].h = h;

            break;
        }
    }
    native_child_geometry(surface_id, x, y, w, h);

    switch (surface_id) {
    case SURFACE_ID_APP:
        if (WINDOWS_len > 1) {
            WINDOWS[1].desc.x = x;

            WINDOWS[1].desc.y = y;

            WINDOWS[1].desc.width = w;

            WINDOWS[1].desc.height = h;
        }
        SURFACE_100_W = w;

        SURFACE_100_H = h;

        break;

    case SURFACE_ID_STATIC:
        SURFACE_101_X = x;

        SURFACE_101_Y = y;

        SURFACE_101_W = w;

        SURFACE_101_H = h;

        break;

    case SURFACE_ID_TEST3:
        SURFACE_102_X = x;

        SURFACE_102_Y = y;

        SURFACE_102_W = w;

        SURFACE_102_H = h;

        break;

    case SURFACE_ID_TEST4:
        SURFACE_103_X = x;

        SURFACE_103_Y = y;

        SURFACE_103_W = w;

        SURFACE_103_H = h;

        break;

    case SURFACE_ID_LINEN: {
        bool resized = SURFACE_200_W != w || SURFACE_200_H != h;

        SURFACE_200_X = x;

        SURFACE_200_Y = y;

        SURFACE_200_W = w;

        SURFACE_200_H = h;

        if (resized && linen_object_count())
            linen_render_object_list();

        break;
    }
    case SURFACE_ID_QUIL:
        SURFACE_201_X = x;

        SURFACE_201_Y = y;

        SURFACE_201_W = w;

        SURFACE_201_H = h;

        break;

    case SURFACE_ID_SPINDLE:
        SURFACE_0x99_X = x;

        SURFACE_0x99_Y = y;

        SURFACE_0x99_W = w;

        SURFACE_0x99_H = h;

        break;

    case SURFACE_ID_MESH:
        SURFACE_202_X = x;

        SURFACE_202_Y = y;

        SURFACE_202_W = w;

        SURFACE_202_H = h;

        break;

    case SURFACE_ID_COLLAR:
        SURFACE_203_X = x;

        SURFACE_203_Y = y;

        SURFACE_203_W = w;

        SURFACE_203_H = h;

        break;

    case SURFACE_ID_BELL_PLACEHOLDER:
        SURFACE_204_X = x;

        SURFACE_204_Y = y;

        SURFACE_204_W = w;

        SURFACE_204_H = h;

        break;

    default:
        break;
    }
}

bool shell_chrome_is_closeable_surface(ShellChromeContext *ctx, uint64_t surface_id) {
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        if (!ctx->apps->present[i])
            continue;

        ShellAppCustody entry = ctx->apps->entries[i];

        if (shell_custody_owns_surface(&entry, surface_id))
            return !entry.dead && !entry.close_pending;
    }
    bool result = false;

    const char *reason = "";

    switch (surface_id) {
    case SURFACE_ID_CURSOR:
    case SURFACE_ID_LAUNCHER:
    case SURFACE_ID_STATUS:
    case SURFACE_ID_CLOCK:
    case SURFACE_ID_BELL:
    case SURFACE_ID_SCENE_SETTINGS:
        reason = "os_protected";

        break;

    case SURFACE_ID_APP:
    case SURFACE_ID_STATIC:
    case SURFACE_ID_TEST3:
    case SURFACE_ID_TEST4:
        result = true;

        reason = "app_disposable";

        break;

    default: {
        bool allowed = false;

        uint8_t state = 0;

        if (shell_chrome_peer_app_closeable(ctx, surface_id, &allowed)) {
            result = allowed;

            reason = "registry";

            sexos_line("[shell.app_registry.lookup] closeable sid=%llu val=%s", U64(surface_id),
                       allowed ? "true" : "false");

        } else if (lifecycle_state(surface_id, &state)) {
            result = true;

            reason = "lifecycle";

        } else {
            result = surface_is_alive(surface_id);

            reason = "alive_fallback";
        }
        break;
    }
    }
    if (chrome_budget(&ctx->chrome->is_closeable_surface__CLOSE_ALLOWED_GATE_BUDGET))
        sexos_line("[silk.close_allowed.gate] sid=%llu allowed=%u reason=%s", U64(surface_id),
                   (unsigned)result, reason);

    return result;
}

bool shell_chrome_frame_close_allowed(ShellChromeContext *ctx, uint32_t frame_id) {
    uint64_t sid = 0;

    return active_surface_for_frame(frame_id, &sid) && is_closeable_surface(sid);
}

void shell_chrome_emit_chrome_diagnostics(ShellChromeContext *ctx, uint32_t frame_id,
                                          const char *reason) {
    if (!VISIBLE_FOCUS_TOPBAR_PROOF_ENABLED)
        return;

    uint64_t sid = 0;

    if (!active_surface_for_frame(frame_id, &sid))
        return;

    bool zoomed = frame_is_zoomed(frame_id), minimized = frame_is_minimized(frame_id),
         focused = FOCUSED_SURFACE_ID == sid, top = frame_has_top_bar(frame_id);

    int32_t bar = top ? FRAME_TOP_BAR_HEIGHT_PX : FRAME_RIM_PX,
            toolbar = top ? FRAME_TOP_BAR_HEIGHT_PX : 0;

    sexos_line(
        "[shell.frame.chrome.size] frame=%u sid=%llu topbar_h=%d tab_h=%d toolbar_h=%d zoomed=%u minimized=%u focused=%u reason=%s",
        frame_id, U64(sid), bar, FRAME_TAB_STRIP_PX, toolbar, (unsigned)zoomed, (unsigned)minimized,
        (unsigned)focused, reason);

    int32_t x = 0, y = 0;

    uint32_t w = 0, h = 0;

    if (get_surface_bounds(sid, &x, &y, &w, &h)) {
        bool active = false;

        for (size_t i = 0; i < MAX_FRAMES; ++i)
            if (FRAMES[i].present && FRAMES[i].frame_id == frame_id &&
                FRAMES[i].scene_id == ACTIVE_SCENE_IDX) {
                active = true;

                break;
            }
        sexos_line(
            "[shell.frame.chrome.state] frame=%u sid=%llu x=%d y=%d w=%u h=%u focused=%u active=%u reason=%s",
            frame_id, U64(sid), x, y, w, h, (unsigned)focused, (unsigned)active, reason);
    }
}

bool shell_chrome_frame_chrome_operation(ShellChromeContext *ctx, uint32_t frame_id,
                                         ShellChromeOperation *out) {
    uint64_t sid = 0;

    if (!active_surface_for_frame(frame_id, &sid))
        return false;

    uint64_t count = frame_tab_count(frame_id), active = frame_active_tab_index(frame_id);

    uint64_t bar = frame_has_top_bar(frame_id),
             hover = HOVERED_FRAME_ID && HOVERED_FRAME_ID == frame_id;

    uint64_t light = hover && HOVERED_FRAME_LIGHT != FRAME_LIGHT_NONE, kind = 3;

    if (light) {
        if (HOVERED_FRAME_LIGHT == FRAME_LIGHT_CLOSE)
            kind = 0;

        else if (HOVERED_FRAME_LIGHT == FRAME_LIGHT_MINIMIZE)
            kind = 1;

        else if (HOVERED_FRAME_LIGHT == FRAME_LIGHT_ZOOM)
            kind = 2;
    }
    uint64_t close = frame_close_allowed(frame_id);

    uint64_t flags = bar | (hover << 1) | (light << 2) | (kind << 3) | (close << 5);

    *out = (ShellChromeOperation){OP_SURFACE_TAB_INFO, sid, count, active | (flags << 8)};

    return true;
}

void shell_chrome_send_frame_tab_info(ShellChromeContext *ctx, uint32_t frame_id) {
    ShellChromeOperation op = {0};

    if (!frame_chrome_operation(frame_id, &op))
        return;

    uint64_t flags = op.control >> 8, light = (flags >> 2) & 1;

    pdx_call(SLOT_DISPLAY, OP_SURFACE_TAB_INFO, op.surface, op.packed, op.control);

    if (chrome_budget(&ctx->chrome->send_frame_tab_info__SHELL_TAB_INFO_SEND_BUDGET))
        sexos_line(
            "[shell.frame.tab.info.send] frame=%u surface=%llu tabs=%llu active=%llu top_bar=%llu hover=%llu light=%u/\"%s\"",
            frame_id, U64(op.surface), U64(op.packed), U64(op.control & 255), U64(flags & 1),
            U64((flags >> 1) & 1), HOVERED_FRAME_LIGHT, light ? "some" : "none");
}

typedef struct ChromeCloseRemoval {
    bool emptied, survived, next;

    uint32_t emptied_id, survived_id;

    uint64_t next_sid;

} ChromeCloseRemoval;

static bool chrome_close_preflight(ShellChromeContext *ctx, uint64_t sid) {
    uint8_t state = 0;

    if (!lifecycle_state(sid, &state)) {
        sexos_line("[silk.lifecycle.close.reject] sid=%llu reason=unknown_lifecycle", U64(sid));

        return false;
    }
    if (LIFECYCLE_GENERATION > UINT64_MAX - 3) {
        sexos_line("[silk.lifecycle.close.reject] sid=%llu reason=generation_exhausted", U64(sid));

        return false;
    }
    for (size_t i = 0; i < MAX_FRAMES; ++i)
        if (FRAMES[i].present) {
            ShellFrame frame = FRAMES[i];

            for (size_t j = 0; j < MAX_TABS_PER_FRAME; ++j)
                if (frame.tabs[j].present && frame.tabs[j].surface_id == sid) {
                    if (frame.tab_count > MAX_TABS_PER_FRAME ||
                        frame.active_tab >= MAX_TABS_PER_FRAME) {
                        sexos_line(
                            "[silk.lifecycle.close.reject] sid=%llu reason=invalid_frame_shape",
                            U64(sid));

                        return false;
                    }
                    return true;
                }
        }
    return true;
}

static void chrome_cancel_close_interaction(ShellChromeContext *ctx, uint64_t sid) {
    if (INTERACTION.tag == INTERACTION_DRAGGING && INTERACTION.surface_id == sid) {
        sexos_line("[frame.light.close.reject.drag] sid=%llu cancel_drag", U64(sid));

        uint8_t state = LIFECYCLE_ALLOCATED;

        (void)lifecycle_state(sid, &state);

        record_tombstone_event(sid, state, state, TOMBSTONE_DRAG_CANCELLED);

        sexos_line("[silk.close.state.clear] sid=%llu reason=drag_cancel", U64(sid));

        try_transition((InteractionState){.tag = INTERACTION_IDLE});
    }
    if (INTERACTION.tag == INTERACTION_RESIZING && INTERACTION.surface_id == sid) {
        sexos_line("[silk.close.state.clear] sid=%llu reason=resize_cancel", U64(sid));

        try_transition((InteractionState){.tag = INTERACTION_IDLE});
    }
    if (INTERACTION.tag == INTERACTION_TAB_DRAGGING) {
        uint32_t frame = 0;

        if (frame_for_surface(sid, &frame) && INTERACTION.frame_id == frame) {
            sexos_line("[silk.close.state.clear] sid=%llu reason=tab_drag_cancel", U64(sid));

            try_transition((InteractionState){.tag = INTERACTION_IDLE});
        }
    }
}

static ChromeCloseRemoval chrome_remove_closed_tab(ShellChromeContext *ctx, uint64_t sid) {
    ChromeCloseRemoval out = {0};

    for (size_t i = 0; i < MAX_FRAMES; ++i) {
        if (!FRAMES[i].present)
            continue;

        size_t found = MAX_TABS_PER_FRAME;

        for (size_t j = 0; j < MAX_TABS_PER_FRAME; ++j)
            if (FRAMES[i].tabs[j].present && FRAMES[i].tabs[j].surface_id == sid) {
                found = j;

                break;
            }
        if (found == MAX_TABS_PER_FRAME)
            continue;

        for (size_t j = found; j + 1 < MAX_TABS_PER_FRAME; ++j)
            FRAMES[i].tabs[j] = FRAMES[i].tabs[j + 1];

        FRAMES[i].tabs[MAX_TABS_PER_FRAME - 1] = (ShellTab){0};

        if (FRAMES[i].tab_count)
            --FRAMES[i].tab_count;

        if (FRAMES[i].tab_count) {
            if (FRAMES[i].active_tab == found)
                FRAMES[i].active_tab = 0;

            else if (FRAMES[i].active_tab > found)
                --FRAMES[i].active_tab;

            uint8_t active = FRAMES[i].active_tab;

            if (FRAMES[i].tabs[active].present) {
                out.next = true;

                out.next_sid = FRAMES[i].tabs[active].surface_id;
            }
            out.survived = true;

            out.survived_id = FRAMES[i].frame_id;

        } else {
            out.emptied = true;

            out.emptied_id = FRAMES[i].frame_id;
        }
        break;
    }
    return out;
}

bool shell_chrome_close_surface_from_frame_light(ShellChromeContext *ctx, uint64_t surface_id) {
    sexos_line("[silk.close.request] sid=%llu", U64(surface_id));

    sexos_line("[silk.lifecycle.close.begin] sid=%llu", U64(surface_id));

    if (!is_closeable_surface(surface_id)) {
        sexos_line("[silk.close.blocked.core] sid=%llu reason=not_closeable", U64(surface_id));

        return false;
    }
    sexos_line("[silk.close.allowed] sid=%llu", U64(surface_id));

    uint8_t state = 0;

    if (lifecycle_state(surface_id, &state) &&
        (state == LIFECYCLE_CLOSING || state == LIFECYCLE_TOMBSTONED ||
         state == LIFECYCLE_DESTROYED)) {
        const char *label = chrome_life_name(state);

        sexos_line("[tombstone.close.reject.dead] sid=%llu state=%s", U64(surface_id), label);

        sexos_line("[lifecycle.destroy.reject] sid=%llu state=%s reason=already_dead",
                   U64(surface_id), label);

        sexos_line(
            "[silk.lifecycle.dead.reject] sid=%llu state=%s reason=already_closing_tombstoned_or_destroyed",
            U64(surface_id), label);

        return false;
    }
    if (!chrome_close_preflight(ctx, surface_id))
        return false;

    shell_request_app_close(surface_id);

    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        if (!ctx->apps->present[i])
            continue;

        ShellAppCustody entry = ctx->apps->entries[i];

        ShellAppCatalogRow row = shell_custody_catalog_find(entry.target, entry.surface);

        if (shell_custody_owns_surface(&entry, surface_id) && row.present &&
            row.app == SHELL_APP_QUIL && entry.close_pending && !entry.dead) {
            sexos_line("[silk.close.deferred] sid=%llu reason=app_decides", U64(surface_id));

            return true;
        }
    }
    chrome_cancel_close_interaction(ctx, surface_id);

    if (FOCUSED_SURFACE_ID == surface_id)
        clear_focus_if_dead();

    switch (surface_id) {
    case SURFACE_ID_APP:
        SURFACE_100_ALIVE = false;

        break;

    case SURFACE_ID_STATIC:
        SURFACE_101_ALIVE = false;

        break;

    case SURFACE_ID_TEST3:
        SURFACE_102_ALIVE = false;

        break;

    case SURFACE_ID_TEST4:
        SURFACE_103_ALIVE = false;

        break;

    default:
        break;
    }
    sexos_line("[silk.close.tombstone] sid=%llu", U64(surface_id));

    uint8_t old = LIFECYCLE_VISIBLE;

    (void)lifecycle_state(surface_id, &old);

    if (!set_lifecycle_state(surface_id, LIFECYCLE_CLOSING))
        return false;

    record_tombstone_event(surface_id, old, LIFECYCLE_CLOSING, TOMBSTONE_CLOSE_REQUESTED);

    if (!set_lifecycle_state(surface_id, LIFECYCLE_TOMBSTONED))
        return false;

    record_tombstone_event(surface_id, LIFECYCLE_CLOSING, LIFECYCLE_TOMBSTONED,
                           TOMBSTONE_CLOSE_REQUESTED);

    if (!set_lifecycle_state(surface_id, LIFECYCLE_DESTROYED))
        return false;

    record_tombstone_event(surface_id, LIFECYCLE_TOMBSTONED, LIFECYCLE_DESTROYED,
                           TOMBSTONE_FINAL_DESTROY);

    sexos_line("[lifecycle.destroy.record] sid=%llu", U64(surface_id));

    sexos_line("[frame.light.close.fsm] sid=%llu", U64(surface_id));

    sexos_line("[silk.lifecycle.tab.close.ok] sid=%llu", U64(surface_id));

    pdx_call(SLOT_DISPLAY, OP_SURFACE_DEACTIVATE, surface_id, 0, 0);

    ChromeCloseRemoval removal = chrome_remove_closed_tab(ctx, surface_id);

    if (removal.emptied) {
        sexos_line("[silk.close.frame.empty] frame=%u sid=%llu", removal.emptied_id,
                   U64(surface_id));

        sexos_line("[silk.lifecycle.frame.empty.destroy] frame=%u sid=%llu", removal.emptied_id,
                   U64(surface_id));

        size_t i = chrome_frame_slot(ctx, removal.emptied_id);

        if (i < MAX_FRAMES)
            FRAMES[i] = (ShellFrame){0};
    }
    if (removal.survived) {
        sexos_line(
            "[silk.chrome.glitch.fix] frame=%u sid=%llu pushing updated tab_count to sexdisplay",
            removal.survived_id, U64(surface_id));

        send_frame_tab_info(removal.survived_id);
    }
    clear_focus_if_dead();

    if (removal.next && surface_is_alive(removal.next_sid) && !is_tombstoned(removal.next_sid)) {
        sexos_line("[silk.close.focus.next] sid=%llu next=%llu", U64(surface_id),
                   U64(removal.next_sid));

        try_set_focus(removal.next_sid);
    }
    sexos_line("[silk.lifecycle.focus.next_live] after_close_sid=%llu current_focus=%llu",
               U64(surface_id), U64(FOCUSED_SURFACE_ID));

    clear_drag_if_dead();

    clear_hover_if_dead();

    clear_hover_if_wrong_scene();

    clear_hover_if_wrong_scene();

    sexos_line("[silk.close.state.clear] sid=%llu reason=post_close_sweep", U64(surface_id));

    tile_active_scene_frames();

    snap_capture_layout();

    sexos_line("[silk.lifecycle.close.done] sid=%llu ok=1", U64(surface_id));

    return true;
}

bool shell_chrome_minimize_frame(ShellChromeContext *ctx, uint32_t frame_id) {
    sexos_line("[silk.lifecycle.minimize.begin] frame=%u", frame_id);

    if (frame_is_minimized(frame_id))
        return false;

    uint64_t sid = 0;

    if (!active_surface_for_frame(frame_id, &sid) || !surface_is_alive(sid))
        return false;

    /* Admission precedes local flags and external hide operations. */
    uint8_t state = 0;

    if (!lifecycle_state(sid, &state)) {
        sexos_line("[silk.lifecycle.minimize.reject] frame=%u reason=unknown_lifecycle", frame_id);

        return false;
    }
    set_frame_minimized(frame_id, true);

    sexos_line("[silk.lifecycle.minimize.snapshot] frame=%u sid=%llu scene=%u", frame_id, U64(sid),
               (unsigned)ACTIVE_SCENE_IDX);

    if (!set_lifecycle_state(sid, LIFECYCLE_MINIMIZED))
        return false;

    if (sid == SURFACE_ID_COMMAND_PALETTE) {
        ctx->apps->palette_visibility = (ShellChromeLatestOperation){true, {0xEE, sid, 0, 0}};

    } else
        pdx_call(SLOT_DISPLAY, OP_SURFACE_DEACTIVATE, sid, 0, 0);

    sexos_line("[silk.lifecycle.minimize.hidden] sid=%llu frame=%u", U64(sid), frame_id);

    clear_drag_if_dead();

    if (HOVERED_FRAME_ID == frame_id) {
        HOVERED_FRAME_ID = 0;

        HOVER_KIND = HOVER_NONE;

        HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

        sexos_line("[shell.frame.minimize.hover.clear] frame=%u", frame_id);
    }
    clear_focus_if_dead();

    if (chrome_budget(&ctx->chrome->minimize_frame__FRAME_MINIMIZE_BUDGET))
        sexos_line("[frame.light.minimize.fsm] frame=%u surface=%llu", frame_id, U64(sid));

    tile_active_scene_frames();

    sexos_line("[shell.interact.tile.return] source=minimize frame=%u", frame_id);

    sexos_line("[shell.interact.minimize] frame=%u sid=%llu", frame_id, U64(sid));

    if (chrome_budget(&ctx->chrome->minimize_frame__TILE_AFTER_MINIMIZE_BUDGET))
        sexos_line("[shell.tile.after_minimize] frame=%u", frame_id);

    snap_capture_layout();

    return true;
}

typedef struct ChromeRestoreCheckpoint {
    size_t frame_slot, life_slot, document, spindle;

    ShellFrame prior_frame;

    SurfaceLifecycle prior_life;

    ShellDocumentDelivery prior_document;

    ShellSpindleDelivery prior_spindle;

} ChromeRestoreCheckpoint;

static bool chrome_restore_checkpoint(ShellChromeContext *ctx, uint32_t frame_id, uint64_t sid,
                                      ChromeRestoreCheckpoint *out) {
    size_t frame = chrome_frame_slot(ctx, frame_id), life = LIFECYCLE_MAX_SURFACES;

    for (size_t i = 0; i < LIFECYCLE_MAX_SURFACES; ++i)
        if (LIFECYCLE_TABLE[i].present && LIFECYCLE_TABLE[i].surface_id == sid) {
            life = i;

            break;
        }
    if (frame == MAX_FRAMES || life == LIFECYCLE_MAX_SURFACES)
        return false;

    size_t document = chrome_document_index(sid), spindle = chrome_spindle_index(sid);

    out->frame_slot = frame;

    out->life_slot = life;

    out->document = document;

    out->spindle = spindle;

    out->prior_frame = FRAMES[frame];

    out->prior_life = LIFECYCLE_TABLE[life].lc;

    if (document < SHELL_DOCUMENT_VIEW_COUNT)
        out->prior_document = ctx->apps->document[document];

    if (spindle < SHELL_SPINDLE_VIEW_COUNT)
        out->prior_spindle = ctx->apps->spindle[spindle];

    return true;
}

static void chrome_restore_refresh(ShellChromeContext *ctx, const ChromeRestoreCheckpoint *saved) {
    if (saved->document < SHELL_DOCUMENT_VIEW_COUNT)
        shell_document_delivery_refresh(&ctx->apps->document[saved->document]);

    if (saved->spindle < SHELL_SPINDLE_VIEW_COUNT)
        shell_spindle_delivery_refresh(&ctx->apps->spindle[saved->spindle]);
}

static void chrome_restore_rollback(ShellChromeContext *ctx, const ChromeRestoreCheckpoint *saved) {
    FRAMES[saved->frame_slot] = saved->prior_frame;

    LIFECYCLE_TABLE[saved->life_slot].lc = saved->prior_life;

    if (saved->document < SHELL_DOCUMENT_VIEW_COUNT)
        ctx->apps->document[saved->document] = saved->prior_document;

    if (saved->spindle < SHELL_SPINDLE_VIEW_COUNT)
        ctx->apps->spindle[saved->spindle] = saved->prior_spindle;
}

bool shell_chrome_restore_minimized_frame_inner(ShellChromeContext *ctx, uint32_t frame_id,
                                                bool focus) {
    sexos_line("[silk.lifecycle.restore.begin] frame=%u", frame_id);

    if (!frame_is_minimized(frame_id))
        return false;

    uint64_t sid = 0;

    if (!active_surface_for_frame(frame_id, &sid) || !surface_is_alive(sid))
        return false;

    uint8_t state = 0;

    if (lifecycle_state(sid, &state) &&
        (state == LIFECYCLE_CLOSING || state == LIFECYCLE_TOMBSTONED ||
         state == LIFECYCLE_DESTROYED)) {
        sexos_line("[lifecycle.tombstone.reject_restore] sid=%llu state=%s", U64(sid),
                   chrome_life_name(state));

        sexos_line("[silk.lifecycle.restore.reject_dead] frame=%u sid=%llu state=%s", frame_id,
                   U64(sid), chrome_life_name(state));

        return false;
    }
    if (!lifecycle_state(sid, &state)) {
        sexos_line("[silk.lifecycle.restore.reject] frame=%u reason=unknown_lifecycle", frame_id);

        return false;
    }
    ChromeRestoreCheckpoint saved = {0};

    if (!chrome_restore_checkpoint(ctx, frame_id, sid, &saved))
        return false;

    chrome_restore_refresh(ctx, &saved);

    set_frame_minimized(frame_id, false);

    if (!set_lifecycle_state(sid, LIFECYCLE_VISIBLE))
        return false;

    int32_t x = 0, y = 0;

    uint32_t w = 0, h = 0;

    bool zoomed = frame_is_zoomed(frame_id);

    if (zoomed)
        layout_maximize(&x, &y, &w, &h);

    else if (!get_surface_bounds(sid, &x, &y, &w, &h)) {
        chrome_restore_rollback(ctx, &saved);

        sexos_line("[silk.lifecycle.restore.reject] frame=%u reason=missing_geometry rolled_back=1",
                   frame_id);

        return false;
    }
    if (sid == SURFACE_ID_COMMAND_PALETTE)
        ctx->apps->palette_visibility = (ShellChromeLatestOperation){
            true,
            {0xEC, sid, ((uint64_t)(uint32_t)y << 32) | (uint32_t)x, ((uint64_t)h << 32) | w}};

    else
        pdx_call(SLOT_DISPLAY, 0xEC, sid, ((uint64_t)y << 32) | (uint64_t)x,
                 ((uint64_t)h << 32) | w);

    if (zoomed)
        update_local_geometry(sid, x, y, w, h);

    send_frame_tab_info(frame_id);

    if (focus && desktop_ready(sid))
        try_set_focus(sid);

    sexos_line("[frame.light.restore.fsm] frame=%u surface=%llu", frame_id, U64(sid));

    sexos_line("[silk.lifecycle.restore.record] frame=%u sid=%llu state=Visible", frame_id,
               U64(sid));

    tile_active_scene_frames();

    sexos_line("[shell.interact.tile.return] source=restore frame=%u", frame_id);

    sexos_line("[shell.interact.restore] frame=%u sid=%llu", frame_id, U64(sid));

    if (chrome_budget(&ctx->chrome->restore_minimized_frame__FRAME_RESTORE_BUDGET))
        sexos_line("[shell.frame.restore] frame=%u surface=%llu", frame_id, U64(sid));

    snap_capture_layout();

    emit_chrome_diagnostics(frame_id, "restore");

    sexos_line("[silk.lifecycle.restore.ok] frame=%u sid=%llu", frame_id, U64(sid));

    return true;
}

bool shell_chrome_restore_minimized_frame(ShellChromeContext *ctx, uint32_t frame_id) {
    return restore_minimized_frame_inner(frame_id, true);
}

bool shell_chrome_zoom_frame(ShellChromeContext *ctx, uint32_t frame_id) {
    sexos_line("[silk.lifecycle.zoom.begin] frame=%u", frame_id);

    if (frame_is_zoomed(frame_id) || frame_is_minimized(frame_id))
        return false;

    uint64_t sid = 0;

    if (!active_surface_for_frame(frame_id, &sid))
        return false;

    if (!surface_is_alive(sid)) {
        sexos_line("[silk.lifecycle.zoom.reject_dead] frame=%u sid=%llu reason=surface_not_alive",
                   frame_id, U64(sid));

        return false;
    }
    int32_t x = 0, y = 0;

    uint32_t w = 0, h = 0;

    if (!get_surface_bounds(sid, &x, &y, &w, &h))
        return false;

    size_t i = chrome_frame_slot(ctx, frame_id);

    if (i < MAX_FRAMES) {
        FRAMES[i].normal_x = x;

        FRAMES[i].normal_y = y;

        FRAMES[i].normal_w = w;

        FRAMES[i].normal_h = h;
    }
    sexos_line("[silk.lifecycle.zoom.snapshot] frame=%u nx=%d ny=%d nw=%u nh=%u", frame_id, x, y, w,
               h);

    set_frame_zoomed(frame_id, true);

    layout_maximize(&x, &y, &w, &h);

    pdx_call(SLOT_DISPLAY, 0xEC, sid, ((uint64_t)y << 32) | (uint64_t)x, ((uint64_t)h << 32) | w);

    update_local_geometry(sid, x, y, w, h);

    sexos_line("[silk.lifecycle.zoom.active] frame=%u sid=%llu zx=%d zy=%d zw=%u zh=%u", frame_id,
               U64(sid), x, y, w, h);

    HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

    if (chrome_budget(&ctx->chrome->zoom_frame__FRAME_ZOOM_BUDGET))
        sexos_line("[shell.frame.zoom] frame=%u surface=%llu", frame_id, U64(sid));

    snap_capture_layout();

    emit_chrome_diagnostics(frame_id, "zoom");

    return true;
}

bool shell_chrome_unzoom_frame(ShellChromeContext *ctx, uint32_t frame_id) {
    if (!frame_is_zoomed(frame_id))
        return false;

    uint64_t sid = 0;

    if (!active_surface_for_frame(frame_id, &sid) || !surface_is_alive(sid))
        return false;

    size_t i = chrome_frame_slot(ctx, frame_id);

    if (i == MAX_FRAMES)
        return false;

    ShellFrame frame = FRAMES[i];

    set_frame_zoomed(frame_id, false);

    pdx_call(SLOT_DISPLAY, 0xEC, sid, ((uint64_t)frame.normal_y << 32) | (uint64_t)frame.normal_x,
             ((uint64_t)frame.normal_h << 32) | frame.normal_w);

    update_local_geometry(sid, frame.normal_x, frame.normal_y, frame.normal_w, frame.normal_h);

    sexos_line("[silk.lifecycle.zoom.restore] frame=%u sid=%llu nx=%d ny=%d nw=%u nh=%u", frame_id,
               U64(sid), frame.normal_x, frame.normal_y, frame.normal_w, frame.normal_h);

    HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

    if (chrome_budget(&ctx->chrome->unzoom_frame__FRAME_UNZOOM_BUDGET))
        sexos_line("[shell.frame.unzoom] frame=%u surface=%llu", frame_id, U64(sid));

    tile_active_scene_frames();

    if (chrome_budget(&ctx->chrome->unzoom_frame__TILE_AFTER_UNZOOM_BUDGET))
        sexos_line("[shell.tile.after_unzoom] frame=%u", frame_id);

    snap_capture_layout();

    emit_chrome_diagnostics(frame_id, "unzoom");

    return true;
}

bool shell_chrome_toggle_zoom_frame(ShellChromeContext *ctx, uint32_t frame_id) {
    uint64_t sid = 0;

    uint8_t state = 0;

    if (active_surface_for_frame(frame_id, &sid) && lifecycle_state(sid, &state) &&
        (state == LIFECYCLE_CLOSING || state == LIFECYCLE_TOMBSTONED ||
         state == LIFECYCLE_DESTROYED)) {
        sexos_line("[frame.light.zoom.fsm.reject] frame=%u surface=%llu lifecycle=invalid",
                   frame_id, U64(sid));

        return false;
    }
    bool result = frame_is_zoomed(frame_id) ? unzoom_frame(frame_id) : zoom_frame(frame_id);

    if (result)
        sexos_line("[frame.light.zoom.fsm] frame=%u", frame_id);

    else
        sexos_line("[frame.light.zoom.fsm.fail] frame=%u zoomed=%u minimized=%u", frame_id,
                   (unsigned)frame_is_zoomed(frame_id), (unsigned)frame_is_minimized(frame_id));

    return result;
}

static Rect chrome_tile_rect(size_t index, size_t count, uint32_t width, uint32_t height) {
    uint32_t half_w = width / 2, half_h = height / 2;

    if (count == 1)
        return (Rect){0, P.bar_height, width, height};

    if (count == 2)
        return (Rect){index ? (int32_t)half_w : 0, P.bar_height, index ? width - half_w : half_w,
                      height};

    if (count == 3) {
        if (index == 0)
            return (Rect){0, P.bar_height, half_w, height};

        return (Rect){(int32_t)half_w,
                      index == 1 ? P.bar_height : chrome_add(P.bar_height, (int32_t)half_h),
                      width - half_w, index == 1 ? half_h : height - half_h};
    }
    if (count == 4)
        return (Rect){index % 2 ? (int32_t)half_w : 0,
                      index < 2 ? P.bar_height : chrome_add(P.bar_height, (int32_t)half_h),
                      index % 2 ? width - half_w : half_w, index < 2 ? half_h : height - half_h};

    uint32_t row = height / (uint32_t)count, offset = row * (uint32_t)index;

    return (Rect){0, chrome_add(P.bar_height, (int32_t)offset), width,
                  index + 1 == count ? height - offset : row};
}

static bool chrome_tile_candidate(ShellChromeContext *ctx, uint64_t sid) {
    const char *reason = "";

    if (!surface_is_alive(sid))
        reason = "dead";

    else if (is_tombstoned(sid))
        reason = "tombstoned";

    else if (!surface_is_lifecycle_focusable(sid))
        reason = "lifecycle";

    else {
        FocusRef ref = {0};

        if (!make_focus_ref(sid, &ref) || focus_ref_is_current(&ref))
            return true;

        reason = "generation";
    }
    sexos_line("[tiling.frame.skip] sid=%llu reason=%s", U64(sid), reason);

    sexos_line("[shell.tile.skip_dead] sid=%llu reason=%s", U64(sid), reason);

    return false;
}

static void chrome_tile_shadow(ShellChromeContext *ctx, uint64_t sid, Rect r) {
    switch (sid) {
    case SURFACE_ID_APP:
        if (WINDOWS_len > 1) {
            WINDOWS[1].desc.x = r.x;
            WINDOWS[1].desc.y = r.y;
            WINDOWS[1].desc.width = r.w;
            WINDOWS[1].desc.height = r.h;
        }
        break;

    case SURFACE_ID_STATIC:
        SURFACE_101_X = r.x;
        SURFACE_101_Y = r.y;
        SURFACE_101_W = r.w;
        SURFACE_101_H = r.h;
        break;

    case SURFACE_ID_TEST3:
        SURFACE_102_X = r.x;
        SURFACE_102_Y = r.y;
        SURFACE_102_W = r.w;
        SURFACE_102_H = r.h;
        break;

    case SURFACE_ID_TEST4:
        SURFACE_103_X = r.x;
        SURFACE_103_Y = r.y;
        SURFACE_103_W = r.w;
        SURFACE_103_H = r.h;
        break;

    case SURFACE_ID_LINEN:
        SURFACE_200_X = r.x;
        SURFACE_200_Y = r.y;
        SURFACE_200_W = r.w;
        SURFACE_200_H = r.h;
        break;

    case SURFACE_ID_QUIL:
        SURFACE_201_X = r.x;
        SURFACE_201_Y = r.y;
        SURFACE_201_W = r.w;
        SURFACE_201_H = r.h;
        break;

    case SURFACE_ID_MESH:
        SURFACE_202_X = r.x;
        SURFACE_202_Y = r.y;
        SURFACE_202_W = r.w;
        SURFACE_202_H = r.h;
        break;

    case SURFACE_ID_COLLAR:
        SURFACE_203_X = r.x;
        SURFACE_203_Y = r.y;
        SURFACE_203_W = r.w;
        SURFACE_203_H = r.h;
        break;

    case SURFACE_ID_BELL_PLACEHOLDER:
        SURFACE_204_X = r.x;
        SURFACE_204_Y = r.y;
        SURFACE_204_W = r.w;
        SURFACE_204_H = r.h;
        break;

    case SURFACE_ID_BROWSER:
        SURFACE_205_X = r.x;
        SURFACE_205_Y = r.y;
        SURFACE_205_W = r.w;
        SURFACE_205_H = r.h;
        break;

    case SURFACE_ID_SPINDLE:
        SURFACE_0x99_X = r.x;
        SURFACE_0x99_Y = r.y;
        SURFACE_0x99_W = r.w;
        SURFACE_0x99_H = r.h;
        break;

    default:
        break;
    }
}

void shell_chrome_tile_active_scene_frames(ShellChromeContext *ctx) {
    sexos_line("[shell.tile.begin]");

    uint64_t tiles[MAX_FRAMES] = {0};

    size_t count = 0;

    for (size_t i = 0; i < MAX_FRAMES; ++i) {
        ShellFrame frame = FRAMES[i];

        if (!frame.present || frame.scene_id != ACTIVE_SCENE_IDX ||
            (frame.flags & (FRAME_FLAG_MINIMIZED | FRAME_FLAG_ZOOMED)))
            continue;

        if (frame.active_tab >= MAX_TABS_PER_FRAME) {
            sexos_line("[tiling.frame.skip] frame=%u reason=invalid_tab", frame.frame_id);

            continue;
        }
        ShellTab tab = frame.tabs[frame.active_tab];

        if (tab.present && chrome_tile_candidate(ctx, tab.surface_id) && count < MAX_FRAMES)
            tiles[count++] = tab.surface_id;
    }
    if (!count) {
        clear_focus_if_dead();

        clear_drag_if_dead();

        clear_hover_if_dead();

        clear_hover_if_wrong_scene();

        HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

        sexos_line("[shell.tile.reject] reason=no_tileable_frames");

        sexos_line("[tiling.done] frames=0");

        return;
    }
    uint32_t width = (uint32_t)desktop_width(),
             height = (uint32_t)chrome_sub(desktop_height(), P.bar_height);

    for (size_t i = 0; i < count; ++i) {
        uint64_t sid = tiles[i];

        Rect r = chrome_tile_rect(i, count, width, height);

        if (chrome_budget(&ctx->chrome->tile_active_scene_frames__SHELL_TILE_APPLY_BUDGET))
            sexos_line("[shell.tile.apply] sid=%llu x=%d y=%d w=%u h=%u", U64(sid), r.x, r.y, r.w,
                       r.h);

        update_local_geometry(sid, r.x, r.y, r.w, r.h);

        chrome_tile_shadow(ctx, sid, r);

        pdx_call(SLOT_DISPLAY, 0xEC, sid, ((uint64_t)r.y << 32) | (uint64_t)r.x,
                 ((uint64_t)r.h << 32) | r.w);

        if (sid == SURFACE_ID_QUIL)
            pdx_call(SLOT_DISPLAY, 0xEF, sid, 0,
                     ((uint64_t)QUIL_PLACEHOLDER_COLOR << 32) | ((uint64_t)r.h << 16) | r.w);

        if (sid == SURFACE_ID_MESH) {
            pdx_call(SLOT_DISPLAY, 0xEF, sid, 0,
                     ((uint64_t)MESH_PLACEHOLDER_COLOR << 32) | ((uint64_t)r.h << 16) | r.w);

            mesh_render_pd_graph();

            if (!shell_chrome_peer_exchange_bool(
                    &ctx->chrome->tile_active_scene_frames__MESH_VISIBLE_MARK, true))
                sexos_line("[mesh.surface.visible.ok] sid=%llu", U64(sid));
        }
    }
    uint64_t focused = FOCUSED_SURFACE_ID;

    if (focused && !(surface_is_alive(focused) && !is_tombstoned(focused) &&
                     surface_is_lifecycle_focusable(focused) && surface_in_active_scene(focused))) {
        sexos_line("[tiling.focus.clear] sid=%llu reason=invalid_after_tiling", U64(focused));

        try_set_focus(tiles[0]);
    }
    sexos_line("[shell.tile.after_lifecycle] frames=%zu", count);
}

void shell_chrome_tile_visible_frames(ShellChromeContext *ctx) {
    sexos_line("[shell.tile.delegate] from=tile_visible_frames to=tile_active_scene_frames");

    tile_active_scene_frames();
}

uint32_t shell_chrome_frame_light_at(ShellChromeContext *ctx, uint32_t frame_id, int32_t x,
                                     int32_t y) {
    uint64_t sid = 0;

    int32_t sx = 0, sy = 0;

    uint32_t sw = 0, sh = 0;

    if (!active_surface_for_frame(frame_id, &sid) || !get_surface_bounds(sid, &sx, &sy, &sw, &sh))
        return FRAME_LIGHT_NONE;

    if (chrome_budget(&ctx->chrome->frame_light_at__LIGHT_HITBOX_BUDGET)) {
        int32_t gap = FRAME_TOP_BAR_LIGHT_GAP_PX, size = FRAME_TOP_BAR_LIGHT_SIZE_PX,
                bottom = chrome_add(sy, FRAME_TOP_BAR_HEIGHT_PX);

        int32_t close = chrome_add(sx, gap), minimum = chrome_add(close, chrome_add(size, gap)),
                zoom = chrome_add(minimum, chrome_add(size, gap));

        sexos_line(
            "[shell.frame.light.hitbox] frame=%u sx=%d sy=%d close=(%d,%d)-(%d,%d) min=(%d,%d)-(%d,%d) zoom=(%d,%d)-(%d,%d)",
            frame_id, sx, sy, close, sy, chrome_add(close, size), bottom, minimum, sy,
            chrome_add(minimum, size), bottom, zoom, sy, chrome_add(zoom, size), bottom);
    }
    int32_t local = chrome_sub(x, sx);

    if (frame_has_top_bar(frame_id)) {
        if (y < sy || y >= chrome_add(sy, FRAME_TOP_BAR_HEIGHT_PX))
            return FRAME_LIGHT_NONE;

        if (local >= 0 && local < 20)
            return FRAME_LIGHT_CLOSE;

        if (local >= 20 && local < 40)
            return FRAME_LIGHT_MINIMIZE;

        if (local >= 40 && local < 60)
            return FRAME_LIGHT_ZOOM;

    } else {
        if (y < sy || y >= chrome_add(sy, FRAME_RIM_PX))
            return FRAME_LIGHT_NONE;

        int32_t second = FRAME_LIGHT_GAP_PX + FRAME_LIGHT_SIZE_PX + FRAME_LIGHT_GAP_PX,
                third = second + FRAME_LIGHT_SIZE_PX + FRAME_LIGHT_GAP_PX;

        if (local >= FRAME_LIGHT_GAP_PX && local < FRAME_LIGHT_GAP_PX + FRAME_LIGHT_SIZE_PX)
            return FRAME_LIGHT_CLOSE;

        if (local >= second && local < second + FRAME_LIGHT_SIZE_PX)
            return FRAME_LIGHT_MINIMIZE;

        if (local >= third && local < third + FRAME_LIGHT_SIZE_PX)
            return FRAME_LIGHT_ZOOM;
    }
    return FRAME_LIGHT_NONE;
}

bool shell_chrome_toggle_top_bar_for_active_frame(ShellChromeContext *ctx) {
    uint32_t frame = 0;

    if (!selected_frame_id(&frame)) {
        if (chrome_budget(
                &ctx->chrome->toggle_top_bar_for_active_frame__TOP_BAR_TOGGLE_REJECT_BUDGET))
            sexos_line("[shell.frame.topbar.toggle.reject] reason=no_active_frame");

        return false;
    }
    bool next = !frame_has_top_bar(frame);

    set_frame_top_bar(frame, next);

    send_frame_tab_info(frame);

    HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

    if (chrome_budget(&ctx->chrome->toggle_top_bar_for_active_frame__TOP_BAR_TOGGLE_BUDGET))
        sexos_line("[shell.frame.topbar.toggle] frame=%u enabled=%u", frame, (unsigned)next);

    snap_capture_layout();

    return true;
}

bool shell_chrome_switch_to_tab(ShellChromeContext *ctx, uint32_t frame_id, uint32_t tab_index) {
    if (!frame_accepts_input(frame_id))
        return false;

    size_t slot = chrome_frame_slot(ctx, frame_id);

    if (slot == MAX_FRAMES)
        return false;

    ShellFrame frame = FRAMES[slot];

    /* Keep the full argument width: narrowing before validation admits 256+n. */
    if (tab_index >= frame.tab_count || tab_index >= MAX_TABS_PER_FRAME)
        return false;

    if (tab_index == frame.active_tab)
        return true;

    uint64_t old = 0;

    if (!active_surface_for_frame(frame_id, &old))
        return false;

    ShellTab next = frame.tabs[tab_index];

    if (!next.present)
        return false;

    if (next.surface_id == old)
        return true;

    int32_t x = frame.normal_x, y = frame.normal_y;

    uint32_t w = frame.normal_w, h = frame.normal_h;

    (void)get_surface_bounds(old, &x, &y, &w, &h);

    FRAMES[slot].active_tab = (uint8_t)tab_index;

    if (surface_is_alive(old))
        pdx_call(SLOT_DISPLAY, 0xEE, old, 0, 0);

    pdx_call(SLOT_DISPLAY, 0xEC, next.surface_id, ((uint64_t)y << 32) | (uint64_t)x,
             ((uint64_t)h << 32) | w);

    update_local_geometry(next.surface_id, x, y, w, h);

    try_set_focus(next.surface_id);

    clear_drag_if_dead();

    HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

    send_frame_tab_info(frame_id);

    if (chrome_budget(&ctx->chrome->switch_to_tab__TAB_SWITCH_BUDGET))
        sexos_line("[shell.frame.tab.switch] frame=%u old=%llu new=%llu tab=%u", frame_id, U64(old),
                   U64(next.surface_id), tab_index);

    snap_capture_layout();

    return true;
}

static const char *chrome_app_family_label(uint64_t sid) {
    if (sid == SURFACE_ID_LINEN)
        return "Linen";

    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        ShellAppCatalogRow row = shell_custody_catalog_at(i);

        bool found = row.surface == sid || (row.shell_surface && row.shell_surface == sid);

        for (size_t j = 0; j < row.child_count && j < 3; ++j)
            if (row.children[j] == sid)
                found = true;

        if (found) {
            if (row.app == SHELL_APP_QUIL)
                return "Quil";

            if (row.app == SHELL_APP_SPINDLE)
                return "Spindle";

            if (row.app == SHELL_APP_BROWSER)
                return "Browser";
        }
    }
    if (sid == SURFACE_ID_QUIL)
        return "Quil";

    if (sid == SURFACE_ID_MESH)
        return "Mesh";

    if (sid == SURFACE_ID_COLLAR)
        return "Collar";

    if (sid == SURFACE_ID_BELL_PLACEHOLDER)
        return "Bell";

    if (sid == SURFACE_ID_SPINDLE)
        return "Spindle";

    return "App";
}

static bool chrome_focus_guards(ShellChromeContext *ctx, uint64_t sid) {
    if (!is_focusable_surface(sid)) {
        sexos_line("[shell.interact.reject] op=focus sid=%llu reason=nonfocusable", U64(sid));

        sexos_line("[shell.focus.reject.nonfocusable] id=%llu", U64(sid));

        sexos_line("[silk.focus.reject] reason=invalid_surface ok=1");

        return false;
    }
    if (!surface_is_alive(sid)) {
        sexos_line("[shell.interact.reject] op=focus sid=%llu reason=dead", U64(sid));

        sexos_line("[shell.focus.reject.dead] id=%llu", U64(sid));

        sexos_line("[silk.focus.reject] reason=invalid_surface ok=1");

        return false;
    }
    if (is_tombstoned(sid)) {
        sexos_line("[shell.interact.reject] op=focus sid=%llu reason=tombstoned", U64(sid));

        sexos_line("[shell.focus.reject.tombstoned] id=%llu", U64(sid));

        sexos_line("[lifecycle.tombstone.reject_focus] sid=%llu reason=tombstoned", U64(sid));

        sexos_line("[silk.focus.reject] reason=invalid_surface ok=1");

        return false;
    }
    if (!surface_is_lifecycle_focusable(sid)) {
        sexos_line("[shell.interact.reject] op=focus sid=%llu reason=lifecycle", U64(sid));

        sexos_line("[focus.lifecycle.reject] id=%llu", U64(sid));

        return false;
    }
    FocusRef ref = {0};

    if (!make_focus_ref(sid, &ref)) {
        sexos_line("[shell.interact.reject] op=focus sid=%llu reason=invalid_generation", U64(sid));

        return false;
    }
    if (!focus_ref_is_current(&ref)) {
        sexos_line("[shell.interact.reject] op=focus sid=%llu reason=stale_generation", U64(sid));

        sexos_line("[focus.generation.reject] id=%llu", U64(sid));

        uint64_t generation = 0;

        if (surface_generation(sid, &generation))
            sexos_line("[lifecycle.generation.stale_reject] sid=%llu gen=Some(%llu)", U64(sid),
                       U64(generation));

        else
            sexos_line("[lifecycle.generation.stale_reject] sid=%llu gen=None", U64(sid));

        return false;
    }
    uint8_t scene = 0;

    if (surface_scene_id(sid, &scene) && scene != ACTIVE_SCENE_IDX) {
        sexos_line("[shell.interact.reject] op=focus sid=%llu reason=inactive_scene", U64(sid));

        sexos_line("[scene.focus.reject.inactive] id=%llu sid_scene=%u active=%u", U64(sid),
                   (unsigned)scene, (unsigned)ACTIVE_SCENE_IDX);

        return false;
    }
    return true;
}

static void chrome_focus_visible(ShellChromeContext *ctx, uint64_t old, uint64_t sid) {
    if (!VISIBLE_FOCUS_TOPBAR_PROOF_ENABLED)
        return;

    uint32_t frame = 0, old_frame = 0;

    (void)frame_for_surface(sid, &frame);

    (void)frame_for_surface(old, &old_frame);

    bool active = false;

    if (frame)
        for (size_t i = 0; i < MAX_FRAMES; ++i)
            if (FRAMES[i].present && FRAMES[i].frame_id == frame &&
                FRAMES[i].scene_id == ACTIVE_SCENE_IDX) {
                active = true;

                break;
            }
    sexos_line(
        "[shell.focus.visible] old=%llu new=%llu frame=%u sid=%llu active=%u reason=focus_set",
        U64(old), U64(sid), frame, U64(sid), (unsigned)active);

    if (frame)
        emit_chrome_diagnostics(frame, "focus_set");
}

static void chrome_focus_selected(ShellChromeContext *ctx, uint64_t sid) {
    if (chrome_budget(&ctx->chrome->try_set_focus__SELECTED_OPTIONS_SET_BUDGET)) {
        uint32_t frame = 0;

        (void)selected_frame_id(&frame);

        uint32_t mask = selected_window_options_mask();

        sexos_line("[shell.selected.options] frame=%llu surface=%llu mask=0x%x", U64(frame),
                   U64(sid), mask);
    }
    uint32_t mask = selected_window_options_mask();

    pdx_call(SLOT_SILKBAR, OP_SILKBAR_FOCUS_STATE, 1, mask, 0);

    const char *label = chrome_app_family_label(sid);

    sexos_line(
        "[shell.silkbar.status.send] focus=%llu app=%s tint=%u bell=%zu ok=1 reason=focus_set",
        U64(sid), label, (unsigned)ACTIVE_TINT_IDX, bell_ring_count());

    send_silkbar_phase2_update(8, sid, 0);

    send_silkbar_phase2_update(9, ACTIVE_TINT_IDX, 0);

    if (chrome_budget(&ctx->chrome->try_set_focus__SELECTED_OPTIONS_SEND_BUDGET))
        sexos_line("[shell.selected.options.send] surface=%llu mask=0x%x", U64(sid), mask);
}

static void chrome_focus_describe(ShellChromeContext *ctx, uint64_t sid) {
    const char *label = "unknown", *role = "Surface";

    if (sid == SURFACE_ID_QUIL) {
        label = "Quil";

        role = "AppPlaceholder";

    } else if (sid == SURFACE_ID_MESH) {
        label = "Mesh";

        role = "AppPlaceholder";

    } else if (sid == SURFACE_ID_COLLAR) {
        label = "Collar";

        role = "AppPlaceholder";

    } else if (sid == SURFACE_ID_BELL_PLACEHOLDER) {
        label = "Bell";

        role = "AppPlaceholder";

    } else if (sid == SURFACE_ID_LINEN) {
        label = "Linen";

        role = "AppPlaceholder";

    } else if (sid == SURFACE_ID_APP) {
        label = "App";

        role = "Frame";

    } else if (sid == SURFACE_ID_STATIC) {
        label = "Test2";

        role = "Frame";

    } else if (sid == SURFACE_ID_TEST3) {
        label = "Test3";

        role = "Frame";

    } else if (sid == SURFACE_ID_TEST4) {
        label = "Test4";

        role = "Frame";

    } else if (sid == SURFACE_ID_CURSOR) {
        label = "Cursor";

        role = "Desktop";

    } else {
        uint32_t frame = 0;

        if (frame_for_surface(sid, &frame)) {
            const char *name = shell_chrome_peer_frame_name(ctx, frame);

            if (name)
                label = name;
        }
    }
    if (chrome_budget(&ctx->chrome->try_set_focus__ACCESS_FOCUS_DESCRIBE_BUDGET))
        sexos_line("[access.focus.describe] target=%llu role=%s label=%s", U64(sid), role, label);
}

bool shell_chrome_try_set_focus_inner(ShellChromeContext *ctx, uint64_t sid,
                                      bool refresh_children) {
    launcher_focus_discard();

    if (!sid) {
        FOCUSED_SURFACE_ID = 0;

        sync_focus_ref();

        sexos_line("[silk-shell.focus.change] from=0 to=0 (clear)");

        pdx_call(SLOT_DISPLAY, 0xED, 0, 0, 0);

        sexos_line("[shell.focus.clear] id=0");

        if (chrome_budget(&ctx->chrome->try_set_focus__SELECTED_OPTIONS_CLEAR_BUDGET))
            sexos_line("[shell.selected.options] frame=0 surface=0 mask=0");

        pdx_call(SLOT_SILKBAR, OP_SILKBAR_FOCUS_STATE, 0, 0, 0);

        sexos_line(
            "[shell.silkbar.status.send] focus=0 app=none tint=0 bell=0 ok=1 reason=focus_cleared");

        return true;
    }
    if (!chrome_focus_guards(ctx, sid))
        return false;

    if (refresh_children) {
        int32_t x = 0, y = 0;

        uint32_t w = 0, h = 0;

        if (get_surface_bounds(sid, &x, &y, &w, &h))
            native_child_geometry(sid, x, y, w, h);
    }
    uint64_t old = FOCUSED_SURFACE_ID;

    FOCUSED_SURFACE_ID = sid;

    sync_focus_ref();

    sexos_line("[focus.ref.commit] id=%llu", U64(sid));

    if (old != sid && mesh_is_visible_in_active_scene()) {
        mesh_render_pd_graph();

        sexos_line("[mesh.pd_graph.refresh] reason=focus_change old=%llu new=%llu", U64(old),
                   U64(sid));
    }
    sexos_line("[shell.focus.set] id=%llu", U64(sid));

    sexos_line("[shell.interact.focus] sid=%llu", U64(sid));

    if (SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED) {
        surface_input_lifetime_mark_focus_live(sid, 1, 1);

        surface_input_lifetime_try_done();
    }
    chrome_focus_visible(ctx, old, sid);

    pdx_call(SLOT_DISPLAY, 0xED, sid, 0, 0);

    chrome_focus_selected(ctx, sid);

    chrome_focus_describe(ctx, sid);

    access_describe_focus();

    return true;
}

bool shell_chrome_try_set_focus(ShellChromeContext *ctx, uint64_t sid) {
    return try_set_focus_inner(sid, true);
}

bool shell_chrome_update_frame_hover_at(ShellChromeContext *ctx, int32_t x, int32_t y) {
    if (INTERACTION.tag == INTERACTION_DRAGGING || INTERACTION.tag == INTERACTION_TAB_DRAGGING)
        return false;

    uint32_t previous = HOVERED_FRAME_ID, frame = 0, kind = HOVER_NONE, light = FRAME_LIGHT_NONE;

    if (y >= P.bar_height) {
        HitTarget target = hit_test_at(x, y);

        if (target.tag == HIT_TARGET_SURFACE) {
            if (frame_for_surface(target.surface_id, &frame)) {
                kind = HOVER_FRAME_BODY;

                light = frame_light_at(frame, x, y);
            }
        } else if (target.tag == HIT_TARGET_FRAME_CHROME) {
            frame = target.frame_id;

            kind = target.kind;

            light = frame_light_at(frame, x, y);
        }
    }
    bool changed = HOVERED_FRAME_ID != frame || HOVER_KIND != kind,
         light_changed = HOVERED_FRAME_LIGHT != light;

    if (changed || light_changed) {
        if (chrome_budget(&ctx->chrome->update_frame_hover_at__INTERACT_HOVER_BUDGET))
            sexos_line("[shell.interact.hover] frame=%u kind=%u light=%u x=%d y=%d", frame, kind,
                       light, x, y);

        if (chrome_budget(&ctx->chrome->update_frame_hover_at__HOVER_STATE_CHANGE_BUDGET)) {
            if (frame)
                sexos_line("[shell.frame.hover.set] frame=%u kind=%u", frame, kind);

            else
                sexos_line("[shell.frame.hover.clear]");
        }
        HOVERED_FRAME_ID = frame;

        HOVER_KIND = kind;

        HOVERED_FRAME_LIGHT = light;

        if (light_changed &&
            chrome_budget(&ctx->chrome->update_frame_hover_at__FRAME_LIGHT_HOVER_BUDGET))
            sexos_line("[shell.frame.light.hover] frame=%u light=%u", frame, light);

        if (frame && frame_tab_count(frame) <= 1) {
            bool visible = frame_chrome_visible(frame);

            if (visible) {
                if (chrome_budget(&ctx->chrome->update_frame_hover_at__TAB_CHROME_SHOW_BUDGET))
                    sexos_line("[tab.chrome.show] frame=%u", frame);

            } else if (chrome_budget(&ctx->chrome->update_frame_hover_at__TAB_CHROME_HIDE_BUDGET))
                sexos_line("[tab.chrome.hide] frame=%u", frame);

            send_frame_tab_info(frame);
        }
        if (frame && frame_tab_count(frame) > 1 && kind != HOVER_NONE &&
            chrome_budget(&ctx->chrome->update_frame_hover_at__TAB_CHROME_PERSIST_BUDGET))
            sexos_line("[tab.chrome.persist.multi] frame=%u", frame);

        if (previous && previous != frame) {
            HOVERED_FRAME_ID = 0;

            HOVERED_FRAME_LIGHT = FRAME_LIGHT_NONE;

            send_frame_tab_info(previous);

            HOVERED_FRAME_ID = frame;

            HOVERED_FRAME_LIGHT = light;
        }
        if (frame)
            send_frame_tab_info(frame);
    }
    return changed || light_changed;
}

static bool chrome_access_utf8(ShellAccessText text) {
    if (text.len > 32)
        return false;
    size_t i = 0;

    while (i < text.len) {
        uint8_t b = text.bytes[i];

        if (b < 0x80) {
            ++i;
            continue;
        }
        size_t need = 0;
        uint8_t low = 0x80, high = 0xBF;

        if (b >= 0xC2 && b <= 0xDF)
            need = 1;

        else if (b >= 0xE0 && b <= 0xEF) {
            need = 2;
            if (b == 0xE0)
                low = 0xA0;
            if (b == 0xED)
                high = 0x9F;
        } else if (b >= 0xF0 && b <= 0xF4) {
            need = 3;
            if (b == 0xF0)
                low = 0x90;
            if (b == 0xF4)
                high = 0x8F;
        } else
            return false;

        if (need > text.len - i - 1)
            return false;

        for (size_t k = 1; k <= need; ++k) {
            uint8_t c = text.bytes[i + k];
            if (c < (k == 1 ? low : 0x80) || c > (k == 1 ? high : 0xBF))
                return false;
        }
        i += need + 1;
    }
    return true;
}

void shell_chrome_access_copy_label(ShellChromeContext *ctx, ShellAccessLabel *dst,
                                    ShellAccessText src) {
    (void)ctx;
    size_t len = src.len < 31 ? src.len : 31;

    for (size_t i = 0; i < len; ++i)
        dst->bytes[i] = src.bytes[i];

    dst->bytes[len] = 0;
}

uint32_t shell_chrome_access_label_token(ShellChromeContext *ctx, const ShellAccessLabel *label) {
    (void)ctx;
    uint32_t hash = 5381;

    for (size_t i = 0; i < 32; ++i) {
        uint8_t b = label->bytes[i];
        if (!b)
            break;
        hash = hash * 33 + b;
    }
    return hash;
}

static void chrome_access_node_label(AccessNode *node, const ShellAccessLabel *label) {
    for (size_t i = 0; i < 32; ++i)
        node->label[i] = label->bytes[i];
}

bool shell_chrome_access_node_is_valid_target(ShellChromeContext *ctx,
                                              const AccessTargetRef *target) {
    if (!target->surface_id && !target->frame_id && target->scene_id == 0xFF)
        return false;

    if (target->surface_id &&
        (!surface_is_alive(target->surface_id) || is_tombstoned(target->surface_id)))
        return false;

    if (target->frame_id && chrome_frame_slot(ctx, target->frame_id) == MAX_FRAMES)
        return false;

    return true;
}

size_t shell_chrome_access_emit_scene_node(ShellChromeContext *ctx, ShellAccessNodes *nodes,
                                           size_t idx, uint8_t scene_id) {
    if (idx >= MAX_ACCESS_NODES || !validate_scene_id(scene_id))
        return idx;

    Scene scene = SCENES[scene_id];
    bool active = scene_id == ACTIVE_SCENE_IDX;

    AccessStateFlags state = 0;
    if (active)
        state |= ACCESS_FOCUSED | ACCESS_SELECTED | ACCESS_VISIBLE;
    if (scene.flags & SCENE_FLAG_EMPTY)
        state |= ACCESS_HIDDEN;

    AccessActionFlags actions = active ? 0 : ACT_SWITCH_SCENE;

    ShellAccessLabel label = {0};
    ShellAccessText text = {.len = ATLAS_LABEL_LEN};
    for (size_t i = 0; i < ATLAS_LABEL_LEN; ++i)
        text.bytes[i] = scene.label[i];

    if (chrome_access_utf8(text)) {
        size_t length = ATLAS_LABEL_LEN;
        while (length && scene.label[length - 1] == 0)
            --length;

        text.len = length;
        access_copy_label(&label, text);

    } else
        access_copy_label(&label, ((ShellAccessText){"scene", 5}));

    AccessNode node = {.present = true,
                       .node_id = 0x1000 | scene_id,
                       .role = ACCESS_ROLE_SCENE_CHIP,
                       .state = state,
                       .actions = actions,
                       .target = {0, 0, scene_id}};

    chrome_access_node_label(&node, &label);
    nodes->values[idx] = node;

    if (chrome_budget(&ctx->chrome->access_emit_scene_node__ACCESS_SCENE_BUDGET))
        sexos_line("[access.node.scene] id=%u role=SceneChip state=0x%x actions=0x%x",
                   (unsigned)scene_id, (unsigned)state, (unsigned)actions);

    return idx + 1;
}

size_t shell_chrome_access_emit_frame_node(ShellChromeContext *ctx, ShellAccessNodes *nodes,
                                           size_t idx, const ShellFrame *frame) {
    if (idx >= MAX_ACCESS_NODES)
        return idx;

    uint32_t fid = frame->frame_id;
    uint64_t sid = 0;
    (void)active_surface_for_frame(fid, &sid);

    bool minimized = (frame->flags & FRAME_FLAG_MINIMIZED) != 0,
         zoomed = (frame->flags & FRAME_FLAG_ZOOMED) != 0,
         active = frame->scene_id == ACTIVE_SCENE_IDX;

    if (sid && (!surface_is_alive(sid) || is_tombstoned(sid))) {
        if (chrome_budget(&ctx->chrome->access_emit_frame_node__ACCESS_SKIP_DEAD_BUDGET))
            sexos_line("[access.node.skip_dead] frame=%u sid=%llu", fid, U64(sid));

        return idx;
    }
    AccessStateFlags state = 0;

    if (sid && FOCUSED_SURFACE_ID == sid)
        state |= ACCESS_FOCUSED;

    if (active && !minimized)
        state |= ACCESS_VISIBLE;

    if (!active)
        state |= ACCESS_HIDDEN;

    if (minimized)
        state |= ACCESS_MINIMIZED;

    if (zoomed)
        state |= ACCESS_ZOOMED;

    AccessActionFlags actions = 0;

    if (sid && active && !minimized)
        actions |= ACT_FOCUS | ACT_ACTIVATE;

    if (sid && !minimized)
        actions |= ACT_MINIMIZE | ACT_CLOSE;

    if (sid && minimized)
        actions |= ACT_RESTORE;

    if (sid && !zoomed)
        actions |= ACT_ZOOM;

    if (zoomed)
        actions |= ACT_UNZOOM;

    ShellAccessLabel label = shell_chrome_peer_frame_label(ctx, fid);

    AccessNode node = {.present = true,
                       .node_id = 0x2000 | fid,
                       .role = ACCESS_ROLE_FRAME,
                       .state = state,
                       .actions = actions,
                       .target = {sid, fid, frame->scene_id}};

    chrome_access_node_label(&node, &label);
    nodes->values[idx] = node;

    if (chrome_budget(&ctx->chrome->access_emit_frame_node__ACCESS_FRAME_BUDGET))
        sexos_line("[access.node.frame] id=%u state=0x%x actions=0x%x", fid, (unsigned)state,
                   (unsigned)actions);

    return idx + 1;
}

static size_t chrome_access_placeholder(ShellChromeContext *ctx, ShellAccessNodes *nodes,
                                        size_t idx, uint64_t sid, uint32_t id,
                                        ShellAccessText name) {
    if (!surface_is_alive(sid) || is_tombstoned(sid) || idx >= MAX_ACCESS_NODES)
        return idx;

    ShellAccessLabel label = {0};
    access_copy_label(&label, name);

    AccessNode node = {.present = true,
                       .node_id = id,
                       .role = ACCESS_ROLE_APP_PLACEHOLDER,
                       .state = ACCESS_VISIBLE,
                       .actions = ACT_FOCUS | ACT_ACTIVATE,
                       .target = {sid, 0, 0}};

    chrome_access_node_label(&node, &label);
    nodes->values[idx] = node;
    return idx + 1;
}

size_t shell_chrome_access_emit_shell_nodes(ShellChromeContext *ctx, ShellAccessNodes *nodes) {
    size_t idx = 0;

    for (uint8_t scene = 0; scene < ATLAS_MAX_SCENES; ++scene)
        idx = access_emit_scene_node(nodes, idx, scene);

    for (size_t i = 0; i < MAX_FRAMES; ++i)
        if (FRAMES[i].present) {
            ShellFrame frame = FRAMES[i];
            idx = access_emit_frame_node(nodes, idx, &frame);
        }
    idx =
        chrome_access_placeholder(ctx, nodes, idx, SURFACE_ID_QUIL,
                                  0x3000 | (uint32_t)SURFACE_ID_QUIL, (ShellAccessText){"Quil", 4});

    idx = chrome_access_placeholder(ctx, nodes, idx, SURFACE_ID_LINEN, 0x3001,
                                    (ShellAccessText){"Linen", 5});

    if (chrome_budget(&ctx->chrome->access_emit_shell_nodes__ACCESS_EMIT_BUDGET))
        sexos_line("[access.node.emit] count=%zu", idx);

    return idx;
}

void shell_chrome_access_describe_node(ShellChromeContext *ctx, const AccessNode *node) {
    ShellAccessLabel label = {0};
    for (size_t i = 0; i < 32; ++i)
        label.bytes[i] = node->label[i];

    uint32_t token = access_label_token(&label);

    if (chrome_budget(&ctx->chrome->access_describe_node__ACCESS_DESCRIBE_NODE_BUDGET)) {
        sexos_line(
            "[access.focus.describe] node_id=%u role=%u state=0x%x actions=0x%x target_sid=%llu target_fid=%u target_scene=%u label_token=0x%x",
            node->node_id, (unsigned)node->role, (unsigned)node->state, (unsigned)node->actions,
            U64(node->target.surface_id), node->target.frame_id, (unsigned)node->target.scene_id,
            token);

        sexos_line("[access.focus.label_token] node_id=%u token=0x%x", node->node_id, token);
    }
}

void shell_chrome_access_describe_focus(ShellChromeContext *ctx) {
    ShellAccessNodes nodes = {0};
    size_t count = access_emit_shell_nodes(&nodes);

    if (!count) {
        if (chrome_budget(&ctx->chrome->access_describe_focus__ACCESS_DESCRIBE_EMPTY_BUDGET))
            sexos_line("[access.focus.describe.reject] reason=empty_tree");
        return;
    }
    uint64_t sid = FOCUSED_SURFACE_ID;

    if (!sid) {
        if (chrome_budget(&ctx->chrome->access_describe_focus__ACCESS_DESCRIBE_NOFOCUS_BUDGET))
            sexos_line("[access.focus.describe.reject] reason=no_focus");
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        AccessNode node = nodes.values[i];
        if (!node.present || node.target.surface_id != sid)
            continue;

        if (!surface_is_alive(sid) || is_tombstoned(sid)) {
            if (chrome_budget(&ctx->chrome->access_describe_focus__ACCESS_DESCRIBE_SKIP_BUDGET))
                sexos_line("[access.focus.describe.skip_dead] target=%llu", U64(sid));
            return;
        }
        access_describe_node(&node);
        return;
    }
    if (chrome_budget(&ctx->chrome->access_describe_focus__ACCESS_DESCRIBE_NOTFOUND_BUDGET))
        sexos_line("[access.focus.describe.reject] reason=not_in_tree target=%llu", U64(sid));
}

bool shell_chrome_access_move_focus(ShellChromeContext *ctx, const ShellAccessNodes *nodes,
                                    size_t length, bool forward) {
    uint64_t candidates[MAX_ACCESS_NODES] = {0};
    size_t count = 0;

    size_t limit = length < MAX_ACCESS_NODES ? length : MAX_ACCESS_NODES;

    for (size_t i = 0; i < limit; ++i) {
        AccessNode node = nodes->values[i];
        uint64_t sid = node.target.surface_id;

        if (!node.present || !sid || !(node.state & ACCESS_VISIBLE) ||
            !(node.actions & ACT_FOCUS) || !surface_is_alive(sid) || is_tombstoned(sid) ||
            !surface_is_lifecycle_focusable(sid))
            continue;

        bool duplicate = false;
        for (size_t j = 0; j < count; ++j)
            if (candidates[j] == sid) {
                duplicate = true;
                break;
            }
        if (duplicate)
            continue;

        uint8_t scene = 0;
        if (surface_scene_id(sid, &scene) && scene != ACTIVE_SCENE_IDX)
            continue;

        candidates[count++] = sid;
    }
    if (!count)
        return false;

    size_t current = count;
    for (size_t i = 0; i < count; ++i)
        if (candidates[i] == FOCUSED_SURFACE_ID) {
            current = i;
            break;
        }
    for (size_t offset = 1; offset <= count; ++offset) {
        size_t index = current < count ? (forward ? (current + offset) % count
                                                  : (current + count - offset) % count)
                                       : (forward ? offset - 1 : count - offset);

        uint64_t sid = candidates[index];

        if (try_set_focus(sid)) {
            sexos_line(
                "[shell.window.action] action=%s frame=0 sid=%llu ok=1 reason=distinct_visible_target",
                forward ? "FocusNext" : "FocusPrev", U64(sid));
            return true;
        }
    }
    return false;
}

static bool chrome_access_activate(ShellChromeContext *ctx) {
    uint64_t sid = FOCUSED_SURFACE_ID;

    if (!sid) {
        if (chrome_budget(
                &ctx->chrome->access_handle_keyboard_action__ACCESS_ACTIVATE_EMPTY_BUDGET))
            sexos_line("[access.action.reject] action=activate reason=no_focus");
        return false;
    }
    if (!surface_is_alive(sid) || is_tombstoned(sid)) {
        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_ACTIVATE_DEAD_BUDGET))
            sexos_line("[access.action.reject] action=activate reason=dead target=%llu", U64(sid));
        return false;
    }
    uint32_t frame = 0;
    (void)frame_for_surface(sid, &frame);

    if (frame_for_surface(sid, &frame)) {
        size_t slot = chrome_frame_slot(ctx, frame);
        bool minimized = slot < MAX_FRAMES && (FRAMES[slot].flags & FRAME_FLAG_MINIMIZED) != 0;

        if (minimized) {
            bool ok = restore_minimized_frame(frame);

            sexos_line(
                "[shell.window.action] action=RestoreMinimized frame=%u sid=%llu ok=%u reason=%s",
                frame, U64(sid), (unsigned)ok, ok ? "ok" : "restore_failed");

            if (chrome_budget(
                    &ctx->chrome->access_handle_keyboard_action__ACCESS_ACTIVATE_RESTORE_BUDGET))
                sexos_line("[access.action.allow] action=activate target=%llu dispatch=restore",
                           U64(sid));

        } else {
            bool ok = minimize_frame(frame);

            sexos_line("[shell.window.action] action=Minimize frame=%u sid=%llu ok=%u reason=%s",
                       frame, U64(sid), (unsigned)ok, ok ? "ok" : "minimize_failed");

            if (chrome_budget(
                    &ctx->chrome->access_handle_keyboard_action__ACCESS_ACTIVATE_MINIMIZE_BUDGET))
                sexos_line("[access.action.allow] action=activate target=%llu dispatch=minimize",
                           U64(sid));
        }
    }
    if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_ACTIVATE_OK_BUDGET))
        sexos_line("[access.keyboard.alt] action=activate target=%llu", U64(sid));

    /* This return means the key was handled;
     the operation's outcome is logged. */
    return true;
}

static bool chrome_access_close(ShellChromeContext *ctx) {
    uint64_t sid = FOCUSED_SURFACE_ID;

    if (!sid) {
        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_CLOSE_NOFOCUS_BUDGET))
            sexos_line("[access.action.reject] action=close reason=no_focus");
        return false;
    }
    if (!surface_is_alive(sid) || is_tombstoned(sid)) {
        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_CLOSE_DEAD_BUDGET))
            sexos_line("[access.action.reject] action=close reason=dead target=%llu", U64(sid));
        return false;
    }
    if (close_surface_from_frame_light(sid)) {
        uint32_t frame = 0;
        (void)frame_for_surface(sid, &frame);

        sexos_line("[shell.window.action] action=Close frame=%u sid=%llu ok=1 reason=ok", frame,
                   U64(sid));

        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_CLOSE_OK_BUDGET))
            sexos_line("[access.action.close] target=%llu", U64(sid));
        return true;
    }
    if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_CLOSE_FAIL_BUDGET))
        sexos_line("[access.action.reject] action=close reason=failed target=%llu", U64(sid));

    uint32_t frame = 0;
    (void)frame_for_surface(sid, &frame);

    sexos_line("[shell.window.action] action=Close frame=%u sid=%llu ok=0 reason=close_failed",
               frame, U64(sid));
    return false;
}

static bool chrome_access_zoom(ShellChromeContext *ctx) {
    uint64_t sid = FOCUSED_SURFACE_ID;

    if (!sid) {
        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_ZOOM_NOFOCUS_BUDGET))
            sexos_line("[access.action.reject] action=zoom reason=no_focus");
        return false;
    }
    if (!surface_is_alive(sid) || is_tombstoned(sid)) {
        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_ZOOM_DEAD_BUDGET))
            sexos_line("[access.action.reject] action=zoom reason=dead target=%llu", U64(sid));
        return false;
    }
    uint32_t frame = 0;

    if (frame_for_surface(sid, &frame) && toggle_zoom_frame(frame)) {
        sexos_line("[shell.window.action] action=ZoomToggle frame=%u sid=%llu ok=1 reason=ok",
                   frame, U64(sid));

        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_ZOOM_OK_BUDGET))
            sexos_line("[access.action.zoom] frame=%u target=%llu", frame, U64(sid));
        return true;
    }
    if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_ZOOM_FAIL_BUDGET))
        sexos_line("[access.action.reject] action=zoom reason=failed target=%llu", U64(sid));

    frame = 0;
    (void)frame_for_surface(sid, &frame);

    sexos_line("[shell.window.action] action=ZoomToggle frame=%u sid=%llu ok=0 reason=zoom_failed",
               frame, U64(sid));
    return false;
}

static bool chrome_access_restore(ShellChromeContext *ctx) {
    uint32_t frame = 0;

    if (first_minimized_frame_id(&frame) && restore_minimized_frame(frame)) {
        sexos_line("[shell.window.action] action=RestoreMinimized frame=%u ok=1 reason=ok", frame);

        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_RESTORE_OK_BUDGET))
            sexos_line("[access.action.restore] frame=%u", frame);
        return true;
    }
    if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_RESTORE_NOOP_BUDGET))
        sexos_line("[access.action.reject] action=restore reason=no_minimized_frame");
    return false;
}

bool shell_chrome_access_handle_keyboard_action(ShellChromeContext *ctx, uint8_t action) {
    ShellAccessNodes nodes = {0};
    size_t count = access_emit_shell_nodes(&nodes);
    if (!count)
        return false;

    switch (action) {
    case ACTION_ACCESS_FOCUS_NEXT:
    case ACTION_ACCESS_FOCUS_PREV:
        return access_move_focus(&nodes, count, action == ACTION_ACCESS_FOCUS_NEXT);

    case ACTION_ACCESS_ACTIVATE:
        return chrome_access_activate(ctx);

    case ACTION_ACCESS_CLOSE:
        return chrome_access_close(ctx);

    case ACTION_ACCESS_ZOOM_TOGGLE:
        return chrome_access_zoom(ctx);

    case ACTION_ACCESS_SCENE_NEXT:
        next_scene();
        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_SCENE_NEXT_BUDGET))
            sexos_line("[access.action.scene_switch] dir=next");
        return true;

    case ACTION_ACCESS_SCENE_PREV:
        prev_scene();
        if (chrome_budget(&ctx->chrome->access_handle_keyboard_action__ACCESS_SCENE_PREV_BUDGET))
            sexos_line("[access.action.scene_switch] dir=prev");
        return true;

    case ACTION_RESTORE_MINIMIZED:
        return chrome_access_restore(ctx);

    default:
        return false;
    }
    return false;
}

typedef struct ChromeDebugText {
    char bytes[384];
    size_t len;
} ChromeDebugText;

typedef struct ChromeDebugChunk {
    char bytes[64];
} ChromeDebugChunk;

static void chrome_debug_byte(ChromeDebugText *text, char byte) {
    if (text->len >= 383) {
        sexos_fail();
        return;
    }
    text->bytes[text->len++] = byte;
    text->bytes[text->len] = 0;
}

static void chrome_debug_chunk(ChromeDebugText *text, ChromeDebugChunk chunk) {
    for (size_t i = 0; i < 64 && chunk.bytes[i]; ++i)
        chrome_debug_byte(text, chunk.bytes[i]);
}

static void chrome_debug_unsigned(ChromeDebugText *text, uint64_t value) {
    char digits[20] = {0};
    size_t count = 0;
    while (count < 20) {
        digits[count++] = (char)('0' + value % 10);
        value /= 10;
        if (!value)
            break;
    }
    while (count)
        chrome_debug_byte(text, digits[--count]);
}

static void chrome_debug_signed(ChromeDebugText *text, int32_t value) {
    if (value < 0)
        chrome_debug_byte(text, '-');
    uint64_t magnitude = value < 0 ? UINT64_C(0) - (uint64_t)(int64_t)value : (uint64_t)value;
    chrome_debug_unsigned(text, magnitude);
}

#define DEBUG_TEXT(text, literal) chrome_debug_chunk(text, ((ChromeDebugChunk){literal}))

static ChromeDebugChunk chrome_edge_chunk(uint8_t edge) {
    switch (edge) {
    case RESIZE_EDGE_LEFT:
        return (ChromeDebugChunk){"Left"};
    case RESIZE_EDGE_RIGHT:
        return (ChromeDebugChunk){"Right"};
    case RESIZE_EDGE_TOP:
        return (ChromeDebugChunk){"Top"};
    case RESIZE_EDGE_BOTTOM:
        return (ChromeDebugChunk){"Bottom"};
    case RESIZE_EDGE_TOP_LEFT:
        return (ChromeDebugChunk){"TopLeft"};
    case RESIZE_EDGE_TOP_RIGHT:
        return (ChromeDebugChunk){"TopRight"};
    case RESIZE_EDGE_BOTTOM_LEFT:
        return (ChromeDebugChunk){"BottomLeft"};
    default:
        return (ChromeDebugChunk){"BottomRight"};
    }
    return (ChromeDebugChunk){"BottomRight"};
}

static ChromeDebugText chrome_interaction_text(InteractionState state) {
    ChromeDebugText text = {0};
    switch (state.tag) {
    case INTERACTION_IDLE:
        DEBUG_TEXT(&text, "Idle");
        break;
    case INTERACTION_CLICK_PENDING:
        DEBUG_TEXT(&text, "ClickPending");
        break;
    case INTERACTION_DRAGGING:
        DEBUG_TEXT(&text, "Dragging { surface_id: ");
        chrome_debug_unsigned(&text, state.surface_id);
        DEBUG_TEXT(&text, ", current_x: ");
        chrome_debug_signed(&text, state.current_x);
        DEBUG_TEXT(&text, ", current_y: ");
        chrome_debug_signed(&text, state.current_y);
        DEBUG_TEXT(&text, " }");
        break;
    case INTERACTION_PANEL_ACTIVE:
        DEBUG_TEXT(&text, "PanelActive { panel: ");
        switch (state.panel) {
        case PANEL_LAUNCHER:
            DEBUG_TEXT(&text, "Launcher");
            break;
        case PANEL_STATUS:
            DEBUG_TEXT(&text, "Status");
            break;
        case PANEL_CLOCK:
            DEBUG_TEXT(&text, "Clock");
            break;
        case PANEL_BELL:
            DEBUG_TEXT(&text, "Bell");
            break;
        default:
            DEBUG_TEXT(&text, "Settings");
            break;
        }
        DEBUG_TEXT(&text, " }");
        break;
    case INTERACTION_RESIZING:
        DEBUG_TEXT(&text, "Resizing { surface_id: ");
        chrome_debug_unsigned(&text, state.surface_id);
        DEBUG_TEXT(&text, ", edge: ");
        chrome_debug_chunk(&text, chrome_edge_chunk(state.edge));
        DEBUG_TEXT(&text, ", start_x: ");
        chrome_debug_signed(&text, state.start_x);
        DEBUG_TEXT(&text, ", start_y: ");
        chrome_debug_signed(&text, state.start_y);
        DEBUG_TEXT(&text, ", start_w: ");
        chrome_debug_unsigned(&text, state.start_w);
        DEBUG_TEXT(&text, ", start_h: ");
        chrome_debug_unsigned(&text, state.start_h);
        DEBUG_TEXT(&text, ", pending_dx: ");
        chrome_debug_signed(&text, state.pending_dx);
        DEBUG_TEXT(&text, ", pending_dy: ");
        chrome_debug_signed(&text, state.pending_dy);
        DEBUG_TEXT(&text, " }");
        break;
    default:
        DEBUG_TEXT(&text, "TabDragging { frame_id: ");
        chrome_debug_unsigned(&text, state.frame_id);
        DEBUG_TEXT(&text, ", start_tab: ");
        chrome_debug_unsigned(&text, state.start_tab);
        DEBUG_TEXT(&text, ", start_x: ");
        chrome_debug_signed(&text, state.start_x);
        DEBUG_TEXT(&text, ", start_y: ");
        chrome_debug_signed(&text, state.start_y);
        DEBUG_TEXT(&text, ", current_tab: ");
        chrome_debug_unsigned(&text, state.current_tab);
        DEBUG_TEXT(&text, " }");
        break;
    }
    return text;
}

void shell_chrome_try_transition(ShellChromeContext *ctx, InteractionState next) {
    InteractionState current = INTERACTION;
    bool allowed = current.tag == INTERACTION_IDLE ||
                   (current.tag == INTERACTION_CLICK_PENDING &&
                    (next.tag == INTERACTION_DRAGGING || next.tag == INTERACTION_IDLE ||
                     next.tag == INTERACTION_PANEL_ACTIVE || next.tag == INTERACTION_RESIZING ||
                     next.tag == INTERACTION_TAB_DRAGGING)) ||
                   ((current.tag == INTERACTION_DRAGGING || current.tag == INTERACTION_RESIZING ||
                     current.tag == INTERACTION_TAB_DRAGGING) &&
                    next.tag == INTERACTION_IDLE) ||
                   (current.tag == INTERACTION_PANEL_ACTIVE &&
                    (next.tag == INTERACTION_IDLE || next.tag == INTERACTION_CLICK_PENDING));
    if (allowed) {
        uint32_t budget = shell_chrome_peer_load_relaxed(&ctx->input->INTERACTION_LOG_BUDGET);
        if (budget) {
            ChromeDebugText from = chrome_interaction_text(current),
                            to = chrome_interaction_text(next);
            sexos_line("[shell.interaction.transition] from=%s to=%s", from.bytes, to.bytes);
            shell_chrome_peer_store_relaxed(&ctx->input->INTERACTION_LOG_BUDGET, budget - 1);
        }
        INTERACTION = next;
    } else {
        ChromeDebugText from = chrome_interaction_text(current), to = chrome_interaction_text(next);
        sexos_line("[shell.interaction.forbidden] from=%s to=%s", from.bytes, to.bytes);
    }
}

bool shell_chrome_move_surface_by(ShellChromeContext *ctx, uint64_t sid, int32_t dx, int32_t dy,
                                  ShellChromeMove *out) {
    int32_t x = 0, y = 0;
    uint32_t w = 0, h = 0;
    if (sid == SURFACE_ID_APP && SURFACE_100_ALIVE) {
        if (WINDOWS_len < 2)
            return false;
        x = WINDOWS[1].desc.x;
        y = WINDOWS[1].desc.y;
        w = WINDOWS[1].desc.width;
        h = WINDOWS[1].desc.height;
    } else if (sid == SURFACE_ID_STATIC && SURFACE_101_ALIVE) {
        x = SURFACE_101_X;
        y = SURFACE_101_Y;
        w = SURFACE_101_W;
        h = SURFACE_101_H;
    } else if (sid == SURFACE_ID_TEST3 && SURFACE_102_ALIVE) {
        x = SURFACE_102_X;
        y = SURFACE_102_Y;
        w = SURFACE_102_W;
        h = SURFACE_102_H;
    } else if (sid == SURFACE_ID_TEST4 && SURFACE_103_ALIVE) {
        x = SURFACE_103_X;
        y = SURFACE_103_Y;
        w = SURFACE_103_W;
        h = SURFACE_103_H;
    } else if (sid == SURFACE_ID_LINEN) {
        x = SURFACE_200_X;
        y = SURFACE_200_Y;
        w = SURFACE_200_W;
        h = SURFACE_200_H;
    } else if (sid == SURFACE_ID_QUIL) {
        x = SURFACE_201_X;
        y = SURFACE_201_Y;
        w = SURFACE_201_W;
        h = SURFACE_201_H;
    } else if (sid == SURFACE_ID_MESH) {
        x = SURFACE_202_X;
        y = SURFACE_202_Y;
        w = SURFACE_202_W;
        h = SURFACE_202_H;
    } else if (sid == SURFACE_ID_COLLAR) {
        x = SURFACE_203_X;
        y = SURFACE_203_Y;
        w = SURFACE_203_W;
        h = SURFACE_203_H;
    } else if (sid == SURFACE_ID_BELL_PLACEHOLDER) {
        x = SURFACE_204_X;
        y = SURFACE_204_Y;
        w = SURFACE_204_W;
        h = SURFACE_204_H;
    } else
        return false;
    int32_t rx = chrome_add(x, dx), ry = chrome_add(y, dy), cx = 0, cy = 0;
    clamp_position(rx, ry, w, h, &cx, &cy);
    if (sid == SURFACE_ID_APP) {
        WINDOWS[1].desc.x = cx;
        WINDOWS[1].desc.y = cy;
    } else if (sid == SURFACE_ID_STATIC) {
        SURFACE_101_X = cx;
        SURFACE_101_Y = cy;
    } else if (sid == SURFACE_ID_TEST3) {
        SURFACE_102_X = cx;
        SURFACE_102_Y = cy;
    } else if (sid == SURFACE_ID_TEST4) {
        SURFACE_103_X = cx;
        SURFACE_103_Y = cy;
    } else if (sid == SURFACE_ID_LINEN) {
        SURFACE_200_X = cx;
        SURFACE_200_Y = cy;
    } else if (sid == SURFACE_ID_QUIL) {
        SURFACE_201_X = cx;
        SURFACE_201_Y = cy;
    } else if (sid == SURFACE_ID_MESH) {
        SURFACE_202_X = cx;
        SURFACE_202_Y = cy;
    } else if (sid == SURFACE_ID_COLLAR) {
        SURFACE_203_X = cx;
        SURFACE_203_Y = cy;
    } else {
        SURFACE_204_X = cx;
        SURFACE_204_Y = cy;
    }
    *out = (ShellChromeMove){cx, cy, cx != rx || cy != ry};
    return true;
}

bool shell_chrome_synthetic_window_drag_target(ShellChromeContext *ctx,
                                               ShellChromeDragTarget *out) {
    uint64_t candidates[5] = {SURFACE_ID_QUIL, SURFACE_ID_LINEN, SURFACE_ID_MESH, SURFACE_ID_COLLAR,
                              FOCUSED_SURFACE_ID};
    uint64_t sid = 0;
    uint32_t frame = 0;
    for (size_t i = 0; i < 5; ++i) {
        uint32_t fid = 0;
        if (frame_for_surface(candidates[i], &fid) && frame_accepts_input(fid)) {
            sid = candidates[i];
            frame = fid;
            break;
        }
    }
    if (!sid) {
        if (chrome_budget(&ctx->chrome->synthetic_window_drag_target__TARGET_REJECT_BUDGET))
            sexos_line("[shell.drag.synthetic.no_target] reason=no_accepting_frame focused=%llu",
                       U64(FOCUSED_SURFACE_ID));
        return false;
    }
    int32_t x = 0, y = 0;
    uint32_t w = 0, h = 0;
    if (!get_surface_bounds(sid, &x, &y, &w, &h)) {
        if (chrome_budget(&ctx->chrome->synthetic_window_drag_target__TARGET_REJECT_BUDGET))
            sexos_line("[shell.drag.synthetic.no_target] reason=no_bounds sid=%llu", U64(sid));
        return false;
    }
    if (w < 4 || h < 4) {
        if (chrome_budget(&ctx->chrome->synthetic_window_drag_target__TARGET_REJECT_BUDGET))
            sexos_line("[shell.drag.synthetic.no_target] reason=too_small sid=%llu w=%u h=%u",
                       U64(sid), w, h);
        return false;
    }
    int32_t right = chrome_sub(chrome_add(x, (int32_t)w), 1),
            bottom = chrome_sub(chrome_add(y, (int32_t)h), 1);
    int32_t px = chrome_clamp(chrome_add(x, 1), x, right),
            py = chrome_clamp(chrome_add(chrome_add(y, FRAME_TOP_BAR_HEIGHT_PX), 8), y, bottom);
    (void)chrome_clamp(chrome_add(px, 120), x, right);
    (void)chrome_clamp(chrome_add(py, 20), y, bottom);
    *out = (ShellChromeDragTarget){frame, sid, x, y, w, h, chrome_max(px, 0), chrome_max(py, 0)};
    return true;
}

void shell_chrome_silk_drag_begin_mark(ShellChromeContext *ctx, uint64_t sid, int32_t x,
                                       int32_t y) {
    DRAG_MOVED_THIS_DRAG = false;
    if (chrome_budget(&ctx->chrome->silk_drag_begin_mark__SILK_DRAG_BEGIN_BUDGET))
        sexos_line("[silk.drag.begin] surface=%llu x=%d y=%d", U64(sid), x, y);
}

void shell_chrome_silk_drag_end_mark(ShellChromeContext *ctx, uint64_t sid) {
    if (chrome_budget(&ctx->chrome->silk_drag_end_mark__SILK_DRAG_END_BUDGET))
        sexos_line("[silk.drag.end] surface=%llu x=%d y=%d", U64(sid), POINTER_X, POINTER_Y);
    if (DRAG_MOVED_THIS_DRAG && !WINDOW_MOVE_PROOF_DONE_EMITTED) {
        WINDOW_MOVE_PROOF_DONE_EMITTED = true;
        sexos_line("[silk.window.move.proof.done] ok=1");
    }
    DRAG_MOVED_THIS_DRAG = false;
}

bool shell_chrome_drag_move_focused(ShellChromeContext *ctx, int32_t dx, int32_t dy) {
    if (INTERACTION.tag != INTERACTION_DRAGGING)
        return false;
    uint64_t sid = INTERACTION.surface_id;
    ShellChromeMove result = {0};
    bool moved = move_surface_by(sid, dx, dy, &result);
    if (!moved)
        return false;
    DRAG_MOVED_THIS_DRAG = true;
    if (chrome_budget(&ctx->chrome->drag_move_focused__SILK_DRAG_MOVE_BUDGET))
        sexos_line("[silk.drag.move] surface=%llu x=%d y=%d clamped=%u", U64(sid), result.x,
                   result.y, (unsigned)result.clamped);
    INPUT_TRACE_DRAG_MOVES += 1;
    if (chrome_budget(&ctx->chrome->drag_move_focused__INTERACT_DRAG_MOVE_BUDGET)) {
        sexos_line("[shell.interact.drag.move] sid=%llu dx=%d dy=%d", U64(sid), dx, dy);
        sexos_line("[shell.drag.update] sid=%llu frame=0 x=%d y=%d dx=%d dy=%d", U64(sid),
                   POINTER_X, POINTER_Y, dx, dy);
    }
    if (chrome_budget(&ctx->chrome->drag_move_focused__DRAG_MOVE_OK_BUDGET))
        sexos_line("[silk.drag.move.ok] sid=%llu dx=%d dy=%d", U64(sid), dx, dy);
    if (chrome_budget(&ctx->chrome->drag_move_focused__INTEGRATED_DRAG_TARGET_BUDGET))
        sexos_line("[shell.integrated.drag_target] id=%llu focus=%llu", U64(sid),
                   U64(FOCUSED_SURFACE_ID));
    if (SHELL_INTERACTION_CONTRACT_PROOF_ENABLED) {
        uint8_t live = surface_is_alive(sid);
        sexos_line(
            "[shell.interact.stage.drag_capture] phase=move target=%llu live=%u capture=1 release=0 ok=1",
            U64(sid), (unsigned)live);
        shell_interaction_contract_mark_stage(SHELL_INTERACTION_STAGE_DRAG);
        shell_interaction_contract_try_done();
    }
    if (CLICK_DRAG_PROOF_ENABLED) {
        uint8_t live = surface_is_alive(sid);
        DRAG_CAPTURE_MOVE_SEEN = true;
        sexos_line("[drag.capture.move] target=%llu live=%u dx=%d dy=%d ok=1", U64(sid),
                   (unsigned)live, dx, dy);
    }
    uint8_t live = surface_is_alive(sid);
    surface_input_lifetime_mark_drag_target(sid, live, "move", live);
    surface_input_lifetime_try_done();
    return true;
}

bool shell_chrome_resize_accumulate_delta(ShellChromeContext *ctx, int32_t dx, int32_t dy) {
    if (INTERACTION.tag != INTERACTION_RESIZING)
        return false;
    InteractionState state = INTERACTION;
    state.pending_dx = chrome_add(state.pending_dx, dx);
    state.pending_dy = chrome_add(state.pending_dy, dy);
    INTERACTION = state;
    if (chrome_budget(&ctx->chrome->resize_accumulate_delta__RESIZE_DELTA_BUDGET)) {
        ChromeDebugChunk edge = chrome_edge_chunk(state.edge);
        sexos_line("[silk.resize.delta] sid=%llu edge=%s dx=%d dy=%d total_dx=%d total_dy=%d",
                   U64(state.surface_id), edge.bytes, dx, dy, state.pending_dx, state.pending_dy);
    }
    return true;
}

void shell_chrome_clear_resize_if_dead(ShellChromeContext *ctx) {
    if (INTERACTION.tag == INTERACTION_RESIZING) {
        uint64_t sid = INTERACTION.surface_id;
        if (!surface_is_alive(sid)) {
            sexos_line("[shell.resize.clear_dead] sid=%llu reason=dead", U64(sid));
            try_transition(((InteractionState){.tag = INTERACTION_IDLE}));
        }
    }
}

bool shell_chrome_try_snap_on_drag_release(ShellChromeContext *ctx, uint64_t sid, int32_t px,
                                           int32_t py) {
    if (!surface_is_alive(sid) || is_tombstoned(sid)) {
        sexos_line("[silk.snap.reject.dead] sid=%llu reason=dead", U64(sid));
        return false;
    }
    uint32_t frame = 0;
    if (!frame_for_surface(sid, &frame)) {
        sexos_line("[silk.snap.none] sid=%llu reason=no_frame x=%d y=%d", U64(sid), px, py);
        return false;
    }
    if (frame_is_minimized(frame)) {
        sexos_line("[silk.snap.reject.dead] sid=%llu reason=minimized", U64(sid));
        return false;
    }
    bool top = py < chrome_add(P.bar_height, SNAP_TOP_PX), left = px < SNAP_EDGE_PX,
         right = px >= chrome_sub(desktop_width(), SNAP_EDGE_PX);
    if (top) {
        sexos_line("[silk.snap.hit.top] sid=%llu frame=%u x=%d y=%d", U64(sid), frame, px, py);
        if (!frame_is_zoomed(frame) && zoom_frame(frame)) {
            sexos_line("[silk.snap.apply] sid=%llu frame=%u kind=top", U64(sid), frame);
            return true;
        }
        sexos_line("[silk.snap.none] sid=%llu reason=top_zoom_skip x=%d y=%d", U64(sid), px, py);
        return false;
    }
    if (left || right) {
        sexos_line(left ? "[silk.snap.hit.left] sid=%llu frame=%u x=%d y=%d"
                        : "[silk.snap.hit.right] sid=%llu frame=%u x=%d y=%d",
                   U64(sid), frame, px, py);
        uint32_t width = (uint32_t)desktop_width(),
                 height = (uint32_t)chrome_sub(desktop_height(), P.bar_height), half = width / 2;
        int32_t x = left ? 0 : (int32_t)half, y = P.bar_height;
        uint32_t w = left ? half : width - half;
        pdx_call(SLOT_DISPLAY, 0xEC, sid, ((uint64_t)(int64_t)y << 32) | (uint64_t)(int64_t)x,
                 ((uint64_t)height << 32) | w);
        update_local_geometry(sid, x, y, w, height);
        sexos_line("[silk.snap.apply] sid=%llu frame=%u kind=%s x=%d y=%d w=%u h=%u", U64(sid),
                   frame, left ? "left" : "right", x, y, w, height);
        return true;
    }
    sexos_line("[silk.snap.none] sid=%llu x=%d y=%d", U64(sid), px, py);
    return false;
}

bool shell_chrome_apply_resize_geometry(ShellChromeContext *ctx, int32_t dx, int32_t dy) {
    if (INTERACTION.tag != INTERACTION_RESIZING)
        return false;
    uint64_t sid = INTERACTION.surface_id;
    uint8_t edge = INTERACTION.edge;
    if (!surface_is_alive(sid)) {
        sexos_line("[silk.resize.reject.dead] sid=%llu reason=dead", U64(sid));
        try_transition(((InteractionState){.tag = INTERACTION_IDLE}));
        return false;
    }
    uint32_t frame = 0;
    if (frame_for_surface(sid, &frame) && frame_is_zoomed(frame))
        return false;
    int32_t x = 0, y = 0;
    uint32_t w = 0, h = 0;
    if (!get_surface_bounds(sid, &x, &y, &w, &h)) {
        sexos_line("[silk.resize.reject.dead] sid=%llu reason=no_bounds", U64(sid));
        return false;
    }
    int32_t nx = 0, ny = 0;
    uint32_t nw = 0, nh = 0;
    compute_resize_rect(x, y, w, h, edge, dx, dy, &nx, &ny, &nw, &nh);
    if (nx == x && ny == y && nw == w && nh == h)
        return false;
    uint32_t ew = w, eh = h;
    if (edge == RESIZE_EDGE_RIGHT || edge == RESIZE_EDGE_BOTTOM_RIGHT ||
        edge == RESIZE_EDGE_TOP_RIGHT)
        ew = (uint32_t)chrome_max(chrome_add((int32_t)w, dx), 0);
    if (edge == RESIZE_EDGE_LEFT || edge == RESIZE_EDGE_BOTTOM_LEFT || edge == RESIZE_EDGE_TOP_LEFT)
        ew = (uint32_t)chrome_max(chrome_sub((int32_t)w, dx), 0);
    if (edge == RESIZE_EDGE_BOTTOM || edge == RESIZE_EDGE_BOTTOM_LEFT ||
        edge == RESIZE_EDGE_BOTTOM_RIGHT)
        eh = (uint32_t)chrome_max(chrome_add((int32_t)h, dy), 0);
    if (edge == RESIZE_EDGE_TOP || edge == RESIZE_EDGE_TOP_LEFT || edge == RESIZE_EDGE_TOP_RIGHT)
        eh = (uint32_t)chrome_max(chrome_sub((int32_t)h, dy), 0);
    ChromeDebugChunk name = chrome_edge_chunk(edge);
    if ((nw != ew || nh != eh) &&
        chrome_budget(&ctx->chrome->apply_resize_geometry__RESIZE_CLAMP_BUDGET))
        sexos_line("[silk.resize.clamp] sid=%llu edge=%s want_w=%u got_w=%u want_h=%u got_h=%u",
                   U64(sid), name.bytes, ew, nw, eh, nh);
    sexos_line("[silk.resize.apply] sid=%llu edge=%s x=%d y=%d w=%u h=%u", U64(sid), name.bytes, nx,
               ny, nw, nh);
    pdx_call(SLOT_DISPLAY, 0xEC, sid, ((uint64_t)(int64_t)ny << 32) | (uint64_t)(int64_t)nx,
             ((uint64_t)nh << 32) | nw);
    update_local_geometry(sid, nx, ny, nw, nh);
    if (chrome_budget(&ctx->chrome->apply_resize_geometry__RESIZE_FLUSH_BUDGET))
        sexos_line("[silk.resize.flush] sid=%llu x=%d y=%d w=%u h=%u", U64(sid), nx, ny, nw, nh);
    return true;
}

static bool chrome_point_rim(int32_t x, int32_t y, int32_t sx, int32_t sy, uint32_t sw, uint32_t sh,
                             int32_t band) {
    int32_t right = chrome_sub(chrome_add(sx, (int32_t)sw), 1),
            bottom = chrome_sub(chrome_add(sy, (int32_t)sh), 1);
    return (x >= sx && x < chrome_add(sx, FRAME_RIM_PX)) ||
           (x > chrome_sub(right, FRAME_RIM_PX) && x <= right) ||
           (y >= sy && y < chrome_add(sy, band)) ||
           (y > chrome_sub(bottom, FRAME_RIM_PX) && y <= bottom);
}

bool shell_chrome_hit_test_surface_chrome(ShellChromeContext *ctx, int32_t x, int32_t y,
                                          uint64_t sid, HitTarget *out) {
    int32_t sx = 0, sy = 0;
    uint32_t sw = 0, sh = 0;
    if (!get_surface_bounds(sid, &sx, &sy, &sw, &sh))
        return false;
    if (chrome_budget(&ctx->chrome->hit_test_surface_chrome__CHROME_BOUNDS_BUDGET))
        sexos_line("[shell.frame.chrome.bounds] sid=%llu x=%d y=%d w=%u h=%u topbar_h=%d", U64(sid),
                   sx, sy, sw, sh, FRAME_TOP_BAR_HEIGHT_PX);
    uint32_t frame = 0;
    if (!frame_for_surface(sid, &frame) || !frame_accepts_input(frame))
        return false;
    bool top = frame_has_top_bar(frame);
    int32_t band = top ? FRAME_TOP_BAR_HEIGHT_PX : FRAME_RIM_PX;
    int32_t exclusion = top ? FRAME_TOP_BAR_LIGHT_EXCLUSION_PX : FRAME_TAB_LIGHT_EXCLUSION_PX;
    if (chrome_budget(&ctx->chrome->hit_test_surface_chrome__DRAG_BOUNDS_BUDGET))
        sexos_line(
            "[shell.drag.bounds] sid=%llu frame=%u sx=%d sy=%d sw=%u sh=%u topbar_y0=%d topbar_y1=%d rim_y0=%d rim_y1=%d draggable_x0=%d draggable_x1=%d",
            U64(sid), frame, sx, sy, sw, sh, sy, chrome_sub(chrome_add(sy, band), 1), sy,
            chrome_sub(chrome_add(sy, (int32_t)sh), 1), sx,
            chrome_sub(chrome_add(sx, (int32_t)sw), 1));
    uint32_t tab = 0;
    if ((FRAME_TAB_STRIP_PX > 0 || top) && y >= sy &&
        y < chrome_add(sy, top ? FRAME_TOP_BAR_HEIGHT_PX : FRAME_TAB_STRIP_PX) &&
        x >= chrome_add(sx, exclusion) &&
        x < chrome_sub(chrome_add(sx, (int32_t)sw), FRAME_RIM_PX) &&
        frame_tab_at(frame, x, y, &tab)) {
        *out = (HitTarget){
            .tag = HIT_TARGET_FRAME_CHROME, .frame_id = frame, .kind = FRAME_CHROME_TAB_STRIP};
        return true;
    }
    int32_t right = chrome_sub(chrome_add(sx, (int32_t)sw), 1),
            bottom = chrome_sub(chrome_add(sy, (int32_t)sh), 1);
    bool left =
        x >= sx &&
        x<chrome_add(sx, FRAME_RESIZE_ZONE_PX), r = x> chrome_sub(right, FRAME_RESIZE_ZONE_PX) &&
        x <= right;
    bool t =
        y >= sy &&
        y<chrome_add(sy, FRAME_RESIZE_ZONE_PX), b = y> chrome_sub(bottom, FRAME_RESIZE_ZONE_PX) &&
        y <= bottom;
    if (b || ((left || r) && y >= chrome_add(sy, band)) || ((left || r) && t)) {
        *out = (HitTarget){
            .tag = HIT_TARGET_FRAME_CHROME, .frame_id = frame, .kind = FRAME_CHROME_RESIZE};
        return true;
    }
    if (chrome_point_rim(x, y, sx, sy, sw, sh, band)) {
        *out = (HitTarget){
            .tag = HIT_TARGET_FRAME_CHROME, .frame_id = frame, .kind = FRAME_CHROME_RIM};
        return true;
    }
    return false;
}

HitTarget shell_chrome_hit_test_at(ShellChromeContext *ctx, int32_t x, int32_t y) {
    uint64_t focus = FOCUSED_SURFACE_ID;
    HitTarget out = {0};
    uint32_t frame = 0;
    if (point_in_surface(x, y, focus)) {
        if (hit_test_surface_chrome(x, y, focus, &out)) {
            if (out.tag == HIT_TARGET_FRAME_CHROME &&
                chrome_budget(&ctx->chrome->hit_test_at__CHROME_HIT_PRODUCED_BUDGET))
                sexos_line("[shell.hit_target.chrome] frame=%u kind=%u x=%d y=%d", out.frame_id,
                           out.kind, x, y);
            return out;
        }
        if (frame_for_surface(focus, &frame)) {
            if (frame_accepts_input(frame))
                return (HitTarget){.tag = HIT_TARGET_SURFACE, .surface_id = focus};
        } else
            return (HitTarget){.tag = HIT_TARGET_SURFACE, .surface_id = focus};
    }
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i)
        if (ctx->apps->present[i]) {
            ShellAppCustody entry = ctx->apps->entries[i];
            ShellCustodyWord sf = shell_custody_shell_surface(&entry);
            uint64_t sid = sf.present ? sf.value : entry.surface;
            if (entry.dead || sid == focus || !point_in_surface(x, y, sid))
                continue;
            if (frame_for_surface(sid, &frame) && frame_accepts_input(frame)) {
                if (hit_test_surface_chrome(x, y, sid, &out))
                    return out;
                return (HitTarget){.tag = HIT_TARGET_SURFACE, .surface_id = sid};
            }
        }
    uint64_t z[9] = {
        SURFACE_ID_QUIL,  SURFACE_ID_MESH,  SURFACE_ID_COLLAR, SURFACE_ID_BELL_PLACEHOLDER,
        SURFACE_ID_LINEN, SURFACE_ID_TEST4, SURFACE_ID_TEST3,  SURFACE_ID_STATIC,
        SURFACE_ID_APP};
    for (size_t i = 0; i < 9; ++i) {
        uint64_t sid = z[i];
        if (sid == focus)
            continue;
        if (!surface_is_alive(sid)) {
            sexos_line("[shell.hit_test.skip] id=%llu reason=dead", U64(sid));
            continue;
        }
        if (!point_in_surface(x, y, sid))
            continue;
        if (hit_test_surface_chrome(x, y, sid, &out)) {
            if (out.tag == HIT_TARGET_FRAME_CHROME &&
                chrome_budget(&ctx->chrome->hit_test_at__CHROME_HIT_Z_BUDGET))
                sexos_line("[shell.hit_target.chrome] frame=%u kind=%u x=%d y=%d z=1", out.frame_id,
                           out.kind, x, y);
            return out;
        }
        if (frame_for_surface(sid, &frame)) {
            if (frame_accepts_input(frame))
                return (HitTarget){.tag = HIT_TARGET_SURFACE, .surface_id = sid};
        } else
            return (HitTarget){.tag = HIT_TARGET_SURFACE, .surface_id = sid};
    }
    return (HitTarget){0};
}

bool shell_chrome_hit_target_surface(ShellChromeContext *ctx, HitTarget target, uint64_t *out) {
    (void)ctx;
    if (target.tag != HIT_TARGET_SURFACE)
        return false;
    *out = target.surface_id;
    return true;
}

const char *shell_chrome_hit_target_label(ShellChromeContext *ctx, HitTarget target, bool bar,
                                          uint64_t *out) {
    (void)ctx;
    *out = 0;
    if (bar)
        return "chrome";
    if (target.tag == HIT_TARGET_SURFACE) {
        *out = target.surface_id;
        return "app";
    }
    if (target.tag == HIT_TARGET_FRAME_CHROME) {
        *out = target.frame_id;
        return "chrome_frame";
    }
    return "none";
}

bool shell_chrome_toggle_os_panel(ShellChromeContext *ctx, bool *active, uint8_t panel,
                                  uint64_t sid, const char *label, uint32_t x, uint32_t y,
                                  uint32_t w, uint32_t h) {
    if (!*active) {
        int32_t cx = 0, cy = 0;
        clamp_position((int32_t)x, (int32_t)y, w, h, &cx, &cy);
        sexos_line("[shell.%s.open.start] id=0x%llx", label, U64(sid));
        sexos_line(
            "[shell.panel.geometry] surface=%llu x=%d y=%d w=%u h=%u desktop_w=%d desktop_h=%d submitted=1",
            U64(sid), cx, cy, w, h, desktop_width(), desktop_height());
        pdx_call(SLOT_DISPLAY, 0xEC, sid, ((uint64_t)(int64_t)cy << 32) | (uint64_t)(int64_t)cx,
                 ((uint64_t)h << 32) | w);
        sexos_line("[shell.%s.open.ok] id=0x%llx", label, U64(sid));
        *active = true;
        (void)set_lifecycle_state(sid, LIFECYCLE_MAPPED);
        try_transition(((InteractionState){.tag = INTERACTION_PANEL_ACTIVE, .panel = panel}));
    } else {
        sexos_line("[shell.%s.close.start] id=0x%llx", label, U64(sid));
        pdx_call(SLOT_DISPLAY, 0xEE, sid, 0, 0);
        sexos_line("[shell.%s.close.ok] id=0x%llx", label, U64(sid));
        *active = false;
        (void)set_lifecycle_state(sid, LIFECYCLE_ALLOCATED);
        try_transition(((InteractionState){.tag = INTERACTION_IDLE}));
    }
    return true;
}

void shell_chrome_toggle_scene_settings_panel(ShellChromeContext *ctx) {
    if (!SCENE_SETTINGS_ACTIVE) {
        sexos_line("[shell.scene.settings.panel.open.start] id=0x%llx",
                   U64(SURFACE_ID_SCENE_SETTINGS));
        pdx_call(SLOT_DISPLAY, 0xEC, SURFACE_ID_SCENE_SETTINGS,
                 ((uint64_t)SCENE_SETTINGS_PANEL_Y << 32) | SCENE_SETTINGS_PANEL_X,
                 ((uint64_t)SCENE_SETTINGS_PANEL_H << 32) | SCENE_SETTINGS_PANEL_W);
        sexos_line("[shell.scene.settings.panel.open.ok] id=0x%llx",
                   U64(SURFACE_ID_SCENE_SETTINGS));
        SCENE_SETTINGS_ACTIVE = true;
        (void)set_lifecycle_state(SURFACE_ID_SCENE_SETTINGS, LIFECYCLE_MAPPED);
        try_transition(
            ((InteractionState){.tag = INTERACTION_PANEL_ACTIVE, .panel = PANEL_SETTINGS}));
    } else {
        sexos_line("[shell.scene.settings.panel.close.start] id=0x%llx",
                   U64(SURFACE_ID_SCENE_SETTINGS));
        pdx_call(SLOT_DISPLAY, 0xEE, SURFACE_ID_SCENE_SETTINGS, 0, 0);
        sexos_line("[shell.scene.settings.panel.close.ok] id=0x%llx",
                   U64(SURFACE_ID_SCENE_SETTINGS));
        SCENE_SETTINGS_ACTIVE = false;
        (void)set_lifecycle_state(SURFACE_ID_SCENE_SETTINGS, LIFECYCLE_ALLOCATED);
        try_transition(((InteractionState){.tag = INTERACTION_IDLE}));
    }
    if (chrome_budget(&ctx->chrome->toggle_scene_settings_panel__PANEL_BUDGET))
        sexos_line("[shell.scene.settings.panel] visible=%u", (unsigned)SCENE_SETTINGS_ACTIVE);
}

bool shell_chrome_handle_silkbar_click(ShellChromeContext *ctx, int32_t px, int32_t py) {
    if (py < 0 || py >= 50 || px < 0)
        return false;
    size_t x = (size_t)px, y = (size_t)py;
    if (y < 10 || y >= 48 || x < 80 || x >= 1200)
        return false;
    SilkBarDispatch action = silkbar_geometry_dispatch((size_t)desktop_width(), x, y);
    switch (action.kind) {
    case SILKBAR_DISPATCH_NONE:
        return false;
    case SILKBAR_DISPATCH_WORKSPACE: {
        uint8_t index = action.workspace ? action.workspace - 1 : 0;
        if (index > 4)
            index = 4;
        sexos_line("[shell.silkbar.click] target=workspace index=%u x=%zu y=%zu",
                   (unsigned)action.workspace, x, y);
        pdx_call(SLOT_SILKBAR, OP_SILKBAR_WORKSPACE_ACTIVE, index, 0, 0);
        uint8_t previous = ACTIVE_SCENE_IDX;
        if (previous != index) {
            ACTIVE_SCENE_IDX = index;
            sync_scene_visibility();
            clear_focus_if_wrong_scene();
            clear_drag_if_dead();
            clear_drag_if_wrong_scene();
            clear_hover_if_wrong_scene();
            tile_active_scene_frames();
            snap_capture_layout();
            atlas_capture_snapshot();
            if (chrome_budget(&ctx->chrome->handle_silkbar_click__SCENE_SWITCH_BUDGET))
                sexos_line("[shell.scene.switch] from=%u to=%u", (unsigned)previous,
                           (unsigned)ACTIVE_SCENE_IDX);
        }
        return true;
    }
    case SILKBAR_DISPATCH_LAUNCHER: {
        sexos_line("[shell.silkbar.click] target=launcher x=%zu y=%zu", x, y);
        bool active = LAUNCHER_ACTIVE;
        (void)toggle_os_panel(&active, PANEL_LAUNCHER, SURFACE_ID_LAUNCHER, "launcher", 80, 55, 240,
                              360);
        LAUNCHER_ACTIVE = active;
        return true;
    }
    case SILKBAR_DISPATCH_CLOCK: {
        sexos_line("[shell.silkbar.click] target=clock x=%zu y=%zu", x, y);
        bool active = CLOCK_ACTIVE;
        (void)toggle_os_panel(&active, PANEL_CLOCK, SURFACE_ID_CLOCK, "clock", 1000, 55, 240, 300);
        CLOCK_ACTIVE = active;
        return true;
    }
    case SILKBAR_DISPATCH_STATUS: {
        sexos_line("[shell.silkbar.click] target=status x=%zu y=%zu", x, y);
        bool active = STATUS_ACTIVE;
        (void)toggle_os_panel(&active, PANEL_STATUS, SURFACE_ID_STATUS, "status", 860, 55, 200,
                              300);
        STATUS_ACTIVE = active;
        return true;
    }
    default:
        sexos_line("[shell.silkbar.click] target=bell x=%zu y=%zu", x, y);
        if (!toggle_bell()) {
            sexos_line("[shell.silkbar.click.reject] target=bell reason=collar_deny");
            return false;
        }
        return true;
    }
    return false;
}

static bool chrome_click_atlas(ShellChromeContext *ctx, int32_t x, int32_t y) {
    if (!ATLAS_MODE_ENABLED)
        return false;
    uint8_t scene = 0;
    if (atlas_scene_at_point(x, y, &scene)) {
        pdx_call(SLOT_DISPLAY, 0xEE, SURFACE_ID_ATLAS_OVERLAY, 0, 0);
        (void)set_lifecycle_state(SURFACE_ID_ATLAS_OVERLAY, LIFECYCLE_ALLOCATED);
        if (scene != ACTIVE_SCENE_IDX)
            switch_scene(scene);
        else {
            sync_scene_visibility();
            clear_focus_if_dead();
            clear_drag_if_dead();
            clear_hover_if_dead();
            clear_hover_if_wrong_scene();
            tile_active_scene_frames();
            snap_capture_layout();
        }
        ATLAS_MODE_ENABLED = false;
        if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__ATLAS_SELECT_BUDGET))
            sexos_line("[shell.atlas.select] id=%u", (unsigned)scene);
        sexos_line("[shell.interact.atlas.select] scene=%u", (unsigned)scene);
    } else if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__ATLAS_MISS_BUDGET))
        sexos_line("[shell.atlas.miss]");
    return true;
}

static int32_t chrome_saturating_difference(int32_t a, int32_t b) {
    int64_t value = (int64_t)a - b;
    return value < INT32_MIN ? INT32_MIN : value > INT32_MAX ? INT32_MAX : (int32_t)value;
}

static bool chrome_click_toolbar(ShellChromeContext *ctx, HitTarget target, int32_t px, int32_t py,
                                 uint8_t buttons) {
    if (!(buttons & 1) || target.tag != HIT_TARGET_SURFACE)
        return false;
    uint64_t frame = target.surface_id;
    ShellAppCatalogRow row = {0};
    for (size_t i = 0; i < SHELL_APP_COUNT; ++i) {
        ShellAppCatalogRow item = shell_custody_catalog_at(i);
        if (item.present && item.app != SHELL_APP_SPINDLE &&
            (item.shell_surface ? item.shell_surface : item.surface) == frame) {
            row = item;
            break;
        }
    }
    if (!row.present || row.app != SHELL_APP_QUIL)
        return false;
    int32_t x = 0, y = 0;
    uint32_t w = 0, h = 0;
    if (!get_surface_bounds(frame, &x, &y, &w, &h))
        return false;
    int32_t cx = row.shell_surface ? chrome_add(x, 8) : x,
            cy = row.shell_surface ? chrome_add(y, 36) : y;
    uint32_t width = row.shell_surface ? chrome_sat_sub(w, 16) : w,
             height = row.shell_surface ? chrome_sat_sub(h, 44) : h;
    ShellDocumentLayout layout = shell_document_layout(row.target, width, height);
    uint8_t action = shell_document_hit(&layout, (uint32_t)chrome_saturating_difference(px, cx),
                                        (uint32_t)chrome_saturating_difference(py, cy));
    if (!action || ctx->apps->document_pointer_sequence == UINT64_MAX)
        return false;
    uint64_t sequence = ctx->apps->document_pointer_sequence + 1;
    ctx->apps->document_pointer_sequence = sequence;
    if (!ctx->apps->document_pointer.pending)
        ctx->apps->document_pointer =
            (ShellDocumentPointer){true, row.route_slot, row.target, action, sequence};
    (void)try_set_focus(frame);
    return true;
}

static void chrome_click_trace(ShellChromeContext *ctx, HitTarget target, int32_t x, int32_t y) {
    if (CLICK_DRAG_PROOF_ENABLED) {
        uint64_t id = target.tag == HIT_TARGET_SURFACE        ? target.surface_id
                      : target.tag == HIT_TARGET_FRAME_CHROME ? target.frame_id
                                                              : 0;
        sexos_line("[click.focus.hit_test] target=%llu hit=%u x=%d y=%d ok=1", U64(id),
                   (unsigned)(target.tag != HIT_TARGET_NONE), x, y);
    }
    if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__DRAG_HIT_TEST_BUDGET)) {
        const char *label = "none";
        uint8_t draggable = 0;
        if (target.tag == HIT_TARGET_SURFACE) {
            label = "app";
            draggable = 1;
        } else if (target.tag == HIT_TARGET_FRAME_CHROME) {
            if (target.kind == FRAME_CHROME_RIM) {
                label = "rim";
                draggable = 1;
            } else if (target.kind == FRAME_CHROME_RESIZE)
                label = "resize";
            else {
                uint32_t light = frame_light_at(target.frame_id, x, y);
                label = light == FRAME_LIGHT_CLOSE              ? "light_close"
                        : light == FRAME_LIGHT_MINIMIZE         ? "light_min"
                        : light == FRAME_LIGHT_ZOOM             ? "light_zoom"
                        : target.kind == FRAME_CHROME_TAB_STRIP ? "tab"
                                                                : "chrome";
            }
        }
        sexos_line("[shell.drag.hit_test] x=%d y=%d result=%s draggable=%u", x, y, label,
                   (unsigned)draggable);
    }
}

static void chrome_click_pending(ShellChromeContext *ctx, HitTarget target, int32_t x, int32_t y,
                                 uint8_t buttons) {
    if (!(buttons & 1)) {
        DRAG_PENDING_ACTIVE = false;
        return;
    }
    DRAG_PENDING_ACTIVE = true;
    DRAG_PENDING_START_X = x;
    DRAG_PENDING_START_Y = y;
    if (target.tag == HIT_TARGET_SURFACE) {
        DRAG_PENDING_TARGET = target.surface_id;
        DRAG_PENDING_KIND = 1;
        sexos_line("[shell.drag.pending] target=%llu kind=app start_x=%d start_y=%d buttons=0x%x",
                   U64(target.surface_id), x, y, (unsigned)buttons);
    } else if (target.tag == HIT_TARGET_FRAME_CHROME) {
        DRAG_PENDING_TARGET = target.frame_id;
        DRAG_PENDING_KIND = target.kind == FRAME_CHROME_RIM         ? 2
                            : target.kind == FRAME_CHROME_RESIZE    ? 3
                            : target.kind == FRAME_CHROME_TAB_STRIP ? 5
                                                                    : 4;
        const char *label = target.kind == FRAME_CHROME_RIM         ? "rim"
                            : target.kind == FRAME_CHROME_RESIZE    ? "resize"
                            : target.kind == FRAME_CHROME_TAB_STRIP ? "tab_strip"
                                                                    : "chrome";
        sexos_line("[shell.drag.pending] target=%u kind=%s start_x=%d start_y=%d buttons=0x%x",
                   target.frame_id, label, x, y, (unsigned)buttons);
    } else {
        DRAG_PENDING_TARGET = 0;
        DRAG_PENDING_KIND = 0;
        sexos_line("[shell.drag.pending] target=0 kind=none start_x=%d start_y=%d buttons=0x%x", x,
                   y, (unsigned)buttons);
    }
    uint64_t id = DRAG_PENDING_TARGET;
    if (SHELL_INTERACTION_CONTRACT_PROOF_ENABLED) {
        uint8_t live = DRAG_PENDING_KIND ? surface_is_alive(id) : 1;
        sexos_line(
            "[shell.interact.stage.drag_capture] phase=begin target=%llu live=%u capture=1 release=0 ok=1",
            U64(id), (unsigned)live);
        shell_interaction_contract_mark_stage(SHELL_INTERACTION_STAGE_DRAG);
        shell_interaction_contract_try_done();
    }
    if (CLICK_DRAG_PROOF_ENABLED) {
        click_drag_proof_begin_drag_once();
        DRAG_CAPTURE_MOVE_SEEN = false;
        uint8_t live = DRAG_PENDING_KIND ? surface_is_alive(id) : 1;
        sexos_line("[drag.capture.begin] target=%llu live=%u x=%d y=%d ok=1", U64(id),
                   (unsigned)live, x, y);
    }
    uint8_t live = DRAG_PENDING_KIND ? surface_is_alive(id) : 1;
    surface_input_lifetime_mark_drag_target(id, live, "begin", live);
    surface_input_lifetime_try_done();
}

static void chrome_click_surface(ShellChromeContext *ctx, uint64_t sid, int32_t x, int32_t y) {
    if (sid != FOCUSED_SURFACE_ID) {
        sexos_line("[shell.click_focus.hit] id=%llu", U64(sid));
        sexos_line("[shell.click_focus.send.start] id=%llu", U64(sid));
        (void)try_set_focus(sid);
        sexos_line("[shell.click_focus.send.ok] id=%llu", U64(sid));
    } else
        sexos_line("[shell.click_focus.hit] id=%llu", U64(sid));
    uint8_t live = surface_is_alive(sid), committed = sid == FOCUSED_SURFACE_ID ? 0 : 1;
    surface_input_lifetime_mark_click_target(sid, live, committed, live);
    surface_input_lifetime_try_done();
    if (live && chrome_budget(&ctx->chrome->click_hit_test_and_focus__CLICK_HIT_LIVE_OK_BUDGET))
        sexos_line("[silk.click.hit.live.ok] target=%llu x=%d y=%d", U64(sid), x, y);
    if (sid == FOCUSED_SURFACE_ID &&
        chrome_budget(&ctx->chrome->click_hit_test_and_focus__FOCUS_SET_OK_BUDGET))
        sexos_line("[silk.focus.set.ok] id=%llu source=click", U64(sid));
    if (sid == FOCUSED_SURFACE_ID &&
        chrome_budget(&ctx->chrome->click_hit_test_and_focus__SILK_FOCUS_CLICK_BUDGET))
        sexos_line("[silk.focus.click] surface=%llu ok=1", U64(sid));
}

static bool chrome_click_rim(ShellChromeContext *ctx, uint32_t frame, int32_t x, int32_t y) {
    for (size_t i = 0; i < MAX_FRAMES; ++i) {
        ShellFrame f = FRAMES[i];
        if (!f.present || f.frame_id != frame)
            continue;
        const char *reason = "";
        bool reject = false, fatal = false;
        uint64_t sid = 0;
        if (f.scene_id != ACTIVE_SCENE_IDX) {
            reason = "inactive_scene";
            reject = true;
        } else if (f.flags & FRAME_FLAG_MINIMIZED) {
            reason = "minimized";
            reject = true;
        } else if (active_surface_for_frame(frame, &sid)) {
            if (!surface_is_alive(sid)) {
                reason = "dead";
                reject = true;
                fatal = true;
            } else if (is_tombstoned(sid)) {
                reason = "tombstoned";
                reject = true;
                fatal = true;
            }
        }
        if (reject)
            sexos_line("[frame.light.reject.inactive] frame=%u reason=%s", frame, reason);
        if (fatal)
            return false;
        break;
    }
    uint32_t light = frame_light_at(frame, x, y);
    if (light == FRAME_LIGHT_CLOSE) {
        uint64_t sid = 0;
        if (active_surface_for_frame(frame, &sid)) {
            if (is_closeable_surface(sid)) {
                if (close_surface_from_frame_light(sid)) {
                    if (chrome_budget(
                            &ctx->chrome->click_hit_test_and_focus__FRAME_LIGHT_CLOSE_BUDGET))
                        sexos_line("[shell.frame.light.close] frame=%u surface=%llu", frame,
                                   U64(sid));
                } else
                    sexos_line(
                        "[shell.frame.light.close.reject] frame=%u surface=%llu reason=failed",
                        frame, U64(sid));
            } else
                sexos_line(
                    "[shell.frame.light.close.reject] frame=%u surface=%llu reason=not_closeable",
                    frame, U64(sid));
        } else
            sexos_line("[shell.frame.light.close.reject] frame=%u reason=no_active_surface", frame);
    } else if (light == FRAME_LIGHT_MINIMIZE) {
        if (!minimize_frame(frame) &&
            chrome_budget(&ctx->chrome->click_hit_test_and_focus__FRAME_MINIMIZE_REJECT_BUDGET))
            sexos_line("[shell.frame.minimize.reject] frame=%u reason=not_minimizable", frame);
    } else if (light == FRAME_LIGHT_ZOOM) {
        if (!toggle_zoom_frame(frame) &&
            chrome_budget(&ctx->chrome->click_hit_test_and_focus__FRAME_ZOOM_REJECT_BUDGET))
            sexos_line("[shell.frame.zoom.reject] frame=%u reason=not_zoomable", frame);
    } else {
        uint64_t sid = 0;
        if (active_surface_for_frame(frame, &sid)) {
            if (surface_is_alive(sid)) {
                try_transition(((InteractionState){.tag = INTERACTION_DRAGGING,
                                                   .surface_id = sid,
                                                   .current_x = x,
                                                   .current_y = y}));
                silk_drag_begin_mark(sid, x, y);
                if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__RIM_DRAG_START_BUDGET)) {
                    sexos_line("[shell.frame.rim.drag.start] frame=%u surface=%llu x=%d y=%d",
                               frame, U64(sid), x, y);
                    sexos_line("[shell.drag.begin] sid=%llu frame=%u x=%d y=%d", U64(sid), frame, x,
                               y);
                }
                if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__RIM_DRAG_BEGIN_OK_BUDGET))
                    sexos_line("[silk.drag.begin.ok] sid=%llu zone=chrome frame=%u x=%d y=%d",
                               U64(sid), frame, x, y);
            } else
                sexos_line("[shell.frame.rim.drag.reject] frame=%u reason=dead", frame);
        } else
            sexos_line("[shell.frame.rim.drag.reject] frame=%u reason=no_active_surface", frame);
    }
    return true;
}

static void chrome_click_resize(ShellChromeContext *ctx, uint32_t frame, int32_t x, int32_t y) {
    uint64_t sid = 0;
    if (!active_surface_for_frame(frame, &sid)) {
        sexos_line("[shell.frame.resize.reject] frame=%u reason=no_active_surface", frame);
        return;
    }
    if (!surface_is_alive(sid)) {
        sexos_line("[shell.frame.resize.reject] frame=%u reason=dead", frame);
        return;
    }
    int32_t bx = 0, by = 0;
    uint32_t w = 0, h = 0;
    if (!get_surface_bounds(sid, &bx, &by, &w, &h))
        return;
    uint8_t edge = compute_resize_edge(x, y, bx, by, w, h);
    ChromeDebugChunk name = chrome_edge_chunk(edge);
    sexos_line("[silk.resize.hit] frame=%u surface=%llu edge=%s x=%d y=%d", frame, U64(sid),
               name.bytes, x, y);
    try_transition(((InteractionState){.tag = INTERACTION_RESIZING,
                                       .surface_id = sid,
                                       .edge = edge,
                                       .start_x = x,
                                       .start_y = y,
                                       .start_w = w,
                                       .start_h = h}));
    if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__RESIZE_BEGIN_BUDGET))
        sexos_line("[silk.resize.begin] sid=%llu frame=%u edge=%s x=%d y=%d w=%u h=%u", U64(sid),
                   frame, name.bytes, bx, by, w, h);
}

static void chrome_click_tab(ShellChromeContext *ctx, uint32_t frame, int32_t x, int32_t y) {
    uint32_t tab = 0;
    if (!frame_tab_at(frame, x, y, &tab))
        return;
    sexos_line("[silk.tab.hit] frame=%u tab=%u x=%d y=%d", frame, tab, x, y);
    TAB_DRAG_PENDING_FRAME_ID = frame;
    TAB_DRAG_PENDING_START_TAB = (uint8_t)tab;
    if (switch_to_tab(frame, tab))
        sexos_line("[silk.tab.select] frame=%u tab=%u", frame, tab);
    else if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__TAB_SWITCH_REJECT_BUDGET))
        sexos_line("[shell.frame.tab.switch.reject] frame=%u tab=%u reason=switch_failed", frame,
                   tab);
}

static void chrome_click_policy(ShellChromeContext *ctx, HitTarget target, int32_t x, int32_t y,
                                uint8_t buttons, bool bar, uint64_t old) {
    bool chrome_drag = false;
    const char *owned_reason = "none";
    if (target.tag == HIT_TARGET_SURFACE) {
        uint64_t sid = target.surface_id;
        uint32_t frame = 0;
        int32_t sx = 0, sy = 0;
        uint32_t sw = 0, sh = 0;
        if (frame_for_surface(sid, &frame) && frame_accepts_input(frame) &&
            get_surface_bounds(sid, &sx, &sy, &sw, &sh)) {
            bool top = frame_has_top_bar(frame);
            int32_t band = top ? FRAME_TOP_BAR_HEIGHT_PX : FRAME_RIM_PX;
            bool rim = chrome_point_rim(x, y, sx, sy, sw, sh, band), strip = false;
            uint32_t tab = 0;
            if (FRAME_TAB_STRIP_PX > 0 || top) {
                int32_t start = chrome_add(sx, top ? FRAME_TOP_BAR_LIGHT_EXCLUSION_PX
                                                   : FRAME_TAB_LIGHT_EXCLUSION_PX),
                        end = chrome_sub(chrome_add(sx, (int32_t)sw), FRAME_RIM_PX);
                strip = y >= sy &&
                        y < chrome_add(sy, top ? FRAME_TOP_BAR_HEIGHT_PX : FRAME_TAB_STRIP_PX) &&
                        x >= start && x < end && frame_tab_at(frame, x, y, &tab);
            }
            uint32_t light = frame_light_at(frame, x, y);
            bool in_light = light == FRAME_LIGHT_CLOSE || light == FRAME_LIGHT_MINIMIZE ||
                            light == FRAME_LIGHT_ZOOM;
            if (rim && !strip && !in_light) {
                chrome_drag = true;
                owned_reason = "surface_rim_topbar_zone";
            } else if (strip)
                owned_reason = "tab_strip";
            else if (in_light)
                owned_reason = "frame_light";
        }
    }
    bool app_owned = target.tag == HIT_TARGET_SURFACE && !is_shell_surface(FOCUSED_SURFACE_ID);
    bool allow = is_shell_surface(FOCUSED_SURFACE_ID) || chrome_drag,
         content = target.tag != HIT_TARGET_FRAME_CHROME;
    sexos_line(
        "[shell.drag.policy] target=%llu kind=%u app_owned=%u chrome_owned=%u allow=%u reason=%s",
        U64(DRAG_PENDING_TARGET), (unsigned)DRAG_PENDING_KIND, (unsigned)app_owned,
        (unsigned)chrome_drag, (unsigned)allow, owned_reason);
    if (!bar && content && allow && point_in_surface(x, y, FOCUSED_SURFACE_ID)) {
        uint64_t sid = FOCUSED_SURFACE_ID;
        try_transition(((InteractionState){
            .tag = INTERACTION_DRAGGING, .surface_id = sid, .current_x = x, .current_y = y}));
        silk_drag_begin_mark(sid, x, y);
        sexos_line("[shell.interact.drag.begin] sid=%llu x=%d y=%d", U64(sid), x, y);
        sexos_line("[shell.drag.begin] sid=%llu frame=0 x=%d y=%d", U64(sid), x, y);
        if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__DRAG_BEGIN_OK_BUDGET))
            sexos_line("[silk.drag.begin.ok] sid=%llu zone=content x=%d y=%d", U64(sid), x, y);
        DRAG_PENDING_ACTIVE = false;
    } else if (!bar && target.tag == HIT_TARGET_FRAME_CHROME &&
               target.kind == FRAME_CHROME_TAB_STRIP)
        sexos_line("[shell.drag.skip.chrome] kind=tab_strip x=%d y=%d", x, y);
    else if (buttons & 1) {
        const char *reason = bar        ? "silkbar_handled"
                             : !content ? "non_content_target"
                             : !allow   ? "focused_not_shell_surface"
                             : !point_in_surface(x, y, FOCUSED_SURFACE_ID)
                                 ? "outside_focused_surface"
                                 : "unknown";
        sexos_line("[shell.drag.begin.reject] reason=%s target=%llu kind=%u buttons=0x%x dx=0 dy=0",
                   reason, U64(DRAG_PENDING_TARGET), (unsigned)DRAG_PENDING_KIND,
                   (unsigned)buttons);
        if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__SILK_DRAG_REJECT_BUDGET))
            sexos_line("[silk.drag.reject] reason=no_surface ok=1");
    }
    if (SHELL_INTERACTION_CONTRACT_PROOF_ENABLED) {
        uint64_t id = 0;
        const char *kind = hit_target_label(target, bar, &id);
        uint64_t now = FOCUSED_SURFACE_ID;
        sexos_line(
            "[shell.interact.stage.click_focus] target=%llu kind=%s old_focus=%llu new_focus=%llu ok=%u",
            U64(id), kind, U64(old), U64(now),
            (unsigned)(target.tag == HIT_TARGET_NONE || now != 0));
        shell_interaction_contract_mark_stage(SHELL_INTERACTION_STAGE_CLICK);
        shell_interaction_contract_try_done();
    }
    if (CLICK_DRAG_PROOF_ENABLED) {
        uint64_t id = target.tag == HIT_TARGET_SURFACE        ? target.surface_id
                      : target.tag == HIT_TARGET_FRAME_CHROME ? target.frame_id
                                                              : 0;
        sexos_line("[click.focus.commit] old_focus=%llu new_focus=%llu target=%llu changed=%u ok=1",
                   U64(old), U64(FOCUSED_SURFACE_ID), U64(id),
                   (unsigned)(old != FOCUSED_SURFACE_ID));
    }
}

ShellChromeClick shell_chrome_click_hit_test_and_focus(ShellChromeContext *ctx, int32_t x,
                                                       int32_t y, uint8_t buttons) {
    uint64_t old = FOCUSED_SURFACE_ID;
    sexos_line("[shell.click_focus.down] x=%d y=%d buttons=0x%x", x, y, (unsigned)buttons);
    if (chrome_click_atlas(ctx, x, y))
        return (ShellChromeClick){.silkbar = true};
    HitTarget target = hit_test_at(x, y);
    if (chrome_click_toolbar(ctx, target, x, y, buttons))
        return (ShellChromeClick){target, true};
    chrome_click_trace(ctx, target, x, y);
    chrome_click_pending(ctx, target, x, y, buttons);
    if (target.tag == HIT_TARGET_SURFACE)
        chrome_click_surface(ctx, target.surface_id, x, y);
    else if (target.tag == HIT_TARGET_NONE)
        sexos_line("[shell.click_focus.miss]");
    else if (target.kind == FRAME_CHROME_RIM) {
        if (!chrome_click_rim(ctx, target.frame_id, x, y))
            return (ShellChromeClick){.silkbar = true};
    } else if (target.kind == FRAME_CHROME_RESIZE)
        chrome_click_resize(ctx, target.frame_id, x, y);
    else if (target.kind == FRAME_CHROME_TAB_STRIP)
        chrome_click_tab(ctx, target.frame_id, x, y);
    else if (chrome_budget(&ctx->chrome->click_hit_test_and_focus__CHROME_CAPTURE_BUDGET))
        sexos_line("[shell.frame.chrome.capture] frame=%u kind=%u x=%d y=%d", target.frame_id,
                   target.kind, x, y);
    bool bar = handle_silkbar_click(x, y);
    chrome_click_policy(ctx, target, x, y, buttons, bar, old);
    return (ShellChromeClick){target, bar};
}


