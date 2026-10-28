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
