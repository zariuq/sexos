#include "sexos_document_service.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static const uint32_t DOCUMENT_TARGETS[SEXOS_DOCUMENT_SESSION_COUNT]={9,16,18,19,20,21,22,23,32,33,34,35,36,37,38,39,15,30,31};
static size_t document_target_index(uint32_t caller)
{for(size_t i=0;i<SEXOS_DOCUMENT_SESSION_COUNT;i++)if(DOCUMENT_TARGETS[i]==caller)return i;return SIZE_MAX;}
static bool document_is_browser(uint32_t caller){return caller==15||caller==30||caller==31;}
static bool document_request_equal(SexosDocumentRequest a,SexosDocumentRequest b)
{return a.seq==b.seq&&a.kind==b.kind&&a.object==b.object&&a.offset==b.offset&&a.a==b.a&&a.b==b.b;}
static void document_session_clear(DocumentSession *session)
{
 memset(session->payload.bytes,0,sizeof session->payload.bytes);session->payload.len=0;memset(session->name.bytes,0,sizeof session->name.bytes);
 session->written=0;session->generation=0;session->object=0;session->bound=false;session->saving=false;
 session->created_present=false;session->created_object=0;session->created_generation=0;session->load_pending=false;session->commit_pending=false;session->pending_request=(SexosDocumentRequest){0};
}
void document_service_init(DocumentService *state){memset(state,0,sizeof *state);}
bool document_service_retire(DocumentService *state,uint32_t caller)
{
 size_t index=document_target_index(caller);if(index==SIZE_MAX)return false;DocumentSession *session=&state->sessions[index];
 document_backend_cancel_work(caller);uint32_t error=0;if(!document_backend_clear_load(caller,&error))return false;document_session_clear(session);session->last_present=false;session->receipt=0;session->dead=true;return true;
}
bool document_service_browser_scrubbed(const DocumentService *state)
{
 for(size_t i=0;i<SEXOS_DOCUMENT_SESSION_COUNT;i++)if(document_is_browser(DOCUMENT_TARGETS[i])){
  const DocumentSession *s=&state->sessions[i];if(s->created_present||s->bound||s->saving||s->payload.len||s->written)return false;
  for(size_t j=0;j<SEXOS_DOCUMENT_MAX;j++)if(s->payload.bytes[j])return false;
  for(size_t j=0;j<16;j++)if(s->name.bytes[j])return false;
 }
 return true;
}
static uint64_t document_error(uint32_t sequence,uint32_t error){return sexos_document_reply(sequence,false,error);}
static bool document_identity(uint8_t object,uint32_t *generation,uint32_t *error)
{if(!document_backend_identity(object,generation,error)){if(!*error)*error=SEXOS_DOCUMENT_IO;return false;}return true;}
static bool document_valid_name(const DocumentName *name)
{
 size_t length=0;while(length<16&&name->bytes[length])length++;if(!length)return false;
 for(size_t i=0;i<length;i++){uint8_t c=name->bytes[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return false;}
 for(size_t i=length;i<16;i++)if(name->bytes[i])return false;
 return true;
}
static DocumentServiceResult document_execute(DocumentSession *session,uint32_t caller,SexosDocumentRequest request)
{
 uint32_t error=0,generation=0;
 if(request.kind==SEXOS_DOCUMENT_ABORT){
  if(request.object||request.offset||request.a||request.b)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  document_backend_cancel_work(caller);if(!document_backend_clear_load(caller,&error))return(DocumentServiceResult){true,document_error(request.seq,error?error:SEXOS_DOCUMENT_IO)};document_session_clear(session);return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,0)};
 }
 if(request.kind==SEXOS_DOCUMENT_CREATE){
  if(session->bound||(document_is_browser(caller)&&session->created_present)||request.object||request.offset)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_BUSY)};
  DocumentName name={0};uint8_t object=0;uint32_t created_generation=0;for(size_t i=0;i<8;i++){name.bytes[i]=(uint8_t)(request.a>>(i*8));name.bytes[i+8]=(uint8_t)(request.b>>(i*8));}
  if(!document_valid_name(&name))return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  if(!document_backend_create(&name,document_is_browser(caller),&object,&created_generation,&error))return(DocumentServiceResult){true,document_error(request.seq,error?error:SEXOS_DOCUMENT_IO)};
  if(object<3||object>=15||!created_generation)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_IO)};
  if(document_is_browser(caller)){session->created_present=true;session->created_object=object;session->created_generation=created_generation;}
  return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,object)};
 }
 if(document_is_browser(caller)){
  if(request.kind!=SEXOS_DOCUMENT_INFO&&request.kind!=SEXOS_DOCUMENT_BEGIN_SAVE&&request.kind!=SEXOS_DOCUMENT_WRITE&&request.kind!=SEXOS_DOCUMENT_COMMIT&&request.kind!=SEXOS_DOCUMENT_END)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_DENIED)};
  if(!session->created_present||request.object!=session->created_object)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_DENIED)};
  if(!document_identity(request.object,&generation,&error))return(DocumentServiceResult){true,document_error(request.seq,error)};
  if(generation!=session->created_generation)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_STALE)};
  if(request.kind==SEXOS_DOCUMENT_END){if(request.offset||request.a||request.b)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};document_session_clear(session);return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,0)};}
 }
 if(request.object<2||request.object>=15)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_DENIED)};
 if(request.kind==SEXOS_DOCUMENT_INFO){
  if(session->bound||request.offset||request.a||request.b)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_BUSY)};
  if(!document_identity(request.object,&generation,&error))return(DocumentServiceResult){true,document_error(request.seq,error)};
  return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,generation)};
 }
 if(request.kind==SEXOS_DOCUMENT_DELETE){
  if(session->bound||request.object<3||request.offset||request.b)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_BUSY)};
  if(!document_identity(request.object,&generation,&error))return(DocumentServiceResult){true,document_error(request.seq,error)};
  if(request.a!=generation)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_STALE)};
  if(!document_backend_delete(request.object,&error))return(DocumentServiceResult){true,document_error(request.seq,error?error:SEXOS_DOCUMENT_IO)};
  return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,0)};
 }
 if(request.kind==SEXOS_DOCUMENT_LOAD){
  if(session->bound||request.offset||request.b>1)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_BUSY)};
  if(!document_identity(request.object,&generation,&error))return(DocumentServiceResult){true,document_error(request.seq,error)};
  if(request.a!=generation)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_STALE)};
  document_session_clear(session);session->object=request.object;session->generation=generation;session->load_pending=true;session->pending_request=request;
  return(DocumentServiceResult){false,0};
 }
 if(request.kind==SEXOS_DOCUMENT_BEGIN_SAVE){
  if(session->bound||request.offset||request.b>SEXOS_DOCUMENT_MAX)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  if(!document_identity(request.object,&generation,&error))return(DocumentServiceResult){true,document_error(request.seq,error)};
  if(request.a!=generation)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_STALE)};
  bool created=session->created_present;uint8_t created_object=session->created_object;uint32_t created_generation=session->created_generation;
  document_session_clear(session);session->created_present=created;session->created_object=created_object;session->created_generation=created_generation;
  session->payload.len=(size_t)request.b;if(!document_backend_name(request.object,&session->name,&error)){document_session_clear(session);return(DocumentServiceResult){true,document_error(request.seq,error?error:SEXOS_DOCUMENT_IO)};}
  session->object=request.object;session->generation=generation;session->bound=true;session->saving=true;return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,0)};
 }
 if(!session->bound||session->object!=request.object)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_STALE)};
 if(!document_identity(request.object,&generation,&error))return(DocumentServiceResult){true,document_error(request.seq,error)};
 if(generation!=session->generation)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_STALE)};
 if(request.kind==SEXOS_DOCUMENT_WRITE){
  size_t offset=request.offset;if(!session->saving||offset!=session->written||offset>=session->payload.len)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  size_t count=session->payload.len-offset;if(count>16)count=16;uint8_t raw[16];for(size_t i=0;i<8;i++){raw[i]=(uint8_t)(request.a>>(i*8));raw[i+8]=(uint8_t)(request.b>>(i*8));}
  for(size_t i=count;i<16;i++)if(raw[i])return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  for(size_t i=0;i<count;i++)session->payload.bytes[offset+i]=raw[i];
  session->written+=count;return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,(uint32_t)count)};
 }
 if(request.kind==SEXOS_DOCUMENT_COMMIT){
  if(!session->saving||session->written!=session->payload.len||request.offset||request.b||sexos_document_hash(&session->payload)!=request.a)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  session->commit_pending=true;session->pending_request=request;return(DocumentServiceResult){false,0};
 }
 if(request.kind==SEXOS_DOCUMENT_READ){
  size_t offset=request.offset,count=(size_t)request.a;if(session->saving||!count||count>4||offset>session->payload.len||count>session->payload.len-offset||request.b)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  uint32_t value=0;for(size_t i=0;i<count;i++)value|=(uint32_t)session->payload.bytes[offset+i]<<(i*8);return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,value)};
 }
 if(request.kind==SEXOS_DOCUMENT_NAME){
  size_t offset=request.offset;if(offset%4||offset>=16||request.a||request.b)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  uint32_t value=0;for(size_t i=0;i<4;i++)value|=(uint32_t)session->name.bytes[offset+i]<<(i*8);return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,value)};
 }
 if(request.kind==SEXOS_DOCUMENT_END){
  if(request.offset||request.a||request.b)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
  document_backend_cancel_work(caller);if(!document_backend_clear_load(caller,&error))return(DocumentServiceResult){true,document_error(request.seq,error?error:SEXOS_DOCUMENT_IO)};document_session_clear(session);return(DocumentServiceResult){true,sexos_document_reply(request.seq,true,0)};
 }
 return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_INVALID)};
}
DocumentServiceResult document_service_handle(DocumentService *state,uint32_t caller,bool live,SexosDocumentRequest request)
{
 size_t index=document_target_index(caller);if(index==SIZE_MAX)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_DENIED)};
 DocumentSession *session=&state->sessions[index];if(session->dead||!live)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_DENIED)};
 if(session->load_pending||session->commit_pending){
  if(document_request_equal(session->pending_request,request))return(DocumentServiceResult){false,0};
  bool fresh_abort=request.kind==SEXOS_DOCUMENT_ABORT&&!request.object&&!request.offset&&!request.a&&!request.b&&(!session->last_present||request.seq>session->last.seq);
  if(!fresh_abort)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_BUSY)};
 }
 if(session->last_present){if(document_request_equal(session->last,request))return(DocumentServiceResult){true,session->receipt};if(request.seq<=session->last.seq)return(DocumentServiceResult){true,document_error(request.seq,SEXOS_DOCUMENT_STALE)};}
 DocumentServiceResult result=document_execute(session,caller,request);if(result.answered){session->last=request;session->last_present=true;session->receipt=result.value;}return result;
}
static DocumentServiceCompletion document_progress_session(DocumentSession *session,uint32_t caller)
{
 uint32_t error=0,generation=0;SexosDocumentRequest request=session->pending_request;
 if(!document_identity(session->object,&generation,&error)||generation!=session->generation){document_backend_cancel_work(caller);bool loading=session->load_pending;session->load_pending=false;session->commit_pending=false;if(loading){uint32_t ignored=0;(void)document_backend_clear_load(caller,&ignored);document_session_clear(session);}uint64_t value=document_error(request.seq,error?error:SEXOS_DOCUMENT_STALE);session->last=request;session->last_present=true;session->receipt=value;return(DocumentServiceCompletion){true,caller,request,value};}
 if(session->load_pending){
  uint8_t status=document_backend_load_step(caller,session->object,&session->payload,&session->name,&error);if(status==DOCUMENT_BACKEND_PENDING)return(DocumentServiceCompletion){0};
  session->load_pending=false;if(status!=DOCUMENT_BACKEND_DONE||session->payload.len>SEXOS_DOCUMENT_MAX){document_backend_cancel_work(caller);uint32_t ignored=0;(void)document_backend_clear_load(caller,&ignored);document_session_clear(session);uint64_t value=document_error(request.seq,error?error:SEXOS_DOCUMENT_IO);session->last=request;session->last_present=true;session->receipt=value;return(DocumentServiceCompletion){true,caller,request,value};}
  uint32_t after=0;if(!document_identity(session->object,&after,&error)||after!=session->generation){document_backend_cancel_work(caller);uint32_t ignored=0;(void)document_backend_clear_load(caller,&ignored);document_session_clear(session);uint64_t value=document_error(request.seq,SEXOS_DOCUMENT_STALE);session->last=request;session->last_present=true;session->receipt=value;return(DocumentServiceCompletion){true,caller,request,value};}
  session->bound=true;if(request.b==1&&!document_backend_publish_load(caller,request,session->generation,&session->payload,&session->name,&error)){document_backend_cancel_work(caller);uint32_t ignored=0;(void)document_backend_clear_load(caller,&ignored);document_session_clear(session);uint64_t value=document_error(request.seq,error?error:SEXOS_DOCUMENT_DENIED);session->last=request;session->last_present=true;session->receipt=value;return(DocumentServiceCompletion){true,caller,request,value};}
  document_backend_cancel_work(caller);uint64_t value=sexos_document_reply(request.seq,true,(uint32_t)session->payload.len);session->last=request;session->last_present=true;session->receipt=value;return(DocumentServiceCompletion){true,caller,request,value};
 }
 uint8_t status=document_backend_save_step(caller,session->object,&session->payload,&error);if(status==DOCUMENT_BACKEND_PENDING)return(DocumentServiceCompletion){0};
 session->commit_pending=false;document_backend_cancel_work(caller);uint64_t value;if(status==DOCUMENT_BACKEND_DONE){session->saving=false;value=sexos_document_reply(request.seq,true,(uint32_t)session->payload.len);}else value=document_error(request.seq,error?error:SEXOS_DOCUMENT_IO);
 session->last=request;session->last_present=true;session->receipt=value;return(DocumentServiceCompletion){true,caller,request,value};
}
DocumentServiceCompletion document_service_progress(DocumentService *state)
{
 for(size_t index=0;index<SEXOS_DOCUMENT_SESSION_COUNT;index++)if(state->sessions[index].load_pending||state->sessions[index].commit_pending)return document_progress_session(&state->sessions[index],DOCUMENT_TARGETS[index]);
 return(DocumentServiceCompletion){0};
}


