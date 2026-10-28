#include "network_service.h"
static void http_fail(HttpResponse *r, HttpError error) {
  r->state = HTTP_FAILED;
  r->error = error;
}
void http_response_init(HttpResponse *r) {
  memset(r, 0, sizeof *r);
  r->state = HTTP_STATUS;
}
static bool ascii_equal(const uint8_t *bytes, size_t length, const char *literal) {
  size_t i = 0;
  for (; literal[i]; ++i) {
    if (i >= length) return false;
    uint8_t byte = bytes[i];
    if (byte >= 'A' && byte <= 'Z') byte = (uint8_t)(byte + ('a' - 'A'));
    if (byte != (uint8_t) literal[i]) return false;
  }
  return i == length;
}
static bool decimal_length(const uint8_t *bytes, size_t length, size_t *value) {
  if (length == 0) return false;
  size_t result = 0;
  for (size_t i = 0; i < length; ++i) {
    if (bytes[i] < '0' || bytes[i] > '9') return false;
    unsigned digit = bytes[i] - '0';
    if (result > (SIZE_MAX - digit) / 10) return false;
    result = result * 10 + digit;
  }
  *value = result;
  return true;
}
static bool token_byte(uint8_t byte) {
  if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') || (byte >= '0' && byte <= '9')) return true;
  const char *extra = "!#$%&'*+-.^_`|~";
  for (size_t i = 0; extra[i]; ++i) if (byte == (uint8_t) extra[i]) return true;
  return false;
}
static void header_line(HttpResponse *r, size_t length) {
  size_t colon = 0;
  while (colon < length && r->line[colon] != ':') {
    if (!token_byte(r->line[colon])) {
      http_fail(r, HTTP_ERROR_SYNTAX);
      return;
    }
    ++colon;
  }
  if (colon == 0 || colon == length) {
    http_fail(r, HTTP_ERROR_SYNTAX);
    return;
  }
  size_t start = colon + 1;
  size_t end = length;
  while (start < end && (r->line[start] == ' ' || r->line[start] == '\t')) ++start;
  while (end > start && (r->line[end - 1] == ' ' || r->line[end - 1] == '\t')) --end;
  if (r->state == HTTP_TRAILERS) {
    if (ascii_equal(r->line, colon, "content-length") || ascii_equal(r->line, colon, "transfer-encoding")) http_fail(r, HTTP_ERROR_SYNTAX);
    return;
  }
  if (ascii_equal(r->line, colon, "content-length")) {
    size_t value;
    if (!decimal_length(r->line + start, end - start, &value) || (r->length_seen && r->declared_length != value)) {
      http_fail(r, HTTP_ERROR_SYNTAX);
      return;
    }
    if (value > NET_HTTP_BODY_CAPACITY) {
      http_fail(r, HTTP_ERROR_CAPACITY);
      return;
    }
    r->length_seen = true;
    r->declared_length = value;
  } else if (ascii_equal(r->line, colon, "transfer-encoding")) {
    if (r->transfer_seen || !ascii_equal(r->line + start, end - start, "chunked")) {
      http_fail(r, HTTP_ERROR_UNSUPPORTED);
      return;
    }
    r->transfer_seen = true;
    r->chunked = true;
  } else if (ascii_equal(r->line, colon, "content-encoding") && !ascii_equal(r->line + start, end - start, "identity")) {
    http_fail(r, HTTP_ERROR_UNSUPPORTED);
  }
}
static void finish_headers(HttpResponse *r) {
  if (r->status < 200) {
    if (r->status == 101 || ++r->interim > 8) {
      http_fail(r, HTTP_ERROR_UNSUPPORTED);
      return;
    }
    r->state = HTTP_STATUS;
    r->length_seen = r->transfer_seen = r->chunked = false;
    r->declared_length = 0;
    return;
  }
  if (r->status == 204 || r->status == 304) {
    r->state = HTTP_COMPLETE;
    return;
  }
  if (r->transfer_seen && r->length_seen) {
    http_fail(r, HTTP_ERROR_SYNTAX);
    return;
  }
  if (r->chunked) {
    r->state = HTTP_CHUNK_SIZE;
    return;
  }
  if (r->length_seen) {
    r->remaining = r->declared_length;
    r->state = r->remaining ? HTTP_FIXED : HTTP_COMPLETE;
  } else r->state = HTTP_UNTIL_CLOSE;
}
static void complete_line(HttpResponse *r, size_t length) {
  if (r->state == HTTP_STATUS) {
    if (length < 12 || memcmp(r->line, "HTTP/1.", 7) || (r->line[7] != '0' && r->line[7] != '1') || r->line[8] != ' ' || r->line[9] < '1' || r->line[9] > '5' || r->line[10] < '0' || r->line[10] > '9' || r->line[11] < '0' || r->line[11] > '9' || (length > 12 && r->line[12] != ' ')) {
      http_fail(r, HTTP_ERROR_SYNTAX);
      return;
    }
    r->status = (uint16_t)((r->line[9] - '0') * 100 + (r->line[10] - '0') * 10 + r->line[11] - '0');
    r->state = HTTP_HEADERS;
  } else if (r->state == HTTP_HEADERS) {
    if (length) header_line(r, length);
    else finish_headers(r);
  } else if (r->state == HTTP_TRAILERS) {
    if (length) header_line(r, length);
    else r->state = HTTP_COMPLETE;
  } else if (r->state == HTTP_CHUNK_SIZE) {
    size_t value = 0;
    size_t i = 0;
    for (; i < length && r->line[i] != ';'; ++i) {
      uint8_t byte = r->line[i];
      unsigned digit;
      if (byte >= '0' && byte <= '9') digit = byte - '0';
      else if (byte >= 'a' && byte <= 'f') digit = byte - 'a' + 10;
      else if (byte >= 'A' && byte <= 'F') digit = byte - 'A' + 10;
      else {
        http_fail(r, HTTP_ERROR_SYNTAX);
        return;
      }
      if (value > (SIZE_MAX - digit) / 16) {
        http_fail(r, HTTP_ERROR_CAPACITY);
        return;
      }
      value = value * 16 + digit;
    }
    if (i == 0) {
      http_fail(r, HTTP_ERROR_SYNTAX);
      return;
    }
    if (value > NET_HTTP_BODY_CAPACITY - r->length) {
      http_fail(r, HTTP_ERROR_CAPACITY);
      return;
    }
    r->remaining = value;
    r->state = value ? HTTP_CHUNK_DATA : HTTP_TRAILERS;
  }
}
size_t http_response_feed(HttpResponse *r, cesta_read_bytes input) {
  size_t consumed = 0;
  while (consumed < input.len && r->state != HTTP_COMPLETE && r->state != HTTP_FAILED) {
    uint8_t byte = input.data[consumed ++];
    if (r->state == HTTP_FIXED || r->state == HTTP_UNTIL_CLOSE || r->state == HTTP_CHUNK_DATA) {
      if (r->length == NET_HTTP_BODY_CAPACITY) {
        http_fail(r, HTTP_ERROR_CAPACITY);
        break;
      }
      r->body[r->length ++] = byte;
      if (r->state == HTTP_FIXED && --r->remaining == 0) r->state = HTTP_COMPLETE;
      else if (r->state == HTTP_CHUNK_DATA && --r->remaining == 0) r->state = HTTP_CHUNK_CR;
    } else if (r->state == HTTP_CHUNK_CR) {
      if (byte == '\r') r->state = HTTP_CHUNK_LF;
      else http_fail(r, HTTP_ERROR_SYNTAX);
    } else if (r->state == HTTP_CHUNK_LF) {
      if (byte == '\n') r->state = HTTP_CHUNK_SIZE;
      else http_fail(r, HTTP_ERROR_SYNTAX);
    } else {
      if (++r->header_bytes > 8192 || r->line_length == NET_HTTP_LINE_CAPACITY) {
        http_fail(r, HTTP_ERROR_CAPACITY);
        break;
      }
      if (byte == '\n') {
        if (r->line_length == 0 || r->line[r->line_length - 1] != '\r') {
          http_fail(r, HTTP_ERROR_SYNTAX);
          break;
        }
        complete_line(r, r->line_length - 1);
        r->line_length = 0;
      } else {
        if ((r->line_length && r->line[r->line_length - 1] == '\r') || (byte < 32 && byte != '\r' && byte != '\t') || byte == 127) {
          http_fail(r, HTTP_ERROR_SYNTAX);
          break;
        }
        r->line[r->line_length ++] = byte;
      }
    }
  }
  return consumed;
}
void http_response_end(HttpResponse *r) {
  if (r->state == HTTP_UNTIL_CLOSE) r->state = HTTP_COMPLETE;
  else if (r->state != HTTP_COMPLETE && r->state != HTTP_FAILED) http_fail(r, HTTP_ERROR_INCOMPLETE);
}
