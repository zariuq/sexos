#include "network_dns_codec.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

enum { DNS_MAX_ANSWERS=8,DNS_MAX_EXTRA=8,DNS_MAX_RECORDS=16,DNS_MAX_NAME=255,DNS_MAX_HOPS=16,DNS_MAX_CHAIN=4 };
typedef struct DnsName { uint8_t bytes[DNS_MAX_NAME]; size_t length; } DnsName;
typedef struct DnsBoundaries { uint8_t bits[NETWORK_DNS_MAX_BYTES/8]; } DnsBoundaries;
typedef struct DnsRecord { uint16_t owner,data,kind; uint32_t ttl; } DnsRecord;
typedef struct DnsRecords { DnsRecord entries[DNS_MAX_RECORDS]; size_t count; } DnsRecords;
static const uint8_t dns_name[18]={5,'p','r','o','b','e',5,'s','e','x','o','s',4,'t','e','s','t',0};
static const uint8_t dns_question[22]={5,'p','r','o','b','e',5,'s','e','x','o','s',4,'t','e','s','t',0,0,1,0,1};
static NetworkDnsParseResult dns_error(NetworkDnsError error,uint8_t rcode)
{return(NetworkDnsParseResult){NETWORK_DNS_RESULT_ERROR,error,rcode,{{0},0}};}
static bool dns_room(const NetworkDnsFrame *frame,size_t start,size_t length)
{return start<=frame->len&&length<=frame->len-start&&start<=NETWORK_DNS_MAX_BYTES&&length<=NETWORK_DNS_MAX_BYTES-start;}
static bool dns_u16(const NetworkDnsFrame *frame,size_t offset,uint16_t *out)
{if(!dns_room(frame,offset,2))return false;*out=(uint16_t)((uint16_t)frame->bytes[offset]<<8|frame->bytes[offset+1]);return true;}
static bool boundary_has(const DnsBoundaries *known,size_t offset)
{return offset<NETWORK_DNS_MAX_BYTES&&(known->bits[offset/8]&(uint8_t)(1u<<(offset%8)))!=0;}
static void boundary_insert(DnsBoundaries *known,size_t offset)
{if(offset<NETWORK_DNS_MAX_BYTES)known->bits[offset/8]|=(uint8_t)(1u<<(offset%8));}
static uint8_t dns_lower(uint8_t value){return value>='A'&&value<='Z'?(uint8_t)(value+('a'-'A')):value;}
static bool name_same(const DnsName *left,const DnsName *right)
{
 if(left->length!=right->length)return false;
 for(size_t i=0;i<left->length;i++)if(left->bytes[i]!=right->bytes[i])return false;
 return true;
}
static bool name_canonical(const DnsName *name)
{
 if(name->length!=sizeof(dns_name))return false;
 for(size_t i=0;i<sizeof(dns_name);i++)if(name->bytes[i]!=dns_name[i])return false;
 return true;
}
static bool decode_name(const NetworkDnsFrame *frame,size_t start,DnsBoundaries *known,bool registration,DnsName *out,size_t *name_end,NetworkDnsError *error)
{
 out->length=0;size_t cursor=start,end=SIZE_MAX,hops=0;
 for(size_t step=0;step<DNS_MAX_NAME+DNS_MAX_HOPS;step++){
  if(cursor>=frame->len||cursor>=NETWORK_DNS_MAX_BYTES){*error=NETWORK_DNS_ERROR_TRUNCATED;return false;}
  uint8_t tag=frame->bytes[cursor];
  if((tag&0xC0)==0xC0){
   uint16_t word=0;if(!dns_u16(frame,cursor,&word)){*error=NETWORK_DNS_ERROR_TRUNCATED;return false;}
   size_t target=word&0x3FFF;if(target>=cursor||!boundary_has(known,target)){*error=NETWORK_DNS_ERROR_UNSUPPORTED;return false;}
   hops++;if(hops>DNS_MAX_HOPS){*error=NETWORK_DNS_ERROR_UNSUPPORTED;return false;}
   if(end==SIZE_MAX){end=cursor+2;}
   cursor=target;continue;
  }
  if((tag&0xC0)!=0){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
  size_t length=tag,expanded=out->length+1+length;
  if(expanded<out->length||expanded>DNS_MAX_NAME){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
  size_t next=cursor+1;if(!dns_room(frame,next,length)){*error=NETWORK_DNS_ERROR_TRUNCATED;return false;}
  out->bytes[out->length]=tag;
  for(size_t i=0;i<length;i++)out->bytes[out->length+1+i]=dns_lower(frame->bytes[next+i]);
  out->length=expanded;cursor=next+length;
  if(length==0){
   size_t physical_end=end==SIZE_MAX?cursor:end;
   if(registration){
    size_t physical=start;
    while(physical<physical_end){
     boundary_insert(known,physical);uint8_t part=frame->bytes[physical];
     if(part==0||(part&0xC0)==0xC0){break;}
     physical+=1+(size_t)part;
    }
   }
   *name_end=physical_end;return true;
  }
 }
 *error=NETWORK_DNS_ERROR_MALFORMED;return false;
}
static uint32_t dns_ttl(const NetworkDnsFrame *frame,size_t offset)
{
 uint32_t value=(uint32_t)frame->bytes[offset]<<24|(uint32_t)frame->bytes[offset+1]<<16|(uint32_t)frame->bytes[offset+2]<<8|frame->bytes[offset+3];
 return(value&UINT32_C(0x80000000))?0:value;
}
static bool read_record(const NetworkDnsFrame *frame,size_t *cursor,DnsBoundaries *known,bool answer,DnsName *scratch,DnsRecord *record,NetworkDnsError *error)
{
 size_t owner=*cursor,next=0;if(!decode_name(frame,owner,known,true,scratch,&next,error))return false;*cursor=next;
 if(!dns_room(frame,*cursor,10)){*error=NETWORK_DNS_ERROR_TRUNCATED;return false;}
 size_t fields=*cursor;uint16_t kind=(uint16_t)((uint16_t)frame->bytes[fields]<<8|frame->bytes[fields+1]);
 if(frame->bytes[fields+2]!=0||frame->bytes[fields+3]!=1){*error=NETWORK_DNS_ERROR_UNSUPPORTED;return false;}
 size_t length=(size_t)((uint16_t)frame->bytes[fields+8]<<8|frame->bytes[fields+9]);
 size_t data=fields+10;
 if((kind==1&&length!=4)||(kind==28&&!answer&&length!=16)){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
 if(!dns_room(frame,data,length)){*error=NETWORK_DNS_ERROR_TRUNCATED;return false;}size_t end=data+length;
 if(kind==1){}
 else if(kind==5||(kind==2&&!answer)){
  if(!decode_name(frame,data,known,true,scratch,&next,error))return false;
  if(next!=end){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
 }else if(kind==6&&!answer){
  if(!decode_name(frame,data,known,true,scratch,&next,error))return false;
  if(!decode_name(frame,next,known,true,scratch,&next,error))return false;
  if(next>end||end-next!=20){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
 }else if(kind==28&&!answer){if(length!=16){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}}
 else {*error=NETWORK_DNS_ERROR_UNSUPPORTED;return false;}
 if(owner>UINT16_MAX||data>UINT16_MAX){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
 *record=(DnsRecord){(uint16_t)owner,(uint16_t)data,kind,dns_ttl(frame,fields+4)};*cursor=end;return true;
}
static bool validate_aliases(const NetworkDnsFrame *frame,const DnsRecords *records,DnsBoundaries *known,DnsName *left,DnsName *right,NetworkDnsError *error)
{
 size_t end=0;
 for(size_t i=0;i<records->count;i++)for(size_t j=i+1;j<records->count;j++){
  DnsRecord a=records->entries[i],b=records->entries[j];if(a.kind!=5&&b.kind!=5)continue;
  if(!decode_name(frame,a.owner,known,false,left,&end,error)||!decode_name(frame,b.owner,known,false,right,&end,error))return false;
  if(!name_same(left,right))continue;
  if(a.kind!=b.kind){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
  if(!decode_name(frame,a.data,known,false,left,&end,error)||!decode_name(frame,b.data,known,false,right,&end,error))return false;
  if(!name_same(left,right)){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
 }
 for(size_t i=0;i<records->count;i++)if(records->entries[i].kind==5){
  if(!decode_name(frame,records->entries[i].owner,known,false,left,&end,error))return false;
  uint16_t visited[DNS_MAX_CHAIN+1]={0};visited[0]=records->entries[i].owner;
  for(size_t depth=0;depth<=DNS_MAX_CHAIN;depth++){
   bool present=false;uint16_t target=0;
   for(size_t j=0;j<records->count;j++)if(records->entries[j].kind==5){
    if(!decode_name(frame,records->entries[j].owner,known,false,right,&end,error))return false;
    if(name_same(left,right)){target=records->entries[j].data;present=true;break;}
   }
   if(!present)break;
   if(depth==DNS_MAX_CHAIN){*error=NETWORK_DNS_ERROR_UNSUPPORTED;return false;}
   if(!decode_name(frame,target,known,false,right,&end,error))return false;
   DnsName swap=*left;*left=*right;*right=swap;
   for(size_t prior=0;prior<=depth;prior++){
    if(!decode_name(frame,visited[prior],known,false,right,&end,error))return false;
    if(name_same(left,right)){*error=NETWORK_DNS_ERROR_MALFORMED;return false;}
   }
   visited[depth+1]=target;
  }
 }
 return true;
}
NetworkDnsQueryResult network_dns_query(uint16_t id,NetworkDnsFrame *out)
{
 if(id==0)return(NetworkDnsQueryResult){false,0,NETWORK_DNS_ERROR_INVALID_ID};
 for(size_t i=0;i<NETWORK_DNS_QUERY_BYTES;i++)out->bytes[i]=0;
 out->bytes[0]=(uint8_t)(id>>8);out->bytes[1]=(uint8_t)id;out->bytes[2]=1;out->bytes[5]=1;
 for(size_t i=0;i<sizeof(dns_question);i++)out->bytes[12+i]=dns_question[i];
 out->len=NETWORK_DNS_QUERY_BYTES;return(NetworkDnsQueryResult){true,NETWORK_DNS_QUERY_BYTES,NETWORK_DNS_ERROR_NONE};
}
NetworkDnsParseResult network_dns_parse(uint16_t expected_id,const NetworkDnsFrame *response)
{
 if(expected_id==0)return dns_error(NETWORK_DNS_ERROR_INVALID_ID,0);
 if(response->len>NETWORK_DNS_MAX_BYTES)return dns_error(NETWORK_DNS_ERROR_OVERSIZED,0);
 if(response->len<12)return dns_error(NETWORK_DNS_ERROR_TRUNCATED,0);
 uint16_t id=0;if(!dns_u16(response,0,&id))return dns_error(NETWORK_DNS_ERROR_TRUNCATED,0);
 if(id==0)return dns_error(NETWORK_DNS_ERROR_INVALID_ID,0);
 if(id!=expected_id)return(NetworkDnsParseResult){NETWORK_DNS_RESULT_UNRELATED,NETWORK_DNS_ERROR_NONE,0,{{0},0}};
 uint16_t questions=0;if(!dns_u16(response,4,&questions))return dns_error(NETWORK_DNS_ERROR_TRUNCATED,0);
 if(questions!=1)return dns_error(NETWORK_DNS_ERROR_MALFORMED,0);
 DnsBoundaries known={0};DnsName current={0},scratch={0};NetworkDnsError error=NETWORK_DNS_ERROR_NONE;size_t cursor=0;
 if(!decode_name(response,12,&known,true,&current,&cursor,&error))return dns_error(error,0);
 if(!dns_room(response,cursor,4))return dns_error(NETWORK_DNS_ERROR_TRUNCATED,0);
 bool canonical=name_canonical(&current)&&response->bytes[cursor]==0&&response->bytes[cursor+1]==1&&response->bytes[cursor+2]==0&&response->bytes[cursor+3]==1;
 cursor+=4;if(!canonical)return(NetworkDnsParseResult){NETWORK_DNS_RESULT_UNRELATED,NETWORK_DNS_ERROR_NONE,0,{{0},0}};
 uint16_t flags=0;if(!dns_u16(response,2,&flags))return dns_error(NETWORK_DNS_ERROR_TRUNCATED,0);
 if((flags&0x8000)==0)return dns_error(NETWORK_DNS_ERROR_MALFORMED,0);
 if(flags&0x7840)return dns_error(NETWORK_DNS_ERROR_UNSUPPORTED,0);
 if(flags&0x0200)return dns_error(NETWORK_DNS_ERROR_TRUNCATED,0);
 if(flags&15)return dns_error(NETWORK_DNS_ERROR_RCODE,(uint8_t)(flags&15));
 uint16_t answers16=0,authority16=0,additional16=0;
 if(!dns_u16(response,6,&answers16)||!dns_u16(response,8,&authority16)||!dns_u16(response,10,&additional16))return dns_error(NETWORK_DNS_ERROR_TRUNCATED,0);
 size_t answers=answers16,authority=authority16,additional=additional16;
 if(answers>DNS_MAX_ANSWERS||authority+additional>DNS_MAX_EXTRA)return dns_error(NETWORK_DNS_ERROR_UNSUPPORTED,0);
 DnsRecords records={0};records.count=answers+authority+additional;
 for(size_t i=0;i<records.count;i++)if(!read_record(response,&cursor,&known,i<answers,&scratch,&records.entries[i],&error))return dns_error(error,0);
 if(cursor!=response->len)return dns_error(NETWORK_DNS_ERROR_MALFORMED,0);
 if(!validate_aliases(response,&records,&known,&current,&scratch,&error))return dns_error(error,0);
 if(!decode_name(response,12,&known,false,&current,&cursor,&error))return dns_error(error,0);
 uint32_t minimum=UINT32_MAX;
 for(size_t depth=0;depth<=DNS_MAX_CHAIN;depth++){
  bool has_address=false,has_alias=false;uint8_t address[4]={0};uint16_t alias=0;
  for(size_t i=0;i<answers;i++){
   DnsRecord record=records.entries[i];if(!decode_name(response,record.owner,&known,false,&scratch,&cursor,&error))return dns_error(error,0);
   if(!name_same(&current,&scratch))continue;
   if(record.ttl<minimum)minimum=record.ttl;
   if(record.kind==1&&!has_address){for(size_t j=0;j<4;j++)address[j]=response->bytes[record.data+j];has_address=true;}
   else if(record.kind==5){alias=record.data;has_alias=true;}
  }
  if(has_address){NetworkDnsParseResult result={NETWORK_DNS_RESULT_ANSWER,NETWORK_DNS_ERROR_NONE,0,{{0},minimum}};for(size_t i=0;i<4;i++)result.answer.address[i]=address[i];return result;}
  if(!has_alias)return dns_error(NETWORK_DNS_ERROR_NO_ANSWER,0);
  if(depth==DNS_MAX_CHAIN)return dns_error(NETWORK_DNS_ERROR_UNSUPPORTED,0);
  if(!decode_name(response,alias,&known,false,&scratch,&cursor,&error))return dns_error(error,0);
  DnsName swap=current;current=scratch;scratch=swap;
 }
 return dns_error(NETWORK_DNS_ERROR_NO_ANSWER,0);
}


