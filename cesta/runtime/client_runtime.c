#include "sexos_client.h"

#include <stdarg.h>

size_t sexos_strlen(const char *text)
{
    size_t length = 0;
    while (text[length]) ++length;
    return length;
}

void sexos_raw_print(const char *text, size_t length)
{
    (void)sexos_pdx_call(SEXOS_SLOT_kernel, SEXOS_KERNEL_raw_print,
                       (uint64_t)(uintptr_t)text, length, 0);
}

_Noreturn void sexos_fail(void)
{
    static const char message[] = "[cesta.client.fault] checked contract failed\n";
    sexos_raw_print(message, sizeof message - 1);
    for (;;) sexos_yield();
}

void packed_words_failure(void) { sexos_fail(); }
void format_bytes_failure(void) { sexos_fail(); }

/* Native bridge obligation: format and string arguments are valid terminated
 * strings. Numeric fields have the C printf types of their explicit format.
 * Checked helpers bound every write into the local line buffer. raw_print
 * consumes it synchronously; the kernel must not retain its stack pointer. */
void sexos_line(const char *format, ...)
{
    /* Public checked views require initialized byte ranges. */
    uint8_t storage[512] = {0};
    size_t length = 0;
    va_list arguments;
    va_start(arguments, format);
    while (*format) {
        const char *literal = format;
        while (*format && *format != '%') ++format;
        if (format != literal) {
            cesta_write_bytes out = {storage + length, sizeof storage - length};
            cesta_read_bytes in = {(const uint8_t *)literal, (size_t)(format - literal)};
            length += format_bytes_copy(out, in);
        }
        if (!*format) break;
        ++format;
        if (*format == '%') {
            cesta_write_bytes out = {storage + length, sizeof storage - length};
            static const uint8_t percent = '%';
            cesta_read_bytes in = {&percent, 1};
            length += format_bytes_copy(out, in);
            ++format;
            continue;
        }
        if (*format == 's') {
            const char *text = va_arg(arguments, const char *);
            cesta_write_bytes out = {storage + length, sizeof storage - length};
            cesta_read_bytes in = {(const uint8_t *)text, sexos_strlen(text)};
            length += format_bytes_copy(out, in);
            ++format;
            continue;
        }
        bool prefix = false;
        if (*format == '#') { prefix = true; ++format; }
        /* Zero-padded hexadecimal width, as Rust's {:016x}: %0Nx, %0Nllx. */
        size_t width = 0;
        if (*format == '0') {
            ++format;
            while (*format >= '0' && *format <= '9') width = width*10 + (size_t)(*format++ - '0');
            if (!width || width > 32) sexos_fail();
        }
        enum { field_unsigned, field_long_long, field_size } field = field_unsigned;
        if (format[0] == 'l' && format[1] == 'l') {
            field = field_long_long;
            format += 2;
        } else if (*format == 'z') {
            field = field_size;
            ++format;
        }
        /* X is hexadecimal with uppercase digits, as Rust's {:X}. */
        bool upper = *format == 'X';
        if ((*format != 'u' && *format != 'x' && !upper && *format != 'd') || (*format != 'x' && prefix)) sexos_fail();
        uint64_t value;
        if (*format == 'd') {
            /* Signed decimal, as Rust's {} on i8..i64: a sign, then the magnitude. */
            if (field == field_size || width) sexos_fail();
            int64_t signed_value = field == field_long_long ? (int64_t)va_arg(arguments, long long) :
                                   va_arg(arguments, int);
            value = (uint64_t)signed_value;
            if (signed_value < 0) {
                cesta_write_bytes minus_out = {storage + length, sizeof storage - length};
                static const uint8_t minus = '-';
                cesta_read_bytes minus_in = {&minus, 1};
                length += format_bytes_copy(minus_out, minus_in);
                value = 0 - value;
            }
        } else {
            value = field == field_long_long ? (uint64_t)va_arg(arguments, unsigned long long) :
                    field == field_size ? (uint64_t)va_arg(arguments, size_t) :
                    va_arg(arguments, unsigned int);
        }
        if (width) {
            if ((*format != 'x' && !upper) || prefix) sexos_fail();
            size_t digits = 1;
            for (uint64_t rest = value >> 4; rest; rest >>= 4) ++digits;
            for (; digits < width; ++digits) {
                cesta_write_bytes zero_out = {storage + length, sizeof storage - length};
                static const uint8_t zero = '0';
                cesta_read_bytes zero_in = {&zero, 1};
                length += format_bytes_copy(zero_out, zero_in);
            }
        }
        cesta_write_bytes out = {storage + length, sizeof storage - length};
        if (*format == 'u' || *format == 'd')
            length += format_bytes_unsigned_decimal(out, value);
        else if (*format == 'x' || upper) {
            size_t start = length;
            length += format_bytes_unsigned_hex(out, value, prefix);
            if (upper)
                for (size_t i = start; i < length; ++i)
                    if (storage[i] >= 'a' && storage[i] <= 'f') storage[i] = (uint8_t)(storage[i] - 'a' + 'A');
        }
        else sexos_fail();
        ++format;
    }
    va_end(arguments);
    cesta_write_bytes out = {storage + length, sizeof storage - length};
    static const uint8_t newline = '\n';
    cesta_read_bytes in = {&newline, 1};
    length += format_bytes_copy(out, in);
    sexos_raw_print((const char *)storage, length);
}

SexosMessage sexos_listen(SexosListener *listener, uint64_t slot)
{
    for (;;) {
        SexosMessage message = sexos_listen_once(slot);
        if (!message.type_id) {
#if defined(SEXOS_PDX_SENDER_TASK) && SEXOS_PDX_SENDER_TASK
            if (!slot) sexos_wait_self(); else sexos_yield();
#else
            sexos_yield();
#endif
            continue;
        }
        if (listener->wrapper_budget) {
            --listener->wrapper_budget;
            sexos_line("[pdx.listen.raw.wrapper] type=%#llx caller=%u a0=%#llx a1=%#llx",
                       (unsigned long long)message.type_id, message.caller_pd,
                       (unsigned long long)message.arg0, (unsigned long long)message.arg1);
        }
        return message;
    }
}

#ifndef SEXOS_HOST
void *memcpy(void *destination, const void *source, size_t count)
{
    volatile uint8_t *out = destination;
    const volatile uint8_t *in = source;
    for (size_t i = 0; i < count; ++i) out[i] = in[i];
    return destination;
}

void *memmove(void *destination, const void *source, size_t count)
{
    volatile uint8_t *out = destination;
    const volatile uint8_t *in = source;
    if ((uintptr_t)out <= (uintptr_t)in) {
        for (size_t i = 0; i < count; ++i) out[i] = in[i];
    } else {
        for (size_t i = count; i; --i) out[i - 1] = in[i - 1];
    }
    return destination;
}

void *memset(void *destination, int value, size_t count)
{
    volatile uint8_t *out = destination;
    for (size_t i = 0; i < count; ++i) out[i] = (uint8_t)value;
    return destination;
}

int memcmp(const void *first, const void *second, size_t count)
{
    const volatile uint8_t *a = first, *b = second;
    for (size_t i = 0; i < count; ++i)
        if (a[i] != b[i]) return (int)a[i] - (int)b[i];
    return 0;
}
#endif
