#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "sexos_client.h"

static const uint8_t body[]="<html><p>A<script>document.write('B')</script>C";
static uint64_t operation,chunk;
static uint8_t presented[256];
static size_t presented_len;

SexosReply sexos_pdx_call(uint64_t slot,uint64_t opcode,uint64_t a0,uint64_t a1,uint64_t a2)
{
 if(slot==SEXOS_SLOT_kernel&&opcode==SEXOS_KERNEL_raw_print){fwrite((const void *)(uintptr_t)a0,1,(size_t)a1,stdout);return(SexosReply){0,0};}
 if(slot==SEXOS_SLOT_net&&opcode==SEXOS_NET_request){operation=a0;chunk=a1;}
 if(slot==SEXOS_SLOT_net&&opcode==SEXOS_NET_dns_resolve)return(SexosReply){0,UINT64_C(0x0a000202)};
 if(slot==SEXOS_SLOT_display&&opcode==SEXOS_DISPLAY_text_chunk){size_t offset=(size_t)(a2&255u),count=(size_t)((a2>>8)&255u);for(size_t i=0;i<count;i++)presented[offset+i]=(uint8_t)(a1>>(8*i));if(offset+count>presented_len)presented_len=offset+count;}
 return(SexosReply){0,0};
}
SexosMessage sexos_listen_once(uint64_t slot)
{
 SexosMessage message={0};if(slot==1){message.type_id=65535;return message;}message.type_id=1;message.caller_pd=42;
 if(operation==SEXOS_NET_text_length)message.arg0=0;else if(operation==SEXOS_NET_body_length)message.arg0=sizeof(body)-1;else if(operation==SEXOS_NET_body_chunk){size_t offset=(size_t)chunk*8;for(size_t i=0;i<8&&offset+i<sizeof(body)-1;i++)message.arg0|=(uint64_t)body[offset+i]<<(8*i);}return message;
}
void sexos_yield(void){}
void sexos_wait_self(void){}
int main(void)
{kaleidoscope_main();printf("P ");for(size_t i=0;i<presented_len;i++)printf("%02x",presented[i]);putchar('\n');return 0;}
