#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network_device.c"
#include "network_send.c"

static const uint64_t bar=UINT64_C(0x72000000);
static uint32_t registers[16384];
static _Alignas(16) uint8_t pages[16][4096];
static unsigned allocated, poll_count, completion_delay=3, tx_count, yields;
static uint8_t completion_error;
static NetworkFrame *rx_output;
static bool rx_expect_valid;
static unsigned rx_returns;
static size_t rx_size;
static uint8_t rx_expected[2048];
static bool pending;
static uint8_t *pending_descriptor, *pending_buffer;
static size_t pending_length;
static uint8_t submitted[2048];

static uint8_t *physical(uint64_t address)
{
    assert(address>=4096 && address/4096<=allocated);
    return pages[address/4096-1]+address%4096;
}
static bool mmio(const void *p) { return (uintptr_t)p>=bar && (uintptr_t)p<bar+65536; }
static void cpu_write(void *p,size_t width,uint64_t value)
{
    assert(!mmio(p));
    if(pending) assert((uintptr_t)p+width<=(uintptr_t)pending_buffer ||
                       (uintptr_t)p>=(uintptr_t)pending_buffer+4096);
    memcpy(p,&value,width);
}

uint8_t sexos_volatile_read8(const uint8_t *p)
{
    if(pending && p==pending_descriptor+12) {
        assert(memcmp(submitted,pending_buffer,pending_length)==0);
        if(++poll_count==completion_delay) {
            pending_descriptor[12]=(uint8_t)(1|completion_error);
            registers[0x3810/4]=registers[0x3818/4];
            pending=false;
        }
    }
    uint8_t value;memcpy(&value,p,1);return value;
}
uint16_t sexos_volatile_read16(const uint16_t *p) { uint16_t v;memcpy(&v,p,2);return v; }
uint32_t sexos_volatile_read32(const uint32_t *p) {
    if(mmio(p)) return registers[((uintptr_t)p-bar)/4];
    uint32_t v;memcpy(&v,p,4);return v;
}
uint64_t sexos_volatile_read64(const uint64_t *p) { uint64_t v;memcpy(&v,p,8);return v; }
void sexos_volatile_write8(uint8_t *p,uint8_t v) { cpu_write(p,1,v); }
void sexos_volatile_write16(uint16_t *p,uint16_t v) { cpu_write(p,2,v); }
void sexos_volatile_write64(uint64_t *p,uint64_t v) { cpu_write(p,8,v); }
void sexos_volatile_write32(uint32_t *p,uint32_t v)
{
    if(!mmio(p)) { cpu_write(p,4,v);return; }
    unsigned reg=(unsigned)(((uintptr_t)p-bar)/4);
    if(reg==0 && (v&(1u<<26))) { registers[reg]=v&~(1u<<26);return; }
    if(reg==0x2818/4 && (registers[0x0100/4]&2)) {
        assert(rx_output && v==rx_returns%8);
        uint64_t ring=(uint64_t)registers[0x2800/4]|(uint64_t)registers[0x2804/4]<<32;
        uint8_t *descriptor=physical(ring)+v*16;
        assert(descriptor[12]==0);
        if(rx_expect_valid) assert(rx_output->length==rx_size && !memcmp(rx_output->data,rx_expected,rx_size));
        else assert(rx_output->length==0);
        uint64_t buffer;memcpy(&buffer,descriptor,8);
        /* Returning a descriptor permits immediate DMA reuse. */
        memset(physical(buffer),0xCD,2048);
        ++rx_returns;
    }
    if(reg==0x3818/4 && (registers[0x0400/4]&2)) {
        assert(!pending && v==(registers[reg]+1)%8);
        uint64_t ring=(uint64_t)registers[0x3800/4]|(uint64_t)registers[0x3804/4]<<32;
        pending_descriptor=physical(ring)+registers[reg]*16;
        uint64_t buffer;uint16_t length;
        memcpy(&buffer,pending_descriptor,8);memcpy(&length,pending_descriptor+8,2);
        assert(length>=60 && length<=1514 && pending_descriptor[11]==0x0B && pending_descriptor[12]==0);
        pending_buffer=physical(buffer);pending_length=length;
        memcpy(submitted,pending_buffer,length);poll_count=0;pending=true;++tx_count;
    }
    registers[reg]=v;
}
void sexos_compiler_fence_seq_cst(void) {}
void sexos_sfence(void) {}
uint64_t sexos_map_pci_bar(uint64_t slot,uint64_t index,uint64_t length)
{ assert(slot==19 && index==0 && length==65536);return bar; }
uint64_t sexos_allocate_memory(uint64_t size)
{ assert(size==4096 && allocated<16);return (uint64_t)(++allocated)*4096; }
uint64_t sexos_map_memory(uint64_t address,uint64_t size)
{ assert(size==4096);return (uint64_t)(uintptr_t)physical(address); }
void sexos_yield(void) { ++yields; }
void sexos_line(const char *format,...) { (void)format; }
_Noreturn void sexos_fail(void) { exit(77); }

int main(int argc,char **argv)
{
    assert(argc==2);
    registers[0x5400/4]=0x12005452;registers[0x5404/4]=0x80005634;
    NetworkDevice device;assert(network_device_open(&device) && allocated==11);
    uint8_t frame[100];for(size_t i=0;i<sizeof frame;++i)frame[i]=(uint8_t)i;
    if(!strcmp(argv[1],"rx-binding")) {
        NetworkFrame output;
        RxContext bad={.device=&device,.frame=&output,.sealed_frame=&output,.descriptor=device.rx_desc+1,.index=0};
        (void)network_rx_run(&bad);abort();
    }
    if(!strcmp(argv[1],"binding")) {
        TxContext bad={.device=&device,.frame={frame,sizeof frame},.held=true,.identity=device.tx_page+1,.budget=1};
        (void)network_tx_run(&bad);abort();
    }
    if(!strcmp(argv[1],"permit")) {
        TxContext once={.device=&device,.frame={frame,sizeof frame}};
        assert(tx_valid(&once) && tx_acquire(&once) && tx_submit(&once));
        assert(once.attempted && !tx_permit(&once) && !tx_valid(&once));
        while(!tx_ready(&once)) assert(!tx_failed(&once));
        tx_transfer(&once);
        assert(!tx_submit(&once) && tx_count==1 && !pending);
        TxContext failed={.device=&device,.frame={frame,sizeof frame}};
        device.tx_busy=true;
        assert(!tx_submit(&failed) && failed.attempted);
        device.tx_busy=false;
        assert(!tx_valid(&failed) && !tx_submit(&failed) && tx_count==1);
    } else if(!strcmp(argv[1],"timeout")) {
        completion_delay=UINT32_MAX;
        assert(!network_device_send(&device,(cesta_read_bytes){frame,sizeof frame}));
        assert(device.failed && device.tx_busy && device.failure==NET_TIMEOUT);
        assert(pending && poll_count==65536 && yields==65536);
        assert(!network_device_send(&device,(cesta_read_bytes){frame,sizeof frame}));
        assert(tx_count==1 && memcmp(submitted,pending_buffer,pending_length)==0);
    } else if(!strcmp(argv[1],"error")) {
        completion_error=4;
        assert(!network_device_send(&device,(cesta_read_bytes){frame,sizeof frame}));
        assert(device.failed && !device.tx_busy && device.failure==NET_DEVICE_ERROR);
    } else {
        assert(!network_device_send(&device,(cesta_read_bytes){frame,1}));assert(tx_count==0);
        for(unsigned round=0;round<32;++round) {
            frame[0]=(uint8_t)round;
            assert(network_device_send(&device,(cesta_read_bytes){frame,sizeof frame}));
            assert(!pending && !device.tx_busy && poll_count==3);
            assert(submitted[0]==round && device.tx_next==(round+1)%8);
        }
        assert(tx_count==32 && yields==64);
        for(unsigned round=0;round<16;++round) {
            uint8_t *desc=(uint8_t *)(uintptr_t)device.rx_desc+device.rx_next*16;
            uint64_t pa;memcpy(&pa,desc,8);memcpy(physical(pa),frame,sizeof frame);
            uint16_t length=sizeof frame;memcpy(desc+8,&length,2);desc[12]=3;desc[13]=0;
            NetworkFrame received;
            rx_output=&received;rx_expect_valid=true;rx_size=sizeof frame;memcpy(rx_expected,frame,sizeof frame);
            assert(network_device_receive(&device,&received)==NET_READY);
            assert(received.length==sizeof frame && !memcmp(received.data,frame,sizeof frame) && desc[12]==0);
        }
        assert(rx_returns==16);
        NetworkFrame received={.length=99};
        unsigned before=rx_returns;
        assert(network_device_receive(&device,&received)==NET_PENDING && received.length==0 && rx_returns==before);
        for(unsigned bad=0;bad<3;++bad) {
            uint8_t *desc=(uint8_t *)(uintptr_t)device.rx_desc+device.rx_next*16;
            uint16_t length=bad==0?4097:100;memcpy(desc+8,&length,2);
            desc[12]=bad==2?1:3;desc[13]=bad==1?1:0;
            rx_output=&received;rx_expect_valid=false;
            assert(network_device_receive(&device,&received)==NET_PENDING && received.length==0 && desc[12]==0);
        }
        assert(rx_returns==19);
    }
    puts("device: bounded waits, ring wrap, exact bytes and buffer ownership passed");
    return 0;
}
