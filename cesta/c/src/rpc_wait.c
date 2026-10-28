#include "sexos_rpc.h"
typedef struct RpcWaitContext {
  uint64_t slot, opcode, arg0, arg1, arg2, budget, id, sealed;
  bool owned;
  SexosRequestResult result;
}
RpcWaitContext;
static bool rpc_wait_identity(RpcWaitContext *c) {
  return c->id != 0 && c->id == c->sealed;
}
static bool rpc_wait_owned(RpcWaitContext *c) {
  return c->owned && rpc_wait_identity(c);
}
static bool rpc_wait_complete(RpcWaitContext *c) {
  return c->result.status == SEXOS_REQUEST_COMPLETE;
}
static bool rpc_wait_acquire(RpcWaitContext *c) {
  if (c->owned || !rpc_wait_identity(c)) return false;
  c->owned = true;
  return rpc_wait_owned(c);
}
static bool rpc_admit_begin(RpcWaitContext *c) {
  SexosReply result = sexos_rpc_begin(c->slot, c->opcode, c->arg0, c->arg1, c->arg2);
  if (result.status == 0) {
    c->result = (SexosRequestResult) {
      SEXOS_REQUEST_COMPLETE, result.value, 0
    };
    return true;
  }
  c->result.status = result.status == SEXOS_RPC_UNSUPPORTED ? SEXOS_REQUEST_PENDING : SEXOS_REQUEST_FAILED;
  c->result.detail = result.status;
  return false;
}
static bool rpc_notify_send(RpcWaitContext *c) {
  SexosReply result = sexos_pdx_call(c->slot, c->opcode, c->arg0, c->arg1, c->arg2);
  c->result = (SexosRequestResult) {
    result.status == 0 ? SEXOS_REQUEST_COMPLETE : result.status == SEXOS_RPC_BUSY ? SEXOS_REQUEST_PENDING : SEXOS_REQUEST_FAILED, result.value, result.status
  };
  return c->result.status == SEXOS_REQUEST_COMPLETE;
}
static bool rpc_wait_poll(RpcWaitContext *c) {
  SexosReply result = sexos_rpc_poll(c->id);
  if (result.status == 0) c->result = (SexosRequestResult) {
    SEXOS_REQUEST_COMPLETE, result.value, 0
  };
  else {
    c->result.status = result.status == SEXOS_RPC_BUSY ? SEXOS_REQUEST_PENDING : SEXOS_REQUEST_FAILED;
    c->result.detail = result.status;
  }
  return rpc_wait_complete(c);
}
static bool rpc_wait_failed(RpcWaitContext *c) {
  return c->result.status == SEXOS_REQUEST_FAILED;
}
static bool rpc_wait_yield(RpcWaitContext *c) {
  (void) c;
  sexos_yield();
  return true;
}
static bool rpc_wait_timeout(RpcWaitContext *c) {
  c->result = (SexosRequestResult) {
    SEXOS_REQUEST_TIMEOUT, 0, 0
  };
  return true;
}
static void rpc_wait_release(RpcWaitContext *c) {
  (void) sexos_rpc_cancel(c->id);
  c->owned = false;
}
static void rpc_wait_transfer(RpcWaitContext *c) {
  c->owned = false;
}
#define rpc_admit_run_CHECKED 1
#define rpc_wait_run_CHECKED 1
#define rpc_notify_run_CHECKED 1
#define rpc_admit_run_CHECK_FAILURE(ctx, kind, id, expected) sexos_fail()
#define rpc_wait_run_CHECK_FAILURE(ctx, kind, id, expected) sexos_fail()
#define rpc_notify_run_CHECK_FAILURE(ctx, kind, id, expected) sexos_fail()

/* Generated from cesta-protocol/1. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_rpc_admit_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_rpc_admit_run_DEFINED

#if defined(rpc_admit_run) || \
    defined(RpcWaitContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(budget) || \
    defined(rpc_admit_run_outcome) || \
    defined(rpc_admit_run_declined) || \
    defined(rpc_admit_run_answered) || \
    defined(rpc_admit_run_done) || \
    defined(rpc_admit_run_raised) || \
    defined(rpc_admit_run_suspended) || \
    defined(rpc_admit_run_resource_failure) || \
    defined(rpc_admit_run_engine_failure) || \
    defined(rpc_admit_begin) || \
    defined(rpc_wait_failed) || \
    defined(rpc_wait_yield) || \
    defined(rpc_wait_timeout) || \
    defined(rpc_admit_run_decline_reason) || \
    defined(rpc_admit_run_decline_count) || \
    defined(rpc_admit_run_counters) || \
    defined(rpc_admit_run_count_decline) || \
    defined(rpc_admit_run_counts) || \
    defined(rpc_admit_run_reason) || \
    defined(rpc_admit_run_COUNTED)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef rpc_admit_run_CHECKED
#define rpc_admit_run_CHECKED 0
#endif
#ifdef rpc_admit_run_COUNT_DECLINE
#define rpc_admit_run_COUNTED 1
#else
#define rpc_admit_run_COUNTED 0
#define rpc_admit_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if rpc_admit_run_CHECKED
#ifndef rpc_admit_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define rpc_admit_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol rpc_admit_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum rpc_admit_run_decline_reason {
    rpc_admit_run_decline_count
} rpc_admit_run_decline_reason;

typedef struct rpc_admit_run_counters {
    uint64_t declines[rpc_admit_run_decline_count ? rpc_admit_run_decline_count : 1];
} rpc_admit_run_counters;

typedef enum rpc_admit_run_outcome {
    rpc_admit_run_declined,
    rpc_admit_run_answered,
    rpc_admit_run_done,
    rpc_admit_run_raised,
    rpc_admit_run_suspended,
    rpc_admit_run_resource_failure,
    rpc_admit_run_engine_failure
} rpc_admit_run_outcome;

static inline rpc_admit_run_outcome
rpc_admit_run(RpcWaitContext *ctx)
{
    (void)ctx;
    {
        _Static_assert(_Generic(&(ctx->budget), uint64_t *: 1, const uint64_t *: 1, default: 0), "protocol limit field must be nonvolatile uint64_t");
        for (uint64_t cesta_remaining = ctx->budget; cesta_remaining != 0; --cesta_remaining) {
            if (rpc_admit_begin(ctx)) {
                return rpc_admit_run_answered;
            }
            if (rpc_wait_failed(ctx)) {
                return rpc_admit_run_raised;
            }
            if (rpc_wait_yield(ctx)) {
                continue;
            }
            continue;
        }
        if (rpc_wait_timeout(ctx)) {
            return rpc_admit_run_raised;
        }
        return rpc_admit_run_raised;
    }
}

#ifdef CESTA_PROTOCOL_rpc_notify_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_rpc_notify_run_DEFINED

#if defined(rpc_notify_run) || \
    defined(RpcWaitContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(budget) || \
    defined(rpc_notify_run_outcome) || \
    defined(rpc_notify_run_declined) || \
    defined(rpc_notify_run_answered) || \
    defined(rpc_notify_run_done) || \
    defined(rpc_notify_run_raised) || \
    defined(rpc_notify_run_suspended) || \
    defined(rpc_notify_run_resource_failure) || \
    defined(rpc_notify_run_engine_failure) || \
    defined(rpc_notify_send) || \
    defined(rpc_wait_failed) || \
    defined(rpc_wait_yield) || \
    defined(rpc_wait_timeout) || \
    defined(rpc_notify_run_decline_reason) || \
    defined(rpc_notify_run_decline_count) || \
    defined(rpc_notify_run_counters) || \
    defined(rpc_notify_run_count_decline) || \
    defined(rpc_notify_run_counts) || \
    defined(rpc_notify_run_reason) || \
    defined(rpc_notify_run_COUNTED)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef rpc_notify_run_CHECKED
#define rpc_notify_run_CHECKED 0
#endif
#ifdef rpc_notify_run_COUNT_DECLINE
#define rpc_notify_run_COUNTED 1
#else
#define rpc_notify_run_COUNTED 0
#define rpc_notify_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if rpc_notify_run_CHECKED
#ifndef rpc_notify_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define rpc_notify_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol rpc_notify_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum rpc_notify_run_decline_reason {
    rpc_notify_run_decline_count
} rpc_notify_run_decline_reason;

typedef struct rpc_notify_run_counters {
    uint64_t declines[rpc_notify_run_decline_count ? rpc_notify_run_decline_count : 1];
} rpc_notify_run_counters;

typedef enum rpc_notify_run_outcome {
    rpc_notify_run_declined,
    rpc_notify_run_answered,
    rpc_notify_run_done,
    rpc_notify_run_raised,
    rpc_notify_run_suspended,
    rpc_notify_run_resource_failure,
    rpc_notify_run_engine_failure
} rpc_notify_run_outcome;

static inline rpc_notify_run_outcome
rpc_notify_run(RpcWaitContext *ctx)
{
    (void)ctx;
    {
        _Static_assert(_Generic(&(ctx->budget), uint64_t *: 1, const uint64_t *: 1, default: 0), "protocol limit field must be nonvolatile uint64_t");
        for (uint64_t cesta_remaining = ctx->budget; cesta_remaining != 0; --cesta_remaining) {
            if (rpc_notify_send(ctx)) {
                return rpc_notify_run_answered;
            }
            if (rpc_wait_failed(ctx)) {
                return rpc_notify_run_raised;
            }
            if (rpc_wait_yield(ctx)) {
                continue;
            }
            continue;
        }
        if (rpc_wait_timeout(ctx)) {
            return rpc_notify_run_raised;
        }
        return rpc_notify_run_raised;
    }
}

#ifdef CESTA_PROTOCOL_rpc_wait_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_rpc_wait_run_DEFINED

#if defined(rpc_wait_run) || \
    defined(RpcWaitContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(budget) || \
    defined(rpc_wait_run_outcome) || \
    defined(rpc_wait_run_declined) || \
    defined(rpc_wait_run_answered) || \
    defined(rpc_wait_run_done) || \
    defined(rpc_wait_run_raised) || \
    defined(rpc_wait_run_suspended) || \
    defined(rpc_wait_run_resource_failure) || \
    defined(rpc_wait_run_engine_failure) || \
    defined(rpc_wait_poll) || \
    defined(rpc_wait_failed) || \
    defined(rpc_wait_yield) || \
    defined(rpc_wait_timeout) || \
    defined(rpc_wait_acquire) || \
    defined(rpc_wait_release) || \
    defined(rpc_wait_transfer) || \
    defined(rpc_wait_identity) || \
    defined(rpc_wait_complete) || \
    defined(rpc_wait_owned) || \
    defined(rpc_wait_run_decline_reason) || \
    defined(rpc_wait_run_decline_count) || \
    defined(rpc_wait_run_counters) || \
    defined(rpc_wait_run_count_decline) || \
    defined(rpc_wait_run_counts) || \
    defined(rpc_wait_run_reason) || \
    defined(rpc_wait_run_COUNTED)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef rpc_wait_run_CHECKED
#define rpc_wait_run_CHECKED 0
#endif
#ifdef rpc_wait_run_COUNT_DECLINE
#define rpc_wait_run_COUNTED 1
#else
#define rpc_wait_run_COUNTED 0
#define rpc_wait_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if rpc_wait_run_CHECKED
#ifndef rpc_wait_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define rpc_wait_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol rpc_wait_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum rpc_wait_run_decline_reason {
    rpc_wait_run_decline_count
} rpc_wait_run_decline_reason;

typedef struct rpc_wait_run_counters {
    uint64_t declines[rpc_wait_run_decline_count ? rpc_wait_run_decline_count : 1];
} rpc_wait_run_counters;

typedef enum rpc_wait_run_outcome {
    rpc_wait_run_declined,
    rpc_wait_run_answered,
    rpc_wait_run_done,
    rpc_wait_run_raised,
    rpc_wait_run_suspended,
    rpc_wait_run_resource_failure,
    rpc_wait_run_engine_failure
} rpc_wait_run_outcome;

static inline rpc_wait_run_outcome
rpc_wait_run(RpcWaitContext *ctx)
{
    (void)ctx;
#if rpc_wait_run_CHECKED
    if (!rpc_wait_identity(ctx)) {
        rpc_wait_run_CHECK_FAILURE(ctx, "fact", "identity", 1);
        for (;;) {}
    }
#endif
#if rpc_wait_run_CHECKED
    if (!rpc_wait_owned(ctx)) {
        rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
        for (;;) {}
    }
#endif
    {
        _Static_assert(_Generic(&(ctx->budget), uint64_t *: 1, const uint64_t *: 1, default: 0), "protocol limit field must be nonvolatile uint64_t");
        for (uint64_t cesta_remaining = ctx->budget; cesta_remaining != 0; --cesta_remaining) {
#if rpc_wait_run_CHECKED
            if (!rpc_wait_identity(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "fact", "identity", 1);
                for (;;) {}
            }
#endif
#if rpc_wait_run_CHECKED
            if (!rpc_wait_owned(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                for (;;) {}
            }
#endif
            if (rpc_wait_poll(ctx)) {
#if rpc_wait_run_CHECKED
                if (!rpc_wait_owned(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                    for (;;) {}
                }
#endif
#if rpc_wait_run_CHECKED
                if (!rpc_wait_complete(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "fact", "complete", 1);
                    for (;;) {}
                }
#endif
                rpc_wait_transfer(ctx);
#if rpc_wait_run_CHECKED
                if (rpc_wait_owned(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
                    for (;;) {}
                }
#endif
#if rpc_wait_run_CHECKED
                if (rpc_wait_owned(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
                    for (;;) {}
                }
#endif
                return rpc_wait_run_answered;
            }
#if rpc_wait_run_CHECKED
            if (!rpc_wait_identity(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "fact", "identity", 1);
                for (;;) {}
            }
#endif
#if rpc_wait_run_CHECKED
            if (!rpc_wait_owned(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                for (;;) {}
            }
#endif
            if (rpc_wait_failed(ctx)) {
#if rpc_wait_run_CHECKED
                if (!rpc_wait_owned(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                    for (;;) {}
                }
#endif
                rpc_wait_release(ctx);
#if rpc_wait_run_CHECKED
                if (rpc_wait_owned(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
                    for (;;) {}
                }
#endif
#if rpc_wait_run_CHECKED
                if (rpc_wait_owned(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
                    for (;;) {}
                }
#endif
                return rpc_wait_run_raised;
            }
#if rpc_wait_run_CHECKED
            if (!rpc_wait_identity(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "fact", "identity", 1);
                for (;;) {}
            }
#endif
#if rpc_wait_run_CHECKED
            if (!rpc_wait_owned(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                for (;;) {}
            }
#endif
            if (rpc_wait_yield(ctx)) {
#if rpc_wait_run_CHECKED
                if (!rpc_wait_identity(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "fact", "identity", 1);
                    for (;;) {}
                }
#endif
#if rpc_wait_run_CHECKED
                if (!rpc_wait_owned(ctx)) {
                    rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                    for (;;) {}
                }
#endif
                continue;
            }
#if rpc_wait_run_CHECKED
            if (!rpc_wait_identity(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "fact", "identity", 1);
                for (;;) {}
            }
#endif
#if rpc_wait_run_CHECKED
            if (!rpc_wait_owned(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                for (;;) {}
            }
#endif
            continue;
        }
#if rpc_wait_run_CHECKED
        if (!rpc_wait_identity(ctx)) {
            rpc_wait_run_CHECK_FAILURE(ctx, "fact", "identity", 1);
            for (;;) {}
        }
#endif
#if rpc_wait_run_CHECKED
        if (!rpc_wait_owned(ctx)) {
            rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
            for (;;) {}
        }
#endif
        if (rpc_wait_timeout(ctx)) {
#if rpc_wait_run_CHECKED
            if (!rpc_wait_owned(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
                for (;;) {}
            }
#endif
            rpc_wait_release(ctx);
#if rpc_wait_run_CHECKED
            if (rpc_wait_owned(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
                for (;;) {}
            }
#endif
#if rpc_wait_run_CHECKED
            if (rpc_wait_owned(ctx)) {
                rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
                for (;;) {}
            }
#endif
            return rpc_wait_run_raised;
        }
#if rpc_wait_run_CHECKED
        if (!rpc_wait_owned(ctx)) {
            rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 1);
            for (;;) {}
        }
#endif
        rpc_wait_release(ctx);
#if rpc_wait_run_CHECKED
        if (rpc_wait_owned(ctx)) {
            rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
            for (;;) {}
        }
#endif
#if rpc_wait_run_CHECKED
        if (rpc_wait_owned(ctx)) {
            rpc_wait_run_CHECK_FAILURE(ctx, "own", "request", 0);
            for (;;) {}
        }
#endif
        return rpc_wait_run_raised;
    }
}
SexosRequestResult sexos_rpc_admit_wait(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t budget) {
  RpcWaitContext context = {
    .slot = slot,.opcode = opcode,.arg0 = arg0,.arg1 = arg1,.arg2 = arg2,.budget = budget
  };
  (void) rpc_admit_run(&context);
  return context.result;
}
SexosRequestResult sexos_rpc_await(uint64_t id, uint64_t budget) {
  RpcWaitContext context = {
    .id = id,.sealed = id,.budget = budget,.result = {
      SEXOS_REQUEST_PENDING, 0, 0
    }
  };
  if (!rpc_wait_acquire(&context)) return (SexosRequestResult) {
    SEXOS_REQUEST_FAILED, 0, SEXOS_RPC_INVALID
  };
  (void) rpc_wait_run(&context);
  return context.result;
}
SexosRequestResult sexos_send(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t budget) {
  RpcWaitContext context = {
    .slot = slot,.opcode = opcode,.arg0 = arg0,.arg1 = arg1,.arg2 = arg2,.budget = budget
  };
  (void) rpc_notify_run(&context);
  return context.result;
}
