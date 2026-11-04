#include "network_dns_job_abi.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

NetworkDnsJobWords network_dns_job_encode(NetworkDnsJobRequest r)
{return(NetworkDnsJobWords){{(uint64_t)r.seq|((uint64_t)r.kind<<24),r.ticket,0}};}
NetworkDnsJobRequestDecode network_dns_job_decode(NetworkDnsJobWords words)
{
 NetworkDnsJobRequest r={(uint32_t)(words.words[0]&NETWORK_JOB_MAX_SEQ),(uint8_t)(words.words[0]>>24),(uint32_t)words.words[1]};
 bool ok=words.words[0]>>32==0&&words.words[1]>>32==0&&words.words[2]==0&&network_dns_job_request_valid(r);
 return(NetworkDnsJobRequestDecode){ok,r,ok?0:NETWORK_JOB_INVALID};
}
bool network_dns_job_request_valid(NetworkDnsJobRequest r)
{
 if(!r.seq||r.seq>NETWORK_JOB_MAX_SEQ)return false;
 bool kind=r.kind>=NETWORK_DNS_JOB_BEGIN&&r.kind<=NETWORK_DNS_JOB_RELEASE;if(!kind)return false;
 return r.kind==NETWORK_DNS_JOB_BEGIN?r.ticket==0:r.ticket>0&&r.ticket<=NETWORK_JOB_MAX_TICKET;
}
NetworkDnsJobStateDecode network_dns_job_state_decode(uint32_t value)
{return(NetworkDnsJobStateDecode){value<=NETWORK_DNS_JOB_CANCELLED,(NetworkDnsJobState)value};}
uint64_t network_dns_job_reply(uint32_t seq,uint32_t value)
{return(UINT64_C(0x79)<<56)|((uint64_t)(seq&NETWORK_JOB_MAX_SEQ)<<32)|value;}
uint64_t network_dns_job_error(uint32_t seq,uint32_t code)
{return(UINT64_C(0xE9)<<56)|((uint64_t)(seq&NETWORK_JOB_MAX_SEQ)<<32)|code;}
NetworkDnsJobReplyDecode network_dns_job_decode_reply(uint64_t word,uint32_t seq)
{
 if(!seq||seq>NETWORK_JOB_MAX_SEQ||((word>>32)&NETWORK_JOB_MAX_SEQ)!=seq)return(NetworkDnsJobReplyDecode){false,false,NETWORK_JOB_STALE};
 uint8_t tag=(uint8_t)(word>>56);if(tag==0x79)return(NetworkDnsJobReplyDecode){true,true,(uint32_t)word};if(tag==0xE9)return(NetworkDnsJobReplyDecode){true,false,(uint32_t)word};return(NetworkDnsJobReplyDecode){false,false,NETWORK_JOB_INVALID};
}


