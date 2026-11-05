#include "linen_capture_transport.h"
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

LinenCaptureStep linen_capture_transport_step(LinenCaptureClient *state, uint64_t slot) {
    LinenCaptureStep step = {0};
    if (state->request != 0) {
        uint64_t request = state->request;
        SexosReply reply = sexos_rpc_poll(request);
        step.status = reply.status;
        if (reply.status == 0)
            step.event = linen_capture_client_reply(state, request, reply.value);
        return step;
    }
    LinenCaptureAction action = linen_capture_client_action(state);
    if (!action.present)
        return step;
    SexosReply admission =
        sexos_rpc_begin(slot, action.opcode, action.job, action.arg1, action.arg2);
    step.status = admission.status;
    if (admission.status != 0)
        return step;
    if (!linen_capture_client_sent(state, admission.value)) {
        (void)sexos_rpc_cancel(admission.value);
        step.status = SEXOS_RPC_INVALID;
        return step;
    }
    step.admitted = true;
    return step;
}

bool linen_capture_transport_retry(LinenCaptureClient *state) {
    if (state->request == 0)
        return true;
    uint64_t request = state->request;
    uint64_t result = sexos_rpc_cancel(request);
    if (result != 0 && result != SEXOS_RPC_INVALID)
        return false;
    return linen_capture_client_retry(state, request);
}

bool linen_capture_transport_cancel(LinenCaptureClient *state) {
    if (!linen_capture_transport_retry(state))
        return false;
    return linen_capture_client_cancel(state);
}


