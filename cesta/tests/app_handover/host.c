#include "sexos_app_private.h"
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
enum { LEAVES=2*APP_PRIVATE_MAX_PAGES+2 };
static uint64_t leaves[LEAVES];
static struct {char kind;uint64_t a,b;} effects[4*APP_PRIVATE_MAX_PAGES+1];
static size_t effect_count;
uint64_t app_private_leaf_read(uint64_t leaf){return leaf<LEAVES?leaves[leaf]:UINT64_MAX;}
void app_private_leaf_write(uint64_t leaf,uint64_t value)
{
    assert(leaf>0 && leaf<LEAVES && effect_count<4*APP_PRIVATE_MAX_PAGES);
    leaves[leaf]=value;effects[effect_count].kind='W';effects[effect_count].a=leaf;effects[effect_count++].b=value;
}
void app_private_leaf_flush(uint64_t va)
{
    assert(effect_count<4*APP_PRIVATE_MAX_PAGES);effects[effect_count].kind='F';effects[effect_count].a=va;effects[effect_count++].b=0;
}
int main(void)
{
    uint64_t v[12];
    while(scanf("%"SCNu64,&v[0])==1){
        for(size_t i=1;i<12;++i)if(scanf("%"SCNu64,&v[i])!=1)return 2;
        if(v[0]){printf("%u\n",app_notice_due((uint8_t)v[1],(uint8_t)v[2],v[3],v[4],(uint8_t)v[5]));continue;}
        AppPrivatePages p={.count=(size_t)v[1]},q={.count=(size_t)v[1]};
        for(size_t i=0;i<LEAVES;++i)leaves[i]=0;
        effect_count=0;
        for(size_t i=0;i<p.count && i<APP_PRIVATE_MAX_PAGES;++i){
            uint64_t physical=(i+1)*4096;
            p.pages[i]=(AppPrivatePage){(i+1)*4096,i+1,physical|5|(UINT64_C(9)<<59)};
            q.pages[i]=(AppPrivatePage){(1024+i+1)*4096,512+i+1,physical|7|(UINT64_C(14)<<59)|(UINT64_C(1)<<63)};
            leaves[i+1]=p.pages[i].expected|v[5];
        }
        size_t at=(size_t)v[2];
        if(at<APP_PRIVATE_MAX_PAGES){
            switch(v[3]){
                case 0:break;case 1:p.pages[at].va=v[4];break;case 2:p.pages[at].leaf=v[4];break;case 3:p.pages[at].expected=v[4];break;
                case 4:q.pages[at].va=v[4];break;case 5:q.pages[at].leaf=v[4];break;case 6:q.pages[at].expected=v[4];break;
                case 7:leaves[at+1]=v[4];break;case 8:leaves[512+at+1]=v[4];break;case 9:q.count=(size_t)v[4];break;default:return 3;
            }
        }
        AppPrivateResult r=app_private_handover(&p,&q);
        printf("%"PRIu64" %zu",r.status,r.count);
        for(size_t i=0;i<effect_count;++i)printf("|%c:%"PRIu64":%"PRIu64,effects[i].kind,effects[i].a,effects[i].b);
        printf("|L");for(size_t i=1;i<LEAVES;++i)printf(":%"PRIu64,leaves[i]);putchar('\n');
    }
    return 0;
}
