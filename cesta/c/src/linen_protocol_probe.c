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
#include "silk_shell_models_linen.h"
#include "shell_linen_runtime.h"
#include "sexos_client.h"

static SilkShellModelsState probe_models;
static ShellLinenRuntime probe_runtime;
const LinenObject LINEN_SEED_OBJECTS[6] = {{0}};
static bool probe_painted;

void shell_models_peer_paint_fast(ShellLinenModelsContext *context) {
    ShellTextBytes text = {
        .len = 14, .bytes = {'L', 'I', 'N', 'E', 'N', ' ', 'C', 'A', 'P', 'T', 'U', 'R', 'E', 'D'}};
    ShellTextMetadata metadata = {context->linen->capture_snapshot.count, 0, 1,
                                  context->models->SELECTED_LINEN_OBJECT_ID};
    if (!shell_linen_text_queue(context->linen, &text, UINT32_C(0x00f0f8ff), metadata))
        sexos_fail();
    probe_painted = true;
}

void linen_protocol_probe_main(void) {
    SexosReply surface = sexos_pdx_call(SEXOS_SLOT_display, UINT64_C(0xEC), UINT64_C(200),
                                        (UINT64_C(500) << 32) | UINT64_C(900),
                                        (UINT64_C(150) << 32) | UINT64_C(300));
    if (surface.status)
        sexos_fail();
    probe_runtime = shell_models_linen_new();
    ShellLinenModelsContext context = {&probe_models, &probe_runtime};
    sexos_line("[linen.protocol.probe.begin] mode=correlated-required legacy=0");
    shell_models_fetch_remote_snapshot(&context);
    for (uint64_t turn = 0; turn < UINT64_C(2000000); ++turn) {
        shell_models_linen_async_poll(&context);
        if (probe_runtime.text.active || probe_runtime.text.latest)
            (void)shell_linen_text_poll(&probe_runtime, 1);
        SexosMessage message = sexos_listen_once(0);
        if (message.type_id == 1) {
            ShellTextReceipt receipt = {0};
            if (shell_linen_text_receive(&probe_runtime, message.caller_pd, message.caller_task,
                                         message.arg0, &receipt)) {
                sexos_line(
                    "[linen.protocol.probe.done] mode=correlated capture=1 published=1 display_ack=1 revision=%llu job=%llu bytes=%zu",
                    (unsigned long long)probe_runtime.presentation_last.revision,
                    (unsigned long long)probe_runtime.presentation_last.job, receipt.bytes);
                while (true)
                    sexos_yield();
            }
        }
        if ((turn & UINT64_C(255)) == 0)
            sexos_yield();
    }
    sexos_line("[linen.protocol.probe.failed] painted=%u capture_phase=%u presentation=%u",
               (unsigned)probe_painted, (unsigned)probe_runtime.capture.phase,
               (unsigned)probe_runtime.presentation_active.present);
    sexos_fail();
}


