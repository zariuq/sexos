#include "sexos_nvme_msix.h"

#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)

int main(void)
{
    NvmeMsixPlan plan=nvme_msix_plan(0x40,UINT32_C(0x00408011),0x2000,0x3000,0x4000,0x22,0);
    CHECK(plan.status==NVME_MSIX_OK&&plan.table_entries==65);
    CHECK(plan.table_offset==0x2000&&plan.pba_offset==0x3000);
    CHECK(plan.address_low==UINT32_C(0xfee00000)&&plan.address_high==0&&plan.message_data==0x22);
    CHECK(plan.vector_masked==1&&plan.vector_enabled==0);
    CHECK(plan.config_masked==UINT32_C(0xc0408011)&&plan.config_enabled==UINT32_C(0x80408011));
    CHECK(nvme_msix_observed(&plan,plan.config_enabled,plan.address_low,0,plan.message_data,0));
    CHECK(!nvme_msix_observed(0,plan.config_enabled,plan.address_low,0,plan.message_data,0));
    CHECK(!nvme_msix_observed(&plan,plan.config_masked,plan.address_low,0,plan.message_data,0));
    CHECK(!nvme_msix_observed(&plan,plan.config_enabled,plan.address_low|0x1000,0,plan.message_data,0));
    CHECK(!nvme_msix_observed(&plan,plan.config_enabled,plan.address_low,0,plan.message_data+1,0));
    CHECK(!nvme_msix_observed(&plan,plan.config_enabled,plan.address_low,0,plan.message_data,1));
    CHECK(nvme_msix_plan(0x3c,UINT32_C(0x00408011),0x2000,0x3000,0x4000,0x22,0).status==NVME_MSIX_INVALID);
    CHECK(nvme_msix_plan(0x42,UINT32_C(0x00408011),0x2000,0x3000,0x4000,0x22,0).status==NVME_MSIX_INVALID);
    CHECK(nvme_msix_plan(0x40,UINT32_C(0x00408005),0x2000,0x3000,0x4000,0x22,0).status==NVME_MSIX_INVALID);
    CHECK(nvme_msix_plan(0x40,UINT32_C(0x00408011),0x2001,0x3000,0x4000,0x22,0).status==NVME_MSIX_UNSUPPORTED);
    CHECK(nvme_msix_plan(0x40,UINT32_C(0x00408011),0x2000,0x3001,0x4000,0x22,0).status==NVME_MSIX_UNSUPPORTED);
    CHECK(nvme_msix_plan(0x40,UINT32_C(0x00408011),0x4000,0x3000,0x4000,0x22,0).status==NVME_MSIX_BOUNDS);
    CHECK(nvme_msix_plan(0x40,UINT32_C(0x00408011),0x2000,0x4000,0x4000,0x22,0).status==NVME_MSIX_BOUNDS);
    CHECK(nvme_msix_plan(0x40,UINT32_C(0x00408011),0x2000,0x3000,0x4000,31,0).status==NVME_MSIX_UNSUPPORTED);
    CHECK(nvme_msix_plan(0x40,UINT32_C(0x00408011),0x2000,0x3000,0x4000,0xff,0).status==NVME_MSIX_UNSUPPORTED);
    plan=nvme_msix_plan(0x40,UINT32_C(0x00408011),0x2000,0x3000,0x4000,0x22,7);
    CHECK(plan.status==NVME_MSIX_OK&&plan.address_low==UINT32_C(0xfee07000));
    return 0;
}
