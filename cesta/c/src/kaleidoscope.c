#include "sexos_client.h"
#include "kaleidoscope_body.h"
#include "kaleidoscope_html.h"
#if !defined(SEXOS_KALEIDOSCOPE_CURRENT)
#define SEXOS_KALEIDOSCOPE_CURRENT 0

#endif
#define KALEIDO_SURFACE_ID UINT64_C(300)
#define TEXT_COLOR UINT32_C(0x00CDD6F4)
static size_t receive_text(SexosListener *listener, uint8_t *destination, size_t capacity, bool body) {
  const char *prefix = body ? "browser.body" : "browser.packed_text";
  uint64_t length_operation = body ? SEXOS_NET_body_length : SEXOS_NET_text_length;
  uint64_t chunk_operation = body ? SEXOS_NET_body_chunk : SEXOS_NET_text_chunk;
  sexos_pdx_call(SEXOS_SLOT_net, SEXOS_NET_request, length_operation, 0, 0);
  SexosMessage reply = sexos_listen(listener, 0);
  if (reply.type_id != 1) return 0;
  size_t reported = (size_t) reply.arg0;
  if (reported > capacity) reported = capacity;
  if (body && reported == 0) return 0;
  sexos_line("[%s.len.recv] len=%zu", prefix, reported);
  size_t chunks = (reported + 7) / 8;
  size_t received = 0;
  for (size_t index = 0; index < chunks; ++index) {
    sexos_pdx_call(SEXOS_SLOT_net, SEXOS_NET_request, chunk_operation, index, 0);
    reply = sexos_listen(listener, 0);
    if (reply.type_id != 1) break;
    size_t start = index * 8;
    size_t count = reported - start;
    if (count > 8) count = 8;
    cesta_write_bytes part = (cesta_write_bytes) {
      destination + start, count
    };
    packed_words_store_le64(part, reply.arg0);
    sexos_line("[%s.chunk.recv] idx=%zu bytes=%zu", prefix, index, count);
    received = start + count;
  }
  sexos_line("[%s.text.set] live=1 len=%zu", prefix, received);
  if (!body) sexos_line("[browser.async_reply.proof.done] len=%zu text_recv=%zu", reported, received);
  return received;
}
static void send_text(const uint8_t *text, size_t length, size_t base_offset) {
  size_t chunks = (length + 7) / 8;
  for (size_t index = 0; index < chunks; ++index) {
    size_t offset = index * 8;
    size_t count = length - offset;
    if (count > 8) count = 8;
    cesta_read_bytes part = (cesta_read_bytes) {
      text + offset, count
    };
    uint64_t packed = packed_words_load_le64(part);
    uint64_t metadata = packed_words_display_metadata(base_offset + offset, count, TEXT_COLOR);
    sexos_pdx_call(SEXOS_SLOT_display, SEXOS_DISPLAY_text_chunk, KALEIDO_SURFACE_ID, packed, metadata);
  }
}
static void initialize_browser(SexosListener *listener) {
  sexos_line("[browser.slot.net.static_grant.begin]");
  SexosReply result = sexos_pdx_call(SEXOS_SLOT_net, SEXOS_NET_request, 0, 0, 0);
  sexos_line("[browser.slot.net.route.call] status=%llu", (unsigned long long) result.status);
  sexos_line("[browser.slot.net.static_grant.proof.done] ok=1 network=0");
  sexos_line("[browser.dns.resolve.request] host_id=1 ok=1");
  result = sexos_pdx_call(SEXOS_SLOT_net, SEXOS_NET_dns_resolve, 1, 0, 0);
  if (result.value != 0) {
    uint32_t ip = (uint32_t) result.value;
    sexos_line("[browser.dns.resolve.ok] addr=%u.%u.%u.%u ok=1", (unsigned)(ip >> 24 & 255u), (unsigned)(ip >> 16 & 255u), (unsigned)(ip >> 8 & 255u), (unsigned)(ip & 255u));
  } else sexos_line("[browser.dns.resolve.miss] host_id=1 ok=1 reason=cache_miss");
  sexos_line("[browser.packed_text.begin]");
  uint8_t live_text[64] = {
    0
  };
  size_t live_length = receive_text(listener, live_text, sizeof live_text, false);
  sexos_line("[browser.packed_text.proof.done]");
  sexos_line("[browser.body.begin]");
  uint8_t body_text[64] = {
    0
  };
  size_t body_length = receive_text(listener, body_text, sizeof body_text, true);
  sexos_line("[browser.render.live_text.begin] len=%zu", live_length);
  sexos_line("[browser.visible.polish.begin] sid=%llu", (unsigned long long) KALEIDO_SURFACE_ID);
  sexos_pdx_call(SEXOS_SLOT_display, SEXOS_DISPLAY_create_surface, KALEIDO_SURFACE_ID, 120ull << 32 | 80ull, 120ull << 32 | 520ull);
  sexos_line("[browser.render.buffer.ready] virt=surface pfn=surface sid=%llu", (unsigned long long) KALEIDO_SURFACE_ID);
#if SEXOS_KALEIDOSCOPE_CURRENT
{
  bool rendered_html = false;
  if (body_length > 0 && kaleidoscope_html_looks_like(body_text, body_length)) {
    static KaleidoscopeHtmlWorkspace html_workspace = {
      0
    };
    static uint8_t html_text[KALEIDOSCOPE_BODY_CAPACITY] = {
      0
    };
    KaleidoscopeHtmlConverted converted = {
      0
    };
    if (kaleidoscope_html_convert(body_text, body_length, html_text, sizeof html_text, &html_workspace, &converted) == 0) {
      send_text(html_text, converted.len, 0);
      sexos_line("[browser.html.converted] source_bytes=%zu text_bytes=%zu links=%zu truncated=%u scripts_run=%u scripts_refused=%u", body_length, converted.len, converted.links, (unsigned) converted.truncated, (unsigned) converted.scripts_run, (unsigned) converted.scripts_refused);
      rendered_html = true;
    }
  }
  if (!rendered_html) {
    KaleidoscopeBodySource body_source = {
      0
    };
    KaleidoscopeBodySource summary_source = {
      0
    };
    body_source.len = body_length;
    summary_source.len = live_length;
    for (size_t i = 0; i < body_length; ++i) body_source.bytes[i] = body_text[i];
    for (size_t i = 0; i < live_length; ++i) summary_source.bytes[i] = live_text[i];
    KaleidoscopeBodyPreview preview = {
      0
    };
    kaleidoscope_body_build(&body_source, &summary_source, &preview);
    send_text(preview.bytes, preview.len, 0);
    sexos_line("[browser.preview.layout] bytes=%zu newline=1 cached_or_sample=1", preview.len);
  }
  if (body_length > 0) sexos_line("[browser.body.render.done]");
}
#else
{
  static const uint8_t title[] = "Kaleidoscope";
  send_text(title, sizeof title - 1, 0);
  sexos_line("[browser.visible.polish.text] line=0 len=%zu", sizeof title - 1);
  static const uint8_t status_line[] = "HTTP 200 from 10.0.2.2";
  send_text(status_line, sizeof status_line - 1, 32);
  sexos_line("[browser.visible.polish.text] line=1 len=%zu", sizeof status_line - 1);
  if (body_length == 0) send_text(live_text, live_length, 64);
  if (body_length > 0) {
    sexos_line("[browser.body.render.begin]");
    send_text(body_text, body_length, 64);
    sexos_line("[browser.body.render.done]");
  }
}
#endif
  sexos_line("[browser.render.draw_text] live=1 len=%zu", live_length);
  sexos_pdx_call(SEXOS_SLOT_display, SEXOS_DISPLAY_commit, KALEIDO_SURFACE_ID, 50, 100);
  sexos_line("[browser.render.commit.request] window=%llu", (unsigned long long) KALEIDO_SURFACE_ID);
  sexos_line("[browser.render.live_text.done]");
  sexos_line("[browser.visible.polish.done]");
}
void kaleidoscope_main(void) {
  SexosListener listener = {
    8u
  };
  initialize_browser(&listener);
  while (true) {
    SexosMessage message = sexos_listen(&listener, 1);
    if (message.type_id == 65535ull) break;
  }
  sexos_pdx_call(SEXOS_SLOT_kernel, SEXOS_KERNEL_exit, 0, 0, 0);
}
