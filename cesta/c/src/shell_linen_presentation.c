#include "shell_linen_runtime.h"
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

static void presentation_terminal(ShellLinenRuntime *runtime, ShellPresentationJob job,
                                  uint8_t outcome) {
    if (!job.present)
        return;
    runtime->presentation_last =
        (ShellPresentationTerminal){true, outcome, job.job, job.revision, job.sequence};
}

static bool presentation_mint(ShellLinenRuntime *runtime, ShellPresentationJob *out,
                              uint64_t revision, uint32_t sequence) {
    if (runtime->next_presentation_job >= UINT64_C(0x7ffffffffffffffe) || !revision || !sequence)
        return false;
    runtime->next_presentation_job += 1;
    *out =
        (ShellPresentationJob){.present = true,
                               .job = UINT64_C(0x8000000000000000) | runtime->next_presentation_job,
                               .revision = revision,
                               .sequence = sequence};
    return true;
}

bool shell_linen_text_queue(ShellLinenRuntime *runtime, const ShellTextBytes *text, uint32_t color,
                            ShellTextMetadata metadata) {
    if (runtime->publication_revision == 0 ||
        runtime->next_presentation_job >= UINT64_C(0x7ffffffffffffffe))
        return false;
    bool active = runtime->text.active;
    if (!shell_text_queue(&runtime->text, text, color, metadata))
        return false;
    ShellPresentationJob job = {0};
    uint32_t sequence = active ? runtime->text.desired.sequence : runtime->text.current.sequence;
    if (!presentation_mint(runtime, &job, runtime->publication_revision, sequence))
        return false;
    if (!active) {
        runtime->presentation_active = job;
        return true;
    }
    if (runtime->presentation_desired.present)
        presentation_terminal(runtime, runtime->presentation_desired,
                              SHELL_PRESENTATION_SUPERSEDED);
    runtime->presentation_desired = job;
    return true;
}

static void presentation_promote(ShellLinenRuntime *runtime) {
    if (runtime->text.active && runtime->presentation_desired.present &&
        runtime->text.current.sequence == runtime->presentation_desired.sequence) {
        runtime->presentation_active = runtime->presentation_desired;
        runtime->presentation_desired = (ShellPresentationJob){0};
    }
}

ShellTextPoll shell_linen_text_poll(ShellLinenRuntime *runtime, size_t budget) {
    if (!runtime->text.active && runtime->text.latest && runtime->presentation_desired.present) {
        runtime->presentation_active = runtime->presentation_desired;
        runtime->presentation_desired = (ShellPresentationJob){0};
    }
    ShellTextBatch before = runtime->text.current;
    ShellTextPoll result = shell_text_poll(&runtime->text, budget);
    if (runtime->presentation_active.present &&
        (before.cleared || before.offset || before.awaiting || runtime->text.current.cleared ||
         runtime->text.current.offset || runtime->text.current.awaiting))
        runtime->presentation_active.submitted = true;
    if (result.kind == SHELL_TEXT_COMPLETE) {
        presentation_terminal(runtime, runtime->presentation_active, SHELL_PRESENTATION_COMPLETE);
        runtime->presentation_active = (ShellPresentationJob){0};
    } else if (result.kind == SHELL_TEXT_FAILED) {
        presentation_terminal(runtime, runtime->presentation_active,
                              runtime->presentation_active.submitted
                                  ? SHELL_PRESENTATION_INDETERMINATE
                                  : SHELL_PRESENTATION_FAILED);
        runtime->presentation_active = (ShellPresentationJob){0};
    }
    presentation_promote(runtime);
    return result;
}

bool shell_linen_text_receive(ShellLinenRuntime *runtime, uint32_t caller, uint32_t task,
                              uint64_t value, ShellTextReceipt *receipt) {
    if (caller != 1 || task != 1 || !runtime->presentation_active.present)
        return false;
    uint32_t sequence = (uint32_t)(value >> 24);
    if (sequence != runtime->presentation_active.sequence)
        return false;
    if (!shell_text_receive(&runtime->text, caller, value, receipt))
        return false;
    presentation_terminal(runtime, runtime->presentation_active, SHELL_PRESENTATION_COMPLETE);
    runtime->presentation_active = (ShellPresentationJob){0};
    return true;
}

void shell_linen_text_cancel(ShellLinenRuntime *runtime) {
    if (runtime->presentation_active.present)
        presentation_terminal(runtime, runtime->presentation_active,
                              runtime->presentation_active.submitted
                                  ? SHELL_PRESENTATION_INDETERMINATE
                                  : SHELL_PRESENTATION_FAILED);
    if (runtime->presentation_desired.present)
        presentation_terminal(runtime, runtime->presentation_desired,
                              SHELL_PRESENTATION_SUPERSEDED);
    runtime->presentation_active = (ShellPresentationJob){0};
    runtime->presentation_desired = (ShellPresentationJob){0};
    shell_text_cancel(&runtime->text);
}


