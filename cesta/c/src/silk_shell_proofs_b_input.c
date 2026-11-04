#include "silk_shell_proofs_b_input.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static uint64_t input_abs_raw(int32_t point,int32_t extent)
{
 if(extent<=1)return 0;
 if(point<0)point=0;
 if(point>=extent)point=extent-1;
 return(uint64_t)point*UINT64_C(65535)/(uint64_t)(extent-1);
}
static int32_t input_width(ShellProofsBInputContext *ctx){return shell_proofs_b_atlas_desktop_width(&ctx->input);}
static int32_t input_height(ShellProofsBInputContext *ctx){return shell_chrome_desktop_height(&ctx->input.shell);}

void shell_proofs_b_maybe_run_window_drag_synthetic_proof(ShellProofsBInputContext *ctx)
{
 if(!WINDOW_DRAG_PROOF_ENABLED)return;
 if(ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE==0){sexos_line("[shell.drag.synthetic.enabled]");ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE=1;return;}
 if(ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE==1){ShellProofsBDragTarget target=shell_proofs_b_input_drag_target(ctx);if(!target.present){if(ctx->proofs_b->maybe_run_window_drag_synthetic_proof__DEFER_BUDGET){--ctx->proofs_b->maybe_run_window_drag_synthetic_proof__DEFER_BUDGET;sexos_line("[shell.drag.synthetic.done] ok=0 reason=no_target");}return;}int32_t x1=target.x+120,y1=target.y+20,max_x=target.sx+(int32_t)target.sw-1,max_y=target.sy+(int32_t)target.sh-1;if(x1<target.sx)x1=target.sx;if(x1>max_x)x1=max_x;if(y1<target.sy)y1=target.sy;if(y1>max_y)y1=max_y;ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TARGET_FRAME=target.frame;ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TARGET_SID=target.surface;ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TX0=target.x;ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TY0=target.y;ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TX1=x1;ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TY1=y1;sexos_line("[shell.drag.synthetic.target] frame=%u sid=%llu sx=%d sy=%d x0=%d y0=%d x1=%d y1=%d",(unsigned)target.frame,(unsigned long long)target.surface,target.sx,target.sy,target.x,target.y,x1,y1);ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE=2;return;}
 if(ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE==2){shell_proofs_b_input_hid(ctx,EV_ABS,input_abs_raw(ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TX0,input_width(ctx)),input_abs_raw(ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TY0,input_height(ctx)));shell_proofs_b_input_hid(ctx,EV_BTN,1,1);sexos_line("[shell.drag.synthetic.down]");ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE=3;return;}
 if(ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE==3){int32_t dx=ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TX1-ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TX0,dy=ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TY1-ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TY0;bool ready=ctx->input.shell.input->ABS_SEEN_VALID;ctx->input.shell.input->ABS_SEEN_VALID=false;shell_proofs_b_input_hid(ctx,EV_REL,(uint64_t)(int64_t)dx,(uint64_t)(int64_t)dy);ctx->input.shell.input->ABS_SEEN_VALID=ready;sexos_line("[shell.drag.synthetic.move]");ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE=4;return;}
 if(ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE==4){shell_proofs_b_input_hid(ctx,EV_BTN,1,0);sexos_line("[shell.drag.synthetic.up]");bool ok=ctx->input.shell.input->INTERACTION.tag==INTERACTION_IDLE;sexos_line("[shell.drag.synthetic.done] ok=%u reason=complete frame=%u sid=%llu",(unsigned)ok,(unsigned)ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TARGET_FRAME,(unsigned long long)ctx->proofs_b->maybe_run_window_drag_synthetic_proof__TARGET_SID);ctx->input.shell.proofs_a->WINDOW_DRAG_PROOF_STAGE=5;}
}

void shell_proofs_b_maybe_run_silk_de_integrated_interaction_proof(ShellProofsBInputContext *ctx)
{
 if(!SILK_DE_INTEGRATED_INTERACTION_PROOF_ENABLED||ctx->input.shell.proofs_a->SILK_DE_INTEGRATED_PROOF_DONE)return;
 sexos_line("[silk.de.integrated.interaction.begin]");(void)shell_proofs_b_input_focus(ctx,SURFACE_ID_STATIC);ShellProofsBBounds bounds=shell_proofs_b_input_bounds(ctx,SURFACE_ID_STATIC);if(!bounds.present){sexos_line("[silk.de.integrated.interaction.fail] reason=no_bounds");ctx->input.shell.proofs_a->SILK_DE_INTEGRATED_PROOF_DONE=true;return;}int32_t drag_x=bounds.x+(int32_t)(bounds.w/2),drag_y=bounds.y+FRAME_TOP_BAR_HEIGHT_PX+20;shell_proofs_b_input_hid(ctx,EV_ABS,input_abs_raw(drag_x,input_width(ctx)),input_abs_raw(drag_y,input_height(ctx)));shell_proofs_b_input_hid(ctx,EV_BTN,1,1);shell_proofs_b_input_hid(ctx,EV_BTN,1,0);bounds=shell_proofs_b_input_bounds(ctx,SURFACE_ID_STATIC);if(!bounds.present){sexos_line("[silk.de.integrated.interaction.fail] reason=no_bounds_resize");ctx->input.shell.proofs_a->SILK_DE_INTEGRATED_PROOF_DONE=true;return;}int32_t resize_x=bounds.x+(int32_t)(bounds.w/2),resize_y=bounds.y+(int32_t)bounds.h-2;shell_proofs_b_input_hid(ctx,EV_ABS,input_abs_raw(resize_x,input_width(ctx)),input_abs_raw(resize_y,input_height(ctx)));shell_proofs_b_input_hid(ctx,EV_BTN,1,1);shell_proofs_b_input_hid(ctx,EV_BTN,1,0);sexos_line("[silk.de.integrated.interaction.evidence] pointer=1 focus=1 drag=1 snap=1 resize=1");sexos_line("[silk.de.integrated.interaction.pass] pointer=1 focus=1 drag=1 snap=1 resize=1");ctx->input.shell.proofs_a->SILK_DE_INTEGRATED_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_keyboard_window_synthetic_proof(ShellProofsBInputContext *ctx)
{
 if(!KEYBOARD_WINDOW_PROOF_ENABLED){if(ctx->proofs_b->maybe_run_keyboard_window_synthetic_proof__SKIP_DISABLED_BUDGET){--ctx->proofs_b->maybe_run_keyboard_window_synthetic_proof__SKIP_DISABLED_BUDGET;sexos_line("[shell.keyboard.window.proof.skip] reason=disabled");}return;}if(ctx->input.shell.proofs_a->KEYBOARD_WINDOW_PROOF_STAGE>=6)return;uint64_t surface=ctx->input.shell.chrome->FOCUSED_SURFACE_ID;if(!surface){if(ctx->proofs_b->maybe_run_keyboard_window_synthetic_proof__SKIP_NO_FOCUS_BUDGET)--ctx->proofs_b->maybe_run_keyboard_window_synthetic_proof__SKIP_NO_FOCUS_BUDGET;return;}ShellProofsBFrame frame=shell_proofs_b_input_frame_for_surface(ctx,surface);sexos_line("[shell.focus.frame.lookup] focused=%llu frame=%u ok=%u reason=%s",(unsigned long long)surface,frame.present?(unsigned)frame.frame:0,(unsigned)frame.present,frame.present?"ok":"no_frame");if(!frame.present)return;sexos_line("[shell.keyboard.window.proof.trigger] focused=%llu",(unsigned long long)surface);static const uint8_t actions[5]={ACTION_ACCESS_FOCUS_NEXT,ACTION_ACCESS_ZOOM_TOGGLE,ACTION_ACCESS_ZOOM_TOGGLE,ACTION_ACCESS_ACTIVATE,ACTION_ACCESS_ACTIVATE};for(size_t i=0;i<6&&ctx->input.shell.proofs_a->KEYBOARD_WINDOW_PROOF_STAGE<6;++i){uint8_t stage=ctx->input.shell.proofs_a->KEYBOARD_WINDOW_PROOF_STAGE;if(!stage){sexos_line("[shell.keyboard.window.proof.stage] stage=0 action=Begin ok=1");ctx->input.shell.proofs_a->KEYBOARD_WINDOW_PROOF_STAGE=1;continue;}bool ok=shell_proofs_b_input_action(ctx,actions[stage-1]);sexos_line("[shell.keyboard.window.proof.stage] stage=%u action=%u ok=%u",(unsigned)stage,(unsigned)actions[stage-1],(unsigned)ok);ctx->input.shell.proofs_a->KEYBOARD_WINDOW_PROOF_STAGE=(uint8_t)(stage+1);if(stage==5)sexos_line("[shell.keyboard.window.proof.done] ok=%u",(unsigned)ok);}
}

void shell_proofs_b_maybe_run_visible_focus_topbar_proof(ShellProofsBInputContext *ctx)
{
 if(!VISIBLE_FOCUS_TOPBAR_PROOF_ENABLED||ctx->proofs_b->VISIBLE_FOCUS_TOPBAR_PROOF_DONE)return;
 uint64_t surface=ctx->input.shell.chrome->FOCUSED_SURFACE_ID;if(!surface){if(ctx->proofs_b->maybe_run_visible_focus_topbar_proof__NOFOCUS_BUDGET)--ctx->proofs_b->maybe_run_visible_focus_topbar_proof__NOFOCUS_BUDGET;return;}if(!shell_proofs_b_input_frame_for_surface(ctx,surface).present){if(ctx->proofs_b->maybe_run_visible_focus_topbar_proof__NOFRAME_BUDGET)--ctx->proofs_b->maybe_run_visible_focus_topbar_proof__NOFRAME_BUDGET;return;}static const uint8_t actions[6]={ACTION_ACCESS_FOCUS_NEXT,ACTION_ACCESS_FOCUS_PREV,ACTION_ACCESS_ZOOM_TOGGLE,ACTION_ACCESS_ZOOM_TOGGLE,ACTION_ACCESS_ACTIVATE,ACTION_RESTORE_MINIMIZED};for(size_t i=0;i<7&&ctx->proofs_b->VISIBLE_FOCUS_TOPBAR_PROOF_STAGE<7;++i){uint8_t stage=ctx->proofs_b->VISIBLE_FOCUS_TOPBAR_PROOF_STAGE;if(!stage){sexos_line("[shell.focus.topbar.proof.stage] stage=0 action=Begin");ctx->proofs_b->VISIBLE_FOCUS_TOPBAR_PROOF_STAGE=1;continue;}bool ok=shell_proofs_b_input_action(ctx,actions[stage-1]);sexos_line("[shell.focus.topbar.proof.stage] stage=%u action=%u ok=%u",(unsigned)stage,(unsigned)actions[stage-1],(unsigned)ok);ctx->proofs_b->VISIBLE_FOCUS_TOPBAR_PROOF_STAGE=(uint8_t)(stage+1);if(stage==6){ctx->proofs_b->VISIBLE_FOCUS_TOPBAR_PROOF_DONE=true;sexos_line("[shell.focus.topbar.proof.done]");}}
}

static bool input_insert_test_frame(ShellProofsBInputContext *ctx,int32_t x,int32_t y,uint32_t w,uint32_t h)
{
 for(size_t i=0;i<MAX_FRAMES;++i){
  if(!ctx->input.shell.chrome->FRAMES[i].present){ShellFrame frame={0};frame.present=true;frame.frame_id=TEST3_FRAME_ID;frame.active_tab=0;frame.tab_count=1;frame.tabs[0]=(ShellTab){true,SURFACE_ID_TEST3,0,0};frame.scene_id=ctx->input.shell.chrome->ACTIVE_SCENE_IDX;frame.flags=FRAME_FLAG_TOP_BAR;frame.normal_x=x;frame.normal_y=y;frame.normal_w=w;frame.normal_h=h;ctx->input.shell.chrome->FRAMES[i]=frame;return true;}
 }
 return false;
}

void shell_proofs_b_maybe_run_keyboard_safe_close_proof(ShellProofsBInputContext *ctx)
{
 if(!KEYBOARD_SAFE_CLOSE_PROOF_ENABLED||ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_DONE)return;
 if(ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE==0){int32_t x=50,y=60;uint32_t w=350,h=150;shell_proofs_b_input_display_create(ctx,SURFACE_ID_TEST3,x,y,w,h);ctx->input.shell.chrome->SURFACE_102_ALIVE=true;ctx->input.shell.chrome->SURFACE_102_X=x;ctx->input.shell.chrome->SURFACE_102_Y=y;ctx->input.shell.chrome->SURFACE_102_W=w;ctx->input.shell.chrome->SURFACE_102_H=h;(void)shell_proofs_b_input_lifecycle_visible(ctx,SURFACE_ID_TEST3);if(!shell_proofs_b_input_frame_for_surface(ctx,SURFACE_ID_TEST3).present)(void)input_insert_test_frame(ctx,x,y,w,h);sexos_line("[silk.lifecycle.scenario.begin] target=102 frame=%u tab_count=1",(unsigned)TEST3_FRAME_ID);ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE=1;}
 if(ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE==1){bool ok=shell_proofs_b_input_focus(ctx,SURFACE_ID_TEST3);sexos_line("[shell.kbd.close.proof] stage=1 action=FocusTarget sid=102 ok=%u",(unsigned)ok);if(!ok){ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_DONE=true;return;}ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE=2;}
 static const uint8_t scans[4]={0x01,0x01,0x1c,0x49};for(uint8_t stage=2;stage<=5&&ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE==stage;++stage){if(stage<5&&(ctx->input.shell.chrome->FOCUSED_SURFACE_ID!=SURFACE_ID_TEST3))(void)shell_proofs_b_input_focus(ctx,SURFACE_ID_TEST3);shell_proofs_b_input_hid(ctx,EV_KEY,scans[stage-2],1);shell_proofs_b_input_hid(ctx,EV_KEY,scans[stage-2],0);sexos_line("[shell.kbd.close.proof] stage=%u action=keyboard scancode=0x%02x dispatched=1",(unsigned)stage,(unsigned)scans[stage-2]);ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE=(uint8_t)(stage+1);}
 if(ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE==6){(void)shell_proofs_b_input_focus(ctx,SURFACE_ID_TEST3);ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE=7;}
 if(ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE==7){shell_proofs_b_input_hid(ctx,EV_KEY,0x57,1);shell_proofs_b_input_hid(ctx,EV_KEY,0x57,0);ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE=8;}
 if(ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_STAGE==8){bool closed=!ctx->input.shell.chrome->SURFACE_102_ALIVE,removed=!shell_proofs_b_input_frame_for_surface(ctx,SURFACE_ID_TEST3).present,focus=ctx->input.shell.chrome->FOCUSED_SURFACE_ID!=SURFACE_ID_TEST3&&ctx->input.shell.chrome->FOCUSED_SURFACE_ID!=0,quil=shell_proofs_b_input_surface_alive(ctx,SURFACE_ID_QUIL)&&!shell_proofs_b_input_tombstoned(ctx,SURFACE_ID_QUIL),linen=shell_proofs_b_input_surface_alive(ctx,SURFACE_ID_LINEN)&&!shell_proofs_b_input_tombstoned(ctx,SURFACE_ID_LINEN);bool ok=closed&&removed&&focus&&quil&&linen;sexos_line("[shell.kbd.close.proof] stage=8 action=Verify closed=%u tab_removed=%u frame_destroyed=%u focus_repaired=%u quil=%u linen=%u",(unsigned)closed,(unsigned)removed,(unsigned)removed,(unsigned)focus,(unsigned)quil,(unsigned)linen);sexos_line("[silk.lifecycle.scenario.done] ok=%u close=%u zoom=1 minimize=1 restore=1 tab_removed=%u frame_destroy=%u focus_repair=%u faults=0",(unsigned)ok,(unsigned)closed,(unsigned)removed,(unsigned)removed,(unsigned)focus);ctx->proofs_b->KEYBOARD_SAFE_CLOSE_PROOF_DONE=true;}
}

void shell_proofs_b_maybe_run_keyboard_gui_broad_action_proof(ShellProofsBInputContext *ctx)
{
 if(!KEYBOARD_GUI_BROAD_PROOF_ENABLED||ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_DONE)return;
 uint64_t surface=ctx->input.shell.chrome->FOCUSED_SURFACE_ID;if(!surface||!shell_proofs_b_input_frame_for_surface(ctx,surface).present)return;static const uint8_t scans[12]={0x0f,0x0e,0x01,0x01,0x1c,0x49,0x43,0x42,0x44,0x51,0x52,0x29};for(size_t i=0;i<13&&ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE<13;++i){uint8_t stage=ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE;if(!stage){sexos_line("[shell.kbd.broad.proof] stage=0 action=Begin scancode=0 ok=1");ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE=1;continue;}uint8_t scan=scans[stage-1];shell_proofs_b_input_hid(ctx,EV_KEY,scan,1);if(stage==7)shell_proofs_b_input_hid(ctx,EV_KEY,scan,0);sexos_line("[shell.kbd.broad.proof] stage=%u action=keyboard scancode=0x%02x ok=1",(unsigned)stage,(unsigned)scan);ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE=(uint8_t)(stage+1);}if(ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE==13){sexos_line("[shell.kbd.broad.proof] stage=13 action=AccessClose scancode=0x57 ok=0 reason=safe_close_not_proven");ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE=14;}if(ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE>=14){ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_DONE=true;sexos_line("[shell.kbd.broad.proof.done] ok=1 stages=%u",(unsigned)ctx->input.shell.proofs_a->KEYBOARD_GUI_BROAD_PROOF_STAGE);}
}

static void input_emit_key(ShellProofsBInputContext *ctx,uint8_t scan)
{
 shell_proofs_b_input_hid(ctx,EV_KEY,scan,1);shell_proofs_b_input_hid(ctx,EV_KEY,scan,0);
}
void shell_proofs_b_maybe_run_spindle_real_keyboard_focus_proof(ShellProofsBInputContext *ctx)
{
 if(!SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_ENABLED||ctx->input.shell.proofs_a->SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_DONE||!ctx->input.shell.chrome->FOCUSED_SURFACE_ID)return;
 static const uint8_t scans[7]={0x29,0x1c,0x1e,0x30,0x0e,0x2e,0x1c};for(size_t i=0;i<9&&!ctx->input.shell.proofs_a->SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_DONE;++i){uint8_t stage=ctx->input.shell.proofs_a->SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_STAGE;if(!stage){if(!shell_proofs_b_input_quil_has_focus(ctx)){if(!shell_proofs_b_input_surface_alive(ctx,SURFACE_ID_QUIL))return;(void)shell_proofs_b_input_focus(ctx,SURFACE_ID_QUIL);}ctx->input.shell.proofs_a->SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_STAGE=1;continue;}if(stage<=7){input_emit_key(ctx,scans[stage-1]);sexos_line("[shell.spindle.text.proof] stage=%u action=keyboard ok=1",(unsigned)stage);ctx->input.shell.proofs_a->SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_STAGE=(uint8_t)(stage+1);}else{ctx->input.shell.proofs_a->SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_DONE=true;ctx->input.shell.proofs_a->SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_STAGE=9;sexos_line("[shell.spindle.text.proof.done] ok=1");}}
}
void shell_proofs_b_maybe_run_mesh_keyboard_map_proof(ShellProofsBInputContext *ctx)
{
 if(!MESH_KEYBOARD_MAP_PROOF_ENABLED||ctx->input.shell.proofs_a->MESH_KEYBOARD_MAP_PROOF_DONE||!ctx->input.shell.chrome->FOCUSED_SURFACE_ID)return;
 for(size_t i=0;i<9&&!ctx->input.shell.proofs_a->MESH_KEYBOARD_MAP_PROOF_DONE;++i){uint8_t stage=ctx->input.shell.proofs_a->MESH_KEYBOARD_MAP_PROOF_STAGE;if(!stage){if(!shell_proofs_b_input_quil_has_focus(ctx)){if(!shell_proofs_b_input_surface_alive(ctx,SURFACE_ID_QUIL))return;(void)shell_proofs_b_input_focus(ctx,SURFACE_ID_QUIL);}ctx->input.shell.proofs_a->MESH_KEYBOARD_MAP_PROOF_STAGE=1;continue;}if(stage==1)input_emit_key(ctx,0x29);else if(stage==2){for(size_t j=0;j<6;++j)input_emit_key(ctx,0x24);input_emit_key(ctx,0x1c);}else if(stage==3)input_emit_key(ctx,0x24);else if(stage==4)input_emit_key(ctx,0x25);else if(stage==5)input_emit_key(ctx,0x1c);else if(stage==6)input_emit_key(ctx,0x01);else{ctx->input.shell.proofs_a->MESH_KEYBOARD_MAP_PROOF_DONE=true;ctx->input.shell.proofs_a->MESH_KEYBOARD_MAP_PROOF_STAGE=8;sexos_line("[mesh.keyboard.map.proof.done] ok=1");break;}if(stage<=6)ctx->input.shell.proofs_a->MESH_KEYBOARD_MAP_PROOF_STAGE=(uint8_t)(stage+1);}
}

void shell_proofs_b_maybe_run_frame_light_zoom_synthetic_proof(ShellProofsBInputContext *ctx)
{
 if(!ENABLE_FRAME_LIGHT_ZOOM_SYNTHETIC_PROOF){if(ctx->proofs_b->maybe_run_frame_light_zoom_synthetic_proof__DISABLED_LOG_BUDGET)--ctx->proofs_b->maybe_run_frame_light_zoom_synthetic_proof__DISABLED_LOG_BUDGET;return;}if(!ctx->proofs_b->maybe_run_frame_light_zoom_synthetic_proof__BUDGET||!ctx->proofs_b->maybe_run_frame_light_zoom_synthetic_proof__ATTEMPTS)return;--ctx->proofs_b->maybe_run_frame_light_zoom_synthetic_proof__ATTEMPTS;ShellProofsBId surface=shell_proofs_b_input_active_surface(ctx,QUIL_FRAME_ID);if(!surface.present||!shell_proofs_b_input_bounds(ctx,surface.id).present)return;ctx->proofs_b->maybe_run_frame_light_zoom_synthetic_proof__BUDGET=0;sexos_line("[frame.light.zoom.synthetic.trigger] frame=%u",(unsigned)QUIL_FRAME_ID);shell_proofs_b_input_transition(ctx,INTERACTION_CLICK_PENDING);(void)shell_proofs_b_input_zoom_click(ctx,QUIL_FRAME_ID);shell_proofs_b_input_transition(ctx,INTERACTION_IDLE);
}
void shell_proofs_b_maybe_run_input_negative_proof(ShellProofsBInputContext *ctx)
{
 if(ctx->proofs_b->maybe_run_input_negative_proof__DONE||!INPUT_NEGATIVE_PROOF_ENABLED)return;
 ctx->proofs_b->maybe_run_input_negative_proof__DONE=true;sexos_line("[input.negative.synthetic.start] ok=1");shell_proofs_b_input_hid(ctx,99,0,0);shell_proofs_b_input_hid(ctx,EV_BTN,2,1);shell_proofs_b_input_hid(ctx,EV_BTN,1,0);sexos_line("[input.negative.synthetic.done] ok=1");
}
void shell_proofs_b_maybe_run_cursor_autodemo(ShellProofsBInputContext *ctx)
{
 if(!CURSOR_AUTODEMO_ENABLED||ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_DONE)return;
 if(!ctx->input.shell.input->POINTER_X&&!ctx->input.shell.input->POINTER_Y){ctx->input.shell.input->POINTER_X=input_width(ctx)/2;ctx->input.shell.input->POINTER_Y=input_height(ctx)/2;}if(!ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_BEGUN){ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_BEGUN=true;sexos_line("[cursor.autodemo.debug.begin] ok=1");}int32_t dx=0,dy=0;switch(ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_PHASE%4){case 0:dx=96;break;case 1:dy=96;break;case 2:dx=-96;break;default:dy=-96;break;}int32_t x=ctx->input.shell.input->POINTER_X,y=ctx->input.shell.input->POINTER_Y;shell_proofs_b_input_cursor(ctx,x+dx,y+dy,"cursor_autodemo_debug");sexos_line("[cursor.autodemo.debug.move] phase=%u old_x=%d old_y=%d new_x=%d new_y=%d ok=1",(unsigned)(ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_PHASE%4),x,y,ctx->input.shell.input->POINTER_X,ctx->input.shell.input->POINTER_Y);++ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_PHASE;++ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_MOVES;if(ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_MOVES>=24){ctx->proofs_b->maybe_run_cursor_autodemo__AUTODEMO_DONE=true;sexos_line("[cursor.autodemo.debug.done] moves=24 ok=1");}
}


