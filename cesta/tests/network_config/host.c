#include "sexos_network_config.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
static uint64_t hash(const char *p,size_t n)
{
    uint64_t h=UINT64_C(14695981039346656037);
    for(size_t i=0;i<n;++i)h=(h^(uint8_t)p[i])*UINT64_C(1099511628211);
    return h;
}
int main(void)
{
    uint64_t v[16];
    while(scanf("%"SCNu64,&v[0])==1){
        for(size_t i=1;i<16;++i)if(scanf("%"SCNu64,&v[i])!=1)return 2;
        NetConfig c={.local=(uint32_t)v[1],.prefix=(uint8_t)v[2],.gateway=(uint32_t)v[3],.dns=(uint32_t)v[4],
            .dns_port=(uint16_t)v[5],.http=(uint32_t)v[6],.http_port=(uint16_t)v[7],.listen_port=(uint16_t)v[8],
            .source_base=(uint16_t)v[9],.profile=(uint8_t)v[10]};
        bool present=true;
        if(v[0]==1){NetConfigResult r=net_config_from_words((NetConfigWords){v[12],v[13],v[14],v[15]});present=r.present;c=r.value;}
        else if(v[0]==2)c=net_config_controlled();
        if(!present){puts("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");continue;}
        const char *name=net_config_hostname(c);NetConfigEndpoint tls=net_config_tls_endpoint(c);
        NetConfigWords w=net_config_to_words(c);NetConfigResult roundtrip=net_config_from_words(w);
        printf("1 %u %u %u %zu %"PRIu64" %u %u %u %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %u %"PRIu64" %"PRIu64"\n",
            net_config_valid(c),net_config_contains(c,(uint32_t)v[11]),name!=NULL,name?strlen(name):0,name?hash(name,strlen(name)):0,
            tls.present,tls.address,tls.port,w.word0,w.word1,w.word2,w.word3,roundtrip.present,
            hash(NET_CONFIG_DNS_NAME,NET_CONFIG_DNS_NAME_LENGTH),hash(NET_CONFIG_DNS_QUESTION,NET_CONFIG_DNS_QUESTION_LENGTH));
    }
    return 0;
}
