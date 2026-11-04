#include "silk_shell_proofs_a_routes.h"
#include "silk_shell_surfaces.h"
#include "silk_shell_atlas.h"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

SilkShellState SILK_SHELL_STATE;
static unsigned focus_calls,quil_calls,pdx_calls,bell_calls,bell_focus_calls,atlas_calls,silkbar_calls;
static unsigned close_calls,hid_calls;static bool drag_target_active;static int32_t window_x=10,window_y=20;

static void chrome_binding(const ShellChromeContext *ctx)
{
 assert(ctx->chrome==&SILK_SHELL_STATE.chrome && ctx->models==&SILK_SHELL_STATE.models &&
  ctx->apps==&SILK_SHELL_STATE.apps && ctx->input==&SILK_SHELL_STATE.input &&
  ctx->proofs_a==&SILK_SHELL_STATE.proofs_a);
}
bool shell_chrome_try_set_focus(ShellChromeContext *ctx,uint64_t surface)
{chrome_binding(ctx);if(surface==SURFACE_ID_BROWSER)return false;assert(surface==SURFACE_ID_LINEN||surface==SURFACE_ID_SPINDLE||surface==SURFACE_ID_QUIL);ctx->chrome->FOCUSED_SURFACE_ID=surface;++focus_calls;return true;}
bool shell_surfaces_focus_or_open_quil(ShellSurfacesContext *ctx)
{chrome_binding(&ctx->shell);assert(ctx->surfaces==&SILK_SHELL_STATE.surfaces);++quil_calls;return true;}
bool shell_surfaces_focus_or_open_bell(ShellSurfacesContext *ctx)
{chrome_binding(&ctx->shell);assert(ctx->surfaces==&SILK_SHELL_STATE.surfaces);ctx->shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_BELL_PLACEHOLDER;++bell_focus_calls;return true;}
void shell_surfaces_bell_select_next_row(ShellSurfacesContext *ctx){(void)ctx;}
void shell_surfaces_bell_select_prev_row(ShellSurfacesContext *ctx){(void)ctx;}
bool shell_surfaces_bell_selected_event_snapshot(ShellSurfacesContext *ctx,BellEvent *event)
{size_t count=ctx->shell.models->BELL_RING_WRITE_INDEX<BELL_RING_CAP?(size_t)ctx->shell.models->BELL_RING_WRITE_INDEX:BELL_RING_CAP;if(!count)return false;size_t at=(size_t)(ctx->shell.models->BELL_RING_WRITE_INDEX-1)%BELL_RING_CAP;*event=ctx->shell.models->BELL_EVENTS[at];return event->present;}
void shell_surfaces_bell_emit_selected_event_detail_proof(ShellSurfacesContext *ctx){BellEvent event={0};ctx->surfaces->BELL_DETAIL_OPEN=shell_surfaces_bell_selected_event_snapshot(ctx,&event);}
void shell_surfaces_bell_close_detail(ShellSurfacesContext *ctx){ctx->surfaces->BELL_DETAIL_OPEN=false;}
bool shell_surfaces_bell_cycle_lane(ShellSurfacesContext *ctx){(void)ctx;return true;}
void shell_atlas_atlas_toggle(ShellAtlasContext *ctx)
{assert(ctx->atlas==&SILK_SHELL_STATE.atlas&&ctx->chrome==&SILK_SHELL_STATE.chrome&&ctx->models==&SILK_SHELL_STATE.models);ctx->chrome->ATLAS_MODE_ENABLED=!ctx->chrome->ATLAS_MODE_ENABLED;++atlas_calls;}
bool shell_atlas_handle_atlas_keyboard(ShellAtlasContext *ctx,uint8_t sc)
{assert(ctx->atlas==&SILK_SHELL_STATE.atlas&&ctx->chrome==&SILK_SHELL_STATE.chrome&&ctx->models==&SILK_SHELL_STATE.models);if(!ctx->chrome->ATLAS_MODE_ENABLED)return false;size_t at=ctx->chrome->ATLAS_SELECTED_SCENE;switch(sc){case 0x4d:ctx->chrome->ATLAS_SELECTED_SCENE=(uint8_t)((at+1)%ATLAS_MAX_SCENES);break;case 0x4b:ctx->chrome->ATLAS_SELECTED_SCENE=(uint8_t)((at+ATLAS_MAX_SCENES-1)%ATLAS_MAX_SCENES);break;case 0x1e:ctx->chrome->SCENES[at].accent=(uint8_t)((ctx->chrome->SCENES[at].accent+1)%8);break;case 0x2c:ctx->chrome->SCENES[at].accent=(uint8_t)((ctx->chrome->SCENES[at].accent+7)%8);break;case 0x1f:ctx->models->SCENE_APPEARANCE_STATE.preset_idx=(uint8_t)((ctx->models->SCENE_APPEARANCE_STATE.preset_idx+1)%4);break;case 0x11:ctx->models->SCENE_APPEARANCE_STATE.preset_idx=(uint8_t)((ctx->models->SCENE_APPEARANCE_STATE.preset_idx+3)%4);break;case 0x1c:ctx->models->ACTIVE_TINT_IDX=ctx->chrome->SCENES[at].accent;ctx->chrome->ATLAS_MODE_ENABLED=false;break;case 1:ctx->chrome->ATLAS_MODE_ENABLED=false;break;default:return false;}++atlas_calls;return true;}
const char *shell_atlas_peer_get_preset_name(ShellAtlasContext *ctx,uint8_t preset)
{assert(ctx->atlas==&SILK_SHELL_STATE.atlas);static const char *const names[4]={"Default","Warm","Cool","HighContrast"};return names[preset%4];}
bool shell_surfaces_open_spindle_in_active_scene(ShellSurfacesContext *ctx){chrome_binding(&ctx->shell);return true;}
bool shell_surfaces_focus_or_open_mesh(ShellSurfacesContext *ctx){chrome_binding(&ctx->shell);ctx->shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_MESH;return true;}
bool shell_surfaces_focus_or_open_linen(ShellSurfacesContext *ctx){chrome_binding(&ctx->shell);ctx->shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_LINEN;return true;}
bool shell_surfaces_toggle_command_palette(ShellSurfacesContext *ctx){ctx->surfaces->COMMAND_PALETTE_OPEN=!ctx->surfaces->COMMAND_PALETTE_OPEN;return true;}
uint8_t shell_surfaces_command_code(size_t index){return index<shell_surfaces_command_count()?(uint8_t)index:UINT8_MAX;}
ShellPaletteStatus shell_surfaces_palette_item_status(ShellSurfacesContext *ctx,uint8_t command)
{chrome_binding(&ctx->shell);return(ShellPaletteStatus){.available=command<shell_surfaces_command_count()&&command%3!=2};}
bool shell_diagnostics_send_silkbar_phase2_update(uint32_t kind,uint64_t a,uint64_t b)
{assert(kind>=SILKBAR_UPDATE_SET_ACTIVE_APP&&kind<=SILKBAR_UPDATE_SET_PALETTE_STATE);(void)a;(void)b;++silkbar_calls;return true;}
bool shell_chrome_frame_close_allowed(ShellChromeContext *ctx,uint32_t frame)
{chrome_binding(ctx);return frame==SPINDLE_FRAME_ID||frame==QUIL_FRAME_ID;}
bool shell_chrome_lifecycle_state(ShellChromeContext *ctx,uint64_t surface,uint8_t *out)
{chrome_binding(ctx);if(surface!=310)return false;*out=LIFECYCLE_VISIBLE;return true;}
bool shell_chrome_is_closeable_surface(ShellChromeContext *ctx,uint64_t surface)
{chrome_binding(ctx);return surface==310||surface==SURFACE_ID_APP;}
bool shell_chrome_close_surface_from_frame_light(ShellChromeContext *ctx,uint64_t surface)
{assert(shell_chrome_is_closeable_surface(ctx,surface));++close_calls;return true;}
uint64_t shell_chrome_abs_screen_to_raw(ShellChromeContext *ctx,int32_t screen,int32_t dimension)
{chrome_binding(ctx);if(dimension<=1)return 0;if(screen<0)screen=0;if(screen>=dimension)screen=dimension-1;return(uint64_t)screen*65535/(uint64_t)(dimension-1);}
int32_t shell_chrome_desktop_width(ShellChromeContext *ctx){chrome_binding(ctx);return 1280;}
int32_t shell_chrome_desktop_height(ShellChromeContext *ctx){chrome_binding(ctx);return 720;}
bool shell_chrome_synthetic_window_drag_target(ShellChromeContext *ctx,ShellChromeDragTarget *out)
{chrome_binding(ctx);drag_target_active=true;*out=(ShellChromeDragTarget){SPINDLE_FRAME_ID,SURFACE_ID_SPINDLE,window_x,window_y,320,200,window_x+4,window_y+4};return true;}
bool shell_chrome_peer_get_surface_bounds(ShellChromeContext *ctx,uint64_t surface,int32_t *x,int32_t *y,uint32_t *w,uint32_t *h)
{chrome_binding(ctx);if(surface!=SURFACE_ID_SPINDLE)return false;*x=window_x;*y=window_y;*w=320;*h=200;return true;}
void shell_input_handle_hid_event(ShellInputContext *ctx,uint64_t event_class,uint64_t a,uint64_t b)
{++hid_calls;if(event_class==EV_BTN){if(b==1&&drag_target_active)ctx->shell.input->INTERACTION=(InteractionState){.tag=INTERACTION_DRAGGING};else if(!b){ctx->shell.input->INTERACTION=(InteractionState){.tag=INTERACTION_IDLE};drag_target_active=false;}}else if(event_class==EV_REL&&ctx->shell.input->INTERACTION.tag==INTERACTION_DRAGGING){window_x+=(int32_t)a;window_y+=(int32_t)b;++ctx->shell.input->INPUT_TRACE_DRAG_MOVES;}}
void shell_input_peer_linen_select_next_object(ShellInputContext *ctx){(void)ctx;}
void shell_input_peer_linen_select_prev_object(ShellInputContext *ctx){(void)ctx;}
uint64_t shell_input_peer_linen_selected_object_id(ShellInputContext *ctx){return ctx->shell.models->LINEN_OBJECTS[0].present?ctx->shell.models->LINEN_OBJECTS[0].object_id:0;}
void linen_object_detail_open(void){SILK_SHELL_STATE.models.LINEN_OBJECT_DETAIL_OPEN=SILK_SHELL_STATE.models.LINEN_OBJECTS[0].present;}
void linen_object_detail_close(void){SILK_SHELL_STATE.models.LINEN_OBJECT_DETAIL_OPEN=false;}
bool shell_surfaces_open_linen_in_active_scene(ShellSurfacesContext *ctx){chrome_binding(&ctx->shell);return true;}
size_t linen_object_count(void){return SILK_SHELL_STATE.models.LINEN_OBJECTS[0].present?1:0;}
void linen_queue_open(uint64_t object,uint64_t choice){assert(object==7001&&choice==0);}
bool shell_input_peer_open_linen_object_in_quil(ShellInputContext *ctx,uint64_t object){(void)ctx;return object==7001;}
bool shell_surfaces_focus_or_open_collar(ShellSurfacesContext *ctx){chrome_binding(&ctx->shell);return true;}
void shell_input_peer_collar_select_next_grant(ShellInputContext *ctx){ctx->shell.models->COLLAR_SELECTED_GRANT_IDX=0;}
void shell_input_peer_collar_select_prev_grant(ShellInputContext *ctx){ctx->shell.models->COLLAR_SELECTED_GRANT_IDX=0;}
bool shell_input_peer_collar_emit_selected_grant_detail(ShellInputContext *ctx){return ctx->shell.models->COLLAR_GRANTS[ctx->shell.models->COLLAR_SELECTED_GRANT_IDX].present;}
bool collar_grant_at_visible_index(uint8_t idx,CollarGrant *out){if(idx||!SILK_SHELL_STATE.models.COLLAR_GRANTS[0].present)return false;*out=SILK_SHELL_STATE.models.COLLAR_GRANTS[0];return true;}
bool shell_surfaces_toggle_collar(ShellSurfacesContext *ctx){ctx->shell.models->COLLAR_OVERLAY_ENABLED=!ctx->shell.models->COLLAR_OVERLAY_ENABLED;return true;}
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c)
{assert((slot==SLOT_LINEN||slot==SLOT_QUIL)&&op==OP_HID_EVENT&&b==1&&c==EV_KEY);if(slot==SLOT_LINEN)assert(a==0x24||a==0x25||a==0x1c);else assert(a==0x1f||a==0x12||a==0x2d);++pdx_calls;return(SexosReply){0,0};}
void shell_diagnostics_bell_send_app_event(const ShellDiagnosticLabel *source,uint64_t event)
{assert(source&&source->len<sizeof source->bytes&&source->bytes[source->len]==0);assert(event>=1001&&event<=2004);++bell_calls;}
void sexos_line(const char *format,...){(void)format;}
void sexos_fail(void){abort();}

int main(void)
{
 SILK_SHELL_STATE.models.LINEN_OBJECTS[0]=(LinenObject){.present=true,.object_id=7001};
 SILK_SHELL_STATE.models.COLLAR_GRANTS[0]=(CollarGrant){.present=true,.grant_id=9001};
 maybe_run_linen_keyboard_route_proof();
 maybe_run_physical_keyboard_to_quil_focus_proof();
 maybe_run_bell_app_event_integration_proof();
 maybe_run_bell_workflow_event_proof();
 maybe_run_window_workflow_v2_proof();
 maybe_run_bell_keyboard_detail_proof();
 maybe_run_bell_detail_seed_proof();
 maybe_run_bell_system_events_proof();
 maybe_run_atlas_scene_keyboard_proof();
 maybe_run_atlas_theme_visual_proof();
 maybe_run_atlas_theme_presets_proof();
 maybe_run_silkbar_keyboard_status_proof();
 maybe_run_silkbar_palette_status_proof();
 maybe_run_silkbar_phase2_shell_proof();
 maybe_run_frame_lights_stub_proof();
 maybe_run_frame_lights_keyboard_proof();
 maybe_run_silk_focus_reject_proof();
 maybe_run_silk_drag_reject_proof();
 maybe_run_silk_window_move_proof();
 maybe_run_quil_visible_typing_e2e_proof();
 maybe_run_linen_object_detail_proof();
 maybe_run_linen_nonblocking_open_proof();
 maybe_run_collar_keyboard_grants_proof();
 assert(SILK_SHELL_STATE.proofs_a.LINEN_KEYBOARD_ROUTE_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.PHYSICAL_KEYBOARD_TO_QUIL_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.BELL_APP_EVENT_INTEGRATION_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.BELL_WORKFLOW_EVENT_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.WINDOW_WORKFLOW_V2_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.BELL_KEYBOARD_DETAIL_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.BELL_DETAIL_SEED_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.BELL_SYSTEM_EVENTS_PROOF_DONE);
 assert(SILK_SHELL_STATE.atlas.ATLAS_SCENE_KEYBOARD_PROOF_DONE);
 assert(SILK_SHELL_STATE.atlas.ATLAS_THEME_VISUAL_PROOF_DONE);
 assert(SILK_SHELL_STATE.atlas.ATLAS_THEME_PRESETS_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.SILKBAR_KEYBOARD_STATUS_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.SILKBAR_PALETTE_STATUS_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.SILKBAR_PHASE2_SHELL_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.FRAME_LIGHTS_STUB_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.FRAME_LIGHTS_KEYBOARD_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.SILK_FOCUS_REJECT_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.SILK_DRAG_REJECT_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.SILK_WINDOW_MOVE_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.QUIL_VISIBLE_TYPING_E2E_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.LINEN_OBJECT_DETAIL_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.LINEN_NONBLOCKING_OPEN_PROOF_DONE);
 assert(SILK_SHELL_STATE.proofs_a.COLLAR_KEYBOARD_GRANTS_PROOF_DONE);
 assert(SILK_SHELL_STATE.models.BELL_RING_WRITE_INDEX==6);
 assert(focus_calls==3&&quil_calls==1&&pdx_calls==6&&bell_calls==8&&silkbar_calls==3&&atlas_calls>0&&close_calls==1&&hid_calls==7);
 maybe_run_linen_keyboard_route_proof();
 maybe_run_physical_keyboard_to_quil_focus_proof();
 maybe_run_bell_app_event_integration_proof();
 maybe_run_bell_workflow_event_proof();
 maybe_run_window_workflow_v2_proof();
 maybe_run_bell_keyboard_detail_proof();
 maybe_run_bell_detail_seed_proof();
 maybe_run_bell_system_events_proof();
 maybe_run_atlas_scene_keyboard_proof();
 maybe_run_atlas_theme_visual_proof();
 maybe_run_atlas_theme_presets_proof();
 maybe_run_silkbar_keyboard_status_proof();
 maybe_run_silkbar_palette_status_proof();
 maybe_run_silkbar_phase2_shell_proof();
 maybe_run_frame_lights_stub_proof();
 maybe_run_frame_lights_keyboard_proof();
 maybe_run_silk_focus_reject_proof();
 maybe_run_silk_drag_reject_proof();
 maybe_run_silk_window_move_proof();
 maybe_run_quil_visible_typing_e2e_proof();
 maybe_run_linen_object_detail_proof();
 maybe_run_linen_nonblocking_open_proof();
 maybe_run_collar_keyboard_grants_proof();
 assert(focus_calls==3&&quil_calls==1&&pdx_calls==6&&bell_calls==8&&bell_focus_calls==3&&silkbar_calls==3&&close_calls==1&&hid_calls==7);
 return 0;
}
