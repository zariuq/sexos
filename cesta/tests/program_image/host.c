/* Hosted observations of the actual pure image parser. */
#include "program_image.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void program_image_bounds_failure(void) { abort(); }

static uint64_t little(const unsigned char *data, size_t length)
{
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i) value |= (uint64_t)data[i] << (8 * i);
    return value;
}

int main(void)
{
    unsigned char header[12];
    size_t got;
    while ((got = fread(header, 1, sizeof header, stdin)) != 0) {
        if (got != sizeof header) return 2;
        uint64_t window = little(header, 8);
        size_t length = (size_t)little(header + 8, 4);
        if (length > 64u * 1024u * 1024u) return 3;
        unsigned char *bytes = malloc(length ? length : 1);
        unsigned char *before = malloc(length ? length : 1);
        if (!bytes || !before || fread(bytes, 1, length, stdin) != length) return 4;
        memcpy(before, bytes, length);
        ProgramImage image = program_image_parse((cesta_read_bytes){bytes, length}, window);
        assert(image.source.data == bytes && image.source.len == length);
        printf("%u", image.error);
        if (!image.error) {
            assert(image.count <= PROGRAM_IMAGE_SEGMENTS && image.window == window);
            printf(" %zu %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64,
                   image.count, image.entry, image.bias, image.rela, image.rela_count, image.stack);
            for (size_t i = 0; i < image.count; ++i) {
                ProgramSegment s = image.segments[i];
                printf(" %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %u %u",
                       s.offset, s.file, s.window, s.memory, (unsigned)s.writable, (unsigned)s.executable);
            }
            for (uint64_t i = 0; i < image.rela_count; ++i) {
                ProgramRelocation r = program_image_relocation(&image, i, 0);
                assert(r.valid);
                printf(" %" PRIu64 " %" PRIu64, r.offset, r.value);
                if (r.value) assert(!program_image_relocation(&image, i, UINT64_MAX).valid);
            }
            assert(!program_image_relocation(&image, image.rela_count, 0).valid);
            assert(!program_image_relocation(&image, UINT64_MAX, 0).valid);
        }
        printf(" | %u\n", (unsigned)program_image_kernel_allowed(&image));
        assert(!memcmp(before, bytes, length));
        free(before);
        free(bytes);
    }
    ProgramImage empty = program_image_parse((cesta_read_bytes){NULL, 0}, 4096);
    assert(empty.error == PROGRAM_IMAGE_HEADER && empty.source.data == NULL && !empty.source.len);
    return ferror(stdin) ? 5 : 0;
}
