#include "sexos_pci_policy.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

PciUpdate pci_policy_authorize(PciCommandGrant g,uint32_t caller,uint64_t set,uint64_t clear)
{
    if(!g.present || !caller || g.subject!=caller || g.device>31 || g.function>7 ||
        !g.vendor_id || g.vendor_id==UINT16_MAX || !g.allowed || (g.allowed&~PCI_NIC_BITS) ||
        set>UINT16_MAX || clear>UINT16_MAX)return (PciUpdate){0};
    if(((set|clear)&~(uint64_t)g.allowed) || (set&clear))return (PciUpdate){0};
    return (PciUpdate){true,g,(uint16_t)set,(uint16_t)clear};
}
PciUpdate pci_policy_held(PciDeviceCapability d,uint32_t caller,uint64_t slot,uint64_t set,uint64_t clear)
{
    if(!d.present || !slot || slot>1024)return (PciUpdate){0};
    PciUpdate u=pci_policy_authorize(d.command,caller,set,clear);
    if(!u.present)return u;
    PciCommandGrant g=u.grant;
    if(g.bus!=d.bus || g.device!=d.device || g.function!=d.function ||
        g.vendor_id!=d.vendor_id || g.device_id!=d.device_id)return (PciUpdate){0};
    uint64_t expected=UINT64_C(0x10000)+(uint64_t)caller*1024+slot;
    if(expected>UINT32_MAX || expected!=d.bar0.grant.region_id ||
        sexos_region_info(&d.bar0).error!=REGION_OK || d.bar0.grant.rights!=(REGION_QUERY|REGION_READ|REGION_WRITE) ||
        d.bar0.grant.has_dma || d.bar0.grant.has_peer || d.bar0.grant.length<PCI_REGISTERS_END || d.bar0.grant.length>65536)
        return (PciUpdate){0};
    return u;
}
bool pci_policy_needs_dma(PciUpdate u){return u.present && (u.set&PCI_BUS_MASTER);}
bool pci_policy_dma(MemoryRegionCapability c)
{
    if(c.grant.region_id!=PCI_DMA_REGION_ID || c.grant.length!=PCI_DMA_BYTES || c.grant.has_peer)return false;
    return sexos_region_view(&c,(MemoryRegionRequest){0,c.grant.length,REGION_KNOWN}).error==REGION_OK;
}
uint16_t pci_policy_next(PciUpdate u,uint16_t previous)
{
    return u.present?(uint16_t)((previous|u.set)&~u.clear):previous;
}
uint8_t pci_policy_model(uint16_t vendor,uint16_t product)
{
    if(vendor!=0x8086)return 0;
    if(product==0x100e)return 1;
    if(product==0x10d3)return 2;
    return 0;
}
PciApplyTrace pci_policy_apply_traced(PciUpdate u)
{
    if(!u.present)return (PciApplyTrace){.reply={.status=PCI_DENIED}};
    PciCommandGrant g=u.grant;
    uint32_t identity=pci_policy_read(g,0);
    if(identity!=((uint32_t)g.vendor_id|((uint32_t)g.device_id<<16)))
        return (PciApplyTrace){.reply={.status=PCI_IO},.identity=identity};
    uint16_t before=(uint16_t)pci_policy_read(g,4);
    uint16_t expected=pci_policy_next(u,before);
    bool attempted=expected!=before;if(attempted)pci_policy_write_command(g,expected);
    uint16_t after=(uint16_t)pci_policy_read(g,4);
    if(after!=expected)return (PciApplyTrace){.reply={.status=PCI_IO},.identity=identity,
        .before=before,.expected=expected,.after=after,.attempted=attempted};
    return (PciApplyTrace){.reply={0,before,after,before!=after,0},.identity=identity,
        .before=before,.expected=expected,.after=after,.attempted=attempted};
}
PciReply pci_policy_apply(PciUpdate u)
{
    return pci_policy_apply_traced(u).reply;
}


