#include "sexos_arm64_boot.h"
#include <string.h>

#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)

int main(void)
{
    Arm64Phase4State state={0},before;uint64_t next=0,interval=0;
    CHECK(arm64_phase4_state_init(&state,100,200)==ARM64_PHASE4_OK);
    CHECK(state.started&&!state.b_dead&&state.current==ARM64_TASK_A&&state.frame_a==100&&state.frame_b==200);
    CHECK(arm64_phase4_state_init(0,100,200)==ARM64_PHASE4_BAD_STATE);
    CHECK(arm64_phase4_state_init(&state,100,100)==ARM64_PHASE4_BAD_STATE);
    CHECK(arm64_phase4_state_init(&state,0,200)==ARM64_PHASE4_BAD_STATE);
    CHECK(arm64_phase4_state_init(&state,100,200)==ARM64_PHASE4_OK);
    before=state;CHECK(arm64_phase4_tick(&state,300,29,&next)==ARM64_PHASE4_BAD_INTERRUPT);
    CHECK(memcmp(&state,&before,sizeof state)==0);
    CHECK(arm64_phase4_tick(&state,300,30,&next)==ARM64_PHASE4_OK);
    CHECK(state.frame_a==300&&state.current==ARM64_TASK_B&&next==200);
    CHECK(arm64_phase4_tick(&state,400,30,&next)==ARM64_PHASE4_OK);
    CHECK(state.frame_b==400&&state.current==ARM64_TASK_A&&next==300);
    CHECK(arm64_phase4_tick(&state,500,30,&next)==ARM64_PHASE4_OK);
    CHECK(state.frame_a==500&&state.current==ARM64_TASK_B&&next==400);
    CHECK(arm64_phase4_kill(&state,600,&next)==ARM64_PHASE4_OK);
    CHECK(state.b_dead&&state.current==ARM64_TASK_A&&state.frame_b==600&&next==500);
    CHECK(arm64_phase4_tick(&state,700,30,&next)==ARM64_PHASE4_OK);
    CHECK(state.current==ARM64_TASK_A&&state.frame_a==700&&next==700);
    CHECK(arm64_phase4_state_init(&state,100,200)==ARM64_PHASE4_OK);
    state.started=false;before=state;CHECK(arm64_phase4_kill(&state,300,&next)==ARM64_PHASE4_BOOT_FAULT);
    CHECK(memcmp(&state,&before,sizeof state)==0);
    state.started=true;state.current=ARM64_TASK_A;before=state;
    CHECK(arm64_phase4_kill(&state,300,&next)==ARM64_PHASE4_SURVIVOR_FAULT);
    CHECK(memcmp(&state,&before,sizeof state)==0);
    Arm64GicObservation g={(1u<<4)|(1u<<1),0,1,0xff,1,1u<<30,1u<<30};
    CHECK(arm64_phase4_gic_valid(g));
    g.pmr=0xf8;CHECK(arm64_phase4_gic_valid(g));
    g.pmr=0xfc;CHECK(arm64_phase4_gic_valid(g));
    g.pmr=0xfe;CHECK(arm64_phase4_gic_valid(g));
    g.pmr=0xff;
    Arm64GicObservation badg=g;badg.distributor_control&=~(1u<<1);CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.redistributor_waker=1u<<2;CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.sre=0;CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.pmr=0xf0;CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.pmr=0xf9;CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.pmr=0x1ff;CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.group_enable=0;CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.timer_group=0;CHECK(!arm64_phase4_gic_valid(badg));
    badg=g;badg.timer_enable=0;CHECK(!arm64_phase4_gic_valid(badg));
    Arm64ApObservation ap={0,0x40090000,0x400a0000,true};CHECK(arm64_phase4_ap_valid(ap));
    Arm64ApObservation badap=ap;badap.psci_result=-1;CHECK(!arm64_phase4_ap_valid(badap));
    badap=ap;badap.ttbr1=badap.ttbr0;CHECK(!arm64_phase4_ap_valid(badap));
    badap=ap;badap.ttbr0++;CHECK(!arm64_phase4_ap_valid(badap));
    badap=ap;badap.acknowledged=false;CHECK(!arm64_phase4_ap_valid(badap));
    CHECK(arm64_phase4_timer_interval(1000000,&interval)&&interval==10000);
    CHECK(!arm64_phase4_timer_interval(99,&interval));
    CHECK(!arm64_phase4_timer_interval(1000000,0));
    return 0;
}
