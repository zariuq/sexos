#include "kaleidoscope_web_page.h"
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

static bool web_prefix(const uint8_t *text, size_t len, const char *prefix, size_t count) {
    if (len < count) return false;
    for (size_t i = 0; i < count; ++i)
        if (text[i] != (uint8_t)prefix[i]) return false;
    return true;
}

static size_t web_first(const uint8_t *text, size_t len, const char *chars) {
    for (size_t i = 0; i < len; ++i)
        for (size_t j = 0; chars[j]; ++j)
            if (text[i] == (uint8_t)chars[j]) return i;
    return len;
}

bool kaleidoscope_web_parse(const uint8_t *url, size_t len, KaleidoscopeWebParts *out) {
    if (!out) return false;
    *out = (KaleidoscopeWebParts){0};
    if (!url) return false;
    size_t start;
    bool tls;
    if (web_prefix(url, len, "https://", 8)) {
        start = 8;
        tls = true;
    } else if (web_prefix(url, len, "http://", 7)) {
        start = 7;
        tls = false;
    } else {
        return false;
    }
    size_t authority_end = start + web_first(url + start, len - start, "/?#");
    size_t tail_end = authority_end + web_first(url + authority_end, len - authority_end, "#");
    const uint8_t *host = url + start;
    size_t host_len = authority_end - start;
    uint32_t port = tls ? 443u : 80u;
    for (size_t i = start; i < authority_end; ++i) {
        if (url[i] == ':') {
            host_len = i - start;
            port = 0;
            for (size_t j = i + 1; j < authority_end; ++j) {
                if (url[j] < '0' || url[j] > '9') return false;
                port = port * 10u + (uint32_t)(url[j] - '0');
                if (port > 65535u) return false;
            }
            break;
        }
    }
    if (!host_len || host_len > 253u || !port) return false;
    out->tls = tls;
    out->host = host;
    out->host_len = host_len;
    out->port = (uint16_t)port;
    out->path = url + authority_end;
    out->path_len = tail_end - authority_end;
    if (!out->path_len) {
        static const uint8_t root[] = "/";
        out->path = root;
        out->path_len = 1;
    }
    return true;
}

static bool web_put(KaleidoscopeWebUrl *out, const uint8_t *origin, size_t origin_len,
                    const uint8_t *path, size_t path_len, bool trailing) {
    if (origin_len > KALEIDOSCOPE_WEB_URL_MAX ||
        path_len > KALEIDOSCOPE_WEB_URL_MAX - origin_len ||
        (trailing && origin_len + path_len == KALEIDOSCOPE_WEB_URL_MAX))
        return false;
    for (size_t i = 0; i < origin_len; ++i) out->bytes[i] = origin[i];
    for (size_t i = 0; i < path_len; ++i) out->bytes[origin_len + i] = path[i];
    if (trailing) out->bytes[origin_len + path_len] = '/';
    out->len = origin_len + path_len + (trailing ? 1u : 0u);
    return true;
}

size_t kaleidoscope_web_resolve(const uint8_t *base, size_t base_len,
                                const uint8_t *target, size_t target_len,
                                KaleidoscopeWebUrl *out) {
    if (!out) return 0;
    *out = (KaleidoscopeWebUrl){0};
    if (!base || !target) return 0;
    target_len = web_first(target, target_len, "#");
    if (web_prefix(target, target_len, "http://", 7) ||
        web_prefix(target, target_len, "https://", 8))
        return web_put(out, target, target_len, NULL, 0, false) ? target_len : 0;
    size_t scheme_end = 0;
    for (size_t i = 0; i + 3 <= base_len; ++i) {
        if (base[i] == ':' && base[i + 1] == '/' && base[i + 2] == '/') {
            scheme_end = i + 3;
            break;
        }
    }
    if (!scheme_end) return 0;
    size_t origin_end = scheme_end + web_first(base + scheme_end, base_len - scheme_end, "/?#");
    if (target_len >= 2 && target[0] == '/' && target[1] == '/')
        return web_put(out, base, scheme_end - 2, target, target_len, false) ?
               scheme_end - 2 + target_len : 0;
    if (target_len && target[0] == '/')
        return web_put(out, base, origin_end, target, target_len, false) ? origin_end + target_len : 0;
    for (size_t i = 0; i < target_len && target[i] != '/'; ++i)
        if (target[i] == ':') return 0;
    size_t path_end = origin_end + web_first(base + origin_end, base_len - origin_end, "?#");
    size_t dir_end = origin_end;
    for (size_t i = origin_end; i < path_end; ++i)
        if (base[i] == '/') dir_end = i + 1;
    if (dir_end == origin_end) {
        if (origin_end >= KALEIDOSCOPE_WEB_URL_MAX || target_len > KALEIDOSCOPE_WEB_URL_MAX - origin_end - 1)
            return 0;
        for (size_t i = 0; i < origin_end; ++i) out->bytes[i] = base[i];
        out->bytes[origin_end] = '/';
        for (size_t i = 0; i < target_len; ++i) out->bytes[origin_end + 1 + i] = target[i];
        out->len = origin_end + 1 + target_len;
        return out->len;
    }

    uint8_t path[KALEIDOSCOPE_WEB_URL_MAX];
    size_t len = 0;
    for (size_t part = 0; part < 2; ++part) {
        const uint8_t *text = part ? target : base;
        size_t text_len = part ? target_len : dir_end;
        size_t begin = part ? 0 : origin_end;
        for (size_t i = begin; i <= text_len; ++i) {
            if (i != text_len && text[i] != '/') continue;
            size_t piece_len = i - begin;
            const uint8_t *piece = text + begin;
            begin = i + 1;
            if (piece_len == 1 && piece[0] == '.') continue;
            if (piece_len == 2 && piece[0] == '.' && piece[1] == '.') {
                while (len) {
                    --len;
                    if (path[len] == '/') break;
                }
                continue;
            }
            if (!piece_len) continue;
            if (len + 1 + piece_len > KALEIDOSCOPE_WEB_URL_MAX) return 0;
            path[len] = '/';
            for (size_t j = 0; j < piece_len; ++j) path[len + 1 + j] = piece[j];
            len += 1 + piece_len;
        }
    }
    bool trailing = target_len && target[target_len - 1] == '/';
    size_t total = origin_end + len + (trailing ? 1u : 0u);
    return web_put(out, base, origin_end, path, len, trailing) ? total : 0;
}

bool kaleidoscope_web_numeric_ipv4(const uint8_t *host, size_t len, uint8_t out[4]) {
    if (!host || !out) return false;
    size_t begin = 0;
    for (size_t slot = 0; slot < 4; ++slot) {
        size_t end = begin;
        while (end < len && host[end] != '.') ++end;
        size_t count = end - begin;
        if (!count || count > 3) return false;
        uint32_t value = 0;
        for (size_t i = begin; i < end; ++i) {
            if (host[i] < '0' || host[i] > '9') return false;
            value = value * 10u + (uint32_t)(host[i] - '0');
        }
        if (value > 255u) return false;
        out[slot] = (uint8_t)value;
        if (slot != 3 && end == len) return false;
        begin = end + 1;
    }
    return begin == len + 1;
}

int kaleidoscope_web_typed(uint8_t code, bool shift) {
    static const uint8_t plain[58] = {
        0,0,49,50,51,52,53,54,55,56,57,48,45,61,0,0,113,119,101,114,116,121,117,105,111,112,91,93,0,0,
        97,115,100,102,103,104,106,107,108,59,39,96,0,92,122,120,99,118,98,110,109,44,46,47,0,42,0,32
    };
    static const uint8_t upper[58] = {
        0,0,33,64,35,36,37,94,38,42,40,41,95,43,0,0,81,87,69,82,84,89,85,73,79,80,123,125,0,0,
        65,83,68,70,71,72,74,75,76,58,34,126,0,124,90,88,67,86,66,78,77,60,62,63,0,42,0,32
    };
    if (code >= 58) return -1;
    uint8_t value = shift ? upper[code] : plain[code];
    return value ? (int)value : -1;
}
