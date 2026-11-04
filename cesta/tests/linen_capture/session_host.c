#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "linen.c"

int main(void)
{
    const uint8_t name[1] = {'a'};
    Session session = {.next_id = UINT64_MAX - 1};
    uint64_t out = 991;
    assert(session_create(&session, 0, name, 1, 3, &out) == 0);
    assert(out == UINT64_MAX - 1 && session.next_id == UINT64_MAX);
    Session held = session; out = 991;
    for (unsigned i = 0; i < 3; ++i) {
        assert(session_create(&session, 0, name, 1, 3, &out) == ERR_ID_EXHAUSTED);
        assert(out == 991 && memcmp(&held, &session, sizeof held) == 0);
    }
    session.next_id = 0; held = session;
    assert(session_create(&session, 0, name, 1, 3, &out) == ERR_ID_EXHAUSTED);
    assert(out == 991 && memcmp(&held, &session, sizeof held) == 0);
    session = (Session){.next_id = UINT64_C(0x8000000100000017)};
    assert(session_create(&session, 0, name, 1, 3, &out) == 0);
    assert(out == UINT64_C(0x8000000100000017));
    puts("PASS: full-width Linen identities; exhaustion preserves table and output");
}
