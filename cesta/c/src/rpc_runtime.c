#include "sexos_rpc.h"
#include "sexos_server.h"
#define RPC_DEFERRED_CAPACITY 8u
static SexosMessage deferred[RPC_DEFERRED_CAPACITY];
static size_t deferred_count;
static bool legacy_poisoned;
static bool kernel_probed;
static bool correlated_kernel;
void sexos_rpc_defer(SexosMessage message) {
  if (deferred_count >= RPC_DEFERRED_CAPACITY) sexos_fail();
  deferred[deferred_count] = message;
  ++deferred_count;
}
bool sexos_rpc_take_deferred(SexosMessage *message) {
  if (deferred_count == 0) return false;
  *message = deferred[0];
  --deferred_count;
  for (size_t i = 0; i < deferred_count; ++i) deferred[i] = deferred[i + 1];
  return true;
}
SexosRequestResult sexos_request(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t budget) {
  if (budget == 0) return (SexosRequestResult) {
    .status = SEXOS_REQUEST_TIMEOUT
  };
  if (!kernel_probed) {
    correlated_kernel = sexos_rpc_register() == 1;
    kernel_probed = true;
  }
  SexosReply admitted = (SexosReply) {
    .status = SEXOS_RPC_UNSUPPORTED
  };
  if (!correlated_kernel) {
    if (legacy_poisoned) return (SexosRequestResult) {
      .status = SEXOS_REQUEST_FAILED,.detail = SEXOS_RPC_INVALID
    };
    admitted = sexos_pdx_call(slot, opcode, arg0, arg1, arg2);
    if (admitted.status != 0) return (SexosRequestResult) {
      .status = SEXOS_REQUEST_FAILED,.detail = admitted.status
    };
    for (uint64_t remaining = budget; remaining > 0; --remaining) {
      if (deferred_count >= RPC_DEFERRED_CAPACITY) {
        legacy_poisoned = true;
        return (SexosRequestResult) {
          .status = SEXOS_REQUEST_FAILED,.detail = SEXOS_RPC_BUSY
        };
      }
      SexosMessage message = sexos_listen_once(0);
      if (message.type_id == 1) return (SexosRequestResult) {
        .status = SEXOS_REQUEST_COMPLETE,.value = message.arg0
      };
      if (message.type_id != 0) sexos_rpc_defer(message);
      sexos_yield();
    }
    legacy_poisoned = true;
    return (SexosRequestResult) {
      .status = SEXOS_REQUEST_TIMEOUT
    };
  }
  SexosRequestResult admission = sexos_rpc_admit_wait(slot, opcode, arg0, arg1, arg2, budget);
  if (admission.status != SEXOS_REQUEST_COMPLETE) return admission;
  return sexos_rpc_await(admission.value, budget);
}
SexosRequest sexos_receive_request(bool correlated) {
  while (true) {
    SexosRequest request = (SexosRequest) {
      .id = 0
    };
    if (correlated) request = sexos_rpc_listen_once(0);
    else {
      if (!sexos_rpc_take_deferred(&request.message)) request.message = sexos_listen_once(0);
    }
    if (request.message.type_id != 0) return request;
    sexos_yield();
  }
}
uint64_t sexos_answer_request(const SexosRequest *request, uint64_t value) {
  if (request->id != 0) return sexos_rpc_reply(request->message.caller_pd, request->id, value);
  return sexos_reply(request->message.caller_pd, value);
}
