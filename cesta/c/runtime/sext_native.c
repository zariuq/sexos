#include "sext_service.h"
#include <stdatomic.h>

uint64_t sext_native_info(uint64_t slot,SextPagingInfo *out)
{
    register uint64_t a __asm__("rax")=59;
    register uint64_t b __asm__("rsi"),c __asm__("rdx"),d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall": "+a"(a),"=S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(slot):"rcx","r11","memory","cc");
    if(a)return a;
    if(d>UINT32_MAX || e>UINT8_MAX)return SEXT_INVALID;
    *out=(SextPagingInfo){b,c,(uint32_t)d,(uint8_t)e};return SEXT_OK;
}
uint64_t sext_backend_query(uint64_t mapping,uint64_t token,SextFaultQuery *out)
{
    register uint64_t a __asm__("rax")=57,b __asm__("rsi")=token;
    register uint64_t c __asm__("rdx"),d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall": "+a"(a),"+S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(mapping):"rcx","r11","memory","cc");
    if(a)return a;
    if(d>UINT32_MAX || e>UINT8_MAX)return SEXT_INVALID;
    *out=(SextFaultQuery){b,c,(uint32_t)d,(uint8_t)e};return SEXT_OK;
}
uint64_t sext_backend_commit(uint64_t mapping,size_t index,uint64_t token,uint8_t rights)
{
    register uint64_t a __asm__("rax")=56,b __asm__("rsi")=SEXT_SLOT_POOL,c __asm__("rdx")=index;
    register uint64_t d __asm__("r10")=token,e __asm__("r8")=rights;
    __asm__ volatile("syscall": "+a"(a),"+S"(b),"+d"(c),"+r"(d),"+r"(e):"D"(mapping):"rcx","r11","memory","cc");
    return a;
}
uint64_t sext_backend_reject(uint64_t mapping,uint64_t token)
{
    register uint64_t a __asm__("rax")=58,b __asm__("rsi")=token;
    register uint64_t c __asm__("rdx"),d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall": "+a"(a),"+S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(mapping):"rcx","r11","memory","cc");
    return a;
}
uint64_t sext_backend_reclaim(uint64_t mapping,size_t index,uint64_t token)
{
    register uint64_t a __asm__("rax")=60,b __asm__("rsi")=SEXT_SLOT_POOL,c __asm__("rdx")=index;
    register uint64_t d __asm__("r10")=token,e __asm__("r8");
    __asm__ volatile("syscall": "+a"(a),"+S"(b),"+d"(c),"+r"(d),"=r"(e):"D"(mapping):"rcx","r11","memory","cc");
    return a;
}
uint64_t sext_backend_cleanup(uint64_t mapping,SextCleanupResult *out)
{
    register uint64_t a __asm__("rax")=61,b __asm__("rsi")=SEXT_SLOT_POOL,c __asm__("rdx");
    register uint64_t d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall": "+a"(a),"+S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(mapping):"rcx","r11","memory","cc");
    if(a)return a;
    *out=(SextCleanupResult){b,c};return SEXT_OK;
}
uint64_t sext_native_catalog(uint64_t slot,SextCatalog *out)
{
    register uint64_t a __asm__("rax")=63,b __asm__("rsi")=1,c __asm__("rdx");
    register uint64_t d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall": "+a"(a),"+S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(slot):"rcx","r11","memory","cc");
    if(a)return a;
    if(b>UINT32_MAX || c>UINT32_MAX || e>UINT32_MAX)return SEXT_INVALID;
    *out=(SextCatalog){(uint32_t)b,(uint32_t)c,d,(uint32_t)e};return SEXT_OK;
}
uint64_t sext_native_retire(uint64_t slot,SextRetiredPool *out)
{
    register uint64_t a __asm__("rax")=65,b __asm__("rsi"),c __asm__("rdx");
    register uint64_t d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall": "+a"(a),"=S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(slot):"rcx","r11","memory","cc");
    if(a)return a;
    if(d>UINT32_MAX)return SEXT_INVALID;
    *out=(SextRetiredPool){b,c,(uint32_t)d};return SEXT_OK;
}
_Noreturn void sext_fatal(const char *reason,uint64_t status)
{
    sexos_line("[sext.fatal] %s status=%llu",reason,(unsigned long long)status);
    for(;;)sexos_yield();
}

