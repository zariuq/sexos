#include "linen_capture.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static void capture_put64(LinenCaptureFrame *out,size_t offset,uint64_t value)
{
 if(offset>LINEN_CAPTURE_MAX_BYTES-8)return;
 for(size_t i=0;i<8;++i)out->bytes[offset+i]=(uint8_t)(value>>(8*i));
}
static uint64_t capture_get64(const LinenCaptureFrame *frame,size_t offset)
{
 if(offset>LINEN_CAPTURE_MAX_BYTES-8)return 0;
 uint64_t value=0;for(size_t i=0;i<8;++i)value|=(uint64_t)frame->bytes[offset+i]<<(8*i);return value;
}
bool linen_capture_encode(const LinenCaptureTable *table,uint64_t revision,LinenCaptureFrame *out)
{
 if(revision==0)return false;
 LinenCaptureFrame next={0};size_t at=LINEN_CAPTURE_HEADER_BYTES;uint16_t mask=0;
 for(size_t i=0;i<LINEN_CAPTURE_SLOTS;++i){
  if(!table->rows[i].present)continue;
  if(table->rows[i].id==0 || table->rows[i].id==UINT64_MAX || table->rows[i].len>LINEN_CAPTURE_NAME_CAPACITY)return false;
  for(size_t j=0;j<i;++j)if(table->rows[j].present && table->rows[j].id==table->rows[i].id)return false;
  if(at>(size_t)LINEN_CAPTURE_MAX_BYTES-LINEN_CAPTURE_ROW_BYTES-table->rows[i].len)return false;
  mask|=(uint16_t)(UINT16_C(1)<<i);capture_put64(&next,at,table->rows[i].id);at+=8;
  next.bytes[at++]=table->rows[i].kind;next.bytes[at++]=table->rows[i].disk;next.bytes[at++]=table->rows[i].len;
  for(size_t j=0;j<table->rows[i].len;++j)next.bytes[at++]=table->rows[i].name[j];
 }
 next.bytes[LINEN_CAPTURE_OFFSET_VERSION]=LINEN_CAPTURE_VERSION;
 next.bytes[LINEN_CAPTURE_OFFSET_LENGTH]=(uint8_t)at;next.bytes[LINEN_CAPTURE_OFFSET_LENGTH+1]=(uint8_t)(at>>8);
 capture_put64(&next,LINEN_CAPTURE_OFFSET_REVISION,revision);
 next.bytes[LINEN_CAPTURE_OFFSET_OCCUPANCY]=(uint8_t)mask;next.bytes[LINEN_CAPTURE_OFFSET_OCCUPANCY+1]=(uint8_t)(mask>>8);
 next.len=at;*out=next;return true;
}
bool linen_capture_decode(const LinenCaptureFrame *frame,LinenCaptureTable *out)
{
 if(frame->len<LINEN_CAPTURE_HEADER_BYTES || frame->len>LINEN_CAPTURE_MAX_BYTES)return false;
 if(frame->bytes[LINEN_CAPTURE_OFFSET_VERSION]!=LINEN_CAPTURE_VERSION || frame->bytes[LINEN_CAPTURE_OFFSET_VERSION+1]!=0)return false;
 size_t length=(size_t)frame->bytes[LINEN_CAPTURE_OFFSET_LENGTH]|((size_t)frame->bytes[LINEN_CAPTURE_OFFSET_LENGTH+1]<<8);
 if(length!=frame->len || capture_get64(frame,LINEN_CAPTURE_OFFSET_REVISION)==0)return false;
 uint16_t mask=(uint16_t)((uint16_t)frame->bytes[LINEN_CAPTURE_OFFSET_OCCUPANCY]|((uint16_t)frame->bytes[LINEN_CAPTURE_OFFSET_OCCUPANCY+1]<<8));
 LinenCaptureTable next={0};size_t at=LINEN_CAPTURE_HEADER_BYTES;
 for(size_t i=0;i<LINEN_CAPTURE_SLOTS;++i){
  if((mask&(UINT16_C(1)<<i))==0)continue;
  if(at>frame->len || frame->len-at<LINEN_CAPTURE_ROW_BYTES)return false;
  next.rows[i].id=capture_get64(frame,at);at+=8;next.rows[i].kind=frame->bytes[at++];next.rows[i].disk=frame->bytes[at++];next.rows[i].len=frame->bytes[at++];
  if(next.rows[i].id==0 || next.rows[i].id==UINT64_MAX || next.rows[i].len>LINEN_CAPTURE_NAME_CAPACITY || next.rows[i].len>frame->len-at)return false;
  for(size_t j=0;j<i;++j)if(next.rows[j].present && next.rows[j].id==next.rows[i].id)return false;
  for(size_t j=0;j<next.rows[i].len;++j)next.rows[i].name[j]=frame->bytes[at++];
  next.rows[i].present=true;
 }
 if(at!=frame->len)return false;
 *out=next;return true;
}
uint64_t linen_capture_error(uint8_t error){return (UINT64_C(0x80)|(uint64_t)error)<<56;}
LinenCaptureServer linen_capture_new(uint32_t owner,uint32_t task)
{
 LinenCaptureServer state={0};state.owner=owner;state.task=task;return state;
}
static bool capture_allowed(const LinenCaptureServer *state,uint32_t caller,uint32_t task)
{
 return state->owner!=0 && state->task!=0 && caller==state->owner && task==state->task;
}
uint64_t linen_capture_begin(LinenCaptureServer *state,uint32_t caller,uint32_t task,uint64_t job,uint64_t version,uint64_t reserved,const LinenCaptureTable *table)
{
 if(!capture_allowed(state,caller,task))return linen_capture_error(LINEN_CAPTURE_DENIED);
 if(version!=LINEN_CAPTURE_VERSION)return linen_capture_error(LINEN_CAPTURE_UNSUPPORTED);
 if(job==0 || reserved!=0)return linen_capture_error(LINEN_CAPTURE_MALFORMED);
 if(state->active){if(job==state->job)return 0;return linen_capture_error(LINEN_CAPTURE_FULL);}
 if(job<=state->retired)return linen_capture_error(LINEN_CAPTURE_STALE);
 if(state->revision==UINT64_MAX)return linen_capture_error(LINEN_CAPTURE_EXHAUSTED);
 LinenCaptureFrame frame={0};
 if(!linen_capture_encode(table,state->revision+1,&frame))return linen_capture_error(LINEN_CAPTURE_MALFORMED);
 state->frame=frame;state->revision+=1;state->job=job;state->active=true;return 0;
}
uint64_t linen_capture_read(const LinenCaptureServer *state,uint32_t caller,uint32_t task,uint64_t job,uint64_t offset,uint64_t reserved)
{
 if(!capture_allowed(state,caller,task))return linen_capture_error(LINEN_CAPTURE_DENIED);
 if(job==0 || reserved!=0)return linen_capture_error(LINEN_CAPTURE_MALFORMED);
 if(!state->active || job!=state->job)return linen_capture_error(LINEN_CAPTURE_STALE);
 if(offset>state->frame.len)return linen_capture_error(LINEN_CAPTURE_RANGE);
 size_t remaining=state->frame.len-(size_t)offset;size_t count=remaining<LINEN_CAPTURE_READ_BYTES?remaining:LINEN_CAPTURE_READ_BYTES;
 uint64_t value=(uint64_t)count<<56;
 for(size_t i=0;i<count;++i)value|=(uint64_t)state->frame.bytes[(size_t)offset+i]<<(8*i);
 return value;
}
uint64_t linen_capture_end(LinenCaptureServer *state,uint32_t caller,uint32_t task,uint64_t job,uint64_t reserved0,uint64_t reserved1)
{
 if(!capture_allowed(state,caller,task))return linen_capture_error(LINEN_CAPTURE_DENIED);
 if(job==0 || reserved0!=0 || reserved1!=0)return linen_capture_error(LINEN_CAPTURE_MALFORMED);
 if(job<=state->retired)return 0;
 if(state->active){
  if(job!=state->job)return linen_capture_error(LINEN_CAPTURE_STALE);
  state->retired=job;state->active=false;return 0;
 }
 return linen_capture_error(LINEN_CAPTURE_STALE);
}
bool linen_capture_append(LinenCaptureFrame *frame,uint64_t word)
{
 uint8_t count=(uint8_t)(word>>56);
 if(count>LINEN_CAPTURE_READ_BYTES || frame->len>(size_t)LINEN_CAPTURE_MAX_BYTES-count)return false;
 if(count==0)return word==0;
 if((word&UINT64_C(0x00ffffffffffffff))>>(8*count))return false;
 for(size_t i=0;i<count;++i)frame->bytes[frame->len++]=(uint8_t)(word>>(8*i));
 return true;
}


