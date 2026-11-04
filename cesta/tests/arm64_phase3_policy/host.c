#include "sexos_arm64_boot.h"
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)

static Arm64MapContract map(Arm64MapRole role,uint64_t va,uint64_t pa,uint64_t bytes,
                            bool el0,bool write,bool execute,bool device)
{
    return (Arm64MapContract){role,va,pa,bytes,el0,write,execute,device};
}

int main(void)
{
    const uint64_t page=UINT64_C(0x1000),block2m=UINT64_C(0x200000),block1g=UINT64_C(0x40000000);
    const uint64_t kernel=UINT64_C(0x40080000),uart=UINT64_C(0x09000000);
    const uint64_t hhdm=UINT64_C(0xffff800000000000);
    Arm64MapContract good[]={
        map(ARM64_MAP_KERNEL_TEXT,kernel,kernel,page,false,false,true,false),
        map(ARM64_MAP_KERNEL_DATA,kernel+page,kernel+page,page,false,true,false,false),
        map(ARM64_MAP_UART,uart,uart,block2m,false,true,false,true),
        map(ARM64_MAP_USER_CODE,UINT64_C(0x00400000),UINT64_C(0x40100000),page,true,false,true,false),
        map(ARM64_MAP_USER_CODE2,UINT64_C(0x00430000),UINT64_C(0x40103000),page,true,false,true,false),
        map(ARM64_MAP_USER_DATA,UINT64_C(0x00410000),UINT64_C(0x40101000),page,true,true,false,false),
        map(ARM64_MAP_USER_STACK,UINT64_C(0x00420000),UINT64_C(0x40102000),page,true,true,false,false),
        map(ARM64_MAP_HHDM_DEVICE,hhdm,0,block1g,false,true,false,true),
        map(ARM64_MAP_HHDM_DRAM,hhdm+block1g,block1g,block1g,false,true,false,false),
    };
    for(size_t i=0;i<sizeof good/sizeof *good;i++)CHECK(arm64_phase3_map_valid(good[i]));
    Arm64MapContract bad=good[0];bad.write=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[0];bad.el0=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[1];bad.execute=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[1];bad.el0=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[2];bad.device=false;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[2];bad.bytes=page;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[3];bad.write=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[3];bad.execute=false;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[4];bad.va+=page;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[5];bad.execute=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[6];bad.va+=page;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[7];bad.pa=block1g;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[8];bad.el0=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[8];bad.device=true;CHECK(!arm64_phase3_map_valid(bad));
    bad=good[8];bad.bytes=0;CHECK(!arm64_phase3_map_valid(bad));
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){1,0x24,kernel})==ARM64_FAULT_SKIP);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){2,0x24,kernel})==ARM64_FAULT_SKIP);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){3,0x20,UINT64_C(0x00410000)})==ARM64_FAULT_REDIRECT);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){4,0x24,UINT64_C(0x50000000)})==ARM64_FAULT_COMPLETE);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){1,0x20,kernel})==ARM64_FAULT_HALT);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){2,0x24,kernel+page})==ARM64_FAULT_HALT);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){3,0x24,UINT64_C(0x00410000)})==ARM64_FAULT_HALT);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){3,0x20,UINT64_C(0x00410004)})==ARM64_FAULT_HALT);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){4,0x24,UINT64_C(0x50001000)})==ARM64_FAULT_HALT);
    CHECK(arm64_phase3_fault_decide((Arm64FaultObservation){9,0x24,kernel})==ARM64_FAULT_HALT);
    return 0;
}
