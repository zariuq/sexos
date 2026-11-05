
#ifndef SEXOS_LINEN_CAPTURE_TRANSPORT_H
#define SEXOS_LINEN_CAPTURE_TRANSPORT_H
#include "linen_capture_client.h"
#include "sexos_rpc.h"

typedef struct LinenCaptureStep {
    uint8_t event;
    uint64_t status;
    bool admitted;
} LinenCaptureStep;

LinenCaptureStep linen_capture_transport_step(LinenCaptureClient *state, uint64_t slot);
bool linen_capture_transport_retry(LinenCaptureClient *state);
bool linen_capture_transport_cancel(LinenCaptureClient *state);
#endif
