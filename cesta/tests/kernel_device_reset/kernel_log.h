#ifndef SEXOS_KERNEL_LOG_H
#define SEXOS_KERNEL_LOG_H
void klog(const char *format, ...) __attribute__((format(printf,1,2)));
#endif
