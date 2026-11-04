#include "sexos_nvme_msix.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

static bool nvme_msix_range(uint32_t offset,uint64_t bytes,uint64_t length)
{
    return offset<=length&&bytes<=length-offset;
}
NvmeMsixPlan nvme_msix_plan(uint8_t capability_offset,uint32_t capability,
    uint32_t table,uint32_t pba,uint64_t bar_length,uint8_t vector,uint8_t destination)
{
    NvmeMsixPlan plan={.status=NVME_MSIX_INVALID,.capability_offset=capability_offset,
        .vector=vector,.destination=destination};
    if(capability_offset<0x40u || (capability_offset&3u) || (capability&0xffu)!=0x11u)
        return plan;
    plan.table_entries=(uint16_t)(((capability>>16)&0x7ffu)+1u);
    if(!plan.table_entries || vector<32u || vector==0xffu){plan.status=NVME_MSIX_UNSUPPORTED;return plan;}
    if((table&7u)!=0 || (pba&7u)!=0){plan.status=NVME_MSIX_UNSUPPORTED;return plan;}
    plan.table_offset=table&~7u;plan.pba_offset=pba&~7u;
    if(!nvme_msix_range(plan.table_offset,16u,bar_length) ||
       !nvme_msix_range(plan.pba_offset,8u,bar_length)){plan.status=NVME_MSIX_BOUNDS;return plan;}
    plan.address_low=UINT32_C(0xfee00000)|((uint32_t)destination<<12);
    plan.address_high=0;plan.message_data=vector;
    plan.vector_masked=1;plan.vector_enabled=0;
    plan.config_masked=capability|UINT32_C(0xc0000000);
    plan.config_enabled=(capability|UINT32_C(0x80000000))&~UINT32_C(0x40000000);
    plan.status=NVME_MSIX_OK;return plan;
}
bool nvme_msix_observed(const NvmeMsixPlan *p,uint32_t capability,
    uint32_t address_low,uint32_t address_high,uint32_t message_data,uint32_t vector_control)
{
    return p&&p->status==NVME_MSIX_OK&&capability==p->config_enabled&&
        address_low==p->address_low&&address_high==p->address_high&&
        message_data==p->message_data&&vector_control==p->vector_enabled;
}


