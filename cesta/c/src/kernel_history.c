#include "sexos_kernel.h"

void kernel_history_boot(void)
{
    if(kernel.genesis_sealed || kernel.scheduler_running || kernel.current)kernel_halt("history grant outside BOOT");
    Domain *files=domain_get(CURRENT_PD_sexfiles);
    if(!files || cap_get(files,HISTORY_SLOT))kernel_halt("history holder/slot");
    HistoryGrant grant={.holder=files->id,.logical=CURRENT_PD_spindle,.base=1808,.sectors=HISTORY_SECTORS};
    size_t count=0;
#define HISTORY_PARTICIPANT(p_target,p_ordinal,p_surface,p_shell_surface,p_lifecycle,p_close,p_activation,p_predecessor,p_logical,p_route) do { if(p_logical==CURRENT_PD_spindle){ Domain *d=domain_get(CURRENT_PD_##p_target); if(!d || !d->task_id || count>=HISTORY_ROSTER_COUNT)kernel_halt("history participant identity"); grant.participants[count++]=(HistoryParticipant){d->id,d->task_id}; } }while(0);
    SEXOS_CURRENT_APP_ROWS(HISTORY_PARTICIPANT)
#undef HISTORY_PARTICIPANT
    if(count!=HISTORY_ROSTER_COUNT || !history_valid(grant) ||
        !cap_set(files,HISTORY_SLOT,(Cap){.kind=CAP_HISTORY_RECOVERY,.history=grant}))kernel_halt("history BOOT grant");
    const Cap *held=cap_get(files,HISTORY_SLOT);
    if(!held || held->kind!=CAP_HISTORY_RECOVERY)kernel_halt("history held grant missing");
    for(uint64_t kind=0;kind<HISTORY_ROSTER_COUNT+2;++kind){
        HistoryReply a=history_query(grant,files->id,kind),b=history_query(held->history,files->id,kind);
        if(a.status!=b.status || a.value0!=b.value0 || a.value1!=b.value1 || a.value2!=b.value2 || a.value3!=b.value3)
            kernel_halt("history held publication mismatch");
    }
    klog("[boot.history.recovery] holder=%u slot=53 logical=%u participants=%u version=1 base=1808 sectors=18 held_query_verified=1 runtime_mint=0\n",files->id,CURRENT_PD_spindle,HISTORY_ROSTER_COUNT);
    for(size_t i=0;i<HISTORY_ROSTER_COUNT;++i)
        klog("[boot.history.participant] index=%llu target=%u task=%u holder=%u immutable=1 runtime_mint=0\n",(unsigned long long)i,grant.participants[i].target,grant.participants[i].task,files->id);
}
bool kernel_history_boot_valid(void)
{
    Domain *files=domain_get(CURRENT_PD_sexfiles),*linen=domain_get(CURRENT_PD_linen);
    if(!files || !linen)return false;
    const Cap *held=cap_get(files,HISTORY_SLOT);
    if(!held || held->kind!=CAP_HISTORY_RECOVERY || !history_valid(held->history) ||
        held->history.holder!=files->id || held->history.logical!=CURRENT_PD_spindle || held->history.base!=1808)return false;
    size_t count=0;
#define HISTORY_AUDIT(p_target,p_ordinal,p_surface,p_shell_surface,p_lifecycle,p_close,p_activation,p_predecessor,p_logical,p_route) do { if(p_logical==CURRENT_PD_spindle){ Domain *d=domain_get(CURRENT_PD_##p_target); const Cap *observer=cap_get(linen,p_lifecycle); if(!d || count>=HISTORY_ROSTER_COUNT || !observer || observer->kind!=CAP_APP_LIFECYCLE || held->history.participants[count].target!=d->id || held->history.participants[count].task!=d->task_id || app_lifecycle_authorize(observer->lifecycle,linen->id,d->id,d->task_id,APP_PRIVATE_QUERY) || observer->lifecycle.rights!=APP_PRIVATE_QUERY || observer->lifecycle.ack_bit)return false; ++count; } }while(0);
    SEXOS_CURRENT_APP_ROWS(HISTORY_AUDIT)
#undef HISTORY_AUDIT
    return count==HISTORY_ROSTER_COUNT;
}
void kernel_history_query(Domain *d,TrapFrame *f)
{
    uint64_t slot=f->rdi,kind=f->rsi;
    f->rax=1;f->rsi=f->rdx=f->r10=f->r8=0;
    if(!kernel_current_task(d))return;
    const Cap *held=cap_get(d,slot);
    if(!held || held->kind!=CAP_HISTORY_RECOVERY)return;
    HistoryReply r=history_query(held->history,d->id,kind);
    f->rax=r.status;f->rsi=r.value0;f->rdx=r.value1;f->r10=r.value2;f->r8=r.value3;
}


