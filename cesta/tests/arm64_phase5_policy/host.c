#include "sexos_arm64_boot.h"

#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)
#define ERR UINT64_C(0xfffffffffffffffc)

int main(void)
{
    CHECK(arm64_phase5_svc_result(1,41)==42);
    CHECK(arm64_phase5_svc_result(1,UINT64_MAX)==0);
    CHECK(arm64_phase5_svc_result(2,42)==0);
    CHECK(arm64_phase5_svc_result(2,0xdead)==ERR);
    CHECK(arm64_phase5_svc_result(3,42)==ERR);
    Arm64Phase5Completion good={5,0x24,UINT64_C(0x60000000),42,ERR,0};
    CHECK(arm64_phase5_completion_valid(good));
    Arm64Phase5Completion bad=good;bad.tag=4;CHECK(!arm64_phase5_completion_valid(bad));
    bad=good;bad.ec=0x20;CHECK(!arm64_phase5_completion_valid(bad));
    bad=good;bad.far+=0x1000;CHECK(!arm64_phase5_completion_valid(bad));
    bad=good;bad.echo=41;CHECK(!arm64_phase5_completion_valid(bad));
    bad=good;bad.cap_bad=0;CHECK(!arm64_phase5_completion_valid(bad));
    bad=good;bad.cap_good=ERR;CHECK(!arm64_phase5_completion_valid(bad));
    return 0;
}
