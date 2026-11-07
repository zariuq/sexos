
#ifndef KALEIDOSCOPE_DECIMAL_H
#define KALEIDOSCOPE_DECIMAL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Exact conversion between decimal text and doubles, as Rust's core library does it: the
   correctly rounded `parse::<f64>`, and the Dragon digit generators behind `{}`, `{:e}` and
   `{:.N}`. No libm, no 128-bit division. */
#define KALEIDOSCOPE_DECIMAL_DIGITS ((size_t)800)

/* A decimal being read: its significant digits (the first nonzero; at most
   KALEIDOSCOPE_DECIMAL_DIGITS kept, `sticky` marking nonzero digits dropped after them), its
   sign, and the power of ten of the last kept digit. Starts zeroed. */
typedef struct KaleidoscopeDecimal {
    size_t count;
    bool sticky, negative;
    int64_t exp10;
    uint8_t digits[KALEIDOSCOPE_DECIMAL_DIGITS];
} KaleidoscopeDecimal;

/* Decimal digits ('0'..'9') d1d2... of the value 0.d1d2... 10^k. */
typedef struct KaleidoscopeDigits {
    size_t len;
    int32_t k;
    uint8_t digits[64];
} KaleidoscopeDigits;

/* Adds the next digit of the integer part (fraction false) or of the fraction part. */
void kaleidoscope_decimal_digit(KaleidoscopeDecimal *decimal, uint8_t digit, bool fraction);
/* The double nearest the decimal times 10^exponent, ties to even. */
double kaleidoscope_decimal_value(const KaleidoscopeDecimal *decimal, int64_t exponent);
/* The fewest digits that read back as value (finite, nonzero), the closest of those. */
void kaleidoscope_decimal_shortest(double value, KaleidoscopeDigits *out);
/* The digits of value (finite, nonzero) down to 10^limit, the last rounded half to even;
   at most 63. */
void kaleidoscope_decimal_exact(double value, int32_t limit, KaleidoscopeDigits *out);
#endif
