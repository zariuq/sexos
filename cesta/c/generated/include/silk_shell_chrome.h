
#ifndef SEXOS_SHELL_CHROME_H
#define SEXOS_SHELL_CHROME_H
#include "silk_shell.h"
#include "silk_shell_viewport.h"
#include "silkbar_geometry.h"

typedef struct ShellChromeClick {
    HitTarget target;
    bool silkbar;
} ShellChromeClick;

typedef struct ShellChromeMove {
    int32_t x, y;
    uint8_t clamped;
} ShellChromeMove;

typedef struct ShellChromeDragTarget {
    uint32_t frame;
    uint64_t sid;
    int32_t x, y;
    uint32_t w, h;
    int32_t px, py;
} ShellChromeDragTarget;

typedef struct ShellAccessLabel {
    uint8_t bytes[32];
} ShellAccessLabel;

typedef struct ShellAccessText {
    uint8_t bytes[32];
    size_t len;
} ShellAccessText;

typedef struct ShellAccessNodes {
    AccessNode values[MAX_ACCESS_NODES];
} ShellAccessNodes;

typedef struct ShellChromeContext {
    SilkShellChromeState *chrome;
    SilkShellModelsState *models;
    ShellChromeApps *apps;
    SilkShellInputState *input;
    SilkShellProofsAState *proofs_a;
} ShellChromeContext;

bool shell_chrome_scancode_to_action(ShellChromeContext *ctx, uint8_t scancode,
                                     uint8_t *out_action);
const char *shell_chrome_action_name(ShellChromeContext *ctx, uint8_t action);
int32_t shell_chrome_desktop_width(ShellChromeContext *ctx);
int32_t shell_chrome_desktop_height(ShellChromeContext *ctx);
void shell_chrome_layout_left(ShellChromeContext *ctx, int32_t *out_x, int32_t *out_y,
                              uint32_t *out_w, uint32_t *out_h);
void shell_chrome_layout_right(ShellChromeContext *ctx, int32_t *out_x, int32_t *out_y,
                               uint32_t *out_w, uint32_t *out_h);
void shell_chrome_layout_maximize(ShellChromeContext *ctx, int32_t *out_x, int32_t *out_y,
                                  uint32_t *out_w, uint32_t *out_h);
void shell_chrome_clamp_position(ShellChromeContext *ctx, int32_t x, int32_t y, uint32_t w,
                                 uint32_t h, int32_t *out_x, int32_t *out_y);
void shell_chrome_snap_end_pos(ShellChromeContext *ctx, uint32_t w, uint32_t h, int32_t *out_x,
                               int32_t *out_y);
bool shell_chrome_lifecycle_register(ShellChromeContext *ctx, uint64_t sid, uint8_t initial_state);
bool shell_chrome_lifecycle_state(ShellChromeContext *ctx, uint64_t sid, uint8_t *out_state);
bool shell_chrome_set_lifecycle_state(ShellChromeContext *ctx, uint64_t sid, uint8_t next);
bool shell_chrome_surface_generation(ShellChromeContext *ctx, uint64_t sid, uint64_t *out_gen);
bool shell_chrome_bump_surface_generation(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_make_focus_ref(ShellChromeContext *ctx, uint64_t sid, FocusRef *out_ref);
bool shell_chrome_focus_ref_is_current(ShellChromeContext *ctx, const FocusRef *r);
void shell_chrome_sync_focus_ref(ShellChromeContext *ctx);
bool shell_chrome_surface_is_lifecycle_live(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_surface_is_lifecycle_focusable(ShellChromeContext *ctx, uint64_t sid);
void shell_chrome_lifecycle_init_all(ShellChromeContext *ctx);
bool shell_chrome_is_shell_surface(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_is_focusable_surface(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_frame_for_surface(ShellChromeContext *ctx, uint64_t sid, uint32_t *out_frame_id);
bool shell_chrome_active_surface_for_frame(ShellChromeContext *ctx, uint32_t id,
                                           uint64_t *out_surface_id);
bool shell_chrome_selected_frame_id(ShellChromeContext *ctx, uint32_t *out_frame_id);
bool shell_chrome_selected_surface_id(ShellChromeContext *ctx, uint64_t *out_sid);
uint32_t shell_chrome_selected_window_options_mask(ShellChromeContext *ctx);
bool shell_chrome_frame_is_minimized(ShellChromeContext *ctx, uint32_t frame_id);
void shell_chrome_set_frame_minimized(ShellChromeContext *ctx, uint32_t frame_id, bool minimized);
bool shell_chrome_frame_is_zoomed(ShellChromeContext *ctx, uint32_t frame_id);
void shell_chrome_set_frame_zoomed(ShellChromeContext *ctx, uint32_t frame_id, bool zoomed);
bool shell_chrome_frame_has_top_bar(ShellChromeContext *ctx, uint32_t frame_id);
void shell_chrome_set_frame_top_bar(ShellChromeContext *ctx, uint32_t frame_id, bool enabled);
bool shell_chrome_first_minimized_frame_id(ShellChromeContext *ctx, uint32_t *out_frame_id);
bool shell_chrome_frame_accepts_input(ShellChromeContext *ctx, uint32_t frame_id);
bool shell_chrome_frame_chrome_visible(ShellChromeContext *ctx, uint32_t frame_id);
void shell_chrome_clear_hover_if_wrong_scene(ShellChromeContext *ctx);
uint32_t shell_chrome_frame_tab_count(ShellChromeContext *ctx, uint32_t frame_id);
uint32_t shell_chrome_frame_active_tab_index(ShellChromeContext *ctx, uint32_t frame_id);
bool shell_chrome_frame_tab_at(ShellChromeContext *ctx, uint32_t frame_id, int32_t x, int32_t y,
                               uint32_t *out_tab);
bool shell_chrome_swap_frame_tabs(ShellChromeContext *ctx, uint32_t frame_id, uint8_t idx_a,
                                  uint8_t idx_b);
uint32_t shell_chrome_frame_light_to_option_mask(ShellChromeContext *ctx, uint32_t light);
uint64_t shell_chrome_abs_screen_to_raw(ShellChromeContext *ctx, int32_t screen, int32_t dim);
uint8_t shell_chrome_compute_resize_edge(ShellChromeContext *ctx, int32_t px, int32_t py,
                                         int32_t sx, int32_t sy, uint32_t sw, uint32_t sh);
void shell_chrome_compute_resize_rect(ShellChromeContext *ctx, int32_t cur_x, int32_t cur_y,
                                      uint32_t cur_w, uint32_t cur_h, uint8_t edge, int32_t dx,
                                      int32_t dy, int32_t *out_x, int32_t *out_y, uint32_t *out_w,
                                      uint32_t *out_h);
Rect shell_chrome_rect_new(int32_t x, int32_t y, uint32_t w, uint32_t h);
bool shell_chrome_rect_contains(const Rect *rect, int32_t x, int32_t y);
void shell_chrome_native_child_geometry(ShellChromeContext *ctx, uint64_t frame, int32_t x,
                                        int32_t y, uint32_t w, uint32_t h);
void shell_chrome_update_local_geometry(ShellChromeContext *ctx, uint64_t surface_id, int32_t x,
                                        int32_t y, uint32_t w, uint32_t h);
bool shell_chrome_is_closeable_surface(ShellChromeContext *ctx, uint64_t surface_id);
bool shell_chrome_frame_close_allowed(ShellChromeContext *ctx, uint32_t frame_id);
void shell_chrome_emit_chrome_diagnostics(ShellChromeContext *ctx, uint32_t frame_id,
                                          const char *reason);
bool shell_chrome_frame_chrome_operation(ShellChromeContext *ctx, uint32_t frame_id,
                                         ShellChromeOperation *out);
void shell_chrome_send_frame_tab_info(ShellChromeContext *ctx, uint32_t frame_id);
bool shell_chrome_close_surface_from_frame_light(ShellChromeContext *ctx, uint64_t surface_id);
bool shell_chrome_minimize_frame(ShellChromeContext *ctx, uint32_t frame_id);
bool shell_chrome_restore_minimized_frame_inner(ShellChromeContext *ctx, uint32_t frame_id,
                                                bool focus);
bool shell_chrome_restore_minimized_frame(ShellChromeContext *ctx, uint32_t frame_id);
bool shell_chrome_zoom_frame(ShellChromeContext *ctx, uint32_t frame_id);
bool shell_chrome_unzoom_frame(ShellChromeContext *ctx, uint32_t frame_id);
bool shell_chrome_toggle_zoom_frame(ShellChromeContext *ctx, uint32_t frame_id);
void shell_chrome_tile_active_scene_frames(ShellChromeContext *ctx);
void shell_chrome_tile_visible_frames(ShellChromeContext *ctx);
uint32_t shell_chrome_frame_light_at(ShellChromeContext *ctx, uint32_t frame_id, int32_t x,
                                     int32_t y);
bool shell_chrome_toggle_top_bar_for_active_frame(ShellChromeContext *ctx);
bool shell_chrome_switch_to_tab(ShellChromeContext *ctx, uint32_t frame_id, uint32_t tab_index);
bool shell_chrome_try_set_focus_inner(ShellChromeContext *ctx, uint64_t sid, bool refresh_children);
bool shell_chrome_try_set_focus(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_update_frame_hover_at(ShellChromeContext *ctx, int32_t x, int32_t y);
void shell_chrome_access_copy_label(ShellChromeContext *ctx, ShellAccessLabel *dst,
                                    ShellAccessText src);
uint32_t shell_chrome_access_label_token(ShellChromeContext *ctx, const ShellAccessLabel *label);
bool shell_chrome_access_node_is_valid_target(ShellChromeContext *ctx,
                                              const AccessTargetRef *target);
size_t shell_chrome_access_emit_scene_node(ShellChromeContext *ctx, ShellAccessNodes *nodes,
                                           size_t idx, uint8_t scene_id);
size_t shell_chrome_access_emit_frame_node(ShellChromeContext *ctx, ShellAccessNodes *nodes,
                                           size_t idx, const ShellFrame *frame);
size_t shell_chrome_access_emit_shell_nodes(ShellChromeContext *ctx, ShellAccessNodes *nodes);
void shell_chrome_access_describe_node(ShellChromeContext *ctx, const AccessNode *node);
void shell_chrome_access_describe_focus(ShellChromeContext *ctx);
bool shell_chrome_access_move_focus(ShellChromeContext *ctx, const ShellAccessNodes *nodes,
                                    size_t length, bool forward);
bool shell_chrome_access_handle_keyboard_action(ShellChromeContext *ctx, uint8_t action);
void shell_chrome_try_transition(ShellChromeContext *ctx, InteractionState next);
bool shell_chrome_move_surface_by(ShellChromeContext *ctx, uint64_t sid, int32_t dx, int32_t dy,
                                  ShellChromeMove *out);
bool shell_chrome_synthetic_window_drag_target(ShellChromeContext *ctx, ShellChromeDragTarget *out);
void shell_chrome_silk_drag_begin_mark(ShellChromeContext *ctx, uint64_t sid, int32_t x, int32_t y);
void shell_chrome_silk_drag_end_mark(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_drag_move_focused(ShellChromeContext *ctx, int32_t dx, int32_t dy);
bool shell_chrome_resize_accumulate_delta(ShellChromeContext *ctx, int32_t dx, int32_t dy);
void shell_chrome_clear_resize_if_dead(ShellChromeContext *ctx);
bool shell_chrome_try_snap_on_drag_release(ShellChromeContext *ctx, uint64_t sid, int32_t px,
                                           int32_t py);
bool shell_chrome_apply_resize_geometry(ShellChromeContext *ctx, int32_t dx, int32_t dy);
bool shell_chrome_hit_test_surface_chrome(ShellChromeContext *ctx, int32_t x, int32_t y,
                                          uint64_t sid, HitTarget *out);
HitTarget shell_chrome_hit_test_at(ShellChromeContext *ctx, int32_t x, int32_t y);
bool shell_chrome_hit_target_surface(ShellChromeContext *ctx, HitTarget target, uint64_t *out);
const char *shell_chrome_hit_target_label(ShellChromeContext *ctx, HitTarget target, bool bar,
                                          uint64_t *out);
bool shell_chrome_toggle_os_panel(ShellChromeContext *ctx, bool *active, uint8_t panel,
                                  uint64_t sid, const char *label, uint32_t x, uint32_t y,
                                  uint32_t w, uint32_t h);
void shell_chrome_toggle_scene_settings_panel(ShellChromeContext *ctx);
bool shell_chrome_handle_silkbar_click(ShellChromeContext *ctx, int32_t px, int32_t py);
ShellChromeClick shell_chrome_click_hit_test_and_focus(ShellChromeContext *ctx, int32_t x,
                                                       int32_t y, uint8_t buttons);
uint32_t shell_chrome_peer_load_relaxed(const atomic_uint_least32_t *value);
void shell_chrome_peer_store_relaxed(atomic_uint_least32_t *value, uint32_t next);
bool shell_chrome_peer_point_in_surface(ShellChromeContext *ctx, int32_t x, int32_t y,
                                        uint64_t sid);
void shell_chrome_peer_surface_input_lifetime_mark_drag_target(ShellChromeContext *ctx,
                                                               uint64_t sid, uint8_t live,
                                                               const char *phase, uint8_t ok);
void shell_chrome_peer_surface_input_lifetime_mark_click_target(ShellChromeContext *ctx,
                                                                uint64_t sid, uint8_t live,
                                                                uint8_t committed, uint8_t ok);
void shell_chrome_peer_shell_interaction_contract_mark_stage(ShellChromeContext *ctx,
                                                             uint8_t stage);
void shell_chrome_peer_shell_interaction_contract_try_done(ShellChromeContext *ctx);
void shell_chrome_peer_click_drag_proof_begin_drag_once(ShellChromeContext *ctx);
bool shell_chrome_peer_atlas_scene_at_point(ShellChromeContext *ctx, int32_t x, int32_t y,
                                            uint8_t *out);
void shell_chrome_peer_switch_scene(ShellChromeContext *ctx, uint8_t scene);
void shell_chrome_peer_sync_scene_visibility(ShellChromeContext *ctx);
void shell_chrome_peer_clear_focus_if_wrong_scene(ShellChromeContext *ctx);
void shell_chrome_peer_clear_drag_if_wrong_scene(ShellChromeContext *ctx);
void shell_chrome_peer_atlas_capture_snapshot(ShellChromeContext *ctx);
bool shell_chrome_peer_toggle_bell(ShellChromeContext *ctx);
bool shell_chrome_peer_validate_scene_id(ShellChromeContext *ctx, uint8_t scene);
void shell_chrome_peer_next_scene(ShellChromeContext *ctx);
void shell_chrome_peer_prev_scene(ShellChromeContext *ctx);
ShellAccessLabel shell_chrome_peer_frame_label(ShellChromeContext *ctx, uint32_t frame);
bool shell_chrome_peer_exchange_bool(atomic_bool *value, bool next);
bool shell_chrome_peer_surface_scene_id(ShellChromeContext *ctx, uint64_t sid, uint8_t *out);
bool shell_chrome_peer_surface_in_active_scene(ShellChromeContext *ctx, uint64_t sid);
void shell_chrome_peer_launcher_focus_discard(ShellChromeContext *ctx);
bool shell_chrome_peer_mesh_is_visible_in_active_scene(ShellChromeContext *ctx);
void shell_chrome_peer_mesh_render_pd_graph(ShellChromeContext *ctx);
size_t shell_chrome_peer_bell_ring_count(ShellChromeContext *ctx);
bool shell_chrome_peer_send_silkbar_phase2_update(ShellChromeContext *ctx, uint32_t kind,
                                                  uint64_t a, uint64_t b);
void shell_chrome_peer_surface_input_lifetime_mark_focus_live(ShellChromeContext *ctx, uint64_t sid,
                                                              uint8_t alive, uint8_t ok);
void shell_chrome_peer_surface_input_lifetime_try_done(ShellChromeContext *ctx);
const char *shell_chrome_peer_frame_name(ShellChromeContext *ctx, uint32_t id);
SexosReply shell_chrome_peer_pdx_call(ShellChromeContext *ctx, uint64_t slot, uint64_t opcode,
                                      uint64_t a, uint64_t b, uint64_t c);
bool shell_chrome_peer_app_closeable(ShellChromeContext *ctx, uint64_t sid, bool *out);
void shell_chrome_peer_shell_request_app_close(ShellChromeContext *ctx, uint64_t sid);
void shell_chrome_peer_clear_focus_if_dead(ShellChromeContext *ctx);
void shell_chrome_peer_clear_drag_if_dead(ShellChromeContext *ctx);
void shell_chrome_peer_clear_hover_if_dead(ShellChromeContext *ctx);
void shell_chrome_peer_record_tombstone_event(ShellChromeContext *ctx, uint64_t sid, uint8_t old,
                                              uint8_t next, uint8_t reason);
void shell_chrome_peer_snap_capture_layout(ShellChromeContext *ctx);
bool shell_chrome_peer_desktop_ready(ShellChromeContext *ctx, uint64_t sid);
size_t shell_chrome_peer_linen_object_count(ShellChromeContext *ctx);
void shell_chrome_peer_linen_render_object_list(ShellChromeContext *ctx);
uint32_t shell_chrome_peer_load_dimension(const atomic_uint_least32_t *dimension);
bool shell_chrome_peer_surface_is_alive(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_peer_is_tombstoned(ShellChromeContext *ctx, uint64_t sid);
bool shell_chrome_peer_get_surface_bounds(ShellChromeContext *ctx, uint64_t sid, int32_t *x,
                                          int32_t *y, uint32_t *w, uint32_t *h);
void shell_chrome_peer_clamp_surface_size(ShellChromeContext *ctx, int32_t x, int32_t y, uint32_t w,
                                          uint32_t h, uint32_t *cw, uint32_t *ch);
bool shell_chrome_peer_app_focusable(ShellChromeContext *ctx, uint64_t sid);
#endif
