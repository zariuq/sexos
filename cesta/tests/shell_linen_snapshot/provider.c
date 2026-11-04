#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell_linen_snapshot.h"
static ShellLinenSnapshot state;
void sexos_fail(void){exit(88);}
static void object(const ShellLinenObject *o){printf("%" PRIu64 ":%u:%u:%u:",o->id,(unsigned)o->kind,(unsigned)o->len,(unsigned)o->disk);for(size_t i=0;i<24;++i)printf("%02x",o->name[i]);}
static void trace(void){
 printf("S %u %u %u %u %u %u %zu %" PRIu64 " %" PRIu64 " ",(unsigned)state.phase,(unsigned)state.awaiting,(unsigned)state.refresh,(unsigned)state.rescan,(unsigned)state.slot,(unsigned)state.offset,state.count,state.phase==SHELL_LINEN_OPEN?state.open_id:0,state.phase==SHELL_LINEN_OPEN?state.open_choice:0);object(&state.current);
 printf(" |objects");for(size_t i=0;i<SHELL_LINEN_CAPACITY;++i){if(state.present[i]){putchar(' ');object(&state.objects[i]);}else printf(" -");}
 printf(" |opens %zu",state.open_count);for(size_t i=0;i<SHELL_LINEN_OPEN_CAPACITY;++i){ShellLinenOpen o=state.opens[i];if(o.present)printf(" %" PRIu64 ":%" PRIu64,o.id,o.choice);else printf(" -");}puts("");
}
int main(int argc,char **argv){
 state=shell_linen_snapshot_new();
 if(argc>1){
  if(!strcmp(argv[1],"sent-twice")){shell_linen_snapshot_refresh(&state,false);ShellLinenRequest out={0};shell_linen_snapshot_request(&state,&out);shell_linen_snapshot_sent(&state);}
  if(!strcmp(argv[1],"sent-idle") || !strcmp(argv[1],"sent-twice")){shell_linen_snapshot_sent(&state);return 90;}
  if(!strcmp(argv[1],"bad-phase"))state.phase=255;
  if(!strcmp(argv[1],"bad-count"))state.count=17;
  if(!strcmp(argv[1],"bad-open-count"))state.open_count=5;
  if(!strcmp(argv[1],"bad-offset"))state.offset=1;
  (void)shell_linen_snapshot_busy(&state);return 90;
 }
 char line[512];while(fgets(line,sizeof(line),stdin)){
  uint64_t n[10]={0};char *at=line,*end;for(size_t i=0;i<10;++i){n[i]=strtoull(at,&end,10);if(at==end)break;at=end;}
  switch(n[0]){
   case 0:state=shell_linen_snapshot_new();break;
   case 1:shell_linen_snapshot_refresh(&state,n[1]!=0);break;
   case 2:printf("R %u\n",(unsigned)shell_linen_snapshot_queue_open(&state,n[1],n[2]));break;
   case 3:printf("R %u %u\n",(unsigned)shell_linen_snapshot_busy(&state),(unsigned)shell_linen_snapshot_can_send(&state));break;
   case 4:{ShellLinenRequest r={0};if(shell_linen_snapshot_request(&state,&r))printf("R 1 %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",r.opcode,r.args[0],r.args[1],r.args[2]);else puts("R 0");break;}
   case 5:shell_linen_snapshot_sent(&state);break;
   case 6:{ShellLinenReply r=shell_linen_snapshot_reply(&state,(uint32_t)n[1],n[2],n[3]);printf("R %u %" PRIu64 "\n",(unsigned)r.kind,r.value);break;}
   case 7:printf("R %u\n",(unsigned)shell_workspace_allows(n[1],n[2],n[3],n[4],n[5],n[6]!=0,n[7]!=0,n[8]!=0));continue;
   default:return 3;
  }trace();
 }return 0;
}
