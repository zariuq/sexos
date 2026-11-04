#include "silk_shell_models_workspace.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

void shell_models_workspace_poll(ShellLinenWorkspaceContext *ctx)
{
 for(size_t i=0;i<SHELL_APP_COUNT;++i){
  if(!ctx->linen->workspace_refresh_pending[i])continue;
  const ShellAppCustody *entry=&ctx->apps->entries[i];
  if(!ctx->apps->present[i] || entry->dead || entry->close_pending){ctx->linen->workspace_refresh_pending[i]=false;continue;}
  uint64_t status=shell_models_workspace_info(entry->slot);
  if(status==SHELL_WORKSPACE_LIVE){
   ctx->linen->workspace_refresh_pending[i]=false;
   shell_linen_snapshot_refresh(&ctx->linen->snapshot,true);
   sexos_line("[linen.workspace.changed] target=%u task=%u refresh=queued read_only=1",entry->target,entry->task);
  }else if(status!=SHELL_WORKSPACE_RETRY)ctx->linen->workspace_refresh_pending[i]=false;
 }
}
bool shell_models_workspace_receive(ShellLinenWorkspaceContext *ctx,const SexosMessage *message)
{
 if(message->type_id!=SHELL_WORKSPACE_CHANGED)return false;
 bool found=false;
 for(size_t i=0;i<SHELL_APP_COUNT;++i){
  const ShellAppCustody *entry=&ctx->apps->entries[i];
  if(!ctx->apps->present[i] || entry->target!=message->caller_pd)continue;
  if(!shell_custody_allows_operation(entry,message->caller_task,message->type_id,message->arg0,message->arg1,message->arg2))continue;
  ctx->linen->workspace_refresh_pending[i]=true;
  found=true;break;
 }
 if(found)shell_models_workspace_poll(ctx);
 return true;
}


