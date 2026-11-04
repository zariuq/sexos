#include "kaleidoscope_html.h"

typedef struct HtmlSlice {size_t start,len;} HtmlSlice;
typedef struct HtmlAttribute {bool present;size_t start,len;} HtmlAttribute;
typedef struct HtmlOutput {
 uint8_t *bytes,*scratch;size_t capacity,len;bool truncated,space,pre;uint8_t newlines,quote;
} HtmlOutput;
static const uint8_t html_truncated[]="\n[page truncated]";

static bool html_space(uint8_t byte)
{return byte==' '||byte=='\t'||byte=='\n'||byte=='\r'||byte==0x0b||byte==0x0c;}
static uint8_t html_lower(uint8_t byte)
{return byte>='A'&&byte<='Z'?(uint8_t)(byte-'A'+'a'):byte;}
static bool html_name(const uint8_t *name,size_t length,const char *word)
{size_t n=0;while(word[n])n++;if(length!=n)return false;for(size_t i=0;i<n;i++)if(html_lower(name[i])!=(uint8_t)word[i])return false;return true;}
static bool html_window(const uint8_t *bytes,size_t at,size_t length,const char *word)
{size_t n=0;while(word[n])n++;if(at+n>length)return false;return html_name(bytes+at,n,word);}
static size_t html_room(const HtmlOutput *out)
{size_t reserve=sizeof(html_truncated)-1;return out->capacity>reserve?out->capacity-reserve:0;}
static void html_byte(HtmlOutput *out,uint8_t byte)
{if(out->len<html_room(out))out->bytes[out->len++]=byte;else out->truncated=true;}
static void html_text(HtmlOutput *out,uint8_t byte)
{
 if(out->pre){if(byte=='\r'){}else if(byte=='\n'){html_byte(out,'\n');if(out->newlines<UINT8_MAX)out->newlines++;}else if(byte=='\t'||byte==0x0c){html_byte(out,' ');out->newlines=0;}else{html_byte(out,byte);out->newlines=0;}out->space=false;return;}
 if(html_space(byte)){out->space=true;return;}if(out->space&&!out->newlines)html_byte(out,' ');if(out->newlines)for(size_t i=0;i<out->quote;i++){html_byte(out,'>');html_byte(out,' ');}out->space=false;out->newlines=0;html_byte(out,byte);
}
static void html_text_bytes(HtmlOutput *out,const uint8_t *bytes,size_t length)
{for(size_t i=0;i<length;i++)html_text(out,bytes[i]);}
static void html_raw(HtmlOutput *out,const uint8_t *bytes,size_t length)
{if(out->newlines)for(size_t i=0;i<out->quote;i++){html_byte(out,'>');html_byte(out,' ');}for(size_t i=0;i<length;i++)html_byte(out,bytes[i]);out->newlines=0;out->space=false;}
static void html_line(HtmlOutput *out,bool blank)
{uint8_t want=blank?2:1;while(out->newlines<want){html_byte(out,'\n');out->newlines++;}out->space=true;}
static size_t html_number(size_t number,uint8_t out[6])
{uint8_t reverse[6];size_t n=0;do{reverse[n++]=(uint8_t)('0'+number%10);number/=10;}while(number&&n<6);for(size_t i=0;i<n;i++)out[i]=reverse[n-1-i];return n;}
static size_t html_width(const uint8_t *bytes,size_t length)
{size_t width=0;for(size_t i=0;i<length;i++)if((bytes[i]&0xc0)!=0x80)width++;return width;}
static bool html_separator(const uint8_t *bytes,size_t at,size_t end)
{return at+3<=end&&bytes[at]==' '&&bytes[at+1]=='|'&&bytes[at+2]==' ';}
static void html_align(HtmlOutput *out,size_t start)
{
 if(out->truncated||start>=out->len){return;}size_t widths[16]={0},line=start;
 while(line<=out->len){size_t end=line;while(end<out->len&&out->bytes[end]!='\n')end++;size_t cell=line,column=0;for(size_t i=line;i<=end;){if(i==end||html_separator(out->bytes,i,end)){if(column<16){size_t width=html_width(out->bytes+cell,i-cell);if(width>widths[column])widths[column]=width;}column++;if(i==end)break;i+=3;cell=i;}else i++;}if(end==out->len)break;line=end+1;}
 size_t made=0;line=start;while(line<=out->len){size_t end=line;while(end<out->len&&out->bytes[end]!='\n')end++;size_t cell=line,column=0;for(size_t i=line;i<=end;){if(i==end||html_separator(out->bytes,i,end)){for(size_t k=cell;k<i;k++)out->scratch[made++]=out->bytes[k];if(i!=end){size_t width=html_width(out->bytes+cell,i-cell);if(column<16)while(width++<widths[column])out->scratch[made++]=' ';out->scratch[made++]=' ';out->scratch[made++]='|';out->scratch[made++]=' ';i+=3;cell=i;column++;}else break;}else i++;}if(end==out->len)break;out->scratch[made++]='\n';line=end+1;}
 if(start+made>html_room(out)){return;}for(size_t i=0;i<made;i++)out->bytes[start+i]=out->scratch[i];out->len=start+made;
}
static size_t html_finish(HtmlOutput *out)
{while(out->len&&out->bytes[out->len-1]=='\n')out->len--;if(out->truncated)for(size_t i=0;i<sizeof(html_truncated)-1&&out->len<out->capacity;i++)out->bytes[out->len++]=html_truncated[i];return out->len;}

static HtmlAttribute html_attribute(const uint8_t *attrs,size_t length,const char *want)
{
 size_t i=0;while(i<length){while(i<length&&html_space(attrs[i]))i++;size_t start=i;while(i<length&&!html_space(attrs[i])&&attrs[i]!='=')i++;size_t name_len=i-start;while(i<length&&html_space(attrs[i]))i++;if(i<length&&attrs[i]=='='){i++;while(i<length&&html_space(attrs[i]))i++;size_t value_start=i,value_end,next;if(i<length&&(attrs[i]=='"'||attrs[i]=='\'')){uint8_t quote=attrs[i++];value_start=i;while(i<length&&attrs[i]!=quote)i++;value_end=i;next=i<length?i+1:i;}else{while(i<length&&!html_space(attrs[i]))i++;value_end=i;next=i;}if(html_name(attrs+start,name_len,want))return(HtmlAttribute){true,value_start,value_end-value_start};i=next;}else if(!name_len)i++;}
 return(HtmlAttribute){0};
}
static bool html_contains_name(const uint8_t *bytes,size_t length,const char *word)
{size_t n=0;while(word[n])n++;for(size_t i=0;i+n<=length;i++)if(html_name(bytes+i,n,word))return true;return false;}
static bool html_utf8(uint32_t code,uint8_t out[4],size_t *length)
{if(code>0x10ffffu||(code>=0xd800u&&code<=0xdfffu)||(code<32u&&code!='\n')||(code>=0x7fu&&code<=0x9fu))return false;if(code<0x80u){out[0]=(uint8_t)code;*length=1;}else if(code<0x800u){out[0]=(uint8_t)(0xc0u|(code>>6));out[1]=(uint8_t)(0x80u|(code&63u));*length=2;}else if(code<0x10000u){out[0]=(uint8_t)(0xe0u|(code>>12));out[1]=(uint8_t)(0x80u|((code>>6)&63u));out[2]=(uint8_t)(0x80u|(code&63u));*length=3;}else{out[0]=(uint8_t)(0xf0u|(code>>18));out[1]=(uint8_t)(0x80u|((code>>12)&63u));out[2]=(uint8_t)(0x80u|((code>>6)&63u));out[3]=(uint8_t)(0x80u|(code&63u));*length=4;}return true;}
typedef struct HtmlEntity {const char *name;uint32_t code;} HtmlEntity;
static bool html_entity(const uint8_t *input,size_t length,uint8_t out[4],size_t *count,size_t *used)
{
 size_t end=0;while(end<length&&end<12&&input[end]!=';')end++;if(end>=length||end>=12||input[end]!=';')return false;uint32_t code=0;if(end&&input[0]=='#'){size_t at=1;unsigned radix=10;if(at<end&&(input[at]=='x'||input[at]=='X')){radix=16;at++;}if(at==end)return false;for(;at<end;at++){int digit=input[at]>='0'&&input[at]<='9'?input[at]-'0':input[at]>='a'&&input[at]<='f'?input[at]-'a'+10:input[at]>='A'&&input[at]<='F'?input[at]-'A'+10:-1;if(digit<0||(unsigned)digit>=radix||code>(UINT32_MAX-(uint32_t)digit)/radix)return false;code=code*radix+(uint32_t)digit;}}else{static const HtmlEntity named[]={{"amp",'&'},{"lt",'<'},{"gt",'>'},{"quot",'"'},{"apos",'\''},{"nbsp",' '},{"copy",0xa9},{"mdash",0x2014},{"ndash",0x2013},{"hellip",0x2026},{"lsquo",0x2018},{"rsquo",0x2019},{"ldquo",0x201c},{"rdquo",0x201d},{"bull",0x2022},{"middot",0xb7},{"times",0xd7},{"divide",0xf7},{"deg",0xb0},{"euro",0x20ac},{"pound",0xa3},{"yen",0xa5},{"cent",0xa2},{"sect",0xa7},{"para",0xb6},{"reg",0xae},{"trade",0x2122},{"larr",0x2190},{"rarr",0x2192},{"uarr",0x2191},{"darr",0x2193},{"plusmn",0xb1},{"frac12",0xbd},{"laquo",0xab},{"raquo",0xbb},{"iexcl",0xa1},{"iquest",0xbf}};if(html_name(input,end,"shy")){*count=0;*used=end+1;return true;}bool found=false;for(size_t i=0;i<sizeof(named)/sizeof(named[0]);i++)if(html_name(input,end,named[i].name)){code=named[i].code;found=true;break;}if(!found)return false;}if(!html_utf8(code,out,count))return false;*used=end+1;return true;
}
static uint32_t html_cp1252(uint8_t byte)
{static const uint16_t high[32]={0x20ac,0xfffd,0x201a,0x0192,0x201e,0x2026,0x2020,0x2021,0x02c6,0x2030,0x0160,0x2039,0x0152,0xfffd,0x017d,0xfffd,0xfffd,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014,0x02dc,0x2122,0x0161,0x203a,0x0153,0xfffd,0x017e,0x0178};return byte>=0x80&&byte<0xa0?high[byte-0x80]:byte;}
static bool html_latin(const uint8_t *html,size_t length)
{size_t limit=length<1024?length:1024;for(size_t i=0;i+8<=limit;i++)if(html_window(html,i,limit,"charset=")){size_t at=i+8;if(at<limit&&(html[at]=='"'||html[at]=='\''))at++;static const char *names[]={"windows-1252","iso-8859-1","latin1","iso8859-1","cp1252"};for(size_t n=0;n<5;n++){size_t k=0;while(names[n][k])k++;if(at+k<=limit&&html_name(html+at,k,names[n]))return true;}return false;}return false;}
static HtmlSlice html_title(const uint8_t *html,size_t length)
{for(size_t i=0;i+6<=length;i++)if(html_window(html,i,length,"<title")){size_t start=i;while(start<length&&html[start]!='>')start++;if(start==length)return(HtmlSlice){0};start++;for(size_t end=start;end+8<=length;end++)if(html_window(html,end,length,"</title>")){bool text=false;for(size_t k=start;k<end;k++)if(!html_space(html[k]))text=true;return text?(HtmlSlice){start,end-start}:(HtmlSlice){0};}}return(HtmlSlice){0};}

bool kaleidoscope_html_looks_like(const uint8_t *bytes,size_t length)
{if(!bytes&&length)return false;size_t start=0;while(start<length&&html_space(bytes[start]))start++;size_t end=start+15<length?start+15:length,available=end-start;if(available>=14&&html_name(bytes+start,14,"<!doctype html"))return true;return available>=5&&html_name(bytes+start,5,"<html");}
static uint8_t html_skip_kind(const uint8_t *name,size_t length)
{if(html_name(name,length,"style"))return 1;if(html_name(name,length,"head"))return 2;if(html_name(name,length,"template"))return 3;if(html_name(name,length,"noscript"))return 4;if(html_name(name,length,"svg"))return 5;return 0;}
static bool html_skip_matches(uint8_t kind,const uint8_t *name,size_t length)
{return(kind==1&&html_name(name,length,"style"))||(kind==2&&html_name(name,length,"head"))||(kind==3&&html_name(name,length,"template"))||(kind==4&&html_name(name,length,"noscript"))||(kind==5&&html_name(name,length,"svg"));}
static bool html_block(const uint8_t *name,size_t length)
{static const char *words[]={"p","div","section","article","header","footer","main","nav","ul","ol","table","blockquote","pre","hr","dl","form","details","fieldset","figure"};for(size_t i=0;i<sizeof(words)/sizeof(words[0]);i++)if(html_name(name,length,words[i]))return true;return false;}
static const char *html_inline2(const uint8_t *name,size_t length)
{if(html_name(name,length,"mark"))return"==";if(html_name(name,length,"del")||html_name(name,length,"s")||html_name(name,length,"strike"))return"~~";if(html_name(name,length,"ins"))return"++";if(html_name(name,length,"u"))return"__";return NULL;}
static uint8_t html_inline(const uint8_t *name,size_t length)
{if(html_name(name,length,"code")||html_name(name,length,"kbd"))return 96;if(html_name(name,length,"strong")||html_name(name,length,"b"))return'*';if(html_name(name,length,"em")||html_name(name,length,"i")||html_name(name,length,"cite"))return'_';return 0;}
static bool html_tag_self_closing(const uint8_t *body,size_t length)
{return length&&body[length-1]=='/';}
static void html_script(KaleidoscopeHtmlWorkspace *workspace,HtmlOutput *out,const uint8_t *source,size_t length,bool plain,uint8_t *ran,uint8_t *refused)
{bool ok=plain&&length<=KALEIDOSCOPE_SCRIPT_MAX_SOURCE;if(ok){workspace->script_source.len=length;for(size_t i=0;i<length;i++)workspace->script_source.bytes[i]=source[i];ok=kaleidoscope_script_run(&workspace->script_source,&workspace->script_workspace,&workspace->script_result)==0;}if(ok){if(*ran<UINT8_MAX)(*ran)++;html_text_bytes(out,workspace->script_result.bytes,workspace->script_result.len);}else if(*refused<UINT8_MAX)(*refused)++;}

uint8_t kaleidoscope_html_convert(const uint8_t *html,size_t length,uint8_t *output,size_t capacity,
 KaleidoscopeHtmlWorkspace *workspace,KaleidoscopeHtmlConverted *converted)
{
 if((!html&&length)||!output||!workspace||!converted||length>KALEIDOSCOPE_HTML_MAX_SOURCE||capacity>KALEIDOSCOPE_HTML_MAX_OUTPUT)return KALEIDOSCOPE_SCRIPT_LIMIT;
 *converted=(KaleidoscopeHtmlConverted){0};HtmlOutput out={.bytes=output,.scratch=workspace->alignment,.capacity=capacity,.space=true,.newlines=2};bool latin=html_latin(html,length);HtmlSlice links[KALEIDOSCOPE_HTML_MAX_LINKS]={{0}},abbr={0};size_t link_count=0,open_link=SIZE_MAX,table=SIZE_MAX,depth=0;uint8_t skip=0,scripts_run=0,scripts_refused=0;uint16_t lists[4]={0};
 HtmlSlice title=html_title(html,length);if(title.len){html_text_bytes(&out,(const uint8_t *)"Title:",6);out.space=true;for(size_t i=0;i<title.len;){if(html[title.start+i]=='&'){uint8_t decoded[4];size_t n,used;if(html_entity(html+title.start+i+1,title.len-i-1,decoded,&n,&used)){html_text_bytes(&out,decoded,n);i+=1+used;continue;}}html_text(&out,html[title.start+i++]);}html_line(&out,true);}
 for(size_t i=0;i<length;){
  uint8_t byte=html[i];
  if(byte=='<'){
   if(i+4<=length&&html[i]=='<'&&html[i+1]=='!'&&html[i+2]=='-'&&html[i+3]=='-'){size_t at=i+4;while(at+3<=length&&!(html[at]=='-'&&html[at+1]=='-'&&html[at+2]=='>'))at++;i=at+3<=length?at+3:length;continue;}
   size_t close=i;while(close<length&&html[close]!='>')close++;if(close==length)break;size_t tag_start=i+1,tag_len=close-tag_start;i=close+1;bool end_tag=tag_len&&html[tag_start]=='/';size_t body_start=tag_start+(end_tag?1:0),body_len=tag_len-(end_tag?1:0),name_len=0;while(name_len<body_len&&!html_space(html[body_start+name_len])&&html[body_start+name_len]!='/')name_len++;const uint8_t *name=html+body_start;
   if(skip){if(end_tag&&html_skip_matches(skip,name,name_len))skip=0;continue;}
   if(!end_tag&&html_name(name,name_len,"script")){size_t script_end=i;while(script_end+9<=length&&!html_window(html,script_end,length,"</script>"))script_end++;bool found=script_end+9<=length;size_t source_len=(found?script_end:length)-i;size_t attrs_start=body_start+name_len,attrs_len=body_len-name_len;HtmlAttribute src=html_attribute(html+attrs_start,attrs_len,"src"),type=html_attribute(html+attrs_start,attrs_len,"type");bool plain=!src.present&&(!type.present||html_name(html+attrs_start+type.start,type.len,"text/javascript"));html_script(workspace,&out,html+i,source_len,plain,&scripts_run,&scripts_refused);i=found?script_end+9:length;continue;}
   if(!end_tag){uint8_t kind=html_skip_kind(name,name_len);if(kind&&!html_tag_self_closing(html+body_start,body_len)){skip=kind;continue;}}
   bool heading=name_len==2&&html_lower(name[0])=='h'&&name[1]>='1'&&name[1]<='6';size_t attrs_start=body_start+name_len,attrs_len=body_len-name_len;
   if(heading){html_line(&out,true);if(!end_tag)html_text_bytes(&out,(const uint8_t *)(name[1]<='2'?"# ":"## "),name[1]<='2'?2:3);}
   else if(html_name(name,name_len,"br"))html_line(&out,false);
   else if(html_name(name,name_len,"li")&&!end_tag){html_line(&out,false);size_t indent=depth?depth:1;for(size_t k=1;k<indent;k++)html_raw(&out,(const uint8_t *)"  ",2);uint16_t next=depth&&depth<=4?lists[depth-1]:0;if(next){uint8_t digits[6];size_t n=html_number(next,digits);html_raw(&out,digits,n);html_raw(&out,(const uint8_t *)". ",2);if(depth<=4&&lists[depth-1]<UINT16_MAX)lists[depth-1]++;}else html_raw(&out,(const uint8_t *)"- ",2);}
   else if((html_name(name,name_len,"dt")||html_name(name,name_len,"dd"))&&!end_tag){html_line(&out,false);if(html_name(name,name_len,"dd"))html_raw(&out,(const uint8_t *)"    ",4);}
   else if(html_name(name,name_len,"tr")&&end_tag)html_line(&out,false);
   else if((html_name(name,name_len,"td")||html_name(name,name_len,"th"))&&!end_tag){if(!out.newlines){html_text_bytes(&out,(const uint8_t *)" |",2);out.space=true;}}
   else if(html_block(name,name_len)){out.pre=false;bool list=html_name(name,name_len,"ol")||html_name(name,name_len,"ul");html_line(&out,!(list&&depth>(end_tag?1u:0u)));if(html_name(name,name_len,"table")){if(!end_tag)table=out.len;else if(table!=SIZE_MAX){html_align(&out,table);table=SIZE_MAX;}}else if(list){if(end_tag){if(depth)depth--;}else{uint16_t first=1;if(html_name(name,name_len,"ol")){HtmlAttribute start=html_attribute(html+attrs_start,attrs_len,"start");if(start.present){size_t value=0;bool valid=start.len>0;for(size_t k=0;k<start.len;k++){uint8_t c=html[attrs_start+start.start+k];if(c<'0'||c>'9'){valid=false;break;}value=value*10+(c-'0');if(value>UINT16_MAX){value=UINT16_MAX;break;}}if(valid&&value)first=(uint16_t)value;}}if(depth<4)lists[depth]=html_name(name,name_len,"ol")?first:0;depth++;}}else if(html_name(name,name_len,"blockquote")){if(end_tag){if(out.quote)out.quote--;}else if(out.quote<4)out.quote++;}else if(html_name(name,name_len,"hr")){html_text_bytes(&out,(const uint8_t *)"----",4);html_line(&out,true);}if(html_name(name,name_len,"pre")&&!end_tag){out.pre=true;if(i<length&&html[i]=='\n')i++;else if(i+2<=length&&html[i]=='\r'&&html[i+1]=='\n')i+=2;}}
   else{const char *marker=html_inline2(name,name_len);uint8_t mark=html_inline(name,name_len);if(marker){if(end_tag)out.space=false;html_text_bytes(&out,(const uint8_t *)marker,2);if(!end_tag)out.space=false;}
    else if(html_name(name,name_len,"figcaption")){html_line(&out,false);if(!end_tag){html_text_bytes(&out,(const uint8_t *)"Figure:",7);out.space=true;}}
    else if(mark){if(end_tag){out.space=false;html_text(&out,mark);}else{html_text(&out,mark);out.space=false;}}
    else if(html_name(name,name_len,"input")&&!end_tag){HtmlAttribute kind=html_attribute(html+attrs_start,attrs_len,"type");const uint8_t *kind_bytes=kind.present?html+attrs_start+kind.start:(const uint8_t *)"text";size_t kind_len=kind.present?kind.len:4;bool checked=html_attribute(html+attrs_start,attrs_len,"checked").present||html_contains_name(html+attrs_start,attrs_len,"checked");if(html_name(kind_bytes,kind_len,"hidden")){}else if(html_name(kind_bytes,kind_len,"checkbox"))html_text_bytes(&out,(const uint8_t *)(checked?"[x]":"[ ]"),3);else if(html_name(kind_bytes,kind_len,"radio"))html_text_bytes(&out,(const uint8_t *)(checked?"(*)":"( )"),3);else if(html_name(kind_bytes,kind_len,"submit")||html_name(kind_bytes,kind_len,"button")||html_name(kind_bytes,kind_len,"reset")){html_text_bytes(&out,(const uint8_t *)"[button:",8);out.space=true;HtmlAttribute value=html_attribute(html+attrs_start,attrs_len,"value");if(value.present)html_text_bytes(&out,html+attrs_start+value.start,value.len);else html_text_bytes(&out,(const uint8_t *)(html_name(kind_bytes,kind_len,"reset")?"Reset":"Submit"),html_name(kind_bytes,kind_len,"reset")?5:6);html_text(&out,']');}else{html_text_bytes(&out,(const uint8_t *)"[input",6);HtmlAttribute value=html_attribute(html+attrs_start,attrs_len,"value");if(!value.present)value=html_attribute(html+attrs_start,attrs_len,"placeholder");if(value.present&&value.len){html_text(&out,':');out.space=true;html_text_bytes(&out,html+attrs_start+value.start,value.len);}html_text(&out,']');}}
    else if((html_name(name,name_len,"button")||html_name(name,name_len,"textarea")||html_name(name,name_len,"select"))&&!end_tag){const char *open=html_name(name,name_len,"button")?"[button:":html_name(name,name_len,"select")?"[select:":"[text:";html_text_bytes(&out,(const uint8_t *)open,html_name(name,name_len,"button")?8:html_name(name,name_len,"select")?8:6);out.space=true;}
    else if((html_name(name,name_len,"button")||html_name(name,name_len,"textarea")||html_name(name,name_len,"select"))&&end_tag){out.space=false;html_text(&out,']');}
    else if(html_name(name,name_len,"option")&&!end_tag){if(out.len&&out.bytes[out.len-1]!=':'&&!(out.len>1&&out.bytes[out.len-2]==':'&&out.bytes[out.len-1]==' ')){out.space=false;html_text(&out,' ');html_text(&out,'|');}out.space=true;}
    else if(html_name(name,name_len,"sup")||html_name(name,name_len,"sub")){if(!end_tag){out.space=false;html_text(&out,html_name(name,name_len,"sup")?'^':'_');out.space=false;}}
    else if(html_name(name,name_len,"q")){if(end_tag)out.space=false;html_text(&out,'"');if(!end_tag)out.space=false;}
    else if(html_name(name,name_len,"summary")&&!end_tag){html_line(&out,false);html_text_bytes(&out,(const uint8_t *)"> ",2);}
    else if(html_name(name,name_len,"summary")&&end_tag)html_line(&out,false);
    else if(html_name(name,name_len,"abbr")&&!end_tag){HtmlAttribute title_attr=html_attribute(html+attrs_start,attrs_len,"title");abbr=title_attr.present?(HtmlSlice){attrs_start+title_attr.start,title_attr.len}:(HtmlSlice){0};}
    else if(html_name(name,name_len,"abbr")&&end_tag){if(abbr.len){html_text(&out,'(');out.space=false;html_text_bytes(&out,html+abbr.start,abbr.len);out.space=false;html_text(&out,')');abbr=(HtmlSlice){0};}}
    else if(html_name(name,name_len,"img")&&!end_tag){html_text_bytes(&out,(const uint8_t *)"[image",6);HtmlAttribute alt=html_attribute(html+attrs_start,attrs_len,"alt");bool visible=false;if(alt.present)for(size_t k=0;k<alt.len;k++)if(!html_space(html[attrs_start+alt.start+k]))visible=true;if(visible){html_text(&out,':');out.space=true;html_text_bytes(&out,html+attrs_start+alt.start,alt.len);}html_text(&out,']');}
    else if(html_name(name,name_len,"a")){if(!end_tag){HtmlAttribute href=html_attribute(html+attrs_start,attrs_len,"href");if(href.present&&link_count<KALEIDOSCOPE_HTML_MAX_LINKS){links[link_count]=(HtmlSlice){attrs_start+href.start,href.len};open_link=link_count++;}}else if(open_link!=SIZE_MAX){uint8_t digits[6];size_t n=html_number(open_link+1,digits);html_text(&out,'[');out.space=false;for(size_t k=0;k<n;k++)html_byte(&out,digits[k]);html_byte(&out,']');open_link=SIZE_MAX;}}
   }
   continue;
  }
  if(skip){i++;continue;}if(byte=='&'){uint8_t decoded[4];size_t n,used;if(html_entity(html+i+1,length-i-1,decoded,&n,&used)){html_text_bytes(&out,decoded,n);i+=1+used;continue;}}
  if(latin&&byte>=0x80){uint8_t decoded[4];size_t n;if(html_utf8(html_cp1252(byte),decoded,&n))html_text_bytes(&out,decoded,n);}else html_text(&out,byte);i++;
 }
 out.pre=false;if(link_count){html_line(&out,true);html_text_bytes(&out,(const uint8_t *)"Links:",6);for(size_t i=0;i<link_count;i++){html_line(&out,false);uint8_t digits[6];size_t n=html_number(i+1,digits);html_byte(&out,'[');for(size_t k=0;k<n;k++)html_byte(&out,digits[k]);html_byte(&out,']');html_byte(&out,' ');out.space=false;out.newlines=0;size_t from=out.len;html_text_bytes(&out,html+links[i].start,links[i].len);if(!out.truncated){converted->target_start[i]=from;converted->target_end[i]=out.len;}}}
 converted->len=html_finish(&out);converted->links=link_count;converted->truncated=out.truncated;converted->scripts_run=scripts_run;converted->scripts_refused=scripts_refused;return 0;
}


