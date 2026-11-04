/*Root extraction, serial evidence and paint callback around the checked value publisher.*/
#include "silk_shell_models_linen.h"
void shell_models_publish_snapshot(ShellLinenModelsContext *ctx)
{
 uint64_t previous=ctx->models->SELECTED_LINEN_OBJECT_ID;
 if(!shell_models_publish_snapshot_checked(ctx->models,ctx->linen,&ctx->linen->snapshot,&LINEN_SEED_OBJECTS,true)){
  ctx->models->LINEN_REMOTE_FETCHED=false;
  sexos_line("[linen.remote.snapshot.rejected] reason=invalid_or_exhausted preserve_view=1 async=1");
  return;
 }
 for(size_t i=0;i<SHELL_LINEN_CAPACITY && i<LINEN_MAX_OBJECTS;++i){
  if(!ctx->linen->snapshot.present[i])continue;
  const ShellLinenObject *row=&ctx->linen->snapshot.objects[i];
  sexos_line("[linen.remote.entry] slot=%zu id=%llu kind=%u name_len=%u",i,(unsigned long long)row->id,(unsigned)row->kind,(unsigned)row->len);
 }
 if(ctx->models->SELECTED_LINEN_OBJECT_ID!=previous && ctx->models->SELECTED_LINEN_OBJECT_ID!=0)
  sexos_line("[linen.object_select.current] id=%llu",(unsigned long long)ctx->models->SELECTED_LINEN_OBJECT_ID);
 sexos_line("[linen.remote.snapshot.ok] count=%zu unique=1 complete=1 selection_preserved=%u",ctx->linen->snapshot.count,(unsigned)(ctx->models->SELECTED_LINEN_OBJECT_ID==previous && previous!=0));
 shell_models_peer_paint_fast(ctx);
}
void shell_models_publish_capture_snapshot(ShellLinenModelsContext *ctx)
{
 uint64_t previous=ctx->models->SELECTED_LINEN_OBJECT_ID;
 if(!shell_models_publish_snapshot_checked(ctx->models,ctx->linen,&ctx->linen->capture_snapshot,&LINEN_SEED_OBJECTS,false)){
  ctx->models->LINEN_REMOTE_FETCHED=false;
  sexos_line("[linen.capture.publish.rejected] reason=invalid_or_exhausted preserve_view=1");
  return;
 }
 if(ctx->models->SELECTED_LINEN_OBJECT_ID!=previous && ctx->models->SELECTED_LINEN_OBJECT_ID!=0)
  sexos_line("[linen.object_select.current] id=%llu",(unsigned long long)ctx->models->SELECTED_LINEN_OBJECT_ID);
 sexos_line("[linen.capture.publish.ok] count=%zu revision=%llu selection_preserved=%u",ctx->linen->capture_snapshot.count,(unsigned long long)ctx->linen->publication_revision,(unsigned)(ctx->models->SELECTED_LINEN_OBJECT_ID==previous && previous!=0));
 shell_models_peer_paint_fast(ctx);
}


