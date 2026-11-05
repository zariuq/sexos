#include "silk_shell_models_linen.h"
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

#if defined(SEXOS_LINEN_CORRELATED_REQUIRED) && SEXOS_LINEN_CORRELATED_REQUIRED
#define SHELL_LINEN_STRONG_DEFAULT 1
#else
#define SHELL_LINEN_STRONG_DEFAULT 0
#endif
static bool shell_models_capture_snapshot(LinenCaptureClient *capture, ShellLinenSnapshot *out) {
    ShellLinenSnapshot next = {0};
    for (size_t i = 0; i < LINEN_CAPTURE_SLOTS && i < SHELL_LINEN_CAPACITY; ++i) {
        const LinenCaptureRow *source = &capture->table.rows[i];
        if (!source->present)
            continue;
        if (source->id == 0 || source->id == UINT64_MAX || source->len > 24)
            return false;
        next.present[i] = true;
        next.objects[i].id = source->id;
        next.objects[i].kind = source->kind;
        next.objects[i].disk = source->disk;
        next.objects[i].len = source->len;
        for (size_t j = 0; j < 24; ++j)
            next.objects[i].name[j] = source->name[j];
        ++next.count;
    }
    *out = next;
    return true;
}

ShellLinenRuntime shell_models_linen_new(void) {
    ShellLinenRuntime state = {0};
    state.snapshot = shell_linen_snapshot_new();
    state.capture = linen_capture_client_new();
    state.publication_revision = 1;
    state.correlated_required = SHELL_LINEN_STRONG_DEFAULT != 0;
    state.text = shell_text_new(SURFACE_ID_LINEN, true);
    return state;
}

void shell_models_fetch_remote_snapshot(ShellLinenModelsContext *ctx) {
    if (ctx->linen->correlated_required) {
        if (!linen_capture_client_busy(&ctx->linen->capture)) {
            bool started = linen_capture_client_refresh(&ctx->linen->capture);
            ctx->models->LINEN_REMOTE_FETCHED = started;
            sexos_line("[linen.capture.begin] job=%llu accepted=%u required=1",
                       (unsigned long long)ctx->linen->capture.job, (unsigned)started);
        } else
            linen_capture_client_refresh(&ctx->linen->capture);
        return;
    }
    if (!shell_linen_snapshot_busy(&ctx->linen->snapshot)) {
        shell_linen_snapshot_refresh(&ctx->linen->snapshot, false);
        ctx->models->LINEN_REMOTE_FETCHED = true;
        sexos_line("[linen.remote.snapshot.begin] async=1");
    }
}

void shell_models_queue_open(ShellLinenModelsContext *ctx, uint64_t object, uint64_t choice) {
    bool queued = shell_linen_snapshot_queue_open(&ctx->linen->snapshot, object, choice);
    sexos_line("[linen.open.queued] id=%llu accepted=%u", (unsigned long long)object,
               (unsigned)queued);
}

void shell_models_linen_async_poll(ShellLinenModelsContext *ctx) {
    if (ctx->linen->correlated_required) {
        LinenCaptureStep step = linen_capture_transport_step(&ctx->linen->capture, SLOT_LINEN);
        if (step.admitted)
            ctx->linen->correlation_refused_logged = false;
        if (step.status == SEXOS_RPC_UNSUPPORTED && !ctx->linen->correlation_refused_logged) {
            ctx->linen->correlation_refused_logged = true;
            ctx->models->LINEN_REMOTE_FETCHED = false;
            sexos_line("[linen.capture.unavailable] required=1 fallback=0 status=%llu",
                       (unsigned long long)step.status);
        }
        if (step.event == LINEN_CLIENT_PUBLISHED) {
            if (shell_models_capture_snapshot(&ctx->linen->capture, &ctx->linen->capture_snapshot))
                shell_models_publish_capture_snapshot(ctx);
            else {
                ctx->models->LINEN_REMOTE_FETCHED = false;
                sexos_line("[linen.capture.decode.rejected] preserve_view=1");
            }
        } else if (step.event == LINEN_CLIENT_FAILED || step.event == LINEN_CLIENT_CANCELLED) {
            ctx->models->LINEN_REMOTE_FETCHED = false;
            sexos_line("[linen.capture.failed] event=%u preserve_view=1", (unsigned)step.event);
        }
        if (!linen_capture_client_busy(&ctx->linen->capture) && ctx->linen->capture.dirty)
            linen_capture_client_refresh(&ctx->linen->capture);
    }
    ShellLinenRequest request = {0};
    if (shell_linen_snapshot_request(&ctx->linen->snapshot, &request)) {
        SexosReply admission = sexos_pdx_call(SLOT_LINEN, request.opcode, request.args[0],
                                              request.args[1], request.args[2]);
        if (!admission.status)
            shell_linen_snapshot_sent(&ctx->linen->snapshot);
    }
}

bool shell_models_linen_async_reply(ShellLinenModelsContext *ctx, const SexosMessage *message) {
    if (message->type_id != 1)
        return false;
    if (message->caller_pd != 7) {
        if (shell_linen_snapshot_busy(&ctx->linen->snapshot))
            sexos_line("[linen.snapshot.foreign_reply] caller=%u ignored=1", message->caller_pd);
        return false;
    }
    ShellLinenReply reply = shell_linen_snapshot_reply(&ctx->linen->snapshot, message->caller_pd,
                                                       message->type_id, message->arg0);
    switch (reply.kind) {
    case SHELL_LINEN_PUBLISHED:
        shell_models_publish_snapshot(ctx);
        break;
    case SHELL_LINEN_REJECTED:
        ctx->models->LINEN_REMOTE_FETCHED = false;
        sexos_line("[linen.remote.snapshot.rejected] preserve_view=1 async=1");
        break;
    case SHELL_LINEN_OPEN_COMPLETE:
        sexos_line("[linen.open.complete] status=%llu", (unsigned long long)reply.value);
        break;
    default:
        break;
    }
    return true;
}


