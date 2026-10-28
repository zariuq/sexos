#if !defined(SEXOS_CESTA_RPC_H)
#define SEXOS_CESTA_RPC_H 1

#include "sexos_client.h"

typedef struct SexosRequest {
  SexosMessage message;
  uint64_t id;
}
SexosRequest;

typedef enum SexosRequestStatus {
  SEXOS_REQUEST_COMPLETE, SEXOS_REQUEST_PENDING, SEXOS_REQUEST_FAILED, SEXOS_REQUEST_TIMEOUT
}
SexosRequestStatus;

typedef struct SexosRequestResult {
  SexosRequestStatus status;
  uint64_t value;
  uint64_t detail;
}
SexosRequestResult;

#define SEXOS_RPC_UNSUPPORTED UINT64_MAX

#define SEXOS_RPC_INVALID (UINT64_MAX - 3)

#define SEXOS_RPC_BUSY (UINT64_MAX - 1)

uint64_t sexos_rpc_register(void);

SexosReply sexos_rpc_begin(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2);

SexosReply sexos_rpc_poll(uint64_t id);

SexosRequestResult sexos_rpc_admit_wait(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t budget);

/*Send a by-value notification once; retry only a queue-full refusal that enqueued nothing.*/

SexosRequestResult sexos_send(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t budget);

SexosRequestResult sexos_rpc_await(uint64_t id, uint64_t budget);

uint64_t sexos_rpc_cancel(uint64_t id);

SexosRequest sexos_rpc_listen_once(uint64_t slot);

uint64_t sexos_rpc_reply(uint32_t caller, uint64_t id, uint64_t value);

/*Compatibility requests require the caller to have no other legacy query outstanding. Native correlated polling leaves all unrelated inbox entries untouched.*/

SexosRequestResult sexos_request(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t budget);

SexosRequest sexos_receive_request(bool correlated);

uint64_t sexos_answer_request(const SexosRequest *request, uint64_t value);

void sexos_rpc_defer(SexosMessage message);

bool sexos_rpc_take_deferred(SexosMessage *message);

#endif
