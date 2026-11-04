#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "silk_shell_atlas.h"
#include "silk_shell_custody.h"
#include "budgets.h"
#define CH (SILK_SHELL_STATE.chrome)
#define AT (SILK_SHELL_STATE.atlas)
#define MD (SILK_SHELL_STATE.models)
#define U(x) ((unsigned long long)(x))
SilkShellState SILK_SHELL_STATE;
const DesktopPolicy P={.width=1280,.height=720,.bar_height=50,.move_step=10,.resize_step=20,.min_width=120,.min_height=80};
const uint32_t ATLAS_ACCENT_COLORS[5]={0,0x805020,0x205080,0x804050,0x807000};
static uint8_t flags[256],life[256];
static bool selected_present,gate=true;
static uint32_t selected;
static int32_t dw=1280,dh=720;
static uint64_t call_status;
static const struct {uint32_t target;uint64_t surface,frame,lifecycle;} app_rows[]={
#define APP_ROW(app,ordinal,target,surface,frame,life,close,has_activate,activate,route,n,c0,c1,c2) {target,surface,frame,life},
SEXOS_SHELL_APP_ROWS(APP_ROW)
#undef APP_ROW
};
#define TEST_APPS (sizeof app_rows/sizeof *app_rows)
static ShellAppCustody custody[TEST_APPS];
static bool custody_present[TEST_APPS];

void sexos_fail(void) {_Exit(88);}
void sexos_line(const char *format,...) {va_list args;va_start(args,format);vprintf(format,args);va_end(args);putchar(10);}
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c){printf("call %llu %llu %llu %llu %llu\n",U(slot),U(op),U(a),U(b),U(c));return (SexosReply){call_status,a^b^c^op};}
static uint8_t bits(uint64_t sid){return flags[sid%256];}
bool surface_is_alive(uint64_t sid){printf("peer alive %llu\n",U(sid));return (bits(sid)&1)!=0;}
bool is_tombstoned(uint64_t sid){printf("peer tomb %llu\n",U(sid));return (bits(sid)&2)!=0;}
bool surface_is_lifecycle_focusable(uint64_t sid){printf("peer focusable %llu\n",U(sid));return (bits(sid)&4)!=0;}
bool try_set_focus(uint64_t sid){printf("peer focus %llu\n",U(sid));if(gate && (!sid || (bits(sid)&7)==5)){CH.FOCUSED_SURFACE_ID=sid;return true;}return false;}
bool lifecycle_state(uint64_t sid,uint8_t *out){printf("peer lifecycle %llu\n",U(sid));if(!(bits(sid)&16))return false;if(out)*out=life[sid%256];return true;}
bool set_lifecycle_state(uint64_t sid,uint8_t next){printf("peer set-lifecycle %llu %u\n",U(sid),(unsigned)next);if(gate)life[sid%256]=next;return gate;}
bool active_surface_for_frame(uint32_t id,uint64_t *out){
 printf("peer active %u\n",id);
 for(size_t i=0;i<MAX_FRAMES;++i){ShellFrame f=CH.FRAMES[i];if(f.present && f.frame_id==id){if(f.active_tab>=MAX_TABS_PER_FRAME || !f.tabs[f.active_tab].present)return false;if(out)*out=f.tabs[f.active_tab].surface_id;return true;}}
 return false;
}
bool get_surface_bounds(uint64_t sid,int32_t *x,int32_t *y,uint32_t *w,uint32_t *h){printf("peer bounds %llu\n",U(sid));if(!(bits(sid)&128))return false;if(x)*x=(int32_t)(sid%91);if(y)*y=(int32_t)(sid%57);if(w)*w=333;if(h)*h=222;return true;}
bool make_focus_ref(uint64_t sid,FocusRef *out){printf("peer make-ref %llu\n",U(sid));if(!(bits(sid)&64))return false;if(out)*out=(FocusRef){.present=true,.surface_id=sid,.generation=7};return true;}
bool focus_ref_is_current(const FocusRef *ref){printf("peer current %llu %llu\n",U(ref->surface_id),U(ref->generation));return (bits(ref->surface_id)&8)!=0;}
bool selected_frame_id(uint32_t *out){puts("peer selected");if(selected_present && out)*out=selected;return selected_present;}
void clear_focus_if_dead(void){puts("peer clear-focus");if(CH.FOCUSED_SURFACE_ID && (bits(CH.FOCUSED_SURFACE_ID)&3)!=1)CH.FOCUSED_SURFACE_ID=0;}
void clear_drag_if_dead(void){puts("peer clear-drag-dead");}
void clear_drag_if_wrong_scene(void){puts("peer clear-drag-scene");}
void clear_hover_if_dead(void){puts("peer clear-hover-dead");}
void clear_hover_if_wrong_scene(void){puts("peer clear-hover-scene");}
void launcher_focus_discard(void){puts("peer discard-launcher");}
void tile_active_scene_frames(void){puts("peer tile");}
void cycle_scene_render_token_preset(void){puts("peer preset-next");MD.SCENE_APPEARANCE_STATE.preset_idx=(uint8_t)((uint8_t)(MD.SCENE_APPEARANCE_STATE.preset_idx+1)%3);}
void cycle_prev_scene_render_token_preset(void){puts("peer preset-prev");MD.SCENE_APPEARANCE_STATE.preset_idx=MD.SCENE_APPEARANCE_STATE.preset_idx==0?2:(uint8_t)(MD.SCENE_APPEARANCE_STATE.preset_idx-1);}
const char *get_preset_name(uint8_t index){printf("peer preset-name %u\n",(unsigned)index);return index==0?"p0":index==1?"p1":index==2?"p2":"unknown";}
void apply_custom_tint_bundle(size_t index){printf("peer tint %zu\n",index);MD.SCENE_APPEARANCE_STATE.use_custom_colors=1;for(size_t i=0;i<8;++i)MD.SCENE_APPEARANCE_STATE.custom_colors[i]=(uint32_t)index*97+(uint32_t)i;}
void resolve_scene_render_tokens(uint32_t *out){puts("peer tokens");for(size_t i=0;i<8;++i)out[i]=1000+(uint32_t)i+MD.ACTIVE_TINT_IDX;}
void push_token_preset(const uint32_t *values){fputs("peer push",stdout);for(size_t i=0;i<8;++i)printf(" %u",values[i]);putchar(10);}
bool send_silkbar_phase2_update(uint32_t kind,uint64_t a,uint64_t b){printf("peer update %u %llu %llu\n",kind,U(a),U(b));return gate;}
size_t bell_ring_count(void){puts("peer bells");return 3;}
bool frame_is_minimized(uint32_t id){printf("peer minimized %u\n",id);for(size_t i=0;i<MAX_FRAMES;++i)if(CH.FRAMES[i].present && CH.FRAMES[i].frame_id==id)return (CH.FRAMES[i].flags&FRAME_FLAG_MINIMIZED)!=0;return false;}
static bool alter_frame(uint32_t id,unsigned kind){if(!gate)return false;for(size_t i=0;i<MAX_FRAMES;++i)if(CH.FRAMES[i].present && CH.FRAMES[i].frame_id==id){if(kind==0)CH.FRAMES[i].flags|=FRAME_FLAG_MINIMIZED;else if(kind==1)CH.FRAMES[i].flags&=~FRAME_FLAG_MINIMIZED;else CH.FRAMES[i].flags^=FRAME_FLAG_ZOOMED;return true;}return false;}
bool minimize_frame(uint32_t id){printf("peer minimize %u\n",id);return alter_frame(id,0);}
bool restore_minimized_frame(uint32_t id){printf("peer restore %u\n",id);return alter_frame(id,1);}
bool toggle_zoom_frame(uint32_t id){printf("peer zoom %u\n",id);return alter_frame(id,2);}
bool switch_to_tab(uint32_t id,uint32_t tab){printf("peer tab %u %u\n",id,tab);if(!gate)return false;for(size_t i=0;i<MAX_FRAMES;++i)if(CH.FRAMES[i].present && CH.FRAMES[i].frame_id==id && tab<CH.FRAMES[i].tab_count){CH.FRAMES[i].active_tab=(uint8_t)tab;return true;}return false;}
bool is_closeable_surface(uint64_t sid){printf("peer closeable %llu\n",U(sid));return (bits(sid)&32)!=0;}
bool close_surface_from_frame_light(uint64_t sid){printf("peer close %llu\n",U(sid));return gate;}
void clamp_position(int32_t x,int32_t y,uint32_t w,uint32_t h,int32_t *out_x,int32_t *out_y){printf("peer clamp-pos %d %d %u %u\n",x,y,w,h);if(out_x)*out_x=x<0?0:x>1000?1000:x;if(out_y)*out_y=y<0?0:y>600?600:y;}
void clamp_surface_size(int32_t x,int32_t y,uint32_t w,uint32_t h,uint32_t *out_w,uint32_t *out_h){printf("peer clamp-size %d %d %u %u\n",x,y,w,h);if(out_w)*out_w=w<1?1:w>1000?1000:w;if(out_h)*out_h=h<1?1:h>600?600:h;}
int32_t desktop_width(void){puts("peer width");return dw;}
int32_t desktop_height(void){puts("peer height");return dh;}
bool shell_custody_has_surface(uint64_t sid,bool terminal){for(size_t i=0;i<TEST_APPS;++i)if(custody_present[i] && shell_custody_owns_surface(&custody[i],sid) && (!terminal || custody[i].dead || custody[i].close_pending))return true;return false;}

static void reset(void){
 memset(&CH,0,sizeof CH);memset(&MD,0,sizeof MD);AT.ATLAS_DRAG_INTENT=(AtlasDragIntent){0};AT.SCENE_SNAPSHOT=(SceneLayoutSnapshot){0};
 memset(flags,253,sizeof flags);memset(life,2,sizeof life);memset(custody,0,sizeof custody);memset(custody_present,0,sizeof custody_present);selected_present=false;selected=0;gate=true;dw=1280;dh=720;call_status=0;
}
static void state(void){
 printf("STATE %u %llu %u %u %u %u %u %u %u %u %d %d",(unsigned)CH.ACTIVE_SCENE_IDX,U(CH.FOCUSED_SURFACE_ID),(unsigned)CH.ATLAS_MODE_ENABLED,(unsigned)CH.ATLAS_SELECTED_SCENE,(unsigned)MD.ACTIVE_TINT_IDX,(unsigned)MD.SCENE_APPEARANCE_STATE.preset_idx,(unsigned)MD.SCENE_APPEARANCE_STATE.use_custom_colors,(unsigned)AT.ATLAS_DRAG_INTENT.active,(unsigned)AT.ATLAS_DRAG_INTENT.scene_id,AT.ATLAS_DRAG_INTENT.frame_id,AT.ATLAS_DRAG_INTENT.start_x,AT.ATLAS_DRAG_INTENT.start_y);
 for(size_t i=0;i<8;++i)printf(" %u",MD.SCENE_APPEARANCE_STATE.custom_colors[i]);
 for(size_t i=0;i<ATLAS_MAX_SCENES;++i){Scene s=CH.SCENES[i];printf("|S %u %u %u ",(unsigned)s.flags,(unsigned)s.accent,(unsigned)s.pinned);for(size_t j=0;j<ATLAS_LABEL_LEN;++j)printf("%02x",(unsigned)s.label[j]);}
 for(size_t i=0;i<MAX_FRAMES;++i){ShellFrame f=CH.FRAMES[i];if(f.present){printf("|F 1 %u %u %u %u %u %d %d %u %u",f.frame_id,(unsigned)f.scene_id,(unsigned)f.active_tab,(unsigned)f.tab_count,f.flags,f.normal_x,f.normal_y,f.normal_w,f.normal_h);for(size_t j=0;j<MAX_TABS_PER_FRAME;++j){ShellTab t=f.tabs[j];if(t.present)printf(":1:%llu:%llu:%u",U(t.surface_id),U(t.title_id),t.flags);else fputs(":0:0:0:0",stdout);}}else fputs("|F 0",stdout);}
 printf("|A %u %u",CH.ATLAS_SNAPSHOT.active_scene_id,(unsigned)CH.ATLAS_SNAPSHOT.scene_count);
 for(size_t i=0;i<ATLAS_MAX_SCENES;++i){SceneDescriptor s=CH.ATLAS_SNAPSHOT.scenes[i];printf(":%u:%u:%u:%u:%u:%u ",s.scene_id,(unsigned)s.flags,(unsigned)s.accent,(unsigned)s.pinned,s.focused_frame_id,(unsigned)s.frame_count);for(size_t j=0;j<ATLAS_LABEL_LEN;++j)printf("%02x",(unsigned)s.label[j]);for(size_t j=0;j<ATLAS_MAX_FRAMES_PER_SCENE;++j)printf(":%u",s.frame_ids[j]);}
 printf("|N %u %u %u %u %u",(unsigned)AT.SCENE_SNAPSHOT.magic,(unsigned)AT.SCENE_SNAPSHOT.version,(unsigned)AT.SCENE_SNAPSHOT.active_scene,(unsigned)AT.SCENE_SNAPSHOT.frame_count,(unsigned)AT.SCENE_SNAPSHOT.checksum);
 for(size_t i=0;i<MAX_FRAMES;++i){FrameSnapshot f=AT.SCENE_SNAPSHOT.frames[i];printf(":%u:%u:%u:%u:%u:%u:%d:%d:%u:%u",(unsigned)f.present,f.frame_id,(unsigned)f.scene_id,(unsigned)f.active_tab,(unsigned)f.tab_count,f.flags,f.normal_x,f.normal_y,f.normal_w,f.normal_h);for(size_t j=0;j<MAX_TABS_PER_FRAME;++j)printf(":%llu",U(f.tab_surfaces[j]));}
 putchar(10);
}

int main(void){
 init_budgets();reset();char line[4096];
 while(fgets(line,sizeof line,stdin)){
  uint64_t v[64]={0};size_t count=0;char *at=line,*end;
  while(count<64){unsigned long long value=strtoull(at,&end,10);if(end==at)break;v[count++]=(uint64_t)value;at=end;}
  if(!count)continue;
#define N(i) (v[i])
  switch(N(0)){
   case 100:reset();break;
   case 101:{assert(N(1)<MAX_FRAMES);ShellFrame f={.present=true,.frame_id=(uint32_t)N(2),.scene_id=(uint8_t)N(3),.flags=(uint32_t)N(4),.active_tab=(uint8_t)N(5),.tab_count=(uint8_t)N(6),.normal_x=(int32_t)N(7),.normal_y=(int32_t)N(8),.normal_w=(uint32_t)N(9),.normal_h=(uint32_t)N(10)};for(size_t j=0;j<MAX_TABS_PER_FRAME;++j)if(N(11+j))f.tabs[j]=(ShellTab){.present=true,.surface_id=N(11+j),.title_id=j,.flags=(uint32_t)j};CH.FRAMES[N(1)]=f;break;}
   case 102:assert(N(1)<MAX_FRAMES);CH.FRAMES[N(1)]=(ShellFrame){0};break;
   case 103:assert(N(1)<256);flags[N(1)]=(uint8_t)N(2);life[N(1)]=(uint8_t)N(3);break;
   case 104:CH.ACTIVE_SCENE_IDX=(uint8_t)N(1);CH.FOCUSED_SURFACE_ID=N(2);CH.ATLAS_MODE_ENABLED=N(3)!=0;CH.ATLAS_SELECTED_SCENE=(uint8_t)N(4);selected_present=N(5)!=0;selected=(uint32_t)N(6);gate=N(7)!=0;break;
   case 105:dw=(int32_t)N(1);dh=(int32_t)N(2);call_status=N(3);break;
   case 106:{size_t i=N(1);assert(i<ATLAS_MAX_SCENES);CH.SCENES[i].accent=(uint8_t)N(2);CH.SCENES[i].pinned=N(3)!=0;CH.SCENES[i].flags=(uint8_t)N(4);atlas_default_label((uint32_t)N(5),CH.SCENES[i].label);break;}
   case 107:{size_t i=N(1);assert(i<TEST_APPS);custody_present[i]=true;custody[i]=shell_custody_new(app_rows[i].lifecycle,app_rows[i].target,N(2),app_rows[i].surface);custody[i].dead=N(3)!=0;custody[i].close_pending=N(4)!=0;break;}
   case 108:MD.SCENE_APPEARANCE_STATE.preset_idx=(uint8_t)N(1);MD.SCENE_APPEARANCE_STATE.use_custom_colors=(uint8_t)N(2);MD.ACTIVE_TINT_IDX=(uint8_t)N(3);break;
   case 109:AT.SCENE_SNAPSHOT=(SceneLayoutSnapshot){.magic=(uint8_t)N(1),.version=(uint8_t)N(2),.active_scene=(uint8_t)N(3),.frame_count=(uint8_t)N(4),.checksum=(uint8_t)N(5),._reserved={(uint8_t)N(6),(uint8_t)N(7),(uint8_t)N(8)}};break;
   case 110:{assert(N(1)<MAX_FRAMES);FrameSnapshot f={.present=(uint8_t)N(2),.frame_id=(uint32_t)N(3),.scene_id=(uint8_t)N(4),.active_tab=(uint8_t)N(5),.tab_count=(uint8_t)N(6),.flags=(uint32_t)N(7),.normal_x=(int32_t)N(8),.normal_y=(int32_t)N(9),.normal_w=(uint32_t)N(10),.normal_h=(uint32_t)N(11)};for(size_t i=0;i<MAX_TABS_PER_FRAME;++i)f.tab_surfaces[i]=N(12+i);AT.SCENE_SNAPSHOT.frames[N(1)]=f;break;}
   case 114:{assert(N(1)<MAX_FRAMES);_Static_assert(offsetof(FrameSnapshot,frame_id)>1,"fixture has a padding byte");unsigned char *raw=(unsigned char *)&AT.SCENE_SNAPSHOT.frames[N(1)];raw[1]=(unsigned char)N(2);break;}
   case 0:collect_atlas_snapshot();break;case 1:printf("R %u\n",(unsigned)surface_in_active_scene(N(1)));break;case 2:{uint8_t x=0;bool ok=surface_scene_id(N(1),&x);printf("R %u\n",ok?(unsigned)x:256u);break;}
   case 3:clear_focus_if_wrong_scene();break;case 4:sync_lifecycle_scene_visibility();break;case 5:sync_scene_visibility();break;
   case 6:{uint8_t out[ATLAS_LABEL_LEN];atlas_default_label((uint32_t)N(1),out);fputs("R ",stdout);for(size_t i=0;i<ATLAS_LABEL_LEN;++i)printf("%02x",(unsigned)out[i]);putchar(10);break;}
   case 7:scene_init_all();break;case 8:scene_update_flags((uint8_t)N(1));break;case 9:printf("R %u\n",(unsigned)validate_scene_id((uint8_t)N(1)));break;
   case 10:{unsigned x=scene_accent_token((uint8_t)N(1));printf("R %u\n",x);break;}case 11:{unsigned x=scene_is_pinned((uint8_t)N(1));printf("R %u\n",x);break;}
   case 12:{uint8_t out[ATLAS_LABEL_LEN];scene_label_token((uint8_t)N(1),out);fputs("R ",stdout);for(size_t i=0;i<ATLAS_LABEL_LEN;++i)printf("%02x",(unsigned)out[i]);putchar(10);break;}
   case 13:atlas_capture_snapshot();break;case 14:printf("R %u\n",(unsigned)atlas_is_enabled());break;case 15:atlas_toggle();break;case 16:atlas_exit();break;
   case 17:{int32_t x=0,y=0;uint32_t w=0,h=0;atlas_card_pos((size_t)N(1),(uint32_t)N(2),&x,&y,&w,&h);printf("R %d %d %u %u\n",x,y,w,h);break;}
   case 18:{uint8_t x=0;bool ok=atlas_scene_at_point((int32_t)N(1),(int32_t)N(2),&x);printf("R %u\n",ok?(unsigned)x:256u);break;}
   case 19:{bool x=handle_atlas_keyboard((uint8_t)N(1));printf("R %u\n",(unsigned)x);break;}case 20:atlas_render_stub();break;case 21:atlas_clear_stub();break;
   case 22:atlas_apply_scene_accent_to_chrome((uint8_t)N(1));break;case 23:switch_scene((uint8_t)N(1));break;case 24:next_scene();break;case 25:prev_scene();break;
   case 26:{bool x=atlas_cycle_scene((int32_t)N(1),"driver");printf("R %u\n",(unsigned)x);break;}
   case 27:{uint32_t x=0;bool ok=next_frame_in_scene((uint32_t)N(1),N(2)!=0,&x);printf("R %llu\n",ok?U(x):U(UINT64_MAX));break;}
   case 28:focus_next_frame();break;case 29:focus_prev_frame();break;case 30:focus_next_tab();break;case 31:focus_prev_tab();break;
   case 32:{bool x=toggle_minimize_focused_frame();printf("R %u\n",(unsigned)x);break;}case 33:{bool x=toggle_zoom_focused_frame();printf("R %u\n",(unsigned)x);break;}case 34:{bool x=close_focused_tab_or_frame_safe();printf("R %u\n",(unsigned)x);break;}
   case 35:{bool x=atlas_same_scene_drop_noop((uint32_t)N(1),(uint8_t)N(2),(uint8_t)N(3));printf("R %u\n",(unsigned)x);break;}
   case 36:{bool x=reparent_frame_to_scene((uint32_t)N(1),(uint8_t)N(2),"driver");printf("R %u\n",(unsigned)x);break;}
   case 37:{bool x=atlas_pointer_drag_begin_at((int32_t)N(1),(int32_t)N(2));printf("R %u\n",(unsigned)x);break;}case 38:{bool x=atlas_pointer_drop_at((int32_t)N(1),(int32_t)N(2));printf("R %u\n",(unsigned)x);break;}case 39:{bool x=atlas_pointer_drag_cancel("driver");printf("R %u\n",(unsigned)x);break;}
   case 40:printf("R %u\n",(unsigned)snap_compute_checksum(&AT.SCENE_SNAPSHOT));break;case 41:snap_capture_layout();break;case 42:printf("R %u\n",(unsigned)snap_validate(&AT.SCENE_SNAPSHOT));break;case 43:{bool x=snap_restore_layout();printf("R %u\n",(unsigned)x);break;}
   default:abort();
  }
  state();
 }
}
