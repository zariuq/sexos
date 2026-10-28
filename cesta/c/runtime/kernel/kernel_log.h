#ifndef SEXOS_KERNEL_LOG_H
#define SEXOS_KERNEL_LOG_H

#include <stddef.h>
#include <stdint.h>

void kernel_serial_init(void);
void kernel_serial_write(const uint8_t *bytes, size_t count);
void klog(const char *format, ...) __attribute__((format(printf, 1, 2)));

#endif
