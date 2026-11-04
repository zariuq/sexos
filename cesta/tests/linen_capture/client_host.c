#include "linen_capture_client.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static uint64_t random_word(uint64_t *state)
{
    *state ^= *state << 13; *state ^= *state >> 7; *state ^= *state << 17;
    return *state;
}

static uint64_t dispatch(LinenCaptureServer *server, const LinenCaptureTable *table,
                         LinenCaptureAction action)
{
    switch (action.opcode) {
    case LINEN_CAPTURE_OP_BEGIN: return linen_capture_begin(server, 3, 3, action.job, action.arg1, action.arg2, table);
    case LINEN_CAPTURE_OP_READ: return linen_capture_read(server, 3, 3, action.job, action.arg1, action.arg2);
    case LINEN_CAPTURE_OP_END: return linen_capture_end(server, 3, 3, action.job, action.arg1, action.arg2);
    default: assert(!"unexpected client operation"); return 0;
    }
}

static void exercise(uint64_t seed)
{
    LinenCaptureTable live = {0}, captured = {0};
    LinenCaptureServer server = linen_capture_new(3, 3);
    LinenCaptureClient client = linen_capture_client_new();
    uint64_t transport = 0, published = 0, cancelled = 0;
    for (size_t round = 0; round < 20; ++round) {
        assert(linen_capture_client_refresh(&client));
        assert(client.job == round + 1);
        bool stopping = round % 4 == 0;
        bool cancellation_started = false;
        bool captured_once = false;
        uint64_t revision = 0;
        for (size_t turn = 0; linen_capture_client_busy(&client); ++turn) {
            assert(turn < 2000);
            LinenCaptureAction action = linen_capture_client_action(&client);
            assert(action.present && action.job == round + 1);
            uint64_t choose = random_word(&seed);
            if ((choose & 7) == 0) assert(linen_capture_client_refresh(&client));
            assert(client.job == round + 1);
            if (stopping && !cancellation_started && (choose & 15) == 0) {
                assert(linen_capture_client_cancel(&client));
                cancellation_started = true;
                if (!linen_capture_client_busy(&client)) {++cancelled; break;}
                continue;
            }
            assert(linen_capture_client_sent(&client, ++transport));
            assert(!linen_capture_client_sent(&client, transport));
            if ((choose & 15) == 1) {
                /* Cancellation before the server hears it: safe transport retry. */
                assert(linen_capture_client_retry(&client, transport));
                assert(!linen_capture_client_sent(&client, transport));
                assert(client.job == round + 1);
                continue;
            }
            uint64_t value = dispatch(&server, &live, action);
            if (action.opcode == LINEN_CAPTURE_OP_BEGIN && value == 0 && !captured_once) {
                captured = live; captured_once = true; revision = server.revision;
            }
            /* Live mutation after each request must not change this capture. */
            size_t slot = (size_t)(choose % LINEN_CAPTURE_SLOTS);
            live.rows[slot] = (LinenCaptureRow){.present = true, .id = (UINT64_C(1) << 63) + slot + 1,
                .kind = (uint8_t)choose, .disk = (uint8_t)(choose >> 8), .len = 24};
            for (size_t i = 0; i < 24; ++i) live.rows[slot].name[i] = (uint8_t)(choose >> (8 * (i % 8)));
            if ((choose & 15) == 2) {
                /* The server performed it, but transport completion was lost. */
                assert(linen_capture_client_retry(&client, transport));
                assert(client.job == round + 1);
                continue;
            }
            LinenCaptureClient before = client;
            assert(linen_capture_client_reply(&client, transport + 1, value) == LINEN_CLIENT_IGNORED);
            assert(memcmp(&client, &before, sizeof client) == 0);
            uint8_t result = linen_capture_client_reply(&client, transport, value);
            before = client;
            assert(linen_capture_client_reply(&client, transport, value) == LINEN_CLIENT_IGNORED);
            assert(memcmp(&client, &before, sizeof client) == 0);
            if (result == LINEN_CLIENT_PUBLISHED) {
                assert(!server.active && !client.may_capture && client.complete);
                LinenCaptureFrame expected = {0}, actual = {0};
                assert(linen_capture_encode(&captured, revision, &expected));
                assert(linen_capture_encode(&client.table, revision, &actual));
                assert(expected.len == actual.len && !memcmp(expected.bytes, actual.bytes, expected.len));
                ++published;
            } else if (result == LINEN_CLIENT_CANCELLED) {
                assert(!server.active && !client.may_capture); ++cancelled;
            } else assert(result == LINEN_CLIENT_PROGRESS);
        }
        assert(!server.active);
        assert(published + cancelled == round + 1);
        assert(server.retired <= client.job);
    }
}

int main(void)
{
    for (uint64_t seed = 1; seed <= 512; ++seed) exercise(seed);
    LinenCaptureClient client = linen_capture_client_new();
    client.next_job = UINT64_MAX;
    assert(!linen_capture_client_refresh(&client) && client.exhausted && client.next_job == UINT64_MAX);
    client = linen_capture_client_new(); assert(linen_capture_client_refresh(&client));
    assert(linen_capture_client_sent(&client, 1));
    assert(!linen_capture_client_cancel(&client));
    assert(linen_capture_client_retry(&client, 1));
    assert(linen_capture_client_cancel(&client));
    assert(client.phase == LINEN_CLIENT_END && client.job == 1);
    puts("PASS: 10240 refresh/cancellation jobs with live mutation, transport loss, retries and stale replies");
}
