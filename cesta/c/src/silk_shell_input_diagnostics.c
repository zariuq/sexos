#include "silk_shell_input.h"
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

bool shell_input_peer_is_spindle_text_key(ShellInputContext *ctx, uint8_t scancode) {
    (void)ctx;
    return shell_diagnostics_is_spindle_text_key(scancode);
}

bool shell_input_peer_is_quil_text_key(ShellInputContext *ctx, uint8_t scancode) {
    (void)ctx;
    return shell_diagnostics_is_quil_text_key(scancode);
}

void shell_input_peer_input_negative_begin_once(ShellInputContext *ctx) {
    shell_diagnostics_input_negative_begin_once(ctx->shell.proofs_a);
}

void shell_input_peer_input_negative_mark_done(ShellInputContext *ctx) {
    shell_diagnostics_input_negative_mark_done(ctx->shell.proofs_a);
}

void shell_input_peer_click_drag_proof_begin_click_once(ShellInputContext *ctx) {
    shell_diagnostics_click_drag_proof_begin_click_once(ctx->shell.proofs_a);
}

void shell_input_peer_shell_interaction_contract_begin_once(ShellInputContext *ctx) {
    shell_diagnostics_shell_interaction_contract_begin_once(ctx->shell.proofs_a);
}

void shell_input_peer_shell_interaction_contract_mark_stage(ShellInputContext *ctx, uint8_t stage) {
    shell_diagnostics_shell_interaction_contract_mark_stage(ctx->shell.proofs_a, stage);
}

void shell_input_peer_shell_interaction_contract_try_done(ShellInputContext *ctx) {
    shell_diagnostics_shell_interaction_contract_try_done(ctx->shell.proofs_a);
}

void shell_input_peer_surface_input_lifetime_mark_dead_clear(ShellInputContext *ctx,
                                                             uint64_t target) {
    shell_diagnostics_surface_input_lifetime_mark_dead_clear(ctx->shell.proofs_a, target);
}

void shell_input_peer_surface_input_lifetime_mark_drag_target(ShellInputContext *ctx,
                                                              uint64_t target, uint8_t live,
                                                              const ShellDiagnosticLabel *phase,
                                                              uint8_t ok) {
    shell_diagnostics_surface_input_lifetime_mark_drag_target(ctx->shell.proofs_a, target, live,
                                                              phase, ok);
}

void shell_input_peer_surface_input_lifetime_mark_key_route(ShellInputContext *ctx, uint64_t target,
                                                            uint8_t live, uint8_t routed,
                                                            uint8_t ok) {
    shell_diagnostics_surface_input_lifetime_mark_key_route(ctx->shell.proofs_a, target, live,
                                                            routed, ok);
}

void shell_input_peer_surface_input_lifetime_try_done(ShellInputContext *ctx) {
    shell_diagnostics_surface_input_lifetime_try_done(ctx->shell.proofs_a);
}

bool shell_input_peer_silk_text_sink_key(ShellInputContext *ctx, uint8_t scancode, uint64_t value) {
    ShellTextSinkContext sink = {.proofs_a = ctx->shell.proofs_a, .chrome = ctx->shell.chrome};
    return shell_diagnostics_silk_text_sink_key(&sink, scancode, value);
}


