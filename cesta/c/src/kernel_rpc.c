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
#include "sexos_kernel.h"
/*Correlated calls are separate from the legacy reply slot. Each caller owns a bounded set of outstanding requests. Only the peer that received a request may complete it; polling or cancellation consumes its identity.*/
static RpcSlot *rpc_find(Domain *caller, uint64_t id) {
  if (id == 0) return NULL;
  for (uint32_t i = 0; i < KERNEL_RPC_CAPACITY; ++i) if (caller->rpc[i].state != RPC_FREE && caller->rpc[i].id == id) return &caller->rpc[i];
  return NULL;
}
static void rpc_call(Domain *caller, TrapFrame *f) {
  const Cap *cap = cap_get(caller, f->rdi);
  f->rax = ERR_CAP_INVALID;
  uint64_t opcode = f->rsi;
  f->rsi = 0;
  if (opcode == 0) return;
  if (cap == NULL || cap->kind != CAP_DOMAIN) return;
  Domain *peer = domain_get(cap->target);
  if (peer == NULL) return;
  uint32_t task = SEXOS_KERNEL_CURRENT_PDX ? kernel_current_task(caller) : 0;
  if (SEXOS_KERNEL_CURRENT_PDX && task == 0) return;
  if (!peer->rpc_enabled) {
    f->rax = NO_ADDRESS;
    return;
  }
  f->rax = ERR_SERVICE_NOT_READY;
  if (caller->rpc_next == UINT64_MAX) return;
  RpcSlot *vacant = NULL;
  for (uint32_t i = 0; i < KERNEL_RPC_CAPACITY; ++i) if (caller->rpc[i].state == RPC_FREE) {
    vacant = &caller->rpc[i];
    break;
  }
  if (vacant == NULL) return;
  uint64_t id = caller->rpc_next + 1;
  RingEntry message = (RingEntry) {
    .type_id = opcode,.caller = caller->id,.sender = caller->id,.caller_task = SEXOS_KERNEL_CURRENT_PDX ? task : 0,.request_id = id,.arg0 = f->rdx,.arg1 = f->r10,.arg2 = f->r8
  };
  if (!domain_enqueue(peer, message)) return;
  *vacant = (RpcSlot) {
    .id = id,.peer = peer->id,.state = RPC_QUEUED
  };
  caller->rpc_next = id;
  f->rsi = id;
  f->rax = 0;
}
bool kernel_rpc_claim(Domain *receiver, const RingEntry *message) {
  Domain *caller = domain_get(message->sender);
  if (caller == NULL) return false;
  RpcSlot *request = rpc_find(caller, message->request_id);
  if (request == NULL || (request->peer != receiver->id || request->state != RPC_QUEUED)) return false;
  request->state = RPC_HEARD;
  return true;
}
/*A heard request is a certificate invalidated by completion. Polling copies the value before consuming the ready slot; publication uses that copy rather than a stale slot reference.*/

typedef struct KernelReplyContext {
    Domain *peer, *caller;
    TrapFrame *frame;
    RpcSlot *request;
    uint64_t id;
} KernelReplyContext;

static bool kernel_reply_is_heard(KernelReplyContext *c) {
    return c->request && c->request->id == c->id && c->request->peer == c->peer->id &&
           c->request->state == RPC_HEARD;
}

static bool kernel_reply_is_ready(KernelReplyContext *c) {
    return c->request && c->request->id == c->id && c->request->peer == c->peer->id &&
           c->request->state == RPC_READY;
}

static bool kernel_reply_inspect(KernelReplyContext *c) {
    if (c->frame->rdi >= KERNEL_MAX_DOMAINS)
        return false;
    c->caller = domain_get((uint32_t)c->frame->rdi);
    if (!c->caller)
        return false;
    c->id = c->frame->rsi;
    c->request = rpc_find(c->caller, c->id);
    return kernel_reply_is_heard(c);
}

static bool kernel_reply_complete(KernelReplyContext *c) {
    if (!kernel_reply_is_heard(c))
        return false;
    c->request->value = c->frame->rdx;
    c->request->state = RPC_READY;
    if (SEXOS_KERNEL_CURRENT_PDX)
        kernel_task_wake(c->caller);
    return true;
}

static bool kernel_reply_publish(KernelReplyContext *c) {
    if (!kernel_reply_is_ready(c))
        return false;
    c->frame->rax = 0;
    return true;
}

typedef struct KernelPollContext {
    Domain *caller;
    TrapFrame *frame;
    RpcSlot *request;
    uint64_t id, value;
    bool copied;
} KernelPollContext;

static bool kernel_poll_is_ready(KernelPollContext *c) {
    return c->request && c->request->id == c->id && c->request->state == RPC_READY;
}

static bool kernel_poll_is_copied(KernelPollContext *c) {
    return c->copied;
}

static bool kernel_poll_inspect(KernelPollContext *c) {
    c->id = c->frame->rdi;
    c->request = rpc_find(c->caller, c->id);
    if (!c->request)
        return false;
    c->frame->rax = ERR_SERVICE_NOT_READY;
    return kernel_poll_is_ready(c);
}

static bool kernel_poll_consume(KernelPollContext *c) {
    if (!kernel_poll_is_ready(c))
        return false;
    c->value = c->request->value;
    *c->request = (RpcSlot){.state = RPC_FREE};
    c->copied = true;
    return true;
}

static bool kernel_poll_publish(KernelPollContext *c) {
    if (!c->copied)
        return false;
    c->frame->rsi = c->value;
    c->frame->rax = 0;
    return true;
}

#define kernel_reply_run_CHECKED 1
#define kernel_poll_run_CHECKED 1
#define kernel_reply_run_CHECK_FAILURE(ctx, kind, id, expected)                                    \
    kernel_halt("reply protocol invariant")
#define kernel_poll_run_CHECK_FAILURE(ctx, kind, id, expected)                                     \
    kernel_halt("poll protocol invariant")

/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_reply_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_reply_run_DEFINED

#if defined(kernel_reply_run) || \
    defined(KernelReplyContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_reply_run_outcome) || \
    defined(kernel_reply_run_declined) || \
    defined(kernel_reply_run_answered) || \
    defined(kernel_reply_run_done) || \
    defined(kernel_reply_run_raised) || \
    defined(kernel_reply_run_suspended) || \
    defined(kernel_reply_run_resource_failure) || \
    defined(kernel_reply_run_engine_failure) || \
    defined(kernel_reply_inspect) || \
    defined(kernel_reply_complete) || \
    defined(kernel_reply_publish) || \
    defined(kernel_reply_is_heard) || \
    defined(kernel_reply_is_ready) || \
    defined(kernel_reply_run_decline_reason) || \
    defined(kernel_reply_run_decline_count) || \
    defined(kernel_reply_run_counters) || \
    defined(kernel_reply_run_count_decline) || \
    defined(kernel_reply_run_counts) || \
    defined(kernel_reply_run_reason) || \
    defined(kernel_reply_run_COUNTED) || \
    defined(kernel_reply_run_declined_after_kernel_reply_inspect_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_reply_run_CHECKED
#define kernel_reply_run_CHECKED 0
#endif
#ifdef kernel_reply_run_COUNT_DECLINE
#define kernel_reply_run_COUNTED 1
#else
#define kernel_reply_run_COUNTED 0
#define kernel_reply_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_reply_run_CHECKED
#ifndef kernel_reply_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_reply_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_reply_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_reply_run_decline_reason {
    kernel_reply_run_declined_after_kernel_reply_inspect_false,
    kernel_reply_run_decline_count
} kernel_reply_run_decline_reason;

typedef struct kernel_reply_run_counters {
    uint64_t declines[kernel_reply_run_decline_count ? kernel_reply_run_decline_count : 1];
} kernel_reply_run_counters;

#if kernel_reply_run_COUNTED
static inline void
kernel_reply_run_count_decline(kernel_reply_run_counters *kernel_reply_run_counts, kernel_reply_run_decline_reason kernel_reply_run_reason)
{
    if (kernel_reply_run_counts && (unsigned)kernel_reply_run_reason < (unsigned)kernel_reply_run_decline_count &&
        kernel_reply_run_counts->declines[kernel_reply_run_reason] != UINT64_MAX)
        ++kernel_reply_run_counts->declines[kernel_reply_run_reason];
}
#endif

typedef enum kernel_reply_run_outcome {
    kernel_reply_run_declined,
    kernel_reply_run_answered,
    kernel_reply_run_done,
    kernel_reply_run_raised,
    kernel_reply_run_suspended,
    kernel_reply_run_resource_failure,
    kernel_reply_run_engine_failure
} kernel_reply_run_outcome;

static inline kernel_reply_run_outcome
kernel_reply_run(KernelReplyContext *ctx)
{
    (void)ctx;
    if (!kernel_reply_inspect(ctx)) {
        kernel_reply_run_COUNT_DECLINE(ctx, kernel_reply_run_declined_after_kernel_reply_inspect_false);
        return kernel_reply_run_declined;
    }
#if kernel_reply_run_CHECKED
    if (!kernel_reply_is_heard(ctx)) {
        kernel_reply_run_CHECK_FAILURE(ctx, "fact", "heard", 1);
        for (;;) {}
    }
#endif
    if (!kernel_reply_complete(ctx)) {
        return kernel_reply_run_engine_failure;
    }
#if kernel_reply_run_CHECKED
    if (!kernel_reply_is_ready(ctx)) {
        kernel_reply_run_CHECK_FAILURE(ctx, "fact", "ready", 1);
        for (;;) {}
    }
#endif
    if (kernel_reply_publish(ctx)) {
        return kernel_reply_run_done;
    }
    return kernel_reply_run_engine_failure;
}
/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_kernel_poll_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_kernel_poll_run_DEFINED

#if defined(kernel_poll_run) || \
    defined(KernelPollContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(kernel_poll_run_outcome) || \
    defined(kernel_poll_run_declined) || \
    defined(kernel_poll_run_answered) || \
    defined(kernel_poll_run_done) || \
    defined(kernel_poll_run_raised) || \
    defined(kernel_poll_run_suspended) || \
    defined(kernel_poll_run_resource_failure) || \
    defined(kernel_poll_run_engine_failure) || \
    defined(kernel_poll_inspect) || \
    defined(kernel_poll_consume) || \
    defined(kernel_poll_publish) || \
    defined(kernel_poll_is_ready) || \
    defined(kernel_poll_is_copied) || \
    defined(kernel_poll_run_decline_reason) || \
    defined(kernel_poll_run_decline_count) || \
    defined(kernel_poll_run_counters) || \
    defined(kernel_poll_run_count_decline) || \
    defined(kernel_poll_run_counts) || \
    defined(kernel_poll_run_reason) || \
    defined(kernel_poll_run_COUNTED) || \
    defined(kernel_poll_run_declined_after_kernel_poll_inspect_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef kernel_poll_run_CHECKED
#define kernel_poll_run_CHECKED 0
#endif
#ifdef kernel_poll_run_COUNT_DECLINE
#define kernel_poll_run_COUNTED 1
#else
#define kernel_poll_run_COUNTED 0
#define kernel_poll_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if kernel_poll_run_CHECKED
#ifndef kernel_poll_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define kernel_poll_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol kernel_poll_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum kernel_poll_run_decline_reason {
    kernel_poll_run_declined_after_kernel_poll_inspect_false,
    kernel_poll_run_decline_count
} kernel_poll_run_decline_reason;

typedef struct kernel_poll_run_counters {
    uint64_t declines[kernel_poll_run_decline_count ? kernel_poll_run_decline_count : 1];
} kernel_poll_run_counters;

#if kernel_poll_run_COUNTED
static inline void
kernel_poll_run_count_decline(kernel_poll_run_counters *kernel_poll_run_counts, kernel_poll_run_decline_reason kernel_poll_run_reason)
{
    if (kernel_poll_run_counts && (unsigned)kernel_poll_run_reason < (unsigned)kernel_poll_run_decline_count &&
        kernel_poll_run_counts->declines[kernel_poll_run_reason] != UINT64_MAX)
        ++kernel_poll_run_counts->declines[kernel_poll_run_reason];
}
#endif

typedef enum kernel_poll_run_outcome {
    kernel_poll_run_declined,
    kernel_poll_run_answered,
    kernel_poll_run_done,
    kernel_poll_run_raised,
    kernel_poll_run_suspended,
    kernel_poll_run_resource_failure,
    kernel_poll_run_engine_failure
} kernel_poll_run_outcome;

static inline kernel_poll_run_outcome
kernel_poll_run(KernelPollContext *ctx)
{
    (void)ctx;
    if (!kernel_poll_inspect(ctx)) {
        kernel_poll_run_COUNT_DECLINE(ctx, kernel_poll_run_declined_after_kernel_poll_inspect_false);
        return kernel_poll_run_declined;
    }
#if kernel_poll_run_CHECKED
    if (!kernel_poll_is_ready(ctx)) {
        kernel_poll_run_CHECK_FAILURE(ctx, "fact", "ready", 1);
        for (;;) {}
    }
#endif
    if (!kernel_poll_consume(ctx)) {
        return kernel_poll_run_engine_failure;
    }
#if kernel_poll_run_CHECKED
    if (!kernel_poll_is_copied(ctx)) {
        kernel_poll_run_CHECK_FAILURE(ctx, "fact", "copied", 1);
        for (;;) {}
    }
#endif
    if (kernel_poll_publish(ctx)) {
        return kernel_poll_run_done;
    }
    return kernel_poll_run_engine_failure;
}
static void rpc_poll(Domain *caller, TrapFrame *f) {
  f->rsi = 0;
  f->rax = ERR_CAP_INVALID;
  KernelPollContext context = (KernelPollContext) {
    .caller = caller,.frame = f
  };
  (void) kernel_poll_run(&context);
}
static void rpc_reply(Domain *peer, TrapFrame *f) {
  f->rax = ERR_CAP_INVALID;
  KernelReplyContext context = (KernelReplyContext) {
    .peer = peer,.frame = f
  };
  (void) kernel_reply_run(&context);
}
void kernel_rpc_syscall(Domain *d, TrapFrame *f, uint64_t number) {
  switch (number) {
    case SEXOS_RPC_register: {
      d->rpc_enabled = true;
      f->rax = 1;
    }
    break;
    case SEXOS_RPC_call: rpc_call(d, f);
    break;
    case SEXOS_RPC_poll: rpc_poll(d, f);
    break;
    case SEXOS_RPC_listen: {
      RingEntry message = kernel_listen_message(d, f->rdi);
      f->rax = message.type_id;
      f->rsi = message.caller;
      f->rdx = message.arg0;
      f->r10 = message.arg1;
      f->r8 = message.arg2;
      if (SEXOS_KERNEL_CURRENT_PDX) f->rdi = message.caller_task;
      f->r9 = message.request_id;
    }
    break;
    case SEXOS_RPC_reply: rpc_reply(d, f);
    break;
    case SEXOS_RPC_cancel: {
      RpcSlot *request = rpc_find(d, f->rdi);
      f->rax = ERR_CAP_INVALID;
      if (request != NULL) {
        *request = (RpcSlot) {
          .state = RPC_FREE
        };
        f->rax = 0;
      }
    }
    break;
    default: f->rax = NO_ADDRESS;
  }
}
