#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kaleidoscope_png.h"
#include "cases.h"
static uint64_t bytes_hash(const uint8_t *bytes,size_t count){uint64_t h=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<count;i++){h^=bytes[i];h*=UINT64_C(0x100000001b3);}return h;}
static uint64_t pixels_hash(const KaleidoscopeQoiPixels *pixels){uint64_t h=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<KALEIDOSCOPE_QOI_MAX_PIXELS;i++)for(size_t k=0;k<4;k++){h^=(uint8_t)(pixels->values[i]>>(8*k));h*=UINT64_C(0x100000001b3);}return h;}
static int zero_bytes(const uint8_t *p,size_t n){for(size_t i=0;i<n;i++)if(p[i])return 0;return 1;}
static int zero_pixels(const KaleidoscopeQoiPixels *p){for(size_t i=0;i<KALEIDOSCOPE_QOI_MAX_PIXELS;i++)if(p->values[i])return 0;return 1;}
int main(int argc,char **argv){if(argc!=2){return 2;}size_t n=(size_t)strtoull(argv[1],NULL,10);if(n>=sizeof(cases)/sizeof(cases[0]))return 3;KaleidoscopeQoiSource source={0};source.len=cases[n].length;size_t copy=source.len<KALEIDOSCOPE_QOI_MAX_INPUT?source.len:KALEIDOSCOPE_QOI_MAX_INPUT;memcpy(source.bytes,cases[n].bytes,copy);KaleidoscopePngScratch scratch;KaleidoscopeQoiPixels pixels;memset(&scratch,0xa5,sizeof scratch);for(size_t i=0;i<KALEIDOSCOPE_QOI_MAX_PIXELS;i++)pixels.values[i]=UINT32_C(0xdeadbeef);KaleidoscopePngResult r=kaleidoscope_png_decode(&source,&scratch,&pixels);if(r.error)printf("E %u %d %d\n",(unsigned)r.error,zero_bytes(scratch.bytes,sizeof scratch.bytes),zero_pixels(&pixels));else printf("O %zu %zu %016llx %016llx\n",r.width,r.height,(unsigned long long)pixels_hash(&pixels),(unsigned long long)bytes_hash(scratch.bytes,sizeof scratch.bytes));return 0;}
