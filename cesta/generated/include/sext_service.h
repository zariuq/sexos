
#ifndef SEXOS_SEXT_SERVICE_H
#define SEXOS_SEXT_SERVICE_H
#include "sexos_server.h"
#include "sext.h"
#include "sexos_app_catalog.h"
_Static_assert(SEXT_FRAME_COUNT == 8, "the upstream boot service has eight frames");
typedef struct { uint32_t target_pd, task_id; uint64_t surface_id; uint32_t coordinator_pd; } SextCatalog;
typedef struct { uint64_t base,length; uint32_t task_id; } SextRetiredPool;
typedef struct { bool present; uint32_t target_pd,task_id; } SextAppNotice;
typedef enum { SEXT_WORK_NONE, SEXT_WORK_FAULT, SEXT_WORK_RELEASE, SEXT_WORK_WAIT, SEXT_WORK_REPLY } SextWorkTag;
typedef struct {
    SextWorkTag tag; SextFaultNotice fault; SextReleaseRequest release;
    SextWaitRequest wait; uint32_t target; uint64_t value;
} SextWork;
typedef struct { bool present; SextExitNotice value; } SextPendingExit;
typedef struct { bool present; SextWorkspace value; } SextWorkspaceSlot;
typedef struct { bool present; uint64_t mapping; uint32_t target; uint64_t token; } SextRetiredFrame;
typedef struct {
    SextPager pager; SextWork pending[2]; SextPendingExit cleanup_pending[2];
    SextAppNotice app_pending[SEXOS_APP_CATALOG_COUNT]; SextWorkspaceSlot workspaces[SEXOS_APP_CATALOG_COUNT];
    SextRetiredFrame retired[SEXT_FRAME_STORAGE]; unsigned turn;
} SextService;
uint64_t sext_native_info(uint64_t slot,SextPagingInfo *out);
uint64_t sext_native_catalog(uint64_t slot,SextCatalog *out);
uint64_t sext_native_retire(uint64_t slot,SextRetiredPool *out);
_Noreturn void sext_fatal(const char *reason,uint64_t status);
void sext_workspace_admit(SextWorkspace *out,uint64_t base,uint64_t bytes);
uint64_t sext_workspace_record(SextWorkspace *w,uint32_t target,uint64_t token);
void sext_workspace_rewind(SextWorkspace *w);
void sext_service_init(SextService *s);
bool sext_service_can_progress(const SextService *s);
bool sext_service_pending(const SextService *s);
void sext_service_turn(SextService *s,bool incoming,SexosMessage message);
void sext_main(void);
#endif
