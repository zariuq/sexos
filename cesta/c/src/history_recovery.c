#include "sexos_history_recovery.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

bool history_valid(HistoryGrant g)
{
    size_t n=HISTORY_ROSTER_COUNT;
    if(!HISTORY_ROSTER_COUNT || HISTORY_ROSTER_COUNT>64 || !g.holder || !g.logical ||
        g.sectors!=HISTORY_SECTORS || g.base>UINT64_MAX-g.sectors)return false;
    for(size_t i=0;i<n;++i){
        HistoryParticipant p=g.participants[i];
        if(!p.target || !p.task)return false;
        for(size_t j=0;j<i;++j)
            if(g.participants[j].target==p.target || g.participants[j].task==p.task)return false;
    }
    return true;
}
HistoryFound history_participant(HistoryGrant g,uint32_t target)
{
    size_t n=HISTORY_ROSTER_COUNT;
    if(!history_valid(g))return (HistoryFound){0};
    for(size_t i=0;i<n;++i)
        if(g.participants[i].target==target)return (HistoryFound){true,g.participants[i]};
    return (HistoryFound){0};
}
bool history_permits(HistoryGrant g,uint32_t holder,uint32_t target,uint32_t task)
{
    HistoryFound found=history_participant(g,target);
    return g.holder==holder && found.present && found.value.target==target && found.value.task==task;
}
HistoryReply history_query(HistoryGrant g,uint32_t holder,uint64_t kind)
{
    size_t n=HISTORY_ROSTER_COUNT;
    if(!history_valid(g) || g.holder!=holder)return (HistoryReply){.status=3};
    if(!kind)return (HistoryReply){0,g.logical,g.holder,HISTORY_ROSTER_COUNT,HISTORY_VERSION};
    if(kind==1)return (HistoryReply){0,g.base,g.sectors,0,0};
    if(kind>=2 && kind-2<n){
        size_t i=(size_t)(kind-2);HistoryParticipant p=g.participants[i];
        return (HistoryReply){0,p.target,p.task,i,0};
    }
    return (HistoryReply){.status=1};
}


