#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "silk_shell_custody.h"

static void state(const ShellAppCustody *e)
{
    printf("S %zu %u %u %" PRIu64 " %u %u %u %u %u %u %" PRIu64 " %u %u %u %d %d %u %u 0\n",
        e->slot,e->target,e->task,e->surface,e->pending,e->dead,e->retired,e->acked,
        e->close_pending,e->has_close_attempt,e->has_close_attempt?e->close_attempt:0,
        e->close_declined,e->registration_reply,e->has_bounds,
        e->has_bounds?e->x:0,e->has_bounds?e->y:0,e->has_bounds?e->w:0,e->has_bounds?e->h:0);
}
static void row(ShellAppCatalogRow r)
{
    if(!r.present){puts("R 0");return;}
    printf("R 1 %u %u %u %" PRIu64 " %" PRIu64 " %zu %" PRIu64 " %u %zu %" PRIu64 " %zu %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
        r.app,r.ordinal,r.target,r.surface,r.shell_surface,r.lifecycle_slot,r.close_slot,
        r.has_activation,r.activation_slot,r.route_slot,r.child_count,r.children[0],r.children[1],r.children[2]);
}
static void word(ShellCustodyWord w) {printf("R %u %" PRIu64 "\n",w.present,w.present?w.value:0);}
int main(void)
{
    ShellAppCustody e=shell_custody_new(0,0,0,0);
    char line[1024];
    while(fgets(line,sizeof(line),stdin)) {
        uint64_t n[9]={0};size_t count=0;
        for(char *p=line;*p && count<9;) {
            char *end;unsigned long long v=strtoull(p,&end,10);
            if(end==p)break;
            n[count++]=v;p=end;
        }
        if(!count)continue;
        switch(n[0]) {
            case 100:e=shell_custody_new(n[1],n[2],n[3],n[4]);puts("R");break;
            case 101:{uint64_t f=n[1];e.pending=f&1;e.dead=f&2;e.retired=f&4;e.acked=f&8;
                e.close_pending=f&16;e.has_close_attempt=f&32;e.close_attempt=n[2];
                e.close_declined=f&64;e.registration_reply=f&128;e.has_bounds=f&256;
                e.x=(int32_t)n[3];e.y=(int32_t)n[4];e.w=n[5];e.h=n[6];puts("R");break;}
            case 0:word(shell_custody_shell_surface(&e));break;
            case 1:printf("R %u\n",shell_custody_is_successor_frame(&e,n[1]));break;
            case 2:printf("R %u\n",shell_custody_owns_surface(&e,n[1]));break;
            case 3:word(shell_custody_close_slot(&e));break;
            case 4:printf("R %u\n",shell_custody_request_close(&e));break;
            case 5:printf("R %u\n",shell_custody_close_due(&e,n[1]));break;
            case 6:shell_custody_attempted_close(&e,n[1]);puts("R");break;
            case 7:printf("R %" PRIu64 "\n",shell_custody_close_kind(&e));break;
            case 8:printf("R %u\n",shell_custody_decline_close(&e,n[1],n[2],n[3],n[4]));break;
            case 9:printf("R %u\n",shell_custody_hint(&e,n[1],n[2],n[3],n[4]));break;
            case 10:printf("R %u\n",shell_custody_attest(&e,n[1],n[2],n[3]));break;
            case 11:printf("R %u\n",shell_custody_allows_operation(&e,n[1],n[2],n[3],n[4],n[5]));break;
            case 12:printf("R %u\n",shell_custody_allows(&e,n[1],n[2],n[3]));break;
            case 13:printf("R %u\n",shell_custody_finish_ack(&e));break;
            case 14:printf("R %u\n",shell_custody_buffer_sender(n[1]));break;
            case 15:row(shell_custody_catalog_at(n[1]));break;
            case 16:row(shell_custody_catalog_find(n[1],n[2]));break;
            default:abort();
        }
        state(&e);
    }
    return 0;
}
