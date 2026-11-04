#include "sexos_app_private.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static AppPrivatePages image;
static uint64_t leaves[APP_PRIVATE_MAX_PAGES+1];
static struct {char kind; uint64_t a,b;} events[APP_PRIVATE_MAX_PAGES*3];
static size_t event_count;
uint64_t app_private_leaf_read(uint64_t leaf)
{
    uint64_t value=leaf<=APP_PRIVATE_MAX_PAGES ? leaves[leaf] : 0;
    events[event_count].kind='R';events[event_count].a=leaf;events[event_count++].b=value;
    return value;
}
void app_private_leaf_write(uint64_t leaf,uint64_t value)
{
    events[event_count].kind='W';events[event_count].a=leaf;events[event_count++].b=value;
    if (leaf<=APP_PRIVATE_MAX_PAGES) leaves[leaf]=value;
}
void app_private_leaf_flush(uint64_t va)
{
    events[event_count].kind='F';events[event_count].a=va;events[event_count++].b=0;
}
int main(void)
{
    uint64_t v[24];
    while (scanf("%" SCNu64,&v[0])==1) {
        for (unsigned i=1;i<24;++i) if (scanf("%" SCNu64,&v[i])!=1) return 2;
        event_count=0; uint64_t out[4]={0};
        switch(v[0]) {
        case 0:
            memset(&image,0,sizeof image);memset(leaves,0,sizeof leaves);image.count=(size_t)v[1];
            for (size_t i=0;i<image.count && i<APP_PRIVATE_MAX_PAGES;++i) {
                image.pages[i]=(AppPrivatePage){(i+1)*4096,i+1,((i+1)*4096)|5};
                leaves[i+1]=image.pages[i].expected|0x60;
            }
            break;
        case 1:
            if (v[1]>=APP_PRIVATE_MAX_PAGES) return 3;
            image.pages[v[1]]=(AppPrivatePage){v[2],v[3],v[4]};
            if (v[3]<=APP_PRIVATE_MAX_PAGES) leaves[v[3]]=v[5];
            break;
        case 2: case 3: {
            AppPrivateResult r=v[0]==2 ? app_private_retire(&image) : app_private_restore(&image);
            out[0]=r.status;out[1]=r.count;break;
        }
        case 4:
            out[0]=app_lifecycle_authorize((AppLifecycleGrant){(uint32_t)v[1],(uint32_t)v[2],(uint32_t)v[3],(uint8_t)v[4],(uint8_t)v[5]},
                (uint32_t)v[6],(uint32_t)v[7],(uint32_t)v[8],(uint8_t)v[9]);break;
        case 5: {
            AppNextIncarnation n=app_incarnation_next((AppIncarnation){(uint32_t)v[1],(uint32_t)v[2]});
            out[0]=n.present;out[1]=n.value.logical_app;out[2]=n.value.number;break;
        }
        case 6:
            out[0]=app_activation_authorize((AppActivationGrant){(uint32_t)v[1],(uint32_t)v[2],(uint32_t)v[3],(uint32_t)v[4],(uint32_t)v[5],(uint32_t)v[6],(uint32_t)v[7]},
                (uint32_t)v[8],(uint32_t)v[9],(uint32_t)v[10],(uint32_t)v[11],(uint32_t)v[12],
                (AppIncarnation){(uint32_t)v[13],(uint32_t)v[14]},(AppIncarnation){(uint32_t)v[15],(uint32_t)v[16]});break;
        case 7: out[0]=app_private_final_flags(v[1],(uint8_t)v[2]);break;
        case 8: if (v[1]<=APP_PRIVATE_MAX_PAGES) leaves[v[1]]=v[2];break;
        case 9: {AppIncarnation n=app_incarnation_original((uint32_t)v[1]);out[0]=n.logical_app;out[1]=n.number;break;}
        default:return 4;
        }
        printf("out %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " E",out[0],out[1],out[2],out[3]);
        for (size_t i=0;i<event_count;++i) printf(" %c:%" PRIu64 ":%" PRIu64,events[i].kind,events[i].a,events[i].b);
        printf(" L");for (size_t i=0;i<=APP_PRIVATE_MAX_PAGES;++i) if(leaves[i]) printf(" %zu:%" PRIu64,i,leaves[i]);
        putchar('\n');
    }
    return 0;
}
