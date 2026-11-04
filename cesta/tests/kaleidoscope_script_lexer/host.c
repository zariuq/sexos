#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kaleidoscope_script_lexer.h"
#include "cases.h"

static void hex(const uint8_t *bytes, size_t length) {
    for (size_t i = 0; i < length; i++) printf("%02x", bytes[i]);
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    size_t index = strtoull(argv[1], NULL, 10);
    if (index >= sizeof cases / sizeof *cases) return 2;
    KaleidoscopeScriptSource source = {0};
    source.len = cases[index].len;
    memcpy(source.bytes, cases[index].bytes, source.len);
    KaleidoscopeScriptLexer lexer;
    kaleidoscope_script_lexer_init(&lexer);
    for (;;) {
        KaleidoscopeScriptToken token = {0};
        unsigned error = kaleidoscope_script_lexer_next(&source, &lexer, &token);
        if (error) { printf("E %u\n", error); break; }
        printf("T %u ", (unsigned)token.newline);
        if (token.kind == KALEIDOSCOPE_TOKEN_END) { puts("0"); break; }
        if (token.kind == KALEIDOSCOPE_TOKEN_NUMBER) {
            uint64_t bits;
            memcpy(&bits, &token.number, sizeof bits);
            printf("1 %016" PRIx64 "\n", bits);
        } else if (token.kind == KALEIDOSCOPE_TOKEN_STRING ||
                   token.kind == KALEIDOSCOPE_TOKEN_TEMPLATE ||
                   token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
            printf("%u ", (unsigned)token.kind);
            hex(source.bytes + token.start, token.len);
            putchar('\n');
        } else if (token.kind == KALEIDOSCOPE_TOKEN_REGEX) {
            printf("4 ");
            hex(source.bytes + token.start, token.len);
            printf(" /");
            hex(source.bytes + token.aux_start, token.aux_len);
            putchar('\n');
        } else {
            printf("6 %u\n", (unsigned)token.punct);
        }
    }
    return 0;
}
