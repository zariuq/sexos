/*Bounded value extraction from the published static-label record array.*/
#include "silk_shell_models_render.h"
ShellLinenItems shell_models_peer_list_items(ShellLinenRenderContext *ctx)
{
 ShellLinenItems items={0};
 for(size_t i=0;i<LINEN_MAX_OBJECTS;++i){
  const LinenObject *object=&ctx->shell.models->LINEN_OBJECTS[i];if(!object->present)continue;
  if(items.count>=SHELL_LINEN_LIST_CAPACITY){sexos_fail();for(;;){}}
  ShellLinenListItem *out=&items.items[items.count++];out->id=object->object_id;
  if(object->name_len){out->len=object->name_len<24?object->name_len:24;for(size_t j=0;j<out->len;++j)out->name[j]=object->name[j];}
  else{
   if(!object->display_name){sexos_fail();for(;;){}}
   while(out->len<SHELL_LINEN_LIST_NAME_CAPACITY && object->display_name[out->len]){out->name[out->len]=(uint8_t)object->display_name[out->len];++out->len;}
  }
 }
 return items;
}


