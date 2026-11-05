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

#include "silk_shell_core.h"
#include "silk_shell_state_defaults.h"
#include <string.h>
SilkShellState SILK_SHELL_STATE;

void silk_shell_state_init(SilkShellState *state) {
    memset(state, 0, sizeof *state);
    silk_shell_apply_static_defaults(state);
    atomic_init(&state->apps.width, 1280);
    atomic_init(&state->apps.height, 720);
    atomic_init(&state->proofs_a.CAP_READY_DISPLAY, false);
    atomic_init(&state->proofs_a.DEFER_EMITTED_DISPLAY, false);
    atomic_init(&state->proofs_a.EDGE_SEND_EMITTED_DISPLAY, false);
    state->input.INTERACTION = (InteractionState){.tag = INTERACTION_IDLE};
    state->chrome.WINDOWS_len = 2;
    state->chrome.WINDOWS[0] = (WindowState){.desc = {.window_id = 1,
                                                      .x = 0,
                                                      .y = 0,
                                                      .width = 1280,
                                                      .height = 720,
                                                      .z_index = 0,
                                                      .focus_state = 0}};
    state->chrome.WINDOWS[1] = (WindowState){.desc = {.window_id = 2,
                                                      .x = 100,
                                                      .y = 100,
                                                      .width = 800,
                                                      .height = 500,
                                                      .z_index = 1,
                                                      .focus_state = 1}};
    state->chrome.FOCUS_ID = 2;
    ShellFrame base = {0};
    base.present = true;
    base.frame_id = 1;
    base.active_tab = 0;
    base.tab_count = 2;
    base.tabs[0] = (ShellTab){true, SURFACE_ID_APP, 0, 0};
    base.tabs[1] = (ShellTab){true, SURFACE_ID_STATIC, 0, 0};
    base.scene_id = 0;
    base.flags = FRAME_FLAG_TOP_BAR;
    base.normal_x = 100;
    base.normal_y = 100;
    base.normal_w = 800;
    base.normal_h = 500;
    state->chrome.FRAMES[0] = base;
}

void silk_shell_proofs_tick(void) {
    maybe_run_command_palette_status_proof();
    maybe_run_command_palette_linen_status_proof();
    maybe_run_quil_status_unblock_proof();
    maybe_run_command_palette_daily_proof();
    maybe_run_app_launcher_proof();
    maybe_run_app_launcher_help_proof();
    maybe_run_linen_search_filter_proof();
    maybe_run_bell_filter_proof();
    maybe_run_atlas_preview_proof();
    maybe_run_atlas_phase_a_state_model_proof();
    maybe_run_atlas_phase_b_snapshot_proof();
    maybe_run_atlas_phase_c_render_stub_proof();
    maybe_run_atlas_phase_d_frame_preview_stub_proof();
    maybe_run_app_registry_readonly_proof();
    maybe_run_app_registry_filter_sort_proof();
    maybe_run_app_registry_launch_intent_proof();
    maybe_run_app_launcher_multi_exec_proof();
    maybe_run_palette_rejects_app_open_batch_proof();
    maybe_run_atlas_phase_e1_click_scene_switch_proof();
    maybe_run_atlas_phase_e2_keyboard_scene_cycle_proof();
    maybe_run_atlas_phase_e3_drag_begin_marker_proof();
    maybe_run_atlas_phase_e4b_same_scene_noop_proof();
    maybe_run_atlas_phase_e4c_cross_scene_reparent_proof();
    maybe_run_atlas_phase_e4c2_true_reparent_proof();
    maybe_run_atlas_phase_e4d_real_pointer_drop_proof();
    maybe_run_atlas_overview_final_closeout_proof();
    maybe_run_window_drag_synthetic_proof();
    maybe_run_silk_de_integrated_interaction_proof();
    maybe_run_keyboard_window_synthetic_proof();
    maybe_run_visible_focus_topbar_proof();
    maybe_run_keyboard_safe_close_proof();
    maybe_run_keyboard_gui_broad_action_proof();
    maybe_run_spindle_real_keyboard_focus_proof();
    maybe_run_mesh_keyboard_map_proof();
    maybe_run_frame_light_zoom_synthetic_proof();
    maybe_run_input_negative_proof();
    maybe_run_cursor_autodemo();
}


