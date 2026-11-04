/* Execute the real BOOT grants, capability lookup, task check and source adapter.
 * The processor is a scripted provider, not a substitute authorization model. */
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_domains.c"
#include "kernel_sources.c"

static unsigned cpuid_calls,seed_calls,io_reads,io_writes;
static uint32_t max_leaf=7,feature=(1u<<18);
static unsigned seed_success=1;
static uint64_t seed_word;
static struct {uint8_t index,value;} events[128];
static size_t event_count,event_next;
static bool selected;
static char log_text[16384];
static size_t log_length;
static unsigned checks;

void cpu_cpuid(uint32_t leaf,uint32_t subleaf,uint32_t result[4])
{
    assert(subleaf==0 && (leaf==0 || leaf==7));++cpuid_calls;
    memset(result,0,4*sizeof *result);
    if(leaf==0)result[0]=max_leaf;else result[1]=feature;
}
bool cpu_rdseed(uint64_t *out)
{
    assert(max_leaf>=7 && (feature&(1u<<18)));++seed_calls;
    *out=seed_word;return seed_calls==seed_success;
}
void cpu_outb(uint16_t port,uint8_t value)
{
    assert(port==0x70 && !selected && event_next<event_count);
    assert(value==events[event_next].index && value!=0x0c && !(value&0x80));
    ++io_writes;selected=true;
}
uint8_t cpu_inb(uint16_t port)
{
    assert(port==0x71 && selected && event_next<event_count);
    ++io_reads;selected=false;return events[event_next++].value;
}
void klog(const char *format,...)
{
    va_list args;va_start(args,format);
    int n=vsnprintf(log_text+log_length,sizeof log_text-log_length,format,args);
    va_end(args);assert(n>=0 && (size_t)n<sizeof log_text-log_length);
    log_length+=(size_t)n;
}
_Noreturn void kernel_halt(const char *reason){fprintf(stderr,"%s\n",reason);abort();}

static void reset_provider(void)
{
    cpuid_calls=seed_calls=io_reads=io_writes=0;
    event_count=event_next=0;selected=false;max_leaf=7;feature=1u<<18;
    seed_success=1;seed_word=0;log_length=0;log_text[0]=0;
}
static void event(uint8_t index,uint8_t value)
{
    assert(event_count<sizeof events/sizeof *events);
    events[event_count].index=index;events[event_count++].value=value;
}
static void snapshot(SourceClockRaw r,unsigned changed)
{
    event(0x0a,r.status_a);event(0x0b,r.status_b);event(0x0d,r.status_d);
    event(0,r.sec);event(2,r.min);event(4,r.hour);event(7,r.day);
    event(8,r.month);event(9,r.year);event(0x32,r.century);
    event(0,r.sec^((changed&1)?1:0));
    if(!(changed&1))event(0x0a,r.status_a^((changed&2)?1:0));
    event(0x0b,r.status_b^((changed&4)?1:0));
}
static SourceClockRaw valid_clock(void)
{
    return (SourceClockRaw){0,0,0,1,1,0x20,0x20,0x26,2,0x80};
}
static TrapFrame invoke(Domain *d,uint64_t number,uint64_t slot,uint64_t kind,
                        uint64_t a,uint64_t b,uint64_t c)
{
    TrapFrame f={.rax=number,.rdi=slot,.rsi=kind,.rdx=a,.r10=b,.r8=c};
    kernel_source_syscall(d,&f,number);assert(!selected && event_next==event_count);
    ++checks;return f;
}
static void denied(Domain *d,uint64_t number,uint64_t slot,uint64_t kind,
                   uint64_t a,uint64_t b,uint64_t c)
{
    reset_provider();TrapFrame f=invoke(d,number,slot,kind,a,b,c);
    assert(f.rax==SOURCE_DENIED && !(f.rsi|f.rdx|f.r10|f.r8));
    assert(!(cpuid_calls|seed_calls|io_reads|io_writes) && !log_length);
}
static Domain *record(uint32_t id,uint8_t key)
{
    Domain *d=domain_boot_record(id);assert(d);
    d->live=true;d->key=key;assert(task_attach(d));d->task_state=TASK_READY;
    return d;
}
static void schedule(Domain *d)
{
    for(size_t i=0;i<kernel.domains.count;++i)
        kernel.domains.records[i].task_state=TASK_READY;
    kernel.current=d->id;d->task_state=TASK_RUNNING;
}
int main(void)
{
#define MAKE_DOMAIN(p_id,p_name,p_module,p_log,p_key,p_layout,p_active) (void)record(p_id,p_key);
    SEXOS_CURRENT_DOMAIN_ROWS(MAKE_DOMAIN)
#undef MAKE_DOMAIN
    Domain *net=domain_get(13),*browser=domain_get(15),*bar=domain_get(6);
    assert(net && browser && bar);net->task_id=137;browser->task_id=151;
    kernel_sources_boot();assert(kernel.cap_storage.count==KERNEL_CURRENT_SOURCE_CAPS);
    const Cap *entropy=cap_get(net,60),*clock=cap_get(net,61),*trust=cap_get(net,62);
    assert(entropy && clock && trust && cap_get(bar,61));
#define CHECK_SOURCE(p_holder,p_slot,p_kind,p_consumer) do { \
    Domain *h=domain_get(CURRENT_PD_##p_holder),*c=domain_get(CURRENT_PD_##p_consumer); \
    const Cap *cap=cap_get(h,p_slot);assert(cap && cap->kind==CAP_##p_kind); \
    assert(cap->source_grant.present && cap->source_grant.source==1); \
    assert(cap->source_grant.holder.pd==h->id && cap->source_grant.holder.task==h->task_id); \
    assert(cap->source_grant.consumer.pd==c->id && cap->source_grant.consumer.task==c->task_id); \
} while(0);
    SEXOS_CURRENT_SOURCE_ROWS(CHECK_SOURCE)
#undef CHECK_SOURCE
    kernel_seal_genesis();assert(!cap_set(browser,60,*entropy));
    schedule(net);
    for(unsigned number=72;number<=74;++number){
        unsigned slot=number-12;
        for(unsigned shift=0;shift<64;++shift)
            if((slot|(UINT64_C(1)<<shift))!=slot)
                denied(net,number,slot|(UINT64_C(1)<<shift),1,0,0,0);
        denied(net,number,0,1,0,0,0);
        denied(net,number,slot,1,UINT64_C(1)<<32,0,0);
        denied(net,number,slot,1,0,UINT64_C(1)<<63,0);
        denied(net,number,slot,1,0,0,UINT64_MAX);
        denied(net,number,slot,UINT64_C(1)<<32|1,0,0,0);
        denied(net,number,slot,0,0,0,0);
        denied(net,number,slot==60?61:60,1,0,0,0);
        Domain copy=*net;denied(&copy,number,slot,1,0,0,0);
        net->task_state=TASK_READY;denied(net,number,slot,1,0,0,0);net->task_state=TASK_RUNNING;
        uint32_t task=net->task_id;net->task_id=0;denied(net,number,slot,1,0,0,0);net->task_id=task;
        net->task_id=task+1;denied(net,number,slot,1,0,0,0);net->task_id=task;
        Cap *actual=(Cap *)(uintptr_t)cap_get(net,slot);Cap saved=*actual;
        actual->source_grant.consumer.task=0;denied(net,number,slot,1,0,0,0);*actual=saved;
        actual->source_grant.present=false;denied(net,number,slot,1,0,0,0);*actual=saved;
    }
    denied(net,99,60,1,0,0,0);
    schedule(browser);denied(browser,72,60,2,0,0,0);schedule(net);
    reset_provider();TrapFrame f=invoke(net,72,60,SOURCE_INFO,0,0,0);
    assert(!f.rax && f.rsi==1 && f.rdx==(UINT64_C(15)<<32|151) && f.r10==8 && f.r8==1);
    assert(cpuid_calls==2 && !seed_calls && !(io_reads|io_writes));
    for(unsigned maximum=0;maximum<9;++maximum){
        reset_provider();max_leaf=maximum;feature=0;
        f=invoke(net,72,60,SOURCE_READ,0,0,0);
        assert(f.rax==SOURCE_UNAVAILABLE && cpuid_calls==(maximum<7?1:2) && !seed_calls);
    }
    for(unsigned attempt=0;attempt<=9;++attempt){
        reset_provider();seed_success=attempt;seed_word=UINT64_C(0xdfe81234abcd5678);
        f=invoke(net,72,60,SOURCE_READ,0,0,0);
        bool success=attempt>=1 && attempt<=8;
        assert(f.rax==(success?0:SOURCE_BUSY) && seed_calls==(success?attempt:8));
        assert(cpuid_calls==2 && !(io_reads|io_writes));
        if(success){assert(f.rsi==seed_word && f.rdx==attempt && !(f.r10|f.r8));
            char payload[32];snprintf(payload,sizeof payload,"%"PRIu64,seed_word);
            if(log_length)assert(strstr(log_text,"private=1"));
            assert(!strstr(log_text,payload) && !strstr(log_text,"abcd5678"));}
        else assert(!(f.rsi|f.rdx|f.r10|f.r8) && !log_length);
    }
    reset_provider();f=invoke(net,72,60,SOURCE_READ,0,0,0);
    assert(!f.rax && !f.rsi && f.rdx==1 && seed_calls==1);
    reset_provider();f=invoke(net,73,61,SOURCE_INFO,0,0,0);
    assert(!f.rax && f.rsi==1 && f.rdx==(UINT64_C(15)<<32|151) && f.r10==4 && f.r8==1);
    assert(!(cpuid_calls|seed_calls|io_reads|io_writes));
    for(unsigned attempt=1;attempt<=4;++attempt){
        reset_provider();SourceClockRaw raw=valid_clock();
        for(unsigned i=1;i<=attempt;++i){snapshot(raw,i<attempt?1:0);snapshot(raw,0);}
        f=invoke(net,73,61,SOURCE_READ,0,0,0);
        assert(!f.rax && f.rsi==UINT64_C(1577836800) && f.rdx==attempt && f.r10==1 && !f.r8);
        assert(io_reads==io_writes && !cpuid_calls && !seed_calls && strstr(log_text,"seconds=1577836800"));
    }
    for(unsigned changed=1;changed<8;++changed){
        reset_provider();SourceClockRaw raw=valid_clock();
        for(unsigned i=0;i<4;++i){snapshot(raw,changed);snapshot(raw,changed);}
        f=invoke(net,73,61,SOURCE_READ,0,0,0);
        assert(f.rax==SOURCE_BUSY && !(f.rsi|f.rdx|f.r10|f.r8) && !log_length);
        assert(io_reads==io_writes && !(cpuid_calls|seed_calls));
    }
    reset_provider();SourceClockRaw raw=valid_clock();raw.status_d=0;
    for(unsigned i=0;i<4;++i){snapshot(raw,0);snapshot(raw,0);}
    f=invoke(net,73,61,SOURCE_READ,0,0,0);assert(f.rax==SOURCE_UNAVAILABLE && !log_length);
    reset_provider();f=invoke(net,74,62,SOURCE_INFO,0,0,0);
    assert(!f.rax && f.rsi==SOURCE_TRUST_PROFILE && f.rdx==(15|(UINT64_C(151)<<32)) &&
           f.r10==SOURCE_TRUST_ROOT_BYTES && f.r8==SOURCE_TRUST_VERSION);
    assert(!(cpuid_calls|seed_calls|io_reads|io_writes) && !log_length);
    schedule(bar);reset_provider();raw=valid_clock();snapshot(raw,0);snapshot(raw,0);
    f=invoke(bar,73,61,SOURCE_READ,0,0,0);assert(!f.rax && f.rsi==UINT64_C(1577836800));
    denied(bar,72,61,SOURCE_READ,0,0,0);
    assert(kernel.clock_samples==5 && kernel.entropy_samples==9);
    printf("source adapter: %u requests;exact held identities, bounded hardware trace, five reply lanes\n",checks);
    return 0;
}
