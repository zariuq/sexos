#include "sexos_device_reset.h"
#include "client_constants.h"

DeviceResetReceipt sexos_device_reset(uint64_t slot,DeviceResetAction action,
                                       uint64_t expected_generation,bool queues_ready)
{
    register uint64_t status __asm__("rax")=SEXOS_SYSCALL_device_reset;
    register uint64_t generation __asm__("rsi")=action;
    register uint64_t interrupt_revision __asm__("rdx")=expected_generation;
    register uint64_t phase __asm__("r10")=queues_ready?1:0;
    register uint64_t hardware __asm__("r8")=0;
    register uint64_t acknowledged __asm__("r9")=0;
    __asm__ volatile("syscall" : "+a"(status), "+S"(generation), "+d"(interrupt_revision),
        "+r"(phase), "+r"(hardware), "+r"(acknowledged)
        : "D"(slot) : "rcx", "r11", "memory", "cc");
    return(DeviceResetReceipt){(DeviceResetStatus)status,generation,interrupt_revision,
        acknowledged,(DeviceResetPhase)phase,(uint32_t)(hardware>>32),(uint32_t)hardware};
}

DeviceResetReceipt sexos_device_interrupt_ack(uint64_t slot,uint64_t expected_generation,
                                               uint64_t interrupt_revision)
{
    register uint64_t status __asm__("rax")=SEXOS_SYSCALL_device_reset;
    register uint64_t generation __asm__("rsi")=DEVICE_RESET_ACK_INTERRUPT;
    register uint64_t produced __asm__("rdx")=expected_generation;
    register uint64_t acknowledged __asm__("r10")=interrupt_revision;
    register uint64_t hardware __asm__("r8")=0;
    register uint64_t returned_ack __asm__("r9")=0;
    __asm__ volatile("syscall" : "+a"(status), "+S"(generation), "+d"(produced),
        "+r"(acknowledged), "+r"(hardware), "+r"(returned_ack)
        : "D"(slot) : "rcx", "r11", "memory", "cc");
    return(DeviceResetReceipt){(DeviceResetStatus)status,generation,produced,
        returned_ack,(DeviceResetPhase)acknowledged,(uint32_t)(hardware>>32),(uint32_t)hardware};
}
