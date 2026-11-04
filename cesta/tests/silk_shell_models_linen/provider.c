#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "silk_shell_models_linen.h"
#include "linen_capture_transport.h"
static SilkShellModelsState models;
static ShellLinenRuntime linen;
static ShellLinenModelsContext ctx={&models,&linen};
static uint64_t status;
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);va_end(ap);putchar('\n');}
void sexos_fail(void){exit(88);}
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c){printf("call %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",slot,op,a,b,c);return(SexosReply){status,0};}
void shell_models_peer_paint_fast(ShellLinenModelsContext *view){printf("paint %" PRIu64 " %u\n",view->models->SELECTED_LINEN_OBJECT_ID,(unsigned)view->models->LINEN_REMOTE_REAL);}
LinenCaptureStep linen_capture_transport_step(LinenCaptureClient *state,uint64_t slot){(void)state;(void)slot;return(LinenCaptureStep){0};}
static void trace(void){
 printf("S %" PRIu64 " %u %u",models.SELECTED_LINEN_OBJECT_ID,(unsigned)models.LINEN_REMOTE_FETCHED,(unsigned)models.LINEN_REMOTE_REAL);
 for(size_t i=0;i<LINEN_MAX_OBJECTS;++i){LinenObject o=models.LINEN_OBJECTS[i];if(!o.present){printf(" -");continue;}
  printf(" %" PRIu64 ":%u:%u:%u:%u:",o.object_id,(unsigned)o.kind,(unsigned)o.state,(unsigned)o.name_len,(unsigned)o.disk_path);
  for(size_t j=0;j<24;++j)printf("%02x",o.name[j]);
  printf(":%" PRIu64 ":%" PRIu64 ":%" PRIu64 ":%" PRIu64 ":%u:%s",o.parent_id,o.project_id,o.grant_ref,o.linked_surface_id,o.flags,o.display_name?o.display_name:"");
 }
 printf(" |pending %u %u\n",(unsigned)shell_linen_snapshot_busy(&linen.snapshot),(unsigned)shell_linen_snapshot_can_send(&linen.snapshot));
}
int main(void){
 char line[512];while(fgets(line,sizeof(line),stdin)){
  uint64_t n[6]={0};char *at=line,*end;for(size_t i=0;i<6;++i){n[i]=strtoull(at,&end,10);if(at==end)break;at=end;}
  switch(n[0]){
   case 0:memset(&models,0,sizeof(models));linen=shell_models_linen_new();status=0;for(size_t i=0;i<6;++i)models.LINEN_OBJECTS[i]=LINEN_SEED_OBJECTS[i];break;
   case 1:shell_models_fetch_remote_snapshot(&ctx);break;
   case 2:shell_models_queue_open(&ctx,n[1],n[2]);break;
   case 3:shell_models_linen_async_poll(&ctx);break;
   case 4:{SexosMessage m={.caller_pd=(uint32_t)n[1],.caller_task=700,.type_id=n[2],.arg0=n[3]};printf("R %u\n",(unsigned)shell_models_linen_async_reply(&ctx,&m));break;}
   case 5:models.SELECTED_LINEN_OBJECT_ID=n[1];break;
   case 6:status=n[1];break;
   default:return 3;
  }trace();
 }return 0;
}
