#include "sexos_server.h"
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((unsigned char)256 == 0 && (char)-1 < 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires x86_64 SysV");
/*Bell attention server, translated from servers/sexbell/src/main.rs. The Rust statics (the queue, the generation, the mute list, the policy table, the spam budget and the log budgets) are fields of one owned Sexbell state; the queue keeps its own record and methods.*/
/*Build-time proof profiles (option_env! in the Rust source).*/
#if defined(SEXOS_BELL_DELIVERY_PROOF)
#define BELL_DELIVERY_PROOF_ENABLED 1

#else
#define BELL_DELIVERY_PROOF_ENABLED 0

#endif
#if defined(SEXOS_BELL_BRIDGE_STUB_PROOF)
#define BELL_BRIDGE_STUB_PROOF_ENABLED 1

#else
#define BELL_BRIDGE_STUB_PROOF_ENABLED 0

#endif
#if defined(SEXOS_BELL_LANE_RENAME_PROOF)
#define BELL_LANE_RENAME_PROOF_ENABLED 1

#else
#define BELL_LANE_RENAME_PROOF_ENABLED 0

#endif
/*Attention lanes. The ordinal order is load-bearing: eviction takes the lowest lane first, so renaming must never reorder them.*/
#define LANE_LATER ((uint8_t)0)
/*Linen-object and workspace events, reserved until Linen lands.*/
#define LANE_PROJECT ((uint8_t)1)
#define LANE_SOON ((uint8_t)2)
/*Dev-mode and OS-health events, such as SelfCapDenied.*/
#define LANE_SYSTEM ((uint8_t)3)
#define LANE_NOW ((uint8_t)4)
/*Reserved for future security-class events.*/
#define LANE_RESERVED_5 ((uint8_t)5)
#define BELL_QUEUE_CAPACITY ((size_t)16)
#define MUTE_LIST_CAPACITY ((size_t)16)
#define POLICY_TABLE_CAPACITY ((size_t)8)
/*Spam budget: at most 8 events per sender per 62-tick window (about a second), 16 senders tracked.*/
#define SPAM_WINDOW_TICKS UINT64_C(62)
#define SPAM_MAX_PER_WINDOW UINT32_C(8)
#define SPAM_BUDGET_SLOTS ((size_t)16)
/*A PD self-reporting its own ERR_CAP_INVALID (sex_pdx::BELL_CATEGORY_SELF_CAP_DENIED); its urgency hint is ignored.*/
#define BELL_CATEGORY_SELF_CAP_DENIED ((uint8_t)6)
typedef struct BellQueueEntry {
/*Assigned on accept; 0 is the invalid sentinel.*/
  uint64_t event_id;
/*The kernel-authoritative sender.*/
  uint32_t caller_pd;
  uint8_t category;
  uint8_t requested_lane;
  uint8_t final_lane;
  uint8_t final_urgency;
  uint8_t privacy_level;
  uint8_t redaction_class;
/*V1: at most one action and one object reference, marker-only.*/
  uint8_t action_count;
  uint8_t action_id;
  uint8_t object_ref_count;
  uint8_t object_ref;
/*1 once dismissed by CLOSE or CLEAR; LIST skips it.*/
  uint8_t dismissed;
  uint8_t pad[2];
}
BellQueueEntry;
_Static_assert(sizeof(BellQueueEntry) == 32 && offsetof(BellQueueEntry, dismissed) == 22, "repr(C) BellQueueEntry");

typedef struct BellQueue {
/*Monotonic; 0 is reserved.*/
  uint64_t next_event_id;
  uint16_t head;
  uint16_t tail;
  uint16_t count;
  BellQueueEntry entries[BELL_QUEUE_CAPACITY];
}
BellQueue;
/*Per-target policy; target_pd 0 marks an unused slot. Flags: bit 0 privacy override, bit 1 lane override, bit 2 forced mute.*/
typedef struct PolicyEntry {
  uint32_t target_pd;
  uint8_t active_flags;
  uint8_t privacy_level;
  uint8_t lane_override;
  uint8_t force_mute;
}
PolicyEntry;
typedef struct SpamSlot {
  uint32_t caller_pd;
  uint32_t count;
  uint64_t window_start;
}
SpamSlot;
typedef struct Sexbell {
  BellQueue queue;
/*Bumped on every change visible through the queue or the mute list, so subscribers can poll LIST only when it moved; wrapping only costs an extra poll.*/
  uint64_t generation;
  uint32_t mute_list[MUTE_LIST_CAPACITY];
  size_t mute_count;
/*Volatile: lost when Bell restarts.*/
  PolicyEntry policy_table[POLICY_TABLE_CAPACITY];
  size_t policy_count;
  SpamSlot spam_slots[SPAM_BUDGET_SLOTS];
  bool bridge_stub_done;
/*Log budgets, in the order the Rust declares them.*/
  uint32_t muted_reject_log;
  uint32_t reject_log;
  uint32_t recv_log;
  uint32_t selfreport_log;
  uint32_t downgrade_log;
  uint32_t spam_reject_log;
  uint32_t queue_drop_log;
  uint32_t queue_push_log;
  uint32_t ok_log;
  uint32_t queue_full_log;
  uint32_t full_reject_log;
  uint32_t list_lane_reject_log;
  uint32_t list_count_reject_log;
  uint32_t readcap_deny_log;
  uint32_t list_item_log;
  uint32_t list_redact_log;
  uint32_t list_reply_log;
  uint32_t close_ok_log;
  uint32_t close_reject_log;
  uint32_t action_dispatch_log;
  uint32_t action_reject_log;
  uint32_t clear_all_log;
  uint32_t clear_lane_log;
  uint32_t clear_reject_log;
  uint32_t mute_add_log;
  uint32_t mute_full_log;
  uint32_t mute_remove_log;
  uint32_t mute_missing_log;
  uint32_t mute_action_log;
  uint32_t policy_deny_log;
  uint32_t policy_field_log;
  uint32_t policy_reduction_log;
  uint32_t policy_full_log;
  uint32_t policy_set_log;
  uint32_t subscribe_deny_log;
  uint32_t subscribe_reply_log;
  uint32_t unknown_log;
}
Sexbell;
/*A budget allows its line while it lasts.*/
static bool take(uint32_t *budget) {
  if (*budget == 0) return false;
  *budget -= 1;
  return true;
}
static BellQueueEntry entry_new(uint64_t event_id, uint32_t caller_pd, uint8_t category, uint8_t requested_lane, uint8_t final_lane, uint8_t final_urgency, uint8_t privacy_level, uint8_t redaction_class, uint8_t action_count, uint8_t action_id, uint8_t object_ref_count, uint8_t object_ref) {
  return (BellQueueEntry) {
    .event_id = event_id,.caller_pd = caller_pd,.category = category,.requested_lane = requested_lane,.final_lane = final_lane,.final_urgency = final_urgency,.privacy_level = privacy_level,.redaction_class = redaction_class,.action_count = action_count,.action_id = action_id,.object_ref_count = object_ref_count,.object_ref = object_ref
  };
}
static bool bell_queue_is_full(const BellQueue *q) {
  return q->count >= BELL_QUEUE_CAPACITY;
}
/*The lowest-priority active entry: the lowest lane. The Rust's comment says ties go to the oldest, but its condition replaces the candidate whenever a later position has the same lane, so the newest of the tied entries is chosen; that observed behaviour is what is ported.*/
static bool bell_queue_find_lowest_priority_index(const BellQueue *q, uint16_t *out) {
  bool found = false;
  uint8_t lowest_lane = 6;
  uint16_t lowest_dist = 0;
  for (size_t i = 0; i < q->count; ++i) {
    size_t idx = (q->head + i) % BELL_QUEUE_CAPACITY;
    const BellQueueEntry *entry = &q->entries[idx];
    if (entry->dismissed != 0) continue;
    if (entry->final_lane < lowest_lane || (entry->final_lane == lowest_lane && (!found || (uint16_t) i > lowest_dist))) {
      lowest_lane = entry->final_lane;
      lowest_dist = (uint16_t) i;
      *out = (uint16_t) idx;
      found = true;
    }
  }
  return found;
}
static uint64_t take_event_id(BellQueue *q) {
  uint64_t event_id = q->next_event_id;
/*Wrap past UINT64_MAX to 1; 0 is the invalid sentinel.*/
  q->next_event_id = event_id == UINT64_MAX ? 1 : event_id + 1;
  return event_id;
}
/*Push an entry and assign its event id. A full queue gives up its lowest-priority active entry to the new one and reports that entry's lane through *dropped_lane; a full queue of dismissed entries refuses (queue_full).*/
static bool bell_queue_push(BellQueue *q, BellQueueEntry entry, uint64_t *event_id, bool *dropped, uint8_t *dropped_lane) {
  *dropped = false;
  if (bell_queue_is_full(q)) {
    uint16_t drop_idx = 0;
    if (!bell_queue_find_lowest_priority_index(q, &drop_idx)) return false;
    *dropped_lane = q->entries[drop_idx].final_lane;
    *dropped = true;
    entry.event_id = take_event_id(q);
    q->entries[drop_idx] = entry;
    *event_id = entry.event_id;
    return true;
  }
  entry.event_id = take_event_id(q);
  q->entries[q->tail % BELL_QUEUE_CAPACITY] = entry;
  q->tail = (uint16_t)((q->tail + 1) % BELL_QUEUE_CAPACITY);
  q->count += 1;
  *event_id = entry.event_id;
  return true;
}
static void bump_generation(uint64_t *generation) {
  *generation += 1;
}
/*LIST and SUBSCRIBE: default-deny; silk-shell (3) and silkbar (6), the privacy-safe aggregate poller.*/
static bool is_list_reader_allowed(uint32_t caller_pd) {
  return caller_pd == 3 || caller_pd == 6;
}
/*SET_POLICY: silk-shell only; silkbar is a reader, not an authority.*/
static bool is_policy_author_allowed(uint32_t caller_pd) {
  return caller_pd == 3;
}
static bool is_muted(const Sexbell *s, uint32_t caller_pd) {
  for (size_t i = 0; i < s->mute_count; ++i) if (s->mute_list[i] == caller_pd) return true;
  return false;
}
/*Idempotent; false when the list is full (mute_list_full).*/
static bool add_mute(Sexbell *s, uint32_t caller_pd) {
  if (is_muted(s, caller_pd)) return true;
  if (s->mute_count >= MUTE_LIST_CAPACITY) return false;
  s->mute_list[s->mute_count] = caller_pd;
  s->mute_count += 1;
  return true;
}
static bool remove_mute(Sexbell *s, uint32_t caller_pd) {
  for (size_t i = 0; i < s->mute_count; ++i) if (s->mute_list[i] == caller_pd) {
    for (size_t j = i; j + 1 < s->mute_count; ++j) s->mute_list[j] = s->mute_list[j + 1];
    s->mute_count -= 1;
    return true;
  }
  return false;
}
static const PolicyEntry *find_policy(const Sexbell *s, uint32_t target_pd) {
  for (size_t i = 0; i < s->policy_count; ++i) if (s->policy_table[i].target_pd == target_pd) return &s->policy_table[i];
  return NULL;
}
static bool is_policy_muted(const Sexbell *s, uint32_t target_pd) {
  const PolicyEntry *entry = find_policy(s, target_pd);
  return entry != NULL && ((entry->active_flags & 4) != 0 && entry->force_mute != 0);
}
/*A privacy override only raises restriction: the larger of the event's level and the policy's.*/
static uint8_t apply_policy_privacy(const Sexbell *s, uint32_t caller_pd, uint8_t event_privacy) {
  const PolicyEntry *entry = find_policy(s, caller_pd);
  if (entry != NULL && (entry->active_flags & 1) != 0) return entry->privacy_level > event_privacy ? entry->privacy_level : event_privacy;
  return event_privacy;
}
static uint8_t apply_policy_lane(const Sexbell *s, uint32_t caller_pd, uint8_t derived_lane) {
  const PolicyEntry *entry = find_policy(s, caller_pd);
  if (entry != NULL && (entry->active_flags & 2) != 0) return entry->lane_override;
  return derived_lane;
}
/*Record one notify for a sender: false when its window already holds the maximum. A new sender takes an empty slot, else the slot with the oldest window.*/
static bool check_spam_budget(Sexbell *s, uint32_t caller_pd) {
  uint64_t now = sexos_get_ticks();
  for (size_t i = 0; i < SPAM_BUDGET_SLOTS; ++i) {
    SpamSlot *slot = &s->spam_slots[i];
    if (slot->caller_pd == caller_pd) {
      if ((now > slot->window_start ? now - slot->window_start : 0) >= SPAM_WINDOW_TICKS) {
        slot->window_start = now;
        slot->count = 1;
        return true;
      }
      if (slot->count >= SPAM_MAX_PER_WINDOW) return false;
      slot->count += 1;
      return true;
    }
  }
  size_t oldest_idx = 0;
  uint64_t oldest_tick = UINT64_MAX;
  for (size_t i = 0; i < SPAM_BUDGET_SLOTS; ++i) {
    if (s->spam_slots[i].caller_pd == 0) {
      s->spam_slots[i] = (SpamSlot) {
        .caller_pd = caller_pd,.count = 1,.window_start = now
      };
      return true;
    }
    if (s->spam_slots[i].window_start < oldest_tick) {
      oldest_tick = s->spam_slots[i].window_start;
      oldest_idx = i;
    }
  }
  s->spam_slots[oldest_idx] = (SpamSlot) {
    .caller_pd = caller_pd,.count = 1,.window_start = now
  };
  return true;
}
/*The highest privacy level a caller may view: silk-shell sees FullHidden; everyone else, by default, only Public.*/
static uint8_t max_privacy_for_caller(uint32_t caller_pd) {
  return caller_pd == 3 ? 3 : 0;
}
/*Categories 0 (Info) through 5 (Error), and 6 (SelfCapDenied).*/
static bool valid_category(uint8_t v) {
  return v <= 6;
}
/*Public through FullHidden.*/
static bool valid_privacy_level(uint8_t v) {
  return v <= 3;
}
/*StructuralMeta through SecretContent.*/
static bool valid_redaction_class(uint8_t v) {
  return v <= 3;
}
/*First-proof lane derivation: no BellCap table exists yet, so every sender is untrusted and lands in LANE_LATER; a nonzero urgency hint is a downgrade, reported with its reason.*/
static uint8_t derive_lane_first_proof(uint8_t urgency_hint, uint8_t *final_urgency, const char **downgrade) {
  *final_urgency = 0;
  *downgrade = urgency_hint == 0 ? NULL : "no_caps_untrusted";
  return LANE_LATER;
}
/*Bell Bridge status stub (phase 1 of BELL_BRIDGE_APP_LAUNCH_PLAN_V1): present but inert, no IPC, launch, focus or rendering.*/
static void maybe_run_bell_bridge_status_stub(Sexbell *s) {
  if (!BELL_BRIDGE_STUB_PROOF_ENABLED || s->bridge_stub_done) return;
  sexos_line("[bell.bridge.status.stub] phase=1 ipc=0 launch=0 focus=0 render=0");
  sexos_line("[bell.bridge.status.ready] ok=1");
  s->bridge_stub_done = true;
}
/*One-shot proof that the lane rename kept the ordinal order eviction relies on.*/
static void maybe_run_bell_lane_rename_proof(void) {
  if (!BELL_LANE_RENAME_PROOF_ENABLED) return;
  sexos_line("[bell.lane.rename] old=0 new=LATER ordinal=0");
  sexos_line("[bell.lane.rename] old=1 new=PROJECT ordinal=1");
  sexos_line("[bell.lane.rename] old=2 new=SOON ordinal=2");
  sexos_line("[bell.lane.rename] old=3 new=SYSTEM ordinal=3");
  sexos_line("[bell.lane.rename] old=4 new=NOW ordinal=4");
  sexos_line("[bell.lane.rename] old=5 new=RESERVED_5 ordinal=5");
  sexos_line("[bell.lane.rename.done] ok=1 order_preserved=%u", (unsigned)((LANE_LATER < LANE_PROJECT) & (LANE_PROJECT < LANE_SOON) & (LANE_SOON < LANE_SYSTEM) & (LANE_SYSTEM < LANE_NOW) & (LANE_NOW < LANE_RESERVED_5)));
}
/*OP_BELL_NOTIFY. arg0 packs category, urgency hint, privacy and redaction in its low four bytes; arg1 the action count and id; arg2 the object reference count and reference.*/
static void notify(Sexbell *s, const SexosMessage *msg) {
  uint8_t category = (uint8_t) msg->arg0;
  uint8_t urgency_hint = (uint8_t)(msg->arg0 >> 8);
  uint8_t privacy_level = (uint8_t)(msg->arg0 >> 16);
  uint8_t redaction_class = (uint8_t)(msg->arg0 >> 24);
  uint8_t action_count = (uint8_t) msg->arg1;
  uint8_t action_id = (uint8_t)(msg->arg1 >> 8);
  uint8_t object_ref_count = (uint8_t) msg->arg2;
  uint8_t object_ref = (uint8_t)(msg->arg2 >> 8);
  uint32_t caller_pd = msg->caller_pd;
/*Muted senders are refused before anything else.*/
  if (is_muted(s, caller_pd) || is_policy_muted(s, caller_pd)) {
    if (take(&s->muted_reject_log)) sexos_line("[bell.notify.reject] caller_pd=%u reason=muted", (unsigned) caller_pd);
    return;
  }
  const char *reject_reason = NULL;
  if (!valid_category(category)) reject_reason = "invalid_category";
  else if (!valid_privacy_level(privacy_level)) reject_reason = "invalid_privacy";
  else if (!valid_redaction_class(redaction_class)) reject_reason = "invalid_redaction";
  else if (urgency_hint > 3) reject_reason = "invalid_urgency";
  else if (action_count > 1) reject_reason = "action_count_invalid";
  else if (action_count == 1 && action_id == 0) reject_reason = "action_id_zero";
  else if (object_ref_count > 1) reject_reason = "object_refs_invalid";
  if (reject_reason != NULL) {
    if (BELL_DELIVERY_PROOF_ENABLED) sexos_line("[bell.event.reject] caller_pd=%u reason=%s", (unsigned) caller_pd, reject_reason);
    if (take(&s->reject_log)) sexos_line("[bell.notify.reject] caller_pd=%u reason=%s", (unsigned) caller_pd, reject_reason);
    return;
  }
  if (take(&s->recv_log)) sexos_line("[bell.notify.recv] caller_pd=%u category=%u requested=%u", (unsigned) caller_pd, (unsigned) category, (unsigned) urgency_hint);
  uint8_t final_urgency = 0;
  const char *downgrade = NULL;
  uint8_t final_lane = derive_lane_first_proof(urgency_hint, &final_urgency, &downgrade);
/*SelfCapDenied always lands in LANE_SYSTEM: a sender's urgency hint must never buy that class a higher lane, or a spoofed category 6 could jump the queue.*/
  if (category == BELL_CATEGORY_SELF_CAP_DENIED) {
    final_lane = LANE_SYSTEM;
    if (take(&s->selfreport_log)) sexos_line("[bell.selfreport.capdenied] caller_pd=%u target_op=%u", (unsigned) caller_pd, (unsigned) object_ref);
  }
  if (downgrade != NULL && take(&s->downgrade_log)) sexos_line("[bell.notify.downgrade] from=%u to=%u reason=%s", (unsigned) urgency_hint, (unsigned) final_lane, downgrade);
  if (!check_spam_budget(s, caller_pd)) {
    if (take(&s->spam_reject_log)) sexos_line("[bell.notify.reject] caller_pd=%u reason=spam_budget_exceeded window=64 max=8", (unsigned) caller_pd);
    return;
  }
/*Policy overrides: privacy only rises, a lane override replaces the derived lane.*/
  uint8_t effective_privacy = apply_policy_privacy(s, caller_pd, privacy_level);
  uint8_t effective_lane = apply_policy_lane(s, caller_pd, final_lane);
  uint64_t event_id = 0;
  bool dropped = false;
  uint8_t dropped_lane = 0;
  if (!bell_queue_push(&s->queue, entry_new(0, caller_pd, category, urgency_hint, effective_lane, final_urgency, effective_privacy, redaction_class, action_count, action_id, object_ref_count, object_ref), &event_id, &dropped, &dropped_lane)) {
    if (BELL_DELIVERY_PROOF_ENABLED) sexos_line("[bell.event.reject] caller_pd=%u reason=queue_full", (unsigned) caller_pd);
    if (take(&s->queue_full_log)) sexos_line("[bell.queue.reject.full] count=%zu", BELL_QUEUE_CAPACITY);
    if (take(&s->full_reject_log)) sexos_line("[bell.notify.reject] caller_pd=%u reason=queue_full", (unsigned) caller_pd);
    return;
  }
  if (BELL_DELIVERY_PROOF_ENABLED) sexos_line("[bell.event.accept] caller_pd=%u event_id=%llu lane=%u", (unsigned) caller_pd, (unsigned long long) event_id, (unsigned) effective_lane);
/*The drop, push and ok markers report the derived lane, before any policy override, as the Rust does.*/
  if (dropped && take(&s->queue_drop_log)) sexos_line("[bell.queue.drop] reason=lowest_priority lane=%u dropped_lane=%u event_id=%llu", (unsigned) final_lane, (unsigned) dropped_lane, (unsigned long long) event_id);
  if (take(&s->queue_push_log)) sexos_line("[bell.queue.push] id=%llu final_lane=%u count=%u", (unsigned long long) event_id, (unsigned) final_lane, (unsigned) s->queue.count);
  if (take(&s->ok_log)) sexos_line("[bell.notify.ok] caller_pd=%u final_lane=%u event_id=%llu", (unsigned) caller_pd, (unsigned) final_lane, (unsigned long long) event_id);
  bump_generation(&s->generation);
}
/*The queue position i steps back from the newest entry.*/
static size_t newest_first(const BellQueue *q, size_t i) {
  return (q->tail + BELL_QUEUE_CAPACITY - (1 + i)) % BELL_QUEUE_CAPACITY;
}
/*OP_BELL_LIST: arg0 packs a lane filter (0xFF all, else 0..5) and a result count (1..4). Protocol errors are reported to any caller; then the read capability is checked before the queue is touched. The reply packs the counts by lane and the redacted count: total in bits 0-7, lanes 0-5 in the next six bytes, redacted in bits 56-63.*/
static void list(Sexbell *s, const SexosMessage *msg) {
  uint8_t lane_filter = (uint8_t) msg->arg0;
  uint8_t max_results = (uint8_t)(msg->arg0 >> 8);
  uint32_t caller_pd = msg->caller_pd;
  if (lane_filter != 0xFF && lane_filter > 5) {
    if (take(&s->list_lane_reject_log)) sexos_line("[bell.list.reject] reason=invalid_lane caller_pd=%u", (unsigned) caller_pd);
    return;
  }
  if (max_results == 0 || max_results > 4) {
    if (take(&s->list_count_reject_log)) sexos_line("[bell.list.reject] reason=invalid_count caller_pd=%u", (unsigned) caller_pd);
    return;
  }
  if (!is_list_reader_allowed(caller_pd)) {
    if (take(&s->readcap_deny_log)) sexos_line("[bell.readcap.deny] caller_pd=%u op=list reason=no_read_cap", (unsigned) caller_pd);
/*An error reply, so the caller does not hang.*/
    (void) sexos_reply(caller_pd, UINT64_MAX);
    return;
  }
  uint8_t caller_max_privacy = max_privacy_for_caller(caller_pd);
  const BellQueue *q = &s->queue;
/*Aggregate counts over the whole queue, not limited by the result count.*/
  uint32_t lane_counts[6] = {
    0
  };
  uint32_t total_visible = 0;
  for (size_t i = 0; i < q->count; ++i) {
    const BellQueueEntry *entry = &q->entries[newest_first(q, i)];
    if (entry->dismissed != 0 || entry->privacy_level > caller_max_privacy) continue;
    if (entry->final_lane < 6) {
      lane_counts[entry->final_lane] += 1;
      total_visible += 1;
    }
  }
/*Newest first, read-only; FullHidden entries above the caller's level are counted but not shown.*/
  uint32_t match_count = 0;
  uint32_t redact_count = 0;
  for (size_t i = 0; i < q->count; ++i) {
    const BellQueueEntry *entry = &q->entries[newest_first(q, i)];
    if (entry->dismissed != 0) continue;
    if (entry->privacy_level > caller_max_privacy) {
      if (entry->privacy_level == 3) redact_count += 1;
      continue;
    }
    if (lane_filter == 0xFF || entry->final_lane == lane_filter) {
      if (take(&s->list_item_log)) sexos_line("[bell.list.item] event_id=%llu final_lane=%u category=%u privacy=%u redaction=%u actions=%u refs=%u", (unsigned long long) entry->event_id, (unsigned) entry->final_lane, (unsigned) entry->category, (unsigned) entry->privacy_level, (unsigned) entry->redaction_class, (unsigned) entry->action_count, (unsigned) entry->object_ref_count);
      match_count += 1;
      if (match_count >= max_results) break;
    }
  }
  if (redact_count > 0 && take(&s->list_redact_log)) sexos_line("[bell.list.redact] reason=full_hidden count=%u caller_pd=%u", (unsigned) redact_count, (unsigned) caller_pd);
  uint64_t packed = total_visible & 0xFF;
  for (unsigned lane = 0; lane < 6; ++lane) packed |= (uint64_t)(lane_counts[lane] & 0xFF) << 8 * (lane + 1);
  packed |= (uint64_t)(redact_count & 0xFF) << 56;
  if (take(&s->list_reply_log)) sexos_line("[bell.list.reply] total=%u lanes=[%u %u %u %u %u %u] redacted=%u", (unsigned) total_visible, (unsigned) lane_counts[0], (unsigned) lane_counts[1], (unsigned) lane_counts[2], (unsigned) lane_counts[3], (unsigned) lane_counts[4], (unsigned) lane_counts[5], (unsigned) redact_count);
  (void) sexos_reply(caller_pd, packed);
}
/*OP_BELL_CLOSE: dismiss the active entry with event id arg0. Any caller may close.*/
static void close_event(Sexbell *s, const SexosMessage *msg) {
  uint64_t event_id = msg->arg0;
  BellQueue *q = &s->queue;
  bool found = false;
  for (size_t i = 0; i < q->count; ++i) {
    BellQueueEntry *entry = &q->entries[(q->head + i) % BELL_QUEUE_CAPACITY];
    if (entry->event_id == event_id && entry->dismissed == 0) {
      entry->dismissed = 1;
      found = true;
      break;
    }
  }
  if (found) {
    if (take(&s->close_ok_log)) sexos_line("[bell.close.ok] event_id=%llu", (unsigned long long) event_id);
    bump_generation(&s->generation);
  } else if (take(&s->close_reject_log)) sexos_line("[bell.close.reject] reason=not_found event_id=%llu caller_pd=%u", (unsigned long long) event_id, (unsigned) msg->caller_pd);
}
/*OP_BELL_ACTION: report the dispatch of action arg1 on the active entry arg0 (marker-only in V1).*/
static void action(Sexbell *s, const SexosMessage *msg) {
  uint64_t event_id = msg->arg0;
  uint8_t action_id = (uint8_t) msg->arg1;
  const BellQueue *q = &s->queue;
  for (size_t i = 0; i < q->count; ++i) {
    const BellQueueEntry *entry = &q->entries[(q->head + i) % BELL_QUEUE_CAPACITY];
    if (entry->event_id == event_id && entry->dismissed == 0 && (entry->action_count > 0 && entry->action_id == action_id)) {
      if (take(&s->action_dispatch_log)) sexos_line("[bell.action.dispatch] event_id=%llu action_id=%u lane=%u", (unsigned long long) event_id, (unsigned) action_id, (unsigned) entry->final_lane);
      return;
    }
  }
  if (take(&s->action_reject_log)) sexos_line("[bell.action.reject] reason=not_found event_id=%llu action_id=%u caller_pd=%u", (unsigned long long) event_id, (unsigned) action_id, (unsigned) msg->caller_pd);
}
/*OP_BELL_CLEAR: 0xFF resets the whole queue; a lane 0..5 dismisses that lane's active entries.*/
static void clear(Sexbell *s, const SexosMessage *msg) {
  uint8_t lane_filter = (uint8_t) msg->arg0;
  uint32_t caller_pd = msg->caller_pd;
  BellQueue *q = &s->queue;
  if (lane_filter == 0xFF) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    if (take(&s->clear_all_log)) sexos_line("[bell.clear.ok] lane=all caller_pd=%u", (unsigned) caller_pd);
    bump_generation(&s->generation);
    return;
  }
  if (lane_filter > 5) {
    if (take(&s->clear_reject_log)) sexos_line("[bell.clear.reject] reason=invalid_lane lane=%u caller_pd=%u", (unsigned) lane_filter, (unsigned) caller_pd);
    return;
  }
  uint32_t dismiss_count = 0;
  for (size_t i = 0; i < q->count; ++i) {
    BellQueueEntry *entry = &q->entries[(q->head + i) % BELL_QUEUE_CAPACITY];
    if (entry->final_lane == lane_filter && entry->dismissed == 0) {
      entry->dismissed = 1;
      dismiss_count += 1;
    }
  }
  if (take(&s->clear_lane_log)) sexos_line("[bell.clear.ok] lane=%u count=%u caller_pd=%u", (unsigned) lane_filter, (unsigned) dismiss_count, (unsigned) caller_pd);
  if (dismiss_count > 0) bump_generation(&s->generation);
}
/*OP_BELL_MUTE_SENDER: arg0 packs the sender (bits 0-31) and the action (bits 32-39: 0 mute, 1 unmute).*/
static void mute_sender(Sexbell *s, const SexosMessage *msg) {
  uint32_t mute_pd = (uint32_t) msg->arg0;
  uint8_t act = (uint8_t)(msg->arg0 >> 32);
  uint32_t caller_pd = msg->caller_pd;
  if (act == 0) {
    if (add_mute(s, mute_pd)) {
      if (take(&s->mute_add_log)) sexos_line("[bell.mute.add] mute_pd=%u caller_pd=%u", (unsigned) mute_pd, (unsigned) caller_pd);
      bump_generation(&s->generation);
    } else if (take(&s->mute_full_log)) sexos_line("[bell.mute.reject] reason=mute_list_full mute_pd=%u caller_pd=%u", (unsigned) mute_pd, (unsigned) caller_pd);
    return;
  }
  if (act == 1) {
    if (remove_mute(s, mute_pd)) {
      if (take(&s->mute_remove_log)) sexos_line("[bell.mute.remove] mute_pd=%u caller_pd=%u", (unsigned) mute_pd, (unsigned) caller_pd);
      bump_generation(&s->generation);
    } else if (take(&s->mute_missing_log)) sexos_line("[bell.mute.reject] reason=not_found mute_pd=%u caller_pd=%u", (unsigned) mute_pd, (unsigned) caller_pd);
    return;
  }
  if (take(&s->mute_action_log)) sexos_line("[bell.mute.reject] reason=invalid_action action=%u caller_pd=%u", (unsigned) act, (unsigned) caller_pd);
}
/*OP_BELL_SET_POLICY: arg0 is the target, arg1 the policy (flags in bits 0-2; privacy in bits 8-9, lane in bits 16-18, mute in bit 24). All flags clear removes the target's entry. A privacy override may never lower an existing one. Every outcome is replied to: 0 when set, all ones when refused.*/
static void set_policy(Sexbell *s, const SexosMessage *msg) {
  uint32_t target_pd = (uint32_t) msg->arg0;
  uint64_t packed = msg->arg1;
  uint32_t caller_pd = msg->caller_pd;
  if (!is_policy_author_allowed(caller_pd)) {
    if (take(&s->policy_deny_log)) sexos_line("[bell.policy.deny] caller_pd=%u target_pd=%u", (unsigned) caller_pd, (unsigned) target_pd);
    (void) sexos_reply(caller_pd, UINT64_MAX);
    return;
  }
  uint8_t active_flags = (uint8_t)(packed & 0x7);
  uint8_t privacy_val = (uint8_t)(packed >> 8 & 0x3);
  uint8_t lane_val = (uint8_t)(packed >> 16 & 0x7);
  uint8_t mute_val = (uint8_t)(packed >> 24 & 0x1);
/*A 2-bit privacy value is never above 3, so only the lane check can refuse.*/
  if (((active_flags & 1) != 0 && privacy_val > 3) || ((active_flags & 2) != 0 && lane_val > 5)) {
    if (take(&s->policy_field_log)) sexos_line("[bell.policy.reject] reason=invalid_field caller_pd=%u target_pd=%u", (unsigned) caller_pd, (unsigned) target_pd);
    (void) sexos_reply(caller_pd, UINT64_MAX);
    return;
  }
  bool changed = false;
  PolicyEntry new_entry = (PolicyEntry) {
    .target_pd = target_pd,.active_flags = active_flags,.privacy_level = privacy_val,.lane_override = lane_val,.force_mute = mute_val
  };
  if (active_flags == 0) {
    for (size_t i = 0; i < s->policy_count; ++i) if (s->policy_table[i].target_pd == target_pd) {
      for (size_t j = i; j + 1 < s->policy_count; ++j) s->policy_table[j] = s->policy_table[j + 1];
      s->policy_table[s->policy_count - 1].target_pd = 0;
      s->policy_count -= 1;
      changed = true;
      break;
    }
  } else {
    if ((active_flags & 1) != 0) {
      const PolicyEntry *existing = find_policy(s, target_pd);
      if (existing != NULL && ((existing->active_flags & 1) != 0 && privacy_val < existing->privacy_level)) {
        if (take(&s->policy_reduction_log)) sexos_line("[bell.policy.reject] reason=privacy_reduction caller_pd=%u target_pd=%u old=%u new=%u", (unsigned) caller_pd, (unsigned) target_pd, (unsigned) existing->privacy_level, (unsigned) privacy_val);
        (void) sexos_reply(caller_pd, UINT64_MAX);
        return;
      }
    }
    bool found = false;
    for (size_t i = 0; i < s->policy_count; ++i) {
      PolicyEntry *old = &s->policy_table[i];
      if (old->target_pd == target_pd) {
        changed = old->active_flags != active_flags || old->privacy_level != privacy_val || (old->lane_override != lane_val || old->force_mute != mute_val);
        *old = new_entry;
        found = true;
        break;
      }
    }
    if (!found) {
      if (s->policy_count >= POLICY_TABLE_CAPACITY) {
        if (take(&s->policy_full_log)) sexos_line("[bell.policy.reject] reason=table_full caller_pd=%u target_pd=%u", (unsigned) caller_pd, (unsigned) target_pd);
        (void) sexos_reply(caller_pd, UINT64_MAX);
        return;
      }
      s->policy_table[s->policy_count] = new_entry;
      s->policy_count += 1;
      changed = true;
    }
  }
  if (changed) bump_generation(&s->generation);
  if (take(&s->policy_set_log)) sexos_line("[bell.policy.set] caller_pd=%u target_pd=%u flags=%u privacy=%u lane=%u mute=%u", (unsigned) caller_pd, (unsigned) target_pd, (unsigned) active_flags, (unsigned) privacy_val, (unsigned) lane_val, (unsigned) mute_val);
  (void) sexos_reply(caller_pd, 0);
}
/*OP_BELL_SUBSCRIBE: the current generation, for LIST readers only.*/
static void subscribe(Sexbell *s, const SexosMessage *msg) {
  uint32_t caller_pd = msg->caller_pd;
  if (!is_list_reader_allowed(caller_pd)) {
    if (take(&s->subscribe_deny_log)) sexos_line("[bell.subscribe.deny] caller_pd=%u", (unsigned) caller_pd);
    (void) sexos_reply(caller_pd, UINT64_MAX);
    return;
  }
  uint64_t gen = s->generation;
  (void) sexos_reply(caller_pd, gen);
  if (take(&s->subscribe_reply_log)) sexos_line("[bell.subscribe.reply] gen=%llu", (unsigned long long) gen);
}
void sexbell_main(void) {
  Sexbell s = (Sexbell) {
    .queue = (BellQueue) {
      .next_event_id = 1
    },.generation = 1,.muted_reject_log = 8,.reject_log = 4,.recv_log = 8,.selfreport_log = 8,.downgrade_log = 8,.spam_reject_log = 8,.queue_drop_log = 16,.queue_push_log = 64,.ok_log = 8,.queue_full_log = 16,.full_reject_log = 4,.list_lane_reject_log = 4,.list_count_reject_log = 4,.readcap_deny_log = 8,.list_item_log = 8,.list_redact_log = 8,.list_reply_log = 8,.close_ok_log = 8,.close_reject_log = 4,.action_dispatch_log = 8,.action_reject_log = 4,.clear_all_log = 4,.clear_lane_log = 4,.clear_reject_log = 4,.mute_add_log = 8,.mute_full_log = 4,.mute_remove_log = 8,.mute_missing_log = 4,.mute_action_log = 4,.policy_deny_log = 8,.policy_field_log = 8,.policy_reduction_log = 8,.policy_full_log = 8,.policy_set_log = 8,.subscribe_deny_log = 8,.subscribe_reply_log = 4,.unknown_log = 8
  };
  sexos_line("[sexbell.init.start]");
  sexos_line("[bell.boot]");
  maybe_run_bell_bridge_status_stub(&s);
  maybe_run_bell_lane_rename_proof();
/*Demo self-notify (V1): one internal event (caller 0: Info, LANE_LATER, Public) exercises the Bell to SilkBar to sexdisplay path; SilkBar polls LIST and forwards the counts for the gold dot and badge.*/
  uint64_t event_id = 0;
  bool dropped = false;
  uint8_t dropped_lane = 0;
  if (bell_queue_push(&s.queue, entry_new(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), &event_id, &dropped, &dropped_lane)) {
    bump_generation(&s.generation);
    sexos_line("[bell.demo.boot] event_id=%llu", (unsigned long long) event_id);
  } else sexos_line("[bell.demo.boot.reject] reason=queue_full");
  sexos_line("[sexbell.ready]");
  SexosListener listener = (SexosListener) {
    .wrapper_budget = 8
  };
  while (1) {
    SexosMessage msg = sexos_listen(&listener, 0);
    switch (msg.type_id) {
      case SEXOS_BELL_notify: notify(&s, &msg);
      break;
      case SEXOS_BELL_list: list(&s, &msg);
      break;
      case SEXOS_BELL_close: close_event(&s, &msg);
      break;
      case SEXOS_BELL_action: action(&s, &msg);
      break;
      case SEXOS_BELL_clear: clear(&s, &msg);
      break;
      case SEXOS_BELL_mute_sender: mute_sender(&s, &msg);
      break;
      case SEXOS_BELL_set_policy: set_policy(&s, &msg);
      break;
      case SEXOS_BELL_subscribe: subscribe(&s, &msg);
      break;
      default: if (take(&s.unknown_log)) sexos_line("[bell.unknown.reject] type_id=%#llx", (unsigned long long) msg.type_id);
    }
  }
}
