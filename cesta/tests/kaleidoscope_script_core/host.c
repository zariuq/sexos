#include "kaleidoscope_script_core.h"
#include "cases.h"
#include <stdio.h>
#include <stdlib.h>

static KaleidoscopeScriptWorkspace workspace;
static KaleidoscopeScriptResult result;

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    size_t index = (size_t)strtoul(argv[1], NULL, 10);
    if (index >= sizeof(cases) / sizeof(cases[0])) return 3;
    uint8_t error = kaleidoscope_script_run(&cases[index], &workspace, &result);
    if (error) {
        static const char labels[] = "?USLT";
        printf("E %c\n", error < sizeof(labels) - 1 ? labels[error] : '?');
        return 0;
    }
    fputs("O ", stdout);
    for (size_t i = 0; i < result.len; ++i) printf("%02x", result.bytes[i]);
    putchar('\n');
    return 0;
}
