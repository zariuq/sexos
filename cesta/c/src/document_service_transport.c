#include "sexos_document_service_transport.h"
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

void document_service_transport_init(DocumentServiceTransport *state, bool correlated_required) {
    memset(state, 0, sizeof *state);
    document_service_init(&state->service);
    state->correlated_required = correlated_required;
}

static size_t document_envelope_slot(const DocumentServiceTransport *state) {
    for (size_t i = 0; i < DOCUMENT_SERVICE_RECEIPTS; i++)
        if (!state->envelopes[i].present)
            return i;
    return SIZE_MAX;
}

bool document_service_transport_prepare(DocumentServiceTransport *state,
                                        const SexosRequest *request, bool live) {
    if (request->message.type_id != SEXOS_DOCUMENT_OP)
        return false;
    size_t slot = document_envelope_slot(state);
    if (slot == SIZE_MAX)
        return true;
    uint32_t sequence = (uint32_t)(request->message.arg0 & SEXOS_DOCUMENT_SEQ_MAX);
    SexosDocumentDecode decoded =
        sexos_document_decode(request->message.arg0, request->message.arg1, request->message.arg2);
    DocumentServiceEnvelope envelope = {true, false, *request, decoded.request, 0};
    if (!decoded.ok)
        envelope.value = sexos_document_reply(sequence, false, SEXOS_DOCUMENT_INVALID),
        envelope.ready = true;
    else if ((state->correlated_required && !request->id) ||
             (request->id && request->message.caller_task != request->message.caller_pd))
        envelope.value = sexos_document_reply(decoded.request.seq, false, SEXOS_DOCUMENT_DENIED),
        envelope.ready = true;
    else {
        DocumentServiceResult result = document_service_handle(
            &state->service, request->message.caller_pd, live, decoded.request);
        envelope.ready = result.answered;
        envelope.value = result.value;
    }
    state->envelopes[slot] = envelope;
    return true;
}

void document_service_transport_progress(DocumentServiceTransport *state) {
    DocumentServiceCompletion completion = document_service_progress(&state->service);
    if (!completion.present)
        return;
    for (size_t i = 0; i < DOCUMENT_SERVICE_RECEIPTS; i++) {
        DocumentServiceEnvelope *envelope = &state->envelopes[i];
        if (envelope->present && !envelope->ready &&
            envelope->transport.message.caller_pd == completion.caller &&
            envelope->document.seq == completion.request.seq) {
            envelope->value = completion.value;
            envelope->ready = true;
            return;
        }
    }
}

bool document_service_transport_poll(DocumentServiceTransport *state) {
    for (size_t i = 0; i < DOCUMENT_SERVICE_RECEIPTS; i++) {
        DocumentServiceEnvelope *envelope = &state->envelopes[i];
        if (!envelope->present || !envelope->ready)
            continue;
        uint64_t status = sexos_answer_request(&envelope->transport, envelope->value);
        if (!status || (envelope->transport.id && status == SEXOS_RPC_INVALID))
            memset(envelope, 0, sizeof *envelope);
    }
    for (size_t i = 0; i < DOCUMENT_SERVICE_RECEIPTS; i++)
        if (state->envelopes[i].present)
            return false;
    return true;
}

bool document_service_transport_pending(const DocumentServiceTransport *state) {
    for (size_t i = 0; i < DOCUMENT_SERVICE_RECEIPTS; i++)
        if (state->envelopes[i].present)
            return true;
    return false;
}

bool document_service_transport_retire(DocumentServiceTransport *state, uint32_t caller) {
    if (!document_service_retire(&state->service, caller))
        return false;
    for (size_t i = 0; i < DOCUMENT_SERVICE_RECEIPTS; i++) {
        DocumentServiceEnvelope *envelope = &state->envelopes[i];
        if (envelope->present && envelope->transport.message.caller_pd == caller) {
            envelope->ready = true;
            envelope->value =
                sexos_document_reply(envelope->document.seq, false, SEXOS_DOCUMENT_DENIED);
        }
    }
    return true;
}


