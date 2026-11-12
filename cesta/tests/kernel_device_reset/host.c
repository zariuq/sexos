#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_kernel.h"

Kernel kernel;
static Domain subject,other;
static Cap nvme;
static _Alignas(4096) uint8_t bar[4096];
static uint32_t pci_identity=UINT32_C(0x5845101b);
static uint32_t pci_class=UINT32_C(0x01080000);
static uint64_t held_slot=16;
static unsigned events;
static bool msix_masked;
static unsigned msix_calls;
static bool internal_history;
static unsigned enqueue_fail;
typedef struct {uint64_t epoch,slot,generation,interrupt_revision,hardware;uint32_t subject,device,identity;uint8_t vector,action;} RecordedEvent;
static RecordedEvent recorded[16];

void klog(const char *format,...){(void)format;}
_Noreturn void kernel_halt(const char *reason){(void)reason;__builtin_abort();}
Domain *domain_get(uint32_t id){return id==subject.id?&subject:id==other.id?&other:NULL;}
bool domain_enqueue(Domain *d,RingEntry entry)
{
    assert(d==&subject&&entry.type_id==0xFF&&entry.arg0==0x22);
    if (enqueue_fail) {--enqueue_fail;return false;}
    kernel.vector_heard_revision[entry.arg0]=entry.arg1;
    return true;
}
uint32_t kernel_current_task(Domain *d){return d==&subject&&kernel.current==d->id&&d->task_state==TASK_RUNNING?d->task_id:0;}
const Cap *cap_get(Domain *d,uint64_t slot){return d==&subject&&slot==held_slot?&nvme:NULL;}
bool kernel_nvme_msix_set_mask(const Cap *lease,bool masked)
{assert(lease==&nvme);++msix_calls;msix_masked=masked;return true;}
uint32_t pci_read(uint8_t bus,uint8_t dev,uint8_t func,uint8_t offset)
{
    assert(bus==nvme.bus&&dev==nvme.dev&&func==nvme.func);
    return offset==0?pci_identity:offset==8?pci_class:0;
}
bool kernel_epoch_current(uint64_t epoch)
{
    return kernel.genesis_sealed&&kernel.epoch==epoch&&epoch<UINT64_MAX&&
        kernel.epoch_event_count<KERNEL_EPOCH_EVENT_CAPACITY;
}
bool kernel_epoch_log_internal(void) { return internal_history; }
void kernel_epoch_record_device_reset(uint64_t expected,uint32_t owner,uint64_t slot,
    uint32_t device,uint32_t identity,uint8_t vector,uint8_t action,uint64_t generation,
    uint64_t interrupt_revision,uint32_t cc,uint32_t csts)
{
    if (internal_history) return;
    assert(kernel_epoch_current(expected)&&events<16);
    recorded[events++]=(RecordedEvent){expected+1,slot,generation,interrupt_revision,
        ((uint64_t)cc<<32)|csts,owner,device,identity,vector,action};
    ++kernel.epoch;++kernel.epoch_event_count;
}

static TrapFrame invoke_raw(Domain *d,uint64_t slot,DeviceResetAction action,
                            uint64_t generation,uint64_t argument)
{
    TrapFrame frame={.rdi=slot,.rsi=action,.rdx=generation,.r10=argument};
    kernel_device_reset_syscall(d,&frame);return frame;
}
static TrapFrame invoke(Domain *d,uint64_t slot,DeviceResetAction action,
                        uint64_t generation,bool queues)
{
    return invoke_raw(d,slot,action,generation,queues?1:0);
}
static void set_hw(uint32_t cc,uint32_t csts)
{
    memcpy(bar+0x14,&cc,sizeof cc);memcpy(bar+0x1c,&csts,sizeof csts);
}

int main(void)
{
    subject=(Domain){.live=true,.id=2,.task_id=2,.task_state=TASK_RUNNING};
    other=(Domain){.live=true,.id=3,.task_id=3,.task_state=TASK_RUNNING};
    MemoryRegionGrant grant={.region_id=0x10810,.virtual_base=(uintptr_t)bar,.length=4096,.rights=7};
    nvme=(Cap){.kind=CAP_PCI,.bus=0,.dev=1,.func=0,.vendor_id=0x101b,.device_id=0x5845,
        .region={true,grant}};
    kernel.genesis_sealed=true;kernel.current=2;kernel.epoch=40;kernel.vector_owner[0x22]=2;
    kernel.vector_revision[0x22]=7;kernel.vector_heard_revision[0x22]=7;
    kernel.vector_ack_revision[0x22]=7;set_hw(0x00460001,1);

    held_slot=17;assert(invoke(&subject,17,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_INVALID);held_slot=16;
    uint32_t identity=pci_identity;pci_identity^=1;
    assert(invoke(&subject,16,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_INVALID);pci_identity=identity;
    TrapFrame r=invoke(&subject,16,DEVICE_RESET_QUERY,0,false);
    assert(r.rax==DEVICE_RESET_OK&&r.rsi==1&&r.rdx==7&&r.r9==7&&
        r.r10==DEVICE_RESET_ACTIVE&&!events);
    ++kernel.vector_revision[0x22];
    r=invoke(&subject,16,DEVICE_RESET_QUERY,0,false);
    assert(r.rax==DEVICE_RESET_OK&&r.rdx==8&&r.r9==7&&!events);
    --kernel.vector_revision[0x22];
    kernel.current=0;
    assert(invoke(&subject,16,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_INVALID);
    kernel.current=2;subject.task_state=TASK_READY;
    assert(invoke(&subject,16,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_INVALID);
    subject.task_state=TASK_RUNNING;
    assert(invoke(&other,16,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_INVALID);
    uint32_t owner=kernel.vector_owner[0x22];kernel.vector_owner[0x22]=3;
    assert(invoke(&subject,16,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_INVALID);kernel.vector_owner[0x22]=owner;
    uint32_t class_code=pci_class;pci_class=0x02080000;
    assert(invoke(&subject,16,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_INVALID);pci_class=class_code;

    uint64_t epoch=kernel.epoch;kernel.epoch=UINT64_MAX;
    assert(invoke(&subject,16,DEVICE_RESET_BEGIN,1,false).rax==DEVICE_RESET_EXHAUSTED&&!events);
    kernel.epoch=epoch;

    r=invoke(&subject,16,DEVICE_RESET_BEGIN,1,false);
    assert(r.rax==DEVICE_RESET_OK&&r.rsi==1&&r.r10==DEVICE_RESET_RESETTING&&events==1&&msix_masked);
    set_hw(0,0);r=invoke(&subject,16,DEVICE_RESET_OBSERVE_QUIESCENT,1,false);
    assert(r.rax==DEVICE_RESET_OK&&r.r10==DEVICE_RESET_QUIESCENT&&events==2);
    set_hw(0x00460001,1);r=invoke(&subject,16,DEVICE_RESET_RESUME,1,true);
    assert(r.rax==DEVICE_RESET_OK&&r.rsi==2&&r.r10==DEVICE_RESET_ACTIVE&&events==3&&!msix_masked);
    assert(recorded[0].action==1&&recorded[1].action==2&&recorded[2].action==3);
    assert(recorded[0].epoch==41&&recorded[1].epoch==42&&recorded[2].epoch==43);
    assert(recorded[0].interrupt_revision==7&&recorded[1].interrupt_revision==7&&recorded[2].interrupt_revision==7);
    assert((recorded[0].hardware&1)==1&&(recorded[1].hardware==0)&&(recorded[2].hardware&1)==1);

    r=invoke(&subject,16,DEVICE_RESET_BEGIN,2,false);assert(r.rax==DEVICE_RESET_OK&&events==4&&msix_masked);
    ++kernel.vector_revision[0x22];set_hw(0,0);
    r=invoke(&subject,16,DEVICE_RESET_OBSERVE_QUIESCENT,2,false);
    assert(r.rax==DEVICE_RESET_INTERRUPTED&&events==4);
    assert(invoke_raw(&subject,16,DEVICE_RESET_ACK_INTERRUPT,2,8).rax==DEVICE_RESET_INVALID&&events==4);
    kernel.vector_heard_revision[0x22]=8;
    r=invoke_raw(&subject,16,DEVICE_RESET_ACK_INTERRUPT,2,8);
    assert(r.rax==DEVICE_RESET_OK&&r.r9==8&&kernel.vector_ack_revision[0x22]==8&&events==5);
    assert(recorded[4].action==DEVICE_RESET_ACK_INTERRUPT&&recorded[4].interrupt_revision==8);
    r=invoke(&subject,16,DEVICE_RESET_OBSERVE_QUIESCENT,2,false);
    assert(r.rax==DEVICE_RESET_OK&&events==6);
    set_hw(0x00460001,1);r=invoke(&subject,16,DEVICE_RESET_RESUME,2,true);
    assert(r.rax==DEVICE_RESET_OK&&r.rsi==3&&events==7&&!msix_masked);
    r=invoke(&subject,16,DEVICE_RESET_BEGIN,3,false);assert(r.rax==DEVICE_RESET_OK&&events==8);
    ++kernel.vector_revision[0x22];
    msix_masked=false;
    assert(kernel_device_reset_owner_dead(&subject)&&events==9&&msix_masked);
    assert(kernel.device_reset.phase==DEVICE_RESET_ORPHANED&&kernel.vector_owner[0x22]==0);
    assert(recorded[8].action==DEVICE_RESET_OWNER_DEAD);

    kernel.vector_owner[0x22]=2;
    kernel.vector_heard_revision[0x22]=kernel.vector_revision[0x22];
    uint64_t heard=kernel.vector_heard_revision[0x22];
    enqueue_fail=2;
    kernel_device_interrupt_publish(0x22);
    kernel_device_interrupt_publish(0x22);
    uint64_t pending=kernel.vector_pending[0x22];
    assert(pending&&pending==heard+1&&kernel.vector_revision[0x22]==pending&&
           kernel.vector_heard_revision[0x22]==heard);
    enqueue_fail=1;
    assert(!kernel_device_interrupt_retry(0x22)&&kernel.vector_pending[0x22]==pending);
    assert(kernel_device_interrupt_retry(0x22)&&!kernel.vector_pending[0x22]&&
           kernel.vector_heard_revision[0x22]==pending&&kernel.vector_revision[0x22]==pending);

    /* Backlog is a protocol interruption, before any mask or reset effect. */
    _Static_assert(DEVICE_RESET_INTERRUPTED==3 && DEVICE_RESET_HARDWARE==4,"reset status ABI");
    events=msix_calls=0;
    kernel.epoch_event_count=0;
    kernel.vector_revision[0x22]=10;
    kernel.vector_heard_revision[0x22]=kernel.vector_ack_revision[0x22]=9;
    kernel.device_reset=device_reset_init(subject.id,16,0x100,pci_identity,0x22,9);
    r=invoke(&subject,16,DEVICE_RESET_BEGIN,1,false);
    assert(r.rax==DEVICE_RESET_INTERRUPTED&&r.r10==DEVICE_RESET_ACTIVE&&
           !events&&!msix_calls&&kernel.device_reset.phase==DEVICE_RESET_ACTIVE);
    assert(invoke_raw(&subject,16,DEVICE_RESET_ACK_INTERRUPT,1,10).rax==DEVICE_RESET_INVALID);
    assert(!events&&!msix_calls&&kernel.vector_ack_revision[0x22]==9);
    kernel.vector_heard_revision[0x22]=10;
    r=invoke_raw(&subject,16,DEVICE_RESET_ACK_INTERRUPT,1,10);
    assert(r.rax==DEVICE_RESET_OK&&r.r9==10&&events==1&&!msix_calls);
    r=invoke(&subject,16,DEVICE_RESET_BEGIN,1,false);
    assert(r.rax==DEVICE_RESET_OK&&r.r10==DEVICE_RESET_RESETTING&&events==2&&msix_calls==1);
    kernel.device_reset=device_reset_init(subject.id,16,0x100,pci_identity,0x22,10);
    uint64_t recorded_epoch=kernel.epoch;
    internal_history=true;
    r=invoke(&subject,16,DEVICE_RESET_BEGIN,1,false);
    assert(r.rax==DEVICE_RESET_OK&&r.r10==DEVICE_RESET_RESETTING&&events==2&&
           msix_calls==2&&kernel.epoch==recorded_epoch);
    internal_history=false;
    puts("kernel device reset controls passed: held NVMe, independent MMIO, vector revision and exact epoch receipts");
}
