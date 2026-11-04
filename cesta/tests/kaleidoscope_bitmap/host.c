#include <stdio.h>
#include <stdlib.h>
#include "kaleidoscope_bitmap.h"
#include "cases.h"

static KaleidoscopeBitmapOutbox outbox;
static KaleidoscopeQoiPixels pixels;
static size_t pixel_count;

static void fill(size_t count,uint32_t seed)
{pixel_count=count;for(size_t i=0;i<count;i++)pixels.values[i]=seed+(uint32_t)i*UINT32_C(0x01020304);}
static void receipt(const KaleidoscopeBitmapReceipt *r)
{printf(" %u %u %u,%u,%u,%u,%u %016llx %llu,%llu",(unsigned)r->sequence,(unsigned)r->geometry_present,r->geometry.x,r->geometry.y,r->geometry.width,r->geometry.height,r->geometry.scale,(unsigned long long)r->hash,(unsigned long long)r->metadata.viewport,(unsigned long long)r->metadata.source);}
int main(int argc,char **argv)
{
 if(argc!=2){return 2;}size_t n=(size_t)strtoull(argv[1],NULL,10);if(n>=sizeof(scenarios)/sizeof(scenarios[0]))return 3;fill(0,0);
 for(size_t i=0;i<scenarios[n].count;i++){Command c=scenarios[n].commands[i];
  if(c.op==0){KaleidoscopeBitmapIdentity id={(uint32_t)c.a,(uint32_t)c.b,c.c,(uint8_t)c.d};kaleidoscope_bitmap_init(&outbox,id,(uint32_t)c.e);puts("I");}
  else if(c.op==1){fill((size_t)c.a,(uint32_t)c.b);printf("F %zu\n",pixel_count);}
  else if(c.op==2){KaleidoscopeBitmapGeometry g={(uint16_t)c.a,(uint16_t)c.b,(uint8_t)c.c,(uint8_t)c.d,(uint8_t)c.e};KaleidoscopeBitmapQueueResult r=kaleidoscope_bitmap_queue(&outbox,&pixels,pixel_count,g,(KaleidoscopeBitmapMetadata){c.f,c.g});printf("Q %u %u\n",(unsigned)r.error,(unsigned)r.sequence);}
  else if(c.op==3){KaleidoscopeBitmapQueueResult r=kaleidoscope_bitmap_clear(&outbox,(KaleidoscopeBitmapMetadata){c.a,c.b});printf("C %u %u\n",(unsigned)r.error,(unsigned)r.sequence);}
  else if(c.op==4){KaleidoscopeBitmapPollResult p=kaleidoscope_bitmap_prepare(&outbox,&pixels,pixel_count);if(p.operation_present){KaleidoscopeBitmapOperation operation=p.operation;p=kaleidoscope_bitmap_resolve(&outbox,(KaleidoscopeBitmapAdmission){(uint8_t)c.a,c.b});printf("P %u %llu O %llu,%llu,%llu,%llu\n",(unsigned)p.state,(unsigned long long)p.error,(unsigned long long)operation.opcode,(unsigned long long)operation.surface,(unsigned long long)operation.packed,(unsigned long long)operation.control);}else printf("P %u %llu -\n",(unsigned)p.state,(unsigned long long)p.error);}
  else if(c.op==5){KaleidoscopeBitmapReceipt r={0};bool ok=kaleidoscope_bitmap_receive(&outbox,(uint32_t)c.a,c.b,&r);printf("R %u",(unsigned)ok);if(ok)receipt(&r);putchar('\n');}
  else if(c.op==6){if(c.a<pixel_count)pixels.values[c.a]^=(uint32_t)c.b;puts("M");}
  else if(c.op==7){kaleidoscope_bitmap_close(&outbox);puts("X");}
  else if(c.op==8)printf("S %u %u\n",(unsigned)kaleidoscope_bitmap_pending(&outbox),(unsigned)outbox.sequence);
 }
 return 0;
}
