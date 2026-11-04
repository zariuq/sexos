#include "sexos_document_growth.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

bool document_growth_init(DocumentGrowthPlan *plan,size_t original,size_t count)
{
 if(!plan||original>count||count>DOCUMENT_GROWTH_MAX_EXTENTS)return false;
 *plan=(DocumentGrowthPlan){0};plan->active=true;plan->original=original;
 plan->count=count;plan->extent=original;
 return true;
}
bool document_growth_set_extent(DocumentGrowthPlan *plan,size_t index,uint16_t sector_count)
{if(!plan||index>=plan->count||!sector_count||sector_count%DOCUMENT_GROWTH_BLOCK_SECTORS)return false;plan->sector_count[index]=sector_count;return true;}
DocumentGrowthAction document_growth_current(const DocumentGrowthPlan *plan)
{
 if(!plan||plan->in_flight)return DOCUMENT_GROWTH_INVALID;
 if(!plan->active)return DOCUMENT_GROWTH_DONE;
 switch(plan->phase){
  case 0:
   if(plan->extent>plan->count)return DOCUMENT_GROWTH_INVALID;
   if(plan->extent<plan->count){
    if(plan->extent>=DOCUMENT_GROWTH_MAX_EXTENTS||plan->sector>=plan->sector_count[plan->extent]||
       plan->sector%DOCUMENT_GROWTH_BLOCK_SECTORS)return DOCUMENT_GROWTH_INVALID;
    return DOCUMENT_GROWTH_ZERO;
   }
   return DOCUMENT_GROWTH_ZERO_BARRIER;
  case 1:return DOCUMENT_GROWTH_DESCRIPTOR;
  case 2:return DOCUMENT_GROWTH_DESCRIPTOR_BARRIER;
  case 3:return DOCUMENT_GROWTH_PUBLISH;
  case 4:return DOCUMENT_GROWTH_PUBLISH_BARRIER;
  default:return DOCUMENT_GROWTH_INVALID;
 }
}
bool document_growth_begin_action(DocumentGrowthPlan *plan,DocumentGrowthAction action)
{
 if(!plan||document_growth_current(plan)!=action||action==DOCUMENT_GROWTH_DONE||action==DOCUMENT_GROWTH_INVALID)return false;
 plan->in_flight=true;plan->pending=action;
 if(action==DOCUMENT_GROWTH_DESCRIPTOR)plan->descriptor_attempted=true;
 return true;
}
bool document_growth_finish_action(DocumentGrowthPlan *plan,DocumentGrowthAction action)
{
 if(!plan||!plan->in_flight||plan->pending!=action)return false;
 switch(action){
  case DOCUMENT_GROWTH_ZERO:
   plan->sector+=DOCUMENT_GROWTH_BLOCK_SECTORS;
   if(plan->sector>=plan->sector_count[plan->extent]){plan->extent++;plan->sector=0;}
   break;
  case DOCUMENT_GROWTH_ZERO_BARRIER:plan->phase=1;break;
  case DOCUMENT_GROWTH_DESCRIPTOR:plan->phase=2;break;
  case DOCUMENT_GROWTH_DESCRIPTOR_BARRIER:plan->phase=3;break;
  case DOCUMENT_GROWTH_PUBLISH:plan->phase=4;break;
  case DOCUMENT_GROWTH_PUBLISH_BARRIER:plan->phase=5;plan->active=false;break;
  default:return false;
 }
 plan->in_flight=false;plan->pending=DOCUMENT_GROWTH_INVALID;return true;
}
bool document_growth_cancel_releases(const DocumentGrowthPlan *plan)
{return plan&&plan->active&&!plan->descriptor_attempted;}
void document_growth_abort(DocumentGrowthPlan *plan)
{if(plan){plan->active=false;plan->in_flight=false;plan->pending=DOCUMENT_GROWTH_INVALID;}}


