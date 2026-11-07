#include "kaleidoscope_script_core.h"
#include "kaleidoscope_decimal.h"
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

/* The compiler's floating-point classification and constants. */
static bool script_isnan(double x) {
    return __builtin_isnan(x);
}

static bool script_isinf(double x) {
    return __builtin_isinf(x);
}

static double script_nan(void) {
    return __builtin_nan("");
}

static double script_inf(void) {
    return __builtin_inf();
}

typedef struct ScriptSlice {
    size_t start, len;
} ScriptSlice;

typedef struct ScriptMachine {
    const KaleidoscopeScriptSource *source;
    KaleidoscopeScriptWorkspace *work;
    KaleidoscopeScriptToken token;
} ScriptMachine;

typedef struct ScriptKey {
    bool name;
    size_t start, len;
    KaleidoscopeScriptValue value;
} ScriptKey;

/* The stack floor and the scripts refused at it. */
typedef struct {
    uintptr_t floor;
    size_t hits;
} ScriptStackFloor;

static ScriptStackFloor script_floor_state;

enum {
    SCRIPT_FLOOR_SET = 0,
    SCRIPT_FLOOR_HITS = 1,
    SCRIPT_FLOOR_CHECK = 2
};

/* The one function touching the floor state: SET stores the floor, HITS reads the count, CHECK
   reports (and counts) whether `address` lies below the floor. */
static size_t script_floor_owner(uint8_t op, uintptr_t address) {
    if (op == SCRIPT_FLOOR_SET) {
        script_floor_state.floor = address;
        return 0;
    }
    if (op == SCRIPT_FLOOR_HITS)
        return script_floor_state.hits;
    if (!script_floor_state.floor || address >= script_floor_state.floor)
        return 0;
    script_floor_state.hits++;
    return 1;
}

void kaleidoscope_script_set_stack_floor(uintptr_t address) {
    script_floor_owner(SCRIPT_FLOOR_SET, address);
}

size_t kaleidoscope_script_floor_hits(void) {
    return script_floor_owner(SCRIPT_FLOOR_HITS, 0);
}

/* Whether this frame lies below the stack floor. */
static bool script_stack_low(void) {
    volatile uint8_t marker = 0;
    return script_floor_owner(SCRIPT_FLOOR_CHECK, (uintptr_t)&marker) != 0;
}

/* The nesting budget and the stack floor, as upstream's `too_deep`. */
static bool script_too_deep(const ScriptMachine *m) {
    return m->work->depth >= KALEIDOSCOPE_SCRIPT_DEPTH_LIMIT || script_stack_low();
}

static KaleidoscopeScriptValue script_num(double number) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_NUMBER, .number = number};
    return v;
}

static KaleidoscopeScriptValue script_str(size_t start, size_t len) {
    KaleidoscopeScriptValue v = {
        .kind = KALEIDOSCOPE_SCRIPT_VALUE_STRING, .start = (uint16_t)start, .len = (uint16_t)len};
    return v;
}

static KaleidoscopeScriptValue script_bool(bool boolean) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN, .boolean = boolean};
    return v;
}

static KaleidoscopeScriptValue script_undef(void) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED};
    return v;
}

static KaleidoscopeScriptValue script_null(void) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_NULL};
    return v;
}

static KaleidoscopeScriptValue script_array(uint8_t index) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_ARRAY, .index = index};
    return v;
}

static KaleidoscopeScriptValue script_object(uint8_t index) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_OBJECT, .index = index};
    return v;
}

static KaleidoscopeScriptValue script_function(uint8_t index) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION, .index = index};
    return v;
}

static KaleidoscopeScriptValue script_regex(uint8_t index) {
    KaleidoscopeScriptValue v = {.kind = KALEIDOSCOPE_SCRIPT_VALUE_REGEX, .index = index};
    return v;
}

static bool script_ident(const ScriptMachine *m, const char *word) {
    size_t n = 0;
    while (word[n])
        n++;
    if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT || m->token.len != n)
        return false;
    for (size_t i = 0; i < n; i++)
        if (m->source->bytes[m->token.start + i] != (uint8_t)word[i])
            return false;
    return true;
}

static uint8_t script_next(ScriptMachine *m) {
    uint8_t e = kaleidoscope_script_lexer_next(m->source, &m->work->lexer, &m->token);
    return e;
}

static bool script_punct(const ScriptMachine *m, uint8_t p) {
    return m->token.kind == KALEIDOSCOPE_TOKEN_PUNCT && m->token.punct == p;
}

static uint8_t script_expect(ScriptMachine *m, uint8_t p) {
    if (!script_punct(m, p))
        return KALEIDOSCOPE_SCRIPT_SYNTAX;
    return script_next(m);
}

static double script_from_bits(uint64_t bits) {
    union {
        uint64_t u;
        double d;
    } v = {bits};

    return v.d;
}

static uint64_t script_to_bits(double x) {
    union {
        double d;
        uint64_t u;
    } v = {x};

    return v.u;
}

static size_t script_uint(uint64_t value, uint8_t *out) {
    uint8_t reverse[24] = {0};
    size_t n = 0;
    do {
        reverse[n++] = (uint8_t)('0' + value % 10);
        value /= 10;
    } while (value);
    for (size_t i = 0; i < n; i++)
        out[i] = reverse[n - 1 - i];
    return n;
}

/* A short text being built. */
typedef struct {
    size_t n;
    uint8_t b[128];
} ScriptNumberText;

static void script_put(ScriptNumberText *t, uint8_t c) {
    if (t->n < sizeof t->b)
        t->b[t->n++] = c;
}

static void script_put_uint(ScriptNumberText *t, uint64_t value) {
    uint8_t reverse[24] = {0};
    size_t n = 1;
    reverse[0] = (uint8_t)('0' + value % 10);
    for (value /= 10; value && n < 20; value /= 10)
        reverse[n++] = (uint8_t)('0' + value % 10);
    while (n)
        script_put(t, reverse[--n]);
}

/* Rust's digits_to_dec_str: the digits in positional form with at least `frac` fraction
   digits. */
static void script_dec_str(ScriptNumberText *t, const KaleidoscopeDigits *digits, size_t frac) {
    int32_t k = digits->k;
    if (k <= 0) {
        size_t zeros = (size_t)-k;
        script_put(t, '0');
        script_put(t, '.');
        for (size_t i = 0; i < zeros; i++)
            script_put(t, '0');
        for (size_t i = 0; i < digits->len; i++)
            script_put(t, digits->digits[i]);
        if (frac > digits->len && frac - digits->len > zeros)
            for (size_t i = 0; i < frac - digits->len - zeros; i++)
                script_put(t, '0');
    } else if ((size_t)k < digits->len) {
        for (size_t i = 0; i < digits->len; i++) {
            if (i == (size_t)k)
                script_put(t, '.');
            script_put(t, digits->digits[i]);
        }
        for (size_t i = digits->len - (size_t)k; i < frac; i++)
            script_put(t, '0');
    } else {
        for (size_t i = 0; i < digits->len; i++)
            script_put(t, digits->digits[i]);
        for (size_t i = digits->len; i < (size_t)k; i++)
            script_put(t, '0');
        if (frac) {
            script_put(t, '.');
            for (size_t i = 0; i < frac; i++)
                script_put(t, '0');
        }
    }
}

/* Rust's `{:.N}` of a finite double (N <= 40): its sign, then the digits. */
static void script_exact_fixed(ScriptNumberText *t, double v, size_t frac) {
    if (script_to_bits(v) >> 63)
        script_put(t, '-');
    KaleidoscopeDigits digits = {0};
    if (v != 0.0)
        kaleidoscope_decimal_exact(v, -(int32_t)frac, &digits);
    if (v == 0.0 || digits.k <= -(int32_t)frac) {
        script_put(t, '0');
        if (frac) {
            script_put(t, '.');
            for (size_t i = 0; i < frac; i++)
                script_put(t, '0');
        }
        return;
    }
    script_dec_str(t, &digits, frac);
}

/* JavaScript's Number::toString as upstream's `number_text`: Rust's shortest digits, in `{:e}`
   form with a signed exponent from 1e21 up or below 1e-6, positional otherwise. */
static void script_number_text(ScriptNumberText *t, double number) {
    static const char nan_word[] = "NaN", infinity_word[] = "Infinity";
    if (script_isnan(number)) {
        for (size_t i = 0; nan_word[i]; i++)
            script_put(t, (uint8_t)nan_word[i]);
        return;
    }
    if (script_isinf(number)) {
        if (number < 0.0)
            script_put(t, '-');
        for (size_t i = 0; infinity_word[i]; i++)
            script_put(t, (uint8_t)infinity_word[i]);
        return;
    }
    if (number == 0.0) {
        script_put(t, '0');
        return;
    }
    KaleidoscopeDigits digits = {0};
    kaleidoscope_decimal_shortest(number, &digits);
    if (number < 0.0)
        script_put(t, '-');
    double a = number < 0.0 ? -number : number;
    if (!(a >= 1e21 || a < 1e-6)) {
        script_dec_str(t, &digits, 0);
        return;
    }
    script_put(t, digits.digits[0]);
    if (digits.len > 1) {
        script_put(t, '.');
        for (size_t i = 1; i < digits.len; i++)
            script_put(t, digits.digits[i]);
    }
    script_put(t, 'e');
    int32_t e = digits.k - 1;
    script_put(t, e < 0 ? '-' : '+');
    script_put_uint(t, (uint64_t)(e < 0 ? -e : e));
}

/* number_text into a caller's buffer of at least 64 bytes. */
static size_t script_format_number(double number, uint8_t *out) {
    ScriptNumberText t = {0};
    script_number_text(&t, number);
    for (size_t i = 0; i < t.n; i++)
        out[i] = t.b[i];
    return t.n;
}

static bool script_parse_text_number(const uint8_t *text, size_t length, double *out) {
    size_t at = 0;
    while (at < length && (text[at] == ' ' || text[at] == '\t' || text[at] == '\n' ||
                           text[at] == '\r' || text[at] == 0x0b || text[at] == 0x0c))
        at++;
    while (length > at &&
           (text[length - 1] == ' ' || text[length - 1] == '\t' || text[length - 1] == '\n' ||
            text[length - 1] == '\r' || text[length - 1] == 0x0b || text[length - 1] == 0x0c))
        length--;
    if (at == length) {
        *out = 0.0;
        return true;
    }
    bool negative = false;
    if (text[at] == '+' || text[at] == '-') {
        negative = text[at] == '-';
        at++;
    }
    if (length - at == 3 && text[at] == 'N' && text[at + 1] == 'a' && text[at + 2] == 'N') {
        *out = script_nan();
        return true;
    }
    if (length - at == 8 && text[at] == 'I' && text[at + 1] == 'n' && text[at + 2] == 'f' &&
        text[at + 3] == 'i' && text[at + 4] == 'n' && text[at + 5] == 'i' && text[at + 6] == 't' &&
        text[at + 7] == 'y') {
        *out = negative ? -script_inf() : script_inf();
        return true;
    }
    KaleidoscopeDecimal decimal = {0};
    decimal.negative = negative;
    bool digit = false;
    while (at < length && text[at] >= '0' && text[at] <= '9') {
        uint8_t d = (uint8_t)(text[at++] - '0');
        digit = true;
        kaleidoscope_decimal_digit(&decimal, d, false);
    }
    if (at < length && text[at] == '.') {
        at++;
        while (at < length && text[at] >= '0' && text[at] <= '9') {
            uint8_t d = (uint8_t)(text[at++] - '0');
            digit = true;
            kaleidoscope_decimal_digit(&decimal, d, true);
        }
    }
    if (!digit)
        return false;
    int64_t exponent = 0;
    if (at < length && (text[at] == 'e' || text[at] == 'E')) {
        at++;
        bool eneg = false;
        if (at < length && (text[at] == '+' || text[at] == '-')) {
            eneg = text[at] == '-';
            at++;
        }
        if (at == length || text[at] < '0' || text[at] > '9')
            return false;
        while (at < length && text[at] >= '0' && text[at] <= '9') {
            if (exponent < INT64_C(1000000000000))
                exponent = exponent * 10 + (text[at] - '0');
            at++;
        }
        if (eneg)
            exponent = -exponent;
    }
    if (at != length)
        return false;
    *out = kaleidoscope_decimal_value(&decimal, exponent);
    return true;
}

static uint8_t script_store(ScriptMachine *m, const uint8_t *bytes, size_t length,
                            KaleidoscopeScriptValue *out) {
    size_t start = m->work->arena_len;
    if (start + length > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t i = 0; i < length; i++)
        m->work->arena[start + i] = bytes[i];
    m->work->arena_len += length;
    *out = script_str(start, length);
    return KALEIDOSCOPE_SCRIPT_OK;
}

static int script_hex_digit(uint8_t byte) {
    if (byte >= '0' && byte <= '9')
        return byte - '0';
    if (byte >= 'a' && byte <= 'f')
        return byte - 'a' + 10;
    if (byte >= 'A' && byte <= 'F')
        return byte - 'A' + 10;
    return -1;
}

static uint8_t script_append_codepoint(ScriptMachine *m, uint32_t code) {
    if (code > 0x10ffffu || (code >= 0xd800u && code <= 0xdfffu))
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    uint8_t bytes[4] = {0};
    size_t n = 0;
    if (code < 0x80u) {
        bytes[0] = (uint8_t)code;
        n = 1;
    } else if (code < 0x800u) {
        bytes[0] = (uint8_t)(0xc0u | (code >> 6));
        bytes[1] = (uint8_t)(0x80u | (code & 63u));
        n = 2;
    } else if (code < 0x10000u) {
        bytes[0] = (uint8_t)(0xe0u | (code >> 12));
        bytes[1] = (uint8_t)(0x80u | ((code >> 6) & 63u));
        bytes[2] = (uint8_t)(0x80u | (code & 63u));
        n = 3;
    } else {
        bytes[0] = (uint8_t)(0xf0u | (code >> 18));
        bytes[1] = (uint8_t)(0x80u | ((code >> 12) & 63u));
        bytes[2] = (uint8_t)(0x80u | ((code >> 6) & 63u));
        bytes[3] = (uint8_t)(0x80u | (code & 63u));
        n = 4;
    }
    if (m->work->arena_len + n > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t i = 0; i < n; i++)
        m->work->arena[m->work->arena_len++] = bytes[i];
    return 0;
}

static uint8_t script_literal(ScriptMachine *m, size_t start, size_t length,
                              KaleidoscopeScriptValue *out) {
    size_t first = m->work->arena_len, i = 0;
    while (i < length) {
        uint8_t byte = m->source->bytes[start + i++];
        if (byte == '\\') {
            if (i >= length)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            uint8_t escaped = m->source->bytes[start + i++];
            if (escaped == 'x' || escaped == 'u') {
                size_t digits = escaped == 'x' ? 2 : 4;
                if (escaped == 'u' && i < length && m->source->bytes[start + i] == '{') {
                    i++;
                    size_t begin = i;
                    while (i < length && m->source->bytes[start + i] != '}')
                        i++;
                    if (i == length)
                        return KALEIDOSCOPE_SCRIPT_SYNTAX;
                    digits = i - begin;
                    i++;
                    i = begin;
                    uint32_t code = 0;
                    if (!digits)
                        return KALEIDOSCOPE_SCRIPT_SYNTAX;
                    for (size_t k = 0; k < digits; k++) {
                        int d = script_hex_digit(m->source->bytes[start + i + k]);
                        if (d < 0 || code > (UINT32_MAX - (uint32_t)d) / 16u)
                            return KALEIDOSCOPE_SCRIPT_SYNTAX;
                        code = code * 16u + (uint32_t)d;
                    }
                    i += digits + 1;
                    uint8_t e = script_append_codepoint(m, code);
                    if (e)
                        return e;
                    continue;
                }
                if (i + digits > length)
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
                uint32_t code = 0;
                for (size_t k = 0; k < digits; k++) {
                    int d = script_hex_digit(m->source->bytes[start + i + k]);
                    if (d < 0)
                        return KALEIDOSCOPE_SCRIPT_SYNTAX;
                    code = code * 16u + (uint32_t)d;
                }
                i += digits;
                uint8_t e = script_append_codepoint(m, code);
                if (e)
                    return e;
                continue;
            }
            byte = escaped == 'n'   ? '\n'
                   : escaped == 't' ? '\t'
                   : escaped == 'r' ? '\r'
                   : escaped == '0' ? 0
                                    : escaped;
        }
        if (m->work->arena_len >= KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        m->work->arena[m->work->arena_len++] = byte;
    }
    *out = script_str(first, m->work->arena_len - first);
    return KALEIDOSCOPE_SCRIPT_OK;
}

static ScriptSlice script_text(KaleidoscopeScriptValue value) {
    ScriptSlice s = {0, 0};
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
        s.start = value.start;
        s.len = value.len;
    }
    return s;
}

static uint8_t script_join(ScriptMachine *m, uint8_t id, const uint8_t *separator,
                           size_t separator_len, KaleidoscopeScriptValue *out);

static uint8_t script_to_text(ScriptMachine *m, KaleidoscopeScriptValue value,
                              KaleidoscopeScriptValue *out) {
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
        *out = value;
        return KALEIDOSCOPE_SCRIPT_OK;
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN) {
        static const uint8_t yes[] = "true", no[] = "false";
        return script_store(m, value.boolean ? yes : no, value.boolean ? 4 : 5, out);
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED) {
        static const uint8_t word[] = "undefined";
        return script_store(m, word, 9, out);
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL) {
        static const uint8_t word[] = "null";
        return script_store(m, word, 4, out);
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
        static const uint8_t comma = ',';
        return script_join(m, value.index, &comma, 1, out);
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
        static const uint8_t word[] = "[object Object]";
        return script_store(m, word, 15, out);
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION) {
        static const uint8_t word[] = "function";
        return script_store(m, word, 8, out);
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_REGEX) {
        KaleidoscopeScriptRegex regex = m->work->regexes[value.index];
        uint8_t text[266] = {0};
        size_t n = 0;
        text[n++] = '/';
        for (size_t i = 0; i < regex.pattern_len; i++)
            text[n++] = m->source->bytes[regex.pattern_start + i];
        text[n++] = '/';
        for (size_t i = 0; i < regex.flags_len; i++)
            text[n++] = m->source->bytes[regex.flags_start + i];
        return script_store(m, text, n, out);
    }
    uint8_t text[64] = {0};
    size_t n = script_format_number(value.number, text);
    return script_store(m, text, n, out);
}

static bool script_truthy(KaleidoscopeScriptValue value) {
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER)
        return value.number != 0.0 && !script_isnan(value.number);
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN)
        return value.boolean;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL)
        return false;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_REGEX)
        return true;
    return script_text(value).len != 0;
}

static double script_to_number(const ScriptMachine *m, KaleidoscopeScriptValue value) {
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER)
        return value.number;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN)
        return value.boolean ? 1.0 : 0.0;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL)
        return 0.0;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION ||
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_REGEX)
        return script_nan();
    ScriptSlice s = script_text(value);
    size_t first = 0, last = s.len;
    while (first < last) {
        uint8_t c = m->work->arena[s.start + first];
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c))
            break;
        first++;
    }
    while (last > first) {
        uint8_t c = m->work->arena[s.start + last - 1];
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c))
            break;
        last--;
    }
    if (last - first >= 2 && m->work->arena[s.start + first] == '0') {
        uint8_t mark = m->work->arena[s.start + first + 1];
        unsigned radix = mark == 'x' || mark == 'X'   ? 16
                         : mark == 'o' || mark == 'O' ? 8
                         : mark == 'b' || mark == 'B' ? 2
                                                      : 0;
        if (radix) {
            if (last - first == 2)
                return script_nan();
            double number = 0.0;
            for (size_t i = first + 2; i < last; i++) {
                int digit = script_hex_digit(m->work->arena[s.start + i]);
                if (digit < 0 || (unsigned)digit >= radix)
                    return script_nan();
                number = number * (double)radix + (double)digit;
            }
            return number;
        }
    }
    double number = 0.0;
    if (!script_parse_text_number(m->work->arena + s.start, s.len, &number))
        return script_nan();
    return number;
}

static uint8_t script_concat(ScriptMachine *m, KaleidoscopeScriptValue left,
                             KaleidoscopeScriptValue right, KaleidoscopeScriptValue *out) {
    uint8_t e = script_to_text(m, left, &left);
    if (e)
        return e;
    e = script_to_text(m, right, &right);
    if (e)
        return e;
    ScriptSlice a = script_text(left), b = script_text(right);
    size_t first = m->work->arena_len;
    if (first + a.len + b.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t i = 0; i < a.len; i++)
        m->work->arena[m->work->arena_len++] = m->work->arena[a.start + i];
    for (size_t i = 0; i < b.len; i++)
        m->work->arena[m->work->arena_len++] = m->work->arena[b.start + i];
    *out = script_str(first, a.len + b.len);
    return KALEIDOSCOPE_SCRIPT_OK;
}

static double script_trunc(double x) {
    if (x >= 4503599627370496.0)
        return x;
    return (double)(uint64_t)x;
}

static double script_rem(double a, double b) {
    if (b == 0.0 || script_isinf(a) || script_isnan(a) || script_isnan(b))
        return script_nan();
    if (script_isinf(b))
        return a;
    double q = a / b, t = q >= 0.0 ? script_trunc(q) : -script_trunc(-q);
    return a - t * b;
}

static double script_floor(double x) {
    if (script_isnan(x) || script_isinf(x) || x >= 4503599627370496.0 || x <= -4503599627370496.0)
        return x;
    double t = x >= 0.0 ? script_trunc(x) : -script_trunc(-x);
    return t > x ? t - 1.0 : t;
}

__extension__ typedef unsigned __int128 ScriptU128;

/* Rust's `as i64`: saturating, NaN to 0. */
static int64_t script_i64(double x) {
    if (script_isnan(x))
        return 0;
    if (x >= 9223372036854775808.0)
        return INT64_MAX;
    if (x <= -9223372036854775808.0)
        return INT64_MIN;
    return (int64_t)x;
}

static double script_copysign(double magnitude, double sign) {
    return script_from_bits((script_to_bits(magnitude) & ~(UINT64_C(1) << 63)) |
                            (script_to_bits(sign) & (UINT64_C(1) << 63)));
}

static double script_fabs(double x) {
    return script_copysign(x, 1.0);
}

static ScriptU128 script_udiv128(ScriptU128 numerator, ScriptU128 denominator) {
    ScriptU128 quotient = 0, remainder = 0;
    for (unsigned bit = 128; bit > 0; --bit) {
        bool high = (remainder >> 127) != 0;
        remainder = (remainder << 1) | ((numerator >> (bit - 1)) & 1);
        if (high || remainder >= denominator) {
            remainder -= denominator;
            quotient |= (ScriptU128)1 << (bit - 1);
        }
    }
    return quotient;
}

/* Upstream's correctly rounded square root: the integer root of the scaled significand, rounded
   once with a sticky bit, then scaled by powers of two. */
static double script_sqrt(double x) {
    if (script_isnan(x) || x < 0.0)
        return script_nan();
    if (x == 0.0 || script_isinf(x))
        return x;
    uint64_t bits = script_to_bits(x);
    ScriptU128 significand = bits & ((UINT64_C(1) << 52) - 1);
    int32_t exponent = (int32_t)((bits >> 52) & 0x7ff);
    if (exponent == 0)
        exponent = 1;
    else
        significand |= (ScriptU128)1 << 52;
    exponent -= 1075;
    if (exponent & 1) {
        significand <<= 1;
        exponent -= 1;
    }
    ScriptU128 big = significand << 74, root = UINT64_MAX;
    for (;;) {
        ScriptU128 next = (root + script_udiv128(big, root)) >> 1;
        if (next >= root)
            break;
        root = next;
    }
    ScriptU128 sticky = root * root != big;
    double out = (double)(uint64_t)(root | sticky);
    int32_t scale = exponent / 2 - 37;
    while (scale > 0) {
        int32_t k = scale < 1000 ? scale : 1000;
        out *= script_from_bits((uint64_t)(k + 1023) << 52);
        scale -= k;
    }
    while (scale < 0) {
        int32_t k = -scale < 1000 ? -scale : 1000;
        out /= script_from_bits((uint64_t)(k + 1023) << 52);
        scale += k;
    }
    return out;
}

static double script_pow(double a, double exponent) {
    if (script_isnan(exponent) || exponent != script_floor(exponent) || exponent > 1024.0 ||
        exponent < -1024.0)
        return script_nan();
    bool negative = exponent < 0.0;
    uint32_t n = (uint32_t)(negative ? -exponent : exponent);
    double value = 1.0, base = a;
    while (n) {
        if (n & 1u)
            value *= base;
        base *= base;
        n >>= 1;
    }
    return negative ? 1.0 / value : value;
}

static int32_t script_int32(double value) {
    if (script_isnan(value) || script_isinf(value))
        return 0;
    double whole = value < 0.0 ? -script_floor(-value) : script_floor(value),
           mod = whole - script_floor(whole / 4294967296.0) * 4294967296.0;
    return (int32_t)(uint32_t)mod;
}

static double script_i32_number(uint32_t bits) {
    return bits <= UINT32_C(0x7fffffff) ? (double)bits : (double)bits - 4294967296.0;
}

static int64_t script_floor_div(int64_t value, int64_t divisor) {
    int64_t quotient = value / divisor, remainder = value % divisor;
    if (remainder < 0)
        quotient--;
    return quotient;
}

static int64_t script_days_from_civil(int64_t year, int64_t month, int64_t day) {
    if (month <= 2)
        year--;
    int64_t era = script_floor_div(year, 400), yoe = year - era * 400,
            doy = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1,
            doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

static void script_civil(int64_t days, int64_t *year, int64_t *month, int64_t *day) {
    int64_t z = days + 719468, era = script_floor_div(z, 146097), doe = z - era * 146097,
            yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365,
            doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
    *day = doy - (153 * mp + 2) / 5 + 1;
    *month = mp < 10 ? mp + 3 : mp - 9;
    *year = yoe + era * 400 + (*month <= 2);
}

static double script_date_ms(const double values[7]) {
    for (size_t i = 0; i < 7; i++)
        if (script_isnan(values[i]) || script_isinf(values[i]))
            return script_nan();
    int64_t year = (int64_t)script_floor(values[0]), month = (int64_t)script_floor(values[1]),
            q = script_floor_div(month, 12);
    year += q;
    month -= q * 12;
    double days =
        (double)script_days_from_civil(year, month + 1, 1) + script_floor(values[2]) - 1.0;
    return days * 86400000.0 + script_floor(values[3]) * 3600000.0 +
           script_floor(values[4]) * 60000.0 + script_floor(values[5]) * 1000.0 +
           script_floor(values[6]);
}

static bool script_parse_digits(const uint8_t *text, size_t length, size_t at, size_t count,
                                double *out) {
    if (at + count > length)
        return false;
    double value = 0.0;
    for (size_t i = 0; i < count; i++) {
        uint8_t c = text[at + i];
        if (c < '0' || c > '9')
            return false;
        value = value * 10.0 + (c - '0');
    }
    *out = value;
    return true;
}

static bool script_parse_iso(const uint8_t *text, size_t length, double *out) {
    if (length < 10 || text[4] != '-' || text[7] != '-')
        return false;
    double values[7] = {0, 0, 0, 0, 0, 0, 0};
    if (!script_parse_digits(text, length, 0, 4, &values[0]) ||
        !script_parse_digits(text, length, 5, 2, &values[1]) ||
        !script_parse_digits(text, length, 8, 2, &values[2]))
        return false;
    values[1] -= 1.0;
    size_t at = 10;
    if (at < length && text[at] == 'T') {
        if (at + 6 > length || text[at + 3] != ':' ||
            !script_parse_digits(text, length, at + 1, 2, &values[3]) ||
            !script_parse_digits(text, length, at + 4, 2, &values[4]))
            return false;
        at += 6;
        if (at < length && text[at] == ':') {
            if (!script_parse_digits(text, length, at + 1, 2, &values[5]))
                return false;
            at += 3;
        }
        if (at < length && text[at] == '.') {
            if (!script_parse_digits(text, length, at + 1, 3, &values[6]))
                return false;
            at += 4;
        }
        if (at < length && text[at] == 'Z')
            at++;
    }
    if (at != length || values[1] > 11.0 || values[2] < 1.0 || values[2] > 31.0)
        return false;
    *out = script_date_ms(values);
    return true;
}

static KaleidoscopeScriptVariable *script_find(ScriptMachine *m, size_t start, size_t length) {
    for (size_t k = m->work->nvars; k > 0; k--) {
        KaleidoscopeScriptVariable *v = &m->work->vars[k - 1];
        if (v->len != length)
            continue;
        bool same = true;
        for (size_t i = 0; i < length; i++)
            if (v->name[i] != m->source->bytes[start + i])
                same = false;
        if (same)
            return v;
    }
    return NULL;
}

static uint8_t script_declare(ScriptMachine *m, size_t start, size_t length,
                              KaleidoscopeScriptValue value, uint8_t kind) {
    if (m->work->nvars >= KALEIDOSCOPE_SCRIPT_VARS_LIMIT || length > KALEIDOSCOPE_SCRIPT_NAME)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    KaleidoscopeScriptVariable *v = &m->work->vars[m->work->nvars++];
    v->len = (uint8_t)length;
    v->kind = kind;
    for (size_t i = 0; i < length; i++)
        v->name[i] = m->source->bytes[start + i];
    v->value = value;
    return 0;
}

static uint8_t script_assign(ScriptMachine *m, size_t start, size_t length,
                             KaleidoscopeScriptValue value, bool declare) {
    KaleidoscopeScriptVariable *v = script_find(m, start, length);
    if (v) {
        if (v->kind == 2 && !declare)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        v->value = value;
        return KALEIDOSCOPE_SCRIPT_OK;
    }
    if (!declare)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    return script_declare(m, start, length, value, 0);
}

static uint8_t script_bind_var(ScriptMachine *m, size_t start, size_t length, bool present,
                               KaleidoscopeScriptValue value) {
    KaleidoscopeScriptVariable *slot = script_find(m, start, length);
    if (slot && (size_t)(slot - m->work->vars) >= m->work->frame_base) {
        if (slot->kind == 2)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        if (present)
            slot->value = value;
        return 0;
    }
    return script_declare(m, start, length, present ? value : script_undef(), 0);
}

static void script_end_block(ScriptMachine *m, size_t since) {
    size_t keep = since;
    for (size_t k = since; k < m->work->nvars; k++)
        if (m->work->vars[k].kind == 0)
            m->work->vars[keep++] = m->work->vars[k];
    m->work->nvars = keep;
}

static uint8_t script_expr(ScriptMachine *m, KaleidoscopeScriptValue *out);
static uint8_t script_items(ScriptMachine *m, uint8_t close, KaleidoscopeScriptValue *out,
                            size_t capacity, size_t *count);
static uint8_t script_region_append(ScriptMachine *m, size_t first, size_t *count,
                                    const uint8_t *bytes, size_t length);
static KaleidoscopeScriptPosition script_position_save(const ScriptMachine *m);
static void script_position_restore(ScriptMachine *m, KaleidoscopeScriptPosition position);
static bool script_function_name(const ScriptMachine *m, const KaleidoscopeScriptFunction *function,
                                 size_t start, size_t len);
static bool script_utf8_valid(const uint8_t *text, size_t length);
static bool script_class_name(const ScriptMachine *m, const KaleidoscopeScriptClass *class_record,
                              const uint8_t *name, size_t name_len);
static uint8_t script_class_static_call(ScriptMachine *m, size_t class_index,
                                        KaleidoscopeScriptValue *out);
static uint8_t script_step_name(ScriptMachine *m, size_t start, size_t len, bool up, bool prefix,
                                KaleidoscopeScriptValue *out);

static int script_text_compare(const ScriptMachine *m, KaleidoscopeScriptValue left,
                               KaleidoscopeScriptValue right) {
    ScriptSlice a = script_text(left), b = script_text(right);
    size_t n = a.len < b.len ? a.len : b.len;
    for (size_t i = 0; i < n; i++) {
        uint8_t x = m->work->arena[a.start + i], y = m->work->arena[b.start + i];
        if (x < y)
            return -1;
        if (x > y)
            return 1;
    }
    return a.len < b.len ? -1 : a.len > b.len ? 1 : 0;
}

static bool script_text_equal(const ScriptMachine *m, KaleidoscopeScriptValue left,
                              KaleidoscopeScriptValue right) {
    return script_text_compare(m, left, right) == 0;
}

static size_t script_find_prop(const ScriptMachine *m, uint8_t id, const uint8_t *key,
                               size_t key_len) {
    KaleidoscopeScriptArray object = m->work->objects[id];
    for (size_t i = 0; i < object.len; i++) {
        size_t slot = object.start + i;
        KaleidoscopeScriptProperty property = m->work->props[slot];
        if (property.len != key_len)
            continue;
        bool same = true;
        for (size_t k = 0; k < key_len; k++)
            if (m->work->arena[property.start + k] != key[k])
                same = false;
        if (same)
            return slot;
    }
    return SIZE_MAX;
}

static KaleidoscopeScriptValue script_get_prop(const ScriptMachine *m, uint8_t id,
                                               const uint8_t *key, size_t key_len) {
    size_t slot = script_find_prop(m, id, key, key_len);
    return slot == SIZE_MAX ? script_undef() : m->work->props[slot].value;
}

static uint8_t script_set_prop(ScriptMachine *m, uint8_t id, const uint8_t *key, size_t key_len,
                               KaleidoscopeScriptValue value) {
    size_t slot = script_find_prop(m, id, key, key_len);
    if (slot != SIZE_MAX) {
        m->work->props[slot].value = value;
        return 0;
    }
    KaleidoscopeScriptValue name = {0};
    uint8_t e = script_store(m, key, key_len, &name);
    if (e)
        return e;
    KaleidoscopeScriptArray *object = &m->work->objects[id];
    if (object->len == object->capacity) {
        size_t capacity = object->capacity ? (size_t)object->capacity * 2 : 4,
               start = m->work->prop_count;
        if (start + capacity > KALEIDOSCOPE_SCRIPT_PROPS_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t i = 0; i < object->len; i++)
            m->work->props[start + i] = m->work->props[object->start + i];
        for (size_t i = object->len; i < capacity; i++)
            m->work->props[start + i] = (KaleidoscopeScriptProperty){0};
        m->work->prop_count += capacity;
        object->start = (uint16_t)start;
        object->capacity = (uint16_t)capacity;
    }
    slot = object->start + object->len;
    object->len++;
    m->work->props[slot] =
        (KaleidoscopeScriptProperty){.start = name.start, .len = name.len, .value = value};
    return 0;
}

static uint8_t script_new_object(ScriptMachine *m, uint8_t *id) {
    if (m->work->nobjects >= KALEIDOSCOPE_SCRIPT_OBJECTS ||
        m->work->prop_count + 4 > KALEIDOSCOPE_SCRIPT_PROPS_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    size_t start = m->work->prop_count, index = m->work->nobjects;
    for (size_t i = 0; i < 4; i++)
        m->work->props[start + i] = (KaleidoscopeScriptProperty){0};
    m->work->prop_count += 4;
    KaleidoscopeScriptArray *object = &m->work->objects[index];
    object->start = (uint16_t)start;
    object->len = 0;
    object->capacity = 4;
    m->work->made_by[index] = 0;
    *id = (uint8_t)index;
    m->work->nobjects++;
    return 0;
}

static uint8_t script_json_byte(ScriptMachine *m, size_t first, size_t *count, uint8_t byte) {
    if (first + *count >= KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->arena[first + (*count)++] = byte;
    m->work->arena_len = first + *count;
    return 0;
}

static uint8_t script_json_bytes(ScriptMachine *m, size_t first, size_t *count,
                                 const uint8_t *bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t e = script_json_byte(m, first, count, bytes[i]);
        if (e)
            return e;
    }
    return 0;
}

static uint8_t script_json_quote(ScriptMachine *m, size_t first, size_t *count,
                                 KaleidoscopeScriptValue value) {
    uint8_t e = script_json_byte(m, first, count, '"');
    if (e)
        return e;
    ScriptSlice text = script_text(value);
    for (size_t i = 0; i < text.len; i++) {
        uint8_t byte = m->work->arena[text.start + i];
        if (byte == '"') {
            static const uint8_t escaped[] = {'\\', '"'};
            e = script_json_bytes(m, first, count, escaped, 2);
        } else if (byte == '\\') {
            static const uint8_t escaped[] = {'\\', '\\'};
            e = script_json_bytes(m, first, count, escaped, 2);
        } else if (byte == '\n') {
            static const uint8_t escaped[] = {'\\', 'n'};
            e = script_json_bytes(m, first, count, escaped, 2);
        } else if (byte == '\t') {
            static const uint8_t escaped[] = {'\\', 't'};
            e = script_json_bytes(m, first, count, escaped, 2);
        } else
            e = script_json_byte(m, first, count, byte <= 31 ? '?' : byte);
        if (e)
            return e;
    }
    return script_json_byte(m, first, count, '"');
}

static bool script_hidden_prop(const ScriptMachine *m, uint8_t id, size_t k);

/* `JSON.stringify` options: the indent unit (empty: none) and the replacer's key list (texts
   on the argument stack). */
typedef struct {
    uint8_t indent[10];
    size_t indent_len;
    bool keys_present;
    size_t keys_start, keys_count;
} ScriptJsonLayout;

/* A newline and `level` indents, when indenting. */
static uint8_t script_json_break(ScriptMachine *m, size_t first, size_t *count,
                                 const ScriptJsonLayout *layout, size_t level) {
    if (!layout->indent_len)
        return 0;
    uint8_t e = script_json_byte(m, first, count, '\n');
    for (size_t i = 0; i < level && !e; i++)
        e = script_json_bytes(m, first, count, layout->indent, layout->indent_len);
    return e;
}

static bool script_json_kept(const ScriptMachine *m, const ScriptJsonLayout *layout,
                             KaleidoscopeScriptProperty property) {
    if (!layout->keys_present)
        return true;
    for (size_t k = 0; k < layout->keys_count; k++) {
        KaleidoscopeScriptValue key = m->work->argument_stack[layout->keys_start + k];
        if (key.len != property.len)
            continue;
        bool same = true;
        for (size_t i = 0; same && i < key.len; i++)
            same = m->work->arena[key.start + i] == m->work->arena[property.start + i];
        if (same)
            return true;
    }
    return false;
}

static uint8_t script_json_append(ScriptMachine *m, KaleidoscopeScriptValue value, size_t first,
                                  size_t *count, const ScriptJsonLayout *layout, size_t level) {
    if (script_too_deep(m)) {
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    }
    m->work->depth++;
    uint8_t error = 0;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER &&
        (script_isnan(value.number) || script_isinf(value.number))) {
        static const uint8_t text[] = "null";
        error = script_json_bytes(m, first, count, text, 4);
    } else if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING)
        error = script_json_quote(m, first, count, value);
    else if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
        error = script_json_byte(m, first, count, '[');
        KaleidoscopeScriptArray array = m->work->arrays[value.index];
        for (size_t i = 0; i < array.len && !error; i++) {
            if (i)
                error = script_json_byte(m, first, count, ',');
            if (!error)
                error = script_json_break(m, first, count, layout, level + 1);
            KaleidoscopeScriptValue element = m->work->elems[array.start + i];
            if (element.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
                element.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION) {
                static const uint8_t text[] = "null";
                error = script_json_bytes(m, first, count, text, 4);
            } else if (!error)
                error = script_json_append(m, element, first, count, layout, level + 1);
        }
        if (!error && array.len)
            error = script_json_break(m, first, count, layout, level);
        if (!error)
            error = script_json_byte(m, first, count, ']');
    } else if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
        error = script_json_byte(m, first, count, '{');
        KaleidoscopeScriptArray object = m->work->objects[value.index];
        bool emitted = false;
        for (size_t i = 0; i < object.len && !error; i++) {
            KaleidoscopeScriptProperty property = m->work->props[object.start + i];
            /* Like JavaScript: undefined and function members are left out, and a replacer
               list keeps only the keys it names. */
            if (property.value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
                property.value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION ||
                script_hidden_prop(m, value.index, object.start + i) ||
                !script_json_kept(m, layout, property))
                continue;
            if (emitted)
                error = script_json_byte(m, first, count, ',');
            if (!error)
                error = script_json_break(m, first, count, layout, level + 1);
            if (!error)
                error =
                    script_json_quote(m, first, count, script_str(property.start, property.len));
            if (!error)
                error = script_json_byte(m, first, count, ':');
            if (!error && layout->indent_len)
                error = script_json_byte(m, first, count, ' ');
            if (!error)
                error = script_json_append(m, property.value, first, count, layout, level + 1);
            emitted = true;
        }
        if (!error && emitted)
            error = script_json_break(m, first, count, layout, level);
        if (!error)
            error = script_json_byte(m, first, count, '}');
    } else if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
               value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION) {
        static const uint8_t text[] = "null";
        error = script_json_bytes(m, first, count, text, 4);
    } else {
        KaleidoscopeScriptValue text = {0};
        error = script_to_text(m, value, &text);
        if (!error) {
            ScriptSlice slice = script_text(text);
            if (first + *count + slice.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
                error = KALEIDOSCOPE_SCRIPT_LIMIT;
            else {
                for (size_t i = 0; i < slice.len; i++)
                    m->work->arena[first + *count + i] = m->work->arena[slice.start + i];
                *count += slice.len;
                m->work->arena_len = first + *count;
            }
        }
    }
    m->work->depth--;
    return error;
}

static uint8_t script_json_layout(ScriptMachine *m, KaleidoscopeScriptValue value,
                                  const ScriptJsonLayout *layout, KaleidoscopeScriptValue *out) {
    size_t first = m->work->arena_len, count = 0;
    uint8_t e = script_json_append(m, value, first, &count, layout, 0);
    if (e)
        return e;
    *out = script_str(first, count);
    return 0;
}

static uint8_t script_json(ScriptMachine *m, KaleidoscopeScriptValue value,
                           KaleidoscopeScriptValue *out) {
    ScriptJsonLayout plain = {0};
    size_t first = m->work->arena_len, count = 0;
    uint8_t e = script_json_append(m, value, first, &count, &plain, 0);
    if (e)
        return e;
    *out = script_str(first, count);
    return 0;
}

static uint8_t script_allocate_array(ScriptMachine *m, size_t count, uint8_t *id) {
    size_t capacity = count < 4 ? 4 : count;
    if (m->work->narrays >= KALEIDOSCOPE_SCRIPT_ARRAYS ||
        m->work->elem_count + capacity > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    size_t start = m->work->elem_count;
    for (size_t i = 0; i < capacity; i++)
        m->work->elems[start + i] = script_undef();
    m->work->elem_count += capacity;
    KaleidoscopeScriptArray *array = &m->work->arrays[m->work->narrays];
    array->start = (uint16_t)start;
    array->len = (uint16_t)count;
    array->capacity = (uint16_t)capacity;
    *id = (uint8_t)m->work->narrays++;
    return 0;
}

static uint8_t script_new_array(ScriptMachine *m, const KaleidoscopeScriptValue *items,
                                size_t count, KaleidoscopeScriptValue *out) {
    uint8_t id = 0;
    uint8_t e = script_allocate_array(m, count, &id);
    if (e)
        return e;
    KaleidoscopeScriptArray array = m->work->arrays[id];
    for (size_t i = 0; i < count; i++)
        m->work->elems[array.start + i] = items[i];
    *out = script_array(id);
    return 0;
}

static uint8_t script_push(ScriptMachine *m, uint8_t id, KaleidoscopeScriptValue value) {
    KaleidoscopeScriptArray *array = &m->work->arrays[id];
    if (array->len == array->capacity) {
        size_t capacity = array->capacity ? (size_t)array->capacity * 2 : 4,
               start = m->work->elem_count;
        if (start + capacity > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t i = 0; i < array->len; i++)
            m->work->elems[start + i] = m->work->elems[array->start + i];
        for (size_t i = array->len; i < capacity; i++)
            m->work->elems[start + i] = script_undef();
        m->work->elem_count += capacity;
        array->start = (uint16_t)start;
        array->capacity = (uint16_t)capacity;
    }
    m->work->elems[array->start + array->len] = value;
    array->len++;
    return 0;
}

static uint8_t script_set_items(ScriptMachine *m, uint8_t id, const KaleidoscopeScriptValue *items,
                                size_t count) {
    KaleidoscopeScriptArray *array = &m->work->arrays[id];
    if (count > array->capacity) {
        size_t capacity = count > (size_t)array->capacity * 2 ? count : (size_t)array->capacity * 2,
               start = m->work->elem_count;
        if (start + capacity > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t i = 0; i < capacity; i++)
            m->work->elems[start + i] = script_undef();
        m->work->elem_count += capacity;
        array->start = (uint16_t)start;
        array->capacity = (uint16_t)capacity;
    }
    for (size_t i = 0; i < count; i++)
        m->work->elems[array->start + i] = items[i];
    array->len = (uint16_t)count;
    return 0;
}

static uint8_t script_join(ScriptMachine *m, uint8_t id, const uint8_t *separator,
                           size_t separator_len, KaleidoscopeScriptValue *out) {
    if (script_too_deep(m)) {
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    }
    m->work->depth++;
    KaleidoscopeScriptArray array = m->work->arrays[id];
    size_t first = m->work->arena_len, joined = 0;
    uint8_t error = 0;
    for (size_t k = 0; k < array.len && !error; k++) {
        if (k) {
            if (first + joined + separator_len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT) {
                error = KALEIDOSCOPE_SCRIPT_LIMIT;
                break;
            }
            for (size_t i = 0; i < separator_len; i++)
                m->work->arena[first + joined + i] = separator[i];
            joined += separator_len;
            m->work->arena_len = first + joined;
        }
        KaleidoscopeScriptValue element = m->work->elems[array.start + k];
        if (element.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED)
            continue;
        KaleidoscopeScriptValue text = {0};
        error = script_to_text(m, element, &text);
        if (error)
            break;
        ScriptSlice slice = script_text(text);
        if (first + joined + slice.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT) {
            error = KALEIDOSCOPE_SCRIPT_LIMIT;
            break;
        }
        for (size_t i = 0; i < slice.len; i++)
            m->work->arena[first + joined + i] = m->work->arena[slice.start + i];
        joined += slice.len;
        m->work->arena_len = first + joined;
    }
    m->work->depth--;
    if (error)
        return error;
    m->work->arena_len = first + joined;
    *out = script_str(first, joined);
    return 0;
}

static uint8_t script_index(ScriptMachine *m, KaleidoscopeScriptValue value, double index,
                            KaleidoscopeScriptValue *out) {
    bool whole = index >= 0.0 && index == script_floor(index) && index < 65536.0;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
        KaleidoscopeScriptArray array = m->work->arrays[value.index];
        *out = whole && (uint16_t)index < array.len ? m->work->elems[array.start + (size_t)index]
                                                    : script_undef();
        return 0;
    }
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
        if (whole && (uint16_t)index < value.len)
            return script_store(m, m->work->arena + value.start + (size_t)index, 1, out);
        *out = script_undef();
        return 0;
    }
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

static size_t script_position(const ScriptMachine *m, KaleidoscopeScriptValue value,
                              size_t fallback, size_t length, bool from_end) {
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED)
        return fallback;
    double x = script_to_number(m, value);
    x = script_isnan(x) ? 0.0 : script_floor(x);
    if (x < 0.0 && from_end)
        x = (double)length + x;
    if (x < 0.0)
        return 0;
    if (x > (double)length)
        return length;
    return (size_t)x;
}

static uint8_t script_invoke(ScriptMachine *m, uint8_t function,
                             const KaleidoscopeScriptValue *args, size_t count,
                             KaleidoscopeScriptValue *out);
static bool script_word(const uint8_t *name, size_t length, const char *word);

/* `s.replace(from, f)` / `replaceAll`: f called with (match, offset, whole string) for each
   match, its text in place of the match, as upstream's `string_method`. */
static uint8_t script_replace_calling(ScriptMachine *m, bool once, KaleidoscopeScriptValue text,
                                      KaleidoscopeScriptValue pattern, uint8_t function,
                                      KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptValue from = {0};
    uint8_t e = script_to_text(m, pattern, &from);
    if (e)
        return e;
    ScriptSlice h = script_text(text), f = script_text(from);
    size_t base = m->work->replacement_count;
    bool done = false;
    /* Each match's replacement text, in order. */
    for (size_t i = 0; f.len && i < h.len;) {
        bool same = !done && i + f.len <= h.len;
        for (size_t k = 0; same && k < f.len; k++)
            same = m->work->arena[h.start + i + k] == m->work->arena[f.start + k];
        if (!same) {
            i++;
            continue;
        }
        if (m->work->live) {
            if (m->work->replacement_count == KALEIDOSCOPE_SCRIPT_REPLACEMENTS)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            KaleidoscopeScriptValue call[3] = {from, script_num((double)i), text}, r = {0};
            e = script_invoke(m, function, call, 3, &r);
            if (!e)
                e = script_to_text(m, r, &r);
            if (e) {
                m->work->replacement_count = base;
                return e;
            }
            ScriptSlice piece = script_text(r);
            m->work->replacement_start[m->work->replacement_count] = (uint16_t)piece.start;
            m->work->replacement_len[m->work->replacement_count++] = (uint16_t)piece.len;
        }
        i += f.len;
        done = once;
    }
    /* The result: the text with each match swapped for its replacement. */
    size_t first = m->work->arena_len, count = 0, next = base;
    done = false;
    if (!f.len) {
        if (first + h.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT) {
            m->work->replacement_count = base;
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        }
        for (size_t i = 0; i < h.len; i++)
            m->work->arena[first + count++] = m->work->arena[h.start + i];
    }
    for (size_t i = 0; f.len && i < h.len;) {
        bool same = !done && i + f.len <= h.len;
        for (size_t k = 0; same && k < f.len; k++)
            same = m->work->arena[h.start + i + k] == m->work->arena[f.start + k];
        size_t piece_start = h.start + i, piece_len = 1;
        if (same) {
            piece_start = 0;
            piece_len = 0;
            if (next < m->work->replacement_count) {
                piece_start = m->work->replacement_start[next];
                piece_len = m->work->replacement_len[next++];
            }
            i += f.len;
            done = once;
        } else
            i++;
        if (first + count + piece_len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT) {
            m->work->replacement_count = base;
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        }
        for (size_t k = 0; k < piece_len; k++)
            m->work->arena[first + count++] = m->work->arena[piece_start + k];
    }
    m->work->replacement_count = base;
    m->work->arena_len = first + count;
    *out = script_str(first, count);
    return 0;
}

/* The pieces of `text.split(sep)`, counted (piece < 0) or the piece-th one returned. */
static size_t script_split_scan(const ScriptMachine *m, ScriptSlice value, bool whole,
                                ScriptSlice sep, size_t piece, ScriptSlice *found) {
    size_t count = 0;
    if (whole) {
        *found = value;
        return 1;
    }
    if (!sep.len) {
        if (piece < value.len) {
            found->start = value.start + piece;
            found->len = 1;
        }
        return value.len;
    }
    size_t from = 0, i = 0;
    while (i + sep.len <= value.len) {
        bool same = true;
        for (size_t k = 0; k < sep.len; k++)
            if (m->work->arena[value.start + i + k] != m->work->arena[sep.start + k])
                same = false;
        if (same) {
            if (count == piece) {
                found->start = value.start + from;
                found->len = i - from;
            }
            count++;
            i += sep.len;
            from = i;
        } else
            i++;
    }
    if (count == piece) {
        found->start = value.start + from;
        found->len = value.len - from;
    }
    return count + 1;
}

/* `text.split(sep, limit)`: a new array of the pieces (no separator: the whole text), at most
   ELEMS of them, as upstream's `string_method`. */
static uint8_t script_split(ScriptMachine *m, KaleidoscopeScriptValue text,
                            KaleidoscopeScriptValue separator_arg,
                            KaleidoscopeScriptValue limit_arg, KaleidoscopeScriptValue *out) {
    ScriptSlice value = script_text(text), sep = {0}, piece = {0};
    bool whole = separator_arg.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED;
    if (!whole) {
        KaleidoscopeScriptValue separator = {0};
        uint8_t e = script_to_text(m, separator_arg, &separator);
        if (e)
            return e;
        sep = script_text(separator);
    }
    size_t count = script_split_scan(m, value, whole, sep, SIZE_MAX, &piece);
    if (count > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    if (limit_arg.kind != KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED) {
        double n = script_to_number(m, limit_arg);
        size_t limit = script_isnan(n) || n <= 0.0 ? 0
                       : n >= 4294967295.0         ? UINT32_MAX
                                                   : (size_t)(uint32_t)n;
        if (count > limit)
            count = limit;
    }
    uint8_t id = 0;
    uint8_t e = script_allocate_array(m, count, &id);
    if (e)
        return e;
    for (size_t i = 0; i < count; i++) {
        if (whole)
            m->work->elems[m->work->arrays[id].start + i] = text;
        else {
            script_split_scan(m, value, false, sep, i, &piece);
            m->work->elems[m->work->arrays[id].start + i] = script_str(piece.start, piece.len);
        }
    }
    *out = script_array(id);
    return 0;
}

/* `s.substr(start, length)` and `s.localeCompare(t)`, as upstream's `string_method`. */
static uint8_t script_string_extra(ScriptMachine *m, const uint8_t *name, size_t name_len,
                                   KaleidoscopeScriptValue text, KaleidoscopeScriptValue first,
                                   KaleidoscopeScriptValue second, KaleidoscopeScriptValue *out) {
    ScriptSlice value = script_text(text);
    if (script_word(name, name_len, "substr")) {
        /* Start (negative: from the end) and length, in bytes. */
        size_t a = script_position(m, first, 0, value.len, true);
        size_t l = script_position(m, second, value.len - a, value.len, false);
        if (l > value.len - a)
            l = value.len - a;
        *out = script_str(value.start + a, l);
        return 0;
    }
    /* ASCII letters case-insensitively first, then lower before upper (close to the default
       collation for plain text). */
    KaleidoscopeScriptValue other = {0};
    uint8_t e = script_to_text(m, first, &other);
    if (e)
        return e;
    ScriptSlice b = script_text(other);
    int order = 0;
    for (size_t i = 0; !order && i < value.len && i < b.len; i++) {
        uint8_t x = m->work->arena[value.start + i], y = m->work->arena[b.start + i];
        uint8_t lx = x >= 'A' && x <= 'Z' ? (uint8_t)(x - 'A' + 'a') : x;
        uint8_t ly = y >= 'A' && y <= 'Z' ? (uint8_t)(y - 'A' + 'a') : y;
        bool ux = x >= 'A' && x <= 'Z', uy = y >= 'A' && y <= 'Z';
        order = lx != ly ? (lx < ly ? -1 : 1) : ux != uy ? (ux ? 1 : -1) : 0;
    }
    if (!order && value.len != b.len)
        order = value.len < b.len ? -1 : 1;
    *out = script_num((double)order);
    return 0;
}

static uint8_t script_string_method(ScriptMachine *m, bool trim, bool trim_start, bool trim_end,
                                    bool at_method, bool index_of, bool char_at, bool char_code,
                                    bool code_point, bool slice, bool substring, bool split,
                                    bool replace, bool replace_all, bool starts, bool ends,
                                    bool includes, bool repeat, bool pad_start, bool pad_end,
                                    bool last_index, KaleidoscopeScriptValue text,
                                    KaleidoscopeScriptValue args[2], KaleidoscopeScriptValue *out) {
    ScriptSlice value = script_text(text);
    if (trim) {
        size_t a = 0, b = value.len;
        while (a < b) {
            uint8_t c = m->work->arena[value.start + a];
            if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c))
                break;
            a++;
        }
        while (b > a) {
            uint8_t c = m->work->arena[value.start + b - 1];
            if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c))
                break;
            b--;
        }
        *out = script_str(value.start + a, b - a);
        return 0;
    }
    if (trim_start || trim_end) {
        size_t a = 0, b = value.len;
        if (trim_start)
            while (a < b) {
                uint8_t c = m->work->arena[value.start + a];
                if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c))
                    break;
                a++;
            }
        else
            while (b > a) {
                uint8_t c = m->work->arena[value.start + b - 1];
                if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c))
                    break;
                b--;
            }
        *out = script_str(value.start + a, b - a);
        return 0;
    }
    if (at_method) {
        double index = script_to_number(m, args[0]);
        index = index < 0.0 ? (double)value.len + script_floor(index) : script_floor(index);
        *out = index >= 0.0 && (size_t)index < value.len
                   ? script_str(value.start + (size_t)index, 1)
                   : script_undef();
        return 0;
    }
    if (starts || ends || includes || last_index) {
        KaleidoscopeScriptValue needle = {0};
        uint8_t e = script_to_text(m, args[0], &needle);
        if (e)
            return e;
        ScriptSlice n = script_text(needle);
        if (starts) {
            bool same = n.len <= value.len;
            for (size_t i = 0; same && i < n.len; i++)
                same = m->work->arena[value.start + i] == m->work->arena[n.start + i];
            *out = script_bool(same);
            return 0;
        }
        if (ends) {
            bool same = n.len <= value.len;
            for (size_t i = 0; same && i < n.len; i++)
                same = m->work->arena[value.start + value.len - n.len + i] ==
                       m->work->arena[n.start + i];
            *out = script_bool(same);
            return 0;
        }
        size_t found = SIZE_MAX;
        if (!n.len)
            found = last_index ? value.len : 0;
        else if (last_index) {
            for (size_t i = 0; i + n.len <= value.len; i++) {
                bool same = true;
                for (size_t k = 0; k < n.len; k++)
                    if (m->work->arena[value.start + i + k] != m->work->arena[n.start + k])
                        same = false;
                if (same)
                    found = i;
            }
        } else {
            for (size_t i = 0; i + n.len <= value.len && found == SIZE_MAX; i++) {
                bool same = true;
                for (size_t k = 0; k < n.len; k++)
                    if (m->work->arena[value.start + i + k] != m->work->arena[n.start + k])
                        same = false;
                if (same)
                    found = i;
            }
        }
        *out = includes ? script_bool(found != SIZE_MAX)
                        : script_num(found == SIZE_MAX ? -1.0 : (double)found);
        return 0;
    }
    if ((replace || replace_all) && args[1].kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
        return script_replace_calling(m, replace, text, args[0], args[1].index, out);
    if (replace || replace_all) {
        KaleidoscopeScriptValue from = {0}, to = {0};
        uint8_t e = script_to_text(m, args[0], &from);
        if (e)
            return e;
        e = script_to_text(m, args[1], &to);
        if (e)
            return e;
        ScriptSlice f = script_text(from), t = script_text(to);
        size_t first = m->work->arena_len, count = 0;
        if (!f.len) {
            if (first + t.len + value.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            for (size_t i = 0; i < t.len; i++)
                m->work->arena[first + count++] = m->work->arena[t.start + i];
            for (size_t i = 0; i < value.len; i++)
                m->work->arena[first + count++] = m->work->arena[value.start + i];
            m->work->arena_len = first + count;
            *out = script_str(first, count);
            return 0;
        }
        bool done = false;
        for (size_t i = 0; i < value.len;) {
            bool same = !done && i + f.len <= value.len;
            for (size_t k = 0; same && k < f.len; k++)
                same = m->work->arena[value.start + i + k] == m->work->arena[f.start + k];
            if (same) {
                if (first + count + t.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
                    return KALEIDOSCOPE_SCRIPT_LIMIT;
                for (size_t k = 0; k < t.len; k++)
                    m->work->arena[first + count++] = m->work->arena[t.start + k];
                i += f.len;
                done = replace;
            } else {
                if (first + count >= KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
                    return KALEIDOSCOPE_SCRIPT_LIMIT;
                m->work->arena[first + count++] = m->work->arena[value.start + i++];
            }
        }
        m->work->arena_len = first + count;
        *out = script_str(first, count);
        return 0;
    }
    if (repeat) {
        double number = script_to_number(m, args[0]);
        if (number < 0.0 || number > 1024.0 || number != script_floor(number))
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        size_t copies = (size_t)number;
        if (value.len && copies > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT / value.len)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        size_t first = m->work->arena_len, total = value.len * copies;
        if (first + total > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t k = 0; k < copies; k++)
            for (size_t i = 0; i < value.len; i++)
                m->work->arena[first + k * value.len + i] = m->work->arena[value.start + i];
        m->work->arena_len = first + total;
        *out = script_str(first, total);
        return 0;
    }
    if (pad_start || pad_end) {
        double requested = script_to_number(m, args[0]);
        if (!(requested <= 1024.0))
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        size_t want = requested <= 0.0 ? 0 : (size_t)requested;
        KaleidoscopeScriptValue fill = {0};
        if (args[1].kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED) {
            static const uint8_t space = ' ';
            uint8_t e = script_store(m, &space, 1, &fill);
            if (e)
                return e;
        } else {
            uint8_t e = script_to_text(m, args[1], &fill);
            if (e)
                return e;
        }
        ScriptSlice f = script_text(fill);
        if (!f.len || want <= value.len) {
            *out = text;
            return 0;
        }
        size_t padding = want - value.len, first = m->work->arena_len;
        if (first + want > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        size_t at = 0;
        if (pad_start)
            for (size_t i = 0; i < padding; i++)
                m->work->arena[first + at++] = m->work->arena[f.start + i % f.len];
        for (size_t i = 0; i < value.len; i++)
            m->work->arena[first + at++] = m->work->arena[value.start + i];
        if (pad_end)
            for (size_t i = 0; i < padding; i++)
                m->work->arena[first + at++] = m->work->arena[f.start + i % f.len];
        m->work->arena_len = first + at;
        *out = script_str(first, at);
        return 0;
    }
    if (index_of) {
        KaleidoscopeScriptValue needle = {0};
        uint8_t e = script_to_text(m, args[0], &needle);
        if (e)
            return e;
        ScriptSlice n = script_text(needle);
        if (!n.len) {
            *out = script_num(0);
            return 0;
        }
        for (size_t i = 0; i + n.len <= value.len; i++) {
            bool same = true;
            for (size_t k = 0; k < n.len; k++)
                if (m->work->arena[value.start + i + k] != m->work->arena[n.start + k])
                    same = false;
            if (same) {
                *out = script_num((double)i);
                return 0;
            }
        }
        *out = script_num(-1);
        return 0;
    }
    if (char_code) {
        size_t at = script_position(m, args[0], 0, value.len, false);
        *out = script_num(at < value.len ? (double)m->work->arena[value.start + at] : script_nan());
        return 0;
    }
    if (code_point) {
        size_t at = script_position(m, args[0], 0, value.len, false);
        if (at >= value.len) {
            *out = script_undef();
            return 0;
        }
        const uint8_t *text = m->work->arena + value.start + at;
        size_t left = value.len - at, width = text[0] >= 0xc0 && text[0] <= 0xdf   ? 2
                                              : text[0] >= 0xe0 && text[0] <= 0xef ? 3
                                              : text[0] >= 0xf0 && text[0] <= 0xf7 ? 4
                                                                                   : 1;
        uint32_t code = text[0];
        if (width <= left && script_utf8_valid(text, width)) {
            code = width == 1   ? text[0]
                   : width == 2 ? (uint32_t)(text[0] & 31u)
                                : (uint32_t)(width == 3 ? text[0] & 15u : text[0] & 7u);
            for (size_t i = 1; i < width; i++)
                code = (code << 6) | (uint32_t)(text[i] & 63u);
        }
        *out = script_num((double)code);
        return 0;
    }
    if (char_at) {
        size_t at = script_position(m, args[0], 0, value.len, false);
        if (at < value.len) {
            *out = script_str(value.start + at, 1);
            return 0;
        }
        return script_store(m, NULL, 0, out);
    }
    if (slice || substring) {
        size_t a = script_position(m, args[0], 0, value.len, slice),
               b = script_position(m, args[1], value.len, value.len, slice);
        if (substring && a > b) {
            size_t swap = a;
            a = b;
            b = swap;
        }
        if (b < a)
            b = a;
        *out = script_str(value.start + a, b - a);
        return 0;
    }
    if (split)
        return script_split(m, text, args[0], args[1], out);
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

static bool script_utf8_valid(const uint8_t *text, size_t length) {
    for (size_t i = 0; i < length;) {
        uint8_t c = text[i];
        size_t n = c < 0x80             ? 1
                   : (c & 0xe0) == 0xc0 ? 2
                   : (c & 0xf0) == 0xe0 ? 3
                   : (c & 0xf8) == 0xf0 ? 4
                                        : 0;
        if (!n || i + n > length)
            return false;
        for (size_t k = 1; k < n; k++)
            if ((text[i + k] & 0xc0) != 0x80)
                return false;
        uint32_t code = n == 1 ? c : n == 2 ? (c & 31u) : n == 3 ? (c & 15u) : (c & 7u);
        for (size_t k = 1; k < n; k++)
            code = (code << 6) | (text[i + k] & 63u);
        if ((n == 2 && code < 0x80) || (n == 3 && code < 0x800) || (n == 4 && code < 0x10000) ||
            code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff))
            return false;
        i += n;
    }
    return true;
}

static uint8_t script_convert(ScriptMachine *m, bool as_string, bool as_number, bool as_boolean,
                              bool parse_float, bool is_nan, bool is_finite, bool encode_uri,
                              bool decode_uri, KaleidoscopeScriptValue *out) {
    uint8_t e = script_expect(m, '(');
    if (e)
        return e;
    KaleidoscopeScriptValue value = script_undef();
    if (!script_punct(m, ')')) {
        e = script_expr(m, &value);
        if (e)
            return e;
    }
    e = script_expect(m, ')');
    if (e)
        return e;
    if (as_string)
        return script_to_text(m, value, out);
    if (as_boolean) {
        *out = script_bool(script_truthy(value));
        return 0;
    }
    if (is_nan) {
        *out = script_bool(script_isnan(script_to_number(m, value)));
        return 0;
    }
    if (is_finite) {
        double x = script_to_number(m, value);
        *out = script_bool(!script_isnan(x) && !script_isinf(x));
        return 0;
    }
    if (as_number) {
        *out = script_num(value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED
                              ? script_nan()
                              : script_to_number(m, value));
        return 0;
    }
    KaleidoscopeScriptValue text = {0};
    e = script_to_text(m, value, &text);
    if (e)
        return e;
    ScriptSlice slice = script_text(text);
    if (encode_uri || decode_uri) {
        size_t first = m->work->arena_len, count = 0;
        if (encode_uri) {
            static const uint8_t hex[] = "0123456789ABCDEF";
            for (size_t i = 0; i < slice.len; i++) {
                uint8_t c = m->work->arena[slice.start + i];
                bool safe = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
                            c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' || c == ')';
                if (safe)
                    e = script_region_append(m, first, &count, &c, 1);
                else {
                    uint8_t encoded[3] = {'%', hex[c >> 4], hex[c & 15]};
                    e = script_region_append(m, first, &count, encoded, 3);
                }
                if (e)
                    return e;
            }
        } else {
            for (size_t i = 0; i < slice.len;) {
                uint8_t c = m->work->arena[slice.start + i++];
                if (c == '%') {
                    if (i + 2 > slice.len)
                        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                    int high = script_hex_digit(m->work->arena[slice.start + i]),
                        low = script_hex_digit(m->work->arena[slice.start + i + 1]);
                    if (high < 0 || low < 0)
                        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                    c = (uint8_t)(high * 16 + low);
                    i += 2;
                }
                e = script_region_append(m, first, &count, &c, 1);
                if (e)
                    return e;
            }
            if (!script_utf8_valid(m->work->arena + first, count))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        *out = script_str(first, count);
        return 0;
    }
    size_t at = 0;
    while (at < slice.len) {
        uint8_t c = m->work->arena[slice.start + at];
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c))
            break;
        at++;
    }
    if (parse_float) {
        double best = script_nan();
        size_t limit = slice.len - at < 64 ? slice.len - at : 64;
        for (size_t n = 1; n <= limit; n++) {
            uint8_t last = m->work->arena[slice.start + at + n - 1];
            if (last == 'e' || last == 'E' || last == '+' || last == '-' || last == '.')
                continue;
            bool plain = false, valid = true;
            for (size_t i = 0; i < n; i++) {
                uint8_t c = m->work->arena[slice.start + at + i];
                plain |= c >= '0' && c <= '9';
                valid &= (c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' || c == '+' ||
                         c == '-';
            }
            double number = 0;
            if (plain && valid &&
                script_parse_text_number(m->work->arena + slice.start + at, n, &number))
                best = number;
        }
        *out = script_num(best);
        return 0;
    }
    bool negative = false;
    if (at < slice.len &&
        (m->work->arena[slice.start + at] == '+' || m->work->arena[slice.start + at] == '-')) {
        negative = m->work->arena[slice.start + at] == '-';
        at++;
    }
    size_t first = at;
    double number = 0.0;
    while (at < slice.len && m->work->arena[slice.start + at] >= '0' &&
           m->work->arena[slice.start + at] <= '9') {
        number = number * 10.0 + (m->work->arena[slice.start + at] - '0');
        at++;
    }
    *out = script_num(at == first ? script_nan() : negative ? -number : number);
    return 0;
}

/* Elementary functions without libm (upstream's fmath): range reduction, then a polynomial. */
#define SCRIPT_PI 3.14159265358979323846264338327950288
#define SCRIPT_FRAC_PI_2 1.57079632679489661923132169163975144
#define SCRIPT_LN_2 0.693147180559945309417232121458176568
#define SCRIPT_LN_10 2.30258509299404568401799145468436421
#define SCRIPT_SQRT_2 1.41421356237309504880168872420969808

/* 2^k for an integer exponent in the normal range (0 or infinity outside it). */
static double fmath_pow2(int64_t k) {
    if (k > 1023)
        return script_inf();
    if (k < -1022)
        return k < -1074 ? 0.0 : script_from_bits(UINT64_C(1) << (k + 1074));
    return script_from_bits((uint64_t)(k + 1023) << 52);
}

static double fmath_round(double x) {
    double t = (double)script_i64(x);
    if (script_fabs(x - t) >= 0.5)
        return t + (x > 0.0 ? 1.0 : -1.0);
    return t;
}

static double fmath_exp(double x) {
    if (script_isnan(x))
        return x;
    if (x > 709.8)
        return script_inf();
    if (x < -745.2)
        return 0.0;
    /* x = k ln2 + r, |r| <= ln2/2, with ln2 split so k ln2 is exact. */
    const double ln2_hi = 6.93147180369123816490e-01, ln2_lo = 1.90821492927058770002e-10;
    double k = fmath_round(x / SCRIPT_LN_2);
    double r = (x - k * ln2_hi) - k * ln2_lo;
    double term = 1.0, sum = 1.0;
    for (int i = 1; i <= 20; i++) {
        term *= r / (double)i;
        sum += term;
        if (script_fabs(term) < 1e-18)
            break;
    }
    int64_t n = script_i64(k);
    /* Scale in two steps so 2^k never leaves the normal range on its own. */
    if (n > 1000)
        return sum * fmath_pow2(n - 1000) * fmath_pow2(1000);
    if (n < -1000)
        return sum * fmath_pow2(n + 1000) * fmath_pow2(-1000);
    return sum * fmath_pow2(n);
}

static double fmath_ln(double x) {
    if (script_isnan(x) || x < 0.0)
        return script_nan();
    if (x == 0.0)
        return -script_inf();
    if (script_isinf(x))
        return x;
    if (x == 1.0)
        return 0.0;
    /* x = m 2^e with m in [sqrt(1/2), sqrt(2)); ln m = 2 atanh((m-1)/(m+1)). */
    double m = x;
    int64_t e = 0;
    if (m < 2.2250738585072014e-308) {
        m *= fmath_pow2(54);
        e -= 54;
    }
    uint64_t bits = script_to_bits(m);
    e += (int64_t)((bits >> 52) & 0x7ff) - 1023;
    m = script_from_bits((bits & UINT64_C(0x000fffffffffffff)) | UINT64_C(0x3ff0000000000000));
    if (m > SCRIPT_SQRT_2) {
        m /= 2.0;
        e += 1;
    }
    double s = (m - 1.0) / (m + 1.0), s2 = s * s, term = s, sum = 0.0, i = 1.0;
    while (i < 60.0) {
        sum += term / i;
        term *= s2;
        i += 2.0;
        if (script_fabs(term) < 1e-20)
            break;
    }
    return (double)e * SCRIPT_LN_2 + 2.0 * sum;
}

/* sin and cos of x reduced to [-pi/4, pi/4] by quarter turns. */
static void fmath_sin_cos(double x, double *sine, double *cosine) {
    if (script_isnan(x) || script_isinf(x)) {
        *sine = *cosine = script_nan();
        return;
    }
    if (x == 0.0) {
        *sine = x;
        *cosine = 1.0;
        return;
    }
    const double pio2_hi = 1.57079632673412561417e+00, pio2_lo = 6.07710050650619224932e-11;
    double q = fmath_round(x / SCRIPT_FRAC_PI_2);
    double r = (x - q * pio2_hi) - q * pio2_lo, r2 = r * r;
    double st = r, s = r, ct = 1.0, c = 1.0;
    for (int i = 1; i <= 12; i++) {
        double n = (double)(2 * i);
        st *= -r2 / (n * (n + 1.0));
        s += st;
        ct *= -r2 / ((n - 1.0) * n);
        c += ct;
    }
    int64_t turn = script_i64(q) % 4;
    if (turn < 0)
        turn += 4;
    *sine = turn == 0 ? s : turn == 1 ? c : turn == 2 ? -s : -c;
    *cosine = turn == 0 ? c : turn == 1 ? -s : turn == 2 ? -c : s;
}

/* atan for any x: |x| > 1 uses pi/2 - atan(1/x); then two half-angle steps make the series
   converge quickly. */
static double fmath_atan(double x) {
    if (script_isnan(x))
        return x;
    if (script_isinf(x))
        return script_copysign(SCRIPT_FRAC_PI_2, x);
    if (script_fabs(x) > 1.0)
        return script_copysign(SCRIPT_FRAC_PI_2, x) - fmath_atan(1.0 / x);
    double t = x;
    for (int k = 0; k < 2; k++)
        t /= 1.0 + script_sqrt(1.0 + t * t);
    double t2 = t * t, term = t, sum = 0.0, i = 1.0;
    while (i < 80.0) {
        sum += term / i;
        term *= -t2;
        i += 2.0;
        if (script_fabs(term) < 1e-20)
            break;
    }
    return 4.0 * sum;
}

static bool script_sign_negative(double x) {
    return (script_to_bits(x) >> 63) != 0;
}

static double fmath_atan2(double y, double x) {
    if (script_isnan(y) || script_isnan(x))
        return script_nan();
    if (x == 0.0) {
        if (y == 0.0)
            return script_sign_negative(x) ? script_copysign(SCRIPT_PI, y) : y;
        return script_copysign(SCRIPT_FRAC_PI_2, y);
    }
    if (script_isinf(x) && script_isinf(y))
        return script_copysign(x > 0.0 ? SCRIPT_PI / 4.0 : 3.0 * SCRIPT_PI / 4.0, y);
    double a = fmath_atan(y / x);
    if (x > 0.0)
        return a;
    return y >= 0.0 && !script_sign_negative(y) ? a + SCRIPT_PI : a - SCRIPT_PI;
}

/* log10 / log2, exact at exact powers of the base (`log10(1000)` is 3). */
static double fmath_log_base(double x, double base, double ln_base) {
    double r = fmath_ln(x) / ln_base, n = fmath_round(r);
    if (!script_isnan(r) && !script_isinf(r) && script_fabs(r - n) < 1e-9 &&
        script_fabs(n) <= 300.0) {
        double p = 1.0;
        for (int64_t k = 0; k < script_i64(script_fabs(n)); k++)
            p *= base;
        if ((n < 0.0 ? 1.0 / p : p) == x)
            return n;
    }
    return r;
}

/* The index of the Math function named by the current token (28: none). */
static size_t script_math_function(const ScriptMachine *m) {
    if (script_ident(m, "floor"))
        return 0;
    if (script_ident(m, "ceil"))
        return 1;
    if (script_ident(m, "round"))
        return 2;
    if (script_ident(m, "abs"))
        return 3;
    if (script_ident(m, "trunc"))
        return 4;
    if (script_ident(m, "sign"))
        return 5;
    if (script_ident(m, "sqrt"))
        return 6;
    if (script_ident(m, "exp"))
        return 7;
    if (script_ident(m, "expm1"))
        return 8;
    if (script_ident(m, "log"))
        return 9;
    if (script_ident(m, "log1p"))
        return 10;
    if (script_ident(m, "log2"))
        return 11;
    if (script_ident(m, "log10"))
        return 12;
    if (script_ident(m, "sin"))
        return 13;
    if (script_ident(m, "cos"))
        return 14;
    if (script_ident(m, "tan"))
        return 15;
    if (script_ident(m, "atan"))
        return 16;
    if (script_ident(m, "asin"))
        return 17;
    if (script_ident(m, "acos"))
        return 18;
    if (script_ident(m, "atan2"))
        return 19;
    if (script_ident(m, "cbrt"))
        return 20;
    if (script_ident(m, "hypot"))
        return 21;
    if (script_ident(m, "sinh"))
        return 22;
    if (script_ident(m, "cosh"))
        return 23;
    if (script_ident(m, "tanh"))
        return 24;
    if (script_ident(m, "pow"))
        return 25;
    if (script_ident(m, "max"))
        return 26;
    if (script_ident(m, "min"))
        return 27;
    return 28;
}

/* A Math constant named by the current token. */
static bool script_math_constant(const ScriptMachine *m, double *out) {
    if (script_ident(m, "PI")) {
        *out = 3.14159265358979323846264338327950288;
        return true;
    }
    if (script_ident(m, "E")) {
        *out = 2.71828182845904523536028747135266250;
        return true;
    }
    if (script_ident(m, "LN2")) {
        *out = 0.693147180559945309417232121458176568;
        return true;
    }
    if (script_ident(m, "LN10")) {
        *out = 2.30258509299404568401799145468436421;
        return true;
    }
    if (script_ident(m, "LOG2E")) {
        *out = 1.44269504088896340735992468100189214;
        return true;
    }
    if (script_ident(m, "LOG10E")) {
        *out = 0.434294481903251827651128918916605082;
        return true;
    }
    if (script_ident(m, "SQRT2")) {
        *out = 1.41421356237309504880168872420969808;
        return true;
    }
    if (script_ident(m, "SQRT1_2")) {
        *out = 0.707106781186547524400844362104849039;
        return true;
    }
    return false;
}

/* `Math.name(...)` and the Math constants, as upstream's `math`. */
static uint8_t script_math(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    uint8_t e = script_expect(m, '.');
    if (e)
        return e;
    if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
        return KALEIDOSCOPE_SCRIPT_SYNTAX;
    size_t which = script_math_function(m);
    double constant = 0.0;
    if (script_math_constant(m, &constant)) {
        *out = script_num(constant);
        return script_next(m);
    }
    e = script_next(m);
    if (e)
        return e;
    e = script_expect(m, '(');
    if (e)
        return e;
    KaleidoscopeScriptValue values[32] = {0};
    size_t n = 0;
    e = script_items(m, ')', values, 32, &n);
    if (e)
        return e;
    double args[32] = {0};
    for (size_t i = 0; i < n; i++)
        args[i] = script_to_number(m, values[i]);
    if (which == 28)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    double a = args[0], value = script_nan(), sine = 0.0, cosine = 0.0;
    bool one = n > 0;
    switch (which) {
    case 0:
        if (one)
            value = script_floor(a);
        break;
    case 1:
        if (one)
            value = -script_floor(-a);
        break;
    case 2:
        if (one)
            value = script_floor(a + 0.5);
        break;
    case 3:
        if (one)
            value = a < 0.0 ? -a : a;
        break;
    case 4:
        if (one)
            value = a < 0.0 ? -script_floor(-a) : script_floor(a);
        break;
    case 5:
        if (one)
            value = script_isnan(a) ? a : a > 0.0 ? 1.0 : a < 0.0 ? -1.0 : a;
        break;
    case 6:
        if (one)
            value = script_sqrt(a);
        break;
    case 7:
        if (one)
            value = fmath_exp(a);
        break;
    case 8:
        if (one)
            value = script_fabs(a) < 1e-5 ? a + a * a / 2.0 : fmath_exp(a) - 1.0;
        break;
    case 9:
        if (one)
            value = fmath_ln(a);
        break;
    case 10:
        if (one)
            value = script_fabs(a) < 1e-5 ? a - a * a / 2.0 : fmath_ln(1.0 + a);
        break;
    case 11:
        if (one)
            value = fmath_log_base(a, 2.0, SCRIPT_LN_2);
        break;
    case 12:
        if (one)
            value = fmath_log_base(a, 10.0, SCRIPT_LN_10);
        break;
    case 13:
    case 14:
    case 15:
        if (one) {
            fmath_sin_cos(a, &sine, &cosine);
            value = which == 13 ? sine : which == 14 ? cosine : sine / cosine;
        }
        break;
    case 16:
        if (one)
            value = fmath_atan(a);
        break;
    case 17:
        if (one)
            value = script_fabs(a) > 1.0 ? script_nan() : fmath_atan2(a, script_sqrt(1.0 - a * a));
        break;
    case 18:
        if (one)
            value = script_fabs(a) > 1.0 ? script_nan() : fmath_atan2(script_sqrt(1.0 - a * a), a);
        break;
    case 19:
        value = n < 2 ? script_nan() : fmath_atan2(a, args[1]);
        break;
    case 20:
        if (one) {
            if (a == 0.0 || script_isnan(a) || script_isinf(a))
                value = a;
            else {
                double r = script_copysign(fmath_exp(fmath_ln(script_fabs(a)) / 3.0), a);
                value = r - (r * r * r - a) / (3.0 * r * r);
            }
        }
        break;
    case 21: {
        double largest = 0.0;
        for (size_t i = 0; i < n; i++) {
            double x = args[i];
            if (script_isinf(x))
                largest = script_inf();
            else if (script_isinf(largest))
                ;
            else if (script_isnan(x) || script_isnan(largest))
                largest = script_nan();
            else if (script_fabs(x) > largest)
                largest = script_fabs(x);
        }
        if (largest == 0.0 || script_isnan(largest) || script_isinf(largest))
            value = largest;
        else {
            double sum = 0.0;
            for (size_t i = 0; i < n; i++)
                sum += (args[i] / largest) * (args[i] / largest);
            value = largest * script_sqrt(sum);
        }
        break;
    }
    case 22:
        if (one)
            value = (fmath_exp(a) - fmath_exp(-a)) / 2.0;
        break;
    case 23:
        if (one)
            value = (fmath_exp(a) + fmath_exp(-a)) / 2.0;
        break;
    case 24:
        if (one) {
            if (script_fabs(a) > 20.0)
                value = script_copysign(1.0, a);
            else {
                double x = fmath_exp(2.0 * a);
                value = (x - 1.0) / (x + 1.0);
            }
        }
        break;
    case 25:
        value = n < 2 ? script_nan() : script_pow(a, args[1]);
        break;
    case 26:
        value = -script_inf();
        for (size_t i = 0; i < n; i++)
            value = script_isnan(args[i]) || script_isnan(value) ? script_nan()
                    : args[i] > value                            ? args[i]
                                                                 : value;
        break;
    default:
        value = script_inf();
        for (size_t i = 0; i < n; i++)
            value = script_isnan(args[i]) || script_isnan(value) ? script_nan()
                    : args[i] < value                            ? args[i]
                                                                 : value;
        break;
    }
    *out = script_num(value);
    return 0;
}

/* A template's substitutions, as upstream's `template_parts`: each value and the offset just
   past its `}` go on the argument stack as a pair; *values counts them. The caller restores
   argument_top. Pieces longer in all than the arena are refused. */
static uint8_t script_template_parts(ScriptMachine *m, size_t raw_start, size_t raw_len,
                                     bool cooked, size_t *values) {
    KaleidoscopeScriptPosition resume = script_position_save(m);
    size_t top = m->work->argument_top, total = 0, i = 0, n = 0;
    uint8_t e = 0;
    while (i < raw_len && !e) {
        if (i + 1 < raw_len && m->source->bytes[raw_start + i] == '$' &&
            m->source->bytes[raw_start + i + 1] == '{') {
            m->work->lexer.at = raw_start + i + 2;
            e = script_next(m);
            KaleidoscopeScriptValue value = {0};
            if (!e)
                e = script_expr(m, &value);
            if (!e && !script_punct(m, '}'))
                e = KALEIDOSCOPE_SCRIPT_SYNTAX;
            if (!e && top + 2 * n + 2 > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
                e = KALEIDOSCOPE_SCRIPT_LIMIT;
            if (e)
                break;
            i = m->work->lexer.at - raw_start;
            m->work->argument_stack[top + 2 * n] = value;
            m->work->argument_stack[top + 2 * n + 1] = script_num((double)i);
            n++;
            m->work->argument_top = top + 2 * n;
        } else if (m->source->bytes[raw_start + i] == '\\' && i + 1 < raw_len) {
            total += cooked ? 1 : 2;
            i += 2;
        } else {
            total++;
            i++;
        }
        if (!e && total > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
            e = KALEIDOSCOPE_SCRIPT_LIMIT;
    }
    if (e)
        return e;
    script_position_restore(m, resume);
    *values = n;
    return 0;
}

/* Appends the piece of a template starting at `from` (up to the next `${`), cooked (`\n`, `\t`,
   other escapes their character) or raw, at the arena end; *next is where it stops. */
static uint8_t script_template_piece(ScriptMachine *m, size_t raw_start, size_t raw_len,
                                     size_t from, bool cooked, size_t first, size_t *count,
                                     size_t *next) {
    size_t i = from;
    while (i < raw_len && !(i + 1 < raw_len && m->source->bytes[raw_start + i] == '$' &&
                            m->source->bytes[raw_start + i + 1] == '{')) {
        uint8_t pair[2] = {m->source->bytes[raw_start + i], 0};
        size_t n = 1;
        if (pair[0] == '\\' && i + 1 < raw_len) {
            uint8_t escaped = m->source->bytes[raw_start + i + 1];
            if (cooked)
                pair[0] = escaped == 'n' ? '\n' : escaped == 't' ? '\t' : escaped;
            else {
                pair[1] = escaped;
                n = 2;
            }
            i += 2;
        } else
            i++;
        if (first + *count + n > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t k = 0; k < n; k++)
            m->work->arena[first + (*count)++] = pair[k];
        m->work->arena_len = first + *count;
    }
    *next = i;
    return 0;
}

/* tag`a${x}b`: calls tag(strings, ...values) with the cooked pieces, as upstream's
   `tagged_template`. */
static uint8_t script_tagged_template(ScriptMachine *m, uint8_t function, size_t raw_start,
                                      size_t raw_len, KaleidoscopeScriptValue *out) {
    size_t top = m->work->argument_top, n = 0;
    uint8_t e = script_template_parts(m, raw_start, raw_len, true, &n);
    if (!e && !m->work->live) {
        m->work->argument_top = top;
        *out = script_undef();
        return 0;
    }
    /* The pieces, stored one by one above the pairs, then their array. */
    size_t strings = top + 2 * n, from = 0;
    if (!e && strings + n + 1 > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
        e = KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t k = 0; k <= n && !e; k++) {
        size_t first = m->work->arena_len, count = 0, next = 0;
        if (k)
            from = (size_t)m->work->argument_stack[top + 2 * k - 1].number;
        e = script_template_piece(m, raw_start, raw_len, from, true, first, &count, &next);
        m->work->argument_stack[strings + k] = script_str(first, count);
        m->work->argument_top = strings + k + 1;
    }
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    if (!e)
        e = script_new_array(m, m->work->argument_stack + strings, n + 1, &args[0]);
    if (!e && n + 1 > KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT)
        e = KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t k = 0; k < n && !e; k++)
        args[1 + k] = m->work->argument_stack[top + 2 * k];
    m->work->argument_top = top;
    if (e)
        return e;
    return script_invoke(m, function, args, n + 1, out);
}

/* String.raw`...`: escapes kept as written, substitutions filled in. */
static uint8_t script_string_raw(ScriptMachine *m, size_t raw_start, size_t raw_len,
                                 KaleidoscopeScriptValue *out) {
    size_t top = m->work->argument_top, n = 0;
    uint8_t e = script_template_parts(m, raw_start, raw_len, false, &n);
    for (size_t k = 0; k < n && !e; k++) {
        KaleidoscopeScriptValue text = {0};
        e = script_to_text(m, m->work->argument_stack[top + 2 * k], &text);
        m->work->argument_stack[top + 2 * k] = text;
    }
    size_t first = m->work->arena_len, count = 0, from = 0;
    for (size_t k = 0; k <= n && !e; k++) {
        size_t next = 0;
        if (k)
            from = (size_t)m->work->argument_stack[top + 2 * k - 1].number;
        e = script_template_piece(m, raw_start, raw_len, from, false, first, &count, &next);
        if (!e && k < n) {
            ScriptSlice text = script_text(m->work->argument_stack[top + 2 * k]);
            if (first + count + text.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
                e = KALEIDOSCOPE_SCRIPT_LIMIT;
            for (size_t i = 0; !e && i < text.len; i++)
                m->work->arena[first + count++] = m->work->arena[text.start + i];
            m->work->arena_len = first + count;
        }
    }
    m->work->argument_top = top;
    if (!e)
        *out = script_str(first, count);
    return e;
}

static uint8_t script_template(ScriptMachine *m, size_t raw_start, size_t raw_len,
                               KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptPosition resume = script_position_save(m);
    size_t first = m->work->arena_len, count = 0, i = 0;
    uint8_t error = 0;
    while (i < raw_len && !error) {
        if (i + 1 < raw_len && m->source->bytes[raw_start + i] == '$' &&
            m->source->bytes[raw_start + i + 1] == '{') {
            m->work->lexer.at = raw_start + i + 2;
            error = script_next(m);
            KaleidoscopeScriptValue value = {0};
            if (!error)
                error = script_expr(m, &value);
            if (!error && !script_punct(m, '}'))
                error = KALEIDOSCOPE_SCRIPT_SYNTAX;
            if (error)
                break;
            error = script_to_text(m, value, &value);
            if (error)
                break;
            ScriptSlice text = script_text(value);
            if (first + count + text.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT) {
                error = KALEIDOSCOPE_SCRIPT_LIMIT;
                break;
            }
            for (size_t k = 0; k < text.len; k++)
                m->work->arena[first + count + k] = m->work->arena[text.start + k];
            count += text.len;
            m->work->arena_len = first + count;
            i = m->work->lexer.at - raw_start;
        } else {
            uint8_t byte = m->source->bytes[raw_start + i++];
            if (byte == '\\' && i < raw_len) {
                byte = m->source->bytes[raw_start + i++];
                if (byte == 'n')
                    byte = '\n';
                else if (byte == 't')
                    byte = '\t';
            }
            if (first + count >= KALEIDOSCOPE_SCRIPT_ARENA_LIMIT) {
                error = KALEIDOSCOPE_SCRIPT_LIMIT;
                break;
            }
            m->work->arena[first + count++] = byte;
            m->work->arena_len = first + count;
        }
    }
    if (error) {
        return error;
    }
    script_position_restore(m, resume);
    *out = script_str(first, count);
    return 0;
}

static uint8_t script_call(ScriptMachine *m, size_t name_start, size_t name_len,
                           KaleidoscopeScriptValue *out);
static uint8_t script_invoke(ScriptMachine *m, uint8_t function,
                             const KaleidoscopeScriptValue *args, size_t count,
                             KaleidoscopeScriptValue *out);
static uint8_t script_argument_list(ScriptMachine *m, KaleidoscopeScriptValue *args, size_t *count);
static uint8_t script_apply(ScriptMachine *m, uint8_t op, KaleidoscopeScriptValue left,
                            KaleidoscopeScriptValue right, KaleidoscopeScriptValue *out);
static uint8_t script_instance_of(ScriptMachine *m, KaleidoscopeScriptValue value,
                                  const uint8_t *name, size_t name_len, bool *out);
static uint8_t script_define(ScriptMachine *m, const uint8_t *name, size_t name_len,
                             KaleidoscopeScriptPosition position, uint8_t *function);
static uint8_t script_assignment(ScriptMachine *m, bool run, KaleidoscopeScriptValue *result);
static uint8_t script_assigns_ahead_with(ScriptMachine *m, bool steps, bool *assigns);
static uint8_t script_arguments_array(ScriptMachine *m, KaleidoscopeScriptValue *out);
static uint8_t script_function_member(ScriptMachine *m, uint8_t function, const uint8_t *name,
                                      size_t name_len, KaleidoscopeScriptValue *out);
static uint8_t script_copy_props(ScriptMachine *m, uint8_t from, uint8_t to);
static uint8_t script_prototype(ScriptMachine *m, uint8_t function, uint8_t *out);
static uint8_t script_set_prototype(ScriptMachine *m, uint8_t function,
                                    KaleidoscopeScriptValue value);
static bool script_frozen(const ScriptMachine *m, uint8_t id);
static bool script_hidden_prop(const ScriptMachine *m, uint8_t id, size_t k);
static uint8_t script_pattern(ScriptMachine *m, bool bind, KaleidoscopeScriptValue source);
static bool script_is_collection(const ScriptMachine *m, KaleidoscopeScriptValue value);
static uint8_t script_collection_item(ScriptMachine *m, uint8_t id, size_t i, bool *present,
                                      KaleidoscopeScriptValue *out);
static uint8_t script_collection_new(ScriptMachine *m, bool map, KaleidoscopeScriptValue from,
                                     KaleidoscopeScriptValue *out);
static uint8_t script_collection_member(ScriptMachine *m, uint8_t id, const uint8_t *name,
                                        size_t name_len, KaleidoscopeScriptValue *out);
static uint8_t script_static_home(const ScriptMachine *m, size_t class_index, const uint8_t *key,
                                  size_t key_len);
static uint8_t script_eval_at(ScriptMachine *m, KaleidoscopeScriptPosition at,
                              KaleidoscopeScriptValue this_value, KaleidoscopeScriptValue *out);
static uint8_t script_escape(ScriptMachine *m, KaleidoscopeScriptValue value,
                             KaleidoscopeScriptValue *out);
static uint8_t script_real_func(const ScriptMachine *m, uint8_t function);
static uint8_t script_skim_body(ScriptMachine *m);
static uint8_t script_skip_params(ScriptMachine *m);
static uint8_t script_arrow_ahead(ScriptMachine *m, bool *arrow);
static uint8_t script_construct(ScriptMachine *m, const uint8_t *name, size_t name_len,
                                const KaleidoscopeScriptValue *args, size_t count,
                                KaleidoscopeScriptValue *out);
static uint8_t script_super_call(ScriptMachine *m, KaleidoscopeScriptValue *out);
static uint8_t script_date_method(ScriptMachine *m, double milliseconds, const uint8_t *name,
                                  size_t name_len, KaleidoscopeScriptValue *out);
static uint8_t script_json_parse(ScriptMachine *m, const uint8_t *text, size_t length, size_t *at,
                                 uint8_t depth, KaleidoscopeScriptValue *out);
static void script_json_ws(const uint8_t *text, size_t length, size_t *at);

static bool script_accessor_named(const ScriptMachine *m, KaleidoscopeScriptAccessor entry,
                                  uint8_t object, const uint8_t *key, size_t key_len) {
    if (entry.object != object || entry.name_len != key_len)
        return false;
    for (size_t k = 0; k < key_len; k++)
        if ((entry.name_in_source ? m->source->bytes[entry.name_start + k]
                                  : m->work->arena[entry.name_start + k]) != key[k])
            return false;
    return true;
}

static size_t script_find_accessor(const ScriptMachine *m, uint8_t object, const uint8_t *key,
                                   size_t key_len) {
    if (script_find_prop(m, object, key, key_len) != SIZE_MAX)
        return SIZE_MAX;
    for (size_t i = 0; i < m->work->accessor_count; i++)
        if (script_accessor_named(m, m->work->accessors[i], object, key, key_len))
            return i;
    return SIZE_MAX;
}

/* Adds a getter or setter for object.key; the key is a range of the source (in_source) or of
   the arena, which both outlive the script. */
static uint8_t script_add_accessor(ScriptMachine *m, uint8_t object, bool in_source,
                                   size_t key_start, size_t key_len, uint8_t function,
                                   bool setter) {
    const uint8_t *key = in_source ? m->source->bytes + key_start : m->work->arena + key_start;
    size_t slot = SIZE_MAX;
    for (size_t i = 0; i < m->work->accessor_count && slot == SIZE_MAX; i++)
        if (script_accessor_named(m, m->work->accessors[i], object, key, key_len))
            slot = i;
    if (slot == SIZE_MAX) {
        if (m->work->accessor_count >= 64)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        slot = m->work->accessor_count++;
        m->work->accessors[slot] = (KaleidoscopeScriptAccessor){.name_start = (uint16_t)key_start,
                                                                .name_len = (uint16_t)key_len,
                                                                .object = object,
                                                                .name_in_source = in_source};
    }
    if (setter) {
        m->work->accessors[slot].setter = function;
        m->work->accessors[slot].setter_present = true;
    } else {
        m->work->accessors[slot].getter = function;
        m->work->accessors[slot].getter_present = true;
    }
    return 0;
}

static uint8_t script_read_prop(ScriptMachine *m, uint8_t object, const uint8_t *key,
                                size_t key_len, KaleidoscopeScriptValue *out) {
    size_t slot = script_find_accessor(m, object, key, key_len);
    if (slot == SIZE_MAX) {
        *out = script_get_prop(m, object, key, key_len);
        return 0;
    }
    KaleidoscopeScriptAccessor entry = m->work->accessors[slot];
    if (!entry.getter_present || !m->work->live) {
        *out = script_undef();
        return 0;
    }
    m->work->this_next = script_object(object);
    m->work->this_next_present = true;
    return script_invoke(m, entry.getter, NULL, 0, out);
}

static uint8_t script_write_prop(ScriptMachine *m, uint8_t object, const uint8_t *key,
                                 size_t key_len, KaleidoscopeScriptValue value) {
    size_t slot = script_find_accessor(m, object, key, key_len);
    if (slot == SIZE_MAX)
        return script_set_prop(m, object, key, key_len, value);
    KaleidoscopeScriptAccessor entry = m->work->accessors[slot];
    if (!entry.setter_present)
        return 0;
    m->work->this_next = script_object(object);
    m->work->this_next_present = true;
    KaleidoscopeScriptValue ignored = {0};
    return script_invoke(m, entry.setter, &value, 1, &ignored);
}

/* `x.toFixed(d)` text as upstream's `fixed`: Rust's exact `{:.40}` of |x|, then rounded at d
   digits half away from zero on the decimal text. At most 64 bytes. */
/* `x.toFixed(d)` text as upstream's `fixed`: Rust's exact `{:.40}` of |x|, then rounded at d
   digits half away from zero on the decimal text. At most 64 bytes. */
static void script_fixed_text(ScriptNumberText *text, double number, size_t digits) {
    if (script_isnan(number) || script_isinf(number) || number >= 1e21 || number <= -1e21) {
        script_number_text(text, number);
        return;
    }
    ScriptNumberText wide = {0};
    uint8_t all[136] = {0};
    script_exact_fixed(&wide, number < 0.0 ? -number : number, 40);
    size_t dot = wide.n, length = 1;
    for (size_t i = 0; i < wide.n; i++)
        if (wide.b[i] == '.' && dot == wide.n)
            dot = i;
        else
            all[length++] = wide.b[i];
    /* all[0] is room for a carry digit; the digits are all[first..length]. */
    size_t first = 1, keep = 1 + dot + digits;
    bool up = keep < length && all[keep] >= '5';
    if (keep < length)
        length = keep;
    if (up)
        for (size_t i = length;;) {
            if (i == first) {
                all[--first] = '1';
                break;
            }
            i--;
            if (all[i] == '9')
                all[i] = '0';
            else {
                all[i]++;
                break;
            }
        }
    size_t int_len = length - first - digits;
    bool nonzero = false;
    for (size_t i = first; i < length; i++)
        nonzero |= all[i] != '0';
    if (number < 0.0 && nonzero)
        script_put(text, '-');
    for (size_t i = 0; i < int_len; i++)
        script_put(text, all[first + i]);
    if (digits) {
        script_put(text, '.');
        for (size_t i = first + int_len; i < length; i++)
            script_put(text, all[i]);
    }
}

static uint8_t script_fixed(ScriptMachine *m, double number, size_t digits,
                            KaleidoscopeScriptValue *out) {
    ScriptNumberText text = {0};
    script_fixed_text(&text, number, digits);
    return script_store(m, text.b, text.n, out);
}

/* `x.toPrecision(p)`, 1 <= p <= 21, as upstream's `precision`: p significant digits, in
   exponential form below 1e-6 or from 10^p up. */
static uint8_t script_precision(ScriptMachine *m, double number, size_t p,
                                KaleidoscopeScriptValue *out) {
    ScriptNumberText text = {0};
    if (script_isnan(number) || script_isinf(number))
        script_number_text(&text, number);
    else if (number == 0.0)
        script_fixed_text(&text, 0.0, p - 1);
    else {
        double a = number < 0.0 ? -number : number, mantissa = a;
        int32_t e = 0;
        while (mantissa >= 10.0) {
            mantissa /= 10.0;
            e++;
        }
        while (mantissa < 1.0) {
            mantissa *= 10.0;
            e--;
        }
        double scale = script_pow(10.0, (double)((int32_t)p - 1));
        if (script_floor(mantissa * scale + 0.5) >= 10.0 * scale) {
            e++;
            mantissa /= 10.0;
        }
        if (e < -6 || e >= (int32_t)p) {
            if (number < 0.0)
                script_put(&text, '-');
            script_fixed_text(&text, mantissa, p - 1);
            script_put(&text, 'e');
            script_put(&text, e < 0 ? '-' : '+');
            script_put_uint(&text, (uint64_t)(e < 0 ? -e : e));
        } else
            script_fixed_text(&text, number, (size_t)((int32_t)p - 1 - e));
    }
    return script_store(m, text.b, text.n, out);
}

/* `x.toLocaleString()` as upstream's `locale_number`: en-US grouping, at most three fraction
   digits. */
static uint8_t script_locale_number(ScriptMachine *m, double number, KaleidoscopeScriptValue *out) {
    ScriptNumberText text = {0}, grouped = {0};
    if (script_isnan(number)) {
        script_number_text(&text, number);
        return script_store(m, text.b, text.n, out);
    }
    if (script_isinf(number)) {
        if (number < 0.0)
            script_put(&text, '-');
        script_put(&text, 0xe2);
        script_put(&text, 0x88);
        script_put(&text, 0x9e);
        return script_store(m, text.b, text.n, out);
    }
    script_fixed_text(&text, number, 3);
    size_t n = text.n;
    while (n && text.b[n - 1] == '0')
        n--;
    if (n && text.b[n - 1] == '.')
        n--;
    size_t start = n && text.b[0] == '-' ? 1 : 0, int_len = n - start;
    for (size_t i = start; i < n; i++)
        if (text.b[i] == '.') {
            int_len = i - start;
            break;
        }
    if (start)
        script_put(&grouped, '-');
    for (size_t i = 0; i < n - start; i++) {
        if (i > 0 && i < int_len && (int_len - i) % 3 == 0)
            script_put(&grouped, ',');
        script_put(&grouped, text.b[start + i]);
    }
    if (grouped.n == 2 && grouped.b[0] == '-' && grouped.b[1] == '0') {
        grouped.b[0] = '0';
        grouped.n = 1;
    }
    return script_store(m, grouped.b, grouped.n, out);
}

static uint8_t script_number_radix(ScriptMachine *m, double number, double radix,
                                   KaleidoscopeScriptValue *out) {
    if (radix == 10.0) {
        uint8_t text[64] = {0};
        size_t n = script_format_number(number, text);
        return script_store(m, text, n, out);
    }
    if (radix < 2.0 || radix > 36.0 || radix != script_floor(radix) || script_isnan(number) ||
        script_isinf(number) || number != script_floor(number) || number >= 9007000000000000.0 ||
        number <= -9007000000000000.0)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    bool negative = number < 0.0;
    uint64_t value = (uint64_t)(negative ? -number : number), base = (uint64_t)radix;
    static const uint8_t alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    uint8_t reverse[64] = {0};
    size_t n = 0;
    do {
        reverse[n++] = alphabet[value % base];
        value /= base;
    } while (value);
    uint8_t text[64] = {0};
    size_t at = 0;
    if (negative)
        text[at++] = '-';
    while (n)
        text[at++] = reverse[--n];
    return script_store(m, text, at, out);
}

static bool script_word(const uint8_t *name, size_t length, const char *word) {
    size_t n = 0;
    while (word[n])
        n++;
    if (n != length)
        return false;
    for (size_t i = 0; i < n; i++)
        if (name[i] != (uint8_t)word[i])
            return false;
    return true;
}

static uint8_t script_regex_compile(ScriptMachine *m, uint8_t index) {
    KaleidoscopeScriptRegex entry = m->work->regexes[index];
    if (entry.pattern_len > KALEIDOSCOPE_REGEX_MAX_PATTERN || entry.flags_len > 8)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    KaleidoscopeRegexSource source = {0};
    source.pattern_len = entry.pattern_len;
    source.flags_len = entry.flags_len;
    for (size_t i = 0; i < entry.pattern_len; i++)
        source.pattern[i] = m->source->bytes[entry.pattern_start + i];
    for (size_t i = 0; i < entry.flags_len; i++)
        source.flags[i] = m->source->bytes[entry.flags_start + i];
    uint8_t error = kaleidoscope_regex_compile(&source, &m->work->regex_work);
    return error == KALEIDOSCOPE_REGEX_OK      ? 0
           : error == KALEIDOSCOPE_REGEX_LIMIT ? KALEIDOSCOPE_SCRIPT_LIMIT
                                               : KALEIDOSCOPE_SCRIPT_SYNTAX;
}

static uint8_t script_regex_find(ScriptMachine *m, uint8_t index, const uint8_t *text,
                                 size_t length, size_t from, KaleidoscopeRegexFind *result) {
    if (length > KALEIDOSCOPE_REGEX_MAX_TEXT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    uint8_t e = script_regex_compile(m, index);
    if (e)
        return e;
    m->work->regex_text.len = length;
    for (size_t i = 0; i < length; i++)
        m->work->regex_text.bytes[i] = text[i];
    *result = kaleidoscope_regex_find(&m->work->regex_work, &m->work->regex_text, from);
    return result->error == KALEIDOSCOPE_REGEX_OK      ? 0
           : result->error == KALEIDOSCOPE_REGEX_LIMIT ? KALEIDOSCOPE_SCRIPT_LIMIT
                                                       : KALEIDOSCOPE_SCRIPT_SYNTAX;
}

static bool script_regex_global(const ScriptMachine *m, uint8_t index) {
    KaleidoscopeScriptRegex entry = m->work->regexes[index];
    for (size_t i = 0; i < entry.flags_len; i++)
        if (m->source->bytes[entry.flags_start + i] == 'g')
            return true;
    return false;
}

static uint8_t script_region_append(ScriptMachine *m, size_t first, size_t *count,
                                    const uint8_t *bytes, size_t length) {
    if (first + *count + length > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t i = 0; i < length; i++)
        m->work->arena[first + *count + i] = bytes[i];
    *count += length;
    m->work->arena_len = first + *count;
    return 0;
}

static uint8_t script_capture_value(ScriptMachine *m, const uint8_t *text,
                                    const KaleidoscopeRegexCapture *capture,
                                    KaleidoscopeScriptValue *out) {
    if (!capture->present) {
        *out = script_undef();
        return 0;
    }
    return script_store(m, text + capture->start, capture->end - capture->start, out);
}

static uint8_t script_arguments(ScriptMachine *m, KaleidoscopeScriptValue *args, size_t *count);

/* `re.exec(text)`: the match and its groups, or null; with `g` it starts at `lastIndex` and
   moves it past the match (back to 0 when nothing more matches). */
static uint8_t script_regex_exec(ScriptMachine *m, uint8_t regex, KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_arguments(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    KaleidoscopeScriptValue text = {0};
    e = script_to_text(m, args[0], &text);
    if (e)
        return e;
    ScriptSlice bytes = script_text(text);
    bool global = script_regex_global(m, regex);
    size_t from = global ? m->work->regexes[regex].last_index : 0;
    KaleidoscopeRegexFind found = {0};
    if (from <= bytes.len) {
        e = script_regex_find(m, regex, m->work->arena + bytes.start, bytes.len, from, &found);
        if (e)
            return e;
    }
    if (from > bytes.len || !found.matched) {
        m->work->regexes[regex].last_index = 0;
        *out = script_null();
        return 0;
    }
    if (global)
        m->work->regexes[regex].last_index = found.captures[0].present ? found.captures[0].end : 0;
    size_t groups = found.groups + 1;
    if (groups > KALEIDOSCOPE_REGEX_CAPTURES)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    /* The pieces are stored first, then the array, as upstream. */
    KaleidoscopeScriptValue pieces[KALEIDOSCOPE_REGEX_CAPTURES] = {0};
    for (size_t g = 0; g < groups; g++) {
        e = script_capture_value(m, m->work->arena + bytes.start, &found.captures[g], &pieces[g]);
        if (e)
            return e;
    }
    return script_new_array(m, pieces, groups, out);
}

static uint8_t script_string_regex(ScriptMachine *m, const uint8_t *name, size_t name_len,
                                   KaleidoscopeScriptValue string, uint8_t regex,
                                   KaleidoscopeScriptValue argument, KaleidoscopeScriptValue *out) {
    ScriptSlice source = script_text(string);
    const uint8_t *text = m->work->arena + source.start;
    bool global = script_regex_global(m, regex), is_match = script_word(name, name_len, "match"),
         is_search = script_word(name, name_len, "search"),
         is_split = script_word(name, name_len, "split"),
         is_replace = script_word(name, name_len, "replace"),
         is_replace_all = script_word(name, name_len, "replaceAll");
    KaleidoscopeRegexFind first_find = {0};
    uint8_t e = script_regex_find(m, regex, text, source.len, 0, &first_find);
    if (e)
        return e;
    if (script_word(name, name_len, "matchAll")) {
        /* Every match with its groups (an array, not an iterator). */
        if (!global)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        size_t top = m->work->argument_top, matches = 0, from = 0;
        while (!e && from <= source.len) {
            KaleidoscopeRegexFind found = {0};
            e = script_regex_find(m, regex, text, source.len, from, &found);
            if (e || !found.matched)
                break;
            if (top + matches == KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK) {
                e = KALEIDOSCOPE_SCRIPT_LIMIT;
                break;
            }
            size_t count = found.groups + 1;
            uint8_t id = 0;
            e = count > KALEIDOSCOPE_REGEX_CAPTURES ? KALEIDOSCOPE_SCRIPT_LIMIT
                                                    : script_allocate_array(m, count, &id);
            for (size_t i = 0; i < count && !e; i++) {
                KaleidoscopeScriptValue piece = {0};
                e = script_capture_value(m, text, &found.captures[i], &piece);
                if (!e)
                    m->work->elems[m->work->arrays[id].start + i] = piece;
            }
            m->work->argument_stack[top + matches++] = script_array(id);
            m->work->argument_top = top + matches;
            size_t a = found.captures[0].start, b = found.captures[0].end;
            from = b == a ? b + 1 : b;
            if (matches > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
                break;
        }
        if (!e)
            e = script_new_array(m, m->work->argument_stack + top, matches, out);
        m->work->argument_top = top;
        return e;
    }
    if (is_search) {
        *out = script_num(first_find.matched ? (double)first_find.captures[0].start : -1.0);
        return 0;
    }
    if (is_match) {
        if (!first_find.matched) {
            *out = script_null();
            return 0;
        }
        if (!global) {
            size_t count = first_find.groups + 1;
            if (count > KALEIDOSCOPE_REGEX_CAPTURES)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            uint8_t id = 0;
            e = script_allocate_array(m, count, &id);
            if (e)
                return e;
            KaleidoscopeScriptArray array = m->work->arrays[id];
            for (size_t i = 0; i < count; i++) {
                e = script_capture_value(m, text, &first_find.captures[i],
                                         &m->work->elems[array.start + i]);
                if (e)
                    return e;
            }
            *out = script_array(id);
            return 0;
        }
        size_t matches = 0, from = 0;
        for (;;) {
            KaleidoscopeRegexFind found = {0};
            e = script_regex_find(m, regex, text, source.len, from, &found);
            if (e)
                return e;
            if (!found.matched)
                break;
            matches++;
            size_t a = found.captures[0].start, b = found.captures[0].end;
            from = b == a ? b + 1 : b;
            if (from > source.len)
                break;
        }
        uint8_t id = 0;
        e = script_allocate_array(m, matches, &id);
        if (e)
            return e;
        KaleidoscopeScriptArray array = m->work->arrays[id];
        from = 0;
        for (size_t i = 0; i < matches; i++) {
            KaleidoscopeRegexFind found = {0};
            e = script_regex_find(m, regex, text, source.len, from, &found);
            if (e)
                return e;
            e = script_capture_value(m, text, &found.captures[0], &m->work->elems[array.start + i]);
            if (e)
                return e;
            size_t a = found.captures[0].start, b = found.captures[0].end;
            from = b == a ? b + 1 : b;
        }
        *out = script_array(id);
        return 0;
    }
    if (is_split) {
        size_t pieces = 1, from = 0;
        for (;;) {
            KaleidoscopeRegexFind found = {0};
            e = script_regex_find(m, regex, text, source.len, from, &found);
            if (e)
                return e;
            if (!found.matched)
                break;
            size_t a = found.captures[0].start, b = found.captures[0].end;
            if (!(a == b && (a == 0 || a == source.len)))
                pieces += 1 + found.groups;
            from = b == a ? b + 1 : b;
            if (from > source.len)
                break;
            if (pieces > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
        }
        uint8_t id = 0;
        e = script_allocate_array(m, pieces, &id);
        if (e)
            return e;
        KaleidoscopeScriptArray array = m->work->arrays[id];
        size_t slot = 0, last = 0;
        from = 0;
        for (;;) {
            KaleidoscopeRegexFind found = {0};
            e = script_regex_find(m, regex, text, source.len, from, &found);
            if (e)
                return e;
            if (!found.matched)
                break;
            size_t a = found.captures[0].start, b = found.captures[0].end;
            if (!(a == b && (a == 0 || a == source.len))) {
                e = script_store(m, text + last, a - last, &m->work->elems[array.start + slot++]);
                if (e)
                    return e;
                for (size_t g = 1; g <= found.groups; g++) {
                    e = script_capture_value(m, text, &found.captures[g],
                                             &m->work->elems[array.start + slot++]);
                    if (e)
                        return e;
                }
                last = b;
            }
            from = b == a ? b + 1 : b;
            if (from > source.len)
                break;
        }
        e = script_store(m, text + last, source.len - last, &m->work->elems[array.start + slot++]);
        if (e)
            return e;
        size_t limit = slot;
        if (argument.kind != KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED) {
            double n = script_to_number(m, argument);
            limit = script_isnan(n) || n <= 0.0 ? 0
                    : n >= 4294967295.0         ? UINT32_MAX
                                                : (size_t)(uint32_t)n;
        }
        m->work->arrays[id].len = (uint16_t)(slot < limit ? slot : limit);
        *out = script_array(id);
        return 0;
    }
    if (!(is_replace || is_replace_all)) {
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    }
    if (is_replace_all && !global) {
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    }
    KaleidoscopeScriptValue replacement = argument;
    ScriptSlice rep = {0};
    if (argument.kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION) {
        e = script_to_text(m, argument, &replacement);
        if (e)
            return e;
        rep = script_text(replacement);
    }
    size_t first = m->work->arena_len, count = 0, last = 0, from = 0;
    for (;;) {
        KaleidoscopeRegexFind found = {0};
        e = script_regex_find(m, regex, text, source.len, from, &found);
        if (e)
            return e;
        if (!found.matched)
            break;
        size_t a = found.captures[0].start, b = found.captures[0].end;
        e = script_region_append(m, first, &count, text + last, a - last);
        if (e)
            return e;
        if (argument.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION) {
            KaleidoscopeScriptValue args[4] = {script_undef(), script_undef(), script_undef(),
                                               script_undef()},
                                    value;
            size_t n = found.groups < 3 ? found.groups + 1 : 4;
            for (size_t g = 0; g < n; g++) {
                e = script_capture_value(m, text, &found.captures[g], &args[g]);
                if (e)
                    return e;
            }
            e = script_invoke(m, argument.index, args, n, &value);
            if (e)
                return e;
            e = script_to_text(m, value, &value);
            if (e)
                return e;
            ScriptSlice rendered = script_text(value);
            e = script_region_append(m, first, &count, m->work->arena + rendered.start,
                                     rendered.len);
            if (e)
                return e;
        } else {
            for (size_t i = 0; i < rep.len;) {
                uint8_t byte = m->work->arena[rep.start + i];
                if (byte == '$' && i + 1 < rep.len) {
                    uint8_t next = m->work->arena[rep.start + i + 1];
                    if (next == '$') {
                        e = script_region_append(m, first, &count, (const uint8_t *)"$", 1);
                        i += 2;
                        continue;
                    }
                    if (next == '&') {
                        e = script_region_append(m, first, &count, text + a, b - a);
                        i += 2;
                        continue;
                    }
                    if (next >= '1' && next <= '9' && (size_t)(next - '0') <= found.groups) {
                        KaleidoscopeRegexCapture capture = found.captures[next - '0'];
                        if (capture.present)
                            e = script_region_append(m, first, &count, text + capture.start,
                                                     capture.end - capture.start);
                        i += 2;
                        continue;
                    }
                }
                e = script_region_append(m, first, &count, &m->work->arena[rep.start + i], 1);
                i++;
                if (e)
                    return e;
            }
        }
        last = b;
        if (e)
            return e;
        if (!global)
            break;
        from = b == a ? b + 1 : b;
        if (from > source.len)
            break;
    }
    e = script_region_append(m, first, &count, text + last, source.len - last);
    if (e)
        return e;
    *out = script_str(first, count);
    return 0;
}

static uint8_t script_arguments(ScriptMachine *m, KaleidoscopeScriptValue *args, size_t *count) {
    return script_argument_list(m, args, count);
}

static size_t script_array_position(const ScriptMachine *m, KaleidoscopeScriptValue value,
                                    size_t fallback, size_t length) {
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED)
        return fallback;
    double x = script_to_number(m, value);
    x = script_isnan(x) ? 0.0 : script_floor(x);
    if (x < 0.0)
        x = (double)length + x;
    if (x < 0.0)
        return 0;
    if (x > (double)length)
        return length;
    return (size_t)x;
}

static uint8_t script_array_method(ScriptMachine *m, uint8_t id, const uint8_t *name,
                                   size_t name_len, const KaleidoscopeScriptValue *args,
                                   size_t count, KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptArray snapshot = m->work->arrays[id];
    size_t start = snapshot.start, len = snapshot.len;
    KaleidoscopeScriptValue first = count ? args[0] : script_undef();
    if (script_word(name, name_len, "lastIndexOf")) {
        double found = -1.0;
        for (size_t i = len; i > 0; i--) {
            KaleidoscopeScriptValue equal = {0};
            uint8_t e = script_apply(m, 'E', m->work->elems[start + i - 1], first, &equal);
            if (e)
                return e;
            if (equal.boolean) {
                found = (double)(i - 1);
                break;
            }
        }
        *out = script_num(found);
        return 0;
    }
    if (script_word(name, name_len, "toString")) {
        static const uint8_t comma = ',';
        return script_join(m, id, &comma, 1, out);
    }
    /* Copying forms: the array itself is left unchanged. */
    if (script_word(name, name_len, "toSorted") || script_word(name, name_len, "toReversed") ||
        script_word(name, name_len, "toSpliced") || script_word(name, name_len, "with")) {
        KaleidoscopeScriptValue copy = {0};
        uint8_t e = script_new_array(m, m->work->elems + start, len, &copy);
        if (e)
            return e;
        KaleidoscopeScriptValue ignored = {0};
        static const uint8_t sort_word[] = "sort", reverse_word[] = "reverse",
                             splice_word[] = "splice";
        if (script_word(name, name_len, "toSorted"))
            e = script_array_method(m, copy.index, sort_word, 4, args, count, &ignored);
        else if (script_word(name, name_len, "toReversed"))
            e = script_array_method(m, copy.index, reverse_word, 7, args, 0, &ignored);
        else if (script_word(name, name_len, "toSpliced"))
            e = script_array_method(m, copy.index, splice_word, 6, args, count, &ignored);
        else {
            double i = script_to_number(m, first);
            i = i < 0.0 ? (double)len + script_floor(i) : script_floor(i);
            if (!(i >= 0.0 && (size_t)i < len))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            m->work->elems[m->work->arrays[copy.index].start + (size_t)i] =
                count > 1 ? args[1] : script_undef();
        }
        if (e)
            return e;
        *out = copy;
        return 0;
    }
    if (script_word(name, name_len, "keys") || script_word(name, name_len, "values") ||
        script_word(name, name_len, "entries")) {
        bool keys = script_word(name, name_len, "keys"),
             values = script_word(name, name_len, "values");
        if (values)
            return script_new_array(m, m->work->elems + start, len, out);
        uint8_t result = 0;
        uint8_t e = script_allocate_array(m, len, &result);
        for (size_t k = 0; !e && k < len; k++) {
            KaleidoscopeScriptValue item = script_num((double)k);
            if (!keys) {
                KaleidoscopeScriptValue pair[2] = {item, m->work->elems[start + k]};
                e = script_new_array(m, pair, 2, &item);
            }
            if (!e)
                m->work->elems[m->work->arrays[result].start + k] = item;
        }
        if (e)
            return e;
        *out = script_array(result);
        return 0;
    }
    if (script_word(name, name_len, "splice") || script_word(name, name_len, "unshift")) {
        bool unshift = script_word(name, name_len, "unshift");
        size_t at = unshift ? 0 : script_array_position(m, first, len, len), removed = 0,
               insert_start = unshift ? 0 : 2;
        if (!unshift) {
            if (count < 2)
                removed = len - at;
            else {
                double n = script_to_number(m, args[1]);
                removed = script_isnan(n) || n < 0.0 ? 0 : (size_t)script_floor(n);
                if (removed > len - at)
                    removed = len - at;
            }
        }
        size_t inserted = count > insert_start ? count - insert_start : 0,
               new_count = at + inserted + (len - at - removed);
        if (new_count > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        KaleidoscopeScriptValue items[KALEIDOSCOPE_SCRIPT_ELEMS_STORAGE] = {0};
        for (size_t i = 0; i < at; i++)
            items[i] = m->work->elems[start + i];
        for (size_t i = 0; i < inserted; i++)
            items[at + i] = args[insert_start + i];
        for (size_t i = at + removed; i < len; i++)
            items[at + inserted + i - at - removed] = m->work->elems[start + i];
        if (!m->work->live) {
            *out = unshift ? script_num((double)len) : script_undef();
            return 0;
        }
        KaleidoscopeScriptValue removed_result = script_undef();
        uint8_t e = 0;
        if (!unshift) {
            e = script_new_array(m, m->work->elems + start + at, removed, &removed_result);
            if (e)
                return e;
        }
        e = script_set_items(m, id, items, new_count);
        if (e)
            return e;
        if (unshift) {
            *out = script_num((double)new_count);
            return 0;
        }
        *out = removed_result;
        return 0;
    }
    if (script_word(name, name_len, "indexOf") || script_word(name, name_len, "includes")) {
        size_t found = SIZE_MAX;
        for (size_t i = 0; i < len; i++) {
            KaleidoscopeScriptValue equal = {0};
            uint8_t e = script_apply(m, 'E', m->work->elems[start + i], first, &equal);
            if (e)
                return e;
            if (equal.boolean) {
                found = i;
                break;
            }
        }
        *out = script_word(name, name_len, "includes")
                   ? script_bool(found != SIZE_MAX)
                   : script_num(found == SIZE_MAX ? -1.0 : (double)found);
        return 0;
    }
    if (script_word(name, name_len, "slice")) {
        size_t from = script_array_position(m, first, 0, len),
               to = script_array_position(m, count > 1 ? args[1] : script_undef(), len, len);
        size_t n = from < to ? to - from : 0;
        uint8_t result = 0;
        uint8_t e = script_allocate_array(m, n, &result);
        if (e)
            return e;
        KaleidoscopeScriptArray target = m->work->arrays[result];
        for (size_t i = 0; i < n; i++)
            m->work->elems[target.start + i] = m->work->elems[start + from + i];
        *out = script_array(result);
        return 0;
    }
    if (script_word(name, name_len, "concat")) {
        size_t total = len;
        for (size_t i = 0; i < count; i++)
            total += args[i].kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY
                         ? m->work->arrays[args[i].index].len
                         : 1;
        if (total > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        uint8_t result = 0;
        uint8_t e = script_allocate_array(m, total, &result);
        if (e)
            return e;
        KaleidoscopeScriptArray target = m->work->arrays[result];
        size_t at = 0;
        for (size_t i = 0; i < len; i++)
            m->work->elems[target.start + at++] = m->work->elems[start + i];
        for (size_t i = 0; i < count; i++) {
            if (args[i].kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                KaleidoscopeScriptArray other = m->work->arrays[args[i].index];
                for (size_t k = 0; k < other.len; k++)
                    m->work->elems[target.start + at++] = m->work->elems[other.start + k];
            } else
                m->work->elems[target.start + at++] = args[i];
        }
        *out = script_array(result);
        return 0;
    }
    if (script_word(name, name_len, "reverse")) {
        if (m->work->live)
            for (size_t i = 0; i < len / 2; i++) {
                KaleidoscopeScriptValue swap = m->work->elems[start + i];
                m->work->elems[start + i] = m->work->elems[start + len - 1 - i];
                m->work->elems[start + len - 1 - i] = swap;
            }
        *out = script_array(id);
        return 0;
    }
    if (script_word(name, name_len, "fill")) {
        size_t from = script_array_position(m, count > 1 ? args[1] : script_undef(), 0, len),
               to = script_array_position(m, count > 2 ? args[2] : script_undef(), len, len);
        if (m->work->live)
            for (size_t i = from; i < (to > from ? to : from); i++)
                m->work->elems[start + i] = first;
        *out = script_array(id);
        return 0;
    }
    if (script_word(name, name_len, "at")) {
        double index = script_to_number(m, first);
        index = index < 0.0 ? (double)len + script_floor(index) : script_floor(index);
        *out = index >= 0.0 && (size_t)index < len ? m->work->elems[start + (size_t)index]
                                                   : script_undef();
        return 0;
    }
    if (script_word(name, name_len, "flat")) {
        size_t total = 0;
        for (size_t i = 0; i < len; i++)
            total += m->work->elems[start + i].kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY
                         ? m->work->arrays[m->work->elems[start + i].index].len
                         : 1;
        if (total > KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        uint8_t result = 0;
        uint8_t e = script_allocate_array(m, total, &result);
        if (e)
            return e;
        KaleidoscopeScriptArray target = m->work->arrays[result];
        size_t at = 0;
        for (size_t i = 0; i < len; i++) {
            KaleidoscopeScriptValue value = m->work->elems[start + i];
            if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                KaleidoscopeScriptArray inner = m->work->arrays[value.index];
                for (size_t k = 0; k < inner.len; k++)
                    m->work->elems[target.start + at++] = m->work->elems[inner.start + k];
            } else
                m->work->elems[target.start + at++] = value;
        }
        *out = script_array(result);
        return 0;
    }
    if (script_word(name, name_len, "sort")) {
        bool comparator = count && args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION;
        if (count && !comparator && m->work->live)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        if (m->work->live)
            for (size_t i = 1; i < len; i++) {
                size_t j = i;
                while (j) {
                    bool greater = false;
                    if (comparator) {
                        KaleidoscopeScriptValue pair[2] = {m->work->elems[start + j - 1],
                                                           m->work->elems[start + j]},
                                                result;
                        uint8_t e = script_invoke(m, args[0].index, pair, 2, &result);
                        if (e)
                            return e;
                        greater = script_to_number(m, result) > 0.0;
                    } else {
                        KaleidoscopeScriptValue left = m->work->elems[start + j - 1],
                                                right = m->work->elems[start + j];
                        if (left.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED)
                            greater = right.kind != KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED;
                        else if (right.kind != KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED) {
                            uint8_t e = script_to_text(m, left, &left);
                            if (e)
                                return e;
                            e = script_to_text(m, right, &right);
                            if (e)
                                return e;
                            greater = script_text_compare(m, left, right) > 0;
                        }
                    }
                    if (!greater)
                        break;
                    KaleidoscopeScriptValue swap = m->work->elems[start + j - 1];
                    m->work->elems[start + j - 1] = m->work->elems[start + j];
                    m->work->elems[start + j] = swap;
                    j--;
                }
            }
        *out = script_array(id);
        return 0;
    }
    if (script_word(name, name_len, "pop") || script_word(name, name_len, "shift")) {
        if (!len) {
            *out = script_undef();
            return 0;
        }
        bool shift = script_word(name, name_len, "shift");
        *out = m->work->elems[start + (shift ? 0 : len - 1)];
        if (m->work->live) {
            if (shift)
                for (size_t i = 1; i < len; i++)
                    m->work->elems[start + i - 1] = m->work->elems[start + i];
            m->work->arrays[id].len--;
        }
        return 0;
    }
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

static uint8_t script_array_callback(ScriptMachine *m, uint8_t id, const uint8_t *name,
                                     size_t name_len, const KaleidoscopeScriptValue *args,
                                     size_t count, KaleidoscopeScriptValue *out) {
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    if (!count || args[0].kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION) {
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    }
    size_t initial = m->work->arrays[id].len;
    if (script_word(name, name_len, "reduceRight")) {
        bool have = count > 1;
        KaleidoscopeScriptValue acc = have ? args[1] : script_undef();
        for (size_t k = initial; k-- > 0;) {
            KaleidoscopeScriptArray now = m->work->arrays[id];
            if (k >= now.len)
                continue;
            KaleidoscopeScriptValue item = m->work->elems[now.start + k];
            if (++m->work->steps > 50000)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            if (!have) {
                acc = item;
                have = true;
                continue;
            }
            KaleidoscopeScriptValue call[4] = {acc, item, script_num((double)k), script_array(id)};
            uint8_t e = script_invoke(m, args[0].index, call, 4, &acc);
            if (e)
                return e;
        }
        if (!have)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        *out = acc;
        return 0;
    }
    bool reduce = script_word(name, name_len, "reduce"), map = script_word(name, name_len, "map"),
         filter = script_word(name, name_len, "filter"),
         flat_map = script_word(name, name_len, "flatMap"),
         find_last = script_word(name, name_len, "findLast"),
         find_last_index = script_word(name, name_len, "findLastIndex");
    KaleidoscopeScriptValue accumulator = count > 1 ? args[1] : script_undef();
    bool have_accumulator = count > 1;
    uint8_t result_id = 0;
    if (map || filter || flat_map) {
        uint8_t e = script_allocate_array(m, initial, &result_id);
        if (e)
            return e;
        m->work->arrays[result_id].len = 0;
    }
    if (find_last || find_last_index) {
        for (size_t next = initial; next > 0; next--) {
            size_t k = next - 1;
            KaleidoscopeScriptArray current = m->work->arrays[id];
            KaleidoscopeScriptValue element =
                k < current.len ? m->work->elems[current.start + k] : script_undef();
            m->work->steps++;
            if (m->work->steps > 50000)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            KaleidoscopeScriptValue call[3] = {element, script_num((double)k), script_array(id)},
                                    value;
            uint8_t e = script_invoke(m, args[0].index, call, 3, &value);
            if (e)
                return e;
            if (script_truthy(value)) {
                *out = find_last ? element : script_num((double)k);
                return 0;
            }
        }
        *out = find_last ? script_undef() : script_num(-1.0);
        return 0;
    }
    for (size_t k = 0; k < initial; k++) {
        KaleidoscopeScriptArray current = m->work->arrays[id];
        if (k >= current.len)
            break;
        KaleidoscopeScriptValue element = m->work->elems[current.start + k];
        m->work->steps++;
        if (m->work->steps > 50000)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        if (reduce) {
            if (!have_accumulator) {
                accumulator = element;
                have_accumulator = true;
                continue;
            }
            KaleidoscopeScriptValue call[4] = {accumulator, element, script_num((double)k),
                                               script_array(id)};
            uint8_t e = script_invoke(m, args[0].index, call, 4, &accumulator);
            if (e)
                return e;
            continue;
        }
        KaleidoscopeScriptValue call[3] = {element, script_num((double)k), script_array(id)}, value;
        uint8_t e = script_invoke(m, args[0].index, call, 3, &value);
        if (e)
            return e;
        bool yes = script_truthy(value);
        if (map || filter || flat_map) {
            if (flat_map && value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                KaleidoscopeScriptArray inner = m->work->arrays[value.index];
                for (size_t i = 0; i < inner.len; i++) {
                    e = script_push(m, result_id, m->work->elems[inner.start + i]);
                    if (e)
                        return e;
                }
            } else if (map || flat_map || yes) {
                e = script_push(m, result_id, map || flat_map ? value : element);
                if (e)
                    return e;
            }
        } else if (script_word(name, name_len, "find") && yes) {
            *out = element;
            return 0;
        } else if (script_word(name, name_len, "findIndex") && yes) {
            *out = script_num((double)k);
            return 0;
        } else if (script_word(name, name_len, "some") && yes) {
            *out = script_bool(true);
            return 0;
        } else if (script_word(name, name_len, "every") && !yes) {
            *out = script_bool(false);
            return 0;
        }
    }
    if (map || filter || flat_map)
        *out = script_array(result_id);
    else if (script_word(name, name_len, "find"))
        *out = script_undef();
    else if (script_word(name, name_len, "findIndex"))
        *out = script_num(-1);
    else if (script_word(name, name_len, "some"))
        *out = script_bool(false);
    else if (script_word(name, name_len, "every"))
        *out = script_bool(true);
    else if (reduce) {
        if (!have_accumulator)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        *out = accumulator;
    } else
        *out = script_undef();
    return 0;
}

static uint8_t script_object_literal(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    uint8_t e = script_next(m), id;
    if (e)
        return e;
    e = script_new_object(m, &id);
    if (e)
        return e;
    while (!script_punct(m, '}')) {
        if (script_punct(m, 'S')) {
            e = script_next(m);
            if (e)
                return e;
            KaleidoscopeScriptValue source = {0};
            e = script_expr(m, &source);
            if (e)
                return e;
            if (source.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT && m->work->live) {
                KaleidoscopeScriptArray record = m->work->objects[source.index];
                for (size_t i = 0; i < record.len; i++) {
                    KaleidoscopeScriptProperty property = m->work->props[record.start + i];
                    e = script_set_prop(m, id, m->work->arena + property.start, property.len,
                                        property.value);
                    if (e)
                        return e;
                }
            } else if (source.kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
                       source.kind != KALEIDOSCOPE_SCRIPT_VALUE_NULL &&
                       source.kind != KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED && m->work->live)
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        } else {
            const uint8_t *key = NULL;
            size_t key_len = 0, name_start = 0, name_len = 0;
            bool shorthand = false;
            KaleidoscopeScriptValue key_value = script_undef();
            if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
                key = m->source->bytes + m->token.start;
                key_len = m->token.len;
                name_start = m->token.start;
                name_len = m->token.len;
                shorthand = true;
                e = script_next(m);
            } else if (m->token.kind == KALEIDOSCOPE_TOKEN_STRING) {
                key = m->source->bytes + m->token.start;
                key_len = m->token.len;
                e = script_next(m);
            } else if (m->token.kind == KALEIDOSCOPE_TOKEN_NUMBER) {
                key_value = script_num(m->token.number);
                e = script_next(m);
                if (!e)
                    e = script_to_text(m, key_value, &key_value);
                if (!e) {
                    ScriptSlice text = script_text(key_value);
                    key = m->work->arena + text.start;
                    key_len = text.len;
                }
            } else if (script_punct(m, '[')) {
                e = script_next(m);
                if (!e)
                    e = script_expr(m, &key_value);
                if (!e)
                    e = script_expect(m, ']');
                if (!e)
                    e = script_to_text(m, key_value, &key_value);
                if (!e) {
                    ScriptSlice text = script_text(key_value);
                    key = m->work->arena + text.start;
                    key_len = text.len;
                }
            } else
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            if (e)
                return e;
            if (shorthand &&
                (script_word(key, key_len, "get") || script_word(key, key_len, "set")) &&
                (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT ||
                 m->token.kind == KALEIDOSCOPE_TOKEN_STRING)) {
                bool setter = script_word(key, key_len, "set");
                const uint8_t *access_key = m->source->bytes + m->token.start;
                size_t access_len = m->token.len;
                e = script_next(m);
                if (e)
                    return e;
                KaleidoscopeScriptPosition at = script_position_save(m);
                e = script_expect(m, '(');
                if (e)
                    return e;
                e = script_skip_params(m);
                if (e)
                    return e;
                e = script_expect(m, ')');
                if (e)
                    return e;
                if (!script_punct(m, '{'))
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
                e = script_skim_body(m);
                if (e)
                    return e;
                uint8_t function = 0;
                e = script_define(m, NULL, 0, at, &function);
                if (e)
                    return e;
                e = script_add_accessor(m, id, true, (size_t)(access_key - m->source->bytes),
                                        access_len, function, setter);
                if (e)
                    return e;
                if (script_punct(m, ',')) {
                    e = script_next(m);
                    if (e)
                        return e;
                }
                continue;
            }
            KaleidoscopeScriptValue value = {0};
            if (script_punct(m, '(')) {
                KaleidoscopeScriptPosition at = script_position_save(m);
                e = script_next(m);
                if (e)
                    return e;
                e = script_skip_params(m);
                if (e)
                    return e;
                e = script_expect(m, ')');
                if (e)
                    return e;
                if (!script_punct(m, '{'))
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
                e = script_skim_body(m);
                if (e)
                    return e;
                uint8_t function = 0;
                e = script_define(m, NULL, 0, at, &function);
                if (e)
                    return e;
                value = script_function(function);
            } else if (shorthand && (script_punct(m, ',') || script_punct(m, '}'))) {
                KaleidoscopeScriptVariable *variable = script_find(m, name_start, name_len);
                if (variable)
                    value = variable->value;
                else {
                    size_t function = SIZE_MAX;
                    for (size_t i = 0; i < m->work->nfuncs; i++)
                        if (m->work->funcs[i].len &&
                            script_function_name(m, &m->work->funcs[i], name_start, name_len)) {
                            function = i;
                            break;
                        }
                    if (function != SIZE_MAX)
                        value = script_function((uint8_t)function);
                    else if (m->work->skim || !m->work->live)
                        value = script_undef();
                    else
                        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                }
            } else {
                e = script_expect(m, ':');
                if (e)
                    return e;
                e = script_expr(m, &value);
                if (e)
                    return e;
            }
            e = script_set_prop(m, id, key, key_len, value);
            if (e)
                return e;
        }
        if (!script_punct(m, ',')) {
            break;
        }
        e = script_next(m);
        if (e)
            return e;
    }
    e = script_expect(m, '}');
    if (e)
        return e;
    *out = script_object(id);
    return 0;
}

static uint8_t script_emit(ScriptMachine *m, KaleidoscopeScriptValue value);

/* `console.log(...)` and its kin: the arguments are evaluated; nothing is printed. */
static uint8_t script_console_call(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    uint8_t e = script_next(m);
    if (e)
        return e;
    if (!(script_ident(m, "log") || script_ident(m, "info") || script_ident(m, "warn") ||
          script_ident(m, "error") || script_ident(m, "debug")))
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    e = script_next(m);
    if (!e)
        e = script_expect(m, '(');
    while (!e && !script_punct(m, ')')) {
        if (script_punct(m, 'S'))
            e = script_next(m);
        KaleidoscopeScriptValue ignored = {0};
        if (!e)
            e = script_expr(m, &ignored);
        if (e || !script_punct(m, ','))
            break;
        e = script_next(m);
    }
    if (!e)
        e = script_expect(m, ')');
    *out = script_undef();
    return e;
}

/* `Array(...)`, `Error(...)` and its kin called without `new`. */
static uint8_t script_construct_call(ScriptMachine *m, const uint8_t *name, size_t name_len,
                                     KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_argument_list(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    return script_construct(m, name, name_len, args, count, out);
}

/* `document.write(...)` / `writeln` in an expression. */
static uint8_t script_write_expression(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    *out = script_undef();
    uint8_t e = script_next(m);
    if (e)
        return e;
    bool line = script_ident(m, "writeln");
    if (!line && !script_ident(m, "write"))
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    e = script_next(m);
    if (e)
        return e;
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    e = script_argument_list(m, args, &count);
    if (e || !m->work->live)
        return e;
    for (size_t i = 0; i < count && !e; i++)
        e = script_emit(m, args[i]);
    if (!e && line) {
        static const uint8_t nl = '\n';
        KaleidoscopeScriptValue value = {0};
        e = script_store(m, &nl, 1, &value);
        if (!e)
            e = script_emit(m, value);
    }
    return e;
}

static uint8_t script_from_with_map(ScriptMachine *m, bool *second);

/* The items `for...of`, spread and the Array/Object statics walk, as upstream's `iterate`:
   an array's items are copied onto the argument stack (callbacks may change the array); a
   string's characters are read in place. The caller restores argument_top afterwards. */
typedef struct {
    bool text;
    size_t start, count;
} ScriptItems;

static uint8_t script_iterate(ScriptMachine *m, KaleidoscopeScriptValue value, ScriptItems *items) {
    items->text = false;
    items->start = m->work->argument_top;
    items->count = 0;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
        items->text = true;
        items->start = value.start;
        items->count = value.len;
        return 0;
    }
    if (script_is_collection(m, value)) {
        for (bool present = true; present;) {
            KaleidoscopeScriptValue item = {0};
            uint8_t e = script_collection_item(m, value.index, items->count, &present, &item);
            if (e)
                return e;
            if (!present)
                break;
            if (items->start + items->count == KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            m->work->argument_stack[items->start + items->count++] = item;
            m->work->argument_top = items->start + items->count;
        }
        return 0;
    }
    if (value.kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY)
        return m->work->live ? KALEIDOSCOPE_SCRIPT_UNSUPPORTED : 0;
    KaleidoscopeScriptArray array = m->work->arrays[value.index];
    if (items->start + array.len > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t i = 0; i < array.len; i++)
        m->work->argument_stack[items->start + i] = m->work->elems[array.start + i];
    items->count = array.len;
    m->work->argument_top = items->start + array.len;
    return 0;
}

static KaleidoscopeScriptValue script_item(const ScriptMachine *m, ScriptItems items, size_t k) {
    return items.text ? script_str(items.start + k, 1) : m->work->argument_stack[items.start + k];
}

/* The keys and values arrays of Set/Map object `id`. */
static uint8_t script_collection(const ScriptMachine *m, uint8_t id, uint8_t *keys,
                                 uint8_t *values) {
    for (size_t i = 0; i < m->work->collection_count; i++)
        if (m->work->collections[i].object == id) {
            *keys = m->work->collections[i].keys;
            *values = m->work->collections[i].values;
            return 0;
        }
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

/* The position of `key` among the keys (SameValueZero: NaN finds NaN), or SIZE_MAX. */
static uint8_t script_collection_find(ScriptMachine *m, uint8_t keys, KaleidoscopeScriptValue key,
                                      size_t *found) {
    *found = SIZE_MAX;
    KaleidoscopeScriptArray array = m->work->arrays[keys];
    for (size_t k = 0; k < array.len; k++) {
        KaleidoscopeScriptValue item = m->work->elems[array.start + k], same = {0};
        if (item.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER &&
            key.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER && script_isnan(item.number) &&
            script_isnan(key.number)) {
            *found = k;
            return 0;
        }
        uint8_t e = script_apply(m, 'E', item, key, &same);
        if (e)
            return e;
        if (same.kind == KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN && same.boolean) {
            *found = k;
            return 0;
        }
    }
    return 0;
}

static uint8_t script_collection_put(ScriptMachine *m, uint8_t id, KaleidoscopeScriptValue key,
                                     KaleidoscopeScriptValue value) {
    uint8_t keys = 0, values = 0;
    size_t k = 0;
    uint8_t e = script_collection(m, id, &keys, &values);
    if (!e)
        e = script_collection_find(m, keys, key, &k);
    if (e)
        return e;
    if (k != SIZE_MAX) {
        if (keys != values)
            m->work->elems[m->work->arrays[values].start + k] = value;
        return 0;
    }
    e = script_push(m, keys, key);
    if (!e && keys != values)
        e = script_push(m, values, value);
    return e;
}

/* Item i of a Set (its value) or Map (a new [key, value] array); *present false past the end. */
static uint8_t script_collection_item(ScriptMachine *m, uint8_t id, size_t i, bool *present,
                                      KaleidoscopeScriptValue *out) {
    uint8_t keys = 0, values = 0;
    uint8_t e = script_collection(m, id, &keys, &values);
    if (e)
        return e;
    KaleidoscopeScriptArray array = m->work->arrays[keys];
    *present = i < array.len;
    if (!*present)
        return 0;
    KaleidoscopeScriptValue key = m->work->elems[array.start + i];
    if (keys == values) {
        *out = key;
        return 0;
    }
    KaleidoscopeScriptValue pair[2] = {key, m->work->elems[m->work->arrays[values].start + i]};
    return script_new_array(m, pair, 2, out);
}

static bool script_is_collection(const ScriptMachine *m, KaleidoscopeScriptValue value) {
    return value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
           (m->work->made_by[value.index] == 65 || m->work->made_by[value.index] == 66);
}

static uint8_t script_iterate(ScriptMachine *m, KaleidoscopeScriptValue value, ScriptItems *items);

/* `new Set(items)` / `new Map(pairs)`. */
static uint8_t script_collection_new(ScriptMachine *m, bool map, KaleidoscopeScriptValue from,
                                     KaleidoscopeScriptValue *out) {
    uint8_t id = 0;
    uint8_t e = script_new_object(m, &id);
    if (e)
        return e;
    m->work->made_by[id] = map ? 66 : 65;
    uint8_t keys = 0, values = 0;
    e = script_allocate_array(m, 0, &keys);
    if (!e && map)
        e = script_allocate_array(m, 0, &values);
    if (e)
        return e;
    if (!map)
        values = keys;
    if (m->work->collection_count == 32)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->collections[m->work->collection_count++] =
        (KaleidoscopeScriptCollection){.object = id, .keys = keys, .values = values};
    if (from.kind != KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED &&
        from.kind != KALEIDOSCOPE_SCRIPT_VALUE_NULL) {
        size_t top = m->work->argument_top;
        ScriptItems items = {0};
        e = script_iterate(m, from, &items);
        for (size_t k = 0; k < items.count && !e; k++) {
            KaleidoscopeScriptValue item = script_item(m, items, k);
            if (!map) {
                e = script_collection_put(m, id, item, item);
                continue;
            }
            if (item.kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                e = KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                break;
            }
            KaleidoscopeScriptArray pair = m->work->arrays[item.index];
            e = script_collection_put(
                m, id, pair.len > 0 ? m->work->elems[pair.start] : script_undef(),
                pair.len > 1 ? m->work->elems[pair.start + 1] : script_undef());
        }
        m->work->argument_top = top;
        if (e)
            return e;
    }
    *out = script_object(id);
    return 0;
}

/* The members of a Set or Map, as upstream's `collection_member`. */
static uint8_t script_collection_member(ScriptMachine *m, uint8_t id, const uint8_t *name,
                                        size_t name_len, KaleidoscopeScriptValue *out) {
    uint8_t keys = 0, values = 0;
    uint8_t e = script_collection(m, id, &keys, &values);
    if (e)
        return e;
    bool map = keys != values;
    if (script_word(name, name_len, "size")) {
        *out = script_num((double)m->work->arrays[keys].len);
        return 0;
    }
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    e = script_arguments(m, args, &count);
    if (e)
        return e;
    KaleidoscopeScriptValue first = count ? args[0] : script_undef();
    size_t k = 0;
    if (script_word(name, name_len, "add") && !map) {
        e = script_collection_put(m, id, first, first);
        *out = script_object(id);
        return e;
    }
    if (script_word(name, name_len, "set") && map) {
        e = script_collection_put(m, id, first, count > 1 ? args[1] : script_undef());
        *out = script_object(id);
        return e;
    }
    if (script_word(name, name_len, "get") && map) {
        e = script_collection_find(m, keys, first, &k);
        *out = k == SIZE_MAX ? script_undef() : m->work->elems[m->work->arrays[values].start + k];
        return e;
    }
    if (script_word(name, name_len, "has")) {
        e = script_collection_find(m, keys, first, &k);
        *out = script_bool(k != SIZE_MAX);
        return e;
    }
    if (script_word(name, name_len, "delete")) {
        e = script_collection_find(m, keys, first, &k);
        if (e)
            return e;
        *out = script_bool(k != SIZE_MAX);
        for (size_t pass = 0; k != SIZE_MAX && pass < (map ? 2u : 1u); pass++) {
            KaleidoscopeScriptArray *array = &m->work->arrays[pass ? values : keys];
            for (size_t i = k + 1; i < array->len; i++)
                m->work->elems[array->start + i - 1] = m->work->elems[array->start + i];
            array->len--;
        }
        return 0;
    }
    if (script_word(name, name_len, "clear")) {
        m->work->arrays[keys].len = 0;
        m->work->arrays[values].len = 0;
        *out = script_undef();
        return 0;
    }
    if (script_word(name, name_len, "keys") || script_word(name, name_len, "values")) {
        KaleidoscopeScriptArray array =
            m->work->arrays[script_word(name, name_len, "keys") ? keys : values];
        return script_new_array(m, m->work->elems + array.start, array.len, out);
    }
    if (script_word(name, name_len, "entries")) {
        size_t top = m->work->argument_top;
        ScriptItems items = {0};
        e = script_iterate(m, script_object(id), &items);
        for (size_t i = 0; i < items.count && !e && !map; i++) {
            KaleidoscopeScriptValue pair[2] = {script_item(m, items, i), script_item(m, items, i)},
                                    entry = {0};
            e = script_new_array(m, pair, 2, &entry);
            m->work->argument_stack[items.start + i] = entry;
        }
        if (!e)
            e = script_new_array(m, m->work->argument_stack + items.start, items.count, out);
        m->work->argument_top = top;
        return e;
    }
    if (script_word(name, name_len, "forEach")) {
        if (first.kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        for (size_t i = 0; i < m->work->arrays[keys].len; i++) {
            if (++m->work->steps > 50000)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            KaleidoscopeScriptValue item = m->work->elems[m->work->arrays[keys].start + i];
            KaleidoscopeScriptValue value = map ? m->work->elems[m->work->arrays[values].start + i]
                                                : item,
                                    ignored = {0};
            KaleidoscopeScriptValue call[3] = {value, item, script_object(id)};
            e = script_invoke(m, first.index, call, 3, &ignored);
            if (e)
                return e;
        }
        *out = script_undef();
        return 0;
    }
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

static uint8_t script_promise_new(ScriptMachine *m, uint8_t *out) {
    if (m->work->promise_count == 64)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    uint8_t id = 0;
    uint8_t e = script_new_object(m, &id);
    if (e)
        return e;
    m->work->made_by[id] = 67;
    m->work->promises[m->work->promise_count++] =
        (KaleidoscopeScriptPromise){.object = id, .state = 0, .value = script_undef()};
    *out = id;
    return 0;
}

static uint8_t script_resolver(ScriptMachine *m, uint8_t promise, bool reject,
                               KaleidoscopeScriptValue *out) {
    if (m->work->resolver_count == 50)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->resolver_promise[m->work->resolver_count] = promise;
    m->work->resolver_rejects[m->work->resolver_count] = reject;
    *out = script_function((uint8_t)(KALEIDOSCOPE_SCRIPT_RESOLVERS + m->work->resolver_count++));
    return 0;
}

static uint8_t script_promise_index(const ScriptMachine *m, uint8_t promise, size_t *index) {
    for (size_t k = 0; k < m->work->promise_count; k++)
        if (m->work->promises[k].object == promise) {
            *index = k;
            return 0;
        }
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

static uint8_t script_queue(ScriptMachine *m, KaleidoscopeScriptReaction reaction, uint8_t state,
                            KaleidoscopeScriptValue value) {
    if (m->work->job_count >= 256)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->jobs[m->work->job_count++] =
        (KaleidoscopeScriptJob){.reaction = reaction, .state = state, .value = value};
    return 0;
}

/* Settles a pending promise and queues the reactions waiting for it, in order. */
static uint8_t script_settle(ScriptMachine *m, uint8_t promise, uint8_t state,
                             KaleidoscopeScriptValue value) {
    size_t k = 0;
    uint8_t e = script_promise_index(m, promise, &k);
    if (e)
        return e;
    if (m->work->promises[k].state != 0)
        return 0;
    m->work->promises[k].state = state;
    m->work->promises[k].value = value;
    size_t kept = 0, count = m->work->waiting_count;
    m->work->waiting_count = 0;
    for (size_t i = 0; i < count; i++) {
        if (m->work->waiting_promise[i] != promise) {
            m->work->waiting[kept] = m->work->waiting[i];
            m->work->waiting_promise[kept++] = m->work->waiting_promise[i];
            continue;
        }
        if (!e)
            e = script_queue(m, m->work->waiting[i], state, value);
    }
    m->work->waiting_count = kept;
    return e;
}

static uint8_t script_react(ScriptMachine *m, uint8_t promise,
                            KaleidoscopeScriptReaction reaction) {
    size_t k = 0;
    uint8_t e = script_promise_index(m, promise, &k);
    if (e)
        return e;
    if (m->work->promises[k].state) {
        return script_queue(m, reaction, m->work->promises[k].state, m->work->promises[k].value);
    }
    if (m->work->waiting_count == 256)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->waiting[m->work->waiting_count] = reaction;
    m->work->waiting_promise[m->work->waiting_count++] = promise;
    return 0;
}

/* Resolves a promise: with another promise it follows that one (itself: rejected). */
static uint8_t script_resolve(ScriptMachine *m, uint8_t promise, KaleidoscopeScriptValue value) {
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT && m->work->made_by[value.index] == 67) {
        if (value.index == promise)
            return script_settle(m, promise, 2, script_undef());
        return script_react(m, value.index,
                            (KaleidoscopeScriptReaction){.child_present = true, .child = promise});
    }
    return script_settle(m, promise, 1, value);
}

/* `p.then(f, g)`, `p.catch(g)`, `p.finally(h)`: a new promise settled by the handler. */
static uint8_t script_promise_then(ScriptMachine *m, uint8_t promise, const uint8_t *name,
                                   size_t name_len, KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_arguments(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    bool first = count > 0 && args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION,
         second = count > 1 && args[1].kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION;
    uint8_t child = 0;
    e = script_promise_new(m, &child);
    if (e)
        return e;
    KaleidoscopeScriptReaction reaction = {.child_present = true, .child = child};
    if (script_word(name, name_len, "then")) {
        reaction.fulfilled_present = first;
        reaction.fulfilled = args[0].index;
        reaction.rejected_present = second;
        reaction.rejected = args[1].index;
    } else if (script_word(name, name_len, "catch")) {
        reaction.rejected_present = first;
        reaction.rejected = args[0].index;
    } else {
        reaction.fulfilled_present = reaction.rejected_present = first;
        reaction.fulfilled = reaction.rejected = args[0].index;
        reaction.finally = true;
    }
    e = script_react(m, promise, reaction);
    if (!e)
        *out = script_object(child);
    return e;
}

/* Calls f; a throw from it is caught and reported in *thrown with the thrown value, the
   machine put back as before the call. */
static uint8_t script_call_settling(ScriptMachine *m, uint8_t function,
                                    const KaleidoscopeScriptValue *args, size_t count, bool *thrown,
                                    KaleidoscopeScriptValue *out) {
    uint8_t depth = m->work->depth, loops = m->work->loops;
    bool live = m->work->live;
    size_t nvars = m->work->nvars, block_base = m->work->block_base;
    *thrown = false;
    uint8_t e = script_invoke(m, function, args, count, out);
    if (e != KALEIDOSCOPE_SCRIPT_THROWN)
        return e;
    m->work->depth = depth;
    m->work->loops = loops;
    m->work->live = live;
    m->work->nvars = nvars;
    m->work->block_base = block_base;
    m->work->jump_present = false;
    m->work->jump_label_present = false;
    m->work->ret_present = false;
    *out = m->work->thrown_present ? m->work->thrown : script_undef();
    m->work->thrown_present = false;
    *thrown = true;
    return 0;
}

static uint8_t script_run_job(ScriptMachine *m, KaleidoscopeScriptJob job) {
    if (++m->work->steps > 50000)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    KaleidoscopeScriptReaction r = job.reaction;
    if (r.slot_present) {
        KaleidoscopeScriptGather gather = m->work->gathers[r.gather];
        if (job.state == 2)
            return script_settle(m, gather.promise, 2, job.value);
        m->work->elems[m->work->arrays[gather.results].start + r.slot] = job.value;
        m->work->gathers[r.gather].pending = --gather.pending;
        if (gather.pending == 0)
            return script_settle(m, gather.promise, 1, script_array(gather.results));
        return 0;
    }
    bool present = job.state == 1 ? r.fulfilled_present : r.rejected_present;
    uint8_t handler = job.state == 1 ? r.fulfilled : r.rejected;
    if (!present) {
        if (!r.child_present)
            return 0;
        return job.state == 1 ? script_resolve(m, r.child, job.value)
                              : script_settle(m, r.child, 2, job.value);
    }
    bool thrown = false;
    KaleidoscopeScriptValue outcome = {0};
    uint8_t e = script_call_settling(m, handler, &job.value, r.finally ? 0 : 1, &thrown, &outcome);
    if (e || !r.child_present)
        return e;
    if (thrown)
        return script_settle(m, r.child, 2, outcome);
    if (r.finally)
        return job.state == 1 ? script_resolve(m, r.child, job.value)
                              : script_settle(m, r.child, 2, job.value);
    return script_resolve(m, r.child, outcome);
}

/* The microtasks: every queued reaction in order, including those queued meanwhile. */
static uint8_t script_drain_jobs(ScriptMachine *m) {
    uint8_t e = 0;
    for (size_t i = 0; i < m->work->job_count && !e; i++)
        e = script_run_job(m, m->work->jobs[i]);
    return e;
}

/* `new Promise(executor)`: the executor runs now with resolve/reject; a throw rejects. */
static uint8_t script_promise_construct(ScriptMachine *m, const KaleidoscopeScriptValue *args,
                                        size_t count, KaleidoscopeScriptValue *out) {
    uint8_t promise = 0;
    uint8_t e = script_promise_new(m, &promise);
    if (e)
        return e;
    if (!count || args[0].kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    KaleidoscopeScriptValue pair[2] = {0}, ignored = {0};
    e = script_resolver(m, promise, false, &pair[0]);
    if (!e)
        e = script_resolver(m, promise, true, &pair[1]);
    bool thrown = false;
    if (!e)
        e = script_call_settling(m, args[0].index, pair, 2, &thrown, &ignored);
    if (!e && thrown)
        e = script_settle(m, promise, 2, ignored);
    if (!e)
        *out = script_object(promise);
    return e;
}

/* `Promise.resolve(v)`, `Promise.reject(v)`, `Promise.all(items)`. */
static uint8_t script_promise_static(ScriptMachine *m, const uint8_t *f, size_t f_len,
                                     KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_arguments(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    uint8_t promise = 0;
    if (script_word(f, f_len, "resolve")) {
        if (args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
            m->work->made_by[args[0].index] == 67) {
            *out = args[0];
            return 0;
        }
        e = script_promise_new(m, &promise);
        if (!e)
            e = script_resolve(m, promise, args[0]);
    } else if (script_word(f, f_len, "reject")) {
        e = script_promise_new(m, &promise);
        if (!e)
            e = script_settle(m, promise, 2, args[0]);
    } else if (script_word(f, f_len, "all")) {
        size_t top = m->work->argument_top;
        ScriptItems items = {0};
        e = script_iterate(m, args[0], &items);
        if (!e)
            e = script_promise_new(m, &promise);
        uint8_t results = 0;
        if (!e)
            e = script_allocate_array(m, items.count, &results);
        if (!e && !items.count)
            e = script_settle(m, promise, 1, script_array(results));
        else if (!e && m->work->gather_count == 32)
            e = KALEIDOSCOPE_SCRIPT_LIMIT;
        else if (!e) {
            uint8_t g = (uint8_t)m->work->gather_count++;
            m->work->gathers[g] = (KaleidoscopeScriptGather){
                .promise = promise, .results = results, .pending = items.count};
            for (size_t k = 0; k < items.count && !e; k++) {
                KaleidoscopeScriptValue item = script_item(m, items, k);
                uint8_t q = item.index;
                if (!(item.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
                      m->work->made_by[item.index] == 67)) {
                    e = script_promise_new(m, &q);
                    if (!e)
                        e = script_settle(m, q, 1, item);
                }
                if (!e)
                    e = script_react(m, q,
                                     (KaleidoscopeScriptReaction){
                                         .slot_present = true, .gather = g, .slot = (uint16_t)k});
            }
        }
        m->work->argument_top = top;
    } else
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    if (!e)
        *out = script_object(promise);
    return e;
}

/* Statics added upstream after the first port: `Array.from(x, f)`, Number constants and
   `Number.parseFloat`/`parseInt`, `Object.groupBy`, `Object.defineProperty`, `Object.hasOwn`.
   *handled is false when the name is none of them. */
static uint8_t script_statics_extra(ScriptMachine *m, bool json, bool object, bool array_name,
                                    bool number, bool promise_name, const uint8_t *f, size_t f_len,
                                    bool *handled, KaleidoscopeScriptValue *out) {
    *handled = true;
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = 0;
    if (json && script_word(f, f_len, "stringify")) {
        /* `JSON.stringify(v, replacer, space)`: a key list (a function replacer is refused) and
           an indent of up to 10 spaces or text. */
        e = script_arguments(m, args, &count);
        if (e)
            return e;
        ScriptJsonLayout layout = {0};
        size_t top = m->work->argument_top;
        if (args[1].kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY && count > 1) {
            KaleidoscopeScriptArray list = m->work->arrays[args[1].index];
            if (top + list.len > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            for (size_t k = 0; k < list.len && !e; k++) {
                KaleidoscopeScriptValue key = {0};
                e = script_to_text(m, m->work->elems[m->work->arrays[args[1].index].start + k],
                                   &key);
                m->work->argument_stack[top + k] = key;
            }
            if (e)
                return e;
            layout.keys_present = true;
            layout.keys_start = top;
            layout.keys_count = list.len;
            m->work->argument_top = top + list.len;
        } else if (!(args[1].kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
                     args[1].kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL || count < 2))
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        if (args[2].kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER) {
            double x = script_isnan(args[2].number) ? 0.0 : args[2].number;
            x = x < 0.0 ? 0.0 : x > 10.0 ? 10.0 : x;
            layout.indent_len = (size_t)x;
            for (size_t k = 0; k < layout.indent_len; k++)
                layout.indent[k] = ' ';
        } else if (args[2].kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
            layout.indent_len = args[2].len < 10 ? args[2].len : 10;
            for (size_t k = 0; k < layout.indent_len; k++)
                layout.indent[k] = m->work->arena[args[2].start + k];
        }
        e = script_json_layout(m, args[0], &layout, out);
        m->work->argument_top = top;
        return e;
    }
    if (array_name && script_word(f, f_len, "from")) {
        bool second = false;
        e = script_from_with_map(m, &second);
        if (e || !second) {
            *handled = second;
            return e;
        }
        /* `Array.from(x, fn)`: each item mapped through fn(item, index). */
        e = script_arguments(m, args, &count);
        if (e)
            return e;
        if (!m->work->live) {
            *out = script_undef();
            return 0;
        }
        if (args[1].kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        size_t top = m->work->argument_top;
        ScriptItems items = {false, top, 0};
        bool blank = args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
                     m->work->made_by[args[0].index] != 65 && m->work->made_by[args[0].index] != 66;
        if (blank) {
            static const uint8_t length_key[] = "length";
            double n = script_to_number(m, script_get_prop(m, args[0].index, length_key, 6));
            if (!(n >= 0.0 && n <= (double)KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT))
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            items.count = (size_t)n;
        } else {
            e = script_iterate(m, args[0], &items);
            if (e) {
                m->work->argument_top = top;
                return e;
            }
        }
        /* The results go above the items on the argument stack, then into one new array. */
        size_t results = m->work->argument_top;
        if (results + items.count > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK) {
            m->work->argument_top = top;
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        }
        for (size_t k = 0; k < items.count && !e; k++) {
            if (++m->work->steps > 50000)
                e = KALEIDOSCOPE_SCRIPT_LIMIT;
            KaleidoscopeScriptValue call[2] = {blank ? script_undef() : script_item(m, items, k),
                                               script_num((double)k)};
            m->work->argument_top = results + k;
            KaleidoscopeScriptValue mapped = {0};
            if (!e)
                e = script_invoke(m, args[1].index, call, 2, &mapped);
            if (!e)
                m->work->argument_stack[results + k] = mapped;
        }
        if (!e)
            e = script_new_array(m, m->work->argument_stack + results, items.count, out);
        m->work->argument_top = top;
        return e;
    }
    if (promise_name)
        return script_promise_static(m, f, f_len, out);
    if (number && !script_punct(m, '(')) {
        static const char *const names[8] = {
            "MAX_SAFE_INTEGER", "MIN_SAFE_INTEGER",  "EPSILON",           "MAX_VALUE",
            "MIN_VALUE",        "POSITIVE_INFINITY", "NEGATIVE_INFINITY", "NaN"};
        double values[8] = {9007199254740991.0,
                            -9007199254740991.0,
                            2.220446049250313e-16,
                            1.7976931348623157e308,
                            5e-324,
                            script_inf(),
                            -script_inf(),
                            script_nan()};
        for (size_t i = 0; i < 8; i++)
            if (script_word(f, f_len, names[i])) {
                *out = script_num(values[i]);
                return 0;
            }
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    }
    if (object && script_word(f, f_len, "groupBy")) {
        /* `Object.groupBy(items, fn)`: arrays of items under fn(item, i). */
        e = script_arguments(m, args, &count);
        if (e)
            return e;
        if (!m->work->live) {
            *out = script_undef();
            return 0;
        }
        if (args[1].kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        size_t top = m->work->argument_top;
        ScriptItems items = {0};
        e = script_iterate(m, args[0], &items);
        uint8_t result = 0;
        if (!e)
            e = script_new_object(m, &result);
        for (size_t k = 0; k < items.count && !e; k++) {
            if (++m->work->steps > 50000) {
                e = KALEIDOSCOPE_SCRIPT_LIMIT;
                break;
            }
            KaleidoscopeScriptValue item = script_item(m, items, k), key = {0};
            KaleidoscopeScriptValue call[2] = {item, script_num((double)k)};
            e = script_invoke(m, args[1].index, call, 2, &key);
            if (!e)
                e = script_to_text(m, key, &key);
            if (e)
                break;
            ScriptSlice text = script_text(key);
            KaleidoscopeScriptValue group =
                script_get_prop(m, result, m->work->arena + text.start, text.len);
            if (group.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY)
                e = script_push(m, group.index, item);
            else {
                e = script_new_array(m, &item, 1, &group);
                if (!e)
                    e = script_set_prop(m, result, m->work->arena + text.start, text.len, group);
            }
        }
        m->work->argument_top = top;
        if (!e)
            *out = script_object(result);
        return e;
    }
    if (number && (script_word(f, f_len, "parseFloat") || script_word(f, f_len, "parseInt")))
        return script_convert(m, false, false, false, script_word(f, f_len, "parseFloat"), false,
                              false, false, false, out);
    if (object && script_word(f, f_len, "defineProperty")) {
        /* `value`, `get` and `set` of a descriptor; the other flags are not modelled
           (properties stay writable and enumerable). */
        e = script_arguments(m, args, &count);
        if (e)
            return e;
        if (!m->work->live) {
            *out = script_undef();
            return 0;
        }
        if (args[0].kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ||
            args[2].kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        uint8_t id = args[0].index, descriptor = args[2].index;
        KaleidoscopeScriptValue key = {0};
        e = script_to_text(m, args[1], &key);
        if (e)
            return e;
        ScriptSlice text = script_text(key);
        static const uint8_t get_key[] = "get", set_key[] = "set", value_key[] = "value";
        KaleidoscopeScriptValue getter = script_get_prop(m, descriptor, get_key, 3),
                                setter = script_get_prop(m, descriptor, set_key, 3);
        if (getter.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            e = script_add_accessor(m, id, false, text.start, text.len, getter.index, false);
        if (!e && setter.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            e = script_add_accessor(m, id, false, text.start, text.len, setter.index, true);
        if (!e && script_find_prop(m, descriptor, value_key, 5) != SIZE_MAX)
            e = script_set_prop(m, id, m->work->arena + text.start, text.len,
                                script_get_prop(m, descriptor, value_key, 5));
        if (!e)
            *out = args[0];
        return e;
    }
    if (object && script_word(f, f_len, "hasOwn")) {
        e = script_arguments(m, args, &count);
        if (e)
            return e;
        if (args[0].kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
            *out = script_bool(false);
            return 0;
        }
        KaleidoscopeScriptValue key = {0};
        e = script_to_text(m, args[1], &key);
        if (e)
            return e;
        ScriptSlice text = script_text(key);
        const uint8_t *bytes = m->work->arena + text.start;
        *out = script_bool(script_find_prop(m, args[0].index, bytes, text.len) != SIZE_MAX ||
                           script_find_accessor(m, args[0].index, bytes, text.len) != SIZE_MAX);
        return 0;
    }
    *handled = false;
    return 0;
}

/* Whether a top-level `,` follows in the argument list ahead (`Array.from(x, fn)`). */
static uint8_t script_from_with_map(ScriptMachine *m, bool *second) {
    KaleidoscopeScriptPosition at = script_position_save(m);
    *second = false;
    uint8_t e = script_expect(m, '(');
    size_t depth = 0;
    while (!e) {
        if (script_punct(m, '(') || script_punct(m, '[') || script_punct(m, '{'))
            depth++;
        else if (script_punct(m, ')') || script_punct(m, ']') || script_punct(m, '}')) {
            if (!depth)
                break;
            depth--;
        } else if (script_punct(m, ',') && !depth) {
            *second = true;
            break;
        } else if (m->token.kind == KALEIDOSCOPE_TOKEN_END)
            break;
        e = script_next(m);
    }
    script_position_restore(m, at);
    return e;
}

/* `structuredClone(v)`: a deep copy of arrays and plain objects (and Errors), as upstream's
   `deep_copy`. */
static uint8_t script_deep_copy(ScriptMachine *m, KaleidoscopeScriptValue value, uint8_t depth,
                                KaleidoscopeScriptValue *out) {
    if (depth > 32)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    *out = value;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
        /* The copies go on the argument stack, then into one new array. */
        size_t top = m->work->argument_top, len = m->work->arrays[value.index].len;
        if (top + len > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t i = 0; i < len; i++)
            m->work->argument_stack[top + i] =
                m->work->elems[m->work->arrays[value.index].start + i];
        m->work->argument_top = top + len;
        uint8_t e = 0;
        for (size_t i = 0; i < len && !e; i++) {
            KaleidoscopeScriptValue copy = {0};
            e = script_deep_copy(m, m->work->argument_stack[top + i], (uint8_t)(depth + 1), &copy);
            if (!e)
                m->work->argument_stack[top + i] = copy;
        }
        if (!e)
            e = script_new_array(m, m->work->argument_stack + top, len, out);
        m->work->argument_top = top;
        return e;
    }
    if (value.kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
        return 0;
    uint16_t made = m->work->made_by[value.index];
    if (made != 0 && made != 64)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    uint8_t copy = 0;
    uint8_t e = script_new_object(m, &copy);
    if (e)
        return e;
    m->work->made_by[copy] = made;
    KaleidoscopeScriptArray record = m->work->objects[value.index];
    for (size_t k = 0; k < record.len; k++) {
        KaleidoscopeScriptProperty property = m->work->props[record.start + k];
        KaleidoscopeScriptValue item = {0};
        e = script_deep_copy(m, property.value, (uint8_t)(depth + 1), &item);
        if (!e)
            e = script_set_prop(m, copy, m->work->arena + property.start, property.len, item);
        if (e)
            return e;
    }
    *out = script_object(copy);
    return 0;
}

static uint8_t script_structured_clone(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_arguments(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    return script_deep_copy(m, args[0], 0, out);
}

static uint8_t script_atom(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    bool arrow = false;
    if (script_punct(m, '(')) {
        uint8_t e = script_arrow_ahead(m, &arrow);
        if (e)
            return e;
    } else if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
        KaleidoscopeScriptPosition at = script_position_save(m);
        uint8_t e = script_next(m);
        arrow = !e && script_punct(m, 'F');
        script_position_restore(m, at);
        if (e)
            return e;
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_NUMBER) {
        *out = script_num(m->token.number);
        return script_next(m);
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_STRING) {
        size_t start = m->token.start, len = m->token.len;
        uint8_t e = script_next(m);
        if (e)
            return e;
        return script_literal(m, start, len, out);
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_REGEX) {
        size_t start = m->token.start, len = m->token.len, flags_start = m->token.aux_start,
               flags_len = m->token.aux_len;
        uint8_t e = script_next(m);
        if (e)
            return e;
        for (size_t i = 0; i < m->work->regex_count; i++)
            if (m->work->regexes[i].pattern_start == start) {
                *out = script_regex((uint8_t)i);
                return 0;
            }
        if (m->work->regex_count == 16)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        uint8_t index = (uint8_t)m->work->regex_count++;
        m->work->regexes[index] = (KaleidoscopeScriptRegex){.pattern_start = (uint16_t)start,
                                                            .pattern_len = (uint16_t)len,
                                                            .flags_start = (uint16_t)flags_start,
                                                            .flags_len = (uint16_t)flags_len};
        e = script_regex_compile(m, index);
        if (e)
            return e;
        *out = script_regex(index);
        return 0;
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_TEMPLATE) {
        size_t start = m->token.start, len = m->token.len;
        uint8_t e = script_next(m);
        if (e)
            return e;
        return script_template(m, start, len, out);
    }
    if (script_punct(m, '(') && arrow) {
        KaleidoscopeScriptPosition at = script_position_save(m);
        uint8_t e = 0;
        while (!script_punct(m, 'F')) {
            e = script_next(m);
            if (e)
                return e;
            if (m->token.kind == KALEIDOSCOPE_TOKEN_END)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
        }
        e = script_next(m);
        if (e)
            return e;
        e = script_skim_body(m);
        if (e)
            return e;
        uint8_t function = 0;
        e = script_define(m, NULL, 0, at, &function);
        if (e)
            return e;
        *out = script_function(function);
        return 0;
    }
    if (script_punct(m, '(')) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        e = script_expr(m, out);
        /* The comma operator: each runs, the last is the value. */
        while (!e && script_punct(m, ',')) {
            e = script_next(m);
            if (!e)
                e = script_expr(m, out);
        }
        if (e)
            return e;
        return script_expect(m, ')');
    }
    if (script_punct(m, '{'))
        return script_object_literal(m, out);
    if (script_punct(m, '[')) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        KaleidoscopeScriptValue items[32] = {0};
        size_t count = 0;
        e = script_items(m, ']', items, 32, &count);
        return e ? e : script_new_array(m, items, count, out);
    }
    if (script_ident(m, "new")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        size_t name_start = m->token.start, name_len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
        KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
        size_t count = 0;
        for (size_t i = 0; i < KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE; i++)
            args[i] = script_undef();
        if (script_punct(m, '(')) {
            e = script_argument_list(m, args, &count);
            if (e)
                return e;
        }
        if (!m->work->live) {
            *out = script_undef();
            return 0;
        }
        return script_construct(m, m->source->bytes + name_start, name_len, args, count, out);
    }
    if (script_ident(m, "function")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        /* A named function expression can call itself by its name. */
        size_t name_start = 0, name_len = 0;
        if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
            name_start = m->token.start;
            name_len = m->token.len;
            e = script_next(m);
            if (e)
                return e;
        }
        KaleidoscopeScriptPosition at = script_position_save(m);
        e = script_expect(m, '(');
        if (e)
            return e;
        e = script_skip_params(m);
        if (e)
            return e;
        e = script_expect(m, ')');
        if (e)
            return e;
        if (!script_punct(m, '{'))
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        e = script_skim_body(m);
        if (e)
            return e;
        uint8_t function = 0;
        e = script_define(m, m->source->bytes + name_start, name_len, at, &function);
        if (e)
            return e;
        *out = script_function(function);
        return 0;
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT && arrow) {
        KaleidoscopeScriptPosition at = script_position_save(m);
        uint8_t e = script_next(m);
        if (e)
            return e;
        e = script_next(m);
        if (e)
            return e;
        e = script_skim_body(m);
        if (e)
            return e;
        uint8_t function = 0;
        e = script_define(m, NULL, 0, at, &function);
        if (e)
            return e;
        *out = script_function(function);
        return 0;
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
        size_t start = m->token.start, len = m->token.len;
        bool nan = script_ident(m, "NaN"), null = script_ident(m, "null"),
             this_name = script_ident(m, "this"), super_name = script_ident(m, "super"),
             undefined = script_ident(m, "undefined"), infinity = script_ident(m, "Infinity"),
             yes = script_ident(m, "true"), no = script_ident(m, "false"),
             math = script_ident(m, "Math"), date_name = script_ident(m, "Date"),
             json = script_ident(m, "JSON"), object = script_ident(m, "Object"),
             array_name = script_ident(m, "Array"), as_string = script_ident(m, "String"),
             as_number = script_ident(m, "Number"), as_boolean = script_ident(m, "Boolean"),
             parse_int = script_ident(m, "parseInt"), parse_float = script_ident(m, "parseFloat"),
             is_nan = script_ident(m, "isNaN"), encode_uri = script_ident(m, "encodeURIComponent"),
             decode_uri = script_ident(m, "decodeURIComponent"),
             is_finite = script_ident(m, "isFinite"), console = script_ident(m, "console"),
             arguments_name = script_ident(m, "arguments"),
             structured_clone = script_ident(m, "structuredClone"),
             promise_name = script_ident(m, "Promise"), document = script_ident(m, "document"),
             error_name = script_ident(m, "Error") || script_ident(m, "TypeError") ||
                          script_ident(m, "RangeError") || script_ident(m, "SyntaxError");
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (nan) {
            *out = script_num(script_nan());
            return 0;
        }
        if (null) {
            *out = script_null();
            return 0;
        }
        if (this_name) {
            *out = m->work->this_value;
            return 0;
        }
        if (console && script_punct(m, '.') && !script_find(m, start, len))
            return script_console_call(m, out);
        if (arguments_name && m->work->func_now_present && !script_find(m, start, len))
            return script_arguments_array(m, out);
        if ((array_name || error_name) && script_punct(m, '(') && !script_find(m, start, len))
            return script_construct_call(m, m->source->bytes + start, len, out);
        if (super_name)
            return script_super_call(m, out);
        if (undefined) {
            *out = script_undef();
            return 0;
        }
        if (infinity) {
            *out = script_num(script_inf());
            return 0;
        }
        if (yes) {
            *out = script_bool(true);
            return 0;
        }
        if (no) {
            *out = script_bool(false);
            return 0;
        }
        if (math)
            return script_math(m, out);
        /* `document.write(...)` as an expression (arrow bodies); its value is undefined. */
        if (document && script_punct(m, '.'))
            return script_write_expression(m, out);
        if ((script_punct(m, 'I') || script_punct(m, 'D')) && !m->token.newline) {
            bool up = script_punct(m, 'I');
            e = script_next(m);
            if (e)
                return e;
            return script_step_name(m, start, len, up, false, out);
        }
        if (date_name && script_punct(m, '.')) {
            e = script_next(m);
            if (e)
                return e;
            if (!script_ident(m, "UTC"))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            e = script_next(m);
            if (e)
                return e;
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            e = script_arguments(m, args, &count);
            if (e)
                return e;
            double values[7] = {0, 0, 1, 0, 0, 0, 0};
            for (size_t i = 0; i < count && i < 7; i++)
                values[i] = script_to_number(m, args[i]);
            *out = script_num(script_date_ms(values));
            return 0;
        }
        if (as_string && script_punct(m, '.')) {
            e = script_next(m);
            if (e)
                return e;
            if (script_ident(m, "raw")) {
                e = script_next(m);
                if (e)
                    return e;
                if (m->token.kind != KALEIDOSCOPE_TOKEN_TEMPLATE)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                size_t raw_start = m->token.start, raw_len = m->token.len;
                e = script_next(m);
                if (e)
                    return e;
                return script_string_raw(m, raw_start, raw_len, out);
            }
            if (!script_ident(m, "fromCharCode"))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            e = script_next(m);
            if (e)
                return e;
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            e = script_arguments(m, args, &count);
            if (e)
                return e;
            if (!m->work->live) {
                *out = script_undef();
                return 0;
            }
            uint8_t text[24] = {0};
            size_t n = 0;
            for (size_t i = 0; i < count; i++) {
                double number = script_to_number(m, args[i]);
                if (script_isnan(number) || script_isinf(number) || number < 0.0)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                uint32_t raw = number > (double)UINT32_MAX ? UINT32_MAX : (uint32_t)number,
                         code = raw & 0xffffu;
                if (code >= 0xd800u && code <= 0xdfffu)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                if (code < 0x80u)
                    text[n++] = (uint8_t)code;
                else if (code < 0x800u) {
                    text[n++] = (uint8_t)(0xc0u | (code >> 6));
                    text[n++] = (uint8_t)(0x80u | (code & 63u));
                } else {
                    text[n++] = (uint8_t)(0xe0u | (code >> 12));
                    text[n++] = (uint8_t)(0x80u | ((code >> 6) & 63u));
                    text[n++] = (uint8_t)(0x80u | (code & 63u));
                }
            }
            return script_store(m, text, n, out);
        }
        if (json || object || array_name || ((as_number || promise_name) && script_punct(m, '.'))) {
            e = script_expect(m, '.');
            if (e)
                return e;
            bool stringify = script_ident(m, "stringify"), parse = script_ident(m, "parse"),
                 keys = script_ident(m, "keys"), values = script_ident(m, "values"),
                 entries = script_ident(m, "entries"),
                 from_entries = script_ident(m, "fromEntries"), assign = script_ident(m, "assign"),
                 is_array = script_ident(m, "isArray"), from = script_ident(m, "from"),
                 of = script_ident(m, "of"), is_integer = script_ident(m, "isInteger"),
                 is_finite = script_ident(m, "isFinite"), number_nan = script_ident(m, "isNaN"),
                 create = script_ident(m, "create"), freeze = script_ident(m, "freeze"),
                 is_frozen = script_ident(m, "isFrozen"),
                 own_names = script_ident(m, "getOwnPropertyNames");
            size_t f_start = m->token.start, f_len = m->token.len;
            e = script_next(m);
            if (e)
                return e;
            bool handled = false;
            e = script_statics_extra(m, json, object, array_name, as_number, promise_name,
                                     m->source->bytes + f_start, f_len, &handled, out);
            if (e || handled)
                return e;
            if (array_name && of) {
                KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
                size_t count = 0;
                e = script_arguments(m, args, &count);
                if (e)
                    return e;
                return script_new_array(m, args, count, out);
            }
            if (object && assign) {
                KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
                size_t count = 0;
                e = script_arguments(m, args, &count);
                if (e)
                    return e;
                if (!m->work->live) {
                    *out = script_undef();
                    return 0;
                }
                if (!count || args[0].kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                uint8_t target = args[0].index;
                for (size_t i = 1; i < count; i++) {
                    if (args[i].kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL ||
                        args[i].kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED)
                        continue;
                    if (args[i].kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
                        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                    KaleidoscopeScriptArray source = m->work->objects[args[i].index];
                    for (size_t k = 0; k < source.len; k++) {
                        KaleidoscopeScriptProperty property = m->work->props[source.start + k];
                        e = script_set_prop(m, target, m->work->arena + property.start,
                                            property.len, property.value);
                        if (e)
                            return e;
                    }
                }
                *out = script_object(target);
                return 0;
            }
            e = script_expect(m, '(');
            if (e)
                return e;
            KaleidoscopeScriptValue value = {0};
            e = script_expr(m, &value);
            if (e)
                return e;
            e = script_expect(m, ')');
            if (e)
                return e;
            if (object && from_entries) {
                if (value.kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                uint8_t result = 0;
                e = script_new_object(m, &result);
                if (e)
                    return e;
                KaleidoscopeScriptArray list = m->work->arrays[value.index];
                for (size_t i = 0; i < list.len; i++) {
                    KaleidoscopeScriptValue entry = m->work->elems[list.start + i];
                    if (entry.kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY)
                        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                    KaleidoscopeScriptArray pair = m->work->arrays[entry.index];
                    KaleidoscopeScriptValue key = pair.len ? m->work->elems[pair.start]
                                                           : script_undef(),
                                            item = pair.len > 1 ? m->work->elems[pair.start + 1]
                                                                : script_undef();
                    e = script_to_text(m, key, &key);
                    if (e)
                        return e;
                    ScriptSlice text = script_text(key);
                    e = script_set_prop(m, result, m->work->arena + text.start, text.len, item);
                    if (e)
                        return e;
                }
                *out = script_object(result);
                return 0;
            }
            if (json && stringify)
                return script_json(m, value, out);
            if (json && parse) {
                if (!m->work->live) {
                    *out = script_undef();
                    return 0;
                }
                e = script_to_text(m, value, &value);
                if (e)
                    return e;
                ScriptSlice input = script_text(value);
                size_t at = 0;
                e = script_json_parse(m, m->work->arena + input.start, input.len, &at, 0, out);
                script_json_ws(m->work->arena + input.start, input.len, &at);
                if (!e && at == input.len)
                    return 0;
                if (e && e != KALEIDOSCOPE_SCRIPT_SYNTAX)
                    return e;
                static const uint8_t message[] = "SyntaxError: bad JSON";
                e = script_store(m, message, 21, &m->work->thrown);
                if (e)
                    return e;
                m->work->thrown_present = true;
                return KALEIDOSCOPE_SCRIPT_THROWN;
            }
            if (array_name && is_array) {
                *out = script_bool(value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY);
                return 0;
            }
            if (array_name && from) {
                if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                    KaleidoscopeScriptArray array = m->work->arrays[value.index];
                    return script_new_array(m, m->work->elems + array.start, array.len, out);
                }
                if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
                    uint8_t id = 0;
                    e = script_allocate_array(m, value.len, &id);
                    if (e)
                        return e;
                    KaleidoscopeScriptArray array = m->work->arrays[id];
                    for (size_t i = 0; i < value.len; i++)
                        m->work->elems[array.start + i] = script_str(value.start + i, 1);
                    *out = script_array(id);
                    return 0;
                }
                if (script_is_collection(m, value)) {
                    size_t top = m->work->argument_top;
                    ScriptItems items = {0};
                    e = script_iterate(m, value, &items);
                    if (!e)
                        e = script_new_array(m, m->work->argument_stack + items.start, items.count,
                                             out);
                    m->work->argument_top = top;
                    return e;
                }
                if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
                    static const uint8_t length_key[] = "length";
                    double n = script_to_number(m, script_get_prop(m, value.index, length_key, 6));
                    if (n < 0.0 || n > (double)KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT)
                        return KALEIDOSCOPE_SCRIPT_LIMIT;
                    uint8_t id = 0;
                    e = script_allocate_array(m, (size_t)n, &id);
                    if (e)
                        return e;
                    *out = script_array(id);
                    return 0;
                }
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            }
            if (as_number && (is_integer || is_finite || number_nan)) {
                bool numeric = value.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER;
                *out = script_bool(
                    numeric &&
                    (is_integer  ? (!script_isnan(value.number) && !script_isinf(value.number) &&
                                   value.number == script_floor(value.number))
                     : is_finite ? (!script_isnan(value.number) && !script_isinf(value.number))
                                 : script_isnan(value.number)));
                return 0;
            }
            if (object && create) {
                /* A new object with a snapshot of `proto`'s properties. */
                if (!m->work->live) {
                    *out = script_undef();
                    return 0;
                }
                uint8_t id = 0;
                e = script_new_object(m, &id);
                if (e)
                    return e;
                if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
                    e = script_copy_props(m, value.index, id);
                else if (value.kind != KALEIDOSCOPE_SCRIPT_VALUE_NULL)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                if (e)
                    return e;
                *out = script_object(id);
                return 0;
            }
            if (object && freeze) {
                if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT && m->work->live &&
                    !script_frozen(m, value.index))
                    m->work->frozen[m->work->frozen_count++] = value.index;
                *out = value;
                return 0;
            }
            if (object && is_frozen) {
                *out = script_bool(value.kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ||
                                   script_frozen(m, value.index));
                return 0;
            }
            if (!object || value.kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ||
                !(keys || values || entries || own_names))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            /* Own properties but hidden ones, gathered on the argument stack. */
            KaleidoscopeScriptArray record = m->work->objects[value.index];
            size_t top = m->work->argument_top, n = 0;
            if (top + record.len > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            for (size_t i = 0; i < record.len && !e; i++) {
                if (script_hidden_prop(m, value.index, record.start + i))
                    continue;
                KaleidoscopeScriptProperty property = m->work->props[record.start + i];
                KaleidoscopeScriptValue item = property.value;
                if (keys || own_names)
                    item = script_str(property.start, property.len);
                else if (entries) {
                    KaleidoscopeScriptValue pair[2] = {script_str(property.start, property.len),
                                                       property.value};
                    e = script_new_array(m, pair, 2, &item);
                }
                m->work->argument_stack[top + n++] = item;
                m->work->argument_top = top + n;
            }
            if (!e)
                e = script_new_array(m, m->work->argument_stack + top, n, out);
            m->work->argument_top = top;
            return e;
        }
        if (script_punct(m, '.'))
            for (size_t c = 0; c < m->work->class_count; c++)
                if (script_class_name(m, &m->work->classes[c], m->source->bytes + start, len))
                    return script_class_static_call(m, c, out);
        if ((as_string || as_number || as_boolean || parse_int || parse_float || is_nan ||
             is_finite || encode_uri || decode_uri) &&
            script_punct(m, '(')) {
            return script_convert(m, as_string, as_number, as_boolean, parse_float, is_nan,
                                  is_finite, encode_uri, decode_uri, out);
        }
        if (structured_clone && script_punct(m, '(') && !script_find(m, start, len))
            return script_structured_clone(m, out);
        if ((as_string || as_number || as_boolean) && !script_find(m, start, len)) {
            *out = script_function((uint8_t)(250 + (as_string ? 0 : as_number ? 1 : 2)));
            return 0;
        }
        if (script_punct(m, '(')) {
            return script_call(m, start, len, out);
        }
        KaleidoscopeScriptVariable *v = script_find(m, start, len);
        if (v) {
            *out = v->value;
            return 0;
        }
        for (size_t i = 0; i < m->work->nfuncs; i++)
            if (m->work->funcs[i].len && script_function_name(m, &m->work->funcs[i], start, len)) {
                *out = script_function((uint8_t)i);
                return 0;
            }
        if (m->work->skim || !m->work->live) {
            *out = script_undef();
            return 0;
        }
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    }
    return KALEIDOSCOPE_SCRIPT_SYNTAX;
}

static uint8_t script_chain(ScriptMachine *m, KaleidoscopeScriptValue *value);

static uint8_t script_postfix(ScriptMachine *m, KaleidoscopeScriptValue *value) {
    bool live = m->work->live;
    uint8_t e = script_chain(m, value);
    bool shorted = live && !m->work->live;
    m->work->live = live;
    if (!e && shorted)
        *value = script_undef();
    return e;
}

static uint8_t script_chain(ScriptMachine *m, KaleidoscopeScriptValue *value) {
    for (;;) {
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION && script_punct(m, '(')) {
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            uint8_t e = script_argument_list(m, args, &count);
            if (e)
                return e;
            if (m->work->live) {
                e = script_invoke(m, value->index, args, count, value);
                if (e)
                    return e;
            } else
                *value = script_undef();
            continue;
        }
        bool optional = script_punct(m, 'O');
        if (optional) {
            uint8_t e = script_next(m);
            if (e)
                return e;
            if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
                value->kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL)
                m->work->live = false;
            if (script_punct(m, '(')) {
                /* `f?.(args)`: called unless f is null/undefined. */
                KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
                size_t count = 0;
                e = script_argument_list(m, args, &count);
                if (e)
                    return e;
                if (!m->work->live)
                    *value = script_undef();
                else if (value->kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                else {
                    e = script_invoke(m, value->index, args, count, value);
                    if (e)
                        return e;
                }
                continue;
            }
        }
        /* A tagged template: tag`a${x}b` calls tag(strings, ...values). */
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION &&
            m->token.kind == KALEIDOSCOPE_TOKEN_TEMPLATE) {
            size_t raw_start = m->token.start, raw_len = m->token.len;
            uint8_t e = script_next(m);
            if (!e)
                e = script_tagged_template(m, value->index, raw_start, raw_len, value);
            if (e)
                return e;
            continue;
        }
        if (script_punct(m, '[')) {
            uint8_t e = script_next(m);
            if (e)
                return e;
            KaleidoscopeScriptValue index = {0};
            e = script_expr(m, &index);
            if (e)
                return e;
            e = script_expect(m, ']');
            if (e)
                return e;
            if (!m->work->live && value->kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
                value->kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY &&
                value->kind != KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
                *value = script_undef();
                continue;
            }
            if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
                e = script_to_text(m, index, &index);
                if (e)
                    return e;
                ScriptSlice key = script_text(index);
                e = script_read_prop(m, value->index, m->work->arena + key.start, key.len, value);
                if (e)
                    return e;
            } else {
                e = script_index(m, *value, script_to_number(m, index), value);
                if (e)
                    return e;
            }
            continue;
        }
        if (!optional) {
            if (!script_punct(m, '.')) {
                return 0;
            }
            uint8_t e = script_next(m);
            if (e)
                return e;
        }
        uint8_t e = 0;
        size_t name_start = m->token.start, name_len = m->token.len;
        const uint8_t *name = m->source->bytes + name_start;
        bool length = script_ident(m, "length"), push = script_ident(m, "push"),
             join = script_ident(m, "join"), to_fixed = script_ident(m, "toFixed"),
             to_string = script_ident(m, "toString"), upper = script_ident(m, "toUpperCase"),
             lower = script_ident(m, "toLowerCase"), trim = script_ident(m, "trim"),
             trim_start = script_ident(m, "trimStart"), trim_end = script_ident(m, "trimEnd"),
             at_method = script_ident(m, "at"), index_of = script_ident(m, "indexOf"),
             char_at = script_ident(m, "charAt"), char_code = script_ident(m, "charCodeAt"),
             code_point = script_ident(m, "codePointAt"), slice = script_ident(m, "slice"),
             substring = script_ident(m, "substring"), split = script_ident(m, "split"),
             replace = script_ident(m, "replace"), replace_all = script_ident(m, "replaceAll"),
             starts = script_ident(m, "startsWith"), ends = script_ident(m, "endsWith"),
             includes = script_ident(m, "includes"), repeat = script_ident(m, "repeat"),
             pad_start = script_ident(m, "padStart"), pad_end = script_ident(m, "padEnd"),
             last_index = script_ident(m, "lastIndexOf");
        e = script_next(m);
        if (e)
            return e;
        if (!m->work->live && value->kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
            value->kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY &&
            value->kind != KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
            if (script_punct(m, '(')) {
                KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
                size_t count = 0;
                e = script_argument_list(m, args, &count);
                if (e)
                    return e;
            }
            *value = script_undef();
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION) {
            e = script_function_member(m, value->index, name, name_len, value);
            if (e)
                return e;
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
            static const uint8_t date_key[] = "__date";
            KaleidoscopeScriptValue date = script_get_prop(m, value->index, date_key, 6);
            if (date.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER &&
                script_find_prop(m, value->index, name, name_len) == SIZE_MAX) {
                KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
                size_t count = 0;
                e = script_arguments(m, args, &count);
                if (e)
                    return e;
                e = script_date_method(m, date.number, name, name_len, value);
                if (e)
                    return e;
                continue;
            }
        }
        if (m->work->live && value->kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
            m->work->made_by[value->index] == 67 &&
            (script_word(name, name_len, "then") || script_word(name, name_len, "catch") ||
             script_word(name, name_len, "finally"))) {
            e = script_promise_then(m, value->index, name, name_len, value);
            if (e)
                return e;
            continue;
        }
        if (m->work->live && script_is_collection(m, *value) &&
            script_find_prop(m, value->index, name, name_len) == SIZE_MAX) {
            e = script_collection_member(m, value->index, name, name_len, value);
            if (e)
                return e;
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT &&
            script_word(name, name_len, "hasOwnProperty") &&
            script_find_prop(m, value->index, name, name_len) == SIZE_MAX) {
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            e = script_arguments(m, args, &count);
            KaleidoscopeScriptValue key = {0};
            if (!e)
                e = script_to_text(m, args[0], &key);
            if (e)
                return e;
            ScriptSlice text = script_text(key);
            const uint8_t *bytes = m->work->arena + text.start;
            *value =
                script_bool(script_find_prop(m, value->index, bytes, text.len) != SIZE_MAX ||
                            script_find_accessor(m, value->index, bytes, text.len) != SIZE_MAX);
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
            uint8_t receiver = value->index;
            KaleidoscopeScriptValue property = {0};
            e = script_read_prop(m, receiver, m->source->bytes + name_start, name_len, &property);
            if (e)
                return e;
            if (property.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION && script_punct(m, '(')) {
                KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
                size_t count = 0;
                e = script_argument_list(m, args, &count);
                if (e)
                    return e;
                if (m->work->live) {
                    m->work->this_next = script_object(receiver);
                    m->work->this_next_present = true;
                    e = script_invoke(m, property.index, args, count, value);
                    if (e)
                        return e;
                } else
                    *value = script_undef();
            } else
                *value = property;
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER && to_string) {
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            e = script_arguments(m, args, &count);
            if (e)
                return e;
            double radix = count ? script_to_number(m, args[0]) : 10.0;
            e = script_number_radix(m, value->number, radix, value);
            if (e)
                return e;
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER &&
            (script_word(name, name_len, "toPrecision") ||
             script_word(name, name_len, "toLocaleString"))) {
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            e = script_arguments(m, args, &count);
            if (e)
                return e;
            if (script_word(name, name_len, "toLocaleString"))
                e = script_locale_number(m, value->number, value);
            else if (!count || args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED) {
                uint8_t text[64] = {0};
                size_t n = script_format_number(value->number, text);
                e = script_store(m, text, n, value);
            } else {
                double p = script_to_number(m, args[0]);
                if (!(p >= 1.0 && p <= 21.0) || p != script_floor(p))
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                e = script_precision(m, value->number, (size_t)p, value);
            }
            if (e)
                return e;
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER && to_fixed) {
            e = script_expect(m, '(');
            if (e)
                return e;
            KaleidoscopeScriptValue precision = script_num(0.0);
            if (!script_punct(m, ')')) {
                e = script_expr(m, &precision);
                if (e)
                    return e;
            }
            e = script_expect(m, ')');
            if (e)
                return e;
            double digits = script_to_number(m, precision);
            if (digits < 0.0 || digits > 20.0 || digits != script_floor(digits))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            e = script_fixed(m, value->number, (size_t)digits, value);
            if (e)
                return e;
            continue;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_REGEX) {
            uint8_t regex = value->index;
            if (script_word(name, name_len, "source")) {
                KaleidoscopeScriptRegex entry = m->work->regexes[regex];
                e = script_store(m, m->source->bytes + entry.pattern_start, entry.pattern_len,
                                 value);
                if (e)
                    return e;
                continue;
            }
            if (script_word(name, name_len, "flags")) {
                KaleidoscopeScriptRegex entry = m->work->regexes[regex];
                e = script_store(m, m->source->bytes + entry.flags_start, entry.flags_len, value);
                if (e)
                    return e;
                continue;
            }
            if (script_word(name, name_len, "lastIndex")) {
                *value = script_num((double)m->work->regexes[regex].last_index);
                continue;
            }
            if (script_word(name, name_len, "exec")) {
                e = script_regex_exec(m, regex, value);
                if (e)
                    return e;
                continue;
            }
            if (script_word(name, name_len, "test")) {
                KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
                size_t count = 0;
                e = script_arguments(m, args, &count);
                if (e)
                    return e;
                KaleidoscopeScriptValue text = count ? args[0] : script_undef();
                e = script_to_text(m, text, &text);
                if (e)
                    return e;
                ScriptSlice bytes = script_text(text);
                KaleidoscopeRegexFind found = {0};
                e = script_regex_find(m, regex, m->work->arena + bytes.start, bytes.len, 0, &found);
                if (e)
                    return e;
                *value = script_bool(found.matched);
                continue;
            }
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING &&
            (script_word(name, name_len, "match") || script_word(name, name_len, "matchAll") ||
             script_word(name, name_len, "search") || script_word(name, name_len, "replace") ||
             script_word(name, name_len, "replaceAll") || script_word(name, name_len, "split"))) {
            KaleidoscopeScriptPosition at = script_position_save(m);
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            e = script_arguments(m, args, &count);
            if (e)
                return e;
            if (count && args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_REGEX) {
                KaleidoscopeScriptValue argument = count > 1 ? args[1] : script_undef();
                e = script_string_regex(m, name, name_len, *value, args[0].index, argument, value);
                if (e)
                    return e;
                continue;
            }
            script_position_restore(m, at);
        }
        if (value->kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
            uint8_t id = value->index;
            KaleidoscopeScriptArray *array = &m->work->arrays[id];
            if (length) {
                *value = script_num((double)array->len);
                continue;
            }
            if (push) {
                e = script_expect(m, '(');
                if (e)
                    return e;
                KaleidoscopeScriptValue item = {0};
                e = script_expr(m, &item);
                if (e)
                    return e;
                e = script_expect(m, ')');
                if (e)
                    return e;
                if (m->work->live) {
                    e = script_push(m, id, item);
                    if (e)
                        return e;
                }
                *value = script_num((double)m->work->arrays[id].len);
                continue;
            }
            if (join) {
                e = script_expect(m, '(');
                if (e)
                    return e;
                KaleidoscopeScriptValue separator = script_undef();
                if (!script_punct(m, ')')) {
                    e = script_expr(m, &separator);
                    if (e)
                        return e;
                }
                e = script_expect(m, ')');
                if (e)
                    return e;
                if (separator.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED) {
                    static const uint8_t comma = ',';
                    e = script_join(m, id, &comma, 1, value);
                } else {
                    e = script_to_text(m, separator, &separator);
                    if (!e) {
                        ScriptSlice s = script_text(separator);
                        e = script_join(m, id, m->work->arena + s.start, s.len, value);
                    }
                }
                if (e)
                    return e;
                continue;
            }
            KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
            size_t count = 0;
            if (script_word(name, name_len, "forEach") || script_word(name, name_len, "map") ||
                script_word(name, name_len, "filter") || script_word(name, name_len, "find") ||
                script_word(name, name_len, "findIndex") ||
                script_word(name, name_len, "findLast") ||
                script_word(name, name_len, "findLastIndex") ||
                script_word(name, name_len, "flatMap") || script_word(name, name_len, "some") ||
                script_word(name, name_len, "every") || script_word(name, name_len, "reduce") ||
                script_word(name, name_len, "reduceRight")) {
                e = script_arguments(m, args, &count);
                if (e)
                    return e;
                e = script_array_callback(m, id, name, name_len, args, count, value);
                if (e)
                    return e;
                continue;
            }
            if (index_of || includes || slice || script_word(name, name_len, "lastIndexOf") ||
                script_word(name, name_len, "concat") || script_word(name, name_len, "reverse") ||
                script_word(name, name_len, "sort") || script_word(name, name_len, "pop") ||
                script_word(name, name_len, "shift") || script_word(name, name_len, "fill") ||
                script_word(name, name_len, "at") || script_word(name, name_len, "flat") ||
                script_word(name, name_len, "toString") || script_word(name, name_len, "splice") ||
                script_word(name, name_len, "unshift") || script_word(name, name_len, "keys") ||
                script_word(name, name_len, "values") || script_word(name, name_len, "entries") ||
                script_word(name, name_len, "toSorted") ||
                script_word(name, name_len, "toReversed") || script_word(name, name_len, "with") ||
                script_word(name, name_len, "toSpliced")) {
                e = script_arguments(m, args, &count);
                if (e)
                    return e;
                e = script_array_method(m, id, name, name_len, args, count, value);
                if (e)
                    return e;
                continue;
            }
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        bool extra =
            script_word(name, name_len, "substr") || script_word(name, name_len, "localeCompare");
        if (!(length || upper || lower || trim || trim_start || trim_end || at_method || index_of ||
              char_at || char_code || code_point || slice || substring || split || replace ||
              replace_all || starts || ends || includes || repeat || pad_start || pad_end ||
              last_index || extra)) {
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        KaleidoscopeScriptValue text = {0};
        e = script_to_text(m, *value, &text);
        if (e)
            return e;
        ScriptSlice bytes = script_text(text);
        if (length) {
            *value = script_num((double)bytes.len);
            continue;
        }
        if (upper || lower) {
            e = script_expect(m, '(');
            if (e)
                return e;
            e = script_expect(m, ')');
            if (e)
                return e;
            size_t first = m->work->arena_len;
            if (first + bytes.len > KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            for (size_t i = 0; i < bytes.len; i++) {
                uint8_t b = m->work->arena[bytes.start + i];
                if (upper && b >= 'a' && b <= 'z')
                    b = (uint8_t)(b - 'a' + 'A');
                if (lower && b >= 'A' && b <= 'Z')
                    b = (uint8_t)(b - 'A' + 'a');
                m->work->arena[m->work->arena_len++] = b;
            }
            *value = script_str(first, bytes.len);
            continue;
        }
        e = script_expect(m, '(');
        if (e)
            return e;
        KaleidoscopeScriptValue args[2] = {script_undef(), script_undef()};
        size_t n = 0;
        if (!script_punct(m, ')'))
            for (;;) {
                if (n == 2)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                e = script_expr(m, &args[n++]);
                if (e)
                    return e;
                if (!script_punct(m, ','))
                    break;
                e = script_next(m);
                if (e)
                    return e;
            }
        e = script_expect(m, ')');
        if (e)
            return e;
        if (extra)
            e = script_string_extra(m, name, name_len, text, args[0], args[1], value);
        else
            e = script_string_method(m, trim, trim_start, trim_end, at_method, index_of, char_at,
                                     char_code, code_point, slice, substring, split, replace,
                                     replace_all, starts, ends, includes, repeat, pad_start,
                                     pad_end, last_index, text, args, value);
        if (e)
            return e;
    }
}

static bool script_typeof_known(const ScriptMachine *m) {
    if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
        return true;
    if (script_find((ScriptMachine *)m, m->token.start, m->token.len))
        return true;
    if (script_ident(m, "NaN") || script_ident(m, "Infinity") || script_ident(m, "true") ||
        script_ident(m, "false") || script_ident(m, "null") || script_ident(m, "undefined") ||
        script_ident(m, "Math") || script_ident(m, "JSON") || script_ident(m, "Object") ||
        script_ident(m, "String") || script_ident(m, "Number") || script_ident(m, "Boolean") ||
        script_ident(m, "parseInt") || script_ident(m, "encodeURIComponent") ||
        script_ident(m, "decodeURIComponent"))
        return true;
    for (size_t i = 0; i < m->work->nfuncs; i++)
        if (script_function_name(m, &m->work->funcs[i], m->token.start, m->token.len))
            return true;
    return false;
}

static uint8_t script_step_name(ScriptMachine *m, size_t start, size_t len, bool up, bool prefix,
                                KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptVariable *slot = script_find(m, start, len);
    KaleidoscopeScriptValue old = slot ? slot->value : script_undef();
    if (!slot && !m->work->skim && m->work->live)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    double before = script_to_number(m, old), after = before + (up ? 1.0 : -1.0);
    if (m->work->live) {
        uint8_t e = script_assign(m, start, len, script_num(after), false);
        if (e)
            return e;
    }
    *out = script_num(prefix ? after : before);
    return 0;
}

static uint8_t script_unary(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    if (script_ident(m, "typeof")) {
        if (script_too_deep(m))
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        uint8_t e = script_next(m);
        if (e)
            return e;
        m->work->depth++;
        if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT && !script_typeof_known(m)) {
            e = script_next(m);
            *out = script_undef();
        } else
            e = script_unary(m, out);
        m->work->depth--;
        if (e)
            return e;
        static const uint8_t number[] = "number", string[] = "string", boolean[] = "boolean",
                             undefined[] = "undefined", object[] = "object",
                             function[] = "function";
        if (out->kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER)
            return script_store(m, number, 6, out);
        if (out->kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING)
            return script_store(m, string, 6, out);
        if (out->kind == KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN)
            return script_store(m, boolean, 7, out);
        if (out->kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED)
            return script_store(m, undefined, 9, out);
        if (out->kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            return script_store(m, function, 8, out);
        return script_store(m, object, 6, out);
    }
    if (script_punct(m, 'I') || script_punct(m, 'D')) {
        bool up = script_punct(m, 'I');
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        size_t start = m->token.start, len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
        return script_step_name(m, start, len, up, true, out);
    }
    if (script_ident(m, "void")) {
        if (script_too_deep(m))
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        uint8_t e = script_next(m);
        if (e)
            return e;
        m->work->depth++;
        e = script_unary(m, out);
        m->work->depth--;
        if (e)
            return e;
        *out = script_undef();
        return 0;
    }
    if (script_punct(m, '-') || script_punct(m, '+') || script_punct(m, '!') ||
        script_punct(m, '~')) {
        uint8_t op = m->token.punct;
        if (script_too_deep(m))
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        uint8_t e = script_next(m);
        if (e)
            return e;
        m->work->depth++;
        e = script_unary(m, out);
        m->work->depth--;
        if (e)
            return e;
        if (op == '!') {
            *out = script_bool(!script_truthy(*out));
            return 0;
        }
        double x = script_to_number(m, *out);
        *out = script_num(op == '-'   ? -x
                          : op == '~' ? script_i32_number(~(uint32_t)script_int32(x))
                                      : x);
        return 0;
    }
    uint8_t e = script_atom(m, out);
    return e ? e : script_postfix(m, out);
}

static uint8_t script_apply(ScriptMachine *m, uint8_t op, KaleidoscopeScriptValue left,
                            KaleidoscopeScriptValue right, KaleidoscopeScriptValue *out) {
    if (op == '|') {
        *out = script_truthy(left) ? left : right;
        return 0;
    }
    if (op == '&') {
        *out = script_truthy(left) ? right : left;
        return 0;
    }
    if (op == 'E' || op == 'N' || op == 'e' || op == 'n') {
        bool strict = op == 'E' || op == 'N', same = false;
        bool left_nullish = left.kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL ||
                            left.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED,
             right_nullish = right.kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL ||
                             right.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED;
        if (left_nullish || right_nullish)
            same = left_nullish && right_nullish && (!strict || left.kind == right.kind);
        else if (left.kind == right.kind) {
            if (left.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING)
                same = script_text_equal(m, left, right);
            else if (left.kind == KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN)
                same = left.boolean == right.boolean;
            else if (left.kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER)
                same = left.number == right.number;
            else if (left.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY ||
                     left.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ||
                     left.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
                same = left.index == right.index;
        } else if (!strict)
            same = script_to_number(m, left) == script_to_number(m, right);
        *out = script_bool(same == (op == 'E' || op == 'e'));
        return 0;
    }
    if (op == '<' || op == '>' || op == 'L' || op == 'G') {
        bool yes = false;
        if (left.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING &&
            right.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
            int order = script_text_compare(m, left, right);
            yes = op == '<'   ? order < 0
                  : op == '>' ? order > 0
                  : op == 'L' ? order <= 0
                              : order >= 0;
        } else {
            double a = script_to_number(m, left), b = script_to_number(m, right);
            yes = op == '<' ? a < b : op == '>' ? a > b : op == 'L' ? a <= b : a >= b;
        }
        *out = script_bool(yes);
        return 0;
    }
    if (op == '+' && (left.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING ||
                      right.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING)) {
        return script_concat(m, left, right, out);
    }
    if (op == 'a' || op == 'o' || op == '^' || op == 'l' || op == 'r' || op == 'u') {
        int32_t a = script_int32(script_to_number(m, left)),
                b = script_int32(script_to_number(m, right));
        uint32_t shift = (uint32_t)b & 31u, bits;
        if (op == 'a')
            bits = (uint32_t)a & (uint32_t)b;
        else if (op == 'o')
            bits = (uint32_t)a | (uint32_t)b;
        else if (op == '^')
            bits = (uint32_t)a ^ (uint32_t)b;
        else if (op == 'l')
            bits = (uint32_t)a << shift;
        else {
            bits = (uint32_t)a >> shift;
            if (op == 'r' && a < 0 && shift)
                bits |= UINT32_MAX << (32u - shift);
        }
        *out = script_num(op == 'u' ? (double)bits : script_i32_number(bits));
        return 0;
    }
    if (op == 'X') {
        *out = script_num(script_pow(script_to_number(m, left), script_to_number(m, right)));
        return 0;
    }
    double a = script_to_number(m, left), b = script_to_number(m, right);
    *out = script_num(op == '+'   ? a + b
                      : op == '-' ? a - b
                      : op == '*' ? a * b
                      : op == '/' ? a / b
                                  : script_rem(a, b));
    return 0;
}

static uint8_t script_binary(ScriptMachine *m, uint8_t minimum, KaleidoscopeScriptValue *out) {
    uint8_t e = script_unary(m, out);
    if (e)
        return e;
    for (;;) {
        if (minimum <= 8 && script_ident(m, "in")) {
            e = script_next(m);
            if (e)
                return e;
            KaleidoscopeScriptValue right = {0};
            e = script_binary(m, 9, &right);
            if (e)
                return e;
            KaleidoscopeScriptValue key = {0};
            e = script_to_text(m, *out, &key);
            if (e)
                return e;
            ScriptSlice text = script_text(key);
            bool present = false;
            if (right.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
                present = script_find_prop(m, right.index, m->work->arena + text.start, text.len) !=
                          SIZE_MAX;
            else if (right.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                size_t index = 0;
                bool valid = text.len > 0;
                for (size_t i = 0; i < text.len; i++) {
                    uint8_t c = m->work->arena[text.start + i];
                    if (c < '0' || c > '9' || index > (SIZE_MAX - (c - '0')) / 10) {
                        valid = false;
                        break;
                    }
                    index = index * 10 + (c - '0');
                }
                present = valid && index < m->work->arrays[right.index].len;
            } else
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            *out = script_bool(present);
            continue;
        }
        if (minimum <= 8 && script_ident(m, "instanceof")) {
            e = script_next(m);
            if (e)
                return e;
            if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            size_t start = m->token.start, len = m->token.len;
            e = script_next(m);
            if (e)
                return e;
            bool yes = 0;
            e = script_instance_of(m, *out, m->source->bytes + start, len, &yes);
            if (e)
                return e;
            *out = script_bool(yes);
            continue;
        }
        if (m->token.kind != KALEIDOSCOPE_TOKEN_PUNCT)
            return 0;
        uint8_t op = m->token.punct,
                precedence = op == '?'                                            ? 1
                             : (op == '|' || op == 'C')                           ? 2
                             : op == '&'                                          ? 3
                             : op == 'o'                                          ? 4
                             : op == '^'                                          ? 5
                             : op == 'a'                                          ? 6
                             : (op == 'E' || op == 'N' || op == 'e' || op == 'n') ? 7
                             : (op == '<' || op == '>' || op == 'L' || op == 'G') ? 8
                             : (op == 'l' || op == 'r' || op == 'u')              ? 9
                             : (op == '+' || op == '-')                           ? 10
                             : (op == '*' || op == '/' || op == '%')              ? 11
                             : op == 'X'                                          ? 12
                                                                                  : 0;
        if (!precedence || precedence < minimum)
            return 0;
        e = script_next(m);
        if (e)
            return e;
        if (op == '?') {
            bool take = script_truthy(*out), live = m->work->live;
            KaleidoscopeScriptValue yes = {0}, no = {0};
            m->work->live = live && take;
            e = script_expr(m, &yes);
            m->work->live = live;
            if (e)
                return e;
            e = script_expect(m, ':');
            if (e)
                return e;
            m->work->live = live && !take;
            e = script_binary(m, 1, &no);
            m->work->live = live;
            if (e)
                return e;
            *out = take ? yes : no;
            continue;
        }
        if (op == '|' || op == '&' || op == 'C') {
            bool decided = op == '|'   ? script_truthy(*out)
                           : op == '&' ? !script_truthy(*out)
                                       : !(out->kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
                                           out->kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL),
                 live = m->work->live;
            KaleidoscopeScriptValue right = {0};
            m->work->live = live && !decided;
            e = script_binary(m, (uint8_t)(precedence + 1), &right);
            m->work->live = live;
            if (e)
                return e;
            if (!decided)
                *out = right;
            continue;
        }
        KaleidoscopeScriptValue right = {0};
        e = script_binary(m, op == 'X' ? precedence : (uint8_t)(precedence + 1), &right);
        if (e)
            return e;
        e = script_apply(m, op, *out, right, out);
        if (e)
            return e;
    }
}

static uint8_t script_expr(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    if (script_too_deep(m))
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->depth++;
    /* An assignment is an expression too (`a = b = 0`, `x => t += x`); its value is the value
       assigned. (`n++` is handled in the atom.) */
    if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
        KaleidoscopeScriptPosition at = script_position_save(m);
        uint8_t e = script_next(m);
        bool assigns = false;
        if (!e)
            e = script_assigns_ahead_with(m, false, &assigns);
        script_position_restore(m, at);
        if (!e && assigns)
            e = script_assignment(m, m->work->live, out);
        if (e || assigns) {
            m->work->depth--;
            return e;
        }
    }
    uint8_t e = script_binary(m, 1, out);
    m->work->depth--;
    return e;
}

static uint8_t script_items(ScriptMachine *m, uint8_t close, KaleidoscopeScriptValue *out,
                            size_t capacity, size_t *count) {
    *count = 0;
    while (!script_punct(m, close)) {
        if (script_punct(m, 'S')) {
            uint8_t e = script_next(m);
            if (e)
                return e;
            KaleidoscopeScriptValue spread = {0};
            e = script_expr(m, &spread);
            if (e)
                return e;
            size_t top = m->work->argument_top;
            ScriptItems items = {0};
            e = script_iterate(m, spread, &items);
            if (!e && *count + items.count > capacity)
                e = KALEIDOSCOPE_SCRIPT_LIMIT;
            for (size_t i = 0; !e && i < items.count; i++)
                out[(*count)++] = script_item(m, items, i);
            m->work->argument_top = top;
            if (e)
                return e;
        } else {
            if (*count == capacity)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            uint8_t e = script_expr(m, &out[(*count)++]);
            if (e)
                return e;
        }
        if (!script_punct(m, ','))
            break;
        uint8_t e = script_next(m);
        if (e)
            return e;
    }
    return script_expect(m, close);
}

static KaleidoscopeScriptPosition script_position_save(const ScriptMachine *m) {
    KaleidoscopeScriptPosition position = {.lexer = m->work->lexer, .token = m->token};
    return position;
}

static void script_position_restore(ScriptMachine *m, KaleidoscopeScriptPosition position) {
    m->work->lexer = position.lexer;
    m->token = position.token;
}

static uint8_t script_statement(ScriptMachine *m, bool run);

static bool script_function_name(const ScriptMachine *m, const KaleidoscopeScriptFunction *function,
                                 size_t start, size_t len) {
    if (function->len != len)
        return false;
    for (size_t i = 0; i < len; i++)
        if (function->name[i] != m->source->bytes[start + i])
            return false;
    return true;
}

static uint8_t script_argument_list(ScriptMachine *m, KaleidoscopeScriptValue *args,
                                    size_t *count) {
    uint8_t e = script_expect(m, '(');
    if (e)
        return e;
    for (size_t i = 0; i < KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE; i++)
        args[i] = script_undef();
    return script_items(m, ')', args, KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT, count);
}

static uint8_t script_call(ScriptMachine *m, size_t name_start, size_t name_len,
                           KaleidoscopeScriptValue *out) {
    size_t function = SIZE_MAX;
    for (size_t i = 0; i < m->work->nfuncs; i++)
        if (m->work->funcs[i].len &&
            script_function_name(m, &m->work->funcs[i], name_start, name_len)) {
            function = i;
            break;
        }
    if (function == SIZE_MAX) {
        KaleidoscopeScriptVariable *variable = script_find(m, name_start, name_len);
        if (variable && variable->value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            function = variable->value.index;
    }
    if (function == SIZE_MAX && !m->work->skim)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_argument_list(m, args, &count);
    if (e)
        return e;
    if (function == SIZE_MAX || !m->work->live) {
        *out = script_undef();
        return 0;
    }
    return script_invoke(m, (uint8_t)function, args, count, out);
}

/* The declared function behind a closure or a `bind` result. */
static uint8_t script_real_func(const ScriptMachine *m, uint8_t function) {
    while (function >= KALEIDOSCOPE_SCRIPT_BOUND &&
           function - KALEIDOSCOPE_SCRIPT_BOUND < (int)m->work->bound_count)
        function = m->work->bounds[function - KALEIDOSCOPE_SCRIPT_BOUND].function;
    if (function >= KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE &&
        function < KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE + KALEIDOSCOPE_SCRIPT_CLOSURES)
        return m->work->closures[function - KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE].function;
    return function;
}

static uint8_t script_close_value(ScriptMachine *m, KaleidoscopeScriptValue value,
                                  bool *have_environment, uint8_t *environment,
                                  KaleidoscopeScriptValue *out) {
    if (value.kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION || !m->work->func_now_present ||
        value.index >= KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE || value.index == m->work->func_now) {
        *out = value;
        return 0;
    }
    size_t outer = m->work->funcs[m->work->func_now].position.lexer.at,
           inner = m->work->funcs[value.index].position.lexer.at;
    if (inner <= outer || inner >= m->work->lexer.at) {
        *out = value;
        return 0;
    }
    if (!*have_environment) {
        if (m->work->env_count >= KALEIDOSCOPE_SCRIPT_ENVS)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        size_t count = m->work->nvars - m->work->frame_base;
        if (count > KALEIDOSCOPE_SCRIPT_VARS_STORAGE)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        *environment = (uint8_t)m->work->env_count++;
        m->work->env_counts[*environment] = count;
        for (size_t i = 0; i < count; i++)
            m->work->env_vars[*environment][i] = m->work->vars[m->work->frame_base + i];
        *have_environment = true;
    }
    if (m->work->closure_count >= KALEIDOSCOPE_SCRIPT_CLOSURES) {
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    }
    size_t closure = m->work->closure_count++;
    m->work->closures[closure] =
        (KaleidoscopeScriptClosure){.function = value.index, .environment = *environment};
    *out = script_function((uint8_t)(KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE + closure));
    return 0;
}

static uint8_t script_escape(ScriptMachine *m, KaleidoscopeScriptValue value,
                             KaleidoscopeScriptValue *out) {
    if (!m->work->func_now_present) {
        *out = value;
        return 0;
    }
    bool have_environment = false;
    uint8_t environment = 0, e;
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
        return script_close_value(m, value, &have_environment, &environment, out);
    if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
        KaleidoscopeScriptArray record = m->work->objects[value.index];
        for (size_t i = 0; i < record.len; i++) {
            KaleidoscopeScriptProperty *property = &m->work->props[record.start + i];
            KaleidoscopeScriptValue closed = {0};
            e = script_close_value(m, property->value, &have_environment, &environment, &closed);
            if (e)
                return e;
            property->value = closed;
        }
    } else if (value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
        KaleidoscopeScriptArray array = m->work->arrays[value.index];
        for (size_t i = 0; i < array.len; i++) {
            KaleidoscopeScriptValue closed = {0};
            e = script_close_value(m, m->work->elems[array.start + i], &have_environment,
                                   &environment, &closed);
            if (e)
                return e;
            m->work->elems[array.start + i] = closed;
        }
    }
    *out = value;
    return 0;
}

/* Copies the properties of one object onto another. */
static uint8_t script_copy_props(ScriptMachine *m, uint8_t from, uint8_t to) {
    KaleidoscopeScriptArray record = m->work->objects[from];
    for (size_t k = 0; k < record.len; k++) {
        KaleidoscopeScriptProperty property = m->work->props[record.start + k];
        uint8_t e =
            script_set_prop(m, to, m->work->arena + property.start, property.len, property.value);
        if (e)
            return e;
    }
    return 0;
}

/* F.prototype: the object for f's declared function, made on first use. */
static uint8_t script_prototype(ScriptMachine *m, uint8_t function, uint8_t *out) {
    uint8_t real = script_real_func(m, function);
    for (size_t p = 0; p < m->work->proto_count; p++)
        if (m->work->proto_function[p] == real) {
            *out = m->work->proto_object[p];
            return 0;
        }
    if (m->work->proto_count == KALEIDOSCOPE_SCRIPT_PROTOS)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    uint8_t id = 0;
    uint8_t e = script_new_object(m, &id);
    if (e)
        return e;
    m->work->proto_function[m->work->proto_count] = real;
    m->work->proto_object[m->work->proto_count++] = id;
    *out = id;
    return 0;
}

/* `F.prototype = {...}`: replaces what new instances copy. */
static uint8_t script_set_prototype(ScriptMachine *m, uint8_t function,
                                    KaleidoscopeScriptValue value) {
    if (value.kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    uint8_t real = script_real_func(m, function);
    for (size_t p = 0; p < m->work->proto_count; p++)
        if (m->work->proto_function[p] == real) {
            m->work->proto_object[p] = value.index;
            return 0;
        }
    if (m->work->proto_count == KALEIDOSCOPE_SCRIPT_PROTOS)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->proto_function[m->work->proto_count] = real;
    m->work->proto_object[m->work->proto_count++] = value.index;
    return 0;
}

static bool script_frozen(const ScriptMachine *m, uint8_t id) {
    for (size_t i = 0; i < m->work->frozen_count; i++)
        if (m->work->frozen[i] == id)
            return true;
    return false;
}

static bool script_same_value(KaleidoscopeScriptValue a, KaleidoscopeScriptValue b) {
    if (a.kind != b.kind)
        return false;
    switch (a.kind) {
    case KALEIDOSCOPE_SCRIPT_VALUE_NUMBER:
        return a.number == b.number;
    case KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN:
        return a.boolean == b.boolean;
    case KALEIDOSCOPE_SCRIPT_VALUE_STRING:
        return a.start == b.start && a.len == b.len;
    case KALEIDOSCOPE_SCRIPT_VALUE_ARRAY:
    case KALEIDOSCOPE_SCRIPT_VALUE_OBJECT:
    case KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION:
        return a.index == b.index;
    case KALEIDOSCOPE_SCRIPT_VALUE_NULL:
    case KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED:
        return true;
    default:
        return false;
    }
}

/* Properties that `Object.keys`, `for...in` and their kin skip, as upstream's `hidden_prop`:
   private (`#name`) fields, values copied from F.prototype and unchanged, and class methods. */
static bool script_hidden_prop(const ScriptMachine *m, uint8_t id, size_t k) {
    KaleidoscopeScriptProperty property = m->work->props[k];
    if (property.len > 0 && m->work->arena[property.start] == '#')
        return true;
    uint16_t made = m->work->made_by[id];
    if (made >= 256) {
        uint8_t function = (uint8_t)(made - 256);
        for (size_t p = 0; p < m->work->proto_count; p++)
            if (m->work->proto_function[p] == function) {
                size_t q = script_find_prop(m, m->work->proto_object[p],
                                            m->work->arena + property.start, property.len);
                if (q != SIZE_MAX && script_same_value(m->work->props[q].value, property.value))
                    return true;
                break;
            }
    }
    if (property.value.kind != KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
        return false;
    size_t c = made >= 1 && made <= 16 ? (size_t)made - 1 : SIZE_MAX;
    while (c != SIZE_MAX && c < m->work->class_count) {
        KaleidoscopeScriptClass record = m->work->classes[c];
        for (size_t i = 0; i < record.method_count; i++) {
            KaleidoscopeScriptClassMethod method = m->work->class_methods[record.method_start + i];
            if (method.function != property.value.index || method.name_len != property.len)
                continue;
            bool same = true;
            for (size_t j = 0; same && j < property.len; j++)
                same =
                    m->source->bytes[method.name_start + j] == m->work->arena[property.start + j];
            if (same)
                return true;
        }
        c = record.parent_present ? record.parent : SIZE_MAX;
    }
    return false;
}

/* `arguments`: a new array of the running call's arguments. */
static uint8_t script_arguments_array(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    uint8_t id = 0;
    uint8_t e = script_allocate_array(m, m->work->args_now_count, &id);
    if (e)
        return e;
    for (size_t i = 0; i < m->work->args_now_count; i++)
        m->work->elems[m->work->arrays[id].start + i] =
            m->work->argument_stack[m->work->args_now_start + i];
    *out = script_array(id);
    return 0;
}

/* `f.prototype`, `f.call(this, ...)`, `f.apply(this, items)` and `f.bind(this, ...)`. */
static uint8_t script_function_member(ScriptMachine *m, uint8_t function, const uint8_t *name,
                                      size_t name_len, KaleidoscopeScriptValue *out) {
    if (script_word(name, name_len, "prototype")) {
        uint8_t id = 0;
        uint8_t e = script_prototype(m, function, &id);
        if (!e)
            *out = script_object(id);
        return e;
    }
    bool call = script_word(name, name_len, "call"), apply = script_word(name, name_len, "apply");
    if (!call && !apply && !script_word(name, name_len, "bind"))
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_arguments(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    KaleidoscopeScriptValue this_value = count ? args[0] : script_undef();
    if (call) {
        m->work->this_next = this_value;
        m->work->this_next_present = true;
        return script_invoke(m, function, count > 1 ? args + 1 : args, count > 1 ? count - 1 : 0,
                             out);
    }
    if (apply) {
        KaleidoscopeScriptValue list = args[1];
        if (list.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED ||
            list.kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL) {
            m->work->this_next = this_value;
            m->work->this_next_present = true;
            return script_invoke(m, function, args, 0, out);
        }
        if (list.kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        /* The items are copied out first: the call may change the array. */
        size_t top = m->work->argument_top, n = m->work->arrays[list.index].len;
        if (top + n > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t i = 0; i < n; i++)
            m->work->argument_stack[top + i] =
                m->work->elems[m->work->arrays[list.index].start + i];
        m->work->argument_top = top + n;
        m->work->this_next = this_value;
        m->work->this_next_present = true;
        e = script_invoke(m, function, m->work->argument_stack + top, n, out);
        m->work->argument_top = top;
        return e;
    }
    if (m->work->bound_count == KALEIDOSCOPE_SCRIPT_BOUNDS)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    KaleidoscopeScriptBound *bound = &m->work->bounds[m->work->bound_count];
    bound->function = function;
    bound->this_value = this_value;
    bound->count = (uint8_t)(count > 1 ? count - 1 : 0);
    for (size_t i = 0; i < bound->count; i++)
        bound->args[i] = args[1 + i];
    *out = script_function((uint8_t)(KALEIDOSCOPE_SCRIPT_BOUND + m->work->bound_count++));
    return 0;
}

static uint8_t script_invoke(ScriptMachine *m, uint8_t function,
                             const KaleidoscopeScriptValue *args, size_t count,
                             KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptValue receiver =
        m->work->this_next_present ? m->work->this_next : script_undef();
    m->work->this_next_present = false;
    if (function >= KALEIDOSCOPE_SCRIPT_RESOLVERS && function < KALEIDOSCOPE_SCRIPT_BUILTIN_FUNCS) {
        size_t k = function - KALEIDOSCOPE_SCRIPT_RESOLVERS;
        if (k >= m->work->resolver_count)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        KaleidoscopeScriptValue value = count ? args[0] : script_undef();
        uint8_t e = m->work->resolver_rejects[k]
                        ? script_settle(m, m->work->resolver_promise[k], 2, value)
                        : script_resolve(m, m->work->resolver_promise[k], value);
        *out = script_undef();
        return e;
    }
    if (function >= 250) {
        KaleidoscopeScriptValue value = count ? args[0] : script_undef();
        if (function == 250) {
            if (!count)
                return script_store(m, NULL, 0, out);
            return script_to_text(m, value, out);
        }
        if (function == 251) {
            *out = script_num(count ? script_to_number(m, value) : 0.0);
            return 0;
        }
        *out = script_bool(script_truthy(value));
        return 0;
    }
    if (script_too_deep(m))
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    m->work->depth++;
    if (function >= KALEIDOSCOPE_SCRIPT_BOUND &&
        function < KALEIDOSCOPE_SCRIPT_BOUND + KALEIDOSCOPE_SCRIPT_BOUNDS) {
        /* A bound function: its leading arguments, then these. As upstream, the call keeps the
           nesting level it took. */
        if (function - KALEIDOSCOPE_SCRIPT_BOUND >= (int)m->work->bound_count)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        KaleidoscopeScriptBound bound = m->work->bounds[function - KALEIDOSCOPE_SCRIPT_BOUND];
        size_t top = m->work->argument_top;
        if (top + bound.count + count > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        for (size_t i = 0; i < bound.count; i++)
            m->work->argument_stack[top + i] = bound.args[i];
        for (size_t i = 0; i < count; i++)
            m->work->argument_stack[top + bound.count + i] = args[i];
        m->work->argument_top = top + bound.count + count;
        m->work->this_next = bound.this_value;
        m->work->this_next_present = true;
        uint8_t e = script_invoke(m, bound.function, m->work->argument_stack + top,
                                  bound.count + count, out);
        m->work->argument_top = top;
        return e;
    }
    KaleidoscopeScriptPosition back = script_position_save(m);
    size_t base = m->work->nvars;
    bool environment_present = function >= KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE;
    size_t environment = 0;
    if (environment_present) {
        size_t closure = function - KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE;
        if (closure >= m->work->closure_count) {
            m->work->depth--;
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        environment = m->work->closures[closure].environment;
        function = m->work->closures[closure].function;
        size_t captured = m->work->env_counts[environment];
        if (m->work->nvars + captured > KALEIDOSCOPE_SCRIPT_VARS_LIMIT) {
            m->work->depth--;
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        }
        for (size_t i = 0; i < captured; i++)
            m->work->vars[m->work->nvars++] = m->work->env_vars[environment][i];
    }
    size_t outer_frame = m->work->frame_base;
    m->work->frame_base = base;
    script_position_restore(m, m->work->funcs[function].position);
    bool parens = script_punct(m, '(');
    uint8_t e = 0;
    if (parens)
        e = script_next(m);
    size_t index = 0;
    while (!e) {
        bool rest = parens && script_punct(m, 'S');
        if (rest) {
            e = script_next(m);
            if (e)
                break;
        }
        if (parens && !rest && (script_punct(m, '[') || script_punct(m, '{'))) {
            /* `({x, y = 2})` / `([k, v])`: the argument is destructured. */
            if (index == KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT) {
                e = KALEIDOSCOPE_SCRIPT_LIMIT;
                break;
            }
            e = script_pattern(m, true, index < count ? args[index] : script_undef());
            index++;
            if (e || !script_punct(m, ','))
                break;
            e = script_next(m);
            continue;
        }
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT) {
            if (rest)
                e = KALEIDOSCOPE_SCRIPT_SYNTAX;
            break;
        }
        if (index == KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT) {
            e = KALEIDOSCOPE_SCRIPT_LIMIT;
            break;
        }
        size_t name_start = m->token.start, name_len = m->token.len;
        e = script_next(m);
        if (e)
            break;
        KaleidoscopeScriptValue value = index < count ? args[index] : script_undef();
        if (rest) {
            e = script_new_array(m, index < count ? args + index : NULL,
                                 index < count ? count - index : 0, &value);
            if (e)
                break;
        } else if (parens && script_punct(m, '=')) {
            e = script_next(m);
            if (e)
                break;
            bool missing = value.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED, live = m->work->live;
            m->work->live = live && missing;
            KaleidoscopeScriptValue fallback = {0};
            e = script_expr(m, &fallback);
            m->work->live = live;
            if (e)
                break;
            if (missing)
                value = fallback;
        }
        e = script_declare(m, name_start, name_len, value, 0);
        index++;
        if (e || rest || !parens || !script_punct(m, ','))
            break;
        e = script_next(m);
    }
    if (!e && parens) {
        e = script_expect(m, ')');
    }
    if (!e && script_punct(m, 'F')) {
        e = script_next(m);
    }
    bool outer_present = m->work->ret_present, outer_func_present = m->work->func_now_present;
    KaleidoscopeScriptValue outer = m->work->ret, outer_this = m->work->this_value;
    uint8_t outer_loops = m->work->loops, outer_func = m->work->func_now;
    bool outer_live = m->work->live;
    m->work->ret_present = false;
    m->work->loops = 0;
    m->work->live = true;
    m->work->this_value = receiver;
    m->work->func_now_present = true;
    m->work->func_now = function;
    /* `arguments` reads this call's arguments, copied onto the argument stack. */
    size_t outer_args_start = m->work->args_now_start, outer_args_count = m->work->args_now_count,
           outer_top = m->work->argument_top;
    if (!e && outer_top + count > KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK)
        e = KALEIDOSCOPE_SCRIPT_LIMIT;
    if (!e) {
        for (size_t i = 0; i < count; i++)
            m->work->argument_stack[outer_top + i] = args[i];
        m->work->args_now_start = outer_top;
        m->work->args_now_count = count;
        m->work->argument_top = outer_top + count;
    }
    if (!e) {
        if (script_punct(m, '{')) {
            e = script_statement(m, true);
            *out = m->work->ret_present ? m->work->ret : script_undef();
        } else {
            e = script_expr(m, out);
            if (!e)
                e = script_escape(m, *out, out);
        }
    }
    if (environment_present) {
        size_t captured = m->work->env_counts[environment];
        for (size_t i = 0; i < captured; i++)
            m->work->env_vars[environment][i] = m->work->vars[base + i];
    }
    m->work->frame_base = outer_frame;
    m->work->args_now_start = outer_args_start;
    m->work->args_now_count = outer_args_count;
    m->work->argument_top = outer_top;
    m->work->func_now_present = outer_func_present;
    m->work->func_now = outer_func;
    m->work->this_value = outer_this;
    m->work->live = outer_live;
    m->work->loops = outer_loops;
    m->work->ret_present = outer_present;
    m->work->ret = outer;
    m->work->nvars = base;
    script_position_restore(m, back);
    m->work->depth--;
    return e;
}

static bool script_class_name(const ScriptMachine *m, const KaleidoscopeScriptClass *class_record,
                              const uint8_t *name, size_t name_len) {
    if (class_record->name_len != name_len)
        return false;
    for (size_t i = 0; i < name_len; i++)
        if (m->source->bytes[class_record->name_start + i] != name[i])
            return false;
    return true;
}

static uint8_t script_instance_of(ScriptMachine *m, KaleidoscopeScriptValue value,
                                  const uint8_t *name, size_t name_len, bool *out) {
    if (!m->work->live) {
        *out = false;
        return 0;
    }
    uint16_t made =
        value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ? m->work->made_by[value.index] : 0;
    if (script_word(name, name_len, "Set") || script_word(name, name_len, "Map") ||
        script_word(name, name_len, "Promise")) {
        *out = made == (script_word(name, name_len, "Set")   ? 65
                        : script_word(name, name_len, "Map") ? 66
                                                             : 67);
        return 0;
    }
    if (script_word(name, name_len, "Array")) {
        *out = value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY;
        return 0;
    }
    if (script_word(name, name_len, "Object")) {
        *out = value.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ||
               value.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY ||
               value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION;
        return 0;
    }
    if (script_word(name, name_len, "Function")) {
        *out = value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION;
        return 0;
    }
    if (script_word(name, name_len, "Error")) {
        *out = made == 64;
        return 0;
    }
    if (script_word(name, name_len, "TypeError") || script_word(name, name_len, "RangeError") ||
        script_word(name, name_len, "SyntaxError")) {
        static const uint8_t key[] = "name";
        if (made != 64 || value.kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
            *out = false;
            return 0;
        }
        KaleidoscopeScriptValue kind = script_get_prop(m, value.index, key, 4);
        *out = kind.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING && kind.len == name_len;
        for (size_t i = 0; *out && i < name_len; i++)
            *out = m->work->arena[kind.start + i] == name[i];
        return 0;
    }
    for (size_t wanted = 0; wanted < m->work->class_count; wanted++)
        if (script_class_name(m, &m->work->classes[wanted], name, name_len)) {
            size_t at = made >= 1 && made <= 16 ? (size_t)made - 1 : SIZE_MAX;
            while (at != SIZE_MAX) {
                if (at == wanted) {
                    *out = true;
                    return 0;
                }
                KaleidoscopeScriptClass record = m->work->classes[at];
                at = record.parent_present ? record.parent : SIZE_MAX;
            }
            *out = false;
            return 0;
        }
    size_t function = SIZE_MAX;
    for (size_t i = 0; i < m->work->nfuncs; i++)
        if (m->work->funcs[i].len &&
            script_function_name(m, &m->work->funcs[i], (size_t)(name - m->source->bytes),
                                 name_len)) {
            function = i;
            break;
        }
    if (function == SIZE_MAX) {
        KaleidoscopeScriptVariable *variable =
            script_find(m, (size_t)(name - m->source->bytes), name_len);
        if (variable && variable->value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            function = variable->value.index;
    }
    if (function == SIZE_MAX)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    *out = made == 256 + script_real_func(m, (uint8_t)function);
    return 0;
}

static uint8_t script_class_static_call(ScriptMachine *m, size_t class_index,
                                        KaleidoscopeScriptValue *out) {
    uint8_t e = script_expect(m, '.');
    if (e)
        return e;
    if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
        return KALEIDOSCOPE_SCRIPT_SYNTAX;
    size_t start = m->token.start, len = m->token.len;
    e = script_next(m);
    if (e)
        return e;
    if (!script_punct(m, '(')) {
        /* `C.x`: a static field (inherited ones too). */
        if (!m->work->live) {
            *out = script_undef();
            return 0;
        }
        uint8_t home = script_static_home(m, class_index, m->source->bytes + start, len);
        *out = script_get_prop(m, home, m->source->bytes + start, len);
        return 0;
    }
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    e = script_argument_list(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    for (size_t at = class_index;;) {
        KaleidoscopeScriptClass record = m->work->classes[at];
        for (size_t i = 0; i < record.static_count; i++) {
            KaleidoscopeScriptClassMethod method =
                m->work->class_static_methods[record.static_start + i];
            if (method.name_len != len)
                continue;
            bool same = true;
            for (size_t k = 0; k < len; k++)
                if (m->source->bytes[method.name_start + k] != m->source->bytes[start + k])
                    same = false;
            if (same)
                return script_invoke(m, method.function, args, count, out);
        }
        if (!record.parent_present)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        at = record.parent;
    }
}

/* Evaluates the expression at `at` with `this`, then returns to where it was, as upstream's
   `eval_at`. */
static uint8_t script_eval_at(ScriptMachine *m, KaleidoscopeScriptPosition at,
                              KaleidoscopeScriptValue this_value, KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptPosition back = script_position_save(m);
    KaleidoscopeScriptValue saved = m->work->this_value;
    bool live = m->work->live;
    m->work->this_value = this_value;
    m->work->live = true;
    script_position_restore(m, at);
    uint8_t e = script_expr(m, out);
    script_position_restore(m, back);
    m->work->this_value = saved;
    m->work->live = live;
    return e;
}

/* The static object holding C.key: C's own or the nearest ancestor's that has it (C's own when
   none has). */
static uint8_t script_static_home(const ScriptMachine *m, size_t class_index, const uint8_t *key,
                                  size_t key_len) {
    size_t k = class_index;
    bool searching = true;
    uint8_t home = m->work->classes[class_index].static_object;
    while (searching) {
        KaleidoscopeScriptClass record = m->work->classes[k];
        if (script_find_prop(m, record.static_object, key, key_len) != SIZE_MAX) {
            home = record.static_object;
            searching = false;
        } else if (!record.parent_present)
            searching = false;
        else
            k = record.parent;
    }
    return home;
}

static uint8_t script_install_class_methods(ScriptMachine *m, size_t class_index, uint8_t object) {
    KaleidoscopeScriptClass class_record = m->work->classes[class_index];
    uint8_t e = 0;
    if (class_record.parent_present) {
        e = script_install_class_methods(m, class_record.parent, object);
        if (e)
            return e;
    }
    for (size_t i = 0; i < class_record.method_count; i++) {
        KaleidoscopeScriptClassMethod method =
            m->work->class_methods[class_record.method_start + i];
        e = script_set_prop(m, object, m->source->bytes + method.name_start, method.name_len,
                            script_function(method.function));
        if (e)
            return e;
    }
    for (size_t i = 0; i < class_record.accessor_count; i++) {
        KaleidoscopeScriptClassAccessor accessor =
            m->work->class_accessors[class_record.accessor_start + i];
        e = script_add_accessor(m, object, true, accessor.name_start, accessor.name_len,
                                accessor.function, accessor.setter);
        if (e)
            return e;
    }
    /* Instance fields, initialized with `this` = the instance. */
    for (size_t i = 0; i < class_record.field_count; i++) {
        KaleidoscopeScriptClassField field = m->work->class_fields[class_record.field_start + i];
        KaleidoscopeScriptValue value = script_undef();
        if (field.init_present) {
            e = script_eval_at(m, field.init, script_object(object), &value);
            if (e)
                return e;
        }
        e = script_set_prop(m, object, m->source->bytes + field.name_start, field.name_len, value);
        if (e)
            return e;
    }
    return 0;
}

static uint8_t script_class_init(ScriptMachine *m, size_t class_index,
                                 KaleidoscopeScriptValue receiver,
                                 const KaleidoscopeScriptValue *args, size_t count,
                                 KaleidoscopeScriptValue *out) {
    KaleidoscopeScriptClass class_record = m->work->classes[class_index];
    if (class_record.constructor_present) {
        m->work->this_next = receiver;
        m->work->this_next_present = true;
        return script_invoke(m, class_record.constructor, args, count, out);
    }
    if (class_record.parent_present)
        return script_class_init(m, class_record.parent, receiver, args, count, out);
    *out = script_undef();
    return 0;
}

static uint8_t script_super_call(ScriptMachine *m, KaleidoscopeScriptValue *out) {
    size_t owner = SIZE_MAX;
    if (m->work->func_now_present)
        for (size_t c = 0; c < m->work->class_count && owner == SIZE_MAX; c++) {
            KaleidoscopeScriptClass record = m->work->classes[c];
            if (record.constructor_present && record.constructor == m->work->func_now)
                owner = c;
            for (size_t i = 0; i < record.method_count; i++)
                if (m->work->class_methods[record.method_start + i].function == m->work->func_now)
                    owner = c;
        }
    bool method = false;
    size_t name_start = 0, name_len = 0;
    if (script_punct(m, '.')) {
        method = true;
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        name_start = m->token.start;
        name_len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
    }
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE] = {0};
    size_t count = 0;
    uint8_t e = script_argument_list(m, args, &count);
    if (e)
        return e;
    if (!m->work->live) {
        *out = script_undef();
        return 0;
    }
    if (owner == SIZE_MAX || !m->work->classes[owner].parent_present)
        return KALEIDOSCOPE_SCRIPT_SYNTAX;
    size_t parent = m->work->classes[owner].parent;
    if (!method) {
        e = script_class_init(m, parent, m->work->this_value, args, count, out);
        if (e)
            return e;
        *out = script_undef();
        return 0;
    }
    for (;;) {
        KaleidoscopeScriptClass record = m->work->classes[parent];
        for (size_t i = 0; i < record.method_count; i++) {
            KaleidoscopeScriptClassMethod candidate =
                m->work->class_methods[record.method_start + i];
            if (candidate.name_len == name_len) {
                bool same = true;
                for (size_t k = 0; k < name_len; k++)
                    if (m->source->bytes[candidate.name_start + k] !=
                        m->source->bytes[name_start + k])
                        same = false;
                if (same) {
                    m->work->this_next = m->work->this_value;
                    m->work->this_next_present = true;
                    return script_invoke(m, candidate.function, args, count, out);
                }
            }
        }
        if (!record.parent_present)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        parent = record.parent;
    }
}

static uint8_t script_date_new(ScriptMachine *m, const KaleidoscopeScriptValue *args, size_t count,
                               KaleidoscopeScriptValue *out) {
    if (!count)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    double milliseconds = 0;
    if (count == 1 && args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER)
        milliseconds = args[0].number;
    else if (count == 1 && args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
        ScriptSlice text = script_text(args[0]);
        if (!script_parse_iso(m->work->arena + text.start, text.len, &milliseconds))
            milliseconds = script_nan();
    } else {
        double values[7] = {0, 0, 1, 0, 0, 0, 0};
        for (size_t i = 0; i < count && i < 7; i++)
            values[i] = script_to_number(m, args[i]);
        milliseconds = script_date_ms(values);
    }
    uint8_t id = 0;
    uint8_t e = script_new_object(m, &id);
    if (e)
        return e;
    static const uint8_t key[] = "__date";
    e = script_set_prop(m, id, key, 6, script_num(milliseconds));
    if (e)
        return e;
    *out = script_object(id);
    return 0;
}

static uint8_t script_construct(ScriptMachine *m, const uint8_t *name, size_t name_len,
                                const KaleidoscopeScriptValue *args, size_t count,
                                KaleidoscopeScriptValue *out) {
    if (script_word(name, name_len, "Array")) {
        if (count == 1 && args[0].kind == KALEIDOSCOPE_SCRIPT_VALUE_NUMBER) {
            double n = args[0].number;
            if (n < 0.0 || n > (double)KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT || n != script_floor(n))
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            uint8_t id = 0;
            uint8_t e = script_allocate_array(m, (size_t)n, &id);
            if (e)
                return e;
            *out = script_array(id);
            return 0;
        }
        return script_new_array(m, args, count, out);
    }
    if (script_word(name, name_len, "Date"))
        return script_date_new(m, args, count, out);
    if (script_word(name, name_len, "Promise"))
        return script_promise_construct(m, args, count, out);
    if (script_word(name, name_len, "Set") || script_word(name, name_len, "Map"))
        return script_collection_new(m, script_word(name, name_len, "Map"),
                                     count ? args[0] : script_undef(), out);
    uint8_t id = 0;
    uint8_t e = script_new_object(m, &id);
    if (e)
        return e;
    if (script_word(name, name_len, "Object")) {
        *out = script_object(id);
        return 0;
    }
    if (script_word(name, name_len, "Error") || script_word(name, name_len, "TypeError") ||
        script_word(name, name_len, "RangeError") || script_word(name, name_len, "SyntaxError")) {
        m->work->made_by[id] = 64;
        static const uint8_t name_key[] = "name", message_key[] = "message";
        KaleidoscopeScriptValue class_text = {0};
        e = script_store(m, name, name_len, &class_text);
        if (e)
            return e;
        e = script_set_prop(m, id, name_key, 4, class_text);
        if (e)
            return e;
        KaleidoscopeScriptValue message = {0};
        if (count)
            e = script_to_text(m, args[0], &message);
        else
            e = script_store(m, NULL, 0, &message);
        if (e)
            return e;
        e = script_set_prop(m, id, message_key, 7, message);
        if (e)
            return e;
        *out = script_object(id);
        return 0;
    }
    for (size_t c = 0; c < m->work->class_count; c++)
        if (script_class_name(m, &m->work->classes[c], name, name_len)) {
            m->work->made_by[id] = (uint16_t)(1 + c);
            e = script_install_class_methods(m, c, id);
            if (e)
                return e;
            KaleidoscopeScriptValue result = {0};
            e = script_class_init(m, c, script_object(id), args, count, &result);
            if (e)
                return e;
            *out = result.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ? result : script_object(id);
            return 0;
        }
    size_t function = SIZE_MAX;
    for (size_t i = 0; i < m->work->nfuncs; i++)
        if (m->work->funcs[i].len &&
            script_function_name(m, &m->work->funcs[i], (size_t)(name - m->source->bytes),
                                 name_len)) {
            function = i;
            break;
        }
    if (function == SIZE_MAX) {
        size_t start = (size_t)(name - m->source->bytes);
        KaleidoscopeScriptVariable *variable = script_find(m, start, name_len);
        if (variable && variable->value.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION)
            function = variable->value.index;
    }
    if (function == SIZE_MAX)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    uint8_t real = script_real_func(m, (uint8_t)function);
    m->work->made_by[id] = (uint16_t)(256 + real);
    /* A snapshot of F.prototype at construction (later additions do not reach existing
       instances). */
    for (size_t p = 0; p < m->work->proto_count; p++)
        if (m->work->proto_function[p] == real) {
            e = script_copy_props(m, m->work->proto_object[p], id);
            if (e)
                return e;
            break;
        }
    m->work->this_next = script_object(id);
    m->work->this_next_present = true;
    KaleidoscopeScriptValue result = {0};
    e = script_invoke(m, (uint8_t)function, args, count, &result);
    if (e)
        return e;
    *out = result.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT ? result : script_object(id);
    return 0;
}

static size_t script_append_padded_uint(uint8_t *text, size_t at, uint64_t value, size_t width) {
    uint8_t digits[32] = {0};
    size_t n = script_uint(value, digits);
    for (size_t i = n; i < width; i++)
        text[at++] = '0';
    for (size_t i = 0; i < n; i++)
        text[at++] = digits[i];
    return at;
}

static uint8_t script_date_method(ScriptMachine *m, double milliseconds, const uint8_t *name,
                                  size_t name_len, KaleidoscopeScriptValue *out) {
    if (script_isnan(milliseconds)) {
        if (script_word(name, name_len, "toISOString"))
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        *out = script_num(script_nan());
        return 0;
    }
    double days_f = script_floor(milliseconds / 86400000.0),
           remainder = milliseconds - days_f * 86400000.0;
    int64_t year = 0, month = 0, day = 0;
    script_civil((int64_t)days_f, &year, &month, &day);
    int64_t hour = (int64_t)(remainder / 3600000.0), minute = (int64_t)(remainder / 60000.0) % 60,
            second = (int64_t)(remainder / 1000.0) % 60, milli = (int64_t)remainder % 1000,
            weekday = (((int64_t)days_f % 7) + 11) % 7;
    if (script_word(name, name_len, "getTime") || script_word(name, name_len, "valueOf")) {
        *out = script_num(milliseconds);
        return 0;
    }
    if (script_word(name, name_len, "getFullYear") ||
        script_word(name, name_len, "getUTCFullYear")) {
        *out = script_num((double)year);
        return 0;
    }
    if (script_word(name, name_len, "getMonth") || script_word(name, name_len, "getUTCMonth")) {
        *out = script_num((double)(month - 1));
        return 0;
    }
    if (script_word(name, name_len, "getDate") || script_word(name, name_len, "getUTCDate")) {
        *out = script_num((double)day);
        return 0;
    }
    if (script_word(name, name_len, "getDay") || script_word(name, name_len, "getUTCDay")) {
        *out = script_num((double)weekday);
        return 0;
    }
    if (script_word(name, name_len, "getHours") || script_word(name, name_len, "getUTCHours")) {
        *out = script_num((double)hour);
        return 0;
    }
    if (script_word(name, name_len, "getMinutes") || script_word(name, name_len, "getUTCMinutes")) {
        *out = script_num((double)minute);
        return 0;
    }
    if (script_word(name, name_len, "getSeconds") || script_word(name, name_len, "getUTCSeconds")) {
        *out = script_num((double)second);
        return 0;
    }
    if (script_word(name, name_len, "getMilliseconds") ||
        script_word(name, name_len, "getUTCMilliseconds")) {
        *out = script_num((double)milli);
        return 0;
    }
    uint8_t text[64] = {0};
    size_t at = 0;
    if (script_word(name, name_len, "toISOString") || script_word(name, name_len, "toJSON")) {
        at = script_append_padded_uint(text, at, (uint64_t)year, 4);
        text[at++] = '-';
        at = script_append_padded_uint(text, at, (uint64_t)month, 2);
        text[at++] = '-';
        at = script_append_padded_uint(text, at, (uint64_t)day, 2);
        text[at++] = 'T';
        at = script_append_padded_uint(text, at, (uint64_t)hour, 2);
        text[at++] = ':';
        at = script_append_padded_uint(text, at, (uint64_t)minute, 2);
        text[at++] = ':';
        at = script_append_padded_uint(text, at, (uint64_t)second, 2);
        text[at++] = '.';
        at = script_append_padded_uint(text, at, (uint64_t)milli, 3);
        text[at++] = 'Z';
        return script_store(m, text, at, out);
    }
    if (script_word(name, name_len, "toDateString")) {
        static const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        for (size_t i = 0; i < 3; i++)
            text[at++] = (uint8_t)days[weekday][i];
        text[at++] = ' ';
        for (size_t i = 0; i < 3; i++)
            text[at++] = (uint8_t)months[month - 1][i];
        text[at++] = ' ';
        at = script_append_padded_uint(text, at, (uint64_t)day, 2);
        text[at++] = ' ';
        at = script_append_padded_uint(text, at, (uint64_t)year, 4);
        return script_store(m, text, at, out);
    }
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

static void script_json_ws(const uint8_t *text, size_t length, size_t *at) {
    while (*at < length &&
           (text[*at] == ' ' || text[*at] == '\t' || text[*at] == '\n' || text[*at] == '\r'))
        (*at)++;
}

static uint8_t script_json_parse(ScriptMachine *m, const uint8_t *text, size_t length, size_t *at,
                                 uint8_t depth, KaleidoscopeScriptValue *out) {
    if (depth > 16)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    script_json_ws(text, length, at);
    if (*at >= length)
        return KALEIDOSCOPE_SCRIPT_SYNTAX;
    uint8_t c = text[*at];
    if (c == '{') {
        (*at)++;
        uint8_t id = 0;
        uint8_t e = script_new_object(m, &id);
        if (e)
            return e;
        script_json_ws(text, length, at);
        if (*at < length && text[*at] == '}') {
            (*at)++;
            *out = script_object(id);
            return 0;
        }
        for (;;) {
            KaleidoscopeScriptValue key = {0}, value = {0};
            e = script_json_parse(m, text, length, at, (uint8_t)(depth + 1), &key);
            if (e)
                return e;
            if (key.kind != KALEIDOSCOPE_SCRIPT_VALUE_STRING)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            script_json_ws(text, length, at);
            if (*at >= length || text[*at] != ':')
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            (*at)++;
            e = script_json_parse(m, text, length, at, (uint8_t)(depth + 1), &value);
            if (e)
                return e;
            ScriptSlice name = script_text(key);
            e = script_set_prop(m, id, m->work->arena + name.start, name.len, value);
            if (e)
                return e;
            script_json_ws(text, length, at);
            if (*at < length && text[*at] == ',')
                (*at)++;
            else if (*at < length && text[*at] == '}') {
                (*at)++;
                *out = script_object(id);
                return 0;
            } else
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
        }
    }
    if (c == '[') {
        (*at)++;
        KaleidoscopeScriptValue array = {0};
        uint8_t e = script_new_array(m, NULL, 0, &array);
        if (e)
            return e;
        script_json_ws(text, length, at);
        if (*at < length && text[*at] == ']') {
            (*at)++;
            *out = array;
            return 0;
        }
        for (;;) {
            KaleidoscopeScriptValue value = {0};
            e = script_json_parse(m, text, length, at, (uint8_t)(depth + 1), &value);
            if (e)
                return e;
            e = script_push(m, array.index, value);
            if (e)
                return e;
            script_json_ws(text, length, at);
            if (*at < length && text[*at] == ',')
                (*at)++;
            else if (*at < length && text[*at] == ']') {
                (*at)++;
                *out = array;
                return 0;
            } else
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
        }
    }
    if (c == '"') {
        (*at)++;
        size_t first = m->work->arena_len;
        for (;;) {
            if (*at >= length)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            uint8_t byte = text[(*at)++];
            if (byte == '"') {
                *out = script_str(first, m->work->arena_len - first);
                return 0;
            }
            if (byte == '\\') {
                if (*at >= length)
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
                uint8_t escaped = text[(*at)++];
                if (escaped == '"' || escaped == '\\' || escaped == '/')
                    byte = escaped;
                else if (escaped == 'b')
                    byte = 8;
                else if (escaped == 'f')
                    byte = 12;
                else if (escaped == 'n')
                    byte = '\n';
                else if (escaped == 'r')
                    byte = '\r';
                else if (escaped == 't')
                    byte = '\t';
                else if (escaped == 'u') {
                    if (*at + 4 > length)
                        return KALEIDOSCOPE_SCRIPT_SYNTAX;
                    uint32_t code = 0;
                    for (size_t i = 0; i < 4; i++) {
                        int d = script_hex_digit(text[*at + i]);
                        if (d < 0)
                            return KALEIDOSCOPE_SCRIPT_SYNTAX;
                        code = code * 16u + (uint32_t)d;
                    }
                    *at += 4;
                    if (code >= 0xd800u && code <= 0xdfffu)
                        code = 0xfffdu;
                    uint8_t append = script_append_codepoint(m, code);
                    if (append)
                        return append;
                    continue;
                } else
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
            } else if (byte <= 31)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            if (m->work->arena_len >= KALEIDOSCOPE_SCRIPT_ARENA_LIMIT)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            m->work->arena[m->work->arena_len++] = byte;
        }
    }
    if (c == 't' && *at + 4 <= length && !__builtin_memcmp(text + *at, "true", 4)) {
        *at += 4;
        *out = script_bool(true);
        return 0;
    }
    if (c == 'f' && *at + 5 <= length && !__builtin_memcmp(text + *at, "false", 5)) {
        *at += 5;
        *out = script_bool(false);
        return 0;
    }
    if (c == 'n' && *at + 4 <= length && !__builtin_memcmp(text + *at, "null", 4)) {
        *at += 4;
        *out = script_null();
        return 0;
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        size_t start = *at;
        while (*at < length) {
            uint8_t d = text[*at];
            if (!((d >= '0' && d <= '9') || d == '-' || d == '+' || d == '.' || d == 'e' ||
                  d == 'E'))
                break;
            (*at)++;
        }
        double number = 0;
        if (!script_parse_text_number(text + start, *at - start, &number))
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        *out = script_num(number);
        return 0;
    }
    return KALEIDOSCOPE_SCRIPT_SYNTAX;
}

static uint8_t script_define(ScriptMachine *m, const uint8_t *name, size_t name_len,
                             KaleidoscopeScriptPosition position, uint8_t *function) {
    for (size_t i = 0; i < m->work->nfuncs; i++)
        if (m->work->funcs[i].position.lexer.at == position.lexer.at) {
            *function = (uint8_t)i;
            return 0;
        }
    if (m->work->nfuncs >= KALEIDOSCOPE_SCRIPT_FUNCS_LIMIT || name_len > KALEIDOSCOPE_SCRIPT_NAME)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    KaleidoscopeScriptFunction *slot = &m->work->funcs[m->work->nfuncs];
    slot->len = (uint8_t)name_len;
    for (size_t i = 0; i < name_len; i++)
        slot->name[i] = name[i];
    slot->position = position;
    *function = (uint8_t)m->work->nfuncs++;
    return 0;
}

static uint8_t script_skim_body(ScriptMachine *m) {
    bool skim = m->work->skim, live = m->work->live;
    m->work->skim = true;
    m->work->live = false;
    KaleidoscopeScriptValue ignored = {0};
    uint8_t e = script_punct(m, '{') ? script_statement(m, false) : script_expr(m, &ignored);
    m->work->skim = skim;
    m->work->live = live;
    return e;
}

static uint8_t script_skip_params(ScriptMachine *m) {
    for (;;) {
        if (script_punct(m, 'S')) {
            uint8_t e = script_next(m);
            if (e)
                return e;
            if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            return script_next(m);
        }
        uint8_t e = 0;
        if (script_punct(m, '[') || script_punct(m, '{'))
            e = script_pattern(m, false, script_undef());
        else if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return 0;
        else
            e = script_next(m);
        if (e)
            return e;
        if (script_punct(m, '=')) {
            e = script_next(m);
            if (e)
                return e;
            bool live = m->work->live;
            m->work->live = false;
            KaleidoscopeScriptValue ignored = {0};
            e = script_expr(m, &ignored);
            m->work->live = live;
            if (e)
                return e;
        }
        if (!script_punct(m, ','))
            return 0;
        e = script_next(m);
        if (e)
            return e;
    }
}

static uint8_t script_arrow_ahead(ScriptMachine *m, bool *arrow) {
    KaleidoscopeScriptPosition at = script_position_save(m);
    unsigned depth = 1;
    uint8_t e = 0;
    while (depth && !e) {
        e = script_next(m);
        if (e)
            break;
        if (script_punct(m, '('))
            depth++;
        else if (script_punct(m, ')'))
            depth--;
        else if (m->token.kind == KALEIDOSCOPE_TOKEN_END)
            break;
    }
    *arrow = false;
    if (!e && !depth) {
        e = script_next(m);
        *arrow = !e && script_punct(m, 'F');
    }
    script_position_restore(m, at);
    return e;
}

static uint8_t script_emit(ScriptMachine *m, KaleidoscopeScriptValue value) {
    uint8_t e = script_to_text(m, value, &value);
    if (e)
        return e;
    ScriptSlice s = script_text(value);
    if (m->work->output_len + s.len > KALEIDOSCOPE_SCRIPT_MAX_OUTPUT)
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    for (size_t i = 0; i < s.len; i++)
        m->work->output[m->work->output_len++] = m->work->arena[s.start + i];
    return 0;
}

static uint8_t script_end(ScriptMachine *m) {
    if (script_punct(m, ';'))
        return script_next(m);
    if (m->token.kind == KALEIDOSCOPE_TOKEN_END || script_punct(m, '}') || m->token.newline)
        return 0;
    return KALEIDOSCOPE_SCRIPT_SYNTAX;
}

static bool script_reserved_name(const ScriptMachine *m) {
    return script_ident(m, "var") || script_ident(m, "let") || script_ident(m, "const") ||
           script_ident(m, "document") || script_ident(m, "if") || script_ident(m, "else") ||
           script_ident(m, "while") || script_ident(m, "for") || script_ident(m, "function") ||
           script_ident(m, "return") || script_ident(m, "break") || script_ident(m, "continue") ||
           script_ident(m, "do") || script_ident(m, "switch") || script_ident(m, "case") ||
           script_ident(m, "default") || script_ident(m, "in") || script_ident(m, "delete") ||
           script_ident(m, "Array") || script_ident(m, "this") || script_ident(m, "throw") ||
           script_ident(m, "try") || script_ident(m, "catch") || script_ident(m, "finally") ||
           script_ident(m, "new") || script_ident(m, "class") || script_ident(m, "Date") ||
           script_ident(m, "JSON") || script_ident(m, "Object") || script_ident(m, "null") ||
           script_ident(m, "undefined") || script_ident(m, "typeof") || script_ident(m, "Math") ||
           script_ident(m, "true") || script_ident(m, "false");
}

static uint8_t script_update(ScriptMachine *m, KaleidoscopeScriptValue old,
                             KaleidoscopeScriptValue *out) {
    uint8_t e = 0;
    if (script_punct(m, '=')) {
        e = script_next(m);
        if (!e)
            e = script_expr(m, out);
    } else if (script_punct(m, 'P') || script_punct(m, 'M') || script_punct(m, 'T') ||
               script_punct(m, 'Q') || script_punct(m, 'R') || script_punct(m, 'W') ||
               script_punct(m, 'v') || script_punct(m, 'w') || script_punct(m, 'x')) {
        uint8_t op = m->token.punct;
        e = script_next(m);
        KaleidoscopeScriptValue right = {0};
        if (!e)
            e = script_expr(m, &right);
        uint8_t binary = op == 'P'   ? '+'
                         : op == 'M' ? '-'
                         : op == 'T' ? '*'
                         : op == 'Q' ? '/'
                         : op == 'W' ? 'X'
                         : op == 'v' ? 'o'
                         : op == 'w' ? 'a'
                         : op == 'x' ? '^'
                                     : '%';
        if (!e)
            e = script_apply(m, binary, old, right, out);
    } else if (script_punct(m, 'y') || script_punct(m, 'z') || script_punct(m, 'q')) {
        uint8_t op = m->token.punct;
        e = script_next(m);
        bool keep = op == 'y'   ? script_truthy(old)
                    : op == 'z' ? !script_truthy(old)
                                : old.kind != KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED &&
                                      old.kind != KALEIDOSCOPE_SCRIPT_VALUE_NULL,
             live = m->work->live;
        m->work->live = live && !keep;
        KaleidoscopeScriptValue right = {0};
        e = script_expr(m, &right);
        m->work->live = live;
        if (!e)
            *out = keep ? old : right;
    } else if (script_punct(m, 'I') || script_punct(m, 'D')) {
        uint8_t op = m->token.punct;
        e = script_next(m);
        *out = script_num(script_to_number(m, old) + (op == 'I' ? 1.0 : -1.0));
    } else
        e = KALEIDOSCOPE_SCRIPT_SYNTAX;
    return e;
}

/* `name op= value` and member-chain assignments; *result is the value assigned. */
static uint8_t script_assignment(ScriptMachine *m, bool run, KaleidoscopeScriptValue *result) {
    *result = script_undef();
    if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT) {
        return KALEIDOSCOPE_SCRIPT_SYNTAX;
    }
    size_t start = m->token.start, len = m->token.len;
    bool is_this = script_ident(m, "this");
    uint8_t e = script_next(m);
    if (e)
        return e;
    KaleidoscopeScriptVariable *slot = script_find(m, start, len);
    KaleidoscopeScriptValue old = is_this ? m->work->this_value
                                  : slot  ? slot->value
                                          : script_undef();
    bool found = is_this || slot;
    /* `C.x = v` / `C.count++`: a class's static fields. */
    if (!found && script_punct(m, '.'))
        for (size_t c = 0; !found && c < m->work->class_count; c++)
            if (script_class_name(m, &m->work->classes[c], m->source->bytes + start, len)) {
                KaleidoscopeScriptPosition at = script_position_save(m);
                e = script_next(m);
                if (!e && m->token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
                    old = script_object(
                        script_static_home(m, c, m->source->bytes + m->token.start, m->token.len));
                    found = true;
                }
                script_position_restore(m, at);
                if (e)
                    return e;
            }
    /* `F.prototype...` for a declared function. */
    if (!found && script_punct(m, '.'))
        for (size_t i = 0; !found && i < m->work->nfuncs; i++)
            if (m->work->funcs[i].len && script_function_name(m, &m->work->funcs[i], start, len)) {
                old = script_function((uint8_t)i);
                found = true;
            }
    if (!found && !m->work->skim && run)
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    if (!script_punct(m, '.') && !script_punct(m, '[')) {
        KaleidoscopeScriptValue value = {0};
        e = script_update(m, old, &value);
        if (e)
            return e;
        *result = value;
        if (run)
            return script_assign(m, start, len, value, false);
        return 0;
    }
    KaleidoscopeScriptValue base = old;
    for (;;) {
        ScriptKey key = {0};
        if (script_punct(m, '.')) {
            e = script_next(m);
            if (e)
                return e;
            if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            key.name = true;
            key.start = m->token.start;
            key.len = m->token.len;
            e = script_next(m);
            if (e)
                return e;
        } else {
            e = script_next(m);
            if (e)
                return e;
            e = script_expr(m, &key.value);
            if (e)
                return e;
            e = script_expect(m, ']');
            if (e)
                return e;
        }
        KaleidoscopeScriptValue current = script_undef();
        if (run) {
            if (base.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
                const uint8_t *bytes;
                size_t key_len = 0;
                if (key.name) {
                    bytes = m->source->bytes + key.start;
                    key_len = key.len;
                } else {
                    e = script_to_text(m, key.value, &key.value);
                    if (e)
                        return e;
                    ScriptSlice text = script_text(key.value);
                    bytes = m->work->arena + text.start;
                    key_len = text.len;
                }
                e = script_read_prop(m, base.index, bytes, key_len, &current);
                if (e)
                    return e;
            } else if (base.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY && key.name &&
                       script_word(m->source->bytes + key.start, key.len, "length"))
                current = script_num((double)m->work->arrays[base.index].len);
            else if (base.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION && key.name &&
                     script_word(m->source->bytes + key.start, key.len, "prototype")) {
                uint8_t id = 0;
                e = script_prototype(m, base.index, &id);
                if (e)
                    return e;
                current = script_object(id);
            } else if (!key.name && (base.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY ||
                                     base.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING)) {
                e = script_index(m, base, script_to_number(m, key.value), &current);
                if (e)
                    return e;
            } else
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        if (script_punct(m, '.') || script_punct(m, '[')) {
            base = current;
            continue;
        }
        KaleidoscopeScriptValue value = {0};
        e = script_update(m, current, &value);
        if (e)
            return e;
        *result = value;
        if (!run)
            return 0;
        /* `F.prototype = {...}` replaces what new instances copy. */
        if (base.kind == KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION && key.name &&
            script_word(m->source->bytes + key.start, key.len, "prototype"))
            return script_set_prototype(m, base.index, value);
        /* Assignments to a frozen object are ignored (sloppy mode). */
        if (base.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT && script_frozen(m, base.index))
            return 0;
        if (base.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
            const uint8_t *bytes;
            size_t key_len = 0;
            if (key.name) {
                bytes = m->source->bytes + key.start;
                key_len = key.len;
            } else {
                e = script_to_text(m, key.value, &key.value);
                if (e)
                    return e;
                ScriptSlice text = script_text(key.value);
                bytes = m->work->arena + text.start;
                key_len = text.len;
            }
            return script_write_prop(m, base.index, bytes, key_len, value);
        }
        if (base.kind != KALEIDOSCOPE_SCRIPT_VALUE_ARRAY)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        KaleidoscopeScriptArray *array = &m->work->arrays[base.index];
        if (key.name) {
            if (!script_word(m->source->bytes + key.start, key.len, "length"))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            double n = script_to_number(m, value);
            if (!(n >= 0.0 && n == script_floor(n) && n <= (double)array->len))
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            array->len = (uint16_t)n;
            return 0;
        }
        double index = script_to_number(m, key.value);
        if (!(index >= 0.0 && index == script_floor(index) && index < 65536.0))
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        if ((uint16_t)index < array->len) {
            m->work->elems[array->start + (size_t)index] = value;
            return 0;
        }
        if ((uint16_t)index == array->len)
            return script_push(m, base.index, value);
        return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
    }
}

/* Whether an assignment operator follows the member chain ahead; with steps, `++`/`--` too, as
   upstream's `assigns_ahead_with`. */
static uint8_t script_assigns_ahead_with(ScriptMachine *m, bool steps, bool *assigns) {
    KaleidoscopeScriptPosition saved = script_position_save(m);
    uint8_t e = 0;
    for (;;) {
        if (script_punct(m, '.')) {
            e = script_next(m);
            if (e)
                break;
            if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                break;
            e = script_next(m);
            if (e)
                break;
            continue;
        }
        if (script_punct(m, '[')) {
            unsigned depth = 0;
            for (;;) {
                if (script_punct(m, '['))
                    depth++;
                else if (script_punct(m, ']')) {
                    depth--;
                    if (!depth) {
                        e = script_next(m);
                        break;
                    }
                } else if (m->token.kind == KALEIDOSCOPE_TOKEN_END)
                    break;
                e = script_next(m);
                if (e)
                    break;
            }
            if (e)
                break;
            continue;
        }
        break;
    }
    *assigns = script_punct(m, '=') || script_punct(m, 'P') || script_punct(m, 'M') ||
               script_punct(m, 'T') || script_punct(m, 'Q') || script_punct(m, 'R') ||
               (steps && (script_punct(m, 'I') || script_punct(m, 'D'))) || script_punct(m, 'W') ||
               script_punct(m, 'v') || script_punct(m, 'w') || script_punct(m, 'x') ||
               script_punct(m, 'y') || script_punct(m, 'z') || script_punct(m, 'q');
    script_position_restore(m, saved);
    return e;
}

static uint8_t script_assigns_ahead(ScriptMachine *m, bool *assigns) {
    return script_assigns_ahead_with(m, true, assigns);
}

static uint8_t script_pattern(ScriptMachine *m, bool bind, KaleidoscopeScriptValue source) {
    uint8_t close = script_punct(m, '[') ? ']' : script_punct(m, '{') ? '}' : 0;
    if (!close)
        return KALEIDOSCOPE_SCRIPT_SYNTAX;
    uint8_t e = script_next(m);
    if (e)
        return e;
    size_t seen_start[8] = {0}, seen_len[8] = {0}, seen = 0, index = 0;
    while (!script_punct(m, close)) {
        if (close == ']' && script_punct(m, ',')) {
            e = script_next(m);
            if (e)
                return e;
            index++;
            continue;
        }
        bool rest = script_punct(m, 'S');
        if (rest) {
            e = script_next(m);
            if (e)
                return e;
        }
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        size_t key_start = m->token.start, key_len = m->token.len, name_start = key_start,
               name_len = key_len;
        e = script_next(m);
        if (e)
            return e;
        bool nested = false;
        KaleidoscopeScriptPosition nested_at = {0};
        if (close == '}' && !rest && script_punct(m, ':')) {
            e = script_next(m);
            if (e)
                return e;
            if (script_punct(m, '[') || script_punct(m, '{')) {
                /* `{a: {b}}` / `{a: [x, y]}`: a pattern for the member. */
                nested = true;
                nested_at = script_position_save(m);
                e = script_pattern(m, false, script_undef());
                if (e)
                    return e;
            } else {
                if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
                name_start = m->token.start;
                name_len = m->token.len;
                e = script_next(m);
                if (e)
                    return e;
            }
        }
        if (seen == 8 || name_len > KALEIDOSCOPE_SCRIPT_NAME)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
        KaleidoscopeScriptValue item = script_undef();
        if (bind) {
            if (close == ']' && source.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                KaleidoscopeScriptArray array = m->work->arrays[source.index];
                if (rest) {
                    size_t from = index < array.len ? index : array.len;
                    e = script_new_array(m, m->work->elems + array.start + from, array.len - from,
                                         &item);
                    if (e)
                        return e;
                } else {
                    e = script_index(m, source, (double)index, &item);
                    if (e)
                        return e;
                }
            } else if (close == '}' && source.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
                if (rest) {
                    uint8_t id = 0;
                    e = script_new_object(m, &id);
                    if (e)
                        return e;
                    KaleidoscopeScriptArray record = m->work->objects[source.index];
                    for (size_t i = 0; i < record.len; i++) {
                        KaleidoscopeScriptProperty property = m->work->props[record.start + i];
                        bool omitted = false;
                        for (size_t k = 0; k < seen; k++) {
                            if (property.len != seen_len[k])
                                continue;
                            bool same = true;
                            for (size_t j = 0; j < property.len; j++)
                                if (m->work->arena[property.start + j] !=
                                    m->source->bytes[seen_start[k] + j])
                                    same = false;
                            if (same)
                                omitted = true;
                        }
                        if (!omitted) {
                            e = script_set_prop(m, id, m->work->arena + property.start,
                                                property.len, property.value);
                            if (e)
                                return e;
                        }
                    }
                    item = script_object(id);
                } else
                    item = script_get_prop(m, source.index, m->source->bytes + key_start, key_len);
            } else
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        if (!rest && script_punct(m, '=')) {
            e = script_next(m);
            if (e)
                return e;
            bool missing = bind && item.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED,
                 live = m->work->live;
            m->work->live = live && missing;
            KaleidoscopeScriptValue fallback = {0};
            e = script_expr(m, &fallback);
            m->work->live = live;
            if (e)
                return e;
            if (missing)
                item = fallback;
        }
        if (nested && bind) {
            KaleidoscopeScriptPosition back = script_position_save(m);
            script_position_restore(m, nested_at);
            e = script_pattern(m, true, item);
            script_position_restore(m, back);
            if (e)
                return e;
        } else if (bind) {
            e = script_bind_var(m, name_start, name_len, true, item);
            if (e)
                return e;
        }
        seen_start[seen] = key_start;
        seen_len[seen] = key_len;
        seen++;
        index++;
        if (rest || !script_punct(m, ','))
            break;
        e = script_next(m);
        if (e)
            return e;
    }
    return script_expect(m, close);
}

static uint8_t script_destructure_statement(ScriptMachine *m, bool run) {
    uint8_t e = script_next(m);
    if (e)
        return e;
    KaleidoscopeScriptPosition pattern = script_position_save(m);
    e = script_pattern(m, false, script_undef());
    if (e)
        return e;
    e = script_expect(m, '=');
    if (e)
        return e;
    KaleidoscopeScriptValue value = {0};
    e = script_expr(m, &value);
    if (e)
        return e;
    if (run) {
        KaleidoscopeScriptPosition end = script_position_save(m);
        script_position_restore(m, pattern);
        e = script_pattern(m, true, value);
        script_position_restore(m, end);
        if (e)
            return e;
    }
    return script_end(m);
}

static bool script_label_equal(const ScriptMachine *m, size_t a_start, size_t a_len, size_t b_start,
                               size_t b_len) {
    if (a_len != b_len)
        return false;
    for (size_t i = 0; i < a_len; i++)
        if (m->source->bytes[a_start + i] != m->source->bytes[b_start + i])
            return false;
    return true;
}

static int script_loop_jump(ScriptMachine *m, bool label_present, size_t label_start,
                            size_t label_len) {
    if (m->work->jump_label_present &&
        (!label_present || !script_label_equal(m, m->work->jump_label_start,
                                               m->work->jump_label_len, label_start, label_len)))
        return m->work->jump_present ? 1 : -1;
    m->work->jump_label_present = false;
    if (!m->work->jump_present)
        return -1;
    int result = m->work->jump_break ? 1 : 0;
    m->work->jump_present = false;
    return result;
}

static uint8_t script_for_each(ScriptMachine *m, bool run, uint8_t kind, size_t name_start,
                               size_t name_len, bool pattern, KaleidoscopeScriptPosition pattern_at,
                               bool of, bool label_present, size_t label_start, size_t label_len) {
    uint8_t e = script_next(m);
    if (e)
        return e;
    KaleidoscopeScriptValue collection = {0};
    e = script_expr(m, &collection);
    if (e)
        return e;
    e = script_expect(m, ')');
    if (e)
        return e;
    KaleidoscopeScriptPosition body = script_position_save(m);
    m->work->loops++;
    size_t index = 0;
    for (;;) {
        script_position_restore(m, body);
        bool available = false;
        KaleidoscopeScriptValue item = script_undef();
        if (run && !m->work->ret_present) {
            if (of && collection.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                KaleidoscopeScriptArray array = m->work->arrays[collection.index];
                if (index < array.len) {
                    item = m->work->elems[array.start + index];
                    available = true;
                }
            } else if (of && script_is_collection(m, collection)) {
                e = script_collection_item(m, collection.index, index, &available, &item);
                if (e)
                    return e;
            } else if (of && collection.kind == KALEIDOSCOPE_SCRIPT_VALUE_STRING) {
                if (index < collection.len) {
                    e = script_index(m, collection, (double)index, &item);
                    if (e)
                        return e;
                    available = true;
                }
            } else if (!of && collection.kind == KALEIDOSCOPE_SCRIPT_VALUE_OBJECT) {
                KaleidoscopeScriptArray object = m->work->objects[collection.index];
                while (index < object.len &&
                       script_hidden_prop(m, collection.index, object.start + index))
                    index++;
                if (index < object.len) {
                    KaleidoscopeScriptProperty property = m->work->props[object.start + index];
                    item = script_str(property.start, property.len);
                    available = true;
                }
            } else if (!of && collection.kind == KALEIDOSCOPE_SCRIPT_VALUE_ARRAY) {
                KaleidoscopeScriptArray array = m->work->arrays[collection.index];
                if (index < array.len) {
                    uint8_t digits[32] = {0};
                    size_t n = script_uint(index, digits);
                    e = script_store(m, digits, n, &item);
                    if (e)
                        return e;
                    available = true;
                }
            } else if (!of && (collection.kind == KALEIDOSCOPE_SCRIPT_VALUE_NULL ||
                               collection.kind == KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED))
                available = false;
            else
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        }
        if (available && pattern) {
            KaleidoscopeScriptPosition back = script_position_save(m);
            script_position_restore(m, pattern_at);
            e = script_pattern(m, true, item);
            script_position_restore(m, back);
            if (e)
                return e;
        } else if (available) {
            KaleidoscopeScriptVariable *slot = script_find(m, name_start, name_len);
            if (slot && (kind == 0 || (size_t)(slot - m->work->vars) >= m->work->block_base))
                slot->value = item;
            else if (kind == 0)
                e = script_assign(m, name_start, name_len, item, true);
            else
                e = script_declare(m, name_start, name_len, item, kind);
            if (e)
                return e;
        }
        e = script_statement(m, available);
        if (e)
            return e;
        int jump = script_loop_jump(m, label_present, label_start, label_len);
        if (!available || jump == 1) {
            m->work->loops--;
            return 0;
        }
        index++;
    }
}

/* A `for` update clause: comma-separated expressions, run only when `run`. */
static uint8_t script_update_list(ScriptMachine *m, bool run) {
    uint8_t e = 0;
    bool more = true;
    while (!e && more) {
        bool live = m->work->live;
        m->work->live = live && run;
        KaleidoscopeScriptValue ignored = {0};
        e = script_expr(m, &ignored);
        m->work->live = live;
        more = !e && script_punct(m, ',');
        if (more)
            e = script_next(m);
    }
    return e;
}

static uint8_t script_for_loop(ScriptMachine *m, bool run) {
    bool label_present = m->work->label_next_present;
    size_t label_start = m->work->label_next_start, label_len = m->work->label_next_len;
    m->work->label_next_present = false;
    KaleidoscopeScriptPosition at = script_position_save(m);
    uint8_t kind = 0;
    bool explicit_kind = false;
    if (script_ident(m, "var") || script_ident(m, "let") || script_ident(m, "const")) {
        explicit_kind = true;
        kind = script_ident(m, "var") ? 0 : script_ident(m, "let") ? 1 : 2;
        uint8_t e = script_next(m);
        if (e)
            return e;
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT) {
        size_t name_start = m->token.start, name_len = m->token.len;
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (script_ident(m, "of") || script_ident(m, "in")) {
            KaleidoscopeScriptPosition none = {0};
            return script_for_each(m, run, explicit_kind ? kind : 0, name_start, name_len, false,
                                   none, script_ident(m, "of"), label_present, label_start,
                                   label_len);
        }
    } else if (explicit_kind && (script_punct(m, '[') || script_punct(m, '{'))) {
        KaleidoscopeScriptPosition pattern = script_position_save(m);
        uint8_t e = script_pattern(m, false, script_undef());
        if (e)
            return e;
        if (script_ident(m, "of") || script_ident(m, "in"))
            return script_for_each(m, run, 0, 0, 0, true, pattern, script_ident(m, "of"),
                                   label_present, label_start, label_len);
    }
    script_position_restore(m, at);
    uint8_t e = 0;
    if (script_punct(m, ';')) {
        e = script_next(m);
        if (e)
            return e;
    } else {
        e = script_statement(m, run);
        if (e)
            return e;
    }
    KaleidoscopeScriptPosition top = script_position_save(m);
    m->work->loops++;
    for (;;) {
        script_position_restore(m, top);
        bool condition = true;
        if (!script_punct(m, ';')) {
            KaleidoscopeScriptValue value = {0};
            e = script_expr(m, &value);
            if (e)
                return e;
            condition = script_truthy(value);
        }
        e = script_expect(m, ';');
        if (e)
            return e;
        KaleidoscopeScriptPosition update = script_position_save(m);
        if (!script_punct(m, ')')) {
            e = script_update_list(m, false);
            if (e)
                return e;
        }
        e = script_expect(m, ')');
        if (e)
            return e;
        bool go = run && !m->work->ret_present && condition;
        e = script_statement(m, go);
        if (e)
            return e;
        int jump = script_loop_jump(m, label_present, label_start, label_len);
        if (!go || jump == 1) {
            m->work->loops--;
            return 0;
        }
        script_position_restore(m, update);
        if (!script_punct(m, ')')) {
            e = script_update_list(m, true);
            if (e)
                return e;
        }
    }
}

static uint8_t script_peek_after(ScriptMachine *m, KaleidoscopeScriptToken *token) {
    KaleidoscopeScriptPosition at = script_position_save(m);
    uint8_t e = script_next(m);
    *token = m->token;
    script_position_restore(m, at);
    return e;
}

static uint8_t script_statement_inner(ScriptMachine *m, bool run) {
    bool destructure = false;
    if (script_ident(m, "var") || script_ident(m, "let") || script_ident(m, "const")) {
        KaleidoscopeScriptToken next = {0};
        uint8_t peek = script_peek_after(m, &next);
        if (peek)
            return peek;
        destructure =
            next.kind == KALEIDOSCOPE_TOKEN_PUNCT && (next.punct == '[' || next.punct == '{');
    }
    bool labelled = false;
    if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT && !script_ident(m, "default") &&
        !script_ident(m, "case") && !script_ident(m, "else")) {
        KaleidoscopeScriptToken next = {0};
        uint8_t peek = script_peek_after(m, &next);
        if (peek)
            return peek;
        labelled = next.kind == KALEIDOSCOPE_TOKEN_PUNCT && next.punct == ':';
    }
    if (script_punct(m, ';'))
        return script_next(m);
    if (script_punct(m, '{')) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        size_t since = m->work->nvars, outer_base = m->work->block_base;
        m->work->block_base = since;
        while (!script_punct(m, '}')) {
            if (m->token.kind == KALEIDOSCOPE_TOKEN_END)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            e = script_statement(m, run);
            if (e)
                return e;
        }
        script_end_block(m, since);
        m->work->block_base = outer_base;
        return script_next(m);
    }
    if (script_ident(m, "if")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        e = script_expect(m, '(');
        if (e)
            return e;
        KaleidoscopeScriptValue condition = {0};
        e = script_expr(m, &condition);
        if (e)
            return e;
        e = script_expect(m, ')');
        if (e)
            return e;
        bool yes = script_truthy(condition);
        e = script_statement(m, run && yes);
        if (e)
            return e;
        if (script_ident(m, "else")) {
            e = script_next(m);
            if (e)
                return e;
            return script_statement(m, run && !yes);
        }
        return 0;
    }
    if ((script_ident(m, "var") || script_ident(m, "let") || script_ident(m, "const")) &&
        destructure)
        return script_destructure_statement(m, run);
    if (script_ident(m, "var") || script_ident(m, "let") || script_ident(m, "const")) {
        uint8_t kind = script_ident(m, "var") ? 0 : script_ident(m, "let") ? 1 : 2;
        uint8_t e = 0;
        for (;;) {
            e = script_next(m);
            if (e)
                return e;
            if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT || script_reserved_name(m))
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            size_t start = m->token.start, len = m->token.len;
            e = script_next(m);
            if (e)
                return e;
            bool has_value = script_punct(m, '=');
            KaleidoscopeScriptValue value = script_undef();
            if (has_value) {
                e = script_next(m);
                if (e)
                    return e;
                e = script_expr(m, &value);
                if (e)
                    return e;
            } else if (kind == 2)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            if (run) {
                if (!kind)
                    e = script_bind_var(m, start, len, has_value, value);
                else {
                    KaleidoscopeScriptVariable *existing = script_find(m, start, len);
                    if (existing && (size_t)(existing - m->work->vars) >= m->work->block_base &&
                        existing->kind != 0)
                        return KALEIDOSCOPE_SCRIPT_SYNTAX;
                    e = script_declare(m, start, len, value, kind);
                }
                if (e)
                    return e;
            }
            if (!script_punct(m, ','))
                break;
        }
        return script_end(m);
    }
    if (script_ident(m, "document")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        e = script_expect(m, '.');
        if (e)
            return e;
        bool line = script_ident(m, "writeln");
        if (!line && !script_ident(m, "write"))
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        e = script_next(m);
        if (e)
            return e;
        e = script_expect(m, '(');
        if (e)
            return e;
        if (!script_punct(m, ')'))
            for (;;) {
                KaleidoscopeScriptValue value = {0};
                e = script_expr(m, &value);
                if (e)
                    return e;
                if (run) {
                    e = script_emit(m, value);
                    if (e)
                        return e;
                }
                if (!script_punct(m, ','))
                    break;
                e = script_next(m);
                if (e)
                    return e;
            }
        e = script_expect(m, ')');
        if (e)
            return e;
        if (line && run) {
            static const uint8_t nl = '\n';
            KaleidoscopeScriptValue value = {0};
            e = script_store(m, &nl, 1, &value);
            if (e)
                return e;
            e = script_emit(m, value);
            if (e)
                return e;
        }
        return script_end(m);
    }
    if (script_ident(m, "class")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        size_t name_start = m->token.start, name_len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
        bool parent_present = false;
        uint8_t parent = 0;
        if (script_ident(m, "extends")) {
            e = script_next(m);
            if (e)
                return e;
            if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            if (run) {
                size_t found = SIZE_MAX;
                for (size_t i = 0; i < m->work->class_count; i++)
                    if (script_class_name(m, &m->work->classes[i],
                                          m->source->bytes + m->token.start, m->token.len)) {
                        found = i;
                        break;
                    }
                if (found == SIZE_MAX)
                    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
                parent_present = true;
                parent = (uint8_t)found;
            }
            e = script_next(m);
            if (e)
                return e;
        }
        e = script_expect(m, '{');
        if (e)
            return e;
        bool constructor_present = false;
        uint8_t constructor = 0;
        size_t method_start = m->work->class_method_count, method_count = 0,
               static_start = m->work->class_static_method_count, static_count = 0,
               accessor_start = m->work->class_accessor_count, accessor_count = 0,
               field_start = m->work->class_field_count, field_count = 0;
        /* Static fields live on an object of their own (`C.x`, `C.x = v`). */
        uint8_t static_object = 0;
        if (run) {
            e = script_new_object(m, &static_object);
            if (e)
                return e;
        }
        while (!script_punct(m, '}')) {
            if (script_punct(m, ';')) {
                e = script_next(m);
                if (e)
                    return e;
                continue;
            }
            if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            size_t member_start = m->token.start, member_len = m->token.len;
            bool static_word = script_ident(m, "static"),
                 accessor_word = script_ident(m, "get") || script_ident(m, "set"),
                 setter = script_ident(m, "set"), is_constructor = script_ident(m, "constructor");
            e = script_next(m);
            if (e)
                return e;
            bool static_method = static_word && m->token.kind == KALEIDOSCOPE_TOKEN_IDENT,
                 accessor_method =
                     !static_method && accessor_word && m->token.kind == KALEIDOSCOPE_TOKEN_IDENT;
            if (static_method || accessor_method) {
                member_start = m->token.start;
                member_len = m->token.len;
                is_constructor = false;
                e = script_next(m);
                if (e)
                    return e;
            }
            if (!accessor_method && !script_punct(m, '(')) {
                /* A field: `x = expr` / `x;` (per instance), `static x = expr`. */
                bool init_present = script_punct(m, '=');
                KaleidoscopeScriptPosition init = {0};
                if (init_present) {
                    e = script_next(m);
                    if (e)
                        return e;
                    init = script_position_save(m);
                    bool live = m->work->live;
                    m->work->live = false;
                    KaleidoscopeScriptValue ignored = {0};
                    e = script_expr(m, &ignored);
                    m->work->live = live;
                    if (e)
                        return e;
                }
                if (run) {
                    if (m->work->class_field_count >= 64)
                        return KALEIDOSCOPE_SCRIPT_LIMIT;
                    m->work->class_fields[m->work->class_field_count++] =
                        (KaleidoscopeScriptClassField){.name_start = (uint16_t)member_start,
                                                       .name_len = (uint16_t)member_len,
                                                       .init_present = init_present,
                                                       .is_static = static_method,
                                                       .init = init};
                    field_count++;
                }
                if (script_punct(m, ';')) {
                    e = script_next(m);
                    if (e)
                        return e;
                } else if (!m->token.newline && !script_punct(m, '}'))
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
                continue;
            }
            KaleidoscopeScriptPosition at = script_position_save(m);
            e = script_expect(m, '(');
            if (e)
                return e;
            e = script_skip_params(m);
            if (e)
                return e;
            e = script_expect(m, ')');
            if (e)
                return e;
            if (!script_punct(m, '{'))
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            e = script_skim_body(m);
            if (e)
                return e;
            if (run) {
                uint8_t function = 0;
                e = script_define(m, NULL, 0, at, &function);
                if (e)
                    return e;
                if (accessor_method) {
                    if (m->work->class_accessor_count >= 64)
                        return KALEIDOSCOPE_SCRIPT_LIMIT;
                    m->work->class_accessors[m->work->class_accessor_count++] =
                        (KaleidoscopeScriptClassAccessor){.name_start = (uint16_t)member_start,
                                                          .name_len = (uint16_t)member_len,
                                                          .function = function,
                                                          .setter = setter};
                    accessor_count++;
                } else if (static_method) {
                    if (m->work->class_static_method_count >= 64)
                        return KALEIDOSCOPE_SCRIPT_LIMIT;
                    m->work->class_static_methods[m->work->class_static_method_count++] =
                        (KaleidoscopeScriptClassMethod){.name_start = (uint16_t)member_start,
                                                        .name_len = (uint16_t)member_len,
                                                        .function = function};
                    static_count++;
                } else if (is_constructor) {
                    constructor_present = true;
                    constructor = function;
                } else {
                    if (m->work->class_method_count >= 64)
                        return KALEIDOSCOPE_SCRIPT_LIMIT;
                    m->work->class_methods[m->work->class_method_count++] =
                        (KaleidoscopeScriptClassMethod){.name_start = (uint16_t)member_start,
                                                        .name_len = (uint16_t)member_len,
                                                        .function = function};
                    method_count++;
                }
            }
        }
        e = script_next(m);
        if (e)
            return e;
        if (run) {
            if (m->work->class_count >= 16)
                return KALEIDOSCOPE_SCRIPT_LIMIT;
            m->work->classes[m->work->class_count++] =
                (KaleidoscopeScriptClass){.name_start = (uint16_t)name_start,
                                          .name_len = (uint16_t)name_len,
                                          .method_start = (uint16_t)method_start,
                                          .method_count = (uint16_t)method_count,
                                          .static_start = (uint16_t)static_start,
                                          .static_count = (uint16_t)static_count,
                                          .accessor_start = (uint16_t)accessor_start,
                                          .accessor_count = (uint16_t)accessor_count,
                                          .field_start = (uint16_t)field_start,
                                          .field_count = (uint16_t)field_count,
                                          .constructor_present = constructor_present,
                                          .parent_present = parent_present,
                                          .constructor = constructor,
                                          .parent = parent,
                                          .static_object = static_object};
            /* Static fields in order, once the class exists (`static b = C.a + 1`); then only
               the instance fields stay in the table. */
            KaleidoscopeScriptClass *record = &m->work->classes[m->work->class_count - 1];
            size_t kept = 0;
            for (size_t i = 0; i < field_count; i++) {
                KaleidoscopeScriptClassField field = m->work->class_fields[field_start + i];
                if (!field.is_static) {
                    m->work->class_fields[field_start + kept++] = field;
                    continue;
                }
                KaleidoscopeScriptValue value = script_undef();
                if (field.init_present) {
                    e = script_eval_at(m, field.init, script_object(static_object), &value);
                    if (e)
                        return e;
                }
                e = script_set_prop(m, static_object, m->source->bytes + field.name_start,
                                    field.name_len, value);
                if (e)
                    return e;
            }
            record->field_count = (uint16_t)kept;
            m->work->class_field_count = field_start + kept;
        }
        return 0;
    }
    if (script_ident(m, "function")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        size_t name_start = m->token.start, name_len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
        KaleidoscopeScriptPosition at = script_position_save(m);
        e = script_expect(m, '(');
        if (e)
            return e;
        e = script_skip_params(m);
        if (e)
            return e;
        e = script_expect(m, ')');
        if (e)
            return e;
        if (!script_punct(m, '{'))
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        e = script_skim_body(m);
        if (e)
            return e;
        if (run) {
            uint8_t function = 0;
            e = script_define(m, m->source->bytes + name_start, name_len, at, &function);
            if (e)
                return e;
        }
        return 0;
    }
    if (script_ident(m, "return")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        KaleidoscopeScriptValue value = script_undef();
        if (!(script_punct(m, ';') || script_punct(m, '}') ||
              m->token.kind == KALEIDOSCOPE_TOKEN_END || m->token.newline)) {
            e = script_expr(m, &value);
            if (e)
                return e;
        }
        if (run) {
            e = script_escape(m, value, &value);
            if (e)
                return e;
            m->work->ret = value;
            m->work->ret_present = true;
        }
        return script_end(m);
    }
    if (script_ident(m, "break") || script_ident(m, "continue")) {
        bool stop = script_ident(m, "break");
        uint8_t e = script_next(m);
        if (e)
            return e;
        bool labelled = m->token.kind == KALEIDOSCOPE_TOKEN_IDENT && !m->token.newline;
        size_t label_start = 0, label_len = 0;
        if (labelled) {
            label_start = m->token.start;
            label_len = m->token.len;
            e = script_next(m);
            if (e)
                return e;
        }
        if ((!labelled && !m->work->loops) || (labelled && !m->work->labels))
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        if (run) {
            m->work->jump_present = true;
            m->work->jump_break = stop;
            m->work->jump_label_present = labelled;
            m->work->jump_label_start = (uint16_t)label_start;
            m->work->jump_label_len = (uint16_t)label_len;
        }
        return script_end(m);
    }
    if (script_ident(m, "throw")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        KaleidoscopeScriptValue value = {0};
        e = script_expr(m, &value);
        if (e)
            return e;
        if (run) {
            m->work->thrown = value;
            m->work->thrown_present = true;
            return KALEIDOSCOPE_SCRIPT_THROWN;
        }
        return script_end(m);
    }
    if (script_ident(m, "try")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (!script_punct(m, '{'))
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        KaleidoscopeScriptPosition at = script_position_save(m);
        uint8_t saved_depth = m->work->depth, saved_loops = m->work->loops;
        bool saved_live = m->work->live;
        size_t saved_nvars = m->work->nvars, saved_base = m->work->block_base;
        bool caught = false;
        KaleidoscopeScriptValue thrown = script_undef();
        e = script_statement(m, run);
        if (e == KALEIDOSCOPE_SCRIPT_THROWN) {
            m->work->depth = saved_depth;
            m->work->loops = saved_loops;
            m->work->live = saved_live;
            m->work->nvars = saved_nvars;
            m->work->block_base = saved_base;
            m->work->jump_present = false;
            m->work->jump_label_present = false;
            script_position_restore(m, at);
            e = script_statement(m, false);
            if (e)
                return e;
            caught = m->work->thrown_present;
            thrown = m->work->thrown;
            m->work->thrown_present = false;
        } else if (e)
            return e;
        if (script_ident(m, "catch")) {
            e = script_next(m);
            if (e)
                return e;
            bool has_name = false;
            size_t name_start = 0, name_len = 0;
            if (script_punct(m, '(')) {
                e = script_next(m);
                if (e)
                    return e;
                if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
                    return KALEIDOSCOPE_SCRIPT_SYNTAX;
                has_name = true;
                name_start = m->token.start;
                name_len = m->token.len;
                e = script_next(m);
                if (e)
                    return e;
                e = script_expect(m, ')');
                if (e)
                    return e;
            }
            size_t since = m->work->nvars;
            if (has_name && caught) {
                e = script_declare(m, name_start, name_len, thrown, 1);
                if (e)
                    return e;
            }
            e = script_statement(m, run && caught);
            script_end_block(m, since);
            if (e)
                return e;
        } else if (caught && !script_ident(m, "finally"))
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        if (script_ident(m, "finally")) {
            e = script_next(m);
            if (e)
                return e;
            return script_statement(m, run);
        }
        return 0;
    }
    if (script_ident(m, "delete")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        size_t object_start = m->token.start, object_len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
        e = script_expect(m, '.');
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_SYNTAX;
        size_t key_start = m->token.start, key_len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
        if (run) {
            KaleidoscopeScriptVariable *variable = script_find(m, object_start, object_len);
            if (!variable || variable->value.kind != KALEIDOSCOPE_SCRIPT_VALUE_OBJECT)
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            uint8_t id = variable->value.index;
            size_t slot = script_find_prop(m, id, m->source->bytes + key_start, key_len);
            if (slot != SIZE_MAX) {
                KaleidoscopeScriptArray *record = &m->work->objects[id];
                for (size_t i = slot + 1; i < record->start + record->len; i++)
                    m->work->props[i - 1] = m->work->props[i];
                record->len--;
            }
        }
        return script_end(m);
    }
    if (script_ident(m, "do")) {
        bool label_present = m->work->label_next_present;
        size_t label_start = m->work->label_next_start, label_len = m->work->label_next_len;
        m->work->label_next_present = false;
        uint8_t e = script_next(m);
        if (e)
            return e;
        KaleidoscopeScriptPosition top = script_position_save(m);
        m->work->loops++;
        bool go = run && !m->work->ret_present;
        for (;;) {
            script_position_restore(m, top);
            e = script_statement(m, go);
            if (e)
                return e;
            int jump = go ? script_loop_jump(m, label_present, label_start, label_len) : -1;
            bool stop = jump == 1;
            if (!script_ident(m, "while"))
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            e = script_next(m);
            if (e)
                return e;
            e = script_expect(m, '(');
            if (e)
                return e;
            KaleidoscopeScriptValue condition = {0};
            e = script_expr(m, &condition);
            if (e)
                return e;
            e = script_expect(m, ')');
            if (e)
                return e;
            bool again = go && !stop && !m->work->ret_present && script_truthy(condition);
            if (!again) {
                m->work->loops--;
                if (script_punct(m, ';'))
                    e = script_next(m);
                return e;
            }
            go = again;
        }
    }
    if (script_ident(m, "switch")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        e = script_expect(m, '(');
        if (e)
            return e;
        KaleidoscopeScriptValue discriminant = {0};
        e = script_expr(m, &discriminant);
        if (e)
            return e;
        e = script_expect(m, ')');
        if (e)
            return e;
        e = script_expect(m, '{');
        if (e)
            return e;
        m->work->loops++;
        bool matched = false;
        while (!script_punct(m, '}')) {
            if (script_ident(m, "case")) {
                e = script_next(m);
                if (e)
                    return e;
                KaleidoscopeScriptValue value = {0};
                e = script_expr(m, &value);
                if (e)
                    return e;
                e = script_expect(m, ':');
                if (e)
                    return e;
                if (!matched) {
                    KaleidoscopeScriptValue equal = {0};
                    e = script_apply(m, 'E', discriminant, value, &equal);
                    if (e)
                        return e;
                    matched = equal.boolean;
                }
            } else if (script_ident(m, "default")) {
                e = script_next(m);
                if (e)
                    return e;
                e = script_expect(m, ':');
                if (e)
                    return e;
                matched = true;
            } else if (m->token.kind == KALEIDOSCOPE_TOKEN_END)
                return KALEIDOSCOPE_SCRIPT_SYNTAX;
            else {
                bool live = run && matched && !m->work->jump_present;
                e = script_statement(m, live);
                if (e)
                    return e;
            }
        }
        e = script_next(m);
        m->work->loops--;
        if (m->work->jump_present && m->work->jump_break && !m->work->jump_label_present)
            m->work->jump_present = false;
        return e;
    }
    if (script_ident(m, "while")) {
        bool label_present = m->work->label_next_present;
        size_t label_start = m->work->label_next_start, label_len = m->work->label_next_len;
        m->work->label_next_present = false;
        uint8_t e = script_next(m);
        if (e)
            return e;
        KaleidoscopeScriptPosition top = script_position_save(m);
        m->work->loops++;
        for (;;) {
            script_position_restore(m, top);
            e = script_expect(m, '(');
            if (e)
                return e;
            KaleidoscopeScriptValue condition = {0};
            e = script_expr(m, &condition);
            if (e)
                return e;
            e = script_expect(m, ')');
            if (e)
                return e;
            bool go = run && !m->work->ret_present && script_truthy(condition);
            e = script_statement(m, go);
            if (e)
                return e;
            int jump = script_loop_jump(m, label_present, label_start, label_len);
            if (!go || jump == 1) {
                m->work->loops--;
                return 0;
            }
        }
    }
    if (script_ident(m, "for")) {
        uint8_t e = script_next(m);
        if (e)
            return e;
        e = script_expect(m, '(');
        if (e)
            return e;
        size_t since = m->work->nvars, outer_base = m->work->block_base;
        m->work->block_base = since;
        e = script_for_loop(m, run);
        script_end_block(m, since);
        m->work->block_base = outer_base;
        return e;
    }
    if (labelled) {
        size_t label_start = m->token.start, label_len = m->token.len;
        uint8_t e = script_next(m);
        if (e)
            return e;
        e = script_expect(m, ':');
        if (e)
            return e;
        if (script_ident(m, "for") || script_ident(m, "while") || script_ident(m, "do")) {
            m->work->label_next_present = true;
            m->work->label_next_start = (uint16_t)label_start;
            m->work->label_next_len = (uint16_t)label_len;
        }
        m->work->labels++;
        e = script_statement(m, run);
        m->work->labels--;
        m->work->label_next_present = false;
        if (m->work->jump_label_present &&
            script_label_equal(m, m->work->jump_label_start, m->work->jump_label_len, label_start,
                               label_len)) {
            m->work->jump_present = false;
            m->work->jump_label_present = false;
        }
        return e;
    }
    if (m->token.kind == KALEIDOSCOPE_TOKEN_IDENT && !script_ident(m, "else") &&
        !script_ident(m, "new") && !script_ident(m, "void") && !script_ident(m, "typeof")) {
        KaleidoscopeScriptPosition at = script_position_save(m);
        uint8_t e = script_next(m);
        if (e)
            return e;
        bool assigns = false;
        e = script_assigns_ahead(m, &assigns);
        if (e)
            return e;
        bool call =
            !assigns && (script_punct(m, '(') || script_punct(m, '.') || script_punct(m, '['));
        script_position_restore(m, at);
        KaleidoscopeScriptValue ignored = {0};
        if (call)
            e = script_expr(m, &ignored);
        else
            e = script_assignment(m, run, &ignored);
        while (!e && script_punct(m, ',')) {
            e = script_next(m);
            if (!e)
                e = script_expr(m, &ignored);
        }
        return e ? e : script_end(m);
    }
    if (script_punct(m, 'I') || script_punct(m, 'D')) {
        uint8_t op = m->token.punct, e = script_next(m);
        if (e)
            return e;
        if (m->token.kind != KALEIDOSCOPE_TOKEN_IDENT)
            return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
        size_t start = m->token.start, len = m->token.len;
        e = script_next(m);
        if (e)
            return e;
        if (run) {
            KaleidoscopeScriptVariable *slot = script_find(m, start, len);
            if (!slot)
                return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
            e = script_assign(
                m, start, len,
                script_num(script_to_number(m, slot->value) + (op == 'I' ? 1.0 : -1.0)), false);
            if (e)
                return e;
        }
        return script_end(m);
    }
    if (script_punct(m, '[') || script_punct(m, '(') ||
        m->token.kind == KALEIDOSCOPE_TOKEN_NUMBER || m->token.kind == KALEIDOSCOPE_TOKEN_STRING ||
        m->token.kind == KALEIDOSCOPE_TOKEN_TEMPLATE || script_ident(m, "new") ||
        script_ident(m, "void") || script_ident(m, "typeof")) {
        KaleidoscopeScriptValue ignored = {0};
        uint8_t e = script_expr(m, &ignored);
        return e ? e : script_end(m);
    }
    return KALEIDOSCOPE_SCRIPT_UNSUPPORTED;
}

static uint8_t script_statement(ScriptMachine *m, bool run) {
    if (script_too_deep(m))
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    run = run && !m->work->ret_present && !m->work->jump_present && !m->work->skim;
    if (run) {
        m->work->steps++;
        if (m->work->steps > 50000)
            return KALEIDOSCOPE_SCRIPT_LIMIT;
    }
    m->work->depth++;
    bool live = m->work->live;
    m->work->live = run;
    uint8_t e = script_statement_inner(m, run);
    m->work->live = live;
    m->work->depth--;
    return e;
}

uint8_t kaleidoscope_script_run(const KaleidoscopeScriptSource *source,
                                KaleidoscopeScriptWorkspace *workspace,
                                KaleidoscopeScriptResult *result) {
    if (!source || !workspace || !result || source->len > KALEIDOSCOPE_SCRIPT_MAX_SOURCE) {
        return KALEIDOSCOPE_SCRIPT_LIMIT;
    }
    workspace->arena_len = workspace->block_base = workspace->nvars = workspace->output_len =
        workspace->nfuncs = workspace->elem_count = workspace->narrays = workspace->prop_count =
            workspace->nobjects = workspace->regex_count = workspace->class_count =
                workspace->class_method_count = workspace->class_static_method_count =
                    workspace->class_accessor_count = workspace->class_field_count =
                        workspace->accessor_count = workspace->closure_count =
                            workspace->env_count = workspace->frame_base =
                                workspace->replacement_count = workspace->proto_count =
                                    workspace->bound_count = workspace->argument_top =
                                        workspace->args_now_start = workspace->args_now_count =
                                            workspace->frozen_count = workspace->collection_count =
                                                workspace->promise_count =
                                                    workspace->waiting_count =
                                                        workspace->job_count =
                                                            workspace->resolver_count =
                                                                workspace->gather_count = 0;
    workspace->depth = workspace->loops = workspace->labels = 0;
    workspace->steps = 0;
    workspace->skim = false;
    workspace->live = true;
    workspace->ret_present = false;
    workspace->jump_present = false;
    workspace->jump_label_present = false;
    workspace->label_next_present = false;
    workspace->this_value = script_undef();
    workspace->this_next_present = false;
    workspace->thrown_present = false;
    workspace->func_now_present = false;
    kaleidoscope_script_lexer_init(&workspace->lexer);
    result->error = KALEIDOSCOPE_SCRIPT_OK;
    result->len = 0;
    ScriptMachine machine = {.source = source, .work = workspace};
    uint8_t error = script_next(&machine);
    while (!error && machine.token.kind != KALEIDOSCOPE_TOKEN_END)
        error = script_statement(&machine, true);
    if (!error)
        error = script_drain_jobs(&machine);
    if (error) {
        result->error = error;
        return error;
    }
    result->len = workspace->output_len;
    for (size_t i = 0; i < result->len; i++)
        result->bytes[i] = workspace->output[i];
    return 0;
}


