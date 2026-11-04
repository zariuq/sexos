#include "silk_shell_surfaces.h"
ShellActivationRaw shell_surfaces_activation_call(uint64_t slot,bool commit)
{
    uint64_t status=66,pd=commit?1:0,task;
    register uint64_t surface __asm__("r10");
    register uint64_t state __asm__("r8");
    __asm__ volatile("syscall" : "+a"(status), "+S"(pd), "=d"(task), "=r"(surface), "=r"(state)
                     : "D"(slot) : "rcx", "r11", "memory", "cc");
    return(ShellActivationRaw){status,pd,task,surface,state};
}
void shell_surfaces_debug_record(const ShellDebugRecord *record)
{
    if(record->len>sizeof(record->bytes)){sexos_fail();return;}
    sexos_raw_print((const char *)record->bytes,record->len);
}


