#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cases.h"
#include "sexdisplay_bitmap.h"

static uint64_t hash_byte(uint64_t hash, uint8_t byte) {
    return (hash ^ byte) * UINT64_C(0x100000001b3);
}

static uint64_t hash_u32s(const uint32_t *values, size_t count) {
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (size_t i = 0; i < count; ++i) {
        for (size_t shift = 0; shift < 32; shift += 8) {
            hash = hash_byte(hash, (uint8_t)(values[i] >> shift));
        }
    }
    return hash;
}

static uint64_t hash_u64s(const uint64_t *values, size_t count) {
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (size_t i = 0; i < count; ++i) {
        for (size_t shift = 0; shift < 64; shift += 8) {
            hash = hash_byte(hash, (uint8_t)(values[i] >> shift));
        }
    }
    return hash;
}

static int64_t signed_bits(uint64_t bits) {
    if (!(bits >> 63)) return (int64_t)bits;
    return -INT64_C(1) - (int64_t)(~bits);
}

static void geometry(bool present, KaleidoscopeBitmapGeometry g) {
    if (!present) {
        printf(" 0,0,0,0,0,0");
    } else {
        printf(" 1,%u,%u,%u,%u,%u", (unsigned)g.x, (unsigned)g.y,
               (unsigned)g.width, (unsigned)g.height, (unsigned)g.scale);
    }
}

static void snapshot(const SexdisplayBitmapStage *stage) {
    printf(" | B%u,%u,%u,%" PRIu64 " Q%u",
           stage->bound ? 1u : 0u, stage->target, stage->task, stage->surface,
           stage->sequence);
    geometry(stage->geometry_present, stage->geometry);
    geometry(stage->published_present, stage->published_geometry);
    printf(" C%u,%016" PRIx64 " P%u Z%u T%u H%016" PRIx64 ",%016" PRIx64
           ",%016" PRIx64 "\n",
           (unsigned)stage->completed, stage->completed_hash,
           (unsigned)stage->completion_phase, stage->poisoned ? 1u : 0u,
           stage->retired ? 1u : 0u,
           hash_u64s(stage->coverage, SEXDISPLAY_BITMAP_COVERAGE_WORDS),
           hash_u32s(stage->staging, KALEIDOSCOPE_BITMAP_CAPACITY),
           hash_u32s(stage->published, KALEIDOSCOPE_BITMAP_CAPACITY));
}

int main(void) {
    SexdisplayBitmapStage stage;
    sexdisplay_bitmap_init(&stage, 15, 150, 300);
    for (size_t index = 0; index < command_count; ++index) {
        const Command *command = &commands[index];
        switch (command->op) {
        case 0:
            sexdisplay_bitmap_init(&stage, (uint32_t)command->a,
                                   (uint32_t)command->b, command->c);
            printf("I");
            break;
        case 1: {
            SexdisplayBitmapChange change = sexdisplay_bitmap_receive(
                &stage, (uint32_t)command->a, (uint32_t)command->b, command->c,
                command->d, command->e, command->f);
            printf("R %u", change.changed ? 1u : 0u);
            geometry(change.old_present, change.old_geometry);
            geometry(change.new_present, change.new_geometry);
            break;
        }
        case 2:
            printf("L %u", sexdisplay_bitmap_release_after_publication(&stage) ? 1u : 0u);
            break;
        case 3: {
            SexdisplayBitmapReply reply = sexdisplay_bitmap_reply(&stage);
            printf("P %u,%u,%016" PRIx64, reply.present ? 1u : 0u,
                   reply.target, reply.value);
            if (reply.present) sexdisplay_bitmap_reply_resolve(&stage, command->a);
            break;
        }
        case 4: {
            uint32_t pixel = UINT32_C(0xdeadbeef);
            bool ok = sexdisplay_bitmap_sample(
                &stage, (uint32_t)command->a, command->b,
                signed_bits(command->c), signed_bits(command->d), &pixel);
            printf("S %u,%08" PRIx32, ok ? 1u : 0u, pixel);
            break;
        }
        case 5:
            sexdisplay_bitmap_reset_surface(&stage, (uint32_t)command->a, command->b);
            printf("D");
            break;
        case 6:
            sexdisplay_bitmap_retire(&stage, (uint32_t)command->a, (uint32_t)command->b);
            printf("T");
            break;
        case 7:
            printf("A %08" PRIx32,
                   sexdisplay_bitmap_blend((uint32_t)command->a, (uint32_t)command->b));
            break;
        default:
            return 2;
        }
        snapshot(&stage);
    }
    return 0;
}
