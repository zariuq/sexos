#include <stdint.h>
#include <stdio.h>
#include "sexos_document_growth.h"

#define MUST(condition) do { if (!(condition)) __builtin_trap(); } while (0)

static void finish(DocumentGrowthPlan *plan, DocumentGrowthAction action) {
    MUST(document_growth_current(plan) == action);
    MUST(document_growth_begin_action(plan, action));
    MUST(document_growth_finish_action(plan, action));
}

static bool initialize(DocumentGrowthPlan *plan, size_t original, size_t count,
                       const uint16_t sectors[DOCUMENT_GROWTH_MAX_EXTENTS]) {
    if (!document_growth_init(plan, original, count)) return false;
    for (size_t i = 0; i < count; i++)
        if (!document_growth_set_extent(plan, i, sectors[i])) return false;
    return true;
}

int main(void) {
    uint16_t sectors[DOCUMENT_GROWTH_MAX_EXTENTS] = {8, 16, 8};
    DocumentGrowthPlan plan;
    MUST(initialize(&plan, 1, 3, sectors));
    MUST(document_growth_cancel_releases(&plan));
    finish(&plan, DOCUMENT_GROWTH_ZERO);
    MUST(plan.extent == 1 && plan.sector == 8);
    finish(&plan, DOCUMENT_GROWTH_ZERO);
    MUST(plan.extent == 2 && plan.sector == 0);
    finish(&plan, DOCUMENT_GROWTH_ZERO);
    MUST(plan.extent == 3 && plan.sector == 0);
    finish(&plan, DOCUMENT_GROWTH_ZERO_BARRIER);
    MUST(document_growth_current(&plan) == DOCUMENT_GROWTH_DESCRIPTOR);
    MUST(document_growth_begin_action(&plan, DOCUMENT_GROWTH_DESCRIPTOR));
    MUST(plan.descriptor_attempted && !document_growth_cancel_releases(&plan));
    MUST(!document_growth_begin_action(&plan, DOCUMENT_GROWTH_DESCRIPTOR));
    MUST(!document_growth_finish_action(&plan, DOCUMENT_GROWTH_PUBLISH));
    MUST(document_growth_finish_action(&plan, DOCUMENT_GROWTH_DESCRIPTOR));
    finish(&plan, DOCUMENT_GROWTH_DESCRIPTOR_BARRIER);
    finish(&plan, DOCUMENT_GROWTH_PUBLISH);
    finish(&plan, DOCUMENT_GROWTH_PUBLISH_BARRIER);
    MUST(document_growth_current(&plan) == DOCUMENT_GROWTH_DONE);
    MUST(!plan.active && !document_growth_cancel_releases(&plan));

    MUST(initialize(&plan, 3, 3, sectors));
    MUST(document_growth_current(&plan) == DOCUMENT_GROWTH_ZERO_BARRIER);
    MUST(!document_growth_begin_action(&plan, DOCUMENT_GROWTH_DESCRIPTOR));
    document_growth_abort(&plan);
    MUST(document_growth_current(&plan) == DOCUMENT_GROWTH_DONE);

    MUST(!initialize(&plan, 4, 3, sectors));
    MUST(!initialize(&plan, 0, DOCUMENT_GROWTH_MAX_EXTENTS + 1, sectors));
    sectors[1] = 0; MUST(!initialize(&plan, 0, 3, sectors));
    sectors[1] = 7; MUST(!initialize(&plan, 0, 3, sectors));
    sectors[1] = 16; MUST(initialize(&plan, 1, 3, sectors));
    plan.sector = 1; MUST(document_growth_current(&plan) == DOCUMENT_GROWTH_INVALID);
    plan.sector = 0; plan.phase = 99;
    MUST(document_growth_current(&plan) == DOCUMENT_GROWTH_INVALID);
    puts("document growth controls passed");
    return 0;
}
