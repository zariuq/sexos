#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#include "silk_shell_custody.h"
#define SHELL_CATALOG_ROW(app,ordinal,target,surface,frame,life,close,has_activate,activate,route,n,c0,c1,c2) \
    {true,app,ordinal,target,surface,frame,life,close,has_activate,activate,route,n,{c0,c1,c2}},
static const ShellAppCatalogRow shell_catalog[SHELL_APP_COUNT]={SEXOS_SHELL_APP_ROWS(SHELL_CATALOG_ROW)};
#undef SHELL_CATALOG_ROW

ShellAppCatalogRow shell_custody_catalog_at(size_t index)
{
    if(index<SHELL_APP_COUNT) return shell_catalog[index];
    return (ShellAppCatalogRow){0};
}
ShellAppCatalogRow shell_custody_catalog_find(uint32_t target,uint64_t surface)
{
    for(size_t i=0;i<SHELL_APP_COUNT;++i) {
        ShellAppCatalogRow row=shell_catalog[i];
        if(row.target==target && row.surface==surface) return row;
    }
    return (ShellAppCatalogRow){0};
}
ShellAppCustody shell_custody_new(size_t slot,uint32_t target,uint32_t task,uint64_t surface)
{
    return (ShellAppCustody){.slot=slot,.target=target,.task=task,.surface=surface};
}
ShellCustodyWord shell_custody_shell_surface(const ShellAppCustody *entry)
{
    ShellAppCatalogRow row=shell_custody_catalog_find(entry->target,entry->surface);
    return (ShellCustodyWord){row.present && row.shell_surface!=0,row.shell_surface};
}
bool shell_custody_is_successor_frame(const ShellAppCustody *entry,uint64_t sid)
{
    ShellAppCatalogRow row=shell_custody_catalog_find(entry->target,entry->surface);
    return row.present && row.app==SHELL_APP_SPINDLE && row.ordinal!=0 &&
        row.shell_surface!=0 && row.shell_surface==sid;
}
bool shell_custody_owns_surface(const ShellAppCustody *entry,uint64_t sid)
{
    ShellCustodyWord frame=shell_custody_shell_surface(entry);
    return entry->surface==sid || (frame.present && frame.value==sid);
}
ShellCustodyWord shell_custody_close_slot(const ShellAppCustody *entry)
{
    ShellAppCatalogRow row=shell_custody_catalog_find(entry->target,entry->surface);
    return (ShellCustodyWord){row.present,row.close_slot};
}
bool shell_custody_request_close(ShellAppCustody *entry)
{
    if(entry->dead || !shell_custody_close_slot(entry).present) return false;
    entry->close_pending=true;
    return true;
}
bool shell_custody_close_due(const ShellAppCustody *entry,uint64_t tick)
{
    return entry->close_pending && !entry->dead &&
        (!entry->has_close_attempt || tick-entry->close_attempt>=16);
}
void shell_custody_attempted_close(ShellAppCustody *entry,uint64_t tick)
{
    entry->has_close_attempt=true;
    entry->close_attempt=tick;
}
uint64_t shell_custody_close_kind(const ShellAppCustody *entry)
{
    return entry->close_declined ? SHELL_CLOSE_CONFIRMED : SHELL_CLOSE_ASK;
}
bool shell_custody_decline_close(ShellAppCustody *entry,uint32_t caller,uint64_t a,uint64_t b,uint64_t c)
{
    if(entry->dead || !entry->close_pending || caller!=entry->target ||
        a!=entry->target || b!=entry->task || c!=SHELL_DECLINE_UNSAVED) return false;
    entry->close_pending=false;
    entry->has_close_attempt=false;
    entry->close_attempt=0;
    entry->close_declined=true;
    return true;
}
bool shell_custody_hint(ShellAppCustody *entry,uint32_t caller,uint64_t target,uint64_t task,uint64_t reserved)
{
    if(caller!=0 || target!=entry->target || task!=entry->task || reserved!=0) return false;
    if(!entry->acked) entry->pending=true;
    return true;
}
bool shell_custody_attest(ShellAppCustody *entry,uint32_t target,uint32_t task,uint64_t required)
{
    if(!entry->pending || target!=entry->target || task!=entry->task || (required&1)==0) return false;
    entry->dead=true;
    entry->close_pending=false;
    entry->registration_reply=false;
    return true;
}
bool shell_custody_allows_operation(const ShellAppCustody *entry,uint32_t caller_task,uint64_t opcode,uint64_t a,uint64_t b,uint64_t c)
{
    if(caller_task==0 || caller_task!=entry->task || entry->dead || entry->close_pending) return false;
    if(opcode==0xFA) return a==entry->surface && b!=0;
    ShellAppCatalogRow row=shell_custody_catalog_find(entry->target,entry->surface);
    if(opcode==0x275 || opcode==0x274 || opcode==0x279)
        return row.present && row.app!=SHELL_APP_SPINDLE && a==entry->target && b==entry->task && c==0;
    if(opcode==0x15)
        return row.present && row.app==SHELL_APP_SPINDLE && a<=7 && b==0 && c==0;
    return opcode==0 && row.present && row.app==SHELL_APP_SPINDLE && a==0 && b==0 && c==0;
}
bool shell_custody_allows(const ShellAppCustody *entry,uint32_t caller,uint64_t surface,uint16_t app_id)
{
    return !entry->dead && !entry->close_pending && caller==entry->target &&
        surface==entry->surface && app_id==entry->target;
}
bool shell_custody_finish_ack(ShellAppCustody *entry)
{
    if(!entry->dead || !entry->retired) return false;
    entry->acked=true;
    entry->pending=false;
    return true;
}
bool shell_custody_buffer_sender(uint32_t caller) {return caller==2;}


