#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

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
