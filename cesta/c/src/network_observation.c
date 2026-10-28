#include "network_service.h"
void network_trace_body(const char *label, cesta_read_bytes body) {
  static const char digits[] = "0123456789abcdef";
  size_t offset = 0;
  do {
    size_t count = body.len - offset;
    if (count > 64) count = 64;
    char hex[129];
    for (size_t i = 0; i < count; ++i) {
      uint8_t byte = body.data[offset + i];
      hex[2 * i] = digits[byte >> 4];
      hex[2 * i + 1] = digits[byte & 15];
    }
    hex[2 * count] = 0;
    sexos_line("[%s] offset=%zu bytes=%zu hex=%s", label, offset, count, hex);
    offset += count;
  } while (offset < body.len);
}
