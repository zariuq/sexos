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

#include "silk_shell_core.h"
#include "silk_shell_models_root.h"
#include "silk_shell_surfaces.h"
#include "silk_shell_atlas.h"
#include "sexos_server.h"

static ShellInputContext core_main_input(void) {
    return (ShellInputContext){.shell = {.input = &SILK_SHELL_STATE.input,
                                         .chrome = &SILK_SHELL_STATE.chrome,
                                         .models = &SILK_SHELL_STATE.models,
                                         .apps = &SILK_SHELL_STATE.apps,
                                         .proofs_a = &SILK_SHELL_STATE.proofs_a},
                               .atlas = &SILK_SHELL_STATE.atlas,
                               .surfaces = &SILK_SHELL_STATE.surfaces};
}

bool startup_messages_pending(void) {
    return shell_startup_pending(&SILK_SHELL_STATE.core);
}

static bool core_display_call(uint64_t opcode, uint64_t a, uint64_t b, uint64_t c) {
    for (size_t attempt = 0; attempt < 64; ++attempt) {
        SexosReply reply = sexos_pdx_call(SLOT_DISPLAY, opcode, a, b, c);
        if (reply.status != SEXOS_ERR_SERVICE_NOT_READY)
            return reply.status == 0;
        sexos_yield();
    }
    return false;
}

static void core_wait_display(void) {
    for (;;) {
        if (SILK_SHELL_STATE.core.startup_ready)
            return;
        if (SILK_SHELL_STATE.core.startup_len == SHELL_STARTUP_INBOX_CAPACITY) {
            sexos_line("[silk-shell.startup.reject] reason=inbox_full");
            sexos_spin_forever();
        }
        SexosMessage message = {0};
        if (!sexos_try_listen(0, &message)) {
            sexos_yield();
            continue;
        }
        ShellStartupObserve observed = shell_startup_observe(&SILK_SHELL_STATE.core, message);
        if (observed == SHELL_STARTUP_REJECTED_CONTROL)
            sexos_line("[display.ready.rejected] caller=%u version=%llu", message.caller_pd,
                       (unsigned long long)message.arg0);
    }
}

static void core_components_init(void) {
    ShellInputContext input = core_main_input();
    ShellSurfacesContext surfaces = {input.shell, input.surfaces};
    ShellAtlasContext atlas = {input.atlas, input.shell.chrome, input.shell.models};
    uint32_t frame = 0;
    (void)shell_surfaces_ensure_quil_frame(&surfaces, &frame);
    (void)shell_surfaces_ensure_linen_frame(&surfaces, &frame);
    shell_chrome_lifecycle_init_all(&input.shell);
    shell_atlas_scene_init_all(&atlas);
    linen_object_table_init();
    quil_buffer_table_init();
    linen_quil_seed_coherence_init();
    collar_init_grants();
    shell_atlas_snap_capture_layout(&atlas);
    app_surface_registry_validate();
    silk_shell_models_runtime_init();
}

static bool core_boot_ui(void) {
    if (!core_display_call(OP_REGISTER_WM, 0, 0, 0)) {
        return false;
    }
    int32_t width = shell_chrome_desktop_width(&(ShellChromeContext){
        &SILK_SHELL_STATE.chrome, &SILK_SHELL_STATE.models, &SILK_SHELL_STATE.apps,
        &SILK_SHELL_STATE.input, &SILK_SHELL_STATE.proofs_a});
    int32_t height = shell_chrome_desktop_height(&(ShellChromeContext){
        &SILK_SHELL_STATE.chrome, &SILK_SHELL_STATE.models, &SILK_SHELL_STATE.apps,
        &SILK_SHELL_STATE.input, &SILK_SHELL_STATE.proofs_a});
    int32_t y = P.bar_height > 0 ? P.bar_height : 0;
    uint32_t h = height > y ? (uint32_t)(height - y) : 0, w = width > 0 ? (uint32_t)width : 0;
    if (!w || !h) {
        return false;
    }
    uint32_t gutter = 16, main_w = w * 72 / 100,
             side_w = w - main_w > gutter ? w - main_w - gutter : 0;
    if (side_w < P.min_width) {
        side_w = (uint32_t)P.min_width;
    }
    if (main_w + side_w + gutter > w) {
        main_w = w > side_w + gutter ? w - side_w - gutter : 0;
    }
    if (!main_w) {
        return false;
    }
    int32_t linen_x = (int32_t)(main_w + gutter);
    SILK_SHELL_STATE.input.SURFACE_201_X = 0;
    SILK_SHELL_STATE.input.SURFACE_201_Y = y;
    SILK_SHELL_STATE.input.SURFACE_201_W = main_w;
    SILK_SHELL_STATE.input.SURFACE_201_H = h;
    SILK_SHELL_STATE.input.SURFACE_200_X = linen_x;
    SILK_SHELL_STATE.input.SURFACE_200_Y = y;
    SILK_SHELL_STATE.input.SURFACE_200_W = side_w;
    SILK_SHELL_STATE.input.SURFACE_200_H = h;
    if (!core_display_call(UINT64_C(0xec), SURFACE_ID_QUIL, ((uint64_t)(uint32_t)y << 32),
                           ((uint64_t)h << 32) | main_w)) {
        return false;
    }
    if (!core_display_call(UINT64_C(0xec), SURFACE_ID_LINEN,
                           ((uint64_t)(uint32_t)y << 32) | (uint32_t)linen_x,
                           ((uint64_t)h << 32) | side_w)) {
        return false;
    }
    if (!core_display_call(UINT64_C(0xed), SURFACE_ID_QUIL, 0, 0)) {
        return false;
    }
    ShellInputContext input = core_main_input();
    (void)shell_chrome_try_set_focus(&input.shell, SURFACE_ID_QUIL);
    sexos_line("[silk-shell.boot.ui.ready] surfaces=2 focus=%llu",
               (unsigned long long)SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID);
    return true;
}

void silk_shell_main(void) {
    silk_shell_state_init(&SILK_SHELL_STATE);
    sexos_line("[silkshell.init.start]");
    core_wait_display();
    sexos_line("[display.ready.accepted] client=silk-shell caller=1 version=1");
    sexos_heap_init();
    core_components_init();
    (void)sexos_set_state(SVC_STATE_LISTENING);
    if (!core_boot_ui()) {
        sexos_line("[silk-shell.boot.reject] reason=display_init");
        sexos_spin_forever();
    }
    boot_load_scene_settings();
    sexos_line("[silk-shell.ready]");
    for (;;) {
        shell_record_spawn_poll();
        workspace_refresh_poll();
        linen_async_poll();
        if (!startup_messages_pending())
            silk_shell_proofs_tick();
        SexosMessage message = {0};
        if (!shell_startup_pop(&SILK_SHELL_STATE.core, &message) &&
            !sexos_try_listen(0, &message)) {
            sexos_yield();
            continue;
        }
        SilkShellDispatchResult result = silk_shell_dispatch(&SILK_SHELL_STATE, &message);
        if (result.reply)
            (void)sexos_reply(message.caller_pd, result.value);
        if (!result.handled)
            sexos_line("[pdx.opcode.unknown] shell type_id=0x%llx caller=%u",
                       (unsigned long long)message.type_id, message.caller_pd);
        if (result.mutated) {
            ShellInputContext input = core_main_input();
            shell_input_emit_snapshot(&input);
        }
    }
}


