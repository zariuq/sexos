#ifndef SEXOS_SHELL_APP_STATE_H
#define SEXOS_SHELL_APP_STATE_H
#include <stdatomic.h>
#include "silk_shell_viewport.h"
typedef struct ShellDocumentPointer {bool pending;uint64_t slot;uint32_t target;uint8_t action;uint64_t sequence;} ShellDocumentPointer;
typedef struct ShellChromeOperation {uint64_t opcode,surface,packed,control;} ShellChromeOperation;
typedef struct ShellChromeLatestOperation {bool pending;ShellChromeOperation operation;} ShellChromeLatestOperation;
typedef struct ShellChromeApps {
    ShellAppCustody entries[SHELL_APP_COUNT];
    bool present[SHELL_APP_COUNT];
    atomic_uint_least32_t width,height;
    ShellDocumentDelivery document[SHELL_DOCUMENT_VIEW_COUNT];
    ShellSpindleDelivery spindle[SHELL_SPINDLE_VIEW_COUNT];
    ShellChromeLatestOperation palette_visibility;
    uint64_t document_pointer_sequence;ShellDocumentPointer document_pointer;
} ShellChromeApps;
#endif
