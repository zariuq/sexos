#include "sexos_history_recovery.h"
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
static uint64_t v[20];static HistoryGrant supplied;
static struct {uint64_t kind;HistoryReply reply;} calls[68];static size_t call_count;
HistoryReply history_query_provider(HistoryQueryContext context,uint64_t kind)
{
    assert(context.slot==53 && context.cookie==0x1ee7 && call_count<68);
    HistoryReply r=history_query(supplied,(uint32_t)v[5],kind);
    if(v[14] && kind==v[14]-1){
        if(!v[15] && v[17])r=(HistoryReply){0};
        switch(v[15]){case 0:r.status=v[16];break;case 1:r.value0=v[16];break;
            case 2:r.value1=v[16];break;case 3:r.value2=v[16];break;case 4:r.value3=v[16];break;default:assert(false);}
    }
    calls[call_count].kind=kind;calls[call_count++].reply=r;return r;
}
int main(void)
{
    while(scanf("%"SCNu64,&v[0])==1){
        for(size_t i=1;i<20;++i)if(scanf("%"SCNu64,&v[i])!=1)return 2;
        supplied=(HistoryGrant){.holder=(uint32_t)v[1],.logical=(uint32_t)v[2],.base=v[3],.sectors=v[4]};
        HistoryTargets targets={0};size_t n=HISTORY_ROSTER_COUNT;
        for(size_t i=0;i<n;++i){supplied.participants[i]=(HistoryParticipant){12+(uint32_t)i,1012+13*(uint32_t)i};targets.values[i]=12+(uint32_t)i;}
        if(v[7] && v[7]-1<n)supplied.participants[v[7]-1]=(HistoryParticipant){(uint32_t)v[8],(uint32_t)v[9]};
        if(v[12] && v[12]-1<n)targets.values[v[12]-1]=(uint32_t)v[13];
        call_count=0;HistoryReply r={0};HistoryGrant result=supplied;
        if(v[0]){
            HistoryDecode d=history_decode_for((uint32_t)v[10],(uint32_t)v[11],targets,(HistoryQueryContext){53,0x1ee7});result=d.value;
            r=(HistoryReply){d.status,result.logical,result.holder,result.base,result.sectors};
        }else r=history_query(supplied,(uint32_t)v[5],v[6]);
        HistoryFound found=history_participant(supplied,(uint32_t)v[18]);
        printf("%"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64"|V%u|P%u:%u:%u:%u|G%u:%u:%"PRIu64":%"PRIu64,
            r.status,r.value0,r.value1,r.value2,r.value3,history_valid(supplied),found.present,found.value.target,found.value.task,
            history_permits(supplied,(uint32_t)v[5],(uint32_t)v[18],(uint32_t)v[19]),result.holder,result.logical,result.base,result.sectors);
        for(size_t i=0;i<n;++i)printf(":%u:%u",result.participants[i].target,result.participants[i].task);
        for(size_t i=0;i<call_count;++i){HistoryReply q=calls[i].reply;printf("|Q%"PRIu64":%"PRIu64":%"PRIu64":%"PRIu64":%"PRIu64":%"PRIu64,calls[i].kind,q.status,q.value0,q.value1,q.value2,q.value3);}
        putchar('\n');
    }
    return 0;
}
