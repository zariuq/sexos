#include "sexos_server.h"
#include "silkbar_model.h"
/*SilkBar producer, translated from servers/silkbar/src/main.rs. The Rust function-local statics are fields of one owned Silkbar state; the loop keeps the Rust order of every budget, send and yield.*/
/*Build-time proof profiles (option_env! in the Rust source).*/
#if defined(SEXOS_BELL_DELIVERY_PROOF)
#define BELL_DELIVERY_PROOF_ENABLED 1

#else
#define BELL_DELIVERY_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SILKBAR_CLOCK_FORCE_STALL_PROOF)
#define CLOCK_FORCE_STALL_PROOF_ENABLED 1

#else
#define CLOCK_FORCE_STALL_PROOF_ENABLED 0

#endif
#if !defined(SEXOS_SILKBAR_DISPLAY_READY)
#define SEXOS_SILKBAR_DISPLAY_READY 1

#endif
#define SILKBAR_DISPLAY_READY_ENABLED (((SEXOS_SILKBAR_DISPLAY_READY) != 0))
#define SILKBAR_DISPLAY_READY_OP UINT64_C(0x101)
#define SILKBAR_DISPLAY_READY_VERSION UINT64_C(1)
#define SILKBAR_STARTUP_CAPACITY ((size_t)256)
#if !defined(SEXOS_SILKBAR_BELL_REPLY_CALLER)
#define SEXOS_SILKBAR_BELL_REPLY_CALLER 10

#endif
/*Clock cadence: ticks per logical second for each tick source.*/
#define STALE_REAL_TICK_FALLBACK_LOOPS ((uint16_t)16)
#define STALE_REAL_TICK_FALLBACK_THRESHOLD ((uint16_t)4)
#define STEADY_CLOCK_THRESHOLD ((uint16_t)62)
#define REAL_TICK_VISIBLE_CLOCK_THRESHOLD STEADY_CLOCK_THRESHOLD
#define SYNTHETIC_VISIBLE_CLOCK_THRESHOLD ((uint16_t)8)
#define LIVE_CLOCK_THRESHOLD REAL_TICK_VISIBLE_CLOCK_THRESHOLD
#define BELL_LIST_ARGS (UINT64_C(0xFF) | (UINT64_C(1) << 8))
typedef struct Silkbar {
  bool degraded;
/*SERIAL_DIET_V1: shared budget for hot-loop diagnostics.*/
  uint32_t hot_log;
  bool cap_ready_display;
  bool defer_emitted_display;
  uint32_t edge_send;
  uint64_t drop_counter;
  uint32_t drop_log;
  uint64_t clock_drop_counter;
  uint32_t clock_drop_log;
  uint32_t cadence_start;
  uint32_t stall_after_start;
  uint32_t stall_before_recv;
  uint32_t stall_before_bell;
  uint32_t bell_reply;
  uint32_t bell_gen_changed;
  uint32_t bell_gen_steady;
  uint32_t bell_gen_denied;
  uint32_t stall_after_bell;
  uint32_t stall_after_recv;
  uint32_t stall_before_shell_sync;
  uint32_t stall_after_shell_sync;
  uint32_t synthetic_visible;
  uint32_t real_tick;
  uint32_t real_tick_resume;
  uint32_t stale;
  uint32_t synthetic_threshold;
  uint32_t cadence_done;
  uint32_t loop_alive;
  uint32_t clock_tick;
  uint32_t bell_gen_cap_err;
  uint32_t force_stall_suppress;
  uint32_t cadence_sample;
  uint32_t boot_canary;
  uint32_t boot_canary_suppress;
  uint32_t source_liveness;
  uint32_t clock_send_count;
}
Silkbar;
typedef struct SilkbarStartup {
  SexosMessage messages[256];
  size_t head;
  size_t len;
  bool ready;
}
SilkbarStartup;
static SilkbarStartup silkbar_startup;
static bool silkbar_startup_observe(SexosMessage message) {
  if (message.type_id == SILKBAR_DISPLAY_READY_OP) {
    if (message.caller_pd == 1 && (message.arg0 == SILKBAR_DISPLAY_READY_VERSION && (message.arg1 == 0 && message.arg2 == 0))) silkbar_startup.ready = true;
    else sexos_line("[display.ready.rejected] client=silkbar caller=%u version=%llu", (unsigned) message.caller_pd, (unsigned long long) message.arg0);
    return true;
  }
  if (silkbar_startup.len >= SILKBAR_STARTUP_CAPACITY) return false;
  silkbar_startup.messages[(silkbar_startup.head + silkbar_startup.len) % SILKBAR_STARTUP_CAPACITY] = message;
  silkbar_startup.len += 1;
  return true;
}
static void silkbar_wait_display(void) {
  while (!silkbar_startup.ready) {
    if (silkbar_startup.len >= SILKBAR_STARTUP_CAPACITY) sexos_fail();
    SexosMessage message;
    if (sexos_try_listen(0, &message)) if (!silkbar_startup_observe(message)) sexos_fail();
    sexos_yield();
  }
}
static bool silkbar_startup_pop(SexosMessage *out) {
  if (silkbar_startup.len == 0) return false;
  *out = silkbar_startup.messages[silkbar_startup.head];
  silkbar_startup.head = (silkbar_startup.head + 1) % SILKBAR_STARTUP_CAPACITY;
  silkbar_startup.len -= 1;
  return true;
}
/*A budget allows its line while it lasts.*/
static bool take(uint32_t *budget) {
  if (*budget == 0) return false;
  *budget -= 1;
  return true;
}
static uint64_t send_update_status(Silkbar *s, SilkBarUpdate update) {
  if (take(&s->edge_send)) sexos_line("[bootgraph.edge.send from=silkbar to=sexdisplay slot=SLOT_DISPLAY op=OP_SILKBAR_UPDATE first=1]");
  SexosReply result = sexos_pdx_call(SEXOS_SLOT_display, SEXOS_SILKBAR_update, (uint64_t) update.kind, (uint64_t) update.index << 32 | (uint64_t) update.a, (uint64_t) update.b);
  if (!s->cap_ready_display) {
    if (result.status == 0) s->cap_ready_display = true;
    else if (result.status == SEXOS_ERR_CAP_INVALID) {
      if (!s->defer_emitted_display) {
        s->defer_emitted_display = true;
        sexos_line("[bootgraph.edge.defer from=silkbar to=sexdisplay slot=5 reason=missing_cap]");
      }
      return result.status;
    }
  }
  if (result.status != 0) {
    uint64_t err = result.status;
    uint64_t n = s->drop_counter;
    s->drop_counter += 1;
    if (take(&s->drop_log)) sexos_line("[silkbar.send_update.drop] kind=%u idx=%u err=%#llx count=%llu", (unsigned) update.kind, (unsigned) update.index, (unsigned long long) err, (unsigned long long) n);
/*Clock-specific drops: 64 entries, so starvation survives startup noise.*/
    if (update.kind == SILKBAR_UPDATE_SET_CLOCK) {
      uint64_t cn = s->clock_drop_counter;
      s->clock_drop_counter += 1;
      if (take(&s->clock_drop_log)) sexos_line("[silkbar.send_update.drop.clock] count=%llu err=%#llx", (unsigned long long) cn, (unsigned long long) err);
    }
    return err;
  }
  return 0;
}
static void send_update(Silkbar *s, SilkBarUpdate update) {
  (void) send_update_status(s, update);
}
/*LIST asks Bell for its lane summary; the reply arrives through listen.*/
static bool bell_list(void) {
  return sexos_pdx_call(SEXOS_SLOT_bell, SEXOS_BELL_list, BELL_LIST_ARGS, 0, 0).status == 0;
}
void silkbar_main(void) {
  Silkbar s = (Silkbar) {
    .hot_log = 120,.edge_send = 1,.drop_log = 16,.clock_drop_log = 64,.cadence_start = 16,.stall_after_start = 2,.stall_before_recv = 2,.stall_before_bell = 2,.bell_reply = 8,.bell_gen_changed = 8,.bell_gen_steady = 1,.bell_gen_denied = 8,.stall_after_bell = 2,.stall_after_recv = 2,.stall_before_shell_sync = 2,.stall_after_shell_sync = 2,.synthetic_visible = 4,.real_tick = 12,.real_tick_resume = 4,.stale = 8,.synthetic_threshold = 1,.cadence_done = 16,.loop_alive = 16,.clock_tick = 12,.bell_gen_cap_err = 8,.force_stall_suppress = 4,.cadence_sample = 32,.boot_canary = 8,.boot_canary_suppress = 4,.source_liveness = 32
  };
  sexos_line("[silkbar.init.start]");
  if (SILKBAR_DISPLAY_READY_ENABLED) {
    silkbar_wait_display();
    sexos_line("[display.ready.accepted] client=silkbar caller=1 version=1");
  }
  sexos_line("[silkbar.boot.begin]");
  sexos_line("[bootgraph.silkbar.spawned]");
  uint32_t contract_err = validate_silkbar_contract();
  s.degraded = contract_err != 0;
  if (s.degraded) {
    sexos_line("[silk.de.contract.producer.fail] reason=%u abi=%u layout=%u theme=%u fp=0x%016llx", (unsigned) contract_err, (unsigned) SILK_DE_BAR_ABI_V1, (unsigned) SILK_DE_BAR_LAYOUT_V1, (unsigned) SILK_DE_BAR_THEME_V1, (unsigned long long) silkbar_contract_fingerprint());
    sexos_line("[silkbar.boot.contract.fail] code=%u", (unsigned) contract_err);
    sexos_line("[bootgraph.silkbar.degraded] reason=contract_fail");
  } else {
    sexos_line("[silk.de.contract.producer.pass] abi=%u layout=%u theme=%u fp=0x%016llx", (unsigned) SILK_DE_BAR_ABI_V1, (unsigned) SILK_DE_BAR_LAYOUT_V1, (unsigned) SILK_DE_BAR_THEME_V1, (unsigned long long) silkbar_contract_fingerprint());
    sexos_line("[silkbar.boot.contract.ok]");
    sexos_line("[bootgraph.silkbar.contract_ready]");
  }
  sexos_line("[silk.contract.validate.version] version=%llu", (unsigned long long) SILKBAR_ABI_VERSION);
  uint8_t focus_state = 0;
  uint8_t last_focus_state = 0xFF;
  uint32_t last_options_mask = 0;
  uint8_t chip_phase = 0;
  bool chip0_net = true;
/*Software clock: one logical second per cadence of yields.*/
  uint8_t hh = DEFAULT_SILK_BAR.clock_hh;
  uint8_t mm = DEFAULT_SILK_BAR.clock_mm;
  uint8_t ss = DEFAULT_SILK_BAR.clock_ss;
  uint64_t loop_iter = 0;
  uint16_t cadence_yields = 0;
/*Prefer real monotonic ticks; fall back to counting yields when get_ticks is 0 or stalls.*/
  uint64_t last_clock_ticks = 0;
  bool clock_source_logged = false;
  uint16_t stale_tick_loops = 0;
  uint8_t boot_clock_sends = 0;
  bool force_stall_seeded = false;
  uint8_t force_stall_ss = 0;
  uint16_t force_stall_repeat_sends = 0;
/*Cached Bell generation (0 forces the first LIST); a LIST awaiting its reply.*/
  uint64_t bell_gen_cached = 0;
  bool bell_pending_list = false;
/*Initial updates are deferred until the loop is live; the clock is not initialized.*/
  SilkBarUpdate init_deferred[SILKBAR_WORKSPACE_COUNT + SILKBAR_CHIP_COUNT];
  size_t init_deferred_count = 0;
  if (!s.degraded) {
    for (uint8_t ws_idx = 0; ws_idx < SILKBAR_WORKSPACE_COUNT; ++ws_idx) {
      init_deferred[init_deferred_count] = silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_ACTIVE, ws_idx, ws_idx == SILKBAR_DEFAULT_ACTIVE_WORKSPACE_IDX ? 1u : 0u, 0);
      ++init_deferred_count;
    }
    for (uint8_t chip_idx = 0; chip_idx < SILKBAR_CHIP_COUNT; ++chip_idx) {
      init_deferred[init_deferred_count] = silkbar_update(SILKBAR_UPDATE_SET_CHIP_VISIBLE, chip_idx, 1, 0);
      ++init_deferred_count;
    }
  }
  size_t init_deferred_next = 0;
  sexos_line("[silkbar.boot.init.defer] count=%zu", init_deferred_count);
  sexos_line("[silkbar.ready]");
  while (1) {
    if (take(&s.cadence_start)) sexos_line("[silkbar.loop.cadence.start] iter=%llu", (unsigned long long) loop_iter);
    if (take(&s.stall_after_start)) sexos_line("[silkbar.stall.after_start] iter=%llu", (unsigned long long) loop_iter);
    if (loop_iter == 0) sexos_line("[bootgraph.silkbar.loop_ready]");
/*At most one upstream message per pass, without blocking.*/
    if (take(&s.stall_before_recv)) sexos_line("[silkbar.stall.before_recv] iter=%llu", (unsigned long long) loop_iter);
    SexosMessage msg;
    if ((SILKBAR_DISPLAY_READY_ENABLED && silkbar_startup_pop(&msg)) || sexos_try_listen(0, &msg)) {
      if (msg.type_id == SEXOS_SILKBAR_workspace_active) {
        uint8_t ws = (uint8_t) msg.arg0;
        if (ws > SILKBAR_WORKSPACE_IDX_MAX) {
          sexos_line("[silkbar.update.reject] kind=%llu idx=%u reason=out_of_bounds", (unsigned long long) msg.type_id, (unsigned) ws);
          sexos_line("[silkbar.cadence.no_skip] reason=workspace iter=%llu", (unsigned long long) loop_iter);
        }
        sexos_line("[silkbar.workspace.recv] index=%u", (unsigned) ws);
        sexos_line("[silkbar.workspace.active.set] index=%u", (unsigned) ws);
        sexos_line("[silkbar.workspace.active.send.start] index=%u", (unsigned) ws);
        if (!s.degraded) for (uint8_t i = 0; i < SILKBAR_WORKSPACE_COUNT; ++i) send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_ACTIVE, i, i == ws ? 1u : 0u, 0));
        sexos_line("[silkbar.workspace.active.send.ok] index=%u", (unsigned) ws);
      } else if (msg.type_id == SEXOS_SILKBAR_focus_state) {
        uint8_t raw = (uint8_t) msg.arg0;
        if (raw > 3) {
          sexos_line("[silkbar.update.reject] kind=%llu idx=%u reason=out_of_bounds", (unsigned long long) msg.type_id, (unsigned) raw);
          sexos_line("[silkbar.cadence.no_skip] reason=options iter=%llu", (unsigned long long) loop_iter);
        }
        focus_state = raw;
/*Selected-window options travel in arg1; old senders pass 0.*/
        uint32_t options_mask = (uint32_t) msg.arg1;
        sexos_line("[silkbar.selected.options.recv] mask=%#x", (unsigned) options_mask);
        if (!s.degraded && options_mask != last_options_mask) {
          last_options_mask = options_mask;
          send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_SELECTED_OPTIONS, 0, options_mask, 0));
          sexos_line("[silkbar.selected.options.forward] mask=%#x", (unsigned) options_mask);
        }
      } else if (msg.type_id == 1 && msg.caller_pd == SEXOS_SILKBAR_BELL_REPLY_CALLER) {
/*Bell reply: a LIST's packed counts, or a SUBSCRIBE's generation.*/
        if (take(&s.stall_before_bell)) sexos_line("[silkbar.stall.before_bell] iter=%llu", (unsigned long long) loop_iter);
        if (bell_pending_list) {
/*Repack for SetBellPresence: total | redacted << 8 | flags << 16; flags bit 0 = Bell answered.*/
          bell_pending_list = false;
          uint64_t packed = msg.arg0;
          uint8_t total = (uint8_t)(packed & 0xFF);
          uint8_t redacted = (uint8_t)(packed >> 56 & 0xFF);
          uint8_t flags = 1;
          uint32_t a = (uint32_t) total | (uint32_t) redacted << 8 | (uint32_t) flags << 16;
          if (take(&s.bell_reply)) sexos_line("[silkbar.bell.poll.reply] total=%u redacted=%u flags=%#x", (unsigned) total, (unsigned) redacted, (unsigned) flags);
          if (!s.degraded) send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_BELL_PRESENCE, 0, a, 0));
          if (BELL_DELIVERY_PROOF_ENABLED) {
            sexos_line("[bell.poll.ok] total=%u redacted=%u flags=%#x", (unsigned) total, (unsigned) redacted, (unsigned) flags);
            sexos_line("[silkbar.bell.state] total=%u redacted=%u flags=%#x", (unsigned) total, (unsigned) redacted, (unsigned) flags);
          }
        } else {
          uint64_t gen = msg.arg0;
          unsigned changed = gen != bell_gen_cached && gen != UINT64_MAX ? 1u : 0u;
          if (changed ? take(&s.bell_gen_changed) : take(&s.bell_gen_steady)) sexos_line("[silkbar.bell.gen.reply] gen=%llu changed=%u", (unsigned long long) gen, changed);
          if (gen == UINT64_MAX) {
/*Denied: fall back to LIST.*/
            if (take(&s.bell_gen_denied)) sexos_line("[silkbar.bell.gen.fallback] reason=denied");
            if (bell_list()) bell_pending_list = true;
            else if (!s.degraded) send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_BELL_PRESENCE, 0, 0, 0));
          } else if (gen != bell_gen_cached) {
/*Generation changed: ask for the new counts.*/
            bell_gen_cached = gen;
            if (bell_list()) bell_pending_list = true;
          }
        }
        if (take(&s.stall_after_bell)) sexos_line("[silkbar.stall.after_bell] iter=%llu", (unsigned long long) loop_iter);
        if (take(&s.hot_log)) sexos_line("[silkbar.stall.after_bell_all] iter=%llu", (unsigned long long) loop_iter);
      } else sexos_line("[pdx.opcode.unknown] silkbar type_id=%#llx caller=%u", (unsigned long long) msg.type_id, (unsigned) msg.caller_pd);
    }
    if (take(&s.stall_after_recv)) sexos_line("[silkbar.stall.after_recv] iter=%llu", (unsigned long long) loop_iter);
    if (take(&s.stall_before_shell_sync)) sexos_line("[silkbar.stall.before_shell_sync] iter=%llu", (unsigned long long) loop_iter);
    if (focus_state != last_focus_state) {
/*Focus drives the urgent highlight: none clears; shell, app and debug mark workspaces 0, 1 and 2.*/
      int urgent_ws = focus_state >= 1 && focus_state <= 3 ? (int) focus_state - 1 : -1;
      if (!s.degraded) for (uint8_t ws = 0; ws < SILKBAR_WORKSPACE_COUNT; ++ws) send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_URGENT, ws, (int) ws == urgent_ws ? 1u : 0u, 0));
      last_focus_state = focus_state;
    }
    if (take(&s.stall_after_shell_sync)) sexos_line("[silkbar.stall.after_shell_sync] iter=%llu", (unsigned long long) loop_iter);
    if (!s.degraded && init_deferred_next < init_deferred_count) {
/*Burst every deferred update in one pass, so sexdisplay redraws once.*/
      size_t count = init_deferred_count;
      while (init_deferred_next < init_deferred_count) {
        send_update(&s, init_deferred[init_deferred_next]);
        ++init_deferred_next;
      }
      sexos_line("[silkbar.boot.init.flush.burst.done] count=%zu", count);
    }
/*Tick-watch cadence: one tick change is one cadence step; a threshold of steps is one logical second.*/
    uint16_t cadence_threshold = LIVE_CLOCK_THRESHOLD;
    uint64_t raw_ticks = sexos_get_ticks();
    if (raw_ticks == 0) {
/*QEMU TCG: count yields at the visible synthetic rate.*/
      stale_tick_loops = 0;
      cadence_threshold = SYNTHETIC_VISIBLE_CLOCK_THRESHOLD;
      cadence_yields = (uint16_t)(cadence_yields + 1);
      if (!clock_source_logged && loop_iter >= 10) {
        sexos_line("[silkbar.clock.source] kind=synthetic raw_ticks=0 threshold=%u", (unsigned) cadence_threshold);
        clock_source_logged = true;
      }
      if (take(&s.synthetic_visible)) sexos_line("[silkbar.clock.synthetic.visible] threshold=%u", (unsigned) cadence_threshold);
    } else if (raw_ticks > last_clock_ticks) {
/*Real ticks advanced: resume from any stale fallback.*/
      bool was_stale = stale_tick_loops >= STALE_REAL_TICK_FALLBACK_LOOPS;
      stale_tick_loops = 0;
      last_clock_ticks = raw_ticks;
      cadence_yields = (uint16_t)(cadence_yields + 1);
      if (!clock_source_logged) {
        sexos_line("[silkbar.clock.source] kind=real raw_ticks=%llu", (unsigned long long) raw_ticks);
        clock_source_logged = true;
      }
      if (take(&s.real_tick)) sexos_line("[silkbar.clock.real_tick] ticks=%llu iter=%llu yield_count=%u", (unsigned long long) raw_ticks, (unsigned long long) loop_iter, (unsigned) cadence_yields);
      if (was_stale && take(&s.real_tick_resume)) sexos_line("[silkbar.clock.real_tick.resume] ticks=%llu", (unsigned long long) raw_ticks);
    } else {
/*Real ticks exist but have not advanced: after a grace window, count yields instead.*/
      stale_tick_loops = (uint16_t)(stale_tick_loops + 1);
      if (stale_tick_loops >= STALE_REAL_TICK_FALLBACK_LOOPS) {
        cadence_threshold = STALE_REAL_TICK_FALLBACK_THRESHOLD;
        cadence_yields = (uint16_t)(cadence_yields + 1);
        if (take(&s.stale)) sexos_line("[silkbar.clock.real_tick.stale] ticks=%llu loops=%u fallback=synthetic threshold=%u", (unsigned long long) raw_ticks, (unsigned) stale_tick_loops, (unsigned) cadence_threshold);
      }
    }
/*Iteration 2 proof, at milestones of the cadence only.*/
    if (loop_iter == 2) {
      uint16_t half = (uint16_t)(cadence_threshold / 2);
      if (cadence_yields == 1 || cadence_yields == half || ((uint16_t)(cadence_yields + 1) == cadence_threshold || cadence_yields == cadence_threshold)) sexos_line("[silkbar.loop.cadence.count] iter=%llu yield_count=%u threshold=%u", (unsigned long long) loop_iter, (unsigned) cadence_yields, (unsigned) cadence_threshold);
    }
    if (loop_iter == 10 && s.synthetic_threshold > 0 && cadence_yields == 1) {
      s.synthetic_threshold -= 1;
      sexos_line("[silkbar.clock.synthetic.threshold] iter=%llu threshold=%u reason=until_real_timer", (unsigned long long) loop_iter, (unsigned) cadence_threshold);
    }
    sexos_yield();
    if (cadence_yields < cadence_threshold) continue;
    cadence_yields = 0;
    if (take(&s.cadence_done) && take(&s.hot_log)) sexos_line("[silkbar.loop.cadence.done] iter=%llu", (unsigned long long) loop_iter);
/*One logical second per cadence.*/
    if (take(&s.hot_log)) sexos_line("[silkbar.loop.iter_advance] old=%llu new=%llu", (unsigned long long) loop_iter, (unsigned long long)(loop_iter + 1));
    loop_iter += 1;
    if (take(&s.loop_alive) && take(&s.hot_log)) sexos_line("[silkbar.loop.alive] iter=%llu", (unsigned long long) loop_iter);
    ss = (uint8_t)(ss + 1);
    if (ss >= 60) {
      ss = 0;
      mm = (uint8_t)(mm + 1);
      if (mm >= 60) {
        mm = 0;
        hh = (uint8_t)(hh + 1);
        if (hh >= 24) hh = 0;
      }
    }
    if (take(&s.clock_tick)) sexos_line("[silkbar.clock.tick] hh=%u mm=%u ss=%u raw_ticks=%llu", (unsigned) hh, (unsigned) mm, (unsigned) ss, (unsigned long long) raw_ticks);
/*Bell presence poll every 2 seconds: SUBSCRIBE for the generation; on failure, LIST.*/
    if (!s.degraded && loop_iter % 2 == 0 && !bell_pending_list) {
      SexosReply result = sexos_pdx_call(SEXOS_SLOT_bell, SEXOS_BELL_subscribe, 0, 0, 0);
      if (result.status != 0) {
        if (take(&s.bell_gen_cap_err)) sexos_line("[silkbar.bell.gen.fallback] reason=cap_err err=%#llx", (unsigned long long) result.status);
        if (bell_list()) bell_pending_list = true;
        else send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_BELL_PRESENCE, 0, 0, 0));
      }
    }
/*Status-chip stub: every 120 seconds, one bounded change.*/
    if (!s.degraded && loop_iter % 120 == 0) {
      switch (chip_phase) {
        case 0: {
          send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_CHIP_KIND, SILKBAR_CHIP_SLOT_CHIP0, chip0_net ? SILKBAR_CHIP_NET : SILKBAR_CHIP_WIFI, 0));
          send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_CHIP_KIND, SILKBAR_CHIP_SLOT_CHIP1, SILKBAR_CHIP_WIFI, 0));
          chip0_net = !chip0_net;
        }
        break;
        case 1: send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_CHIP_KIND, SILKBAR_CHIP_SLOT_CHIP2, SILKBAR_CHIP_BATTERY, 0));
        break;
        case 2: send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_CHIP_KIND, SILKBAR_CHIP_SLOT_CLOCK, SILKBAR_CHIP_NET, 0));
        break;
        default: send_update(&s, silkbar_update(SILKBAR_UPDATE_SET_CHIP_KIND, SILKBAR_CHIP_SLOT_CLOCK, SILKBAR_CHIP_BATTERY, 0));
      }
      chip_phase = (uint8_t)((chip_phase + 1) & 3);
    }
    bool clock_sent = false;
    uint64_t clock_status = UINT64_MAX;
    if (!s.degraded) {
      if (CLOCK_FORCE_STALL_PROOF_ENABLED) {
        if (!force_stall_seeded) {
          force_stall_seeded = true;
          force_stall_ss = ss;
          clock_sent = true;
          sexos_line("[silkbar.clock.force_stall.seed] hh=%u mm=%u ss=%u", (unsigned) hh, (unsigned) mm, (unsigned) ss);
          clock_status = send_update_status(&s, silkbar_update(SILKBAR_UPDATE_SET_CLOCK, 0, hh, (uint32_t) mm << 8 | ss));
        } else if (force_stall_repeat_sends < 124) {
          uint8_t burst = 0;
          while (burst < 4 && force_stall_repeat_sends < 124) {
            force_stall_repeat_sends = (uint16_t)(force_stall_repeat_sends + 1);
            clock_status = send_update_status(&s, silkbar_update(SILKBAR_UPDATE_SET_CLOCK, 0, hh, (uint32_t) mm << 8 | force_stall_ss));
            burst = (uint8_t)(burst + 1);
          }
          clock_sent = true;
          if (force_stall_repeat_sends % 20 == 0) sexos_line("[silkbar.clock.force_stall.repeat] ss=%u sends=%u", (unsigned) force_stall_ss, (unsigned) force_stall_repeat_sends);
        } else if (take(&s.force_stall_suppress)) sexos_line("[silkbar.clock.force_stall.suppress] repeats=%u status=hold", (unsigned) force_stall_repeat_sends);
      } else {
/*Bounded clock send: the cadence itself limits the rate.*/
        clock_sent = true;
        clock_status = send_update_status(&s, silkbar_update(SILKBAR_UPDATE_SET_CLOCK, 0, hh, (uint32_t) mm << 8 | ss));
      }
    }
    if (clock_sent && take(&s.cadence_sample) && take(&s.hot_log)) sexos_line("[silkbar.clock.cadence.sample] ss=%u yields=%u threshold=%u sent=1 ok=1", (unsigned) ss, (unsigned) cadence_yields, (unsigned) cadence_threshold);
    if (!s.degraded) {
      if (clock_sent) boot_clock_sends = (uint8_t)(boot_clock_sends + 1);
      if (take(&s.boot_canary)) sexos_line("[silkbar.clock.boot_canary] send=%u threshold=%u", (unsigned) boot_clock_sends, (unsigned) cadence_threshold);
    } else if (take(&s.boot_canary_suppress)) sexos_line("[silkbar.clock.boot_canary] suppressed=degraded threshold=%u", (unsigned) cadence_threshold);
/*Clock-source liveness, once per clock minute.*/
    if (!s.degraded && clock_sent && loop_iter > 0 && loop_iter % 60 == 0) if (take(&s.source_liveness)) sexos_line("[silkbar.clock.source.liveness] iter=%llu ss=%u threshold=%u ok=1", (unsigned long long) loop_iter, (unsigned) ss, (unsigned) cadence_threshold);
    if (loop_iter == 1) {
      if (s.degraded) sexos_line("[bootgraph.silkbar.degraded] reason=clock_send_disabled");
      else sexos_line("[bootgraph.silkbar.clock_ready]");
    }
/*Clock-send liveness: the first 12 sends, then every 64th.*/
    s.clock_send_count += 1;
    if (s.clock_send_count <= 12 || s.clock_send_count % 64 == 0) sexos_line("[silkbar.clock.send] hh=%u mm=%u ss=%u status=%#llx", (unsigned) hh, (unsigned) mm, (unsigned) ss, (unsigned long long) clock_status);
  }
}
