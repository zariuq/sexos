#ifndef KALEIDOSCOPE_SCRIPT_LEXER_H
#define KALEIDOSCOPE_SCRIPT_LEXER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define KALEIDOSCOPE_SCRIPT_MAX_SOURCE ((size_t)4096)
enum {KALEIDOSCOPE_SCRIPT_OK=0,KALEIDOSCOPE_SCRIPT_UNSUPPORTED=1,KALEIDOSCOPE_SCRIPT_SYNTAX=2,KALEIDOSCOPE_SCRIPT_LIMIT=3,KALEIDOSCOPE_SCRIPT_THROWN=4};
enum {KALEIDOSCOPE_TOKEN_END=0,KALEIDOSCOPE_TOKEN_NUMBER=1,KALEIDOSCOPE_TOKEN_STRING=2,
 KALEIDOSCOPE_TOKEN_TEMPLATE=3,KALEIDOSCOPE_TOKEN_REGEX=4,KALEIDOSCOPE_TOKEN_IDENT=5,
 KALEIDOSCOPE_TOKEN_PUNCT=6};
typedef struct KaleidoscopeScriptSource {uint8_t bytes[KALEIDOSCOPE_SCRIPT_MAX_SOURCE];size_t len;} KaleidoscopeScriptSource;
typedef struct KaleidoscopeScriptToken {
 uint8_t kind,punct;bool newline;double number;
 size_t start,len,aux_start,aux_len;
} KaleidoscopeScriptToken;
typedef struct KaleidoscopeScriptLexer {size_t at;KaleidoscopeScriptToken current;} KaleidoscopeScriptLexer;
void kaleidoscope_script_lexer_init(KaleidoscopeScriptLexer *state);
uint8_t kaleidoscope_script_lexer_next(const KaleidoscopeScriptSource *source,KaleidoscopeScriptLexer *state,KaleidoscopeScriptToken *out);
#endif
