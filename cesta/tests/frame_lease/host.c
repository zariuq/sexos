#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include "frame_lease.h"

static FrameLeaseState state;

static void dump(void)
{
    printf("A %d %zu %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu32 "\n",
           frame_lease_audit(&state), state.count, state.free_pages, state.lent,
           state.reserve, state.next_incarnation);
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i) {
        FrameLeaseEntry e = state.entries[i];
        if (e.phase == FRAME_LEASE_EMPTY) continue;
        printf("E %" PRIu64 " %" PRIu64 " %u %u %u %" PRIu32 " %" PRIu32 "\n",
               e.base, e.pages, e.order, e.phase, e.kind, e.holder, e.incarnation);
    }
}

static int value(uint64_t *out)
{
    return scanf(" %" SCNu64, out) == 1;
}

int main(void)
{
    char op = 0;
    frame_lease_init(&state, 0);
    while (scanf(" %c", &op) == 1) {
        uint64_t status = FRAME_LEASE_OK, base = 0, pages = 0, reserve = 0;
        unsigned order = 0, kind = 0;
        uint32_t holder = 0, incarnation = 0;
        FrameLease lease = {0};
        if (op == 'I') {
            if (!value(&reserve)) return 2;
            frame_lease_init(&state, reserve);
        } else if (op == 'R') {
            if (!value(&base) || !value(&pages)) return 2;
            status = frame_lease_region(&state, base, pages);
        } else if (op == 'T') {
            if (!value(&base) || !value(&pages) || !value(&reserve)) return 2;
            order = (unsigned)base;
            kind = (unsigned)pages;
            holder = (uint32_t)reserve;
            FrameLeaseResult result = frame_lease_take(&state, (uint8_t)order,
                                                       (uint8_t)kind, holder);
            status = result.status;
            lease = result.value;
        } else if (op == 'D' || op == 'G' || op == 'C') {
            if (!value(&base) || !value(&pages) || !value(&reserve) ||
                scanf(" %" SCNu32 " %" SCNu32, &holder, &incarnation) != 2) return 2;
            lease = (FrameLease){base, holder, incarnation, (uint8_t)pages, (uint8_t)reserve};
            status = op == 'D' ? frame_lease_detach(&state, lease) :
                     op == 'G' ? frame_lease_give(&state, lease) : frame_lease_commit(&state, lease);
        } else if (op == 'X') {
            if (!value(&reserve)) return 2;
            state.next_incarnation = (uint32_t)reserve;
        } else if (op == 'V') {
            status = frame_lease_available(&state);
        } else {
            return 2;
        }
        if (op == 'T')
            printf("T %" PRIu64 " %" PRIu64 " %u %u %" PRIu32 " %" PRIu32 "\n",
                   status, lease.base, lease.order, lease.kind, lease.holder, lease.incarnation);
        else
            printf("%c %" PRIu64 "\n", op, status);
        dump();
    }
    return 0;
}
