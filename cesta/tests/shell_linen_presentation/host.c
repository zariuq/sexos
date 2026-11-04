#include "shell_linen_runtime.h"
#include <assert.h>
#include <stdio.h>

static uint8_t admission=SHELL_TEXT_ACCEPTED;
ShellTextAdmission shell_text_display_admit(ShellTextOperation operation)
{(void)operation;return(ShellTextAdmission){admission,admission?77:0};}
void shell_text_native_erase(ShellTextBatch *batch)
{
 volatile uint8_t *bytes=batch->text.bytes;
 for(size_t i=0;i<SHELL_TEXT_CAPACITY;++i)bytes[i]=0;
 *batch=(ShellTextBatch){0};
}
static uint64_t ack(uint32_t sequence)
{return UINT64_C(0x56)|(UINT64_C(200)<<8)|((uint64_t)sequence<<24);}
static ShellTextBytes text(uint8_t byte)
{ShellTextBytes value={.len=1};value.bytes[0]=byte;return value;}

int main(void)
{
 ShellLinenRuntime r={0};r.publication_revision=7;r.text=shell_text_new(200,true);
 ShellTextBytes a=text('a'),b=text('b'),c=text('c'),d=text('d');
 ShellTextMetadata metadata={1,2,3,4};
 assert(shell_linen_text_queue(&r,&a,1,metadata));
 assert(r.presentation_active.job==(UINT64_C(1)<<63|1)&&r.presentation_active.revision==7&&r.presentation_active.sequence==1);
 assert(shell_linen_text_queue(&r,&b,2,metadata));
 assert(r.presentation_desired.job==(UINT64_C(1)<<63|2)&&r.presentation_desired.sequence==2);
 assert(shell_linen_text_queue(&r,&c,3,metadata));
 assert(r.presentation_last.present&&r.presentation_last.outcome==SHELL_PRESENTATION_SUPERSEDED);
 assert(r.presentation_last.job==(UINT64_C(1)<<63|2)&&r.presentation_desired.job==(UINT64_C(1)<<63|3));
 while(!r.text.current.awaiting){
  ShellTextPoll p=shell_linen_text_poll(&r,1);assert(p.kind==SHELL_TEXT_PENDING);
 }
 assert(r.presentation_active.submitted);
 ShellTextReceipt receipt={0};
 assert(!shell_linen_text_receive(&r,1,2,ack(1),&receipt));
 assert(!shell_linen_text_receive(&r,1,1,ack(2),&receipt));
 assert(shell_linen_text_receive(&r,1,1,ack(1),&receipt));
 assert(receipt.bytes==1&&receipt.metadata.selected==4);
 assert(!r.presentation_active.present&&r.presentation_desired.present);
 (void)shell_linen_text_poll(&r,1);
 assert(r.presentation_active.job==(UINT64_C(1)<<63|3)&&!r.presentation_desired.present);
 assert(r.presentation_active.submitted);
 shell_linen_text_cancel(&r);
 assert(r.presentation_last.outcome==SHELL_PRESENTATION_INDETERMINATE&&r.presentation_last.job==(UINT64_C(1)<<63|3));
 assert(!r.presentation_active.present&&!r.presentation_desired.present&&!r.text.active&&!r.text.latest);

 assert(shell_linen_text_queue(&r,&d,4,metadata));
 admission=SHELL_TEXT_FULL;
 ShellTextPoll deferred=shell_linen_text_poll(&r,1);assert(deferred.kind==SHELL_TEXT_DEFERRED&&!r.presentation_active.submitted);
 shell_linen_text_cancel(&r);
 assert(r.presentation_last.outcome==SHELL_PRESENTATION_FAILED&&r.presentation_last.job==(UINT64_C(1)<<63|4));

 admission=SHELL_TEXT_ACCEPTED;
 assert(shell_linen_text_queue(&r,&d,4,metadata));
 ShellTextPoll submitted=shell_linen_text_poll(&r,1);
 assert(submitted.kind==SHELL_TEXT_PENDING&&r.presentation_active.submitted);
 admission=SHELL_TEXT_REJECTED;
 ShellTextPoll uncertain=shell_linen_text_poll(&r,1);
 assert(uncertain.kind==SHELL_TEXT_FAILED);
 assert(r.presentation_last.outcome==SHELL_PRESENTATION_INDETERMINATE&&r.presentation_last.job==(UINT64_C(1)<<63|5));

 uint32_t sequence=r.text.sequence;r.next_presentation_job=UINT64_C(0x7ffffffffffffffe);
 assert(!shell_linen_text_queue(&r,&a,1,metadata)&&r.text.sequence==sequence);
 r.next_presentation_job=0;r.publication_revision=0;
 assert(!shell_linen_text_queue(&r,&a,1,metadata)&&r.text.sequence==sequence);
 puts("PASS: active/desired presentation jobs, supersession, authenticated acknowledgment, uncertainty and exhaustion");
}
