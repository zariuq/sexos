#include "spindle_sextime.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

#define SEXTIME_UNIX_OFFSET INT64_C(1356048000)
#define SEXTIME_SECONDS_PER_DAY INT64_C(86400)
#define SEXTIME_EPOCH_UNIX_DAYS INT64_C(15695)
#define SEXTIME_EPOCH_LONG_COUNT_DAYS INT64_C(1872000)
typedef uint8_t SextimeRow[SPINDLE_SEXTIME_LINE_MAX];
typedef uint8_t SextimeText[19];
typedef struct SextimeOut {SextimeRow *bytes;size_t len;} SextimeOut;
static const SextimeText SEXTIME_TEXT_SEXTIME="SexTime ";
static const SextimeText SEXTIME_TEXT_SECONDS=" s";
static const SextimeText SEXTIME_TEXT_UTC="UTC ";
static const SextimeText SEXTIME_TEXT_LONG_COUNT="Long Count ";
static const SextimeText SEXTIME_TEXT_BEFORE="before 0.0.0.0.0";
static const SextimeText SEXTIME_TEXT_GMT=" (GMT 584283)";

static void sextime_put(SextimeOut *out,uint8_t byte)
{if(out->len<SPINDLE_SEXTIME_LINE_MAX)(*out->bytes)[out->len++]=byte;}
static void sextime_bytes(SextimeOut *out,const SextimeText *text,size_t len)
{if(len>sizeof(*text)){sexos_fail();return;}for(size_t i=0;i<len;++i)sextime_put(out,(*text)[i]);}
static uint64_t sextime_magnitude(int64_t value)
{return value<0?(uint64_t)(-(value+1))+1:(uint64_t)value;}
static void sextime_unsigned(SextimeOut *out,uint64_t value,size_t width)
{uint8_t digits[20]={0};size_t n=0;for(;;){digits[n++]=(uint8_t)('0'+value%10);value/=10;if(!value&&n>=width)break;}while(n)sextime_put(out,digits[--n]);}
static void sextime_signed(SextimeOut *out,int64_t value)
{if(value<0)sextime_put(out,'-');sextime_unsigned(out,sextime_magnitude(value),1);}
static int64_t sextime_div_euclid(int64_t value,int64_t divisor)
{int64_t quotient=value/divisor,remainder=value%divisor;if(remainder<0)--quotient;return quotient;}
static int64_t sextime_rem_euclid(int64_t value,int64_t divisor)
{int64_t remainder=value%divisor;return remainder<0?remainder+divisor:remainder;}

bool spindle_sextime_parse(const SpindleSextimeInput *input,int64_t *unix_seconds)
{
 if(input->too_long||!input->len||input->len>SPINDLE_SEXTIME_INPUT_MAX)return false;
 size_t at=0;bool negative=input->bytes[0]=='-';if(negative)++at;if(at==input->len)return false;
 int64_t value=0;for(;at<input->len;++at){uint8_t byte=input->bytes[at];if(byte<'0'||byte>'9')return false;int64_t digit=(int64_t)(byte-'0');if(value>(INT64_MAX-digit)/10)return false;value=value*10+digit;}
 if(negative){if(value==INT64_MIN)return false;value=-value;}*unix_seconds=value;return true;
}

SpindleSextimeLines spindle_sextime_render(int64_t unix_seconds)
{
 SpindleSextimeLines lines={0};if(unix_seconds<INT64_MIN+SEXTIME_UNIX_OFFSET)return lines;
 int64_t seconds=unix_seconds-SEXTIME_UNIX_OFFSET,day=sextime_div_euclid(seconds,SEXTIME_SECONDS_PER_DAY);
 int64_t second_of_day=sextime_rem_euclid(seconds,SEXTIME_SECONDS_PER_DAY);
 if(day>INT64_MAX-SEXTIME_EPOCH_UNIX_DAYS||day<INT64_MIN+SEXTIME_EPOCH_UNIX_DAYS)return lines;
 int64_t unix_days=day+SEXTIME_EPOCH_UNIX_DAYS;
 uint64_t abs_days=sextime_magnitude(unix_days);if(abs_days>UINT64_C(366000000))return lines;
 int64_t z=unix_days+INT64_C(719468),era=sextime_div_euclid(z,INT64_C(146097)),doe=sextime_rem_euclid(z,INT64_C(146097));
 int64_t yoe=(doe-doe/1460+doe/36524-doe/146096)/365;
 int64_t doy=doe-(365*yoe+yoe/4-yoe/100),mp=(5*doy+2)/153;
 uint8_t civil_day=(uint8_t)(doy-(153*mp+2)/5+1),month=(uint8_t)(mp<10?mp+3:mp-9);
 int64_t year=yoe+era*400+(month<=2?1:0);
 uint8_t hour=(uint8_t)(second_of_day/3600),minute=(uint8_t)(second_of_day/60%60),second=(uint8_t)(second_of_day%60);
 {SextimeOut first={&lines.rows[0],0};sextime_bytes(&first,&SEXTIME_TEXT_SEXTIME,8);sextime_signed(&first,seconds);sextime_bytes(&first,&SEXTIME_TEXT_SECONDS,2);lines.lens[0]=first.len;}
 {SextimeOut civil={&lines.rows[1],0};sextime_bytes(&civil,&SEXTIME_TEXT_UTC,4);sextime_signed(&civil,year);
 sextime_put(&civil,'-');sextime_unsigned(&civil,month,2);sextime_put(&civil,'-');sextime_unsigned(&civil,civil_day,2);
 sextime_put(&civil,'T');sextime_unsigned(&civil,hour,2);sextime_put(&civil,':');sextime_unsigned(&civil,minute,2);sextime_put(&civil,':');sextime_unsigned(&civil,second,2);sextime_put(&civil,'Z');lines.lens[1]=civil.len;}
 {SextimeOut count={&lines.rows[2],0};sextime_bytes(&count,&SEXTIME_TEXT_LONG_COUNT,11);
 if(day<INT64_MIN+SEXTIME_EPOCH_LONG_COUNT_DAYS)sextime_bytes(&count,&SEXTIME_TEXT_BEFORE,16);
 else {int64_t days=day+SEXTIME_EPOCH_LONG_COUNT_DAYS;if(days<0)sextime_bytes(&count,&SEXTIME_TEXT_BEFORE,16);else{uint64_t left=(uint64_t)days,baktun=left/UINT64_C(144000);left%=UINT64_C(144000);uint64_t katun=left/7200;left%=7200;uint64_t tun=left/360;left%=360;uint64_t winal=left/20,kin=left%20;
  sextime_unsigned(&count,baktun,1);sextime_put(&count,'.');sextime_unsigned(&count,katun,1);sextime_put(&count,'.');sextime_unsigned(&count,tun,1);sextime_put(&count,'.');sextime_unsigned(&count,winal,1);sextime_put(&count,'.');sextime_unsigned(&count,kin,1);sextime_bytes(&count,&SEXTIME_TEXT_GMT,13);}}
 lines.lens[2]=count.len;}lines.ok=true;return lines;
}


