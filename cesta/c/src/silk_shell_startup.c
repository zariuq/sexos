#include "silk_shell_core.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

ShellStartupObserve shell_startup_observe(SilkShellCoreState *state,SexosMessage message)
{
 if(message.type_id==OP_DISPLAY_READY){
  if(message.caller_pd==1 && message.arg0==DISPLAY_READY_VERSION && message.arg1==0 && message.arg2==0){state->startup_ready=true;return SHELL_STARTUP_READY;}
  return SHELL_STARTUP_REJECTED_CONTROL;
 }
 if(state->startup_len==SHELL_STARTUP_INBOX_CAPACITY)return SHELL_STARTUP_FULL;
 size_t index=(state->startup_head+state->startup_len)%SHELL_STARTUP_INBOX_CAPACITY;
 state->startup_messages[index]=message;state->startup_present[index]=true;++state->startup_len;return SHELL_STARTUP_DEFERRED;
}
bool shell_startup_pop(SilkShellCoreState *state,SexosMessage *out)
{
 if(!state->startup_len)return false;
 size_t index=state->startup_head;
 if(!state->startup_present[index])return false;
 if(out)*out=state->startup_messages[index];
 state->startup_present[index]=false;state->startup_head=(state->startup_head+1)%SHELL_STARTUP_INBOX_CAPACITY;--state->startup_len;return true;
}
bool shell_startup_pending(const SilkShellCoreState *state){return state->startup_len!=0;}


