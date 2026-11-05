#include "network_service.h"
typedef struct TxContext {
  NetworkDevice *device;
  cesta_read_bytes frame;
  uint64_t budget, identity;
  bool held, submitted, attempted;
  NetworkStep step;
}
TxContext;
static bool tx_frame_fact(TxContext *c) {
  return c->frame.data && c->frame.len >= 14 && c->frame.len <= 1514;
}
static bool tx_valid(TxContext *c) {
  return !c->attempted && tx_frame_fact(c);
}
static bool tx_permit(TxContext *c) {
  return !c->attempted;
}
static bool tx_submitted(TxContext *c) {
  return c->submitted;
}
static bool tx_complete(TxContext *c) {
  return c->step == NET_READY;
}
static bool tx_owned(TxContext *c) {
  return c->held;
}
static bool tx_binding(TxContext *c) {
  return !c->held || c->device->tx_page == c->identity;
}
static bool tx_acquire(TxContext *c) {
  if (c->device->failed || c->device->tx_busy || c->held) return false;
  c->identity = c->device->tx_page;
  c->held = true;
  return tx_owned(c);
}
static bool tx_submit(TxContext *c) {
  if (c->attempted) return false;
  c->attempted = true;
  c->submitted = network_device_submit(c->device, c->frame);
  return c->submitted;
}
static bool tx_ready(TxContext *c) {
  c->step = network_device_poll_tx(c->device);
  return tx_complete(c);
}
static bool tx_failed(TxContext *c) {
  return c->step == NET_FAILED;
}
static bool tx_yield(TxContext *c) {
  (void) c;
  sexos_yield();
  return true;
}
static bool tx_timeout(TxContext *c) {
  c->device->failed = true;
  c->device->failure = NET_TIMEOUT;
  c->step = NET_FAILED;
  return true;
}
static void tx_release(TxContext *c) {
  c->held = false;
}
static void tx_transfer(TxContext *c) {
  c->held = false;
}
#define network_tx_run_CHECKED 1
#define network_tx_run_CHECK_FAILURE(ctx,kind,id,expected) sexos_fail()
/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_network_tx_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_network_tx_run_DEFINED

#if defined(network_tx_run) || \
    defined(TxContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(budget) || \
    defined(network_tx_run_outcome) || \
    defined(network_tx_run_declined) || \
    defined(network_tx_run_answered) || \
    defined(network_tx_run_done) || \
    defined(network_tx_run_raised) || \
    defined(network_tx_run_suspended) || \
    defined(network_tx_run_resource_failure) || \
    defined(network_tx_run_engine_failure) || \
    defined(tx_valid) || \
    defined(tx_submit) || \
    defined(tx_ready) || \
    defined(tx_failed) || \
    defined(tx_yield) || \
    defined(tx_timeout) || \
    defined(tx_acquire) || \
    defined(tx_release) || \
    defined(tx_transfer) || \
    defined(tx_frame_fact) || \
    defined(tx_permit) || \
    defined(tx_submitted) || \
    defined(tx_complete) || \
    defined(tx_binding) || \
    defined(tx_owned) || \
    defined(network_tx_run_decline_reason) || \
    defined(network_tx_run_decline_count) || \
    defined(network_tx_run_counters) || \
    defined(network_tx_run_count_decline) || \
    defined(network_tx_run_counts) || \
    defined(network_tx_run_reason) || \
    defined(network_tx_run_COUNTED) || \
    defined(network_tx_run_declined_after_tx_valid_false)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef network_tx_run_CHECKED
#define network_tx_run_CHECKED 0
#endif
#ifdef network_tx_run_COUNT_DECLINE
#define network_tx_run_COUNTED 1
#else
#define network_tx_run_COUNTED 0
#define network_tx_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if network_tx_run_CHECKED
#ifndef network_tx_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define network_tx_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol network_tx_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum network_tx_run_decline_reason {
    network_tx_run_declined_after_tx_valid_false,
    network_tx_run_decline_count
} network_tx_run_decline_reason;

typedef struct network_tx_run_counters {
    uint64_t declines[network_tx_run_decline_count ? network_tx_run_decline_count : 1];
} network_tx_run_counters;

#if network_tx_run_COUNTED
static inline void
network_tx_run_count_decline(network_tx_run_counters *network_tx_run_counts, network_tx_run_decline_reason network_tx_run_reason)
{
    if (network_tx_run_counts && (unsigned)network_tx_run_reason < (unsigned)network_tx_run_decline_count &&
        network_tx_run_counts->declines[network_tx_run_reason] != UINT64_MAX)
        ++network_tx_run_counts->declines[network_tx_run_reason];
}
#endif

typedef enum network_tx_run_outcome {
    network_tx_run_declined,
    network_tx_run_answered,
    network_tx_run_done,
    network_tx_run_raised,
    network_tx_run_suspended,
    network_tx_run_resource_failure,
    network_tx_run_engine_failure
} network_tx_run_outcome;

static inline network_tx_run_outcome
network_tx_run(TxContext *ctx)
{
    (void)ctx;
#if network_tx_run_CHECKED
    if (!tx_binding(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (tx_owned(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (!tx_binding(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
    if (!tx_valid(ctx)) {
#if network_tx_run_CHECKED
        if (tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
            for (;;) {}
        }
#endif
        network_tx_run_COUNT_DECLINE(ctx, network_tx_run_declined_after_tx_valid_false);
        return network_tx_run_declined;
    }
#if network_tx_run_CHECKED
    if (!tx_frame_fact(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "fact", "valid_frame", 1);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (!tx_binding(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (tx_owned(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
        for (;;) {}
    }
#endif
    if (!tx_acquire(ctx)) {
#if network_tx_run_CHECKED
        if (tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
            for (;;) {}
        }
#endif
#if network_tx_run_CHECKED
        if (tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
            for (;;) {}
        }
#endif
        return network_tx_run_raised;
    }
#if network_tx_run_CHECKED
    if (!tx_owned(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (!tx_frame_fact(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "fact", "valid_frame", 1);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (!tx_permit(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "fact", "effect_permit", 1);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (!tx_binding(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_tx_run_CHECKED
    if (!tx_owned(ctx)) {
        network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
        for (;;) {}
    }
#endif
    if (!tx_submit(ctx)) {
#if network_tx_run_CHECKED
        if (!tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
            for (;;) {}
        }
#endif
        tx_release(ctx);
#if network_tx_run_CHECKED
        if (tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
            for (;;) {}
        }
#endif
#if network_tx_run_CHECKED
        if (tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
            for (;;) {}
        }
#endif
        return network_tx_run_raised;
    }
    {
        _Static_assert(_Generic(&(ctx->budget), uint64_t *: 1, const uint64_t *: 1, default: 0), "protocol limit field must be nonvolatile uint64_t");
        for (uint64_t cesta_remaining = ctx->budget; cesta_remaining != 0; --cesta_remaining) {
#if network_tx_run_CHECKED
            if (!tx_submitted(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_binding(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_owned(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                for (;;) {}
            }
#endif
            if (tx_ready(ctx)) {
#if network_tx_run_CHECKED
                if (!tx_owned(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (!tx_submitted(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (!tx_binding(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                    for (;;) {}
                }
#endif
                tx_transfer(ctx);
#if network_tx_run_CHECKED
                if (tx_owned(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (!tx_complete(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "fact", "complete", 1);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (tx_owned(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
                    for (;;) {}
                }
#endif
                return network_tx_run_answered;
            }
#if network_tx_run_CHECKED
            if (!tx_submitted(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_binding(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_owned(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                for (;;) {}
            }
#endif
            if (tx_failed(ctx)) {
#if network_tx_run_CHECKED
                if (!tx_owned(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (!tx_submitted(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (!tx_binding(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                    for (;;) {}
                }
#endif
                tx_transfer(ctx);
#if network_tx_run_CHECKED
                if (tx_owned(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (tx_owned(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
                    for (;;) {}
                }
#endif
                return network_tx_run_raised;
            }
#if network_tx_run_CHECKED
            if (!tx_submitted(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_binding(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_owned(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                for (;;) {}
            }
#endif
            if (tx_yield(ctx)) {
#if network_tx_run_CHECKED
                if (!tx_binding(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (!tx_submitted(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                    for (;;) {}
                }
#endif
#if network_tx_run_CHECKED
                if (!tx_owned(ctx)) {
                    network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                    for (;;) {}
                }
#endif
                continue;
            }
#if network_tx_run_CHECKED
            if (!tx_binding(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_submitted(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_owned(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                for (;;) {}
            }
#endif
            continue;
        }
#if network_tx_run_CHECKED
        if (!tx_submitted(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
            for (;;) {}
        }
#endif
#if network_tx_run_CHECKED
        if (!tx_binding(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
#if network_tx_run_CHECKED
        if (!tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
            for (;;) {}
        }
#endif
        if (tx_timeout(ctx)) {
#if network_tx_run_CHECKED
            if (!tx_owned(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_submitted(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (!tx_binding(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
            tx_transfer(ctx);
#if network_tx_run_CHECKED
            if (tx_owned(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
                for (;;) {}
            }
#endif
#if network_tx_run_CHECKED
            if (tx_owned(ctx)) {
                network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
                for (;;) {}
            }
#endif
            return network_tx_run_raised;
        }
#if network_tx_run_CHECKED
        if (!tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 1);
            for (;;) {}
        }
#endif
#if network_tx_run_CHECKED
        if (!tx_submitted(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "fact", "submitted", 1);
            for (;;) {}
        }
#endif
#if network_tx_run_CHECKED
        if (!tx_binding(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
        tx_transfer(ctx);
#if network_tx_run_CHECKED
        if (tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
            for (;;) {}
        }
#endif
#if network_tx_run_CHECKED
        if (tx_owned(ctx)) {
            network_tx_run_CHECK_FAILURE(ctx, "own", "frame", 0);
            for (;;) {}
        }
#endif
        return network_tx_run_raised;
    }
}
bool network_device_send(NetworkDevice *device, cesta_read_bytes frame) {
  TxContext context = {
    .device = device,.frame = frame,.budget = 65536,.step = NET_PENDING
  };
  (void) network_tx_run(&context);
  return context.step == NET_READY;
}
