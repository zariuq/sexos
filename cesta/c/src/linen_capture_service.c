#include "linen_capture_service.h"
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

bool linen_capture_service_prepare(LinenCaptureServer *state, const LinenCaptureTable *table,
                                   const SexosRequest *request, LinenCaptureAnswer *answer) {
    uint64_t opcode = request->message.type_id;
    if (opcode != LINEN_CAPTURE_OP_BEGIN && opcode != LINEN_CAPTURE_OP_READ &&
        opcode != LINEN_CAPTURE_OP_END)
        return false;
    if (answer->present)
        return false;
    uint64_t value = linen_capture_error(LINEN_CAPTURE_DENIED);
    if (request->id != 0) {
        uint32_t caller = request->message.caller_pd;
        uint32_t task = request->message.caller_task;
        uint64_t job = request->message.arg0;
        uint64_t a = request->message.arg1;
        uint64_t b = request->message.arg2;
        if (opcode == LINEN_CAPTURE_OP_BEGIN)
            value = linen_capture_begin(state, caller, task, job, a, b, table);
        else if (opcode == LINEN_CAPTURE_OP_READ)
            value = linen_capture_read(state, caller, task, job, a, b);
        else
            value = linen_capture_end(state, caller, task, job, a, b);
    }
    answer->request = *request;
    answer->value = value;
    answer->present = true;
    return true;
}

bool linen_capture_answer_poll(LinenCaptureAnswer *answer) {
    if (!answer->present)
        return true;
    uint64_t result = sexos_answer_request(&answer->request, answer->value);
    if (result != 0 && !(answer->request.id != 0 && result == SEXOS_RPC_INVALID))
        return false;
    answer->present = false;
    return true;
}


