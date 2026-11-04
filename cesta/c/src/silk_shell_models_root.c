/*Native bindings from the canonical shell root to checked component contexts.*/
#include "silk_shell_models_root.h"
static ShellLinenModelsContext models_linen_view(void)
{
 return(ShellLinenModelsContext){&SILK_SHELL_STATE.models,&SILK_SHELL_STATE.linen};
}
static ShellLinenRenderContext models_render_view(void)
{
 return(ShellLinenRenderContext){{&SILK_SHELL_STATE.chrome,&SILK_SHELL_STATE.models,
  &SILK_SHELL_STATE.apps,&SILK_SHELL_STATE.input,&SILK_SHELL_STATE.proofs_a},&SILK_SHELL_STATE.linen};
}
void silk_shell_models_runtime_init(void)
{
 SILK_SHELL_STATE.linen=shell_models_linen_new();
}
void linen_fetch_remote_snapshot(void)
{
 ShellLinenModelsContext view=models_linen_view();shell_models_fetch_remote_snapshot(&view);
}
void linen_queue_open(uint64_t object,uint64_t choice)
{
 ShellLinenModelsContext view=models_linen_view();shell_models_queue_open(&view,object,choice);
}
void linen_async_poll(void)
{
 ShellLinenModelsContext view=models_linen_view();shell_models_linen_async_poll(&view);
}
bool linen_async_reply(const SexosMessage *message)
{
 ShellLinenModelsContext view=models_linen_view();return shell_models_linen_async_reply(&view,message);
}
void linen_render_object_list(void)
{
 ShellLinenRenderContext view=models_render_view();shell_models_render_object_list(&view);
}
void linen_render_static_ui(void)
{
 ShellLinenRenderContext view=models_render_view();shell_models_render_static_ui(&view);
}
void linen_paint_surface(void)
{
 ShellLinenRenderContext view=models_render_view();shell_models_paint_surface(&view,startup_messages_pending());
}
void linen_paint_surface_fast(void)
{
 ShellLinenRenderContext view=models_render_view();shell_models_paint_surface_fast(&view);
}
void shell_models_peer_paint_fast(ShellLinenModelsContext *view)
{
 if(view->models!=&SILK_SHELL_STATE.models || view->linen!=&SILK_SHELL_STATE.linen){sexos_fail();for(;;){}}
 linen_paint_surface_fast();
}

void workspace_refresh_poll(void)
{
 ShellLinenWorkspaceContext view={&SILK_SHELL_STATE.apps,&SILK_SHELL_STATE.linen};shell_models_workspace_poll(&view);
}
bool workspace_refresh_receive(const SexosMessage *message)
{
 ShellLinenWorkspaceContext view={&SILK_SHELL_STATE.apps,&SILK_SHELL_STATE.linen};return shell_models_workspace_receive(&view,message);
}


