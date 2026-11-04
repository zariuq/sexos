#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_pager_state.h"

static PgBindingResult validate(PgMapping m,uint64_t frame)
{
    PgFramePool pool={1,0x10000,PG_PAGES*4096};
    PgFaultView fault={101,m.target_pd,m.target_pd,14,m.base_va,4};
    return pager_mapping_validate(m,pool,14,fault,frame,PG_READ|PG_WRITE);
}
int main(void)
{
    PgMapping first={1,1,10,0x4000,PG_PAGES*4096,PG_READ|PG_WRITE,4};
    PgMapping second={2,1,15,0x9000,PG_PAGES*4096,PG_READ|PG_WRITE,4};
    assert(!validate(first,0).status&&!validate(first,3).status);
    assert(validate(first,4).status==PG_DENIED);
    assert(!validate(second,4).status&&!validate(second,7).status);
    assert(validate(second,0).status==PG_DENIED);
    PgMapping oversized=first;oversized.quota=5;
    assert(validate(oversized,0).status==PG_DENIED);
    PgBinding a=validate(first,0).value,b=validate(second,4).value;
    PgLedger left={0},right={0};
    assert(pager_ledger_install(&left,a,4)==PG_OK);
    assert(pager_ledger_install(&left,b,4)==PG_OK);
    assert(pager_ledger_install(&right,b,4)==PG_OK);
    assert(pager_ledger_install(&right,a,4)==PG_OK);
    assert(!memcmp(&left,&right,sizeof left));
    PgBinding forged=a;forged.frame_index=4;
    assert(pager_ledger_install(&(PgLedger){0},forged,4)==PG_DENIED);
    puts("pager partition controls passed: kernel refuses cross-range frames and independent installs commute");
}
