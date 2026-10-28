#include "network_service.h"
static NetworkDevice device;
static HttpResponse response;
static uint32_t published;
static uint64_t body_word(size_t index) {
  if (NetworkResult_kind(published) != 1 || index > SIZE_MAX / 8) return UINT64_MAX;
  size_t start = index * 8;
  if (start >= response.length) return UINT64_MAX;
  size_t count = response.length - start;
  if (count > 8) count = 8;
  return packed_words_load_le64((cesta_read_bytes) {
    response.body + start, count
  });
}
static uint64_t service_answer(const SexosMessage *message) {
  if (message->type_id == SEXOS_NET_dns_resolve) return 0;
  if (message->type_id != SEXOS_NET_request) return UINT64_MAX;
  switch (message->arg0) {
    case SEXOS_NET_http_result: return published;
    case SEXOS_NET_body_length: return NetworkResult_kind(published) == 1 ? response.length : 0;
    case SEXOS_NET_body_chunk: return body_word((size_t) message->arg1);
    case SEXOS_NET_text_length: return 0;
    case SEXOS_NET_text_chunk: return UINT64_MAX;
    default: return UINT64_MAX;
  }
}
void network_service_main(void) {
  bool correlated = sexos_rpc_register() == 1;
  NetworkFailure failure = NET_OK;
  http_response_init(&response);
  if (!network_device_open(&device)) failure = device.failure;
  else failure = network_fetch(&device, &response);
  bool complete = failure == NET_OK && response.state == HTTP_COMPLETE;
  if (!complete && failure == NET_OK) failure = NET_HTTP_ERROR;
  published = NetworkResult_encode(complete ? 1 : 2, failure, response.error, response.status, complete ? response.length : 0, 0);
  if (!NetworkResult_valid(published)) sexos_fail();
  sexos_line("[network.fetch.result] kind=%u transport=%u framing=%u status=%u bytes=%zu", complete ? 1u : 2u, (unsigned) failure, (unsigned) response.error, (unsigned) response.status, complete ? response.length : 0);
  if (complete) network_trace_body("network.fetch.body", (cesta_read_bytes) {
    response.body, response.length
  });
  for (;;) {
    SexosRequest request = sexos_receive_request(correlated);
    uint64_t answer = service_answer(&request.message);
    uint64_t delivered = sexos_answer_request(&request, answer);
    if (delivered) sexos_line("[network.reply.refused] status=%llu", (unsigned long long) delivered);
  }
}
