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
