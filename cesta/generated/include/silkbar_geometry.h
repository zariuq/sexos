
#ifndef SEXOS_SILKBAR_GEOMETRY_H
#define SEXOS_SILKBAR_GEOMETRY_H
#include <stddef.h>
#include <stdint.h>

enum {
    SILKBAR_DISPATCH_NONE = 0,
    SILKBAR_DISPATCH_LAUNCHER = 1,
    SILKBAR_DISPATCH_WORKSPACE = 2,
    SILKBAR_DISPATCH_STATUS = 3,
    SILKBAR_DISPATCH_CLOCK = 4,
    SILKBAR_DISPATCH_BELL = 5
};

typedef struct SilkBarDispatch {
    uint8_t kind, workspace;
} SilkBarDispatch;

SilkBarDispatch silkbar_geometry_dispatch(size_t width, size_t x, size_t y);
#endif
