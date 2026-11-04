static ShellActivationRaw activations[512];
static bool launch_available=true;
static ShellSurfaceBool launch_focus;
static uint8_t access_decision;
static uint64_t stale_status=UINT64_C(0xfffffffffffffffe);
static bool replay_corrupt;
static uint32_t commit_count[512];
ShellActivationRaw shell_surfaces_activation_call(uint64_t slot,bool commit){
 printf("surface activation %" PRIu64 " %u\n",slot,commit);
 if(commit)++commit_count[slot];
 if(commit && !activations[slot].status && activations[slot].state==1)activations[slot].state=2;
 ShellActivationRaw result=activations[slot];
 if(replay_corrupt&&commit&&commit_count[slot]>=2)result.state=3;
 return result;
}
void shell_surfaces_peer_launcher_focus_discard(ShellSurfacesContext *c){shell_chrome_peer_launcher_focus_discard(&c->shell);}
void shell_surfaces_peer_launcher_action_record(ShellSurfacesContext *c,uint32_t target,uint64_t frame,uint8_t action,uint8_t from,bool activated,bool ok){
 (void)c;printf("surface action-record %u %" PRIu64 " %u %u %u %u\n",target,frame,action,from,activated,ok);
}
ShellSurfaceBool shell_surfaces_peer_launcher_focus_existing(ShellSurfacesContext *c,uint32_t target,uint64_t frame,bool has,uint8_t action){
 (void)c;printf("surface focus-existing %u %" PRIu64 " %u %u\n",target,frame,has,action);return launch_focus;
}
bool shell_surfaces_peer_native_launcher_available(ShellSurfacesContext *c,uint32_t target){(void)c;printf("surface available %u\n",target);return launch_available;}
SexosReply shell_surfaces_peer_raw_pdx_call(ShellSurfacesContext *c,uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t d){
 (void)c;printf("surface rawcall %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",slot,op,a,b,d);return(SexosReply){stale_status,0};
}
SexosReply shell_surfaces_peer_pdx_call(ShellSurfacesContext *c,uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t d){return shell_chrome_peer_pdx_call(&c->shell,slot,op,a,b,d);}
void shell_surfaces_peer_linen_refresh(ShellSurfacesContext *c,bool notify){(void)c;printf("surface linen-refresh %u\n",notify);}
void shell_surfaces_peer_linen_paint_surface(ShellSurfacesContext *c){(void)c;puts("surface linen-paint");}
void shell_surfaces_peer_linen_paint_surface_fast(ShellSurfacesContext *c){(void)c;puts("surface linen-fast");}
size_t shell_surfaces_peer_linen_object_count(ShellSurfacesContext *c){return shell_chrome_peer_linen_object_count(&c->shell);}
size_t shell_surfaces_peer_quil_buffer_count(ShellSurfacesContext *c){(void)c;puts("surface quil-count");return 5;}
void shell_surfaces_peer_quil_render_buffer_list(ShellSurfacesContext *c){(void)c;puts("surface quil-render");}
void shell_surfaces_peer_mesh_render_fact_list(ShellSurfacesContext *c){(void)c;puts("surface mesh-render");}
void shell_surfaces_peer_mesh_emit_linen_quil_links(ShellSurfacesContext *c){(void)c;puts("surface mesh-links");}
void shell_surfaces_peer_collar_render_grants_text(ShellSurfacesContext *c){(void)c;puts("surface collar-render");}
void shell_surfaces_peer_collar_auto_grant_from_manifest(ShellSurfacesContext *c,const AppManifest *m){(void)c;printf("surface autogrant %" PRIu64 " %u\n",m->surface_id,m->capabilities.bits);}
void shell_surfaces_peer_registration_reply_poll(ShellSurfacesContext *c){printf("surface registration-poll");for(size_t i=0;i<SHELL_APP_COUNT;++i)if(c->shell.apps->present[i]&&c->shell.apps->entries[i].registration_reply)printf(" %u",c->shell.apps->entries[i].target);putchar('\n');}
void shell_surfaces_peer_bell_render_event_list(ShellSurfacesContext *c){(void)c;puts("surface bell-render");}
void shell_surfaces_peer_spindle_render(ShellSurfacesContext *c){(void)c;puts("surface spindle-render");}
void shell_surfaces_peer_snap_capture_layout(ShellSurfacesContext *c){shell_chrome_peer_snap_capture_layout(&c->shell);}
bool shell_surfaces_peer_surface_is_alive(ShellSurfacesContext *c,uint64_t sid){return shell_chrome_peer_surface_is_alive(&c->shell,sid);}
uint8_t shell_surfaces_peer_collar_check_operation(ShellSurfacesContext *c,uint8_t op,uint64_t caller,uint64_t object){(void)c;printf("surface collar-check %u %" PRIu64 " %" PRIu64 "\n",op,caller,object);return access_decision;}
