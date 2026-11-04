#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell_text_delivery.h"
static ShellTextOutbox out;
static ShellTextBytes text;
static uint8_t mode;
static uint64_t remaining,error;
ShellTextAdmission shell_text_display_admit(ShellTextOperation op)
{
    printf("O %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",op.opcode,op.surface,op.packed,op.control);
    if(remaining){--remaining;return(ShellTextAdmission){SHELL_TEXT_ACCEPTED,0};}
    return(ShellTextAdmission){mode,error};
}
static void receipt(ShellTextReceipt r)
{
    printf(" %zu %zu %zu %zu %" PRIu64,r.bytes,r.metadata.count,r.metadata.first,r.metadata.rows,r.metadata.selected);
}
static void batch(const ShellTextBatch *b)
{
    printf(" %zu %u %zu %u %u %u %u %u %zu %zu %zu %" PRIu64 " %" PRIu64,
        b->text.len,b->color,b->offset,b->sequence,b->cleared,b->awaiting,b->selection_start,b->selection_end,
        b->metadata.count,b->metadata.first,b->metadata.rows,b->metadata.selected,shell_text_hash(&b->text));
    printf(" %u",b->selection_sent);
}
static void state(void)
{
    printf("S %" PRIu64 " %u %u %u %u",out.surface,out.atomic,out.sequence,out.active,out.latest);
    if(out.active)batch(&out.current);
    putchar('|');
    if(out.latest)batch(&out.desired);
    putchar('\n');
}
int main(void)
{
    char line[2048];
    while(fgets(line,sizeof(line),stdin)){
        uint64_t n[16]={0};char *at=line,*end;size_t count=0;
        while(count<16){uint64_t value=strtoull(at,&end,10);if(end==at)break;n[count++]=value;at=end;}
        if(!count)continue;
        switch(n[0]){
        case 0:out=shell_text_new(n[1],n[2]);text=(ShellTextBytes){0};mode=0;remaining=0;error=0;break;
        case 1:text.len=n[1];for(size_t i=0;i<512;++i)text.bytes[i]=(uint8_t)(n[3]?'a'+(i+n[2])%26:i+n[2]);break;
        case 2:printf("Q %u\n",shell_text_queue_selected(&out,&text,n[1],(ShellTextMetadata){n[2],n[3],n[4],n[5]},n[6],n[7]));break;
        case 3:{ShellTextPoll r=shell_text_poll(&out,n[1]);printf("P %u %" PRIu64,r.kind,r.status);receipt(r.receipt);putchar('\n');break;}
        case 4:mode=n[1];remaining=n[2];error=n[3];break;
        case 5:{ShellTextReceipt r={0};bool accepted=shell_text_receive(&out,n[1],n[2],&r);printf("R %u",accepted);receipt(r);putchar('\n');break;}
        case 6:if(n[1]>=512)abort();text.bytes[n[1]]=n[2];break;
        case 7:shell_text_cancel(&out);break;
        case 8:out.sequence=n[1];break;
        case 9:printf("V %u %u %" PRIu64 " %" PRIu64 "\n",shell_text_valid(&text),shell_text_boundary(&text,n[1]),shell_text_hash(&text),shell_text_selected_hash(&text,n[2],n[3]));break;
        case 10:{ShellTextBatch b={.text={.len=17},.color=77,.metadata={1,2,3,4},.selection_start=2,.selection_end=3,.selection_sent=true};for(size_t i=0;i<512;++i)b.text.bytes[i]=255;shell_text_native_erase(&b);bool good=!b.selection_start&&!b.selection_end&&!b.selection_sent&&b.text.len==17&&b.color==77&&b.metadata.selected==4;for(size_t i=0;i<512;++i)good=good&&!b.text.bytes[i];printf("E %u\n",good);break;}
        case 11:out.current.text.len=n[1];out.current.offset=n[2];break;
        default:abort();}
        state();
    }
}
