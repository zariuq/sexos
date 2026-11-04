#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kaleidoscope_html.h"
#include "cases.h"

static KaleidoscopeHtmlWorkspace workspace;
static uint8_t output[KALEIDOSCOPE_HTML_MAX_OUTPUT];

int main(int argc,char **argv)
{
 if(argc!=2){return 2;}size_t index=(size_t)strtoul(argv[1],NULL,10);if(index>=sizeof(cases)/sizeof(cases[0]))return 3;
 memset(&workspace,0,sizeof(workspace));memset(output,0,sizeof(output));KaleidoscopeHtmlConverted converted;
 uint8_t error=kaleidoscope_html_convert(cases[index].bytes,cases[index].length,output,cases[index].capacity,&workspace,&converted);
 if(error){printf("E %u\n",(unsigned)error);return 0;}printf("O ");for(size_t i=0;i<converted.len;i++)printf("%02x",output[i]);
 printf(" %zu %zu %u %u %u %u",converted.len,converted.links,(unsigned)converted.truncated,(unsigned)converted.scripts_run,(unsigned)converted.scripts_refused,(unsigned)kaleidoscope_html_looks_like(cases[index].bytes,cases[index].length));
 for(size_t i=0;i<converted.links;i++){printf(" %zu:%zu",converted.target_start[i],converted.target_end[i]);}putchar('\n');return 0;
}
