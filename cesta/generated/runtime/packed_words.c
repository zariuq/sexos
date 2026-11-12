#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

#ifndef CESTA_CHECKED_HINTS_DEFINED
#define CESTA_CHECKED_HINTS_DEFINED
#ifndef CESTA_CHECKED_OPTIMIZE
#define CESTA_CHECKED_OPTIMIZE 0
#endif
#if CESTA_CHECKED_OPTIMIZE != 0 && CESTA_CHECKED_OPTIMIZE != 1
#error "CESTA_CHECKED_OPTIMIZE must be 0 or 1"
#endif
#if CESTA_CHECKED_OPTIMIZE && (defined(__GNUC__) || defined(__clang__))
#define CESTA_CHECKED_COLD __attribute__((cold, noinline))
#define CESTA_CHECKED_ALLOC_SIZE __attribute__((alloc_size(1)))
#else
#define CESTA_CHECKED_COLD
#define CESTA_CHECKED_ALLOC_SIZE
#endif
#endif

_Static_assert(CHAR_BIT == 8 && sizeof(size_t) * CHAR_BIT == 64 &&
               PTRDIFF_MAX == SIZE_MAX / 2,
               "checked module word-bits disagrees with the C target");

#ifdef packed_words_failure
#error "checked identifier is a preprocessor macro"
#endif
#ifdef packed_words_load_le64
#error "checked identifier is a preprocessor macro"
#endif
#ifdef packed_words_store_le64
#error "checked identifier is a preprocessor macro"
#endif
#ifdef packed_words_display_metadata
#error "checked identifier is a preprocessor macro"
#endif
#ifdef bytes
#error "checked identifier is a preprocessor macro"
#endif
#ifdef packed
#error "checked identifier is a preprocessor macro"
#endif
#ifdef index
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_result
#error "checked identifier is a preprocessor macro"
#endif
#ifdef offset
#error "checked identifier is a preprocessor macro"
#endif
#ifdef count
#error "checked identifier is a preprocessor macro"
#endif
#ifdef color
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_packed_words_read
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_packed_words_write
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_packed_words_mul
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_packed_words_shift
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_packed_words_fail
#error "checked identifier is a preprocessor macro"
#endif

#ifndef CESTA_CHECKED_BYTE_VIEWS_DEFINED
#define CESTA_CHECKED_BYTE_VIEWS_DEFINED
typedef struct { const uint8_t *data; size_t len; } cesta_read_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_write_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_owned_bytes;
#endif

void packed_words_failure(void);
uint64_t packed_words_load_le64(cesta_read_bytes);
void packed_words_store_le64(cesta_write_bytes, uint64_t);
uint64_t packed_words_display_metadata(size_t, size_t, uint32_t);

static CESTA_CHECKED_COLD _Noreturn void cesta_packed_words_fail(void)
{
    packed_words_failure();
    for (;;) {}
}

static inline uint8_t cesta_packed_words_read(const uint8_t *data, size_t len, size_t index)
{
    if (index >= len) cesta_packed_words_fail();
    return data[index];
}

static inline void cesta_packed_words_write(uint8_t *data, size_t len, size_t index, uint8_t value)
{
    if (index >= len) cesta_packed_words_fail();
    data[index] = value;
}

static inline uint64_t cesta_packed_words_mul(uint64_t a, uint64_t b, uint64_t limit)
{
    if (a > limit || b > limit || (b && a > limit / b)) cesta_packed_words_fail();
    return a * b;
}

static inline uint64_t cesta_packed_words_shift(uint64_t value, size_t count, unsigned width, bool left)
{
    if (count >= width) cesta_packed_words_fail();
    return left ? value << count : value >> count;
}

uint64_t packed_words_load_le64(cesta_read_bytes bytes)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX / sizeof(uint8_t) ||
        (bytes.len && (uintptr_t)bytes.data % _Alignof(uint8_t)))
        cesta_packed_words_fail();
    if (!((bytes.len) <= (((size_t)UINT64_C(8))))) cesta_packed_words_fail();
    uint64_t packed = ((uint64_t)UINT64_C(0));
    (void)packed;
    for (size_t index = 0; index < bytes.len; ++index) {
        packed = ((uint64_t)(((uint64_t)(packed)) | ((uint64_t)(((uint64_t)cesta_packed_words_shift((uint64_t)(((uint64_t)(cesta_packed_words_read(bytes.data, bytes.len, index)))), ((size_t)cesta_packed_words_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), 64, true))))));
    }
    uint64_t cesta_result = packed;
    return cesta_result;
}

void packed_words_store_le64(cesta_write_bytes bytes, uint64_t packed)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX / sizeof(uint8_t) ||
        (bytes.len && (uintptr_t)bytes.data % _Alignof(uint8_t)))
        cesta_packed_words_fail();
    (void)packed;
    if (!((bytes.len) <= (((size_t)UINT64_C(8))))) cesta_packed_words_fail();
    for (size_t index = 0; index < bytes.len; ++index) {
        cesta_packed_words_write(bytes.data, bytes.len, index, ((uint8_t)(((uint64_t)cesta_packed_words_shift((uint64_t)(packed), ((size_t)cesta_packed_words_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), 64, false)))));
    }
    return;
}

uint64_t packed_words_display_metadata(size_t offset, size_t count, uint32_t color)
{
    (void)offset;
    (void)count;
    (void)color;
    if (!((offset) <= (((size_t)UINT64_C(255))))) cesta_packed_words_fail();
    if (!((count) <= (((size_t)UINT64_C(8))))) cesta_packed_words_fail();
    uint64_t cesta_result = ((uint64_t)(((uint64_t)(((uint64_t)(offset)))) | ((uint64_t)(((uint64_t)(((uint64_t)(((uint64_t)cesta_packed_words_shift((uint64_t)(((uint64_t)(count))), ((size_t)UINT64_C(8)), 64, true)))) | ((uint64_t)(((uint64_t)cesta_packed_words_shift((uint64_t)(((uint64_t)(color))), ((size_t)UINT64_C(32)), 64, true))))))))));
    return cesta_result;
}
