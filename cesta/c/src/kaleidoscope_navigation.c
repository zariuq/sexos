#include "kaleidoscope_navigation.h"
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

void kaleidoscope_http_path_initial(KaleidoscopeHttpPath *out) {
    static const uint8_t initial[] = "/welcome";
    *out = (KaleidoscopeHttpPath){0};
    for (size_t i = 0; i < sizeof initial - 1; i++) {
        out->bytes[i] = initial[i];
    }
    out->len = sizeof initial - 1;
}

bool kaleidoscope_http_path_parse(const KaleidoscopeHttpPath *input, KaleidoscopeHttpPath *out) {
    *out = (KaleidoscopeHttpPath){0};
    if (!input || !input->len || input->len > KALEIDOSCOPE_HTTP_PATH_MAX || input->bytes[0] != '/')
        return false;
    size_t segment = 0;
    for (size_t i = 0; i < input->len; i++) {
        uint8_t byte = input->bytes[i];
        bool allowed = (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') ||
                       (byte >= '0' && byte <= '9') || byte == '/' || byte == '-' || byte == '.' ||
                       byte == '_' || byte == '~';
        if (!allowed)
            return false;
        if (byte == '/' && i + 1 < input->len && input->bytes[i + 1] == '/')
            return false;
        if (byte == '/' || i + 1 == input->len) {
            size_t end = byte == '/' ? i : i + 1, len = end - segment;
            if ((len == 1 && input->bytes[segment] == '.') ||
                (len == 2 && input->bytes[segment] == '.' && input->bytes[segment + 1] == '.'))
                return false;
            segment = i + 1;
        }
    }
    for (size_t i = 0; i < input->len; i++) {
        out->bytes[i] = input->bytes[i];
    }
    out->len = input->len;
    return true;
}

bool kaleidoscope_http_path_equal(const KaleidoscopeHttpPath *left,
                                  const KaleidoscopeHttpPath *right) {
    if (!left || !right || left->len != right->len)
        return false;
    for (size_t i = 0; i < left->len; i++)
        if (left->bytes[i] != right->bytes[i])
            return false;
    return true;
}

void kaleidoscope_navigation_init(KaleidoscopeNavigation *state) {
    *state = (KaleidoscopeNavigation){0};
    for (size_t i = 0; i < 3; i++)
        kaleidoscope_http_path_initial(&state->visited[i]);
}

void kaleidoscope_navigation_begin(KaleidoscopeNavigation *state) {
    kaleidoscope_navigation_init(state);
    state->count = 1;
    state->active = true;
}

void kaleidoscope_navigation_current(const KaleidoscopeNavigation *state,
                                     KaleidoscopeHttpPath *out) {
    if (!state->count)
        kaleidoscope_http_path_initial(out);
    else
        *out = state->visited[state->count - 1];
}

bool kaleidoscope_navigation_active(const KaleidoscopeNavigation *state) {
    return state->active;
}

size_t kaleidoscope_navigation_hops(const KaleidoscopeNavigation *state) {
    return state->count ? state->count - 1 : 0;
}

bool kaleidoscope_navigation_follow(KaleidoscopeNavigation *state,
                                    const KaleidoscopeHttpPath *path) {
    size_t count = state->count;
    if (!state->active || count == 3) {
        state->active = false;
        return false;
    }
    for (size_t i = 0; i < count; i++)
        if (kaleidoscope_http_path_equal(&state->visited[i], path)) {
            state->active = false;
            return false;
        }
    state->visited[count] = *path;
    state->count++;
    return true;
}

void kaleidoscope_navigation_complete(KaleidoscopeNavigation *state) {
    state->active = false;
}

void kaleidoscope_navigation_cancel(KaleidoscopeNavigation *state) {
    kaleidoscope_navigation_init(state);
}


