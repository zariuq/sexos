#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif

#include "silk_shell_surfaces.h"
const CommandDef COMMAND_LIST[11] = {
    {0, "Open Spindle"}, {1, "Open Quil"},        {2, "Open Linen"},    {3, "Open Atlas"},
    {4, "Open Bell"},    {5, "Open Collar"},      {6, "Open Mesh"},     {7, "Restore Minimized"},
    {8, "Zoom Toggle"},  {9, "Minimize Focused"}, {10, "Open Browser"},
};

uint8_t shell_surfaces_command_code(size_t index) {
    return index < 11 ? COMMAND_LIST[index].command : 255;
}

ShellPaletteString shell_surfaces_command_label(size_t index) {
    ShellPaletteString result = {0};
    if (index >= 11)
        return result;
    const char *name = COMMAND_LIST[index].name;
    while (result.len < 127 && name[result.len]) {
        result.bytes[result.len] = name[result.len];
        ++result.len;
    }
    return result;
}


