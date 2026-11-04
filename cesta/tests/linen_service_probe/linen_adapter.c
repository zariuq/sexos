#include "linen.c"
#include "linen_adapter.h"
#include <assert.h>

static Linen server;

void probe_linen_init(void)
{
    server = (Linen){.session = {.next_id = UINT64_C(0x8000000100000001)}, .correlated = true};
    server.capture = linen_capture_new(3, 3);
    server.session.slots[0] = (LinenSlot){.present = true, .object = {
        .object_id = UINT64_C(0xfedcba9876543210), .kind = OBJECT_KIND_SESSION,
        .owner_pd = 3, .name_len = 24, .sexfiles_object_id = 15}};
    for (size_t i = 0; i < 24; ++i)
        server.session.slots[0].object.name[i] = (uint8_t)(240 + i);
}

void probe_linen_dispatch(SexosRequest *request) { linen_dispatch_request(&server, request); }
bool probe_linen_capture_active(void) { return server.capture.active; }
void probe_linen_mutate_live_table(void)
{
    assert(server.capture.active);
    server.session.slots[0].object.object_id = 77;
    server.session.slots[0].object.name[0] = 1;
}
