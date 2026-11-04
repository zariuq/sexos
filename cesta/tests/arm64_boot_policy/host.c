#include <assert.h>
#include <stdio.h>
#include "sexos_arm64_boot.h"
int main(void)
{
    assert(arm64_boot_validate((Arm64BootObservation){0,2,1,false})==ARM64_BOOT_OK);
    assert(arm64_boot_validate((Arm64BootObservation){0,1,1,false})==ARM64_BOOT_OK);
    assert(arm64_boot_validate((Arm64BootObservation){0,0,1,false})==ARM64_BOOT_BAD_ENTERED_EL);
    assert(arm64_boot_validate((Arm64BootObservation){0,3,1,false})==ARM64_BOOT_BAD_ENTERED_EL);
    assert(arm64_boot_validate((Arm64BootObservation){0,2,2,false})==ARM64_BOOT_BAD_CURRENT_EL);
    assert(arm64_boot_validate((Arm64BootObservation){0x40000001,2,1,true})==ARM64_BOOT_BAD_DTB);
    assert(arm64_boot_validate((Arm64BootObservation){0x40000000,2,1,false})==ARM64_BOOT_BAD_DTB);
    assert(arm64_boot_validate((Arm64BootObservation){0x40000000,2,1,true})==ARM64_BOOT_OK);
    puts("ARM64 boot policy controls passed: EL drop and optional aligned DTB fail closed");
}
