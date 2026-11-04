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

#ifdef sexstore_pages_failure
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_crc32c_update
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_crc32c
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_crc16_ibm_update
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_crc16_ibm
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_load_le
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_store_le
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_record_offset
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_page_valid
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_page_seq
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_page_begin
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_page_put_record
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_page_seal
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_record_valid
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_record_state
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_record_generation
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_record_key
#error "checked identifier is a preprocessor macro"
#endif
#ifdef sexstore_pages_record_value
#error "checked identifier is a preprocessor macro"
#endif
#ifdef crc
#error "checked identifier is a preprocessor macro"
#endif
#ifdef bytes
#error "checked identifier is a preprocessor macro"
#endif
#ifdef state
#error "checked identifier is a preprocessor macro"
#endif
#ifdef index
#error "checked identifier is a preprocessor macro"
#endif
#ifdef bit
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_result
#error "checked identifier is a preprocessor macro"
#endif
#ifdef offset
#error "checked identifier is a preprocessor macro"
#endif
#ifdef width
#error "checked identifier is a preprocessor macro"
#endif
#ifdef field
#error "checked identifier is a preprocessor macro"
#endif
#ifdef value
#error "checked identifier is a preprocessor macro"
#endif
#ifdef page
#error "checked identifier is a preprocessor macro"
#endif
#ifdef page_id
#error "checked identifier is a preprocessor macro"
#endif
#ifdef stored
#error "checked identifier is a preprocessor macro"
#endif
#ifdef zeros
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_3_storage
#error "checked identifier is a preprocessor macro"
#endif
#ifdef head
#error "checked identifier is a preprocessor macro"
#endif
#ifdef tail
#error "checked identifier is a preprocessor macro"
#endif
#ifdef through_head
#error "checked identifier is a preprocessor macro"
#endif
#ifdef through_field
#error "checked identifier is a preprocessor macro"
#endif
#ifdef through_tail
#error "checked identifier is a preprocessor macro"
#endif
#ifdef valid
#error "checked identifier is a preprocessor macro"
#endif
#ifdef seq
#error "checked identifier is a preprocessor macro"
#endif
#ifdef generation
#error "checked identifier is a preprocessor macro"
#endif
#ifdef key
#error "checked identifier is a preprocessor macro"
#endif
#ifdef flags
#error "checked identifier is a preprocessor macro"
#endif
#ifdef record
#error "checked identifier is a preprocessor macro"
#endif
#ifdef whole
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_1_start
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_1_count
#error "checked identifier is a preprocessor macro"
#endif
#ifdef magic
#error "checked identifier is a preprocessor macro"
#endif
#ifdef slot_id
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_6_storage
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_sexstore_pages_read
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_sexstore_pages_write
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_sexstore_pages_add
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_sexstore_pages_mul
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_sexstore_pages_shift
#error "checked identifier is a preprocessor macro"
#endif
#ifdef cesta_sexstore_pages_fail
#error "checked identifier is a preprocessor macro"
#endif

#ifndef CESTA_CHECKED_BYTE_VIEWS_DEFINED
#define CESTA_CHECKED_BYTE_VIEWS_DEFINED
typedef struct { const uint8_t *data; size_t len; } cesta_read_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_write_bytes;
typedef struct { uint8_t *data; size_t len; } cesta_owned_bytes;
#endif

void sexstore_pages_failure(void);
uint32_t sexstore_pages_crc32c_update(uint32_t, cesta_read_bytes);
uint32_t sexstore_pages_crc32c(cesta_read_bytes);
uint16_t sexstore_pages_crc16_ibm_update(uint16_t, cesta_read_bytes);
uint16_t sexstore_pages_crc16_ibm(cesta_read_bytes);
uint64_t sexstore_pages_load_le(cesta_read_bytes, size_t, size_t);
void sexstore_pages_store_le(cesta_write_bytes, size_t, size_t, uint64_t);
size_t sexstore_pages_record_offset(size_t);
bool sexstore_pages_page_valid(cesta_read_bytes);
uint32_t sexstore_pages_page_seq(cesta_read_bytes);
void sexstore_pages_page_begin(cesta_write_bytes, uint32_t);
void sexstore_pages_page_put_record(cesta_write_bytes, size_t, uint8_t, uint8_t, uint32_t, uint64_t);
void sexstore_pages_page_seal(cesta_write_bytes);
bool sexstore_pages_record_valid(cesta_read_bytes, size_t);
uint8_t sexstore_pages_record_state(cesta_read_bytes, size_t);
uint8_t sexstore_pages_record_generation(cesta_read_bytes, size_t);
uint32_t sexstore_pages_record_key(cesta_read_bytes, size_t);
uint64_t sexstore_pages_record_value(cesta_read_bytes, size_t);

static CESTA_CHECKED_COLD _Noreturn void cesta_sexstore_pages_fail(void)
{
    sexstore_pages_failure();
    for (;;) {}
}

static inline uint8_t cesta_sexstore_pages_read(const uint8_t *data, size_t len, size_t index)
{
    if (index >= len) cesta_sexstore_pages_fail();
    return data[index];
}

static inline void cesta_sexstore_pages_write(uint8_t *data, size_t len, size_t index, uint8_t value)
{
    if (index >= len) cesta_sexstore_pages_fail();
    data[index] = value;
}

static inline uint64_t cesta_sexstore_pages_add(uint64_t a, uint64_t b, uint64_t limit)
{
    if (b > limit || a > limit - b) cesta_sexstore_pages_fail();
    return a + b;
}

static inline uint64_t cesta_sexstore_pages_mul(uint64_t a, uint64_t b, uint64_t limit)
{
    if (a > limit || b > limit || (b && a > limit / b)) cesta_sexstore_pages_fail();
    return a * b;
}

static inline uint64_t cesta_sexstore_pages_shift(uint64_t value, size_t count, unsigned width, bool left)
{
    if (count >= width) cesta_sexstore_pages_fail();
    return left ? value << count : value >> count;
}

uint32_t sexstore_pages_crc32c_update(uint32_t crc, cesta_read_bytes bytes)
{
    (void)crc;
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    uint32_t state = crc;
    (void)state;
    for (size_t index = 0; index < bytes.len; ++index) {
        state = ((uint32_t)(((uint64_t)(state)) ^ ((uint64_t)(((uint32_t)(cesta_sexstore_pages_read(bytes.data, bytes.len, index)))))));
        size_t bit = ((size_t)UINT64_C(0));
        (void)bit;
        while ((bit) < (((size_t)UINT64_C(8)))) {
            if ((((uint32_t)(((uint64_t)(state)) & ((uint64_t)(((uint32_t)UINT64_C(1))))))) != (((uint32_t)UINT64_C(0)))) {
                state = ((uint32_t)(((uint64_t)(((uint32_t)cesta_sexstore_pages_shift((uint64_t)(state), ((size_t)UINT64_C(1)), 32, false)))) ^ ((uint64_t)(((uint32_t)UINT64_C(2197175160))))));
            } else {
                state = ((uint32_t)cesta_sexstore_pages_shift((uint64_t)(state), ((size_t)UINT64_C(1)), 32, false));
            }
            bit = ((size_t)cesta_sexstore_pages_add((uint64_t)(bit), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
        }
    }
    uint32_t cesta_result = state;
    return cesta_result;
}

uint32_t sexstore_pages_crc32c(cesta_read_bytes bytes)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    const uint32_t state = sexstore_pages_crc32c_update(((uint32_t)UINT64_C(4294967295)), (cesta_read_bytes){bytes.data, bytes.len});
    (void)state;
    uint32_t cesta_result = ((uint32_t)(((uint64_t)(state)) ^ ((uint64_t)(((uint32_t)UINT64_C(4294967295))))));
    return cesta_result;
}

uint16_t sexstore_pages_crc16_ibm_update(uint16_t crc, cesta_read_bytes bytes)
{
    (void)crc;
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    uint16_t state = crc;
    (void)state;
    for (size_t index = 0; index < bytes.len; ++index) {
        state = ((uint16_t)(((uint64_t)(state)) ^ ((uint64_t)(((uint16_t)cesta_sexstore_pages_shift((uint64_t)(((uint16_t)(cesta_sexstore_pages_read(bytes.data, bytes.len, index)))), ((size_t)UINT64_C(8)), 16, true))))));
        size_t bit = ((size_t)UINT64_C(0));
        (void)bit;
        while ((bit) < (((size_t)UINT64_C(8)))) {
            if ((((uint16_t)(((uint64_t)(state)) & ((uint64_t)(((uint16_t)UINT64_C(32768))))))) != (((uint16_t)UINT64_C(0)))) {
                state = ((uint16_t)(((uint64_t)(((uint16_t)cesta_sexstore_pages_shift((uint64_t)(state), ((size_t)UINT64_C(1)), 16, true)))) ^ ((uint64_t)(((uint16_t)UINT64_C(32773))))));
            } else {
                state = ((uint16_t)cesta_sexstore_pages_shift((uint64_t)(state), ((size_t)UINT64_C(1)), 16, true));
            }
            bit = ((size_t)cesta_sexstore_pages_add((uint64_t)(bit), (uint64_t)(((size_t)UINT64_C(1))), UINT64_C(18446744073709551615)));
        }
    }
    uint16_t cesta_result = state;
    return cesta_result;
}

uint16_t sexstore_pages_crc16_ibm(cesta_read_bytes bytes)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    const uint16_t state = sexstore_pages_crc16_ibm_update(((uint16_t)UINT64_C(0)), (cesta_read_bytes){bytes.data, bytes.len});
    (void)state;
    uint16_t cesta_result = state;
    return cesta_result;
}

uint64_t sexstore_pages_load_le(cesta_read_bytes bytes, size_t offset, size_t width)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)offset;
    (void)width;
    if (!((width) <= (((size_t)UINT64_C(8))))) cesta_sexstore_pages_fail();
    const size_t cesta_3_start = offset;
    const size_t cesta_3_count = width;
    if (cesta_3_start > bytes.len || cesta_3_count > bytes.len - cesta_3_start)
        cesta_sexstore_pages_fail();
    cesta_read_bytes field = {bytes.data, cesta_3_count};
    if (cesta_3_start) field.data += cesta_3_start;
    (void)field;
    uint64_t value = ((uint64_t)UINT64_C(0));
    (void)value;
    for (size_t index = 0; index < field.len; ++index) {
        value = ((uint64_t)(((uint64_t)(value)) | ((uint64_t)(((uint64_t)cesta_sexstore_pages_shift((uint64_t)(((uint64_t)(cesta_sexstore_pages_read(field.data, field.len, index)))), ((size_t)cesta_sexstore_pages_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), 64, true))))));
    }
    uint64_t cesta_result = value;
    return cesta_result;
}

void sexstore_pages_store_le(cesta_write_bytes bytes, size_t offset, size_t width, uint64_t value)
{
    (void)bytes;
    if ((bytes.len && !bytes.data) || bytes.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)offset;
    (void)width;
    (void)value;
    if (!((width) <= (((size_t)UINT64_C(8))))) cesta_sexstore_pages_fail();
    const size_t cesta_4_start = offset;
    const size_t cesta_4_count = width;
    if (cesta_4_start > bytes.len || cesta_4_count > bytes.len - cesta_4_start)
        cesta_sexstore_pages_fail();
    cesta_write_bytes field = {bytes.data, cesta_4_count};
    if (cesta_4_start) field.data += cesta_4_start;
    (void)field;
    for (size_t index = 0; index < field.len; ++index) {
        cesta_sexstore_pages_write(field.data, field.len, index, ((uint8_t)(((uint64_t)cesta_sexstore_pages_shift((uint64_t)(value), ((size_t)cesta_sexstore_pages_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), 64, false)))));
    }
    return;
}

size_t sexstore_pages_record_offset(size_t index)
{
    (void)index;
    if (!((index) < (((size_t)UINT64_C(16))))) cesta_sexstore_pages_fail();
    size_t cesta_result = ((size_t)cesta_sexstore_pages_add((uint64_t)(((size_t)UINT64_C(16))), (uint64_t)(((size_t)cesta_sexstore_pages_mul((uint64_t)(index), (uint64_t)(((size_t)UINT64_C(24))), UINT64_C(18446744073709551615)))), UINT64_C(18446744073709551615)));
    return cesta_result;
}

bool sexstore_pages_page_valid(cesta_read_bytes page)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    if (!((page.len) == (((size_t)UINT64_C(512))))) cesta_sexstore_pages_fail();
    const uint64_t page_id = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, ((size_t)UINT64_C(0)), ((size_t)UINT64_C(4)));
    (void)page_id;
    if ((page_id) != (((uint64_t)UINT64_C(42405)))) {
        bool cesta_result = false;
        return cesta_result;
    } else {
    }
    const uint64_t stored = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, ((size_t)UINT64_C(8)), ((size_t)UINT64_C(4)));
    (void)stored;
    uint8_t cesta_3_storage[4] = {0};
    cesta_write_bytes zeros = {cesta_3_storage, (size_t)UINT64_C(4)};
    (void)zeros;
    const size_t cesta_4_start = ((size_t)UINT64_C(0));
    const size_t cesta_4_count = ((size_t)UINT64_C(8));
    if (cesta_4_start > page.len || cesta_4_count > page.len - cesta_4_start)
        cesta_sexstore_pages_fail();
    cesta_read_bytes head = {page.data, cesta_4_count};
    if (cesta_4_start) head.data += cesta_4_start;
    (void)head;
    const size_t cesta_5_start = ((size_t)UINT64_C(12));
    const size_t cesta_5_count = ((size_t)UINT64_C(500));
    if (cesta_5_start > page.len || cesta_5_count > page.len - cesta_5_start)
        cesta_sexstore_pages_fail();
    cesta_read_bytes tail = {page.data, cesta_5_count};
    if (cesta_5_start) tail.data += cesta_5_start;
    (void)tail;
    const uint32_t through_head = sexstore_pages_crc32c_update(((uint32_t)UINT64_C(4294967295)), (cesta_read_bytes){head.data, head.len});
    (void)through_head;
    const uint32_t through_field = sexstore_pages_crc32c_update(through_head, (cesta_read_bytes){zeros.data, zeros.len});
    (void)through_field;
    const uint32_t through_tail = sexstore_pages_crc32c_update(through_field, (cesta_read_bytes){tail.data, tail.len});
    (void)through_tail;
    bool cesta_result = (stored) == (((uint64_t)(((uint32_t)(((uint64_t)(through_tail)) ^ ((uint64_t)(((uint32_t)UINT64_C(4294967295)))))))));
    return cesta_result;
}

uint32_t sexstore_pages_page_seq(cesta_read_bytes page)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    const bool valid = sexstore_pages_page_valid((cesta_read_bytes){page.data, page.len});
    (void)valid;
    if (valid) {
        const uint64_t seq = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, ((size_t)UINT64_C(4)), ((size_t)UINT64_C(4)));
        (void)seq;
        uint32_t cesta_result = ((uint32_t)(seq));
        return cesta_result;
    } else {
    }
    uint32_t cesta_result = ((uint32_t)UINT64_C(0));
    return cesta_result;
}

void sexstore_pages_page_begin(cesta_write_bytes page, uint32_t seq)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)seq;
    if (!((page.len) == (((size_t)UINT64_C(512))))) cesta_sexstore_pages_fail();
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)UINT64_C(0)), ((size_t)UINT64_C(4)), ((uint64_t)UINT64_C(42405)));
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)UINT64_C(4)), ((size_t)UINT64_C(4)), ((uint64_t)(seq)));
    return;
}

void sexstore_pages_page_put_record(cesta_write_bytes page, size_t index, uint8_t state, uint8_t generation, uint32_t key, uint64_t value)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)index;
    (void)state;
    (void)generation;
    (void)key;
    (void)value;
    if (!((page.len) == (((size_t)UINT64_C(512))))) cesta_sexstore_pages_fail();
    const size_t offset = sexstore_pages_record_offset(index);
    (void)offset;
    uint8_t flags = ((uint8_t)UINT64_C(0));
    (void)flags;
    if ((state) == (((uint8_t)UINT64_C(1)))) {
        flags = ((uint8_t)UINT64_C(1));
    } else {
    }
    if ((state) == (((uint8_t)UINT64_C(2)))) {
        flags = ((uint8_t)UINT64_C(2));
    } else {
    }
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, offset, ((size_t)UINT64_C(2)), ((uint64_t)UINT64_C(54757)));
    cesta_sexstore_pages_write(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(2))), UINT64_C(18446744073709551615))), ((uint8_t)UINT64_C(1)));
    cesta_sexstore_pages_write(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(3))), UINT64_C(18446744073709551615))), flags);
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(4))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(2)), ((uint64_t)(index)));
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(6))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(2)), ((uint64_t)UINT64_C(0)));
    cesta_sexstore_pages_write(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))), state);
    cesta_sexstore_pages_write(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(9))), UINT64_C(18446744073709551615))), generation);
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(10))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(2)), ((uint64_t)UINT64_C(0)));
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(12))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(4)), ((uint64_t)(key)));
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(16))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(8)), value);
    const size_t cesta_8_start = offset;
    const size_t cesta_8_count = ((size_t)UINT64_C(24));
    if (cesta_8_start > page.len || cesta_8_count > page.len - cesta_8_start)
        cesta_sexstore_pages_fail();
    cesta_read_bytes record = {page.data, cesta_8_count};
    if (cesta_8_start) record.data += cesta_8_start;
    (void)record;
    const uint16_t crc = sexstore_pages_crc16_ibm((cesta_read_bytes){record.data, record.len});
    (void)crc;
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(6))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(2)), ((uint64_t)(crc)));
    return;
}

void sexstore_pages_page_seal(cesta_write_bytes page)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    if (!((page.len) == (((size_t)UINT64_C(512))))) cesta_sexstore_pages_fail();
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)UINT64_C(8)), ((size_t)UINT64_C(4)), ((uint64_t)UINT64_C(0)));
    cesta_read_bytes whole = (cesta_read_bytes){page.data, page.len};
    (void)whole;
    const uint32_t crc = sexstore_pages_crc32c((cesta_read_bytes){whole.data, whole.len});
    (void)crc;
    sexstore_pages_store_le((cesta_write_bytes){page.data, page.len}, ((size_t)UINT64_C(8)), ((size_t)UINT64_C(4)), ((uint64_t)(crc)));
    return;
}

bool sexstore_pages_record_valid(cesta_read_bytes page, size_t index)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)index;
    if (!((page.len) == (((size_t)UINT64_C(512))))) cesta_sexstore_pages_fail();
    const size_t offset = sexstore_pages_record_offset(index);
    (void)offset;
    const uint64_t magic = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, offset, ((size_t)UINT64_C(2)));
    (void)magic;
    if ((magic) != (((uint64_t)UINT64_C(54757)))) {
        bool cesta_result = false;
        return cesta_result;
    } else {
    }
    if ((cesta_sexstore_pages_read(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(2))), UINT64_C(18446744073709551615))))) != (((uint8_t)UINT64_C(1)))) {
        bool cesta_result = false;
        return cesta_result;
    } else {
    }
    const uint64_t slot_id = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(4))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(2)));
    (void)slot_id;
    if ((slot_id) != (((uint64_t)(index)))) {
        bool cesta_result = false;
        return cesta_result;
    } else {
    }
    const uint64_t stored = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(6))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(2)));
    (void)stored;
    uint8_t cesta_6_storage[2] = {0};
    cesta_write_bytes zeros = {cesta_6_storage, (size_t)UINT64_C(2)};
    (void)zeros;
    const size_t cesta_7_start = offset;
    const size_t cesta_7_count = ((size_t)UINT64_C(6));
    if (cesta_7_start > page.len || cesta_7_count > page.len - cesta_7_start)
        cesta_sexstore_pages_fail();
    cesta_read_bytes head = {page.data, cesta_7_count};
    if (cesta_7_start) head.data += cesta_7_start;
    (void)head;
    const size_t cesta_8_start = ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615)));
    const size_t cesta_8_count = ((size_t)UINT64_C(16));
    if (cesta_8_start > page.len || cesta_8_count > page.len - cesta_8_start)
        cesta_sexstore_pages_fail();
    cesta_read_bytes tail = {page.data, cesta_8_count};
    if (cesta_8_start) tail.data += cesta_8_start;
    (void)tail;
    const uint16_t through_head = sexstore_pages_crc16_ibm_update(((uint16_t)UINT64_C(0)), (cesta_read_bytes){head.data, head.len});
    (void)through_head;
    const uint16_t through_field = sexstore_pages_crc16_ibm_update(through_head, (cesta_read_bytes){zeros.data, zeros.len});
    (void)through_field;
    const uint16_t through_tail = sexstore_pages_crc16_ibm_update(through_field, (cesta_read_bytes){tail.data, tail.len});
    (void)through_tail;
    if ((((uint64_t)(through_tail))) != (stored)) {
        bool cesta_result = false;
        return cesta_result;
    } else {
    }
    bool cesta_result = (cesta_sexstore_pages_read(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))))) <= (((uint8_t)UINT64_C(2)));
    return cesta_result;
}

uint8_t sexstore_pages_record_state(cesta_read_bytes page, size_t index)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)index;
    const size_t offset = sexstore_pages_record_offset(index);
    (void)offset;
    uint8_t cesta_result = cesta_sexstore_pages_read(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(8))), UINT64_C(18446744073709551615))));
    return cesta_result;
}

uint8_t sexstore_pages_record_generation(cesta_read_bytes page, size_t index)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)index;
    const size_t offset = sexstore_pages_record_offset(index);
    (void)offset;
    uint8_t cesta_result = cesta_sexstore_pages_read(page.data, page.len, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(9))), UINT64_C(18446744073709551615))));
    return cesta_result;
}

uint32_t sexstore_pages_record_key(cesta_read_bytes page, size_t index)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)index;
    const size_t offset = sexstore_pages_record_offset(index);
    (void)offset;
    const uint64_t key = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(12))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(4)));
    (void)key;
    uint32_t cesta_result = ((uint32_t)(key));
    return cesta_result;
}

uint64_t sexstore_pages_record_value(cesta_read_bytes page, size_t index)
{
    (void)page;
    if ((page.len && !page.data) || page.len > (size_t)PTRDIFF_MAX)
        cesta_sexstore_pages_fail();
    (void)index;
    const size_t offset = sexstore_pages_record_offset(index);
    (void)offset;
    const uint64_t value = sexstore_pages_load_le((cesta_read_bytes){page.data, page.len}, ((size_t)cesta_sexstore_pages_add((uint64_t)(offset), (uint64_t)(((size_t)UINT64_C(16))), UINT64_C(18446744073709551615))), ((size_t)UINT64_C(8)));
    (void)value;
    uint64_t cesta_result = value;
    return cesta_result;
}

