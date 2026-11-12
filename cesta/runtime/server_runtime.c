#include "sexos_server.h"
#include "program_memory.h"

/* pdx_try_listen_raw in crates/sex-pdx: a listen that finds nothing yields
 * once, so other protection domains run even without a preemptive timer. */
bool sexos_try_listen(uint64_t slot, SexosMessage *out)
{
    SexosMessage message = sexos_listen_once(slot);
    if (!message.type_id) {
        sexos_yield();
        return false;
    }
    *out = message;
    return true;
}

#include <string.h>

#define SEXOS_HEAP_BYTES (128u * 1024u)
#define SEXOS_HEAP_STEP_PAGES 64u
#define SEXOS_HEAP_MAX_PAGES_PER_CALL 256u

typedef struct SexosHeapNode {
    struct SexosHeapNode *next;
    size_t size;
} SexosHeapNode;

typedef struct {
    SexosHeapNode *first;
    uintptr_t bottom, top;
} SexosHeap;

typedef enum {
    SEXOS_HEAP_UNASKED,
    SEXOS_HEAP_GROWS,
    SEXOS_HEAP_FIXED
} SexosHeapGrowthState;

__attribute__((aligned(4096))) static uint8_t sexos_heap_storage[SEXOS_HEAP_BYTES];
static SexosHeap sexos_heap_fixed, sexos_heap_growth;
static SexosHeapGrowthState sexos_heap_growth_state;
static uintptr_t sexos_heap_growth_base;
static bool sexos_heap_lock;

static void sexos_heap_lock_take(void)
{
    while (__atomic_test_and_set(&sexos_heap_lock, __ATOMIC_ACQUIRE)) {}
}

static void sexos_heap_lock_release(void)
{
    __atomic_clear(&sexos_heap_lock, __ATOMIC_RELEASE);
}

static void sexos_heap_region_init(SexosHeap *heap, uintptr_t base, size_t bytes)
{
    heap->bottom = base;
    heap->top = base + bytes;
    heap->first = (SexosHeapNode *)base;
    heap->first->next = NULL;
    heap->first->size = bytes;
}

static uintptr_t sexos_heap_align_up(uintptr_t value, size_t align)
{
    if (align <= 1 || value > UINTPTR_MAX - (align - 1)) return value;
    return (value + align - 1) & ~(uintptr_t)(align - 1);
}

static void *sexos_heap_allocate(SexosHeap *heap, size_t size, size_t align)
{
    if (!size || (align & (align - 1)) || size > UINTPTR_MAX - sizeof(SexosHeapNode)) return NULL;
    if (align < _Alignof(SexosHeapNode)) align = _Alignof(SexosHeapNode);
    for (SexosHeapNode **link = &heap->first; *link; link = &(*link)->next) {
        SexosHeapNode *node = *link;
        uintptr_t begin = (uintptr_t)node;
        uintptr_t aligned = sexos_heap_align_up(begin, align);
        if (aligned < begin || aligned > UINTPTR_MAX - size) continue;
        uintptr_t end = aligned + size;
        uintptr_t node_end = begin + node->size;
        if (end > node_end) continue;
        end = sexos_heap_align_up(end, _Alignof(SexosHeapNode));
        if (end > node_end) end = node_end;
        size_t prefix = aligned - begin;
        size_t suffix = node_end - end;
        if (suffix < sizeof(SexosHeapNode)) {
            end = node_end;
            suffix = 0;
        }
        if (prefix >= sizeof(SexosHeapNode)) {
            node->size = prefix;
            if (suffix) {
                SexosHeapNode *rest = (SexosHeapNode *)end;
                rest->size = suffix;
                rest->next = node->next;
                node->next = rest;
            }
        } else {
            *link = node->next;
            if (suffix) {
                SexosHeapNode *rest = (SexosHeapNode *)end;
                rest->size = suffix;
                rest->next = *link;
                *link = rest;
            }
        }
        return (void *)aligned;
    }
    return NULL;
}

static void sexos_heap_insert_free(SexosHeap *heap, uintptr_t address, size_t size)
{
    SexosHeapNode **link = &heap->first;
    while (*link && (uintptr_t)*link < address) link = &(*link)->next;
    SexosHeapNode *node = (SexosHeapNode *)address;
    node->size = size;
    node->next = *link;
    *link = node;
    if (node->next && address + size == (uintptr_t)node->next) {
        node->size += node->next->size;
        node->next = node->next->next;
    }
    if (link != &heap->first) {
        SexosHeapNode *previous = (SexosHeapNode *)((uint8_t *)link - offsetof(SexosHeapNode, next));
        if ((uintptr_t)previous + previous->size == address) {
            previous->size += node->size;
            previous->next = node->next;
        }
    }
}

static void sexos_heap_extend(SexosHeap *heap, uintptr_t end)
{
    if (end <= heap->top) return;
    size_t bytes = end - heap->top;
    if (!heap->bottom) {
        sexos_heap_region_init(heap, sexos_heap_growth_base, end - sexos_heap_growth_base);
        return;
    }
    if (!heap->first) {
        SexosHeapNode *node = (SexosHeapNode *)heap->top;
        node->size = bytes;
        node->next = NULL;
        heap->first = node;
        heap->top = end;
        return;
    }
    SexosHeapNode *last = heap->first;
    while (last->next) last = last->next;
    if ((uintptr_t)last + last->size == heap->top) {
        last->size += bytes;
    } else {
        SexosHeapNode *node = (SexosHeapNode *)heap->top;
        node->size = bytes;
        node->next = NULL;
        last->next = node;
    }
    heap->top = end;
}

static SexosHeapUsage sexos_heap_usage_one(const SexosHeap *heap)
{
    SexosHeapUsage usage = {0, 0, 0};
    if (!heap->bottom) return usage;
    usage.total = heap->top - heap->bottom;
    for (const SexosHeapNode *node = heap->first; node; node = node->next) usage.free += node->size;
    usage.used = usage.total - usage.free;
    return usage;
}

static void *sexos_heap_alloc_grown(size_t size, size_t align)
{
    if (sexos_heap_growth_state == SEXOS_HEAP_FIXED) return NULL;
    if (sexos_heap_growth_state == SEXOS_HEAP_UNASKED) {
        SexosProgramMemory reply = sexos_program_memory_grow(0);
        if (reply.status != PROGRAM_MEMORY_OK) {
            sexos_heap_growth_state = SEXOS_HEAP_FIXED;
            return NULL;
        }
        sexos_heap_growth_state = SEXOS_HEAP_GROWS;
        sexos_heap_growth_base = (uintptr_t)reply.base;
        if (reply.pages) {
            if (reply.pages > SIZE_MAX / 4096u) return NULL;
            sexos_heap_region_init(&sexos_heap_growth, (uintptr_t)reply.base,
                                   (size_t)reply.pages * 4096u);
        }
    }
    for (;;) {
        void *result = sexos_heap_allocate(&sexos_heap_growth, size, align);
        if (result) return result;
        size_t pages = (size + align + 64u + 4095u) / 4096u;
        if (pages < SEXOS_HEAP_STEP_PAGES) pages = SEXOS_HEAP_STEP_PAGES;
        if (pages > SEXOS_HEAP_MAX_PAGES_PER_CALL) pages = SEXOS_HEAP_MAX_PAGES_PER_CALL;
        SexosProgramMemory reply = sexos_program_memory_grow(pages);
        if (reply.status != PROGRAM_MEMORY_OK || reply.pages < pages ||
            (uintptr_t)reply.base != sexos_heap_growth_base || reply.pages > SIZE_MAX / 4096u)
            return NULL;
        uintptr_t end = (uintptr_t)reply.base + (size_t)reply.pages * 4096u;
        if (end <= sexos_heap_growth.top) return NULL;
        sexos_heap_extend(&sexos_heap_growth, end);
    }
}

static void sexos_heap_init_locked(void)
{
    if (!sexos_heap_fixed.bottom)
        sexos_heap_region_init(&sexos_heap_fixed, (uintptr_t)sexos_heap_storage, SEXOS_HEAP_BYTES);
}

void sexos_heap_init(void)
{
    sexos_heap_lock_take();
    sexos_heap_init_locked();
    sexos_heap_lock_release();
}

void *sexos_malloc(size_t size, size_t align)
{
    if (!size) return NULL;
    sexos_heap_lock_take();
    sexos_heap_init_locked();
    void *result = NULL;
    if (size <= SEXOS_HEAP_BYTES) result = sexos_heap_allocate(&sexos_heap_fixed, size, align);
    if (!result) result = sexos_heap_alloc_grown(size, align);
    sexos_heap_lock_release();
    return result;
}

void sexos_free(void *pointer, size_t size, size_t align)
{
    (void)align;
    if (!pointer || !size) return;
    sexos_heap_lock_take();
    uintptr_t address = (uintptr_t)pointer;
    if (address >= (uintptr_t)sexos_heap_storage &&
        address + size <= (uintptr_t)sexos_heap_storage + SEXOS_HEAP_BYTES)
        sexos_heap_insert_free(&sexos_heap_fixed, address, size);
    else
        sexos_heap_insert_free(&sexos_heap_growth, address, size);
    sexos_heap_lock_release();
}

void *sexos_calloc(size_t count, size_t size, size_t align)
{
    if (count && size > SIZE_MAX / count) return NULL;
    size_t bytes = count * size;
    void *result = sexos_malloc(bytes, align);
    if (result) memset(result, 0, bytes);
    return result;
}

void *sexos_realloc(void *pointer, size_t old_size, size_t new_size, size_t align)
{
    if (!pointer) return sexos_malloc(new_size, align);
    if (!new_size) {
        sexos_free(pointer, old_size, align);
        return NULL;
    }
    if (old_size == new_size) return pointer;
    void *result = sexos_malloc(new_size, align);
    if (!result) return NULL;
    memcpy(result, pointer, old_size < new_size ? old_size : new_size);
    sexos_free(pointer, old_size, align);
    return result;
}

SexosHeapUsage sexos_heap_usage(void)
{
    sexos_heap_lock_take();
    sexos_heap_init_locked();
    SexosHeapUsage fixed = sexos_heap_usage_one(&sexos_heap_fixed);
    SexosHeapUsage grown = sexos_heap_usage_one(&sexos_heap_growth);
    SexosHeapUsage usage = {fixed.used + grown.used, fixed.free + grown.free, fixed.total + grown.total};
    sexos_heap_lock_release();
    return usage;
}

#ifdef SEXOS_HOST
__attribute__((weak)) SexosProgramMemory sexos_program_memory_grow(uint64_t pages)
{
    (void)pages;
    return (SexosProgramMemory){PROGRAM_MEMORY_DENIED, 0, 0, 0};
}

void sexos_heap_reset_for_restart(void)
{
    sexos_heap_fixed = (SexosHeap){0};
    sexos_heap_growth = (SexosHeap){0};
    sexos_heap_growth_state = SEXOS_HEAP_UNASKED;
    sexos_heap_growth_base = 0;
    sexos_heap_lock = false;
}
#endif
