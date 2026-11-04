#include "sexos_history_recovery.h"

HistoryReply history_query_provider(HistoryQueryContext context,uint64_t kind)
{
    register uint64_t a __asm__("rax")=HISTORY_SYS_QUERY,b __asm__("rsi")=kind;
    register uint64_t c __asm__("rdx"),d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall":"+a"(a),"+S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(context.slot):"rcx","r11","memory","cc");
    return (HistoryReply){a,b,c,d,e};
}
HistoryDecode history_get(uint64_t slot)
{
    HistoryTargets targets={0};size_t count=0;
#define HISTORY_TARGET(p_target,p_ordinal,p_surface,p_shell_surface,p_lifecycle,p_close,p_activation,p_predecessor,p_logical,p_route) do {if(p_logical==CURRENT_PD_spindle)targets.values[count++]=CURRENT_PD_##p_target;}while(0);
    SEXOS_CURRENT_APP_ROWS(HISTORY_TARGET)
#undef HISTORY_TARGET
    if(count!=HISTORY_ROSTER_COUNT)return (HistoryDecode){.status=1};
    return history_decode_for(CURRENT_PD_sexfiles,CURRENT_PD_spindle,targets,(HistoryQueryContext){slot,0});
}
