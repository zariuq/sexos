#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "network_tcp.c"

bool network_device_send(NetworkDevice *d,cesta_read_bytes frame) { (void)d;(void)frame;abort(); }
NetworkStep network_device_receive(NetworkDevice *d,NetworkFrame *frame) { (void)d;(void)frame;return NET_FAILED; }
void sexos_yield(void) {}
void sexos_line(const char *format,...) { (void)format; }

int main(void)
{
    char text[9000];
    while(fgets(text,sizeof text,stdin)) {
        uint8_t bytes[4096];size_t count=0;
        for(size_t i=0;text[i] && text[i]!='\n';i+=2) {
            unsigned value;assert(count<sizeof bytes && sscanf(text+i,"%2x",&value)==1);
            bytes[count++]=(uint8_t)value;
        }
        NetworkFrame frame={.length=count};assert(count<=sizeof frame.data);memcpy(frame.data,bytes,count);size_t header=0,length=0;
        const uint8_t *tcp=tcp_receive(&frame,&header,&length);
        if(!tcp) puts("refused");
        else {
            printf("accepted %zu %zu ",header,length-header);
            for(size_t i=header;i<length;++i) printf("%02x",tcp[i]);
            putchar('\n');
        }
    }
    return 0;
}
