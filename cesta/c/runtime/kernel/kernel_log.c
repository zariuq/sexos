/* The kernel's serial line: COM1 and one formatter. Part of the native boundary, like the userland's
 * bounded line assembly. Each call writes one record with interrupts held off, so records from
 * interrupt handlers never split a record of the interrupted code. */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu_x86_64.h"
#include "kernel_log.h"

#define COM1 0x3F8

static int serial_present;

void kernel_serial_init(void)
{
    /* The scratch register holds what was written only when a UART is present. */
    cpu_outb(COM1 + 7, 0x5A);
    if (cpu_inb(COM1 + 7) != 0x5A)
        return;
    cpu_outb(COM1 + 1, 0x00);
    cpu_outb(COM1 + 3, 0x80);
    cpu_outb(COM1 + 0, 0x03);
    cpu_outb(COM1 + 1, 0x00);
    cpu_outb(COM1 + 3, 0x03);
    cpu_outb(COM1 + 2, 0xC7);
    cpu_outb(COM1 + 4, 0x0B);
    serial_present = 1;
}

static void put(char c)
{
    if (!serial_present)
        return;
    while ((cpu_inb(COM1 + 5) & 0x20) == 0)
        cpu_pause();
    cpu_outb(COM1, (uint8_t)c);
}

void kernel_serial_write(const uint8_t *bytes, size_t count)
{
    int enabled = cpu_irq_save();
    for (size_t i = 0; i < count; i++)
        put((char)bytes[i]);
    cpu_irq_restore(enabled);
}

static void put_unsigned(uint64_t value, unsigned base, int upper, int width, char pad)
{
    char digits[24];
    const char *set = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int n = 0;
    do {
        digits[n++] = set[value % base];
        value /= base;
    } while (value != 0);
    for (int i = n; i < width; i++)
        put(pad);
    while (n > 0)
        put(digits[--n]);
}

/* A printf subset, checked by the compiler through the format attribute: flags # and 0, a decimal
 * width, the length modifiers l, ll and z, and the conversions d i u x X c s p %. */
void klog(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int enabled = cpu_irq_save();
    for (const char *p = format; *p != '\0'; p++) {
        if (*p != '%') {
            put(*p);
            continue;
        }
        p++;
        int alternate = 0;
        char pad = ' ';
        for (;; p++) {
            if (*p == '#')
                alternate = 1;
            else if (*p == '0')
                pad = '0';
            else
                break;
        }
        int width = 0;
        while (*p >= '0' && *p <= '9')
            width = width * 10 + (*p++ - '0');
        int length = 0;
        if (*p == 'z') {
            length = 2;
            p++;
        } else {
            while (*p == 'l') {
                length++;
                p++;
            }
        }
        switch (*p) {
        case 'd':
        case 'i': {
            int64_t v = length >= 1 ? (int64_t)va_arg(args, long long) : (int64_t)va_arg(args, int);
            if (v < 0) {
                put('-');
                put_unsigned((uint64_t)0 - (uint64_t)v, 10, 0, width, pad);
            } else {
                put_unsigned((uint64_t)v, 10, 0, width, pad);
            }
            break;
        }
        case 'u':
        case 'x':
        case 'X': {
            uint64_t v = length >= 1 ? (uint64_t)va_arg(args, unsigned long long) : (uint64_t)va_arg(args, unsigned);
            if (*p == 'u') {
                put_unsigned(v, 10, 0, width, pad);
            } else {
                if (alternate) {
                    put('0');
                    put('x');
                    width = width > 2 ? width - 2 : 0;
                }
                put_unsigned(v, 16, *p == 'X', width, pad);
            }
            break;
        }
        case 'p':
            put('0');
            put('x');
            put_unsigned((uint64_t)(uintptr_t)va_arg(args, void *), 16, 0, 0, ' ');
            break;
        case 's': {
            const char *s = va_arg(args, const char *);
            while (*s != '\0')
                put(*s++);
            break;
        }
        case 'c':
            put((char)va_arg(args, int));
            break;
        case '%':
            put('%');
            break;
        default:
            put('%');
            if (*p == '\0')
                p--;
            else
                put(*p);
            break;
        }
    }
    cpu_irq_restore(enabled);
    va_end(args);
}
