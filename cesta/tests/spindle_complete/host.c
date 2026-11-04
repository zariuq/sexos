#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "spindle_complete.h"

size_t sexos_strlen(const char *text)
{
    size_t length=0;
    while(text[length])length++;
    return length;
}

static void hex(const uint8_t *bytes,size_t length)
{
    for(size_t i=0;i<length;i++)printf("%02x",bytes[i]);
}

int main(void)
{
    uint8_t word[128];
    while(fgets((char *)word,sizeof word,stdin)){
        size_t length=strcspn((char *)word,"\r\n");
        SpindleCompleteWord input={0};input.len=length;
        if(length>SPINDLE_COMPLETE_WORD_MAX)input.too_long=true;
        else memcpy(input.bytes,word,length);
        SpindleCompletion result=spindle_complete(&input);
        printf("%zu ",result.matches);hex(result.extra,result.extra_len);putchar(' ');
        hex(result.candidates,result.candidates_len);putchar('\n');
    }
    return 0;
}
