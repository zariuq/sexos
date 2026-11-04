#include "network_engine_frame.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static NetworkEngineTicket engine_frame_ticket(const NetworkEngineFrame *frame,NetConfig config,uint8_t protocol,size_t minimum)
{
 if(!net_config_valid(config)||frame->length>NETWORK_ENGINE_MAX_FRAME||frame->length<minimum)return(NetworkEngineTicket){0};
 uint32_t source=(uint32_t)frame->bytes[26]<<24|(uint32_t)frame->bytes[27]<<16|(uint32_t)frame->bytes[28]<<8|frame->bytes[29];
 if(frame->bytes[12]!=8||frame->bytes[13]!=0||frame->bytes[14]>>4!=4||frame->bytes[23]!=protocol||source!=config.local)return(NetworkEngineTicket){0};
 size_t header=(size_t)(frame->bytes[14]&15)*4;
 size_t transport=protocol==6?20:8;
 if(header<20||frame->length<14+header+transport)return(NetworkEngineTicket){0};
 size_t offset=14+header;
 uint16_t port=(uint16_t)((uint16_t)frame->bytes[offset]<<8|frame->bytes[offset+1]);
 if(port<=config.source_base)return(NetworkEngineTicket){0};
 return(NetworkEngineTicket){true,(uint32_t)(port-config.source_base)};
}
NetworkEngineTicket network_engine_client_ticket(const NetworkEngineFrame *frame,NetConfig config)
{return engine_frame_ticket(frame,config,6,54);}
NetworkEngineTicket network_engine_dns_ticket(const NetworkEngineFrame *frame,NetConfig config)
{return engine_frame_ticket(frame,config,17,42);}


