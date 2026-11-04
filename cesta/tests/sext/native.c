#include "sext_service.h"
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>
uint64_t sext_abi_inputs[12],sext_abi_outputs[6],sext_abi_count;
int main(void){
    uint64_t v[12];
    for(;;){
        for(size_t i=0;i<12;++i){int n=scanf("%"SCNu64,&v[i]);if(!i&&n==EOF)return 0;assert(n==1);}
        uint64_t kind=v[0],a=v[1],b=v[2],c=v[3],d=v[4],out[6]={0};
        for(size_t i=0;i<6;++i)sext_abi_outputs[i]=v[i+6];
        sext_abi_count=0;
        if(kind==0){SextPagingInfo r={0};out[0]=sext_native_info(a,&r);if(!out[0]){out[1]=r.base;out[2]=r.length;out[3]=r.target_pd;out[4]=r.rights;}}
        else if(kind==1){SextFaultQuery r={0};out[0]=sext_backend_query(a,b,&r);if(!out[0]){out[1]=r.page_va;out[2]=r.error_code;out[3]=r.target_pd;out[4]=r.allowed_rights;}}
        else if(kind==2)out[0]=sext_backend_commit(a,(size_t)b,c,(uint8_t)d);
        else if(kind==3)out[0]=sext_backend_reject(a,b);
        else if(kind==4)out[0]=sext_backend_reclaim(a,(size_t)b,c);
        else if(kind==5){SextCleanupResult r={0};out[0]=sext_backend_cleanup(a,&r);if(!out[0]){out[1]=r.frame_bitmap;out[2]=r.task_id;}}
        else if(kind==6){SextCatalog r={0};out[0]=sext_native_catalog(a,&r);if(!out[0]){out[1]=r.target_pd;out[2]=r.task_id;out[3]=r.surface_id;out[4]=r.coordinator_pd;}}
        else if(kind==7){SextRetiredPool r={0};out[0]=sext_native_retire(a,&r);if(!out[0]){out[1]=r.base;out[2]=r.length;out[3]=r.task_id;}}
        else if(kind==8){SexosMessage r=sexos_listen_once(a);out[0]=r.type_id;out[1]=r.arg0;out[2]=r.arg1;out[3]=r.arg2;out[4]=r.caller_pd;out[5]=r.caller_task;}
        else if(kind==9){SexosMessage r={UINT64_MAX,91,92,93,UINT32_MAX,UINT32_MAX};out[0]=sexos_try_listen(a,&r);out[1]=r.type_id;out[2]=r.arg0;out[3]=r.arg1;out[4]=r.caller_pd;out[5]=r.caller_task;}
#if defined(SEXOS_PDX_SENDER_TASK) && SEXOS_PDX_SENDER_TASK
        else if(kind==10)sexos_wait_self();
#endif
        else assert(!"unknown ABI case");
        printf("out");for(size_t i=0;i<6;++i)printf(" %"PRIu64,out[i]);printf(" traps %"PRIu64,sext_abi_count);
        for(size_t t=0;t<sext_abi_count;++t){printf(" in");for(size_t i=0;i<6;++i)printf(" %"PRIu64,sext_abi_inputs[t*6+i]);}
        putchar('\n');
    }
}

