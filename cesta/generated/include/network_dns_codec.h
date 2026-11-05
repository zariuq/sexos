
#ifndef SEXOS_NETWORK_DNS_CODEC_H
#define SEXOS_NETWORK_DNS_CODEC_H
#include "sexos_network_config.h"

enum {
    NETWORK_DNS_MAX_BYTES = 512,
    NETWORK_DNS_QUERY_BYTES = 34
};

typedef enum NetworkDnsError {
    NETWORK_DNS_ERROR_NONE = 0,
    NETWORK_DNS_ERROR_INVALID_ID = 1,
    NETWORK_DNS_ERROR_OVERSIZED = 2,
    NETWORK_DNS_ERROR_TRUNCATED = 3,
    NETWORK_DNS_ERROR_MALFORMED = 4,
    NETWORK_DNS_ERROR_UNSUPPORTED = 5,
    NETWORK_DNS_ERROR_RCODE = 6,
    NETWORK_DNS_ERROR_NO_ANSWER = 7
} NetworkDnsError;

typedef enum NetworkDnsResultKind {
    NETWORK_DNS_RESULT_ERROR = 0,
    NETWORK_DNS_RESULT_UNRELATED = 1,
    NETWORK_DNS_RESULT_ANSWER = 2
} NetworkDnsResultKind;

typedef struct NetworkDnsFrame {
    uint8_t bytes[NETWORK_DNS_MAX_BYTES];
    size_t len;
} NetworkDnsFrame;

typedef struct NetworkDnsQueryResult {
    bool ok;
    size_t len;
    NetworkDnsError error;
} NetworkDnsQueryResult;

typedef struct NetworkDnsAnswer {
    uint8_t address[4];
    uint32_t ttl;
} NetworkDnsAnswer;

typedef struct NetworkDnsParseResult {
    NetworkDnsResultKind kind;
    NetworkDnsError error;
    uint8_t rcode;
    NetworkDnsAnswer answer;
} NetworkDnsParseResult;

NetworkDnsQueryResult network_dns_query(uint16_t id, NetworkDnsFrame *out);
NetworkDnsParseResult network_dns_parse(uint16_t expected_id, const NetworkDnsFrame *response);
#endif
