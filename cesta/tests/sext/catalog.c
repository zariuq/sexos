#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sext_service.c"
static SextCatalog catalog[SEXOS_APP_CATALOG_COUNT];
static uint64_t slots[SEXOS_APP_CATALOG_COUNT];
static size_t queries;
uint64_t sext_native_catalog(uint64_t slot,SextCatalog *out)
{
    ++queries;for(size_t i=0;i<SEXOS_APP_CATALOG_COUNT;++i)if(slots[i]==slot){*out=catalog[i];return 0;}
    return SEXT_INVALID;
}
uint64_t sexos_reply(uint32_t target,uint64_t value){(void)target;(void)value;abort();}
void sexos_line(const char *format,...){(void)format;abort();}
int main(void)
{
    size_t count=0;if(scanf("%zu",&count)!=1 || count!=SEXOS_APP_CATALOG_COUNT)return 2;
    for(size_t i=0;i<count;++i) {
        unsigned target=0;unsigned long long surface=0,slot=0;
        if(scanf("%u %llu %llu",&target,&surface,&slot)!=3)return 3;
        slots[i]=(uint64_t)slot;catalog[i]=(SextCatalog){target,target,(uint64_t)surface,3};
        assert(sext_lifecycle_slot(i)==slots[i]);
    }
    assert(sext_lifecycle_slot(count)==0 && sext_lifecycle_slot(SIZE_MAX)==0);
    for(size_t i=0;i<count;++i) {
        SextService s={0};queries=0;
        SexosMessage notice={.type_id=0x107,.arg0=catalog[i].target_pd,.arg1=catalog[i].task_id,.caller_pd=0};
        sext_service_accept(&s,notice);assert(queries==count);
        for(size_t j=0;j<count;++j)assert(s.app_pending[j].present==(i==j));
        assert(s.app_pending[i].target_pd==catalog[i].target_pd && s.app_pending[i].task_id==catalog[i].task_id);
        s=(SextService){0};queries=0;notice.caller_pd=13;sext_service_accept(&s,notice);assert(queries==0);
        for(size_t j=0;j<count;++j)assert(!s.app_pending[j].present);
    }
    printf("catalog slots and authenticated notice matching passed:%zu\n",count);return 0;
}
