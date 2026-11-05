
#ifndef SEXOS_SHELL_MODELS_BELL_H
#define SEXOS_SHELL_MODELS_BELL_H
#include "silk_shell.h"

typedef struct ShellBellModelsContext {
    SilkShellModelsState *models;
} ShellBellModelsContext;

void shell_models_bell_record_event(ShellBellModelsContext *ctx, uint64_t object_id,
                                    uint64_t buffer_id);
size_t shell_models_bell_ring_count(const ShellBellModelsContext *ctx);
#endif
