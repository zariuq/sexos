#include "sexos_kernel.h"

#if SEXOS_KERNEL_CURRENT_SMP
static SmpParkState kernel_smp_state;
static uint32_t kernel_smp_ap_arrived;
static uint32_t kernel_smp_ap_lapic[SMP_PARK_MAX_CORES];
static uint8_t kernel_smp_ap_parked[SMP_PARK_MAX_CORES];

static _Noreturn void kernel_ap_stop(void)
{
    for(;;)cpu_hlt();
}
void kernel_ap_park(LimineMpInfo *info)
{
    cpu_cli();
    if(!info)kernel_ap_stop();
    size_t slot=(size_t)info->extra_argument;
    if(slot>=SMP_PARK_MAX_CORES)kernel_ap_stop();
    __atomic_store_n(&kernel_smp_ap_lapic[slot],info->lapic_id,__ATOMIC_RELAXED);
    if(__atomic_exchange_n(&kernel_smp_ap_parked[slot],1u,__ATOMIC_RELEASE)!=0)
        kernel_ap_stop();
    __atomic_fetch_add(&kernel_smp_ap_arrived,1u,__ATOMIC_RELEASE);
    kernel_ap_stop();
}

bool kernel_smp_park_boot(void)
{
    const LimineMpResponse *response=(const LimineMpResponse *)limine_mp_request.response;
    if(!response){
        if(smp_park_init(&kernel_smp_state,1,0,kernel.epoch)!=SMP_PARK_OK ||
           smp_park_enroll(&kernel_smp_state,0,0,true)!=SMP_PARK_OK ||
           smp_park_seal(&kernel_smp_state)!=SMP_PARK_OK)return false;
        klog("[smp.parked] supported=0 cpus=1 aps=0 generation=%llu\n",
            (unsigned long long)kernel_smp_state.generation);
        return true;
    }
    if(!response->cpu_count || response->cpu_count>SMP_PARK_MAX_CORES || !response->cpus)
        return false;
    size_t count=(size_t)response->cpu_count,bsp_slot=count;
    if(smp_park_init(&kernel_smp_state,count,response->bsp_lapic_id,kernel.epoch)!=SMP_PARK_OK)
        return false;
    for(size_t i=0;i<count;++i){
        LimineMpInfo *info=response->cpus[i];
        if(!info)return false;
        bool bsp=info->lapic_id==response->bsp_lapic_id;
        if(bsp){if(bsp_slot!=count)return false;bsp_slot=i;}
        if(smp_park_enroll(&kernel_smp_state,i,info->lapic_id,bsp)!=SMP_PARK_OK)
            return false;
    }
    if(bsp_slot==count)return false;
    __atomic_store_n(&kernel_smp_ap_arrived,0u,__ATOMIC_RELAXED);
    for(size_t i=0;i<count;++i){
        __atomic_store_n(&kernel_smp_ap_lapic[i],0u,__ATOMIC_RELAXED);
        __atomic_store_n(&kernel_smp_ap_parked[i],0u,__ATOMIC_RELAXED);
    }
    for(size_t i=0;i<count;++i)if(i!=bsp_slot){
        LimineMpInfo *info=response->cpus[i];
        info->extra_argument=i;
        __atomic_store_n(&info->goto_address,kernel_ap_park,__ATOMIC_RELEASE);
    }
    uint32_t expected=(uint32_t)(count-1u);
    uint64_t spins=0;
    while(__atomic_load_n(&kernel_smp_ap_arrived,__ATOMIC_ACQUIRE)!=expected &&
          spins<UINT64_C(100000000)){cpu_pause();++spins;}
    if(__atomic_load_n(&kernel_smp_ap_arrived,__ATOMIC_ACQUIRE)!=expected)return false;
    for(size_t i=0;i<count;++i)if(i!=bsp_slot){
        uint8_t parked=__atomic_load_n(&kernel_smp_ap_parked[i],__ATOMIC_ACQUIRE);
        uint32_t lapic=__atomic_load_n(&kernel_smp_ap_lapic[i],__ATOMIC_ACQUIRE);
        if(parked!=1u || smp_park_arrive(&kernel_smp_state,i,lapic)!=SMP_PARK_OK)
            return false;
    }
    if(smp_park_seal(&kernel_smp_state)!=SMP_PARK_OK)return false;
    klog("[smp.parked] supported=1 cpus=%zu aps=%u bsp_lapic=%u generation=%llu x2apic=%u\n",
        count,expected,response->bsp_lapic_id,
        (unsigned long long)kernel_smp_state.generation,(unsigned)(response->flags&1u));
    return true;
}
bool kernel_smp_epoch_current(uint64_t expected)
{
    return smp_park_can_commit(&kernel_smp_state,expected);
}
bool kernel_smp_epoch_advance(uint64_t expected,uint64_t next)
{
#if defined(SEXOS_SMP_EPOCH_CORRUPT_PROBE) && SEXOS_SMP_EPOCH_CORRUPT_PROBE
    ++next;
#endif
    return smp_park_advance(&kernel_smp_state,expected,next)==SMP_PARK_OK;
}
#endif


