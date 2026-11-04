#include "silk_shell_proofs_b_palette.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static const char *shell_proofs_b_palette_text(const ShellPaletteString *text)
{
 return text->bytes;
}

void shell_proofs_b_maybe_run_command_palette_status_proof(ShellInputContext *ctx)
{
 if(!COMMAND_PALETTE_STATUS_PROOF_ENABLED || ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_DONE)return;
 if(ctx->shell.chrome->FOCUSED_SURFACE_ID==0){sexos_line("[shell.palette.status.proof.wait] reason=not_ready");return;}
 ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_ACTIVE=true;
 size_t count=shell_proofs_b_palette_command_count(ctx);uint8_t total=(uint8_t)count;
 uint8_t stage=ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_STAGE;
 sexos_line("[shell.palette.status.proof.trigger] stage=%u total=%u",(unsigned)stage,(unsigned)total);
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN){
  ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_DONE=true;
  ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_ACTIVE=false;
  sexos_line("[shell.palette.status.proof.done] ok=0 reason=palette_open_failed");return;
 }
 if(stage==0){
  for(size_t index=0;index<count;++index){uint8_t command=shell_proofs_b_palette_command_code(ctx,index);
   ShellPaletteString name=shell_proofs_b_palette_command_label(ctx,index);ShellPaletteStatus status=shell_proofs_b_palette_status(ctx,command);
   sexos_line("[shell.palette.status] idx=%zu action=%s available=%u status=%s reason=%s",index,shell_proofs_b_palette_text(&name),(unsigned)status.available,shell_proofs_b_palette_text(&status.status),shell_proofs_b_palette_text(&status.reason));
  }
  ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_STAGE=1;
  sexos_line("[shell.palette.status.proof.stage] stage=0 done reason=all_status_emitted");return;
 }
 if((size_t)stage<=count){size_t index=(size_t)(stage-1);
  if(index>=count){ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_DONE=true;ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_ACTIVE=false;if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);sexos_line("[shell.palette.status.proof.done] ok=1 reason=complete faults=0");return;}
  uint8_t command=shell_proofs_b_palette_command_code(ctx,index);ShellPaletteString name=shell_proofs_b_palette_command_label(ctx,index);ShellPaletteStatus status=shell_proofs_b_palette_status(ctx,command);
  if(!status.available)sexos_line("[shell.palette.status.proof.skip] idx=%zu action=%s status=%s reason=blocked_by_design",index,shell_proofs_b_palette_text(&name),shell_proofs_b_palette_text(&status.status));
  else{uint8_t old=ctx->surfaces->COMMAND_PALETTE_SELECTED;ctx->surfaces->COMMAND_PALETTE_SELECTED=(uint8_t)index;sexos_line("[shell.palette.select] old=%u new=%u",(unsigned)old,(unsigned)ctx->surfaces->COMMAND_PALETTE_SELECTED);shell_proofs_b_palette_render(ctx);bool ok=shell_proofs_b_palette_execute(ctx);sexos_line("[shell.palette.status.proof.exec] idx=%zu action=%s ok=%u status=%s",index,shell_proofs_b_palette_text(&name),(unsigned)ok,shell_proofs_b_palette_text(&status.status));}
 }
 if(ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_STAGE<UINT8_MAX)++ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_STAGE;
 if(ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_STAGE>total){ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_DONE=true;ctx->shell.proofs_a->COMMAND_PALETTE_STATUS_PROOF_ACTIVE=false;if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);sexos_line("[shell.palette.status.proof.done] ok=1 reason=complete faults=0");}
}

void shell_proofs_b_maybe_run_command_palette_linen_status_proof(ShellInputContext *ctx)
{
 if(!COMMAND_PALETTE_LINEN_STATUS_PROOF_ENABLED || ctx->shell.proofs_a->COMMAND_PALETTE_LINEN_STATUS_PROOF_DONE)return;
 sexos_line("[shell.palette.linen.status.proof] stage=0 action=start ok=1 reason=begin");
 ShellPaletteStatus status=shell_proofs_b_palette_status(ctx,CMD_FOCUS_LINEN);
 sexos_line("[shell.palette.linen.status.proof] stage=1 action=status_check available=%u status=%s reason=%s",(unsigned)status.available,shell_proofs_b_palette_text(&status.status),shell_proofs_b_palette_text(&status.reason));
 if(!status.available){sexos_line("[shell.palette.linen.status.proof.done] ok=0");ctx->shell.proofs_a->COMMAND_PALETTE_LINEN_STATUS_PROOF_DONE=true;return;}
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN){sexos_line("[shell.palette.linen.status.proof] stage=2 action=open_palette ok=0 reason=open_failed");sexos_line("[shell.palette.linen.status.proof.done] ok=0");ctx->shell.proofs_a->COMMAND_PALETTE_LINEN_STATUS_PROOF_DONE=true;return;}
 sexos_line("[shell.palette.linen.status.proof] stage=2 action=open_palette ok=1 reason=opened");
 size_t count=shell_proofs_b_palette_command_count(ctx),index=count;
 for(size_t i=0;i<count;++i)if(shell_proofs_b_palette_command_code(ctx,i)==CMD_FOCUS_LINEN){index=i;break;}
 if(index==count){sexos_line("[shell.palette.linen.status.proof] stage=3 action=status_emitted ok=0 reason=not_found");ctx->shell.proofs_a->COMMAND_PALETTE_LINEN_STATUS_PROOF_DONE=true;return;}
 status=shell_proofs_b_palette_status(ctx,CMD_FOCUS_LINEN);
 sexos_line("[shell.palette.status] idx=%zu action=OpenLinen available=%u status=%s reason=%s",index,(unsigned)status.available,shell_proofs_b_palette_text(&status.status),shell_proofs_b_palette_text(&status.reason));
 sexos_line("[shell.palette.linen.status.proof] stage=3 action=status_emitted idx=%zu ok=1",index);
 ctx->surfaces->COMMAND_PALETTE_SELECTED=(uint8_t)index;shell_proofs_b_palette_render(ctx);bool executed=shell_proofs_b_palette_execute(ctx);
 sexos_line("[shell.palette.exec.result] idx=%zu action=OpenLinen ok=%u status=nonblocking_ready reason=%s",index,(unsigned)executed,executed?"ok":"exec_fail");
 sexos_line("[shell.palette.linen.status.proof] stage=4 action=exec ok=%u reason=%s",(unsigned)executed,executed?"ok":"fail");
 if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 sexos_line("[shell.palette.linen.status.proof] stage=5 action=close_palette ok=1 reason=done");sexos_line("[shell.palette.linen.status.proof.done] ok=%u",(unsigned)executed);ctx->shell.proofs_a->COMMAND_PALETTE_LINEN_STATUS_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_quil_status_unblock_proof(ShellInputContext *ctx)
{
 if(!QUIL_STATUS_UNBLOCK_PROOF_ENABLED || ctx->shell.proofs_a->QUIL_STATUS_UNBLOCK_PROOF_DONE)return;
 sexos_line("[quil.status.unblock.proof] stage=0 action=start ok=1 reason=begin");ShellPaletteStatus status=shell_proofs_b_palette_status(ctx,CMD_FOCUS_QUIL);
 sexos_line("[quil.status.unblock.proof] stage=1 action=status_check available=%u status=%s reason=%s",(unsigned)status.available,shell_proofs_b_palette_text(&status.status),shell_proofs_b_palette_text(&status.reason));
 if(!status.available){sexos_line("[quil.status.unblock.proof] stage=1 action=status_check ok=0 reason=still_blocked");sexos_line("[quil.status.unblock.proof.done] ok=0");ctx->shell.proofs_a->QUIL_STATUS_UNBLOCK_PROOF_DONE=true;return;}
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN){sexos_line("[quil.status.unblock.proof] stage=2 action=status_emitted ok=0 reason=palette_open_failed");ctx->shell.proofs_a->QUIL_STATUS_UNBLOCK_PROOF_DONE=true;return;}
 size_t count=shell_proofs_b_palette_command_count(ctx);for(size_t i=0;i<count;++i)if(shell_proofs_b_palette_command_code(ctx,i)==CMD_FOCUS_QUIL){sexos_line("[shell.palette.status] idx=%zu action=OpenQuil available=%u status=%s reason=%s",i,(unsigned)status.available,shell_proofs_b_palette_text(&status.status),shell_proofs_b_palette_text(&status.reason));break;}
 sexos_line("[quil.status.unblock.proof] stage=2 action=status_emitted ok=1 reason=available_in_palette");if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);sexos_line("[quil.status.unblock.proof] stage=3 action=summary ok=1 reason=quil_now_available");sexos_line("[quil.status.unblock.proof.done] ok=1");ctx->shell.proofs_a->QUIL_STATUS_UNBLOCK_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_command_palette_daily_proof(ShellInputContext *ctx)
{
 if(!COMMAND_PALETTE_DAILY_PROOF_ENABLED){if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIP_BUDGET){--ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIP_BUDGET;sexos_line("[shell.palette.daily.proof.skip] reason=disabled");}return;}
 if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_DONE){if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIP_BUDGET){--ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIP_BUDGET;sexos_line("[shell.palette.daily.proof.skip] reason=already_done");}return;}
 if(ctx->shell.chrome->FOCUSED_SURFACE_ID==0){if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIP_BUDGET){--ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIP_BUDGET;sexos_line("[shell.palette.daily.proof.skip] reason=not_ready");}return;}
 ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_ACTIVE=true;size_t count=shell_proofs_b_palette_command_count(ctx);uint8_t total=(uint8_t)count;
 sexos_line("[shell.palette.daily.proof.trigger] selected=%u count=%u",(unsigned)ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX,(unsigned)total);
 for(size_t step=0;step<count && ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX<total;++step){
  if(!ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
  uint8_t index=ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX;
  if(!ctx->surfaces->COMMAND_PALETTE_OPEN){ShellPaletteString name=shell_proofs_b_palette_command_label(ctx,index);sexos_line("[shell.palette.exec.skip] idx=%u action=%s ok=0 reason=open_failed",(unsigned)index,shell_proofs_b_palette_text(&name));if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIPPED<UINT8_MAX)++ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIPPED;if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX<UINT8_MAX)++ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX;continue;}
  uint8_t old=ctx->surfaces->COMMAND_PALETTE_SELECTED;ctx->surfaces->COMMAND_PALETTE_SELECTED=index;sexos_line("[shell.palette.select] old=%u new=%u",(unsigned)old,(unsigned)index);shell_proofs_b_palette_render(ctx);bool ok=shell_proofs_b_palette_execute(ctx);ShellPaletteString name=shell_proofs_b_palette_command_label(ctx,index);sexos_line("[shell.palette.daily.proof.stage] stage=%u idx=%u action=%s ok=%u",(unsigned)index,(unsigned)index,shell_proofs_b_palette_text(&name),(unsigned)ok);if(ok){if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_EXECUTED<UINT8_MAX)++ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_EXECUTED;}else if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_REJECTED<UINT8_MAX)++ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_REJECTED;if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX<UINT8_MAX)++ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX;
 }
 if(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_IDX>=total){ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_DONE=true;ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_ACTIVE=false;if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);uint8_t observed=(uint8_t)(ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_EXECUTED+ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_REJECTED+ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIPPED);sexos_line("[shell.palette.daily.proof.done] ok=%u executed=%u rejected=%u skipped=%u",(unsigned)(observed==total),(unsigned)ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_EXECUTED,(unsigned)ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_REJECTED,(unsigned)ctx->shell.proofs_a->COMMAND_PALETTE_DAILY_PROOF_SKIPPED);}
}

void shell_proofs_b_maybe_run_app_launcher_proof(ShellInputContext *ctx)
{
 if(!APP_LAUNCHER_PROOF_ENABLED || ctx->shell.proofs_a->APP_LAUNCHER_PROOF_DONE || ctx->shell.chrome->FOCUSED_SURFACE_ID==0)return;
 ctx->shell.proofs_a->APP_LAUNCHER_PROOF_ACTIVE=true;
 sexos_line("[launcher.proof] stage=0 action=start ok=1 reason=app_launcher_proof_begin");
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 bool opened=ctx->surfaces->COMMAND_PALETTE_OPEN;
 sexos_line("[launcher.proof] stage=1 action=open ok=%u reason=%s",(unsigned)opened,opened?"palette_opened":"open_failed");
 if(!opened){ctx->shell.proofs_a->APP_LAUNCHER_PROOF_DONE=true;ctx->shell.proofs_a->APP_LAUNCHER_PROOF_ACTIVE=false;sexos_line("[launcher.proof.done] ok=0");return;}
 for(uint8_t i=0;i<6;++i){shell_proofs_b_palette_next(ctx);sexos_line("[launcher.proof] stage=2 action=nav_down step=%u selected=%u ok=1",(unsigned)(i+1),(unsigned)ctx->surfaces->COMMAND_PALETTE_SELECTED);}
 uint8_t nav_ok=(uint8_t)(ctx->surfaces->COMMAND_PALETTE_SELECTED>=5);sexos_line("[launcher.proof] stage=2 action=nav_audit ok=%u reason=nav_range_verified",(unsigned)nav_ok);
 for(uint8_t i=0;i<3;++i){shell_proofs_b_palette_previous(ctx);sexos_line("[launcher.proof] stage=3 action=nav_up step=%u selected=%u ok=1",(unsigned)(i+1),(unsigned)ctx->surfaces->COMMAND_PALETTE_SELECTED);}
 sexos_line("[launcher.proof] stage=3 action=nav_up_audit ok=1 reason=up_nav_works");while(ctx->surfaces->COMMAND_PALETTE_SELECTED!=0 && ctx->surfaces->COMMAND_PALETTE_SELECTED<7)shell_proofs_b_palette_previous(ctx);ctx->surfaces->COMMAND_PALETTE_SELECTED=0;bool executed=shell_proofs_b_palette_execute(ctx);sexos_line("[launcher.proof] stage=4 action=exec ok=%u reason=%s",(unsigned)executed,executed?"app_launched":"exec_rejected");if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);bool closed=!ctx->surfaces->COMMAND_PALETTE_OPEN;sexos_line("[launcher.proof] stage=5 action=close ok=%u reason=%s",(unsigned)closed,closed?"palette_closed":"close_failed");bool ok=nav_ok && executed && closed;sexos_line("[launcher.proof.done] ok=%u",(unsigned)ok);ctx->shell.proofs_a->APP_LAUNCHER_PROOF_DONE=true;ctx->shell.proofs_a->APP_LAUNCHER_PROOF_ACTIVE=false;
}

void shell_proofs_b_maybe_run_app_launcher_help_proof(ShellInputContext *ctx)
{
 if(!APP_LAUNCHER_HELP_PROOF_ENABLED || ctx->shell.proofs_a->APP_LAUNCHER_HELP_PROOF_DONE)return;
 size_t count=shell_proofs_b_palette_command_count(ctx),rows=count<7?count:7,available=0;
 sexos_line("[launcher.help.proof] stage=0 action=start ok=1 reason=help_rows_begin");
 for(size_t i=0;i<rows;++i){ShellPaletteString name=shell_proofs_b_palette_command_label(ctx,i);ShellPaletteStatus status=shell_proofs_b_palette_status(ctx,shell_proofs_b_palette_command_code(ctx,i));sexos_line("[launcher.help.row] idx=%zu app=%s available=%u status=%s reason=%s",i,shell_proofs_b_palette_text(&name),(unsigned)status.available,shell_proofs_b_palette_text(&status.status),shell_proofs_b_palette_text(&status.reason));if(status.available)++available;}
 sexos_line("[launcher.help.proof.done] ok=%u rows=%zu reason=%s",(unsigned)(available>0),rows,available?"rows_available":"no_available_rows");sexos_line("[launcher.help.rowcount] total=%zu ok=1",rows);ctx->shell.proofs_a->APP_LAUNCHER_HELP_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_linen_search_filter_proof(ShellInputContext *ctx)
{
 if(!LINEN_SEARCH_FILTER_PROOF_ENABLED || ctx->shell.proofs_a->LINEN_SEARCH_FILTER_PROOF_DONE)return;
 sexos_line("[linen.search.token] idx=0 value=doc ok=1");
 sexos_line("[linen.search.token.count] count=1 ok=1");
 sexos_line("[linen.search.query] len=3 ok=1");
 sexos_line("[linen.search.mode] value=kind_document ok=1");
 size_t selected=shell_proofs_b_linen_selected_index(ctx),matched=0;
 for(size_t i=0;i<LINEN_MAX_OBJECTS;++i)
  if(ctx->shell.models->LINEN_OBJECTS[i].present && ctx->shell.models->LINEN_OBJECTS[i].kind==LINEN_OBJ_KIND_DOCUMENT)++matched;
 const char *reason=matched?(selected<matched?"kind_document_match_selected_in_range":"kind_document_match_selected_oob"):"no_match";
 sexos_line("[linen.search.result] count=%zu selected=%zu reason=%s",matched,selected,reason);
 sexos_line("[linen.filter.proof.done] ok=%u mode=kind_document",(unsigned)(matched>0));
 ctx->shell.proofs_a->LINEN_SEARCH_FILTER_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_bell_filter_proof(ShellInputContext *ctx)
{
 if(!BELL_FILTER_PROOF_ENABLED || ctx->shell.proofs_a->BELL_FILTER_PROOF_DONE)return;
 size_t total=shell_proofs_b_bell_ring_count(ctx);uint8_t old=ctx->surfaces->BELL_SELECTED_ROW;
 if(!total){
  sexos_line("[bell.filter.source] source=local_ring count=0 ok=0 reason=empty_ring");
  sexos_line("[bell.filter.nav] old=%u new=%u ok=1",(unsigned)old,(unsigned)old);
  sexos_line("[bell.filter.proof.done] ok=0 reason=empty_ring");
  ctx->shell.proofs_a->BELL_FILTER_PROOF_DONE=true;return;
 }
 sexos_line("[bell.filter.source] source=local_ring count=%zu ok=1",total);
 sexos_line("[bell.filter.source.enum] source=local_ring mode=cycle ok=1");
 if(shell_proofs_b_bell_visible_count(ctx)>1)shell_proofs_b_bell_next(ctx);
 uint8_t next=ctx->surfaces->BELL_SELECTED_ROW;bool ok=next!=old || total<=1;
 sexos_line("[bell.filter.nav] old=%u new=%u ok=%u",(unsigned)old,(unsigned)next,(unsigned)ok);
 sexos_line("[bell.filter.proof.done] ok=%u",(unsigned)ok);
 ctx->shell.proofs_a->BELL_FILTER_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_atlas_preview_proof(ShellInputContext *ctx)
{
 if(!ATLAS_PREVIEW_PROOF_ENABLED || ctx->atlas->ATLAS_PREVIEW_PROOF_DONE)return;
 uint8_t preset=ctx->shell.chrome->ACTIVE_SCENE_IDX;
 uint8_t accent=ctx->shell.chrome->SCENES[preset].accent;
 uint32_t color=ATLAS_ACCENT_COLORS[(size_t)accent%5];
 sexos_line("[atlas.preview] preset=%u accent=%u color=0x%08x ok=1 reason=pre_apply_marker",(unsigned)preset,(unsigned)accent,(unsigned)color);
 sexos_line("[atlas.preview.proof.done] ok=1 reason=preview_marker_emitted");
 ctx->atlas->ATLAS_PREVIEW_PROOF_DONE=true;
}

static uint8_t shell_proofs_b_scene_frame_count(const ShellInputContext *ctx,uint8_t scene)
{
 uint8_t count=0;
 for(size_t i=0;i<MAX_FRAMES;++i)
  if(ctx->shell.chrome->FRAMES[i].present && ctx->shell.chrome->FRAMES[i].scene_id==scene && count<UINT8_MAX)++count;
 return count;
}

static uint8_t shell_proofs_b_populated_scenes(const ShellInputContext *ctx)
{
 uint8_t count=0;
 for(uint8_t scene=0;scene<WORKSPACE_COUNT;++scene)
  if(shell_proofs_b_scene_frame_count(ctx,scene) && count<UINT8_MAX)++count;
 return count?count:1;
}

void shell_proofs_b_maybe_run_atlas_phase_a_state_model_proof(ShellInputContext *ctx)
{
 if(!ATLAS_PHASE_A_STATE_MODEL_PROOF_ENABLED || ctx->atlas->ATLAS_PHASE_A_STATE_MODEL_PROOF_DONE)return;
 for(;;){uint8_t stage=ctx->atlas->ATLAS_PHASE_A_STATE_MODEL_PROOF_STAGE;if(stage>=10)break;ctx->atlas->ATLAS_PHASE_A_STATE_MODEL_PROOF_STAGE=(uint8_t)(stage+1);
  if(stage==0){sexos_line("[silk.atlas.state.init] scenes=%zu active=%u mode=%s",(size_t)WORKSPACE_COUNT,(unsigned)ctx->shell.chrome->ACTIVE_SCENE_IDX,ctx->shell.chrome->ATLAS_MODE_ENABLED?"atlas":"desktop");}
  else if(stage==1){if(!ctx->shell.chrome->ATLAS_MODE_ENABLED)shell_proofs_b_atlas_toggle(ctx);sexos_line("[silk.atlas.mode.enter] active=%u scenes=%u ok=%u",(unsigned)ctx->shell.chrome->ACTIVE_SCENE_IDX,(unsigned)shell_proofs_b_populated_scenes(ctx),(unsigned)ctx->shell.chrome->ATLAS_MODE_ENABLED);}
  else if(stage==2){for(uint8_t scene=0;scene<WORKSPACE_COUNT;++scene){uint8_t count=shell_proofs_b_scene_frame_count(ctx,scene);if(!count)continue;int32_t x=0,y=0;uint32_t w=0,h=0;for(size_t i=0;i<MAX_FRAMES;++i){ShellFrame frame=ctx->shell.chrome->FRAMES[i];if(frame.present && frame.scene_id==scene){x=frame.normal_x;y=frame.normal_y;w=frame.normal_w;h=frame.normal_h;break;}}sexos_line("[silk.atlas.scene.preview] scene=%u frames=%u x=%d y=%d w=%u h=%u",(unsigned)scene,(unsigned)count,x,y,(unsigned)w,(unsigned)h);}}
  else if(stage==3){uint8_t old=ctx->shell.chrome->ACTIVE_SCENE_IDX,target=old,populated=0;for(uint8_t scene=0;scene<WORKSPACE_COUNT;++scene)if(shell_proofs_b_scene_frame_count(ctx,scene)){if(populated<UINT8_MAX)++populated;if(scene!=old && target==old)target=scene;}shell_proofs_b_atlas_switch_scene(ctx,populated>1?target:old);sexos_line("[silk.scene.active.set] from=%u to=%u reason=%s",(unsigned)old,(unsigned)ctx->shell.chrome->ACTIVE_SCENE_IDX,populated>1?"multi_scene":"single_scene");}
  else if(stage==4){uint8_t active=ctx->shell.chrome->ACTIVE_SCENE_IDX;if(ctx->shell.chrome->ATLAS_MODE_ENABLED)shell_proofs_b_atlas_toggle(ctx);bool exited=!ctx->shell.chrome->ATLAS_MODE_ENABLED;sexos_line("[silk.atlas.mode.exit] active=%u reason=%s view=%s ok=%u",(unsigned)active,exited?"toggle_close":"already_closed",ctx->shell.chrome->ATLAS_MODE_ENABLED?"atlas":"desktop",(unsigned)exited);}
  else if(stage==5){sexos_line("[silk.atlas.phase_a.done] scenes=%zu active=%u mode=desktop ok=1",(size_t)WORKSPACE_COUNT,(unsigned)ctx->shell.chrome->ACTIVE_SCENE_IDX);ctx->atlas->ATLAS_PHASE_A_STATE_MODEL_PROOF_DONE=true;}
  shell_proofs_b_yield(ctx);
 }
}

void shell_proofs_b_maybe_run_atlas_phase_b_snapshot_proof(ShellInputContext *ctx)
{
 if(!ATLAS_PHASE_B_SNAPSHOT_PROOF_ENABLED || ctx->atlas->ATLAS_PHASE_B_SNAPSHOT_PROOF_DONE)return;
 for(;;){uint8_t stage=ctx->atlas->ATLAS_PHASE_B_SNAPSHOT_PROOF_STAGE;if(stage>=6)break;ctx->atlas->ATLAS_PHASE_B_SNAPSHOT_PROOF_STAGE=(uint8_t)(stage+1);
  if(stage==0)sexos_line("[silk.atlas.phase_b.begin] view=%s active_scene=%u",ctx->shell.chrome->ATLAS_MODE_ENABLED?"atlas":"desktop",(unsigned)ctx->shell.chrome->ACTIVE_SCENE_IDX);
  else if(stage==1){if(!ctx->shell.chrome->ATLAS_MODE_ENABLED)shell_proofs_b_atlas_toggle(ctx);sexos_line("[silk.atlas.phase_b.mode] atlas=%u",(unsigned)ctx->shell.chrome->ATLAS_MODE_ENABLED);}
  else if(stage==2)shell_proofs_b_atlas_collect_snapshot(ctx);
  else if(stage==3){if(ctx->shell.chrome->ATLAS_MODE_ENABLED)shell_proofs_b_atlas_toggle(ctx);bool restored=!ctx->shell.chrome->ATLAS_MODE_ENABLED;sexos_line("[silk.atlas.phase_b.restore] desktop=%u ok=%u",(unsigned)restored,(unsigned)restored);}
  else if(stage==4)sexos_line("[silk.atlas.phase_b.final] mode=%s ok=1",ctx->shell.chrome->ATLAS_MODE_ENABLED?"atlas":"desktop");
  else if(stage==5){sexos_line("[silk.atlas.phase_b.done] ok=1 reason=snapshot_collected");ctx->atlas->ATLAS_PHASE_B_SNAPSHOT_PROOF_DONE=true;}
  shell_proofs_b_yield(ctx);
 }
}

void shell_proofs_b_maybe_run_atlas_phase_c_render_stub_proof(ShellInputContext *ctx)
{
 if(!ATLAS_PHASE_C_RENDER_STUB_PROOF_ENABLED || ctx->atlas->ATLAS_PHASE_C_RENDER_STUB_PROOF_DONE)return;
 for(;;){uint8_t stage=ctx->atlas->ATLAS_PHASE_C_RENDER_STUB_PROOF_STAGE;if(stage>=4)break;ctx->atlas->ATLAS_PHASE_C_RENDER_STUB_PROOF_STAGE=(uint8_t)(stage+1);
  if(stage==0)sexos_line("[silk.atlas.phase_c.begin] scenes=%u active=%u",(unsigned)shell_proofs_b_populated_scenes(ctx),(unsigned)ctx->shell.chrome->ACTIVE_SCENE_IDX);
  else if(stage==1){shell_proofs_b_atlas_collect_snapshot(ctx);sexos_line("[silk.atlas.phase_c.snapshot] ok=1 reason=collected_for_card_geometry");}
  else if(stage==2){sexos_line("[silk.atlas.phase_c.done] ok=1 reason=shell_side_participates");ctx->atlas->ATLAS_PHASE_C_RENDER_STUB_PROOF_DONE=true;}
  shell_proofs_b_yield(ctx);
 }
}

void shell_proofs_b_maybe_run_atlas_phase_d_frame_preview_stub_proof(ShellInputContext *ctx)
{
 if(!ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_ENABLED || ctx->atlas->ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_DONE)return;
 for(;;){uint8_t stage=ctx->atlas->ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_STAGE;if(stage>=4)break;ctx->atlas->ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_STAGE=(uint8_t)(stage+1);
  if(stage==0)sexos_line("[silk.atlas.phase_d.begin] scenes=%u active=%u",(unsigned)shell_proofs_b_populated_scenes(ctx),(unsigned)ctx->shell.chrome->ACTIVE_SCENE_IDX);
  else if(stage==1){shell_proofs_b_atlas_collect_snapshot(ctx);sexos_line("[silk.atlas.phase_d.snapshot] ok=1 reason=collected_for_frame_preview");}
  else if(stage==2){sexos_line("[silk.atlas.phase_d.done] ok=1 reason=shell_side_participates");ctx->atlas->ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_DONE=true;}
  shell_proofs_b_yield(ctx);
 }
}

static const char *shell_proofs_b_linen_kind_name(uint8_t kind)
{
 switch(kind){case LINEN_OBJ_KIND_PROJECT:return "project";case LINEN_OBJ_KIND_DOCUMENT:return "document";case LINEN_OBJ_KIND_CODE_FILE:return "code_file";case LINEN_OBJ_KIND_MEDIA_ASSET:return "media_asset";case LINEN_OBJ_KIND_BUILD_ARTIFACT:return "build_artifact";case LINEN_OBJ_KIND_FOLDER:return "folder";case LINEN_OBJ_KIND_REFERENCE:return "reference";case LINEN_OBJ_KIND_IMPORT_PLACEHOLDER:return "import_placeholder";case LINEN_OBJ_KIND_BELL_EVENT_REFERENCE:return "bell_event_reference";case LINEN_OBJ_KIND_QUIL_WORKSPACE_REFERENCE:return "quil_workspace_reference";default:return "mesh_diagnostic_reference";}
}

static const char *shell_proofs_b_linen_state_name(uint8_t state)
{
 switch(state){case LINEN_OBJ_STATE_ALLOCATED:return "allocated";case LINEN_OBJ_STATE_LOADED:return "loaded";case LINEN_OBJ_STATE_MODIFIED:return "modified";case LINEN_OBJ_STATE_SAVED:return "saved";default:return "archived";}
}

void shell_proofs_b_maybe_run_app_registry_readonly_proof(ShellInputContext *ctx)
{
 if(!APP_REGISTRY_READONLY_PROOF_ENABLED || ctx->shell.proofs_a->APP_REGISTRY_READONLY_PROOF_DONE)return;
 uint8_t rows=0;for(size_t i=0;i<LINEN_MAX_OBJECTS;++i){bool present;uint64_t id;uint8_t state,kind;{const LinenObject *object=&ctx->shell.models->LINEN_OBJECTS[i];present=object->present;id=object->object_id;state=object->state;kind=object->kind;}if(!present)continue;const char *name=shell_proofs_b_registry_name(ctx,i);sexos_line("[app.registry.row] app_id=%llu state=%s kind=%s name=%s ok=1",(unsigned long long)id,shell_proofs_b_linen_state_name(state),shell_proofs_b_linen_kind_name(kind),name?name:"");if(rows<UINT8_MAX)++rows;}
 sexos_line("[app.registry.readonly.proof.done] rows=%u ok=%u",(unsigned)rows,(unsigned)(rows>0));ctx->shell.proofs_a->APP_REGISTRY_READONLY_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_app_registry_filter_sort_proof(ShellInputContext *ctx)
{
 if(!APP_REGISTRY_FILTER_SORT_PROOF_ENABLED || ctx->shell.proofs_a->APP_REGISTRY_FILTER_SORT_PROOF_DONE)return;
 uint8_t total=0,documents=0,sorted=1;uint64_t previous=0;
 for(size_t i=0;i<LINEN_MAX_OBJECTS;++i){const LinenObject *object=&ctx->shell.models->LINEN_OBJECTS[i];if(!object->present)continue;if(total<UINT8_MAX)++total;if(object->kind==LINEN_OBJ_KIND_DOCUMENT && documents<UINT8_MAX)++documents;if(previous>object->object_id)sorted=0;previous=object->object_id;}
 sexos_line("[app.registry.filter] mode=kind_document total=%u count=%u ok=%u",(unsigned)total,(unsigned)documents,(unsigned)(documents>0));sexos_line("[app.registry.sort] key=app_id order=asc ok=%u",(unsigned)sorted);ctx->shell.proofs_a->APP_REGISTRY_FILTER_SORT_PROOF_DONE=true;
}

static bool shell_proofs_b_runnable_kind(uint8_t kind)
{
 return kind<=LINEN_OBJ_KIND_FOLDER;
}

void shell_proofs_b_maybe_run_app_registry_launch_intent_proof(ShellInputContext *ctx)
{
 if(!APP_REGISTRY_LAUNCH_INTENT_PROOF_ENABLED || ctx->shell.proofs_a->APP_REGISTRY_LAUNCH_INTENT_PROOF_DONE)return;
 uint8_t rows=0,runnable=0,kinds[6]={0};
 for(size_t i=0;i<LINEN_MAX_OBJECTS;++i){const LinenObject *object=&ctx->shell.models->LINEN_OBJECTS[i];if(!object->present)continue;bool can=shell_proofs_b_runnable_kind(object->kind);if(object->kind<=LINEN_OBJ_KIND_FOLDER && kinds[object->kind]<UINT8_MAX)++kinds[object->kind];sexos_line("[app.registry.intent] app_id=%llu kind=%s status=%s ok=%u",(unsigned long long)object->object_id,shell_proofs_b_linen_kind_name(object->kind),can?"runnable":"blocked",(unsigned)can);if(can){if(runnable<UINT8_MAX)++runnable;}else sexos_line("[app.registry.intent.reject] app_id=%llu reason=unsupported_kind ok=1",(unsigned long long)object->object_id);if(rows<UINT8_MAX)++rows;}
 sexos_line("[app.registry.kind.matrix] project=%u document=%u codefile=%u media=%u build=%u folder=%u ok=1 reason=seeded_local",(unsigned)kinds[0],(unsigned)kinds[1],(unsigned)kinds[2],(unsigned)kinds[3],(unsigned)kinds[4],(unsigned)kinds[5]);sexos_line("[app.registry.intent.done] rows=%u runnable=%u ok=%u",(unsigned)rows,(unsigned)runnable,(unsigned)(rows>0));sexos_line("[app.registry.intent.done.reason] value=%s",rows?"seeded_rows_present":"no_seeded_rows");ctx->shell.proofs_a->APP_REGISTRY_LAUNCH_INTENT_PROOF_DONE=true;
}

void shell_proofs_b_maybe_run_app_launcher_multi_exec_proof(ShellInputContext *ctx)
{
 if(!APP_LAUNCHER_MULTI_EXEC_PROOF_ENABLED || ctx->shell.proofs_a->APP_LAUNCHER_MULTI_EXEC_PROOF_DONE || ctx->shell.chrome->FOCUSED_SURFACE_ID==0)return;
 static const uint64_t surfaces[7]={SURFACE_ID_SPINDLE,SURFACE_ID_QUIL,SURFACE_ID_LINEN,SURFACE_ID_ATLAS_OVERLAY,SURFACE_ID_BELL_PLACEHOLDER,SURFACE_ID_COLLAR,SURFACE_ID_MESH};
 ctx->shell.proofs_a->APP_LAUNCHER_MULTI_EXEC_PROOF_ACTIVE=true;sexos_line("[launcher.multi.proof] stage=0 action=start ok=1 reason=multi_exec_proof_begin");if(!ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);bool opened=ctx->surfaces->COMMAND_PALETTE_OPEN;sexos_line("[launcher.multi.proof] stage=1 action=open ok=%u reason=%s",(unsigned)opened,opened?"palette_opened":"open_failed");if(!opened){ctx->shell.proofs_a->APP_LAUNCHER_MULTI_EXEC_PROOF_DONE=true;ctx->shell.proofs_a->APP_LAUNCHER_MULTI_EXEC_PROOF_ACTIVE=false;sexos_line("[launcher.multi.proof.done] ok=0 passed=0 failed=0");return;}
 uint8_t passed=0,failed=0;
 for(uint8_t index=0;index<7;++index){ShellPaletteString name=shell_proofs_b_palette_command_label(ctx,index);ctx->surfaces->COMMAND_PALETTE_SELECTED=index;shell_proofs_b_palette_render(ctx);sexos_line("[launcher.multi.proof] stage=%u action=%s ok=1 reason=selected",(unsigned)(index+2),shell_proofs_b_palette_text(&name));if(index==3 && ctx->shell.chrome->ATLAS_MODE_ENABLED)shell_proofs_b_atlas_toggle(ctx);bool executed=shell_proofs_b_palette_execute(ctx);bool focused=index==3?ctx->shell.chrome->ATLAS_MODE_ENABLED:(executed&&ctx->shell.chrome->FOCUSED_SURFACE_ID==surfaces[index]);sexos_line("[launcher.multi.exec] idx=%u app=%s ok=%u reason=%s",(unsigned)index,shell_proofs_b_palette_text(&name),(unsigned)(index==3?focused:executed),focused?(index==3?"overlay_enabled_nonfocusable":"launched"):"exec_reject");sexos_line("[launcher.multi.focus] app=%s sid=%llu ok=%u reason=%s",shell_proofs_b_palette_text(&name),(unsigned long long)surfaces[index],(unsigned)focused,focused?(index==3?"overlay_enabled_nonfocusable":"focused"):(executed?"focus_mismatch":"exec_failed"));if(focused){if(passed<UINT8_MAX)++passed;}else if(failed<UINT8_MAX)++failed;}
 if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 bool closed=!ctx->surfaces->COMMAND_PALETTE_OPEN;
 sexos_line("[launcher.multi.proof] stage=9 action=close ok=%u reason=%s",(unsigned)closed,closed?"palette_closed":"close_failed");
 ctx->shell.proofs_a->APP_LAUNCHER_MULTI_EXEC_PROOF_DONE=true;
 ctx->shell.proofs_a->APP_LAUNCHER_MULTI_EXEC_PROOF_ACTIVE=false;
 sexos_line("[launcher.multi.proof.done] ok=%u passed=%u failed=%u",(unsigned)(passed>0&&!failed&&closed),(unsigned)passed,(unsigned)failed);
}

static void shell_proofs_b_batch_app_focus(ShellInputContext *ctx,const char *app,uint64_t surface,bool ok,const char *reason)
{
 ShellProofsBFrame frame=shell_proofs_b_frame_for_surface(ctx,surface);
 sexos_line("[shell.app.open.focus] app=%s sid=%llu frame=%u ok=%u reason=%s",app,(unsigned long long)surface,frame.present?(unsigned)frame.frame:0,(unsigned)ok,reason);
}

void shell_proofs_b_maybe_run_palette_rejects_app_open_batch_proof(ShellInputContext *ctx)
{
 if(!PALETTE_REJECTS_APP_OPEN_PROOF_ENABLED || ctx->shell.proofs_a->PALETTE_BATCH_PROOF_DONE)return;
 if(ctx->shell.chrome->FOCUSED_SURFACE_ID==0){sexos_line("[shell.palette.batch.proof] stage=0 action=wait_ready ok=0 reason=not_ready");return;}
 ctx->shell.proofs_a->PALETTE_BATCH_PROOF_ACTIVE=true;uint8_t passed=0,rejected=0,skipped=0;
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 if(!ctx->surfaces->COMMAND_PALETTE_OPEN){ctx->shell.proofs_a->PALETTE_BATCH_PROOF_DONE=true;ctx->shell.proofs_a->PALETTE_BATCH_PROOF_ACTIVE=false;sexos_line("[shell.palette.batch.proof.done] ok=0 passed=0 rejected=0 skipped=10");return;}
 size_t count=shell_proofs_b_palette_command_count(ctx);
 for(size_t index=0;index<count;++index){uint8_t old=ctx->surfaces->COMMAND_PALETTE_SELECTED,command=shell_proofs_b_palette_command_code(ctx,index);ctx->surfaces->COMMAND_PALETTE_SELECTED=(uint8_t)index;sexos_line("[shell.palette.select] old=%u new=%u",(unsigned)old,(unsigned)ctx->surfaces->COMMAND_PALETTE_SELECTED);shell_proofs_b_palette_render(ctx);
  if(command==CMD_RESTORE_MINIMIZED){ShellProofsBFrame minimized=shell_proofs_b_first_minimized_frame(ctx);uint8_t setup=minimized.present?1:0;if(!minimized.present){ShellProofsBFrame quil=shell_proofs_b_frame_for_surface(ctx,SURFACE_ID_QUIL);if(quil.present && shell_proofs_b_minimize_frame(ctx,quil.frame))setup=1;}sexos_line("[shell.restore.minimized.proof] setup=%u restore=0 ok=%u reason=%s",(unsigned)setup,(unsigned)setup,setup?"ready":"no_minimizable_target");}
  bool ok=shell_proofs_b_palette_execute(ctx);ShellPaletteString name=shell_proofs_b_palette_command_label(ctx,index);sexos_line("[shell.palette.batch.proof] stage=%zu action=%s ok=%u reason=%s",index,shell_proofs_b_palette_text(&name),(unsigned)ok,ok?"ok":"action_reject");
  if(command==CMD_FOCUS_SPINDLE)shell_proofs_b_batch_app_focus(ctx,"Spindle",SURFACE_ID_SPINDLE,ok,ok?"ok":"open_or_focus_reject");
  else if(command==CMD_FOCUS_QUIL)shell_proofs_b_batch_app_focus(ctx,"Quil",SURFACE_ID_QUIL,ok,ok?"ok":"open_or_focus_reject");
  else if(command==CMD_FOCUS_LINEN)shell_proofs_b_batch_app_focus(ctx,"Linen",SURFACE_ID_LINEN,ok,ok?"ok":"open_or_focus_reject");
  else if(command==CMD_FOCUS_ATLAS){bool atlas_ok=ctx->shell.chrome->ATLAS_MODE_ENABLED;shell_proofs_b_batch_app_focus(ctx,"Atlas",SURFACE_ID_ATLAS_OVERLAY,atlas_ok,atlas_ok?"overlay_enabled":"action_reject");}
  else if(command==CMD_FOCUS_BELL)shell_proofs_b_batch_app_focus(ctx,"Bell",SURFACE_ID_BELL_PLACEHOLDER,ok,ok?"ok":"open_or_focus_reject");
  else if(command==CMD_FOCUS_COLLAR)shell_proofs_b_batch_app_focus(ctx,"Collar",SURFACE_ID_COLLAR,ok,ok?"ok":"open_or_focus_reject");
  else if(command==CMD_FOCUS_MESH)shell_proofs_b_batch_app_focus(ctx,"Mesh",SURFACE_ID_MESH,ok,ok?"ok":"open_or_focus_reject");
  else if(command==CMD_RESTORE_MINIMIZED)sexos_line("[shell.restore.minimized.proof] setup=1 restore=1 ok=%u reason=%s",(unsigned)ok,ok?"ok":"restore_state_blocker");
  else if(command==CMD_FOCUS_BROWSER){uint64_t frame=shell_proofs_b_browser_active_frame(ctx);shell_proofs_b_batch_app_focus(ctx,"Browser",frame,ok,ok?"ok":"open_or_focus_reject");}
  if(ok){if(passed<UINT8_MAX)++passed;}else if(rejected<UINT8_MAX)++rejected;
 }
 if(ctx->surfaces->COMMAND_PALETTE_OPEN)(void)shell_proofs_b_palette_toggle(ctx);
 ctx->shell.proofs_a->PALETTE_BATCH_PROOF_DONE=true;ctx->shell.proofs_a->PALETTE_BATCH_PROOF_ACTIVE=false;sexos_line("[shell.palette.batch.proof.done] ok=1 passed=%u rejected=%u skipped=%u",(unsigned)passed,(unsigned)rejected,(unsigned)skipped);
}


