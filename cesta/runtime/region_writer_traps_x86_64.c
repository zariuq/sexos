#include "sexos_region_writer.h"
#include "sexos_memory_regions.h"
#include "client_constants.h"

static MemoryRegionReply region_call(uint64_t number,uint64_t slot,uint64_t offset,
                                     uint64_t length,uint64_t rights)
{
    register uint64_t status __asm__("rax")=number;
    register uint64_t a __asm__("rsi")=offset;
    register uint64_t b __asm__("rdx")=length;
    register uint64_t c __asm__("r10")=rights;
    register uint64_t d __asm__("r8")=0;
    __asm__ volatile("syscall" : "+a"(status), "+S"(a), "+d"(b), "+r"(c), "+r"(d)
        : "D"(slot) : "rcx", "r11", "memory", "cc");
    return (MemoryRegionReply){status,a,b,c,d};
}

MemoryRegionReply sexos_memory_region_view(uint64_t slot,uint64_t offset,
                                           uint64_t length,uint64_t rights)
{
    return region_call(54,slot,offset,length,rights);
}

MemoryRegionReply sexos_memory_region_info(uint64_t slot)
{
    return region_call(55,slot,0,0,0);
}

static RegionWriterReceipt writer_call(uint64_t slot,uint64_t action,
                                       uint64_t expected,uint64_t job)
{
    register uint64_t status __asm__("rax")=SEXOS_SYSCALL_region_writer;
    register uint64_t revision __asm__("rsi")=action;
    register uint64_t current __asm__("rdx")=expected;
    register uint64_t active_job __asm__("r10")=job;
    register uint64_t peer __asm__("r8")=0;
    __asm__ volatile("syscall" : "+a"(status), "+S"(revision), "+d"(current),
        "+r"(active_job), "+r"(peer) : "D"(slot) : "rcx", "r11", "memory", "cc");
    if(current>UINT32_MAX || peer>UINT32_MAX)return (RegionWriterReceipt){.status=REGION_WRITER_INVALID};
    return (RegionWriterReceipt){(RegionWriterStatus)status,revision,(uint32_t)current,
        active_job,(uint32_t)peer};
}

RegionWriterReceipt sexos_region_writer_query(uint64_t slot)
{
    return writer_call(slot,0,0,0);
}

RegionWriterReceipt sexos_region_writer_change(uint64_t slot,RegionWriterAction action,
                                                uint64_t expected_revision,uint64_t job)
{
    return writer_call(slot,action,expected_revision,job);
}
