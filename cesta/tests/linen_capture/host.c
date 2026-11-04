#include "linen_capture.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static void print_frame(const LinenCaptureFrame *frame)
{
    printf("%zu ", frame->len);
    for (size_t i = 0; i < frame->len; ++i) printf("%02x", frame->bytes[i]);
    puts("");
}

int main(void)
{
    LinenCaptureTable table = {0};
    LinenCaptureServer server = linen_capture_new(13, 13);
    LinenCaptureFrame assembled = {0};
    char command[16];
    while (scanf("%15s", command) == 1) {
        if (!strcmp(command, "clear")) {
            table = (LinenCaptureTable){0};
        } else if (!strcmp(command, "row")) {
            size_t i; unsigned present, kind, disk, length;
            uint64_t id;
            assert(scanf("%zu%u%" SCNu64 "%u%u%u", &i, &present, &id, &kind, &disk, &length) == 6);
            assert(i < LINEN_CAPTURE_SLOTS && length <= LINEN_CAPTURE_NAME_CAPACITY);
            table.rows[i] = (LinenCaptureRow){.present = present != 0, .id = id,
                .kind = (uint8_t)kind, .disk = (uint8_t)disk, .len = (uint8_t)length};
            for (size_t j = 0; j < length; ++j) {
                unsigned byte; assert(scanf("%u", &byte) == 1);
                table.rows[i].name[j] = (uint8_t)byte;
            }
        } else if (!strcmp(command, "encode")) {
            uint64_t revision; assert(scanf("%" SCNu64, &revision) == 1);
            LinenCaptureFrame frame = {0};
            if (linen_capture_encode(&table, revision, &frame)) print_frame(&frame); else puts("invalid");
        } else if (!strcmp(command, "decode")) {
            LinenCaptureFrame frame = {0}; size_t n;
            assert(scanf("%zu", &n) == 1 && n <= LINEN_CAPTURE_MAX_BYTES);
            frame.len = n;
            for (size_t i = 0; i < n; ++i) {unsigned byte; assert(scanf("%u", &byte) == 1); frame.bytes[i] = (uint8_t)byte;}
            LinenCaptureTable decoded = {0};
            if (!linen_capture_decode(&frame, &decoded)) puts("invalid");
            else {
                uint64_t revision = 0;
                for (size_t i = 0; i < 8; ++i) revision |= (uint64_t)frame.bytes[LINEN_CAPTURE_OFFSET_REVISION + i] << (8 * i);
                LinenCaptureFrame encoded = {0}; assert(linen_capture_encode(&decoded, revision, &encoded)); print_frame(&encoded);
            }
        } else if (!strcmp(command, "reset")) {
            server = linen_capture_new(13, 13); assembled = (LinenCaptureFrame){0};
        } else if (!strcmp(command, "counter")) {
            assert(scanf("%" SCNu64, &server.revision) == 1);
        } else if (!strcmp(command, "begin") || !strcmp(command, "read") || !strcmp(command, "end")) {
            uint32_t caller, task; uint64_t job, a, b, result;
            assert(scanf("%" SCNu32 "%" SCNu32 "%" SCNu64 "%" SCNu64 "%" SCNu64, &caller, &task, &job, &a, &b) == 5);
            if (!strcmp(command, "begin")) result = linen_capture_begin(&server, caller, task, job, a, b, &table);
            else if (!strcmp(command, "read")) result = linen_capture_read(&server, caller, task, job, a, b);
            else result = linen_capture_end(&server, caller, task, job, a, b);
            printf("%016" PRIx64 "\n", result);
        } else if (!strcmp(command, "append")) {
            uint64_t word; assert(scanf("%" SCNu64, &word) == 1);
            bool ok = linen_capture_append(&assembled, word);
            printf("%u ", (unsigned)ok); print_frame(&assembled);
        } else assert(!"unknown test command");
    }
    return 0;
}
