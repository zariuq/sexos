#include "kaleidoscope_bitmap.h"
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

static bool bitmap_geometry_valid(KaleidoscopeBitmapGeometry geometry) {
    return geometry.width >= 1 && geometry.width <= 48 && geometry.height >= 1 &&
           geometry.height <= 48 && geometry.scale >= 1 && geometry.scale <= 4;
}

static size_t bitmap_geometry_len(KaleidoscopeBitmapGeometry geometry) {
    return (size_t)geometry.width * geometry.height;
}

static bool bitmap_geometry_equal(KaleidoscopeBitmapGeometry a, KaleidoscopeBitmapGeometry b) {
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height &&
           a.scale == b.scale;
}

static bool bitmap_metadata_equal(KaleidoscopeBitmapMetadata a, KaleidoscopeBitmapMetadata b) {
    return a.viewport == b.viewport && a.source == b.source;
}

static void bitmap_erase_unsent(KaleidoscopeBitmapBatch *batch) {
    if (batch->unsent_present && batch->unsent.kind == KALEIDOSCOPE_BITMAP_REQUEST_CHUNK) {
        batch->unsent.pixels[0] = 0;
        batch->unsent.pixels[1] = 0;
    }
}

static void bitmap_drop(KaleidoscopeBitmapBatch *batch, bool *present) {
    bitmap_erase_unsent(batch);
    *batch = (KaleidoscopeBitmapBatch){0};
    *present = false;
}

static KaleidoscopeBitmapBatch bitmap_batch(uint32_t sequence, bool geometry_present,
                                            KaleidoscopeBitmapGeometry geometry, uint64_t hash,
                                            KaleidoscopeBitmapMetadata metadata) {
    KaleidoscopeBitmapBatch batch = {.receipt = {.sequence = sequence,
                                                 .geometry_present = geometry_present,
                                                 .geometry = geometry,
                                                 .hash = hash,
                                                 .metadata = metadata},
                                     .phase = geometry_present ? KALEIDOSCOPE_BITMAP_PHASE_BEGIN
                                                               : KALEIDOSCOPE_BITMAP_PHASE_CLEAR};
    return batch;
}

void kaleidoscope_bitmap_init(KaleidoscopeBitmapOutbox *outbox, KaleidoscopeBitmapIdentity identity,
                              uint32_t sequence_floor) {
    if (outbox)
        *outbox = (KaleidoscopeBitmapOutbox){.identity = identity, .sequence = sequence_floor};
}

uint64_t kaleidoscope_bitmap_hash(const KaleidoscopeQoiPixels *pixels, size_t count) {
    if (!pixels || count > KALEIDOSCOPE_BITMAP_CAPACITY) {
        return 0;
    }
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (size_t i = 0; i < count; i++)
        for (size_t k = 0; k < 4; k++) {
            hash ^= (uint8_t)(pixels->values[i] >> (8 * k));
            hash *= UINT64_C(0x100000001b3);
        }
    return hash;
}

KaleidoscopeBitmapQueueResult kaleidoscope_bitmap_queue(KaleidoscopeBitmapOutbox *outbox,
                                                        const KaleidoscopeQoiPixels *pixels,
                                                        size_t count,
                                                        KaleidoscopeBitmapGeometry geometry,
                                                        KaleidoscopeBitmapMetadata metadata) {
    if (!outbox || outbox->closed) {
        return (KaleidoscopeBitmapQueueResult){KALEIDOSCOPE_BITMAP_QUEUE_CLOSED, 0};
    }
    if (!bitmap_geometry_valid(geometry)) {
        return (KaleidoscopeBitmapQueueResult){KALEIDOSCOPE_BITMAP_QUEUE_INVALID_GEOMETRY, 0};
    }
    if (!pixels || count != bitmap_geometry_len(geometry)) {
        return (KaleidoscopeBitmapQueueResult){KALEIDOSCOPE_BITMAP_QUEUE_INVALID_PIXELS, 0};
    }
    if ((outbox->active_present && !outbox->active.receipt.geometry_present) ||
        (outbox->desired_present && !outbox->desired.receipt.geometry_present))
        return (KaleidoscopeBitmapQueueResult){KALEIDOSCOPE_BITMAP_QUEUE_CLEAR_PENDING, 0};
    uint64_t hash = kaleidoscope_bitmap_hash(pixels, count);
    bool known_present = outbox->desired_present || outbox->active_present;
    KaleidoscopeBitmapReceipt known =
        outbox->desired_present ? outbox->desired.receipt : outbox->active.receipt;
    if (known_present && known.geometry_present &&
        bitmap_geometry_equal(known.geometry, geometry) && known.hash == hash &&
        bitmap_metadata_equal(known.metadata, metadata))
        return (KaleidoscopeBitmapQueueResult){0, known.sequence};
    if (outbox->sequence == UINT32_MAX) {
        return (KaleidoscopeBitmapQueueResult){KALEIDOSCOPE_BITMAP_QUEUE_SEQUENCE_EXHAUSTED, 0};
    }
    uint32_t sequence = ++outbox->sequence;
    KaleidoscopeBitmapBatch batch = bitmap_batch(sequence, true, geometry, hash, metadata);
    if (!outbox->active_present) {
        outbox->active = batch;
        outbox->active_present = true;
        if (outbox->desired_present)
            bitmap_drop(&outbox->desired, &outbox->desired_present);
    } else {
        if (outbox->desired_present)
            bitmap_drop(&outbox->desired, &outbox->desired_present);
        outbox->desired = batch;
        outbox->desired_present = true;
    }
    return (KaleidoscopeBitmapQueueResult){0, sequence};
}

KaleidoscopeBitmapQueueResult kaleidoscope_bitmap_clear(KaleidoscopeBitmapOutbox *outbox,
                                                        KaleidoscopeBitmapMetadata metadata) {
    if (!outbox || outbox->closed) {
        return (KaleidoscopeBitmapQueueResult){KALEIDOSCOPE_BITMAP_QUEUE_CLOSED, 0};
    }
    bool known_present = outbox->desired_present || outbox->active_present;
    KaleidoscopeBitmapReceipt known =
        outbox->desired_present ? outbox->desired.receipt : outbox->active.receipt;
    if (known_present && !known.geometry_present && bitmap_metadata_equal(known.metadata, metadata))
        return (KaleidoscopeBitmapQueueResult){0, known.sequence};
    bool waiting =
        outbox->active_present && outbox->active.phase == KALEIDOSCOPE_BITMAP_PHASE_AWAITING;
    if (outbox->sequence == UINT32_MAX) {
        if (outbox->desired_present && outbox->desired.receipt.geometry_present)
            bitmap_drop(&outbox->desired, &outbox->desired_present);
        if (!waiting && outbox->active_present && outbox->active.receipt.geometry_present)
            bitmap_drop(&outbox->active, &outbox->active_present);
        return (KaleidoscopeBitmapQueueResult){KALEIDOSCOPE_BITMAP_QUEUE_SEQUENCE_EXHAUSTED, 0};
    }
    if (outbox->desired_present) {
        bitmap_drop(&outbox->desired, &outbox->desired_present);
    }
    if (!waiting && outbox->active_present) {
        bitmap_drop(&outbox->active, &outbox->active_present);
    }
    uint32_t sequence = ++outbox->sequence;
    KaleidoscopeBitmapGeometry none = {0};
    KaleidoscopeBitmapBatch batch = bitmap_batch(sequence, false, none, 0, metadata);
    if (waiting) {
        outbox->desired = batch;
        outbox->desired_present = true;
    } else {
        outbox->active = batch;
        outbox->active_present = true;
    }
    return (KaleidoscopeBitmapQueueResult){0, sequence};
}

static KaleidoscopeBitmapOperation bitmap_wire(KaleidoscopeBitmapIdentity identity,
                                               KaleidoscopeBitmapRequest request) {
    KaleidoscopeBitmapOperation operation = {.surface = identity.surface};
    if (request.kind == KALEIDOSCOPE_BITMAP_REQUEST_BEGIN) {
        operation.opcode = KALEIDOSCOPE_BITMAP_BEGIN;
        operation.packed = request.sequence | ((uint64_t)request.geometry.width << 32) |
                           ((uint64_t)request.geometry.height << 40) |
                           ((uint64_t)request.geometry.scale << 48);
        operation.control = request.geometry.x | ((uint64_t)request.geometry.y << 16);
    } else if (request.kind == KALEIDOSCOPE_BITMAP_REQUEST_CHUNK) {
        operation.opcode = KALEIDOSCOPE_BITMAP_CHUNK;
        operation.packed = request.pixels[0] | ((uint64_t)request.pixels[1] << 32);
        operation.control =
            ((uint64_t)request.sequence << 32) | request.offset | ((uint64_t)request.count << 16);
    } else if (request.kind == KALEIDOSCOPE_BITMAP_REQUEST_COMMIT) {
        operation.opcode = KALEIDOSCOPE_BITMAP_COMMIT;
        operation.packed = request.sequence;
        operation.control = request.hash;
    } else {
        operation.opcode = KALEIDOSCOPE_BITMAP_CLEAR;
        operation.packed = request.sequence;
    }
    return operation;
}

KaleidoscopeBitmapPollResult kaleidoscope_bitmap_prepare(KaleidoscopeBitmapOutbox *outbox,
                                                         const KaleidoscopeQoiPixels *pixels,
                                                         size_t count) {
    if (!outbox || outbox->closed) {
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_CLOSED};
    }
    if (!outbox->active_present && outbox->desired_present) {
        outbox->active = outbox->desired;
        outbox->active_present = true;
        outbox->desired = (KaleidoscopeBitmapBatch){0};
        outbox->desired_present = false;
    }
    if (!outbox->active_present) {
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_IDLE};
    }
    if (outbox->active.phase == KALEIDOSCOPE_BITMAP_PHASE_AWAITING) {
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_PENDING};
    }
    if (outbox->active.phase == KALEIDOSCOPE_BITMAP_PHASE_BEGIN ||
        outbox->active.phase == KALEIDOSCOPE_BITMAP_PHASE_CHUNK) {
        if (!pixels || count != bitmap_geometry_len(outbox->active.receipt.geometry) ||
            kaleidoscope_bitmap_hash(pixels, count) != outbox->active.receipt.hash) {
            bitmap_drop(&outbox->active, &outbox->active_present);
            return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_INVALID_PIXELS};
        }
    }
    if (!outbox->active.unsent_present) {
        KaleidoscopeBitmapRequest request = {.sequence = outbox->active.receipt.sequence};
        if (outbox->active.phase == KALEIDOSCOPE_BITMAP_PHASE_BEGIN) {
            request.kind = KALEIDOSCOPE_BITMAP_REQUEST_BEGIN;
            request.geometry = outbox->active.receipt.geometry;
        } else if (outbox->active.phase == KALEIDOSCOPE_BITMAP_PHASE_CHUNK) {
            request.kind = KALEIDOSCOPE_BITMAP_REQUEST_CHUNK;
            request.offset = outbox->active.offset;
            request.count = count - outbox->active.offset < 2 ? count - outbox->active.offset : 2;
            for (size_t i = 0; i < request.count; i++)
                request.pixels[i] = pixels->values[outbox->active.offset + i];
        } else if (outbox->active.phase == KALEIDOSCOPE_BITMAP_PHASE_COMMIT) {
            request.kind = KALEIDOSCOPE_BITMAP_REQUEST_COMMIT;
            request.hash = outbox->active.receipt.hash;
        } else
            request.kind = KALEIDOSCOPE_BITMAP_REQUEST_CLEAR;
        outbox->active.unsent = request;
        outbox->active.unsent_present = true;
    }
    return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_SEND,
                                          .operation_present = true,
                                          .operation =
                                              bitmap_wire(outbox->identity, outbox->active.unsent)};
}

KaleidoscopeBitmapPollResult kaleidoscope_bitmap_resolve(KaleidoscopeBitmapOutbox *outbox,
                                                         KaleidoscopeBitmapAdmission admission) {
    if (!outbox || outbox->closed) {
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_CLOSED};
    }
    if (!outbox->active_present || !outbox->active.unsent_present) {
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_IDLE};
    }
    if (admission.kind == KALEIDOSCOPE_BITMAP_ADMIT_FULL) {
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_DEFERRED};
    }
    if (admission.kind == KALEIDOSCOPE_BITMAP_ADMIT_REJECTED) {
        uint64_t error = admission.error;
        bitmap_drop(&outbox->active, &outbox->active_present);
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_REJECTED,
                                              .error = error};
    }
    if (admission.kind != KALEIDOSCOPE_BITMAP_ADMIT_ACCEPTED)
        return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_REJECTED,
                                              .error = UINT64_MAX};
    KaleidoscopeBitmapRequest request = outbox->active.unsent;
    outbox->active.unsent = (KaleidoscopeBitmapRequest){0};
    outbox->active.unsent_present = false;
    if (request.kind == KALEIDOSCOPE_BITMAP_REQUEST_BEGIN) {
        outbox->active.phase = KALEIDOSCOPE_BITMAP_PHASE_CHUNK;
        outbox->active.offset = 0;
    } else if (request.kind == KALEIDOSCOPE_BITMAP_REQUEST_CHUNK) {
        outbox->active.offset = request.offset + request.count;
        outbox->active.phase =
            outbox->active.offset == bitmap_geometry_len(outbox->active.receipt.geometry)
                ? KALEIDOSCOPE_BITMAP_PHASE_COMMIT
                : KALEIDOSCOPE_BITMAP_PHASE_CHUNK;
    } else
        outbox->active.phase = KALEIDOSCOPE_BITMAP_PHASE_AWAITING;
    return (KaleidoscopeBitmapPollResult){.state = KALEIDOSCOPE_BITMAP_POLL_PENDING};
}

typedef struct BitmapApplied {
    bool present;
    uint16_t surface;
    uint32_t sequence;
} BitmapApplied;

static BitmapApplied bitmap_applied(uint32_t caller, uint64_t word) {
    if (caller != 1 || (uint8_t)word != 0x57 || word >> 56)
        return (BitmapApplied){0};
    uint16_t surface = (uint16_t)(word >> 8);
    uint32_t sequence = (uint32_t)(word >> 24);
    if (!surface || !sequence ||
        word != (UINT64_C(0x57) | ((uint64_t)surface << 8) | ((uint64_t)sequence << 24)))
        return (BitmapApplied){0};
    return (BitmapApplied){true, surface, sequence};
}

bool kaleidoscope_bitmap_receive(KaleidoscopeBitmapOutbox *outbox, uint32_t caller, uint64_t word,
                                 KaleidoscopeBitmapReceipt *receipt) {
    if (!outbox || !receipt || outbox->closed)
        return false;
    BitmapApplied applied = bitmap_applied(caller, word);
    if (!applied.present || !outbox->active_present ||
        outbox->active.phase != KALEIDOSCOPE_BITMAP_PHASE_AWAITING ||
        applied.surface != outbox->identity.surface ||
        applied.sequence != outbox->active.receipt.sequence)
        return false;
    *receipt = outbox->active.receipt;
    outbox->active = (KaleidoscopeBitmapBatch){0};
    outbox->active_present = false;
    return true;
}

bool kaleidoscope_bitmap_pending(const KaleidoscopeBitmapOutbox *outbox) {
    return outbox && (outbox->active_present || outbox->desired_present);
}

void kaleidoscope_bitmap_close(KaleidoscopeBitmapOutbox *outbox) {
    if (!outbox)
        return;
    if (outbox->active_present)
        bitmap_drop(&outbox->active, &outbox->active_present);
    if (outbox->desired_present)
        bitmap_drop(&outbox->desired, &outbox->desired_present);
    outbox->closed = true;
}


