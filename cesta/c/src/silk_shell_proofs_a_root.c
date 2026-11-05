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

#include "silk_shell_proofs_a.h"

void maybe_run_bell_workflow_detail_proof(void) {
    shell_proofs_a_maybe_run_bell_workflow_detail_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_app_lifecycle_state_proof(void) {
    shell_proofs_a_maybe_run_app_lifecycle_state_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_app_lifecycle_close_restore_proof(void) {
    shell_proofs_a_maybe_run_app_lifecycle_close_restore_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_bell_delivery_audit_proof(void) {
    shell_proofs_a_maybe_run_bell_delivery_audit_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_app_lifecycle_summary_v2_proof(void) {
    shell_proofs_a_maybe_run_app_lifecycle_summary_v2_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_app_registry_lifecycle_v2_proof(void) {
    shell_proofs_a_maybe_run_app_registry_lifecycle_v2_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_stub_proof(void) {
    shell_proofs_a_maybe_run_browser_stub_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_localdoc_stub_proof(void) {
    shell_proofs_a_maybe_run_browser_localdoc_stub_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_webstub_localdoc_surface_text_proof(void) {
    shell_proofs_a_maybe_run_webstub_localdoc_surface_text_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_webstub_static_text_render_proof(void) {
    shell_proofs_a_maybe_run_webstub_static_text_render_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_shell_draw_text_helper_proof(void) {
    shell_proofs_a_maybe_run_shell_draw_text_helper_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_stub_v2_proof(void) {
    shell_proofs_a_maybe_run_browser_stub_v2_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_localdoc_viewer_proof(void) {
    shell_proofs_a_maybe_run_browser_localdoc_viewer_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_url_bar_intent_proof(void) {
    shell_proofs_a_maybe_run_browser_url_bar_intent_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_history_proof(void) {
    shell_proofs_a_maybe_run_browser_history_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_bookmarks_proof(void) {
    shell_proofs_a_maybe_run_browser_bookmarks_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_tabs_proof(void) {
    shell_proofs_a_maybe_run_browser_tabs_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_actions_proof(void) {
    shell_proofs_a_maybe_run_browser_actions_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_dashboard_proof(void) {
    shell_proofs_a_maybe_run_browser_dashboard_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_find_proof(void) {
    shell_proofs_a_maybe_run_browser_find_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_reader_proof(void) {
    shell_proofs_a_maybe_run_browser_reader_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_save_proof(void) {
    shell_proofs_a_maybe_run_browser_save_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_export_proof(void) {
    shell_proofs_a_maybe_run_browser_export_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_url_parse_proof(void) {
    shell_proofs_a_maybe_run_browser_url_parse_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_sexnet_source3_proof(void) {
    shell_proofs_a_maybe_run_browser_sexnet_source3_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_render_stability_proof(void) {
    shell_proofs_a_maybe_run_browser_render_stability_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_sexnet_browser_cap_proof(void) {
    shell_proofs_a_maybe_run_sexnet_browser_cap_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_sexnet_status_route_proof(void) {
    shell_proofs_a_maybe_run_sexnet_status_route_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_net_grant_proof(void) {
    shell_proofs_a_maybe_run_browser_net_grant_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_http_client_status_proof(void) {
    shell_proofs_a_maybe_run_http_client_status_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_http_req_builder_proof(void) {
    shell_proofs_a_maybe_run_http_req_builder_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_sexnet_http_handshake_proof(void) {
    shell_proofs_a_maybe_run_sexnet_http_handshake_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_qemu_e1000_pci_enum_proof(void) {
    shell_proofs_a_maybe_run_qemu_e1000_pci_enum_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_pci_net_status_proof(void) {
    shell_proofs_a_maybe_run_pci_net_status_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_e1000_bar_meta_proof(void) {
    shell_proofs_a_maybe_run_e1000_bar_meta_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_e1000_driver_status_proof(void) {
    shell_proofs_a_maybe_run_e1000_driver_status_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_e1000_ring_alloc_proof(void) {
    shell_proofs_a_maybe_run_e1000_ring_alloc_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_html_proof(void) {
    shell_proofs_a_maybe_run_browser_html_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_html_link_proof(void) {
    shell_proofs_a_maybe_run_browser_html_link_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_html_history_proof(void) {
    shell_proofs_a_maybe_run_browser_html_history_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_url_intent_proof(void) {
    shell_proofs_a_maybe_run_browser_url_intent_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_browser_placeholder_surface_visual_proof(void) {
    shell_proofs_a_maybe_run_browser_placeholder_surface_visual_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_frame_chrome_model_proof(void) {
    shell_proofs_a_maybe_run_frame_chrome_model_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_frame_rim_markers_proof(void) {
    shell_proofs_a_maybe_run_frame_rim_markers_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_bell_launch_outcome_proof(void) {
    shell_proofs_a_maybe_run_bell_launch_outcome_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_project_scene_link_proof(void) {
    shell_proofs_a_maybe_run_project_scene_link_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_mesh_graph_status_proof(void) {
    shell_proofs_a_maybe_run_mesh_graph_status_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_collar_grant_status_proof(void) {
    shell_proofs_a_maybe_run_collar_grant_status_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_atlas_scene_stub_proof(void) {
    shell_proofs_a_maybe_run_atlas_scene_stub_proof(&SILK_SHELL_STATE.atlas);
}

void maybe_run_scene_lifecycle_markers_proof(void) {
    shell_proofs_a_maybe_run_scene_lifecycle_markers_proof(&SILK_SHELL_STATE.proofs_a);
}

void maybe_run_scene_keyboard_switch_proof(void) {
    shell_proofs_a_maybe_run_scene_keyboard_switch_proof(&SILK_SHELL_STATE.proofs_a);
}


