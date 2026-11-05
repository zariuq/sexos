
#ifndef SEXOS_SOURCE_PROFILE_H
#define SEXOS_SOURCE_PROFILE_H
#include "source_profile_rows.h"
#define SOURCE_CONSTANT(name, value)                                                               \
    enum {                                                                                         \
        name = value                                                                               \
    };
SEXOS_SOURCE_TRUST_CONSTANTS(SOURCE_CONSTANT)
#undef SOURCE_CONSTANT
#endif
