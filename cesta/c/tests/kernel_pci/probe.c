
#include "sexos_client.h"
#include "sexos_pci_policy.h"
static PciReply probe_call(uint64_t number,uint64_t slot,uint64_t a0,uint64_t a1,uint64_t a2,uint64_t a3)
{
    register uint64_t a __asm__("rax")=number,b __asm__("rsi")=a0,c __asm__("rdx")=a1;
    register uint64_t d __asm__("r10")=a2,e __asm__("r8")=a3;
    __asm__ volatile("syscall":"+a"(a),"+S"(b),"+d"(c),"+r"(d),"+r"(e):"D"(slot):"rcx","r11","memory","cc");
    return (PciReply){a,b,c,d,e};
}
static _Noreturn void failed(uint64_t stage)
{
    sexos_line("[pci.probe.failed] stage=%llu",(unsigned long long)stage);
    for(;;)sexos_yield();
}
static void require(bool ok,uint64_t stage){if(!ok)failed(stage);}
static void denied(PciReply r,uint64_t stage)
{
    require(r.status==PCI_DENIED && !(r.value0|r.value1|r.value2|r.value3),stage);
}
void pci_probe_main(void)
{
    PciReply id=probe_call(69,19,0,0,0,0);
    denied(probe_call(69,19|(UINT64_C(1)<<32),0,0,0,0),1);
    denied(probe_call(68,19|(UINT64_C(1)<<32),6,0,0,0),2);
    denied(probe_call(71,19|(UINT64_C(1)<<32),0,0,0,0),3);
    denied(probe_call(68,19,UINT64_C(1)<<32|6,0,0,0),4);
    denied(probe_call(68,19,1,0,0,0),5);
    denied(probe_call(68,19,2,2,0,0),6);
    denied(probe_call(68,19,0,0,1,0),7);
    denied(probe_call(71,19,1,0,0,0),8);
    if(!id.status){
        require(id.value0==0x8086 && (id.value1==0x100e || id.value1==0x10d3) && !(id.value2|id.value3),9);
        PciReply info=probe_call(55,32,0,0,0,0);
        require(!info.status && info.value0==PCI_DMA_BYTES && info.value1==15 && !info.value2 && info.value3==PCI_DMA_REGION_ID,10);
        PciReply config=probe_call(71,19,0,0,0,0);
        require(!config.status && config.value0==UINT64_C(0x010001180a00020f) && config.value1==UINT64_C(0x0a0002020a000202) &&
            config.value2==UINT64_C(0xbb80b3fcb3feb3ff) && config.value3==UINT64_C(0x0a000202),11);
        PciReply before=probe_call(68,19,0,0,0,0);
        require(!before.status && before.value0<=UINT16_MAX && before.value0==before.value1 && !(before.value2|before.value3),12);
        PciReply enabled=probe_call(68,19,6,0,0,0);
        require(!enabled.status && enabled.value0==before.value1 && enabled.value1==(before.value1|6) &&
            enabled.value2==(enabled.value0!=enabled.value1) && !enabled.value3,13);
        PciReply disabled=probe_call(68,19,0,4,0,0);
        require(!disabled.status && disabled.value0==enabled.value1 && disabled.value1==(enabled.value1&~UINT64_C(4)) && disabled.value2==1 && !disabled.value3,14);
        PciReply restored=probe_call(68,19,4,0,0,0);
        require(!restored.status && restored.value1==enabled.value1 && restored.value2==1 && !restored.value3,15);
        PciReply replay=probe_call(68,19,4,0,0,0);
        require(!replay.status && replay.value0==restored.value1 && replay.value1==restored.value1 && !(replay.value2|replay.value3),16);
        sexos_line("[pci.probe.done] vendor=%llu product=%llu enabled=%llu dma=held config=boot query_noop=1 disable=1 reenable=1 repeated_noop=1\n",
            (unsigned long long)id.value0,(unsigned long long)id.value1,(unsigned long long)enabled.value1);
    }else{
        denied(id,17);denied(probe_call(68,19,6,0,0,0),18);denied(probe_call(71,19,0,0,0,0),19);
        sexos_line("[pci.probe.unheld] rejected=1 data_zero=1");
    }
    for(;;){while(sexos_listen_once(0).type_id){};sexos_wait_self();}
}


