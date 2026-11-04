/* Generated from a declared Cesta protocol. Declare the context and native hooks first. */

#ifdef CESTA_PROTOCOL_network_rx_run_DEFINED
#error "duplicate protocol definition"
#endif
#define CESTA_PROTOCOL_network_rx_run_DEFINED

#if defined(network_rx_run) || \
    defined(RxContext) || \
    defined(ctx) || \
    defined(cesta_remaining) || \
    defined(network_rx_run_outcome) || \
    defined(network_rx_run_declined) || \
    defined(network_rx_run_answered) || \
    defined(network_rx_run_done) || \
    defined(network_rx_run_raised) || \
    defined(network_rx_run_suspended) || \
    defined(network_rx_run_resource_failure) || \
    defined(network_rx_run_engine_failure) || \
    defined(rx_poll) || \
    defined(rx_valid) || \
    defined(rx_copy) || \
    defined(rx_discard) || \
    defined(rx_descriptor_acquire) || \
    defined(rx_descriptor_release) || \
    defined(rx_descriptor_transfer) || \
    defined(rx_bytes_acquire) || \
    defined(rx_bytes_release) || \
    defined(rx_bytes_transfer) || \
    defined(rx_binding) || \
    defined(rx_available) || \
    defined(rx_frame_fact) || \
    defined(rx_copied) || \
    defined(rx_settled) || \
    defined(rx_descriptor_owned) || \
    defined(rx_bytes_owned) || \
    defined(network_rx_run_decline_reason) || \
    defined(network_rx_run_decline_count) || \
    defined(network_rx_run_counters) || \
    defined(network_rx_run_count_decline) || \
    defined(network_rx_run_counts) || \
    defined(network_rx_run_reason) || \
    defined(network_rx_run_COUNTED)
#error "protocol identifier conflicts with a macro"
#endif

#ifndef network_rx_run_CHECKED
#define network_rx_run_CHECKED 0
#endif
#ifdef network_rx_run_COUNT_DECLINE
#define network_rx_run_COUNTED 1
#else
#define network_rx_run_COUNTED 0
#define network_rx_run_COUNT_DECLINE(ctx, reason) ((void)0)
#endif
#if network_rx_run_CHECKED
#ifndef network_rx_run_CHECK_FAILURE
#include <stdio.h>
#include <stdlib.h>
#define network_rx_run_CHECK_FAILURE(ctx, kind, id, expected) \
    (fprintf(stderr, "protocol network_rx_run: %s %s must be %s\n", \
             kind, id, (expected) ? "true" : "false"), abort())
#endif
#endif
#include <stdint.h>

typedef enum network_rx_run_decline_reason {
    network_rx_run_decline_count
} network_rx_run_decline_reason;

typedef struct network_rx_run_counters {
    uint64_t declines[network_rx_run_decline_count ? network_rx_run_decline_count : 1];
} network_rx_run_counters;

typedef enum network_rx_run_outcome {
    network_rx_run_declined,
    network_rx_run_answered,
    network_rx_run_done,
    network_rx_run_raised,
    network_rx_run_suspended,
    network_rx_run_resource_failure,
    network_rx_run_engine_failure
} network_rx_run_outcome;

static inline network_rx_run_outcome
network_rx_run(RxContext *ctx)
{
    (void)ctx;
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (rx_bytes_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
    if (!rx_poll(ctx)) {
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_done;
    }
#if network_rx_run_CHECKED
    if (!rx_available(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "available", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
        for (;;) {}
    }
#endif
    if (!rx_descriptor_acquire(ctx)) {
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_raised;
    }
#if network_rx_run_CHECKED
    if (!rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_available(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "available", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_binding(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
        for (;;) {}
    }
#endif
#if network_rx_run_CHECKED
    if (!rx_descriptor_owned(ctx)) {
        network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
        for (;;) {}
    }
#endif
    if (rx_valid(ctx)) {
#if network_rx_run_CHECKED
        if (!rx_frame_fact(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "valid_frame", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_binding(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        if (!rx_bytes_acquire(ctx)) {
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
                for (;;) {}
            }
#endif
            rx_descriptor_release(ctx);
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
            return network_rx_run_resource_failure;
        }
#if network_rx_run_CHECKED
        if (!rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_frame_fact(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "valid_frame", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_binding(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
            for (;;) {}
        }
#endif
        if (rx_copy(ctx)) {
#if network_rx_run_CHECKED
            if (!rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_copied(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "copied", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_binding(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
            rx_bytes_transfer(ctx);
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_settled(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "settled", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_binding(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
            rx_descriptor_transfer(ctx);
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_copied(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "copied", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_settled(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "settled", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
            return network_rx_run_answered;
        }
#if network_rx_run_CHECKED
        if (!rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 1);
            for (;;) {}
        }
#endif
        rx_bytes_release(ctx);
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
        rx_descriptor_release(ctx);
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_raised;
    } else {
#if network_rx_run_CHECKED
        if (!rx_available(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "available", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_binding(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
        if (rx_discard(ctx)) {
#if network_rx_run_CHECKED
            if (!rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_settled(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "settled", 1);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (!rx_binding(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "fact", "binding", 1);
                for (;;) {}
            }
#endif
            rx_descriptor_transfer(ctx);
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_descriptor_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
                for (;;) {}
            }
#endif
#if network_rx_run_CHECKED
            if (rx_bytes_owned(ctx)) {
                network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
                for (;;) {}
            }
#endif
            return network_rx_run_done;
        }
#if network_rx_run_CHECKED
        if (!rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 1);
            for (;;) {}
        }
#endif
        rx_descriptor_release(ctx);
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_descriptor_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "descriptor", 0);
            for (;;) {}
        }
#endif
#if network_rx_run_CHECKED
        if (rx_bytes_owned(ctx)) {
            network_rx_run_CHECK_FAILURE(ctx, "own", "bytes", 0);
            for (;;) {}
        }
#endif
        return network_rx_run_raised;
    }
}
