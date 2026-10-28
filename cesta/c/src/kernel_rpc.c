#include "sexos_kernel.h"
/*Correlated calls are separate from the legacy reply slot. Each caller owns a bounded set of outstanding requests. Only the peer that received a request may complete it; polling or cancellation consumes its identity.*/
static RpcSlot *rpc_find(Domain *caller, uint64_t id) {
  if (id == 0) return NULL;
  for (uint32_t i = 0; i < KERNEL_RPC_CAPACITY; ++i) if (caller->rpc[i].state != RPC_FREE && caller->rpc[i].id == id) return &caller->rpc[i];
  return NULL;
}
static void rpc_call(Domain *caller, TrapFrame *f) {
  const Cap *cap = cap_get(caller, f->rdi);
  f->rax = ERR_CAP_INVALID;
  uint64_t opcode = f->rsi;
  f->rsi = 0;
  if (opcode == 0) return;
  if (cap == NULL || cap->kind != CAP_DOMAIN) return;
  Domain *peer = domain_get(cap->target);
  if (peer == NULL) return;
  if (!peer->rpc_enabled) {
    f->rax = NO_ADDRESS;
    return;
  }
  f->rax = ERR_SERVICE_NOT_READY;
  if (caller->rpc_next == UINT64_MAX) return;
  RpcSlot *vacant = NULL;
  for (uint32_t i = 0; i < KERNEL_RPC_CAPACITY; ++i) if (caller->rpc[i].state == RPC_FREE) {
    vacant = &caller->rpc[i];
    break;
  }
  if (vacant == NULL) return;
  uint64_t id = caller->rpc_next + 1;
  RingEntry message = (RingEntry) {
    .type_id = opcode,.caller = caller->id,.sender = caller->id,.request_id = id,.arg0 = f->rdx,.arg1 = f->r10,.arg2 = f->r8
  };
  if (!domain_enqueue(peer, message)) return;
  *vacant = (RpcSlot) {
    .id = id,.peer = peer->id,.state = RPC_QUEUED
  };
  caller->rpc_next = id;
  f->rsi = id;
  f->rax = 0;
}
bool kernel_rpc_claim(Domain *receiver, const RingEntry *message) {
  Domain *caller = domain_get(message->sender);
  if (caller == NULL) return false;
  RpcSlot *request = rpc_find(caller, message->request_id);
  if (request == NULL || (request->peer != receiver->id || request->state != RPC_QUEUED)) return false;
  request->state = RPC_HEARD;
  return true;
}
static void rpc_poll(Domain *caller, TrapFrame *f) {
  RpcSlot *request = rpc_find(caller, f->rdi);
  f->rsi = 0;
  f->rax = ERR_CAP_INVALID;
  if (request == NULL) return;
  if (request->state != RPC_READY) {
    f->rax = ERR_SERVICE_NOT_READY;
    return;
  }
  f->rsi = request->value;
  *request = (RpcSlot) {
    .state = RPC_FREE
  };
  f->rax = 0;
}
static void rpc_reply(Domain *peer, TrapFrame *f) {
  Domain *caller = domain_get((uint32_t) f->rdi);
  f->rax = ERR_CAP_INVALID;
  if (caller == NULL) return;
  RpcSlot *request = rpc_find(caller, f->rsi);
  if (request == NULL || (request->peer != peer->id || request->state != RPC_HEARD)) return;
  request->value = f->rdx;
  request->state = RPC_READY;
  f->rax = 0;
}
void kernel_rpc_syscall(Domain *d, TrapFrame *f, uint64_t number) {
  switch (number) {
    case SEXOS_RPC_register: {
      d->rpc_enabled = true;
      f->rax = 1;
    }
    break;
    case SEXOS_RPC_call: rpc_call(d, f);
    break;
    case SEXOS_RPC_poll: rpc_poll(d, f);
    break;
    case SEXOS_RPC_listen: {
      RingEntry message = kernel_listen_message(d, (uint32_t) f->rdi);
      f->rax = message.type_id;
      f->rsi = message.caller;
      f->rdx = message.arg0;
      f->r10 = message.arg1;
      f->r8 = message.arg2;
      f->r9 = message.request_id;
    }
    break;
    case SEXOS_RPC_reply: rpc_reply(d, f);
    break;
    case SEXOS_RPC_cancel: {
      RpcSlot *request = rpc_find(d, f->rdi);
      f->rax = ERR_CAP_INVALID;
      if (request != NULL) {
        *request = (RpcSlot) {
          .state = RPC_FREE
        };
        f->rax = 0;
      }
    }
    break;
    default: f->rax = NO_ADDRESS;
  }
}
