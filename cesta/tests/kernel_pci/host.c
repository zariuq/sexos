#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_domains.c"
#include "kernel_pci.c"

static uint64_t v[48];
static uint32_t selected,command_word;
static unsigned command_reads;
static struct {char direction;unsigned width,port;uint32_t value;} events[16];
static size_t event_count;
void klog(const char *format,...){(void)format;}
_Noreturn void kernel_halt(const char *reason){fprintf(stderr,"%s\n",reason);abort();}
static void event(char direction,unsigned width,unsigned port,uint32_t value)
{
    assert(event_count<sizeof events/sizeof *events);
    events[event_count].direction=direction;events[event_count].width=width;
    events[event_count].port=port;events[event_count++].value=value;
}
void cpu_outl(uint16_t port,uint32_t value)
{
    assert(port==0xcf8);selected=value;event('O',32,port,value);
}
uint32_t cpu_inl(uint16_t port)
{
    assert(port==0xcfc);
    uint32_t expected=UINT32_C(0x80000000)|((uint32_t)(uint8_t)v[11]<<16)|
        ((uint32_t)(uint8_t)v[12]<<11)|((uint32_t)(uint8_t)v[13]<<8);
    assert((selected&~UINT32_C(0xfc))==expected);
    uint32_t value=0;
    if((selected&0xfc)==0)value=(uint32_t)v[42];
    else {assert((selected&0xfc)==4);value=command_word;++command_reads;if(v[44]==2 && command_reads==2)value^=1;}
    event('I',32,port,value);return value;
}
void cpu_outw(uint16_t port,uint16_t value)
{
    assert(port==0xcfc && (selected&0xfc)==4);event('O',16,port,value);
    if(v[44]!=1)command_word=(command_word&UINT32_C(0xffff0000))|value;
}
static MemoryRegionCapability region(size_t at)
{
    return (MemoryRegionCapability){v[at]!=0,{.region_id=(uint32_t)v[at+1],.virtual_base=v[at+2],.length=v[at+3],
        .rights=(uint8_t)v[at+4],.has_dma=v[at+5]!=0,.dma_base=v[at+6],.has_peer=v[at+7]!=0,.peer_pd=(uint32_t)v[at+8]}};
}
int main(void)
{
    static Domain domain;uint32_t prior=0;
    while(scanf("%"SCNu64,&v[0])==1){
        for(size_t i=1;i<48;++i)if(scanf("%"SCNu64,&v[i])!=1)return 2;
        event_count=0;selected=0;command_reads=0;command_word=(uint32_t)v[43];
        PciCommandGrant grant={v[9]!=0,(uint32_t)v[10],(uint8_t)v[16],(uint8_t)v[17],(uint8_t)v[18],(uint16_t)v[19],(uint16_t)v[20],(uint16_t)v[21]};
        PciReply result={0};
        if(v[0]==0){
            PciUpdate u=pci_policy_authorize(grant,(uint32_t)v[1],v[3],v[4]);
            result=(PciReply){u.present,pci_policy_needs_dma(u),pci_policy_next(u,(uint16_t)v[43]),pci_policy_model((uint16_t)v[14],(uint16_t)v[15]),0};
        }else{
            if(prior)kernel.domains.slots[prior]=NULL;
            uint32_t id=(uint32_t)v[1];assert(id>0 && id<KERNEL_MAX_DOMAINS);prior=id;
            domain.id=id;domain.live=true;domain.task_id=id+100;domain.task_state=TASK_RUNNING;
            kernel.domains.slots[id]=&domain;kernel.current=id;
            Cap device={.kind=v[8]?CAP_PCI:CAP_DOMAIN,.bus=(uint8_t)v[11],.dev=(uint8_t)v[12],.func=(uint8_t)v[13],
                .vendor_id=(uint16_t)v[14],.device_id=(uint16_t)v[15],.command=grant,.region=region(22)};
            Cap dma={.kind=v[32]?CAP_REGION:CAP_DOMAIN,.region=region(33)};
            domain.caps[19]=v[7]?&device:NULL;domain.caps[32]=v[31]?&dma:NULL;
            kernel.network_config_present=v[45]!=0;kernel.network_config_owner=(uint32_t)v[46];kernel.network_config=net_config_controlled();
            kernel.genesis_sealed=true;kernel.epoch=0;
            kernel.epoch_event_count=v[47]?KERNEL_EPOCH_EVENT_CAPACITY:0;
            TrapFrame f={.rdi=v[2],.rsi=v[3],.rdx=v[4],.r10=v[5],.r8=v[6]};
            kernel_pci_syscall(&domain,&f,v[0]==1?69:v[0]==2?68:71);
            result=(PciReply){f.rax,f.rsi,f.rdx,f.r10,f.r8};
            if(v[0]==2 && result.status!=PCI_DENIED && result.status!=PCI_EXHAUSTED){
                assert(kernel.epoch==1 && kernel.epoch_event_count==1);
                KernelEpochEvent e=kernel.epoch_events[0];
                assert(e.epoch==1 && e.kind==KERNEL_EPOCH_PCI_COMMAND && e.subject==domain.id &&
                    e.address==v[2] && e.token==v[3] && e.frame==v[4] && e.device_status==result.status);
                uint32_t packed=((uint32_t)(uint8_t)v[11]<<16)|((uint32_t)(uint8_t)v[12]<<8)|(uint8_t)v[13];
                uint32_t identity=(uint32_t)v[42],expected_identity=(uint16_t)v[14]|((uint32_t)(uint16_t)v[15]<<16);
                assert(e.device==packed && e.device_identity==identity && e.device_expected_identity==expected_identity);
                if(!result.status)assert(e.device_before==result.value0 && e.device_after==result.value1 &&
                    e.device_expected==result.value1 && e.device_attempted==(result.value0!=result.value1));
            }else assert(!kernel.epoch && kernel.epoch_event_count==(v[47]?KERNEL_EPOCH_EVENT_CAPACITY:0));
        }
        printf("%"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %u",result.status,result.value0,result.value1,result.value2,result.value3,command_word);
        for(size_t i=0;i<event_count;++i)printf("|%c%u:%u:%u",events[i].direction,events[i].width,events[i].port,events[i].value);
        putchar('\n');
    }
    return 0;
}
