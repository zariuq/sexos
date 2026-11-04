#include "silk_shell_models_linen.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

bool shell_models_publish_snapshot_checked(SilkShellModelsState *models,ShellLinenRuntime *linen,const ShellLinenSnapshot *snapshot,const LinenObject (*seeds)[6],bool legacy_seed_empty)
{
 if(linen->publication_revision==UINT64_MAX)return false;
 LinenObject next[LINEN_MAX_OBJECTS]={0};
 size_t count=0;
 for(size_t i=0;i<SHELL_LINEN_CAPACITY && i<LINEN_MAX_OBJECTS;++i){
  if(!snapshot->present[i])continue;
  const ShellLinenObject *row=&snapshot->objects[i];
  if(row->id==0 || row->id==UINT64_MAX || row->len>24)return false;
  for(size_t j=0;j<i;++j)if(snapshot->present[j] && snapshot->objects[j].id==row->id)return false;
  shell_models_remote_object(&next[i],row);++count;
 }
 if(count!=snapshot->count)return false;
 bool real=count!=0;
 if(!real && legacy_seed_empty){
  for(size_t i=0;i<6 && i<LINEN_MAX_OBJECTS;++i)next[i]=(*seeds)[i];
 }
 uint64_t previous=models->SELECTED_LINEN_OBJECT_ID;
 uint64_t selected=0;bool retained=false;
 for(size_t i=0;i<LINEN_MAX_OBJECTS;++i){
  if(!next[i].present)continue;
  if(selected==0)selected=next[i].object_id;
  if(next[i].object_id==previous)retained=true;
 }
 if(retained)selected=previous;
 for(size_t i=0;i<LINEN_MAX_OBJECTS;++i)models->LINEN_OBJECTS[i]=next[i];
 models->LINEN_REMOTE_REAL=real;
 models->LINEN_REMOTE_FETCHED=true;
 models->SELECTED_LINEN_OBJECT_ID=selected;
 linen->publication_revision+=1;
 return true;
}


