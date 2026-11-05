
#ifndef SEXOS_HISTORY_RECOVERY_H
#define SEXOS_HISTORY_RECOVERY_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "kernel_current.h"
#ifndef HISTORY_ROSTER_COUNT
#define COUNT_HISTORY_ROW(p_target, p_ordinal, p_surface, p_shell_surface, p_lifecycle, p_close,   \
                          p_activation, p_predecessor, p_logical, p_route)                         \
    +(p_logical == CURRENT_PD_spindle ? 1 : 0)

enum {
    HISTORY_ROSTER_COUNT = 0 SEXOS_CURRENT_APP_ROWS(COUNT_HISTORY_ROW)
};

#undef COUNT_HISTORY_ROW
#endif
#define HISTORY_ROSTER_STORAGE (HISTORY_ROSTER_COUNT ? HISTORY_ROSTER_COUNT : 1)

enum {
    HISTORY_VERSION = 1,
    HISTORY_SLOT = 53,
    HISTORY_SYS_QUERY = 67,
    HISTORY_SECTORS = 18
};

typedef struct {
    uint32_t target, task;
} HistoryParticipant;

typedef struct {
    uint32_t holder, logical;
    HistoryParticipant participants[HISTORY_ROSTER_STORAGE];
    uint64_t base, sectors;
} HistoryGrant;

typedef struct {
    bool present;
    HistoryParticipant value;
} HistoryFound;

typedef struct {
    uint64_t status, value0, value1, value2, value3;
} HistoryReply;

typedef struct {
    uint32_t values[HISTORY_ROSTER_STORAGE];
} HistoryTargets;

typedef struct {
    uint64_t slot, cookie;
} HistoryQueryContext;

typedef struct {
    uint64_t status;
    HistoryGrant value;
} HistoryDecode;

bool history_valid(HistoryGrant grant);
HistoryFound history_participant(HistoryGrant grant, uint32_t target);
bool history_permits(HistoryGrant grant, uint32_t holder, uint32_t target, uint32_t task);
HistoryReply history_query(HistoryGrant grant, uint32_t holder, uint64_t kind);
HistoryDecode history_decode_for(uint32_t holder, uint32_t logical, HistoryTargets targets,
                                 HistoryQueryContext context);
HistoryReply history_query_provider(HistoryQueryContext context, uint64_t kind);
HistoryDecode history_get(uint64_t slot);
#endif
