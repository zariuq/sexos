
#include "sexos_client.h"
/* Freestanding hardware probe. The normal kernel and real Sext service run
 * unchanged. Other BOOT payloads use the inert inbox loop in this same image. */
typedef struct {uint64_t status,base,length,target,rights;} ProbeInfo;
static ProbeInfo info(void)
{
    register uint64_t a __asm__("rax")=59,b __asm__("rsi"),c __asm__("rdx"),d __asm__("r10"),e __asm__("r8");
    __asm__ volatile("syscall":"+a"(a),"=S"(b),"=d"(c),"=r"(d),"=r"(e):"D"(UINT64_C(35)):"rcx","r11","memory","cc");
    return (ProbeInfo){a,b,c,d,e};
}
static uint64_t ticks(void)
{
    register uint64_t a __asm__("rax")=34;
    __asm__ volatile("syscall":"+a"(a)::"rcx","r11","memory","cc");return a;
}
static uint64_t delay(void)
{
    register uint64_t a __asm__("rax")=70,b __asm__("rsi")=0,c __asm__("rdx")=0,d __asm__("r10")=0,e __asm__("r8")=0;
    __asm__ volatile("syscall":"+a"(a),"+S"(b),"+d"(c),"+r"(d),"+r"(e):"D"(UINT64_C(1)):"rcx","r11","memory","cc");return a?UINT64_MAX:b;
}
static _Noreturn void failed(uint64_t target,uint64_t stage)
{
    sexos_line("[pager.probe.failed] target=%llu stage=%llu",(unsigned long long)target,(unsigned long long)stage);
    for(;;)sexos_yield();
}
static _Noreturn void finish(void)
{
    register uint64_t a __asm__("rax")=62;
    __asm__ volatile("syscall":"+a"(a):"D"(UINT64_C(0)):"rcx","r11","memory","cc");
    for(;;)sexos_yield();
}
typedef struct {uint64_t status,value;} ThreadReply;
static volatile uint32_t worker_done,worker_ready;
static uint64_t worker_target,worker_id;
static uint8_t worker_stack[65536] __attribute__((aligned(16)));

static ThreadReply thread_call(uint64_t op,uint64_t a,uint64_t b,uint64_t c)
{
    register uint64_t rax __asm__("rax")=83,rdi __asm__("rdi")=op,rsi __asm__("rsi")=a;
    register uint64_t rdx __asm__("rdx")=b,r10 __asm__("r10")=c,r8 __asm__("r8");
    __asm__ volatile("syscall":"+a"(rax),"+S"(rsi),"+d"(rdx),"+r"(r10),"=r"(r8):"D"(rdi):"rcx","r11","memory","cc");
    return (ThreadReply){rax,rsi};
}

static _Noreturn void worker_entry(uint64_t page)
{
    while(!worker_ready) {
        ThreadReply reply=thread_call(3,(uintptr_t)&worker_ready,0,0);
        if(reply.status && reply.status!=8)failed(worker_target,13);
    }
    volatile uint64_t *words=(volatile uint64_t *)(uintptr_t)page;
    if(words[0])failed(worker_target,8);
    words[0]=worker_target*10000+7;
    words[511]=worker_target*20000+7;
    if(words[0]!=worker_target*10000+7 || words[511]!=worker_target*20000+7)failed(worker_target,9);
    sexos_line("[pager.probe.worker.resumed] target=%llu worker=%llu zero=1 write=1",
               (unsigned long long)worker_target,(unsigned long long)worker_id);
    (void)thread_call(2,(uintptr_t)&worker_done,0,0);
    failed(worker_target,10);
}

static void worker_probe(uint64_t target,uint64_t page)
{
    worker_target=target;worker_done=0;worker_ready=0;
    ThreadReply reply=thread_call(1,(uintptr_t)worker_entry,
                                  (uintptr_t)(worker_stack+sizeof worker_stack),page);
    if(reply.status || !reply.value)failed(target,11);
    worker_id=reply.value;worker_ready=1;
    reply=thread_call(4,(uintptr_t)&worker_ready,1,0);
    if(reply.status || reply.value>1)failed(target,14);
    while(!worker_done) {
        reply=thread_call(3,(uintptr_t)&worker_done,0,0);
        if(reply.status && reply.status!=8)failed(target,12);
        if(!worker_done)sexos_yield();
    }
    sexos_line("[pager.probe.worker] target=%llu worker=%llu join=1",
               (unsigned long long)target,(unsigned long long)worker_id);
}

static void memory_probe(uint64_t target)
{
    SexosProgramMemory before=sexos_program_memory_grow(0);
    if(before.status || before.pages)failed(target,15);
    SexosProgramMemory memory=sexos_program_memory_grow(2);
    if(memory.status || memory.base!=before.base || memory.pages!=2 || memory.pressure>2)failed(target,16);
    volatile uint64_t *words=(volatile uint64_t *)(uintptr_t)memory.base;
    if(words[0] || words[511])failed(target,17);
    words[0]=target*30000+1;words[511]=target*40000+1;
    if(words[0]!=target*30000+1 || words[511]!=target*40000+1)failed(target,18);
    sexos_line("[pager.probe.memory] target=%llu base=%#llx held=2 zero=1 write=1",
               (unsigned long long)target,(unsigned long long)memory.base);
}

void pager_probe_main(void)
{
    ProbeInfo p=info();
    if(p.status) {
        for(;;) {while(sexos_listen_once(0).type_id){};sexos_wait_self();}
    }
    if((p.target!=10 && p.target!=15) || !p.base || p.length!=32768 || p.rights!=6)failed(p.target,1);
    for(uint64_t cycle=1;cycle<=2;++cycle) {
        uint64_t value=0,canary=0;
        __asm__ volatile("movabs $0x123456789abcdef0,%%r12\n\tmovq (%2),%0\n\tmovq %%r12,%1"
            :"=&r"(value),"=&r"(canary):"r"(p.base):"r12","memory");
        if(value || canary!=UINT64_C(0x123456789abcdef0))failed(p.target,2);
        volatile uint64_t *page=(volatile uint64_t *)(uintptr_t)p.base;
        page[0]=p.target*100+cycle;page[511]=p.target*1000+cycle;
        if(page[0]!=p.target*100+cycle || page[511]!=p.target*1000+cycle)failed(p.target,3);
        sexos_line("[pager.probe.resumed] target=%llu cycle=%llu zero=1 registers=1 write=1",(unsigned long long)p.target,(unsigned long long)cycle);
        if(cycle==1) {
            SexosReply accepted=sexos_pdx_call(2,0x104,p.base,1,0);if(accepted.status)failed(p.target,4);
            bool done=false;
            for(size_t poll=0;poll<4096;++poll) {
                SexosMessage m=sexos_listen_once(0);
                if(m.type_id==1 && m.caller_pd==14 && m.caller_task && m.arg0==256){done=true;break;}
                sexos_yield();
            }
            if(!done)failed(p.target,5);
            sexos_line("[pager.probe.released] target=%llu request=1 correlated=1",(unsigned long long)p.target);
        }
    }
    worker_probe(p.target,p.base+7*4096);
    for(size_t drain=0;drain<4096;++drain)if(!sexos_listen_once(0).type_id)break;
    bool timer_subject=p.target==15;
    if(timer_subject) {
        uint64_t start=ticks();size_t expirations=0;
        for(size_t attempt=0;attempt<512 && expirations<64;++attempt) {uint64_t receipt=delay();if(receipt==UINT64_MAX)failed(p.target,6);if(receipt==1)++expirations;}
        if(expirations!=64 || ticks()-start<64)failed(p.target,7);
    }
    memory_probe(p.target);
    sexos_line("[pager.probe.done] target=%llu cycles=2 timed_wake=%u",(unsigned long long)p.target,(unsigned)timer_subject);finish();
}


