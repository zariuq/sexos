#include "linen.c"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static unsigned answer_calls;
static bool refuse_next;
static SexosRequest answered_request;
static uint64_t answered_value;

SexosReply sexos_pdx_call(uint64_t slot,uint64_t opcode,uint64_t a,uint64_t b,uint64_t c)
{(void)slot;(void)opcode;(void)a;(void)b;(void)c;return(SexosReply){0,0};}
SexosMessage sexos_listen(SexosListener *listener,uint64_t slot)
{(void)listener;(void)slot;return(SexosMessage){0};}
SexosRequest sexos_receive_request(bool correlated)
{(void)correlated;return(SexosRequest){0};}
uint64_t sexos_rpc_register(void){return 1;}
uint64_t sexos_answer_request(const SexosRequest *request,uint64_t value)
{
 ++answer_calls;answered_request=*request;answered_value=value;
 if(refuse_next){refuse_next=false;return SEXOS_RPC_BUSY;}
 return 0;
}
void sexos_line(const char *format,...){(void)format;}
void sexos_spin_loop_hint(void){}
_Noreturn void sexos_spin_forever(void){assert(!"unexpected spin");for(;;){}}
void sexos_yield(void){}

static SexosRequest request(uint64_t id,uint32_t task,uint64_t opcode,uint64_t a,uint64_t b,uint64_t c)
{
 return(SexosRequest){.message={.type_id=opcode,.arg0=a,.arg1=b,.arg2=c,.caller_pd=3,.caller_task=task},.id=id};
}

int main(void)
{
 Linen l={.session={.next_id=UINT64_C(0x8000000100000001)},.correlated=true};
 l.capture=linen_capture_new(3,3);
 l.session.slots[0]=(LinenSlot){.present=true,.object={.object_id=UINT64_C(0xfedcba9876543210),
  .kind=OBJECT_KIND_SESSION,.owner_pd=3,.name_len=24,.sexfiles_object_id=15}};
 for(size_t i=0;i<24;++i)l.session.slots[0].object.name[i]=(uint8_t)(240+i);

 SexosRequest begin=request(101,3,LINEN_CAPTURE_OP_BEGIN,1,LINEN_CAPTURE_VERSION,0);
 refuse_next=true;linen_dispatch_request(&l,&begin);
 assert(l.answer.present && answer_calls==1 && answered_request.id==101 && answered_value==0);
 assert(linen_capture_answer_poll(&l.answer) && !l.answer.present && answer_calls==2);
 LinenCaptureFrame retained=l.capture.frame;

 /* Mutation after Begin cannot alter the retained capture. */
 l.session.slots[0].object.object_id=77;l.session.slots[0].object.name[0]=1;
 LinenCaptureFrame received={0};uint64_t id=102;
 for(size_t offset=0;;offset+=LINEN_CAPTURE_READ_BYTES){
  SexosRequest read=request(id++,3,LINEN_CAPTURE_OP_READ,1,offset,0);
  linen_dispatch_request(&l,&read);
  assert(!l.answer.present && answered_request.id==read.id);
  if(answered_value==0)break;
  assert(linen_capture_append(&received,answered_value));
 }
 assert(received.len==retained.len && !memcmp(received.bytes,retained.bytes,retained.len));
 LinenCaptureTable decoded={0};assert(linen_capture_decode(&received,&decoded));
 assert(decoded.rows[0].id==UINT64_C(0xfedcba9876543210) && decoded.rows[0].len==24);
 for(size_t i=0;i<24;++i)assert(decoded.rows[0].name[i]==(uint8_t)(240+i));
 SexosRequest end=request(id++,3,LINEN_CAPTURE_OP_END,1,0,0);
 linen_dispatch_request(&l,&end);assert(answered_value==0 && !l.capture.active);

 SexosRequest denied=request(id++,4,LINEN_CAPTURE_OP_BEGIN,2,LINEN_CAPTURE_VERSION,0);
 linen_dispatch_request(&l,&denied);
 assert(answered_value==linen_capture_error(LINEN_CAPTURE_DENIED) && !l.capture.active);

 /* Complete request envelopes survive FIFO deferral. */
 l.defer_len=0;
 for(size_t i=0;i<LINEN_DEFER_CAP;++i){
  SexosRequest queued=request(UINT64_C(0x8000000000000000)+i,100+(uint32_t)i,0x500+i,i,i+1,i+2);
  linen_defer_stash(&l,&queued);
 }
 for(size_t i=0;i<LINEN_DEFER_CAP;++i){
  DeferredRequest queued={0};assert(linen_defer_pop(&l,&queued));
  assert(queued.request.id==UINT64_C(0x8000000000000000)+i);
  assert(queued.request.message.caller_task==100+i && queued.request.message.type_id==0x500+i);
  assert(queued.request.message.arg0==i && queued.request.message.arg2==i+2);
 }
 assert(!l.defer_len);

 /* Saturation answers the correlated request instead of silently dropping it. */
 for(size_t i=0;i<LINEN_DEFER_CAP;++i){
  SexosRequest queued=request(200+i,3,0x600+i,0,0,0);linen_defer_stash(&l,&queued);
 }
 SexosRequest overflow=request(999,3,0x777,0,0,0);linen_defer_stash(&l,&overflow);
 assert(answered_request.id==999 && answered_value==REPLY_NOT_READY);

 /* Unknown correlated operations complete their own obligation. */
 l.defer_len=0;SexosRequest unknown=request(1000,3,0xdead,0,0,0);
 linen_dispatch_request(&l,&unknown);
 assert(answered_request.id==1000 && answered_value==REPLY_CAP_INVALID);

 /* Non-shell legacy callers receive one attempt and do not occupy the retained slot. */
 SexosRequest legacy=request(0,12,OP_LINEN_OPEN_INTENT,UINT64_C(0xffff),0,0);
 legacy.message.caller_pd=12;linen_dispatch_request(&l,&legacy);
 assert(!l.answer.present && answered_request.id==0 && answered_request.message.caller_pd==12);
 puts("PASS: actual Linen dispatch preserves capture coherence, request IDs, sender tasks, FIFO deferral and pending answers");
}
