#include "sexos_memory_regions.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    uint64_t v[14];
    while (scanf("%" SCNu64, &v[0]) == 1) {
        for (unsigned i=1; i<14; ++i) if (scanf("%" SCNu64, &v[i]) != 1) return 2;
        MemoryRegionCapability c = {v[0] != 0,
            {(uint32_t)v[1],v[2],v[3],(uint8_t)v[4],v[5] != 0,v[6],v[7] != 0,(uint32_t)v[8]}};
        MemoryRegionRequest q = {v[10],v[11],(uint8_t)v[12]};
        uint64_t out[7]={0};
        if (v[9] == 4) out[1]=sexos_region_valid(&c.grant);
        else if (v[9] == 2) {
            MemoryRegionInfoResult r=sexos_region_info(&c);
            out[0]=r.error; out[1]=r.value.region_id; out[2]=r.value.length;
            out[3]=r.value.rights; out[4]=r.value.has_peer; out[5]=r.value.peer_pd;
        } else if (v[9] == 0 || v[9] == 1 || v[9] == 3) {
            MemoryRegionViewResult r;
            if (v[9] == 0) r=sexos_region_view(&c,q);
            else if (v[9] == 1) r=sexos_region_register_view(&c,v[10],v[11],v[12]);
            else r=sexos_region_peer_view(&c,(uint32_t)v[13],q);
            out[0]=r.error; out[1]=r.value.region_id; out[2]=r.value.virtual_address;
            out[3]=r.value.length; out[4]=r.value.has_dma; out[5]=r.value.dma_address;
        } else {
            MemoryRegionReply r=sexos_region_query(&c,v[9],v[10],v[11],v[12]);
            out[0]=r.status; out[1]=r.value0; out[2]=r.value1; out[3]=r.value2; out[4]=r.value3;
        }
        for (unsigned i=0; i<7; ++i) printf("%s%" PRIu64,i ? " " : "",out[i]);
        putchar('\n');
    }
    return 0;
}
