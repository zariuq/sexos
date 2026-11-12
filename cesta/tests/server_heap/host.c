#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_server.h"
#include "program_memory.h"

__attribute__((aligned(4096))) static uint8_t backing[4 * 1024 * 1024];
static uint64_t held, calls[128], call_count, page_limit = UINT64_MAX;
static bool deny;

SexosProgramMemory sexos_program_memory_grow(uint64_t pages)
{
    if (call_count < sizeof calls / sizeof calls[0]) calls[call_count++] = pages;
    if (deny) return (SexosProgramMemory){PROGRAM_MEMORY_DENIED, (uintptr_t)backing, held, 0};
    if (pages && held + pages > page_limit)
        return (SexosProgramMemory){PROGRAM_MEMORY_FULL, (uintptr_t)backing, held, 0};
    held += pages;
    return (SexosProgramMemory){PROGRAM_MEMORY_OK, (uintptr_t)backing, held, 0};
}

SexosMessage sexos_listen_once(uint64_t slot)
{
    (void)slot;
    return (SexosMessage){0};
}

void sexos_yield(void) {}

void sexos_heap_reset_for_restart(void);

static void reset(void)
{
    held = call_count = 0;
    page_limit = UINT64_MAX;
    deny = false;
    sexos_heap_reset_for_restart();
    sexos_heap_init();
}

static void basic_growth_and_coalescing(void)
{
    reset();
    uint8_t *fixed = sexos_malloc(120000, 16);
    assert(fixed && !((uintptr_t)fixed & 15));
    uint8_t *grown = sexos_malloc(16384, 16);
    assert(grown && call_count == 2 && calls[0] == 0 && calls[1] == 64 && held == 64);
    SexosHeapUsage usage = sexos_heap_usage();
    assert(usage.total >= 128 * 1024 + 64 * 4096 && usage.used < usage.total);
    sexos_free(fixed, 120000, 16);
    sexos_free(grown, 16384, 16);
    uint8_t *whole = sexos_malloc(128 * 1024, 4096);
    assert(whole);
    sexos_free(whole, 128 * 1024, 4096);
    usage = sexos_heap_usage();
    assert(usage.used <= 4 * sizeof(uintptr_t) && usage.free <= usage.total);
}

static void cold_growth_uses_only_held_pages(void)
{
    reset();
    uint8_t *p = sexos_malloc(200000, 16);
    assert(p && held == 64);
    uint8_t *q = sexos_malloc(140000, 16);
    assert(q && held == 128);
    assert(sexos_heap_usage().total == 128 * 1024 + 128 * 4096);
    sexos_free(p, 200000, 16);
    sexos_free(q, 140000, 16);
}

static void alignment_calloc_and_realloc(void)
{
    reset();
    uint8_t *p = sexos_calloc(16, 17, 64);
    assert(p && !((uintptr_t)p & 63));
    for (size_t i = 0; i < 16 * 17; ++i) assert(!p[i]);
    for (size_t i = 0; i < 16 * 17; ++i) p[i] = (uint8_t)(i ^ 0x5a);
    uint8_t *q = sexos_realloc(p, 16 * 17, 64, 64);
    assert(q && !((uintptr_t)q & 63));
    for (size_t i = 0; i < 64; ++i) assert(q[i] == (uint8_t)(i ^ 0x5a));
    sexos_free(q, 64, 64);
    assert(!sexos_malloc(0, 1));
    assert(!sexos_calloc(SIZE_MAX, 2, 1));
}

static void refusal_is_terminal_and_bounded(void)
{
    reset();
    assert(sexos_malloc(1000, 16));
    deny = true;
    assert(!sexos_malloc(256 * 1024, 16));
    uint64_t before = call_count;
    assert(!sexos_malloc(256 * 1024, 16));
    assert(call_count == before);
}

static void adoption_on_restart(void)
{
    reset();
    assert(sexos_malloc(120000, 16));
    assert(sexos_malloc(16384, 16));
    assert(held == 64);
    uint64_t old_held = held;
    call_count = 0;
    sexos_heap_reset_for_restart();
    sexos_heap_init();
    assert(sexos_malloc(200000, 16));
    assert(call_count == 1 && calls[0] == 0 && held == old_held);
    SexosHeapUsage usage = sexos_heap_usage();
    assert(usage.total >= 128 * 1024 + old_held * 4096);
}

static void completely_consumed_regions_extend(void)
{
    reset();
    uint8_t *all = sexos_malloc(128 * 1024, 4096);
    assert(all);
    uint8_t *grown = sexos_malloc(64 * 4096, 4096);
    assert(grown);
    uint8_t *next = sexos_malloc(16, 16);
    assert(next);
    SexosHeapUsage usage = sexos_heap_usage();
    assert(usage.total > 128 * 1024 + 64 * 4096);
    sexos_free(all, 128 * 1024, 4096);
    sexos_free(grown, 64 * 4096, 4096);
    sexos_free(next, 16, 16);
}

typedef struct {
    uint8_t *pointer;
    size_t size;
    uint8_t fill;
} Block;

static uint32_t random_state = 0x12345678;
static uint32_t next_random(void)
{
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}

static void randomized_allocations(void)
{
    reset();
    Block blocks[64] = {0};
    const size_t alignments[] = {1, 8, 16, 64, 256};
    for (size_t step = 0; step < 1200; ++step) {
        size_t index = next_random() % 64;
        if (blocks[index].pointer) {
            Block *b = &blocks[index];
            for (size_t i = 0; i < b->size; ++i) assert(b->pointer[i] == b->fill);
            sexos_free(b->pointer, b->size, alignments[index % 5]);
            blocks[index] = (Block){0};
            continue;
        }
        size_t size = next_random() % 5000 + 1;
        size_t alignment = alignments[next_random() % 5];
        uint8_t *pointer = sexos_malloc(size, alignment);
        if (!pointer) continue;
        assert(!((uintptr_t)pointer & (alignment - 1)));
        uint8_t fill = (uint8_t)(next_random() | 1);
        for (size_t i = 0; i < size; ++i) pointer[i] = fill;
        blocks[index] = (Block){pointer, size, fill};
        for (size_t other = 0; other < 64; ++other) {
            if (other == index || !blocks[other].pointer) continue;
            Block *b = &blocks[other];
            assert(pointer + size <= b->pointer || b->pointer + b->size <= pointer);
        }
    }
    for (size_t i = 0; i < 64; ++i)
        if (blocks[i].pointer) sexos_free(blocks[i].pointer, blocks[i].size, alignments[i % 5]);
}

int main(void)
{
    basic_growth_and_coalescing();
    cold_growth_uses_only_held_pages();
    alignment_calloc_and_realloc();
    refusal_is_terminal_and_bounded();
    adoption_on_restart();
    completely_consumed_regions_extend();
    randomized_allocations();
    puts("server heap: bounded growth, alignment, coalescing, refusal, restart and randomized controls passed");
    return 0;
}
