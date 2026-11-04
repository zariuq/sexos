#include "silk_shell.h"
#include "shell_linen_snapshot.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

void shell_models_remote_object(LinenObject *out,const ShellLinenObject *source)
{
 out->present=true;out->object_id=source->id;
 out->kind=source->kind==1?LINEN_OBJ_KIND_PROJECT:LINEN_OBJ_KIND_DOCUMENT;
 out->state=LINEN_OBJ_STATE_SAVED;out->parent_id=0;out->project_id=0;
 out->grant_ref=0;out->linked_surface_id=0;out->flags=0;
 out->display_name="[linen.remote]";
 for(size_t i=0;i<24;++i)out->name[i]=source->name[i];
 out->name_len=source->len;out->disk_path=source->disk;
}


