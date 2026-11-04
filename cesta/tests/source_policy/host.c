#include "sexos_source_policy.h"
#include <stdio.h>
#include <inttypes.h>
static uint64_t v[32];static unsigned features,samples,pairs;
bool source_entropy_available(void){++features;return v[13]!=0;}
SourceWord source_entropy_sample(void){++samples;return (SourceWord){samples==v[14],v[15]};}
SourceClockPair source_clock_pair(void)
{
    unsigned mode=(unsigned)(v[16]>>(pairs*2))&3;++pairs;
    SourceClockRaw a={(uint8_t)v[18],(uint8_t)v[19],(uint8_t)v[20],(uint8_t)v[21],(uint8_t)v[22],(uint8_t)v[23],(uint8_t)v[24],(uint8_t)v[25],(uint8_t)v[26],(uint8_t)v[27]},b=a;
    if(mode==2) {
        uint8_t *byte=(uint8_t *)&b;
        if(v[28]>=10)return (SourceClockPair){0};
        byte[v[28]]^=(uint8_t)v[29];
    }
    if(mode==3){a.status_d=0;b=a;}
    return (SourceClockPair){mode!=0,a,b};
}
int main(void)
{
    _Static_assert(sizeof(SourceClockRaw)==10,"raw RTC byte schema");
    while(scanf("%"SCNu64,&v[0])==1){
        for(size_t i=1;i<32;++i)if(scanf("%"SCNu64,&v[i])!=1)return 2;
        features=samples=pairs=0;
        SourceGrant g={v[1]!=0,{(uint32_t)v[2],(uint32_t)v[3]},{(uint32_t)v[4],(uint32_t)v[5]},(uint32_t)v[6]};
        SourceIdentity caller={(uint32_t)v[7],(uint32_t)v[8]};SourceReply r={0};
        if(v[0]==0)r=source_entropy_invoke(g,caller,v[9],v[10],v[11],v[12]);
        else if(v[0]==1)r=source_clock_invoke(g,caller,v[9],v[10],v[11],v[12]);
        else if(v[0]==2)r=source_trust_invoke(g,caller,v[9],v[10],v[11],v[12],(SourceTrustPolicy){1,1,376});
        else if(v[0]==3){
            SourceClockRaw raw={(uint8_t)v[18],(uint8_t)v[19],(uint8_t)v[20],(uint8_t)v[21],(uint8_t)v[22],(uint8_t)v[23],(uint8_t)v[24],(uint8_t)v[25],(uint8_t)v[26],(uint8_t)v[27]};
            SourceWord value=source_clock_seconds(raw);r=(SourceReply){0,value.present,value.value,0,0};
        }else return 3;
        printf("%"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %u %u %u\n",r.status,r.value0,r.value1,r.value2,r.value3,features,samples,pairs);
    }
    return 0;
}
