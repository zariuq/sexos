#ifndef KALEIDOSCOPE_HTML_H
#define KALEIDOSCOPE_HTML_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kaleidoscope_script_core.h"
#define KALEIDOSCOPE_HTML_MAX_SOURCE ((size_t)16384)
#define KALEIDOSCOPE_HTML_MAX_OUTPUT ((size_t)8192)
#define KALEIDOSCOPE_HTML_MAX_LINKS ((size_t)16)
typedef struct KaleidoscopeHtmlConverted {
 size_t len,links;
 bool truncated;
 size_t target_start[KALEIDOSCOPE_HTML_MAX_LINKS],target_end[KALEIDOSCOPE_HTML_MAX_LINKS];
 uint8_t scripts_run,scripts_refused;
} KaleidoscopeHtmlConverted;
typedef struct KaleidoscopeHtmlWorkspace {
 KaleidoscopeScriptSource script_source;
 KaleidoscopeScriptWorkspace script_workspace;
 KaleidoscopeScriptResult script_result;
 uint8_t alignment[KALEIDOSCOPE_HTML_MAX_OUTPUT];
} KaleidoscopeHtmlWorkspace;
bool kaleidoscope_html_looks_like(const uint8_t *bytes,size_t length);
uint8_t kaleidoscope_html_convert(const uint8_t *html,size_t length,uint8_t *output,size_t capacity,
 KaleidoscopeHtmlWorkspace *workspace,KaleidoscopeHtmlConverted *converted);
#endif


