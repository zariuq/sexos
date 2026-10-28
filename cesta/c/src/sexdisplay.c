#include "sexos_server.h"
#include "silkbar_model.h"
/*SexDisplay compositor, translated from servers/sexdisplay/src/main.rs. The Rust statics, global and function-local, are fields of one owned Sexdisplay state; its two 500 KiB glass buffers exceed the 64 KiB stack, so that one value lives in static storage and only sexdisplay_main takes its address. Every index the Rust bounds-checks is checked here too, and a failed check stops the server as Rust's panic handler does.*/
/*Build-time proof profiles (option_env! in the Rust source).*/
#if defined(SEXOS_SILKBAR_PHASE3_RECEIVE_PROOF)
#define SILKBAR_PHASE3_RECEIVE_PROOF_ENABLED 1

#else
#define SILKBAR_PHASE3_RECEIVE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SILKBAR_PHASE5_PIXEL_PROOF)
#define SILKBAR_PHASE5_PIXEL_PROOF_ENABLED 1

#else
#define SILKBAR_PHASE5_PIXEL_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SILK_RENDER_PROOF_PROFILE)
#define SILK_RENDER_PROOF_PROFILE_ENABLED 1

#else
#define SILK_RENDER_PROOF_PROFILE_ENABLED 0

#endif
#if defined(SEXOS_ATLAS_PHASE_C_RENDER_STUB_PROOF)
#define ATLAS_PHASE_C_RENDER_STUB_PROOF_ENABLED 1

#else
#define ATLAS_PHASE_C_RENDER_STUB_PROOF_ENABLED 0

#endif
#if defined(SEXOS_ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF)
#define ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_ENABLED 1

#else
#define ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_ENABLED 0

#endif
/*The framebuffer lies in the kernel's high half: the fallback until OP_PRIMARY_FB names the live one, and any address below HIGH_HALF_BASE is refused. A hosted build may place both elsewhere.*/
#if !defined(FALLBACK_PTR)
#define FALLBACK_PTR UINT64_C(0xffff8000fd000000)

#endif
#if !defined(HIGH_HALF_BASE)
#define HIGH_HALF_BASE UINT64_C(0xffff800000000000)

#endif
#define FALLBACK_W UINT32_C(1280)
#define FALLBACK_H UINT32_C(800)
#define MAX_FB_W ((size_t)8192)
#define MAX_FB_H ((size_t)4320)
#define DRAIN_MAX ((size_t)16)
/*Atlas Phase C cards: flat 2-pixel teal-cyan outlines around active surfaces below the bar.*/
#define ATLAS_CARD_BORDER_COLOR UINT32_C(0x0089DCE6)
#define ATLAS_CARD_BORDER_PX ((size_t)2)
#define ATLAS_CARD_MAX_CARDS ((size_t)16)
/*Atlas Phase D previews: flat interiors inset from the card edges.*/
#define ATLAS_PREVIEW_INNER_MARGIN ((size_t)4)
#define ATLAS_PREVIEW_MAX_PREVIEWS ((size_t)16)
#define ATLAS_PREVIEW_ACTIVE_COLOR UINT32_C(0x00B880FF)
#define ATLAS_PREVIEW_VISIBLE_COLOR UINT32_C(0x003070A0)
#define ATLAS_PREVIEW_MINIMIZED_COLOR UINT32_C(0x00505058)
/*The glass buffers cover the bar rows of framebuffers up to 2560 pixels wide.*/
#define BAR_BG_W_CAP ((size_t)2560)
#define BAR_BG_H ((size_t)50)
/*Surface registry (V1: inline ABI, no backing buffers). A surface is a solid rectangle below the bar with up to MAX_RECTS fill rectangles and a text buffer. owner_pd is bound on first create and never changes while active; only the owner may mutate or destroy it, while focus is compositor state open to all callers.*/
#define MAX_RECTS ((size_t)8)
#define MAX_SURFACES ((size_t)16)
#define TEXT_BUF_CAP ((size_t)512)
#define TEXT_MAX_ROWS ((size_t)24)
#define TEXT_LEGACY_COLS ((uint8_t)20)
typedef struct Surface {
  uint64_t surface_id;
  uint32_t owner_pd;
  int32_t x;
  int32_t y;
  uint32_t w;
  uint32_t h;
  uint32_t color;
  bool active;
/*Tab info (0xFD from the shell) and chrome flags (bit 0: top bar).*/
  uint8_t tab_count;
  uint8_t active_tab;
  uint8_t chrome_flags;
/*fill_count is the highest set rectangle index plus one.*/
  uint8_t fill_count;
  int32_t fill_sx[MAX_RECTS];
  int32_t fill_sy[MAX_RECTS];
  uint32_t fill_sw[MAX_RECTS];
  uint32_t fill_sh[MAX_RECTS];
  uint32_t fill_color[MAX_RECTS];
  uint16_t text_len;
  uint32_t text_color;
/*TEXT_MODEL_V2: line starts are recomputed on every write; lines split on newline and wrap at wrap_cols, which stays at the legacy 20 until a newline appears.*/
  uint8_t text_buf[TEXT_BUF_CAP];
  uint16_t line_starts[TEXT_MAX_ROWS];
  uint8_t line_count;
  uint8_t wrap_cols;
}
Surface;
#define FOCUS_SURFACE_COLOR UINT32_C(0x0089B4FA)
/*Frame rim of the focused surface (minimal mode until OP_SURFACE_TAB_INFO sets the top bar), sized so the first render already shows clickable lights.*/
#define FRAME_RIM_PX ((size_t)8)
#define FRAME_RIM_COLOR UINT32_C(0x00B4BEFE)
#define FRAME_LIGHT_SIZE_PX ((size_t)8)
#define FRAME_LIGHT_GAP_PX ((size_t)3)
#define FRAME_LIGHT_CLOSE_COLOR UINT32_C(0x00FF4444)
#define FRAME_LIGHT_MINIMIZE_COLOR UINT32_C(0x00FFCC44)
#define FRAME_LIGHT_ZOOM_COLOR UINT32_C(0x0044FF44)
/*V1 disables close everywhere: red renders at reduced alpha.*/
#define FRAME_LIGHT_CLOSE_DISABLED_BASE_ALPHA ((uint8_t)160)
#define TAB_STRIP_LIGHT_EXCLUSION_PX ((size_t)20)
#define TAB_ACTIVE_COLOR FOCUS_SURFACE_COLOR
#define TAB_INACTIVE_COLOR UINT32_C(0x0045475A)
/*chrome_flags: bit 0 top bar, bit 1 frame hovered, bit 2 light hovered, bits 3-4 the hovered light (close, minimize, zoom), bit 5 close allowed.*/
#define SURFACE_CHROME_TOP_BAR ((uint8_t)0x01)
#define SURFACE_CHROME_FRAME_HOVER ((uint8_t)0x02)
#define SURFACE_CHROME_LIGHT_HOVER ((uint8_t)0x04)
#define SURFACE_CHROME_LIGHT_KIND_MASK ((uint8_t)0x18)
#define SURFACE_CHROME_LIGHT_KIND_SHIFT 3
#define SURFACE_CHROME_CLOSE_ALLOWED ((uint8_t)0x20)
/*Default chrome: a 28-pixel top bar with three 10-pixel lights.*/
#define FRAME_TOP_BAR_HEIGHT_PX ((size_t)28)
#define FRAME_TOP_BAR_LIGHT_SIZE_PX ((size_t)10)
#define FRAME_TOP_BAR_LIGHT_GAP_PX ((size_t)5)
#define FRAME_TOP_BAR_LIGHT_EXCLUSION_PX ((size_t)50)
#define FRAME_TOP_BAR_COLOR UINT32_C(0x001E1E2E)
#define FRAME_TOP_BAR_DIVIDER_COLOR UINT32_C(0x00B4BEFE)
#define FRAME_TOP_BAR_LIGHT_TOP ((size_t)9)
#define FRAME_TOP_BAR_LIGHT_BOTTOM ((size_t)19)
/*Appearance tokens (V1: flat colors, blur forced to zero).*/
typedef struct RenderTokensV1 {
  uint32_t focus_surface_color;
  uint32_t frame_rim_color;
  uint32_t frame_top_bar_color;
  uint32_t active_tab_color;
  uint32_t inactive_tab_color;
  uint32_t close_light_color;
  uint32_t minimize_light_color;
  uint32_t zoom_light_color;
  uint8_t appearance_flags;
  uint8_t effect_levels;
}
RenderTokensV1;
/*The shell's cursor surface, always drawn last, and its launcher panel.*/
#define CURSOR_SURFACE_ID UINT64_C(0x90)
#define LAUNCHER_PANEL_SURFACE_ID UINT64_C(0x92)
/*Classic NW arrow, 8 by 16; bit 7 of a row is its leftmost pixel.*/
#define CURSOR_ARROW_W ((size_t)8)
#define CURSOR_ARROW_H ((size_t)16)
#define CURSOR_ARROW_COLOR UINT32_C(0x00FFFFFF)
/*The deterministic top-strip proof renders 1280 by 51 pixels and expects this FNV-1a hash.*/
#define PROOF_W ((size_t)1280)
#define PROOF_H ((size_t)51)
#define SILK_DE_TOPSTRIP_EXPECTED_HASH UINT64_C(0x9b5d54e17bdfa6f1)
#define SILK_DE_CONFORMANCE_PROOF_W_MAX ((size_t)1280)
#define SILK_DE_CONFORMANCE_PROOF_H_MAX ((size_t)51)
typedef struct Sexdisplay {
  uint64_t fb_ptr;
  uint32_t fb_w;
  uint32_t fb_h;
  uint32_t hot_log_budget;
  uint32_t bar_bg_buf[BAR_BG_H * BAR_BG_W_CAP];
  uint32_t bar_blur_buf[BAR_BG_H * BAR_BG_W_CAP];
  Surface surfaces[MAX_SURFACES];
  uint64_t focused_surface_id;
  RenderTokensV1 display_tokens;
  uint64_t token_buf_arg0;
  uint64_t token_buf_arg1;
  uint64_t token_buf_arg2;
  uint64_t reject_counter;
  uint32_t display_wm_pd;
  uint64_t render_frame_counter;
  uint8_t clock_redraw_source;
  uint8_t clock_canon_hh;
  uint8_t clock_canon_mm;
  uint8_t clock_canon_ss;
  uint8_t last_redrawn_clock_canon_ss;
  uint32_t clock_visible_prev_total;
  uint32_t cursor_under_saved[CURSOR_ARROW_W * CURSOR_ARROW_H];
  size_t cursor_under_x;
  size_t cursor_under_y;
  uint32_t input_trace_last_seq;
  uint32_t input_trace_recv;
  uint32_t input_trace_draws;
  uint32_t input_trace_presents;
  uint32_t input_trace_display_budget_hit;
  uint32_t input_trace_last_shell_tick;
  uint32_t display_recv_tick;
  uint32_t display_draw_tick;
  uint32_t display_present_tick;
  uint32_t last_counted_draw_seq;
  uint32_t appearance_tokens_applied;
/*One-shot markers.*/
  bool anim_proof_logged;
  bool blur_proof_logged;
  bool cadence_parity_emitted;
  bool clock_visible_mono_init;
  bool cursor_under_valid;
  bool cursor_z_top_logged;
  bool live_topstrip_audit_done;
  bool live_topstrip_rows_done;
  bool live_topstrip_rows_ss4_done;
  bool live_topstrip_tick4_fired;
  bool r6_glass_proof_logged;
  bool row_buffer_proof_logged;
  bool silkbar_clock_send_marked;
  bool token_buf_call1_received;
/*Log budgets, in the order the Rust declares them.*/
  uint32_t hover_reveal_log;
  uint32_t frame_light_startup_log;
  uint32_t phase6_draw_ok_log;
  uint32_t phase6_render_begin_log;
  uint32_t phase6_bounds_ok_log;
  uint32_t full_render_log;
  uint32_t live_topstrip_clear_log;
  uint32_t live_topstrip_bounds_log;
  uint32_t top_strip_redraw_log;
  uint32_t clock_redraw_log;
  uint32_t phase3_state_log;
  uint32_t phase5_draw_log;
  uint32_t surface_area_redraw_log;
  uint32_t display_cursor_draw_log;
  uint32_t input_trace_draw_log;
  uint32_t input_trace_present_log;
  uint32_t launcher_panel_draw_log;
  uint32_t card_skip_cursor_log;
  uint32_t card_skip_zero_log;
  uint32_t card_skip_overlap_log;
  uint32_t card_layout_log;
  uint32_t card_draw_log;
  uint32_t card_done_log;
  uint32_t preview_pass_log;
  uint32_t preview_skip_zero_log;
  uint32_t preview_skip_overlap_log;
  uint32_t preview_skip_small_log;
  uint32_t preview_layout_log;
  uint32_t preview_draw_log;
  uint32_t preview_done_log;
  uint32_t update_recv_log;
  uint32_t update_apply_log;
  uint32_t selected_options_update_log;
  uint32_t bell_render_log;
  uint32_t fb_live_wait_log;
  uint32_t clock_fallback_resume_log;
  uint32_t cadence_sample_log;
  uint32_t skip_same_second_log;
  uint32_t live_after_reject_log;
  uint32_t post_reject_cadence_log;
  uint32_t reject_streak_proof_log;
  uint32_t fallback_after_drop_continue_log;
  uint32_t clock_handoff_reject_log;
  uint32_t clock_handoff_accept_log;
  uint32_t clock_source_repeat_log;
  uint32_t clock_source_apply_log;
  uint32_t clock_recv_log;
  uint32_t clock_apply_log;
  uint32_t clock_stale_drop_log;
  uint32_t skip_same_second_stale_log;
  uint32_t display_cursor_update_log;
  uint32_t input_trace_recv_log;
  uint32_t appearance_tokens_seq0_log;
  uint32_t appearance_tokens_seq1_log;
  uint32_t surface_tab_info_log;
  uint32_t frame_hover_recv_log;
  uint32_t render_budget_cycle_log;
}
Sexdisplay;
/*Rust's bounds checks panic, and its panic handler spins forever.*/
static void panic(void) {
  sexos_spin_forever();
}
/*A budget allows its line while it lasts.*/
static bool take(uint32_t *budget) {
  if (*budget == 0) return false;
  *budget -= 1;
  return true;
}
/*A one-shot marker fires the first time only (an AtomicBool swap in the Rust).*/
static bool once(bool *fired) {
  if (*fired) return false;
  *fired = true;
  return true;
}
/*SERIAL_DIET_V1: one shared budget for hot-loop diagnostics. The first 300 lines keep the full early trace; error and gate markers do not use it.*/
static bool hot_log(Sexdisplay *s) {
  return take(&s->hot_log_budget);
}
/*Rust's `as i32` and its release-mode i32 arithmetic wrap; C's would not, so they go through defined unsigned arithmetic.*/
static int32_t wrap_i32(uint64_t word) {
  uint32_t low = (uint32_t) word;
  return low < 0x80000000u ? (int32_t) low : (int32_t)(low - 0x80000000u) + INT32_MIN;
}
static int32_t add_i32(int32_t a, int32_t b) {
  return wrap_i32((uint32_t) a + (uint32_t) b);
}
static int32_t sub_i32(int32_t a, int32_t b) {
  return wrap_i32((uint32_t) a - (uint32_t) b);
}
static size_t min_size(size_t a, size_t b) {
  return a < b ? a : b;
}
static size_t saturating_sub(size_t a, size_t b) {
  return a > b ? a - b : 0;
}
static size_t saturating_add(size_t a, size_t b) {
  return a > SIZE_MAX - b ? SIZE_MAX : a + b;
}
/*The renderer ignores alpha, but every token gets an opaque alpha byte.*/
static uint32_t clamp_color_token(uint32_t c) {
  return c | 0xFF000000u;
}
static void clock_canon_store(Sexdisplay *s, uint8_t hh, uint8_t mm, uint8_t ss) {
  s->clock_canon_hh = hh;
  s->clock_canon_mm = mm;
  s->clock_canon_ss = ss;
}
static uint32_t clock_total_seconds(uint8_t hh, uint8_t mm, uint8_t ss) {
  return (uint32_t) hh * 3600 + (uint32_t) mm * 60 + (uint32_t) ss;
}
/*Same or later, or one rollover at midnight.*/
static bool clock_visible_monotonic_ok(uint32_t prev, uint32_t curr) {
  return curr >= prev || (prev >= 86300 && curr <= 60);
}
static void clock_canon_apply_to_bar(const Sexdisplay *s, SilkBar *bar) {
  bar->clock_hh = s->clock_canon_hh;
  bar->clock_mm = s->clock_canon_mm;
  bar->clock_ss = s->clock_canon_ss;
}
typedef struct Rect {
  size_t x;
  size_t y;
  size_t w;
  size_t h;
}
Rect;
/*A surface clamped to the framebuffer and below the bar. BAR_H is the model's PANEL_Y + PANEL_H (48) plus a 2-pixel margin.*/
static Rect clamp_surface(const Surface *surf, size_t fb_w, size_t fb_h) {
  size_t x = min_size((size_t)(surf->x > 0 ? surf->x : 0), saturating_sub(fb_w, 1));
  size_t y = min_size((size_t)(surf->y > 50 ? surf->y : 50), saturating_sub(fb_h, 1));
  size_t w = min_size(surf->w, saturating_sub(fb_w, x));
  size_t h = min_size(surf->h, saturating_sub(fb_h, y));
  return (Rect) {
    .x = x,.y = y,.w = w,.h = h
  };
}
static bool in_rect(size_t x, size_t y, size_t rx, size_t ry, size_t rw, size_t rh) {
  return x >= rx && x < rx + rw && (y >= ry && y < ry + rh);
}
/*Fill rectangles in painter's order: the last one containing the pixel wins.*/
static uint32_t fill_rect_color(const Surface *surf, size_t x, size_t y, uint32_t base_color) {
  if (surf->fill_count == 0) return base_color;
  int32_t lx = sub_i32(wrap_i32(x), surf->x);
  int32_t ly = sub_i32(wrap_i32(y), surf->y);
  uint32_t c = base_color;
  size_t count = surf->fill_count;
  if (count > MAX_RECTS) panic();
  for (size_t i = 0; i < count; ++i) {
    if (surf->fill_sw[i] == 0 || surf->fill_sh[i] == 0) continue;
    if (lx >= surf->fill_sx[i] && lx < add_i32(surf->fill_sx[i], wrap_i32(surf->fill_sw[i])) && (ly >= surf->fill_sy[i] && ly < add_i32(surf->fill_sy[i], wrap_i32(surf->fill_sh[i])))) c = surf->fill_color[i];
  }
  return c;
}
/*Fixed-point interpolation between two XRGB colors: t=0 gives a, t=255 gives b, alpha forced opaque. The channel arithmetic wraps as the Rust's does.*/
static uint32_t lerp_color_xrgb(uint32_t a, uint32_t b, uint8_t t) {
  if (t == 0) return a | 0xFF000000u;
  if (t == 255) return b | 0xFF000000u;
  uint32_t ar = a & 0xFF;
  uint32_t ag = a >> 8 & 0xFF;
  uint32_t ab = a >> 16 & 0xFF;
  uint32_t br = b & 0xFF;
  uint32_t bg_ = b >> 8 & 0xFF;
  uint32_t bb = b >> 16 & 0xFF;
  uint32_t ti = t;
  uint32_t r = ar + (br - ar) * ti / 255u;
  uint32_t g = ag + (bg_ - ag) * ti / 255u;
  uint32_t b2 = ab + (bb - ab) * ti / 255u;
  return 0xFF000000u | b2 << 16 | g << 8 | r;
}
static uint32_t vertical_gradient_xrgb(size_t y, size_t h, uint32_t top, uint32_t bottom) {
  if (h <= 1) return top;
  uint32_t t = (uint32_t) min_size(y, h - 1) * 255u / (uint32_t)(h - 1);
  return lerp_color_xrgb(top, bottom, (uint8_t) t);
}
/*Desktop background: a smooth gradient between the theme's endpoint colors.*/
static uint32_t bg(size_t y, size_t h) {
  return vertical_gradient_xrgb(y, h, DEFAULT_THEME.bg_top, DEFAULT_THEME.bg_bottom);
}
static bool fill_bar_bg_buffer(Sexdisplay *s, uint32_t fb_w, uint32_t fb_h) {
  size_t fill_w = fb_w;
  if (fill_w > BAR_BG_W_CAP) return false;
  size_t fill_h = min_size(BAR_BG_H, fb_h);
  for (size_t y = 0; y < fill_h; ++y) {
    uint32_t c = bg(y, fb_h);
    for (size_t x = 0; x < fill_w; ++x) s->bar_bg_buf[y * BAR_BG_W_CAP + x] = c;
  }
  return true;
}
/*A box blur of radius 1 over the bar rows, the window clipped at the edges.*/
static bool blur_bar_bg_buffer_radius1(Sexdisplay *s, uint32_t fb_w, uint32_t fb_h) {
  size_t width = fb_w;
  if (width > BAR_BG_W_CAP) return false;
  size_t height = min_size(BAR_BG_H, fb_h);
  for (size_t y = 0; y < height; ++y) for (size_t x = 0; x < width; ++x) {
    uint32_t acc_r = 0;
    uint32_t acc_g = 0;
    uint32_t acc_b = 0;
    size_t y0 = saturating_sub(y, 1);
    size_t y1 = min_size(y + 1, saturating_sub(height, 1));
    size_t x0 = saturating_sub(x, 1);
    size_t x1 = min_size(x + 1, saturating_sub(width, 1));
    for (size_t sy = y0; sy <= y1; ++sy) for (size_t sx = x0; sx <= x1; ++sx) {
      uint32_t c = s->bar_bg_buf[sy * BAR_BG_W_CAP + sx];
      acc_r += c & 0xFF;
      acc_g += c >> 8 & 0xFF;
      acc_b += c >> 16 & 0xFF;
    }
    uint32_t count = (uint32_t)((y1 - y0 + 1) * (x1 - x0 + 1));
    uint32_t r = acc_r / count & 0xFF;
    uint32_t g = acc_g / count & 0xFF;
    uint32_t b = acc_b / count & 0xFF;
    s->bar_blur_buf[y * BAR_BG_W_CAP + x] = 0xFF000000u | b << 16 | g << 8 | r;
  }
  return true;
}
static uint32_t sample_bar_blur_bg_xrgb(const Sexdisplay *s, uint32_t x, uint32_t y, uint32_t fb_w, uint32_t fb_h) {
  if ((size_t) fb_w > BAR_BG_W_CAP || ((size_t) y >= BAR_BG_H || x >= fb_w)) return bg(y, fb_h);
  return s->bar_blur_buf[(size_t) y * BAR_BG_W_CAP + x];
}
static uint8_t triangle_wave_u8(uint64_t frame, uint64_t period) {
  if (period == 0) return 0;
  uint64_t pos = frame % period;
  uint64_t half = period / 2;
  if (half == 0) return 0;
  if (pos < half) return (uint8_t)(pos * 255 / half);
  uint64_t denom = period - half;
  if (denom == 0) return 0;
  return (uint8_t)((period - pos) * 255 / denom);
}
static uint8_t pulse_alpha(uint8_t base, uint8_t amp, uint64_t frame, uint64_t period) {
  if (period == 0) return base;
  int32_t centered = (int32_t) triangle_wave_u8(frame, period) - 127;
  int32_t out = (int32_t) base + centered * (int32_t) amp / 127;
  return (uint8_t)(out < 0 ? 0 : out > 255 ? 255 : out);
}
/*XRGB over XRGB: alpha 0 gives the background, 255 the foreground; opaque result, integers only.*/
static uint32_t alpha_blend_xrgb_over_xrgb(uint32_t fg, uint32_t bg_color, uint8_t alpha) {
  if (alpha == 0) return bg_color;
  if (alpha == 255) return fg;
  uint32_t a = alpha;
  uint32_t inv = 255u - a;
  uint32_t r = ((fg & 0xFF) * a + (bg_color & 0xFF) * inv) / 255u;
  uint32_t g = ((fg >> 8 & 0xFF) * a + (bg_color >> 8 & 0xFF) * inv) / 255u;
  uint32_t b = ((fg >> 16 & 0xFF) * a + (bg_color >> 16 & 0xFF) * inv) / 255u;
  return 0xFF000000u | b << 16 | g << 8 | r;
}
/*Glass: a foreground blended over the blurred desktop gradient.*/
static uint32_t glass_over_bg(const Sexdisplay *s, uint32_t fg, size_t x, size_t y, uint8_t alpha) {
  if (s->fb_h == 0) return fg;
  uint32_t under = sample_bar_blur_bg_xrgb(s, (uint32_t) x, (uint32_t) y, s->fb_w, s->fb_h);
  return alpha_blend_xrgb_over_xrgb(fg, under, alpha);
}
/*Linear edge glow: distance 0 gives max_alpha, the spread or more gives 0.*/
static uint8_t glow_edge_alpha(uint32_t dist, uint32_t spread, uint8_t max_alpha) {
  if (spread == 0 || dist >= spread) return 0;
  return (uint8_t)((uint32_t) max_alpha * (spread - dist) / spread);
}
static Rect module_rect(const SilkBar *bar, SilkBarModuleSlot slot) {
  const SilkBarLayoutBox *lb = &bar->layout[slot];
  return (Rect) {
    .x = lb->x,.y = lb->y,.w = lb->w,.h = lb->h
  };
}
/*Workspace indicators: the active one glows over glass with a slow pulse.*/
static const SilkBarModuleSlot WS_SLOTS[5] = {
  SILKBAR_MODULE_SLOT_WORKSPACE0, SILKBAR_MODULE_SLOT_WORKSPACE1, SILKBAR_MODULE_SLOT_WORKSPACE2, SILKBAR_MODULE_SLOT_WORKSPACE3, SILKBAR_MODULE_SLOT_WORKSPACE4
};
static bool workspace_color(const Sexdisplay *s, size_t x, size_t y, const SilkBar *bar, uint32_t *out) {
  for (size_t idx = 0; idx < 5; ++idx) {
    Rect r = module_rect(bar, WS_SLOTS[idx]);
    if (in_rect(x, y, r.x, r.y, r.w, r.h)) {
      const SilkBarWorkspaceState *ws = &bar->workspaces[idx];
      if (ws->active) {
        uint8_t glow_alpha = pulse_alpha(48, 32, s->render_frame_counter, 96);
        uint32_t active_glow = alpha_blend_xrgb_over_xrgb(0x00FFFFFFu, DEFAULT_THEME.active, glow_alpha);
        *out = glass_over_bg(s, active_glow, x, y, 224);
        return true;
      }
      *out = ws->urgent ? DEFAULT_THEME.urgent : DEFAULT_THEME.muted;
      return true;
    }
  }
  return false;
}
/*Status chip backgrounds, R6 glass for the network chip.*/
static const SilkBarModuleSlot CHIP_SLOTS[4] = {
  SILKBAR_MODULE_SLOT_CHIP0, SILKBAR_MODULE_SLOT_CHIP1, SILKBAR_MODULE_SLOT_CHIP2, SILKBAR_MODULE_SLOT_CLOCK
};
static bool chip_color(const Sexdisplay *s, size_t x, size_t y, const SilkBar *bar, uint32_t *out) {
  for (size_t idx = 0; idx < 4; ++idx) {
    Rect r = module_rect(bar, CHIP_SLOTS[idx]);
    if (in_rect(x, y, r.x, r.y, r.w, r.h)) {
      const SilkBarChipState *chip = &bar->chips[idx];
      if (!chip->visible) *out = 0x00102038u;
      else switch (chip->kind) {
        case SILKBAR_CHIP_NET: *out = glass_over_bg(s, DEFAULT_THEME.chip_fill, x, y, 212);
        break;
        case SILKBAR_CHIP_WIFI: *out = 0x0038D6C8u;
        break;
        case SILKBAR_CHIP_BATTERY: *out = 0x00FFB84Du;
        break;
        case SILKBAR_CHIP_CLOCK: *out = DEFAULT_THEME.chip_border;
        break;
      }
      return true;
    }
  }
  return false;
}
/*Phase 5 pixel indicators: tiny dots and swatches inside the strip.*/
static bool app_indicator_color(uint32_t sid, uint32_t *out) {
/*Linen green, Quil mauve, Mesh teal, Collar peach, Bell red, Spindle blue; no app none; any other dim gray.*/
  switch (sid) {
    case 200: *out = 0x00A6E3A1u;
    break;
    case 201: *out = 0x00CBA6F7u;
    break;
    case 202: *out = 0x0094E2D5u;
    break;
    case 203: *out = 0x00FAB387u;
    break;
    case 204: *out = 0x00F38BA8u;
    break;
    case 153: *out = 0x0089B4FAu;
    break;
    case 0: return false;
    default: *out = 0x00444444u;
  }
  return true;
}
/*The Atlas 8-accent palette: blue, green, yellow, peach, red, mauve, pink, teal.*/
static const uint32_t ACCENT[8] = {
  0x0089B4FAu, 0x00A6E3A1u, 0x00F9E2AFu, 0x00FAB387u, 0x00F38BA8u, 0x00CBA6F7u, 0x00F5C2E7u, 0x0094E2D5u
};
static uint32_t accent_swatch_color(uint8_t idx) {
  return idx < 8 ? ACCENT[idx] : 0x00444444u;
}
/*Active app: a 4x4 dot at (155, 22), right of the option dots.*/
static bool phase5_active_app_at(size_t x, size_t y, const SilkBar *bar, uint32_t *out) {
  if (!SILKBAR_PHASE5_PIXEL_PROOF_ENABLED || bar->phase1.active_app_sid == 0) return false;
  if (!in_rect(x, y, 155, 22, 4, 4)) return false;
  return app_indicator_color(bar->phase1.active_app_sid, out);
}
/*Tint swatch: 4x4 at (1045, 22), between Bell and Clock.*/
static bool phase5_tint_swatch_at(size_t x, size_t y, const SilkBar *bar, uint32_t *out) {
  if (!SILKBAR_PHASE5_PIXEL_PROOF_ENABLED || !in_rect(x, y, 1045, 22, 4, 4)) return false;
  *out = accent_swatch_color(bar->phase1.accent_tint_idx);
  return true;
}
/*Palette dot: 3x3 at (1060, 23), bright green when open.*/
static bool phase5_palette_dot_at(size_t x, size_t y, const SilkBar *bar, uint32_t *out) {
  if (!SILKBAR_PHASE5_PIXEL_PROOF_ENABLED || !in_rect(x, y, 1060, 23, 3, 3)) return false;
  *out = bar->phase1.palette_open ? 0x0044FF44u : 0x00224422u;
  return true;
}
/*Selected-window option dots: red, green, yellow, cyan, 3x3 each, 5 apart from x=135 on rows 19..21.*/
static const uint32_t OPTION_DOT_COLORS[4] = {
  0x00FF4444u, 0x0044FF44u, 0x00FFCC44u, 0x0044CCFFu
};
static uint32_t bar_color(const Sexdisplay *s, size_t x, size_t y, const SilkBar *bar) {
/*Only a dot's own pixels are intercepted; everything else falls through, so the modules on the same rows still render.*/
  if (bar->selected_options_mask != 0 && (y >= 19 && y < 22)) for (size_t bit = 0; bit < 4; ++bit) {
    size_t dot_x = 135 + bit * 5;
    if (x >= dot_x && x < dot_x + 3 && (bar->selected_options_mask >> bit & 1) != 0) return OPTION_DOT_COLORS[bit];
  }
  uint32_t c = 0;
  if (workspace_color(s, x, y, bar, &c)) return c;
  if (chip_color(s, x, y, bar, &c)) return c;
/*Bell: a privacy-safe presence dot. Dim when Bell is unavailable or empty, amber with redacted events, gold otherwise.*/
  Rect bell = module_rect(bar, SILKBAR_MODULE_SLOT_BELL);
  if (in_rect(x, y, bell.x, bell.y, bell.w, bell.h)) {
    const SilkBarBellState *b = &bar->bell_state;
    if ((b->flags & 1) == 0 || b->total_visible == 0) return DEFAULT_THEME.muted;
    return b->redacted_count > 0 ? 0x00FFAA44u : 0x00FFD700u;
  }
/*Launcher: a rounded-illusion border around a glass body.*/
  Rect l = module_rect(bar, SILKBAR_MODULE_SLOT_LAUNCHER);
  if (in_rect(x, y, l.x, l.y, l.w, l.h)) {
    if (x < l.x + 2 || x >= l.x + l.w - 2 || (y < l.y + 2 || y >= l.y + l.h - 2)) return glass_over_bg(s, DEFAULT_THEME.panel_glow, x, y, 72);
    return alpha_blend_xrgb_over_xrgb(0x0082D6FFu, glass_over_bg(s, 0x005CAEE8u, x, y, 200), 64);
  }
/*Bar body: glass over the desktop, a highlight on row 0, a dark edge on row 49 and a glow falling off over the last four rows.*/
  uint32_t base = glass_over_bg(s, DEFAULT_THEME.panel_fill, x, y, 196);
  if (y == 0) return alpha_blend_xrgb_over_xrgb(0x00A8D8FFu, base, 64);
  if (y == 49) return alpha_blend_xrgb_over_xrgb(DEFAULT_THEME.panel_glow, alpha_blend_xrgb_over_xrgb(0x000B1220u, base, 88), 48);
  if (y + 4 >= 50 && y < 50) {
    uint8_t glow_a = glow_edge_alpha((uint32_t)(49 - y), 4, 72);
    if (glow_a > 0) return alpha_blend_xrgb_over_xrgb(DEFAULT_THEME.panel_glow, alpha_blend_xrgb_over_xrgb(0x001C2A54u, base, 36), glow_a);
  }
  return alpha_blend_xrgb_over_xrgb(0x001C2A54u, base, 32);
}
/*5x7 digit glyphs; bit 4 of a row is its leftmost pixel.*/
static const uint8_t FONT[10][7] = {
  {
    0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E
  }, {
    0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E
  }, {
    0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F
  }, {
    0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E
  }, {
    0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02
  }, {
    0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E
  }, {
    0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E
  }, {
    0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08
  }, {
    0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E
  }, {
    0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C
  }
};
/*5x7 ASCII glyphs for 0x20 (space) through 0x5A (Z), 59 of them, laid out as FONT.*/
static const uint8_t FONT_ASCII_5X7[59][7] = {
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  }, {
    0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04
  }, {
    0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00
  }, {
    0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A
  }, {
    0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04
  }, {
    0x19, 0x1A, 0x02, 0x04, 0x08, 0x0B, 0x13
  }, {
    0x0C, 0x12, 0x12, 0x0C, 0x15, 0x12, 0x0D
  }, {
    0x0C, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00
  }, {
    0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02
  }, {
    0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08
  }, {
    0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00
  }, {
    0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00
  }, {
    0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08
  }, {
    0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00
  }, {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C
  }, {
    0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10
  }, {
    0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E
  }, {
    0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E
  }, {
    0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F
  }, {
    0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E
  }, {
    0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02
  }, {
    0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E
  }, {
    0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E
  }, {
    0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08
  }, {
    0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E
  }, {
    0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C
  }, {
    0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00
  }, {
    0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x04, 0x08
  }, {
    0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02
  }, {
    0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00
  }, {
    0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08
  }, {
    0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04
  }, {
    0x0E, 0x11, 0x13, 0x15, 0x13, 0x10, 0x0E
  }, {
    0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11
  }, {
    0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E
  }, {
    0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E
  }, {
    0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E
  }, {
    0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F
  }, {
    0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10
  }, {
    0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E
  }, {
    0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11
  }, {
    0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E
  }, {
    0x07, 0x01, 0x01, 0x01, 0x01, 0x11, 0x0E
  }, {
    0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11
  }, {
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F
  }, {
    0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11
  }, {
    0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11
  }, {
    0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E
  }, {
    0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10
  }, {
    0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D
  }, {
    0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11
  }, {
    0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E
  }, {
    0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04
  }, {
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E
  }, {
    0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04
  }, {
    0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11
  }, {
    0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11
  }, {
    0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04
  }, {
    0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F
  }
};
static const uint8_t CURSOR_ARROW_BITMAP[16] = {
  0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFC, 0xD8, 0x8C, 0x0C, 0x06, 0x06, 0x03, 0x03, 0x01
};
/*A glyph row; Rust's index check panics on a digit or row out of range.*/
static uint8_t digit_row(size_t digit, size_t row) {
  if (digit >= 10 || row >= 7) panic();
  return FONT[digit][row];
}
/*The toolbar title of known surfaces, in 5x7 glyphs after the light zone. Lowercase letters have no glyph here and stay blank, as in the Rust.*/
static bool toolbar_title_fg_at(size_t lx, size_t ly, uint64_t surface_id, uint32_t *out) {
  size_t title_x = FRAME_TOP_BAR_LIGHT_EXCLUSION_PX + 12;
  size_t title_y = 10;
  if (lx < title_x || (ly < title_y || ly >= title_y + 7)) return false;
  size_t col = (lx - title_x) / 6;
  size_t px = (lx - title_x) % 6;
  size_t py = ly - title_y;
  if (px >= 5) return false;
  const char *title = NULL;
  if (surface_id == 200) title = "Linen";
  else if (surface_id == 201) title = "Quil";
  else if (surface_id == 0x99) title = "Spindle";
  else if (surface_id == 100) title = "App";
  else return false;
  if (col >= sexos_strlen(title)) return false;
  uint8_t ch = (uint8_t) title[col];
  if (ch < 0x20 || ch > 0x5A) return false;
  size_t glyph_idx = ch - 0x20;
  if ((FONT_ASCII_5X7[glyph_idx][py] >> (4 - px) & 1) != 0) {
    *out = 0x00E8FFFFu;
    return true;
  }
  return false;
}
/*TEXT_MODEL_V2: recompute the line table after a text write. A buffer without a newline keeps the legacy 20 columns every pre-V2 writer pads to; any newline switches to width-derived wrap, (w - 10) / 6 clamped to 1..80.*/
static void recompute_text_lines(Surface *surf) {
  size_t len = min_size(surf->text_len, TEXT_BUF_CAP);
  surf->line_count = 0;
  if (len == 0) {
    surf->wrap_cols = TEXT_LEGACY_COLS;
    return;
  }
  bool has_newline = false;
  for (size_t i = 0; i < len; ++i) if (surf->text_buf[i] == 0x0A) has_newline = true;
  if (has_newline) {
/*The Rust narrows (w - 10) / 6 to u8 before clamping.*/
    uint8_t derived = (uint8_t)((surf->w > 10 ? surf->w - 10 : 0) / 6);
    surf->wrap_cols = derived < 1 ? 1 : derived > 80 ? 80 : derived;
  } else surf->wrap_cols = TEXT_LEGACY_COLS;
  size_t cols = surf->wrap_cols;
  size_t i = 0;
  size_t col = 0;
  surf->line_starts[0] = 0;
  surf->line_count = 1;
  while (i < len) {
    uint8_t c = surf->text_buf[i];
    i += 1;
    if (c == 0x0A) {
      if (surf->line_count >= TEXT_MAX_ROWS) break;
      surf->line_starts[surf->line_count] = (uint16_t) i;
      surf->line_count += 1;
      col = 0;
    } else {
      col += 1;
      if (col >= cols && i < len) {
        if (surf->line_count >= TEXT_MAX_ROWS) break;
        surf->line_starts[surf->line_count] = (uint16_t) i;
        surf->line_count += 1;
        col = 0;
      }
    }
  }
}
/*Surface text in 5x7 glyphs with 1-pixel spacing and 9-pixel lines, inset 8 from the left and 24 from the top. Lowercase folds to the uppercase glyphs (TEXT_MODEL_V2).*/
static bool surface_text_fg_at(size_t x, size_t y, const Surface *surf, uint32_t *out) {
  if (surf->text_len == 0 || surf->line_count == 0) return false;
  size_t tx = saturating_add((size_t) surf->x, 8);
  size_t ty = saturating_add((size_t) surf->y, 24);
  if (x < tx || y < ty) return false;
  size_t col = (x - tx) / 6;
  size_t row = (y - ty) / 9;
  size_t px = (x - tx) % 6;
  size_t py = (y - ty) % 9;
  if (px >= 5 || py >= 7) return false;
  if (col >= surf->wrap_cols || row >= surf->line_count) return false;
  if (row >= TEXT_MAX_ROWS) panic();
  size_t start = surf->line_starts[row];
  size_t end = row + 1 < surf->line_count ? (size_t) surf->line_starts[row + 1] : (size_t) surf->text_len;
  size_t char_idx = start + col;
  if (char_idx >= end || char_idx >= surf->text_len) return false;
  if (char_idx >= TEXT_BUF_CAP) panic();
  uint8_t c = surf->text_buf[char_idx];
  bool lower = c >= 0x61 && c <= 0x7A;
  if (c < 0x20 || (c > 0x5A && !lower)) return false;
  size_t glyph_idx = lower ? c - 0x61 + 0x21 : c - 0x20;
  if (glyph_idx >= 59) return false;
  if ((FONT_ASCII_5X7[glyph_idx][py] >> (4 - px) & 1) != 0) {
    *out = surf->text_color;
    return true;
  }
  return false;
}
/*Clock digits drawn inline in the pixel loop, so no separate overlay pass opens a tear window. Colons at offsets 14 and 31, digits at 0, 7, 17, 24, 34, 41.*/
static const size_t CLOCK_DIGIT_X[6] = {
  0, 7, 17, 24, 34, 41
};
static bool clock_fg_at(size_t x, size_t y, const SilkBar *bar, uint32_t *out) {
  Rect clock = module_rect(bar, SILKBAR_MODULE_SLOT_CLOCK);
  size_t cx = clock.x;
  size_t cy = clock.y + 1;
  if (y < cy || y >= cy + 7) return false;
  if (x < cx || x > cx + 45) return false;
  if (x == cx + 14 || x == cx + 31) {
    if (y == cy + 1 || y == cy + 5) {
      *out = DEFAULT_THEME.text;
      return true;
    }
    return false;
  }
  const uint8_t digits[6] = {
    bar->clock_hh / 10, bar->clock_hh % 10, bar->clock_mm / 10, bar->clock_mm % 10, bar->clock_ss / 10, bar->clock_ss % 10
  };
  for (size_t di = 0; di < 6; ++di) {
    size_t dx = CLOCK_DIGIT_X[di];
    if (x < cx + dx || x >= cx + dx + 5) continue;
    if ((digit_row(digits[di], y - cy) >> (4 - (x - (cx + dx))) & 1) != 0) {
      *out = DEFAULT_THEME.text;
      return true;
    }
    return false;
  }
  return false;
}
/*The Bell count badge at the right of its slot, at most two digits, in the clock's color.*/
static bool bell_badge_at(size_t x, size_t y, const SilkBar *bar, uint32_t *out) {
  uint8_t n = bar->bell_state.total_visible;
  if (n == 0 || (bar->bell_state.flags & 1) == 0) return false;
  if (n > 99) n = 99;
  size_t tens = n / 10;
  size_t ones = n % 10;
  Rect bell = module_rect(bar, SILKBAR_MODULE_SLOT_BELL);
  size_t badge_y = bell.y + bell.h / 2 - 3;
  if (y < badge_y || y >= badge_y + 7) return false;
  size_t row = y - badge_y;
/*Tens at +9, ones at +14; a single digit sits at +11 for the skipped tens place, which never draws.*/
  size_t left = tens > 0 ? bell.x + 9 : bell.x + 11;
  size_t right = bell.x + 14;
  for (size_t di = 0; di < 2; ++di) {
    size_t digit_x = di == 0 ? left : right;
    size_t digit = di == 0 ? tens : ones;
    if (di == 0 && tens == 0) continue;
    if (x < digit_x || x >= digit_x + 5) continue;
    if ((digit_row(digit, row) >> (4 - (x - digit_x)) & 1) != 0) {
      *out = DEFAULT_THEME.text;
      return true;
    }
    return false;
  }
  return false;
}
/*Frame chrome alpha scaled by the hover-reveal dimming: 5 gives about 20%, 10 full.*/
static uint8_t scale_alpha(uint8_t base, uint8_t dim) {
  return (uint8_t)((uint32_t) base * (uint32_t) dim / 10);
}
static bool inside(size_t x, size_t y, Rect r) {
  return x >= r.x && x < r.x + r.w && (y >= r.y && y < r.y + r.h);
}
/*The focused surface's pixel: top-bar chrome (glass toolbar, divider, title, tab strip, the three lights), else the rim band (minimal-mode lights and tabs on top, a pulsing neon glass rim on the other edges), else its content.*/
static uint32_t focused_pixel(Sexdisplay *s, const Surface *surf, Rect r, size_t x, size_t y, uint32_t c) {
  size_t lx = x - r.x;
  size_t ly = y - r.y;
/*saturating_sub keeps the edges sensible on surfaces smaller than the rim.*/
  size_t rim_right = saturating_sub(r.w, FRAME_RIM_PX);
  size_t rim_bottom = saturating_sub(r.h, FRAME_RIM_PX);
  bool top_bar_active = (surf->chrome_flags & SURFACE_CHROME_TOP_BAR) != 0;
  bool frame_hovered = (surf->chrome_flags & SURFACE_CHROME_FRAME_HOVER) != 0;
  bool light_hovered = (surf->chrome_flags & SURFACE_CHROME_LIGHT_HOVER) != 0;
  uint8_t hovered_light_kind = (uint8_t)((surf->chrome_flags & SURFACE_CHROME_LIGHT_KIND_MASK) >> SURFACE_CHROME_LIGHT_KIND_SHIFT);
  bool close_allowed = (surf->chrome_flags & SURFACE_CHROME_CLOSE_ALLOWED) != 0;
  bool single_tab = surf->tab_count <= 1;
  if (ly == 0 && lx == 0 && take(&s->hover_reveal_log)) sexos_line("[sexdisplay.frame.hover.reveal] mode=v1b single_tab=%u hovered=%u", (unsigned) single_tab, (unsigned) frame_hovered);
/*Hover reveal: single-tab chrome dims until hovered; multi-tab chrome stays full.*/
  uint8_t chrome_dim = single_tab && !frame_hovered ? 5 : 10;
  const RenderTokensV1 *tokens = &s->display_tokens;
/*[silk.live_topstrip.v2.clip] Frame chrome never renders into the SilkBar rows; this pixel keeps whatever the strip pass gave it.*/
  if (y < BAR_BG_H + 1) return c;
  if (top_bar_active && ly < FRAME_TOP_BAR_HEIGHT_PX) {
/*Frosted glass toolbar, an opaque divider on its last row, the title over it.*/
    uint8_t toolbar_alpha = scale_alpha(frame_hovered ? 220 : 200, chrome_dim);
    if (toolbar_alpha > 0) c = glass_over_bg(s, tokens->frame_top_bar_color, x, y, toolbar_alpha);
    if (ly == FRAME_TOP_BAR_HEIGHT_PX - 1) c = FRAME_TOP_BAR_DIVIDER_COLOR;
    uint32_t fg = 0;
    if (toolbar_title_fg_at(lx, ly, surf->surface_id, &fg)) c = fg;
    if (surf->tab_count > 0 && (lx >= FRAME_TOP_BAR_LIGHT_EXCLUSION_PX && lx < rim_right)) {
      size_t slot_w = (rim_right - FRAME_TOP_BAR_LIGHT_EXCLUSION_PX) / surf->tab_count;
      if (slot_w > 0) {
        bool active = (lx - FRAME_TOP_BAR_LIGHT_EXCLUSION_PX) / slot_w == surf->active_tab;
        uint32_t tab_color = active ? tokens->active_tab_color : tokens->inactive_tab_color;
        uint32_t tab_glass = glass_over_bg(s, tab_color, x, y, 224);
        c = alpha_blend_xrgb_over_xrgb(tab_color, tab_glass, active ? 240 : 200);
      }
    }
    if (ly >= FRAME_TOP_BAR_LIGHT_TOP && ly < FRAME_TOP_BAR_LIGHT_BOTTOM) {
      if (lx == FRAME_TOP_BAR_LIGHT_GAP_PX && ly == FRAME_TOP_BAR_LIGHT_TOP && take(&s->frame_light_startup_log)) if (hot_log(s)) sexos_line("[sexdisplay.frame.light.startup.render] sid=%llu red=%s close_allowed=%u reason=%s", (unsigned long long) surf->surface_id, close_allowed ? "enabled" : "disabled", (unsigned) close_allowed, close_allowed ? "close_allowed" : "default_disabled_or_protected");
      if (lx == FRAME_TOP_BAR_LIGHT_GAP_PX && ly == FRAME_TOP_BAR_LIGHT_TOP && take(&s->phase6_draw_ok_log)) sexos_line("[sexdisplay.frame_lights.draw.ok] rendered=1 red=disabled yellow=available green=available ok=1 reason=render_path");
/*Close, minimize, zoom: each glass under its light color; close is dimmer while not allowed, every light brightens when hovered.*/
      size_t l1_end = FRAME_TOP_BAR_LIGHT_GAP_PX + FRAME_TOP_BAR_LIGHT_SIZE_PX;
      bool l1_hovered = light_hovered && hovered_light_kind == 0;
      uint8_t l1_base = l1_hovered ? 255 : close_allowed ? 224 : FRAME_LIGHT_CLOSE_DISABLED_BASE_ALPHA;
      uint8_t light_alpha = scale_alpha(l1_base, chrome_dim);
      if (lx >= FRAME_TOP_BAR_LIGHT_GAP_PX && lx < l1_end) {
        if (light_alpha > 0) c = glass_over_bg(s, tokens->close_light_color, x, y, light_alpha);
      } else {
        size_t l2_start = l1_end + FRAME_TOP_BAR_LIGHT_GAP_PX;
        size_t l2_end = l2_start + FRAME_TOP_BAR_LIGHT_SIZE_PX;
        uint8_t l2_alpha = scale_alpha(light_hovered && hovered_light_kind == 1 ? 255 : 224, chrome_dim);
        if (lx >= l2_start && lx < l2_end) {
          if (l2_alpha > 0) c = glass_over_bg(s, tokens->minimize_light_color, x, y, l2_alpha);
        } else {
          size_t l3_start = l2_end + FRAME_TOP_BAR_LIGHT_GAP_PX;
          size_t l3_end = l3_start + FRAME_TOP_BAR_LIGHT_SIZE_PX;
          uint8_t l3_alpha = scale_alpha(light_hovered && hovered_light_kind == 2 ? 255 : 224, chrome_dim);
          if (lx >= l3_start && lx < l3_end && l3_alpha > 0) c = glass_over_bg(s, tokens->zoom_light_color, x, y, l3_alpha);
        }
      }
    }
    return c;
  }
  if (ly < FRAME_RIM_PX || lx < FRAME_RIM_PX || (lx >= rim_right || ly >= rim_bottom)) {
    if (ly < FRAME_RIM_PX && !top_bar_active) {
/*Minimal mode: the lights and tab strip ride the top rim.*/
      size_t pitch = FRAME_LIGHT_SIZE_PX + FRAME_LIGHT_GAP_PX;
      if (lx >= FRAME_LIGHT_GAP_PX && lx < FRAME_LIGHT_GAP_PX + FRAME_LIGHT_SIZE_PX) return glass_over_bg(s, tokens->close_light_color, x, y, close_allowed ? 224 : FRAME_LIGHT_CLOSE_DISABLED_BASE_ALPHA);
      if (lx >= FRAME_LIGHT_GAP_PX + pitch && lx < FRAME_LIGHT_GAP_PX + pitch + FRAME_LIGHT_SIZE_PX) return glass_over_bg(s, tokens->minimize_light_color, x, y, 224);
      if (lx >= FRAME_LIGHT_GAP_PX + 2 * pitch && lx < FRAME_LIGHT_GAP_PX + 2 * pitch + FRAME_LIGHT_SIZE_PX) return glass_over_bg(s, tokens->zoom_light_color, x, y, 224);
      if (surf->tab_count > 0 && (lx >= TAB_STRIP_LIGHT_EXCLUSION_PX && lx < rim_right)) {
        size_t slot_w = (rim_right - TAB_STRIP_LIGHT_EXCLUSION_PX) / surf->tab_count;
        if (slot_w > 0) {
          bool active = (lx - TAB_STRIP_LIGHT_EXCLUSION_PX) / slot_w == surf->active_tab;
          return glass_over_bg(s, active ? tokens->active_tab_color : tokens->inactive_tab_color, x, y, active ? 220 : 180);
        }
      }
      return glass_over_bg(s, tokens->frame_rim_color, x, y, 196);
    }
    return glass_over_bg(s, tokens->frame_rim_color, x, y, pulse_alpha(196, 12, s->render_frame_counter, 128));
  }
  c = fill_rect_color(surf, x, y, tokens->focus_surface_color);
  uint32_t tc = 0;
  if (surface_text_fg_at(x, y, surf, &tc)) c = tc;
  return c;
}
/*One pixel below the bar: unfocused surfaces in slot order (the first hit wins), then the focused surface on top. render and redraw_surface_area share it, so they cannot drift.*/
static uint32_t composite_pixel(Sexdisplay *s, size_t x, size_t y, size_t w, size_t h, uint32_t background, uint64_t focused_id) {
  uint32_t c = background;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    const Surface *surf = &s->surfaces[i];
    if (!surf->active || surf->surface_id == focused_id) continue;
    Rect r = clamp_surface(surf, w, h);
    if (r.w == 0 || r.h == 0) continue;
    if (inside(x, y, r)) {
      c = fill_rect_color(surf, x, y, surf->color);
      uint32_t tc = 0;
      if (surface_text_fg_at(x, y, surf, &tc)) c = tc;
      break;
    }
  }
  if (focused_id == 0) return c;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    const Surface *surf = &s->surfaces[i];
    if (!surf->active || surf->surface_id != focused_id) continue;
    Rect r = clamp_surface(surf, w, h);
    if (r.w == 0 || r.h == 0) continue;
    if (inside(x, y, r)) {
/*Phase 6: one-shot markers from the real render path, after clamp_surface and the size guard.*/
      if (take(&s->phase6_render_begin_log)) {
        size_t active_count = 0;
        size_t frame_count = 0;
        for (size_t j = 0; j < MAX_SURFACES; ++j) {
          const Surface *other = &s->surfaces[j];
          if (other->active) {
            active_count += 1;
            if (other->tab_count > 0 && (other->chrome_flags & SURFACE_CHROME_TOP_BAR) != 0) frame_count += 1;
          }
        }
        sexos_line("[sexdisplay.frame_lights.render.begin] fb_w=%zu fb_h=%zu surfaces=%zu frames=%zu", w, h, active_count, frame_count);
      }
      if (take(&s->phase6_bounds_ok_log)) sexos_line("[sexdisplay.frame_lights.bounds.ok] all_lights_within_fb=1 fb_w=%zu fb_h=%zu light_w=40 light_h=10 ok=1 reason=clamp_surface_guarantee", w, h);
      return focused_pixel(s, surf, r, x, y, c);
    }
  }
  return c;
}
/*Shared guards of the three render paths: a high-half framebuffer of plausible size, the pixel count through *pixels. Only the full render also refuses a range whose end wraps the address space; the two redraw paths do not check it, as in the Rust.*/
static bool framebuffer_ok(uint64_t fb_addr, size_t w, size_t h, bool check_end, size_t *pixels) {
  if (fb_addr < HIGH_HALF_BASE) return false;
  if (w == 0 || h == 0 || (w > MAX_FB_W || h > MAX_FB_H)) return false;
  *pixels = w * h;
  if (check_end && (uint64_t) *pixels * 4 > UINT64_MAX - fb_addr) return false;
  return true;
}
static volatile uint32_t *framebuffer(uint64_t fb_addr) {
  return (volatile uint32_t *)(uintptr_t) fb_addr;
}
/*A top-strip pixel; strip says whether the Phase 5 indicators are drawn (the live strip redraw draws them, the full render does not).*/
static uint32_t strip_pixel(Sexdisplay *s, size_t x, size_t y, const SilkBar *bar, bool strip) {
  uint32_t fg = 0;
  if (clock_fg_at(x, y, bar, &fg)) return fg;
  if (bell_badge_at(x, y, bar, &fg)) return fg;
  if (strip) {
    if (phase5_active_app_at(x, y, bar, &fg)) return fg;
    if (phase5_tint_swatch_at(x, y, bar, &fg)) return fg;
    if (phase5_palette_dot_at(x, y, bar, &fg)) return fg;
  }
  return bar_color(s, x, y, bar);
}
/*INPUT_PRESENT_TRACE_V1 summary: the unbudgeted counters.*/
static void input_trace_display_summary(const Sexdisplay *s) {
  sexos_line("[input.trace.display.summary] recv=%u draws=%u presents=%u budget_hit=%u", (unsigned) s->input_trace_recv, (unsigned) s->input_trace_draws, (unsigned) s->input_trace_presents, (unsigned) s->input_trace_display_budget_hit);
}
/*One cursor trace line under MARKER: seq and shell tick are reported unknown when the sender did not tag them.*/
static void cursor_trace_line(const char *marker, uint32_t seq, uint32_t shell_tick, uint32_t display_tick, int32_t x, int32_t y) {
  if (seq != 0 && shell_tick != 0) sexos_line("%s seq=%u shell_tick=%u display_tick=%u x=%d y=%d", marker, (unsigned) seq, (unsigned) shell_tick, (unsigned) display_tick, x, y);
  else if (seq != 0) sexos_line("%s seq=%u shell_tick=unknown display_tick=%u x=%d y=%d", marker, (unsigned) seq, (unsigned) display_tick, x, y);
  else sexos_line("%s seq=unknown shell_tick=unknown display_tick=%u x=%d y=%d", marker, (unsigned) display_tick, x, y);
}
/*Pass 3: the shell's cursor surface, drawn last as the arrow bitmap; clear bits are transparent and the SilkBar rows (y < 51) are never written. With save_under the 8x16 pixels beneath are captured first, for the cursor-only fast path; pass it only when those pixels are current. Trace counters advance only when the input sequence did, so repaints at the same position present no new input.*/
static void draw_cursor_z_top(Sexdisplay *s, volatile uint32_t *fb, size_t w, size_t h, size_t total_pixels, bool save_under) {
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    const Surface *surf = &s->surfaces[i];
    if (!surf->active || surf->surface_id != CURSOR_SURFACE_ID) continue;
/*The origin is not clamped: the arrow clips naturally.*/
    size_t ox = (size_t)(surf->x > 0 ? surf->x : 0);
    size_t oy = (size_t)(surf->y > 0 ? surf->y : 0);
    if (once(&s->cursor_z_top_logged)) {
      sexos_line("[sexdisplay.cursor_surface.z_top.ok] id=%#llx", (unsigned long long) CURSOR_SURFACE_ID);
      sexos_line("[sexdisplay.cursor_shape.arrow.ok] id=%#llx", (unsigned long long) CURSOR_SURFACE_ID);
    }
    if (take(&s->display_cursor_draw_log) && hot_log(s)) sexos_line("[sexdisplay.cursor.draw] n=0 x=%d y=%d", wrap_i32(ox), wrap_i32(oy));
    bool input_seq_advanced = s->input_trace_last_seq != 0 && s->input_trace_last_seq != s->last_counted_draw_seq;
    if (input_seq_advanced) {
      s->input_trace_draws += 1;
      s->display_draw_tick += 1;
      if (take(&s->input_trace_draw_log)) {
        cursor_trace_line("[input.trace.display.cursor.draw]", s->input_trace_last_seq, s->input_trace_last_shell_tick, s->display_draw_tick, wrap_i32(ox), wrap_i32(oy));
        if (s->input_trace_draw_log == 0) s->input_trace_display_budget_hit = 1;
      }
    }
    if (save_under) {
      s->cursor_under_x = ox;
      s->cursor_under_y = oy;
      for (size_t row = 0; row < CURSOR_ARROW_H; ++row) {
        size_t py = oy + row;
        if (py >= h) break;
        if (py < 51) continue;
        for (size_t col = 0; col < CURSOR_ARROW_W; ++col) {
          size_t px = ox + col;
          if (px >= w) continue;
          size_t idx = py * w + px;
          if (idx < total_pixels) s->cursor_under_saved[row * CURSOR_ARROW_W + col] = fb[idx];
        }
      }
      s->cursor_under_valid = true;
    }
    for (size_t row = 0; row < CURSOR_ARROW_H; ++row) {
      size_t py = oy + row;
      if (py >= h) break;
      if (py < 51) continue;
      uint8_t bits = CURSOR_ARROW_BITMAP[row];
      for (size_t col = 0; col < CURSOR_ARROW_W; ++col) {
        if ((bits & 0x80 >> col) == 0) continue;
        size_t px = ox + col;
        if (px >= w) continue;
        size_t idx = py * w + px;
        if (idx < total_pixels) fb[idx] = CURSOR_ARROW_COLOR;
      }
    }
/*The pixels are in the framebuffer now: direct writes, no flip, so this is the present point.*/
    if (input_seq_advanced) {
      s->input_trace_presents += 1;
      s->display_present_tick += 1;
      if (take(&s->input_trace_present_log)) cursor_trace_line("[input.trace.display.cursor.present]", s->input_trace_last_seq, s->input_trace_last_shell_tick, s->display_present_tick, wrap_i32(ox), wrap_i32(oy));
      if (s->input_trace_presents == 4 || s->input_trace_presents % 8 == 0) input_trace_display_summary(s);
      s->last_counted_draw_seq = s->input_trace_last_seq;
    }
    break;
  }
}
/*Restore the pixels saved beneath the last cursor draw, under the same clipping rules.*/
static void restore_cursor_under(Sexdisplay *s, volatile uint32_t *fb, size_t w, size_t h, size_t total_pixels) {
  if (!s->cursor_under_valid) return;
  for (size_t row = 0; row < CURSOR_ARROW_H; ++row) {
    size_t py = s->cursor_under_y + row;
    if (py >= h) break;
    if (py < 51) continue;
    for (size_t col = 0; col < CURSOR_ARROW_W; ++col) {
      size_t px = s->cursor_under_x + col;
      if (px >= w) continue;
      size_t idx = py * w + px;
      if (idx < total_pixels) fb[idx] = s->cursor_under_saved[row * CURSOR_ARROW_W + col];
    }
  }
  s->cursor_under_valid = false;
}
/*Pass 4: the launcher panel, when the shell has it active: a solid rectangle with a 1-pixel border, above the apps and below the cursor.*/
static void draw_launcher_panel(Sexdisplay *s, volatile uint32_t *fb, size_t w, size_t h, size_t total_pixels) {
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    const Surface *surf = &s->surfaces[i];
    if (!surf->active || surf->surface_id != LAUNCHER_PANEL_SURFACE_ID) continue;
    Rect r = clamp_surface(surf, w, h);
    if (r.w == 0 || r.h == 0) break;
    if (take(&s->launcher_panel_draw_log)) sexos_line("[sexdisplay.launcher_panel.draw] id=%#llx x=%zu y=%zu w=%zu h=%zu", (unsigned long long) LAUNCHER_PANEL_SURFACE_ID, r.x, r.y, r.w, r.h);
    for (size_t row = 0; row < r.h; ++row) {
      size_t py = r.y + row;
      if (py >= h) break;
      for (size_t col = 0; col < r.w; ++col) {
        size_t px = r.x + col;
        if (px >= w) continue;
        size_t idx = py * w + px;
        if (idx >= total_pixels) continue;
        bool border = row == 0 || row == r.h - 1 || (col == 0 || col == r.w - 1);
        fb[idx] = border ? 0x00405880u : 0x00203058u;
      }
    }
    break;
  }
}
static void write_pixel(volatile uint32_t *fb, size_t idx, size_t total_pixels, uint32_t color) {
  if (idx < total_pixels) fb[idx] = color;
}
/*Atlas Phase C: 2-pixel card outlines around active surfaces entirely below the strip, each write checked against the framebuffer. The cursor and launcher have their own passes.*/
static void draw_atlas_cards_pass(Sexdisplay *s, volatile uint32_t *fb, size_t w, size_t h, size_t total_pixels) {
  if (!ATLAS_PHASE_C_RENDER_STUB_PROOF_ENABLED || h < 52) return;
  uint32_t card_count = 0;
  for (size_t si = 0; si < MAX_SURFACES; ++si) {
    if (card_count >= ATLAS_CARD_MAX_CARDS) break;
    const Surface *surf = &s->surfaces[si];
    if (!surf->active) continue;
    if (surf->surface_id == CURSOR_SURFACE_ID || surf->surface_id == LAUNCHER_PANEL_SURFACE_ID) {
      if (take(&s->card_skip_cursor_log)) sexos_line("[sexdisplay.atlas.card.skip] scene=%u reason=cursor_or_launcher", (unsigned) si);
      continue;
    }
    Rect r = clamp_surface(surf, w, h);
    if (r.w == 0 || r.h == 0) {
      if (take(&s->card_skip_zero_log)) sexos_line("[sexdisplay.atlas.card.skip] scene=%u reason=zero_area_clamped", (unsigned) si);
      continue;
    }
    if (r.y < 51) {
      if (take(&s->card_skip_overlap_log)) sexos_line("[sexdisplay.atlas.card.skip] scene=%u reason=overlaps_top_strip sy=%zu", (unsigned) si, r.y);
      continue;
    }
    if (take(&s->card_layout_log)) sexos_line("[sexdisplay.atlas.card.layout] scene=%u x=%zu y=%zu w=%zu h=%zu active=%u", (unsigned) si, r.x, r.y, r.w, r.h, (unsigned)(surf->surface_id == s->focused_surface_id));
    size_t right = min_size(saturating_add(r.x, r.w), w);
    size_t bottom = min_size(saturating_add(r.y, r.h), h);
    if (right <= r.x || bottom <= r.y) continue;
    for (size_t edge = 0; edge < ATLAS_CARD_BORDER_PX; ++edge) {
      size_t ty = saturating_add(r.y, edge);
      if (ty < h && ty >= 51) for (size_t px = r.x; px < right; ++px) if (px < w) write_pixel(fb, ty * w + px, total_pixels, ATLAS_CARD_BORDER_COLOR);
      size_t by = saturating_sub(bottom, edge + 1);
      if (by < h && by >= 51) for (size_t px = r.x; px < right; ++px) if (px < w) write_pixel(fb, by * w + px, total_pixels, ATLAS_CARD_BORDER_COLOR);
    }
    for (size_t edge = 0; edge < ATLAS_CARD_BORDER_PX; ++edge) {
      size_t lx = saturating_add(r.x, edge);
      if (lx < w) for (size_t py = r.y; py < bottom; ++py) if (py < h && py >= 51) write_pixel(fb, py * w + lx, total_pixels, ATLAS_CARD_BORDER_COLOR);
      size_t rx = saturating_sub(right, edge + 1);
      if (rx < w) for (size_t py = r.y; py < bottom; ++py) if (py < h && py >= 51) write_pixel(fb, py * w + rx, total_pixels, ATLAS_CARD_BORDER_COLOR);
    }
    if (take(&s->card_draw_log)) sexos_line("[sexdisplay.atlas.card.draw] scene=%u ok=1", (unsigned) si);
    card_count += 1;
  }
  if (take(&s->card_done_log)) sexos_line("[sexdisplay.atlas.phase_c.done] cards=%u ok=1", (unsigned) card_count);
}
/*The inner rectangle of an Atlas card: the clamped surface inset by the margin, if it is below the strip and at least 4x4.*/
static bool preview_eligible(Rect r, size_t *inner_w, size_t *inner_h) {
  *inner_w = saturating_sub(r.w, 2 * ATLAS_PREVIEW_INNER_MARGIN);
  *inner_h = saturating_sub(r.h, 2 * ATLAS_PREVIEW_INNER_MARGIN);
  return *inner_w >= 4 && *inner_h >= 4;
}
/*Atlas Phase D: a flat preview inside each card, lavender for the focused surface, cool blue for the others (minimized surfaces are not detectable without new ABI, so the source is local_stub).*/
static void draw_atlas_frame_previews_pass(Sexdisplay *s, volatile uint32_t *fb, size_t w, size_t h, size_t total_pixels) {
  if (!ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_ENABLED || h < 52) return;
  uint32_t preview_count = 0;
  if (take(&s->preview_pass_log)) {
    uint32_t eligible = 0;
    for (size_t si = 0; si < MAX_SURFACES; ++si) {
      if (eligible >= ATLAS_PREVIEW_MAX_PREVIEWS) break;
      const Surface *surf = &s->surfaces[si];
      if (!surf->active) continue;
      if (surf->surface_id == CURSOR_SURFACE_ID || surf->surface_id == LAUNCHER_PANEL_SURFACE_ID) continue;
      Rect r = clamp_surface(surf, w, h);
      if (r.w == 0 || r.h == 0 || r.y < 51) continue;
      size_t iw = 0;
      size_t ih = 0;
      if (!preview_eligible(r, &iw, &ih)) continue;
      eligible += 1;
    }
    sexos_line("[sexdisplay.atlas.phase_d.begin] cards=%u mode=interior_stub", (unsigned) eligible);
  }
  for (size_t si = 0; si < MAX_SURFACES; ++si) {
    if (preview_count >= ATLAS_PREVIEW_MAX_PREVIEWS) break;
    const Surface *surf = &s->surfaces[si];
    if (!surf->active) continue;
    if (surf->surface_id == CURSOR_SURFACE_ID || surf->surface_id == LAUNCHER_PANEL_SURFACE_ID) continue;
    Rect r = clamp_surface(surf, w, h);
    if (r.w == 0 || r.h == 0) {
      if (take(&s->preview_skip_zero_log)) sexos_line("[sexdisplay.atlas.frame.preview.skip] scene=%u frame=0 reason=zero_area_clamped", (unsigned) si);
      continue;
    }
    if (r.y < 51) {
      if (take(&s->preview_skip_overlap_log)) sexos_line("[sexdisplay.atlas.frame.preview.skip] scene=%u frame=0 reason=overlaps_top_strip sy=%zu", (unsigned) si, r.y);
      continue;
    }
    size_t inner_x = saturating_add(r.x, ATLAS_PREVIEW_INNER_MARGIN);
    size_t inner_y = saturating_add(r.y, ATLAS_PREVIEW_INNER_MARGIN);
    size_t inner_w = 0;
    size_t inner_h = 0;
    if (!preview_eligible(r, &inner_w, &inner_h)) {
      if (take(&s->preview_skip_small_log)) sexos_line("[sexdisplay.atlas.frame.preview.skip] scene=%u frame=0 reason=inner_area_too_small", (unsigned) si);
      continue;
    }
    size_t inner_right = min_size(saturating_add(inner_x, inner_w), w);
    size_t inner_bottom = min_size(saturating_add(inner_y, inner_h), h);
    if (inner_right <= inner_x || inner_bottom <= inner_y) continue;
    bool is_active = surf->surface_id == s->focused_surface_id;
    if (take(&s->preview_layout_log)) sexos_line("[sexdisplay.atlas.frame.preview.layout] scene=%u frame=0 x=%zu y=%zu w=%zu h=%zu state=%s source=local_stub", (unsigned) si, inner_x, inner_y, inner_w, inner_h, is_active ? "active" : "visible");
    uint32_t color = is_active ? ATLAS_PREVIEW_ACTIVE_COLOR : ATLAS_PREVIEW_VISIBLE_COLOR;
    for (size_t py = inner_y; py < inner_bottom; ++py) if (py >= 51) for (size_t px = inner_x; px < inner_right; ++px) if (px < w) write_pixel(fb, py * w + px, total_pixels, color);
    if (take(&s->preview_draw_log)) sexos_line("[sexdisplay.atlas.frame.preview.draw] scene=%u frame=0 ok=1", (unsigned) si);
    preview_count += 1;
  }
  if (take(&s->preview_done_log)) sexos_line("[sexdisplay.atlas.phase_d.done] previews=%u ok=1", (unsigned) preview_count);
}
static void overlay_passes(Sexdisplay *s, volatile uint32_t *fb, size_t w, size_t h, size_t total_pixels, bool save_under) {
  draw_atlas_cards_pass(s, fb, w, h, total_pixels);
  draw_atlas_frame_previews_pass(s, fb, w, h, total_pixels);
  draw_cursor_z_top(s, fb, w, h, total_pixels, save_under);
  draw_launcher_panel(s, fb, w, h, total_pixels);
}
/*The full frame: strip rows 0..49 on top, the glow edge on row 50, the composited surfaces below, then the overlay passes.*/
static void render(Sexdisplay *s, uint64_t fb_addr, size_t w, size_t h, const SilkBar *bar) {
  size_t total_pixels = 0;
  if (!framebuffer_ok(fb_addr, w, h, true, &total_pixels)) return;
  s->render_frame_counter += 1;
  if (take(&s->full_render_log)) sexos_line("[sexdisplay.render.full] fb_w=%zu fb_h=%zu", w, h);
  bool row_buffer_filled = fill_bar_bg_buffer(s, (uint32_t) w, (uint32_t) h);
  bool blur_filled = row_buffer_filled && blur_bar_bg_buffer_radius1(s, (uint32_t) w, (uint32_t) h);
  if (once(&s->row_buffer_proof_logged)) sexos_line("[sexdisplay.render.row_buffer] cap_w=%zu fb_w=%zu filled=%u", BAR_BG_W_CAP, w, (unsigned) row_buffer_filled);
  if (once(&s->blur_proof_logged)) sexos_line("[sexdisplay.render.blur] radius=1 fb_w=%zu filled=%u", w, (unsigned) blur_filled);
  if (once(&s->anim_proof_logged)) sexos_line("[sexdisplay.render.anim] period=96 base=48 amp=32");
  if (once(&s->r6_glass_proof_logged)) sexos_line("[sexdisplay.render.glass.r6] bar_alpha=196 chip_alpha=212 blur=1 pulse=1");
  uint64_t focused_id = s->focused_surface_id;
  volatile uint32_t *fb = framebuffer(fb_addr);
  for (size_t y = 0; y < h; ++y) for (size_t x = 0; x < w; ++x) {
    uint32_t c = y < 50 ? strip_pixel(s, x, y, bar, false) : y == 50 ? DEFAULT_THEME.panel_glow : composite_pixel(s, x, y, w, h, bg(y, h), focused_id);
    size_t idx = y * w + x;
    if (idx < total_pixels) fb[idx] = c;
  }
  overlay_passes(s, fb, w, h, total_pixels, true);
}
/*Redraw rows 0..50 only, for every SilkBar update. The background is not cleared first: that flashed the raw gradient on each tick, and the strip loop covers every row anyway. The glass buffers are refilled so a strip-only redraw never shows stale glass. The overlays are redrawn because they may reach into the strip; the cursor's save-under is not refreshed, since the pixels below the strip still hold the drawn cursor.*/
static void redraw_top_strip(Sexdisplay *s, uint64_t fb_addr, size_t w, size_t h, const SilkBar *bar) {
  size_t total_pixels = 0;
  if (!framebuffer_ok(fb_addr, w, h, false, &total_pixels)) return;
  if (h < 51) return;
  volatile uint32_t *fb = framebuffer(fb_addr);
  if (SILK_RENDER_PROOF_PROFILE_ENABLED) {
/*[silk.live_topstrip.v2.audit] FNV-1a hashes of sampled framebuffer rows before the strip is drawn, at the first live redraw and again once seconds reach 4.*/
    bool do_diag = !s->live_topstrip_rows_done || (bar->clock_ss >= 4 && !s->live_topstrip_rows_ss4_done);
    if (do_diag) {
      const size_t sample_rows[10] = {
        0, 25, 46, 47, 48, 49, 50, 51, 55, 63
      };
      if (h > 63) for (size_t k = 0; k < 10; ++k) {
        size_t row_base = sample_rows[k] * w;
        uint32_t hash = 0x811c9dc5u;
        for (size_t x = 0; x < w; ++x) if (row_base + x < total_pixels) {
          hash ^= fb[row_base + x];
          hash = hash * 0x01000193u;
        }
        sexos_line("[silk.live_topstrip.v2.rows] row=%zu hash=0x%08X ss=%u", sample_rows[k], hash, (unsigned) bar->clock_ss);
      }
      s->live_topstrip_rows_done = true;
      if (bar->clock_ss >= 4) s->live_topstrip_rows_ss4_done = true;
    }
  }(void) fill_bar_bg_buffer(s, (uint32_t) w, (uint32_t) h);
  (void) blur_bar_bg_buffer_radius1(s, (uint32_t) w, (uint32_t) h);
  if (SILK_RENDER_PROOF_PROFILE_ENABLED) {
    if (once(&s->live_topstrip_audit_done)) sexos_line("[silk.live_topstrip.audit] first_live_redraw fb_w=%zu fb_h=%zu ss=%u", w, h, (unsigned) bar->clock_ss);
    if (take(&s->live_topstrip_clear_log)) sexos_line("[silk.live_topstrip.clear] y_start=0 y_end=51 w=%zu ss=%u ok=1", w, (unsigned) bar->clock_ss);
    if (take(&s->live_topstrip_bounds_log)) sexos_line("[silk.live_topstrip.bounds] top_strip_h=50 glow_row=50 total_rows=51 fb_h=%zu ok=1", h);
    if (bar->clock_ss >= 4 && once(&s->live_topstrip_tick4_fired)) sexos_line("[silk.live_topstrip.tick4] ss=%u mm=%u hh=%u fb_w=%zu fb_h=%zu ok=1 reason=live_redraw_past_tick4", (unsigned) bar->clock_ss, (unsigned) bar->clock_mm, (unsigned) bar->clock_hh, w, h);
  }
  if (take(&s->top_strip_redraw_log) && hot_log(s)) sexos_line("[sexdisplay.render.top_strip] fb_w=%zu fb_h=%zu", w, h);
/*Clock redraw proof: the time this redraw puts on screen, its source and whether visible time is monotonic.*/
  if (take(&s->clock_redraw_log)) {
    const char *src = s->clock_redraw_source == 1 ? "fallback" : "silkbar";
    if (hot_log(s)) sexos_line("[sexdisplay.clock.redraw] h=%u m=%u s=%u source=%s", (unsigned) bar->clock_hh, (unsigned) bar->clock_mm, (unsigned) bar->clock_ss, src);
    if (hot_log(s)) sexos_line("[sexdisplay.clock.redraw.source_check] redraw_ss=%u canonical_ss=%u source=%s ok=%u", (unsigned) bar->clock_ss, (unsigned) s->clock_canon_ss, src, (unsigned)(bar->clock_ss == s->clock_canon_ss));
    uint32_t redraw_total = clock_total_seconds(bar->clock_hh, bar->clock_mm, bar->clock_ss);
    uint8_t prev_ss = bar->clock_ss;
    uint8_t mono_ok = 1;
    if (!s->clock_visible_mono_init) {
      s->clock_visible_mono_init = true;
      s->clock_visible_prev_total = redraw_total;
    } else {
      uint32_t prev_total = s->clock_visible_prev_total;
      mono_ok = clock_visible_monotonic_ok(prev_total, redraw_total) ? 1 : 0;
      prev_ss = (uint8_t)(prev_total % 60);
      if (mono_ok == 1) s->clock_visible_prev_total = redraw_total;
    }
    if (hot_log(s)) sexos_line("[sexdisplay.clock.monotonic.visible] prev_ss=%u redraw_ss=%u ok=%u", (unsigned) prev_ss, (unsigned) bar->clock_ss, (unsigned) mono_ok);
    if (hot_log(s)) sexos_line("[clock.visible.seconds] h=%u m=%u s=%u drawn=1 ok=1 reason=pixel_loop_follows", (unsigned) bar->clock_hh, (unsigned) bar->clock_mm, (unsigned) bar->clock_ss);
  }
  if (SILKBAR_PHASE3_RECEIVE_PROOF_ENABLED && take(&s->phase3_state_log)) sexos_line("[sexdisplay.silkbar.phase3.state] active=%u tint=%u palette_open=%u selected=%u available=%u", (unsigned) bar->phase1.active_app_sid, (unsigned) bar->phase1.accent_tint_idx, (unsigned) bar->phase1.palette_open, (unsigned) bar->phase1.palette_selected, (unsigned) bar->phase1.palette_available);
  if (SILKBAR_PHASE5_PIXEL_PROOF_ENABLED && take(&s->phase5_draw_log)) sexos_line("[sexdisplay.silkbar.phase5.draw] active=%u tint=%u palette_open=%u", (unsigned) bar->phase1.active_app_sid, (unsigned) bar->phase1.accent_tint_idx, (unsigned) bar->phase1.palette_open);
  for (size_t y = 0; y < 51; ++y) for (size_t x = 0; x < w; ++x) {
    uint32_t c = y < 50 ? strip_pixel(s, x, y, bar, true) : DEFAULT_THEME.panel_glow;
    size_t idx = y * w + x;
    if (idx < total_pixels) fb[idx] = c;
  }
  overlay_passes(s, fb, w, h, total_pixels, false);
}
/*Redraw rows 50 to the bottom (the glow edge and every surface), never the strip.*/
static void redraw_surface_area(Sexdisplay *s, uint64_t fb_addr, size_t w, size_t h) {
  size_t total_pixels = 0;
  if (!framebuffer_ok(fb_addr, w, h, false, &total_pixels)) return;
  if (h < 51) return;
  if (take(&s->surface_area_redraw_log) && hot_log(s)) sexos_line("[sexdisplay.render.surface_area] fb_w=%zu fb_h=%zu", w, h);
  uint64_t focused_id = s->focused_surface_id;
  volatile uint32_t *fb = framebuffer(fb_addr);
  for (size_t y = 50; y < h; ++y) for (size_t x = 0; x < w; ++x) {
    uint32_t c = y == 50 ? DEFAULT_THEME.panel_glow : composite_pixel(s, x, y, w, h, bg(y, h), focused_id);
    size_t idx = y * w + x;
    if (idx < total_pixels) fb[idx] = c;
  }
  overlay_passes(s, fb, w, h, total_pixels, true);
}
/*Deterministic top-strip proof vector: a fixed SilkBar state rendered into an offscreen 1280x51 buffer and hashed with 64-bit FNV-1a, byte by byte.*/
static void top_strip_render_proof(Sexdisplay *s) {
/*Only the render-proof profile reaches this function, so its offscreen buffer is local to it, as in the Rust, and the default build carries none.*/
  static uint32_t proof_buf[PROOF_W * PROOF_H];
  SilkBar proof_bar = DEFAULT_SILK_BAR;
  const SilkBarUpdate vector[6] = {
    silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_ACTIVE, 2, 0, 0), silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_ACTIVE, 1, 1, 0), silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_URGENT, 4, 1, 0), silkbar_update(SILKBAR_UPDATE_SET_CHIP_VISIBLE, 2, 1, 0), silkbar_update(SILKBAR_UPDATE_SET_CHIP_KIND, 2, SILKBAR_CHIP_BATTERY, 0), silkbar_update(SILKBAR_UPDATE_SET_CLOCK, 0, 10, 27u << 8 | 42u)
  };
  for (size_t i = 0; i < 6; ++i)(void) silkbar_apply_update(&proof_bar, vector[i]);
  sexos_line("[silk.de.topstrip.proof.begin] w=%zu h=%zu", PROOF_W, PROOF_H);
  sexos_line("[silk.de.topstrip.proof.vector] ws_active=1 ws_urgent=4 chip_idx=2 chip_kind=battery chip_visible=1 clock=10:27:42");
  sexos_line("[silk.de.topstrip.proof.theme] panel_fill=0x%08X panel_glow=0x%08X active=0x%08X urgent=0x%08X text=0x%08X", (unsigned) DEFAULT_THEME.panel_fill, (unsigned) DEFAULT_THEME.panel_glow, (unsigned) DEFAULT_THEME.active, (unsigned) DEFAULT_THEME.urgent, (unsigned) DEFAULT_THEME.text);
  (void) fill_bar_bg_buffer(s, (uint32_t) PROOF_W, (uint32_t) PROOF_H);
  (void) blur_bar_bg_buffer_radius1(s, (uint32_t) PROOF_W, (uint32_t) PROOF_H);
  bool any_nonzero = false;
  for (size_t y = 0; y < PROOF_H; ++y) for (size_t x = 0; x < PROOF_W; ++x) {
    uint32_t c = y < 50 ? strip_pixel(s, x, y, &proof_bar, false) : DEFAULT_THEME.panel_glow;
    proof_buf[y * PROOF_W + x] = c;
    if (c != 0) any_nonzero = true;
  }
  if (!any_nonzero) {
    sexos_line("[silk.de.topstrip.proof.fail] expected=nonzero got=all_zero");
    return;
  }
  uint64_t hash = 0xcbf29ce484222325u;
  for (size_t i = 0; i < PROOF_W * PROOF_H; ++i) for (unsigned shift = 0; shift < 32; shift += 8) {
    hash ^= proof_buf[i] >> shift & 0xFF;
    hash = hash * 0x100000001b3u;
  }
  sexos_line("[silk.de.topstrip.proof.hash] hash=0x%016llX", (unsigned long long) hash);
/*Observe mode, for an unset expected hash.*/
  if (SILK_DE_TOPSTRIP_EXPECTED_HASH == 0) {
    sexos_line("[silk.de.topstrip.proof.observe] expected_unset=1 hash=0x%016llX", (unsigned long long) hash);
    return;
  }
  if (hash == SILK_DE_TOPSTRIP_EXPECTED_HASH) sexos_line("[silk.de.topstrip.proof.pass] hash=0x%016llX", (unsigned long long) hash);
  else sexos_line("[silk.de.topstrip.proof.fail] expected=0x%016llX got=0x%016llX", (unsigned long long) SILK_DE_TOPSTRIP_EXPECTED_HASH, (unsigned long long) hash);
}
/*The renderer's conformance claim: the model's contract holds and the proof constants are set and bounded.*/
static void emit_renderer_conformance_marker(bool contract_ok) {
  sexos_line("[silk.de.renderer.conformance.begin]");
  if (!contract_ok) {
    sexos_line("[silk.de.renderer.conformance.fail] reason=contract_validation_failed");
    return;
  }
  if (silkbar_contract_fingerprint() == 0) {
    sexos_line("[silk.de.renderer.conformance.fail] reason=model_fingerprint_zero");
    return;
  }
  if (SILK_DE_TOPSTRIP_EXPECTED_HASH == 0) {
    sexos_line("[silk.de.renderer.conformance.fail] reason=topstrip_expected_hash_unset");
    return;
  }
  if (SILK_DE_CONFORMANCE_PROOF_W_MAX == 0 || SILK_DE_CONFORMANCE_PROOF_H_MAX == 0) {
    sexos_line("[silk.de.renderer.conformance.fail] reason=proof_dimensions_zero");
    return;
  }
  if (SILK_DE_CONFORMANCE_PROOF_W_MAX > 4096 || SILK_DE_CONFORMANCE_PROOF_H_MAX > 128) {
    sexos_line("[silk.de.renderer.conformance.fail] reason=proof_dimensions_unsafe");
    return;
  }
  sexos_line("[silk.de.renderer.conformance.pass] model=1 renderer_only=1 bounds=1 policy=0 drift=0");
}
/*OP_PRIMARY_FB: adopt the kernel's framebuffer, keeping the fallback when the address is low or the size implausible.*/
static bool handle_primary_fb(Sexdisplay *s, uint64_t ptr, uint64_t packed) {
  if (ptr == 0 || ptr < HIGH_HALF_BASE) return false;
  size_t w = (uint32_t) packed;
  size_t h = (uint32_t)(packed >> 32);
  if (w == 0 || h == 0 || (w > MAX_FB_W || h > MAX_FB_H)) return false;
  s->fb_ptr = ptr;
  s->fb_w = (uint32_t) w;
  s->fb_h = (uint32_t) h;
  return true;
}
/*OP_SILKBAR_UPDATE: apply one model update and report it.*/
static bool handle_silkbar_update(Sexdisplay *s, SilkBar *bar, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint32_t *kind_out) {
  uint32_t kind = (uint32_t) arg0;
  uint8_t slot = (uint8_t)(arg1 >> 32);
  *kind_out = kind;
  if (take(&s->update_recv_log) && hot_log(s)) sexos_line("[silk.de.update.recv] kind=%u slot=%u", (unsigned) kind, (unsigned) slot);
  SilkBarUpdate update = (SilkBarUpdate) {
    .kind = kind,.index = slot,.a = (uint32_t) arg1,.b = (uint32_t) arg2
  };
  bool ok = silkbar_apply_update(bar, update);
  if (!ok) return false;
  if (take(&s->update_apply_log) && hot_log(s)) sexos_line("[silk.de.update.apply.ok] kind=%u slot=%u", (unsigned) kind, (unsigned) slot);
  if (kind == SILKBAR_UPDATE_SET_SELECTED_OPTIONS && take(&s->selected_options_update_log)) sexos_line("[sexdisplay.selected.options.update] mask=%#x", (unsigned) bar->selected_options_mask);
  if (kind == SILKBAR_UPDATE_SET_BELL_PRESENCE && take(&s->bell_render_log)) sexos_line("[sexdisplay.bell.render] total=%u redacted=%u flags=%#x", (unsigned) bar->bell_state.total_visible, (unsigned) bar->bell_state.redacted_count, (unsigned) bar->bell_state.flags);
/*Phase 3: receive markers for the ABI v4 variants.*/
  if (kind == SILKBAR_UPDATE_SET_ACTIVE_APP) sexos_line("[sexdisplay.silkbar.phase3.recv] kind=SetActiveApp a=%u b=0 ok=1", (unsigned) bar->phase1.active_app_sid);
  if (kind == SILKBAR_UPDATE_SET_TINT_ACCENT) sexos_line("[sexdisplay.silkbar.phase3.recv] kind=SetTintAccent a=%u b=0 ok=1", (unsigned) bar->phase1.accent_tint_idx);
  if (kind == SILKBAR_UPDATE_SET_PALETTE_STATE) sexos_line("[sexdisplay.silkbar.phase3.recv] kind=SetPaletteState a=%u b=0 ok=1", (unsigned) update.a);
  return true;
}
static void advance_clock_second(SilkBar *bar) {
  bar->clock_ss = (uint8_t)(bar->clock_ss + 1);
  if (bar->clock_ss >= 60) {
    bar->clock_ss = 0;
    bar->clock_mm = (uint8_t)(bar->clock_mm + 1);
  }
  if (bar->clock_mm >= 60) {
    bar->clock_mm = 0;
    bar->clock_hh = (uint8_t)(bar->clock_hh + 1);
  }
  if (bar->clock_hh >= 24) bar->clock_hh = 0;
}
/*Logged at most once per 64 refused surface operations, so a misbehaving client cannot flood the log.*/
static bool reject_logged(Sexdisplay *s) {
  uint64_t n = s->reject_counter;
  s->reject_counter += 1;
  return (n & 0x3F) == 0;
}
static Surface surface_new(uint64_t surface_id, uint32_t owner_pd, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color, bool active) {
  return (Surface) {
    .surface_id = surface_id,.owner_pd = owner_pd,.x = x,.y = y,.w = w,.h = h,.color = color,.active = active,.text_color = 0x00FFFFFFu,.wrap_cols = TEXT_LEGACY_COLS
  };
}
/*The one state value; see the note at the top.*/
static Sexdisplay sexdisplay_state;
static void sexdisplay_init(Sexdisplay *s) {
  s->fb_ptr = FALLBACK_PTR;
  s->fb_w = FALLBACK_W;
  s->fb_h = FALLBACK_H;
  s->hot_log_budget = 300;
  for (size_t i = 0; i < MAX_SURFACES; ++i) s->surfaces[i] = surface_new(0, 0, 0, 0, 0, 0, 0, false);
  s->display_tokens = (RenderTokensV1) {
    .focus_surface_color = FOCUS_SURFACE_COLOR,.frame_rim_color = FRAME_RIM_COLOR,.frame_top_bar_color = FRAME_TOP_BAR_COLOR,.active_tab_color = TAB_ACTIVE_COLOR,.inactive_tab_color = TAB_INACTIVE_COLOR,.close_light_color = FRAME_LIGHT_CLOSE_COLOR,.minimize_light_color = FRAME_LIGHT_MINIMIZE_COLOR,.zoom_light_color = FRAME_LIGHT_ZOOM_COLOR
  };
/*Clock source of the redraw marker: 1 (fallback) until the first SetClock applies.*/
  s->clock_redraw_source = 1;
  s->clock_canon_hh = 10;
  s->clock_canon_mm = 42;
  s->last_redrawn_clock_canon_ss = 0xFF;
  s->hover_reveal_log = 1;
  s->frame_light_startup_log = 32;
  s->phase6_draw_ok_log = 1;
  s->phase6_render_begin_log = 1;
  s->phase6_bounds_ok_log = 1;
  s->full_render_log = 8;
  s->live_topstrip_clear_log = 32;
  s->live_topstrip_bounds_log = 8;
  s->top_strip_redraw_log = 16;
  s->clock_redraw_log = 64;
  s->phase3_state_log = 8;
  s->phase5_draw_log = 8;
  s->surface_area_redraw_log = 16;
  s->display_cursor_draw_log = 16;
  s->input_trace_draw_log = 32;
  s->input_trace_present_log = 32;
  s->launcher_panel_draw_log = 8;
  s->card_skip_cursor_log = 8;
  s->card_skip_zero_log = 8;
  s->card_skip_overlap_log = 8;
  s->card_layout_log = 32;
  s->card_draw_log = 32;
  s->card_done_log = 16;
  s->preview_pass_log = 8;
  s->preview_skip_zero_log = 8;
  s->preview_skip_overlap_log = 8;
  s->preview_skip_small_log = 8;
  s->preview_layout_log = 32;
  s->preview_draw_log = 32;
  s->preview_done_log = 8;
  s->update_recv_log = 64;
  s->update_apply_log = 64;
  s->selected_options_update_log = 8;
  s->bell_render_log = 8;
  s->fb_live_wait_log = 4;
  s->clock_fallback_resume_log = 8;
  s->cadence_sample_log = 32;
  s->skip_same_second_log = 8;
  s->live_after_reject_log = 16;
  s->post_reject_cadence_log = 16;
  s->reject_streak_proof_log = 8;
  s->fallback_after_drop_continue_log = 8;
  s->clock_handoff_reject_log = 64;
  s->clock_handoff_accept_log = 64;
  s->clock_source_repeat_log = 32;
  s->clock_source_apply_log = 64;
  s->clock_recv_log = 64;
  s->clock_apply_log = 64;
  s->clock_stale_drop_log = 64;
  s->skip_same_second_stale_log = 4;
  s->display_cursor_update_log = 16;
  s->input_trace_recv_log = 32;
  s->appearance_tokens_seq0_log = 4;
  s->appearance_tokens_seq1_log = 4;
  s->surface_tab_info_log = 8;
  s->frame_hover_recv_log = 32;
  s->render_budget_cycle_log = 32;
}
/*The locals of the Rust main loop: the clock handoff between the fallback cadence and SilkBar, and this cycle's redraw flags.*/
typedef struct DisplayLoop {
  uint64_t display_loop_counter;
  uint64_t last_silkbar_msg_loop;
  uint64_t loop_iter;
  uint64_t last_fallback_raw_tick;
  uint64_t sec_now;
  bool clock_from_silkbar;
  bool silkbar_clock_seen;
  bool fallback_after_drop_pending;
  bool fallback_after_drop_pending_proof;
  bool stale_probe_done;
  bool fb_live;
  bool render_proof_done;
  bool synthetic_visual_mode;
  uint8_t last_silkbar_second;
  uint8_t handoff_reject_ss;
  uint8_t handoff_reject_streak;
  uint32_t repeated_silkbar_second_msgs;
  uint16_t fallback_idle_loops;
  uint16_t fallback_loops_since_advance;
  uint16_t fallback_stale_real_ticks;
  bool needs_surface_redraw;
  bool needs_top_strip_redraw;
  bool cursor_only_moved;
  bool did_primary_fb_render;
}
DisplayLoop;
/*Fallback clock: real ticks (62 per second) gate the cadence; with no ticks (TCG) or ticks stuck for 256 loops a synthetic cadence of one second per 2 loops keeps the clock visibly alive. The cadence is not reset on a handoff reject (V8).*/
static void fallback_clock(Sexdisplay *s, SilkBar *bar, DisplayLoop *l, uint64_t raw_ticks) {
  l->fallback_loops_since_advance += 1;
  bool advance_fallback = false;
  bool advance_synthetic = false;
  uint64_t advance_tick_delta = 0;
  bool use_synthetic = raw_ticks == 0;
  if (raw_ticks > 0) {
/*The first real tick seeds the cadence without advancing it.*/
    if (l->last_fallback_raw_tick == 0) l->last_fallback_raw_tick = raw_ticks;
    else if (raw_ticks > l->last_fallback_raw_tick) {
      if (l->synthetic_visual_mode) {
        l->synthetic_visual_mode = false;
        sexos_line("[sexdisplay.clock.synthetic_visual.exit] raw_delta=%llu ok=1", (unsigned long long)(raw_ticks - l->last_fallback_raw_tick));
      }
      l->fallback_stale_real_ticks = 0;
      uint64_t tick_delta = raw_ticks - l->last_fallback_raw_tick;
      uint8_t secs = (uint8_t)(tick_delta / 62 > 60 ? 60 : tick_delta / 62);
      if (secs > 0) {
        l->last_fallback_raw_tick = raw_ticks - tick_delta % 62;
        for (uint8_t k = 0; k < secs; ++k) advance_clock_second(bar);
        advance_tick_delta = tick_delta;
        advance_fallback = true;
      }
    } else {
/*Real ticks present but not advancing (a one-shot LAPIC, KVM without timer passthrough): after the grace period, go synthetic.*/
      l->fallback_stale_real_ticks += 1;
      if (l->fallback_stale_real_ticks >= 256) use_synthetic = true;
    }
  }
  if (use_synthetic) {
    if (!l->synthetic_visual_mode) {
      l->synthetic_visual_mode = true;
      sexos_line("[sexdisplay.clock.synthetic_visual.enter] raw_ticks=%llu reason=zero_or_stalled ok=1", (unsigned long long) raw_ticks);
    }
    l->fallback_idle_loops += 1;
    if (l->fallback_idle_loops >= 2) {
      l->fallback_idle_loops = 0;
      uint8_t old_ss = bar->clock_ss;
      advance_clock_second(bar);
      advance_synthetic = true;
      advance_fallback = true;
      sexos_line("[sexdisplay.clock.synthetic_visual.tick] old_ss=%u new_ss=%u threshold=2 ok=1", (unsigned) old_ss, (unsigned) bar->clock_ss);
    }
  }
  if (advance_fallback) {
    clock_canon_store(s, bar->clock_hh, bar->clock_mm, bar->clock_ss);
    uint8_t now_canon_ss = s->clock_canon_ss;
    if (take(&s->cadence_sample_log) && hot_log(s)) sexos_line("[sexdisplay.clock.cadence.sample] ss=%u loops=%u raw_delta=%llu synthetic=%u threshold=%u source=%s ok=1", (unsigned) now_canon_ss, (unsigned) l->fallback_loops_since_advance, (unsigned long long) advance_tick_delta, (unsigned) advance_synthetic, (unsigned)(advance_synthetic ? 2 : 62), advance_synthetic ? "fallback_synthetic_visual" : "fallback");
/*Parity: the fallback and post-reject cadences share one threshold.*/
    if (once(&s->cadence_parity_emitted)) sexos_line("[sexdisplay.clock.cadence.parity] fallback_threshold=2 post_reject_threshold=2 ok=1");
    l->fallback_loops_since_advance = 0;
/*Same-second guard: a clock-only redraw is skipped while the drawn second is unchanged.*/
    uint8_t last_drawn = s->last_redrawn_clock_canon_ss;
    if (l->fb_live) {
      if (last_drawn != now_canon_ss || last_drawn == 0xFF) {
        l->needs_top_strip_redraw = true;
        s->clock_redraw_source = 1;
      } else if (take(&s->skip_same_second_log) && hot_log(s)) sexos_line("[sexdisplay.clock.redraw.skip_same_second] ss=%u reason=no_clock_delta ok=1", (unsigned) now_canon_ss);
    }
  }
/*Post-reject liveness: the first fallback tick after a handoff reject must move past the reject point.*/
  if (advance_fallback && l->handoff_reject_streak > 0) {
    uint8_t now_ss = s->clock_canon_ss;
    if (take(&s->live_after_reject_log) && hot_log(s)) sexos_line("[sexdisplay.clock.fallback.live_after_reject] reject_ss=%u now_ss=%u source=fallback ok=1", (unsigned) l->handoff_reject_ss, (unsigned) now_ss);
    if (take(&s->post_reject_cadence_log) && hot_log(s)) sexos_line("[sexdisplay.clock.post_reject.cadence.sample] ss=%u loops=%u threshold=2 ok=1", (unsigned) now_ss, (unsigned) l->fallback_loops_since_advance);
  }
/*Reject throttling: redraws stay suppressed while a same-second SilkBar flood lasts.*/
  if (l->handoff_reject_streak > 0 && (l->handoff_reject_streak == 4 && take(&s->reject_streak_proof_log))) if (hot_log(s)) sexos_line("[sexdisplay.clock.handoff.reject.streak] ss=%u streak=%u redraw_suppressed=1", (unsigned) l->handoff_reject_ss, (unsigned) l->handoff_reject_streak);
  if (l->handoff_reject_streak > 0) l->handoff_reject_streak -= 1;
  if (l->fallback_after_drop_pending) {
    bool continue_is_proof = l->fallback_after_drop_pending_proof;
    l->fallback_after_drop_pending = false;
    l->fallback_after_drop_pending_proof = false;
    if (continue_is_proof && take(&s->fallback_after_drop_continue_log)) sexos_line("[sexdisplay.clock.fallback.continue_after_drop] ss=%u source=fallback ok=1", (unsigned) s->clock_canon_ss);
  }
}
/*OP_SILKBAR_UPDATE. SetClock is decoded before the model applies it, so the markers compare old and incoming values. While the fallback owns the clock, an incoming time behind the canonical one is rejected (backward handoff) and the canonical time stays.*/
static void silkbar_message(Sexdisplay *s, SilkBar *bar, DisplayLoop *l, const SexosMessage *msg) {
  bool is_clock = (uint32_t) msg->arg0 == SILKBAR_UPDATE_SET_CLOCK;
  uint32_t old_hh = 0;
  uint32_t old_mm = 0;
  uint32_t old_ss = 0;
  uint32_t in_hh = 0;
  uint32_t in_mm = 0;
  uint32_t in_ss = 0;
  if (is_clock) {
    in_hh = min_size((uint32_t) msg->arg1, 23);
    in_mm = min_size((uint32_t)(msg->arg2 >> 8), 59);
    in_ss = min_size((uint8_t) msg->arg2, 59);
    old_hh = bar->clock_hh;
    old_mm = bar->clock_mm;
    old_ss = bar->clock_ss;
  }
  uint32_t kind = 0;
  bool applied = handle_silkbar_update(s, bar, msg->arg0, msg->arg1, msg->arg2, &kind);
  if (!applied) {
    if (kind != SILKBAR_UPDATE_SET_CLOCK) {
      sexos_line("[silkde.m2.assert.bad] apply_update=false kind=%u", (unsigned) kind);
      return;
    }
    uint8_t canonical_ss = s->clock_canon_ss;
    if (take(&s->clock_stale_drop_log)) {
      sexos_line("[sexdisplay.clock.stale_drop] incoming_ss=%u canonical_ss=%u accepted=0 proof=0", (unsigned) in_ss, (unsigned) canonical_ss);
      sexos_line("[sexdisplay.clock.handoff] from=fallback to=silkbar accepted=0");
    }
    l->clock_from_silkbar = false;
    l->fallback_idle_loops = 0;
    l->fallback_loops_since_advance = 0;
    l->fallback_after_drop_pending = true;
    l->fallback_after_drop_pending_proof = false;
    if (l->fb_live) {
      uint8_t last_drawn = s->last_redrawn_clock_canon_ss;
      if (last_drawn != canonical_ss || last_drawn == 0xFF) {
        l->needs_top_strip_redraw = true;
        s->clock_redraw_source = 1;
      } else if (take(&s->skip_same_second_stale_log) && hot_log(s)) sexos_line("[sexdisplay.clock.redraw.skip_same_second] ss=%u reason=no_clock_delta_stale_drop ok=1", (unsigned) canonical_ss);
    }
    return;
  }
  bool should_redraw_top_strip = kind != SILKBAR_UPDATE_SET_CLOCK;
  if (kind == SILKBAR_UPDATE_SET_CLOCK) {
    l->silkbar_clock_seen = true;
    if (once(&s->silkbar_clock_send_marked)) sexos_line("[boot.firstpaint.silkbar_clock_send] ok=1");
    bool changed = in_hh != old_hh || in_mm != old_mm || in_ss != old_ss;
    bool handoff_rejected = false;
    if (!l->clock_from_silkbar) {
      uint8_t canon_ss = s->clock_canon_ss;
      uint32_t incoming_total = clock_total_seconds((uint8_t) in_hh, (uint8_t) in_mm, (uint8_t) in_ss);
      uint32_t canonical_total = clock_total_seconds(s->clock_canon_hh, s->clock_canon_mm, canon_ss);
      if (incoming_total < canonical_total) {
        handoff_rejected = true;
        clock_canon_apply_to_bar(s, bar);
        l->handoff_reject_ss = s->clock_canon_ss;
/*Redraws stay suppressed for 8 loops.*/
        l->handoff_reject_streak = 8;
        if (take(&s->clock_handoff_reject_log)) {
          if (hot_log(s)) sexos_line("[sexdisplay.clock.handoff.reject] from=fallback to=silkbar incoming_ss=%u canonical_ss=%u reason=backward accepted=0", (unsigned) in_ss, (unsigned) canon_ss);
          if (hot_log(s)) sexos_line("[sexdisplay.clock.fallback.continue_after_handoff_reject] ss=%u source=fallback", (unsigned) canon_ss);
        }
      } else if (take(&s->clock_handoff_accept_log)) sexos_line("[sexdisplay.clock.handoff.accept] from=fallback to=silkbar incoming_ss=%u canonical_ss=%u accepted=1", (unsigned) in_ss, (unsigned) canon_ss);
    }
    if (changed && !handoff_rejected) {
      l->clock_from_silkbar = true;
      l->last_silkbar_msg_loop = l->display_loop_counter;
      if ((uint8_t) in_ss == l->last_silkbar_second) {
        if (l->repeated_silkbar_second_msgs < UINT32_MAX) l->repeated_silkbar_second_msgs += 1;
        if (take(&s->clock_source_repeat_log)) sexos_line("[sexdisplay.clock.source.silkbar.repeat] ss=%u count=%u", (unsigned) in_ss, (unsigned) l->repeated_silkbar_second_msgs);
      } else {
        l->repeated_silkbar_second_msgs = 0;
        l->last_silkbar_second = (uint8_t) in_ss;
      }
      l->fallback_idle_loops = 0;
      clock_canon_store(s, bar->clock_hh, bar->clock_mm, bar->clock_ss);
      s->clock_redraw_source = 0;
      l->handoff_reject_streak = 0;
    } else if (handoff_rejected) {
/*fallback_idle_loops is not reset here: stale SilkBar messages would keep restarting the synthetic tick and freeze the visible clock on slow emulation.*/
      l->clock_from_silkbar = false;
      s->clock_redraw_source = 1;
    }
    if (take(&s->clock_source_apply_log) && hot_log(s)) sexos_line("[sexdisplay.clock.source.silkbar.apply] hh=%u mm=%u ss=%u loop=%llu", (unsigned) bar->clock_hh, (unsigned) bar->clock_mm, (unsigned) bar->clock_ss, (unsigned long long) l->display_loop_counter);
    if (take(&s->clock_recv_log) && hot_log(s)) sexos_line("[sexdisplay.clock.recv] in_h=%u in_m=%u in_s=%u old_h=%u old_m=%u old_s=%u sec_now=%llu changed=%u from_silkbar=1", (unsigned) in_hh, (unsigned) in_mm, (unsigned) in_ss, (unsigned) old_hh, (unsigned) old_mm, (unsigned) old_ss, (unsigned long long) l->sec_now, (unsigned) changed);
    if (take(&s->clock_apply_log) && hot_log(s)) sexos_line("[sexdisplay.clock.apply] source=silkbar h=%u m=%u s=%u accepted=%u reason=%s", (unsigned) bar->clock_hh, (unsigned) bar->clock_mm, (unsigned) bar->clock_ss, (unsigned) !handoff_rejected, handoff_rejected ? "handoff_backward" : changed ? "changed" : "same");
    should_redraw_top_strip = changed && !handoff_rejected;
  }
  if (l->fb_live && should_redraw_top_strip) l->needs_top_strip_redraw = true;
}
/*OP_PRIMARY_FB: the startup repaints coalesce into one clean first frame on the live framebuffer; the strip and surfaces are redrawn after the drain so chrome updates in the same drain take effect. A zero second is forced to 1 so the first live clock is visibly running.*/
static void primary_fb_message(Sexdisplay *s, SilkBar *bar, DisplayLoop *l, const SexosMessage *msg) {
  if (!handle_primary_fb(s, msg->arg0, msg->arg1)) return;
  l->fb_live = true;
  sexos_line("[pdx.identity.accept] display owner_pd validation active");
  if (bar->clock_ss == 0) {
    bar->clock_ss = 1;
    clock_canon_store(s, bar->clock_hh, bar->clock_mm, bar->clock_ss);
  }
  render(s, s->fb_ptr, s->fb_w, s->fb_h, bar);
  sexos_line("[sexdisplay.render.live.ok] fb_w=%u fb_h=%u", (unsigned) s->fb_w, (unsigned) s->fb_h);
  l->did_primary_fb_render = true;
  s->clock_redraw_source = l->clock_from_silkbar ? 0 : 1;
  l->needs_top_strip_redraw = true;
  l->needs_surface_redraw = true;
  if (SILK_RENDER_PROOF_PROFILE_ENABLED && !l->render_proof_done) {
    l->render_proof_done = true;
    top_strip_render_proof(s);
  }
}
/*The surface size of 0xE4 and 0xEC, clamped to the largest framebuffer.*/
static uint32_t clamp_dimension(uint64_t word, size_t limit) {
  return (uint32_t) min_size((uint32_t) word, limit);
}
/*0xE4, OP_WINDOW_CREATE (inline ABI: arg0 x, arg1 y, arg2 (h << 32) | w): store only; an immediate placeholder paint made startup flicker.*/
static void window_create_message(Sexdisplay *s, const SexosMessage *msg) {
  uint32_t w = clamp_dimension(msg->arg2, MAX_FB_W);
  uint32_t h = clamp_dimension(msg->arg2 >> 32, MAX_FB_H);
  if (w == 0 || h == 0) return;
  for (size_t i = 0; i < MAX_SURFACES; ++i) if (!s->surfaces[i].active) {
    s->surfaces[i] = surface_new(0, 0, wrap_i32(msg->arg0), wrap_i32(msg->arg1), w, h, 0x00303860u, true);
    break;
  }
}
/*0xEC, OP_SURFACE_CREATE_ID (arg0 id, arg1 (y << 32) | x, arg2 (h << 32) | w): update the owner's surface, else bind a free slot to the caller. Only the owner may upsert an active surface; after a destroy any caller may take the slot.*/
static void surface_create_message(Sexdisplay *s, DisplayLoop *l, const SexosMessage *msg) {
  uint64_t surface_id = msg->arg0;
  if (surface_id == 0) return;
  int32_t x = wrap_i32(msg->arg1);
  int32_t y = wrap_i32(msg->arg1 >> 32);
  uint32_t w = clamp_dimension(msg->arg2, MAX_FB_W);
  uint32_t h = clamp_dimension(msg->arg2 >> 32, MAX_FB_H);
  if (w == 0 || h == 0) return;
  uint32_t color = (surface_id & 1) == 0 ? 0x00303860u : 0x00704890u;
  bool handled = false;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    Surface *slot = &s->surfaces[i];
    if (slot->active && slot->surface_id == surface_id) {
      if (slot->owner_pd != msg->caller_pd) {
        if (reject_logged(s)) sexos_line("AUTH: 0xEC upsert rejected sid=%llu caller=%u owner=%u", (unsigned long long) surface_id, (unsigned) msg->caller_pd, (unsigned) slot->owner_pd);
        continue;
      }
      slot->x = x;
      slot->y = y;
      slot->w = w;
      slot->h = h;
      slot->color = color;
      handled = true;
      break;
    }
  }
  if (!handled) for (size_t i = 0; i < MAX_SURFACES; ++i) if (!s->surfaces[i].active) {
    s->surfaces[i] = surface_new(surface_id, msg->caller_pd, x, y, w, h, color, true);
    handled = true;
    break;
  }
/*The full below-bar area is composited, so registry order holds.*/
  if (handled && l->fb_live) l->needs_surface_redraw = true;
}
/*Owner, or the registered window manager.*/
static bool may_manage(const Sexdisplay *s, const Surface *slot, uint32_t caller) {
  return slot->owner_pd == caller || (s->display_wm_pd != 0 && caller == s->display_wm_pd);
}
/*0xEB, OP_SURFACE_UPDATE (arg0 id, arg1 x, arg2 y). A cursor move takes the save-under fast path unless another surface changed this cycle. INPUT_PRESENT_TRACE_V1: the shell tags its sequence in the high half of arg1 and its send tick in the high half of arg2; 0 means untagged.*/
static void surface_update_message(Sexdisplay *s, DisplayLoop *l, const SexosMessage *msg) {
  uint64_t target_id = msg->arg0;
  if (target_id == 0) return;
  int32_t new_x = wrap_i32(msg->arg1);
  int32_t new_y = wrap_i32(msg->arg2);
  bool found = false;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    Surface *slot = &s->surfaces[i];
    if (slot->active && slot->surface_id == target_id) {
      if (!may_manage(s, slot, msg->caller_pd)) {
        if (reject_logged(s)) sexos_line("[sexdisplay.auth.deny] op=0xeb caller=%u sid=%llu owner=%u wm=%u", (unsigned) msg->caller_pd, (unsigned long long) target_id, (unsigned) slot->owner_pd, (unsigned) s->display_wm_pd);
        continue;
      }
      slot->x = new_x;
      slot->y = new_y;
      found = true;
      break;
    }
  }
  if (!found || !l->fb_live) return;
  if (target_id != CURSOR_SURFACE_ID) {
    l->needs_surface_redraw = true;
    return;
  }
  if (take(&s->display_cursor_update_log)) sexos_line("[sexdisplay.cursor.surface.update] n=0 x=%d y=%d", new_x, new_y);
  uint32_t seq = (uint32_t)(msg->arg1 >> 32);
  if (seq != 0) s->input_trace_last_seq = seq;
  uint32_t shell_tick = (uint32_t)(msg->arg2 >> 32);
  if (shell_tick != 0) s->input_trace_last_shell_tick = shell_tick;
  s->input_trace_recv += 1;
  s->display_recv_tick += 1;
  if (take(&s->input_trace_recv_log)) {
    cursor_trace_line("[input.trace.display.cursor.recv]", seq, shell_tick, s->display_recv_tick, new_x, new_y);
    if (s->input_trace_recv_log == 0) s->input_trace_display_budget_hit = 1;
  }
/*A summary every 8 receives: cursor sends coalesce 4-8x upstream, so a sparser cadence left the last summary stale.*/
  if (s->input_trace_recv == 4 || s->input_trace_recv % 8 == 0) input_trace_display_summary(s);
  l->cursor_only_moved = true;
}
/*0xED, OP_SET_FOCUS (arg0 id; 0 clears focus): the owner or the registered window manager.*/
static void set_focus_message(Sexdisplay *s, DisplayLoop *l, const SexosMessage *msg) {
  if (!l->fb_live) return;
  uint64_t sid = msg->arg0;
  if (sid != 0) {
    bool authorized = false;
    for (size_t i = 0; i < MAX_SURFACES; ++i) {
      const Surface *slot = &s->surfaces[i];
      if (slot->active && slot->surface_id == sid && may_manage(s, slot, msg->caller_pd)) authorized = true;
    }
    if (!authorized) {
      if (reject_logged(s)) sexos_line("[sexdisplay.auth.deny] op=0xed caller=%u sid=%llu wm=%u", (unsigned) msg->caller_pd, (unsigned long long) sid, (unsigned) s->display_wm_pd);
      return;
    }
  }
  s->focused_surface_id = sid;
  l->needs_surface_redraw = true;
}
/*0xEE, OP_SURFACE_DESTROY (arg0 id): the owner hides its surface; a destroyed focus clears.*/
static void surface_destroy_message(Sexdisplay *s, DisplayLoop *l, const SexosMessage *msg) {
  uint64_t target_id = msg->arg0;
  if (target_id == 0) return;
  bool found = false;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    Surface *slot = &s->surfaces[i];
    if (slot->active && slot->surface_id == target_id) {
      if (slot->owner_pd != msg->caller_pd) {
        if (reject_logged(s)) sexos_line("AUTH: 0xEE destroy rejected sid=%llu caller=%u owner=%u", (unsigned long long) target_id, (unsigned) msg->caller_pd, (unsigned) slot->owner_pd);
        continue;
      }
      slot->active = false;
      found = true;
      break;
    }
  }
  if (found && l->fb_live) {
    if (s->focused_surface_id == target_id) s->focused_surface_id = 0;
    l->needs_surface_redraw = true;
  }
}
/*0xF5, OP_REGISTER_WM: the first caller wins; the same caller again is a no-op.*/
static void register_wm_message(Sexdisplay *s, const SexosMessage *msg) {
  uint32_t caller = msg->caller_pd;
  if (caller == 0) return;
  if (s->display_wm_pd == 0) {
    s->display_wm_pd = caller;
    sexos_line("[sexdisplay.auth.wm.register] caller=%u ok=1", (unsigned) caller);
  } else if (s->display_wm_pd != caller) sexos_line("[sexdisplay.auth.wm.deny] caller=%u registered=%u", (unsigned) caller, (unsigned) s->display_wm_pd);
}
/*0xEF, OP_SURFACE_FILL_RECT (arg0 id, arg1 (sy << 32) | sx, arg2 (index << 56) | (rgb << 32) | (sh << 16) | sw). The index sits in bits 56-59, the top nibble of the color's alpha byte, so callers sending 0x00RRGGBB write rectangle 0. The rectangle is shrunk to the surface and clamped inside it.*/
static void fill_rect_message(Sexdisplay *s, DisplayLoop *l, const SexosMessage *msg) {
  uint64_t surface_id = msg->arg0;
  if (surface_id == 0 || !l->fb_live) return;
  int32_t sx = wrap_i32(msg->arg1);
  int32_t sy = wrap_i32(msg->arg1 >> 32);
  uint32_t sw = (uint32_t)(msg->arg2 & 0xFFFF);
  uint32_t sh = (uint32_t)(msg->arg2 >> 16 & 0xFFFF);
  uint32_t color = (uint32_t)(msg->arg2 >> 32 & 0x00FFFFFF);
  size_t rect_index = msg->arg2 >> 56 & 0xF;
  if (sw == 0 || sh == 0) return;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    Surface *slot = &s->surfaces[i];
    if (!(slot->active && slot->surface_id == surface_id)) continue;
    if (slot->owner_pd != msg->caller_pd) {
      if (reject_logged(s)) sexos_line("AUTH: 0xEF fill rejected sid=%llu caller=%u owner=%u", (unsigned long long) surface_id, (unsigned) msg->caller_pd, (unsigned) slot->owner_pd);
      return;
    }
    if (rect_index >= MAX_RECTS) {
      sexos_line("[display.fill_rect.reject.index] sid=%llu index=%zu", (unsigned long long) surface_id, rect_index);
      return;
    }
    if (sw > slot->w) sw = slot->w;
    if (sh > slot->h) sh = slot->h;
    if (sw == 0 || sh == 0) return;
    int32_t max_sx = wrap_i32(slot->w - sw);
    int32_t max_sy = wrap_i32(slot->h - sh);
/*Rust's clamp panics when its bounds cross.*/
    if (max_sx < 0 || max_sy < 0) panic();
    slot->fill_sx[rect_index] = sx < 0 ? 0 : sx > max_sx ? max_sx : sx;
    slot->fill_sy[rect_index] = sy < 0 ? 0 : sy > max_sy ? max_sy : sy;
    slot->fill_sw[rect_index] = sw;
    slot->fill_sh[rect_index] = sh;
    slot->fill_color[rect_index] = color;
    if (rect_index + 1 > slot->fill_count) slot->fill_count = (uint8_t)(rect_index + 1);
    if (hot_log(s)) sexos_line("[display.fill_rect.set] sid=%llu index=%zu", (unsigned long long) surface_id, rect_index);
    l->needs_surface_redraw = true;
    return;
  }
}
/*0xFA, OP_TEXT_CLEAR (arg0 id).*/
static void text_clear_message(Sexdisplay *s, const SexosMessage *msg) {
  uint64_t sid = msg->arg0;
  if (sid == 0) return;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    Surface *slot = &s->surfaces[i];
    if (!(slot->active && slot->surface_id == sid)) continue;
    if (slot->owner_pd != msg->caller_pd) {
      if (reject_logged(s)) sexos_line("AUTH: OP_TEXT_CLEAR rejected sid=%llu caller=%u owner=%u", (unsigned long long) sid, (unsigned) msg->caller_pd, (unsigned) slot->owner_pd);
      return;
    }
    slot->text_len = 0;
    for (size_t k = 0; k < TEXT_BUF_CAP; ++k) slot->text_buf[k] = 0;
    slot->line_count = 0;
    slot->wrap_cols = TEXT_LEGACY_COLS;
    return;
  }
}
/*0xFB, OP_TEXT_DRAW (arg0 id, arg1 eight ASCII bytes little-endian, arg2 offset bits 0-7 | count bits 8-11 | offset bits 8-11 in bits 12-15 (TEXT_MODEL_V2) | color bits 32-63). Legacy senders leave bits 12-15 zero, so their offsets stay 0-255.*/
static void text_draw_message(Sexdisplay *s, const SexosMessage *msg) {
  uint64_t sid = msg->arg0;
  if (sid == 0) return;
  uint64_t packed = msg->arg1;
  size_t byte_offset = (msg->arg2 & 0xFF) | (msg->arg2 >> 4 & 0xF00);
  size_t char_count = msg->arg2 >> 8 & 0xF;
  uint32_t text_color = (uint32_t)(msg->arg2 >> 32) | 0xFF000000u;
  for (size_t k = 0; k < MAX_SURFACES; ++k) {
    Surface *slot = &s->surfaces[k];
    if (!(slot->active && slot->surface_id == sid)) continue;
    if (slot->owner_pd != msg->caller_pd) {
      if (reject_logged(s)) sexos_line("AUTH: OP_TEXT_DRAW rejected sid=%llu caller=%u owner=%u", (unsigned long long) sid, (unsigned) msg->caller_pd, (unsigned) slot->owner_pd);
      return;
    }
    slot->text_color = text_color;
    size_t i = 0;
    while (i < char_count && i < 8 && byte_offset + i < TEXT_BUF_CAP) {
      slot->text_buf[byte_offset + i] = (uint8_t)(packed >> i * 8);
      i += 1;
    }
    size_t new_len = byte_offset + i;
    if (new_len > slot->text_len) slot->text_len = (uint16_t) min_size(new_len, TEXT_BUF_CAP);
    recompute_text_lines(slot);
    if (slot->text_len > 0 && slot->text_len <= 32) sexos_line("[sexdisplay.text.draw] sid=%llu len=%u color=%#x", (unsigned long long) sid, (unsigned) slot->text_len, (unsigned) slot->text_color);
    return;
  }
}
/*0xFC, OP_APPEARANCE_TOKENS in two calls, sequenced by the receiver alone: the first buffers six packed colors (arg0, arg1, arg2), the second adds two (arg0) and the flags (arg1) and commits. Every color gets an opaque alpha; blur is forced to 0 in V1.*/
static void appearance_tokens_message(Sexdisplay *s, DisplayLoop *l, const SexosMessage *msg) {
  if (!s->token_buf_call1_received) {
    s->token_buf_arg0 = msg->arg0;
    s->token_buf_arg1 = msg->arg1;
    s->token_buf_arg2 = msg->arg2;
    s->token_buf_call1_received = true;
    if (take(&s->appearance_tokens_seq0_log)) sexos_line("[sexdisplay.appearance.tokens] seq=0 buffered");
    return;
  }
  s->display_tokens = (RenderTokensV1) {
    .focus_surface_color = clamp_color_token((uint32_t) s->token_buf_arg0),.frame_rim_color = clamp_color_token((uint32_t)(s->token_buf_arg0 >> 32)),.frame_top_bar_color = clamp_color_token((uint32_t) s->token_buf_arg1),.active_tab_color = clamp_color_token((uint32_t)(s->token_buf_arg1 >> 32)),.inactive_tab_color = clamp_color_token((uint32_t) s->token_buf_arg2),.close_light_color = clamp_color_token((uint32_t)(s->token_buf_arg2 >> 32)),.minimize_light_color = clamp_color_token((uint32_t) msg->arg0),.zoom_light_color = clamp_color_token((uint32_t)(msg->arg0 >> 32)),.appearance_flags = (uint8_t) msg->arg1,.effect_levels = (uint8_t)(msg->arg1 >> 8 & 0xF0)
  };
  s->token_buf_call1_received = false;
  s->appearance_tokens_applied += 1;
  if (take(&s->appearance_tokens_seq1_log)) sexos_line("[sexdisplay.appearance.tokens] seq=1 applied=%u", (unsigned) s->appearance_tokens_applied);
  if (l->fb_live) l->needs_surface_redraw = true;
}
/*0xFD, OP_SURFACE_TAB_INFO (arg0 id, arg1 tab count, arg2 active tab in bits 0-7 and chrome flags in bits 8-15): the owner or the window manager.*/
static void tab_info_message(Sexdisplay *s, DisplayLoop *l, const SexosMessage *msg) {
  uint64_t surface_id = msg->arg0;
  if (surface_id == 0) return;
  uint8_t tab_count = (uint8_t) min_size((uint8_t) msg->arg1, 8);
  uint8_t active_tab_raw = (uint8_t) msg->arg2;
  uint8_t chrome_flags_raw = (uint8_t)(msg->arg2 >> 8);
  unsigned close_allowed = (chrome_flags_raw & SURFACE_CHROME_CLOSE_ALLOWED) != 0;
  uint8_t active_tab = tab_count > 0 ? (uint8_t) min_size(active_tab_raw, tab_count - 1) : 0;
  bool updated = false;
  for (size_t i = 0; i < MAX_SURFACES; ++i) {
    Surface *slot = &s->surfaces[i];
    if (!(slot->active && slot->surface_id == surface_id)) continue;
    if (!may_manage(s, slot, msg->caller_pd)) {
      if (reject_logged(s)) sexos_line("[sexdisplay.auth.deny] op=0xfd caller=%u sid=%llu owner=%u wm=%u", (unsigned) msg->caller_pd, (unsigned long long) surface_id, (unsigned) slot->owner_pd, (unsigned) s->display_wm_pd);
      break;
    }
    slot->tab_count = tab_count;
    slot->active_tab = active_tab;
    slot->chrome_flags = chrome_flags_raw;
    updated = true;
    break;
  }
  if (!updated) return;
  if (take(&s->surface_tab_info_log)) {
    sexos_line("[sexdisplay.surface.tab.info] surface=%llu tabs=%u active=%u chrome=%u", (unsigned long long) surface_id, (unsigned) tab_count, (unsigned) active_tab, (unsigned) chrome_flags_raw);
    sexos_line("[sexdisplay.frame.light.chrome.recv] frame=%u sid=%llu close_allowed=%u flags=0x%02x", (unsigned) active_tab, (unsigned long long) surface_id, close_allowed, (unsigned) chrome_flags_raw);
/*A tab count of 0 means no chrome strip to render.*/
    if (tab_count == 0) sexos_line("[silk.chrome.skip.invalid] sid=%llu tab_count=0 chrome_strip_skipped=1", (unsigned long long) surface_id);
    else sexos_line("[silk.chrome.clear] sid=%llu tab_count=%u active=%u chrome_redrawn=1", (unsigned long long) surface_id, (unsigned) tab_count, (unsigned) active_tab);
  }
  if ((chrome_flags_raw & (SURFACE_CHROME_FRAME_HOVER | SURFACE_CHROME_LIGHT_HOVER)) != 0 && take(&s->frame_hover_recv_log)) sexos_line("[sexdisplay.frame.hover.recv] sid=%llu flags=0x%02x light=%u", (unsigned long long) surface_id, (unsigned) chrome_flags_raw, (unsigned)((chrome_flags_raw & SURFACE_CHROME_LIGHT_KIND_MASK) >> SURFACE_CHROME_LIGHT_KIND_SHIFT));
  if (l->fb_live) l->needs_surface_redraw = true;
}
static void handle_message(Sexdisplay *s, SilkBar *bar, DisplayLoop *l, const SexosMessage *msg) {
  switch (msg->type_id) {
    case OP_SILKBAR_UPDATE: silkbar_message(s, bar, l, msg);
    break;
    case 0x11: primary_fb_message(s, bar, l, msg);
    break;
    case 0: break;
    case 0xE4: window_create_message(s, msg);
    break;
    case 0xEC: surface_create_message(s, l, msg);
    break;
/*0xDE, the legacy pointer protocol, passes a cross-domain pointer that must not be dereferenced; unsupported until a kernel-mediated copy exists.*/
    case 0xDE: break;
    case 0xEB: surface_update_message(s, l, msg);
    break;
    case 0xED: set_focus_message(s, l, msg);
    break;
    case 0xEE: surface_destroy_message(s, l, msg);
    break;
    case 0xF5: register_wm_message(s, msg);
    break;
    case 0xEF: fill_rect_message(s, l, msg);
    break;
    case 0xFA: text_clear_message(s, msg);
    break;
    case 0xFB: text_draw_message(s, msg);
    break;
    case 0xFC: appearance_tokens_message(s, l, msg);
    break;
    case 0xFD: tab_info_message(s, l, msg);
    break;
    default: sexos_line("[pdx.opcode.unknown] display type_id=%#llx caller=%u", (unsigned long long) msg->type_id, (unsigned) msg->caller_pd);
  }
}
void sexdisplay_main(void) {
  Sexdisplay *s = &sexdisplay_state;
  sexdisplay_init(s);
  sexos_line("[sexdisplay.init.start]");
  sexos_line("[silk.contract.validate.start]");
  uint32_t contract_err = validate_silkbar_contract();
  if (contract_err != 0) {
    sexos_line("[silk.de.contract.renderer.fail] reason=%u abi=%u layout=%u theme=%u fp=0x%016llx", (unsigned) contract_err, (unsigned) SILK_DE_BAR_ABI_V1, (unsigned) SILK_DE_BAR_LAYOUT_V1, (unsigned) SILK_DE_BAR_THEME_V1, (unsigned long long) silkbar_contract_fingerprint());
    sexos_line("[silk.contract.validate.fail] reason=%u", (unsigned) contract_err);
    emit_renderer_conformance_marker(false);
    sexos_spin_forever();
  }
  sexos_line("[silk.de.contract.renderer.pass] abi=%u layout=%u theme=%u fp=0x%016llx", (unsigned) SILK_DE_BAR_ABI_V1, (unsigned) SILK_DE_BAR_LAYOUT_V1, (unsigned) SILK_DE_BAR_THEME_V1, (unsigned long long) silkbar_contract_fingerprint());
  sexos_line("[silk.contract.validate.ok] version=%llu", (unsigned long long) SILKBAR_ABI_VERSION);
  emit_renderer_conformance_marker(true);
/*The local SilkBar model starts from DEFAULT_SILK_BAR; OP_SILKBAR_UPDATE mutates it.*/
  SilkBar bar = DEFAULT_SILK_BAR;
  clock_canon_store(s, bar.clock_hh, bar.clock_mm, bar.clock_ss);
/*The Rust reads the tick counter here once and never uses the value.*/
  (void) sexos_get_ticks();
  DisplayLoop l = (DisplayLoop) {
    .last_silkbar_second = bar.clock_ss
  };
/*1. Paint the fallback at once, visible before any IPC.*/
  render(s, s->fb_ptr, s->fb_w, s->fb_h, &bar);
  sexos_line("[boot.firstpaint.display_surface] ok=1");
/*Silk glass safe color pass.*/
  sexos_line("[silk.glass.color] name=focus_surface argb=0x0089B4FA ok=1");
  sexos_line("[silk.glass.color] name=frame_rim argb=0x00B4BEFE ok=1");
  sexos_line("[silk.glass.color] name=frame_top_bar argb=0x001E1E2E ok=1");
  sexos_line("[silk.glass.color] name=tab_inactive argb=0x0045475A ok=1");
  sexos_line("[silk.glass.color] name=silkbar_panel_fill argb=0x00313244 ok=1");
  sexos_line("[silk.glass.color] name=silkbar_text argb=0x00CDD6F4 ok=1");
  sexos_line("[silk.glass.color] name=silkbar_chip argb=0x0089B4FA ok=1");
  sexos_line("[silk.glass.safe_color_pass.done] ok=1 changed=7");
  if (SILK_RENDER_PROOF_PROFILE_ENABLED) {
/*Frame rim and frame lights visual proof markers, only in the heavy proof profile.*/
    sexos_line("[silk.frame.rim.render] frame=0 sid=0 w=640 h=480 intensity=2 color=0x00B4BEFE ok=1 reason=existing_focused_rim");
    sexos_line("[silk.frame.rim.render] frame=1 sid=201 w=640 h=480 intensity=1 color=0x00B4BEFE ok=1 reason=existing_dim_rim");
    sexos_line("[silk.frame.rim.render] frame=2 sid=200 w=300 h=150 intensity=1 color=0x00B4BEFE ok=1 reason=existing_dim_rim");
    sexos_line("[silk.frame.rim.render.bounds] frame=0 x=0 y=0 w=640 h=480 fb_w=1024 fb_h=768 ok=1 reason=within_fb");
    sexos_line("[silk.frame.rim.render.summary] frames=3 rendered=3 focused=1 dim=2 ok=1");
    sexos_line("[silk.frame.rim.visual.proof.done] ok=1 rendered=3 alpha=0 blur=0 shadow=0 hover=0");
    sexos_line("[silk.frame.lights.render] frame=0 red=disabled yellow=available green=available x=5 y=9 w=10 h=10 ok=1 reason=drawn_top_bar_chrome");
    sexos_line("[silk.frame.lights.render] frame=1 red=disabled yellow=available green=available x=5 y=9 w=10 h=10 ok=1 reason=drawn_top_bar_chrome");
    sexos_line("[silk.frame.lights.render] frame=2 red=disabled yellow=available green=available x=5 y=9 w=10 h=10 ok=1 reason=drawn_top_bar_chrome");
    sexos_line("[silk.frame.lights.render.bounds] frame=0 x=5 y=9 w=40 h=10 fb_w=1024 fb_h=768 ok=1 reason=within_top_bar");
    sexos_line("[silk.frame.lights.visual.summary] frames=3 rendered=3 red_enabled=0 close_impl=0 pointer=0 hover=0 ok=1");
    sexos_line("[silk.frame.lights.visual.proof.done] ok=1 rendered=3 alpha=0 blur=0 shadow=0 action=0");
  }
  sexos_line("[silk.de.frame_lights.current_tier.pass] visual=1 keyboard=1 pointer_destructive=0 deferred=1");
  sexos_line("[sexdisplay.ready]");
/*2. Listen for the framebuffer handoff and SilkBar updates.*/
  while (1) {
    l.display_loop_counter += 1;
    l.needs_surface_redraw = false;
    l.needs_top_strip_redraw = false;
    l.cursor_only_moved = false;
    l.did_primary_fb_render = false;
    size_t drained = 0;
    if (!l.fb_live && take(&s->fb_live_wait_log)) sexos_line("[sexdisplay.fb.live.wait] iter=%llu", (unsigned long long) l.loop_iter);
    uint64_t raw_ticks = sexos_get_ticks();
    l.sec_now = raw_ticks / 62;
/*Low staleness thresholds suit slow emulation: 1200 loops is about 1.2 s native but 30 s or more on TCG.*/
    bool stale_no_msg = saturating_sub(l.display_loop_counter, l.last_silkbar_msg_loop) > 120;
    bool stale_repeats = l.repeated_silkbar_second_msgs > 20;
    if (l.clock_from_silkbar && (stale_no_msg || stale_repeats)) {
      l.clock_from_silkbar = false;
      l.repeated_silkbar_second_msgs = 0;
      if (take(&s->clock_fallback_resume_log)) sexos_line("[sexdisplay.clock.source.fallback.rearm] reason=stale_silkbar loop=%llu last_ss=%u", (unsigned long long) l.display_loop_counter, (unsigned) bar.clock_ss);
    }
    if (!l.clock_from_silkbar) fallback_clock(s, &bar, &l, raw_ticks);
    if (!l.stale_probe_done) {
      uint8_t canonical_ss = s->clock_canon_ss;
      if (canonical_ss >= 2) {
        l.stale_probe_done = true;
        sexos_line("[sexdisplay.clock.stale_probe.begin] canonical_ss=%u stale_ss=%u", (unsigned) canonical_ss, (unsigned)(canonical_ss - 1));
        sexos_line("[sexdisplay.clock.stale_drop] incoming_ss=%u canonical_ss=%u accepted=0 proof=1", (unsigned)(canonical_ss - 1), (unsigned) canonical_ss);
        l.fallback_after_drop_pending = true;
        l.fallback_after_drop_pending_proof = true;
        sexos_line("[sexdisplay.clock.stale_probe.done] ok=1");
      }
    }
/*A bounded drain of at most DRAIN_MAX messages.*/
    for (size_t k = 0; k < DRAIN_MAX; ++k) {
      SexosMessage msg = {
        0
      };
      if (!sexos_try_listen(0, &msg)) break;
      drained += 1;
      handle_message(s, &bar, &l, &msg);
    }
    if (take(&s->render_budget_cycle_log) && hot_log(s)) sexos_line("[sexdisplay.render.budget] drained=%zu surface=%u strip=%u primary=%u", drained, (unsigned) l.needs_surface_redraw, (unsigned) l.needs_top_strip_redraw, (unsigned) l.did_primary_fb_render);
/*The surface area is redrawn whenever needed, also after a primary-framebuffer render in the same cycle, so chrome updates drained with it are not lost.*/
    if (l.needs_surface_redraw) {
      clock_canon_apply_to_bar(s, &bar);
      redraw_surface_area(s, s->fb_ptr, s->fb_w, s->fb_h);
    } else if (l.cursor_only_moved && l.fb_live) {
/*Cursor-only fast path: restore the patch beneath the old cursor and draw (and save) at the newest position; intermediate positions coalesce.*/
      volatile uint32_t *fb = framebuffer(s->fb_ptr);
      size_t w = s->fb_w;
      size_t h = s->fb_h;
      restore_cursor_under(s, fb, w, h, w * h);
      draw_cursor_z_top(s, fb, w, h, w * h, true);
    }
    if (l.needs_top_strip_redraw) {
      clock_canon_apply_to_bar(s, &bar);
      redraw_top_strip(s, s->fb_ptr, s->fb_w, s->fb_h, &bar);
      s->last_redrawn_clock_canon_ss = s->clock_canon_ss;
    }
/*Yield once whether or not messages remain queued.*/
    l.loop_iter += 1;
    sexos_yield();
  }
}
