#include <stdio.h>
#include <stdlib.h>
#include "kaleidoscope_presentation.h"
#include "cases.h"

static KaleidoscopePresentation state;
static void snapshot(void)
{printf(" | %u %llu %u %u %u %u %u %u %u %u\n",(unsigned)state.mode,(unsigned long long)state.generation,(unsigned)state.text_present,(unsigned)state.layout_present,(unsigned)state.applied,(unsigned)state.clear_present,(unsigned)state.clear_applied,(unsigned)state.sequence_floor,(unsigned)state.exhausted,(unsigned)kaleidoscope_presentation_can_queue(&state,1));}
int main(int argc,char **argv)
{
 if(argc!=2){return 2;}size_t n=(size_t)strtoull(argv[1],NULL,10);if(n>=sizeof(scenarios)/sizeof(scenarios[0]))return 3;
 for(size_t i=0;i<scenarios[n].count;i++){Command c=scenarios[n].commands[i];printf("A %llu",(unsigned long long)c.op);
  if(c.op==0){kaleidoscope_presentation_init(&state);}
  else if(c.op==1)printf(" %u",(unsigned)kaleidoscope_presentation_text_applied(&state,c.a,c.b,c.c));
  else if(c.op==2){KaleidoscopeBitmapGeometry g={(uint16_t)c.a,(uint16_t)c.b,(uint8_t)c.c,(uint8_t)c.d,(uint8_t)c.e};printf(" %u",(unsigned)kaleidoscope_presentation_expect(&state,g,c.f,(uint32_t)c.g,(uint32_t)c.h,c.i));}
  else if(c.op==3){KaleidoscopeBitmapReceipt r={(uint32_t)c.a,(bool)c.b,{(uint16_t)c.c,(uint16_t)c.d,(uint8_t)c.e,(uint8_t)c.f,(uint8_t)c.g},c.h,{c.i,c.j}};printf(" %u",(unsigned)kaleidoscope_presentation_applied(&state,&r,c.k,(uint32_t)c.l,c.m));}
  else if(c.op==4)printf(" %u",(unsigned)kaleidoscope_presentation_expect_clear(&state,c.a,(uint32_t)c.b));
  else if(c.op==5){KaleidoscopeBitmapReceipt r={(uint32_t)c.a,(bool)c.b,{0},c.c,{c.d,c.e}};printf(" %u",(unsigned)kaleidoscope_presentation_cleared(&state,&r,c.f));}
  else if(c.op==6)printf(" %u",(unsigned)kaleidoscope_presentation_invalidate(&state));
  else if(c.op==7){KaleidoscopePresentationSelect s=kaleidoscope_presentation_select(&state,(uint8_t)c.a);printf(" %u %u",(unsigned)s.ok,(unsigned)s.changed);}
  else if(c.op==8)state.generation=c.a;
  snapshot();
 }
 return 0;
}
