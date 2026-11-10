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

#ifdef program_image_bounds_failure
#error "checked identifier is a preprocessor macro"
#endif
#ifdef program_image_bytes_u8_at
#error "checked identifier is a preprocessor macro"
#endif
#ifdef program_image_bytes_key_write
#error "checked identifier is a preprocessor macro"
#endif
#ifdef program_image_bytes_u16_at
#error "checked identifier is a preprocessor macro"
#endif
#ifdef program_image_bytes_u32_at
#error "checked identifier is a preprocessor macro"
#endif
#ifdef program_image_bytes_u64_at
#error "checked identifier is a preprocessor macro"
#endif
#ifdef bytes
#error "checked identifier is a preprocessor macro"
#endif
#ifdef offset
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_result
#error "checked identifier is a preprocessor macro"
#endif
#ifdef start
#error "checked identifier is a preprocessor macro"
#endif
#ifdef count
#error "checked identifier is a preprocessor macro"
#endif
#ifdef code
#error "checked identifier is a preprocessor macro"
#endif
#ifdef index
#error "checked identifier is a preprocessor macro"
#endif
#ifdef value
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_program_image_bytes_read
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_program_image_bytes_add
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_program_image_bytes_sub
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_program_image_bytes_mul
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_program_image_bytes_shift
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_program_image_bytes_fail
#error "checked identifier is a preprocessor macro"
#endif

#ifndef CESTA_CHECKED_BYTE_VIEWS_DEFINED
#define CESTA_CHECKED_BYTE_VIEWS_DEFINED
typedef struct { const uint8_t *data; size_t len; } cesta_read_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_write_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_owned_bytes;
#endif

void program_image_bounds_failure(void);
uint8_t program_image_bytes_u8_at(cesta_read_bytes, size_t);
bool program_image_bytes_key_write(cesta_read_bytes, size_t, size_t);
uint16_t program_image_bytes_u16_at(cesta_read_bytes, size_t);
uint32_t program_image_bytes_u32_at(cesta_read_bytes, size_t);
uint64_t program_image_bytes_u64_at(cesta_read_bytes, size_t);

static CESTA_CHECKED_COLD _Noreturn void cesta_program_image_bytes_fail(void)
{
    program_image_bounds_failure();
    for (;;) {}
}

static inline uint8_t cesta_program_image_bytes_read(const uint8_t *data, size_t len, size_t index)
{
    if (index >= len) cesta_program_image_bytes_fail();
    return data[index];
}

static inline uint64_t cesta_program_image_bytes_add(uint64_t a, uint64_t b, uint64_t limit)
{
    if (b > limit || a > limit - b) cesta_program_image_bytes_fail();
    return a + b;
}

static inline uint64_t cesta_program_image_bytes_sub(uint64_t a, uint64_t b, uint64_t limit)
{
    if (a > limit || a < b) cesta_program_image_bytes_fail();
    return a - b;
}

static inline uint64_t cesta_program_image_bytes_mul(uint64_t a, uint64_t b, uint64_t limit)
{
    if (a > limit || b > limit || (b && a > limit / b)) cesta_program_image_bytes_fail();
    return a * b;
}

static inline uint64_t cesta_program_image_bytes_shift(uint64_t value, size_t count, unsigned width, bool left)
{
    if (count >= width) cesta_program_image_bytes_fail();
    return left ? value << count : value >> count;
}

uint8_t program_image_bytes_u8_at(cesta_read_bytes bytes, size_t offset)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX / sizeof(uint8_t) ||
        (bytes.len && (uintptr_t)bytes.data % _Alignof(uint8_t)))
        cesta_program_image_bytes_fail();
    (void)offset;
    uint8_t cesta_result = cesta_program_image_bytes_read(bytes.data, bytes.len, offset);
    return cesta_result;
}

bool program_image_bytes_key_write(cesta_read_bytes bytes, size_t start, size_t count)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX / sizeof(uint8_t) ||
        (bytes.len && (uintptr_t)bytes.data % _Alignof(uint8_t)))
        cesta_program_image_bytes_fail();
    (void)start;
    (void)count;
    const size_t cesta_3_start = start;
    const size_t cesta_3_count = count;
    if (cesta_3_start > bytes.len || cesta_3_count > bytes.len - cesta_3_start)
        cesta_program_image_bytes_fail();
    cesta_read_bytes code = {bytes.data, cesta_3_count};
    if (cesta_3_start) code.data += cesta_3_start;
    (void)code;
    if ((count) < (((size_t)UINT64_C(3)))) {
        bool cesta_result = false;
        return cesta_result;
    } else {
    }
    size_t index = ((size_t)UINT64_C(0));
    (void)index;
    while ((index) <= (((size_t)cesta_program_image_bytes_sub((uint64_t)(count), (uint64_t)(((size_t)UINT64_C(3))), UINT64_C(18446744073709551615))))) {
        if (((cesta_program_image_bytes_read(code.data, code.len, index)) == (((uint8_t)UINT64_C(15)))) && (((cesta_program_image_bytes_read(code.data, code.len, ((size_t)cesta_program_image_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615))))) == (((uint8_t)UINT64_C(1)))) && ((cesta_program_image_bytes_read(code.data, code.len, ((size_t)cesta_program_image_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(2))), UINT64_C(18446744073709551615))))) == (((uint8_t)UINT64_C(239)))))) {
            bool cesta_result = true;
            return cesta_result;
        } else {
        }
        index = ((size_t)cesta_program_image_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
    }
    bool cesta_result = false;
    return cesta_result;
}

uint16_t program_image_bytes_u16_at(cesta_read_bytes bytes, size_t offset)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX / sizeof(uint8_t) ||
        (bytes.len && (uintptr_t)bytes.data % _Alignof(uint8_t)))
        cesta_program_image_bytes_fail();
    (void)offset;
    if (!((offset) <= (bytes.len))) cesta_program_image_bytes_fail();
    if (!((((size_t)UINT64_C(2))) <= (((size_t)cesta_program_image_bytes_sub((uint64_t)(bytes.len), (uint64_t)(offset), UINT64_C(18446744073709551615)))))) cesta_program_image_bytes_fail();
    uint16_t value = ((uint16_t)UINT64_C(0));
    (void)value;
    size_t index = ((size_t)UINT64_C(0));
    (void)index;
    while ((index) < (((size_t)UINT64_C(2)))) {
        value = ((uint16_t)(((uint64_t)(value)) | ((uint64_t)(((uint16_t)cesta_program_image_bytes_shift((uint64_t)(((uint16_t)(cesta_program_image_bytes_read(bytes.data, bytes.len, ((size_t)cesta_program_image_bytes_add((uint64_t)(offset), (uint64_t)(index), UINT64_C(18446744073709551615))))))), ((size_t)cesta_program_image_bytes_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), 16, true))))));
        index = ((size_t)cesta_program_image_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
    }
    uint16_t cesta_result = value;
    return cesta_result;
}

uint32_t program_image_bytes_u32_at(cesta_read_bytes bytes, size_t offset)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX / sizeof(uint8_t) ||
        (bytes.len && (uintptr_t)bytes.data % _Alignof(uint8_t)))
        cesta_program_image_bytes_fail();
    (void)offset;
    if (!((offset) <= (bytes.len))) cesta_program_image_bytes_fail();
    if (!((((size_t)UINT64_C(4))) <= (((size_t)cesta_program_image_bytes_sub((uint64_t)(bytes.len), (uint64_t)(offset), UINT64_C(18446744073709551615)))))) cesta_program_image_bytes_fail();
    uint32_t value = ((uint32_t)UINT64_C(0));
    (void)value;
    size_t index = ((size_t)UINT64_C(0));
    (void)index;
    while ((index) < (((size_t)UINT64_C(4)))) {
        value = ((uint32_t)(((uint64_t)(value)) | ((uint64_t)(((uint32_t)cesta_program_image_bytes_shift((uint64_t)(((uint32_t)(cesta_program_image_bytes_read(bytes.data, bytes.len, ((size_t)cesta_program_image_bytes_add((uint64_t)(offset), (uint64_t)(index), UINT64_C(18446744073709551615))))))), ((size_t)cesta_program_image_bytes_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), 32, true))))));
        index = ((size_t)cesta_program_image_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
    }
    uint32_t cesta_result = value;
    return cesta_result;
}

uint64_t program_image_bytes_u64_at(cesta_read_bytes bytes, size_t offset)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX / sizeof(uint8_t) ||
        (bytes.len && (uintptr_t)bytes.data % _Alignof(uint8_t)))
        cesta_program_image_bytes_fail();
    (void)offset;
    if (!((offset) <= (bytes.len))) cesta_program_image_bytes_fail();
    if (!((((size_t)UINT64_C(8))) <= (((size_t)cesta_program_image_bytes_sub((uint64_t)(bytes.len), (uint64_t)(offset), UINT64_C(18446744073709551615)))))) cesta_program_image_bytes_fail();
    uint64_t value = ((uint64_t)UINT64_C(0));
    (void)value;
    size_t index = ((size_t)UINT64_C(0));
    (void)index;
    while ((index) < (((size_t)UINT64_C(8)))) {
        value = ((uint64_t)(((uint64_t)(value)) | ((uint64_t)(((uint64_t)cesta_program_image_bytes_shift((uint64_t)(((uint64_t)(cesta_program_image_bytes_read(bytes.data, bytes.len, ((size_t)cesta_program_image_bytes_add((uint64_t)(offset), (uint64_t)(index), UINT64_C(18446744073709551615))))))), ((size_t)cesta_program_image_bytes_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), 64, true))))));
        index = ((size_t)cesta_program_image_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
    }
    uint64_t cesta_result = value;
    return cesta_result;
}
