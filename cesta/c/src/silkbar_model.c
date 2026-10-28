#include "silkbar_model.h"
/*SilkBar top-strip model, translated from crates/silkbar-model/src/lib.rs.*/
static uint32_t min_u32(uint32_t a, uint32_t b) {
  return a < b ? a : b;
}
static uint32_t saturating_sub_u32(uint32_t a, uint32_t b) {
  return a > b ? a - b : 0u;
}
/*Equality of the data-carrying cases: tags, then the payload the tag uses.*/
bool silkbar_module_equal(SilkBarModule a, SilkBarModule b) {
  if (a.tag != b.tag) return false;
  if (a.tag == SILKBAR_MODULE_WORKSPACES || a.tag == SILKBAR_MODULE_STATUS_CHIP) return a.index == b.index;
  return true;
}
bool silkbar_action_equal(SilkBarAction a, SilkBarAction b) {
  if (a.tag != b.tag) return false;
  if (a.tag == SILKBAR_ACTION_SWITCH_WORKSPACE) return a.workspace == b.workspace;
  if (a.tag == SILKBAR_ACTION_TOGGLE_MODULE) return silkbar_module_equal(a.module, b.module);
  return true;
}
/*Lighten each RGB component by 0x22, saturating.*/
uint32_t silkbar_lighten(uint32_t c) {
  uint32_t r = min_u32((c & 0xFFu) + 0x22u, 0xFFu);
  uint32_t g = min_u32((c >> 8 & 0xFFu) + 0x22u, 0xFFu);
  uint32_t b = min_u32((c >> 16 & 0xFFu) + 0x22u, 0xFFu);
  return (c & 0xFF000000u) | b << 16 | g << 8 | r;
}
/*Darken each RGB component by 0x44, saturating.*/
uint32_t silkbar_darken(uint32_t c) {
  uint32_t r = saturating_sub_u32(c & 0xFFu, 0x44u);
  uint32_t g = saturating_sub_u32(c >> 8 & 0xFFu, 0x44u);
  uint32_t b = saturating_sub_u32(c >> 16 & 0xFFu, 0x44u);
  return (c & 0xFF000000u) | b << 16 | g << 8 | r;
}
/*The 14 derived bevel, highlight and shadow colours.*/
SilkBarBevelPalette silkbar_theme_bevels(const SilkBarTheme *theme) {
  SilkBarBevelPalette p = (SilkBarBevelPalette) {
    .ws_active_highlight = silkbar_lighten(theme->active),.ws_active_shadow = silkbar_darken(theme->active),.ws_inactive_highlight = silkbar_lighten(theme->muted),.ws_inactive_shadow = silkbar_darken(theme->muted),.chip_highlight = silkbar_lighten(theme->chip_fill),.glow_top_inner = silkbar_darken(theme->panel_glow),.glow_bottom_inner = silkbar_darken(silkbar_darken(theme->panel_glow)),.glow_bottom_outer = silkbar_darken(silkbar_darken(silkbar_darken(theme->panel_glow))),.panel_border_top = silkbar_lighten(theme->panel_fill),.panel_border_bottom = silkbar_darken(theme->panel_fill),.launcher_highlight = silkbar_lighten(theme->panel_fill),.launcher_shadow = silkbar_darken(theme->panel_fill),.launcher_fill = theme->panel_fill,.panel_body = theme->panel_fill
  };
  return p;
}
/*The action of the module at (x, y) in the panel, or none.*/
SilkBarAction silkbar_hit_test_action(const SilkBar *bar, size_t x, size_t y) {
  SilkBarAction none = (SilkBarAction) {
    .tag = SILKBAR_ACTION_NONE
  };
  if (y < PANEL_Y || y >= PANEL_Y + PANEL_H || (x < PANEL_X || x >= PANEL_X + PANEL_W)) return none;
  for (size_t i = 0; i < LAYOUT_COUNT; ++i) {
    const SilkBarLayoutBox *lb = &bar->layout[i];
    if (x >= lb->x && x < lb->x + lb->w && (y >= lb->y && y < lb->y + lb->h)) return lb->action;
  }
  return none;
}
SilkBarUpdate silkbar_update(uint32_t kind, uint8_t index, uint32_t a, uint32_t b) {
  SilkBarUpdate update = (SilkBarUpdate) {
    .kind = kind,.index = index,.a = a,.b = b
  };
  return update;
}
bool silkbar_apply_raw_update(SilkBar *bar, uint32_t kind, uint8_t index, uint32_t a, uint32_t b) {
  return silkbar_apply_update(bar, silkbar_update(kind, index, a, b));
}
/*Apply an update. Unknown kinds, out-of-bounds indices and invalid payloads leave the bar unchanged and return false.*/
bool silkbar_apply_update(SilkBar *bar, SilkBarUpdate update) {
  size_t idx = (size_t) update.index;
  switch (update.kind) {
    case 0: {
      if (idx >= WORKSPACE_COUNT) return false;
      bar->workspaces[idx].active = update.a != 0;
      return true;
    }
    case 1: {
      if (idx >= WORKSPACE_COUNT) return false;
      bar->workspaces[idx].urgent = update.a != 0;
      return true;
    }
    case 2: {
      if (idx >= MAX_CHIPS) return false;
      bar->chips[idx].visible = update.a != 0;
      return true;
    }
    case 3: {
      if (idx >= MAX_CHIPS) return false;
      if (update.a > 3) return false;
      bar->chips[idx].kind = (SilkBarChipKind) update.a;
      return true;
    }
    case 4: {
/*a = hh (0-23); b packs (mm << 8) | ss.*/
      bar->clock_hh = (uint8_t) min_u32(update.a, 23);
      bar->clock_mm = (uint8_t) min_u32(update.b >> 8, 59);
      bar->clock_ss = (uint8_t) min_u32((uint8_t) update.b, 59);
      return true;
    }
    case 5: {
/*SetThemeToken is reserved: theme tokens travel via OP_APPEARANCE_TOKENS.*/
      return false;
    }
    case 6: {
      bar->selected_options_mask = update.a;
      return true;
    }
    case 7: {
      bar->bell_state.total_visible = (uint8_t) update.a;
      bar->bell_state.redacted_count = (uint8_t)(update.a >> 8);
      bar->bell_state.flags = (uint8_t)(update.a >> 16);
      return true;
    }
    case 8: {
      bar->phase1.active_app_sid = update.a;
      return true;
    }
    case 9: {
      if (update.a > 7) return false;
      bar->phase1.accent_tint_idx = (uint8_t) update.a;
      return true;
    }
    case 10: {
      bar->phase1.palette_open = (update.a & 1) != 0;
      bar->phase1.palette_selected = (uint8_t)(update.a >> 1 & 0xFF);
      bar->phase1.palette_available = (uint8_t)(update.a >> 9 & 0xFF);
      return true;
    }
    default: return false;
  }
}
/*Ring buffer. Indices are reduced modulo the capacity on every access, so no state reaches outside the array.*/
SilkBarUpdateQueue silkbar_queue_empty(void) {
  SilkBarUpdateQueue queue = {
    0
  };
  return queue;
}
bool silkbar_queue_push(SilkBarUpdateQueue *queue, SilkBarUpdate update) {
  uint32_t cap = (uint32_t) UPDATE_QUEUE_CAP;
  uint32_t next_tail = (queue->tail + 1) % cap;
  if (next_tail == queue->head) return false;
  queue->updates[queue->tail % cap] = update;
  queue->tail = next_tail;
  return true;
}
bool silkbar_queue_pop(SilkBarUpdateQueue *queue, SilkBarUpdate *out) {
  uint32_t cap = (uint32_t) UPDATE_QUEUE_CAP;
  if (queue->head == queue->tail) return false;
  *out = queue->updates[queue->head % cap];
  queue->head = (queue->head + 1) % cap;
  return true;
}
uint32_t silkbar_queue_drain_into(SilkBarUpdateQueue *queue, SilkBar *bar) {
  uint32_t count = 0;
  SilkBarUpdate update;
  while (silkbar_queue_pop(queue, &update)) if (silkbar_apply_update(bar, update)) ++count;
  return count;
}
/*Queue invariants: empty pop, push/pop identity, full rejection, clock drain.*/
bool silkbar_validate_invariants(void) {
  SilkBarUpdateQueue q = silkbar_queue_empty();
  SilkBarUpdate p;
  if (silkbar_queue_pop(&q, &p)) return false;
  if (!silkbar_queue_push(&q, silkbar_update(4, 0, 10, 30))) return false;
  if (!silkbar_queue_pop(&q, &p)) return false;
  if (p.kind != 4 || p.index != 0 || (p.a != 10 || p.b != 30)) return false;
  SilkBarUpdateQueue q2 = silkbar_queue_empty();
  SilkBarUpdate dummy = silkbar_update(0, 0, 0, 0);
  for (size_t i = 0; i < UPDATE_QUEUE_CAP - 1; ++i) if (!silkbar_queue_push(&q2, dummy)) return false;
  if (silkbar_queue_push(&q2, dummy)) return false;
  SilkBarUpdateQueue q3 = silkbar_queue_empty();
  if (!silkbar_queue_push(&q3, silkbar_update(4, 0, 15, 45u << 8 | 30u))) return false;
  SilkBar bar = DEFAULT_SILK_BAR;
  if (silkbar_queue_drain_into(&q3, &bar) != 1) return false;
  if (bar.clock_hh != 15 || bar.clock_mm != 45 || bar.clock_ss != 30) return false;
  return true;
}
/*Startup gate for silkbar and sexdisplay: 0 passes, 1 contract, 2 vectors.*/
uint32_t validate_silkbar_contract(void) {
  if (!silkbar_validate_contract()) return 1;
  if (!silkbar_validate_deterministic_vectors()) return 2;
  return 0;
}
/*Contract gate for the top strip: rejects ABI and layout drift.*/
bool silkbar_validate_contract(void) {
  if (SILK_DE_CONTRACT_MAGIC == 0) return false;
  if (ABI_VERSION != SILK_DE_BAR_ABI_V1) return false;
  if ((uint32_t) LAYOUT_COUNT != SILK_DE_BAR_LAYOUT_V1) return false;
  if (SILK_DE_BAR_THEME_V1 != 10) return false;
  if (WORKSPACE_COUNT != SILK_DE_REQUIRED_WORKSPACE_SLOTS) return false;
  if (MAX_CHIPS != SILK_DE_REQUIRED_CHIP_SLOTS) return false;
  if (LAYOUT_COUNT != SILK_DE_REQUIRED_MODULES) return false;
  if (MAX_CHIPS != SILK_DE_REQUIRED_CHIPS) return false;
  if (sizeof DEFAULT_SILK_BAR.layout / sizeof DEFAULT_SILK_BAR.layout[0] != SILK_DE_REQUIRED_MODULES) return false;
  if (sizeof DEFAULT_SILK_BAR.chips / sizeof DEFAULT_SILK_BAR.chips[0] != SILK_DE_REQUIRED_CHIPS) return false;
  if (SILKBAR_WORKSPACE_COUNT != WORKSPACE_COUNT) return false;
  if (SILKBAR_CHIP_COUNT != MAX_CHIPS) return false;
  if (SILKBAR_WORKSPACE_IDX_MIN != 0 || SILKBAR_CHIP_IDX_MIN != 0) return false;
  if ((size_t) SILKBAR_WORKSPACE_IDX_MAX + 1 != SILKBAR_WORKSPACE_COUNT) return false;
  if ((size_t) SILKBAR_CHIP_IDX_MAX + 1 != SILKBAR_CHIP_COUNT) return false;
  if (SILKBAR_DEFAULT_ACTIVE_WORKSPACE_IDX > SILKBAR_WORKSPACE_IDX_MAX) return false;
  if (SILKBAR_UPDATE_SIZE != 16) return false;
  if (UPDATE_QUEUE_CAP != 32) return false;
  if (DEFAULT_THEME.bg_top == 0 || DEFAULT_THEME.bg_bottom == 0 || (DEFAULT_THEME.panel_fill == 0 || DEFAULT_THEME.panel_glow == 0) || (DEFAULT_THEME.text == 0 || DEFAULT_THEME.active == 0 || (DEFAULT_THEME.urgent == 0 || DEFAULT_THEME.chip_fill == 0 || DEFAULT_THEME.chip_border == 0))) return false;
/*Renderer-safe top-strip geometry: bounded and non-zero.*/
  if (PANEL_W == 0 || PANEL_H == 0 || (PANEL_W > 4096 || PANEL_H > 256)) return false;
/*ChipSlot discriminants must match the renderer's CHIP_SLOTS indices.*/
  if (SILKBAR_CHIP_SLOT_CHIP0 != 0) return false;
  if (SILKBAR_CHIP_SLOT_CHIP1 != 1) return false;
  if (SILKBAR_CHIP_SLOT_CHIP2 != 2) return false;
  if (SILKBAR_CHIP_SLOT_CLOCK != 3) return false;
  return true;
}
uint64_t silkbar_contract_fingerprint(void) {
  uint64_t fp = (uint64_t) SILK_DE_CONTRACT_MAGIC;
  fp ^= (uint64_t) SILK_DE_BAR_ABI_V1 << 32;
  fp ^= (uint64_t) SILK_DE_BAR_LAYOUT_V1 << 24;
  fp ^= (uint64_t) SILK_DE_BAR_THEME_V1 << 16;
  fp ^= (uint64_t) SILK_DE_REQUIRED_WORKSPACE_SLOTS << 8;
  fp ^= (uint64_t) SILK_DE_REQUIRED_CHIP_SLOTS;
  fp ^= (uint64_t) SILKBAR_UPDATE_SIZE << 40;
  fp ^= (uint64_t) UPDATE_QUEUE_CAP << 48;
  return fp;
}
/*Deterministic vectors for update semantics, checked before the renderer starts.*/
bool silkbar_validate_deterministic_vectors(void) {
  SilkBar bar = DEFAULT_SILK_BAR;
  SilkBarUpdateQueue q = silkbar_queue_empty();
  SilkBarUpdate vectors[10] = {
    silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_ACTIVE, 0, 1, 0), silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_ACTIVE, 2, 0, 0), silkbar_update(SILKBAR_UPDATE_SET_WORKSPACE_URGENT, 1, 1, 0), silkbar_update(SILKBAR_UPDATE_SET_CHIP_VISIBLE, 2, 0, 0), silkbar_update(SILKBAR_UPDATE_SET_CHIP_KIND, 1, SILKBAR_CHIP_BATTERY, 0), silkbar_update(SILKBAR_UPDATE_SET_CLOCK, 0, 23, 59u << 8 | 58u), silkbar_update(SILKBAR_UPDATE_SET_SELECTED_OPTIONS, 0, OPTION_MOVE, 0), silkbar_update(SILKBAR_UPDATE_SET_ACTIVE_APP, 0, 200, 0), silkbar_update(SILKBAR_UPDATE_SET_TINT_ACCENT, 0, 3, 0), silkbar_update(SILKBAR_UPDATE_SET_PALETTE_STATE, 0, 1u | 2u << 1 | 7u << 9, 0)
  };
  for (size_t i = 0; i < 10; ++i) if (!silkbar_queue_push(&q, vectors[i])) return false;
  if (silkbar_queue_drain_into(&q, &bar) != 10) return false;
  if (!bar.workspaces[0].active || bar.workspaces[2].active) return false;
  if (!bar.workspaces[1].urgent) return false;
  if (bar.chips[2].visible) return false;
  if (bar.chips[1].kind != SILKBAR_CHIP_BATTERY) return false;
  if (bar.clock_hh != 23 || bar.clock_mm != 59 || bar.clock_ss != 58) return false;
  if (bar.selected_options_mask != OPTION_MOVE) return false;
  if (bar.phase1.active_app_sid != 200) return false;
  if (bar.phase1.accent_tint_idx != 3) return false;
  if (!bar.phase1.palette_open) return false;
  if (bar.phase1.palette_selected != 2) return false;
  if (bar.phase1.palette_available != 7) return false;
  return true;
}
/*Default instances.*/
const SilkBar DEFAULT_SILK_BAR = {
  .layout = {
    {
      LAUNCHER_X, LAUNCHER_Y, LAUNCHER_W, LAUNCHER_H, {
        SILKBAR_MODULE_LAUNCHER, 0
      }, {
        SILKBAR_ACTION_OPEN_LAUNCHER, 0, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }, {
      WS_X0, WS_Y, WS_INACTIVE_W, WS_H, {
        SILKBAR_MODULE_WORKSPACES, 0
      }, {
        SILKBAR_ACTION_SWITCH_WORKSPACE, 1, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }, {
      WS_X1, WS_Y, WS_INACTIVE_W, WS_H, {
        SILKBAR_MODULE_WORKSPACES, 1
      }, {
        SILKBAR_ACTION_SWITCH_WORKSPACE, 2, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }, {
      WS_X2, WS_Y, WS_ACTIVE_W, WS_H, {
        SILKBAR_MODULE_WORKSPACES, 2
      }, {
        SILKBAR_ACTION_SWITCH_WORKSPACE, 3, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }, {
      WS_X3, WS_Y, WS_INACTIVE_W, WS_H, {
        SILKBAR_MODULE_WORKSPACES, 3
      }, {
        SILKBAR_ACTION_SWITCH_WORKSPACE, 4, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }, {
      WS_X4, WS_Y, WS_INACTIVE_W, WS_H, {
        SILKBAR_MODULE_WORKSPACES, 4
      }, {
        SILKBAR_ACTION_SWITCH_WORKSPACE, 5, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }, {
      CHIP_X0, CHIP_Y, CHIP_W, CHIP_H, {
        SILKBAR_MODULE_STATUS_CHIP, 0
      }, {
        SILKBAR_ACTION_TOGGLE_MODULE, 0, {
          SILKBAR_MODULE_STATUS_CHIP, 0
        }
      }
    }, {
      CHIP_X1, CHIP_Y, CHIP_W, CHIP_H, {
        SILKBAR_MODULE_STATUS_CHIP, 1
      }, {
        SILKBAR_ACTION_TOGGLE_MODULE, 0, {
          SILKBAR_MODULE_STATUS_CHIP, 1
        }
      }
    }, {
      CHIP_X2, CHIP_Y, CHIP_W, CHIP_H, {
        SILKBAR_MODULE_STATUS_CHIP, 2
      }, {
        SILKBAR_ACTION_TOGGLE_MODULE, 0, {
          SILKBAR_MODULE_STATUS_CHIP, 2
        }
      }
    }, {
      CHIP_X3, CHIP_Y, CLOCK_W, CHIP_H, {
        SILKBAR_MODULE_CLOCK, 0
      }, {
        SILKBAR_ACTION_OPEN_CLOCK, 0, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }, {
      CHIP_X_BELL, CHIP_Y, CHIP_W, CHIP_H, {
        SILKBAR_MODULE_BELL, 0
      }, {
        SILKBAR_ACTION_OPEN_BELL, 0, {
          SILKBAR_MODULE_LAUNCHER, 0
        }
      }
    }
  },.workspaces = {
    {
      1, false, false
    }, {
      2, false, false
    }, {
      3, true, false
    }, {
      4, false, false
    }, {
      5, false, false
    }
  },.chips = {
    {
      SILKBAR_CHIP_NET, true
    }, {
      SILKBAR_CHIP_WIFI, true
    }, {
      SILKBAR_CHIP_BATTERY, true
    }, {
      SILKBAR_CHIP_CLOCK, true
    }
  },.clock_hh = 10,.clock_mm = 42,.clock_ss = 0,.selected_options_mask = 0,.bell_state = {
    0, 0, 0, 0
  },.phase1 = {
    0, 0, false, 0, 0, 0
  }
};
const SilkBarTheme DEFAULT_THEME = {
  .bg_top = 0x00081424u,.bg_bottom = 0x001E1E2Eu,.panel_fill = 0x00313244u,.panel_glow = 0x0045475Au,.text = 0x00CDD6F4u,.muted = 0x00304068u,.active = 0x00A8A0FFu,.urgent = 0x00FF6666u,.chip_fill = 0x0089B4FAu,.chip_border = 0x006F86A8u
};

