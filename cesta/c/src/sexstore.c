#include "sexos_server.h"
#include "sexstore.h"
/*Key-value store server, translated from servers/sexstore/src/main.rs. The Rust statics (the slot table, the durable region and the log budgets) are fields of one owned Sexstore state. The Rust installs an allocator it never uses; this translation has none.*/
/*Build-time proof profile (option_env! in the Rust source).*/
#if defined(SEXOS_SEXSTORE_KV_PROOF)
#define SEXSTORE_KV_PROOF_ENABLED 1

#else
#define SEXSTORE_KV_PROOF_ENABLED 0

#endif
/*Opcodes, local to sexstore until silk-shell integration promotes them to sex-pdx.*/
#define OP_KV_GET UINT64_C(0xB0)
#define OP_KV_PUT UINT64_C(0xB1)
#define OP_KV_DEL UINT64_C(0xB2)
/*Status codes (E6, aligned with E2). A GET hit replies with the stored value, bit 63 clear; every other reply is REPLY_STATUS_BIT over a code.*/
#define KV_OK UINT64_C(0x00)
#define KV_NOT_FOUND UINT64_C(0x01)
#define KV_FULL UINT64_C(0x02)
#define KV_INVALID_KEY UINT64_C(0x03)
#define KV_INVALID_VALUE UINT64_C(0x04)
#define KV_DENIED UINT64_C(0x05)
#define REPLY_STATUS_BIT UINT64_C(0x8000000000000000)
/*silk-shell's domain in the fixed spawn order of kernel/src/init.rs (module_paths[2]); the only caller E4 authorizes. It must follow any change to that order.*/
#define KV_SHELL_CALLER UINT64_C(3)
typedef struct Sexstore {
  KvTable kv;
  Durable durable;
  uint8_t proof_stage;
  uint32_t log_put;
  uint32_t log_get;
/*E4: policy and validation markers.*/
  uint32_t log_policy_allow;
  uint32_t log_policy_deny;
  uint32_t log_key_invalid;
  uint32_t log_value_invalid;
  uint32_t log_reply_error;
/*E6: generation and tombstone markers.*/
  uint32_t log_generation_bump;
  uint32_t log_tombstone_record;
  uint32_t log_tombstone_get;
  uint32_t log_tombstone_revive;
/*E7: structured allow and reject markers.*/
  uint32_t log_put_allow;
  uint32_t log_put_reject;
  uint32_t log_get_allow;
  uint32_t log_get_reject;
  uint32_t log_delete_allow;
  uint32_t log_delete_reject;
}
Sexstore;
/*A budget allows its line while it lasts.*/
static bool take(uint32_t *budget) {
  if (*budget == 0) return false;
  *budget -= 1;
  return true;
}
/*The owner class of a key: 0 invalid (key 0), 1 the shell range 0x01..0x0F, 2 reserved (0x10 and up).*/
static uint8_t store_key_owner_class(uint32_t key) {
  if (key == 0) return 0;
  if (key <= 0x0F) return 1;
  return 2;
}
/*E4: only silk-shell, and only on the shell range.*/
static bool store_cap_allowed(uint64_t caller_pd, uint32_t key) {
  return store_key_owner_class(key) == 1 && caller_pd == KV_SHELL_CALLER;
}
/*Bit 63 would collide with REPLY_STATUS_BIT in a GET reply. Key 1 holds the scene settings envelope: magic 0xAC, version 0x01, and in byte 7 the XOR of bytes 0..6 masked to 7 bits, which keeps bit 63 clear.*/
static bool store_validate_value(uint32_t key, uint64_t value) {
  if ((value & REPLY_STATUS_BIT) != 0) return false;
  if (key == 0x01) {
    uint8_t b[8] = {
      0
    };
    for (size_t i = 0; i < 8; ++i) b[i] = (uint8_t)(value >> 8 * i);
    if (b[0] != 0xAC || b[1] != 0x01) return false;
    uint8_t chk = 0;
    for (size_t i = 0; i < 7; ++i) chk ^= b[i];
    if (b[7] != (chk & 0x7F)) return false;
  }
  return true;
}
/*Generations count writes 1..255 and wrap to 1, never 0.*/
static void bump_generation(KvSlot *slot) {
  slot->generation = (uint8_t)(slot->generation >= 255 ? 1 : slot->generation + 1);
}
/*The slot holding KEY, active or tombstoned, or KV_SLOT_COUNT.*/
static size_t find_slot(const KvTable *table, uint32_t key) {
  for (size_t i = 0; i < KV_SLOT_COUNT; ++i) if (table->slots[i].state != SLOT_EMPTY && table->slots[i].key == key) return i;
  return KV_SLOT_COUNT;
}
/*The KV contract proof (SEXOS_SEXSTORE_KV_PROOF) drives the table directly through these, at boot.*/
static void kv_reset_for_proof(KvTable *table) {
  for (size_t i = 0; i < KV_SLOT_COUNT; ++i) table->slots[i] = (KvSlot) {
    .state = SLOT_EMPTY
  };
}
static uint64_t kv_put_for_proof(KvTable *table, uint64_t caller, uint32_t key, uint64_t val) {
  if (!store_cap_allowed(caller, key)) return REPLY_STATUS_BIT | (store_key_owner_class(key) == 0 ? KV_INVALID_KEY : KV_DENIED);
  if (!store_validate_value(key, val)) return REPLY_STATUS_BIT | KV_INVALID_VALUE;
  size_t found = find_slot(table, key);
  if (found < KV_SLOT_COUNT) {
    KvSlot *slot = &table->slots[found];
    slot->state = SLOT_ACTIVE;
    slot->val = val;
    bump_generation(slot);
    return REPLY_STATUS_BIT | KV_OK;
  }
/*The first empty slot, else the first tombstoned one.*/
  size_t empty = KV_SLOT_COUNT;
  size_t tomb = KV_SLOT_COUNT;
  for (size_t j = 0; j < KV_SLOT_COUNT; ++j) {
    if (table->slots[j].state == SLOT_EMPTY && empty == KV_SLOT_COUNT) empty = j;
    if (table->slots[j].state == SLOT_TOMBSTONED && tomb == KV_SLOT_COUNT) tomb = j;
  }
  size_t idx = empty < KV_SLOT_COUNT ? empty : tomb;
  if (idx == KV_SLOT_COUNT) return REPLY_STATUS_BIT | KV_FULL;
  table->slots[idx] = (KvSlot) {
    .state = SLOT_ACTIVE,.generation = 1,.key = key,.val = val
  };
  return REPLY_STATUS_BIT | KV_OK;
}
static uint64_t kv_get_for_proof(const KvTable *table, uint64_t caller, uint32_t key) {
  if (!store_cap_allowed(caller, key)) return REPLY_STATUS_BIT | (store_key_owner_class(key) == 0 ? KV_INVALID_KEY : KV_DENIED);
  size_t found = find_slot(table, key);
  if (found < KV_SLOT_COUNT && table->slots[found].state == SLOT_ACTIVE) return table->slots[found].val;
  return REPLY_STATUS_BIT | KV_NOT_FOUND;
}
/*A put without policy or envelope checks; it never reclaims tombstones.*/
static uint64_t kv_put_raw_for_proof(KvTable *table, uint32_t key, uint64_t val) {
  size_t found = find_slot(table, key);
  if (found < KV_SLOT_COUNT) {
    KvSlot *slot = &table->slots[found];
    slot->state = SLOT_ACTIVE;
    slot->val = val;
    bump_generation(slot);
    return REPLY_STATUS_BIT | KV_OK;
  }
  for (size_t j = 0; j < KV_SLOT_COUNT; ++j) if (table->slots[j].state == SLOT_EMPTY) {
    table->slots[j] = (KvSlot) {
      .state = SLOT_ACTIVE,.generation = 1,.key = key,.val = val
    };
    return REPLY_STATUS_BIT | KV_OK;
  }
  return REPLY_STATUS_BIT | KV_FULL;
}
static void run_kv_contract_proof_stage(Sexstore *s) {
  uint8_t stage = s->proof_stage;
  if (stage >= 6) return;
  s->proof_stage = (uint8_t)(stage + 1);
  sexos_line("[sexstore.kv.proof] stage=%u", (unsigned) stage);
  KvTable *kv = &s->kv;
  if (stage == 0) {
    kv_reset_for_proof(kv);
    uint64_t put = kv_put_for_proof(kv, KV_SHELL_CALLER, 0x02, 0x2A02);
    uint64_t get = kv_get_for_proof(kv, KV_SHELL_CALLER, 0x02);
    sexos_line("[sexstore.kv.proof.roundtrip] ok=%u put=%#llx get=%#llx", (unsigned)(put == (REPLY_STATUS_BIT | KV_OK) && get == 0x2A02), (unsigned long long) put, (unsigned long long) get);
    return;
  }
  if (stage == 1) {
    uint64_t get = kv_get_for_proof(kv, KV_SHELL_CALLER, 0x03);
    sexos_line("[sexstore.kv.proof.missing_key] ok=%u res=%#llx", (unsigned)(get == (REPLY_STATUS_BIT | KV_NOT_FOUND)), (unsigned long long) get);
    return;
  }
  if (stage == 2) {
    uint64_t put = kv_put_for_proof(kv, KV_SHELL_CALLER, 0x00, 1);
    sexos_line("[sexstore.kv.proof.oversized_key] ok=%u res=%#llx bound=4bytes", (unsigned)(put == (REPLY_STATUS_BIT | KV_INVALID_KEY)), (unsigned long long) put);
    return;
  }
  if (stage == 3) {
    uint64_t put = kv_put_for_proof(kv, KV_SHELL_CALLER, 0x01, REPLY_STATUS_BIT);
    sexos_line("[sexstore.kv.proof.oversized_value] ok=%u res=%#llx bound=8bytes", (unsigned)(put == (REPLY_STATUS_BIT | KV_INVALID_VALUE)), (unsigned long long) put);
    return;
  }
  if (stage == 4) {
    kv_reset_for_proof(kv);
    uint64_t last = REPLY_STATUS_BIT | KV_OK;
    for (size_t i = 0; i < KV_SLOT_COUNT; ++i) last = kv_put_raw_for_proof(kv, (uint32_t)(i + 1), (uint64_t) i + 10);
    uint64_t full = kv_put_raw_for_proof(kv, (uint32_t)(KV_SLOT_COUNT + 1), 0x55);
    sexos_line("[sexstore.kv.proof.table_full] ok=%u last=%#llx full=%#llx slots=%u", (unsigned)(last == (REPLY_STATUS_BIT | KV_OK) && full == (REPLY_STATUS_BIT | KV_FULL)), (unsigned long long) last, (unsigned long long) full, (unsigned) KV_SLOT_COUNT);
    return;
  }
  uint64_t put = kv_put_for_proof(kv, 99, 0x01, 0x1234);
  uint64_t get = kv_get_for_proof(kv, 99, 0x01);
  sexos_line("[sexstore.kv.proof.owner_deny] ok=%u put=%#llx get=%#llx", (unsigned)(put == (REPLY_STATUS_BIT | KV_DENIED) && get == (REPLY_STATUS_BIT | KV_DENIED)), (unsigned long long) put, (unsigned long long) get);
}
/*A caller without authority for KEY: log the refusal under REJECT_MARKER and reply invalid_key (key 0) or denied. The GET path logs the denied class as a number, PUT and DEL as a name.*/
static void refuse(Sexstore *s, uint64_t caller, uint32_t key, const char *reject_marker, uint32_t *reject_log, bool numeric_class) {
  uint8_t cls = store_key_owner_class(key);
  if (cls == 0) {
    if (take(&s->log_key_invalid)) sexos_line("[sexstore.key.invalid] caller=%llu key=0x00", (unsigned long long) caller);
  } else if (take(&s->log_policy_deny)) {
    if (numeric_class) sexos_line("[sexstore.policy.deny] caller=%llu key=%u class=%u", (unsigned long long) caller, (unsigned) key, (unsigned) cls);
    else if (cls == 2) sexos_line("[sexstore.policy.deny] caller=%llu key=%u class=reserved", (unsigned long long) caller, (unsigned) key);
    else sexos_line("[sexstore.policy.deny] caller=%llu key=%u class=shell reason=no_cap", (unsigned long long) caller, (unsigned) key);
  }
  if (take(reject_log)) {
    if (cls == 0) sexos_line("%s caller=%llu key=%u status=invalid_key reason=zero_key", reject_marker, (unsigned long long) caller, (unsigned) key);
    else sexos_line("%s caller=%llu key=%u status=denied reason=no_cap", reject_marker, (unsigned long long) caller, (unsigned) key);
  }(void) sexos_reply((uint32_t) caller, REPLY_STATUS_BIT | (cls == 0 ? KV_INVALID_KEY : KV_DENIED));
}
static void allow(Sexstore *s, uint64_t caller, uint32_t key, const char *op) {
  if (take(&s->log_policy_allow)) sexos_line("[sexstore.policy.allow] caller=%llu key=%u op=%s", (unsigned long long) caller, (unsigned) key, op);
}
static void handle_put(Sexstore *s, uint64_t caller, uint32_t key, uint64_t val) {
  if (!store_cap_allowed(caller, key)) {
    refuse(s, caller, key, "[sexstore.put.reject]", &s->log_put_reject, false);
    return;
  }
  allow(s, caller, key, "PUT");
/*E4: the value envelope of known keys.*/
  if (!store_validate_value(key, val)) {
    if (take(&s->log_value_invalid)) sexos_line("[sexstore.value.invalid] caller=%llu key=%u", (unsigned long long) caller, (unsigned) key);
    if (take(&s->log_put_reject)) sexos_line("[sexstore.put.reject] caller=%llu key=%u status=invalid_value reason=envelope_fail", (unsigned long long) caller, (unsigned) key);
    (void) sexos_reply((uint32_t) caller, REPLY_STATUS_BIT | KV_INVALID_VALUE);
    return;
  }
/*E6: update the key's slot, reviving a tombstone, else insert into an empty slot, else reclaim a tombstoned one. Each change is written durably after the RAM commit.*/
  size_t found = find_slot(&s->kv, key);
  bool was_update = found < KV_SLOT_COUNT;
  if (was_update) {
    KvSlot *slot = &s->kv.slots[found];
    bool was_tombstoned = slot->state == SLOT_TOMBSTONED;
    slot->val = val;
    if (was_tombstoned) {
      slot->state = SLOT_ACTIVE;
      if (take(&s->log_tombstone_revive)) sexos_line("[sexstore.tombstone.revive] key=%u old_gen=%u", (unsigned) key, (unsigned) slot->generation);
    }
    bump_generation(slot);
    if (take(&s->log_generation_bump)) sexos_line("[sexstore.generation.bump] key=%u slot=%u gen=%u op=%s", (unsigned) key, (unsigned) found, (unsigned) slot->generation, was_tombstoned ? "revive" : "put");
    if (take(&s->log_put_allow)) sexos_line("[sexstore.put.allow] caller=%llu key=%u status=ok state=%u gen=%u", (unsigned long long) caller, (unsigned) key, (unsigned) slot->state, (unsigned) slot->generation);
    (void) sexstore_durable_write_all(&s->durable, &s->kv);
    (void) sexos_reply((uint32_t) caller, REPLY_STATUS_BIT | KV_OK);
  } else {
    bool inserted = false;
    for (size_t pass = 0; pass < 2 && !inserted; ++pass) for (size_t i = 0; i < KV_SLOT_COUNT; ++i) {
      KvSlot *slot = &s->kv.slots[i];
      if (slot->state == (pass == 0 ? SLOT_EMPTY : SLOT_TOMBSTONED)) {
/*A reclaimed slot starts a new key's lifecycle at generation 1 (E10).*/
        *slot = (KvSlot) {
          .state = SLOT_ACTIVE,.generation = 1,.key = key,.val = val
        };
        inserted = true;
        if (take(&s->log_generation_bump)) sexos_line("[sexstore.generation.bump] key=%u slot=%u gen=1 op=%s", (unsigned) key, (unsigned) i, pass == 0 ? "insert" : "reclaim");
        (void) sexstore_durable_write_all(&s->durable, &s->kv);
        break;
      }
    }
/*A full table changes nothing, so it writes nothing.*/
    if (inserted) {
      if (take(&s->log_put_allow)) sexos_line("[sexstore.put.allow] caller=%llu key=%u status=ok state=1 gen=1", (unsigned long long) caller, (unsigned) key);
    } else if (take(&s->log_put_reject)) sexos_line("[sexstore.put.reject] caller=%llu key=%u status=full reason=table_full", (unsigned long long) caller, (unsigned) key);
    (void) sexos_reply((uint32_t) caller, REPLY_STATUS_BIT | (inserted ? KV_OK : KV_FULL));
  }
  if (take(&s->log_put)) sexos_line("[sexstore.kv.put] key=%u ok=%u", (unsigned) key, (unsigned) was_update);
}
static void handle_get(Sexstore *s, uint64_t caller, uint32_t key) {
  if (!store_cap_allowed(caller, key)) {
    refuse(s, caller, key, "[sexstore.get.reject]", &s->log_get_reject, true);
    return;
  }
  allow(s, caller, key, "GET");
  size_t found = find_slot(&s->kv, key);
  uint8_t found_state = found < KV_SLOT_COUNT ? s->kv.slots[found].state : SLOT_EMPTY;
  if (found_state == SLOT_ACTIVE) {
    const KvSlot *slot = &s->kv.slots[found];
    if (take(&s->log_get_allow)) sexos_line("[sexstore.get.allow] caller=%llu key=%u status=ok state=1 gen=%u", (unsigned long long) caller, (unsigned) key, (unsigned) slot->generation);
    (void) sexos_reply((uint32_t) caller, slot->val);
  } else {
    if (found_state == SLOT_TOMBSTONED) {
      if (take(&s->log_tombstone_get)) sexos_line("[sexstore.tombstone.get] key=%u slot=%u gen=%u", (unsigned) key, (unsigned) found, (unsigned) s->kv.slots[found].generation);
      if (take(&s->log_get_reject)) sexos_line("[sexstore.get.reject] caller=%llu key=%u status=not_found reason=tombstoned", (unsigned long long) caller, (unsigned) key);
    } else if (take(&s->log_get_reject)) sexos_line("[sexstore.get.reject] caller=%llu key=%u status=not_found reason=missing", (unsigned long long) caller, (unsigned) key);
    (void) sexos_reply((uint32_t) caller, REPLY_STATUS_BIT | KV_NOT_FOUND);
  }
  if (take(&s->log_get)) sexos_line("[sexstore.kv.get] key=%u hit=%u", (unsigned) key, (unsigned)(found_state == SLOT_ACTIVE));
}
static void handle_delete(Sexstore *s, uint64_t caller, uint32_t key) {
  if (!store_cap_allowed(caller, key)) {
    refuse(s, caller, key, "[sexstore.delete.reject]", &s->log_delete_reject, false);
    return;
  }
  allow(s, caller, key, "DEL");
  size_t found = find_slot(&s->kv, key);
  if (found == KV_SLOT_COUNT) {
    if (take(&s->log_delete_reject)) sexos_line("[sexstore.delete.reject] caller=%llu key=%u status=not_found reason=missing", (unsigned long long) caller, (unsigned) key);
    (void) sexos_reply((uint32_t) caller, REPLY_STATUS_BIT | KV_NOT_FOUND);
    return;
  }
  KvSlot *slot = &s->kv.slots[found];
  if (slot->state == SLOT_ACTIVE) {
/*Active becomes tombstoned, with a generation bump.*/
    slot->state = SLOT_TOMBSTONED;
    bump_generation(slot);
    if (take(&s->log_tombstone_record)) sexos_line("[sexstore.tombstone.record] key=%u slot=%u gen=%u reason=delete", (unsigned) key, (unsigned) found, (unsigned) slot->generation);
    if (take(&s->log_generation_bump)) sexos_line("[sexstore.generation.bump] key=%u slot=%u gen=%u op=tombstone", (unsigned) key, (unsigned) found, (unsigned) slot->generation);
    if (take(&s->log_delete_allow)) sexos_line("[sexstore.delete.allow] caller=%llu key=%u status=ok state=2 gen=%u reason=delete", (unsigned long long) caller, (unsigned) key, (unsigned) slot->generation);
  } else {
/*Already tombstoned: idempotent, and written again.*/
    if (take(&s->log_tombstone_record)) sexos_line("[sexstore.tombstone.record] key=%u slot=%u gen=%u reason=delete_idempotent", (unsigned) key, (unsigned) found, (unsigned) slot->generation);
    if (take(&s->log_delete_allow)) sexos_line("[sexstore.delete.allow] caller=%llu key=%u status=ok reason=idempotent", (unsigned long long) caller, (unsigned) key);
  }(void) sexstore_durable_write_all(&s->durable, &s->kv);
  (void) sexos_reply((uint32_t) caller, REPLY_STATUS_BIT | KV_OK);
}
void sexstore_main(void) {
  Sexstore s = (Sexstore) {
    .durable = sexstore_durable_new(),.log_put = 32,.log_get = 32,.log_policy_allow = 32,.log_policy_deny = 32,.log_key_invalid = 8,.log_value_invalid = 8,.log_reply_error = 8,.log_generation_bump = 64,.log_tombstone_record = 32,.log_tombstone_get = 32,.log_tombstone_revive = 16,.log_put_allow = 32,.log_put_reject = 16,.log_get_allow = 32,.log_get_reject = 16,.log_delete_allow = 16,.log_delete_reject = 8
  };
  sexos_line("[sexstore.init.start]");
/*E13: the empty table first, then whatever the durable pages hold. A first boot writes page A.*/
  (void) sexstore_durable_init(&s.durable, &s.kv);
  (void) sexstore_durable_load_into_ram(&s.durable, &s.kv);
  sexos_line("[sexstore.status.mapping] KV_OK=0x00 KV_NOT_FOUND=0x01 KV_FULL=0x02 KV_INVALID_KEY=0x03 KV_INVALID_VALUE=0x04 KV_DENIED=0x05 REPLY_BIT=0x8000");
  if (SEXSTORE_KV_PROOF_ENABLED) while (s.proof_stage < 6) run_kv_contract_proof_stage(&s);
  sexos_line("[sexstore.ready]");
  SexosListener listener = (SexosListener) {
    .wrapper_budget = 8
  };
  while (1) {
/*Slot 0 is the server's own message ring.*/
    SexosMessage msg = sexos_listen(&listener, 0);
    uint64_t caller = msg.caller_pd;
    uint32_t key = (uint32_t) msg.arg0;
    if (msg.type_id == OP_KV_PUT) handle_put(&s, caller, key, msg.arg1);
    else if (msg.type_id == OP_KV_GET) handle_get(&s, caller, key);
    else if (msg.type_id == OP_KV_DEL) handle_delete(&s, caller, key);
    else {
/*An unknown opcode is answered with 0.*/
      if (take(&s.log_reply_error)) sexos_line("[sexstore.reply.error] caller=%llu op=%#llx", (unsigned long long) caller, (unsigned long long) msg.type_id);
      (void) sexos_reply((uint32_t) caller, 0);
    }
  }
}
