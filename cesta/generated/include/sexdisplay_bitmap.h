#ifndef SEXDISPLAY_BITMAP_H
#define SEXDISPLAY_BITMAP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kaleidoscope_bitmap.h"
#define SEXDISPLAY_BITMAP_COVERAGE_WORDS ((size_t)18)
enum {SEXDISPLAY_BITMAP_COMPLETE_NONE=0,SEXDISPLAY_BITMAP_COMPLETE_IMAGE=1,SEXDISPLAY_BITMAP_COMPLETE_CLEAR=2};
enum {SEXDISPLAY_BITMAP_COMPLETION_NONE=0,SEXDISPLAY_BITMAP_COMPLETION_UNRELEASED=1,SEXDISPLAY_BITMAP_COMPLETION_ELIGIBLE=2,SEXDISPLAY_BITMAP_COMPLETION_SENT=3};
typedef struct SexdisplayBitmapChange {bool changed,old_present,new_present;KaleidoscopeBitmapGeometry old_geometry,new_geometry;} SexdisplayBitmapChange;
typedef struct SexdisplayBitmapReply {bool present;uint32_t target;uint64_t value;} SexdisplayBitmapReply;
typedef struct SexdisplayBitmapStage {
 bool bound,retired,geometry_present,published_present,poisoned;
 uint32_t target,task,sequence;uint64_t surface,completed_hash;
 KaleidoscopeBitmapGeometry geometry,published_geometry;
 uint64_t coverage[SEXDISPLAY_BITMAP_COVERAGE_WORDS];
 uint32_t staging[KALEIDOSCOPE_BITMAP_CAPACITY],published[KALEIDOSCOPE_BITMAP_CAPACITY];
 uint8_t completed,completion_phase;
} SexdisplayBitmapStage;
void sexdisplay_bitmap_init(SexdisplayBitmapStage *stage,uint32_t target,uint32_t task,uint64_t surface);
SexdisplayBitmapChange sexdisplay_bitmap_receive(SexdisplayBitmapStage *stage,uint32_t caller,uint32_t caller_task,uint64_t surface,uint64_t opcode,uint64_t packed,uint64_t control);
bool sexdisplay_bitmap_sample(const SexdisplayBitmapStage *stage,uint32_t owner,uint64_t surface,int64_t x,int64_t y,uint32_t *pixel);
bool sexdisplay_bitmap_release_after_publication(SexdisplayBitmapStage *stage);
SexdisplayBitmapReply sexdisplay_bitmap_reply(const SexdisplayBitmapStage *stage);
void sexdisplay_bitmap_reply_resolve(SexdisplayBitmapStage *stage,uint64_t status);
void sexdisplay_bitmap_reset_surface(SexdisplayBitmapStage *stage,uint32_t owner,uint64_t surface);
void sexdisplay_bitmap_retire(SexdisplayBitmapStage *stage,uint32_t owner,uint32_t task);
uint32_t sexdisplay_bitmap_blend(uint32_t rgba,uint32_t base);
#endif


