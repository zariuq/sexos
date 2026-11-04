#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_smp_park.h"

static SmpParkState build(const size_t order[3])
{
    SmpParkState state;
    assert(smp_park_init(&state,4,0,0)==SMP_PARK_OK);
    assert(smp_park_enroll(&state,0,0,true)==SMP_PARK_OK);
    assert(smp_park_enroll(&state,1,2,false)==SMP_PARK_OK);
    assert(smp_park_enroll(&state,2,4,false)==SMP_PARK_OK);
    assert(smp_park_enroll(&state,3,6,false)==SMP_PARK_OK);
    assert(!smp_park_can_commit(&state,0));
    for(size_t i=0;i<3;++i){size_t slot=order[i];assert(smp_park_arrive(&state,slot,(uint32_t)(slot*2))==SMP_PARK_OK);}
    assert(smp_park_seal(&state)==SMP_PARK_OK);
    assert(smp_park_all_quiescent(&state));
    return state;
}
int main(void)
{
    const size_t left_order[3]={1,2,3},right_order[3]={3,1,2};
    SmpParkState left=build(left_order),right=build(right_order);
    assert(!memcmp(&left,&right,sizeof left));
    assert(smp_park_can_commit(&left,0));
    assert(smp_park_advance(&left,0,1)==SMP_PARK_OK);
    assert(smp_park_can_commit(&left,1));
    assert(smp_park_advance(&left,0,1)==SMP_PARK_STALE);
    assert(smp_park_advance(&left,1,3)==SMP_PARK_INVALID);
    SmpParkState bad;
    assert(smp_park_init(&bad,4,0,0)==SMP_PARK_OK);
    assert(smp_park_enroll(&bad,1,0,false)==SMP_PARK_INVALID);
    assert(smp_park_enroll(&bad,1,2,true)==SMP_PARK_INVALID);
    assert(smp_park_enroll(&bad,0,0,true)==SMP_PARK_OK);
    assert(smp_park_enroll(&bad,1,2,false)==SMP_PARK_OK);
    assert(smp_park_enroll(&bad,2,2,false)==SMP_PARK_DUPLICATE);
    assert(smp_park_arrive(&bad,1,3)==SMP_PARK_INVALID);
    assert(smp_park_arrive(&bad,1,2)==SMP_PARK_OK);
    assert(smp_park_arrive(&bad,1,2)==SMP_PARK_DUPLICATE);
    assert(smp_park_seal(&bad)==SMP_PARK_NOT_READY);
    SmpParkState absent;
    assert(smp_park_init(&absent,2,0,0)==SMP_PARK_OK);
    assert(smp_park_enroll(&absent,0,0,true)==SMP_PARK_OK);
    assert(smp_park_enroll(&absent,1,2,false)==SMP_PARK_OK);
    assert(smp_park_seal(&absent)==SMP_PARK_NOT_READY);
    SmpParkState unparked;
    assert(smp_park_init(&unparked,2,0,0)==SMP_PARK_OK);
    assert(smp_park_enroll(&unparked,0,0,true)==SMP_PARK_OK);
    assert(smp_park_enroll(&unparked,1,2,false)==SMP_PARK_OK);
    assert(smp_park_arrive(&unparked,1,2)==SMP_PARK_OK);
    unparked.cores[1].parked=false;
    assert(smp_park_seal(&unparked)==SMP_PARK_NOT_READY);
    SmpParkState corrupt=right;corrupt.cores[1].parked=false;
    assert(!smp_park_all_quiescent(&corrupt));
    assert(!smp_park_can_commit(&corrupt,0));
    SmpParkState exhausted=right;exhausted.generation=UINT64_MAX;
    assert(!smp_park_can_commit(&exhausted,UINT64_MAX));
    assert(smp_park_advance(&exhausted,UINT64_MAX,0)==SMP_PARK_EXHAUSTED);
    puts("SMP park controls passed: AP arrival order erases, pre-quiescent and stale commits refuse");
}
