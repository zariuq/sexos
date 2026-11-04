#ifndef SEXOS_TEST_LINEN_SERVICE_PROBE_ADAPTER_H
#define SEXOS_TEST_LINEN_SERVICE_PROBE_ADAPTER_H
#include "sexos_rpc.h"
void probe_linen_init(void);
void probe_linen_dispatch(SexosRequest *request);
bool probe_linen_capture_active(void);
void probe_linen_mutate_live_table(void);
#endif
