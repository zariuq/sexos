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

static ShellInputContext proofs_b_context(void) {
    return (ShellInputContext){.shell = {.input = &SILK_SHELL_STATE.input,
                                         .chrome = &SILK_SHELL_STATE.chrome,
                                         .models = &SILK_SHELL_STATE.models,
                                         .apps = &SILK_SHELL_STATE.apps,
                                         .proofs_a = &SILK_SHELL_STATE.proofs_a},
                               .atlas = &SILK_SHELL_STATE.atlas,
                               .surfaces = &SILK_SHELL_STATE.surfaces};
}

static ShellProofsBInputContext proofs_b_input_context(void) {
    return (ShellProofsBInputContext){proofs_b_context(), &SILK_SHELL_STATE.proofs_b};
}

void maybe_run_command_palette_status_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_command_palette_status_proof(&ctx);
}

void maybe_run_command_palette_linen_status_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_command_palette_linen_status_proof(&ctx);
}

void maybe_run_quil_status_unblock_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_quil_status_unblock_proof(&ctx);
}

void maybe_run_command_palette_daily_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_command_palette_daily_proof(&ctx);
}

void maybe_run_app_launcher_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_app_launcher_proof(&ctx);
}

void maybe_run_app_launcher_help_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_app_launcher_help_proof(&ctx);
}

void maybe_run_linen_search_filter_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_linen_search_filter_proof(&ctx);
}

void maybe_run_bell_filter_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_bell_filter_proof(&ctx);
}

void maybe_run_atlas_preview_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_preview_proof(&ctx);
}

void maybe_run_atlas_phase_a_state_model_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_a_state_model_proof(&ctx);
}

void maybe_run_atlas_phase_b_snapshot_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_b_snapshot_proof(&ctx);
}

void maybe_run_atlas_phase_c_render_stub_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_c_render_stub_proof(&ctx);
}

void maybe_run_atlas_phase_d_frame_preview_stub_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_d_frame_preview_stub_proof(&ctx);
}

void maybe_run_app_registry_readonly_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_app_registry_readonly_proof(&ctx);
}

void maybe_run_app_registry_filter_sort_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_app_registry_filter_sort_proof(&ctx);
}

void maybe_run_app_registry_launch_intent_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_app_registry_launch_intent_proof(&ctx);
}

void maybe_run_app_launcher_multi_exec_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_app_launcher_multi_exec_proof(&ctx);
}

void maybe_run_palette_rejects_app_open_batch_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_palette_rejects_app_open_batch_proof(&ctx);
}

void maybe_run_atlas_phase_e1_click_scene_switch_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_e1_click_scene_switch_proof(&ctx);
}

void maybe_run_atlas_phase_e2_keyboard_scene_cycle_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_e2_keyboard_scene_cycle_proof(&ctx);
}

void maybe_run_atlas_phase_e3_drag_begin_marker_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_e3_drag_begin_marker_proof(&ctx);
}

void maybe_run_atlas_phase_e4b_same_scene_noop_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_e4b_same_scene_noop_proof(&ctx);
}

void maybe_run_atlas_phase_e4c_cross_scene_reparent_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_e4c_cross_scene_reparent_proof(&ctx);
}

void maybe_run_atlas_phase_e4c2_true_reparent_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_e4c2_true_reparent_proof(&ctx);
}

void maybe_run_atlas_phase_e4d_real_pointer_drop_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_phase_e4d_real_pointer_drop_proof(&ctx);
}

void maybe_run_atlas_overview_final_closeout_proof(void) {
    ShellInputContext ctx = proofs_b_context();
    shell_proofs_b_maybe_run_atlas_overview_final_closeout_proof(&ctx, &SILK_SHELL_STATE.proofs_b);
}

void maybe_run_window_drag_synthetic_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_window_drag_synthetic_proof(&ctx);
}

void maybe_run_silk_de_integrated_interaction_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_silk_de_integrated_interaction_proof(&ctx);
}

void maybe_run_keyboard_window_synthetic_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_keyboard_window_synthetic_proof(&ctx);
}

void maybe_run_visible_focus_topbar_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_visible_focus_topbar_proof(&ctx);
}

void maybe_run_keyboard_safe_close_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_keyboard_safe_close_proof(&ctx);
}

void maybe_run_keyboard_gui_broad_action_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_keyboard_gui_broad_action_proof(&ctx);
}

void maybe_run_spindle_real_keyboard_focus_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_spindle_real_keyboard_focus_proof(&ctx);
}

void maybe_run_mesh_keyboard_map_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_mesh_keyboard_map_proof(&ctx);
}

void maybe_run_frame_light_zoom_synthetic_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_frame_light_zoom_synthetic_proof(&ctx);
}

void maybe_run_input_negative_proof(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_input_negative_proof(&ctx);
}

void maybe_run_cursor_autodemo(void) {
    ShellProofsBInputContext ctx = proofs_b_input_context();
    shell_proofs_b_maybe_run_cursor_autodemo(&ctx);
}


