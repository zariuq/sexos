#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "silk_shell_viewport.h"
static uint64_t status;
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c){
    printf("SEND %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",slot,op,a,b,c);
    return (SexosReply){status,0};
}
static void state(const ShellViewportDelivery *d,const ShellWorkspaceRefresh *r){
    ShellViewportNotice n=d->desired?d->notice:(ShellViewportNotice){0};
    printf("D %" PRIu64 " %u %u %u %u %u %u %u %" PRIu64 " %u %u\n",d->sequence,d->dead,d->desired,d->last,n.target,n.task,n.w,n.h,n.sequence,d->last?d->last_w:0,d->last?d->last_h:0);
    printf("W %u %u %u %u\n",r->target,r->task,r->pending,r->closed);
}
int main(void){
    unsigned kind=0;ShellDocumentDelivery document=shell_document_delivery_new();ShellSpindleDelivery spindle=shell_spindle_delivery_new();
    ShellWorkspaceRefresh refresh=shell_workspace_refresh_new(0,0);char line[2048];
    while(fgets(line,sizeof(line),stdin)){
        uint64_t n[12]={0};size_t count=0;
        for(char *p=line;*p&&count<12;){char *end;unsigned long long value=strtoull(p,&end,10);if(p==end)break;n[count++]=value;p=end;}
        if(!count)continue;
        switch(n[0]){
        case 100:kind=n[1];document=shell_document_delivery_new();spindle=shell_spindle_delivery_new();refresh=shell_workspace_refresh_new(n[2],n[3]);break;
        case 101:if(!kind)document.state.sequence=n[1];else spindle.state.sequence=n[1];break;
        case 0:if(!kind)shell_document_delivery_queue(&document,n[1],n[2],n[3],n[4]);else shell_spindle_delivery_queue(&spindle,n[1],n[2],n[3],n[4]);break;
        case 1:if(!kind)shell_document_delivery_refresh(&document);else shell_spindle_delivery_refresh(&spindle);break;
        case 2:printf("R %u\n",!kind?shell_document_delivery_pending(&document):shell_spindle_delivery_pending(&spindle));break;
        case 3:if(!kind)shell_document_delivery_retire(&document);else shell_spindle_delivery_retire(&spindle);break;
        case 4:status=n[4];if(!kind)shell_document_delivery_poll(&document,n[1],n[2],n[3]);else shell_spindle_delivery_poll(&spindle,n[1],n[2],n[3]);break;
        case 5:{ShellViewportNotice a={0};bool valid=shell_viewport_decode(kind,n[1],n[2],n[3],n[4],n[5],n[6],n[7],&a);if(valid)printf("R 1 %u %u %u %u %" PRIu64 "\n",a.target,a.task,a.w,a.h,a.sequence);else puts("R 0");break;}
        case 10:refresh=shell_workspace_refresh_new(n[1],n[2]);break;
        case 11:shell_workspace_refresh_changed(&refresh);break;
        case 12:printf("R %u\n",shell_workspace_refresh_pending(&refresh));break;
        case 13:shell_workspace_refresh_close(&refresh);break;
        case 14:status=n[2];shell_workspace_refresh_poll(&refresh,n[1]);break;
        case 15:{ShellSpindleLayout a=shell_spindle_layout(n[1],n[2]);printf("R %zu %zu %zu %zu %zu",a.cols,a.rows[0],a.rows[1],shell_spindle_columns(&a),shell_spindle_lines(&a));for(size_t i=0;i<4;++i){ShellViewRect p=a.panes[i];printf(" %u %u %u %u",p.x,p.y,p.w,p.h);}puts("");break;}
        case 16:{ShellDocumentLayout a=shell_document_layout(n[1],n[2],n[3]);printf("R %u %u %u %zu %zu %u %zu %zu %zu %zu %zu %zu %u\n",a.w,a.h,a.scale,a.cols,a.rows,a.top,a.toolbar_rows,shell_document_header_rows(&a),shell_document_toolbar_per_row(&a),shell_document_hint_rows(&a),shell_document_chrome_rows(&a),shell_document_body_rows(&a),shell_document_hit(&a,n[4],n[5]));break;}
        case 17:{ShellViewChildren a=shell_desktop_children(n[1],n[2],n[3],n[4],n[5]);printf("R");for(size_t i=0;i<4;++i){ShellViewChild c=a.values[i];if(c.present)printf(" 1 %" PRIu64 " %d %d %u %u",c.sid,c.x,c.y,c.w,c.h);else printf(" 0");}puts("");break;}
        case 18:{ShellCustodyWord a=shell_desktop_parent_frame(n[1]);printf("R %u %" PRIu64 "\n",a.present,a.value);break;}
        case 19:{ShellDocumentLayout a=shell_document_layout(9,512,512);a.scale=n[1];printf("R %u\n",shell_document_hit(&a,8,48));break;}
        default:abort();}
        state(!kind?&document.state:&spindle.state,&refresh);
    }
    return 0;
}
