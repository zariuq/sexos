#if !defined(SEXOS_PAGER_PARTITIONED_ALLOCATOR)
#define SEXOS_PAGER_PARTITIONED_ALLOCATOR 0

#endif
#include "sexos_pager_state.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

bool pager_fault_eligible(uint64_t address,uint64_t error,uint64_t cs)
{
    return (cs&3)==3 && address && address<(UINT64_C(1)<<47) && (error==4 || error==6);
}
uint64_t pager_token_issue(uint64_t *last,uint64_t *out)
{
    *out=0;
    if (*last==UINT64_MAX) return PG_EXHAUSTED;
    *last+=1; *out=*last; return PG_OK;
}
static uint64_t fault_matches(PgFaultRecord r,uint32_t pager,uint64_t token)
{
    if (!token || r.token!=token) return PG_INVALID;
    if (!pager || r.pager_pd!=pager) return PG_DENIED;
    return PG_OK;
}
uint64_t pager_fault_prepare(PgFaultState *s,PgFaultRecord r)
{
    if (!pager_fault_eligible(r.fault_addr,r.error_code,3) || !r.token || !r.target_pd || !r.pager_pd ||
        r.target_pd==r.pager_pd || r.page_addr!=(r.fault_addr&~UINT64_C(4095)) || !r.saved_rip || !r.saved_frame)
        return PG_INVALID;
    if (s->retired || s->phase!=PG_IDLE) return PG_DENIED;
    if (r.token<=s->last_token) return PG_INVALID;
    s->last_token=r.token; s->active=r; s->phase=PG_PREPARED; return PG_OK;
}
uint64_t pager_fault_publish(PgFaultState *s,uint64_t token)
{
    if (s->phase!=PG_PREPARED || s->active.token!=token) return PG_INVALID;
    s->phase=PG_PENDING; return PG_OK;
}
uint64_t pager_fault_cancel(PgFaultState *s,uint64_t token)
{
    if (s->phase!=PG_PREPARED || s->active.token!=token) return PG_INVALID;
    s->phase=PG_IDLE; return PG_OK;
}
PgFaultResult pager_fault_query(const PgFaultState *s,uint32_t pager,uint64_t token)
{
    if (s->phase==PG_IDLE) return (PgFaultResult){.status=PG_INVALID};
    uint64_t error=fault_matches(s->active,pager,token);
    if (error) return (PgFaultResult){.status=error};
    if (s->phase!=PG_PENDING) return (PgFaultResult){.status=PG_RETRY};
    return (PgFaultResult){.present=true,.record=s->active};
}
PgFaultResult pager_fault_claim(PgFaultState *s,uint32_t pager,uint64_t token)
{
    PgFaultResult r=pager_fault_query(s,pager,token);
    if (!r.status) s->phase=PG_CLAIMED;
    return r;
}
uint64_t pager_fault_rollback(PgFaultState *s,uint32_t pager,uint64_t token)
{
    if (s->phase!=PG_CLAIMED) return PG_INVALID;
    uint64_t error=fault_matches(s->active,pager,token);
    if (error) return error;
    s->phase=PG_PENDING; return PG_OK;
}
PgFaultResult pager_fault_complete(PgFaultState *s,uint32_t pager,uint64_t token)
{
    if (s->has_completed && s->completed.token==token) return (PgFaultResult){.status=fault_matches(s->completed,pager,token)};
    if (s->phase!=PG_CLAIMED) return (PgFaultResult){.status=PG_INVALID};
    uint64_t error=fault_matches(s->active,pager,token);
    if (error) return (PgFaultResult){.status=error};
    PgFaultRecord r=s->active;
    s->completed=r; s->has_completed=true; s->phase=PG_IDLE;
    return (PgFaultResult){.present=true,.record=r};
}
uint64_t pager_fault_retire(PgFaultState *s,uint32_t target,uint32_t pager,uint32_t task)
{
    if (s->phase!=PG_IDLE && (s->active.target_pd!=target || s->active.pager_pd!=pager || s->active.task_id!=task)) return PG_DENIED;
    if (s->has_completed && (s->completed.target_pd!=target || s->completed.pager_pd!=pager || s->completed.task_id!=task)) return PG_DENIED;
    s->phase=PG_IDLE; s->has_completed=false; s->retired=true; return PG_OK;
}
PgFaultResult pager_fault_reject(PgFaultState *s,uint32_t pager,uint64_t token)
{
    PgFaultResult r=pager_fault_query(s,pager,token);
    if (!r.status) s->phase=PG_IDLE;
    return r;
}
static bool pager_partition_owns(PgMapping m,uint64_t frames,uint64_t frame)
{
    if(!SEXOS_PAGER_PARTITIONED_ALLOCATOR)return frame<frames;
    if(!m.range_id || !m.quota || m.quota>frames/PG_CLIENTS)return false;
    uint64_t base=(uint64_t)(m.range_id-1)*m.quota;
    return base<=frame && frame-base<m.quota && base<=frames-m.quota;
}
PgBindingResult pager_mapping_validate(PgMapping m,PgFramePool p,uint32_t pager,PgFaultView f,uint64_t frame,uint64_t rights)
{
    if (!m.pool_id || p.pool_id!=m.pool_id || !m.range_id || m.range_id>PG_CLIENTS || !m.target_pd ||
        !pager || m.target_pd==pager || !m.length || m.length>PG_PAGES*4096u || m.length%4096 ||
        !p.length || p.length>PG_PAGES*4096u || p.length%4096 || !m.base_va || m.base_va%4096 ||
        !p.base_va || p.base_va%4096 || m.base_va>UINT64_MAX-m.length || m.base_va+m.length>(UINT64_C(1)<<47) ||
        p.base_va>UINT64_MAX-p.length || p.base_va+p.length>(UINT64_C(1)<<47) || !m.quota || m.quota>PG_PAGES)
        return (PgBindingResult){.status=PG_INVALID};
    if (!f.token || f.target_pd!=m.target_pd || f.pager_pd!=pager || f.page%4096 || (f.error&~UINT64_C(6)) || !(f.error&4))
        return (PgBindingResult){.status=PG_DENIED};
    if (rights>UINT8_MAX || !(rights&PG_READ) || (rights&~UINT64_C(6)) || (rights&~(uint64_t)m.rights) ||
        ((f.error&2) && !(rights&PG_WRITE)) || f.page<m.base_va || f.page-m.base_va>=m.length ||
        frame>=p.length/4096 || !pager_partition_owns(m,p.length/4096,frame))
        return (PgBindingResult){.status=PG_DENIED};
    return (PgBindingResult){.value={f.token,f.task_id,f.target_pd,pager,m.range_id,(size_t)((f.page-m.base_va)/4096),(size_t)frame,(uint8_t)rights}};
}
bool pager_binding_equal(PgBinding a,PgBinding b)
{
    return a.token==b.token && a.task_id==b.task_id && a.target_pd==b.target_pd && a.pager_pd==b.pager_pd &&
        a.range_id==b.range_id && a.page_index==b.page_index && a.frame_index==b.frame_index && a.rights==b.rights;
}
PgCommit pager_ledger_token(const PgLedger *s,uint64_t token)
{
    for (size_t i=0;i<PG_PAGES;++i) if (s->entries[i].present && s->entries[i].binding.token==token) return s->entries[i];
    return (PgCommit){0};
}
bool pager_ledger_is_retired(const PgLedger *s,uint32_t range,uint64_t token)
{
    return !range || range>PG_CLIENTS || !token || token<=s->retired_highwater[range-1];
}
uint64_t pager_ledger_check(const PgLedger *s,PgBinding b,uint8_t quota)
{
    if (b.frame_index>=PG_PAGES || b.page_index>=PG_PAGES || !b.range_id || b.range_id>PG_CLIENTS || !b.token || !quota || quota>PG_PAGES) return PG_INVALID;
    if(SEXOS_PAGER_PARTITIONED_ALLOCATOR) {
        uint64_t base=(uint64_t)(b.range_id-1)*quota;
        if(quota>PG_PAGES/PG_CLIENTS || b.frame_index<base || b.frame_index-base>=quota)return PG_DENIED;
    }
    if (s->closed[b.range_id-1].present || pager_ledger_is_retired(s,b.range_id,b.token)) return PG_DENIED;
    if (pager_ledger_token(s,b.token).present || s->entries[b.frame_index].present) return PG_DENIED;
    size_t used=0;
    for (size_t i=0;i<PG_PAGES;++i) {
        if (!s->entries[i].present) continue;
        PgBinding old=s->entries[i].binding;
        if (old.target_pd==b.target_pd && old.range_id==b.range_id && old.page_index==b.page_index) return PG_DENIED;
        if (old.target_pd==b.target_pd) ++used;
    }
    return used>=quota?PG_EXHAUSTED:PG_OK;
}
uint64_t pager_ledger_install(PgLedger *s,PgBinding b,uint8_t quota)
{
    uint64_t error=pager_ledger_check(s,b,quota);
    if (error) return error;
    s->entries[b.frame_index]=(PgCommit){true,false,b}; return PG_OK;
}
uint64_t pager_ledger_finalize(PgLedger *s,PgBinding b)
{
    if (b.frame_index>=PG_PAGES || !s->entries[b.frame_index].present) return PG_INVALID;
    if (!pager_binding_equal(s->entries[b.frame_index].binding,b)) return PG_DENIED;
    s->entries[b.frame_index].finalized=true; return PG_OK;
}
uint64_t pager_ledger_retire(PgLedger *s,PgBinding b)
{
    if (b.frame_index>=PG_PAGES || !s->entries[b.frame_index].present) return PG_DENIED;
    PgCommit r=s->entries[b.frame_index];
    if (!pager_binding_equal(r.binding,b) || !r.finalized) return PG_DENIED;
    s->retired[b.frame_index]=(PgMaybeBinding){true,b};
    if (b.token>s->retired_highwater[b.range_id-1]) s->retired_highwater[b.range_id-1]=b.token;
    s->entries[b.frame_index]=(PgCommit){0}; return PG_OK;
}
uint64_t pager_ledger_cleanup_plan(const PgLedger *s,uint32_t range,uint32_t target,uint32_t pager,uint32_t task,PgCleanupPlan *out)
{
    *out=(PgCleanupPlan){0};
    if (!range || range>PG_CLIENTS) return PG_INVALID;
    if (s->closed[range-1].present) return PG_DENIED;
    for (size_t i=0;i<PG_PAGES;++i) {
        if (!s->entries[i].present) continue;
        PgBinding b=s->entries[i].binding;
        if (b.range_id!=range && b.target_pd!=target) continue;
        if (b.range_id!=range || b.target_pd!=target || b.pager_pd!=pager || b.task_id!=task || b.frame_index!=i) return PG_DENIED;
        out->entries[i]=(PgMaybeBinding){true,b};
    }
    return PG_OK;
}
uint64_t pager_ledger_close(PgLedger *s,uint32_t range,uint32_t target,uint32_t pager,uint32_t task)
{
    PgCleanupPlan plan={0};uint64_t error=pager_ledger_cleanup_plan(s,range,target,pager,task,&plan);
    if (error) return error;
    uint8_t frames=0;
    for (size_t i=0;i<PG_PAGES;++i) {
        if (!plan.entries[i].present) continue;
        PgBinding b=plan.entries[i].value; frames|=(uint8_t)(1u<<b.frame_index);
        s->retired[b.frame_index]=(PgMaybeBinding){true,b};
        if (b.token>s->retired_highwater[range-1]) s->retired_highwater[range-1]=b.token;
        s->entries[b.frame_index]=(PgCommit){0};
    }
    s->closed[range-1]=(PgCleanup){true,target,task,frames}; return PG_OK;
}


