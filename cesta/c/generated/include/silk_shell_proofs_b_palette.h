
#ifndef SEXOS_SHELL_PROOFS_B_PALETTE_H
#define SEXOS_SHELL_PROOFS_B_PALETTE_H
#include "silk_shell_input.h"
#include "silk_shell_surfaces.h"

typedef struct ShellProofsBFrame {
    bool present;
    uint32_t frame;
} ShellProofsBFrame;

size_t shell_proofs_b_palette_command_count(ShellInputContext *ctx);
uint8_t shell_proofs_b_palette_command_code(ShellInputContext *ctx, size_t index);
ShellPaletteString shell_proofs_b_palette_command_label(ShellInputContext *ctx, size_t index);
ShellPaletteStatus shell_proofs_b_palette_status(ShellInputContext *ctx, uint8_t command);
bool shell_proofs_b_palette_toggle(ShellInputContext *ctx);
void shell_proofs_b_palette_render(ShellInputContext *ctx);
bool shell_proofs_b_palette_execute(ShellInputContext *ctx);
void shell_proofs_b_palette_next(ShellInputContext *ctx);
void shell_proofs_b_palette_previous(ShellInputContext *ctx);
size_t shell_proofs_b_linen_selected_index(ShellInputContext *ctx);
size_t shell_proofs_b_bell_ring_count(ShellInputContext *ctx);
uint8_t shell_proofs_b_bell_visible_count(ShellInputContext *ctx);
void shell_proofs_b_bell_next(ShellInputContext *ctx);
void shell_proofs_b_atlas_toggle(ShellInputContext *ctx);
void shell_proofs_b_atlas_switch_scene(ShellInputContext *ctx, uint8_t scene);
void shell_proofs_b_atlas_collect_snapshot(ShellInputContext *ctx);
void shell_proofs_b_yield(ShellInputContext *ctx);
const char *shell_proofs_b_registry_name(ShellInputContext *ctx, size_t index);
ShellProofsBFrame shell_proofs_b_first_minimized_frame(ShellInputContext *ctx);
ShellProofsBFrame shell_proofs_b_frame_for_surface(ShellInputContext *ctx, uint64_t surface);
bool shell_proofs_b_minimize_frame(ShellInputContext *ctx, uint32_t frame);
uint64_t shell_proofs_b_browser_active_frame(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_command_palette_status_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_command_palette_linen_status_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_quil_status_unblock_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_command_palette_daily_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_app_launcher_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_app_launcher_help_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_linen_search_filter_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_bell_filter_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_atlas_preview_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_atlas_phase_a_state_model_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_atlas_phase_b_snapshot_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_atlas_phase_c_render_stub_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_atlas_phase_d_frame_preview_stub_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_app_registry_readonly_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_app_registry_filter_sort_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_app_registry_launch_intent_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_app_launcher_multi_exec_proof(ShellInputContext *ctx);
void shell_proofs_b_maybe_run_palette_rejects_app_open_batch_proof(ShellInputContext *ctx);
#endif
