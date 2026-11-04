#include "silk_shell_models_bell.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

void shell_models_bell_record_event(ShellBellModelsContext *ctx,uint64_t object_id,uint64_t buffer_id)
{
 SilkShellModelsState *state=ctx->models;
 size_t index=(size_t)state->BELL_RING_WRITE_INDEX%BELL_RING_CAP;
 uint64_t sequence=state->BELL_EVENT_SEQUENCE++;
 BellEvent previous=state->BELL_EVENTS[index];
 state->BELL_EVENTS[index]=(BellEvent){.present=true,.event_id=sequence,
  .kind=BELL_EVENT_OBJECT_LINKED_TO_BUFFER,.object_id=object_id,.buffer_id=buffer_id,
  .sequence=state->BELL_RING_WRITE_INDEX};
 ++state->BELL_RING_WRITE_INDEX;
 if(previous.present)sexos_line("[bell.ring.overwrite] idx=%zu prev_event_id=%llu",index,(unsigned long long)previous.event_id);
 sexos_line("[bell.ring.write] idx=%zu event_id=%llu object_id=%llu buffer_id=%llu",index,
  (unsigned long long)sequence,(unsigned long long)object_id,(unsigned long long)buffer_id);
}
size_t shell_models_bell_ring_count(const ShellBellModelsContext *ctx)
{
 size_t count=(size_t)ctx->models->BELL_RING_WRITE_INDEX;return count<BELL_RING_CAP?count:BELL_RING_CAP;
}


