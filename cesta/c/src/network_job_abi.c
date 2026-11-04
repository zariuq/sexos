#include "network_job_abi.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

NetworkJobWords network_job_encode(NetworkJobRequest r)
{return(NetworkJobWords){{(uint64_t)r.seq|((uint64_t)r.kind<<24)|((uint64_t)r.offset<<32)|((uint64_t)r.length<<48),(uint64_t)r.ticket,r.a}};}
NetworkJobRequestDecode network_job_decode(NetworkJobWords words)
{
 NetworkJobRequest r={(uint32_t)(words.words[0]&NETWORK_JOB_MAX_SEQ),(uint8_t)(words.words[0]>>24),(uint16_t)(words.words[0]>>32),(uint16_t)(words.words[0]>>48),(uint32_t)words.words[1],words.words[2]};
 if(words.words[1]>UINT32_MAX||!r.seq||r.kind<NETWORK_JOB_BEGIN||r.kind>NETWORK_JOB_FINISH)return(NetworkJobRequestDecode){false,{0},NETWORK_JOB_INVALID};
 return(NetworkJobRequestDecode){true,r,0};
}
bool network_job_request_valid(NetworkJobRequest r)
{
 if(!r.seq||r.seq>NETWORK_JOB_MAX_SEQ)return false;
 switch(r.kind){
  case NETWORK_JOB_BEGIN:return !r.ticket&&!r.offset&&!r.a&&r.length&&r.length<=NETWORK_JOB_MAX_BYTES;
  case NETWORK_JOB_BEGIN_RESOLVED:case NETWORK_JOB_BEGIN_STREAM_RESOLVED:case NETWORK_JOB_BEGIN_DUPLEX_RESOLVED:
   return !r.ticket&&!r.offset&&r.a&&r.a<=NETWORK_JOB_MAX_TICKET&&r.length&&r.length<=NETWORK_JOB_MAX_BYTES;
  case NETWORK_JOB_WRITE:return r.length&&r.length<=8&&(r.length==8||(r.a>>(r.length*8))==0);
  case NETWORK_JOB_READ:return !r.a&&r.length&&r.length<=4;
  case NETWORK_JOB_START:case NETWORK_JOB_STATUS:case NETWORK_JOB_CANCEL:case NETWORK_JOB_RELEASE:case NETWORK_JOB_FINISH:
   return !r.offset&&!r.length&&!r.a;
  default:return false;
 }
}
NetworkJobStateDecode network_job_state_decode(uint8_t value)
{if(value>NETWORK_JOB_CANCELLED)return(NetworkJobStateDecode){0};return(NetworkJobStateDecode){true,(NetworkJobState)value};}
uint32_t network_job_status(NetworkJobState state,size_t response_length)
{return((uint32_t)state<<24)|((uint32_t)response_length&NETWORK_JOB_MAX_SEQ);}
NetworkJobStatusDecode network_job_decode_status(uint32_t value)
{NetworkJobStateDecode state=network_job_state_decode((uint8_t)(value>>24));return(NetworkJobStatusDecode){state.ok,state.state,(size_t)(value&NETWORK_JOB_MAX_SEQ)};}
uint64_t network_job_reply(uint32_t seq,uint32_t value)
{return(UINT64_C(0x78)<<56)|((uint64_t)(seq&NETWORK_JOB_MAX_SEQ)<<32)|value;}
uint64_t network_job_error(uint32_t seq,uint32_t code)
{return(UINT64_C(0xE8)<<56)|((uint64_t)(seq&NETWORK_JOB_MAX_SEQ)<<32)|code;}
NetworkJobReplyDecode network_job_decode_reply(uint64_t word,uint32_t seq)
{
 if(!seq||seq>NETWORK_JOB_MAX_SEQ||(((uint32_t)(word>>32))&NETWORK_JOB_MAX_SEQ)!=seq)return(NetworkJobReplyDecode){0};
 uint8_t tag=(uint8_t)(word>>56);if(tag==0x78)return(NetworkJobReplyDecode){true,true,(uint32_t)word};
 if(tag==0xE8){return(NetworkJobReplyDecode){true,false,(uint32_t)word};}
 return(NetworkJobReplyDecode){0};
}


