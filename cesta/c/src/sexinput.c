#include "sexos_server.h"
/*Input normalizer, translated from servers/sexinput/src/main.rs. The Rust statics, global and function-local, are fields of one owned Sexinput state; the emit closure of the normalizer is a callback with an explicit environment. The loop keeps the Rust order of every budget, send, listen and yield.*/
/*Build-time profiles (option_env! in the Rust source).*/
#if defined(SEXOS_PROOFS_DISABLED)
#define SYNTHETIC_INPUT_PROOFS_DISABLED 1

#else
#define SYNTHETIC_INPUT_PROOFS_DISABLED 0

#endif
#if defined(SEXOS_SILKBAR_CLICK_PROOF)
#define SILKBAR_CLICK_PROOF_ENABLED 1

#else
#define SILKBAR_CLICK_PROOF_ENABLED 0

#endif
#if defined(SEXOS_KEYBOARD_CURSOR)
#define KEYBOARD_CURSOR_ENABLED 1

#else
#define KEYBOARD_CURSOR_ENABLED 0

#endif
#if defined(SEXOS_KEYBOARD_PROOF)
#define KEYBOARD_PROOF_ENABLED 1

#else
#define KEYBOARD_PROOF_ENABLED 0

#endif
#if defined(SEXOS_CURSOR_AUTODEMO)
#define CURSOR_AUTODEMO_ENABLED 1

#else
#define CURSOR_AUTODEMO_ENABLED 0

#endif
/*Tablet reports carry 0..32767; the desktop of silk-shell is 1280x720.*/
#define TABLET_RAW_MAX ((int32_t)32767)
#define SCREEN_W ((int32_t)1280)
#define SCREEN_H ((int32_t)720)
#define DELTA_CLAMP ((int32_t)64)
#define NORMALIZED_EVENT_CAPACITY 4
typedef struct HidPointerRawReport {
  int16_t dx;
  int16_t dy;
  uint8_t buttons;
  int8_t wheel;
  bool is_abs;
}
HidPointerRawReport;
/*The emit closure: a function and the environment it captured.*/
typedef void(*EmitEvent)(void *context, uint64_t arg0, uint64_t arg1, uint64_t arg2);
/*The statics of normalize_pointer_report_v1.*/
typedef struct PointerNormalizer {
  bool abs_init;
  int32_t last_sx;
  int32_t last_sy;
  uint32_t raw_blocked_log;
  uint32_t delta_emit_log;
  uint32_t abs_forward_log;
  uint32_t evrel_log;
  uint32_t motion_forward_log;
}
PointerNormalizer;
/*TOUCHPAD_ABS_CONTACT_V1: the single-contact model.*/
typedef struct Contact {
  bool down;
  uint16_t x;
  uint16_t y;
  uint16_t start_x;
  uint16_t start_y;
  uint32_t drag_log;
  bool spurious_up_logged;
  bool last_btn0;
}
Contact;
typedef struct Sexinput {
  uint8_t last_buttons;
  uint8_t last_usb_key;
  bool cap_ready_shell;
  bool defer_emitted_shell;
  bool edge_send_emitted_shell;
  bool real_usb_pointer_seen;
  bool synthetic_drag_proof_done;
  bool synthetic_silkbar_click_proof_done;
  PointerNormalizer normalizer;
  Contact contact;
/*One-shot markers and budgets of the main loop, in source order.*/
  bool click_proof_gated;
  bool abs_translation_proof_fired;
  uint32_t raw_payload_log;
  uint32_t pointer_recv_log;
  uint32_t compat_mode_log;
  uint32_t compat_send_log;
  uint32_t compat_drop_log;
  uint32_t report_reject_log;
  uint32_t packed_mode_log;
  bool mouse_to_normalizer_marked;
  bool normalizer_out_marked;
  uint32_t mouse_real_delta_log;
  uint32_t mouse_live_log;
  uint32_t mouse_real_button_log;
  uint32_t packed_send_log;
  uint32_t hid_emit_rel_log;
  uint32_t zero_drop_log;
  bool delta_only_pass_marked;
  bool pointer_pass_marked;
  uint32_t empty_drop_log;
  uint32_t error_drop_log;
  uint32_t kbd_recv_log;
  uint32_t usb_cursor_key_log;
  uint32_t usb_cursor_rel_log;
  uint32_t key_up_emit_log;
  uint32_t keyup_ok_log;
  bool kbd_to_hid_marked;
  uint32_t key_down_emit_log;
  uint32_t keydown_ok_log;
  uint32_t kbd_drop_log;
  uint32_t ps2_key_ok_log;
  uint32_t ps2_cursor_key_log;
  uint32_t ps2_cursor_rel_log;
  bool kbd_self_test_done;
  bool silkbar_click_disabled_marked;
}
Sexinput;
/*A budget allows its line while it lasts.*/
static bool take(uint32_t *budget) {
  if (*budget == 0) return false;
  *budget -= 1;
  return true;
}
/*A one-shot marker fires the first time only.*/
static bool once(bool *fired) {
  if (*fired) return false;
  *fired = true;
  return true;
}
/*Rust's `as` from a PDX word to a narrower signed integer keeps the low bits.*/
static int32_t wrap_i32(uint64_t word) {
  uint32_t low = (uint32_t) word;
  return low < 0x80000000 ? (int32_t) low : (int32_t)(low - 0x80000000) + INT32_MIN;
}
static int16_t wrap_i16(uint64_t word) {
  int32_t low = (int32_t)(word & 0xFFFF);
  return (int16_t)(low < 0x8000 ? low : low - 0x10000);
}
static int8_t wrap_i8(uint64_t word) {
  int32_t low = (int32_t)(word & 0xFF);
  return (int8_t)(low < 0x80 ? low : low - 0x100);
}
static int32_t clamp_delta(int32_t delta) {
  if (delta < -DELTA_CLAMP) return -DELTA_CLAMP;
  if (delta > DELTA_CLAMP) return DELTA_CLAMP;
  return delta;
}
/*pdx_call to the shell whose result the Rust source ignores.*/
static void hid_event(uint64_t arg0, uint64_t arg1, uint64_t arg2) {
  (void) sexos_pdx_call(SEXOS_SLOT_shell, SEXOS_INPUT_hid_event, arg0, arg1, arg2);
}
static void shell_mouse_report(uint64_t buttons) {
  (void) sexos_pdx_call(SEXOS_SLOT_shell, SEXOS_INPUT_usb_mouse_report, 0, buttons, 0);
}
/*Parse a boot-mouse report, detect button edges, and emit normalized EV_REL and EV_BTN events: at most one movement, then up to three button edges. Movement goes first so the cursor is current when silk-shell sees a click. Absolute reports are scaled to the screen and sent as a clamped delta.*/
static size_t normalize_pointer_report_v1(PointerNormalizer *n, HidPointerRawReport report, uint8_t *last_buttons, EmitEvent emit, void *context) {
  size_t count = 0;
  uint8_t buttons = (uint8_t)(report.buttons & 0x07);
  uint8_t changed = (uint8_t)(buttons ^ *last_buttons);
  int32_t dx = report.dx;
  int32_t dy = report.dy;
/*The Rust `reason` is fixed by the branch: abs_delta for absolute reports, motion otherwise.*/
  if (report.is_abs) {
    int32_t sx = dx <= 0 ? 0 : dx >= TABLET_RAW_MAX ? SCREEN_W - 1 : dx * (SCREEN_W - 1) / TABLET_RAW_MAX;
    int32_t sy = dy <= 0 ? 0 : dy >= TABLET_RAW_MAX ? SCREEN_H - 1 : dy * (SCREEN_H - 1) / TABLET_RAW_MAX;
    int32_t ddx = 0;
    int32_t ddy = 0;
    if (n->abs_init) {
      ddx = clamp_delta(sx - n->last_sx);
      ddy = clamp_delta(sy - n->last_sy);
    } else n->abs_init = true;
    n->last_sx = sx;
    n->last_sy = sy;
    if ((dx > DELTA_CLAMP || dy > DELTA_CLAMP) && take(&n->raw_blocked_log)) sexos_line("[usb.tablet.emit.raw_blocked] raw_dx=%d raw_dy=%d ok=1", dx, dy);
    if (take(&n->delta_emit_log)) {
      sexos_line("[usb.tablet.delta.clamp] limit=%d dx_in=%d dy_in=%d dx_out=%d dy_out=%d ok=1", DELTA_CLAMP, dx, dy, ddx, ddy);
      sexos_line("[usb.tablet.abs.delta] dx=%d dy=%d buttons=%u ok=1", ddx, ddy, (unsigned) report.buttons);
      sexos_line("[usb.tablet.emit.delta_only] buttons=%u dx=%d dy=%d ok=1", (unsigned) report.buttons, ddx, ddy);
    }
    if (ddx != 0 || ddy != 0) {
      if (take(&n->abs_forward_log)) sexos_line("[sexinput.pointer.forward.reason=abs_delta]");
      if (take(&n->evrel_log)) sexos_line("[usb.pointer.producer.evrel] dx=%d dy=%d buttons=%u ok=1", ddx, ddy, (unsigned) report.buttons);
      emit(context, (uint64_t) ddx, (uint64_t) ddy, SEXOS_EV_rel);
      count += 1;
    }
  } else if (dx != 0 || dy != 0) {
    if (take(&n->motion_forward_log)) sexos_line("[sexinput.pointer.forward.reason=motion]");
    emit(context, (uint64_t) dx, (uint64_t) dy, SEXOS_EV_rel);
    count += 1;
  }
  if (changed != 0) sexos_line("[usb.hid.pointer.button] buttons=%u left=%u ok=1", (unsigned) buttons, (unsigned)((buttons & 0x01) != 0));
  uint8_t bit = 1;
  uint8_t btn_id = 1;
  while (bit <= 0x04) {
    if ((changed & bit) != 0) {
      emit(context, btn_id, (buttons & bit) != 0 ? 1 : 0, SEXOS_EV_btn);
      count += 1;
    }
    bit = (uint8_t)(bit << 1);
    btn_id += 1;
  }
  *last_buttons = buttons;
/*V1 keeps the wheel in the report model but emits no wheel events yet.*/
  return count;
}
typedef struct NormalizedEvent {
  uint64_t arg0;
  uint64_t arg1;
  uint64_t arg2;
}
NormalizedEvent;
typedef struct CollectedEvents {
  NormalizedEvent events[NORMALIZED_EVENT_CAPACITY];
  size_t count;
}
CollectedEvents;
static void collect_event(void *context, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
  CollectedEvents *collected = context;
  if (collected->count < NORMALIZED_EVENT_CAPACITY) {
    collected->events[collected->count] = (NormalizedEvent) {
      .arg0 = arg0,.arg1 = arg1,.arg2 = arg2
    };
    collected->count += 1;
  }
}
/*The synthetic drag proof sends each event at once and marks the drag.*/
static void drag_proof_emit(void *context, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
  (void) context;
  hid_event(arg0, arg1, arg2);
  if (arg2 == SEXOS_EV_btn && (arg0 == 1 && arg1 == 1)) sexos_line("[sexinput.drag_proof.down]");
  else if (arg2 == SEXOS_EV_rel) sexos_line("[sexinput.drag_proof.move]");
  else if (arg2 == SEXOS_EV_btn && (arg0 == 1 && arg1 == 0)) sexos_line("[sexinput.drag_proof.up]");
}
/*Send one HID event to silk-shell: 0 with *sent, or the failing status. Until a send first succeeds, a missing capability defers: yield once and report not sent.*/
static uint64_t send_shell_hid_event(Sexinput *s, uint64_t arg0, uint64_t arg1, uint64_t arg2, bool *sent) {
  SexosReply result = sexos_pdx_call(SEXOS_SLOT_shell, SEXOS_INPUT_hid_event, arg0, arg1, arg2);
  if (result.status == 0) {
    s->cap_ready_shell = true;
    if (once(&s->edge_send_emitted_shell)) sexos_line("[bootgraph.edge.send from=sexinput to=silk-shell slot=6 op=OP_HID_EVENT first=1]");
    *sent = true;
    return 0;
  }
  if (!s->cap_ready_shell && result.status == SEXOS_ERR_CAP_INVALID) {
    if (once(&s->defer_emitted_shell)) sexos_line("[bootgraph.edge.defer from=sexinput to=silk-shell slot=6 reason=missing_cap]");
    sexos_yield();
    *sent = false;
    return 0;
  }
  return result.status;
}
/*USB HID keyboard usage IDs and their PS/2 set 1 scancodes: Scroll Lock, Enter, F11, F12, Space, Backspace, Esc, then a to z.*/
typedef struct KeyMapping {
  uint8_t hid;
  uint8_t ps2;
}
KeyMapping;
static const KeyMapping HID_TO_PS2[] = {
  {
    0x47, 0x46
  }, {
    0x28, 0x1C
  }, {
    0x44, 0x57
  }, {
    0x45, 0x58
  }, {
    0x2C, 0x39
  }, {
    0x2A, 0x0E
  }, {
    0x29, 0x01
  }, {
    0x04, 0x1E
  }, {
    0x05, 0x30
  }, {
    0x06, 0x2E
  }, {
    0x07, 0x20
  }, {
    0x08, 0x12
  }, {
    0x09, 0x21
  }, {
    0x0A, 0x22
  }, {
    0x0B, 0x23
  }, {
    0x0C, 0x17
  }, {
    0x0D, 0x24
  }, {
    0x0E, 0x25
  }, {
    0x0F, 0x26
  }, {
    0x10, 0x32
  }, {
    0x11, 0x31
  }, {
    0x12, 0x18
  }, {
    0x13, 0x19
  }, {
    0x14, 0x10
  }, {
    0x15, 0x13
  }, {
    0x16, 0x1F
  }, {
    0x17, 0x14
  }, {
    0x18, 0x16
  }, {
    0x19, 0x2F
  }, {
    0x1A, 0x11
  }, {
    0x1B, 0x2D
  }, {
    0x1C, 0x15
  }, {
    0x1D, 0x2C
  }
};
static bool hid_to_ps2(uint8_t hid, uint8_t *scancode) {
  for (size_t i = 0; i < sizeof HID_TO_PS2 / sizeof HID_TO_PS2[0]; ++i) if (HID_TO_PS2[i].hid == hid) {
    *scancode = HID_TO_PS2[i].ps2;
    return true;
  }
  return false;
}
/*Cursor keys of the development fallback, as up, down, left, right: a letter and an arrow each.*/
typedef struct CursorKey {
  uint8_t letter;
  uint8_t arrow;
  int32_t dx;
  int32_t dy;
}
CursorKey;
static const CursorKey USB_CURSOR_KEYS[4] = {
  {
    0x1A, 0x52, 0, -8
  }, {
    0x16, 0x51, 0, 8
  }, {
    0x04, 0x50, -8, 0
  }, {
    0x07, 0x4F, 8, 0
  }
};
static const CursorKey PS2_CURSOR_KEYS[4] = {
  {
    0x11, 0x48, 0, -8
  }, {
    0x1F, 0x50, 0, 8
  }, {
    0x1E, 0x4B, -8, 0
  }, {
    0x20, 0x4D, 8, 0
  }
};
static bool cursor_step(const CursorKey *keys, uint64_t code, int32_t *dx, int32_t *dy) {
  for (size_t i = 0; i < 4; ++i) if (code == keys[i].letter || code == keys[i].arrow) {
    *dx = keys[i].dx;
    *dy = keys[i].dy;
    return true;
  }
  return false;
}
/*Development keyboard cursor: one 8-pixel EV_REL step per cursor key.*/
static void cursor_move(const CursorKey *keys, uint64_t code, uint32_t *key_log, uint32_t *rel_log) {
  int32_t dx = 0;
  int32_t dy = 0;
  if (!cursor_step(keys, code, &dx, &dy)) return;
  if (take(key_log)) sexos_line("[keyboard_cursor.key] code=0x%llx dx=%d dy=%d", (unsigned long long) code, dx, dy);
  hid_event((uint64_t) dx, (uint64_t) dy, SEXOS_EV_rel);
  if (take(rel_log)) sexos_line("[keyboard_cursor.emit.rel] dx=%d dy=%d", dx, dy);
}
/*TOUCHPAD_ABS_CONTACT_V1: button bit 0 of the absolute stream drives one contact. It only logs; what the shell receives is unchanged. A release edge while the model is idle is a desync guard, never expected.*/
static void track_contact(Contact *c, uint8_t buttons, uint16_t cx, uint16_t cy) {
  bool btn0 = (buttons & 1) != 0;
  if (btn0 && !c->down) {
    c->down = true;
    c->x = cx;
    c->y = cy;
    c->start_x = cx;
    c->start_y = cy;
    sexos_line("[sexinput.contact.down] x=%u y=%u", (unsigned) cx, (unsigned) cy);
  } else if (btn0) {
    int32_t ddx = (int32_t) cx - (int32_t) c->x;
    int32_t ddy = (int32_t) cy - (int32_t) c->y;
    c->x = cx;
    c->y = cy;
    if ((ddx != 0 || ddy != 0) && take(&c->drag_log)) sexos_line("[sexinput.contact.drag] x=%u y=%u dx=%d dy=%d", (unsigned) cx, (unsigned) cy, ddx, ddy);
  } else if (c->down) {
    int32_t tdx = (int32_t) cx - (int32_t) c->start_x;
    int32_t tdy = (int32_t) cy - (int32_t) c->start_y;
    c->down = false;
    sexos_line("[sexinput.contact.up] x=%u y=%u total_dx=%d total_dy=%d", (unsigned) cx, (unsigned) cy, tdx, tdy);
  } else if (c->last_btn0 && once(&c->spurious_up_logged)) sexos_line("[sexinput.contact.spurious_up.ignored]");
  c->last_btn0 = btn0;
}
static void log_pointer_drop(uint32_t *budget, uint64_t event_class, int32_t a0, int32_t a1, uint64_t err) {
  if (take(budget)) sexos_line("[sexinput.pointer.drop] reason=shell_send_fail class=%llu a0=%d a1=%d err=%llu", (unsigned long long) event_class, a0, a1, (unsigned long long) err);
}
/*OP_USB_MOUSE_REPORT from sexusb. True means the Rust loop continues at once: forwarded compatibility tuples and rejected reports skip the rest of the iteration, its yield included.*/
static bool usb_mouse_report(Sexinput *s, const SexosMessage *req) {
  s->real_usb_pointer_seen = true;
/*Raw payload, in hex to avoid sign-extension ambiguity; 64 lines cover the zero report and the first real ones.*/
  if (take(&s->raw_payload_log)) sexos_line("[sexinput.pointer.raw] a0=%#llx a1=%#llx a2=%#llx caller=%u", (unsigned long long) req->arg0, (unsigned long long) req->arg1, (unsigned long long) req->arg2, (unsigned) req->caller_pd);
  uint64_t cls = req->arg0;
  int32_t a0 = wrap_i32(req->arg1);
  int32_t a1 = wrap_i32(req->arg2);
  if (take(&s->pointer_recv_log)) sexos_line("[sexinput.pointer.recv] class=%llu a0=%d a1=%d", (unsigned long long) cls, a0, a1);
/*Compatibility: a producer that already sends normalized tuples (class in arg0) is forwarded as is.*/
  if (cls == SEXOS_EV_rel || (cls == SEXOS_EV_abs || cls == SEXOS_EV_btn)) {
    if (take(&s->compat_mode_log)) sexos_line("[sexinput.pointer.mode] mode=%s class=%llu a0=%d a1=%d", cls == SEXOS_EV_rel ? "rel" : cls == SEXOS_EV_abs ? "abs" : "btn", (unsigned long long) cls, a0, a1);
    if (cls == SEXOS_EV_btn) {
      if (req->arg1 != 0) sexos_line("[sexinput.pointer.button.down] btn=%llu pressed=1", (unsigned long long) req->arg0);
      else sexos_line("[sexinput.pointer.button.up] btn=%llu pressed=0", (unsigned long long) req->arg0);
    }
    if (take(&s->compat_send_log)) sexos_line("[sexinput.pointer.send] class=%llu a0=%d a1=%d", (unsigned long long) cls, a0, a1);
    bool sent = false;
    uint64_t err = send_shell_hid_event(s, req->arg1, req->arg2, cls, &sent);
    if (err != 0) log_pointer_drop(&s->compat_drop_log, cls, a0, a1, err);
    return true;
  }
  uint8_t buttons = (uint8_t) req->arg1;
  uint64_t packed = req->arg2;
  bool is_abs = (packed >> 32 & 1) != 0;
  int16_t dx = is_abs ? wrap_i16(packed) : wrap_i8(packed);
  int16_t dy = is_abs ? wrap_i16(packed >> 16) : wrap_i8(packed >> 8);
  int8_t wheel = is_abs ? 0 : wrap_i8(packed >> 16);
/*USB_HID_POINTER_PRODUCER_V1: absolute coordinates beyond 0..32767 and button bits beyond 0x07 are dropped before scaling; the delta clamp stays the second line of defence.*/
  bool abs_range_bad = is_abs && ((uint16_t) dx > 32767 || (uint16_t) dy > 32767);
  bool buttons_bad = (buttons & ~0x07) != 0;
  if (abs_range_bad || buttons_bad) {
    if (take(&s->report_reject_log)) sexos_line("[sexinput.usb.report.reject] x=%u y=%u buttons=%#x reason=%s", (unsigned)(uint16_t) dx, (unsigned)(uint16_t) dy, (unsigned) buttons, abs_range_bad ? "range" : "buttons");
    return true;
  }
  if (is_abs) track_contact(&s->contact, buttons, (uint16_t) dx, (uint16_t) dy);
  const char *kind = is_abs ? "absolute" : buttons == 0 && (dx == 0 && (dy == 0 && wheel == 0)) ? "relative_or_zero" : "relative";
  sexos_line("[sexinput.usb_mouse.decode.ok] buttons=%#x dx=%d dy=%d wheel=%d is_abs=%s", (unsigned) buttons, dx, dy, wheel, is_abs ? "true" : "false");
  sexos_line("[usb.hid.pointer.normalized] buttons=%u dx=%d dy=%d kind=%s ok=1", (unsigned) buttons, dx, dy, kind);
  sexos_line("[input.hid.pointer.recv] source=usb buttons=%u dx=%d dy=%d ok=1", (unsigned) buttons, dx, dy);
  if (take(&s->packed_mode_log)) sexos_line("[sexinput.pointer.mode] mode=%s buttons=%#x dx=%d dy=%d wheel=%d", is_abs ? "abs" : "rel", (unsigned) buttons, dx, dy, wheel);
/*AP13: one marker when the first nonzero report reaches the normalizer; the bootgraph zero report does not count.*/
  if ((buttons != 0 || (dx != 0 || (dy != 0 || is_abs))) && once(&s->mouse_to_normalizer_marked)) sexos_line("[usb.mouse.to_normalizer] dx=%d dy=%d buttons=%u wheel=%d is_abs=%u ok=1", dx, dy, (unsigned) buttons, wheel, (unsigned) is_abs);
  sexos_line("[sexinput.usb_mouse.normalize.start]");
  CollectedEvents normalized = {
    0
  };
  (void) normalize_pointer_report_v1(&s->normalizer, (HidPointerRawReport) {
    .dx = dx,.dy = dy,.buttons = buttons,.wheel = wheel,.is_abs = is_abs
  }, &s->last_buttons, collect_event, &normalized);
  size_t count = normalized.count;
  const NormalizedEvent *events = normalized.events;
  sexos_line("[sexinput.usb_mouse.normalize.ok]");
/*AP13 and AP14: the first normalized output, once.*/
  if (count > 0 && once(&s->normalizer_out_marked)) for (size_t i = 0; i < count; ++i) {
    sexos_line("[usb.mouse.normalizer.out] class=%s code=%d value=%d ok=1", events[i].arg2 == SEXOS_EV_rel ? "EV_REL" : events[i].arg2 == SEXOS_EV_abs ? "EV_ABS" : "EV_BTN", wrap_i32(events[i].arg0), wrap_i32(events[i].arg1));
    sexos_line("[usb.pointer.producer.normalized] class=%llu ok=1", (unsigned long long) events[i].arg2);
  }
/*Liveness of real reports from sexusb; synthetic proofs send HID events directly.*/
  if (take(&s->mouse_real_delta_log)) sexos_line("[sexinput.mouse.real.delta] dx=%d dy=%d buttons=%#x", dx, dy, (unsigned) buttons);
  if (take(&s->mouse_live_log)) sexos_line("[sexinput.mouse.live] n=0 dx=%d dy=%d buttons=0x%x", dx, dy, (unsigned) buttons);
  if (buttons != 0 && take(&s->mouse_real_button_log)) sexos_line("[sexinput.mouse.real.button] buttons=%#x dx=%d dy=%d", (unsigned) buttons, dx, dy);
/*Only the normalized events reach the shell; forwarding the raw report as well applied each delta twice.*/
  sexos_line("[sexinput.usb_mouse.shell_send.start]");
  uint64_t send_err = 0;
  for (size_t i = 0; i < count; ++i) {
    NormalizedEvent event = events[i];
    if (take(&s->packed_send_log)) sexos_line("[sexinput.pointer.send] class=%llu a0=%d a1=%d", (unsigned long long) event.arg2, wrap_i32(event.arg0), wrap_i32(event.arg1));
    if (event.arg2 == SEXOS_EV_rel && take(&s->hid_emit_rel_log)) sexos_line("[sexinput.hid.emit.rel] n=0 dx=%d dy=%d", wrap_i32(event.arg0), wrap_i32(event.arg1));
    bool sent = false;
    uint64_t err = send_shell_hid_event(s, event.arg0, event.arg1, event.arg2, &sent);
    if (err != 0 && send_err == 0) send_err = err;
  }
  if (count == 0 && take(&s->zero_drop_log)) sexos_line("[usb.pointer.zero_drop] buttons=%u ok=1", (unsigned) buttons);
  if (send_err != 0) {
    log_pointer_drop(&s->error_drop_log, SEXOS_EV_rel, dx, dy, send_err);
    sexos_line("[sexinput.usb_mouse.shell_send.fail] err=%llu", (unsigned long long) send_err);
    return false;
  }
  if (count == 0) {
    log_pointer_drop(&s->empty_drop_log, SEXOS_EV_rel, dx, dy, send_err);
    sexos_line("[sexinput.usb_mouse.shell_send.fail] err=%llu", (unsigned long long) send_err);
    return false;
  }
/*The marker shows the delta actually sent: the first EV_REL, else the report's own (0 when absolute).*/
  int32_t marker_dx = is_abs ? 0 : dx;
  int32_t marker_dy = is_abs ? 0 : dy;
  for (size_t i = 0; i < count; ++i) if (events[i].arg2 == SEXOS_EV_rel) {
    marker_dx = wrap_i32(events[i].arg0);
    marker_dy = wrap_i32(events[i].arg1);
    break;
  }
  sexos_line("[usb.hid.pointer.emit] op=OP_HID_EVENT buttons=%u dx=%d dy=%d ok=1", (unsigned) buttons, marker_dx, marker_dy);
  if (is_abs && once(&s->delta_only_pass_marked)) sexos_line("[usb.tablet.emit_delta_only.pass] ok=1");
  if (once(&s->pointer_pass_marked)) sexos_line("[usb.hid.pointer.pass] ok=1");
  sexos_line("[sexinput.usb_mouse.shell_send.ok]");
  return false;
}
/*One EV_KEY edge to the shell and its report.*/
static void send_key(Sexinput *s, uint8_t sc, bool down, uint8_t modifiers) {
  bool sent = false;
  uint64_t err = send_shell_hid_event(s, sc, down ? 1 : 0, SEXOS_EV_key, &sent);
  if (err != 0) {
    sexos_line("[sexinput.key.send] code=%u down=%u mod=%u dst=%u ok=0 err=%#llx", (unsigned) sc, (unsigned) down, (unsigned) modifiers, (unsigned) SEXOS_SLOT_shell, (unsigned long long) err);
    return;
  }
  sexos_line("[sexinput.key.send] code=%u down=%u mod=%u dst=%u ok=%u err=0", (unsigned) sc, (unsigned) down, (unsigned) modifiers, (unsigned) SEXOS_SLOT_shell, (unsigned) sent);
  if (down) {
    if (take(&s->keydown_ok_log)) sexos_line("[input.keyboard.keydown.ok] code=%u source=usb", (unsigned) sc);
  } else if (take(&s->keyup_ok_log)) sexos_line("[input.keyboard.keyup.ok] code=%u source=usb", (unsigned) sc);
}
/*OP_USB_KEYBOARD_REPORT from sexusb: arg1 holds the modifiers, arg2 the first keycode (a USB HID usage ID). Mapped keys go to silk-shell as EV_KEY in PS/2 set 1; the first key slot alone gives press and release edges.*/
static void usb_keyboard_report(Sexinput *s, const SexosMessage *req) {
  uint8_t modifiers = (uint8_t) req->arg1;
  uint8_t keycode = (uint8_t) req->arg2;
  if (take(&s->kbd_recv_log)) sexos_line("[sexinput.kbd.recv] key=0x%x mod=0x%x", (unsigned) keycode, (unsigned) modifiers);
  if (KEYBOARD_CURSOR_ENABLED && keycode != 0) cursor_move(USB_CURSOR_KEYS, keycode, &s->usb_cursor_key_log, &s->usb_cursor_rel_log);
  uint8_t previous = s->last_usb_key;
  if (keycode == previous) return;
  uint8_t sc = 0;
  if (previous != 0 && hid_to_ps2(previous, &sc)) {
    if (take(&s->key_up_emit_log)) sexos_line("[sexinput.key.emit] code=%u down=0 mod=%u", (unsigned) sc, (unsigned) modifiers);
    send_key(s, sc, false, modifiers);
  }
  if (keycode != 0) {
    if (hid_to_ps2(keycode, &sc)) {
      if (once(&s->kbd_to_hid_marked)) sexos_line("[usb.keyboard.to_hid] key=%u ev_key=1 ok=1", (unsigned) keycode);
      if (take(&s->key_down_emit_log)) {
        sexos_line("[sexinput.usb_kbd.evkey] hid=0x%x sc=0x%x", (unsigned) keycode, (unsigned) sc);
        sexos_line("[sexinput.key.emit] code=%u down=1 mod=%u", (unsigned) sc, (unsigned) modifiers);
      }
      send_key(s, sc, true, modifiers);
    } else if (take(&s->kbd_drop_log)) sexos_line("[sexinput.usb_kbd.drop] hid=0x%x reason=unmapped", (unsigned) keycode);
  }
  s->last_usb_key = keycode;
}
/*Kernel RawInput (0x201 on SLOT_INPUT): a PS/2 scancode, forwarded as EV_KEY with the break bit stripped.*/
static void ps2_scancode(Sexinput *s, uint64_t scancode) {
  sexos_line("[sexinput.ps2.scancode] raw=%#llx", (unsigned long long) scancode);
  uint64_t value = (scancode & 0x80) == 0 ? 1 : 0;
  uint64_t code = scancode & 0x7F;
  hid_event(code, value, SEXOS_EV_key);
  if (take(&s->ps2_key_ok_log)) {
    if (value == 1) sexos_line("[input.keyboard.keydown.ok] code=%llu source=ps2", (unsigned long long) code);
    else sexos_line("[input.keyboard.keyup.ok] code=%llu source=ps2", (unsigned long long) code);
  }
  if (KEYBOARD_CURSOR_ENABLED && value == 1) cursor_move(PS2_CURSOR_KEYS, code, &s->ps2_cursor_key_log, &s->ps2_cursor_rel_log);
}
/*Synthetic SilkBar click script, one step per listed tick: the cursor goes to a chip, then the left button goes down (with a marker naming the target) and up. The first step releases the left button the drag proof may hold. Clicks on launcher, workspace 3, status and clock open their panels; the second three close them; the bell comes last.*/
typedef struct ClickStep {
  uint64_t tick;
  uint64_t arg0;
  uint64_t arg1;
  uint64_t event_class;
  const char *target;
}
ClickStep;
static const ClickStep SILKBAR_CLICK_SCRIPT[] = {
  {
    2, 1, 0, SEXOS_EV_btn, NULL
  }, {
    3, 100, 25, SEXOS_EV_abs, NULL
  }, {
    4, 1, 1, SEXOS_EV_btn, "launcher"
  }, {
    5, 1, 0, SEXOS_EV_btn, NULL
  }, {
    7, 635, 25, SEXOS_EV_abs, NULL
  }, {
    8, 1, 1, SEXOS_EV_btn, "workspace index=3"
  }, {
    9, 1, 0, SEXOS_EV_btn, NULL
  }, {
    11, 940, 25, SEXOS_EV_abs, NULL
  }, {
    12, 1, 1, SEXOS_EV_btn, "status"
  }, {
    13, 1, 0, SEXOS_EV_btn, NULL
  }, {
    15, 1100, 25, SEXOS_EV_abs, NULL
  }, {
    16, 1, 1, SEXOS_EV_btn, "clock"
  }, {
    17, 1, 0, SEXOS_EV_btn, NULL
  }, {
    19, 100, 25, SEXOS_EV_abs, NULL
  }, {
    20, 1, 1, SEXOS_EV_btn, "launcher_close"
  }, {
    21, 1, 0, SEXOS_EV_btn, NULL
  }, {
    23, 940, 25, SEXOS_EV_abs, NULL
  }, {
    24, 1, 1, SEXOS_EV_btn, "status_close"
  }, {
    25, 1, 0, SEXOS_EV_btn, NULL
  }, {
    27, 1100, 25, SEXOS_EV_abs, NULL
  }, {
    28, 1, 1, SEXOS_EV_btn, "clock_close"
  }, {
    29, 1, 0, SEXOS_EV_btn, NULL
  }, {
    31, 1025, 25, SEXOS_EV_abs, NULL
  }, {
    32, 1, 1, SEXOS_EV_btn, "bell"
  }, {
    33, 1, 0, SEXOS_EV_btn, NULL
  }
};
#define SILKBAR_CLICK_STEPS (sizeof SILKBAR_CLICK_SCRIPT / sizeof SILKBAR_CLICK_SCRIPT[0])
/*Synthetic F5/F6 keyboard proof: F5 press and release, F5 again (the cycle wraps), then F6; each event is sent before its marker.*/
typedef struct KeyProofStep {
  uint64_t tick;
  uint64_t code;
  uint64_t value;
  const char *marker;
}
KeyProofStep;
static const KeyProofStep KEYBOARD_PROOF_SCRIPT[] = {
  {
    50, 63, 1, "[sexinput.kbd_proof.f5.down]"
  }, {
    55, 63, 0, "[sexinput.kbd_proof.f5.up]"
  }, {
    100, 63, 1, NULL
  }, {
    105, 63, 0, NULL
  }, {
    150, 64, 1, "[sexinput.kbd_proof.f6.down]"
  }, {
    155, 64, 0, "[sexinput.kbd_proof.f6.up]"
  }
};
#define KEYBOARD_PROOF_STEPS (sizeof KEYBOARD_PROOF_SCRIPT / sizeof KEYBOARD_PROOF_SCRIPT[0])
/*The synthetic drag proof: EV_ABS anchor and left button down, a move, then button up, at ticks 5, 6 and 7 (clear of the EV_KEY edge at 3 and 4 and the click focus at 10, 14 and 15).*/
static const uint64_t DRAG_TICKS[3] = {
  5, 6, 7
};
void sexinput_main(void) {
  Sexinput s = (Sexinput) {
    .normalizer = (PointerNormalizer) {
      .raw_blocked_log = 16,.delta_emit_log = 2048,.abs_forward_log = 2048,.evrel_log = 64,.motion_forward_log = 2048
    },.contact = (Contact) {
      .drag_log = 16
    },.raw_payload_log = 64,.pointer_recv_log = 16,.compat_mode_log = 32,.compat_send_log = 2048,.compat_drop_log = 16,.report_reject_log = 16,.packed_mode_log = 64,.mouse_real_delta_log = 16,.mouse_live_log = 16,.mouse_real_button_log = 16,.packed_send_log = 2048,.hid_emit_rel_log = 16,.zero_drop_log = 64,.empty_drop_log = 16,.error_drop_log = 16,.kbd_recv_log = 16,.usb_cursor_key_log = 16,.usb_cursor_rel_log = 16,.key_up_emit_log = 64,.keyup_ok_log = 8,.key_down_emit_log = 64,.keydown_ok_log = 8,.kbd_drop_log = 16,.ps2_key_ok_log = 16,.ps2_cursor_key_log = 16,.ps2_cursor_rel_log = 16
  };
  sexos_line("[sexinput.init.start]");
  sexos_heap_init();
  sexos_line("[sexinput] Normalizer Starting...");
/*USB_HID_POINTER_PRODUCER_V1: the raw 0x260 path carries absolute coordinates in 0..32767 (is_abs in bit 32 of arg2) and button bits within 0x07.*/
  sexos_line("[sexinput.usb.contract] op=0x260 raw_max=32767 buttons_mask=0x7 ok=1");
/*Gate diagnostics; each Rust budget of one fires here, at boot.*/
  sexos_line("[proof.gate.state] enabled=%u source=%s", (unsigned) !SYNTHETIC_INPUT_PROOFS_DISABLED, SYNTHETIC_INPUT_PROOFS_DISABLED ? "env" : "default");
  sexos_line("[keyboard_cursor.gate] enabled=%u source=%s", (unsigned) KEYBOARD_CURSOR_ENABLED, KEYBOARD_CURSOR_ENABLED ? "env" : "default");
  sexos_line("[cursor.autodemo.gate] enabled=%u", (unsigned) CURSOR_AUTODEMO_ENABLED);
/*The keyboard cursor fallback no longer implies the autodemo.*/
  if (KEYBOARD_CURSOR_ENABLED) sexos_line("[cursor.input.autodemo.decoupled] keyboard_cursor=1 autodemo=%u ok=1", (unsigned) CURSOR_AUTODEMO_ENABLED);
  (void) sexos_set_state(SEXOS_SERVICE_STATE_listening);
  uint64_t tick = 0;
  uint8_t drag_proof_stage = 0;
  uint8_t synth_click_stage = 0;
  size_t silkbar_click_stage = 0;
  size_t kbd_proof_stage = 0;
  uint8_t ev_key_edge_stage = 0;
  sexos_line("[sexinput.ready]");
  while (1) {
    if (once(&s.click_proof_gated)) sexos_line("[sexinput.synthetic.click.proof.gated] ok=1");
/*0a. One-shot absolute translation proof at the first iteration: center-of-range raw coordinates, chosen to pass the shell's zero_init and edge_before_ready filters.*/
    if (!SYNTHETIC_INPUT_PROOFS_DISABLED && once(&s.abs_translation_proof_fired)) {
      sexos_line("[sexinput.abs.translation.proof] x_raw=16000 y_raw=12000 ok=1");
      hid_event(16000, 12000, SEXOS_EV_abs);
    }
/*0. Reports from sexusb on the default ring.*/
    SexosMessage req = {
      0
    };
    if (sexos_try_listen(0, &req)) {
      sexos_line("[sexinput.usb_mouse.recv] type=%#llx", (unsigned long long) req.type_id);
      if (req.type_id == SEXOS_INPUT_usb_mouse_report) {
        if (usb_mouse_report(&s, &req)) continue;
      } else if (req.type_id == SEXOS_INPUT_usb_keyboard_report) usb_keyboard_report(&s, &req);
    }
/*1. Raw scancodes from the kernel; the poll does not block, so the synthetic producers run on idle ticks.*/
    if (sexos_try_listen(SEXOS_SLOT_input, &req) && req.type_id == SEXOS_INPUT_raw_input) ps2_scancode(&s, req.arg0);
/*2c. Keyboard cursor self-test: one EV_REL(0, -8) on the first iteration, on the path real cursor keys take, so the pipeline is proven without host input routing.*/
    if (KEYBOARD_CURSOR_ENABLED && once(&s.kbd_self_test_done)) {
      sexos_line("[keyboard_cursor.self_test] dx=0 dy=-8");
      hid_event(0, (uint64_t) -8, SEXOS_EV_rel);
      sexos_line("[keyboard_cursor.self_test.ok]");
    }
/*3. Synthetic drag proof, once: real USB input cancels it, and it never replays.*/
    tick += 1;
    if (!SYNTHETIC_INPUT_PROOFS_DISABLED && (!s.real_usb_pointer_seen && (!s.synthetic_drag_proof_done && (drag_proof_stage < 3 && tick == DRAG_TICKS[drag_proof_stage])))) {
      HidPointerRawReport report = (HidPointerRawReport) {
        .buttons = 0x01
      };
      if (drag_proof_stage == 0) {
        hid_event(200, 200, SEXOS_EV_abs);
        sexos_line("[sexinput.drag_proof.start]");
      } else if (drag_proof_stage == 1) {
        report.dx = 6;
        report.dy = 4;
      } else {
        s.synthetic_drag_proof_done = true;
        sexos_line("[sexinput.drag_proof.done]");
        report.buttons = 0x00;
      }(void) normalize_pointer_report_v1(&s.normalizer, report, &s.last_buttons, drag_proof_emit, NULL);
      drag_proof_stage = (uint8_t)((drag_proof_stage + 1) % 3);
    }
/*4. Synthetic SilkBar click proof through HID events.*/
    if (!SYNTHETIC_INPUT_PROOFS_DISABLED && (SILKBAR_CLICK_PROOF_ENABLED && !s.synthetic_silkbar_click_proof_done)) {
      if (silkbar_click_stage < SILKBAR_CLICK_STEPS && tick == SILKBAR_CLICK_SCRIPT[silkbar_click_stage].tick) {
        const ClickStep *step = &SILKBAR_CLICK_SCRIPT[silkbar_click_stage];
        if (step->target) sexos_line("[sexinput.synthetic.silkbar_click] target=%s", step->target);
        hid_event(step->arg0, step->arg1, step->event_class);
        silkbar_click_stage += 1;
        if (silkbar_click_stage == SILKBAR_CLICK_STEPS) s.synthetic_silkbar_click_proof_done = true;
      }
    } else if (once(&s.silkbar_click_disabled_marked)) sexos_line("[sexinput.synthetic.silkbar_click.disabled]");
/*5. Synthetic click focus through the USB mouse path: an EV_ABS anchor at (940, 560), inside the Linen surface, then a click. Explicit positioning is immune to the EV_ABS events of the SilkBar script.*/
    if (!SYNTHETIC_INPUT_PROOFS_DISABLED && !s.real_usb_pointer_seen) {
      if (synth_click_stage == 0 && tick == 10) {
        sexos_line("[sexinput.synthetic.click_focus.start]");
        shell_mouse_report(0);
        synth_click_stage = 1;
      } else if (synth_click_stage == 1 && tick == 14) {
        hid_event(940, 560, SEXOS_EV_abs);
        sexos_line("[sexinput.synthetic.click_focus.down]");
        shell_mouse_report(0x01);
        synth_click_stage = 2;
      } else if (synth_click_stage == 2 && tick == 15) {
        sexos_line("[sexinput.synthetic.click_focus.up]");
        shell_mouse_report(0);
        synth_click_stage = 3;
      }
    }
/*6a. KEYBOARD_EDGE_PROOF_V1: Enter (scancode 0x1C) down at tick 3, up at tick 4.*/
    if (!SYNTHETIC_INPUT_PROOFS_DISABLED) {
      if (ev_key_edge_stage == 0 && tick == 3) {
        hid_event(0x1C, 1, SEXOS_EV_key);
        sexos_line("[sexinput.key.ev_key.down code=0x1c]");
        ev_key_edge_stage = 1;
      } else if (ev_key_edge_stage == 1 && tick == 4) {
        hid_event(0x1C, 0, SEXOS_EV_key);
        sexos_line("[sexinput.key.ev_key.up code=0x1c]");
        ev_key_edge_stage = 2;
      }
    }
/*6. Synthetic F5/F6 keyboard proof.*/
    if (KEYBOARD_PROOF_ENABLED && (kbd_proof_stage < KEYBOARD_PROOF_STEPS && tick == KEYBOARD_PROOF_SCRIPT[kbd_proof_stage].tick)) {
      const KeyProofStep *step = &KEYBOARD_PROOF_SCRIPT[kbd_proof_stage];
      hid_event(step->code, step->value, SEXOS_EV_key);
      if (step->marker) sexos_line("%s", step->marker);
      kbd_proof_stage += 1;
    }
/*Cursor autodemo: OP_AUTODEMO_TICK every 30 ticks from 30 to 750 (24 pings) wakes the shell's demo.*/
    if (CURSOR_AUTODEMO_ENABLED && (tick >= 30 && (tick <= 750 && (tick - 30) % 30 == 0)))(void) sexos_pdx_call(SEXOS_SLOT_shell, SEXOS_INPUT_autodemo_tick, 0, 0, 0);
    sexos_yield();
  }
}
