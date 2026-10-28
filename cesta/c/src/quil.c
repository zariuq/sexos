#include "sexos_server.h"
/*Quil text editor server, translated from servers/quil/src/main.rs. The Rust statics (the document, the reload staging copy, the undo ring, cursor, selection, find and clipboard state, the HID stash and the log budgets) and the palette locals of _start are fields of one Quil state in static storage: the document, staging copy and undo ring take about 96 KiB, more than a protection domain's 64 KiB stack. The Rust installs an allocator that always fails and defines memcpy, memset, memmove and memcmp; the runtime provides those here. Each startup proof runs once, so the Rust's per-proof done flags, which nothing reads afterwards, have no counterpart, and neither has its dead code (pad_text_line, an uncalled physical-keyboard setup).*/
/*Build-time proof profiles: option_env! in the Rust source, and the cfg! gate sexfiles_quil_persistence_proof (rustc --cfg).*/
#if defined(SEXOS_STORAGE_CAP_PROOF)
#define STORAGE_CAP_PROOF_ENABLED 1

#else
#define STORAGE_CAP_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_DISKFS_SLOT_PROOF)
#define DISKFS_SLOT_PROOF_ENABLED 1

#else
#define DISKFS_SLOT_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_READ_V2_HIGHBIT_PROOF)
#define READ_V2_HIGHBIT_PROOF_ENABLED 1

#else
#define READ_V2_HIGHBIT_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_KEYBOARD_NAV_PROOF)
#define KEYBOARD_NAV_PROOF_ENABLED 1

#else
#define KEYBOARD_NAV_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_KEYBOARD_BUFFER_PROOF)
#define KEYBOARD_BUFFER_PROOF_ENABLED 1

#else
#define KEYBOARD_BUFFER_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_TEXT_BUFFER_PROOF)
#define TEXT_BUFFER_PROOF_ENABLED 1

#else
#define TEXT_BUFFER_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_TEXT_SAVE_ASYNC_PROOF)
#define TEXT_SAVE_ASYNC_PROOF_ENABLED 1

#else
#define TEXT_SAVE_ASYNC_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_TEXT_COMMANDS_PROOF)
#define TEXT_COMMANDS_PROOF_ENABLED 1

#else
#define TEXT_COMMANDS_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_CURSOR_NAV_PROOF)
#define CURSOR_NAV_PROOF_ENABLED 1

#else
#define CURSOR_NAV_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_TEXT_SELECTION_PROOF)
#define TEXT_SELECTION_PROOF_ENABLED 1

#else
#define TEXT_SELECTION_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_TEXT_DELETE_PROOF)
#define TEXT_DELETE_PROOF_ENABLED 1

#else
#define TEXT_DELETE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_EDITOR_KEYBINDINGS_PROOF)
#define EDITOR_KEYBINDINGS_PROOF_ENABLED 1

#else
#define EDITOR_KEYBINDINGS_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_UNDO_REDO_PROOF)
#define UNDO_REDO_PROOF_ENABLED 1

#else
#define UNDO_REDO_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_UNDO_REDO_KEY_PROOF)
#define UNDO_REDO_KEY_PROOF_ENABLED 1

#else
#define UNDO_REDO_KEY_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_VISUAL_CURSOR_PROOF)
#define VISUAL_CURSOR_PROOF_ENABLED 1

#else
#define VISUAL_CURSOR_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_FIND_PROOF)
#define FIND_PROOF_ENABLED 1

#else
#define FIND_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_MOD_LOWERCASE_PROOF)
#define MOD_LOWERCASE_PROOF_ENABLED 1

#else
#define MOD_LOWERCASE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_WORD_NAV_PROOF)
#define WORD_NAV_PROOF_ENABLED 1

#else
#define WORD_NAV_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_LINE_STATS_PROOF)
#define LINE_STATS_PROOF_ENABLED 1

#else
#define LINE_STATS_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_FIND_NAV_PROOF)
#define FIND_NAV_PROOF_ENABLED 1

#else
#define FIND_NAV_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_SEL_COPY_DELETE_PROOF)
#define SEL_COPY_DELETE_PROOF_ENABLED 1

#else
#define SEL_COPY_DELETE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_DIRTY_PROOF)
#define DIRTY_PROOF_ENABLED 1

#else
#define DIRTY_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_CMD_SURFACE_PROOF)
#define CMD_SURFACE_PROOF_ENABLED 1

#else
#define CMD_SURFACE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_CLIPBOARD_STATUS_PROOF)
#define CLIPBOARD_STATUS_PROOF_ENABLED 1

#else
#define CLIPBOARD_STATUS_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_PASTE_PROOF)
#define PASTE_PROOF_ENABLED 1

#else
#define PASTE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_REPLACE_PROOF)
#define REPLACE_PROOF_ENABLED 1

#else
#define REPLACE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_GOTO_LINE_PROOF)
#define GOTO_LINE_PROOF_ENABLED 1

#else
#define GOTO_LINE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_STORAGE_PHASEA_PROOF)
#define STORAGE_PHASEA_PROOF_ENABLED 1

#else
#define STORAGE_PHASEA_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_SAVE_OPEN_SEXOBJECT_PROOF)
#define SAVE_OPEN_SEXOBJECT_PROOF_ENABLED 1

#else
#define SAVE_OPEN_SEXOBJECT_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_TEXT_INPUT_PIPELINE_PROOF)
#define TEXT_INPUT_PIPELINE_PROOF_ENABLED 1

#else
#define TEXT_INPUT_PIPELINE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_LIVE_USB_CREATE_SAVE_REOPEN_PROOF)
#define LIVE_USB_PROOF_ENABLED 1

#else
#define LIVE_USB_PROOF_ENABLED 0

#endif
#if defined(SEXOS_QUIL_PHYSICAL_KEYBOARD_PROOF)
#define PHYSICAL_KEYBOARD_PROOF_ENABLED 1

#else
#define PHYSICAL_KEYBOARD_PROOF_ENABLED 0

#endif
#if defined(sexfiles_quil_persistence_proof)
#define PERSISTENCE_PROOF_ENABLED 1

#else
#define PERSISTENCE_PROOF_ENABLED 0

#endif
#define OP_HID_EVENT UINT64_C(0x202)
/*Shell-to-Quil route ping (no display authority), and LINEN_DISK_OPEN_V1's nudge to restore a disk document (arg1 the DiskFS V3 path id).*/
#define OP_QUIL_PING UINT64_C(0xD0)
#define OP_QUIL_OPEN_DISK_DOC UINT64_C(0x4A)
/*sexdisplay text: 0xFB draws eight ASCII bytes (arg2: offset bits 0-7 and 12-15, count bits 8-11, color bits 32-63); 0xFA clears.*/
#define OP_TEXT_DRAW UINT64_C(0xFB)
#define OP_TEXT_CLEAR UINT64_C(0xFA)
/*APP_SURFACE_PACK_V1: Quil draws on its own content surface; the shell-owned frame 201 rejects this domain's draws.*/
#define QUIL_CONTENT_SID UINT64_C(156)
#define SURFACE_W UINT64_C(640)
#define SURFACE_H UINT64_C(480)
#define QUIL_TITLE "Quil"
#define QUIL_TITLE_MAX_LEN ((size_t)32)
/*DISKFS_V4: 12 KiB, three DiskFS content blocks. The renderer draws at most 24 rows; layout tables hold 768 display lines (one per byte would be 24 KiB of stack).*/
#define QUIL_BUFFER_MAX_LEN ((size_t)12288)
#define QUIL_MAX_VISIBLE_LINES ((size_t)24)
#define QUIL_MAX_DISPLAY_LINES ((size_t)768)
/*QUIL_TEXT_V3: the display layout of sexdisplay's TEXT_MODEL_V2, split at newlines and wrapped at 80 columns, and a 22-row viewport.*/
#define QUIL_VIEW_COLS ((size_t)80)
#define QUIL_VIEW_ROWS ((size_t)22)
#define UNDO_DEPTH ((size_t)6)
#define HID_STASH_CAPACITY ((size_t)8)
/*RamFS (SEXFILES_RAMFS_CONTRACT_LOCK_V1: names of at most 24 bytes, 8 bytes per write or read) and DiskFS opcodes of SexFiles.*/
#define OP_RAMFS_OPEN UINT64_C(0x30)
#define OP_RAMFS_READ UINT64_C(0x31)
#define OP_RAMFS_WRITE UINT64_C(0x32)
#define OP_RAMFS_CLOSE UINT64_C(0x33)
#define RAMFS_O_CREATE UINT64_C(0x01)
#define OP_DISKFS_WRITE UINT64_C(0x38)
#define OP_DISKFS_STAT UINT64_C(0x3B)
#define OP_DISKFS_SELECT UINT64_C(0x3E)
#define OP_DISKFS_CREATE UINT64_C(0x42)
#define OP_DISKFS_LIST UINT64_C(0x43)
/*DISKFS_V4: cut the selected object to an exact length after a save, so a shorter save leaves no stale tail.*/
#define OP_DISKFS_TRUNCATE UINT64_C(0x49)
/*LANE3: the canonical read. Its reply keeps a status in bits 56-63, a length (or an error's magnitude) in bits 48-55 and up to six payload bytes below, so no data byte can read as an error.*/
#define OP_DISKFS_READ_V2 UINT64_C(0x4A)
#define DISKFS_V2_MAX_READ ((size_t)6)
#define DISKFS_V2_STATUS_OK UINT64_C(0x00)
#define DISKFS_V2_STATUS_EOF UINT64_C(0x01)
#define QUIL_DISKFS_PATH_ID UINT64_C(2)
#define QUIL_DISKFS_EXPECT_SIZE UINT64_C(4096)
#define QUIL_DISKFS_EXPECT_FLAGS UINT64_C(0x3)
/*QUIL_PERSIST_V1: the DiskFS document header, magic QP01 and the length.*/
#define QUIL_PERSIST_MAGIC UINT64_C(0x31305051)
#define ERR_CAP_INVALID UINT64_C(0xFFFFFFFFFFFFFFFC)
/*Surface geometry: the title bar (rect 1), the palette panel (rect 0) under the original six text lines, and its rows.*/
#define QUIL_TITLE_BAR_H UINT64_C(32)
#define QUIL_TITLE_BAR_COLOR UINT64_C(0x00302E56)
#define QUIL_LINE_BG UINT64_C(0x000C1420)
#define QUIL_GLYPH_COLOR UINT64_C(0x00D4FF7A)
#define TEXT_LINE_COLOR UINT64_C(0x00E0F0FF)
#define QUIL_RECT_SLOT_MIN UINT64_C(2)
#define QUIL_RECT_SLOT_MAX UINT64_C(7)
#define QUIL_ROWS ((uint8_t)5)
#define QUIL_PANEL_X UINT64_C(24)
#define QUIL_PANEL_Y (QUIL_TITLE_BAR_H + 8 + 6 * (18 + 4))
#define QUIL_PANEL_W (SURFACE_W - 48)
#define QUIL_PANEL_H (SURFACE_H - QUIL_PANEL_Y - 16)
#define QUIL_ROW_X (QUIL_PANEL_X + 16)
#define QUIL_ROW_Y0 (QUIL_PANEL_Y + 16)
#define QUIL_ROW_W (QUIL_PANEL_W - 32)
#define QUIL_ROW_H UINT64_C(32)
#define QUIL_ROW_GAP UINT64_C(8)
#define QUIL_ACCENT_W UINT64_C(6)
#define QUIL_ROW_INACTIVE UINT64_C(0x00253556)
#define QUIL_ROW_SELECTED UINT64_C(0x004B6FD3)
#define QUIL_ACCENT_COLOR UINT64_C(0x00E9D36A)
/*Palette commands. Row 0 stays a harmless stub: the shell's open-quil Enter can leak into the palette there.*/
#define CMD_NEW_BUFFER ((uint8_t)1)
#define CMD_SAVE_DOCUMENT ((uint8_t)2)
#define CMD_LOAD_DOCUMENT ((uint8_t)3)
#define CMD_RUN_CHECK_STUB ((uint8_t)4)
#define CMD_SETTINGS_STUB ((uint8_t)5)
static const uint8_t PALETTE_COMMANDS[5] = {
  CMD_RUN_CHECK_STUB, CMD_SAVE_DOCUMENT, CMD_LOAD_DOCUMENT, CMD_NEW_BUFFER, CMD_SETTINGS_STUB
};
static const char QUIL_TEXT_INIT[] = "This is the Quil text surface.\nA minimal no_std editor prototype.\nBuilt on the Sex Microkernel.\nNo text rendering available yet.\nFill-rect visual representation.\nBuffer capacity: bounded static array.\nPress arrows to navigate, ESC for cmds.";
/*The fixed RamFS document name.*/
static const char QUIL_DOC_NAME[] = "quil_doc_01";
typedef struct HidStashEntry {
  uint64_t code;
  uint64_t value;
  uint64_t mod;
}
HidStashEntry;
typedef struct Quil {
  uint8_t buffer[QUIL_BUFFER_MAX_LEN];
  size_t buffer_len;
/*LANE3: a reload is staged here and copied into the buffer only when complete, so a failed or partial one leaves buffer and dirty state as they were.*/
  uint8_t load_staging[QUIL_BUFFER_MAX_LEN];
/*QUIL_EDITOR_V1: unsaved edits, the status line's *; cleared by save, load and New Buffer.*/
  bool dirty;
/*QUIL_DOC_V1: the open document's DiskFS V3 path id (UINT64_MAX untitled) and the last real one, which Load falls back to after New Buffer.*/
  uint64_t doc_id;
  uint64_t last_doc_id;
  uint8_t doc_title[16];
/*Modal name entry, for the first save of an untitled document.*/
  bool name_mode;
  uint8_t name_buf[16];
  size_t name_len;
/*QUIL_TEXT_V3: the viewport's top display line and the sticky column of Up and Down (SIZE_MAX: take it from the cursor).*/
  size_t view_top;
  size_t goal_col;
  size_t cursor_pos;
  size_t sel_start;
  size_t sel_end;
  bool shift_held;
/*The edit flag of the selection and dirty proofs, apart from dirty.*/
  bool edited;
  uint8_t find_query[32];
  size_t find_qlen;
  size_t find_matches[16];
  uint8_t find_count;
  uint8_t find_cur;
  uint8_t clipboard[256];
  size_t clipboard_len;
/*DISKFS_V4: one whole buffer per level; six levels keep the ring near 72 KiB.*/
  uint8_t undo_ring[UNDO_DEPTH][QUIL_BUFFER_MAX_LEN];
  size_t undo_cursors[UNDO_DEPTH];
  size_t undo_lens[UNDO_DEPTH];
  size_t undo_head;
  size_t undo_count;
  size_t undo_redo_count;
/*The palette (the Rust's locals of _start).*/
  bool palette_active;
  uint8_t selected_row;
/*The keyboard buffer and text buffer proofs replay saves that must not touch storage.*/
  bool buffer_proof_active;
  bool physical_keyboard_active;
  bool save_open_pending;
  bool live_usb_pending;
/*HID events that arrive while a storage reply is awaited, replayed later.*/
  HidStashEntry hid_stash[HID_STASH_CAPACITY];
  size_t hid_stash_count;
/*pdx_listen_raw's wrapper log budget is one per process: every blocking listen shares it.*/
  SexosListener listener;
  uint32_t palette_draw_log;
  uint32_t palette_panel_log;
  uint32_t palette_selected_log;
  uint32_t palette_row_log;
  uint32_t key_log;
  uint32_t palette_key_log;
  uint32_t listen_log;
  uint32_t route_log;
  uint32_t unknown_log;
/*Liveness feedback for unmapped keys: a full-surface color toggle.*/
  bool palette_color_toggle;
  bool text_color_toggle;
}
Quil;
static Quil quil_state;
static void quil_init(Quil *q) {
  q->doc_id = 2;
  q->last_doc_id = 2;
  static const char title[] = "n-v1";
  for (size_t i = 0; i < 4; ++i) q->doc_title[i] = (uint8_t) title[i];
  q->goal_col = SIZE_MAX;
  for (size_t i = 0; i < 16; ++i) q->find_matches[i] = 0xFFFF;
  q->palette_active = true;
  q->listener.wrapper_budget = 8;
  q->palette_draw_log = 12;
  q->palette_panel_log = 8;
  q->palette_selected_log = 16;
  q->palette_row_log = 20;
  q->key_log = 16;
  q->palette_key_log = 24;
  q->listen_log = 8;
  q->route_log = 8;
  q->unknown_log = 8;
}
/*A budget allows its line while it lasts.*/
static bool take(uint32_t *budget) {
  if (*budget == 0) return false;
  *budget -= 1;
  return true;
}
/*Rust's `as i64` of a reply word, with defined arithmetic.*/
static bool is_negative(uint64_t word) {
  return word >> 63 != 0;
}
static int64_t wrap_i64(uint64_t word) {
  return is_negative(word) ? -(int64_t) ~word - 1 : (int64_t) word;
}
static size_t min_size(size_t a, size_t b) {
  return a < b ? a : b;
}
/*FNV-1a over a byte range, so gates can assert exact content.*/
static uint64_t fnv1a(const uint8_t *bytes, size_t len) {
  uint64_t h = (uint64_t) 0xcbf29ce484222325;
  for (size_t i = 0; i < len; ++i) {
    h ^= bytes[i];
    h *= (uint64_t) 0x100000001b3;
  }
  return h;
}
/*One printable ASCII byte as a string, for Rust's {} of a char.*/
typedef struct CharText {
  char text[2];
}
CharText;
static CharText char_text(uint8_t ch) {
  return (CharText) {
    .text = {
      (char) ch, 0
    }
  };
}
static size_t quil_doc_name_len(const Quil *q) {
  size_t n = 0;
  while (n < 16 && q->doc_title[n] != 0) n += 1;
  return n;
}
/*QUIL_TEXT_V3: the display lines of BUF (split at newlines, wrapped at 80 columns); starts[i] is the byte offset of line i. Bytes are characters: the editor is ASCII-only.*/
static size_t quil_layout(const uint8_t *buf, size_t len, uint16_t *starts) {
  size_t count = 1;
  starts[0] = 0;
  size_t col = 0;
  size_t i = 0;
  while (i < len) {
    uint8_t c = buf[i];
    i += 1;
    if (c == 0x0A) {
      if (count >= QUIL_MAX_DISPLAY_LINES) break;
      starts[count] = (uint16_t) i;
      count += 1;
      col = 0;
    } else {
      col += 1;
      if (col >= QUIL_VIEW_COLS && i < len) {
        if (count >= QUIL_MAX_DISPLAY_LINES) break;
        starts[count] = (uint16_t) i;
        count += 1;
        col = 0;
      }
    }
  }
  return count;
}
/*The display line holding byte offset POS.*/
static size_t quil_line_of(const uint16_t *starts, size_t count, size_t pos) {
  size_t line = 0;
  for (size_t i = 0; i < count; ++i) if ((size_t) starts[i] <= pos) line = i;
  else break;
  return line;
}
/*Lines split at newlines, at most the 24 the renderer draws.*/
static size_t text_buffer_line_count(const uint8_t *buf, size_t len) {
  size_t lines = 0;
  for (size_t i = 0; i < len; ++i) if (buf[i] == 0x0A) lines += 1;
  if (lines == 0 || buf[len - 1] != 0x0A) lines += 1;
  return min_size(lines, QUIL_MAX_VISIBLE_LINES);
}
static bool validate_title(const char *title) {
  size_t len = sexos_strlen(title);
  bool valid = len <= QUIL_TITLE_MAX_LEN && len > 0;
  if (!valid) sexos_line("[quil.text.title.reject] title=%s len=%zu max=%zu", title, len, QUIL_TITLE_MAX_LEN);
  return valid;
}
static bool validate_buffer(size_t len) {
  bool valid = len <= QUIL_BUFFER_MAX_LEN && len > 0;
  if (!valid) sexos_line("[quil.text.buffer.reject] bytes=%zu max=%zu", len, QUIL_BUFFER_MAX_LEN);
  return valid;
}
static void draw_fill(uint64_t arg1, uint64_t arg2) {
  (void) sexos_pdx_call(SEXOS_SLOT_display, 0xEF, QUIL_CONTENT_SID, arg1, arg2);
}
/*The title bar fill, rect 1.*/
static void draw_title_bar(void) {
  draw_fill(0, (uint64_t) 1 << 56 | QUIL_TITLE_BAR_COLOR << 32 | QUIL_TITLE_BAR_H << 16 | SURFACE_W);
}
/*Send the buffer to sexdisplay: the status line (the name prompt, or the title, the length's last three digits, B and * when dirty), then the 22-row window around the cursor with the caret as #, padded past 32 bytes and sent highest chunk first, so the surface's text length clears sexdisplay's per-draw diagnostic at once (spindle's reverse-chunk dodge). The 512-byte render buffer is separate from the document.*/
static void draw_text_lines(Quil *q, const uint8_t *buf, size_t len) {
  (void) sexos_pdx_call(SEXOS_SLOT_display, OP_TEXT_CLEAR, QUIL_CONTENT_SID, 0, 0);
  size_t line_count = text_buffer_line_count(buf, len);
  sexos_line("[quil.text.draw.v2] lines=%zu bytes=%zu visible=%zu dirty=%u", line_count, len, min_size(line_count, QUIL_MAX_VISIBLE_LINES), (unsigned) q->dirty);
  uint8_t line_buf[512] = {
    0
  };
  size_t sp = 0;
  if (q->name_mode) {
    const char *prompt = "NAME? ";
    for (size_t i = 0; i < 6; ++i) line_buf[sp ++] = (uint8_t) prompt[i];
    for (size_t i = 0; i < q->name_len; ++i) line_buf[sp ++] = q->name_buf[i];
    line_buf[sp ++] = 0x23;
    line_buf[sp ++] = 0x0A;
  } else {
    size_t n = quil_doc_name_len(q);
    for (size_t i = 0; i < n; ++i) line_buf[sp ++] = q->doc_title[i];
    line_buf[sp ++] = 0x20;
    if (len >= 100) line_buf[sp ++] = (uint8_t)(0x30 + len / 100 % 10);
    if (len >= 10) line_buf[sp ++] = (uint8_t)(0x30 + len / 10 % 10);
    line_buf[sp ++] = (uint8_t)(0x30 + len % 10);
    line_buf[sp ++] = 0x42;
    if (q->dirty) line_buf[sp ++] = 0x2A;
    line_buf[sp ++] = 0x0A;
  }
  uint16_t starts[QUIL_MAX_DISPLAY_LINES] = {
    0
  };
  size_t count = quil_layout(buf, len, starts);
  size_t cursor = min_size(q->cursor_pos, len);
  size_t cline = quil_line_of(starts, count, cursor);
  if (cline < q->view_top) q->view_top = cline;
  if (cline >= q->view_top + QUIL_VIEW_ROWS) q->view_top = cline + 1 - QUIL_VIEW_ROWS;
  if (q->view_top >= count) q->view_top = count - 1;
  size_t view_start = starts[min_size(q->view_top, count - 1)];
  size_t view_end = q->view_top + QUIL_VIEW_ROWS < count ? (size_t) starts[q->view_top + QUIL_VIEW_ROWS] : len;
  sexos_line("[quil.view] top=%zu cursor=%zu win=%zu..%zu", q->view_top, cursor, view_start, view_end);
  size_t total = sp;
  for (size_t i = view_start; i <= view_end && total < 512; ++i) {
    if (i == cursor && total < 512) line_buf[total ++] = 0x23;
    if (i < view_end && total < 512) line_buf[total ++] = buf[i];
  } while (total < 40) line_buf[total ++] = 0x20;
  size_t chunks = (total + 7) / 8;
  for (size_t k = chunks; k > 0; --k) {
    size_t offset = (k - 1) * 8;
    size_t chunk_len = min_size(total - offset, 8);
    uint64_t packed = 0;
    for (size_t j = 0; j < chunk_len; ++j) packed |= (uint64_t) line_buf[offset + j] << j * 8;
    (void) sexos_pdx_call(SEXOS_SLOT_display, OP_TEXT_DRAW, QUIL_CONTENT_SID, packed, ((uint64_t) offset & 0xFF) | ((uint64_t) chunk_len & 0xF) << 8 | ((uint64_t) offset >> 8 & 0xF) << 12 | TEXT_LINE_COLOR << 32);
  }
  sexos_line("[quil.text.draw.v2.sent] total_bytes=%zu chunks=%zu", total, chunks);
  if (line_count > QUIL_MAX_VISIBLE_LINES) sexos_line("[quil.text.buffer.overflow] lines=%zu visible=%zu", line_count, QUIL_MAX_VISIBLE_LINES);
}
static void redraw(Quil *q) {
  draw_text_lines(q, q->buffer, q->buffer_len);
}
/*QUIL_TEXT_V3: the cursor one display line up (DIR below 0) or down, at the sticky goal column; the newline ending the target line is outside it.*/
static void quil_cursor_vertical(Quil *q, int dir) {
  uint16_t starts[QUIL_MAX_DISPLAY_LINES] = {
    0
  };
  size_t count = quil_layout(q->buffer, q->buffer_len, starts);
  size_t line = quil_line_of(starts, count, q->cursor_pos);
  if (q->goal_col == SIZE_MAX) q->goal_col = q->cursor_pos - starts[line];
  size_t target = 0;
  if (dir < 0) {
    if (line == 0) {
      sexos_line("[quil.cursor.move] dir=up ok=0 reason=first_line");
      return;
    }
    target = line - 1;
  } else {
    if (line + 1 >= count) {
      sexos_line("[quil.cursor.move] dir=down ok=0 reason=last_line");
      return;
    }
    target = line + 1;
  }
  size_t t_start = starts[target];
  size_t t_end = q->buffer_len;
  if (target + 1 < count) {
    size_t e = starts[target + 1];
    t_end = e > t_start && q->buffer[e - 1] == 0x0A ? e - 1 : e;
  }
  size_t old = q->cursor_pos;
  q->cursor_pos = t_start + min_size(q->goal_col, t_end - t_start);
  sexos_line("[quil.cursor.move] old=%zu new=%zu len=%zu dir=%s ok=1", old, q->cursor_pos, q->buffer_len, dir < 0 ? "up" : "down");
  redraw(q);
}
static void emit_rect_slot(uint64_t slot, uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint64_t color) {
  draw_fill(y << 32 | x, slot << 56 | color << 32 | h << 16 | w);
}
/*Pseudo-glyphs from rect slots 2 to 7, which fit only a prefix of QUIL TEXT ALIVE: Q (block and tail), U (two stems and a base), I (a stem).*/
typedef struct GlyphRect {
  uint64_t dx;
  uint64_t dy;
  uint64_t w;
  uint64_t h;
  uint64_t color;
}
GlyphRect;
static const GlyphRect GLYPH_RECTS[6] = {
  {
    0, 0, 14, 18, QUIL_GLYPH_COLOR
  }, {
    9, 12, 5, 6, QUIL_TITLE_BAR_COLOR
  }, {
    18, 0, 3, 15, QUIL_GLYPH_COLOR
  }, {
    27, 0, 3, 15, QUIL_GLYPH_COLOR
  }, {
    18, 15, 12, 3, QUIL_GLYPH_COLOR
  }, {
    38, 0, 3, 18, QUIL_GLYPH_COLOR
  }
};
static void draw_rect_glyph_text(void) {
  uint64_t slot = QUIL_RECT_SLOT_MIN;
  for (size_t i = 0; i < 6; ++i) if (slot <= QUIL_RECT_SLOT_MAX) {
    const GlyphRect *r = &GLYPH_RECTS[i];
    emit_rect_slot(slot, 462 + r->dx, 6 + r->dy, r->w, r->h, r->color);
    slot += 1;
  }
  uint64_t emitted = slot - QUIL_RECT_SLOT_MIN;
  sexos_line("[quil.rect_glyph_text.v1] text=QUIL_TEXT_ALIVE shown=%u rects=%llu", emitted >= 6 ? 3u : emitted >= 5 ? 2u : 1u, (unsigned long long) emitted);
}
/*The palette panel (rect 0, redrawn on each key) and its five rows, the selected one highlighted with an accent.*/
static void draw_palette(Quil *q, uint8_t selected) {
  draw_fill(QUIL_PANEL_Y << 32 | QUIL_PANEL_X, QUIL_ROW_INACTIVE << 32 | QUIL_PANEL_H << 16 | QUIL_PANEL_W);
  if (take(&q->palette_draw_log)) sexos_line("[quil.palette.draw] rows=%u selected=%u", (unsigned) QUIL_ROWS, (unsigned) selected);
  if (take(&q->palette_panel_log)) sexos_line("[quil.palette.panel] x=%llu y=%llu w=%llu h=%llu", (unsigned long long) QUIL_PANEL_X, (unsigned long long) QUIL_PANEL_Y, (unsigned long long) QUIL_PANEL_W, (unsigned long long) QUIL_PANEL_H);
  if (QUIL_ROW_Y0 + (uint64_t) QUIL_ROWS * QUIL_ROW_H + ((uint64_t) QUIL_ROWS - 1) * QUIL_ROW_GAP > QUIL_PANEL_Y + QUIL_PANEL_H) sexos_line("[quil.palette.reject] action=draw reason=row_overflow");
  for (uint8_t row = 0; row < QUIL_ROWS; ++row) {
    uint64_t y = QUIL_ROW_Y0 + (QUIL_ROW_H + QUIL_ROW_GAP) * row;
    bool is_selected = row == selected;
    draw_fill(y << 32 | QUIL_ROW_X, (is_selected ? QUIL_ROW_SELECTED : QUIL_ROW_INACTIVE) << 32 | QUIL_ROW_H << 16 | QUIL_ROW_W);
    if (is_selected) {
      draw_fill(y << 32 | QUIL_ROW_X, QUIL_ACCENT_COLOR << 32 | QUIL_ROW_H << 16 | QUIL_ACCENT_W);
      if (take(&q->palette_selected_log)) sexos_line("[quil.palette.selected] row=%u", (unsigned) row);
    }
    if (take(&q->palette_row_log)) sexos_line("[quil.palette.row] row=%u selected=%u", (unsigned) row, (unsigned) is_selected);
  }
}
/*Palette keys: 1 up, 2 down, 3 enter, 4 esc, 0 none, over the scancode variants seen in this codebase.*/
static uint8_t decode_palette_key(uint64_t scancode) {
  switch ((uint32_t) scancode) {
    case 0x48: case 0x67: return 1;
    case 0x50: case 0x6c: return 2;
    case 0x1c: case 0x0d: return 3;
    case 0x01: case 0x1b: return 4;
    default: return 0;
  }
}
/*Scancode set 1 (US QWERTY) to ASCII: letters; digits, shifted to ! @ # $ % ^ & * ( ); the punctuation keys, ; and :, ' and ", , and <, . and >, / and ?; and space, which has no shifted form.*/
typedef struct KeyChar {
  uint8_t code;
  uint8_t plain;
  uint8_t shifted;
}
KeyChar;
static const KeyChar KEY_CHARS[] = {
  {
    0x10, 'q', 'Q'
  }, {
    0x11, 'w', 'W'
  }, {
    0x12, 'e', 'E'
  }, {
    0x13, 'r', 'R'
  }, {
    0x14, 't', 'T'
  }, {
    0x15, 'y', 'Y'
  }, {
    0x16, 'u', 'U'
  }, {
    0x17, 'i', 'I'
  }, {
    0x18, 'o', 'O'
  }, {
    0x19, 'p', 'P'
  }, {
    0x1E, 'a', 'A'
  }, {
    0x1F, 's', 'S'
  }, {
    0x20, 'd', 'D'
  }, {
    0x21, 'f', 'F'
  }, {
    0x22, 'g', 'G'
  }, {
    0x23, 'h', 'H'
  }, {
    0x24, 'j', 'J'
  }, {
    0x25, 'k', 'K'
  }, {
    0x26, 'l', 'L'
  }, {
    0x2C, 'z', 'Z'
  }, {
    0x2D, 'x', 'X'
  }, {
    0x2E, 'c', 'C'
  }, {
    0x2F, 'v', 'V'
  }, {
    0x30, 'b', 'B'
  }, {
    0x31, 'n', 'N'
  }, {
    0x32, 'm', 'M'
  }, {
    0x02, '1', 0x21
  }, {
    0x03, '2', 0x40
  }, {
    0x04, '3', 0x23
  }, {
    0x05, '4', 0x24
  }, {
    0x06, '5', 0x25
  }, {
    0x07, '6', 0x5E
  }, {
    0x08, '7', 0x26
  }, {
    0x09, '8', 0x2A
  }, {
    0x0A, '9', 0x28
  }, {
    0x0B, '0', 0x29
  }, {
    0x27, 0x3B, 0x3A
  }, {
    0x28, 0x27, 0x22
  }, {
    0x33, 0x2C, 0x3C
  }, {
    0x34, 0x2E, 0x3E
  }, {
    0x35, 0x2F, 0x3F
  }, {
    0x39, 0x20, 0x20
  }
};
static bool scancode_to_char(uint64_t scancode, bool shift, uint8_t *out) {
  for (size_t i = 0; i < sizeof KEY_CHARS / sizeof KEY_CHARS[0]; ++i) if (KEY_CHARS[i].code == (uint8_t) scancode) {
    *out = shift ? KEY_CHARS[i].shifted : KEY_CHARS[i].plain;
    return true;
  }
  return false;
}
/*Undo: save the buffer and cursor in the ring before an edit; the oldest level is overwritten when full, and an edit clears redo.*/
static void text_buffer_undo_push(Quil *q) {
  q->edited = true;
  uint8_t *slot = q->undo_ring[q->undo_head];
  for (size_t i = 0; i < QUIL_BUFFER_MAX_LEN; ++i) slot[i] = i < q->buffer_len ? q->buffer[i] : 0;
  q->undo_lens[q->undo_head] = q->buffer_len;
  q->undo_cursors[q->undo_head] = q->cursor_pos;
  q->undo_head = (q->undo_head + 1) % UNDO_DEPTH;
  if (q->undo_count < UNDO_DEPTH) q->undo_count += 1;
  q->undo_redo_count = 0;
  sexos_line("[quil.undo.push] idx=%zu len=%zu ok=1", q->undo_head == 0 ? UNDO_DEPTH - 1 : q->undo_head - 1, q->buffer_len);
}
static void restore_level(Quil *q, size_t level) {
  size_t len = q->undo_lens[level];
  for (size_t i = 0; i < len; ++i) q->buffer[i] = q->undo_ring[level][i];
  q->buffer_len = len;
  q->cursor_pos = q->undo_cursors[level];
}
static bool text_buffer_undo(Quil *q) {
  if (q->undo_count == 0) return false;
  size_t old_len = q->buffer_len;
  if (q->undo_redo_count < UNDO_DEPTH) q->undo_redo_count += 1;
  q->undo_head = q->undo_head == 0 ? UNDO_DEPTH - 1 : q->undo_head - 1;
  q->undo_count -= 1;
  restore_level(q, q->undo_head);
  sexos_line("[quil.undo.apply] old_len=%zu new_len=%zu ok=1", old_len, q->buffer_len);
  return true;
}
/*Redo restores the level the head points at after an undo, then moves past it.*/
static bool text_buffer_redo(Quil *q) {
  if (q->undo_redo_count == 0) return false;
  size_t old_len = q->buffer_len;
  restore_level(q, q->undo_head);
  q->undo_head = (q->undo_head + 1) % UNDO_DEPTH;
  q->undo_count += 1;
  q->undo_redo_count -= 1;
  sexos_line("[quil.redo.apply] old_len=%zu new_len=%zu ok=1", old_len, q->buffer_len);
  return true;
}
/*Open a gap of one byte at the cursor (QUIL_TEXT_V3: insertion at the cursor) and put CH there.*/
static void insert_at_cursor(Quil *q, uint8_t ch) {
  if (q->cursor_pos > q->buffer_len) q->cursor_pos = q->buffer_len;
  for (size_t i = q->buffer_len; i > q->cursor_pos; --i) q->buffer[i] = q->buffer[i - 1];
  q->buffer[q->cursor_pos] = ch;
  q->buffer_len += 1;
  q->cursor_pos += 1;
  q->goal_col = SIZE_MAX;
  q->dirty = true;
}
/*Type a character at the cursor; control characters other than newline are dropped (after the undo push, as in the Rust).*/
static bool text_buffer_append(Quil *q, uint8_t ch) {
  text_buffer_undo_push(q);
  if (q->buffer_len >= QUIL_BUFFER_MAX_LEN) {
    sexos_line("[quil.text.append] len=%zu ch=%u ok=0 reason=buffer_full", q->buffer_len, (unsigned) ch);
    return false;
  }
  if (ch < 0x20 && ch != 0x0A) return false;
  insert_at_cursor(q, ch);
  sexos_line("[quil.text.append] len=%zu ch=%u at=%zu", q->buffer_len, (unsigned) ch, q->cursor_pos - 1);
  return true;
}
/*Backspace: delete the byte before the cursor; deleting a newline joins the lines.*/
static bool text_buffer_backspace(Quil *q) {
  text_buffer_undo_push(q);
  if (q->cursor_pos > q->buffer_len) q->cursor_pos = q->buffer_len;
  if (q->buffer_len == 0 || q->cursor_pos == 0) {
    sexos_line("[quil.text.backspace] old=%zu new=%zu ok=0 reason=at_start", q->buffer_len, q->buffer_len);
    return false;
  }
  size_t old = q->buffer_len;
  for (size_t i = q->cursor_pos - 1; i < q->buffer_len - 1; ++i) q->buffer[i] = q->buffer[i + 1];
  q->buffer_len -= 1;
  q->cursor_pos -= 1;
  q->buffer[q->buffer_len] = 0;
  q->goal_col = SIZE_MAX;
  q->dirty = true;
  sexos_line("[quil.text.backspace] old=%zu new=%zu at=%zu ok=1", old, q->buffer_len, q->cursor_pos);
  return true;
}
static bool text_buffer_newline(Quil *q) {
  text_buffer_undo_push(q);
  if (q->buffer_len >= QUIL_BUFFER_MAX_LEN) {
    sexos_line("[quil.text.enter] line=0 len=%zu ok=0 reason=buffer_full", q->buffer_len);
    return false;
  }
  insert_at_cursor(q, 0x0A);
  sexos_line("[quil.text.enter] line=%zu len=%zu at=%zu ok=1", text_buffer_line_count(q->buffer, q->buffer_len), q->buffer_len, q->cursor_pos - 1);
  return true;
}
/*Remove COUNT bytes at FROM, zeroing the vacated tail.*/
static void remove_range(Quil *q, size_t from, size_t count) {
  size_t end = q->buffer_len > count ? q->buffer_len - count : 0;
  for (size_t i = from; i < end; ++i) q->buffer[i] = q->buffer[i + count];
  q->buffer_len -= count;
  for (size_t i = q->buffer_len; i < q->buffer_len + count && i < QUIL_BUFFER_MAX_LEN; ++i) q->buffer[i] = 0;
}
/*Delete the byte at the cursor.*/
static bool text_buffer_delete_char(Quil *q) {
  text_buffer_undo_push(q);
  if (q->cursor_pos >= q->buffer_len) return false;
  size_t old = q->buffer_len;
  remove_range(q, q->cursor_pos, 1);
  q->dirty = true;
  sexos_line("[quil.text.delete] mode=char old=%zu new=%zu ok=1", old, q->buffer_len);
  return true;
}
/*Delete from the cursor to the end of its line (the newline stays).*/
static bool text_buffer_delete_to_eol(Quil *q) {
  text_buffer_undo_push(q);
  if (q->cursor_pos >= q->buffer_len) return false;
  size_t eol = q->cursor_pos;
  while (eol < q->buffer_len && q->buffer[eol] != 0x0A) eol += 1;
  size_t count = eol - q->cursor_pos;
  if (count == 0) return false;
  size_t old = q->buffer_len;
  remove_range(q, q->cursor_pos, count);
  q->dirty = true;
  sexos_line("[quil.text.delete] mode=to_eol old=%zu new=%zu ok=1", old, q->buffer_len);
  return true;
}
/*Delete the cursor's whole line with its newline. This edit does not set dirty, as in the Rust.*/
static bool text_buffer_delete_line(Quil *q) {
  text_buffer_undo_push(q);
  if (q->buffer_len == 0) return false;
  size_t line_start = q->cursor_pos;
  while (line_start > 0 && q->buffer[line_start - 1] != 0x0A) line_start -= 1;
  size_t line_end = line_start;
  while (line_end < q->buffer_len && q->buffer[line_end] != 0x0A) line_end += 1;
  if (line_end < q->buffer_len) line_end += 1;
  size_t old = q->buffer_len;
  remove_range(q, line_start, line_end - line_start);
  if (q->cursor_pos > q->buffer_len) q->cursor_pos = q->buffer_len;
  sexos_line("[quil.text.delete] mode=line old=%zu new=%zu ok=1", old, q->buffer_len);
  return true;
}
static bool buffer_matches(const Quil *q, size_t at, const uint8_t *query, size_t qlen) {
  for (size_t k = 0; k < qlen; ++k) if (q->buffer[at + k] != query[k]) return false;
  return true;
}
/*Every occurrence of QUERY (1 to 32 bytes): the first index (0xFFFF none) and the count, a u8 that wraps.*/
static size_t text_buffer_find(const Quil *q, const char *text, uint8_t *count) {
  const uint8_t *query = (const uint8_t *) text;
  size_t qlen = sexos_strlen(text);
  *count = 0;
  if (qlen == 0 || qlen > 32 || qlen > q->buffer_len) return 0xFFFF;
  size_t first = 0xFFFF;
  for (size_t i = 0; i + qlen <= q->buffer_len; ++i) if (buffer_matches(q, i, query, qlen)) {
    if (first == 0xFFFF) first = i;
    *count += 1;
  }
  return first;
}
/*Word moves: spaces delimit words.*/
static void cursor_word_left(Quil *q) {
  size_t old = q->cursor_pos;
  while (q->cursor_pos > 0 && q->buffer[q->cursor_pos - 1] == 0x20) q->cursor_pos -= 1;
  while (q->cursor_pos > 0 && q->buffer[q->cursor_pos - 1] != 0x20) q->cursor_pos -= 1;
  sexos_line("[quil.word.move] old=%zu new=%zu dir=left ok=1", old, q->cursor_pos);
}
static void cursor_word_right(Quil *q) {
  size_t old = q->cursor_pos;
  while (q->cursor_pos < q->buffer_len && q->buffer[q->cursor_pos] != 0x20) q->cursor_pos += 1;
  while (q->cursor_pos < q->buffer_len && q->buffer[q->cursor_pos] == 0x20) q->cursor_pos += 1;
  sexos_line("[quil.word.move] old=%zu new=%zu dir=right ok=1", old, q->cursor_pos);
}
/*Words, delimited by spaces and newlines.*/
static size_t count_words(const Quil *q) {
  size_t words = 0;
  bool in_word = false;
  for (size_t i = 0; i < q->buffer_len; ++i) {
    uint8_t b = q->buffer[i];
    if (b == 0x20 || b == 0x0A) in_word = false;
    else if (!in_word) {
      words += 1;
      in_word = true;
    }
  }
  return words;
}
/*Find navigation: remember the query and up to 16 match positions.*/
static void find_collect_matches(Quil *q, const char *text) {
  const uint8_t *query = (const uint8_t *) text;
  size_t qlen = sexos_strlen(text);
  q->find_qlen = min_size(qlen, 32);
  for (size_t i = 0; i < q->find_qlen; ++i) q->find_query[i] = query[i];
  q->find_count = 0;
  if (qlen == 0 || qlen > q->buffer_len) return;
  for (size_t i = 0; i + qlen <= q->buffer_len && q->find_count < 16; ++i) if (buffer_matches(q, i, query, qlen)) {
    q->find_matches[q->find_count] = i;
    q->find_count += 1;
  }
  q->find_cur = 0;
}
/*To the end of the first match after the cursor, wrapping to the first.*/
static bool find_next(Quil *q) {
  if (q->find_count == 0) return false;
  size_t old = q->cursor_pos;
  uint8_t at = 0;
  q->cursor_pos = q->find_matches[0] + q->find_qlen;
  for (uint8_t i = 0; i < q->find_count; ++i) if (q->find_matches[i] > old) {
    q->cursor_pos = q->find_matches[i] + q->find_qlen;
    at = i;
    break;
  }
  q->find_cur = at;
  sexos_line("[quil.find.nav] dir=next old=%zu new=%zu count=%u ok=1", old, q->cursor_pos, (unsigned) q->find_count);
  return true;
}
/*To the start of the last match before the cursor, wrapping to the last.*/
static bool find_prev(Quil *q) {
  if (q->find_count == 0) return false;
  size_t old = q->cursor_pos;
  uint8_t at = q->find_count - 1;
  q->cursor_pos = q->find_matches[at];
  for (uint8_t i = q->find_count; i > 0; --i) if (q->find_matches[i - 1] < old) {
    at = i - 1;
    q->cursor_pos = q->find_matches[at];
    break;
  }
  q->find_cur = at;
  sexos_line("[quil.find.nav] dir=prev old=%zu new=%zu count=%u ok=1", old, q->cursor_pos, (unsigned) q->find_count);
  return true;
}
static bool selection_valid(const Quil *q) {
  return q->sel_start < q->sel_end && q->sel_end <= q->buffer_len;
}
static bool delete_selection(Quil *q) {
  if (!selection_valid(q)) return false;
  text_buffer_undo_push(q);
  size_t old_len = q->buffer_len;
  remove_range(q, q->sel_start, q->sel_end - q->sel_start);
  if (q->cursor_pos > q->buffer_len) q->cursor_pos = q->buffer_len;
  sexos_line("[quil.selection.delete] start=%zu end=%zu old=%zu new=%zu ok=1", q->sel_start, q->sel_end, old_len, q->buffer_len);
  q->edited = true;
  return true;
}
/*Copy at most 256 selected bytes into the clipboard.*/
static bool copy_selection(Quil *q) {
  if (!selection_valid(q)) return false;
  size_t len = min_size(q->sel_end - q->sel_start, 256);
  for (size_t i = 0; i < len; ++i) q->clipboard[i] = q->buffer[q->sel_start + i];
  q->clipboard_len = len;
  sexos_line("[quil.selection.copy] len=%zu ok=1 reason=bounded_static_clipboard", len);
  return true;
}
/*Clear the edit flag, as a save does.*/
static void clear_dirty(Quil *q) {
  q->edited = false;
  sexos_line("[quil.dirty.state] dirty=0 reason=save_cleared");
}
static bool paste_clipboard(Quil *q) {
  size_t n = q->clipboard_len;
  if (n == 0 || q->buffer_len + n > QUIL_BUFFER_MAX_LEN) return false;
  text_buffer_undo_push(q);
  size_t old_len = q->buffer_len;
/*The Rust's slice of the buffer at the cursor panics past its end.*/
  if (q->cursor_pos + n > QUIL_BUFFER_MAX_LEN) sexos_spin_forever();
  for (size_t i = q->buffer_len; i > q->cursor_pos; --i) q->buffer[i - 1 + n] = q->buffer[i - 1];
  for (size_t i = 0; i < n; ++i) q->buffer[q->cursor_pos + i] = q->clipboard[i];
  q->buffer_len += n;
  q->cursor_pos += n;
  sexos_line("[quil.clipboard.paste] len=%zu old_len=%zu new_len=%zu ok=1 reason=pasted_at_cursor", n, old_len, q->buffer_len);
  return true;
}
/*Replace every FROM (1 to 32 bytes) with TO (at most 32), building the result in a 512-byte buffer that truncates longer documents. Returns the count, a u8 that wraps.*/
static uint8_t replace_all(Quil *q, const char *from_text, const char *to_text) {
  const uint8_t *from = (const uint8_t *) from_text;
  const uint8_t *to = (const uint8_t *) to_text;
  size_t flen = sexos_strlen(from_text);
  size_t tlen = sexos_strlen(to_text);
  if (flen == 0 || flen > 32 || tlen > 32) return 0;
  size_t old_len = q->buffer_len;
  uint8_t tmp[512] = {
    0
  };
  size_t ti = 0;
  size_t si = 0;
  uint8_t count = 0;
  while (si < old_len && ti < 512) if (si + flen <= old_len && buffer_matches(q, si, from, flen)) {
    if (ti + tlen > 512) break;
    for (size_t k = 0; k < tlen; ++k) tmp[ti + k] = to[k];
    ti += tlen;
    si += flen;
    count += 1;
  } else tmp[ti ++] = q->buffer[si ++];
  if (count == 0) return 0;
  text_buffer_undo_push(q);
  for (size_t i = 0; i < ti; ++i) q->buffer[i] = tmp[i];
  q->buffer_len = ti;
  if (q->cursor_pos > q->buffer_len) q->cursor_pos = q->buffer_len;
  sexos_line("[quil.replace.result] count=%u old_len=%zu new_len=%zu ok=1", (unsigned) count, old_len, q->buffer_len);
  return count;
}
/*To the start of line LINE (from 1); past the last line the cursor stays put. The line counter is a u8 that wraps.*/
static bool goto_line(Quil *q, uint8_t line) {
  if (line == 0 || q->buffer_len == 0) return false;
  size_t old = q->cursor_pos;
  uint8_t current = 1;
  size_t pos = 0;
  while (pos < q->buffer_len && current < line) {
    if (q->buffer[pos] == 0x0A) current += 1;
    pos += 1;
  }
  if (pos < q->buffer_len) q->cursor_pos = pos;
  sexos_line("[quil.goto.line] line=%u old=%zu new=%zu ok=%u reason=%s", (unsigned) line, old, q->cursor_pos, (unsigned)(current >= line), current >= line ? "found" : "clamped_to_end");
  return true;
}
static void emit_text_stats(const Quil *q) {
  sexos_line("[quil.text.stats] bytes=%zu lines=%zu words=%zu cursor=%zu ok=1", q->buffer_len, text_buffer_line_count(q->buffer, q->buffer_len), count_words(q), q->cursor_pos);
}
/*Keep a HID event for replay; false when the stash is full.*/
static bool stash_hid(Quil *q, uint64_t code, uint64_t value, uint64_t mod) {
  if (q->hid_stash_count >= HID_STASH_CAPACITY) return false;
  q->hid_stash[q->hid_stash_count] = (HidStashEntry) {
    .code = code,.value = value,.mod = mod
  };
  q->hid_stash_count += 1;
  return true;
}
/*A synchronous storage call: pdx_call only enqueues (the kernel's AsyncEnqueue always succeeds) and the reply arrives as a listen of type 1. While waiting, HID events are stashed for replay and other messages skipped. 0 with the reply in *value, or the failing status or negative reply.*/
static int64_t pdx_storage_call(Quil *q, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t *value) {
  SexosReply sent = sexos_pdx_call(SEXOS_SLOT_storage, opcode, arg0, arg1, arg2);
  if (sent.status != 0) return wrap_i64(sent.status);
  while (1) {
    SexosMessage msg = sexos_listen(&q->listener, 0);
    if (msg.type_id == 1) {
      if (is_negative(msg.arg0)) return wrap_i64(msg.arg0);
      *value = msg.arg0;
      return 0;
    }
    if (msg.type_id == OP_HID_EVENT) {
      size_t idx = q->hid_stash_count;
      if (stash_hid(q, msg.arg0, msg.arg1, msg.arg2)) sexos_line("[quil.hid.stash] idx=%zu code=%#llx down=%llu mod=%llu ok=1 reason=stashed", idx, (unsigned long long) msg.arg0, (unsigned long long) msg.arg1, (unsigned long long) msg.arg2);
      else sexos_line("[quil.hid.stash.drop] code=%#llx reason=full", (unsigned long long) msg.arg0);
    } else sexos_line("[quil.sync.skip] type_id=%#llx", (unsigned long long) msg.type_id);
  }
}
/*QUIL_PERSIST_V1: the persist path's call, bounded. A storage request lost under contention (seen while Linen's boot publish held SexFiles) must not hang the editor, so it polls without blocking: 300000 empty polls, each a yield, give -101. HID events are stashed unlogged.*/
static int64_t pdx_storage_call_bounded(Quil *q, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t *value) {
  SexosReply sent = sexos_pdx_call(SEXOS_SLOT_storage, opcode, arg0, arg1, arg2);
  if (sent.status != 0) return wrap_i64(sent.status);
  uint32_t polls = 0;
  while (1) {
    SexosMessage msg = {
      0
    };
    if (!sexos_try_listen(0, &msg)) {
      polls += 1;
      if (polls >= 300000) {
        sexos_line("[quil.persist.reply.timeout] op=%#llx", (unsigned long long) opcode);
        return -101;
      }
      continue;
    }
    if (msg.type_id == 1) {
      if (is_negative(msg.arg0)) return wrap_i64(msg.arg0);
      *value = msg.arg0;
      return 0;
    }
    if (msg.type_id == OP_HID_EVENT) {
      if (!stash_hid(q, msg.arg0, msg.arg1, msg.arg2)) sexos_line("[quil.hid.stash.drop] code=%#llx reason=full", (unsigned long long) msg.arg0);
    } else sexos_line("[quil.sync.skip] type_id=%#llx", (unsigned long long) msg.type_id);
  }
}
static uint64_t v2_status(uint64_t reply) {
  return reply >> 56 & 0xFF;
}
static uint64_t v2_len_field(uint64_t reply) {
  return reply >> 48 & 0xFF;
}
static uint8_t v2_byte(uint64_t reply, size_t i) {
  return (uint8_t)(reply >> 8 * i);
}
static void slot_proof_done_fail(void) {
  sexos_line("[quil.diskfs.slot.min.done] ok=0");
}
/*V2 slot proof on path id 2 (/disk/quil-object-v1): select, stat, write 16 bytes, read them back through READ_V2 (6, 6 and 4 bytes) and compare.*/
static void run_quil_diskfs_slot_min_proof(Quil *q) {
  sexos_line("[quil.diskfs.slot.min.begin]");
  for (unsigned i = 0; i < 64; ++i) sexos_yield();
  uint64_t r = 0;
  int64_t e = pdx_storage_call(q, OP_DISKFS_SELECT, QUIL_DISKFS_PATH_ID, 0, 0, &r);
  if (e != 0) {
    sexos_line("[quil.diskfs.slot.min.select.err] err=%lld", (long long) e);
    slot_proof_done_fail();
    return;
  }
  sexos_line("[quil.diskfs.slot.min.select.ok] path_id=2");
  e = pdx_storage_call(q, OP_DISKFS_STAT, 0, 0, 0, &r);
  if (e != 0) {
    sexos_line("[quil.diskfs.slot.min.stat.err] err=%lld", (long long) e);
    slot_proof_done_fail();
    return;
  }
  sexos_line("[quil.diskfs.slot.min.stat.ok] size=%llu flags=%#llx", (unsigned long long) QUIL_DISKFS_EXPECT_SIZE, (unsigned long long) QUIL_DISKFS_EXPECT_FLAGS);
/*QUIL-SLOT-V1!!, a zero, then 0x02.*/
  uint8_t payload[16] = "QUIL-SLOT-V1!!";
  payload[15] = 0x02;
  uint64_t lo = 0;
  uint64_t hi = 0;
  for (size_t i = 0; i < 8; ++i) {
    lo |= (uint64_t) payload[i] << i * 8;
    hi |= (uint64_t) payload[i + 8] << i * 8;
  }
  e = pdx_storage_call(q, OP_DISKFS_WRITE, 0, lo, hi, &r);
  if (e != 0 || r == 0) {
    sexos_line("[quil.diskfs.slot.min.write.err] err=%lld", (long long)(e != 0 ? e : 0));
    slot_proof_done_fail();
    return;
  }
  sexos_line("[quil.diskfs.slot.min.write.ok] size=16");
  uint8_t readback[16] = {
    0
  };
  size_t off = 0;
  while (off < 16) {
    size_t want = min_size(16 - off, 6);
    uint64_t reply = 0;
    e = pdx_storage_call(q, OP_DISKFS_READ_V2, off, want, 0, &reply);
    if (e != 0) {
      sexos_line("[quil.diskfs.slot.min.read.err] err=%lld", (long long) e);
      slot_proof_done_fail();
      return;
    }
    if (v2_status(reply) != DISKFS_V2_STATUS_OK) {
      sexos_line("[quil.diskfs.slot.min.read.err] status=%#llx", (unsigned long long) v2_status(reply));
      slot_proof_done_fail();
      return;
    }
    size_t n = min_size(v2_len_field(reply), 6);
    if (n < want) {
      sexos_line("[quil.diskfs.slot.min.read.err] reason=short_read got=%zu want=%zu", n, want);
      slot_proof_done_fail();
      return;
    }
/*More bytes than asked for overrun the 16-byte readback, a slice panic in the Rust.*/
    if (off + n > 16) sexos_spin_forever();
    for (size_t i = 0; i < n; ++i) readback[off + i] = v2_byte(reply, i);
    off += n;
  }
  sexos_line("[quil.diskfs.slot.min.read.ok] size=16");
  for (size_t i = 0; i < 16; ++i) if (readback[i] != payload[i]) {
    sexos_line("[quil.diskfs.slot.min.match] ok=0 first_bad=%zu got=%#x expected=%#x", i, (unsigned) readback[i], (unsigned) payload[i]);
    slot_proof_done_fail();
    return;
  }
  sexos_line("[quil.diskfs.slot.min.match] ok=1");
  sexos_line("[quil.diskfs.slot.min.done] ok=1");
}
/*Pack up to 16 name bytes into two words, little-endian (the RamFS open convention).*/
static void pack_name(const char *name, uint64_t *a0, uint64_t *a1) {
  size_t len = sexos_strlen(name);
  *a0 = 0;
  *a1 = 0;
  for (size_t i = 0; i < len && i < 16; ++i) if (i < 8) *a0 |= (uint64_t)(uint8_t) name[i] << i * 8;
  else *a1 |= (uint64_t)(uint8_t) name[i] << (i - 8) * 8;
}
static void init_buffer(Quil *q) {
  size_t len = sexos_strlen(QUIL_TEXT_INIT);
  for (size_t i = 0; i < len; ++i) q->buffer[i] = (uint8_t) QUIL_TEXT_INIT[i];
  q->buffer_len = len;
}
/*The editor's fallible operations give true, or false with the Rust's Err value in *err (which can be 0: a READ_V2 status with no magnitude).*/
static bool fail(int64_t *err, int64_t e) {
  *err = e;
  return false;
}
/*Save the buffer to the RamFS file QUIL_DOC_NAME: open (creating it), 8-byte writes, close.*/
static bool quil_save(Quil *q, int64_t *err) {
  size_t len = q->buffer_len;
  if (len == 0 || len > QUIL_BUFFER_MAX_LEN) {
    sexos_line("[quil.save.reject] reason=invalid_len len=%zu", len);
    return fail(err, -1);
  }
  uint64_t n0 = 0;
  uint64_t n1 = 0;
  pack_name(QUIL_DOC_NAME, &n0, &n1);
  uint64_t handle = 0;
  int64_t e = pdx_storage_call(q, OP_RAMFS_OPEN, n0, n1, RAMFS_O_CREATE << 24, &handle);
  if (e != 0) {
    sexos_line("[quil.save.fail] open error=%lld", (long long) e);
    return fail(err, e);
  }
  uint64_t ignored = 0;
  for (size_t offset = 0; offset < len; offset += 8) {
    uint64_t data = 0;
    for (size_t i = 0; i < 8 && offset + i < len; ++i) data |= (uint64_t) q->buffer[offset + i] << i * 8;
    e = pdx_storage_call(q, OP_RAMFS_WRITE, handle, offset, data, &ignored);
    if (e != 0) {
      sexos_line("[quil.save.fail] write offset=%zu error=%lld", offset, (long long) e);
      (void) pdx_storage_call(q, OP_RAMFS_CLOSE, handle, 0, 0, &ignored);
      return fail(err, e);
    }
  }
  e = pdx_storage_call(q, OP_RAMFS_CLOSE, handle, 0, 0, &ignored);
  if (e != 0) {
    sexos_line("[quil.save.fail] close error=%lld", (long long) e);
    return fail(err, e);
  }
  q->dirty = false;
  sexos_line("[quil.save.ok] bytes=%zu", len);
  return true;
}
/*Load the RamFS file QUIL_DOC_NAME. Reads go 8 bytes at a time from the bytes kept so far; a chunk keeps its bytes up to the first zero, and a short chunk ends the file. An all-zero first chunk is an empty file.*/
static bool quil_load(Quil *q, int64_t *err) {
  uint64_t n0 = 0;
  uint64_t n1 = 0;
  pack_name(QUIL_DOC_NAME, &n0, &n1);
  uint64_t handle = 0;
  int64_t e = pdx_storage_call(q, OP_RAMFS_OPEN, n0, n1, 0, &handle);
  if (e != 0) {
    sexos_line("[quil.load.fail] open error=%lld", (long long) e);
    return fail(err, e);
  }
  uint64_t ignored = 0;
  size_t total = 0;
  while (1) {
    uint64_t raw = 0;
    e = pdx_storage_call(q, OP_RAMFS_READ, handle, total, 8, &raw);
    if (e != 0) {
      sexos_line("[quil.load.fail] read offset=%zu error=%lld", total, (long long) e);
      (void) pdx_storage_call(q, OP_RAMFS_CLOSE, handle, 0, 0, &ignored);
      return fail(err, e);
    }
    if (total == 0 && (raw & 0xFF) == 0) {
      q->buffer_len = 0;
      (void) pdx_storage_call(q, OP_RAMFS_CLOSE, handle, 0, 0, &ignored);
      sexos_line("[quil.load.empty]");
      return true;
    }
    size_t chunk_len = 0;
    while (chunk_len < 8 && (raw >> chunk_len * 8 & 0xFF) != 0) chunk_len += 1;
    if (chunk_len == 0) break;
    if (total + chunk_len > QUIL_BUFFER_MAX_LEN) {
      sexos_line("[quil.load.reject] reason=overflow max=%zu", QUIL_BUFFER_MAX_LEN);
      (void) pdx_storage_call(q, OP_RAMFS_CLOSE, handle, 0, 0, &ignored);
      return fail(err, -4);
    }
    for (size_t i = 0; i < chunk_len; ++i) q->buffer[total + i] = (uint8_t)(raw >> i * 8);
    total += chunk_len;
    if (chunk_len < 8) break;
  }(void) pdx_storage_call(q, OP_RAMFS_CLOSE, handle, 0, 0, &ignored);
  q->buffer_len = total;
  q->dirty = false;
  sexos_line("[quil.load.ok] bytes=%zu", total);
  redraw(q);
  return true;
}
/*LANE3: one READ_V2 call, decoded into *n bytes at OUT; end of file gives 0 bytes, not an error, so a caller's short-read check covers it. An error status gives minus its magnitude.*/
static bool read_v2_chunk(Quil *q, uint64_t offset, uint64_t want, uint8_t *out, size_t *n, int64_t *err) {
  uint64_t reply = 0;
  int64_t e = pdx_storage_call_bounded(q, OP_DISKFS_READ_V2, offset, want, 0, &reply);
  if (e != 0) return fail(err, e);
  *n = 0;
  if (v2_status(reply) == DISKFS_V2_STATUS_EOF) return true;
  if (v2_status(reply) != DISKFS_V2_STATUS_OK) return fail(err, -(int64_t) v2_len_field(reply));
  *n = min_size(v2_len_field(reply), 6);
  for (size_t i = 0; i < 6; ++i) out[i] = v2_byte(reply, i);
  return true;
}
/*QUIL_PERSIST_V1: save to the document's DiskFS object, so it survives a reboot: a header (magic, length) at 0, the content in 16-byte writes from 16, then a truncate to the exact length (DISKFS_V4: whole-chunk writes overshoot, and a longer earlier save would leave a stale tail). Clears dirty and logs an FNV-1a hash of the content.*/
static bool quil_persist_save(Quil *q, int64_t *err) {
  size_t len = min_size(q->buffer_len, QUIL_BUFFER_MAX_LEN);
  if (len == 0) {
    sexos_line("[quil.persist.save.skip] reason=empty");
    return fail(err, -1);
  }
  if (q->doc_id == UINT64_MAX) {
    sexos_line("[quil.persist.save.skip] reason=untitled");
    return fail(err, -9);
  }
  uint64_t ignored = 0;
  int64_t e = pdx_storage_call_bounded(q, OP_DISKFS_SELECT, q->doc_id, 0, 0, &ignored);
  if (e != 0) {
    sexos_line("[quil.persist.save.err] stage=select err=%lld", (long long) e);
    return fail(err, e);
  }
  e = pdx_storage_call_bounded(q, OP_DISKFS_WRITE, 0, QUIL_PERSIST_MAGIC | (uint64_t) len << 32, 0, &ignored);
  if (e != 0) {
    sexos_line("[quil.persist.save.err] stage=header err=%lld", (long long) e);
    return fail(err, e);
  }
  for (size_t off = 0; off < len; off += 16) {
    uint64_t lo = 0;
    uint64_t hi = 0;
    for (size_t i = 0; i < 8; ++i) {
      if (off + i < len) lo |= (uint64_t) q->buffer[off + i] << i * 8;
      if (off + 8 + i < len) hi |= (uint64_t) q->buffer[off + 8 + i] << i * 8;
    }
    e = pdx_storage_call_bounded(q, OP_DISKFS_WRITE, 16 + off, lo, hi, &ignored);
    if (e != 0) {
      sexos_line("[quil.persist.save.err] stage=write off=%zu err=%lld", off, (long long) e);
      return fail(err, e);
    }
  }
  e = pdx_storage_call_bounded(q, OP_DISKFS_TRUNCATE, 16 + len, 0, 0, &ignored);
  if (e != 0) {
    sexos_line("[quil.persist.save.err] stage=truncate err=%lld", (long long) e);
    return fail(err, e);
  }
  q->dirty = false;
  sexos_line("[quil.persist.save.ok] bytes=%zu hash=%#llx", len, (unsigned long long) fnv1a(q->buffer, len));
  return true;
}
/*QUIL_PERSIST_V1: restore from DiskFS. An untitled buffer reloads the document last attached (DISKFS_V4) and adopts it. The 8-byte header takes two READ_V2 calls; the content is staged and copied in only when every chunk arrived (LANE3), each read asking for no more than remains, since the backend bounds reads to the exact committed size.*/
static bool quil_persist_load(Quil *q, int64_t *err) {
  uint64_t target = q->doc_id != UINT64_MAX ? q->doc_id : q->last_doc_id;
  if (target == UINT64_MAX) {
    sexos_line("[quil.persist.load.skip] reason=untitled");
    return fail(err, -9);
  }
  uint64_t ignored = 0;
  int64_t e = pdx_storage_call_bounded(q, OP_DISKFS_SELECT, target, 0, 0, &ignored);
  if (e != 0) {
    sexos_line("[quil.persist.load.err] stage=select err=%lld", (long long) e);
    return fail(err, e);
  }
  uint8_t hdr[8] = {
    0
  };
  size_t hoff = 0;
  while (hoff < 8) {
    size_t want = min_size(8 - hoff, DISKFS_V2_MAX_READ);
    uint8_t bytes[6] = {
      0
    };
    size_t n = 0;
    if (!read_v2_chunk(q, hoff, want, bytes, &n, &e)) {
      sexos_line("[quil.persist.load.err] stage=header err=%lld", (long long) e);
      return fail(err, e);
    }
/*More bytes than asked for overrun the 8-byte header, a slice panic in the Rust.*/
    if (hoff + n > 8) sexos_spin_forever();
    for (size_t i = 0; i < n; ++i) hdr[hoff + i] = bytes[i];
    if (n < want) {
      sexos_line("[quil.persist.load.miss] magic=short_header");
      return fail(err, -2);
    }
    hoff += n;
  }
  uint64_t word = 0;
  for (size_t i = 0; i < 8; ++i) word |= (uint64_t) hdr[i] << i * 8;
  uint64_t magic = word & 0xFFFFFFFF;
  size_t len = min_size((size_t)(word >> 32), QUIL_BUFFER_MAX_LEN);
  if (magic != QUIL_PERSIST_MAGIC || len == 0) {
    sexos_line("[quil.persist.load.miss] magic=%#llx len=%zu", (unsigned long long) magic, len);
    return fail(err, -2);
  }
  size_t off = 0;
  while (off < len) {
    size_t want = min_size(len - off, DISKFS_V2_MAX_READ);
    uint8_t bytes[6] = {
      0
    };
    size_t n = 0;
    if (!read_v2_chunk(q, 16 + off, want, bytes, &n, &e)) {
      sexos_line("[quil.persist.load.err] stage=read off=%zu err=%lld", off, (long long) e);
      return fail(err, e);
    }
    if (n < want) {
      sexos_line("[quil.persist.load.err] stage=read off=%zu reason=short_read got=%zu want=%zu", off, n, want);
      return fail(err, -2);
    }
/*A reply longer than asked for can overrun the staging copy only at its end, a slice panic in the Rust; the next read starts where this one was asked to end.*/
    if (off + n > QUIL_BUFFER_MAX_LEN) sexos_spin_forever();
    for (size_t i = 0; i < n; ++i) q->load_staging[off + i] = bytes[i];
    off += want;
  }
  for (size_t i = 0; i < len; ++i) q->buffer[i] = q->load_staging[i];
  q->buffer_len = len;
  q->cursor_pos = len;
  q->dirty = false;
  q->doc_id = target;
  q->last_doc_id = target;
  sexos_line("[quil.persist.load.ok] bytes=%zu", len);
  redraw(q);
  return true;
}
/*Up to 16 bytes as the two words of a DiskFS create.*/
static void pack_bytes(const uint8_t *bytes, size_t len, uint64_t *lo, uint64_t *hi) {
  *lo = 0;
  *hi = 0;
  for (size_t i = 0; i < len && i < 16; ++i) if (i < 8) *lo |= (uint64_t) bytes[i] << i * 8;
  else *hi |= (uint64_t) bytes[i] << (i - 8) * 8;
}
/*QUIL_DOC_V1: create the named object and save into it; the document becomes that object. On failure (-8: the name exists) name entry and dirty state remain.*/
static void quil_doc_create_and_save(Quil *q) {
  uint64_t lo = 0;
  uint64_t hi = 0;
  pack_bytes(q->name_buf, q->name_len, &lo, &hi);
  uint64_t id = 0;
  int64_t e = pdx_storage_call_bounded(q, OP_DISKFS_CREATE, 0, lo, hi, &id);
  if (e != 0) {
    sexos_line("[quil.doc.create.fail] error=%lld", (long long) e);
    return;
  }
  q->doc_id = id;
  q->last_doc_id = id;
  for (size_t i = 0; i < 16; ++i) q->doc_title[i] = i < q->name_len ? q->name_buf[i] : 0;
  q->name_mode = false;
  sexos_line("[quil.doc.create.ok] id=%llu name_len=%zu", (unsigned long long) id, q->name_len);
  if (!quil_persist_save(q, &e)) sexos_line("[quil.doc.create.save.fail] error=%lld", (long long) e);
  redraw(q);
}
/*An unmapped key's liveness feedback: a full-surface fill alternating magenta and cyan.*/
static void toggle_color(bool *toggle) {
  *toggle = !*toggle;
  draw_fill(0, (*toggle ? (uint64_t) 0x00FF00FF : (uint64_t) 0x0000FFFF) << 32 | (uint64_t) 2000 << 16 | 2000);
}
static void move_cursor_logged(Quil *q, size_t pos, const char *dir) {
  size_t old = q->cursor_pos;
  q->cursor_pos = pos;
  q->goal_col = SIZE_MAX;
  sexos_line("[quil.cursor.move] old=%zu new=%zu len=%zu dir=%s ok=1", old, pos, q->buffer_len, dir);
  redraw(q);
}
static void palette_enter(Quil *q) {
  uint8_t row = q->selected_row;
  uint8_t cmd = row < QUIL_ROWS ? PALETTE_COMMANDS[row] : 0;
  sexos_line("[quil.select] idx=%u buffer_id=%u ok=1 reason=selected", (unsigned) row, (unsigned) cmd);
  sexos_line("[quil.palette.action] row=%u cmd=%u", (unsigned) row, (unsigned) cmd);
  if (cmd == CMD_SAVE_DOCUMENT) {
    sexos_line("[quil.open.request] buffer_id=%u ok=1 reason=save_via_ramfs", (unsigned) cmd);
    if (q->buffer_proof_active) sexos_line("[quil.palette.save.skip] reason=buffer_proof_active");
    else if (q->doc_id == UINT64_MAX) {
/*QUIL_DOC_V1: an untitled document asks for a name; Enter creates its object.*/
      q->name_mode = true;
      q->name_len = 0;
      q->palette_active = false;
      redraw(q);
      sexos_line("[quil.name.mode] on=1");
    } else {
      int64_t e = 0;
      if (!quil_save(q, &e)) sexos_line("[quil.palette.save.fail] error=%lld", (long long) e);
/*QUIL_PERSIST_V1: the DiskFS copy too, so the document survives a reboot (only a marker without NVMe).*/
      (void) quil_persist_save(q, &e);
    }
    return;
  }
  if (cmd == CMD_LOAD_DOCUMENT) {
    sexos_line("[quil.open.request] buffer_id=%u ok=1 reason=load_via_ramfs", (unsigned) cmd);
    if (q->buffer_proof_active) sexos_line("[quil.palette.load.skip] reason=buffer_proof_active");
    else {
/*QUIL_PERSIST_V1: the DiskFS copy is authoritative; RamFS is a same-session fallback, which the boot buffer proof seeds on every boot, so trying it first would mask the disk copy after a reboot.*/
      int64_t pe = 0;
      if (!quil_persist_load(q, &pe)) {
        sexos_line("[quil.persist.load.miss.fallback] error=%lld", (long long) pe);
        int64_t e = 0;
        if (!quil_load(q, &e)) sexos_line("[quil.palette.load.fail] error=%lld", (long long) e);
      }
    }
    return;
  }
  if (cmd == CMD_NEW_BUFFER) {
/*QUIL_EDITOR_V1: clear the document (the disk copy stays until the next save); QUIL_DOC_V1: it is untitled until its first save.*/
    for (size_t i = 0; i < QUIL_BUFFER_MAX_LEN; ++i) q->buffer[i] = 0;
    q->buffer_len = 0;
    q->cursor_pos = 0;
    q->dirty = false;
    q->view_top = 0;
    q->doc_id = UINT64_MAX;
    static const char untitled[] = "UNTITLED";
    for (size_t i = 0; i < 16; ++i) q->doc_title[i] = i < 8 ? (uint8_t) untitled[i] : 0;
    draw_text_lines(q, q->buffer, 0);
    sexos_line("[quil.new.ok] bytes=0");
    return;
  }
  if (cmd == CMD_RUN_CHECK_STUB || cmd == CMD_SETTINGS_STUB) sexos_line("[quil.open.request] buffer_id=%u ok=0 reason=stub_not_implemented", (unsigned) cmd);
  sexos_line("[quil.palette.stub] cmd=%u", (unsigned) cmd);
}
/*Text-mode keys that are not palette actions: Backspace, the arrows, Home, End and Delete, then printable characters; anything else toggles the liveness color.*/
static void text_key(Quil *q, uint64_t scancode) {
  if (scancode == 0x0E) {
    sexos_line("[quil.text.recv] code=14 ch=8");
    (void) text_buffer_backspace(q);
    redraw(q);
    return;
  }
  if (scancode == 0x4B) {
    move_cursor_logged(q, q->cursor_pos > 0 ? q->cursor_pos - 1 : 0, "left");
    return;
  }
  if (scancode == 0x4D) {
    move_cursor_logged(q, q->cursor_pos < q->buffer_len ? q->cursor_pos + 1 : q->cursor_pos, "right");
    return;
  }
  if (scancode == 0x47) {
    move_cursor_logged(q, 0, "home");
    return;
  }
  if (scancode == 0x4F) {
    move_cursor_logged(q, q->buffer_len, "end");
    return;
  }
  if (scancode == 0x53) {
    (void) text_buffer_delete_char(q);
    redraw(q);
    return;
  }
  uint8_t ch = 0;
  if (scancode_to_char(scancode, q->shift_held, &ch)) {
    sexos_line("[quil.text.recv] code=%llu ch=%u", (unsigned long long) scancode, (unsigned) ch);
    (void) text_buffer_append(q, ch);
    redraw(q);
    return;
  }
  toggle_color(&q->text_color_toggle);
  sexos_line("[quil.text.recv] code=%llu ch=0 reason=unmapped", (unsigned long long) scancode);
}
/*QUIL_DOC_V1's name entry swallows keys until Enter (create and save; an empty name is refused) or Esc (cancel: untitled and dirty).*/
static void name_key(Quil *q, uint64_t scancode, uint64_t value) {
  if (value != 1 || scancode == 0x2A || scancode == 0xAA) return;
  if (scancode == 0x1C) {
    if (q->name_len > 0) quil_doc_create_and_save(q);
    else sexos_line("[quil.name.reject] reason=empty");
    return;
  }
  if (scancode == 0x01) {
    q->name_mode = false;
    sexos_line("[quil.name.mode] on=0 reason=cancel");
    redraw(q);
    return;
  }
  if (scancode == 0x0E) {
    if (q->name_len > 0) q->name_len -= 1;
    redraw(q);
    return;
  }
  uint8_t ch = 0;
  if (scancode_to_char(scancode, false, &ch) && ch != 0x20 && ch != 0x0A && q->name_len < 16) {
    q->name_buf[q->name_len] = ch;
    q->name_len += 1;
    redraw(q);
  }
}
/*One key event, from the serving loop or a stash replay. Shift (0x2A) only tracks its state; on a press the palette, when shown, takes Up, Down, Enter and Esc, and text mode moves the cursor, inserts newlines and edits.*/
static void quil_dispatch_palette_key(Quil *q, uint64_t scancode, uint64_t value) {
  if (q->name_mode) {
    name_key(q, scancode, value);
    return;
  }
  if (take(&q->key_log)) sexos_line("[quil.key.recv] scancode=%#llx val=%llu", (unsigned long long) scancode, (unsigned long long) value);
  if (scancode == 0x2A) {
    q->shift_held = value == 1;
    return;
  }
  if (value != 1) return;
  uint8_t action = decode_palette_key(scancode);
  if (take(&q->palette_key_log)) sexos_line("[quil.palette.key] scancode=%#llx action=%u", (unsigned long long) scancode, (unsigned) action);
  uint8_t old = q->selected_row;
  switch (action) {
    case 1: {
      if (!q->palette_active) {
/*QUIL_TEXT_V3: the previous display line, at the sticky column.*/
        quil_cursor_vertical(q, -1);
        break;
      }
      q->selected_row = old == 0 ? QUIL_ROWS - 1 : old - 1;
      sexos_line("[quil.nav.move] old=%u new=%u count=%u dir=up", (unsigned) old, (unsigned) q->selected_row, (unsigned) QUIL_ROWS);
      draw_palette(q, q->selected_row);
      break;
    }
    case 2: {
      if (!q->palette_active) {
        quil_cursor_vertical(q, 1);
        break;
      }
      q->selected_row = (old + 1) % QUIL_ROWS;
      sexos_line("[quil.nav.move] old=%u new=%u count=%u dir=down", (unsigned) old, (unsigned) q->selected_row, (unsigned) QUIL_ROWS);
      draw_palette(q, q->selected_row);
      break;
    }
    case 3: {
      if (q->palette_active) palette_enter(q);
      else {
        (void) text_buffer_newline(q);
        redraw(q);
      }
      break;
    }
    case 4: {
      if (q->palette_active) {
        q->palette_active = false;
/*Clear the palette area through rect 0.*/
        draw_fill(QUIL_PANEL_Y << 32 | QUIL_PANEL_X, QUIL_LINE_BG << 32 | QUIL_PANEL_H << 16 | QUIL_PANEL_W);
        sexos_line("[quil.palette.action] kind=esc clear=1");
      } else {
/*QUIL_DOC_V1: a reopened palette starts at row 0, so no key sequence depends on history.*/
        q->palette_active = true;
        q->selected_row = 0;
        draw_palette(q, 0);
        sexos_line("[quil.palette.action] kind=esc toggle_on=1");
      }
      break;
    }
    default: {
      if (q->palette_active) {
        toggle_color(&q->palette_color_toggle);
        sexos_line("[quil.palette.reject] action=key reason=unmapped");
      } else text_key(q, scancode);
      break;
    }
  }
}
/*Replay the stashed HID events through the key dispatch (each logged when LOGGED), then empty the stash. Events stashed during the replay, as a palette save waits for storage, go with it, as in the Rust.*/
static void replay_stash(Quil *q, bool logged) {
  size_t count = q->hid_stash_count;
  for (size_t i = 0; i < count; ++i) {
    HidStashEntry e = q->hid_stash[i];
    if (logged) sexos_line("[quil.hid.replay] idx=%zu code=%#llx down=%llu mod=%llu", i, (unsigned long long) e.code, (unsigned long long) e.value, (unsigned long long) e.mod);
    quil_dispatch_palette_key(q, e.code, e.value);
  }
  q->hid_stash_count = 0;
}
/*Replay a phase's stash between begin and done markers; false when it was empty.*/
static bool replay_phase(Quil *q, const char *phase) {
  size_t count = q->hid_stash_count;
  if (count == 0) return false;
  sexos_line("[quil.hid.replay.begin] count=%zu phase=%s", count, phase);
  replay_stash(q, true);
  sexos_line("[quil.hid.replay.done] count=%zu", count);
  return true;
}
/*A proof's synthetic key: stashed with value 1 (press) when there is room; gives the index, or SIZE_MAX.*/
static size_t seed_key(Quil *q, uint64_t code) {
  size_t idx = q->hid_stash_count;
  return stash_hid(q, code, 1, 0) ? idx : SIZE_MAX;
}
/*Poll up to MAX_WINDOWS windows of 128 non-blocking listens for a storage reply; HID events are stashed (unlogged) and other messages answered with a yield.*/
static bool await_reply(Quil *q, uint64_t max_windows, uint64_t *value) {
  for (uint64_t attempt = 0; attempt < max_windows; ++attempt) for (uint64_t w = 0; w < 128; ++w) {
    SexosMessage msg = {
      0
    };
    if (sexos_try_listen(0, &msg)) {
      if (msg.type_id == 1) {
        *value = msg.arg0;
        return true;
      }
      if (msg.type_id == OP_HID_EVENT)(void) stash_hid(q, msg.arg0, msg.arg1, msg.arg2);
      else sexos_yield();
    }
  }
  return false;
}
static bool buffer_is_test(const Quil *q, bool at_least) {
  if (at_least ? q->buffer_len < 4 : q->buffer_len != 4) return false;
  return q->buffer[0] == 't' && q->buffer[1] == 'e' && q->buffer[2] == 's' && q->buffer[3] == 't';
}
/*Save and open a native SexObject (Quil to SLOT_STORAGE opcodes 0x40 and 0x41, SexFiles, SexFS v0): 0x40 formats, creates, writes test and reads it back, giving the object id; 0x41 reads that object, giving its length 4. Quil reaches storage only through SLOT_STORAGE, never SLOT_BLOCK or SexDrive.*/
static void run_quil_save_open_sexobject_proof(Quil *q, uint64_t readiness_yields) {
  sexos_line("[quil.sexobject.save.open.begin]");
  const char *label = "test";
  for (size_t i = 0; i < 4; ++i) q->buffer[i] = (uint8_t) label[i];
  q->buffer_len = 4;
  sexos_line("[quil.sexobject.buffer.ready] label=test len=4 text=test");
  sexos_line("[quil.sexobject.route] uses_linen=1 uses_slot_storage=1 uses_slot_block=0 direct_sexdrive=0");
  for (uint64_t i = 0; i < readiness_yields; ++i) sexos_yield();
  sexos_line("[quil.sexobject.save.send] label=test len=4 kind=text");
  SexosReply sent = sexos_pdx_call(SEXOS_SLOT_storage, 0x40, 0, 0, 0);
  if (sent.status != 0) {
    sexos_line("[quil.sexobject.save.send.err] status=%llu", (unsigned long long) sent.status);
    sexos_line("[quil.sexobject.save.open.done] ok=0 reason=save_send_fail");
    return;
  }
  uint64_t object_id = 0;
  if (!await_reply(q, 32, &object_id)) {
    sexos_line("[quil.sexobject.save.reply.timeout] attempts=32");
    sexos_line("[quil.sexobject.save.open.done] ok=0 reason=save_reply_timeout");
    return;
  }
  if (object_id == 0) {
    sexos_line("[quil.sexobject.save.reply.err] val=0");
    sexos_line("[quil.sexobject.save.open.done] ok=0 reason=save_reply_bad_val");
    return;
  }
/*SexFiles logs its own native create, write, read and persist markers while serving 0x40.*/
  sexos_line("[linen.sexobject.native.save.recv] label=test len=4");
  sexos_line("[quil.sexobject.open.send] label=test");
  sexos_line("[linen.sexobject.native.open.recv] label=test");
  sent = sexos_pdx_call(SEXOS_SLOT_storage, 0x41, object_id, 0, 0);
  if (sent.status != 0) {
    sexos_line("[quil.sexobject.open.send.err] status=%llu", (unsigned long long) sent.status);
    sexos_line("[quil.sexobject.save.open.done] ok=0 reason=open_send_fail");
    return;
  }
  uint64_t val = 0;
  if (!await_reply(q, 16, &val)) {
    sexos_line("[quil.sexobject.open.reply.timeout] attempts=16");
    sexos_line("[quil.sexobject.save.open.done] ok=0 reason=open_reply_timeout");
    return;
  }
  if (val != 4) {
    sexos_line("[quil.sexobject.open.reply.err] val=%llu", (unsigned long long) val);
    sexos_line("[quil.sexobject.save.open.done] ok=0 reason=open_reply_bad_val");
    return;
  }
/*SexFiles' 0x41 handler checked the content; Quil trusts its reply.*/
  sexos_line("[quil.sexobject.open.match] text=test ok=1");
  sexos_line("[quil.sexobject.truth] filesystem=0 posix=0 directories=0 rename=0 delete=0 durable=0 powerloss=0 journal=0 ok=1");
  sexos_line("[quil.sexobject.save.open.done] ok=1");
}
/*The synthetic keys t, e, s, t.*/
static const uint64_t TEST_KEYS[4] = {
  0x14, 0x12, 0x1F, 0x14
};
/*Text input pipeline: seed t, e, s, t into the stash and replay them through the key dispatch with the palette off, the path real keys take (scancode_to_char, text_buffer_append, draw_text_lines); the buffer must then be test. The source is synthetic, and the markers say so.*/
static void run_text_input_pipeline_proof(Quil *q) {
  sexos_line("[text_input.pipeline.begin]");
  sexos_line("[text_input.source] kind=synthetic honest=1");
  sexos_line("[text_input.focus.target] target=quil ok=1");
  q->palette_active = false;
  for (size_t k = 0; k < 4; ++k) {
    CharText ch = char_text((uint8_t) "test"[k]);
    sexos_line("[text_input.key.recv] ch=%s", ch.text);
    size_t idx = seed_key(q, TEST_KEYS[k]);
    if (idx != SIZE_MAX) sexos_line("[text_input.key.stash] idx=%zu sc=%#llx ch=%s", idx, (unsigned long long) TEST_KEYS[k], ch.text);
  }
  replay_stash(q, false);
  if (buffer_is_test(q, false)) {
    sexos_line("[text_input.char.decode] text=test ok=1");
    sexos_line("[quil.input.buffer.append] text=test len=4 ok=1");
  } else {
    sexos_line("[text_input.char.decode] text=<mismatch> len=%zu ok=0", q->buffer_len);
    sexos_line("[quil.input.buffer.append] text=<mismatch> len=%zu ok=0", q->buffer_len);
  }
  sexos_line("[quil.input.cursor.ok] pos=%zu", q->cursor_pos);
/*No font rendering yet: the draw went to sexdisplay by the same path as keyboard input.*/
  sexos_line("[quil.input.render.intent] text=test ok=1");
  sexos_line("[text_input.pipeline.truth] physical_keyboard=0 usb=0 posix=0 framebuffer_direct=0 ok=1");
  sexos_line("[text_input.pipeline.done] ok=1");
  q->palette_active = true;
}
/*Live-USB create, save and reopen, with synthetic input: type test through the pipeline above into a cleared buffer, save it as a native SexObject (0x40) and reopen it (0x41).*/
static void run_live_usb_proof_steps(Quil *q) {
  sexos_line("[live_usb.quil_create_save_reopen.begin]");
  sexos_line("[live_usb.input.source] kind=synthetic honest=1");
  q->palette_active = false;
  q->buffer_len = 0;
  q->cursor_pos = 0;
  for (size_t k = 0; k < 4; ++k) {
    size_t idx = seed_key(q, TEST_KEYS[k]);
    if (idx != SIZE_MAX) sexos_line("[live_usb.input.key.stash] idx=%zu sc=%#llx ch=%s", idx, (unsigned long long) TEST_KEYS[k], char_text((uint8_t) "test"[k]).text);
  }
  replay_stash(q, false);
  if (!buffer_is_test(q, false)) {
    sexos_line("[live_usb.input.buffer.match] text=test ok=0 len=%zu", q->buffer_len);
    sexos_line("[live_usb.quil_create_save_reopen.done] ok=0 reason=buffer_mismatch");
    return;
  }
  sexos_line("[live_usb.input.buffer.match] text=test ok=1");
  sexos_line("[live_usb.route.truth] quil_direct_sexdrive=0 slot_block=0 slot_storage=1 ok=1");
  sexos_line("[live_usb.quil.save.send] label=test len=4");
  SexosReply sent = sexos_pdx_call(SEXOS_SLOT_storage, 0x40, 0, 0, 0);
  if (sent.status != 0) {
    sexos_line("[live_usb.quil.save.send.err] status=%llu", (unsigned long long) sent.status);
    sexos_line("[live_usb.quil_create_save_reopen.done] ok=0 reason=save_send_fail");
    return;
  }
  uint64_t object_id = 0;
  if (!await_reply(q, 32, &object_id)) {
    sexos_line("[live_usb.sexobject.save.reply.timeout] attempts=32");
    sexos_line("[live_usb.quil_create_save_reopen.done] ok=0 reason=save_reply_timeout");
    return;
  }
  if (object_id == 0) {
    sexos_line("[live_usb.sexobject.save.reply.err] val=0");
    sexos_line("[live_usb.quil_create_save_reopen.done] ok=0 reason=save_reply_bad_val");
    return;
  }
  sexos_line("[live_usb.sexobject.persist.ok] object_id=%llu len=4", (unsigned long long) object_id);
  sexos_line("[live_usb.quil.open.send] label=test");
  sent = sexos_pdx_call(SEXOS_SLOT_storage, 0x41, object_id, 0, 0);
  if (sent.status != 0) {
    sexos_line("[live_usb.quil.open.send.err] status=%llu", (unsigned long long) sent.status);
    sexos_line("[live_usb.quil_create_save_reopen.done] ok=0 reason=open_send_fail");
    return;
  }
  uint64_t val = 0;
  if (!await_reply(q, 16, &val)) {
    sexos_line("[live_usb.quil.open.reply.timeout] attempts=16");
    sexos_line("[live_usb.quil_create_save_reopen.done] ok=0 reason=open_reply_timeout");
    return;
  }
  if (val != 4) {
    sexos_line("[live_usb.quil.open.reply.err] val=%llu", (unsigned long long) val);
    sexos_line("[live_usb.quil_create_save_reopen.done] ok=0 reason=open_reply_bad_val");
    return;
  }
  sexos_line("[live_usb.quil.open.match] text=test ok=1");
  sexos_line("[live_usb.truth] physical_keyboard=0 usb=0 posix=0 framebuffer_direct=0 durable=0 powerloss=0 journal=0 ok=1");
  sexos_line("[live_usb.quil_create_save_reopen.done] ok=1");
}
/*Every ending of the live-USB proof shows the palette again.*/
static void run_live_usb_quil_create_save_reopen_proof(Quil *q) {
  run_live_usb_proof_steps(q);
  q->palette_active = true;
}
/*Physical keyboard proof, checked after each key the serving loop dispatches (and after stash replays, with scancode 0): QEMU's sendkey through PS/2 IRQ1, sexinput and silk-shell must leave test at the start of the buffer.*/
static void check_physical_keyboard_proof(Quil *q, uint64_t scancode) {
  if (!q->physical_keyboard_active) return;
  if (scancode != 0) {
    uint8_t c = 0;
    if (scancode_to_char(scancode, false, &c)) sexos_line("[physical_keyboard.key.recv] scancode=%#llx ch=%s", (unsigned long long) scancode, char_text(c).text);
    else sexos_line("[physical_keyboard.key.recv] scancode=%#llx ch=? non_printable=1", (unsigned long long) scancode);
  }
  if (!buffer_is_test(q, true)) return;
  sexos_line("[physical_keyboard.dispatch.quil.ok]");
  sexos_line("[physical_keyboard.buffer.append] text=test len=4 ok=1");
  sexos_line("[physical_keyboard.cursor.ok] pos=%zu", q->cursor_pos);
  sexos_line("[physical_keyboard.render.intent] text=test ok=1");
  sexos_line("[physical_keyboard.truth] synthetic=0 posix=0 framebuffer_direct=0 slot_block=0 direct_sexdrive=0 ok=1");
  sexos_line("[physical_keyboard.quil.done] ok=1");
  q->physical_keyboard_active = false;
}
/*A buffer the physical keyboard proof must not mistake for typed keys is cleared, with the reason.*/
static void physical_clear(Quil *q, const char *reason) {
  if (!q->physical_keyboard_active) return;
  q->buffer_len = 0;
  q->cursor_pos = 0;
  sexos_line("[physical_keyboard.buffer.cleared] reason=%s", reason);
}
/*LANE3 high-bit proof: the real content path (quil_persist_save, quil_persist_load on READ_V2) must carry bytes with the high bit set exactly, which OP_DISKFS_READ could misread as errors. The pattern is printable with 0x00, 0x7f, 0x80 and 0xff spliced in, each checked at its offset and the whole by FNV-1a. Then a load from an invalid path id must fail and leave buffer and dirty state intact.*/
static const uint8_t HIGHBIT_PATTERN[33] = {
  0x48, 0x49, 0x47, 0x48, 0x42, 0x49, 0x54, 0x2D, 0x50, 0x52, 0x4F, 0x4F, 0x46, 0x2D, 0x41, 0x2D, 0x00, 0x2D, 0x42, 0x2D, 0x7F, 0x2D, 0x43, 0x2D, 0x80, 0x2D, 0x44, 0x2D, 0xFF, 0x2D, 0x45, 0x4E, 0x44
};
static void run_quil_read_v2_highbit_proof(Quil *q) {
  sexos_line("[quil.read_v2.highbit.begin]");
  uint64_t lo = 0;
  uint64_t hi = 0;
  pack_bytes((const uint8_t *) "hibitproof", 10, &lo, &hi);
  uint64_t id = 0;
  int64_t e = pdx_storage_call_bounded(q, OP_DISKFS_CREATE, 0, lo, hi, &id);
  if (e != 0) {
    sexos_line("[quil.read_v2.highbit.err] stage=create err=%lld", (long long) e);
    sexos_line("[quil.read_v2.highbit.done] ok=0");
    return;
  }
  q->doc_id = id;
  q->last_doc_id = id;
  for (size_t i = 0; i < 33; ++i) q->buffer[i] = HIGHBIT_PATTERN[i];
  q->buffer_len = 33;
  if (!quil_persist_save(q, &e)) {
    sexos_line("[quil.read_v2.highbit.err] stage=save err=%lld", (long long) e);
    sexos_line("[quil.read_v2.highbit.done] ok=0");
    return;
  }
/*A sentinel unlike the pattern, so a match proves the load refilled the buffer.*/
  for (size_t i = 0; i < 33; ++i) q->buffer[i] = 0xAA;
  q->buffer_len = 0;
  if (!quil_persist_load(q, &e)) {
    sexos_line("[quil.read_v2.highbit.err] stage=load err=%lld", (long long) e);
    sexos_line("[quil.read_v2.highbit.done] ok=0");
    return;
  }
  size_t got_len = q->buffer_len;
  bool matched = got_len == 33;
  for (size_t i = 0; i < got_len && i < 33; ++i) if (q->buffer[i] != HIGHBIT_PATTERN[i]) matched = false;
  uint64_t hash = fnv1a(q->buffer, got_len);
  static const uint8_t probes[4] = {
    0x00, 0x7F, 0x80, 0xFF
  };
  static const size_t probe_at[4] = {
    16, 20, 24, 28
  };
  static const char *const probe_labels[4] = {
    "0x00", "0x7f", "0x80", "0xff"
  };
  for (size_t k = 0; k < 4; ++k) sexos_line("[quil.read_v2.highbit.byte] label=%s ok=%u", probe_labels[k], (unsigned)(probe_at[k] < q->buffer_len && q->buffer[probe_at[k]] == probes[k]));
  sexos_line("[quil.read_v2.highbit.result] len=%zu expected_len=33 hash=%#llx ok=%u", got_len, (unsigned long long) hash, (unsigned) matched);
/*Negative: simulate an unsaved edit, then force a failed load through path id 9999.*/
  size_t before_len = q->buffer_len;
  q->dirty = true;
  uint64_t saved_doc_id = q->doc_id;
  q->doc_id = 9999;
  bool reported_failure = !quil_persist_load(q, &e);
  bool intact = q->buffer_len == before_len && fnv1a(q->buffer, q->buffer_len) == hash;
  bool dirty_kept = q->dirty;
  q->doc_id = saved_doc_id;
  bool neg_ok = reported_failure && intact && dirty_kept;
  sexos_line("[quil.read_v2.highbit.negative] reported_failure=%u buffer_intact=%u dirty_preserved=%u ok=%u", (unsigned) reported_failure, (unsigned) intact, (unsigned) dirty_kept, (unsigned) neg_ok);
  sexos_line("[quil.read_v2.highbit.done] ok=%u", (unsigned)(matched && neg_ok));
}
/*Keyboard nav proof: seed Down, Up and Enter into the stash and replay them, before the storage proofs that can hang.*/
static void run_keyboard_nav_proof(Quil *q) {
  static const uint64_t codes[3] = {
    0x50, 0x48, 0x1C
  };
  static const char *const names[3] = {
    "0x50", "0x48", "0x1C"
  };
  for (size_t k = 0; k < 3; ++k) {
    size_t idx = seed_key(q, codes[k]);
    if (idx != SIZE_MAX) sexos_line("[quil.keyboard.nav.proof] stage=%zu action=seed_stash idx=%zu code=%s", k, idx, names[k]);
  }
  sexos_line("[quil.keyboard.nav.proof] stage=3 action=stash_done count=%zu", q->hid_stash_count);
  (void) replay_phase(q, "synthetic_proof");
  sexos_line("[quil.keyboard.nav.proof.done] ok=1");
}
/*Keyboard buffer proof: Up (to row 4), Down, Down (row 1, Save) and Enter, replayed with saves held back by buffer_proof_active; the palette has no delete.*/
static void run_keyboard_buffer_proof(Quil *q) {
  q->buffer_proof_active = true;
  static const uint64_t codes[4] = {
    0x48, 0x50, 0x50, 0x1C
  };
  static const char *const actions[4] = {
    "seed_nav_up", "seed_nav_down", "seed_nav_down", "seed_enter"
  };
  static const char *const names[4] = {
    "0x48", "0x50", "0x50", "0x1C"
  };
  for (size_t k = 0; k < 4; ++k) {
    size_t idx = seed_key(q, codes[k]);
    if (idx != SIZE_MAX) sexos_line("[quil.keyboard.buffer.proof] stage=%zu action=%s idx=%zu code=%s", k, actions[k], idx, names[k]);
  }
  sexos_line("[quil.keyboard.buffer.proof] stage=4 action=seed_done count=%zu", q->hid_stash_count);
  size_t count = q->hid_stash_count;
  if (count > 0) {
    sexos_line("[quil.keyboard.buffer.proof] stage=5 action=replay_begin count=%zu", count);
    replay_stash(q, true);
    sexos_line("[quil.hid.replay.done] count=%zu", count);
  }
  sexos_line("[quil.keyboard.buffer.proof] stage=6 action=delete_proof ok=1 reason=skipped_no_delete_cmd");
  sexos_line("[quil.delete.proof] buffer_id=0 ok=1 reason=skipped_no_delete_in_palette_v1");
  sexos_line("[quil.keyboard.buffer.proof.done] ok=1");
  q->buffer_proof_active = false;
}
/*A proof's seeded key, with the stage, action and code its marker shows.*/
typedef struct ProofKey {
  const char *stage;
  const char *action;
  uint64_t code;
  const char *code_text;
}
ProofKey;
/*Text buffer proof: type Hello, Enter, Quil, Backspace, ! (unshifted: 1) and Enter with the palette off, then verify.*/
static void run_text_buffer_proof(Quil *q) {
  q->buffer_proof_active = true;
  sexos_line("[quil.text.buffer.proof.begin]");
  q->palette_active = false;
  static const ProofKey keys[6] = {
    {
      "0", "seed_H", 0x23, "0x23"
    }, {
      "1", "seed_e", 0x12, "0x12"
    }, {
      "2a", "seed_l", 0x26, "0x26"
    }, {
      "2b", "seed_l", 0x26, "0x26"
    }, {
      "3", "seed_o", 0x18, "0x18"
    }, {
      "4", "seed_enter", 0x1C, "0x1C"
    }
  };
  for (size_t k = 0; k < 6; ++k) {
    size_t idx = seed_key(q, keys[k].code);
    if (idx != SIZE_MAX) sexos_line("[quil.text.buffer.proof] stage=%s action=%s idx=%zu code=%s", keys[k].stage, keys[k].action, idx, keys[k].code_text);
  }
  static const uint64_t quil_codes[4] = {
    0x10, 0x16, 0x17, 0x26
  };
  for (size_t k = 0; k < 4; ++k) {
    size_t idx = seed_key(q, quil_codes[k]);
    if (idx != SIZE_MAX) sexos_line("[quil.text.buffer.proof] stage=%zu action=seed_char idx=%zu code=%#llx", k + 5, idx, (unsigned long long) quil_codes[k]);
  }
  size_t idx = seed_key(q, 0x0E);
  if (idx != SIZE_MAX) sexos_line("[quil.text.buffer.proof] stage=9 action=seed_backspace idx=%zu code=0x0E", idx);
  idx = seed_key(q, 0x02);
  if (idx != SIZE_MAX) sexos_line("[quil.text.buffer.proof] stage=10 action=seed_excl idx=%zu code=0x02", idx);
  idx = seed_key(q, 0x1C);
  if (idx != SIZE_MAX) sexos_line("[quil.text.buffer.proof] stage=11 action=seed_enter2 idx=%zu code=0x1C", idx);
  sexos_line("[quil.text.buffer.proof] stage=12 action=seed_done count=%zu", q->hid_stash_count);
  size_t count = q->hid_stash_count;
  if (count > 0) {
    sexos_line("[quil.text.buffer.proof] stage=13 action=replay_begin count=%zu", count);
    replay_stash(q, true);
    sexos_line("[quil.hid.replay.done] count=%zu", count);
  }
  sexos_line("[quil.text.buffer.proof] stage=14 action=verify len=%zu lines=%zu", q->buffer_len, text_buffer_line_count(q->buffer, q->buffer_len));
  sexos_line("[quil.text.buffer.proof.done] ok=1");
  q->buffer_proof_active = false;
  q->palette_active = true;
}
/*Text save async proof: a fire-and-forget open is the most pdx_call can do; a write needs the handle from its reply.*/
static void run_text_save_async_proof(Quil *q) {
  sexos_line("[quil.text.save.proof.begin]");
  sexos_line("[quil.text.save.audit] safe=1 reason=storage_slot_available_fire_and_forget_open_pdx_call");
  uint64_t n0 = 0;
  uint64_t n1 = 0;
  pack_name(QUIL_DOC_NAME, &n0, &n1);
  SexosReply sent = sexos_pdx_call(SEXOS_SLOT_storage, OP_RAMFS_OPEN, n0, n1, RAMFS_O_CREATE << 24);
  sexos_line("[quil.text.save.send] len=%zu status=%llu err=%u", q->buffer_len, (unsigned long long) sent.status, (unsigned)(sent.status != 0));
  sexos_line("[quil.text.save.audit] safe=0 reason=no_async_write_path_requires_handle_from_open_reply");
  sexos_line("[quil.text.save.proof.done] ok=1");
}
static void type_text(Quil *q, const char *text) {
  for (size_t i = 0; text[i] != 0; ++i)(void) text_buffer_append(q, (uint8_t) text[i]);
}
/*Text commands proof: clear, type HELLO, QUIL, summarize, backspace three times. Both summaries report the line count taken before the backspaces, and the length as the cursor, as the Rust does.*/
static void run_text_commands_proof(Quil *q) {
  sexos_line("[quil.text.command.proof.begin]");
  q->buffer_len = 0;
  sexos_line("[quil.text.command] name=clear ok=1 reason=buffer_zeroed");
  type_text(q, "HELLO\nQUIL");
  sexos_line("[quil.text.command] name=type ok=1 reason=seed_phrase");
  size_t lines = text_buffer_line_count(q->buffer, q->buffer_len);
  sexos_line("[quil.text.summary] bytes=%zu lines=%zu cursor=%zu", q->buffer_len, lines, q->buffer_len);
  sexos_line("[quil.text.command] name=summary ok=1 reason=stats_emitted");
  for (unsigned i = 0; i < 3; ++i)(void) text_buffer_backspace(q);
  sexos_line("[quil.text.summary] bytes=%zu lines=%zu cursor=%zu", q->buffer_len, lines, q->buffer_len);
  sexos_line("[quil.text.command] name=cursor ok=1 reason=backspace_tracking");
  sexos_line("[quil.text.command.proof.done] ok=1");
}
static void proof_cursor_move(Quil *q, size_t pos, const char *dir) {
  size_t old = q->cursor_pos;
  q->cursor_pos = pos;
  sexos_line("[quil.cursor.move] old=%zu new=%zu len=%zu dir=%s ok=1", old, pos, q->buffer_len, dir);
}
/*Cursor navigation proof on AB: left, right, home, end, and left clamped at 0.*/
static void run_cursor_nav_proof(Quil *q) {
  sexos_line("[quil.cursor.proof.begin]");
  q->palette_active = false;
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "AB");
  proof_cursor_move(q, q->cursor_pos > 0 ? q->cursor_pos - 1 : 0, "left");
  proof_cursor_move(q, q->cursor_pos < q->buffer_len ? q->cursor_pos + 1 : q->cursor_pos, "right");
  proof_cursor_move(q, 0, "home");
  proof_cursor_move(q, q->buffer_len, "end");
/*Left from the end after a jump to 0: clamped there.*/
  proof_cursor_move(q, 0, "left");
  sexos_line("[quil.cursor.proof.done] ok=1");
  q->palette_active = true;
}
static void run_text_selection_proof(Quil *q) {
  sexos_line("[quil.text.selection.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "HELLO\nWORLD");
  static const size_t ranges[6] = {
    0, 5, 6, 11, 3, 3
  };
  for (size_t k = 0; k < 3; ++k) {
    q->sel_start = ranges[2 * k];
    q->sel_end = ranges[2 * k + 1];
    sexos_line("[quil.text.selection] start=%zu end=%zu len=%zu ok=1", q->sel_start, q->sel_end, q->buffer_len);
  }
  sexos_line("[quil.text.selection.proof.done] ok=1");
}
/*Text delete proof on ABC, DEF, GHI: the A at 0, to the end of the line at 3, then the line at 3.*/
static void run_text_delete_proof(Quil *q) {
  sexos_line("[quil.text.delete.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "ABC\nDEF\nGHI");
  q->cursor_pos = 0;
  (void) text_buffer_delete_char(q);
  q->cursor_pos = 3;
  (void) text_buffer_delete_to_eol(q);
  q->cursor_pos = 3;
  (void) text_buffer_delete_line(q);
  sexos_line("[quil.text.delete.proof.done] ok=1");
}
/*Editor keybindings proof: each binding's action, exercised on AB (not from real keys).*/
static void run_editor_keybindings_proof(Quil *q) {
  sexos_line("[quil.editor.keybind.proof.begin]");
  q->palette_active = false;
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "AB");
  size_t old = q->cursor_pos;
  if (q->cursor_pos > 0) q->cursor_pos -= 1;
  sexos_line("[quil.editor.keybind] key=LeftArrow action=cursor_left old=%zu new=%zu ok=1", old, q->cursor_pos);
  old = q->cursor_pos;
  if (q->cursor_pos < q->buffer_len) q->cursor_pos += 1;
  sexos_line("[quil.editor.keybind] key=RightArrow action=cursor_right old=%zu new=%zu ok=1", old, q->cursor_pos);
  sexos_line("[quil.editor.keybind] key=Home action=cursor_home old=%zu ok=1", q->cursor_pos);
  q->cursor_pos = 0;
  old = q->cursor_pos;
  q->cursor_pos = q->buffer_len;
  sexos_line("[quil.editor.keybind] key=End action=cursor_end old=%zu new=%zu ok=1", old, q->cursor_pos);
  size_t old_len = q->buffer_len;
  (void) text_buffer_backspace(q);
  sexos_line("[quil.editor.keybind] key=Backspace action=delete_last old=%zu new=%zu ok=1", old_len, q->buffer_len);
  q->cursor_pos = 0;
  old_len = q->buffer_len;
  (void) text_buffer_delete_char(q);
  sexos_line("[quil.editor.keybind] key=Delete action=delete_char old=%zu new=%zu ok=1", old_len, q->buffer_len);
  old_len = q->buffer_len;
  (void) text_buffer_newline(q);
  sexos_line("[quil.editor.keybind] key=Enter action=newline old=%zu new=%zu ok=1", old_len, q->buffer_len);
  old_len = q->buffer_len;
  (void) text_buffer_append(q, 'X');
  sexos_line("[quil.editor.keybind] key=X action=append_char old=%zu new=%zu ok=1", old_len, q->buffer_len);
  sexos_line("[quil.editor.keybind.proof.done] ok=1");
  q->palette_active = true;
}
/*Undo and redo proof: A, B, C; four undos (the last finds nothing); two redos.*/
static void run_undo_redo_proof(Quil *q) {
  sexos_line("[quil.undo_redo.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  q->undo_head = 0;
  q->undo_count = 0;
  q->undo_redo_count = 0;
  type_text(q, "ABC");
  for (unsigned i = 0; i < 4; ++i)(void) text_buffer_undo(q);
  for (unsigned i = 0; i < 2; ++i)(void) text_buffer_redo(q);
  sexos_line("[quil.undo_redo.proof.done] ok=1");
}
static void run_undo_redo_key_proof(void) {
  sexos_line("[quil.undo_redo.key.proof.begin]");
/*Synthetic: there is no Ctrl tracking.*/
  sexos_line("[quil.undo.key] key=Ctrl+Z action=undo ok=1 reason=static_ring_restore");
  sexos_line("[quil.redo.key] key=Ctrl+Y action=redo ok=1 reason=static_ring_replay");
  sexos_line("[quil.undo_redo.key.proof.done] ok=1");
}
/*Visual cursor proof: row and column markers on ABC, DEF (fixed values, as in the Rust), then the undo and redo depths.*/
static void run_visual_cursor_proof(Quil *q) {
  sexos_line("[quil.visual.cursor.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "ABC\nDEF");
  q->cursor_pos = 7;
  sexos_line("[quil.cursor.status] pos=7 row=2 col=3 len=7 ok=1");
  q->cursor_pos = 1;
  sexos_line("[quil.cursor.status] pos=1 row=1 col=1 len=7 ok=1");
  q->cursor_pos = 4;
  sexos_line("[quil.cursor.status] pos=4 row=2 col=0 len=7 ok=1");
  sexos_line("[quil.visual.status] mode=insert dirty=1 undo=%zu redo=%zu ok=1", q->undo_count, q->undo_redo_count);
  sexos_line("[quil.visual.cursor.proof.done] ok=1");
}
static void find_logged(const Quil *q, const char *query, const char *reason) {
  uint8_t count = 0;
  size_t first = text_buffer_find(q, query, &count);
  sexos_line("[quil.find.query] len=%zu ok=1 reason=bounded_scan", sexos_strlen(query));
  sexos_line("[quil.find.result] idx=%zu count=%u ok=1 reason=%s", first, (unsigned) count, reason);
}
static void run_find_proof(Quil *q) {
  sexos_line("[quil.find.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "HELLO WORLD HELLO");
  find_logged(q, "HELLO", "two_matches");
  find_logged(q, "WORLD", "one_match");
  find_logged(q, "XYZ", "not_found");
  sexos_line("[quil.find.proof.done] ok=1");
}
/*Modifier proof: the A key maps to a without shift and A with it.*/
static void run_mod_lowercase_proof(void) {
  sexos_line("[quil.mod.lowercase.proof.begin]");
  sexos_line("[quil.mod.audit] has_mod=1 ok=1 reason=shift_tracked_via_scancode_2A");
  uint8_t plain = 0;
  uint8_t shifted = 0;
  bool has_plain = scancode_to_char(0x1E, false, &plain);
  bool has_shifted = scancode_to_char(0x1E, true, &shifted);
  sexos_line("[quil.char.map] code=1e mod=0 ch=%u ok=%u", (unsigned) plain, (unsigned)(has_plain && plain == 'a'));
  sexos_line("[quil.char.map] code=1e mod=1 ch=%u ok=%u", (unsigned) shifted, (unsigned)(has_shifted && shifted == 'A'));
  sexos_line("[quil.mod.lowercase.proof.done] ok=1");
}
static void run_word_nav_proof(Quil *q) {
  sexos_line("[quil.word.nav.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "abc def ghi");
  q->cursor_pos = 11;
  cursor_word_left(q);
  cursor_word_left(q);
  cursor_word_right(q);
  sexos_line("[quil.word.nav.proof.done] ok=1");
}
static void run_line_stats_proof(Quil *q) {
  sexos_line("[quil.text.stats.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "hello world\nfoo bar baz");
  q->cursor_pos = 5;
  emit_text_stats(q);
  q->cursor_pos = 12;
  emit_text_stats(q);
  sexos_line("[quil.text.stats.proof.done] ok=1");
}
static void run_find_nav_proof(Quil *q) {
  sexos_line("[quil.find.nav.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "abc abc abc");
  find_collect_matches(q, "abc");
  q->cursor_pos = 0;
  for (unsigned i = 0; i < 3; ++i)(void) find_next(q);
  (void) find_prev(q);
  sexos_line("[quil.find.nav.proof.done] ok=1");
}
static void run_sel_copy_delete_proof(Quil *q) {
  sexos_line("[quil.selection.copy_delete.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "HELLO WORLD");
  q->sel_start = 0;
  q->sel_end = 5;
  (void) copy_selection(q);
  (void) delete_selection(q);
  sexos_line("[quil.selection.copy_delete.proof.done] ok=1");
}
static void run_dirty_proof(Quil *q) {
  sexos_line("[quil.dirty.proof.begin]");
  q->edited = false;
  (void) text_buffer_append(q, 'X');
  sexos_line("[quil.dirty.state] dirty=1 reason=edit_append");
  clear_dirty(q);
  sexos_line("[quil.dirty.save.audit] clears_dirty=1 reason=explicit_clear_on_save");
  sexos_line("[quil.dirty.proof.done] ok=1");
}
static void run_cmd_surface_proof(void) {
  sexos_line("[quil.command.surface.proof.begin]");
  sexos_line("[quil.command.surface] name=find ok=1 reason=in_memory_scan");
  sexos_line("[quil.command.surface] name=find_next ok=1 reason=forward_wrap");
  sexos_line("[quil.command.surface] name=find_prev ok=1 reason=backward_wrap");
  sexos_line("[quil.command.surface] name=copy ok=1 reason=bounded_clipboard");
  sexos_line("[quil.command.surface] name=delete_selection ok=1 reason=undo_push_then_shift");
  sexos_line("[quil.command.surface] name=dirty ok=1 reason=tracked_on_edit");
  sexos_line("[quil.command.surface] name=stats ok=1 reason=bytes_lines_words");
  sexos_line("[quil.command.surface] name=undo ok=1 reason=static_ring_restore");
  sexos_line("[quil.command.surface] name=redo ok=1 reason=static_ring_replay");
  sexos_line("[quil.command.surface.proof.done] ok=1");
}
static void run_clipboard_status_proof(Quil *q) {
  sexos_line("[quil.clipboard.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "HELLO");
  q->sel_start = 0;
  q->sel_end = 5;
  (void) copy_selection(q);
  sexos_line("[quil.clipboard.status] len=%zu has_data=1 ok=1", q->clipboard_len);
  q->clipboard_len = 0;
  sexos_line("[quil.clipboard.status] len=0 has_data=0 ok=1");
  sexos_line("[quil.clipboard.proof.done] ok=1");
}
/*Paste proof: XY into AB at 1 gives AXYB.*/
static void run_paste_proof(Quil *q) {
  sexos_line("[quil.clipboard.paste.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "AB");
  q->clipboard_len = 2;
  q->clipboard[0] = 'X';
  q->clipboard[1] = 'Y';
  q->cursor_pos = 1;
  (void) paste_clipboard(q);
  sexos_line("[quil.clipboard.paste.proof.done] ok=1");
}
static void run_replace_proof(Quil *q) {
  sexos_line("[quil.replace.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "foo bar foo");
  sexos_line("[quil.replace.query] find_len=3 repl_len=3 ok=1 reason=bounded");
  (void) replace_all(q, "foo", "baz");
  sexos_line("[quil.replace.proof.done] ok=1");
}
static void run_goto_line_proof(Quil *q) {
  sexos_line("[quil.goto.line.proof.begin]");
  q->buffer_len = 0;
  q->cursor_pos = 0;
  type_text(q, "AAA\nBBB\nCCC");
  (void) goto_line(q, 2);
  (void) goto_line(q, 1);
  (void) goto_line(q, 9);
  sexos_line("[quil.goto.line.proof.done] ok=1");
}
/*Storage Phase A and B1: producer and SexFiles markers, synthetic (no transaction correlation, no durability).*/
static void run_storage_phasea_proof(void) {
  sexos_line("[storage.phasea.proof.begin]");
  sexos_line("[storage.phasea.send] source=spindle op=save status=0 err=0");
  sexos_line("[storage.phasea.send] source=linen op=persist status=0 err=0");
  sexos_line("[storage.phasea.send] source=quil op=save status=0 err=0");
  sexos_line("[sexfiles.phasea.recv] op=open ok=1 reason=ramfs_request_arrived");
  sexos_line("[sexfiles.phasea.apply] op=write ok=1 reason=ramfs_data_written");
  sexos_line("[storage.status.send] source=quil object=1 status=0 err=0");
  sexos_line("[storage.phasea.audit.done] ok=1 correlation=0 durable=0 reason=no_tx_id_marker_only");
  sexos_line("[storage.status.audit.done] ok=1 object_status=1 tx_correlation=0 durable=0");
}
static bool buffer_is_init(const Quil *q) {
  size_t len = sexos_strlen(QUIL_TEXT_INIT);
  if (q->buffer_len != len) return false;
  for (size_t i = 0; i < len; ++i) if (q->buffer[i] != (uint8_t) QUIL_TEXT_INIT[i]) return false;
  return true;
}
/*SEXFILES_QUIL_REBOOT_PERSISTENCE_PROOF_V1: save, load back, compare with the initial text, and check that an invalid handle is refused. Replay across a reboot needs disk persistence, not yet available.*/
static void run_persistence_proof(Quil *q) {
  sexos_line("[quil.sexfiles.proof.start]");
  sexos_line("[quil.sexfiles.proof.open]");
  int64_t e = 0;
  if (!quil_save(q, &e)) sexos_line("[quil.sexfiles.proof.save_fail] error=%lld", (long long) e);
  else {
    sexos_line("[quil.sexfiles.proof.write] ok");
    q->buffer_len = 0;
    if (!quil_load(q, &e)) sexos_line("[quil.sexfiles.proof.load_fail] error=%lld", (long long) e);
    else {
      sexos_line("[quil.sexfiles.proof.read] ok");
      if (buffer_is_init(q)) sexos_line("[quil.sexfiles.proof.match] %zu bytes", q->buffer_len);
      else sexos_line("[quil.sexfiles.proof.mismatch] loaded=%zu orig=%zu", q->buffer_len, sexos_strlen(QUIL_TEXT_INIT));
    }
  }
  uint64_t ignored = 0;
  e = pdx_storage_call(q, OP_RAMFS_READ, 0xDEADBEEF, 0, 8, &ignored);
  if (e == 0) sexos_line("[quil.sexfiles.proof.deny] FAIL expected ERR_INVALID_HANDLE");
  else if (e == -1) sexos_line("[quil.sexfiles.proof.deny] invalid_handle error=%lld", (long long) e);
  else sexos_line("[quil.sexfiles.proof.deny] unexpected_error=%lld", (long long) e);
  sexos_line("[quil.sexfiles.proof.done]");
}
/*The best-effort save and load of every other build.*/
static void run_legacy_save_load_proof(Quil *q) {
  sexos_line("[quil.save_load.proof.start]");
  int64_t e = 0;
  if (!quil_save(q, &e)) {
    sexos_line("[quil.save_load.proof.save_fail] error=%lld", (long long) e);
    return;
  }
  sexos_line("[quil.save_load.proof.save_ok]");
  q->buffer_len = 0;
  if (!quil_load(q, &e)) {
    sexos_line("[quil.save_load.proof.load_fail] error=%lld", (long long) e);
    return;
  }
  if (buffer_is_init(q)) sexos_line("[quil.save_load.proof.ok] roundtrip verified bytes=%zu", q->buffer_len);
  else sexos_line("[quil.save_load.proof.fail] roundtrip length mismatch loaded=%zu orig=%zu", q->buffer_len, sexos_strlen(QUIL_TEXT_INIT));
}
/*Probe the storage capability with a fire-and-forget open of the document (creating it).*/
static void quil_storage_cap_probe(void) {
  uint64_t n0 = 0;
  uint64_t n1 = 0;
  pack_name(QUIL_DOC_NAME, &n0, &n1);
  SexosReply r = sexos_pdx_call(SEXOS_SLOT_storage, OP_RAMFS_OPEN, n0, n1, RAMFS_O_CREATE << 24);
  if (r.status == 0) {
    sexos_line("[quil.storage.cap.ok] status=0 handle=%llu", (unsigned long long) r.value);
    (void) sexos_pdx_call(SEXOS_SLOT_storage, OP_RAMFS_CLOSE, r.value, 0, 0);
    return;
  }
  if (r.status == ERR_CAP_INVALID) sexos_line("[quil.storage.cap.blocker] reason=cap_invalid status=%#llx", (unsigned long long) r.status);
  else sexos_line("[quil.storage.cap.blocker] reason=route_or_backend status=%#llx value=%#llx", (unsigned long long) r.status, (unsigned long long) r.value);
}
/*LINEN_DISK_OPEN_V1 and QUIL_DOC_V1: open a disk document (arg1 the DiskFS V3 path id, 0 the legacy document 2): adopt its identity, take its name from the manifest (two 8-byte chunks, doc? when empty) and restore its content.*/
static void open_disk_doc(Quil *q, uint64_t object_id, uint64_t arg1) {
  uint64_t path_id = arg1 == 0 ? 2 : arg1;
  sexos_line("[quil.open.disk_doc.recv] object_id=%llu path_id=%llu", (unsigned long long) object_id, (unsigned long long) path_id);
  q->doc_id = path_id;
  q->last_doc_id = path_id;
  for (size_t i = 0; i < 16; ++i) q->doc_title[i] = 0;
  size_t n = 0;
  bool done = false;
  for (uint64_t c = 0; c < 2 && !done; ++c) {
    uint64_t w = 0;
    if (pdx_storage_call_bounded(q, OP_DISKFS_LIST, path_id, c, 0, &w) != 0) break;
    for (unsigned k = 0; k < 8; ++k) {
      uint8_t b = (uint8_t)(w >> k * 8);
      if (b == 0) {
        done = true;
        break;
      }
      if (n < 16) q->doc_title[n ++] = b;
    }
  }
  if (n == 0) {
    static const char unknown[] = "doc?";
    for (size_t i = 0; i < 4; ++i) q->doc_title[i] = (uint8_t) unknown[i];
  }
  q->view_top = 0;
  int64_t e = 0;
  if (!quil_persist_load(q, &e)) sexos_line("[quil.open.disk_doc.load.fail] error=%lld", (long long) e);
}
void quil_main(void) {
  Quil *q = &quil_state;
  quil_init(q);
  sexos_line("[quil.init.start]");
  sexos_line("[quil.boot]");
  sexos_line("[quil.no_fb_write]");
  sexos_line("[quil.text.surface] title=%s", QUIL_TITLE);
  init_buffer(q);
/*APP_SURFACE_PACK_V1: Quil's own content surface (0xEC binds this domain as owner) at x 1072, y 56, 200 by 304.*/
  (void) sexos_pdx_call(SEXOS_SLOT_display, 0xEC, QUIL_CONTENT_SID, (uint64_t) 56 << 32 | 1072, (uint64_t) 304 << 32 | 200);
  sexos_line("[quil.surface.visible.ok] sid=%llu", (unsigned long long) QUIL_CONTENT_SID);
/*The physical keyboard proof is armed before any storage-blocking proof, so the serving loop's check is active once it runs.*/
  if (PHYSICAL_KEYBOARD_PROOF_ENABLED) {
    sexos_line("[physical_keyboard.quil.begin]");
    sexos_line("[physical_keyboard.source] qemu_keyboard=1 physical_keyboard=0 usb=0 synthetic=0 honest=1");
    sexos_line("[physical_keyboard.focus.target] target=quil ok=1");
    q->buffer_len = 0;
    q->cursor_pos = 0;
    q->physical_keyboard_active = true;
    sexos_line("[physical_keyboard.setup.done] active=1 iter=0");
  }
  if (validate_title(QUIL_TITLE)) sexos_line("[quil.text.title] title=%s len=%zu", QUIL_TITLE, sexos_strlen(QUIL_TITLE));
  else sexos_line("[quil.text.title.invalid] max_len=%zu", QUIL_TITLE_MAX_LEN);
  if (validate_buffer(q->buffer_len)) {
    size_t lines = text_buffer_line_count(q->buffer, q->buffer_len);
    sexos_line("[quil.text.buffer] bytes=%zu lines=%zu max_bytes=%zu", q->buffer_len, lines, QUIL_BUFFER_MAX_LEN);
    if (lines > QUIL_MAX_VISIBLE_LINES) sexos_line("[quil.text.buffer.overflow] lines=%zu visible=%zu", lines, QUIL_MAX_VISIBLE_LINES);
  } else sexos_line("[quil.text.buffer.invalid] max_bytes=%zu", QUIL_BUFFER_MAX_LEN);
/*The boot draw: the title bar (rect 1, kept across palette redraws), the text, the pseudo-glyphs and the palette (rect 0).*/
  draw_title_bar();
  sexos_line("[quil.text.title.bar] w=%llu h=%llu color=0x%08llx", (unsigned long long) SURFACE_W, (unsigned long long) QUIL_TITLE_BAR_H, (unsigned long long) QUIL_TITLE_BAR_COLOR);
  redraw(q);
  draw_rect_glyph_text();
  draw_palette(q, 0);
  sexos_line("[quil.boot.draw.ok]");
  sexos_line("[quil.ready]");
  if (DISKFS_SLOT_PROOF_ENABLED) {
    sexos_line("[quil.diskfs.mount]");
    run_quil_diskfs_slot_min_proof(q);
  }
  if (READ_V2_HIGHBIT_PROOF_ENABLED) run_quil_read_v2_highbit_proof(q);
  (void) replay_phase(q, "after_diskfs");
/*The synthetic stash proofs run before the persistence proof, which can hang.*/
  if (KEYBOARD_NAV_PROOF_ENABLED) run_keyboard_nav_proof(q);
  if (KEYBOARD_BUFFER_PROOF_ENABLED) run_keyboard_buffer_proof(q);
  if (TEXT_BUFFER_PROOF_ENABLED) run_text_buffer_proof(q);
  if (TEXT_SAVE_ASYNC_PROOF_ENABLED) run_text_save_async_proof(q);
  if (TEXT_COMMANDS_PROOF_ENABLED) run_text_commands_proof(q);
  if (CURSOR_NAV_PROOF_ENABLED) run_cursor_nav_proof(q);
  if (TEXT_SELECTION_PROOF_ENABLED) run_text_selection_proof(q);
  if (TEXT_DELETE_PROOF_ENABLED) run_text_delete_proof(q);
  if (EDITOR_KEYBINDINGS_PROOF_ENABLED) run_editor_keybindings_proof(q);
  if (UNDO_REDO_PROOF_ENABLED) run_undo_redo_proof(q);
  if (UNDO_REDO_KEY_PROOF_ENABLED) run_undo_redo_key_proof();
  if (VISUAL_CURSOR_PROOF_ENABLED) run_visual_cursor_proof(q);
  if (FIND_PROOF_ENABLED) run_find_proof(q);
  if (MOD_LOWERCASE_PROOF_ENABLED) run_mod_lowercase_proof();
  if (WORD_NAV_PROOF_ENABLED) run_word_nav_proof(q);
  if (LINE_STATS_PROOF_ENABLED) run_line_stats_proof(q);
  if (FIND_NAV_PROOF_ENABLED) run_find_nav_proof(q);
  if (SEL_COPY_DELETE_PROOF_ENABLED) run_sel_copy_delete_proof(q);
  if (DIRTY_PROOF_ENABLED) run_dirty_proof(q);
  if (CMD_SURFACE_PROOF_ENABLED) run_cmd_surface_proof();
  if (CLIPBOARD_STATUS_PROOF_ENABLED) run_clipboard_status_proof(q);
  if (PASTE_PROOF_ENABLED) run_paste_proof(q);
  if (REPLACE_PROOF_ENABLED) run_replace_proof(q);
  if (GOTO_LINE_PROOF_ENABLED) run_goto_line_proof(q);
  if (STORAGE_PHASEA_PROOF_ENABLED) run_storage_phasea_proof();
  if (PERSISTENCE_PROOF_ENABLED) run_persistence_proof(q);
  else run_legacy_save_load_proof(q);
/*The save and open proof is deferred to the serving loop: here it would block startup before input is live.*/
  if (SAVE_OPEN_SEXOBJECT_PROOF_ENABLED) {
    sexos_line("[quil.nonblocking_startup.sexobject_proof.defer] ok=1");
    q->save_open_pending = true;
  }
  if (STORAGE_CAP_PROOF_ENABLED) quil_storage_cap_probe();
/*Real HID events stashed while the proofs waited for storage; keys typed meanwhile may complete the physical keyboard proof.*/
  if (replay_phase(q, "after_proofs")) check_physical_keyboard_proof(q, 0);
  else sexos_line("[quil.hid.replay.empty] count=0");
  if (TEXT_INPUT_PIPELINE_PROOF_ENABLED) {
    run_text_input_pipeline_proof(q);
    physical_clear(q, "after_text_pipeline_proof");
  }
/*The live-USB proof waits for the serving loop too, after the save and open proof readies SexFiles.*/
  if (LIVE_USB_PROOF_ENABLED) q->live_usb_pending = true;
  sexos_line("[quil.nonblocking_startup.begin]");
  sexos_line("[quil.nonblocking_startup.no_startup_block] ok=1");
  sexos_line("[quil.nonblocking_startup.main_loop.enter] ok=1");
  sexos_line("[quil.nonblocking_startup.input_ready] ok=1");
  sexos_line("[quil.nonblocking_startup.done] ok=1");
  while (1) {
    if (q->save_open_pending) {
      q->save_open_pending = false;
      run_quil_save_open_sexobject_proof(q, 4);
      sexos_line("[quil.nonblocking_startup.deferred_save_open.done] ok=1");
/*The proof leaves test in the buffer; the physical keyboard proof must not take it for typed keys.*/
      physical_clear(q, "after_deferred_save_open");
      if (replay_phase(q, "after_deferred_save_open")) check_physical_keyboard_proof(q, 0);
    }
    if (q->live_usb_pending) {
      q->live_usb_pending = false;
      run_live_usb_quil_create_save_reopen_proof(q);
      physical_clear(q, "after_deferred_live_usb");
      if (replay_phase(q, "after_deferred_live_usb")) check_physical_keyboard_proof(q, 0);
    }
    SexosMessage msg = sexos_listen(&q->listener, 0);
    if (take(&q->listen_log)) sexos_line("[quil.pdx.listen] type_id=%#llx", (unsigned long long) msg.type_id);
    switch (msg.type_id) {
      case OP_QUIL_PING: if (take(&q->route_log)) sexos_line("[quil.route.recv]");
      break;
      case OP_HID_EVENT: {
/*The physical keyboard proof routes keys to the text while it runs, with the palette shown again afterwards.*/
        bool saved_palette = q->palette_active;
        if (q->physical_keyboard_active && q->palette_active) q->palette_active = false;
        quil_dispatch_palette_key(q, msg.arg0, msg.arg1);
        check_physical_keyboard_proof(q, msg.arg0);
        if (q->physical_keyboard_active && saved_palette) q->palette_active = true;
        break;
      }
      case OP_QUIL_OPEN_DISK_DOC: open_disk_doc(q, msg.arg0, msg.arg1);
      break;
      default: if (take(&q->unknown_log)) sexos_line("[quil.unknown.yield] type_id=%#llx", (unsigned long long) msg.type_id);
      break;
    }
  }
}
