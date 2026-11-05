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

#include "silk_shell_diagnostics_a.h"

bool shell_diagnostics_is_spindle_text_key(uint8_t sc) {
    return sc == 0x1c || sc == 0x0e || sc == 1 || sc == 0x0f || sc == 0x39 ||
           (sc >= 2 && sc <= 0x0b) || (sc >= 0x10 && sc <= 0x19) || (sc >= 0x1e && sc <= 0x26) ||
           (sc >= 0x2c && sc <= 0x32) || sc == 0x0c || sc == 0x0d || sc == 0x1a || sc == 0x1b ||
           sc == 0x27 || sc == 0x28 || sc == 0x2b || (sc >= 0x33 && sc <= 0x35) || sc == 0x48 ||
           sc == 0x50 || sc == 0x4d || sc == 0x4b || sc == 0x47 || sc == 0x4f || sc == 0x49 ||
           sc == 0x51;
}

bool shell_diagnostics_is_quil_text_key(uint8_t sc) {
    return sc == 0x1c || sc == 0x0e || sc == 1 || sc == 0x39 || (sc >= 2 && sc <= 0x0b) ||
           (sc >= 0x10 && sc <= 0x19) || (sc >= 0x1e && sc <= 0x26) || (sc >= 0x2c && sc <= 0x32) ||
           sc == 0x48 || sc == 0x50 || sc == 0x4b || sc == 0x4d || sc == 0x47 || sc == 0x4f;
}

static const char *diagnostic_label(const ShellDiagnosticLabel *label) {
    if (label->len >= sizeof(label->bytes) || label->bytes[label->len])
        sexos_fail();
    return label->bytes;
}

bool shell_diagnostics_send_silkbar_phase2_update(uint32_t kind, uint64_t a, uint64_t b) {
    if (!SILKBAR_PHASE2_SHELL_PROOF_ENABLED)
        return false;
    SexosReply result = shell_diagnostics_pdx_call(SLOT_DISPLAY, OP_SILKBAR_UPDATE, kind, a, b);
    const char *name = kind == 8    ? "SetActiveApp"
                       : kind == 9  ? "SetTintAccent"
                       : kind == 10 ? "SetPaletteState"
                                    : "Unknown";
    bool admitted = !result.status;
    sexos_line("[shell.silkbar.phase2.send] kind=%s a=%llu b=%llu ok=%u reason=%s", name,
               (unsigned long long)a, (unsigned long long)b, (unsigned)admitted,
               admitted ? "sent" : "enqueue_fail");
    return admitted;
}

void shell_diagnostics_bell_send_app_event(const ShellDiagnosticLabel *source, uint64_t event) {
    SexosReply result =
        shell_diagnostics_pdx_call(SLOT_BELL, OP_BELL_NOTIFY, 0x100, event & 255, 0);
    sexos_line("[bell.app.event] source=%s event_id=%llu ok=%u reason=%s", diagnostic_label(source),
               (unsigned long long)event, (unsigned)!result.status,
               result.status ? "enqueue_fail" : "fire_and_forget");
}

#define DRAW_BODY(bytes, length, capacity)                                                         \
    size_t limit = (length) < (capacity) ? (length) : (capacity), offset = 0;                      \
    bool admitted = true;                                                                          \
    while (offset < limit) {                                                                       \
        size_t chunk = limit - offset;                                                             \
        if (chunk > 8)                                                                             \
            chunk = 8;                                                                             \
        uint64_t packed = 0;                                                                       \
        for (size_t i = 0; i < chunk; ++i)                                                         \
            packed |= (uint64_t)(bytes)[offset + i] << (i * 8);                                    \
        uint64_t control = (offset & 255) | ((uint64_t)chunk << 8) |                               \
                           (((uint64_t)(offset >> 8) & 15) << 12) | (color << 32);                 \
        SexosReply reply =                                                                         \
            shell_diagnostics_pdx_call(SLOT_DISPLAY, 0xfb, surface, packed, control);              \
        if (reply.status) {                                                                        \
            admitted = false;                                                                      \
        }                                                                                          \
        offset += chunk;                                                                           \
    }                                                                                              \
    return (ShellDiagnosticDraw){offset, offset > 0 && admitted};

ShellDiagnosticDraw shell_diagnostics_shell_draw_text(uint64_t surface, const ShellTextBytes *text,
                                                      uint64_t color) {
    DRAW_BODY(text->bytes, text->len, SHELL_TEXT_CAPACITY)
}

static ShellDiagnosticDraw diagnostic_draw_sink(uint64_t surface, const ShellSinkBytes *text,
                                                size_t len, uint64_t color) {
    DRAW_BODY((*text), len, SILK_TEXT_SINK_CAP)
}

bool shell_diagnostics_sink_scancode_to_char(uint8_t scan, uint8_t *out) {
    static const uint8_t map[58] = {
        [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U',
        [0x17] = 'I', [0x18] = 'O', [0x19] = 'P', [0x1e] = 'A', [0x1f] = 'S', [0x20] = 'D',
        [0x21] = 'F', [0x22] = 'G', [0x23] = 'H', [0x2c] = 'Z', [0x2d] = 'X', [0x2f] = 'V',
        [0x30] = 'B', [0x31] = 'N', [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0a] = '9',
        [0x0b] = '0', [0x39] = ' '};
    if (scan >= sizeof(map) || !map[scan])
        return false;
    *out = map[scan];
    return true;
}

bool shell_diagnostics_silk_text_sink_redraw(ShellTextSinkContext *ctx) {
    size_t len = ctx->proofs_a->SILK_TEXT_LEN;
    if (len > SILK_TEXT_SINK_CAP)
        return false;
    SexosReply clear =
        shell_diagnostics_pdx_call(SLOT_DISPLAY, OP_TEXT_CLEAR, SILK_TEXT_SINK_SID, 0, 0);
    if (!len) {
        sexos_line("[silk.text.draw] surface=%llu len=0 ok=%u",
                   (unsigned long long)SILK_TEXT_SINK_SID, (unsigned)!clear.status);
        return !clear.status;
    }
    ShellDiagnosticDraw result = {0};
    {
        const SilkShellProofsAState *view = ctx->proofs_a;
        result = diagnostic_draw_sink(SILK_TEXT_SINK_SID, &view->SILK_TEXT_BUF, len, 0x00e8ffff);
    }
    bool admitted = result.admitted && !clear.status;
    sexos_line("[silk.text.draw] surface=%llu len=%zu ok=%u",
               (unsigned long long)SILK_TEXT_SINK_SID, result.attempted, (unsigned)admitted);
    return admitted;
}

bool shell_diagnostics_silk_text_sink_key(ShellTextSinkContext *ctx, uint8_t scan, uint64_t value) {
    if (ctx->chrome->FOCUSED_SURFACE_ID != SILK_TEXT_SINK_SID || !ctx->chrome->SURFACE_100_ALIVE)
        return false;
    size_t len = ctx->proofs_a->SILK_TEXT_LEN;
    if (len > SILK_TEXT_SINK_CAP)
        return false;
    uint8_t ch = 0;
    bool printable = shell_diagnostics_sink_scancode_to_char(scan, &ch);
    if (!printable && scan != 0x0e)
        return false;
    if (value != 1)
        return true;
    if (ctx->proofs_a->silk_text_sink_key__SILK_KEY_ROUTE_BUDGET) {
        --ctx->proofs_a->silk_text_sink_key__SILK_KEY_ROUTE_BUDGET;
        sexos_line("[silk.key.route] surface=%llu code=%u printable=%u",
                   (unsigned long long)SILK_TEXT_SINK_SID, (unsigned)scan, (unsigned)printable);
    }
    if (scan == 0x0e) {
        if (len) {
            ctx->proofs_a->SILK_TEXT_LEN = len - 1;
            sexos_line("[silk.text.backspace] surface=%llu len=%zu",
                       (unsigned long long)SILK_TEXT_SINK_SID, len - 1);
            (void)shell_diagnostics_silk_text_sink_redraw(ctx);
        }
        return true;
    }
    if (printable && len < SILK_TEXT_SINK_CAP) {
        ctx->proofs_a->SILK_TEXT_BUF[len] = ch;
        ctx->proofs_a->SILK_TEXT_LEN = len + 1;
        const char chars[2] = {(char)ch, 0};
        sexos_line("[silk.text.append] surface=%llu len=%zu ch=%s",
                   (unsigned long long)SILK_TEXT_SINK_SID, len + 1, chars);
        bool ok = shell_diagnostics_silk_text_sink_redraw(ctx);
        if (ok && !ctx->proofs_a->SILK_TEXT_PROOF_DONE_EMITTED) {
            ctx->proofs_a->SILK_TEXT_PROOF_DONE_EMITTED = true;
            sexos_line("[silk.text.input.proof.done] ok=1");
        }
    }
    return true;
}

void shell_diagnostics_input_negative_begin_once(SilkShellProofsAState *state) {
    if (!INPUT_NEGATIVE_PROOF_ENABLED || state->INPUT_NEGATIVE_BEGIN_EMITTED)
        return;
    state->INPUT_NEGATIVE_BEGIN_EMITTED = true;
    sexos_line("[input.negative.once] ok=1 malformed_unavailable=1");
}

void shell_diagnostics_input_negative_mark_done(SilkShellProofsAState *state) {
    if (!INPUT_NEGATIVE_PROOF_ENABLED || state->INPUT_NEGATIVE_DONE_EMITTED)
        return;
    if (state->INPUT_NEGATIVE_UNKNOWN_CLASS_SEEN && state->INPUT_NEGATIVE_BAD_BUTTON_SEEN &&
        state->INPUT_NEGATIVE_BUTTON_UP_NO_CAPTURE_SEEN) {
        state->INPUT_NEGATIVE_DONE_EMITTED = true;
        sexos_line(
            "[input.negative.done] ok=1 unknown_class=1 bad_button=1 no_focus_key_reuse=1 dead_target_reuse=1 button_up_no_capture=1 malformed_unavailable=1");
    }
}

void shell_diagnostics_surface_input_lifetime_begin_once(SilkShellProofsAState *state) {
    if (!SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED ||
        state->SURFACE_ID_LIFETIME_PROOF_BEGIN_EMITTED)
        return;
    state->SURFACE_ID_LIFETIME_PROOF_BEGIN_EMITTED = true;
    sexos_line("[surface.input_lifetime.begin] ok=1");
}

void shell_diagnostics_surface_input_lifetime_mark_focus_live(SilkShellProofsAState *state,
                                                              uint64_t focused, uint8_t live,
                                                              uint8_t ok) {
    if (!SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED)
        return;
    if (live == 1 && ok == 1)
        state->SURFACE_ID_LIFETIME_PROOF_FOCUS_LIVE_SEEN = true;
    sexos_line("[surface.input_lifetime.focus_live] focused=%llu live=%u ok=%u",
               (unsigned long long)focused, (unsigned)live, (unsigned)ok);
}

void shell_diagnostics_surface_input_lifetime_mark_key_route(SilkShellProofsAState *state,
                                                             uint64_t target, uint8_t live,
                                                             uint8_t routed, uint8_t ok) {
    if (!SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED)
        return;
    if (live == 1 && ok == 1)
        state->SURFACE_ID_LIFETIME_PROOF_KEY_ROUTE_SEEN = true;
    sexos_line("[surface.input_lifetime.key_route_guard] target=%llu live=%u routed=%u ok=%u",
               (unsigned long long)target, (unsigned)live, (unsigned)routed, (unsigned)ok);
}

void shell_diagnostics_surface_input_lifetime_mark_click_target(SilkShellProofsAState *state,
                                                                uint64_t target, uint8_t live,
                                                                uint8_t committed, uint8_t ok) {
    if (!SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED)
        return;
    if (live == 1 && ok == 1)
        state->SURFACE_ID_LIFETIME_PROOF_CLICK_TARGET_SEEN = true;
    sexos_line("[surface.input_lifetime.click_target_guard] target=%llu live=%u committed=%u ok=%u",
               (unsigned long long)target, (unsigned)live, (unsigned)committed, (unsigned)ok);
}

void shell_diagnostics_surface_input_lifetime_mark_drag_target(SilkShellProofsAState *state,
                                                               uint64_t target, uint8_t live,
                                                               const ShellDiagnosticLabel *phase,
                                                               uint8_t ok) {
    if (!SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED)
        return;
    if (live == 1 && ok == 1)
        state->SURFACE_ID_LIFETIME_PROOF_DRAG_TARGET_SEEN = true;
    sexos_line("[surface.input_lifetime.drag_target_guard] target=%llu live=%u phase=%s ok=%u",
               (unsigned long long)target, (unsigned)live, diagnostic_label(phase), (unsigned)ok);
}

void shell_diagnostics_surface_input_lifetime_mark_dead_clear(SilkShellProofsAState *state,
                                                              uint64_t target) {
    if (!SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED)
        return;
    state->SURFACE_ID_LIFETIME_PROOF_DEAD_CLEAR_SEEN = true;
    sexos_line("[surface.input_lifetime.dead_clear] target=%llu cleared=1 ok=1",
               (unsigned long long)target);
}

void shell_diagnostics_surface_input_lifetime_try_done(SilkShellProofsAState *state) {
    if (!SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF_ENABLED)
        return;
    if (state->SURFACE_ID_LIFETIME_PROOF_FOCUS_LIVE_SEEN &&
        state->SURFACE_ID_LIFETIME_PROOF_KEY_ROUTE_SEEN &&
        state->SURFACE_ID_LIFETIME_PROOF_CLICK_TARGET_SEEN &&
        state->SURFACE_ID_LIFETIME_PROOF_DRAG_TARGET_SEEN)
        sexos_line("[surface.input_lifetime.done] ok=1 dead_clear=%u",
                   (unsigned)state->SURFACE_ID_LIFETIME_PROOF_DEAD_CLEAR_SEEN);
}

void shell_diagnostics_shell_interaction_contract_begin_once(SilkShellProofsAState *state) {
    if (!SHELL_INTERACTION_CONTRACT_PROOF_ENABLED ||
        state->SHELL_INTERACTION_CONTRACT_BEGIN_EMITTED)
        return;
    state->SHELL_INTERACTION_CONTRACT_BEGIN_EMITTED = true;
    sexos_line("[shell.interact.contract.begin] mode=proof ok=1");
}

void shell_diagnostics_click_drag_proof_begin_click_once(SilkShellProofsAState *state) {
    if (!CLICK_DRAG_PROOF_ENABLED || state->CLICK_FOCUS_PROOF_BEGIN_EMITTED)
        return;
    state->CLICK_FOCUS_PROOF_BEGIN_EMITTED = true;
    sexos_line("[click.focus.proof.begin] ok=1");
}

void shell_diagnostics_click_drag_proof_begin_drag_once(SilkShellProofsAState *state) {
    if (!CLICK_DRAG_PROOF_ENABLED || state->DRAG_PROOF_BEGIN_EMITTED)
        return;
    state->DRAG_PROOF_BEGIN_EMITTED = true;
    sexos_line("[drag.proof.begin] ok=1");
}

void shell_diagnostics_shell_interaction_contract_mark_stage(SilkShellProofsAState *state,
                                                             uint8_t stage) {
    if (!SHELL_INTERACTION_CONTRACT_PROOF_ENABLED)
        return;
    state->SHELL_INTERACTION_CONTRACT_STAGE_MASK |= stage;
}

void shell_diagnostics_shell_interaction_contract_try_done(SilkShellProofsAState *state) {
    if (!SHELL_INTERACTION_CONTRACT_PROOF_ENABLED || state->SHELL_INTERACTION_CONTRACT_DONE_EMITTED)
        return;
    uint8_t required = SHELL_INTERACTION_STAGE_POINTER | SHELL_INTERACTION_STAGE_CLICK |
                       SHELL_INTERACTION_STAGE_DRAG | SHELL_INTERACTION_STAGE_KEY;
    if ((state->SHELL_INTERACTION_CONTRACT_STAGE_MASK & required) == required) {
        state->SHELL_INTERACTION_CONTRACT_DONE_EMITTED = true;
        sexos_line(
            "[shell.interact.contract.done] pointer=1 click=1 drag=1 key=1 dead_guard=%u no_focus_key=%u ok=1",
            (unsigned)((state->SHELL_INTERACTION_CONTRACT_STAGE_MASK &
                        SHELL_INTERACTION_STAGE_DEAD_GUARD) != 0),
            (unsigned)((state->SHELL_INTERACTION_CONTRACT_STAGE_MASK &
                        SHELL_INTERACTION_STAGE_NO_FOCUS_KEY) != 0));
    }
}


