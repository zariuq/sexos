#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

_Static_assert(CHAR_BIT == 8 && sizeof(size_t) * CHAR_BIT == 64 &&
               PTRDIFF_MAX == SIZE_MAX / 2,
               "checked module word-bits disagrees with the C target");

#ifdef format_bytes_failure
#error "checked identifier is a preprocessor macro"
#endif
#ifdef format_bytes_copy
#error "checked identifier is a preprocessor macro"
#endif
#ifdef format_bytes_unsigned_decimal
#error "checked identifier is a preprocessor macro"
#endif
#ifdef format_bytes_unsigned_hex
#error "checked identifier is a preprocessor macro"
#endif
#ifdef destination
#error "checked identifier is a preprocessor macro"
#endif
#ifdef source
#error "checked identifier is a preprocessor macro"
#endif
#ifdef index
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_result
#error "checked identifier is a preprocessor macro"
#endif
#ifdef value
#error "checked identifier is a preprocessor macro"
#endif
#ifdef reversed
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_2_storage
#error "checked identifier is a preprocessor macro"
#endif
#ifdef remaining
#error "checked identifier is a preprocessor macro"
#endif
#ifdef count
#error "checked identifier is a preprocessor macro"
#endif
#ifdef prefix
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_3_storage
#error "checked identifier is a preprocessor macro"
#endif
#ifdef digit
#error "checked identifier is a preprocessor macro"
#endif
#ifdef character
#error "checked identifier is a preprocessor macro"
#endif
#ifdef start
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_format_bytes_read
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_format_bytes_write
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_format_bytes_add
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_format_bytes_sub
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_format_bytes_fail
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_format_bytes_div
#error "checked identifier is a preprocessor macro"
#endif

#ifndef CESTA_CHECKED_BYTE_VIEWS_DEFINED
#define CESTA_CHECKED_BYTE_VIEWS_DEFINED
typedef struct { const uint8_t *data; size_t len; } cesta_read_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_write_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_owned_bytes;
#endif

void format_bytes_failure(void);
size_t format_bytes_copy(cesta_write_bytes, cesta_read_bytes);
size_t format_bytes_unsigned_decimal(cesta_write_bytes, uint64_t);
size_t format_bytes_unsigned_hex(cesta_write_bytes, uint64_t, bool);

static _Noreturn void cesta_format_bytes_fail(void)
{
    format_bytes_failure();
    for (;;) {}
}

static inline uint8_t cesta_format_bytes_read(const uint8_t *data, size_t len, size_t index)
{
    if (index >= len) cesta_format_bytes_fail();
    return data[index];
}

static inline void cesta_format_bytes_write(uint8_t *data, size_t len, size_t index, uint8_t value)
{
    if (index >= len) cesta_format_bytes_fail();
    data[index] = value;
}

static inline uint64_t cesta_format_bytes_add(uint64_t a, uint64_t b, uint64_t limit)
{
    if (b > limit || a > limit - b) cesta_format_bytes_fail();
    return a + b;
}

static inline uint64_t cesta_format_bytes_sub(uint64_t a, uint64_t b, uint64_t limit)
{
    if (a > limit || a < b) cesta_format_bytes_fail();
    return a - b;
}

static inline uint64_t cesta_format_bytes_div(uint64_t value, uint64_t divisor, bool remainder)
{
    if (!divisor) cesta_format_bytes_fail();
    return remainder ? value % divisor : value / divisor;
}

size_t format_bytes_copy(cesta_write_bytes destination, cesta_read_bytes source)
{
    (void)destination;
    if ((destination.len && !destination.data) || destination.len > (size_t)PTRDIFF_MAX)
        cesta_format_bytes_fail();
    (void)source;
    if ((source.len && !source.data) || source.len > (size_t)PTRDIFF_MAX)
        cesta_format_bytes_fail();
    if (!((destination.len) >= (source.len))) cesta_format_bytes_fail();
    for (size_t index = 0; index < source.len; ++index) {
        cesta_format_bytes_write(destination.data, destination.len, index, cesta_format_bytes_read(source.data, source.len, index));
    }
    size_t cesta_result = source.len;
    return cesta_result;
}

size_t format_bytes_unsigned_decimal(cesta_write_bytes destination, uint64_t value)
{
    (void)destination;
    if ((destination.len && !destination.data) || destination.len > (size_t)PTRDIFF_MAX)
        cesta_format_bytes_fail();
    (void)value;
    uint8_t cesta_2_storage[20] = {0};
    cesta_write_bytes reversed = {cesta_2_storage, (size_t)UINT64_C(20)};
    (void)reversed;
    uint64_t remaining = value;
    (void)remaining;
    size_t count = ((size_t)UINT64_C(0));
    (void)count;
    while (true) {
        cesta_format_bytes_write(reversed.data, reversed.len, count, ((uint8_t)cesta_format_bytes_add((uint64_t)(((uint8_t)UINT64_C(48))), (uint64_t)(((uint8_t)(((uint64_t)cesta_format_bytes_div((uint64_t)(remaining), (uint64_t)(((uint64_t)UINT64_C(10))), true))))), UINT64_C(255))));
        count = ((size_t)cesta_format_bytes_add((uint64_t)(count), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
        remaining = ((uint64_t)cesta_format_bytes_div((uint64_t)(remaining), (uint64_t)(((uint64_t)UINT64_C(10))), false));
        if ((remaining) == (((uint64_t)UINT64_C(0)))) {
            break;
        } else {
        }
    }
    if (!((destination.len) >= (count))) cesta_format_bytes_fail();
    size_t index = ((size_t)UINT64_C(0));
    (void)index;
    while ((index) < (count)) {
        cesta_format_bytes_write(destination.data, destination.len, index, cesta_format_bytes_read(reversed.data, reversed.len, ((size_t)cesta_format_bytes_sub((uint64_t)(((size_t)cesta_format_bytes_sub((uint64_t)(count), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)))), (uint64_t)(index), UINT64_C(18446744073709551615)))));
        index = ((size_t)cesta_format_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
    }
    size_t cesta_result = count;
    return cesta_result;
}

size_t format_bytes_unsigned_hex(cesta_write_bytes destination, uint64_t value, bool prefix)
{
    (void)destination;
    if ((destination.len && !destination.data) || destination.len > (size_t)PTRDIFF_MAX)
        cesta_format_bytes_fail();
    (void)value;
    (void)prefix;
    uint8_t cesta_3_storage[16] = {0};
    cesta_write_bytes reversed = {cesta_3_storage, (size_t)UINT64_C(16)};
    (void)reversed;
    uint64_t remaining = value;
    (void)remaining;
    size_t count = ((size_t)UINT64_C(0));
    (void)count;
    while (true) {
        const uint8_t digit = ((uint8_t)(((uint64_t)cesta_format_bytes_div((uint64_t)(remaining), (uint64_t)(((uint64_t)UINT64_C(16))), true))));
        (void)digit;
        uint8_t character = ((uint8_t)UINT64_C(0));
        (void)character;
        if ((digit) < (((uint8_t)UINT64_C(10)))) {
            character = ((uint8_t)cesta_format_bytes_add((uint64_t)(((uint8_t)UINT64_C(48))), (uint64_t)(digit), UINT64_C(255)));
        } else {
            character = ((uint8_t)cesta_format_bytes_add((uint64_t)(((uint8_t)UINT64_C(87))), (uint64_t)(digit), UINT64_C(255)));
        }
        cesta_format_bytes_write(reversed.data, reversed.len, count, character);
        count = ((size_t)cesta_format_bytes_add((uint64_t)(count), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
        remaining = ((uint64_t)cesta_format_bytes_div((uint64_t)(remaining), (uint64_t)(((uint64_t)UINT64_C(16))), false));
        if ((remaining) == (((uint64_t)UINT64_C(0)))) {
            break;
        } else {
        }
    }
    size_t start = ((size_t)UINT64_C(0));
    (void)start;
    if (prefix) {
        if (!((destination.len) >= (((size_t)UINT64_C(2))))) cesta_format_bytes_fail();
        cesta_format_bytes_write(destination.data, destination.len, ((size_t)UINT64_C(0)), ((uint8_t)UINT64_C(48)));
        cesta_format_bytes_write(destination.data, destination.len, ((size_t)UINT64_C(1)), ((uint8_t)UINT64_C(120)));
        start = ((size_t)UINT64_C(2));
    } else {
    }
    if (!((destination.len) >= (((size_t)cesta_format_bytes_add((uint64_t)(start), (uint64_t)(count), UINT64_C(18446744073709551615)))))) cesta_format_bytes_fail();
    size_t index = ((size_t)UINT64_C(0));
    (void)index;
    while ((index) < (count)) {
        cesta_format_bytes_write(destination.data, destination.len, ((size_t)cesta_format_bytes_add((uint64_t)(start), (uint64_t)(index), UINT64_C(18446744073709551615))), cesta_format_bytes_read(reversed.data, reversed.len, ((size_t)cesta_format_bytes_sub((uint64_t)(((size_t)cesta_format_bytes_sub((uint64_t)(count), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)))), (uint64_t)(index), UINT64_C(18446744073709551615)))));
        index = ((size_t)cesta_format_bytes_add((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
    }
    size_t cesta_result = ((size_t)cesta_format_bytes_add((uint64_t)(start), (uint64_t)(count), UINT64_C(18446744073709551615)));
    return cesta_result;
}
