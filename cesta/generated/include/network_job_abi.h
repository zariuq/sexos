
#ifndef SEXOS_NETWORK_JOB_ABI_H
#define SEXOS_NETWORK_JOB_ABI_H
#include "sexos_client.h"
#define NETWORK_JOB_OP UINT64_C(0x20C)
#define NETWORK_JOB_MAX_BYTES ((size_t)4096)
#define NETWORK_JOB_STREAM_RING_BYTES ((size_t)1024)
#define NETWORK_JOB_MAX_STREAM_BYTES ((size_t)13312)
#define NETWORK_JOB_STREAM_TOTAL_TICKS UINT64_C(30000)
#define NETWORK_JOB_STREAM_IDLE_TICKS UINT64_C(5000)
#define NETWORK_JOB_TURN ((size_t)256)
#define NETWORK_JOB_MAX_SEQ UINT32_C(0xFFFFFF)
#define NETWORK_JOB_MAX_TICKET UINT32_C(17535)
#define NETWORK_JOB_MAX_DUPLEX_BYTES ((size_t)32768)
#define NETWORK_JOB_BEGIN ((uint8_t)1)
#define NETWORK_JOB_WRITE ((uint8_t)2)
#define NETWORK_JOB_START ((uint8_t)3)
#define NETWORK_JOB_STATUS ((uint8_t)4)
#define NETWORK_JOB_READ ((uint8_t)5)
#define NETWORK_JOB_CANCEL ((uint8_t)6)
#define NETWORK_JOB_RELEASE ((uint8_t)7)
#define NETWORK_JOB_BEGIN_RESOLVED ((uint8_t)8)
#define NETWORK_JOB_BEGIN_STREAM_RESOLVED ((uint8_t)9)
#define NETWORK_JOB_BEGIN_DUPLEX_RESOLVED ((uint8_t)10)
#define NETWORK_JOB_FINISH ((uint8_t)11)
#define NETWORK_JOB_INVALID UINT32_C(1)
#define NETWORK_JOB_DENIED UINT32_C(2)
#define NETWORK_JOB_STALE UINT32_C(3)
#define NETWORK_JOB_BUSY UINT32_C(4)
#define NETWORK_JOB_OFFLINE UINT32_C(5)
#define NETWORK_JOB_TIMEOUT UINT32_C(6)
#define NETWORK_JOB_IO UINT32_C(7)
#define NETWORK_JOB_EXHAUSTED UINT32_C(8)

typedef enum NetworkJobState {
    NETWORK_JOB_IDLE = 0,
    NETWORK_JOB_STAGING = 1,
    NETWORK_JOB_CONNECTING = 2,
    NETWORK_JOB_SENDING = 3,
    NETWORK_JOB_RECEIVING = 4,
    NETWORK_JOB_COMPLETE = 5,
    NETWORK_JOB_FAILED = 6,
    NETWORK_JOB_CANCELLED = 7
} NetworkJobState;

typedef struct NetworkJobRequest {
    uint32_t seq;
    uint8_t kind;
    uint16_t offset, length;
    uint32_t ticket;
    uint64_t a;
} NetworkJobRequest;

typedef struct NetworkJobWords {
    uint64_t words[3];
} NetworkJobWords;

typedef struct NetworkJobRequestDecode {
    bool ok;
    NetworkJobRequest request;
    uint32_t error;
} NetworkJobRequestDecode;

typedef struct NetworkJobStateDecode {
    bool ok;
    NetworkJobState state;
} NetworkJobStateDecode;

typedef struct NetworkJobStatusDecode {
    bool ok;
    NetworkJobState state;
    size_t response_length;
} NetworkJobStatusDecode;

typedef struct NetworkJobReplyDecode {
    bool valid, success;
    uint32_t value;
} NetworkJobReplyDecode;

NetworkJobWords network_job_encode(NetworkJobRequest request);
NetworkJobRequestDecode network_job_decode(NetworkJobWords words);
bool network_job_request_valid(NetworkJobRequest request);
NetworkJobStateDecode network_job_state_decode(uint8_t value);
uint32_t network_job_status(NetworkJobState state, size_t response_length);
NetworkJobStatusDecode network_job_decode_status(uint32_t value);
uint64_t network_job_reply(uint32_t seq, uint32_t value);
uint64_t network_job_error(uint32_t seq, uint32_t code);
NetworkJobReplyDecode network_job_decode_reply(uint64_t word, uint32_t seq);
#endif
