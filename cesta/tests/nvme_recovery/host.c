/* The implementation is the actual exported driver, not a parallel adapter. */
#include "sexdrive.c"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

static _Alignas(4096) uint8_t mmio[16384], sq[4096], cq[4096];
static _Alignas(4096) uint8_t read_page[4096], write_page[4096], caller_page[4096];
static DeviceResetState policy;
static uint64_t produced, heard, acknowledged;
static unsigned begins, queries, acknowledgements, yields, observes;
static uint64_t clock_ticks;
static unsigned clock_step, complete_after_yields;
static DeviceResetStatus force_begin;
static bool stop_after_begin;
static bool interrupt_after_begin, stop_after_observe;
static jmp_buf restart;

void sexos_line(const char *format, ...) { (void)format; }
uint64_t sexos_get_ticks(void) { return clock_ticks; }
void sexos_yield(void) {
    ++yields;
    clock_ticks += clock_step;
    if (complete_after_yields && yields == complete_after_yields) {
        uint32_t head = sexdrive_state.nvme_io_state.cq_head;
        uint32_t phase = sexdrive_state.nvme_io_state.cq_phase;
        uint32_t command = ((const uint32_t *)sq)[0];
        if ((command & 0xff) == 2) memset(read_page, 0x6b, sizeof read_page);
        ((volatile uint32_t *)cq)[head * 4 + 2] =
            ((const volatile uint32_t *)mmio)[0x1008 / 4] | (1u << 16);
        ((volatile uint32_t *)cq)[head * 4 + 3] = (command >> 16) | (phase << 16);
    }
}
void sexos_spin_loop_hint(void) {}
_Noreturn void sexos_fail(void) { abort(); }
uint64_t sexos_map_pci_bar(uint64_t slot, uint64_t index, uint64_t bytes)
{
    (void)slot; (void)index; (void)bytes;
    /* This fixture ends at the accepted reset entry, before rebuilding hardware. */
    abort();
}

DeviceResetReceipt sexos_device_reset(uint64_t slot, DeviceResetAction action,
                                      uint64_t generation, bool queues)
{
    assert(slot == 16);
    if (action == DEVICE_RESET_BEGIN) {
        ++begins;
    } else if (action == DEVICE_RESET_QUERY) {
        ++queries;
    } else {
        assert(action == DEVICE_RESET_OBSERVE_QUIESCENT);
        ++observes;
    }
    DeviceResetState next=policy;
    DeviceResetReceipt receipt = device_reset_apply(&next, (DeviceResetRequest){
        .action=action,.expected_generation=generation,.interrupt_revision=produced,
        .acknowledged_revision=acknowledged,.cc=action==DEVICE_RESET_OBSERVE_QUIESCENT?0:1,
        .csts=action==DEVICE_RESET_OBSERVE_QUIESCENT?0:1,.queues_ready=queues});
    if (action == DEVICE_RESET_BEGIN && receipt.status == DEVICE_RESET_OK && force_begin != DEVICE_RESET_OK)
        receipt.status=force_begin;
    if (receipt.status == DEVICE_RESET_OK) policy=next;
    if (action == DEVICE_RESET_BEGIN && receipt.status == DEVICE_RESET_OK && interrupt_after_begin) produced=1;
    if (action == DEVICE_RESET_BEGIN && receipt.status == DEVICE_RESET_OK && stop_after_begin)
        longjmp(restart, 1);
    if (action == DEVICE_RESET_OBSERVE_QUIESCENT && receipt.status == DEVICE_RESET_OK && stop_after_observe)
        longjmp(restart, 1);
    return receipt;
}

DeviceResetReceipt sexos_device_interrupt_ack(uint64_t slot, uint64_t generation, uint64_t revision)
{
    assert(slot == 16);
    ++acknowledgements;
    if (!revision || revision != heard)
        return (DeviceResetReceipt){.status=DEVICE_RESET_INVALID,.generation=policy.generation,
                                    .phase=policy.phase};
    DeviceResetReceipt receipt = device_reset_apply(&policy, (DeviceResetRequest){
        .action=DEVICE_RESET_ACK_INTERRUPT,.expected_generation=generation,
        .interrupt_revision=produced,.acknowledged_revision=revision,.cc=1,.csts=1});
    if (receipt.status == DEVICE_RESET_OK) acknowledged = receipt.acknowledged_revision;
    return receipt;
}

static void setup(void)
{
    memset(mmio,0,sizeof mmio); memset(sq,0,sizeof sq); memset(cq,0,sizeof cq);
    memset(read_page,0,sizeof read_page); memset(write_page,0,sizeof write_page);
    memset(caller_page,0xa5,sizeof caller_page);
    sexdrive_state_init(&sexdrive_state);
    sexdrive_state.nvme_io_state.ready=true;
    sexdrive_state.nvme_io_state.map_va=(uintptr_t)mmio;
    sexdrive_state.nvme_io_state.io_sq_va=(uintptr_t)sq;
    sexdrive_state.nvme_io_state.io_cq_va=(uintptr_t)cq;
    sexdrive_state.nvme_io_state.sq1tdbl=0x1008;
    sexdrive_state.nvme_io_state.cq1hdbl=0x100c;
    sexdrive_state.dma_device=dma_device_epoch_init(1);
    sexdrive_state.read_lease=dma_lease_init((DmaLeaseBuffer){(uintptr_t)read_page,0x300000,4096},&sexdrive_state.dma_device);
    sexdrive_state.write_lease=dma_lease_init((DmaLeaseBuffer){(uintptr_t)write_page,0x301000,4096},&sexdrive_state.dma_device);
    policy=device_reset_init(2,16,0x100,0x5845101b,0x22,0);
    produced=1; heard=acknowledged=0;
    begins=queries=acknowledgements=yields=observes=0;
    clock_ticks=0;clock_step=50;complete_after_yields=0;
    force_begin=DEVICE_RESET_OK; stop_after_begin=false;
    interrupt_after_begin=stop_after_observe=false;
}

static uint64_t request(unsigned kind, bool retry)
{
    if (kind == 0) return nvme_read_into_mapped_va_attempt(0,512,(uintptr_t)caller_page,retry);
    if (kind == 1) return nvme_write_one_block_attempt(UINT64_C(2047)*512,512,(uintptr_t)caller_page,retry);
    return nvme_flush_attempt(retry);
}

static SexosMessage irq(void)
{
    return (SexosMessage){.type_id=0xff,.arg0=0x22,.arg1=1,.arg2=1,.caller_pd=1,.caller_task=0};
}

static void interrupted_io(unsigned kind)
{
    setup();
    uint16_t before=sexdrive_state.nvme_io_state.next_cid;
    assert(request(kind,true) == BLOCK_ERR_TIMEOUT);
    assert(begins==1 && acknowledged==0 && policy.phase==DEVICE_RESET_ACTIVE);
    assert(!sexdrive_state.nvme_io_state.ready && sexdrive_state.reset_recovery_pending);
    assert(sexdrive_state.dma_device.phase==DMA_DEVICE_ACTIVE && sexdrive_state.dma_device.generation==1);
    assert(dma_lease_is_quarantined(&sexdrive_state.read_lease) == (kind==0));
    assert(dma_lease_is_quarantined(&sexdrive_state.write_lease) == (kind==1));
    if (kind!=1) for (size_t i=0;i<sizeof caller_page;++i) assert(caller_page[i]==0xa5);
    uint16_t after=sexdrive_state.nvme_io_state.next_cid;
    assert(after==(uint16_t)(before+1));
    uint8_t old_sq[sizeof sq]; memcpy(old_sq,sq,sizeof sq);
    assert(request(kind,true)==BLOCK_ERR_TIMEOUT && sexdrive_state.nvme_io_state.next_cid==after);
    assert(!memcmp(sq,old_sq,sizeof sq) && begins==1);
    assert(!nvme_recovery_maybe_start() && begins==1 && sexdrive_state.reset_recovery_pending);

    SexosMessage message=irq(); message.caller_pd=2;
    assert(!nvme_handle_interrupt(message) && !acknowledgements);
    message=irq(); message.caller_task=7;
    assert(!nvme_handle_interrupt(message) && !acknowledgements);
    message=irq(); message.arg2=2;
    assert(nvme_handle_interrupt(message) && !acknowledgements && begins==1);
    message=irq(); message.arg1=2;
    assert(nvme_handle_interrupt(message) && acknowledgements==1 && !acknowledged && begins==1);

    heard=1;
    stop_after_begin=true;
    if (!setjmp(restart)) {
        (void)nvme_handle_interrupt(irq());
        assert(!"an acknowledged deferred I/O must reach reset entry");
    }
    assert(acknowledged==1 && begins==2 && policy.phase==DEVICE_RESET_RESETTING);
    /* No new IRQ, generation advance, buffer reuse or resubmission was needed. */
    assert(produced==1 && sexdrive_state.dma_device.generation==1);
    assert(sexdrive_state.nvme_io_state.next_cid==after && !memcmp(sq,old_sq,sizeof sq));
    assert(dma_lease_is_quarantined(&sexdrive_state.read_lease) == (kind==0));
    assert(dma_lease_is_quarantined(&sexdrive_state.write_lease) == (kind==1));
    stop_after_begin=false;
    (void)nvme_handle_interrupt(irq());
    assert(begins==2 && acknowledged==1);
}

static void late_completion(void)
{
    for (unsigned kind = 0; kind < 3; ++kind) {
        setup();
        clock_step = 1;
        complete_after_yields = 246;
        assert(request(kind, true) == 0);
        assert(yields == 246 && clock_ticks < 5000 && !begins && !queries);
        assert(sexdrive_state.nvme_io_state.ready && sexdrive_state.dma_device.generation == 1);
        assert(!dma_lease_is_quarantined(&sexdrive_state.read_lease) &&
               !dma_lease_is_quarantined(&sexdrive_state.write_lease));
        if (kind == 0)
            for (size_t i = 0; i < 512; ++i) assert(caller_page[i] == 0x6b);
    }
}

static void clock_boundaries(void)
{
    setup();
    clock_ticks = 100;
    assert(nvme_wait_started() == 100);
    assert(nvme_wait_pending(0, 1000000, 100));
    clock_ticks = 5099;
    assert(nvme_wait_pending(4096, 1000000, 100));
    clock_ticks = 5100;
    assert(!nvme_wait_pending(4096, 1000000, 100));
    clock_ticks = 99;
    assert(!nvme_wait_pending(4096, 1000000, 100));
    clock_ticks = 0;
    assert(!nvme_wait_pending(4096, 1000000, UINT64_MAX - 1));
    clock_ticks = 100;
    assert(!nvme_wait_pending(UINT32_MAX, 1000000, 100));
    assert(nvme_wait_pending(1003520, 1000000, 100));
}

int main(void)
{
    clock_boundaries();
    late_completion();
    _Static_assert(DEVICE_RESET_INTERRUPTED==3 && DEVICE_RESET_HARDWARE==4,"reset status ABI");
    for (unsigned kind=0;kind<3;++kind) interrupted_io(kind);
    setup(); produced=0; interrupt_after_begin=true;
    assert(request(0,true)==BLOCK_ERR_TIMEOUT && begins==1 && observes==1);
    assert(policy.phase==DEVICE_RESET_RESETTING && sexdrive_state.dma_device.phase==DMA_DEVICE_RESETTING);
    assert(sexdrive_state.reset_recovery_pending && dma_lease_is_quarantined(&sexdrive_state.read_lease));
    heard=1; stop_after_observe=true;
    if (!setjmp(restart)) {
        (void)nvme_handle_interrupt(irq());
        assert(!"acknowledged reset must reach observed quiescence without another begin");
    }
    assert(begins==1 && observes==2 && acknowledged==1 && policy.phase==DEVICE_RESET_QUIESCENT);
    assert(sexdrive_state.dma_device.generation==1 && dma_lease_is_quarantined(&sexdrive_state.read_lease));
    for (unsigned kind=0;kind<3;++kind) {
        setup(); produced=0; force_begin=DEVICE_RESET_HARDWARE;
        assert(request(kind,true)==BLOCK_ERR_NO_DEVICE && !sexdrive_state.reset_recovery_pending);
        setup();
        assert(request(kind,false)==BLOCK_ERR_NO_DEVICE && !begins && !sexdrive_state.reset_recovery_pending);
    }
    setup(); sexdrive_state.nvme_io_state.ready=false;
    assert(request(0,true)==BLOCK_ERR_NO_DEVICE && !begins);
    setup(); heard=1;
    assert(nvme_handle_interrupt(irq()) && acknowledged==1 && !begins && !queries);
    puts("NVMe deferred recovery prefix: read/write/flush, exact IRQ authority, no resubmit/reuse before reset; terminal and one-shot controls passed");
    return 0;
}
