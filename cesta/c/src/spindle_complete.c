#include "spindle_complete.h"
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

static const char spindle_commands[][32] = {"about",
                                            "app-info",
                                            "app-state",
                                            "app-status",
                                            "apps",
                                            "atlas",
                                            "authority-status",
                                            "bell",
                                            "bell-launch-events",
                                            "bell-status",
                                            "bell-test",
                                            "blockers",
                                            "browser",
                                            "browser-localdoc",
                                            "browser-localdoc-status",
                                            "browser-roadmap",
                                            "browser-status",
                                            "browser-surface",
                                            "calc",
                                            "catdoc",
                                            "catdocx",
                                            "clear",
                                            "close",
                                            "collar-grants",
                                            "collar-status",
                                            "daily",
                                            "disk",
                                            "disk-stat",
                                            "echo",
                                            "edit",
                                            "edit-help",
                                            "edit-status",
                                            "editor",
                                            "events",
                                            "faults",
                                            "files",
                                            "filldoc",
                                            "filldocx",
                                            "focus-help",
                                            "frame-chrome",
                                            "frame-lights",
                                            "frame-lights-keys",
                                            "frame-rim",
                                            "help",
                                            "history",
                                            "history-cancel",
                                            "history-retry",
                                            "history-status",
                                            "input",
                                            "keys",
                                            "launch",
                                            "launch-outcomes",
                                            "lifecycle",
                                            "linen-list",
                                            "linen-open",
                                            "linen-search",
                                            "linen-status",
                                            "load",
                                            "ls",
                                            "mesh-graph",
                                            "mesh-graph-status",
                                            "mkdoc",
                                            "mvdoc",
                                            "net-cancel",
                                            "net-probe",
                                            "net-retry",
                                            "net-status",
                                            "notify",
                                            "object-new",
                                            "object-search",
                                            "object-tag",
                                            "pd",
                                            "probe-cancel",
                                            "probe-help",
                                            "probe-retry",
                                            "probe-status",
                                            "project-scene-link",
                                            "project-scene-status",
                                            "proof",
                                            "quil",
                                            "rmdoc",
                                            "route",
                                            "save",
                                            "scene-keys",
                                            "scene-lifecycle",
                                            "scene-lifecycle-status",
                                            "scene-status",
                                            "scene-switch-status",
                                            "search",
                                            "servers",
                                            "session",
                                            "sextime",
                                            "status",
                                            "truncdoc",
                                            "url",
                                            "url-status",
                                            "web",
                                            "window-keys",
                                            "windows",
                                            "winreset"};

SpindleCompletion spindle_complete(const SpindleCompleteWord *word) {
    SpindleCompletion out = {0};
    if (!word || word->too_long || word->len > SPINDLE_COMPLETE_WORD_MAX)
        return out;
    size_t first = SIZE_MAX, common = 0;
    bool candidates_done = false;
    for (size_t i = 0; i < sizeof spindle_commands / sizeof spindle_commands[0]; i++) {
        bool prefix = true;
        for (size_t k = 0; k < word->len; k++)
            if (!spindle_commands[i][k] || (uint8_t)spindle_commands[i][k] != word->bytes[k]) {
                prefix = false;
                break;
            }
        if (!prefix || !spindle_commands[i][word->len])
            continue;
        size_t command_length = 0;
        while (command_length < 32 && spindle_commands[i][command_length])
            command_length++;
        if (first == SIZE_MAX) {
            first = i;
            common = command_length;
        } else {
            size_t k = 0;
            while (k < common && spindle_commands[i][k] == spindle_commands[first][k])
                k++;
            common = k;
        }
        out.matches++;
        if (!candidates_done &&
            out.candidates_len + 2 + command_length <= SPINDLE_COMPLETE_CANDIDATES_MAX) {
            out.candidates[out.candidates_len++] = ' ';
            out.candidates[out.candidates_len++] = ' ';
            for (size_t k = 0; k < command_length; k++)
                out.candidates[out.candidates_len++] = (uint8_t)spindle_commands[i][k];
        } else
            candidates_done = true;
    }
    if (first == SIZE_MAX)
        return out;
    out.matched = true;
    out.extra_len = common - word->len;
    for (size_t i = 0; i < out.extra_len; i++)
        out.extra[i] = (uint8_t)spindle_commands[first][word->len + i];
    return out;
}


