#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "silk_shell_chrome.h"

const DesktopPolicy P={1280,720,50,10,20,120,80,{100,100,800,500},{180,160,500,300},{50,60,350,150},{900,560,300,120}};
static SilkShellChromeState chrome;
static SilkShellModelsState models;
static SilkShellInputState input;
static SilkShellProofsAState proofs_a;
static ShellChromeApps apps;
static ShellChromeContext ctx={.chrome=&chrome,.models=&models,.apps=&apps,.input=&input,.proofs_a=&proofs_a};
static FocusRef test_ref;
static uint8_t flags[1024];
static bool ready_denied[1024],ready_pending[1024];
static uint64_t call_status;
static bool point_geometry,bounds_present[1024],atlas_pick_present;static uint8_t atlas_pick;
static int32_t bounds_x[1024],bounds_y[1024];static uint32_t bounds_w[1024],bounds_h[1024];
static HitTarget hit;
static ShellAccessNodes test_nodes;
bool shell_chrome_peer_validate_scene_id(ShellChromeContext *c,uint8_t scene){(void)c;printf("peer validate-scene %u\n",scene);return scene<ATLAS_MAX_SCENES;}
void shell_chrome_peer_next_scene(ShellChromeContext *c){puts("peer next-scene");c->chrome->ACTIVE_SCENE_IDX=(c->chrome->ACTIVE_SCENE_IDX+1)%ATLAS_MAX_SCENES;}
void shell_chrome_peer_prev_scene(ShellChromeContext *c){puts("peer prev-scene");c->chrome->ACTIVE_SCENE_IDX=(c->chrome->ACTIVE_SCENE_IDX+ATLAS_MAX_SCENES-1)%ATLAS_MAX_SCENES;}
static void nodes_dump(const ShellAccessNodes *nodes){for(size_t i=0;i<MAX_ACCESS_NODES;++i){AccessNode n=nodes->values[i];if(!n.present)continue;printf("N %zu %u %u %u %u %" PRIu64 " %u %u ",i,n.node_id,n.role,n.state,n.actions,n.target.surface_id,n.target.frame_id,n.target.scene_id);for(size_t j=0;j<32;++j)printf("%02x",n.label[j]);putchar('\n');}}

static size_t linen_count;
#include "chrome_budgets.h"
void sexos_line(const char *format,...) {va_list args;va_start(args,format);vprintf(format,args);va_end(args);putchar('\n');}
void sexos_fail(void) {abort();}
bool shell_chrome_peer_surface_is_alive(ShellChromeContext *c,uint64_t sid) {(void)c;printf("peer alive %" PRIu64 "\n",sid);return flags[sid%1024]&1;}
bool shell_chrome_peer_is_tombstoned(ShellChromeContext *c,uint64_t sid) {(void)c;printf("peer tomb %" PRIu64 "\n",sid);return flags[sid%1024]&2;}
bool shell_chrome_peer_app_focusable(ShellChromeContext *c,uint64_t sid) {
    (void)c;printf("peer app %" PRIu64 "\n",sid);
    return sid==SURFACE_ID_LINEN || sid==SURFACE_ID_QUIL || sid==SURFACE_ID_MESH ||
        sid==SURFACE_ID_COLLAR || sid==SURFACE_ID_BELL_PLACEHOLDER ||
        sid==SURFACE_ID_COMMAND_PALETTE || sid==SURFACE_ID_SPINDLE || sid==SURFACE_ID_BROWSER;
}
bool shell_chrome_peer_get_surface_bounds(ShellChromeContext *c,uint64_t sid,int32_t *x,int32_t *y,uint32_t *w,uint32_t *h) {
    (void)c;printf("peer bounds %" PRIu64 "\n",sid);if(flags[sid%1024]&4)return false;
    *x=bounds_present[sid%1024]?bounds_x[sid%1024]:(int32_t)(sid%91);*y=bounds_present[sid%1024]?bounds_y[sid%1024]:(int32_t)(sid%57);*w=bounds_present[sid%1024]?bounds_w[sid%1024]:333;*h=bounds_present[sid%1024]?bounds_h[sid%1024]:222;return true;
}
void shell_chrome_peer_clamp_surface_size(ShellChromeContext *c,int32_t x,int32_t y,uint32_t w,uint32_t h,uint32_t *cw,uint32_t *ch) {
    (void)c;printf("peer clamp %d %d %u %u\n",x,y,w,h);*cw=w<1?1:w>1000?1000:w;*ch=h<1?1:h>600?600:h;
}

SexosReply shell_chrome_peer_pdx_call(ShellChromeContext *c,uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t d){
 (void)c;printf("call %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",slot,op,a,b,d);return (SexosReply){call_status,a^b^d};
}
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t d){return shell_chrome_peer_pdx_call(&ctx,slot,op,a,b,d);}
bool shell_chrome_peer_app_closeable(ShellChromeContext *c,uint64_t sid,bool *out){
 bool found=shell_chrome_peer_app_focusable(c,sid);if(found)*out=sid!=SURFACE_ID_COMMAND_PALETTE;return found;
}
void shell_chrome_peer_shell_request_app_close(ShellChromeContext *c,uint64_t sid){
 printf("peer request-close %" PRIu64 "\n",sid);
 for(size_t i=0;i<SHELL_APP_COUNT;++i)if(c->apps->present[i]&&shell_custody_owns_surface(&c->apps->entries[i],sid)){shell_custody_request_close(&c->apps->entries[i]);break;}
}
void shell_chrome_peer_clear_focus_if_dead(ShellChromeContext *c){
 puts("peer clear-focus");if(c->chrome->FOCUSED_SURFACE_ID&&!shell_chrome_surface_is_lifecycle_live(c,c->chrome->FOCUSED_SURFACE_ID)){c->chrome->FOCUSED_SURFACE_ID=0;c->models->FOCUSED_SURFACE=(FocusRef){0};}
}
void shell_chrome_peer_clear_drag_if_dead(ShellChromeContext *c){
 puts("peer clear-drag");if(c->input->INTERACTION.tag==INTERACTION_DRAGGING&&!shell_chrome_surface_is_lifecycle_focusable(c,c->input->INTERACTION.surface_id))c->input->INTERACTION=(InteractionState){0};
}
void shell_chrome_peer_clear_hover_if_dead(ShellChromeContext *c){(void)c;puts("peer clear-hover");}
void shell_chrome_peer_record_tombstone_event(ShellChromeContext *c,uint64_t sid,uint8_t old,uint8_t next,uint8_t why){(void)c;printf("peer tomb-event %" PRIu64 " %u %u %u\n",sid,old,next,why);}
void shell_chrome_peer_snap_capture_layout(ShellChromeContext *c){(void)c;puts("peer capture");}
bool shell_chrome_peer_desktop_ready(ShellChromeContext *c,uint64_t sid){
 (void)c;bool tracks=sid==200;
 for(size_t i=0;i<SHELL_APP_COUNT;++i){ShellAppCatalogRow row=shell_custody_catalog_at(i);if(sid==row.surface||(row.shell_surface&&sid==row.shell_surface))tracks=true;for(size_t j=0;j<row.child_count;++j)if(sid==row.children[j])tracks=true;}
 return tracks&&!ready_denied[sid%1024]&&!ready_pending[sid%1024];
}
size_t shell_chrome_peer_linen_object_count(ShellChromeContext *c){(void)c;puts("peer linen-count");return linen_count;}
void shell_chrome_peer_linen_render_object_list(ShellChromeContext *c){(void)c;puts("peer linen-render");}
static void view_state(const ShellViewportDelivery *d){
 ShellViewportNotice n=d->desired?d->notice:(ShellViewportNotice){0};
 printf("|V %" PRIu64 " %u %u %u %u %u %u %u %" PRIu64 " %u %u",d->sequence,d->dead,d->desired,d->last,n.target,n.task,n.w,n.h,n.sequence,d->last?d->last_w:0,d->last?d->last_h:0);
}


void shell_chrome_peer_launcher_focus_discard(ShellChromeContext *c){(void)c;puts("peer discard-launcher");}
bool shell_chrome_peer_surface_scene_id(ShellChromeContext *c,uint64_t sid,uint8_t *out){
 printf("peer scene %" PRIu64 "\n",sid);for(size_t i=0;i<MAX_FRAMES;++i){ShellFrame f=c->chrome->FRAMES[i];if(!f.present)continue;for(size_t j=0;j<MAX_TABS_PER_FRAME;++j)if(f.tabs[j].present&&f.tabs[j].surface_id==sid){*out=f.scene_id;return true;}}return false;
}
bool shell_chrome_peer_surface_in_active_scene(ShellChromeContext *c,uint64_t sid){
 printf("peer in-scene %" PRIu64 "\n",sid);for(size_t i=0;i<MAX_FRAMES;++i){ShellFrame f=c->chrome->FRAMES[i];if(!f.present)continue;for(size_t j=0;j<MAX_TABS_PER_FRAME;++j)if(f.tabs[j].present&&f.tabs[j].surface_id==sid)return f.scene_id==c->chrome->ACTIVE_SCENE_IDX;}return true;
}
bool shell_chrome_peer_mesh_is_visible_in_active_scene(ShellChromeContext *c){(void)c;puts("peer mesh-visible");return false;}
void shell_chrome_peer_mesh_render_pd_graph(ShellChromeContext *c){(void)c;puts("peer mesh-render");}
size_t shell_chrome_peer_bell_ring_count(ShellChromeContext *c){(void)c;puts("peer bell-count");return 3;}
bool shell_chrome_peer_send_silkbar_phase2_update(ShellChromeContext *c,uint32_t kind,uint64_t a,uint64_t b){(void)c;printf("peer bar-update %u %" PRIu64 " %" PRIu64 "\n",kind,a,b);return true;}
void shell_chrome_peer_surface_input_lifetime_mark_focus_live(ShellChromeContext *c,uint64_t sid,uint8_t alive,uint8_t ok){(void)c;printf("peer mark-focus %" PRIu64 " %u %u\n",sid,alive,ok);}
void shell_chrome_peer_surface_input_lifetime_try_done(ShellChromeContext *c){(void)c;puts("peer lifetime-done");}
const char *shell_chrome_peer_frame_name(ShellChromeContext *c,uint32_t id){(void)c;printf("peer frame-name %u\n",id);switch(id){case LINEN_FRAME_ID:return "linen";case QUIL_FRAME_ID:return "quil";case MESH_FRAME_ID:return "mesh";case COLLAR_FRAME_ID:return "collar";case BELL_FRAME_ID:return "bell";case COMMAND_PALETTE_FRAME_ID:return "command_palette";case SPINDLE_FRAME_ID:return "spindle";case BROWSER_FRAME_ID:return "browser";default:break;}return NULL;}

ShellAccessLabel shell_chrome_peer_frame_label(ShellChromeContext *c,uint32_t id){const char *name=shell_chrome_peer_frame_name(c,id);if(!name)name="Frame";ShellAccessLabel label={0};size_t len=strlen(name);if(len>31)len=31;memcpy(label.bytes,name,len);return label;}

bool shell_chrome_peer_point_in_surface(ShellChromeContext *c,int32_t x,int32_t y,uint64_t sid){
 printf("peer point %d %d %" PRIu64 "\n",x,y,sid);
 if(point_geometry){size_t i=sid%1024;int32_t sx=bounds_present[i]?bounds_x[i]:(int32_t)(sid%91),sy=bounds_present[i]?bounds_y[i]:(int32_t)(sid%57);uint32_t w=bounds_present[i]?bounds_w[i]:333,h=bounds_present[i]?bounds_h[i]:222;return(flags[i]&1)&&x>=(int64_t)sx&&(int64_t)x<(int64_t)sx+w&&y>=(int64_t)sy&&(int64_t)y<(int64_t)sy+h;}
 if(hit.tag==HIT_TARGET_SURFACE)return hit.surface_id==sid;
 if(hit.tag==HIT_TARGET_FRAME_CHROME){for(size_t i=0;i<MAX_FRAMES;++i){ShellFrame f=c->chrome->FRAMES[i];if(f.present&&f.frame_id==hit.frame_id&&f.active_tab<MAX_TABS_PER_FRAME&&f.tabs[f.active_tab].present&&f.tabs[f.active_tab].surface_id==sid)return true;}}return false;
}
bool shell_chrome_peer_atlas_scene_at_point(ShellChromeContext *c,int32_t x,int32_t y,uint8_t *out){(void)c;printf("peer atlas-point %d %d\n",x,y);*out=atlas_pick;return atlas_pick_present;}
void shell_chrome_peer_switch_scene(ShellChromeContext *c,uint8_t scene){printf("peer switch-scene %u\n",scene);c->chrome->ACTIVE_SCENE_IDX=scene;}
void shell_chrome_peer_sync_scene_visibility(ShellChromeContext *c){(void)c;puts("peer sync-scene");}
void shell_chrome_peer_clear_focus_if_wrong_scene(ShellChromeContext *c){(void)c;puts("peer clear-focus-scene");}
void shell_chrome_peer_clear_drag_if_wrong_scene(ShellChromeContext *c){(void)c;puts("peer clear-drag-scene");}
void shell_chrome_peer_atlas_capture_snapshot(ShellChromeContext *c){(void)c;puts("peer atlas-capture");}
bool shell_chrome_peer_toggle_bell(ShellChromeContext *c){(void)c;puts("peer toggle-bell");return call_status==0;}
void shell_chrome_peer_surface_input_lifetime_mark_drag_target(ShellChromeContext *c,uint64_t sid,uint8_t live,const char *phase,uint8_t ok){(void)c;printf("peer mark-drag %" PRIu64 " %u %s %u\n",sid,live,phase,ok);}
void shell_chrome_peer_surface_input_lifetime_mark_click_target(ShellChromeContext *c,uint64_t sid,uint8_t live,uint8_t committed,uint8_t ok){(void)c;printf("peer mark-click %" PRIu64 " %u %u %u\n",sid,live,committed,ok);}
void shell_chrome_peer_shell_interaction_contract_mark_stage(ShellChromeContext *c,uint8_t stage){(void)c;printf("peer interaction-stage %u\n",stage);}
void shell_chrome_peer_shell_interaction_contract_try_done(ShellChromeContext *c){(void)c;puts("peer interaction-done");}
void shell_chrome_peer_click_drag_proof_begin_drag_once(ShellChromeContext *c){(void)c;puts("peer proof-drag-begin");}
static void hit_dump(HitTarget t){printf("R %u %" PRIu64 " %u %u\n",t.tag,t.tag==HIT_TARGET_SURFACE?t.surface_id:0,t.tag==HIT_TARGET_FRAME_CHROME?t.frame_id:0,t.tag==HIT_TARGET_FRAME_CHROME?t.kind:0);}
static InteractionState state_of(const uint64_t *n){return(InteractionState){.tag=n[1],.surface_id=n[1]==2||n[1]==4?n[2]:0,.current_x=n[1]==2?n[3]:0,.current_y=n[1]==2?n[4]:0,.panel=n[1]==3?n[5]:0,.edge=n[1]==4?n[6]:0,.start_x=n[1]==4||n[1]==5?n[7]:0,.start_y=n[1]==4||n[1]==5?n[8]:0,.start_w=n[1]==4?n[9]:0,.start_h=n[1]==4?n[10]:0,.pending_dx=n[1]==4?n[11]:0,.pending_dy=n[1]==4?n[12]:0,.frame_id=n[1]==5?n[13]:0,.start_tab=n[1]==5?n[14]:0,.current_tab=n[1]==5?n[15]:0};}
static void reset(void) {
    uint32_t interaction_budget=atomic_load(&input.INTERACTION_LOG_BUDGET);
    memset(&proofs_a,0,sizeof(proofs_a));point_geometry=false;memset(bounds_present,0,sizeof(bounds_present));atlas_pick_present=false;
    reset_chrome();memset(&test_nodes,0,sizeof(test_nodes));for(size_t i=0;i<ATLAS_MAX_SCENES;++i)chrome.SCENES[i].flags=SCENE_FLAG_EMPTY;hit=(HitTarget){0};memset(&models,0,sizeof(models));memset(&apps,0,sizeof(apps));memset(&input,0,sizeof(input));
    memset(ready_denied,0,sizeof(ready_denied));memset(ready_pending,0,sizeof(ready_pending));call_status=0;linen_count=0;chrome.WINDOWS_len=2;
    memset(&test_ref,0,sizeof(test_ref));memset(flags,1,sizeof(flags));models.LIFECYCLE_GENERATION=1;
    atomic_init(&input.INTERACTION_LOG_BUDGET,interaction_budget);atomic_init(&apps.width,1280);atomic_init(&apps.height,720);
}
static void state(void) {
    printf("S %" PRIu64 " %u %u %u %u %" PRIu64,chrome.FOCUSED_SURFACE_ID,chrome.ACTIVE_SCENE_IDX,chrome.HOVERED_FRAME_ID,chrome.HOVER_KIND,chrome.HOVERED_FRAME_LIGHT,models.LIFECYCLE_GENERATION);
    for(size_t i=0;i<MAX_FRAMES;++i){ShellFrame f=chrome.FRAMES[i];if(f.present){printf("|F %u %u %u %u %u",f.frame_id,f.active_tab,f.tab_count,f.scene_id,f.flags);printf("|N %d %d %u %u",f.normal_x,f.normal_y,f.normal_w,f.normal_h);
        for(size_t j=0;j<MAX_TABS_PER_FRAME;++j){ShellTab t=f.tabs[j];if(t.present)printf(":%" PRIu64 ":%" PRIu64 ":%u",t.surface_id,t.title_id,t.flags);else printf(":-");}
    }else printf("|-");}
    // Compare every occupied entry; spare capacity has an independent control.
    for(size_t i=0;i<LIFECYCLE_MAX_SURFACES;++i){LifecycleTableEntry e=models.LIFECYCLE_TABLE[i];if(e.present)printf("|L %" PRIu64 " %u %" PRIu64,e.surface_id,e.lc.state,e.lc.generation);}
    FocusRef refs[2]={models.FOCUSED_SURFACE,test_ref};
    for(size_t i=0;i<2;++i){FocusRef r=refs[i];if(r.present)printf("|R %" PRIu64 " %" PRIu64,r.surface_id,r.generation);else printf("|-");}

    for(size_t i=0;i<SHELL_APP_COUNT;++i){ShellAppCustody e=apps.entries[i];if(apps.present[i])printf("|C %u %u %" PRIu64 " %u %u %u %d %d %u %u",e.target,e.task,e.surface,e.dead,e.close_pending,e.has_bounds,e.has_bounds?e.x:0,e.has_bounds?e.y:0,e.has_bounds?e.w:0,e.has_bounds?e.h:0);else printf("|-");}
    for(size_t i=0;i<SHELL_DOCUMENT_VIEW_COUNT;++i)view_state(&apps.document[i].state);
    for(size_t i=0;i<SHELL_SPINDLE_VIEW_COUNT;++i)view_state(&apps.spindle[i].state);
    if(apps.palette_visibility.pending){ShellChromeOperation o=apps.palette_visibility.operation;printf("|P 1 %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64,o.opcode,o.surface,o.packed,o.control);}else printf("|P 0");
    for(size_t i=0;i<2;++i){SexosWindowDescriptor d=chrome.WINDOWS[i].desc;printf("|W %d %d %u %u",d.x,d.y,d.width,d.height);}
    printf("|A %u %u %u %u %u %u",chrome.SURFACE_100_ALIVE,chrome.SURFACE_101_ALIVE,chrome.SURFACE_102_ALIVE,chrome.SURFACE_103_ALIVE,chrome.SURFACE_100_W,chrome.SURFACE_100_H);
    printf("|G %d %d %u %u",chrome.SURFACE_101_X,chrome.SURFACE_101_Y,chrome.SURFACE_101_W,chrome.SURFACE_101_H);
    printf("|G %d %d %u %u",chrome.SURFACE_102_X,chrome.SURFACE_102_Y,chrome.SURFACE_102_W,chrome.SURFACE_102_H);
    printf("|G %d %d %u %u",chrome.SURFACE_103_X,chrome.SURFACE_103_Y,chrome.SURFACE_103_W,chrome.SURFACE_103_H);
    printf("|G %d %d %u %u",input.SURFACE_200_X,input.SURFACE_200_Y,input.SURFACE_200_W,input.SURFACE_200_H);
    printf("|G %d %d %u %u",input.SURFACE_201_X,input.SURFACE_201_Y,input.SURFACE_201_W,input.SURFACE_201_H);
    printf("|G %d %d %u %u",input.SURFACE_202_X,input.SURFACE_202_Y,input.SURFACE_202_W,input.SURFACE_202_H);
    printf("|G %d %d %u %u",input.SURFACE_203_X,input.SURFACE_203_Y,input.SURFACE_203_W,input.SURFACE_203_H);
    printf("|G %d %d %u %u",input.SURFACE_204_X,input.SURFACE_204_Y,input.SURFACE_204_W,input.SURFACE_204_H);
    printf("|G %d %d %u %u",input.SURFACE_205_X,input.SURFACE_205_Y,input.SURFACE_205_W,input.SURFACE_205_H);
    printf("|G %d %d %u %u",input.SURFACE_0x99_X,input.SURFACE_0x99_Y,input.SURFACE_0x99_W,input.SURFACE_0x99_H);

    InteractionState in=input.INTERACTION;unsigned tag=in.tag;uint64_t id=tag==INTERACTION_TAB_DRAGGING?in.frame_id:(tag==INTERACTION_DRAGGING||tag==INTERACTION_RESIZING)?in.surface_id:0;printf("|I %u %" PRIu64,tag,id);
    printf("|D %u %" PRIu64 " %u %d %d %u %u %d %d %u %u %u %u %u %u %u %u",input.DRAG_PENDING_ACTIVE,input.DRAG_PENDING_TARGET,input.DRAG_PENDING_KIND,input.DRAG_PENDING_START_X,input.DRAG_PENDING_START_Y,input.TAB_DRAG_PENDING_FRAME_ID,input.TAB_DRAG_PENDING_START_TAB,input.POINTER_X,input.POINTER_Y,input.INPUT_TRACE_DRAG_MOVES,chrome.DRAG_MOVED_THIS_DRAG,chrome.WINDOW_MOVE_PROOF_DONE_EMITTED,proofs_a.DRAG_CAPTURE_MOVE_SEEN,input.LAUNCHER_ACTIVE,input.STATUS_ACTIVE,input.CLOCK_ACTIVE,input.SCENE_SETTINGS_ACTIVE);
    printf("|E %u %" PRIu64 " %d %d %u %u %d %d %u %u %d %d %u %u %u",in.tag,in.surface_id,in.current_x,in.current_y,in.panel,in.edge,in.start_x,in.start_y,in.start_w,in.start_h,in.pending_dx,in.pending_dy,in.frame_id,in.start_tab,in.current_tab);
    ShellDocumentPointer dp=apps.document_pointer;if(dp.pending)printf("|DP %" PRIu64 " 1 %" PRIu64 " %u %u %" PRIu64,apps.document_pointer_sequence,dp.slot,dp.target,dp.action,dp.sequence);else printf("|DP %" PRIu64 " 0",apps.document_pointer_sequence);
    puts("");
}
static void optional(bool valid,uint64_t value) {printf("R %u %" PRIu64 "\n",valid,valid?value:0);}
int main(void) {
    init_budgets();reset();char line[2048];
    while(fgets(line,sizeof(line),stdin)) {
        uint64_t n[40]={0};size_t count=0;
        for(char *p=line;*p && count<40;){char *end;unsigned long long v=strtoull(p,&end,10);if(end==p)break;n[count++]=v;p=end;}
        if(!count)continue;
        uint8_t byte=0;uint32_t word=0,w=0,h=0;uint64_t wide=0;int32_t x=0,y=0;bool found=false;
        switch(n[0]){
        case 97:printf("R %zu\n",LIFECYCLE_MAX_SURFACES);break;
        case 100:reset();break;
        case 101:{ShellFrame f={.present=true,.frame_id=n[2],.active_tab=n[3],.tab_count=n[4],.scene_id=n[5],.flags=n[6]};
            for(size_t i=0;i<MAX_TABS_PER_FRAME;++i){if(n[i+7])f.tabs[i]=(ShellTab){true,n[i+7],i,i};}
            chrome.FRAMES[n[1]]=f;break;}
        case 102:chrome.FRAMES[n[1]].present=false;break;
        case 103:chrome.FOCUSED_SURFACE_ID=n[1];chrome.ACTIVE_SCENE_IDX=n[2];chrome.HOVERED_FRAME_ID=n[3];chrome.HOVER_KIND=n[4];chrome.HOVERED_FRAME_LIGHT=n[5];atomic_store(&apps.width,n[6]);atomic_store(&apps.height,n[7]);break;
        case 104:flags[n[1]]=n[2];break;
        case 105:{ShellAppCatalogRow r=shell_custody_catalog_at(n[1]);ShellAppCustody e=shell_custody_new(r.lifecycle_slot,r.target,n[2],r.surface);e.dead=n[3];e.close_pending=n[4];e.has_bounds=n[5];e.w=300;e.h=200;apps.present[n[1]]=true;apps.entries[n[1]]=e;break;}
        case 106:models.LIFECYCLE_GENERATION=n[1];models.LIFECYCLE_TABLE[0]=(LifecycleTableEntry){true,n[2],{n[3],n[4]}};break;
        case 0:found=shell_chrome_scancode_to_action(&ctx,n[1],&byte);optional(found,byte);break;
        case 1:printf("R %s\n",shell_chrome_action_name(&ctx,n[1]));break;
        case 2:printf("R %d %d\n",shell_chrome_desktop_width(&ctx),shell_chrome_desktop_height(&ctx));break;
        case 3:case 4:case 5:
            if(n[0]==3)shell_chrome_layout_left(&ctx,&x,&y,&w,&h);
            else if(n[0]==4)shell_chrome_layout_right(&ctx,&x,&y,&w,&h);
            else shell_chrome_layout_maximize(&ctx,&x,&y,&w,&h);
            printf("R %d %d %u %u\n",x,y,w,h);break;
        case 6:shell_chrome_clamp_position(&ctx,n[1],n[2],n[3],n[4],&x,&y);printf("R %d %d\n",x,y);break;
        case 7:shell_chrome_snap_end_pos(&ctx,n[1],n[2],&x,&y);printf("R %d %d\n",x,y);break;
        case 8:printf("R %u\n",shell_chrome_lifecycle_register(&ctx,n[1],n[2]));break;
        case 9:found=shell_chrome_lifecycle_state(&ctx,n[1],&byte);optional(found,byte);break;
        case 10:printf("R %u\n",shell_chrome_set_lifecycle_state(&ctx,n[1],n[2]));break;
        case 11:found=shell_chrome_surface_generation(&ctx,n[1],&wide);optional(found,wide);break;
        case 12:printf("R %u\n",shell_chrome_bump_surface_generation(&ctx,n[1]));break;
        case 13:test_ref=(FocusRef){0};found=shell_chrome_make_focus_ref(&ctx,n[1],&test_ref);optional(found,test_ref.generation);break;
        case 14:printf("R %u\n",test_ref.present && shell_chrome_focus_ref_is_current(&ctx,&test_ref));break;
        case 15:shell_chrome_sync_focus_ref(&ctx);break;
        case 16:printf("R %u\n",shell_chrome_surface_is_lifecycle_live(&ctx,n[1]));break;
        case 17:printf("R %u\n",shell_chrome_surface_is_lifecycle_focusable(&ctx,n[1]));break;
        case 18:shell_chrome_lifecycle_init_all(&ctx);break;
        case 19:printf("R %u\n",shell_chrome_is_shell_surface(&ctx,n[1]));break;
        case 20:printf("R %u\n",shell_chrome_is_focusable_surface(&ctx,n[1]));break;
        case 21:found=shell_chrome_frame_for_surface(&ctx,n[1],&word);optional(found,word);break;
        case 22:found=shell_chrome_active_surface_for_frame(&ctx,n[1],&wide);optional(found,wide);break;
        case 23:found=shell_chrome_selected_frame_id(&ctx,&word);optional(found,word);break;
        case 24:found=shell_chrome_selected_surface_id(&ctx,&wide);optional(found,wide);break;
        case 25:printf("R %u\n",shell_chrome_selected_window_options_mask(&ctx));break;
        case 26:printf("R %u\n",shell_chrome_frame_is_minimized(&ctx,n[1]));break;
        case 27:shell_chrome_set_frame_minimized(&ctx,n[1],n[2]);break;
        case 28:printf("R %u\n",shell_chrome_frame_is_zoomed(&ctx,n[1]));break;
        case 29:shell_chrome_set_frame_zoomed(&ctx,n[1],n[2]);break;
        case 30:printf("R %u\n",shell_chrome_frame_has_top_bar(&ctx,n[1]));break;
        case 31:shell_chrome_set_frame_top_bar(&ctx,n[1],n[2]);break;
        case 32:found=shell_chrome_first_minimized_frame_id(&ctx,&word);optional(found,word);break;
        case 33:printf("R %u\n",shell_chrome_frame_accepts_input(&ctx,n[1]));break;
        case 34:printf("R %u\n",shell_chrome_frame_chrome_visible(&ctx,n[1]));break;
        case 35:shell_chrome_clear_hover_if_wrong_scene(&ctx);break;
        case 36:printf("R %u\n",shell_chrome_frame_tab_count(&ctx,n[1]));break;
        case 37:printf("R %u\n",shell_chrome_frame_active_tab_index(&ctx,n[1]));break;
        case 38:found=shell_chrome_frame_tab_at(&ctx,n[1],n[2],n[3],&word);optional(found,word);break;
        case 39:printf("R %u\n",shell_chrome_swap_frame_tabs(&ctx,n[1],n[2],n[3]));break;
        case 40:printf("R %u\n",shell_chrome_frame_light_to_option_mask(&ctx,n[1]));break;
        case 41:printf("R %" PRIu64 "\n",shell_chrome_abs_screen_to_raw(&ctx,n[1],n[2]));break;
        case 42:printf("R %u\n",shell_chrome_compute_resize_edge(&ctx,n[1],n[2],n[3],n[4],n[5],n[6]));break;
        case 43:shell_chrome_compute_resize_rect(&ctx,n[1],n[2],n[3],n[4],n[5],n[6],n[7],&x,&y,&w,&h);printf("R %d %d %u %u\n",x,y,w,h);break;
        case 44:{Rect r=shell_chrome_rect_new(n[1],n[2],n[3],n[4]);printf("R %u\n",shell_chrome_rect_contains(&r,n[5],n[6]));break;}

        case 120:input.INTERACTION=(InteractionState){.tag=n[1]==1?INTERACTION_DRAGGING:n[1]==2?INTERACTION_RESIZING:n[1]==3?INTERACTION_TAB_DRAGGING:INTERACTION_IDLE,.surface_id=n[1]==1||n[1]==2?n[2]:0,.frame_id=n[1]==3?n[2]:0};break;
        case 121:chrome.SURFACE_100_ALIVE=n[1];chrome.SURFACE_101_ALIVE=n[1];chrome.SURFACE_102_ALIVE=n[1];chrome.SURFACE_103_ALIVE=n[1];linen_count=n[2];call_status=n[3];break;
        case 122:ready_denied[n[1]%1024]=n[2];ready_pending[n[1]%1024]=n[3];break;
        case 123:{ShellViewportDelivery *d=n[1]?&apps.spindle[n[2]].state:&apps.document[n[2]].state;d->last=true;d->last_w=n[3];d->last_h=n[4];d->sequence=n[5];break;}
        case 124:hit=(HitTarget){.tag=n[1],.surface_id=n[2],.frame_id=n[2],.kind=n[3]};models.ACTIVE_TINT_IDX=n[4];break;
        case 126:{Scene scene={.flags=n[2]};for(size_t i=0;i<ATLAS_LABEL_LEN;++i)scene.label[i]=n[i+3];chrome.SCENES[n[1]]=scene;break;}
        case 127:test_nodes.values[n[1]]=(AccessNode){.present=n[2]!=0,.node_id=n[1],.role=ACCESS_ROLE_FRAME,.state=n[3],.actions=n[4],.target={n[5],n[6],n[7]}};break;
        case 67:{ShellAccessLabel dst;memset(&dst,n[2],sizeof(dst));ShellAccessText src={.len=n[1]};for(size_t i=0;i<32;++i)src.bytes[i]=n[i+3];shell_chrome_access_copy_label(&ctx,&dst,src);printf("B ");for(size_t i=0;i<32;++i)printf("%02x",dst.bytes[i]);printf(" %u\n",shell_chrome_access_label_token(&ctx,&dst));break;}
        case 68:{AccessTargetRef target={n[1],n[2],n[3]};printf("R %u\n",shell_chrome_access_node_is_valid_target(&ctx,&target));break;}
        case 69:printf("R %zu\n",shell_chrome_access_emit_scene_node(&ctx,&test_nodes,n[1],n[2]));nodes_dump(&test_nodes);break;
        case 70:{ShellFrame frame=chrome.FRAMES[n[2]];printf("R %zu\n",shell_chrome_access_emit_frame_node(&ctx,&test_nodes,n[1],&frame));nodes_dump(&test_nodes);break;}
        case 71:printf("R %zu\n",shell_chrome_access_emit_shell_nodes(&ctx,&test_nodes));nodes_dump(&test_nodes);break;
        case 72:printf("R %u\n",shell_chrome_access_handle_keyboard_action(&ctx,n[1]));break;
        case 73:shell_chrome_access_describe_focus(&ctx);break;
        case 74:printf("R %u\n",shell_chrome_access_move_focus(&ctx,&test_nodes,n[1],n[2]));break;
        case 128:point_geometry=n[1];input.POINTER_X=n[2];input.POINTER_Y=n[3];break;
        case 129:input.INTERACTION=state_of(n);break;
        case 130:bounds_present[n[1]]=n[2];bounds_x[n[1]]=n[3];bounds_y[n[1]]=n[4];bounds_w[n[1]]=n[5];bounds_h[n[1]]=n[6];break;
        case 131:chrome.ATLAS_MODE_ENABLED=n[1];atlas_pick_present=n[2];atlas_pick=n[3];break;
        case 132:apps.document_pointer_sequence=n[1];apps.document_pointer=(ShellDocumentPointer){n[2]!=0,n[3],n[4],n[5],n[6]};break;
        case 75:shell_chrome_try_transition(&ctx,state_of(n));break;
        case 76:{ShellChromeMove result={0};if(shell_chrome_move_surface_by(&ctx,n[1],n[2],n[3],&result))printf("R 1 %d %d %u\n",result.x,result.y,result.clamped);else puts("R 0");break;}
        case 77:{ShellChromeDragTarget result={0};if(shell_chrome_synthetic_window_drag_target(&ctx,&result))printf("R 1 %u %" PRIu64 " %d %d %u %u %d %d\n",result.frame,result.sid,result.x,result.y,result.w,result.h,result.px,result.py);else puts("R 0");break;}
        case 78:shell_chrome_silk_drag_begin_mark(&ctx,n[1],n[2],n[3]);break;
        case 79:shell_chrome_silk_drag_end_mark(&ctx,n[1]);break;
        case 80:printf("R %u\n",shell_chrome_drag_move_focused(&ctx,n[1],n[2]));break;
        case 81:printf("R %u\n",shell_chrome_resize_accumulate_delta(&ctx,n[1],n[2]));break;
        case 82:shell_chrome_clear_resize_if_dead(&ctx);break;
        case 83:printf("R %u\n",shell_chrome_try_snap_on_drag_release(&ctx,n[1],n[2],n[3]));break;
        case 84:printf("R %u\n",shell_chrome_apply_resize_geometry(&ctx,n[1],n[2]));break;
        case 85:{HitTarget target={0};(void)shell_chrome_hit_test_surface_chrome(&ctx,n[1],n[2],n[3],&target);hit_dump(target);break;}
        case 86:hit_dump(shell_chrome_hit_test_at(&ctx,n[1],n[2]));break;
        case 87:{HitTarget target={.tag=n[1],.surface_id=n[2],.frame_id=n[3],.kind=n[4]};uint64_t sid=0;bool ok=shell_chrome_hit_target_surface(&ctx,target,&sid);optional(ok,sid);const char *label=shell_chrome_hit_target_label(&ctx,target,n[5],&sid);printf("T %s %" PRIu64 "\n",label,sid);break;}
        case 88:{ShellChromeClick result=shell_chrome_click_hit_test_and_focus(&ctx,n[1],n[2],n[3]);hit_dump(result.target);printf("R %u\n",result.silkbar);break;}
        case 89:{bool active=n[1]!=0;bool ok=shell_chrome_toggle_os_panel(&ctx,&active,n[2],n[3],"driver",n[4],n[5],n[6],n[7]);printf("R %u %u\n",ok,active);break;}
        case 90:shell_chrome_toggle_scene_settings_panel(&ctx);break;
        case 91:printf("R %u\n",shell_chrome_handle_silkbar_click(&ctx,n[1],n[2]));break;
        case 125:if(models.FOCUSED_SURFACE.present)printf("R 1 %" PRIu64 " %" PRIu64 "\n",models.FOCUSED_SURFACE.surface_id,models.FOCUSED_SURFACE.generation);else puts("R 0");break;
        case 59:shell_chrome_tile_active_scene_frames(&ctx);break;
        case 60:shell_chrome_tile_visible_frames(&ctx);break;
        case 61:printf("R %u\n",shell_chrome_frame_light_at(&ctx,n[1],n[2],n[3]));break;
        case 62:printf("R %u\n",shell_chrome_toggle_top_bar_for_active_frame(&ctx));break;
        case 63:printf("R %u\n",shell_chrome_switch_to_tab(&ctx,n[1],n[2]));break;
        case 64:printf("R %u\n",shell_chrome_try_set_focus(&ctx,n[1]));break;
        case 65:printf("R %u\n",shell_chrome_try_set_focus_inner(&ctx,n[1],n[2]));break;
        case 66:printf("R %u\n",shell_chrome_update_frame_hover_at(&ctx,n[1],n[2]));break;
        case 45:printf("R %u\n",shell_chrome_is_closeable_surface(&ctx,n[1]));break;
        case 46:printf("R %u\n",shell_chrome_frame_close_allowed(&ctx,n[1]));break;
        case 47:printf("R %u\n",shell_chrome_close_surface_from_frame_light(&ctx,n[1]));break;
        case 48:printf("R %u\n",shell_chrome_minimize_frame(&ctx,n[1]));break;
        case 49:printf("R %u\n",shell_chrome_restore_minimized_frame(&ctx,n[1]));break;
        case 50:printf("R %u\n",shell_chrome_restore_minimized_frame_inner(&ctx,n[1],n[2]));break;
        case 51:shell_chrome_emit_chrome_diagnostics(&ctx,n[1],"driver");break;
        case 52:shell_chrome_update_local_geometry(&ctx,n[1],n[2],n[3],n[4],n[5]);break;
        case 53:printf("R %u\n",shell_chrome_zoom_frame(&ctx,n[1]));break;
        case 54:printf("R %u\n",shell_chrome_unzoom_frame(&ctx,n[1]));break;
        case 55:printf("R %u\n",shell_chrome_toggle_zoom_frame(&ctx,n[1]));break;
        case 56:shell_chrome_native_child_geometry(&ctx,n[1],n[2],n[3],n[4],n[5]);break;
        case 57:{ShellChromeOperation op={0};if(shell_chrome_frame_chrome_operation(&ctx,n[1],&op))printf("R 1 %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",op.opcode,op.surface,op.packed,op.control);else puts("R 0");break;}
        case 58:shell_chrome_send_frame_tab_info(&ctx,n[1]);break;
        default:abort();}
        state();
    }
    return 0;
}
