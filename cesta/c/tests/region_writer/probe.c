
#include "sexos_server.h"
enum { WRITER_SLOT=17, BLOCK_SLOT=15, PROBE_OPCODE=0x2d0, HOME_PD=11, PEER_PD=2 };
static _Noreturn void finish_with_claim(void)
{
    register uint64_t status __asm__("rax")=62;
    __asm__ volatile("syscall" : "+a"(status) : "D"(UINT64_C(99)) : "rcx","r11","memory","cc");
    for(;;)sexos_yield();
}
static _Noreturn void failed(uint64_t stage)
{
    sexos_line("[region.writer.probe.failed] stage=%llu",(unsigned long long)stage);
    for(;;)sexos_yield();
}
static void require(bool ok,uint64_t stage){if(!ok)failed(stage);}
static uint64_t writer_view(void)
{
    MemoryRegionReply info=sexos_memory_region_info(WRITER_SLOT);
    require(!info.status && info.value0==4096 && info.value1==7 && info.value3==4,1);
    MemoryRegionReply view=sexos_memory_region_view(WRITER_SLOT,0,4096,REGION_QUERY|REGION_READ|REGION_WRITE);
    require(!view.status && view.value0 && view.value1==4096 && view.value3==4,2);
    return view.value0;
}
static _Noreturn void home(void)
{
    volatile uint64_t *shared=(volatile uint64_t *)(uintptr_t)writer_view();
    RegionWriterReceipt query=sexos_region_writer_query(WRITER_SLOT);
    require(!query.status && query.current==HOME_PD && query.peer==PEER_PD && !query.job,3);
    *shared=UINT64_C(0x1111222233334444);
    RegionWriterReceipt offered=sexos_region_writer_change(WRITER_SLOT,REGION_WRITER_OFFER,query.revision,99);
    require(!offered.status && offered.current==HOME_PD && offered.job==99,4);
    SexosReply sent=sexos_pdx_call(BLOCK_SLOT,PROBE_OPCODE,99,0,0);
    require(!sent.status,5);
    SexosListener listener={0};SexosMessage reply=sexos_listen(&listener,0);
    require(reply.type_id==1 && reply.arg0==UINT64_C(0x2222333344445555),6);
    RegionWriterReceipt returned=sexos_region_writer_query(WRITER_SLOT);
    require(!returned.status && returned.current==HOME_PD && !returned.job && returned.revision==offered.revision+2,7);
    require(*shared==UINT64_C(0x2222333344445555),8);
    *shared=UINT64_C(0x3333444455556666);
    RegionWriterReceipt second=sexos_region_writer_change(WRITER_SLOT,REGION_WRITER_OFFER,returned.revision,100);
    require(!second.status && second.current==HOME_PD && second.job==100,15);
    sent=sexos_pdx_call(BLOCK_SLOT,PROBE_OPCODE+1,100,0,0);require(!sent.status,16);
    reply=sexos_listen(&listener,0);
    require(reply.type_id==1 && reply.arg0==SEXOS_ERR_SERVICE_NOT_READY,17);
    RegionWriterReceipt recovered=sexos_region_writer_query(WRITER_SLOT);
    require(!recovered.status && recovered.current==HOME_PD && !recovered.job && recovered.revision==second.revision+2,18);
    *shared=UINT64_C(0x4444555566667777);
    sexos_line("[region.writer.probe.home.done] revision=%llu value=%#llx recovered=1",
        (unsigned long long)recovered.revision,(unsigned long long)*shared);
    for(;;)sexos_wait_self();
}
static _Noreturn void peer(void)
{
    volatile uint64_t *shared=(volatile uint64_t *)(uintptr_t)writer_view();
    SexosListener listener={0};SexosMessage request=sexos_listen(&listener,0);
    require(request.type_id==PROBE_OPCODE && request.caller_pd==HOME_PD && request.arg0==99,9);
    RegionWriterReceipt offered=sexos_region_writer_query(WRITER_SLOT);
    require(!offered.status && offered.current==HOME_PD && offered.peer==PEER_PD && offered.job==99,10);
    RegionWriterReceipt claimed=sexos_region_writer_change(WRITER_SLOT,REGION_WRITER_CLAIM,offered.revision,99);
    require(!claimed.status && claimed.current==PEER_PD && claimed.job==99,11);
    require(*shared==UINT64_C(0x1111222233334444),12);
    *shared=UINT64_C(0x2222333344445555);
    RegionWriterReceipt released=sexos_region_writer_change(WRITER_SLOT,REGION_WRITER_RELEASE,claimed.revision,99);
    require(!released.status && released.current==HOME_PD && !released.job,13);
    require(!sexos_reply(HOME_PD,UINT64_C(0x2222333344445555)),14);
    sexos_line("[region.writer.probe.peer.done] revision=%llu job=0",
        (unsigned long long)released.revision);
    request=sexos_listen(&listener,0);
    require(request.type_id==PROBE_OPCODE+1 && request.caller_pd==HOME_PD && request.arg0==100,19);
    offered=sexos_region_writer_query(WRITER_SLOT);
    require(!offered.status && offered.current==HOME_PD && offered.job==100,20);
    claimed=sexos_region_writer_change(WRITER_SLOT,REGION_WRITER_CLAIM,offered.revision,100);
    require(!claimed.status && claimed.current==PEER_PD && claimed.job==100,21);
    *shared=UINT64_C(0xdeadbeefdeadbeef);
    finish_with_claim();
}
void region_writer_probe_main(void)
{
    MemoryRegionReply info=sexos_memory_region_info(WRITER_SLOT);
    if(info.status){sexos_line("[region.writer.probe.unheld] rejected=1");for(;;)sexos_wait_self();}
    if(info.value2==PEER_PD)home();
    if(info.value2==HOME_PD)peer();
    failed(15);
}


