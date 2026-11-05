
#ifndef KALEIDOSCOPE_PRESENTATION_H
#define KALEIDOSCOPE_PRESENTATION_H
#include <stdbool.h>
#include <stdint.h>
#include "kaleidoscope_bitmap.h"

enum {
    KALEIDOSCOPE_PRESENTATION_FIT = 0,
    KALEIDOSCOPE_PRESENTATION_ACTUAL = 1
};

typedef struct KaleidoscopePresentationLayout {
    uint64_t generation;
    KaleidoscopeBitmapGeometry geometry;
    uint64_t viewport;
    uint32_t ticket, sequence;
    uint64_t hash;
} KaleidoscopePresentationLayout;

typedef struct KaleidoscopePresentationClear {
    uint64_t generation, viewport;
    uint32_t sequence;
} KaleidoscopePresentationClear;

typedef struct KaleidoscopePresentation {
    uint8_t mode;
    uint64_t generation;
    bool text_present, layout_present, applied, clear_present, clear_applied, exhausted;
    uint64_t text_generation, text_viewport;
    KaleidoscopePresentationLayout layout;
    KaleidoscopePresentationClear clear;
    uint32_t sequence_floor;
} KaleidoscopePresentation;

typedef struct KaleidoscopePresentationSelect {
    bool ok, changed;
} KaleidoscopePresentationSelect;

void kaleidoscope_presentation_init(KaleidoscopePresentation *presentation);
bool kaleidoscope_presentation_invalidate(KaleidoscopePresentation *presentation);
KaleidoscopePresentationSelect
kaleidoscope_presentation_select(KaleidoscopePresentation *presentation, uint8_t mode);
bool kaleidoscope_presentation_text_applied(KaleidoscopePresentation *presentation,
                                            uint64_t generation, uint64_t viewport,
                                            uint64_t current_viewport);
bool kaleidoscope_presentation_can_queue(const KaleidoscopePresentation *presentation,
                                         uint64_t viewport);
bool kaleidoscope_presentation_expect(KaleidoscopePresentation *presentation,
                                      KaleidoscopeBitmapGeometry geometry, uint64_t viewport,
                                      uint32_t ticket, uint32_t sequence, uint64_t hash);
bool kaleidoscope_presentation_expect_clear(KaleidoscopePresentation *presentation,
                                            uint64_t viewport, uint32_t sequence);
bool kaleidoscope_presentation_cleared(KaleidoscopePresentation *presentation,
                                       const KaleidoscopeBitmapReceipt *receipt,
                                       uint64_t current_viewport);
bool kaleidoscope_presentation_applied(KaleidoscopePresentation *presentation,
                                       const KaleidoscopeBitmapReceipt *receipt,
                                       uint64_t current_viewport, uint32_t current_ticket,
                                       uint64_t current_hash);
#endif
