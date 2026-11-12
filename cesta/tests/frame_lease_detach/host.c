#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frame_lease.h"

_Static_assert(sizeof(FrameLease) == 24, "lease ABI");
_Static_assert(_Alignof(FrameLease) == 8, "lease alignment");
_Static_assert(offsetof(FrameLease, base) == 0, "lease base");
_Static_assert(offsetof(FrameLease, holder) == 8, "lease holder");
_Static_assert(offsetof(FrameLease, incarnation) == 12, "lease incarnation");
_Static_assert(offsetof(FrameLease, order) == 16, "lease order");
_Static_assert(offsetof(FrameLease, kind) == 17, "lease kind");
_Static_assert(sizeof(FrameLeaseEntry) == 32, "entry ABI");
_Static_assert(_Alignof(FrameLeaseEntry) == 8, "entry alignment");
_Static_assert(offsetof(FrameLeaseEntry, base) == 0, "entry base");
_Static_assert(offsetof(FrameLeaseEntry, pages) == 8, "entry pages");
_Static_assert(offsetof(FrameLeaseEntry, holder) == 16, "entry holder");
_Static_assert(offsetof(FrameLeaseEntry, incarnation) == 20, "entry incarnation");
_Static_assert(offsetof(FrameLeaseEntry, order) == 24, "entry order");
_Static_assert(offsetof(FrameLeaseEntry, phase) == 25, "entry phase");
_Static_assert(offsetof(FrameLeaseEntry, kind) == 26, "entry kind");
_Static_assert(offsetof(FrameLeaseState, count) == 131072, "table extent");

#ifdef DETACH_NATIVE
#define detach_under_test frame_lease_detach
#else
extern uint64_t detach_under_test(FrameLeaseState *, FrameLease);
#endif

typedef struct {
    uint64_t left[4];
    FrameLeaseState state;
    uint64_t right[4];
} Guarded;

static Guarded current, expected, baseline;
static uint64_t calls, random_state = UINT64_C(0xa0761d6478bd642f);

static uint64_t random_word(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 7;
    random_state ^= random_state << 17;
    return random_state;
}

static FrameLease request(void) {
    FrameLease result;
    memset(&result, 0, sizeof result);
    result.base = UINT64_C(0x12345000);
    result.holder = UINT32_C(0xf1234567);
    result.incarnation = UINT32_C(0xefabcdef);
    result.kind = FRAME_LEASE_PROGRAM;
    result.order = 5;
    return result;
}

static void install(size_t index, FrameLease lease) {
    FrameLeaseEntry *entry = &current.state.entries[index];
    entry->base = lease.base;
    entry->holder = lease.holder;
    entry->incarnation = lease.incarnation;
    entry->order = lease.order;
    entry->kind = lease.kind;
    entry->phase = FRAME_LEASE_LIVE;
}

static void check(FrameLease lease, uint64_t status, size_t selected) {
    memcpy(&expected, &current, sizeof current);
    if (status == FRAME_LEASE_OK)
        expected.state.entries[selected].phase = FRAME_LEASE_DETACHED;
    uint64_t observed = detach_under_test(&current.state, lease);
    if (observed != status || memcmp(&current, &expected, sizeof current)) {
        fprintf(stderr, "detach mismatch: case=%" PRIu64 " selected=%zu status=%" PRIu64
                        " expected=%" PRIu64 "\n", calls, selected, observed, status);
        exit(1);
    }
    ++calls;
}

int main(void) {
    memset(&baseline, 0xa5, sizeof baseline);
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i)
        baseline.state.entries[i].phase = FRAME_LEASE_EMPTY;
    FrameLease original = request();
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i) {
        memcpy(&current, &baseline, sizeof current);
        FrameLease lease = original;
        lease.kind = i & 1 ? FRAME_LEASE_TRANSIENT : FRAME_LEASE_PROGRAM;
        install(i, lease);
        check(lease, FRAME_LEASE_OK, i);
        check(lease, FRAME_LEASE_DENIED, 0);
    }
    for (unsigned field = 0; field < 6; ++field) {
        memcpy(&current, &baseline, sizeof current);
        install(4095, original);
        FrameLease wrong = original;
        switch (field) {
        case 0: wrong.base += 4096; break;
        case 1: ++wrong.holder; break;
        case 2: ++wrong.incarnation; break;
        case 3: ++wrong.order; break;
        case 4: wrong.kind = FRAME_LEASE_TRANSIENT; break;
        default: current.state.entries[4095].phase = FRAME_LEASE_DETACHED; break;
        }
        check(wrong, FRAME_LEASE_DENIED, 0);
    }
    for (unsigned invalid = 0; invalid < 9; ++invalid) {
        memcpy(&current, &baseline, sizeof current);
        FrameLease bad = original;
        switch (invalid) {
        case 0: bad.base = 0; break;
        case 1: ++bad.base; break;
        case 2: bad.holder = 0; break;
        case 3: bad.incarnation = 0; break;
        case 4: bad.order = 19; break;
        case 5: bad.order = 255; break;
        case 6: bad.kind = FRAME_LEASE_PINNED; break;
        case 7: bad.kind = 3; break;
        default: bad.kind = 255; break;
        }
        install(0, bad);
        check(bad, FRAME_LEASE_INVALID, 0);
        if (detach_under_test(NULL, bad) != FRAME_LEASE_INVALID) exit(1);
        ++calls;
    }
    for (unsigned trial = 0; trial < 1000; ++trial) {
        memcpy(&current, &baseline, sizeof current);
        FrameLease lease = original;
        lease.base = random_word() & ~UINT64_C(4095);
        if (!lease.base) lease.base = 4096;
        lease.holder = (uint32_t)random_word() | 1;
        lease.incarnation = (uint32_t)random_word() | 1;
        lease.order = (uint8_t)(random_word() % 19);
        lease.kind = (uint8_t)(1 + (random_word() & 1));
        size_t first = FRAME_LEASE_CAPACITY;
        for (unsigned duplicate = 0; duplicate < 20; ++duplicate) {
            size_t at = (size_t)(random_word() % FRAME_LEASE_CAPACITY);
            install(at, lease);
            if (at < first) first = at;
        }
        check(lease, FRAME_LEASE_OK, first);
    }
    memcpy(&current, &baseline, sizeof current);
    check(original, FRAME_LEASE_DENIED, 0);
    printf("detach: %" PRIu64 " cases; exact whole-state and guard-byte preservation\n", calls);
    return 0;
}
