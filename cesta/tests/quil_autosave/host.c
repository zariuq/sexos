#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "quil_autosave.h"
#define CHECK(x) do{if(!(x))return __LINE__;}while(0)
static QuilAutosaveTitle title(const char *text){QuilAutosaveTitle t={0};size_t n=strlen(text);if(n>16)n=16;memcpy(t.bytes,text,n);return t;}
static void name_is(const QuilAutosave *a,const char *input,const char *want,size_t n){QuilAutosaveTitle t=title(input);QuilAutosaveName got=quil_autosave_name(a,&t);if(memcmp(got.bytes,want,n))__builtin_trap();}
int main(void){
 QuilAutosave a;quil_autosave_init(&a);CHECK(a.queued_save==-1&&!quil_autosave_pending(&a));
 CHECK(!quil_autosave_due(&a,0,false,1,true)&&!quil_autosave_armed(&a));
 CHECK(!quil_autosave_due(&a,0,true,1,true)&&quil_autosave_armed(&a));
 CHECK(!quil_autosave_due(&a,QUIL_AUTOSAVE_IDLE_MS-1,true,1,true));
 CHECK(!quil_autosave_due(&a,QUIL_AUTOSAVE_IDLE_MS+5,true,2,true));
 CHECK(!quil_autosave_due(&a,2*QUIL_AUTOSAVE_IDLE_MS+10,true,2,false));
 CHECK(quil_autosave_due(&a,2*QUIL_AUTOSAVE_IDLE_MS+15,true,2,true));
 CHECK(quil_autosave_pending(&a)&&!quil_autosave_armed(&a));
 CHECK(!quil_autosave_due(&a,9*QUIL_AUTOSAVE_IDLE_MS,true,2,true));
 QuilAutosaveDone done=quil_autosave_finish(&a,true,7,2,0);CHECK(done.kind==QUIL_AUTOSAVE_SAVED&&done.draft==7&&quil_autosave_kept(&a,2)&&!quil_autosave_kept(&a,3));
 uint8_t draft=0;CHECK(quil_autosave_target(&a,&draft)&&draft==7);
 CHECK(!quil_autosave_due(&a,20*QUIL_AUTOSAVE_IDLE_MS,true,2,true));
 CHECK(!quil_autosave_due(&a,30*QUIL_AUTOSAVE_IDLE_MS,true,2,true));
 quil_autosave_init(&a);name_is(&a,"notes","notes.draft\0\0\0\0\0",16);name_is(&a,"","untitled.draft\0\0",16);name_is(&a,"abcdefghijklmnop","abcdefghij.draft",16);name_is(&a,"a b~c","a_b_c.draft\0\0\0\0\0",16);
 a.pending=true;done=quil_autosave_finish(&a,false,0,1,QUIL_AUTOSAVE_EXISTS);CHECK(done.kind==QUIL_AUTOSAVE_NAME_TAKEN&&done.next==1);name_is(&a,"notes","notes01.draft\0\0\0",16);
 for(unsigned i=1;i<99;i++){quil_autosave_finish(&a,false,0,1,QUIL_AUTOSAVE_EXISTS);}
 done=quil_autosave_finish(&a,false,0,1,QUIL_AUTOSAVE_EXISTS);CHECK(done.kind==QUIL_AUTOSAVE_GAVE_UP&&!quil_autosave_due(&a,0,true,5,true));
 quil_autosave_init(&a);CHECK(!quil_autosave_due(&a,0,true,1,true));CHECK(quil_autosave_due(&a,QUIL_AUTOSAVE_IDLE_MS,true,1,true));a.queued_save=0;quil_autosave_reset(&a);CHECK(a.pending&&a.queued_save==0&&!a.draft_present);done=quil_autosave_finish(&a,true,3,1,0);CHECK(done.kind==QUIL_AUTOSAVE_SAVED&&done.draft==3);
 a.pending=true;done=quil_autosave_finish(&a,false,0,2,9);CHECK(done.kind==QUIL_AUTOSAVE_FAILED&&!a.draft_present);
 quil_autosave_init(&a);CHECK(!quil_autosave_due(&a,UINT64_MAX-5,true,4,true));CHECK(!quil_autosave_due(&a,3,true,4,true));
 puts("quil autosave controls passed");return 0;
}
