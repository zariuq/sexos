
#ifndef SEXOS_SHELL_LINEN_RUNTIME_H
#define SEXOS_SHELL_LINEN_RUNTIME_H
#include "shell_linen_snapshot.h"
#include "shell_text_delivery.h"
#include "silk_shell_custody.h"
#include "linen_capture_client.h"

enum {
    SHELL_PRESENTATION_PENDING = 0,
    SHELL_PRESENTATION_COMPLETE = 1,
    SHELL_PRESENTATION_SUPERSEDED = 2,
    SHELL_PRESENTATION_FAILED = 3,
    SHELL_PRESENTATION_INDETERMINATE = 4
};

typedef struct ShellPresentationJob {
    bool present, submitted;
    uint64_t job, revision;
    uint32_t sequence;
} ShellPresentationJob;

typedef struct ShellPresentationTerminal {
    bool present;
    uint8_t outcome;
    uint64_t job, revision;
    uint32_t sequence;
} ShellPresentationTerminal;

typedef struct ShellLinenRuntime {
    ShellLinenSnapshot snapshot, capture_snapshot;
    ShellTextOutbox text;
    LinenCaptureClient capture;
    uint64_t publication_revision;
    uint64_t next_presentation_job;
    ShellPresentationJob presentation_active, presentation_desired;
    ShellPresentationTerminal presentation_last;
    bool correlated_required, correlation_refused_logged;
    bool repaint_after_create;
    bool workspace_refresh_pending[SHELL_APP_COUNT];
} ShellLinenRuntime;

bool shell_linen_text_queue(ShellLinenRuntime *runtime, const ShellTextBytes *text, uint32_t color,
                            ShellTextMetadata metadata);
ShellTextPoll shell_linen_text_poll(ShellLinenRuntime *runtime, size_t budget);
bool shell_linen_text_receive(ShellLinenRuntime *runtime, uint32_t caller, uint32_t task,
                              uint64_t value, ShellTextReceipt *receipt);
void shell_linen_text_cancel(ShellLinenRuntime *runtime);
#endif
