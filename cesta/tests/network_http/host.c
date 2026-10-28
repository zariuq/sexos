#include "network_service.h"
#include <assert.h>
#include <stdio.h>

static unsigned nybble(char c)
{
    if (c >= '0' && c <= '9') return (unsigned)(c - '0');
    assert(c >= 'a' && c <= 'f');
    return (unsigned)(c - 'a' + 10);
}

int main(void)
{
    unsigned end;
    size_t split, stride;
    char text[32769];
    while (scanf("%u %zu %zu %32768s", &end, &split, &stride, text) == 4) {
        uint8_t wire[16384];
        size_t length = 0;
        for (size_t i = 0; text[i]; i += 2) {
            assert(text[i+1] && length < sizeof wire);
            wire[length++] = (uint8_t)(nybble(text[i]) * 16 + nybble(text[i+1]));
        }
        assert(stride > 0 && split <= length);
        HttpResponse response;
        http_response_init(&response);
        size_t offset = 0;
        if (split) {
            assert(http_response_feed(&response, (cesta_read_bytes){wire, split}) <= split);
            offset = split;
        }
        while (offset < length) {
            size_t count = length - offset;
            if (count > stride) count = stride;
            assert(http_response_feed(&response, (cesta_read_bytes){wire+offset, count}) <= count);
            offset += count;
        }
        if (end) http_response_end(&response);
        printf("%u %u %u %zu ", (unsigned)response.state, (unsigned)response.error,
               (unsigned)response.status, response.length);
        for (size_t i = 0; i < response.length; ++i) printf("%02x", response.body[i]);
        putchar('\n');
    }
    return 0;
}
