#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "silk_shell_models_paint.h"
#include "linen_capture_transport.h"
static SilkShellModelsState models;
static SilkShellInputState input;
static SilkShellChromeState chrome;
static ShellChromeApps apps;
static SilkShellProofsAState proofs;
static ShellLinenRuntime linen;
static ShellLinenRenderContext ctx={{&chrome,&models,&apps,&input,&proofs},&linen};
static ShellLinenModelsContext remote={&models,&linen};
static const char fallback[]="Fallback label with a deliberately long but static lifetime";
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);va_end(ap);putchar('\n');}
void sexos_fail(void){exit(88);}
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c){printf("D %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",slot,op,a,b,c);return(SexosReply){0,0};}
void shell_chrome_set_frame_top_bar(ShellChromeContext *view,uint32_t id,bool value){(void)view;printf("T %u %u\n",id,(unsigned)value);}
void shell_chrome_send_frame_tab_info(ShellChromeContext *view,uint32_t id){(void)view;printf("F %u\n",id);}
ShellTextAdmission shell_text_display_admit(ShellTextOperation op){printf("O %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",op.opcode,op.surface,op.packed,op.control);return(ShellTextAdmission){SHELL_TEXT_ACCEPTED,0};}
LinenCaptureStep linen_capture_transport_step(LinenCaptureClient *state,uint64_t slot){(void)state;(void)slot;return(LinenCaptureStep){0};}
void shell_models_peer_paint_fast(ShellLinenModelsContext *view){if(view->models!=&models||view->linen!=&linen)exit(88);shell_models_paint_surface_fast(&ctx);}
static void batch(const ShellTextBatch *b){printf(" %zu %u %zu %u %u %u %zu %zu %zu %" PRIu64 " ",b->text.len,b->color,b->offset,b->sequence,(unsigned)b->cleared,(unsigned)b->awaiting,b->metadata.count,b->metadata.first,b->metadata.rows,b->metadata.selected);for(size_t i=0;i<b->text.len;++i)printf("%02x",b->text.bytes[i]);}
static void trace(void){printf("S %u %u %u",linen.text.sequence,(unsigned)linen.text.active,(unsigned)linen.text.latest);if(linen.text.active)batch(&linen.text.current);putchar('|');if(linen.text.latest)batch(&linen.text.desired);putchar('\n');}
int main(void){char line[512];while(fgets(line,sizeof(line),stdin)){
 uint64_t n[12]={0};char *at=line,*end;for(size_t i=0;i<12;++i){n[i]=strtoull(at,&end,10);if(at==end)break;at=end;}
 switch(n[0]){
 case 0:memset(&models,0,sizeof(models));memset(&input,0,sizeof(input));linen=shell_models_linen_new();break;
 case 1:input.SURFACE_200_W=(uint32_t)n[1];input.SURFACE_200_H=(uint32_t)n[2];input.SURFACE_200_X=(int32_t)(uint32_t)n[3];input.SURFACE_200_Y=(int32_t)(uint32_t)n[4];models.SELECTED_LINEN_OBJECT_ID=n[5];models.LINEN_UI_SELECTED=(uint8_t)n[6];break;
 case 2:if(n[1]>=16)return 88;models.LINEN_OBJECTS[n[1]]=(LinenObject){.present=true,.object_id=n[2],.display_name=fallback,.name_len=(uint8_t)n[3]};for(size_t i=0;i<24;++i)models.LINEN_OBJECTS[n[1]].name[i]=(uint8_t)(n[4]?i*13+n[1]*7:65+(i+n[1])%26);break;
 case 3:shell_models_render_object_list(&ctx);break;
 case 4:shell_models_render_static_ui(&ctx);break;
 case 5:{ShellTextPoll p=shell_linen_text_poll(&linen,n[1]);printf("P %u\n",p.kind);break;}
 case 6:{ShellTextReceipt r={0};bool ok=shell_linen_text_receive(&linen,(uint32_t)n[1],1,n[2],&r);printf("R %u %zu %zu %zu %zu %" PRIu64 "\n",(unsigned)ok,r.bytes,r.metadata.count,r.metadata.first,r.metadata.rows,r.metadata.selected);break;}
 case 7:shell_linen_text_cancel(&linen);break;
 case 8:shell_models_paint_surface(&ctx,n[1]!=0);break;
 case 9:shell_models_paint_surface_fast(&ctx);break;
 case 10:models.LINEN_REMOTE_FETCHED=n[1]!=0;models.LINEN_REMOTE_REAL=n[2]!=0;break;
 case 11:shell_models_linen_async_poll(&remote);break;
 case 12:{SexosMessage m={.caller_pd=(uint32_t)n[1],.caller_task=700,.type_id=n[2],.arg0=n[3]};printf("A %u\n",(unsigned)shell_models_linen_async_reply(&remote,&m));break;}
 default:return 3;
 }trace();
 }return 0;}
