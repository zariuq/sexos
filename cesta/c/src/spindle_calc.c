#include "spindle_calc.h"
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

#define CALC_SCALE ((SpindleCalcInt)1000000000)
#define CALC_MAX_DEPTH ((uint8_t)16)
#define CALC_INT_MAX ((SpindleCalcInt)(((SpindleCalcUInt) - 1) >> 1))
#define CALC_INT_MIN (-CALC_INT_MAX - 1)

/* A value's real and imaginary parts, each scaled. */
typedef struct CalcComplex {
    SpindleCalcInt re;
    SpindleCalcInt im;
} CalcComplex;

typedef struct CalcParser {
    const SpindleCalcInput *input;
    size_t at;
    uint8_t depth;
    bool inexact;
    bool has_answer;
    CalcComplex answer;
    bool current;
    const SpindleCalcEnv *env;
} CalcParser;

static SpindleCalcUInt calc_magnitude(SpindleCalcInt value) {
    return value < 0 ? (SpindleCalcUInt)(-(value + 1)) + 1 : (SpindleCalcUInt)value;
}

static bool calc_add_checked(SpindleCalcInt left, SpindleCalcInt right, SpindleCalcInt *out) {
    if ((right > 0 && left > CALC_INT_MAX - right) || (right < 0 && left < CALC_INT_MIN - right))
        return false;
    *out = left + right;
    return true;
}

static bool calc_sub_checked(SpindleCalcInt left, SpindleCalcInt right, SpindleCalcInt *out) {
    if ((right > 0 && left < CALC_INT_MIN + right) || (right < 0 && left > CALC_INT_MAX + right))
        return false;
    *out = left - right;
    return true;
}

static void calc_udivmod(SpindleCalcUInt numerator, SpindleCalcUInt denominator,
                         SpindleCalcUInt *quotient, SpindleCalcUInt *remainder) {
    SpindleCalcUInt q = 0, r = 0;
    for (unsigned bit = 128; bit > 0; --bit) {
        bool high = (r >> 127) != 0;
        r = (r << 1) | ((numerator >> (bit - 1)) & 1);
        if (high || r >= denominator) {
            r -= denominator;
            q |= (SpindleCalcUInt)1 << (bit - 1);
        }
    }
    *quotient = q;
    *remainder = r;
}

static bool calc_signed_from_magnitude(SpindleCalcUInt magnitude, bool negative,
                                       SpindleCalcInt *out) {
    SpindleCalcUInt min_magnitude = (SpindleCalcUInt)CALC_INT_MAX + 1;
    if (negative) {
        if (magnitude > min_magnitude)
            return false;
        if (magnitude == min_magnitude)
            *out = CALC_INT_MIN;
        else
            *out = -(SpindleCalcInt)magnitude;
        return true;
    }
    if (magnitude > (SpindleCalcUInt)CALC_INT_MAX) {
        return false;
    }
    *out = (SpindleCalcInt)magnitude;
    return true;
}

static bool calc_divmod_signed(SpindleCalcInt numerator, SpindleCalcInt denominator,
                               SpindleCalcInt *quotient, SpindleCalcInt *remainder) {
    SpindleCalcUInt q = 0, r = 0;
    calc_udivmod(calc_magnitude(numerator), calc_magnitude(denominator), &q, &r);
    if (!calc_signed_from_magnitude(q, (numerator < 0) != (denominator < 0), quotient))
        return false;
    return calc_signed_from_magnitude(r, numerator < 0, remainder);
}

static bool calc_mul_checked(SpindleCalcInt left, SpindleCalcInt right, SpindleCalcInt *out) {
    if (!left || !right) {
        *out = 0;
        return true;
    }
    bool negative = (left < 0) != (right < 0);
    SpindleCalcUInt a = calc_magnitude(left), b = calc_magnitude(right);
    SpindleCalcUInt min_magnitude = (SpindleCalcUInt)CALC_INT_MAX + 1;
    SpindleCalcUInt limit = negative ? min_magnitude : (SpindleCalcUInt)CALC_INT_MAX;
    SpindleCalcUInt bound = 0, unused = 0;
    calc_udivmod(limit, b, &bound, &unused);
    if (a > bound) {
        return false;
    }
    SpindleCalcUInt product = a * b;
    if (negative) {
        if (product == min_magnitude)
            *out = CALC_INT_MIN;
        else
            *out = -(SpindleCalcInt)product;
    } else {
        *out = (SpindleCalcInt)product;
    }
    return true;
}

/* A numerator over a denominator, rounded toward zero; a remainder makes it inexact. */
static bool calc_quotient(CalcParser *parser, SpindleCalcInt numerator, SpindleCalcInt denominator,
                          SpindleCalcInt *out, SpindleCalcError *error) {
    SpindleCalcInt quotient = 0, remainder = 0;
    if (!calc_divmod_signed(numerator, denominator, &quotient, &remainder)) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    if (remainder)
        parser->inexact = true;
    *out = quotient;
    return true;
}

static bool calc_scaled_mul(CalcParser *parser, SpindleCalcInt left, SpindleCalcInt right,
                            SpindleCalcInt *out, SpindleCalcError *error) {
    SpindleCalcInt product = 0;
    if (!calc_mul_checked(left, right, &product)) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    return calc_quotient(parser, product, CALC_SCALE, out, error);
}

static bool calc_real(CalcComplex value, SpindleCalcInt *out, SpindleCalcError *error) {
    if (value.im) {
        *error = SPINDLE_CALC_NOT_REAL;
        return false;
    }
    *out = value.re;
    return true;
}

/* (a+bi)(c+di) = (ac-bd) + (ad+bc)i; two real operands multiply as reals. */
static bool calc_complex_mul(CalcParser *parser, CalcComplex left, CalcComplex right,
                             CalcComplex *out, SpindleCalcError *error) {
    SpindleCalcInt re = 0, im = 0;
    if (!left.im && !right.im) {
        if (!calc_scaled_mul(parser, left.re, right.re, &re, error))
            return false;
        *out = (CalcComplex){re, 0};
        return true;
    }
    SpindleCalcInt ac = 0, bd = 0, ad = 0, bc = 0;
    if (!calc_mul_checked(left.re, right.re, &ac) || !calc_mul_checked(left.im, right.im, &bd) ||
        !calc_sub_checked(ac, bd, &re) || !calc_mul_checked(left.re, right.im, &ad) ||
        !calc_mul_checked(left.im, right.re, &bc) || !calc_add_checked(ad, bc, &im)) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    if (!calc_quotient(parser, re, CALC_SCALE, &re, error) ||
        !calc_quotient(parser, im, CALC_SCALE, &im, error))
        return false;
    *out = (CalcComplex){re, im};
    return true;
}

/* (a+bi)/(c+di) = ((ac+bd) + (bc-ad)i) / (c^2+d^2); a real divisor divides each part. */
static bool calc_complex_div(CalcParser *parser, CalcComplex left, CalcComplex right,
                             CalcComplex *out, SpindleCalcError *error) {
    if (!right.re && !right.im) {
        *error = SPINDLE_CALC_DIVIDE_BY_ZERO;
        return false;
    }
    SpindleCalcInt re = left.re, im = left.im, denominator = right.re;
    if (right.im) {
        SpindleCalcInt cc = 0, dd = 0, ac = 0, bd = 0, bc = 0, ad = 0;
        if (!calc_mul_checked(right.re, right.re, &cc) ||
            !calc_mul_checked(right.im, right.im, &dd) || !calc_add_checked(cc, dd, &denominator) ||
            !calc_mul_checked(left.re, right.re, &ac) ||
            !calc_mul_checked(left.im, right.im, &bd) || !calc_add_checked(ac, bd, &re) ||
            !calc_mul_checked(left.im, right.re, &bc) ||
            !calc_mul_checked(left.re, right.im, &ad) || !calc_sub_checked(bc, ad, &im)) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
    }
    if (!calc_mul_checked(re, CALC_SCALE, &re) || !calc_mul_checked(im, CALC_SCALE, &im)) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    if (!calc_quotient(parser, re, denominator, &re, error) ||
        !calc_quotient(parser, im, denominator, &im, error))
        return false;
    *out = (CalcComplex){re, im};
    return true;
}

/* The integer square root, rounded down: Newton from above, no floats in Spindle. */
static SpindleCalcUInt calc_isqrt(SpindleCalcUInt n) {
    if (!n)
        return 0;
    unsigned bits = 0;
    for (SpindleCalcUInt t = n; t; t >>= 1)
        bits++;
    SpindleCalcUInt root = (SpindleCalcUInt)1 << ((bits + 1u) / 2u);
    for (;;) {
        SpindleCalcUInt quotient = 0, remainder = 0;
        calc_udivmod(n, root, &quotient, &remainder);
        SpindleCalcUInt next = (root + quotient) >> 1;
        if (next >= root)
            break;
        root = next;
    }
    return root;
}

/* |z| = sqrt(re^2 + im^2): both parts carry the scale, so the root does too. */
static bool calc_modulus(CalcParser *parser, CalcComplex value, CalcComplex *out,
                         SpindleCalcError *error) {
    SpindleCalcUInt a = calc_magnitude(value.re), b = calc_magnitude(value.im);
    if ((a >> 64) || (b >> 64) || a * a > ~(SpindleCalcUInt)0 - b * b) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    SpindleCalcUInt n = a * a + b * b, root = calc_isqrt(n);
    if (root * root != n)
        parser->inexact = true;
    *out = (CalcComplex){(SpindleCalcInt)root, 0};
    return true;
}

static bool calc_alpha(uint8_t byte) {
    return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z');
}

static bool calc_alnum(uint8_t byte) {
    return calc_alpha(byte) || (byte >= '0' && byte <= '9');
}

static bool calc_match6(const CalcParser *parser, size_t length, uint8_t a, uint8_t b, uint8_t c,
                        uint8_t d, uint8_t e, uint8_t f) {
    if (parser->at + length > parser->input->len)
        return false;
    return (length < 1 || parser->input->bytes[parser->at] == a) &&
           (length < 2 || parser->input->bytes[parser->at + 1] == b) &&
           (length < 3 || parser->input->bytes[parser->at + 2] == c) &&
           (length < 4 || parser->input->bytes[parser->at + 3] == d) &&
           (length < 5 || parser->input->bytes[parser->at + 4] == e) &&
           (length < 6 || parser->input->bytes[parser->at + 5] == f);
}

static bool calc_reserved(const SpindleCalcInput *input, size_t start, size_t length) {
    static const char names[][8] = {"ans", "pi",  "e",     "sqrt",  "abs",
                                    "min", "max", "round", "floor", "ceil"};
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++) {
        size_t n = 0;
        while (n < 8 && names[i][n])
            n++;
        if (n != length) {
            continue;
        }
        size_t k = 0;
        while (k < n && (uint8_t)names[i][k] == input->bytes[start + k])
            k++;
        if (k == n)
            return true;
    }
    return false;
}

/* Transcendental functions work at 18 decimal places (W) and round to the 9 shown; no floats. */
#define CALC_W ((SpindleCalcInt)1000000000000000000LL)
#define CALC_LN2_W ((SpindleCalcInt)693147180559945309LL)
#define CALC_LN10_W ((SpindleCalcInt)2302585092994045684LL)
#define CALC_PI_W ((SpindleCalcInt)3141592653589793238LL)

/* Truncating division and remainder, as Rust's `/` and `%`, for a nonzero divisor. */
static SpindleCalcInt calc_div(SpindleCalcInt numerator, SpindleCalcInt denominator) {
    SpindleCalcInt quotient = 0, remainder = 0;
    (void)calc_divmod_signed(numerator, denominator, &quotient, &remainder);
    return quotient;
}

static SpindleCalcInt calc_rem(SpindleCalcInt numerator, SpindleCalcInt denominator) {
    SpindleCalcInt quotient = 0, remainder = 0;
    (void)calc_divmod_signed(numerator, denominator, &quotient, &remainder);
    return remainder;
}

/* A W-scaled value rounded (half away from zero) to the 9-place scale. */
static SpindleCalcInt calc_to_scale(SpindleCalcInt w) {
    SpindleCalcInt half = 500000000;
    return w < 0 ? -calc_div(-w + half, CALC_SCALE) : calc_div(w + half, CALC_SCALE);
}

/* e^x for a W-scaled x, as a 9-place value: x = k ln2 + r with |r| <= ln2/2, then a Taylor
   series for e^r. */
static bool calc_exp_w(SpindleCalcInt x, SpindleCalcInt *out, SpindleCalcError *error) {
    if (x > (SpindleCalcInt)88 * CALC_W) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    if (x < (SpindleCalcInt)-30 * CALC_W) {
        *out = 0;
        return true;
    }
    SpindleCalcInt half_ln2 = 346573590279972654LL;
    SpindleCalcInt k = calc_div(x < 0 ? x - half_ln2 : x + half_ln2, CALC_LN2_W);
    SpindleCalcInt r = x - k * CALC_LN2_W, term = CALC_W, sum = CALC_W;
    for (SpindleCalcInt i = 1; term != 0 && i < 60; i++) {
        term = calc_div(calc_div(term * r, CALC_W), i);
        sum += term;
    }
    SpindleCalcInt v = calc_to_scale(sum);
    if (k >= 0) {
        if (k >= 127 ||
            !calc_mul_checked(v, (SpindleCalcInt)((SpindleCalcUInt)1 << (unsigned)k), out)) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        return true;
    }
    *out = -k >= 127 ? 0 : (v + ((SpindleCalcInt)1 << (unsigned)(-k - 1))) >> (unsigned)(-k);
    return true;
}

/* ln(x) for a positive 9-place x, W-scaled: x = m 2^k with m in [1, 2), and
   ln m = 2 atanh((m - 1) / (m + 1)). */
static bool calc_ln_w(SpindleCalcInt x, SpindleCalcInt *out, SpindleCalcError *error) {
    if (x <= 0) {
        *error = SPINDLE_CALC_DOMAIN;
        return false;
    }
    SpindleCalcInt k = 0, m = x;
    while (m >= 2 * CALC_SCALE) {
        m >>= 1;
        k++;
    }
    while (m < CALC_SCALE) {
        m <<= 1;
        k--;
    }
    m = m * CALC_SCALE;
    SpindleCalcInt z = calc_div((m - CALC_W) * CALC_W, m + CALC_W);
    SpindleCalcInt z2 = calc_div(z * z, CALC_W), term = z, sum = 0;
    for (SpindleCalcInt i = 1; term != 0 && i < 200; i += 2) {
        sum += calc_div(term, i);
        term = calc_div(term * z2, CALC_W);
    }
    *out = k * CALC_LN2_W + 2 * sum;
    return true;
}

/* sin and cos of a W-scaled angle of at most 1,000,000 radians, W-scaled. */
static void calc_sin_cos_at(SpindleCalcInt x, SpindleCalcInt *sine, SpindleCalcInt *cosine) {
    SpindleCalcInt two_pi = 2 * CALC_PI_W, a = calc_rem(x, two_pi);
    if (a > CALC_PI_W)
        a -= two_pi;
    else if (a < -CALC_PI_W)
        a += two_pi;
    SpindleCalcInt a2 = calc_div(a * a, CALC_W), s = a, st = a, c = CALC_W, ct = CALC_W;
    for (SpindleCalcInt i = 1; (st != 0 || ct != 0) && i < 60; i++) {
        st = calc_div(calc_div(-st * a2, CALC_W), (2 * i) * (2 * i + 1));
        ct = calc_div(calc_div(-ct * a2, CALC_W), (2 * i - 1) * (2 * i));
        s += st;
        c += ct;
    }
    *sine = s;
    *cosine = c;
}

/* sin and cos of a 9-place x (radians), W-scaled. */
static bool calc_sin_cos_w(SpindleCalcInt x, SpindleCalcInt *sine, SpindleCalcInt *cosine,
                           SpindleCalcError *error) {
    if (calc_magnitude(x) > (SpindleCalcUInt)1000000 * (SpindleCalcUInt)CALC_SCALE) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    calc_sin_cos_at(x * CALC_SCALE, sine, cosine);
    return true;
}

/* A 9-place m times a W-scaled f, rounded half away from zero; m = q W + r keeps the product in
   range. */
static bool calc_times_w(SpindleCalcInt m, SpindleCalcInt f, SpindleCalcInt *out,
                         SpindleCalcError *error) {
    SpindleCalcInt q = 0, r = 0, high = 0;
    (void)calc_divmod_signed(m, CALC_W, &q, &r);
    SpindleCalcInt half = (r < 0) != (f < 0) ? -500000000000000000LL : 500000000000000000LL;
    if (!calc_mul_checked(q, f, &high) ||
        !calc_add_checked(high, calc_div(r * f + half, CALC_W), out)) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    return true;
}

/* e^(x + yi) = e^x (cos y + i sin y) for W-scaled x and y, as 9-place parts. */
static bool calc_exp_complex_w(SpindleCalcInt x, SpindleCalcInt y, CalcComplex *out,
                               SpindleCalcError *error) {
    if (calc_magnitude(y) > (SpindleCalcUInt)1000000 * (SpindleCalcUInt)CALC_W) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    SpindleCalcInt m = 0, s = 0, c = 0, re = 0, im = 0;
    if (!calc_exp_w(x, &m, error))
        return false;
    calc_sin_cos_at(y, &s, &c);
    if (!calc_times_w(m, c, &re, error) || !calc_times_w(m, s, &im, error))
        return false;
    *out = (CalcComplex){re, im};
    return true;
}

/* The built-in named at the cursor and followed by `(`, numbered from 1; 0 if none. */
static uint8_t calc_builtin(const CalcParser *parser, size_t *name_length) {
    static const char names[][10] = {"abs", "min", "max",  "round", "floor",    "ceil",
                                     "re",  "im",  "conj", "norm",  "interfere"};
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++) {
        size_t n = 0;
        while (n < 10 && names[i][n])
            n++;
        if (parser->at + n >= parser->input->len)
            continue;
        size_t k = 0;
        while (k < n && (uint8_t)names[i][k] == parser->input->bytes[parser->at + k])
            k++;
        if (k == n && parser->input->bytes[parser->at + n] == '(') {
            *name_length = n;
            return (uint8_t)(i + 1);
        }
    }
    return 0;
}

static void calc_skip(CalcParser *parser) {
    while (parser->at < parser->input->len && parser->input->bytes[parser->at] == ' ')
        ++parser->at;
}

static bool calc_peek(CalcParser *parser, uint8_t *out) {
    calc_skip(parser);
    if (parser->at >= parser->input->len)
        return false;
    *out = parser->input->bytes[parser->at];
    return true;
}

static bool calc_expr(CalcParser *parser, CalcComplex *out, SpindleCalcError *error);
static bool calc_unary(CalcParser *parser, CalcComplex *out, SpindleCalcError *error);

static bool calc_number(CalcParser *parser, SpindleCalcInt *out, SpindleCalcError *error) {
    if (parser->current && parser->at + 2 <= parser->input->len &&
        parser->input->bytes[parser->at] == '0' &&
        (parser->input->bytes[parser->at + 1] == 'x' ||
         parser->input->bytes[parser->at + 1] == 'X' ||
         parser->input->bytes[parser->at + 1] == 'b' ||
         parser->input->bytes[parser->at + 1] == 'B')) {
        unsigned radix = (parser->input->bytes[parser->at + 1] == 'x' ||
                          parser->input->bytes[parser->at + 1] == 'X')
                             ? 16u
                             : 2u;
        parser->at += 2;
        SpindleCalcInt value = 0;
        size_t digits = 0;
        while (parser->at < parser->input->len) {
            uint8_t byte = parser->input->bytes[parser->at];
            unsigned digit;
            if (byte >= '0' && byte <= '9')
                digit = (unsigned)(byte - '0');
            else if (byte >= 'a' && byte <= 'f')
                digit = 10u + (unsigned)(byte - 'a');
            else if (byte >= 'A' && byte <= 'F')
                digit = 10u + (unsigned)(byte - 'A');
            else
                break;
            if (digit >= radix)
                break;
            SpindleCalcInt next = 0;
            if (!calc_mul_checked(value, (SpindleCalcInt)radix, &next) ||
                !calc_add_checked(next, (SpindleCalcInt)digit, &value)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            parser->at++;
            digits++;
        }
        if (!digits) {
            *error = SPINDLE_CALC_SYNTAX;
            return false;
        }
        if (!calc_mul_checked(value, CALC_SCALE, out)) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        return true;
    }
    SpindleCalcInt whole = 0, frac = 0;
    uint32_t places = 0;
    size_t digits = 0;
    bool dot = false;
    while (parser->at < parser->input->len) {
        uint8_t byte = parser->input->bytes[parser->at];
        if (byte >= '0' && byte <= '9') {
            SpindleCalcInt digit = (SpindleCalcInt)(byte - '0');
            if (dot) {
                if (places < 9) {
                    frac = frac * 10 + digit;
                    ++places;
                } else if (digit)
                    parser->inexact = true;
            } else {
                SpindleCalcInt next = 0;
                if (!calc_mul_checked(whole, (SpindleCalcInt)10, &next) ||
                    !calc_add_checked(next, digit, &whole)) {
                    *error = SPINDLE_CALC_OVERFLOW;
                    return false;
                }
            }
            ++digits;
            ++parser->at;
            continue;
        }
        if (byte == '.' && !dot) {
            dot = true;
            ++parser->at;
            continue;
        }
        break;
    }
    if (!digits) {
        *error = SPINDLE_CALC_SYNTAX;
        return false;
    }
    SpindleCalcInt scaled = 0;
    if (!calc_mul_checked(whole, CALC_SCALE, &scaled)) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    SpindleCalcInt factor = 1;
    for (uint32_t i = places; i < 9; ++i)
        factor *= 10;
    SpindleCalcInt fraction = frac * factor;
    if (!calc_add_checked(scaled, fraction, out)) {
        *error = SPINDLE_CALC_OVERFLOW;
        return false;
    }
    return true;
}

static bool calc_atom(CalcParser *parser, CalcComplex *out, SpindleCalcError *error) {
    uint8_t byte = 0;
    if (!calc_peek(parser, &byte)) {
        *error = SPINDLE_CALC_SYNTAX;
        return false;
    }
    if (byte == '(') {
        ++parser->at;
        if (parser->depth >= CALC_MAX_DEPTH) {
            *error = SPINDLE_CALC_TOO_DEEP;
            return false;
        }
        ++parser->depth;
        bool ok = calc_expr(parser, out, error);
        --parser->depth;
        if (!ok)
            return false;
        if (!calc_peek(parser, &byte) || byte != ')') {
            *error = SPINDLE_CALC_SYNTAX;
            return false;
        }
        ++parser->at;
        return true;
    }
    if ((byte >= '0' && byte <= '9') || byte == '.') {
        SpindleCalcInt value = 0;
        if (!calc_number(parser, &value, error))
            return false;
        /* `4i`: an imaginary number. */
        if (parser->current && parser->at < parser->input->len &&
            parser->input->bytes[parser->at] == 'i' &&
            (parser->at + 1 == parser->input->len ||
             !calc_alnum(parser->input->bytes[parser->at + 1]))) {
            parser->at++;
            *out = (CalcComplex){0, value};
            return true;
        }
        *out = (CalcComplex){value, 0};
        return true;
    }
    if (parser->current && byte == 's' && calc_match6(parser, 5, 's', 'q', 'r', 't', '(', 0)) {
        parser->at += 4;
        CalcComplex argument = {0, 0};
        SpindleCalcInt value = 0;
        if (!calc_atom(parser, &argument, error) || !calc_real(argument, &value, error))
            return false;
        if (value < 0) {
            *error = SPINDLE_CALC_DOMAIN;
            return false;
        }
        SpindleCalcUInt magnitude = (SpindleCalcUInt)value, limit = (SpindleCalcUInt)-1;
        SpindleCalcUInt bound = 0, unused = 0;
        calc_udivmod(limit, (SpindleCalcUInt)CALC_SCALE, &bound, &unused);
        if (magnitude > bound) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        SpindleCalcUInt n = magnitude * (SpindleCalcUInt)CALC_SCALE, root = calc_isqrt(n);
        if (root * root != n)
            parser->inexact = true;
        if (root > (SpindleCalcUInt)CALC_INT_MAX) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        *out = (CalcComplex){(SpindleCalcInt)root, 0};
        return true;
    }
    size_t name_length = 0;
    uint8_t builtin = parser->current ? calc_builtin(parser, &name_length) : 0;
    if (builtin) {
        parser->at += name_length + 1;
        CalcComplex values[8] = {{0, 0}};
        size_t count = 0;
        for (;;) {
            if (count == 8) {
                *error = SPINDLE_CALC_SYNTAX;
                return false;
            }
            if (parser->depth >= CALC_MAX_DEPTH) {
                *error = SPINDLE_CALC_TOO_DEEP;
                return false;
            }
            parser->depth++;
            bool ok = calc_expr(parser, &values[count], error);
            parser->depth--;
            if (!ok) {
                return false;
            }
            count++;
            if (!calc_peek(parser, &byte)) {
                *error = SPINDLE_CALC_SYNTAX;
                return false;
            }
            if (byte == ',') {
                parser->at++;
                continue;
            }
            if (byte == ')') {
                parser->at++;
                break;
            }
            *error = SPINDLE_CALC_SYNTAX;
            return false;
        }
        bool one = count == 1;
        SpindleCalcInt value = 0, next = 0;
        if (builtin == 11) {
            /* 2 Re(a conj(b)) = 2(re a re b + im a im b): the cross term of |a+b|^2. */
            if (count != 2) {
                *error = SPINDLE_CALC_SYNTAX;
                return false;
            }
            if (!calc_mul_checked(values[0].re, values[1].re, &value) ||
                !calc_mul_checked(values[0].im, values[1].im, &next) ||
                !calc_add_checked(value, next, &value) || !calc_mul_checked(value, 2, &value)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            if (!calc_quotient(parser, value, CALC_SCALE, &value, error))
                return false;
            *out = (CalcComplex){value, 0};
            return true;
        }
        if (builtin == 2 || builtin == 3) {
            if (!calc_real(values[0], &value, error))
                return false;
            bool minimum = builtin == 2;
            for (size_t i = 1; i < count; i++) {
                if (!calc_real(values[i], &next, error))
                    return false;
                if (minimum ? next < value : next > value)
                    value = next;
            }
            *out = (CalcComplex){value, 0};
            return true;
        }
        if (!one) {
            *error = SPINDLE_CALC_SYNTAX;
            return false;
        }
        CalcComplex z = values[0];
        if (builtin == 1 && z.im)
            return calc_modulus(parser, z, out, error);
        if (builtin == 7 || builtin == 8) {
            *out = (CalcComplex){builtin == 7 ? z.re : z.im, 0};
            return true;
        }
        if (builtin == 9) {
            if (!calc_sub_checked(0, z.im, &next)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            *out = (CalcComplex){z.re, next};
            return true;
        }
        if (builtin == 10) {
            if (!calc_mul_checked(z.re, z.re, &value) || !calc_mul_checked(z.im, z.im, &next) ||
                !calc_add_checked(value, next, &value)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            if (!calc_quotient(parser, value, CALC_SCALE, &value, error))
                return false;
            *out = (CalcComplex){value, 0};
            return true;
        }
        if (!calc_real(z, &value, error))
            return false;
        if (builtin == 1) {
            if (value == CALC_INT_MIN) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            *out = (CalcComplex){value < 0 ? -value : value, 0};
            return true;
        }
        SpindleCalcInt quotient = 0, remainder = 0, whole = 0;
        if (!calc_divmod_signed(value, CALC_SCALE, &quotient, &remainder)) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        bool fits = false;
        if (builtin == 5 || builtin == 6) {
            if (builtin == 5 && value < 0 && remainder)
                quotient--;
            if (builtin == 6 && value > 0 && remainder)
                quotient++;
            fits = calc_mul_checked(quotient, CALC_SCALE, &whole);
        } else {
            SpindleCalcUInt mag = calc_magnitude(value), rounded = 0, unused = 0;
            calc_udivmod(mag + (SpindleCalcUInt)CALC_SCALE / 2u, (SpindleCalcUInt)CALC_SCALE,
                         &rounded, &unused);
            SpindleCalcUInt maximum = 0;
            calc_udivmod((SpindleCalcUInt)CALC_INT_MAX, (SpindleCalcUInt)CALC_SCALE, &maximum,
                         &unused);
            fits = rounded <= maximum &&
                   calc_signed_from_magnitude(rounded * (SpindleCalcUInt)CALC_SCALE, value < 0,
                                              &whole);
        }
        if (!fits) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        *out = (CalcComplex){whole, 0};
        return true;
    }
    uint8_t function = 0;
    if (parser->current) {
        if (calc_match6(parser, 4, 'e', 'x', 'p', '(', 0, 0))
            function = 1;
        else if (calc_match6(parser, 3, 'l', 'n', '(', 0, 0, 0))
            function = 2;
        else if (calc_match6(parser, 4, 'l', 'o', 'g', '(', 0, 0))
            function = 3;
        else if (calc_match6(parser, 4, 's', 'i', 'n', '(', 0, 0))
            function = 4;
        else if (calc_match6(parser, 4, 'c', 'o', 's', '(', 0, 0))
            function = 5;
        else if (calc_match6(parser, 4, 't', 'a', 'n', '(', 0, 0))
            function = 6;
    }
    if (function) {
        parser->at += function == 2 ? 2 : 3;
        CalcComplex z = {0, 0};
        SpindleCalcInt v = 0, result = 0, sine = 0, cosine = 0;
        if (!calc_atom(parser, &z, error))
            return false;
        if (function == 1 && z.im) {
            /* e^(a+bi) = e^a (cos b + i sin b). */
            SpindleCalcInt x = 0, y = 0;
            parser->inexact = true;
            if (!calc_mul_checked(z.re, CALC_SCALE, &x) ||
                !calc_mul_checked(z.im, CALC_SCALE, &y)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            return calc_exp_complex_w(x, y, out, error);
        }
        if (!calc_real(z, &v, error))
            return false;
        /* Exact only at the trivial points (exp 0, ln 1, sin/tan 0, cos 0). */
        if (!(function == 2 || function == 3 ? v == CALC_SCALE : v == 0))
            parser->inexact = true;
        if (function == 1) {
            SpindleCalcInt x = 0;
            if (!calc_mul_checked(v, CALC_SCALE, &x)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            if (!calc_exp_w(x, &result, error))
                return false;
        } else if (function == 2 || function == 3) {
            if (!calc_ln_w(v, &result, error))
                return false;
            result = function == 2 ? calc_to_scale(result)
                                   : calc_to_scale(calc_div(result * CALC_W, CALC_LN10_W));
        } else {
            if (!calc_sin_cos_w(v, &sine, &cosine, error))
                return false;
            if (function == 4) {
                result = calc_to_scale(sine);
            } else if (function == 5) {
                result = calc_to_scale(cosine);
            } else {
                SpindleCalcInt numerator = 0;
                if (calc_magnitude(cosine) < 1000u) {
                    *error = SPINDLE_CALC_DOMAIN;
                    return false;
                }
                if (!calc_mul_checked(sine, CALC_W, &numerator)) {
                    *error = SPINDLE_CALC_OVERFLOW;
                    return false;
                }
                result = calc_to_scale(calc_div(numerator, cosine));
            }
        }
        *out = (CalcComplex){result, 0};
        return true;
    }
    if (parser->current && byte == 'p' && calc_match6(parser, 2, 'p', 'i', 0, 0, 0, 0) &&
        (parser->at + 2 == parser->input->len ||
         !calc_alnum(parser->input->bytes[parser->at + 2]))) {
        parser->at += 2;
        parser->inexact = true;
        *out = (CalcComplex){(SpindleCalcInt)3141592654LL, 0};
        return true;
    }
    if (parser->current && byte == 'e' && calc_match6(parser, 1, 'e', 0, 0, 0, 0, 0) &&
        (parser->at + 1 == parser->input->len ||
         !calc_alnum(parser->input->bytes[parser->at + 1]))) {
        parser->at++;
        parser->inexact = true;
        *out = (CalcComplex){(SpindleCalcInt)2718281828LL, 0};
        return true;
    }
    if (byte == 'a' && parser->at + 3 <= parser->input->len &&
        parser->input->bytes[parser->at + 1] == 'n' &&
        parser->input->bytes[parser->at + 2] == 's') {
        bool alnum = false;
        if (parser->at + 3 < parser->input->len) {
            uint8_t next = parser->input->bytes[parser->at + 3];
            alnum = (next >= '0' && next <= '9') || (next >= 'A' && next <= 'Z') ||
                    (next >= 'a' && next <= 'z');
        }
        if (!alnum) {
            parser->at += 3;
            if (!parser->has_answer) {
                *error = SPINDLE_CALC_NO_ANSWER;
                return false;
            }
            *out = parser->answer;
            return true;
        }
    }
    if (parser->current && calc_alpha(byte)) {
        size_t start = parser->at;
        while (parser->at < parser->input->len && calc_alnum(parser->input->bytes[parser->at]))
            parser->at++;
        size_t length = parser->at - start;
        if (parser->env)
            for (size_t i = 0; i < parser->env->count; i++) {
                const SpindleCalcVariable *variable = &parser->env->variables[i];
                if (variable->len != length)
                    continue;
                size_t k = 0;
                while (k < length && variable->name[k] == parser->input->bytes[start + k])
                    k++;
                if (k == length) {
                    *out = (CalcComplex){variable->value, variable->imag};
                    return true;
                }
            }
        *error = SPINDLE_CALC_UNKNOWN;
        return false;
    }
    *error = SPINDLE_CALC_SYNTAX;
    return false;
}

static bool calc_unary(CalcParser *parser, CalcComplex *out, SpindleCalcError *error) {
    uint8_t byte = 0;
    if (calc_peek(parser, &byte) && (byte == '-' || byte == '+')) {
        ++parser->at;
        if (parser->depth >= CALC_MAX_DEPTH) {
            *error = SPINDLE_CALC_TOO_DEEP;
            return false;
        }
        ++parser->depth;
        bool ok = calc_unary(parser, out, error);
        --parser->depth;
        if (!ok)
            return false;
        if (byte == '-') {
            SpindleCalcInt re = 0, im = 0;
            if (!calc_sub_checked(0, out->re, &re) || !calc_sub_checked(0, out->im, &im)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            *out = (CalcComplex){re, im};
        }
        return true;
    }
    CalcComplex base = {0, 0};
    if (!calc_atom(parser, &base, error))
        return false;
    if (!parser->current || !calc_peek(parser, &byte) || byte != '*' ||
        parser->at + 1 >= parser->input->len || parser->input->bytes[parser->at + 1] != '*') {
        *out = base;
        return true;
    }
    parser->at += 2;
    if (parser->depth >= CALC_MAX_DEPTH) {
        *error = SPINDLE_CALC_TOO_DEEP;
        return false;
    }
    parser->depth++;
    CalcComplex exponent = {0, 0};
    bool ok = calc_unary(parser, &exponent, error);
    parser->depth--;
    if (!ok)
        return false;
    SpindleCalcInt quotient = 0, remainder = 0;
    (void)calc_divmod_signed(exponent.re, CALC_SCALE, &quotient, &remainder);
    if (exponent.im || remainder) {
        /* A fractional or complex power of a positive base: exp(e * ln(base)). */
        SpindleCalcInt positive = 0, l = 0, x = 0, y = 0, v = 0;
        if (!calc_real(base, &positive, error))
            return false;
        if (positive <= 0) {
            *error = SPINDLE_CALC_DOMAIN;
            return false;
        }
        parser->inexact = true;
        if (!calc_ln_w(positive, &l, error))
            return false;
        if (!calc_mul_checked(l, exponent.re, &x) ||
            (exponent.im && !calc_mul_checked(l, exponent.im, &y))) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        if (exponent.im)
            return calc_exp_complex_w(calc_div(x, CALC_SCALE), calc_div(y, CALC_SCALE), out, error);
        if (!calc_exp_w(calc_div(x, CALC_SCALE), &v, error))
            return false;
        *out = (CalcComplex){v, 0};
        return true;
    }
    if (calc_magnitude(quotient) > 64u) {
        *error = SPINDLE_CALC_EXPONENT;
        return false;
    }
    CalcComplex value = {CALC_SCALE, 0};
    SpindleCalcUInt count = calc_magnitude(quotient);
    for (SpindleCalcUInt i = 0; i < count; i++)
        if (!calc_complex_mul(parser, value, base, &value, error))
            return false;
    if (quotient < 0 &&
        !calc_complex_div(parser, (CalcComplex){CALC_SCALE, 0}, value, &value, error))
        return false;
    *out = value;
    return true;
}

static bool calc_term(CalcParser *parser, CalcComplex *out, SpindleCalcError *error) {
    CalcComplex value = {0, 0};
    if (!calc_unary(parser, &value, error))
        return false;
    uint8_t op = 0;
    while (calc_peek(parser, &op) && (op == '*' || op == '/' || op == '%')) {
        if (op == '*' && parser->at + 1 < parser->input->len &&
            parser->input->bytes[parser->at + 1] == '*')
            break;
        ++parser->at;
        CalcComplex right = {0, 0};
        if (!calc_unary(parser, &right, error))
            return false;
        if (op == '*') {
            if (!calc_complex_mul(parser, value, right, &value, error))
                return false;
        } else if (op == '/') {
            if (!calc_complex_div(parser, value, right, &value, error))
                return false;
        } else {
            SpindleCalcInt left = 0, divisor = 0, quotient = 0, remainder = 0;
            if (!calc_real(value, &left, error) || !calc_real(right, &divisor, error))
                return false;
            if (!divisor) {
                *error = SPINDLE_CALC_DIVIDE_BY_ZERO;
                return false;
            }
            if (!calc_divmod_signed(left, divisor, &quotient, &remainder)) {
                *error = SPINDLE_CALC_OVERFLOW;
                return false;
            }
            value = (CalcComplex){remainder, 0};
        }
    }
    *out = value;
    return true;
}

static bool calc_expr(CalcParser *parser, CalcComplex *out, SpindleCalcError *error) {
    CalcComplex value = {0, 0};
    if (!calc_term(parser, &value, error))
        return false;
    uint8_t op = 0;
    while (calc_peek(parser, &op) && (op == '+' || op == '-')) {
        ++parser->at;
        CalcComplex right = {0, 0};
        SpindleCalcInt re = 0, im = 0;
        if (!calc_term(parser, &right, error))
            return false;
        bool valid = op == '+' ? calc_add_checked(value.re, right.re, &re) &&
                                     calc_add_checked(value.im, right.im, &im)
                               : calc_sub_checked(value.re, right.re, &re) &&
                                     calc_sub_checked(value.im, right.im, &im);
        if (!valid) {
            *error = SPINDLE_CALC_OVERFLOW;
            return false;
        }
        value = (CalcComplex){re, im};
    }
    *out = value;
    return true;
}

static SpindleCalcResult calc_eval_raw(const SpindleCalcInput *input, bool has_answer,
                                       CalcComplex answer, bool current,
                                       const SpindleCalcEnv *env) {
    SpindleCalcResult result = {0};
    if (!input || input->too_long || input->len > SPINDLE_CALC_INPUT_MAX) {
        result.error = SPINDLE_CALC_SYNTAX;
        return result;
    }
    CalcParser parser = {input, 0, 0, false, has_answer, answer, current, env};
    CalcComplex value = {0, 0};
    if (!calc_expr(&parser, &value, &result.error)) {
        return result;
    }
    uint8_t extra = 0;
    if (calc_peek(&parser, &extra)) {
        result.error = SPINDLE_CALC_SYNTAX;
        return result;
    }
    result.ok = true;
    result.value = (SpindleCalcValue){value.re, value.im, parser.inexact};
    return result;
}

SpindleCalcResult spindle_calc_eval(const SpindleCalcInput *input, bool has_answer,
                                    SpindleCalcInt answer) {
    SpindleCalcEnv empty = {0};
    return calc_eval_raw(input, has_answer, (CalcComplex){answer, 0}, false, &empty);
}

void spindle_calc_env_init(SpindleCalcEnv *env) {
    if (env)
        *env = (SpindleCalcEnv){0};
}

static bool calc_suffix(const SpindleCalcInput *input, size_t end, uint8_t a, uint8_t b, uint8_t c,
                        uint8_t d, uint8_t e, uint8_t f, uint8_t g) {
    return end >= 7 && input->bytes[end - 7] == a && input->bytes[end - 6] == b &&
           input->bytes[end - 5] == c && input->bytes[end - 4] == d && input->bytes[end - 3] == e &&
           input->bytes[end - 2] == f && input->bytes[end - 1] == g;
}

SpindleCalcSessionResult spindle_calc_eval_session(const SpindleCalcInput *input,
                                                   SpindleCalcEnv *env) {
    SpindleCalcSessionResult session = {0};
    session.radix = 10;
    if (!input || !env || input->too_long || input->len > SPINDLE_CALC_COMMAND_MAX) {
        session.result.error = SPINDLE_CALC_SYNTAX;
        return session;
    }
    size_t end = input->len;
    while (end && input->bytes[end - 1] == ' ')
        end--;
    size_t suffix = 0;
    if (calc_suffix(input, end, ' ', 'i', 'n', ' ', 'h', 'e', 'x')) {
        session.radix = 16;
        suffix = 7;
    } else if (calc_suffix(input, end, ' ', 'i', 'n', ' ', 'b', 'i', 'n')) {
        session.radix = 2;
        suffix = 7;
    } else if (calc_suffix(input, end, ' ', 'i', 'n', ' ', 'o', 'c', 't')) {
        session.radix = 8;
        suffix = 7;
    }
    size_t expression_end = end - suffix, start = 0;
    if ((suffix ? expression_end : input->len) > SPINDLE_CALC_INPUT_MAX) {
        session.result.error = SPINDLE_CALC_SYNTAX;
        return session;
    }
    while (start < expression_end && input->bytes[start] == ' ')
        start++;
    size_t name_end = start;
    while (name_end < expression_end && calc_alnum(input->bytes[name_end]))
        name_end++;
    size_t after = name_end;
    while (after < expression_end && input->bytes[after] == ' ')
        after++;
    bool assignment = name_end > start && calc_alpha(input->bytes[start]) &&
                      after < expression_end && input->bytes[after] == '=';
    if (assignment && (name_end - start > 8 || calc_reserved(input, start, name_end - start))) {
        session.result.error = SPINDLE_CALC_SYNTAX;
        return session;
    }
    size_t expr_start = assignment ? after + 1 : 0;
    SpindleCalcInput expression = {0};
    expression.len = expression_end - expr_start;
    for (size_t i = 0; i < expression.len; i++)
        expression.bytes[i] = input->bytes[expr_start + i];
    session.result = calc_eval_raw(&expression, env->has_answer,
                                   (CalcComplex){env->answer, env->answer_imag}, true, env);
    if (!session.result.ok)
        return session;
    if (assignment) {
        size_t length = name_end - start, slot = env->count;
        for (size_t i = 0; i < env->count; i++) {
            SpindleCalcVariable *v = &env->variables[i];
            if (v->len != length)
                continue;
            size_t k = 0;
            while (k < length && v->name[k] == input->bytes[start + k])
                k++;
            if (k == length) {
                slot = i;
                break;
            }
        }
        if (slot == env->count) {
            if (env->count == SPINDLE_CALC_VARIABLES) {
                session.result.ok = false;
                session.result.error = SPINDLE_CALC_OVERFLOW;
                return session;
            }
            env->count++;
        }
        SpindleCalcVariable *variable = &env->variables[slot];
        for (size_t i = 0; i < 8; i++)
            variable->name[i] = 0;
        for (size_t i = 0; i < length; i++) {
            variable->name[i] = input->bytes[start + i];
            session.assigned_name[i] = input->bytes[start + i];
        }
        variable->len = (uint8_t)length;
        variable->value = session.result.value.scaled;
        variable->imag = session.result.value.imag;
        session.assigned = true;
        session.assigned_len = (uint8_t)length;
    }
    env->has_answer = true;
    env->answer = session.result.value.scaled;
    env->answer_imag = session.result.value.imag;
    return session;
}

/* Rendered text before it is cut to a line: a complex value can be longer. */
typedef struct CalcText {
    uint8_t bytes[2 * SPINDLE_CALC_LINE_MAX];
    size_t len;
} CalcText;

static void calc_put(CalcText *text, uint8_t byte) {
    if (text->len < sizeof text->bytes)
        text->bytes[text->len++] = byte;
}

/* One scaled part's digits; trailing zeros trimmed. */
static void calc_put_fixed(CalcText *text, SpindleCalcUInt magnitude) {
    SpindleCalcUInt whole = 0, frac = 0;
    calc_udivmod(magnitude, (SpindleCalcUInt)CALC_SCALE, &whole, &frac);
    uint8_t digits[40] = {0};
    size_t count = 0;
    while (count < sizeof digits) {
        SpindleCalcUInt next = 0, digit = 0;
        calc_udivmod(whole, 10, &next, &digit);
        digits[count++] = (uint8_t)('0' + digit);
        whole = next;
        if (!whole)
            break;
    }
    while (count)
        calc_put(text, digits[--count]);
    if (frac) {
        calc_put(text, '.');
        size_t places = 9;
        for (;;) {
            SpindleCalcUInt next = 0, digit = 0;
            calc_udivmod(frac, 10, &next, &digit);
            if (digit || places == 0)
                break;
            frac = next;
            --places;
        }
        uint8_t f[9] = {0};
        for (size_t i = places; i > 0; --i) {
            SpindleCalcUInt next = 0, digit = 0;
            calc_udivmod(frac, 10, &next, &digit);
            f[i - 1] = (uint8_t)('0' + digit);
            frac = next;
        }
        for (size_t i = 0; i < places; ++i)
            calc_put(text, f[i]);
    }
}

/* `= 1.5`, `~ 0.333333333`; `= 3+4i`, `= -0.5i`: the real part when nonzero, then the
   imaginary part. */
static CalcText calc_text(SpindleCalcValue value) {
    CalcText text = {{0}, 0};
    calc_put(&text, value.inexact ? '~' : '=');
    calc_put(&text, ' ');
    if (value.scaled || !value.imag) {
        if (value.scaled < 0)
            calc_put(&text, '-');
        calc_put_fixed(&text, calc_magnitude(value.scaled));
        if (!value.imag)
            return text;
        calc_put(&text, value.imag < 0 ? '-' : '+');
    } else if (value.imag < 0)
        calc_put(&text, '-');
    calc_put_fixed(&text, calc_magnitude(value.imag));
    calc_put(&text, 'i');
    return text;
}

static SpindleCalcLine calc_line(const CalcText *text) {
    SpindleCalcLine line = {{0}, 0};
    line.len = text->len < SPINDLE_CALC_LINE_MAX ? text->len : SPINDLE_CALC_LINE_MAX;
    for (size_t i = 0; i < line.len; i++)
        line.bytes[i] = text->bytes[i];
    return line;
}

/* A complex value longer than the line is cut here; spindle_calc_render_session refuses it. */
SpindleCalcLine spindle_calc_render(SpindleCalcValue value) {
    CalcText text = calc_text(value);
    return calc_line(&text);
}

/* A whole part's digits in base 16, 2 or 8 after its sign and prefix; plus writes `+` before a
   non-negative part. */
static void calc_put_radix(CalcText *text, SpindleCalcInt whole, uint32_t radix, bool plus) {
    static const uint8_t alphabet[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                         '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    uint8_t digits[128] = {0};
    size_t count = 0;
    SpindleCalcUInt magnitude = calc_magnitude(whole);
    bool more = true;
    while (more && count < sizeof digits) {
        SpindleCalcUInt q = 0, r = 0;
        calc_udivmod(magnitude, radix, &q, &r);
        digits[count++] = alphabet[(size_t)r];
        magnitude = q;
        more = magnitude != 0;
    }
    if (whole < 0)
        calc_put(text, '-');
    else if (plus)
        calc_put(text, '+');
    calc_put(text, '0');
    calc_put(text, radix == 16 ? 'x' : radix == 2 ? 'b' : 'o');
    while (count)
        calc_put(text, digits[--count]);
}

SpindleCalcRenderResult spindle_calc_render_session(SpindleCalcSessionResult session) {
    SpindleCalcRenderResult out = {0};
    if (!session.result.ok) {
        out.error = session.result.error;
        return out;
    }
    SpindleCalcValue value = session.result.value;
    if (session.radix == 10) {
        CalcText text = calc_text(value);
        if (value.imag && text.len > SPINDLE_CALC_LINE_MAX) {
            out.error = SPINDLE_CALC_OVERFLOW;
            return out;
        }
        out.ok = true;
        out.line = calc_line(&text);
        return out;
    }
    SpindleCalcInt whole = 0, remainder = 0, imag_whole = 0, imag_remainder = 0;
    if (value.inexact || !calc_divmod_signed(value.scaled, CALC_SCALE, &whole, &remainder) ||
        remainder || !calc_divmod_signed(value.imag, CALC_SCALE, &imag_whole, &imag_remainder) ||
        imag_remainder) {
        out.error = SPINDLE_CALC_NOT_WHOLE;
        return out;
    }
    CalcText text = {{0}, 0};
    calc_put(&text, '=');
    calc_put(&text, ' ');
    if (!value.imag || value.scaled)
        calc_put_radix(&text, whole, session.radix, false);
    if (value.imag) {
        calc_put_radix(&text, imag_whole, session.radix, value.scaled != 0);
        calc_put(&text, 'i');
    }
    if (text.len > SPINDLE_CALC_LINE_MAX) {
        out.error = SPINDLE_CALC_OVERFLOW;
        return out;
    }
    out.ok = true;
    out.line = calc_line(&text);
    return out;
}

const char *spindle_calc_error_text(SpindleCalcError error) {
    switch (error) {
    case SPINDLE_CALC_DIVIDE_BY_ZERO:
        return "calc: division by zero";
    case SPINDLE_CALC_OVERFLOW:
        return "calc: number too large";
    case SPINDLE_CALC_TOO_DEEP:
        return "calc: nested too deeply";
    case SPINDLE_CALC_NO_ANSWER:
        return "calc: no previous answer (ans)";
    case SPINDLE_CALC_EXPONENT:
        return "calc: ** whole exponents go up to 64";
    case SPINDLE_CALC_DOMAIN:
        return "calc: square root of a negative number";
    case SPINDLE_CALC_UNKNOWN:
        return "calc: unknown name (set one with  calc x = 2)";
    case SPINDLE_CALC_NOT_WHOLE:
        return "calc: only whole results show in hex/bin/oct";
    case SPINDLE_CALC_NOT_REAL:
        return "calc: needs a real number";
    default:
        return "usage: calc <expr>  e.g. calc 2*(3+4)/5";
    }
}

const char *spindle_calc_error_name(SpindleCalcError error) {
    switch (error) {
    case SPINDLE_CALC_DIVIDE_BY_ZERO:
        return "DivideByZero";
    case SPINDLE_CALC_OVERFLOW:
        return "Overflow";
    case SPINDLE_CALC_TOO_DEEP:
        return "TooDeep";
    case SPINDLE_CALC_NO_ANSWER:
        return "NoAnswer";
    case SPINDLE_CALC_EXPONENT:
        return "Exponent";
    case SPINDLE_CALC_DOMAIN:
        return "Domain";
    case SPINDLE_CALC_UNKNOWN:
        return "Unknown";
    case SPINDLE_CALC_NOT_WHOLE:
        return "NotWhole";
    case SPINDLE_CALC_NOT_REAL:
        return "NotReal";
    default:
        return "Syntax";
    }
}


