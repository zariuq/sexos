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

static uint64_t history_reply_error(HistoryReply r)
{
    return r.status && (r.value0|r.value1|r.value2|r.value3)?1:r.status;
}
HistoryDecode history_decode_for(uint32_t holder,uint32_t logical,HistoryTargets targets,HistoryQueryContext context)
{
    size_t n=HISTORY_ROSTER_COUNT;
    if(!HISTORY_ROSTER_COUNT || HISTORY_ROSTER_COUNT>64 || !holder || !logical)return (HistoryDecode){.status=1};
    HistoryReply header=history_query_provider(context,0);
    uint64_t error=history_reply_error(header);
    if(error)return (HistoryDecode){.status=error};
    if(header.value0!=logical || header.value1!=holder || header.value2!=HISTORY_ROSTER_COUNT || header.value3!=HISTORY_VERSION)
        return (HistoryDecode){.status=1};
    HistoryReply extent=history_query_provider(context,1);error=history_reply_error(extent);
    if(error)return (HistoryDecode){.status=error};
    if(extent.value2 || extent.value3)return (HistoryDecode){.status=1};
    HistoryGrant g={.holder=holder,.logical=logical,.base=extent.value0,.sectors=extent.value1};
    for(size_t i=0;i<n;++i){
        HistoryReply raw=history_query_provider(context,2+i);error=history_reply_error(raw);
        if(error)return (HistoryDecode){.status=error};
        if(raw.value0!=targets.values[i] || raw.value1>UINT32_MAX || raw.value2!=i || raw.value3)
            return (HistoryDecode){.status=1};
        g.participants[i]=(HistoryParticipant){(uint32_t)raw.value0,(uint32_t)raw.value1};
    }
    if(!history_valid(g))return (HistoryDecode){.status=3};
    return (HistoryDecode){0,g};
}


