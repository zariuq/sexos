#include "sext.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
static SextFrameAllocator allocator;
static SextPager pager;
static bool ready;
static SextPagingInfo pool;
static SextMappings mappings;
static SextFaultQuery query;
static uint64_t query_status,commit_status,reject_status,reclaim_status,cleanup_status;
static SextCleanupResult cleanup_result;
static uint8_t memory[SEXT_FRAME_STORAGE*4096];
static uint64_t memory_hash(void) {
    uint64_t hash=UINT64_C(14695981039346656037); size_t bytes=(size_t)SEXT_FRAME_COUNT*4096;
    for(size_t i=0;i<bytes;++i) hash=(hash^memory[i])*UINT64_C(1099511628211);
    return hash;
}
uint64_t sext_backend_query(uint64_t slot,uint64_t token,SextFaultQuery *out) {
    printf("query %"PRIu64" %"PRIu64"\n",slot,token);
    if(!query_status)*out=query;
    return query_status;
}
void sext_backend_zero(uint64_t address,uint64_t bytes) {
    printf("zero %"PRIu64" %"PRIu64"\n",address,bytes);
    uint64_t size=(uint64_t)SEXT_FRAME_COUNT*4096;
    assert(address>=UINT64_C(0x400000) && address-UINT64_C(0x400000)<=size &&
           bytes<=size-(address-UINT64_C(0x400000)));
    memset(memory+(size_t)(address-UINT64_C(0x400000)),0,(size_t)bytes);
}
uint64_t sext_backend_commit(uint64_t slot,size_t index,uint64_t token,uint8_t rights) {
    printf("commit %"PRIu64" %zu %"PRIu64" %u\n",slot,index,token,(unsigned)rights);return commit_status;
}
uint64_t sext_backend_reject(uint64_t slot,uint64_t token) {
    printf("reject %"PRIu64" %"PRIu64"\n",slot,token);return reject_status;
}
uint64_t sext_backend_reclaim(uint64_t slot,size_t index,uint64_t token) {
    printf("reclaim %"PRIu64" %zu %"PRIu64"\n",slot,index,token);return reclaim_status;
}
uint64_t sext_backend_cleanup(uint64_t slot,SextCleanupResult *out) {
    printf("cleanup %"PRIu64"\n",slot);if(!cleanup_status)*out=cleanup_result;return cleanup_status;
}
static void binding(SextMaybeBinding b) {
    if(b.present)printf(" B %zu %"PRIu64" %u %"PRIu64,b.value.index,b.value.token,b.value.target,b.value.page);
    else printf(" -");
}
static void bindings(SextBindings b) {
    size_t length=SEXT_FRAME_COUNT;
    for(size_t i=0;i<length;++i)binding(b.entries[i]);
}
static void allocation(const SextFrameAllocator *a) {
    printf(" q%zu",a->quota);size_t length=SEXT_FRAME_COUNT;
    for(size_t i=0;i<length;++i) {
        SextFrame f=a->frames[i];
        printf(" F %u %"PRIu64" %u %"PRIu64" %u",(unsigned)f.state,f.token,f.target,f.page,(unsigned)f.zeroed);
    }
}
static void state(void) {
    printf("state A");allocation(&allocator);printf(" P%u",(unsigned)ready);
    if(ready) {
        printf(" pool %"PRIu64" %"PRIu64" %u %u",pager.pool.base,pager.pool.length,pager.pool.target_pd,(unsigned)pager.pool.rights);
        allocation(&pager.allocator);
        for(size_t i=0;i<2;++i) {
            SextMapping m=pager.mappings.entries[i];
            if(m.present)printf(" M %"PRIu64" %"PRIu64" %"PRIu64" %u %u",m.slot,m.info.base,m.info.length,m.info.target_pd,(unsigned)m.info.rights);
            else printf(" M-");
            SextActive a=pager.active[i];
            if(a.present) {
                printf(" A %"PRIu64" %"PRIu64" %"PRIu64" %u %u",a.notice.fault_addr,a.notice.error_code,a.notice.token,a.notice.caller_pd,(unsigned)a.may_have_published);
                if(a.commit)printf(" C %zu %u",a.index,(unsigned)a.rights);else printf(" R %u",(unsigned)a.reason);
            } else printf(" A-");
            SextRelease r=pager.releases[i];
            if(r.present) {
                printf(" L %"PRIu64" %"PRIu64" %u",r.request.page,r.request.request_id,r.request.caller_pd);binding(r.binding);
                if(r.done)printf(" D %"PRIu64,r.status);else printf(" P");
            } else printf(" L-");
            SextClosed x=pager.closed[i];
            if(x.present)printf(" X %"PRIu64" %"PRIu64,x.value.frame_bitmap,x.value.task_id);else printf(" X-");
        }
    }
    printf(" bytes %"PRIu64"\n",memory_hash());
}
static void outcome(SextOutcome o) {
    printf("out %u",(unsigned)o.tag);
    if(o.tag==SEXT_OUT_FATAL)printf(" %"PRIu64,o.status);
    if(o.tag==SEXT_OUT_COMMITTED)printf(" %zu %"PRIu64" %u %"PRIu64,o.index,o.page,o.target,o.token);
    if(o.tag==SEXT_OUT_REJECTED)printf(" %u %"PRIu64" %u %"PRIu64,(unsigned)o.reason,o.page,o.target,o.token);
    putchar('\n');
}
static void codec(uint64_t kind,uint64_t op,uint64_t caller,uint64_t a,uint64_t b,uint64_t c) {
    if(kind==0){SextFaultNotice o={0};bool ok=sext_decode_fault(op,caller,a,b,c,&o);printf("codec %u",(unsigned)ok);if(ok)printf(" %"PRIu64" %"PRIu64" %"PRIu64" %u",o.fault_addr,o.error_code,o.token,o.caller_pd);}
    else if(kind==1){SextReleaseRequest o={0};bool ok=sext_decode_release(op,caller,a,b,c,&o);printf("codec %u",(unsigned)ok);if(ok)printf(" %"PRIu64" %"PRIu64" %u",o.page,o.request_id,o.caller_pd);}
    else if(kind==2){SextExitNotice o={0};bool ok=sext_decode_exit(op,caller,a,b,c,&o);printf("codec %u",(unsigned)ok);if(ok)printf(" %u %"PRIu64,o.target_pd,o.task_id);}
    else if(kind==3){SextWaitRequest o={0};bool ok=sext_decode_wait(op,caller,a,b,c,&o);printf("codec %u",(unsigned)ok);if(ok)printf(" %u %"PRIu64" %u",o.target_pd,o.request_id,o.caller_pd);}
    else if(kind==4)printf("value %"PRIu64,sext_release_reply(a,b));
    else {uint64_t out=0;bool ok=sext_release_reply_status(a,b,&out);printf("codec %u",(unsigned)ok);if(ok)printf(" %"PRIu64,out);}
    putchar('\n');
}
int main(void) {
    sext_allocator_init(&allocator,4);memset(memory,0xa5,sizeof memory);
    pool=(SextPagingInfo){UINT64_C(0x400000),(uint64_t)SEXT_FRAME_COUNT*4096,0,6};
    mappings=(SextMappings){{{true,36,{UINT64_C(0x800000),32768,15,6}},{true,37,{UINT64_C(0x900000),32768,10,6}}}};
    query=(SextFaultQuery){UINT64_C(0x800000),4,15,6};cleanup_status=SEXT_LIVE;
    uint64_t v[9];
    while(scanf("%"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64,
                &v[0],&v[1],&v[2],&v[3],&v[4],&v[5],&v[6],&v[7],&v[8])==9) {
        uint64_t op=v[0],a=v[1],b=v[2],c=v[3],d=v[4],e=v[5],f=v[6],g=v[7];
        printf("cmd");for(size_t i=0;i<9;++i)printf(" %"PRIu64,v[i]);putchar('\n');
        switch(op) {
        case 0:sext_allocator_init(&allocator,(size_t)a);break;
        case 1:{SextAllocation r=sext_allocator_reserve(&allocator,a,(uint32_t)b,c);if(r.ok)printf("reserve 1 %zu %u\n",r.index,(unsigned)r.needs_zero);else printf("reserve 0 %u\n",(unsigned)r.error);break;}
        case 2:printf("bool %u\n",(unsigned)sext_allocator_mark_zeroed(&allocator,(size_t)a,b));break;
        case 3:printf("bool %u\n",(unsigned)sext_allocator_commit(&allocator,(size_t)a,b));break;
        case 4:printf("bool %u\n",(unsigned)sext_allocator_rollback(&allocator,(size_t)a,b));break;
        case 5:printf("binding");binding(sext_allocator_begin_reclaim(&allocator,(uint32_t)a,b));putchar('\n');break;
        case 6:printf("bool %u\n",(unsigned)sext_allocator_finish_reclaim(&allocator,(SextBinding){(size_t)a,b,(uint32_t)c,d}));break;
        case 7:printf("count %zu\n",sext_allocator_finish_cleanup(&allocator,(uint32_t)a));break;
        case 8:printf("count %zu %zu\n",sext_allocator_used_by(&allocator,(uint32_t)a),sext_allocator_used(&allocator));break;
        case 9:printf("bindings");bindings(sext_allocator_bindings(&allocator,(uint32_t)a));putchar('\n');break;
        case 10:printf("mask %"PRIu64"\n",sext_allocator_target_mask(&allocator,(uint32_t)a));break;
        case 11:pool=(SextPagingInfo){a,b,(uint32_t)c,(uint8_t)d};break;
        case 12:assert(a<2);mappings.entries[a]=(SextMapping){b!=0,c,{d,e,(uint32_t)f,(uint8_t)g}};break;
        case 13:{uint64_t r=sext_pager_init(&pager,pool,mappings,(size_t)a);if(!r)ready=true;printf("init %"PRIu64"\n",r);break;}
        case 14:if(ready)outcome(sext_pager_step(&pager,(SextFaultNotice){a,b,c,(uint32_t)d}));else puts("uninitialized");break;
        case 15:if(ready){SextReleaseOutcome r=sext_pager_release(&pager,(SextReleaseRequest){a,b,(uint32_t)c});printf("release %u",(unsigned)r.tag);if(r.tag!=SEXT_RELEASE_RETRY)printf(" %"PRIu64,r.status);if(r.tag==SEXT_RELEASE_DONE){binding(r.binding);printf(" %u",(unsigned)r.replay);}putchar('\n');}else puts("uninitialized");break;
        case 16:if(ready){SextCleanupOutcome r=sext_pager_cleanup(&pager,(SextExitNotice){(uint32_t)a,b});printf("cleanup_out %u",(unsigned)r.tag);if(r.tag==SEXT_CLEANUP_FATAL)printf(" %"PRIu64,r.status);if(r.tag==SEXT_CLEANUP_CLOSED)printf(" %"PRIu64" %"PRIu64" %zu %u",r.result.frame_bitmap,r.result.task_id,r.released,(unsigned)r.replay);putchar('\n');}else puts("uninitialized");break;
        case 17:if(ready){SextWaitOutcome r=sext_pager_wait(&pager,(SextWaitRequest){(uint32_t)a,b,(uint32_t)c});printf("wait %u",(unsigned)r.ready);if(r.ready)printf(" %"PRIu64,r.status);putchar('\n');}else puts("uninitialized");break;
        case 18:if(ready){uint32_t target=(uint32_t)a;size_t i=sext_pager_client_index(&pager,target);SextMapping m=sext_pager_mapping(&pager,target);SextClosed x=sext_pager_closed(&pager,target);printf("lookup %zu",i);if(m.present)printf(" M %"PRIu64" %"PRIu64" %"PRIu64" %u %u",m.slot,m.info.base,m.info.length,m.info.target_pd,(unsigned)m.info.rights);else printf(" M-");if(x.present)printf(" X %"PRIu64" %"PRIu64,x.value.frame_bitmap,x.value.task_id);else printf(" X-");printf(" used %zu %zu",sext_allocator_used_by(&pager.allocator,target),sext_allocator_used(&pager.allocator));bindings(sext_allocator_bindings(&pager.allocator,target));putchar('\n');}else puts("uninitialized");break;
        case 19:codec(a,b,c,d,e,f);break;
        case 20:query_status=a;query=(SextFaultQuery){b,c,(uint32_t)d,(uint8_t)e};break;
        case 21:commit_status=a;reject_status=b;reclaim_status=c;cleanup_status=d;cleanup_result=(SextCleanupResult){e,f};break;
        case 22:{uint64_t bytes=(uint64_t)SEXT_FRAME_COUNT*4096;if(a<bytes)memory[a]=(uint8_t)b;break;}
        case 23:memset(memory,0xa5,sizeof memory);break;
        default:assert(!"unknown operation");
        }
        state();
    }
    return 0;
}

