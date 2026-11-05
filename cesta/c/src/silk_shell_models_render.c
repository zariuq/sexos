#include "silk_shell_models_render.h"
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

static void render_invalid(void) {
    sexos_fail();
    for (;;) {
    }
}

static void render_queue(ShellLinenRenderContext *ctx, const ShellTextBytes *text, uint32_t color,
                         ShellTextMetadata metadata) {
    if (!shell_linen_text_queue(ctx->linen, text, color, metadata))
        render_invalid();
}

void shell_models_render_object_list(ShellLinenRenderContext *ctx) {
    shell_chrome_set_frame_top_bar(&ctx->shell, LINEN_FRAME_ID, true);
    shell_chrome_send_frame_tab_info(&ctx->shell, LINEN_FRAME_ID);
    ShellLinenItems items = shell_models_peer_list_items(ctx);
    uint64_t selected = ctx->shell.models->SELECTED_LINEN_OBJECT_ID;
    uint32_t w = ctx->shell.input->SURFACE_200_W, h = ctx->shell.input->SURFACE_200_H;
    ShellLinenList view = shell_linen_list(&items, selected, w, h);
    for (uint64_t index = 0; index < 8; ++index)
        (void)sexos_pdx_call(SLOT_DISPLAY, 0xef, SURFACE_ID_LINEN, 0, index << 56);
    uint64_t header_h = view.row_y >= 28 ? view.row_y - 28 : 0;
    (void)sexos_pdx_call(SLOT_DISPLAY, 0xef, SURFACE_ID_LINEN, UINT64_C(28) << 32,
                         (UINT64_C(0x0022384a) << 32) | (header_h << 16) | w);
    (void)sexos_pdx_call(SLOT_DISPLAY, 0xef, SURFACE_ID_LINEN, (uint64_t)view.row_y << 32,
                         (UINT64_C(1) << 56) | (UINT64_C(0x000c1420) << 32) |
                             ((uint64_t)((uint32_t)view.rows * view.row_h) << 16) | w);
    if (view.has_selected) {
        uint32_t y = view.row_y + (uint32_t)view.selected_row * view.row_h;
        (void)sexos_pdx_call(SLOT_DISPLAY, 0xef, SURFACE_ID_LINEN, (uint64_t)y << 32,
                             (UINT64_C(2) << 56) | (UINT64_C(0x002a4960) << 32) |
                                 ((uint64_t)view.row_h << 16) | w);
    }
    for (size_t row = 0; row < view.rows; ++row) {
        uint32_t y = view.row_y + (uint32_t)row * view.row_h;
        (void)sexos_pdx_call(SLOT_DISPLAY, 0xef, SURFACE_ID_LINEN, (uint64_t)y << 32,
                             ((uint64_t)(row + 3) << 56) | (UINT64_C(0x005ed9c3) << 32) |
                                 ((uint64_t)view.row_h << 16) | 3);
    }
    render_queue(ctx, &view.text, 0x00f0f8ff,
                 (ShellTextMetadata){items.count, view.first, view.rows, selected});
    sexos_line(
        "[linen.list.text] x=%d y=%d w=%u h=%u scale=%u text_color=0xf0f8ff rows=%zu selected=%llu row_y=%u row_h=%u first=%zu committed_view=1",
        ctx->shell.input->SURFACE_200_X, ctx->shell.input->SURFACE_200_Y, w, h, w >= 256 ? 2u : 1u,
        view.rows, (unsigned long long)selected, view.row_y, view.row_h, view.first);
    for (size_t row = 0; row < view.rows; ++row) {
        size_t wanted = 2 + row * 2, start = 0, line = 0;
        while (start < view.text.len && line < wanted) {
            if (view.text.bytes[start] == 10)
                ++line;
            ++start;
        }
        size_t end = start;
        while (end < view.text.len && view.text.bytes[end] != 10)
            ++end;
        char label[80] = {0};
        size_t length = end >= start + 2 ? end - start - 2 : 0;
        if (length >= 80)
            render_invalid();
        for (size_t i = 0; i < length; ++i)
            label[i] = (char)view.text.bytes[start + 2 + i];
        sexos_line("[linen.list.label] id=%llu row=%zu text=%s",
                   (unsigned long long)items.items[view.first + row].id, row, label);
    }
    sexos_line("[linen.object_select.current] id=%llu", (unsigned long long)selected);
    sexos_line("[linen.object_list.done] count=%zu rows=%zu", items.count, view.rows);
}

void shell_models_render_static_ui(ShellLinenRenderContext *ctx) {
    uint32_t width = ctx->shell.input->SURFACE_200_W;
    uint32_t height = ctx->shell.input->SURFACE_200_H;
    if (width == 0 || height == 0)
        return;
    size_t selected = ctx->shell.models->LINEN_UI_SELECTED;
    if (selected >= LINEN_UI_ROW_COUNT)
        selected = LINEN_UI_ROW_COUNT - 1;
    uint32_t header = LINEN_UI_ROW_COLORS[selected];
    (void)sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_LINEN, 0,
                         (uint64_t)header << 32 | (uint64_t)LINEN_LIST_HEADER_H << 16 | width);
    uint32_t background_height =
        (uint32_t)LINEN_LIST_ACCENT_BARS * (LINEN_LIST_ROW_H + LINEN_LIST_ROW_GAP) -
        LINEN_LIST_ROW_GAP;
    (void)sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_LINEN, (uint64_t)LINEN_LIST_HEADER_H << 32,
                         UINT64_C(1) << 56 | (uint64_t)LINEN_LIST_BG_COLOR << 32 |
                             (uint64_t)background_height << 16 | width);
    uint32_t selected_y =
        LINEN_LIST_HEADER_H + (uint32_t)selected * (LINEN_LIST_ROW_H + LINEN_LIST_ROW_GAP);
    (void)sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_LINEN, (uint64_t)selected_y << 32,
                         UINT64_C(2) << 56 | (uint64_t)header << 32 |
                             (uint64_t)LINEN_LIST_ROW_H << 16 | width);
    const uint8_t labels[5][8] = {{80, 82, 79, 74, 69, 67, 84, 83},
                                  {83, 69, 88, 32, 77, 73, 67, 82},
                                  {72, 65, 78, 68, 79, 85, 84, 83},
                                  {72, 65, 78, 68, 79, 70, 70, 83},
                                  {81, 85, 73, 76, 32, 68, 70, 84}};
    for (size_t i = 0; i < LINEN_UI_ROW_COUNT; ++i) {
        uint32_t y = LINEN_LIST_HEADER_H + (uint32_t)i * (LINEN_LIST_ROW_H + LINEN_LIST_ROW_GAP);
        uint64_t rectangle = ((uint64_t)i + 3) & 7;
        uint32_t color = i == selected ? header : LINEN_UI_ROW_COLORS[i];
        (void)sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_LINEN, (uint64_t)y << 32,
                             rectangle << 56 | (uint64_t)color << 32 |
                                 (uint64_t)LINEN_LIST_ROW_H << 16 | LINEN_ACCENT_BAR_W);
    }
    ShellTextBytes text = {0};
    text.len = LINEN_UI_ROW_COUNT * 20 + 8;
    for (size_t i = 0; i < text.len; ++i)
        text.bytes[i] = 32;
    text.bytes[0] = 76;
    text.bytes[1] = 73;
    text.bytes[2] = 78;
    text.bytes[3] = 69;
    text.bytes[4] = 78;
    for (size_t i = 0; i < LINEN_UI_ROW_COUNT; ++i)
        for (size_t j = 0; j < 8; ++j)
            text.bytes[(i + 1) * 20 + j] = labels[i][j];
    render_queue(ctx, &text, 0x00b8cce0, (ShellTextMetadata){0, 0, LINEN_UI_ROW_COUNT, selected});
    sexos_line("[linen.ui.render] rows=%zu selected=%zu", LINEN_UI_ROW_COUNT, selected);
}


