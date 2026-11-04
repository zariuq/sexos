#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sext.h"

uint64_t sext_backend_query(uint64_t slot,uint64_t token,SextFaultQuery *out){(void)slot;(void)token;(void)out;return SEXT_INVALID;}
void sext_backend_zero(uint64_t address,uint64_t bytes){(void)address;(void)bytes;}
uint64_t sext_backend_commit(uint64_t mapping,size_t index,uint64_t token,uint8_t rights){(void)mapping;(void)index;(void)token;(void)rights;return SEXT_INVALID;}
uint64_t sext_backend_reject(uint64_t mapping,uint64_t token){(void)mapping;(void)token;return SEXT_INVALID;}
uint64_t sext_backend_reclaim(uint64_t mapping,size_t index,uint64_t token){(void)mapping;(void)index;(void)token;return SEXT_INVALID;}
uint64_t sext_backend_cleanup(uint64_t mapping,SextCleanupResult *out){(void)mapping;(void)out;return SEXT_INVALID;}

static SextPager pager(void)
{
    SextPager p={0};
    SextPagingInfo pool={UINT64_C(0x400000),SEXT_FRAME_COUNT*SEXT_PAGE_BYTES,0,SEXT_READ|SEXT_WRITE};
    SextMappings maps={{{true,36,{UINT64_C(0x800000),32768,15,SEXT_READ|SEXT_WRITE}},
                         {true,37,{UINT64_C(0x900000),32768,10,SEXT_READ|SEXT_WRITE}}}};
    assert(sext_pager_init(&p,pool,maps,4)==SEXT_OK);
    assert(p.allocator.partitioned);
    for(size_t i=0;i<4;++i)assert(p.allocator.owners[i]==15);
    for(size_t i=4;i<8;++i)assert(p.allocator.owners[i]==10);
    return p;
}
static SextAllocation reserve(SextFrameAllocator *a,uint64_t token,uint32_t target,uint64_t page)
{
    SextAllocation r=sext_allocator_reserve(a,token,target,page);assert(r.ok);return r;
}

int main(void)
{
    SextPager left=pager(),right=pager();
    SextPager invalid={0};
    SextPagingInfo invalid_pool={UINT64_C(0x400000),SEXT_FRAME_COUNT*SEXT_PAGE_BYTES,0,SEXT_READ|SEXT_WRITE};
    SextMappings invalid_maps={{{true,36,{UINT64_C(0x800000),32768,15,SEXT_READ|SEXT_WRITE}},
                                 {true,37,{UINT64_C(0x900000),32768,10,SEXT_READ|SEXT_WRITE}}}};
    assert(sext_pager_init(&invalid,invalid_pool,invalid_maps,5)==SEXT_INVALID);
    assert(!invalid.allocator.partitioned);
    for(size_t i=0;i<SEXT_FRAME_COUNT;++i)assert(invalid.allocator.owners[i]==0);
    SextAllocation a0=reserve(&left.allocator,1,15,0x800000);
    SextAllocation b0=reserve(&left.allocator,2,10,0x900000);
    SextAllocation b1=reserve(&right.allocator,2,10,0x900000);
    SextAllocation a1=reserve(&right.allocator,1,15,0x800000);
    assert(a0.index==0&&a1.index==0&&b0.index==4&&b1.index==4);
    assert(memcmp(&left.allocator,&right.allocator,sizeof left.allocator)==0);
    for(uint64_t i=1;i<4;++i){
        assert(reserve(&left.allocator,10+i,15,0x800000+i*4096).index==i);
        assert(reserve(&left.allocator,20+i,10,0x900000+i*4096).index==4+i);
    }
    SextAllocation full=sext_allocator_reserve(&left.allocator,99,15,0x804000);
    assert(!full.ok&&full.error==SEXT_ALLOC_QUOTA);
    SextAllocation unknown=sext_allocator_reserve(&left.allocator,100,99,0xa00000);
    assert(!unknown.ok&&unknown.error==SEXT_ALLOC_INVALID);
    assert(sext_allocator_rollback(&left.allocator,4,2));
    assert(reserve(&left.allocator,101,10,0x904000).index==4);
    assert(sext_allocator_reserve(&left.allocator,102,15,0x805000).error==SEXT_ALLOC_QUOTA);
    puts("Sext partition controls passed: stable range-local frame identities and commuting independent reservations");
}
