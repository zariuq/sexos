#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kaleidoscope_qoi.h"
#include "cases.h"

static uint64_t pixel_hash(const KaleidoscopeQoiPixels *pixels)
{
 uint64_t hash=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<KALEIDOSCOPE_QOI_MAX_PIXELS;i++)for(size_t k=0;k<4;k++){hash^=(uint8_t)(pixels->values[i]>>(8*k));hash*=UINT64_C(0x100000001b3);}return hash;
}
static int all_zero(const KaleidoscopeQoiPixels *pixels)
{for(size_t i=0;i<KALEIDOSCOPE_QOI_MAX_PIXELS;i++)if(pixels->values[i])return 0;return 1;}
int main(int argc,char **argv)
{
 if(argc!=2){return 2;}size_t n=(size_t)strtoull(argv[1],NULL,10);if(n>=sizeof(cases)/sizeof(cases[0]))return 3;
 KaleidoscopeQoiSource source={0};source.len=cases[n].length;size_t copy=source.len<KALEIDOSCOPE_QOI_MAX_INPUT?source.len:KALEIDOSCOPE_QOI_MAX_INPUT;memcpy(source.bytes,cases[n].bytes,copy);
 KaleidoscopeQoiDecoder decoder;KaleidoscopeQoiPixels pixels;for(size_t i=0;i<KALEIDOSCOPE_QOI_MAX_PIXELS;i++)pixels.values[i]=UINT32_C(0xdeadbeef);
 uint8_t init=kaleidoscope_qoi_init(&source,&decoder);if(init){printf("I %u\n",(unsigned)init);return 0;}
 if(cases[n].action==1)source.bytes[0]^=1;
 if(cases[n].action==2){kaleidoscope_qoi_cancel(&decoder,&pixels);KaleidoscopeQoiStep again=kaleidoscope_qoi_advance(&decoder,&source,&pixels);printf("C %u %zu %016llx %d\n",(unsigned)again.error,decoder.produced,(unsigned long long)pixel_hash(&pixels),all_zero(&pixels));return 0;}
 printf("S");KaleidoscopeQoiStep step={0};for(size_t turn=0;turn<32;turn++){step=kaleidoscope_qoi_advance(&decoder,&source,&pixels);printf(":%u:%u:%zu",(unsigned)step.error,(unsigned)step.state,decoder.produced);if(step.error||step.state==KALEIDOSCOPE_QOI_READY)break;}
 printf(" %zu %zu %016llx %d",decoder.width,decoder.height,(unsigned long long)pixel_hash(&pixels),all_zero(&pixels));
 if(step.error){pixels.values[0]=1;KaleidoscopeQoiStep again=kaleidoscope_qoi_advance(&decoder,&source,&pixels);printf(" P%u:%d",(unsigned)again.error,all_zero(&pixels));}putchar('\n');return 0;
}
