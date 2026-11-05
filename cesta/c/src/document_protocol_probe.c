#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif
#include "sexos_document_client_transport.h"
#include "sexos_client.h"

#ifndef DOCUMENT_PROTOCOL_PROBE_MODE
#define DOCUMENT_PROTOCOL_PROBE_MODE 0
#endif
static DocumentClient probe_client;
static DocumentClientTransport probe_transport;
static uint8_t probe_phase;
#if DOCUMENT_PROTOCOL_PROBE_MODE == 0
static uint8_t probe_object;
#endif

#if DOCUMENT_PROTOCOL_PROBE_MODE != 1
static void probe_draw(void) {
    SexosReply surface = sexos_pdx_call(SEXOS_SLOT_display, UINT64_C(0xEC), UINT64_C(156),
                                        (UINT64_C(56) << 32) | UINT64_C(1072),
                                        (UINT64_C(304) << 32) | UINT64_C(200));
    if (surface.status)
        sexos_fail();
    (void)sexos_pdx_call(SEXOS_SLOT_display, UINT64_C(0xFA), UINT64_C(156), 0, 0);
    const uint8_t text[8] = {'D', 'O', 'C', ' ', 'O', 'K', '!', ' '};
    uint64_t packed = 0;
    for (size_t i = 0; i < 8; i++)
        packed |= (uint64_t)text[i] << (i * 8);
    SexosReply draw = sexos_pdx_call(SEXOS_SLOT_display, UINT64_C(0xFB), UINT64_C(156), packed,
                                     (UINT64_C(8) << 8) | UINT64_C(0x00e8ffff) << 32);
    if (draw.status)
        sexos_fail();
}
#endif
static void probe_event(DocumentClientEvent event) {
#if DOCUMENT_PROTOCOL_PROBE_MODE != 1
    static const uint8_t bytes[13] = {'C', 'E', 'S', 'T', 'A', ' ', 'D',
                                      'O', 'C', 'U', 'M', 'E', 'N'};
#endif
    sexos_line(
        "[document.protocol.probe.event] phase=%u outcome=%u object=%u error=%u revision=%llu",
        (unsigned)probe_phase, (unsigned)event.outcome, (unsigned)event.object,
        (unsigned)event.error, (unsigned long long)event.revision);
#if DOCUMENT_PROTOCOL_PROBE_MODE == 1
    if (probe_phase == 0 && event.outcome == DOCUMENT_OUTCOME_SAVED && event.object == 3) {
        sexos_line("[document.protocol.probe.persisted] mode=save object=3 bytes=13 barriers=6");
        while (true)
            sexos_yield();
    }
#elif DOCUMENT_PROTOCOL_PROBE_MODE == 2
    if (probe_phase == 0) {
        if (event.outcome != DOCUMENT_OUTCOME_LOADED || event.object != 3 ||
            probe_client.payload.len != sizeof bytes)
            sexos_fail();
        for (size_t i = 0; i < sizeof bytes; i++)
            if (probe_client.payload.bytes[i] != bytes[i])
                sexos_fail();
        probe_draw();
        probe_phase = 1;
        if (!document_client_delete(&probe_client, 3, 2))
            sexos_fail();
        return;
    }
    if (probe_phase == 1 && event.outcome == DOCUMENT_OUTCOME_DELETED && event.object == 3) {
        sexos_line(
            "[document.protocol.probe.recovered] mode=reboot object=3 bytes=13 exact=1 deleted=1");
        while (true)
            sexos_yield();
    }
#else
    if (probe_phase == 0) {
        if (event.outcome != DOCUMENT_OUTCOME_SAVED || event.object < 3 || event.object >= 15)
            sexos_fail();
        probe_object = event.object;
        probe_phase = 1;
        if (!document_client_load(&probe_client, probe_object, 2))
            sexos_fail();
        return;
    }
    if (probe_phase == 1) {
        if (event.outcome != DOCUMENT_OUTCOME_LOADED || event.object != probe_object ||
            probe_client.payload.len != sizeof bytes)
            sexos_fail();
        for (size_t i = 0; i < sizeof bytes; i++)
            if (probe_client.payload.bytes[i] != bytes[i])
                sexos_fail();
        probe_draw();
        probe_phase = 2;
        if (!document_client_delete(&probe_client, probe_object, 2))
            sexos_fail();
        return;
    }
    if (probe_phase == 2) {
        if (event.outcome != DOCUMENT_OUTCOME_DELETED || event.object != probe_object)
            sexos_fail();
        sexos_line(
            "[document.protocol.probe.done] mode=correlated journal=1 snapshot=1 display=1 delete=1 object=%u",
            (unsigned)probe_object);
        while (true)
            sexos_yield();
    }
#endif
    sexos_fail();
}

void document_protocol_probe_main(void) {
    document_client_init(&probe_client);
    document_client_transport_init(&probe_transport);
    MemoryRegionReply info = sexos_memory_region_info(64);
    MemoryRegionReply view =
        sexos_memory_region_view(64, 0, DOCUMENT_SNAPSHOT_BYTES, REGION_QUERY | REGION_READ);
    sexos_line(
        "[document.protocol.probe.region] info=%llu length=%llu rights=%llu peer=%llu region=%llu view=%llu address=%#llx view_length=%llu dma=%llu view_region=%llu",
        (unsigned long long)info.status, (unsigned long long)info.value0,
        (unsigned long long)info.value1, (unsigned long long)info.value2,
        (unsigned long long)info.value3, (unsigned long long)view.status,
        (unsigned long long)view.value0, (unsigned long long)view.value1,
        (unsigned long long)view.value2, (unsigned long long)view.value3);
    DocumentSnapshotRegion snapshot;
    if (!document_snapshot_resolve_client(&snapshot) ||
        !document_client_set_snapshot(&probe_client, snapshot))
        sexos_fail();
    static const uint8_t name[16] = {'c', 'e', 's', 't', 'a', '-', 'p', 'r',
                                     'o', 'b', 'e', '.', 't', 'x', 't', 0};
    static const uint8_t bytes[13] = {'C', 'E', 'S', 'T', 'A', ' ', 'D',
                                      'O', 'C', 'U', 'M', 'E', 'N'};
    sexos_line("[document.protocol.probe.begin] mode=correlated-required legacy=0 reboot_mode=%u",
               (unsigned)DOCUMENT_PROTOCOL_PROBE_MODE);
#if DOCUMENT_PROTOCOL_PROBE_MODE == 2
    (void)name;
    (void)bytes;
    if (!document_client_load(&probe_client, 3, 1))
        sexos_fail();
#else
    if (!document_client_save(&probe_client, false, 0, name, bytes, sizeof bytes, 1))
        sexos_fail();
#endif
    for (uint64_t turn = 0; turn < UINT64_C(4000000); turn++) {
        DocumentClientEvent event = {0};
        (void)document_client_transport_step(&probe_client, &probe_transport, &event);
        if (event.present)
            probe_event(event);
        SexosMessage message = sexos_listen_once(0);
        if (message.type_id && message.type_id != 1)
            sexos_line("[document.protocol.probe.message] type=%#llx caller=%u",
                       (unsigned long long)message.type_id, (unsigned)message.caller_pd);
        if (document_client_transport_pending(&probe_transport))
            sexos_wait_self();
        else
            sexos_yield();
    }
    sexos_line("[document.protocol.probe.failed] phase=%u busy=%u", (unsigned)probe_phase,
               (unsigned)document_client_busy(&probe_client));
    sexos_fail();
}


