#ifndef SEXOS_SILK_SHELL_CORE_H
#define SEXOS_SILK_SHELL_CORE_H
#include "silk_shell_proofs_b_input.h"
void silk_shell_state_init(SilkShellState *state);
void silk_shell_proofs_tick(void);
typedef struct SilkShellDispatchResult {bool handled,mutated,reply;uint64_t value;} SilkShellDispatchResult;
typedef enum ShellStartupObserve {SHELL_STARTUP_READY=0,SHELL_STARTUP_DEFERRED=1,SHELL_STARTUP_REJECTED_CONTROL=2,SHELL_STARTUP_FULL=3} ShellStartupObserve;
ShellStartupObserve shell_startup_observe(SilkShellCoreState *state,SexosMessage message);
bool shell_startup_pop(SilkShellCoreState *state,SexosMessage *out);
bool shell_startup_pending(const SilkShellCoreState *state);
SilkShellDispatchResult silk_shell_dispatch(SilkShellState *state,const SexosMessage *message);
#endif
