#include "network_job_machine.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static bool request_equal(NetworkJobRequest a,NetworkJobRequest b)
{
 return a.seq==b.seq&&a.kind==b.kind&&a.offset==b.offset&&a.length==b.length&&a.ticket==b.ticket&&a.a==b.a;
}
bool network_job_identity_equal(NetworkJobIdentity a,NetworkJobIdentity b)
{return a.pd==b.pd&&a.task==b.task;}
static bool live_state(NetworkJobState state)
{return state==NETWORK_JOB_CONNECTING||state==NETWORK_JOB_SENDING||state==NETWORK_JOB_RECEIVING;}
static bool active_state(NetworkJobState state)
{return live_state(state)||state==NETWORK_JOB_COMPLETE;}
static size_t minimum_size(size_t a,size_t b){return a<b?a:b;}
void network_job_machine_init(NetworkJobMachine *m)
{
 *m=(NetworkJobMachine){0};m->next_ticket=1;m->state=NETWORK_JOB_IDLE;
}
NetworkJobSnapshot network_job_machine_snapshot(const NetworkJobMachine *m)
{return(NetworkJobSnapshot){m->state,m->ticket,m->staged,m->sent,m->response_len};}
NetworkJobStreamSnapshot network_job_machine_stream_snapshot(const NetworkJobMachine *m)
{
 if(!m->streaming)return(NetworkJobStreamSnapshot){0};
 return(NetworkJobStreamSnapshot){true,m->response_len,m->consumed,m->response_len-m->consumed};
}
bool network_job_machine_can_transmit(const NetworkJobMachine *m,uint32_t ticket)
{return m->owner_present&&m->ticket==ticket&&active_state(m->state);}
static size_t pending_slot(const NetworkJobMachine *m)
{
 for(size_t offset=0;offset<NETWORK_JOB_OWNER_SLOTS;offset++){
  size_t slot=(m->reply_cursor+offset)%NETWORK_JOB_OWNER_SLOTS;
  if(m->pending[slot].present)return slot;
 }
 return NETWORK_JOB_OWNER_SLOTS;
}
NetworkJobPendingReply network_job_machine_pending_reply(const NetworkJobMachine *m)
{
 size_t slot=pending_slot(m);if(slot==NETWORK_JOB_OWNER_SLOTS)return(NetworkJobPendingReply){0};
 return(NetworkJobPendingReply){true,m->pending[slot].owner,m->pending[slot].word};
}
void network_job_machine_reply_accepted(NetworkJobMachine *m)
{
 size_t slot=pending_slot(m);if(slot!=NETWORK_JOB_OWNER_SLOTS){m->pending[slot].present=false;m->reply_cursor=(slot+1)%NETWORK_JOB_OWNER_SLOTS;}
}
void network_job_machine_reply_deferred(NetworkJobMachine *m)
{
 size_t slot=pending_slot(m);if(slot!=NETWORK_JOB_OWNER_SLOTS)m->reply_cursor=(slot+1)%NETWORK_JOB_OWNER_SLOTS;
}
static void scrub(NetworkJobMachine *m,NetworkJobWire wire)
{
 for(size_t i=0;i<sizeof(m->request);i++)m->request[i]=0;
 for(size_t i=0;i<sizeof(m->response);i++)m->response[i]=0;
 m->request_len=0;m->staged=0;m->sent=0;m->response_len=0;m->finished_send=false;
 m->deadline=0;m->last_now=0;m->streaming=false;m->duplex=false;m->finish_requested=false;
 m->consumed=0;m->idle_deadline=0;m->resolution_present=false;m->resolution=(NetworkJobResolution){0};
 network_job_wire_abort(wire);
}
static void fail(NetworkJobMachine *m,NetworkJobWire wire){scrub(m,wire);m->state=NETWORK_JOB_FAILED;}
bool network_job_machine_retire(NetworkJobMachine *m,NetworkJobIdentity owner,NetworkJobWire wire)
{
 if(!network_job_wire_valid(wire))return false;
 bool held=m->owner_present&&network_job_identity_equal(m->owner,owner);
 if(held){scrub(m,wire);m->owner_present=false;m->ticket=0;m->seq=0;m->state=NETWORK_JOB_IDLE;}
 for(size_t slot=0;slot<NETWORK_JOB_OWNER_SLOTS;slot++){
  if(m->cached[slot].present&&network_job_identity_equal(m->cached[slot].owner,owner)){
   m->cached[slot].present=false;m->pending[slot].present=false;
  }
 }
 return held;
}
static bool respond(NetworkJobMachine *m,size_t slot,NetworkJobIdentity owner,NetworkJobRequest r,bool success,uint32_t value)
{
 NetworkJobReceipt receipt={true,owner,r,success?network_job_reply(r.seq,value):network_job_error(r.seq,value)};
 m->pending[slot]=receipt;
 if(!(m->cached[slot].present&&r.seq<=m->cached[slot].request.seq))m->cached[slot]=receipt;
 return true;
}
static bool expired(const NetworkJobMachine *m,uint64_t now)
{return now<m->last_now||now>=m->deadline||(m->streaming&&now>=m->idle_deadline);}
static void stream_progress(NetworkJobMachine *m,uint64_t now)
{
 uint64_t next=UINT64_MAX-now<NETWORK_JOB_STREAM_IDLE_TICKS?m->deadline:now+NETWORK_JOB_STREAM_IDLE_TICKS;
 m->idle_deadline=next<m->deadline?next:m->deadline;
}
static bool receive_bounded(NetworkJobMachine *m,NetworkJobWire wire,size_t budget,size_t *received,bool *drained)
{
 size_t maximum=m->duplex?NETWORK_JOB_MAX_DUPLEX_BYTES:NETWORK_JOB_MAX_STREAM_BYTES;
 if(m->response_len==maximum){uint8_t extra=0;NetworkJobWriteView out={&extra,1};NetworkJobWireResult result=network_job_wire_receive(wire,out,0,1);if(!result.ok||result.count!=0)return false;*received=0;*drained=true;return true;}
 size_t occupied=m->response_len-m->consumed;
 size_t at=m->response_len%NETWORK_JOB_STREAM_RING_BYTES;
 size_t room=minimum_size(NETWORK_JOB_STREAM_RING_BYTES-occupied,NETWORK_JOB_STREAM_RING_BYTES-at);
 room=minimum_size(room,budget);room=minimum_size(room,maximum-m->response_len);
 if(room==0){*received=0;*drained=false;return true;}
 NetworkJobWireResult result;{NetworkJobWriteView out={m->response,sizeof(m->response)};result=network_job_wire_receive(wire,out,at,room);}if(!result.ok||result.count>room)return false;
 m->response_len+=result.count;*received=result.count;*drained=result.count==0;return true;
}
static bool apply(NetworkJobMachine *m,NetworkJobIdentity owner,NetworkJobRequest r,uint64_t now,
 const NetworkJobResolution *resolution,uint32_t admission_error,NetworkJobWire wire,uint32_t *value,uint32_t *error)
{
 if(!owner.pd||!owner.task||!network_job_request_valid(r)){*error=NETWORK_JOB_INVALID;return false;}
 if(r.kind==NETWORK_JOB_BEGIN||r.kind==NETWORK_JOB_BEGIN_RESOLVED||r.kind==NETWORK_JOB_BEGIN_STREAM_RESOLVED||r.kind==NETWORK_JOB_BEGIN_DUPLEX_RESOLVED){
  if(m->owner_present){*error=NETWORK_JOB_BUSY;return false;}
  if(m->next_ticket>NETWORK_JOB_MAX_TICKET){*error=NETWORK_JOB_EXHAUSTED;return false;}
  bool needs_resolution=r.kind!=NETWORK_JOB_BEGIN;
  if(needs_resolution){
   if(admission_error){*error=admission_error;return false;}
   if(!resolution||!network_job_identity_equal(resolution->owner,owner)||resolution->generation!=(uint32_t)r.a||
      now<resolution->observed_tick||now>=resolution->expires_tick){*error=NETWORK_JOB_STALE;return false;}
  }
  m->ticket=m->next_ticket;m->next_ticket++;
  m->owner_present=true;m->owner=owner;m->seq=r.seq;m->state=NETWORK_JOB_STAGING;
  m->request_len=r.length;m->duplex=r.kind==NETWORK_JOB_BEGIN_DUPLEX_RESOLVED;
  m->streaming=r.kind==NETWORK_JOB_BEGIN_STREAM_RESOLVED||m->duplex;
  if(needs_resolution){m->resolution_present=true;m->resolution=*resolution;}else m->resolution_present=false;
  *value=m->ticket;return true;
 }
 if(!m->owner_present||!network_job_identity_equal(m->owner,owner)){*error=NETWORK_JOB_DENIED;return false;}
 if(r.ticket!=m->ticket){*error=NETWORK_JOB_STALE;return false;}
 if(m->streaming&&live_state(m->state)&&r.kind!=NETWORK_JOB_CANCEL&&r.kind!=NETWORK_JOB_RELEASE){
  if(expired(m,now)){fail(m,wire);*error=NETWORK_JOB_TIMEOUT;return false;}m->last_now=now;
 }
 switch(r.kind){
  case NETWORK_JOB_WRITE:{
   if(m->duplex&&m->state!=NETWORK_JOB_STAGING){
    if(!live_state(m->state)||m->finish_requested){*error=NETWORK_JOB_BUSY;return false;}
    if((size_t)r.offset!=m->staged||r.length>NETWORK_JOB_MAX_DUPLEX_BYTES-m->staged){*error=NETWORK_JOB_INVALID;return false;}
    size_t end=m->staged+r.length;if(end-m->sent>NETWORK_JOB_MAX_BYTES){*error=NETWORK_JOB_BUSY;return false;}
    for(size_t i=0;i<r.length;i++)m->request[(m->staged+i)%NETWORK_JOB_MAX_BYTES]=(uint8_t)(r.a>>(8*i));
    m->staged=end;*value=(uint32_t)end;return true;
   }
   if(m->state!=NETWORK_JOB_STAGING){*error=NETWORK_JOB_BUSY;return false;}
   if((size_t)r.offset!=m->staged||r.length>m->request_len-m->staged){*error=NETWORK_JOB_INVALID;return false;}
   for(size_t i=0;i<r.length;i++)m->request[m->staged+i]=(uint8_t)(r.a>>(8*i));
   m->staged+=r.length;*value=(uint32_t)m->staged;return true;
  }
  case NETWORK_JOB_START:{
   if(m->state!=NETWORK_JOB_STAGING||m->staged!=m->request_len){*error=NETWORK_JOB_BUSY;return false;}
   uint64_t duration=m->streaming?NETWORK_JOB_STREAM_TOTAL_TICKS:UINT64_C(5000);
   if(UINT64_MAX-now<duration){fail(m,wire);*error=NETWORK_JOB_TIMEOUT;return false;}
   m->deadline=now+duration;m->last_now=now;m->state=NETWORK_JOB_CONNECTING;
   if(m->streaming)stream_progress(m,now);
   NetworkJobWireResult result=network_job_wire_begin(wire,m->ticket);if(!result.ok){fail(m,wire);*error=result.error;return false;}
   *value=0;return true;
  }
  case NETWORK_JOB_STATUS:*value=network_job_status(m->state,m->response_len);return true;
  case NETWORK_JOB_READ:{
   if(m->streaming){
    if(!(m->state==NETWORK_JOB_RECEIVING||m->state==NETWORK_JOB_COMPLETE||(m->duplex&&(m->state==NETWORK_JOB_CONNECTING||m->state==NETWORK_JOB_SENDING)))){*error=NETWORK_JOB_BUSY;return false;}
    if((size_t)r.offset!=m->consumed){*error=NETWORK_JOB_INVALID;return false;}
    if(r.length>m->response_len-m->consumed){*error=NETWORK_JOB_BUSY;return false;}
    uint32_t word=0;for(size_t i=0;i<r.length;i++){size_t cell=(m->consumed+i)%NETWORK_JOB_STREAM_RING_BYTES;word|=(uint32_t)m->response[cell]<<(8*i);m->response[cell]=0;}
    m->consumed+=r.length;stream_progress(m,now);*value=word;return true;
   }
   if(m->state!=NETWORK_JOB_COMPLETE){*error=NETWORK_JOB_BUSY;return false;}
   if((size_t)r.offset>m->response_len||r.length>m->response_len-r.offset){*error=NETWORK_JOB_INVALID;return false;}
   uint32_t word=0;for(size_t i=0;i<r.length;i++)word|=(uint32_t)m->response[r.offset+i]<<(8*i);*value=word;return true;
  }
  case NETWORK_JOB_FINISH:
   if(!m->duplex||!active_state(m->state)){*error=NETWORK_JOB_BUSY;return false;}m->finish_requested=true;*value=0;return true;
  case NETWORK_JOB_CANCEL:scrub(m,wire);m->state=NETWORK_JOB_CANCELLED;*value=0;return true;
  case NETWORK_JOB_RELEASE:scrub(m,wire);m->owner_present=false;m->ticket=0;m->seq=0;m->state=NETWORK_JOB_IDLE;*value=0;return true;
  default:*error=NETWORK_JOB_INVALID;return false;
 }
}
bool network_job_machine_handle_resolved(NetworkJobMachine *m,NetworkJobIdentity owner,NetworkJobRequest r,uint64_t now,
 bool other_held,const NetworkJobResolution *resolution,uint32_t admission_error,NetworkJobWire wire)
{
 if(!network_job_wire_valid(wire))return false;
 size_t slot=NETWORK_JOB_OWNER_SLOTS;
 for(size_t i=0;i<NETWORK_JOB_OWNER_SLOTS;i++)if(m->cached[i].present&&network_job_identity_equal(m->cached[i].owner,owner)){slot=i;break;}
 if(slot==NETWORK_JOB_OWNER_SLOTS)for(size_t i=0;i<NETWORK_JOB_OWNER_SLOTS;i++)if(!m->cached[i].present){slot=i;break;}
 if(slot==NETWORK_JOB_OWNER_SLOTS)return false;
 if(m->pending[slot].present)return request_equal(m->pending[slot].request,r);
 if(m->cached[slot].present){
  if(request_equal(m->cached[slot].request,r)){m->pending[slot]=m->cached[slot];return true;}
  if(r.seq<=m->cached[slot].request.seq)return respond(m,slot,owner,r,false,NETWORK_JOB_STALE);
 }
 if(m->owner_present&&network_job_identity_equal(m->owner,owner)&&r.seq<=m->seq)return respond(m,slot,owner,r,false,NETWORK_JOB_STALE);
 bool begin=r.kind==NETWORK_JOB_BEGIN||r.kind==NETWORK_JOB_BEGIN_RESOLVED||r.kind==NETWORK_JOB_BEGIN_STREAM_RESOLVED||r.kind==NETWORK_JOB_BEGIN_DUPLEX_RESOLVED;
 uint32_t value=0,error=0;bool success;
 if(other_held&&begin){success=false;error=NETWORK_JOB_BUSY;}
 else success=apply(m,owner,r,now,resolution,admission_error,wire,&value,&error);
 if(m->owner_present&&network_job_identity_equal(m->owner,owner)&&r.seq<=NETWORK_JOB_MAX_SEQ&&r.seq>m->seq)m->seq=r.seq;
 return respond(m,slot,owner,r,success,success?value:error);
}
bool network_job_machine_handle_exclusive(NetworkJobMachine *m,NetworkJobIdentity owner,NetworkJobRequest r,uint64_t now,bool other_held,NetworkJobWire wire)
{NetworkJobResolution none={0};return network_job_machine_handle_resolved(m,owner,r,now,other_held,&none,NETWORK_JOB_DENIED,wire);}
bool network_job_machine_handle(NetworkJobMachine *m,NetworkJobIdentity owner,NetworkJobRequest r,uint64_t now,NetworkJobWire wire)
{return network_job_machine_handle_exclusive(m,owner,r,now,false,wire);}
static void advance_duplex(NetworkJobMachine *m,uint64_t now,NetworkJobWire wire)
{
 size_t received=0;bool drained=false;if(!receive_bounded(m,wire,NETWORK_JOB_TURN/2,&received,&drained)){fail(m,wire);return;}
 if(received)stream_progress(m,now);
 NetworkJobLink link=network_job_wire_link(wire);if(link==NETWORK_JOB_LINK_CLOSED){fail(m,wire);return;}
 if((link==NETWORK_JOB_LINK_OPEN||link==NETWORK_JOB_LINK_PEER_FIN)&&m->sent<m->staged){
  size_t at=m->sent%NETWORK_JOB_MAX_BYTES;
  size_t count=minimum_size(m->staged-m->sent,NETWORK_JOB_MAX_BYTES-at);
  count=minimum_size(count,NETWORK_JOB_TURN-received);
  NetworkJobWireResult result;{NetworkJobReadView data={m->request,sizeof(m->request)};result=network_job_wire_send(wire,data,at,count);}if(!result.ok||result.count>count){fail(m,wire);return;}
  for(size_t i=0;i<result.count;i++){m->request[at+i]=0;}
  m->sent+=result.count;if(result.count)stream_progress(m,now);
 }
 if(m->finish_requested&&m->sent==m->staged&&network_job_wire_queued(wire)==0&&!m->finished_send){network_job_wire_finish_send(wire);m->finished_send=true;}
 link=network_job_wire_link(wire);
 if(m->finished_send&&link==NETWORK_JOB_LINK_PEER_FIN&&drained&&m->consumed==m->response_len&&network_job_wire_queued(wire)==0)m->state=NETWORK_JOB_COMPLETE;
 else if(link!=NETWORK_JOB_LINK_OPENING)m->state=m->sent<m->staged||network_job_wire_queued(wire)!=0?NETWORK_JOB_SENDING:NETWORK_JOB_RECEIVING;
}
void network_job_machine_advance(NetworkJobMachine *m,uint64_t now,NetworkJobWire wire)
{
 if(!network_job_wire_valid(wire)||!live_state(m->state))return;
 if(expired(m,now)){fail(m,wire);return;}m->last_now=now;
 NetworkJobLink initial_link=network_job_wire_link(wire);
 if(initial_link==NETWORK_JOB_LINK_CLOSED){fail(m,wire);return;}
 if(initial_link==NETWORK_JOB_LINK_OPENING){m->state=NETWORK_JOB_CONNECTING;return;}
 if(m->duplex){advance_duplex(m,now,wire);return;}
 size_t count=0;bool drained=false;
 if(m->streaming){if(!receive_bounded(m,wire,NETWORK_JOB_TURN,&count,&drained)){fail(m,wire);return;}}
 else{
  size_t room=minimum_size(NETWORK_JOB_MAX_BYTES-m->response_len,NETWORK_JOB_TURN);
  NetworkJobWireResult result;
  if(room){{NetworkJobWriteView out={m->response,sizeof(m->response)};result=network_job_wire_receive(wire,out,m->response_len,room);}if(!result.ok||result.count>room){fail(m,wire);return;}}
  else{uint8_t extra=0;NetworkJobWriteView out={&extra,1};result=network_job_wire_receive(wire,out,0,1);if(!result.ok||result.count!=0){fail(m,wire);return;}}
  count=result.count;
  m->response_len+=count;drained=count==0;
 }
 if(m->streaming&&count)stream_progress(m,now);
 if(network_job_wire_link(wire)==NETWORK_JOB_LINK_CLOSED){fail(m,wire);return;}
 if(network_job_wire_link(wire)==NETWORK_JOB_LINK_PEER_FIN){
  if(m->sent!=m->request_len||network_job_wire_queued(wire)!=0){fail(m,wire);return;}
  if(!m->finished_send){network_job_wire_finish_send(wire);m->finished_send=true;}
  m->state=drained?NETWORK_JOB_COMPLETE:NETWORK_JOB_RECEIVING;return;
 }
 if(count==0&&network_job_wire_link(wire)==NETWORK_JOB_LINK_OPEN&&m->sent<m->request_len){
  size_t end=minimum_size(m->sent+NETWORK_JOB_TURN,m->request_len);
  NetworkJobWireResult result;{NetworkJobReadView data={m->request,sizeof(m->request)};result=network_job_wire_send(wire,data,m->sent,end-m->sent);}if(!result.ok||result.count>end-m->sent){fail(m,wire);return;}
  m->sent+=result.count;if(m->streaming&&result.count)stream_progress(m,now);
 }
 if(m->sent==m->request_len&&network_job_wire_queued(wire)==0&&!m->finished_send){network_job_wire_finish_send(wire);m->finished_send=true;m->state=NETWORK_JOB_RECEIVING;}
}


