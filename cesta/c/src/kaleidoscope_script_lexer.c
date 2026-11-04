#include "kaleidoscope_script_lexer.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static bool script_alpha(uint8_t c){return(c>='A'&&c<='Z')||(c>='a'&&c<='z');}
static bool script_digit(uint8_t c){return c>='0'&&c<='9';}
static bool script_alnum(uint8_t c){return script_alpha(c)||script_digit(c);}
static bool script_ident_start(uint8_t c){return script_alpha(c)||c=='_'||c=='$';}
static bool script_current_ident(const KaleidoscopeScriptSource *source,const KaleidoscopeScriptLexer *state,uint64_t word,size_t length)
{
 if(state->current.kind!=KALEIDOSCOPE_TOKEN_IDENT||state->current.len!=length)return false;
 for(size_t i=0;i<length;i++){if(source->bytes[state->current.start+i]!=(uint8_t)(word>>(i*8)))return false;}return true;
}
static bool script_after_value(const KaleidoscopeScriptSource *source,const KaleidoscopeScriptLexer *state)
{
 uint8_t kind=state->current.kind;if(kind==KALEIDOSCOPE_TOKEN_NUMBER||kind==KALEIDOSCOPE_TOKEN_STRING||kind==KALEIDOSCOPE_TOKEN_TEMPLATE||kind==KALEIDOSCOPE_TOKEN_REGEX)return true;
 if(kind==KALEIDOSCOPE_TOKEN_IDENT)return !(script_current_ident(source,state,UINT64_C(0x6e7275746572),6)||script_current_ident(source,state,UINT64_C(0x666f65707974),6)||script_current_ident(source,state,UINT64_C(0x65736163),4)||script_current_ident(source,state,UINT64_C(0x6e69),2)||script_current_ident(source,state,UINT64_C(0x6574656c6564),6)||script_current_ident(source,state,UINT64_C(0x776f726874),5)||script_current_ident(source,state,UINT64_C(0x6f64),2)||script_current_ident(source,state,UINT64_C(0x65736c65),4));
 return kind==KALEIDOSCOPE_TOKEN_PUNCT&&(state->current.punct==')'||state->current.punct==']');
}
static bool script_decimal(const KaleidoscopeScriptSource *source,size_t start,size_t end,double *out)
{
 size_t at=start;double value=0.0;bool digits=false;while(at<end&&script_digit(source->bytes[at])){digits=true;value=value*10.0+(double)(source->bytes[at]-'0');at++;}
 if(at<end&&source->bytes[at]=='.'){at++;double scale=1.0;while(at<end&&script_digit(source->bytes[at])){digits=true;scale*=10.0;value+=(double)(source->bytes[at]-'0')/scale;at++;}}
 if(!digits){return false;}int exponent=0;if(at<end&&(source->bytes[at]=='e'||source->bytes[at]=='E')){at++;bool negative=false;if(at<end&&(source->bytes[at]=='+'||source->bytes[at]=='-')){negative=source->bytes[at]=='-';at++;}if(at==end||!script_digit(source->bytes[at]))return false;while(at<end&&script_digit(source->bytes[at])){if(exponent<10000)exponent=exponent*10+(source->bytes[at]-'0');at++;}if(negative)exponent=-exponent;}
 if(at!=end){return false;}bool negative_exponent=exponent<0;unsigned magnitude=(unsigned)(negative_exponent?-exponent:exponent);double factor=1.0,base=10.0;while(magnitude){if(magnitude&1u)factor*=base;base*=base;magnitude>>=1;}value=negative_exponent?value/factor:value*factor;*out=value;return true;
}
static bool script_number(const KaleidoscopeScriptSource *source,size_t start,size_t end,double *out)
{
 if(end-start>2&&source->bytes[start]=='0'&&(source->bytes[start+1]=='x'||source->bytes[start+1]=='X')){double value=0.0;if(start+2==end)return false;for(size_t i=start+2;i<end;i++){uint8_t c=source->bytes[i];uint8_t d;if(script_digit(c))d=(uint8_t)(c-'0');else if(c>='a'&&c<='f')d=(uint8_t)(c-'a'+10);else if(c>='A'&&c<='F')d=(uint8_t)(c-'A'+10);else return false;value=value*16.0+d;}*out=value;return true;}
 return script_decimal(source,start,end,out);
}
static bool script_three(const KaleidoscopeScriptSource *source,size_t at,uint8_t a,uint8_t b,uint8_t c)
{return at+3<=source->len&&source->bytes[at]==a&&source->bytes[at+1]==b&&source->bytes[at+2]==c;}
static bool script_two(const KaleidoscopeScriptSource *source,size_t at,uint8_t a,uint8_t b)
{return at+2<=source->len&&source->bytes[at]==a&&source->bytes[at+1]==b;}
void kaleidoscope_script_lexer_init(KaleidoscopeScriptLexer *state){*state=(KaleidoscopeScriptLexer){0};}
uint8_t kaleidoscope_script_lexer_next(const KaleidoscopeScriptSource *source,KaleidoscopeScriptLexer *state,KaleidoscopeScriptToken *out)
{
 if(!source||!state||!out||source->len>KALEIDOSCOPE_SCRIPT_MAX_SOURCE||state->at>source->len){return KALEIDOSCOPE_SCRIPT_LIMIT;}*out=(KaleidoscopeScriptToken){0};bool newline=false;
 for(;;){
  while(state->at<source->len&&(source->bytes[state->at]==' '||source->bytes[state->at]=='\t'||source->bytes[state->at]=='\n'||source->bytes[state->at]=='\r'||source->bytes[state->at]==0x0b||source->bytes[state->at]==0x0c)){newline|=source->bytes[state->at]=='\n';state->at++;}
  if(script_two(source,state->at,'/','/')){while(state->at<source->len&&source->bytes[state->at]!='\n')state->at++;continue;}
  if(script_two(source,state->at,'/','*')){size_t i=state->at+2;while(i+1<source->len&&!script_two(source,i,'*','/'))i++;if(i+1>=source->len)return KALEIDOSCOPE_SCRIPT_SYNTAX;state->at=i+2;continue;}
  break;
 }
 out->newline=newline;if(state->at>=source->len){out->kind=KALEIDOSCOPE_TOKEN_END;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}uint8_t c=source->bytes[state->at];
 if(c=='/'&&!script_after_value(source,state)){
  size_t start=state->at+1,i=start;bool in_class=false;while(i<source->len&&(source->bytes[i]!='/'||in_class)){if(source->bytes[i]==92)i++;else if(source->bytes[i]=='[')in_class=true;else if(source->bytes[i]==']')in_class=false;else if(source->bytes[i]=='\n')return KALEIDOSCOPE_SCRIPT_SYNTAX;i++;}
  if(i>=source->len){return KALEIDOSCOPE_SCRIPT_SYNTAX;}size_t flags=i+1;while(flags<source->len&&script_alpha(source->bytes[flags]))flags++;out->kind=KALEIDOSCOPE_TOKEN_REGEX;out->start=start;out->len=i-start;out->aux_start=i+1;out->aux_len=flags-i-1;state->at=flags;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;
 }
 if(script_digit(c)||(c=='.'&&state->at+1<source->len&&script_digit(source->bytes[state->at+1]))){
  size_t start=state->at;while(state->at<source->len&&(script_alnum(source->bytes[state->at])||source->bytes[state->at]=='.'||((source->bytes[state->at]=='+'||source->bytes[state->at]=='-')&&state->at>start&&(source->bytes[state->at-1]=='e'||source->bytes[state->at-1]=='E'))))state->at++;
  out->kind=KALEIDOSCOPE_TOKEN_NUMBER;out->start=start;out->len=state->at-start;if(!script_number(source,start,state->at,&out->number))return KALEIDOSCOPE_SCRIPT_SYNTAX;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;
 }
 if(c==96){size_t start=state->at+1,i=start;while(i<source->len&&source->bytes[i]!=96)i++;if(i>=source->len)return KALEIDOSCOPE_SCRIPT_SYNTAX;out->kind=KALEIDOSCOPE_TOKEN_TEMPLATE;out->start=start;out->len=i-start;state->at=i+1;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}
 if(c=='"'||c=='\''){size_t start=state->at+1,i=start;while(i<source->len&&source->bytes[i]!=c){if(source->bytes[i]==92)i++;if(i<source->len&&source->bytes[i]=='\n')return KALEIDOSCOPE_SCRIPT_SYNTAX;i++;}if(i>=source->len)return KALEIDOSCOPE_SCRIPT_SYNTAX;out->kind=KALEIDOSCOPE_TOKEN_STRING;out->start=start;out->len=i-start;state->at=i+1;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}
 if(script_ident_start(c)){size_t start=state->at;while(state->at<source->len&&(script_ident_start(source->bytes[state->at])||script_digit(source->bytes[state->at])))state->at++;out->kind=KALEIDOSCOPE_TOKEN_IDENT;out->start=start;out->len=state->at-start;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}
 if(script_two(source,state->at,'?','.')&&(state->at+2>=source->len||!script_digit(source->bytes[state->at+2]))){out->kind=KALEIDOSCOPE_TOKEN_PUNCT;out->punct='O';state->at+=2;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}
#define SCRIPT_OP3(A,B,C,K) if(script_three(source,state->at,(A),(B),(C))){out->kind=KALEIDOSCOPE_TOKEN_PUNCT;out->punct=(K);state->at+=3;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}
#define SCRIPT_OP2(A,B,K) if(script_two(source,state->at,(A),(B))){out->kind=KALEIDOSCOPE_TOKEN_PUNCT;out->punct=(K);state->at+=2;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}
 SCRIPT_OP3('=','=','=', 'E') SCRIPT_OP3('!','=','=', 'N') SCRIPT_OP3('&','&','=', 'z') SCRIPT_OP3('|','|','=', 'y') SCRIPT_OP3('?','?','=', 'q') SCRIPT_OP3('*','*','=', 'W') SCRIPT_OP3('>','>','>', 'u') SCRIPT_OP3('.','.','.', 'S')
 SCRIPT_OP2('=','=', 'e') SCRIPT_OP2('!','=', 'n') SCRIPT_OP2('<','=', 'L') SCRIPT_OP2('>','=', 'G') SCRIPT_OP2('&','&', '&') SCRIPT_OP2('|','|', '|') SCRIPT_OP2('?','?', 'C') SCRIPT_OP2('*','*', 'X') SCRIPT_OP2('|','=', 'v') SCRIPT_OP2('&','=', 'w') SCRIPT_OP2('^','=', 'x') SCRIPT_OP2('<','<', 'l') SCRIPT_OP2('>','>', 'r') SCRIPT_OP2('+','+', 'I') SCRIPT_OP2('-','-', 'D') SCRIPT_OP2('+','=', 'P') SCRIPT_OP2('-','=', 'M') SCRIPT_OP2('*','=', 'T') SCRIPT_OP2('/','=', 'Q') SCRIPT_OP2('%','=', 'R') SCRIPT_OP2('=','>', 'F')
#undef SCRIPT_OP2
#undef SCRIPT_OP3
 if(c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||c=='('||c==')'||c=='='||c==';'||c==','||c=='.'||c=='<'||c=='>'||c=='!'||c=='?'||c==':'||c=='{'||c=='}'||c=='['||c==']'||c=='&'||c=='|'||c=='^'||c=='~'){out->kind=KALEIDOSCOPE_TOKEN_PUNCT;out->punct=c=='&'?'a':c=='|'?'o':c;state->at++;state->current=*out;return KALEIDOSCOPE_SCRIPT_OK;}
 return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}


