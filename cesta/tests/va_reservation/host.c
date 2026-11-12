/* The production reservation routine; RAM and mapping routines are not called. */
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define memcpy sexos_memcpy
#define memmove sexos_memmove
#define memset sexos_memset
#define memcmp sexos_memcmp
#include "kernel_memory.c"
#undef memcpy
#undef memmove
#undef memset
#undef memcmp

Kernel kernel;

int main(void)
{
    uint64_t cursor, bytes;
    static Frames before;
    while (scanf("%" SCNu64 " %" SCNu64, &cursor, &bytes) == 2) {
        memset(&frames, 0xa5, sizeof frames);
        frames.va_next = cursor;
        memcpy(&before, &frames, sizeof frames);
        uint64_t result = va_alloc(bytes);
        before.va_next = frames.va_next;
        assert(!memcmp(&before, &frames, sizeof frames));
        printf("%" PRIu64 " %" PRIu64 "\n", result, frames.va_next);
    }
    return 0;
}
