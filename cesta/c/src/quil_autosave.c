#include "quil_autosave.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

void quil_autosave_init(QuilAutosave *state)
{if(state){*state=(QuilAutosave){.queued_save=-1};}}
void quil_autosave_reset(QuilAutosave *state)
{if(!state)return;int8_t queued=state->queued_save;bool pending=state->pending;quil_autosave_init(state);state->queued_save=queued;state->pending=pending;}
bool quil_autosave_pending(const QuilAutosave *state){return state&&state->pending;}
bool quil_autosave_armed(const QuilAutosave *state){return state&&state->seen_present&&!state->pending;}
bool quil_autosave_due(QuilAutosave *state,uint64_t now,bool dirty,uint64_t revision,bool ready)
{
 if(!state)return false;
 if(!dirty||state->pending||state->suffix>99u||(state->drafted_present&&state->drafted_revision==revision)){state->seen_present=false;return false;}
 if(!state->seen_present||state->seen_revision!=revision){state->seen_present=true;state->seen_revision=revision;state->seen_at=now;return false;}
 uint64_t elapsed=now>=state->seen_at?now-state->seen_at:0;
 if(!ready||elapsed<QUIL_AUTOSAVE_IDLE_MS)return false;
 state->pending=true;return true;
}
QuilAutosaveName quil_autosave_name(const QuilAutosave *state,const QuilAutosaveTitle *title)
{
 QuilAutosaveName out={0};if(!state||!title)return out;size_t length=0;while(length<16&&title->bytes[length])length++;
 static const uint8_t untitled[8]={'u','n','t','i','t','l','e','d'};size_t room=state->suffix?8:10,n=0;
 if(!length){for(size_t i=0;i<8;i++)out.bytes[n++]=untitled[i];}
 else for(size_t i=0;i<length&&i<room;i++){uint8_t byte=title->bytes[i];bool valid=(byte>='A'&&byte<='Z')||(byte>='a'&&byte<='z')||(byte>='0'&&byte<='9')||byte=='-'||byte=='_'||byte=='.';out.bytes[n++]=valid?byte:'_';}
 if(state->suffix){out.bytes[n++]=(uint8_t)('0'+state->suffix/10u);out.bytes[n++]=(uint8_t)('0'+state->suffix%10u);}
 static const uint8_t extension[6]={'.','d','r','a','f','t'};for(size_t i=0;i<6;i++)out.bytes[n++]=extension[i];return out;
}
bool quil_autosave_target(const QuilAutosave *state,uint8_t *draft)
{if(!state||!draft||!state->draft_present)return false;*draft=state->draft;return true;}
bool quil_autosave_kept(const QuilAutosave *state,uint64_t revision)
{return state&&state->draft_present&&state->drafted_present&&state->drafted_revision==revision;}
QuilAutosaveDone quil_autosave_finish(QuilAutosave *state,bool saved,uint8_t object,uint64_t revision,uint32_t error)
{
 QuilAutosaveDone done={.kind=QUIL_AUTOSAVE_FAILED,.error=error};if(!state)return done;state->pending=false;state->seen_present=false;
 if(saved){state->draft_present=true;state->draft=object;state->drafted_present=true;state->drafted_revision=revision;done.kind=QUIL_AUTOSAVE_SAVED;done.draft=object;done.revision=revision;return done;}
 if(error==QUIL_AUTOSAVE_EXISTS&&!state->draft_present){state->suffix++;if(state->suffix>99u){done.kind=QUIL_AUTOSAVE_GAVE_UP;}else{done.kind=QUIL_AUTOSAVE_NAME_TAKEN;done.next=state->suffix;}return done;}
 if(state->draft_present){state->draft_present=false;}
 return done;
}


