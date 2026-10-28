#include "network_service.h"
static bool query(uint64_t operation, uint64_t argument, uint64_t *value) {
  SexosRequestResult result = sexos_request(SEXOS_SLOT_net, SEXOS_NET_request, operation, argument, 0, 1048576);
  if (result.status != SEXOS_REQUEST_COMPLETE) {
    sexos_line("[browser.network.request.failed] kind=%u detail=%llu", (unsigned) result.status, (unsigned long long) result.detail);
    return false;
  }
  *value = result.value;
  return true;
}
static bool draw_text(const uint8_t *text, size_t length, size_t offset) {
  for (size_t start = 0; start < length; start += 8) {
    size_t count = length - start;
    if (count > 8) count = 8;
    uint64_t word = packed_words_load_le64((cesta_read_bytes) {
      text + start, count
    });
    uint64_t metadata = packed_words_display_metadata(offset + start, count, 0x00CDD6F4);
    SexosRequestResult delivered = sexos_send(SEXOS_SLOT_display, SEXOS_DISPLAY_text_chunk, 300, word, metadata, 1048576);
    sexos_line("[browser.network.draw] offset=%zu bytes=%zu word=0x%016llX status=%llu", offset + start, count, (unsigned long long) word, (unsigned long long) delivered.detail);
    if (delivered.status != SEXOS_REQUEST_COMPLETE) return false;
  }
  return true;
}
void network_browser_main(void) {
  uint64_t raw;
  bool received = query(SEXOS_NET_http_result, 0, &raw);
  uint32_t result = received && raw <= UINT32_MAX ? (uint32_t) raw : UINT32_MAX;
  bool complete = NetworkResult_valid(result) && NetworkResult_kind(result) == 1;
  size_t length = complete ? (size_t) NetworkResult_length(result) : 0;
  uint8_t body[NET_HTTP_BODY_CAPACITY] = {
    0
  };
  if (complete) {
    for (size_t offset = 0; offset < length; offset += 8) {
      uint64_t word;
      if (!query(SEXOS_NET_body_chunk, offset / 8, &word)) {
        complete = false;
        break;
      }
      size_t count = length - offset;
      if (count > 8) count = 8;
      packed_words_store_le64((cesta_write_bytes) {
        body + offset, count
      }, word);
    }
  }
  if (complete) network_trace_body("browser.network.body", (cesta_read_bytes) {
    body, length
  });
  SexosRequestResult created = sexos_send(SEXOS_SLOT_display, SEXOS_DISPLAY_create_surface, 300, (uint64_t) 120 << 32 | 80, (uint64_t) 120 << 32 | 520, 1048576);
  sexos_line("[browser.network.surface] kind=%u detail=%llu", (unsigned) created.status, (unsigned long long) created.detail);
  bool presented = created.status == SEXOS_REQUEST_COMPLETE;
  static const uint8_t title[] = "Kaleidoscope";
  if (presented) presented = draw_text(title, sizeof title - 1, 0);
  uint8_t status_line[32] = {
    0
  };
  size_t status_length;
  if (complete) {
    static const uint8_t prefix[] = "HTTP ";
    memcpy(status_line, prefix, sizeof prefix - 1);
    status_length = sizeof prefix - 1;
    status_length += format_bytes_unsigned_decimal((cesta_write_bytes) {
      status_line + status_length, sizeof status_line - status_length
    }, NetworkResult_status(result));
    static const uint8_t suffix[] = " from 10.0.2.2";
    memcpy(status_line + status_length, suffix, sizeof suffix - 1);
    status_length += sizeof suffix - 1;
  } else {
    static const uint8_t failure[] = "Network request failed";
    memcpy(status_line, failure, sizeof failure - 1);
    status_length = sizeof failure - 1;
  }
  if (presented) presented = draw_text(status_line, status_length, 32);
  if (complete) {
    uint8_t display[64];
    size_t display_length = length < sizeof display ? length : sizeof display;
    for (size_t i = 0; i < display_length; ++i) display[i] = body[i] >= 32 && body[i] < 127 ? body[i] : ' ';
    if (presented) presented = draw_text(display, display_length, 64);
  }
  if (presented) {
    SexosRequestResult committed = sexos_send(SEXOS_SLOT_display, SEXOS_DISPLAY_commit, 300, 50, 100, 1048576);
    sexos_line("[browser.network.commit] kind=%u detail=%llu", (unsigned) committed.status, (unsigned long long) committed.detail);
    presented = committed.status == SEXOS_REQUEST_COMPLETE;
  }
  sexos_line("[browser.network.result] complete=%u status=%u bytes=%zu", complete ? 1u : 0u, complete ? (unsigned) NetworkResult_status(result) : 0u, complete ? length : 0);
  sexos_line(presented ? "[browser.visible.polish.done]" : "[browser.visible.failed]");
  for (;;) sexos_yield();
}
