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
