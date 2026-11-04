#include "sexos_document_protocol.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

uint64_t sexos_document_header(SexosDocumentRequest r)
{return(uint64_t)r.seq|((uint64_t)r.kind<<24)|((uint64_t)r.offset<<32)|((uint64_t)r.object<<56);}
SexosDocumentDecode sexos_document_decode(uint64_t header,uint64_t a,uint64_t b)
{
 SexosDocumentRequest r={(uint32_t)(header&UINT64_C(0xffffff)),(uint8_t)(header>>24),(uint8_t)(header>>56),(uint32_t)((header>>32)&UINT64_C(0xffffff)),a,b};
 return(SexosDocumentDecode){r.seq&&r.kind>=SEXOS_DOCUMENT_INFO&&r.kind<=SEXOS_DOCUMENT_DELETE,r};
}
uint64_t sexos_document_reply(uint32_t seq,bool success,uint32_t value)
{return((uint64_t)(success?0x55u:0xe5u)<<56)|((uint64_t)seq<<32)|value;}
SexosDocumentReply sexos_document_decode_reply(uint64_t value,uint32_t seq)
{
 if(((value>>32)&UINT64_C(0xffffff))!=seq)return(SexosDocumentReply){0};
 uint8_t tag=(uint8_t)(value>>56);if(tag!=0x55u&&tag!=0xe5u)return(SexosDocumentReply){0};
 return(SexosDocumentReply){true,tag==0x55u,(uint32_t)value};
}
uint64_t sexos_document_hash(const SexosDocumentPayload *payload)
{
 if(!payload||payload->len>SEXOS_DOCUMENT_MAX)return 0;
 uint64_t hash=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<payload->len;i++)hash=(hash^payload->bytes[i])*UINT64_C(0x100000001b3);return hash;
}


