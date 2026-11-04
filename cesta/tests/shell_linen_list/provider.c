#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "shell_linen_list.h"
void sexos_fail(void){exit(88);}
int main(void){unsigned long long a[6];while(scanf("%llu %llu %llu %llu %llu %llu",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5])==6){
 ShellLinenItems items={.count=a[0]};if(items.count>16)return 88;
 for(size_t i=0;i<items.count;++i){items.items[i].id=a[5]==2?i%2+1:i+1;items.items[i].len=a[4]<126?a[4]:126;for(size_t j=0;j<items.items[i].len;++j)items.items[i].name[j]=a[5]==1?(uint8_t)(i*7+j*13):(uint8_t)(65+(i+j)%26);}
 ShellLinenList out=shell_linen_list(&items,a[1],a[2],a[3]);printf("%zu %zu %u %zu %u %u %zu ",out.first,out.rows,(unsigned)out.has_selected,out.has_selected?out.selected_row:0,out.row_y,out.row_h,out.text.len);for(size_t i=0;i<512;++i)printf("%02x",out.text.bytes[i]);puts("");
 }return 0;}
