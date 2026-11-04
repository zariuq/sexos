#include "shell_linen_list.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

void sexos_fail(void);
static void list_invalid(void){sexos_fail();for(;;){}}
static void list_line(ShellLinenList *out,const ShellLinenLine *line,size_t cols)
{
 if(line->len>128)list_invalid();
 size_t limit=cols?cols-1:0;if(limit>80)limit=80;
 size_t n=line->len<limit?line->len:limit;
 for(size_t i=0;i<n;++i){
  if(out->text.len+1>=SHELL_TEXT_CAPACITY)return;
  uint8_t b=line->bytes[i];out->text.bytes[out->text.len]=(i+1==n && line->len>n)?'~':(b>=32 && b<=126)?b:'?';++out->text.len;
 }
 if(out->text.len<SHELL_TEXT_CAPACITY)out->text.bytes[out->text.len++]=10;
}
ShellLinenList shell_linen_list(const ShellLinenItems *items,uint64_t selected,uint32_t width,uint32_t height)
{
 if(items->count>SHELL_LINEN_LIST_CAPACITY)list_invalid();
 uint32_t scale=width>=256?2:1,line_h=9*scale;
 uint32_t cols=(width>16?width-16:0)/(6*scale);if(cols<1)cols=1;if(cols>80)cols=80;
 ShellLinenList out={0};out.row_y=36+2*line_h;out.row_h=2*line_h;
 if(cols<4 || height<out.row_y+out.row_h)return out;
 size_t rows=(height-out.row_y)/out.row_h;if(rows>5)rows=5;if(rows>items->count)rows=items->count;
 out.rows=rows;if(!rows)return out;
 size_t index=0;for(size_t i=0;i<items->count;++i)if(items->items[i].id==selected){index=i;break;}
 out.first=index/rows*rows;if(out.rows>items->count-out.first)out.rows=items->count-out.first;
 static const ShellLinenLine title={{76,73,78,69,78,32,47,32,70,73,76,69,83},13};
 static const ShellLinenLine help={{74,47,75,32,83,69,76,69,67,84,32,32,69,78,84,69,82,32,79,80,69,78},22};
 static const ShellLinenLine empty={{0},0};
 static const uint8_t untitled[10]={40,85,78,84,73,84,76,69,68,41};
 list_line(&out,&title,cols);list_line(&out,&help,cols);
 for(size_t row=0;row<out.rows;++row){
  ShellLinenListItem item=items->items[out.first+row];if(item.len>SHELL_LINEN_LIST_NAME_CAPACITY)list_invalid();
  ShellLinenLine line={0};bool chosen=item.id==selected;line.bytes[0]=chosen?'>':' ';line.bytes[1]=' ';
  size_t n=item.len?item.len:10;
  for(size_t i=0;i<n;++i)line.bytes[i+2]=item.len?item.name[i]:untitled[i];
  line.len=n+2;list_line(&out,&line,cols);list_line(&out,&empty,cols);
  if(chosen){out.has_selected=true;out.selected_row=row;}
 }
 return out;
}


