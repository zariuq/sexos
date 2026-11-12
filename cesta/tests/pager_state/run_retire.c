#include "sexos_pager_state.h"
#include <assert.h>
#include <stdio.h>

static PgFaultRecord record(uint64_t token, uint32_t task)
{
    return (PgFaultRecord){token, 10, 14, task, 0x4001, 0x4000, 6, 0x1000, 0x2000};
}

static void complete(PgFaultState *state, uint64_t token, uint32_t task)
{
    assert(!pager_fault_prepare(state, record(token, task)));
    assert(!pager_fault_publish(state, token));
    assert(!pager_fault_claim(state, 14, token).status);
    PgFaultResult result = pager_fault_complete(state, 14, token);
    assert(!result.status && result.present && result.record.task_id == task);
}

int main(void)
{
    PgFaultState state = {0};
    complete(&state, 1, 0x40000001);
    assert(!pager_fault_retire_run(&state, 10, 14));
    assert(state.retired && state.phase == PG_IDLE && !state.has_completed);

    state = (PgFaultState){0};
    complete(&state, 1, 1010);
    assert(!pager_fault_prepare(&state, record(2, 0x40000002)));
    assert(!pager_fault_publish(&state, 2));
    assert(pager_fault_retire_run(&state, 15, 14) == PG_DENIED);
    assert(!state.retired && state.has_completed && state.phase == PG_PENDING);
    assert(pager_fault_retire_run(&state, 10, 15) == PG_DENIED);
    assert(!state.retired && state.has_completed && state.phase == PG_PENDING);
    assert(!pager_fault_retire_run(&state, 10, 14));
    assert(state.retired && !state.has_completed && state.phase == PG_IDLE);

    state = (PgFaultState){0};
    state.phase = PG_PENDING;
    state.active = record(1, 0);
    assert(pager_fault_retire_run(&state, 10, 14) == PG_DENIED);
    assert(!state.retired && state.phase == PG_PENDING);

    state = (PgFaultState){0};
    state.has_completed = true;
    state.completed = record(1, 0);
    assert(pager_fault_retire_run(&state, 10, 14) == PG_DENIED);
    assert(!state.retired && state.has_completed);

    PgLedger ledger = {0};
    PgBinding main_page = {1, 1010, 10, 14, 1, 0, 0, 6};
    PgBinding worker_page = {2, 0x40000002, 10, 14, 1, 1, 1, 6};
    assert(!pager_ledger_install(&ledger, main_page, 4));
    assert(!pager_ledger_install(&ledger, worker_page, 4));
    PgCleanupPlan plan;
    assert(pager_ledger_cleanup_plan(&ledger, 1, 10, 14, 1010, &plan) == PG_DENIED);
    assert(!pager_ledger_cleanup_run(&ledger, 1, 10, 14, &plan));
    assert(plan.entries[0].present && plan.entries[1].present);
    assert(pager_ledger_cleanup_run(&ledger, 1, 15, 14, &plan) == PG_DENIED);
    assert(!ledger.closed[0].present);
    assert(pager_ledger_cleanup_run(&ledger, 1, 10, 15, &plan) == PG_DENIED);
    assert(!ledger.closed[0].present);
    assert(pager_ledger_close_run(&ledger, 1, 10, 14, 0) == PG_DENIED);
    assert(!ledger.closed[0].present);
    assert(!pager_ledger_close_run(&ledger, 1, 10, 14, 1010));
    assert(ledger.closed[0].present && ledger.closed[0].task_id == 1010 && ledger.closed[0].frames == 3);

    ledger = (PgLedger){0};
    PgBinding empty_task = {3, 0, 10, 14, 1, 0, 0, 6};
    ledger.entries[0].present = true;
    ledger.entries[0].binding = empty_task;
    assert(pager_ledger_cleanup_run(&ledger, 1, 10, 14, &plan) == PG_DENIED);

    puts("run-wide fault retirement: worker and main records, exact target/pager and nonzero task controls passed");
    return 0;
}
