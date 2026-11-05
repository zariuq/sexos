
#ifndef SEXOS_APP_PRIVATE_H
#define SEXOS_APP_PRIVATE_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define APP_PRIVATE_MAX_PAGES 512u
#define APP_PRIVATE_PHYSICAL UINT64_C(0x000fffffffffF000)

enum {
    APP_PRIVATE_OK = 0,
    APP_PRIVATE_INVALID = 1,
    APP_PRIVATE_RETRY = 2,
    APP_PRIVATE_DENIED = 3,
    APP_PRIVATE_LIVE = 5
};

enum {
    APP_PRIVATE_QUERY = 1,
    APP_PRIVATE_ACK = 2,
    APP_PRIVATE_RETIRE = 4,
    APP_PRIVATE_REQUIRED = 31
};

typedef struct {
    uint32_t target_pd, task_id, holder_pd;
    uint8_t rights, ack_bit;
} AppLifecycleGrant;

typedef struct {
    uint64_t va, leaf, expected;
} AppPrivatePage;

typedef struct {
    AppPrivatePage pages[APP_PRIVATE_MAX_PAGES];
    size_t count;
} AppPrivatePages;

typedef struct {
    uint64_t status;
    size_t count;
} AppPrivateResult;

typedef struct {
    uint32_t logical_app, number;
} AppIncarnation;

typedef struct {
    bool present;
    AppIncarnation value;
} AppNextIncarnation;

typedef struct {
    uint32_t holder, logical_app, incarnation, predecessor, predecessor_task, successor,
        successor_task;
} AppActivationGrant;

uint64_t app_lifecycle_authorize(AppLifecycleGrant grant, uint32_t holder, uint32_t target,
                                 uint32_t task, uint8_t right);
uint64_t app_private_final_flags(uint64_t flags, uint8_t key);
AppPrivateResult app_private_retire(const AppPrivatePages *pages);
AppPrivateResult app_private_restore_check(const AppPrivatePages *pages);
AppPrivateResult app_private_restore(const AppPrivatePages *pages);
AppPrivateResult app_private_handover_check(const AppPrivatePages *pages,
                                            const AppPrivatePages *workspace);
AppPrivateResult app_private_handover(const AppPrivatePages *pages,
                                      const AppPrivatePages *workspace);
bool app_notice_due(uint8_t acked, uint8_t hinted, uint64_t last, uint64_t now, uint8_t recipient);
AppIncarnation app_incarnation_original(uint32_t target);
AppNextIncarnation app_incarnation_next(AppIncarnation old);
uint64_t app_activation_authorize(AppActivationGrant grant, uint32_t holder, uint32_t predecessor,
                                  uint32_t predecessor_task, uint32_t successor,
                                  uint32_t successor_task, AppIncarnation old, AppIncarnation next);
uint64_t app_private_leaf_read(uint64_t leaf);
void app_private_leaf_write(uint64_t leaf, uint64_t value);
void app_private_leaf_flush(uint64_t va);
#endif
