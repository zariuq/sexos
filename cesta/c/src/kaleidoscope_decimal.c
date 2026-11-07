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

static double decimal_from_bits(uint64_t bits) {
    union {
        uint64_t u;
        double d;
    } v = {bits};

    return v.d;
}

static uint64_t decimal_to_bits(double x) {
    union {
        double d;
        uint64_t u;
    } v = {x};

    return v.u;
}

/* Naturals held in 32-bit words, least significant first, with no leading zero word. 4096 bits
   hold every intermediate, up to 800 parsed significant digits. */
#define DECIMAL_BIG_WORDS 128

typedef struct {
    size_t n;
    uint32_t w[DECIMAL_BIG_WORDS];
} DecimalBig;

static void decimal_big_trim(DecimalBig *b) {
    while (b->n && !b->w[b->n - 1])
        b->n--;
}

static void decimal_big_set(DecimalBig *b, uint64_t value) {
    b->w[0] = (uint32_t)value;
    b->w[1] = (uint32_t)(value >> 32);
    b->n = 2;
    decimal_big_trim(b);
}

static void decimal_big_mul_small(DecimalBig *b, uint32_t k) {
    uint64_t carry = 0;
    for (size_t i = 0; i < b->n; i++) {
        uint64_t v = (uint64_t)b->w[i] * k + carry;
        b->w[i] = (uint32_t)v;
        carry = v >> 32;
    }
    if (carry && b->n < DECIMAL_BIG_WORDS)
        b->w[b->n++] = (uint32_t)carry;
    decimal_big_trim(b);
}

static void decimal_big_add_small(DecimalBig *b, uint32_t k) {
    uint64_t carry = k;
    for (size_t i = 0; i < b->n && carry; i++) {
        uint64_t v = (uint64_t)b->w[i] + carry;
        b->w[i] = (uint32_t)v;
        carry = v >> 32;
    }
    if (carry && b->n < DECIMAL_BIG_WORDS)
        b->w[b->n++] = (uint32_t)carry;
}

static void decimal_big_mul_pow10(DecimalBig *b, size_t k) {
    static const uint32_t small[9] = {1u,      10u,      100u,      1000u,     10000u,
                                      100000u, 1000000u, 10000000u, 100000000u};
    while (k >= 9) {
        decimal_big_mul_small(b, 1000000000u);
        k -= 9;
    }
    if (k)
        decimal_big_mul_small(b, small[k]);
}

/* Divides by a small divisor, returning the remainder. */
static uint32_t decimal_big_div_small(DecimalBig *b, uint32_t divisor) {
    uint64_t remainder = 0;
    for (size_t i = b->n; i-- > 0;) {
        uint64_t current = remainder << 32 | b->w[i];
        b->w[i] = (uint32_t)(current / divisor);
        remainder = current % divisor;
    }
    decimal_big_trim(b);
    return (uint32_t)remainder;
}

static void decimal_big_shl(DecimalBig *b, size_t bits) {
    if (!b->n)
        return;
    size_t words = bits / 32, shift = bits % 32, total = b->n + words + 1;
    if (total > DECIMAL_BIG_WORDS)
        total = DECIMAL_BIG_WORDS;
    for (size_t i = total; i-- > 0;) {
        uint32_t high = i >= words && i - words < b->n ? b->w[i - words] : 0;
        uint32_t low = i >= words + 1 && i - words - 1 < b->n ? b->w[i - words - 1] : 0;
        b->w[i] = shift ? high << shift | low >> (32 - shift) : high;
    }
    b->n = total;
    decimal_big_trim(b);
}

static void decimal_big_shr1(DecimalBig *b) {
    for (size_t i = 0; i < b->n; i++)
        b->w[i] = b->w[i] >> 1 | (i + 1 < b->n ? b->w[i + 1] << 31 : 0u);
    decimal_big_trim(b);
}

static int decimal_big_cmp(const DecimalBig *a, const DecimalBig *b) {
    if (a->n != b->n)
        return a->n < b->n ? -1 : 1;
    for (size_t i = a->n; i-- > 0;)
        if (a->w[i] != b->w[i])
            return a->w[i] < b->w[i] ? -1 : 1;
    return 0;
}

static void decimal_big_add(DecimalBig *a, const DecimalBig *b) {
    size_t length = a->n > b->n ? a->n : b->n;
    uint64_t carry = 0;
    for (size_t i = 0; i < length; i++) {
        uint64_t v = carry + (i < a->n ? a->w[i] : 0u) + (i < b->n ? b->w[i] : 0u);
        a->w[i] = (uint32_t)v;
        carry = v >> 32;
    }
    a->n = length;
    if (carry && length < DECIMAL_BIG_WORDS)
        a->w[a->n++] = (uint32_t)carry;
}

/* a -= b, given a >= b. */
static void decimal_big_sub(DecimalBig *a, const DecimalBig *b) {
    uint64_t borrow = 0;
    for (size_t i = 0; i < a->n; i++) {
        uint64_t take = (uint64_t)(i < b->n ? b->w[i] : 0u) + borrow;
        borrow = a->w[i] < take;
        a->w[i] = (uint32_t)((uint64_t)a->w[i] - take);
    }
    decimal_big_trim(a);
}

static size_t decimal_big_bits(const DecimalBig *b) {
    if (!b->n)
        return 0;
    size_t bits = (b->n - 1) * 32;
    for (uint32_t top = b->w[b->n - 1]; top; top >>= 1)
        bits++;
    return bits;
}

static void decimal_big_copy(DecimalBig *to, const DecimalBig *from) {
    to->n = from->n;
    for (size_t i = 0; i < from->n; i++)
        to->w[i] = from->w[i];
}

/* Rust's flt2dec decoding of a finite nonzero double: v = mant 2^exp, and every number in
   ((mant - minus) 2^exp, (mant + plus) 2^exp) reads back as v, the ends too when inclusive. */
typedef struct {
    uint64_t mant, minus, plus;
    int32_t exp;
    bool inclusive;
} DecimalDecoded;

static DecimalDecoded decimal_decode(double v) {
    uint64_t bits = decimal_to_bits(v), fraction = bits & UINT64_C(0xfffffffffffff);
    int32_t field = (int32_t)((bits >> 52) & 0x7ff);
    uint64_t mant = field == 0 ? fraction << 1 : fraction | UINT64_C(1) << 52;
    DecimalDecoded d = {mant, 1, 1, field - 1075, (mant & 1) == 0};
    if (field != 0 && mant == UINT64_C(1) << 52) {
        d.mant = mant << 2;
        d.plus = 2;
        d.exp -= 2;
    } else if (field != 0) {
        d.mant = mant << 1;
        d.exp -= 1;
    }
    return d;
}

/* Rust's estimate_scaling_factor: k with 10^(k-1) < mant 2^exp <= 10^(k+1). */
static int32_t decimal_estimate_scale(uint64_t mant, int32_t exp) {
    int64_t bits = 0;
    for (uint64_t x = mant - 1; x; x >>= 1)
        bits++;
    int64_t p = (bits + exp) * INT64_C(1292913986);
    return (int32_t)(p >= 0 ? p >> 32 : -(int64_t)(((uint64_t)-p - 1) >> 32) - 1);
}

/* The next digit floor(mant / scale) < 10, leaving the remainder in mant. */
static uint8_t decimal_big_digit(DecimalBig *mant, const DecimalBig *scale) {
    uint8_t digit = 0;
    while (decimal_big_cmp(mant, scale) >= 0) {
        decimal_big_sub(mant, scale);
        digit++;
    }
    return digit;
}

/* Rust's round_up: increments the decimal digits; on a carry out of all nines, returns the
   digit to append (and the digits become 10...0). */
static uint8_t decimal_round_up(KaleidoscopeDigits *digits) {
    for (size_t i = digits->len; i-- > 0;)
        if (digits->digits[i] != '9') {
            digits->digits[i]++;
            for (size_t j = i + 1; j < digits->len; j++)
                digits->digits[j] = '0';
            return 0;
        }
    if (!digits->len)
        return '1';
    digits->digits[0] = '1';
    for (size_t j = 1; j < digits->len; j++)
        digits->digits[j] = '0';
    return '0';
}

/* Rust's Dragon `format_shortest`: the fewest digits that read back as v (finite, nonzero),
   the closest of those. */
void kaleidoscope_decimal_shortest(double v, KaleidoscopeDigits *out) {
    DecimalDecoded d = decimal_decode(v);
    int rounding = d.inclusive ? 1 : 0;
    int32_t k = decimal_estimate_scale(d.mant + d.plus, d.exp);
    DecimalBig mant = {0}, minus = {0}, plus = {0}, scale = {0}, sum = {0};
    decimal_big_set(&mant, d.mant);
    decimal_big_set(&minus, d.minus);
    decimal_big_set(&plus, d.plus);
    decimal_big_set(&scale, 1);
    if (d.exp < 0)
        decimal_big_shl(&scale, (size_t)-d.exp);
    else {
        decimal_big_shl(&mant, (size_t)d.exp);
        decimal_big_shl(&minus, (size_t)d.exp);
        decimal_big_shl(&plus, (size_t)d.exp);
    }
    if (k >= 0)
        decimal_big_mul_pow10(&scale, (size_t)k);
    else {
        decimal_big_mul_pow10(&mant, (size_t)-k);
        decimal_big_mul_pow10(&minus, (size_t)-k);
        decimal_big_mul_pow10(&plus, (size_t)-k);
    }
    decimal_big_copy(&sum, &mant);
    decimal_big_add(&sum, &plus);
    if (decimal_big_cmp(&scale, &sum) < rounding)
        k++;
    else {
        decimal_big_mul_small(&mant, 10);
        decimal_big_mul_small(&minus, 10);
        decimal_big_mul_small(&plus, 10);
    }
    out->len = 0;
    bool down = false, up = false;
    while (out->len < 17) {
        out->digits[out->len++] = (uint8_t)('0' + decimal_big_digit(&mant, &scale));
        down = decimal_big_cmp(&mant, &minus) < rounding;
        decimal_big_copy(&sum, &mant);
        decimal_big_add(&sum, &plus);
        up = decimal_big_cmp(&scale, &sum) < rounding;
        if (down || up)
            break;
        decimal_big_mul_small(&mant, 10);
        decimal_big_mul_small(&minus, 10);
        decimal_big_mul_small(&plus, 10);
    }
    bool increase = up && !down;
    if (up && down) {
        decimal_big_copy(&sum, &mant);
        decimal_big_shl(&sum, 1);
        increase = decimal_big_cmp(&sum, &scale) >= 0;
    }
    if (increase) {
        uint8_t carry = decimal_round_up(out);
        if (carry) {
            out->digits[out->len++] = carry;
            k++;
        }
    }
    out->k = k;
}

/* Rust's Dragon `format_exact` as `{:.N}` calls it: the digits of v (finite, nonzero) down to
   10^limit, the last rounded half to even; at most 63 here. */
void kaleidoscope_decimal_exact(double v, int32_t limit, KaleidoscopeDigits *out) {
    DecimalDecoded d = decimal_decode(v);
    int32_t k = decimal_estimate_scale(d.mant, d.exp);
    size_t room = 21 + ((size_t)((d.exp < 0 ? -12 : 5) * d.exp) >> 4);
    DecimalBig mant = {0}, scale = {0}, t = {0};
    decimal_big_set(&mant, d.mant);
    decimal_big_set(&scale, 1);
    if (d.exp < 0)
        decimal_big_shl(&scale, (size_t)-d.exp);
    else
        decimal_big_shl(&mant, (size_t)d.exp);
    if (k >= 0)
        decimal_big_mul_pow10(&scale, (size_t)k);
    else
        decimal_big_mul_pow10(&mant, (size_t)-k);
    /* Rust's fixup: whether v rounded at `room` digits reaches 10^k. */
    decimal_big_copy(&t, &scale);
    size_t tens = room;
    while (tens > 9) {
        decimal_big_div_small(&t, 1000000000u);
        tens -= 9;
    }
    static const uint32_t powers[10] = {1u,      10u,      100u,      1000u,      10000u,
                                        100000u, 1000000u, 10000000u, 100000000u, 1000000000u};
    decimal_big_div_small(&t, powers[tens] << 1);
    decimal_big_add(&t, &mant);
    if (decimal_big_cmp(&t, &scale) >= 0)
        k++;
    else
        decimal_big_mul_small(&mant, 10);
    size_t length = k < limit                             ? 0
                    : (size_t)((int64_t)k - limit) < room ? (size_t)((int64_t)k - limit)
                                                          : room;
    if (length > 63)
        length = 63;
    out->len = length;
    out->k = k;
    for (size_t i = 0; i < length; i++) {
        if (!mant.n) {
            for (size_t j = i; j < length; j++)
                out->digits[j] = '0';
            return;
        }
        out->digits[i] = (uint8_t)('0' + decimal_big_digit(&mant, &scale));
        decimal_big_mul_small(&mant, 10);
    }
    decimal_big_mul_small(&scale, 5);
    int order = decimal_big_cmp(&mant, &scale);
    if (order > 0 || (order == 0 && length > 0 && (out->digits[length - 1] & 1))) {
        uint8_t carry = decimal_round_up(out);
        if (carry) {
            out->k++;
            if (out->k > limit && out->len < room && out->len < 63)
                out->digits[out->len++] = carry;
        }
    }
}

/* Adds the next digit of the integer part (fraction false) or the fraction part. */
void kaleidoscope_decimal_digit(KaleidoscopeDecimal *decimal, uint8_t digit, bool fraction) {
    if (decimal->count == 0 && digit == 0) {
        if (fraction)
            decimal->exp10--;
        return;
    }
    if (decimal->count < KALEIDOSCOPE_DECIMAL_DIGITS) {
        decimal->digits[decimal->count++] = digit;
        if (fraction)
            decimal->exp10--;
        return;
    }
    decimal->sticky |= digit != 0;
    if (!fraction)
        decimal->exp10++;
}

/* Rust's correctly rounded `parse::<f64>` value of a decimal. */
double kaleidoscope_decimal_value(const KaleidoscopeDecimal *decimal, int64_t exponent) {
    uint64_t sign = decimal->negative ? UINT64_C(1) << 63 : 0;
    int64_t exp10 = decimal->exp10 + exponent, magnitude = (int64_t)decimal->count + exp10;
    if (!decimal->count || magnitude < -324)
        return decimal_from_bits(sign);
    if (magnitude > 310)
        return decimal_from_bits(sign | UINT64_C(0x7ff0000000000000));
    if (!decimal->sticky && decimal->count <= 15 && exp10 >= -22 && exp10 <= 22) {
        static const double powers[23] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
                                          1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
                                          1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};
        uint64_t whole = 0;
        for (size_t i = 0; i < decimal->count; i++)
            whole = whole * 10 + decimal->digits[i];
        double r = exp10 >= 0 ? (double)whole * powers[exp10] : (double)whole / powers[-exp10];
        return decimal_from_bits(decimal_to_bits(r) | sign);
    }
    DecimalBig num = {0}, den = {0};
    for (size_t i = 0; i < decimal->count; i++) {
        decimal_big_mul_small(&num, 10);
        decimal_big_add_small(&num, decimal->digits[i]);
    }
    decimal_big_set(&den, 1);
    if (exp10 >= 0)
        decimal_big_mul_pow10(&num, (size_t)exp10);
    else
        decimal_big_mul_pow10(&den, (size_t)-exp10);
    /* q = floor(num 2^s / den) has 55 or 56 bits. */
    int64_t s = 55 - (int64_t)decimal_big_bits(&num) + (int64_t)decimal_big_bits(&den);
    if (s > 0)
        decimal_big_shl(&num, (size_t)s);
    else if (s < 0)
        decimal_big_shl(&den, (size_t)-s);
    decimal_big_shl(&den, 56);
    uint64_t q = 0;
    for (int bit = 56; bit >= 0; bit--) {
        if (decimal_big_cmp(&num, &den) >= 0) {
            decimal_big_sub(&num, &den);
            q |= UINT64_C(1) << bit;
        }
        decimal_big_shr1(&den);
    }
    bool rest = num.n != 0 || decimal->sticky;
    int64_t bits = 0;
    for (uint64_t x = q; x; x >>= 1)
        bits++;
    int64_t lead = bits - 1 - s, drop = bits - 53;
    if (lead < -1022)
        drop += -1022 - lead;
    uint64_t kept = 0;
    if (drop < 64) {
        kept = q >> drop;
        uint64_t remainder = q & ((UINT64_C(1) << drop) - 1), half = UINT64_C(1) << (drop - 1);
        if (remainder > half || (remainder == half && (rest || (kept & 1))))
            kept++;
    }
    int64_t unit = drop - s;
    if (kept == UINT64_C(1) << 53) {
        kept >>= 1;
        unit++;
    }
    if (kept < UINT64_C(1) << 52)
        return decimal_from_bits(sign | kept);
    int64_t biased = unit + 52 + 1023;
    if (biased >= 2047)
        return decimal_from_bits(sign | UINT64_C(0x7ff0000000000000));
    return decimal_from_bits(sign | (uint64_t)biased << 52 | (kept & UINT64_C(0xfffffffffffff)));
}


