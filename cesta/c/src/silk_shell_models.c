#include "silk_shell.h"
#include "silk_shell_models_root.h"
/*Application manifests, canvas operations and shell object models.*/
const uint8_t SPINDLE_SYNTH_SEQ[6] = {
  0x1E, 0x30, 0x2E, 0x0E, 0x20, 0x1C
};
const AppSurfaceSpec APP_SURFACES[8] = {
  {
    .surface_id = SURFACE_ID_LINEN,.frame_id = LINEN_FRAME_ID,.name = "linen",.boot_x = LINEN_BOOT_X,.boot_y = LINEN_BOOT_Y,.boot_w = LINEN_BOOT_W,.boot_h = LINEN_BOOT_H,.closeable = true,.focusable = true
  }, {
    .surface_id = SURFACE_ID_QUIL,.frame_id = QUIL_FRAME_ID,.name = "quil",.boot_x = QUIL_BOOT_X,.boot_y = QUIL_BOOT_Y,.boot_w = QUIL_BOOT_W,.boot_h = QUIL_BOOT_H,.closeable = true,.focusable = true
  }, {
    .surface_id = SURFACE_ID_MESH,.frame_id = MESH_FRAME_ID,.name = "mesh",.boot_x = MESH_BOOT_X,.boot_y = MESH_BOOT_Y,.boot_w = MESH_BOOT_W,.boot_h = MESH_BOOT_H,.closeable = true,.focusable = true
  }, {
    .surface_id = SURFACE_ID_COLLAR,.frame_id = COLLAR_FRAME_ID,.name = "collar",.boot_x = COLLAR_BOOT_X,.boot_y = COLLAR_BOOT_Y,.boot_w = COLLAR_BOOT_W,.boot_h = COLLAR_BOOT_H,.closeable = true,.focusable = true
  }, {
    .surface_id = SURFACE_ID_BELL_PLACEHOLDER,.frame_id = BELL_FRAME_ID,.name = "bell",.boot_x = BELL_BOOT_X,.boot_y = BELL_BOOT_Y,.boot_w = BELL_BOOT_W,.boot_h = BELL_BOOT_H,.closeable = true,.focusable = true
  }, {
    .surface_id = SURFACE_ID_COMMAND_PALETTE,.frame_id = COMMAND_PALETTE_FRAME_ID,.name = "command_palette",.boot_x = COMMAND_PALETTE_BOOT_X,.boot_y = COMMAND_PALETTE_BOOT_Y,.boot_w = COMMAND_PALETTE_BOOT_W,.boot_h = COMMAND_PALETTE_BOOT_H,.closeable = false,.focusable = true
  }, {
    .surface_id = SURFACE_ID_SPINDLE,.frame_id = SPINDLE_FRAME_ID,.name = "spindle",.boot_x = SPINDLE_BOOT_X,.boot_y = SPINDLE_BOOT_Y,.boot_w = SPINDLE_BOOT_W,.boot_h = SPINDLE_BOOT_H,.closeable = true,.focusable = true
  }, {
    .surface_id = SURFACE_ID_BROWSER,.frame_id = BROWSER_FRAME_ID,.name = "browser",.boot_x = BROWSER_BOOT_X,.boot_y = BROWSER_BOOT_Y,.boot_w = BROWSER_BOOT_W,.boot_h = BROWSER_BOOT_H,.closeable = true,.focusable = true
  }
};
const LinenObject LINEN_SEED_OBJECTS[6] = {
  {
    .present = true,.object_id = 1,.kind = LINEN_OBJ_KIND_PROJECT,.state = LINEN_OBJ_STATE_LOADED,.parent_id = 0,.project_id = 1,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "SexOS Kernel",.name = {
      0
    },.name_len = 0,.disk_path = 0
  }, {
    .present = true,.object_id = 2,.kind = LINEN_OBJ_KIND_DOCUMENT,.state = LINEN_OBJ_STATE_SAVED,.parent_id = 1,.project_id = 1,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Compositor Lifecycle Spec",.name = {
      0
    },.name_len = 0,.disk_path = 0
  }, {
    .present = true,.object_id = 3,.kind = LINEN_OBJ_KIND_CODE_FILE,.state = LINEN_OBJ_STATE_LOADED,.parent_id = 1,.project_id = 1,.grant_ref = 0,.linked_surface_id = SURFACE_ID_LINEN,.flags = 0,.display_name = "Silk Shell main.rs",.name = {
      0
    },.name_len = 0,.disk_path = 0
  }, {
    .present = true,.object_id = 4,.kind = LINEN_OBJ_KIND_MEDIA_ASSET,.state = LINEN_OBJ_STATE_SAVED,.parent_id = 0,.project_id = 0,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Desktop Screenshot",.name = {
      0
    },.name_len = 0,.disk_path = 0
  }, {
    .present = true,.object_id = 5,.kind = LINEN_OBJ_KIND_BUILD_ARTIFACT,.state = LINEN_OBJ_STATE_SAVED,.parent_id = 1,.project_id = 1,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Current ISO Build",.name = {
      0
    },.name_len = 0,.disk_path = 0
  }, {
    .present = true,.object_id = 6,.kind = LINEN_OBJ_KIND_FOLDER,.state = LINEN_OBJ_STATE_LOADED,.parent_id = 0,.project_id = 0,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Drafts",.name = {
      0
    },.name_len = 0,.disk_path = 0
  }
};
const uint32_t LINEN_UI_ROW_COLORS[5] = {
  0x003060A0, 0x006040A0, 0x00306060, 0x00805030, 0x00204060
};
const QuilBuffer QUIL_SEED_BUFFERS[6] = {
  {
    .present = true,.buffer_id = 1,.kind = QUIL_BUF_KIND_CODE,.state = QUIL_BUF_STATE_OPEN,.linen_object_ref = 0,.project_id = 1,.grant_ref = 0,.linked_surface_id = SURFACE_ID_QUIL,.flags = 0,.display_name = "main.rs"
  }, {
    .present = true,.buffer_id = 2,.kind = QUIL_BUF_KIND_TEXT,.state = QUIL_BUF_STATE_OPEN,.linen_object_ref = 2,.project_id = 1,.grant_ref = 0,.linked_surface_id = SURFACE_ID_QUIL,.flags = 0,.display_name = "Compositor Lifecycle Spec"
  }, {
    .present = true,.buffer_id = 3,.kind = QUIL_BUF_KIND_DESIGN_NOTE,.state = QUIL_BUF_STATE_OPEN,.linen_object_ref = 0,.project_id = 1,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Frame Tiling Design"
  }, {
    .present = true,.buffer_id = 4,.kind = QUIL_BUF_KIND_BUILD_OUTPUT,.state = QUIL_BUF_STATE_READ_ONLY,.linen_object_ref = 5,.project_id = 1,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Current ISO Build"
  }, {
    .present = true,.buffer_id = 5,.kind = QUIL_BUF_KIND_REVIEW_NOTE,.state = QUIL_BUF_STATE_OPEN,.linen_object_ref = 0,.project_id = 1,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Review: A7 Opcode Audit"
  }, {
    .present = true,.buffer_id = 6,.kind = QUIL_BUF_KIND_AGENT_TASK,.state = QUIL_BUF_STATE_ALLOCATED,.linen_object_ref = 0,.project_id = 0,.grant_ref = 0,.linked_surface_id = 0,.flags = 0,.display_name = "Refactor tiling loop"
  }
};
const uint32_t TOKEN_PRESETS[PRESET_COUNT][8] = {
  {
    0x007AAFA4, 0x00B8F2E8, 0x0088C2B7, 0x007AAFA4, 0x006080B0, 0x00FF4444, 0x00FFCC44, 0x0044FF44
  }, {
    0x00503080, 0x00A060FF, 0x00604090, 0x00503080, 0x00302050, 0x00FF4080, 0x00FFAA00, 0x0040FF80
  }, {
    0x00282828, 0x00808080, 0x00404040, 0x00505050, 0x00303030, 0x00CC4444, 0x00CCAA44, 0x0044CC44
  }, {
    0x00000000, 0x00FFFFFF, 0x00111111, 0x00FFFF00, 0x00555555, 0x00FF4444, 0x00FFDD00, 0x0000FF44
  }
};
const char *const PRESET_NAMES[PRESET_COUNT] = {
  "Default", "Warm", "Cool", "HighContrast"
};
const SceneAppearanceState DEFAULT_SCENE_APPEARANCE = {
  .preset_idx = 0,.use_custom_colors = 0,.chrome_flags = 0,.accessibility_flags = 0,.custom_colors = {
    0
  }
};
const uint32_t CUSTOM_TINT_BUNDLES[TINT_COUNT][8] = {
  {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
  }, {
    0x00000000, 0x00D4822A, 0x00B86420, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
  }, {
    0x00000000, 0x0080C8FF, 0x004488CC, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
  }, {
    0x00CC5566, 0x00FF8090, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
  }, {
    0x00000000, 0x00DDBB00, 0x00000000, 0x00DDBB00, 0x00000000, 0x00000000, 0x00000000, 0x00000000
  }
};
const DesktopPolicy P = {
  .width = 1280,.height = 720,.bar_height = 50,.move_step = 10,.resize_step = 20,.min_width = 120,.min_height = 80,.boot_rect_100 = {
    100, 100, 800, 500
  },.boot_rect_101 = {
    180, 160, 500, 300
  },.boot_rect_102 = {
    50, 60, 350, 150
  },.boot_rect_103 = {
    900, 560, 300, 120
  }
};
const ChromeTemplate SILK_CHROME_TEMPLATE_DEFAULT = {
  .rim_px = 4,.top_bar_height_px = 28,.light_size_px = 4,.light_gap_px = 2,.top_bar_light_size_px = 10,.top_bar_light_gap_px = 5,.top_bar_light_exclusion_px = 50,.tab_light_exclusion_px = 20,.tab_min_width_px = 12,.tab_strip_px = 4,.settings_panel_x = 870,.settings_panel_y = 60,.settings_panel_w = 340,.settings_panel_h = 280,.control_preset_up = {
    0, 0, 0, 0
  },.control_preset_down = {
    0, 0, 0, 0
  },.control_reset = {
    0, 0, 0, 0
  },.control_close = {
    0, 0, 0, 0
  },.control_topbar_toggle = {
    0, 0, 0, 0
  }
};
const uint32_t ATLAS_ACCENT_COLORS[5] = {
  0x00000000, 0x00805020, 0x00205080, 0x00804050, 0x00807000
};
bool app_capability_bits_validate(uint8_t bits, AppCapabilityBits *out) {
  if ((bits & (uint8_t) ~APP_CAP_KNOWN) != 0) return false;
  if (out != NULL) out->bits = bits;
  return true;
}
uint8_t app_capability_bits_bits(AppCapabilityBits self) {
  return self.bits;
}
bool app_capability_bits_has_bell(AppCapabilityBits self) {
  return (self.bits & APP_CAP_BELL) != 0;
}
bool app_capability_bits_has_sexfiles(AppCapabilityBits self) {
  return (self.bits & APP_CAP_SEXFILES) != 0;
}
const char *app_capability_bits_describe(AppCapabilityBits self) {
  switch (self.bits) {
    case APP_CAP_NONE: return "none";
    case APP_CAP_BELL: return "bell";
    case APP_CAP_SEXFILES: return "sexfiles";
    case APP_CAP_KNOWN: return "bell+sexfiles";
    default: return "unknown";
  }
}
void app_manifest_pack(const AppManifest *self, uint64_t *arg0, uint64_t *arg1, uint64_t *arg2) {
  if (arg0 != NULL) *arg0 = self->surface_id;
  if (arg1 != NULL) *arg1 = self->title_id;
  if (arg2 != NULL) *arg2 = (uint64_t) self->app_id << 8 | self->capabilities.bits;
}
bool app_manifest_unpack(uint64_t surface_id, uint64_t title_id, uint64_t arg2, AppManifest *out) {
  if ((arg2 & UINT64_C(0xFFFFFFFFFF000000)) != 0) return false;
  AppCapabilityBits capabilities;
  if (!app_capability_bits_validate((uint8_t) arg2, &capabilities)) return false;
  if (out != NULL) {
    *out = (AppManifest) {
      .surface_id = surface_id,.title_id = title_id,.app_id = (uint16_t)(arg2 >> 8),.capabilities = capabilities,
    };
  }
  return true;
}
bool app_manifest_has_identity(const AppManifest *self) {
  return self->app_id != 0;
}
Canvas canvas_new(uint32_t *fb_ptr, uint32_t w, uint32_t h) {
  return (Canvas) {
    .fb = fb_ptr,.fb_len = (size_t)(w * h),.width = w,.height = h
  };
}
void canvas_fill_rect(Canvas *self, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
  uint32_t y_end = y + h;
  uint32_t x_end = x + w;
  if (y_end > self->height) y_end = self->height;
  if (x_end > self->width) x_end = self->width;
  for (uint32_t py = y; py < y_end; ++py) {
    for (uint32_t px = x; px < x_end; ++px) {
      size_t index = (size_t)(py * self->width + px);
      if (index < self->fb_len) self->fb[index] = color;
    }
  }
}
void canvas_draw_panel(Canvas *self, const ShellState *state) {
  canvas_fill_rect(self, 0, 0, self->width, PANEL_HEIGHT, UINT32_C(0xFF0A0A14));
  if (state->is_launcher_open) canvas_fill_rect(self, 0, PANEL_HEIGHT, LAUNCHER_WIDTH, 400, UINT32_C(0xFF1E1E2E));
}
uint32_t pdx_compositor_client_create_window(int32_t x, int32_t y, uint32_t w, uint32_t h) {
  (void) h;
  return (uint32_t) sexos_pdx_call(SLOT_DISPLAY, OP_WINDOW_CREATE, (uint64_t) x, (uint64_t) y, w).value;
}
void pdx_compositor_client_set_bg(uint32_t color) {
  (void) sexos_pdx_call(SLOT_DISPLAY, OP_SET_BG, color, 0, 0);
}
void pdx_compositor_client_render_bar(uint32_t window_id) {
  (void) sexos_pdx_call(SLOT_DISPLAY, OP_RENDER_BAR, window_id, 0, 0);
}
void linen_object_table_init(void) {
  for (size_t i = 0; i < sizeof LINEN_SEED_OBJECTS / sizeof LINEN_SEED_OBJECTS[0]; ++i) {
    if (i < LINEN_MAX_OBJECTS) {
      LinenObject object = LINEN_SEED_OBJECTS[i];
      object.present = true;
      SILK_SHELL_STATE.models.LINEN_OBJECTS[i] = object;
      sexos_line("[linen.object.seed] id=%llu kind=%u name=%s", (unsigned long long) object.object_id, (unsigned) object.kind, object.display_name);
    }
  }
  sexos_line("[linen.object_table.init] count=%zu", sizeof LINEN_SEED_OBJECTS / sizeof LINEN_SEED_OBJECTS[0]);
}
size_t linen_object_count(void) {
  size_t count = 0;
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) if (SILK_SHELL_STATE.models.LINEN_OBJECTS[i].present) ++count;
  return count;
}
bool linen_object_by_id(uint64_t id, LinenObject *out) {
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
    const LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
    if (object->present && object->object_id == id) {
      if (out != NULL) *out = *object;
      return true;
    }
  }
  return false;
}
const char *linen_object_kind_name(uint8_t kind) {
  switch (kind) {
    case LINEN_OBJ_KIND_PROJECT: return "Project";
    case LINEN_OBJ_KIND_DOCUMENT: return "Document";
    case LINEN_OBJ_KIND_CODE_FILE: return "CodeFile";
    case LINEN_OBJ_KIND_MEDIA_ASSET: return "MediaAsset";
    case LINEN_OBJ_KIND_BUILD_ARTIFACT: return "BuildArtifact";
    case LINEN_OBJ_KIND_FOLDER: return "Folder";
    case LINEN_OBJ_KIND_REFERENCE: return "Reference";
    case LINEN_OBJ_KIND_IMPORT_PLACEHOLDER: return "ImportPlaceholder";
    case LINEN_OBJ_KIND_BELL_EVENT_REFERENCE: return "BellEventRef";
    case LINEN_OBJ_KIND_QUIL_WORKSPACE_REFERENCE: return "QuilWorkspaceRef";
    case LINEN_OBJ_KIND_MESH_DIAGNOSTIC_REFERENCE: return "MeshDiagRef";
    default: return "unknown";
  }
}
const char *linen_object_state_name(uint8_t state) {
  switch (state) {
    case LINEN_OBJ_STATE_ALLOCATED: return "Allocated";
    case LINEN_OBJ_STATE_LOADED: return "Loaded";
    case LINEN_OBJ_STATE_MODIFIED: return "Modified";
    case LINEN_OBJ_STATE_SAVED: return "Saved";
    case LINEN_OBJ_STATE_ARCHIVED: return "Archived";
    default: return "unknown";
  }
}
void linen_select_first_valid_object(void) {
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
    const LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
    if (object->present) {
      SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = object->object_id;
      sexos_line("[linen.object_select.current] id=%llu", (unsigned long long) object->object_id);
      return;
    }
  }
  SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = 0;
}
uint64_t linen_selected_object_id(void) {
  if (SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID == 0) {
    linen_select_first_valid_object();
    if (SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID == 0) {
      sexos_line("[linen.object_select.reject] reason=no_objects");
      return 0;
    }
    sexos_line("[linen.object_select.repair] id=%llu", (unsigned long long) SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID);
  }
  sexos_line("[linen.object_select.current] id=%llu", (unsigned long long) SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID);
  return SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID;
}
void linen_select_next_object(void) {
  uint64_t current = SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID;
  bool found_current = false;
  uint64_t first_valid = 0;
  uint64_t next_valid = 0;
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
    const LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
    if (!object->present) continue;
    if (first_valid == 0) first_valid = object->object_id;
    if (found_current && next_valid == 0) {
      next_valid = object->object_id;
      break;
    }
    if (object->object_id == current) found_current = true;
  }
  if (next_valid != 0) {
    SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = next_valid;
    sexos_line("[linen.object_select.next] prev=%llu next=%llu", (unsigned long long) current, (unsigned long long) next_valid);
  } else if (first_valid != 0 && current != first_valid) {
    SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = first_valid;
    sexos_line("[linen.object_select.next] prev=%llu next=%llu wrap", (unsigned long long) current, (unsigned long long) first_valid);
  } else {
    sexos_line("[linen.object_select.reject] reason=single_object id=%llu", (unsigned long long) current);
  }
}
void linen_select_prev_object(void) {
  uint64_t current = SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID;
  uint64_t prev_valid = 0;
  uint64_t last_valid = 0;
  uint64_t first_valid = 0;
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
    const LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
    if (!object->present) continue;
    if (first_valid == 0) first_valid = object->object_id;
    if (object->object_id == current) break;
    prev_valid = object->object_id;
    last_valid = object->object_id;
  }
  if (prev_valid != 0 && prev_valid != current) {
    SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = prev_valid;
    sexos_line("[linen.object_select.prev] prev=%llu current=%llu", (unsigned long long) prev_valid, (unsigned long long) current);
  } else if (last_valid != 0 && last_valid != current) {
    SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = last_valid;
    sexos_line("[linen.object_select.prev] prev=%llu current=%llu wrap", (unsigned long long) last_valid, (unsigned long long) current);
  } else {
    sexos_line("[linen.object_select.reject] reason=single_object id=%llu", (unsigned long long) current);
  }
}
size_t linen_selected_index(void) {
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
    const LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
    if (object->present && object->object_id == SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID) return i;
  }
  return 0;
}
void linen_object_detail_open(void) {
  if (SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID != SURFACE_ID_LINEN) {
    sexos_line("[linen.detail.reject] reason=not_focused");
    sexos_line("[linen.detail.open] idx=0 object_id=0 ok=0 reason=not_focused");
    return;
  }
  uint64_t id = linen_selected_object_id();
  if (id == 0) {
    sexos_line("[linen.detail.reject] reason=no_object");
    sexos_line("[linen.detail.open] idx=0 object_id=0 ok=0 reason=no_object");
    return;
  }
  LinenObject object;
  if (!linen_object_by_id(id, &object)) {
    sexos_line("[linen.detail.reject] reason=object_not_found id=%llu", (unsigned long long) id);
    sexos_line("[linen.detail.open] idx=0 object_id=%llu ok=0 reason=object_not_found", (unsigned long long) id);
    return;
  }
  SILK_SHELL_STATE.models.LINEN_OBJECT_DETAIL_OPEN = true;
  sexos_line("[linen.detail.open] idx=%zu object_id=%llu ok=1 reason=ok", linen_selected_index(), (unsigned long long) id);
  sexos_line("[linen.detail.metadata] object_id=%llu kind=%s state=%s parent_id=%llu grant_ref=%llu", (unsigned long long) id, linen_object_kind_name(object.kind), linen_object_state_name(object.state), (unsigned long long) object.parent_id, (unsigned long long) object.grant_ref);
}
void linen_object_detail_close(void) {
  if (SILK_SHELL_STATE.models.LINEN_OBJECT_DETAIL_OPEN) {
    SILK_SHELL_STATE.models.LINEN_OBJECT_DETAIL_OPEN = false;
    sexos_line("[linen.detail.close] ok=1 reason=ok");
  } else {
    sexos_line("[linen.detail.close] ok=0 reason=not_open");
  }
}
uint32_t linen_kind_color(uint8_t kind) {
  switch (kind) {
    case LINEN_OBJ_KIND_PROJECT: return UINT32_C(0x004080C0);
    case LINEN_OBJ_KIND_DOCUMENT: return UINT32_C(0x0040C080);
    case LINEN_OBJ_KIND_CODE_FILE: return UINT32_C(0x00C0A040);
    case LINEN_OBJ_KIND_MEDIA_ASSET: return UINT32_C(0x00C04080);
    case LINEN_OBJ_KIND_BUILD_ARTIFACT: return UINT32_C(0x00806040);
    case LINEN_OBJ_KIND_FOLDER: return UINT32_C(0x00808080);
    case LINEN_OBJ_KIND_REFERENCE: return UINT32_C(0x006060C0);
    case LINEN_OBJ_KIND_IMPORT_PLACEHOLDER: return UINT32_C(0x00C06040);
    case LINEN_OBJ_KIND_BELL_EVENT_REFERENCE: return UINT32_C(0x00C04040);
    case LINEN_OBJ_KIND_QUIL_WORKSPACE_REFERENCE: return UINT32_C(0x0040C0C0);
    case LINEN_OBJ_KIND_MESH_DIAGNOSTIC_REFERENCE: return UINT32_C(0x00A060C0);
    default: return 0;
  }
}
uint32_t linen_selected_object_accent(void) {
  uint64_t id = SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID;
  if (id != 0) {
    for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
      const LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
      if (object->present && object->object_id == id) return linen_kind_color(object->kind);
    }
  }
  return LINEN_LIST_HEADER_COLOR;
}
void quil_buffer_table_init(void) {
  for (size_t i = 0; i < sizeof QUIL_SEED_BUFFERS / sizeof QUIL_SEED_BUFFERS[0]; ++i) {
    if (i < QUIL_MAX_BUFFERS) {
      QuilBuffer buffer = QUIL_SEED_BUFFERS[i];
      buffer.present = true;
      SILK_SHELL_STATE.models.QUIL_BUFFERS[i] = buffer;
      sexos_line("[quil.buffer.seed] id=%llu kind=%u name=%s", (unsigned long long) buffer.buffer_id, (unsigned) buffer.kind, buffer.display_name);
    }
  }
  sexos_line("[quil.buffer_table.init] count=%zu", sizeof QUIL_SEED_BUFFERS / sizeof QUIL_SEED_BUFFERS[0]);
}
size_t quil_buffer_count(void) {
  size_t count = 0;
  for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) if (SILK_SHELL_STATE.models.QUIL_BUFFERS[i].present) ++count;
  return count;
}
bool quil_buffer_by_id(uint64_t id, QuilBuffer *out) {
  for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) {
    const QuilBuffer *buffer = &SILK_SHELL_STATE.models.QUIL_BUFFERS[i];
    if (buffer->present && buffer->buffer_id == id) {
      if (out != NULL) *out = *buffer;
      return true;
    }
  }
  return false;
}
const char *quil_buffer_kind_name(uint8_t kind) {
  switch (kind) {
    case QUIL_BUF_KIND_TEXT: return "Text";
    case QUIL_BUF_KIND_CODE: return "Code";
    case QUIL_BUF_KIND_DESIGN_NOTE: return "DesignNote";
    case QUIL_BUF_KIND_REVIEW_NOTE: return "ReviewNote";
    case QUIL_BUF_KIND_DIAGNOSTIC: return "Diagnostic";
    case QUIL_BUF_KIND_BUILD_OUTPUT: return "BuildOutput";
    case QUIL_BUF_KIND_AGENT_TASK: return "AgentTask";
    case QUIL_BUF_KIND_LINEN_OBJECT_VIEW: return "LinenObjectView";
    default: return "unknown";
  }
}
const char *quil_buffer_state_name(uint8_t state) {
  switch (state) {
    case QUIL_BUF_STATE_ALLOCATED: return "Allocated";
    case QUIL_BUF_STATE_OPEN: return "Open";
    case QUIL_BUF_STATE_DIRTY: return "Dirty";
    case QUIL_BUF_STATE_READ_ONLY: return "ReadOnly";
    case QUIL_BUF_STATE_CLOSED: return "Closed";
    case QUIL_BUF_STATE_MISSING: return "Missing";
    default: return "unknown";
  }
}
uint32_t quil_buffer_kind_color(uint8_t kind) {
  switch (kind) {
    case QUIL_BUF_KIND_TEXT: return UINT32_C(0x00808080);
    case QUIL_BUF_KIND_CODE: return UINT32_C(0x0040A060);
    case QUIL_BUF_KIND_DESIGN_NOTE: return UINT32_C(0x004060C0);
    case QUIL_BUF_KIND_REVIEW_NOTE: return UINT32_C(0x00C06040);
    case QUIL_BUF_KIND_DIAGNOSTIC: return UINT32_C(0x00C04080);
    case QUIL_BUF_KIND_BUILD_OUTPUT: return UINT32_C(0x00806040);
    case QUIL_BUF_KIND_AGENT_TASK: return UINT32_C(0x006080C0);
    case QUIL_BUF_KIND_LINEN_OBJECT_VIEW: return UINT32_C(0x00A060C0);
    default: return 0;
  }
}
void linen_quil_seed_coherence_init(void) {
  size_t linked = 0;
  for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) {
    const QuilBuffer *buffer = &SILK_SHELL_STATE.models.QUIL_BUFFERS[i];
    if (!buffer->present || buffer->linen_object_ref == 0 || buffer->linked_surface_id == 0) continue;
    for (size_t j = 0; j < LINEN_MAX_OBJECTS; ++j) {
      LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[j];
      if (!object->present || object->object_id != buffer->linen_object_ref) continue;
      if (object->linked_surface_id != buffer->linked_surface_id) {
        object->linked_surface_id = buffer->linked_surface_id;
        sexos_line("[linen.quil.seed_link] object_id=%llu buffer_id=%llu surface_id=%llu", (unsigned long long) object->object_id, (unsigned long long) buffer->buffer_id, (unsigned long long) buffer->linked_surface_id);
        ++linked;
      }
      break;
    }
  }
  sexos_line("[linen.quil.seed_coherence.done] linked=%zu", linked);
}
const AppSurfaceSpec *app_surface_spec(uint64_t surface_id) {
  for (size_t i = 0; i < sizeof APP_SURFACES / sizeof APP_SURFACES[0]; ++i) if (APP_SURFACES[i].surface_id == surface_id) return &APP_SURFACES[i];
  return NULL;
}
const AppSurfaceSpec *app_surface_spec_by_frame(uint32_t frame_id) {
  for (size_t i = 0; i < sizeof APP_SURFACES / sizeof APP_SURFACES[0]; ++i) if (APP_SURFACES[i].frame_id == frame_id) return &APP_SURFACES[i];
  return NULL;
}
const char *get_preset_name(uint8_t index) {
  size_t selected = index;
  if (selected >= PRESET_COUNT) selected = PRESET_COUNT - 1;
  return PRESET_NAMES[selected];
}
uint64_t pack_u32_pair(uint32_t low, uint32_t high) {
  return (uint64_t) low | (uint64_t) high << 32;
}
void resolve_scene_render_tokens(uint32_t *out_tokens) {
  const SceneAppearanceState *state = &SILK_SHELL_STATE.models.SCENE_APPEARANCE_STATE;
  size_t selected = (size_t) state->preset_idx % PRESET_COUNT;
  for (size_t i = 0; i < 8; ++i) {
    uint32_t color = TOKEN_PRESETS[selected][i];
    if (state->use_custom_colors != 0 && state->custom_colors[i] != 0) color = state->custom_colors[i];
    out_tokens[i] = color;
  }
}
uint64_t pack_scene_settings_blob(uint8_t preset, uint8_t chrome, uint8_t access) {
  uint8_t checksum = (SCENE_BLOB_MAGIC ^ SCENE_BLOB_VERSION ^ preset ^ chrome ^ access) & 0x7f;
  return (uint64_t) SCENE_BLOB_MAGIC | (uint64_t) SCENE_BLOB_VERSION << 8 | (uint64_t) preset << 16 | (uint64_t) chrome << 24 | (uint64_t) access << 32 | (uint64_t) checksum << 56;
}
bool unpack_scene_settings_blob(uint64_t blob, uint8_t *out_preset, uint8_t *out_chrome, uint8_t *out_access) {
  uint8_t bytes[8];
  for (size_t i = 0; i < 8; ++i) bytes[i] = (uint8_t)(blob >> i * 8);
  if (bytes[0] != SCENE_BLOB_MAGIC || bytes[1] != SCENE_BLOB_VERSION) return false;
  uint8_t checksum = (bytes[0] ^ bytes[1] ^ bytes[2] ^ bytes[3] ^ bytes[4] ^ bytes[5] ^ bytes[6]) & 0x7f;
  if (bytes[7] != checksum) return false;
  if (out_preset != NULL) *out_preset = bytes[2];
  if (out_chrome != NULL) *out_chrome = bytes[3];
  if (out_access != NULL) *out_access = bytes[4];
  return true;
}
bool store_reply_is_status(uint64_t reply) {
  return (reply & STORE_REPLY_STATUS_BIT) != 0;
}
uint64_t store_reply_status(uint64_t reply) {
  return reply & ~STORE_REPLY_STATUS_BIT;
}
bool store_reply_is_value(uint64_t reply) {
  return (reply & STORE_REPLY_STATUS_BIT) == 0;
}
void record_collar_audit(uint8_t operation, uint64_t object_id, uint64_t subject_id, uint8_t decision, uint64_t grant_ref, uint64_t reason) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  size_t index = (size_t) state->COLLAR_AUDIT_WRITE_INDEX % COLLAR_AUDIT_CAP;
  uint64_t event_id = state->COLLAR_AUDIT_WRITE_INDEX;
  state->COLLAR_AUDIT_EVENTS[index] = (CollarAuditEvent) {
    .present = true,.event_id = event_id,.operation = operation,.object_id = object_id,.subject_id = subject_id,.decision = decision,.grant_ref = grant_ref,.reason = reason,
  };
  ++state->COLLAR_AUDIT_WRITE_INDEX;
  if ((size_t) event_id >= COLLAR_AUDIT_CAP) sexos_line("[collar.audit.overwrite] idx=%zu", index);
  sexos_line("[collar.audit.write] event_id=%llu op=%u object=%llu subject=%llu decision=%u grant=%llu reason=%llu", (unsigned long long) event_id, (unsigned) operation, (unsigned long long) object_id, (unsigned long long) subject_id, (unsigned) decision, (unsigned long long) grant_ref, (unsigned long long) reason);
  if (COLLAR_ENFORCE_PROOF_ENABLED) sexos_line("[collar.audit] event_id=%llu op=%u object=%llu subject=%llu decision=%u grant=%llu reason=%llu", (unsigned long long) event_id, (unsigned) operation, (unsigned long long) object_id, (unsigned long long) subject_id, (unsigned) decision, (unsigned long long) grant_ref, (unsigned long long) reason);
}
uint8_t collar_grant_count(void) {
  uint8_t count = 0;
  for (size_t i = 0; i < COLLAR_GRANT_CAP; ++i) {
    const CollarGrant *grant = &SILK_SHELL_STATE.models.COLLAR_GRANTS[i];
    if (grant->present && grant->state == COLLAR_GRANT_ACTIVE && count != UINT8_MAX) ++count;
  }
  return count;
}
bool collar_grant_at_visible_index(uint8_t index, CollarGrant *out) {
  uint8_t cursor = 0;
  for (size_t i = 0; i < COLLAR_GRANT_CAP; ++i) {
    const CollarGrant *grant = &SILK_SHELL_STATE.models.COLLAR_GRANTS[i];
    if (!grant->present || grant->state != COLLAR_GRANT_ACTIVE) continue;
    if (cursor == index) {
      if (out != NULL) *out = *grant;
      return true;
    }
    if (cursor != UINT8_MAX) ++cursor;
  }
  return false;
}
void collar_auto_grant_from_manifest(const AppManifest *manifest) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  uint64_t count = 0;
  uint8_t bits[2] = {
    APP_CAP_BELL, APP_CAP_SEXFILES
  };
  uint8_t operations[2] = {
    COLLAR_OP_ACCESS_BELL, COLLAR_OP_ACCESS_SEXFILES
  };
  for (size_t i = 0; i < 2; ++i) {
    if ((manifest->capabilities.bits & bits[i]) == 0) continue;
    uint64_t generation = state->COLLAR_GRANT_GENERATION ++;
    size_t index = ((size_t) generation - 1) % COLLAR_GRANT_CAP;
    state->COLLAR_GRANTS[index] = (CollarGrant) {
      .present = true,.grant_id = generation,.subject_id = manifest->surface_id,.object_id = manifest->surface_id,.operation_mask = UINT64_C(1) << operations[i],.generation = generation,.state = COLLAR_GRANT_ACTIVE,
    };
    ++count;
    sexos_line("[collar.grant.manifest] grant_id=%llu sid=%llu cap=%s op=%s", (unsigned long long) generation, (unsigned long long) manifest->surface_id, i == 0 ? "BELL" : "SEXFILES", i == 0 ? "AccessBell" : "AccessSexFiles");
  }
  sexos_line("[collar.grant.manifest.done] sid=%llu grants_created=%llu", (unsigned long long) manifest->surface_id, (unsigned long long) count);
}
CollarReview collar_review_manifest(const AppManifest *manifest) {
  uint8_t requested = manifest->capabilities.bits;
  uint8_t granted = requested & APP_CAP_KNOWN;
  uint8_t denied = requested & (uint8_t) ~APP_CAP_KNOWN;
  bool allowed = granted == requested;
  sexos_line("[collar.review] sid=%llu app_id=%u requested=0x%x granted=0x%x denied=0x%x allowed=%s", (unsigned long long) manifest->surface_id, (unsigned) manifest->app_id, (unsigned) requested, (unsigned) granted, (unsigned) denied, allowed ? "true" : "false");
  return (CollarReview) {
    .allowed = allowed,.granted_caps = granted,.denied_caps = denied
  };
}
void collar_select_next_grant(void) {
  uint8_t count = collar_grant_count();
  uint8_t old = SILK_SHELL_STATE.models.COLLAR_SELECTED_GRANT_IDX;
  if (count == 0) {
    sexos_line("[collar.grant.nav] old=%u new=%u count=0", (unsigned) old, (unsigned) old);
    return;
  }
  uint8_t next = (uint8_t)(old + 1);
  if (next >= count) next = 0;
  SILK_SHELL_STATE.models.COLLAR_SELECTED_GRANT_IDX = next;
  sexos_line("[collar.grant.nav] old=%u new=%u count=%u", (unsigned) old, (unsigned) next, (unsigned) count);
  collar_render_grants_text();
}
void collar_select_prev_grant(void) {
  uint8_t count = collar_grant_count();
  uint8_t old = SILK_SHELL_STATE.models.COLLAR_SELECTED_GRANT_IDX;
  if (count == 0) {
    sexos_line("[collar.grant.nav] old=%u new=%u count=0", (unsigned) old, (unsigned) old);
    return;
  }
  uint8_t previous = old == 0 ? (uint8_t)(count - 1) : (uint8_t)(old - 1);
  SILK_SHELL_STATE.models.COLLAR_SELECTED_GRANT_IDX = previous;
  sexos_line("[collar.grant.nav] old=%u new=%u count=%u", (unsigned) old, (unsigned) previous, (unsigned) count);
  collar_render_grants_text();
}
bool collar_emit_selected_grant_detail(void) {
  uint8_t index = SILK_SHELL_STATE.models.COLLAR_SELECTED_GRANT_IDX;
  CollarGrant grant;
  if (collar_grant_at_visible_index(index, &grant)) {
    sexos_line("[collar.grant.detail] idx=%u grant_id=%llu ok=1 reason=ok", (unsigned) index, (unsigned long long) grant.grant_id);
    return true;
  }
  sexos_line("[collar.grant.detail] idx=%u grant_id=0 ok=0 reason=no_active_grant", (unsigned) index);
  return false;
}
size_t mesh_fact_count(void) {
  size_t count = (size_t) SILK_SHELL_STATE.models.MESH_FACT_WRITE_INDEX;
  return count < MESH_FACT_RING_CAP ? count : MESH_FACT_RING_CAP;
}
void mesh_for_each_fact(MeshFactCallback callback, void *context) {
  size_t count = mesh_fact_count();
  if (count == 0) return;
  size_t start = ((size_t) SILK_SHELL_STATE.models.MESH_FACT_WRITE_INDEX - 1) % MESH_FACT_RING_CAP;
  for (size_t i = 0; i < count; ++i) {
    size_t index = (start + MESH_FACT_RING_CAP - i) % MESH_FACT_RING_CAP;
    const MeshFact *fact = &SILK_SHELL_STATE.models.MESH_FACTS[index];
    if (fact->present) callback(fact, context);
  }
}
uint32_t mesh_fact_row_color(const MeshFact *fact) {
  LinenObject object;
  if (linen_object_by_id(fact->subject_id, &object)) return linen_kind_color(object.kind);
  return UINT32_C(0x00383010);
}
bool mesh_is_visible_in_active_scene(void) {
  for (size_t i = 0; i < MAX_FRAMES; ++i) {
    const ShellFrame *frame = &SILK_SHELL_STATE.chrome.FRAMES[i];
    if (frame->present && frame->frame_id == MESH_FRAME_ID && frame->scene_id == SILK_SHELL_STATE.chrome.ACTIVE_SCENE_IDX && (frame->flags & FRAME_FLAG_MINIMIZED) == 0) {
      uint64_t surface;
      if (active_surface_for_frame(MESH_FRAME_ID, &surface) && surface_is_alive(surface)) return true;
    }
  }
  return false;
}
uint8_t mesh_visible_fact_count(void) {
  uint8_t count = (uint8_t) mesh_fact_count();
  return count < MESH_LIST_ROW_RECTS ? count : MESH_LIST_ROW_RECTS;
}
uint32_t mesh_selected_row_highlight(uint32_t color) {
  uint32_t red = (color >> 16 & 0xff) + 0x40;
  uint32_t green = (color >> 8 & 0xff) + 0x40;
  uint32_t blue = (color & 0xff) + 0x40;
  if (red > 0xff) red = 0xff;
  if (green > 0xff) green = 0xff;
  if (blue > 0xff) blue = 0xff;
  return red << 16 | green << 8 | blue;
}
void mesh_select_next_row(void) {
  uint8_t count = mesh_visible_fact_count();
  if (count <= 1) {
    sexos_line("[mesh.selection.reject] reason=single_or_empty count=%u", (unsigned) count);
    return;
  }
  uint8_t current = SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW;
  uint8_t next = (uint8_t)(current + 1);
  if (next >= count) next = 0;
  SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW = next;
  sexos_line("[mesh.selection.next] prev=%u next=%u", (unsigned) current, (unsigned) next);
  mesh_render_fact_list();
}
void mesh_select_prev_row(void) {
  uint8_t count = mesh_visible_fact_count();
  if (count <= 1) {
    sexos_line("[mesh.selection.reject] reason=single_or_empty count=%u", (unsigned) count);
    return;
  }
  uint8_t current = SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW;
  uint8_t previous = current == 0 ? (uint8_t)(count - 1) : (uint8_t)(current - 1);
  SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW = previous;
  sexos_line("[mesh.selection.prev] prev=%u next=%u", (unsigned) current, (unsigned) previous);
  mesh_render_fact_list();
}
bool mesh_selected_fact_snapshot(MeshFact *out) {
  size_t count = mesh_fact_count();
  if (count == 0) return false;
  size_t start = ((size_t) SILK_SHELL_STATE.models.MESH_FACT_WRITE_INDEX - 1) % MESH_FACT_RING_CAP;
  for (size_t i = 0; i < count; ++i) {
    size_t index = (start + MESH_FACT_RING_CAP - i) % MESH_FACT_RING_CAP;
    const MeshFact *fact = &SILK_SHELL_STATE.models.MESH_FACTS[index];
    if (fact->present && (uint8_t) i == SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW) {
      if (out != NULL) *out = *fact;
      return true;
    }
  }
  return false;
}
bool mesh_emit_selected_fact_detail_proof(void) {
  if (SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID != SURFACE_ID_MESH) {
    sexos_line("[mesh.detail.reject] reason=not_focused");
    return false;
  }
  MeshFact fact;
  if (!mesh_selected_fact_snapshot(&fact)) {
    sexos_line("[mesh.detail.reject] reason=no_fact");
    return false;
  }
  sexos_line("[mesh.detail.open] fact_id=%llu kind=ObjectLinkedToBuffer", (unsigned long long) fact.fact_id);
  sexos_line("[mesh.detail.fact] fact_id=%llu kind=ObjectLinkedToBuffer subject_id=%llu object_id=%llu ref_id=%llu", (unsigned long long) fact.fact_id, (unsigned long long) fact.subject_id, (unsigned long long) fact.object_id, (unsigned long long) fact.ref_id);
  sexos_line("[mesh.detail.object_link] subject_id=%llu object_id=%llu ref_id=%llu", (unsigned long long) fact.subject_id, (unsigned long long) fact.object_id, (unsigned long long) fact.ref_id);
  sexos_line("[mesh.detail.done] fact_id=%llu", (unsigned long long) fact.fact_id);
  return true;
}
void mesh_focus_linen_at_selected_fact(const MeshFact *fact) {
  sexos_line("[mesh.action.focus_linen] subject_id=%llu", (unsigned long long) fact->subject_id);
  open_linen_in_active_scene();
  SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = fact->subject_id;
  sexos_line("[linen.open.nonblocking] path=mesh_detail ok=1 reason=no_redundant_paint");
}
void bell_record_event(uint64_t object_id, uint64_t buffer_id) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  size_t index = (size_t) state->BELL_RING_WRITE_INDEX % BELL_RING_CAP;
  uint64_t sequence = state->BELL_EVENT_SEQUENCE ++;
  BellEvent previous = state->BELL_EVENTS[index];
  state->BELL_EVENTS[index] = (BellEvent) {
    .present = true,.event_id = sequence,.kind = BELL_EVENT_OBJECT_LINKED_TO_BUFFER,.object_id = object_id,.buffer_id = buffer_id,.sequence = state->BELL_RING_WRITE_INDEX,
  };
  ++state->BELL_RING_WRITE_INDEX;
  if (previous.present) sexos_line("[bell.ring.overwrite] idx=%zu prev_event_id=%llu", index, (unsigned long long) previous.event_id);
  sexos_line("[bell.ring.write] idx=%zu event_id=%llu object_id=%llu buffer_id=%llu", index, (unsigned long long) sequence, (unsigned long long) object_id, (unsigned long long) buffer_id);
}
size_t bell_ring_count(void) {
  size_t count = (size_t) SILK_SHELL_STATE.models.BELL_RING_WRITE_INDEX;
  return count < BELL_RING_CAP ? count : BELL_RING_CAP;
}
void bell_for_each_event(BellEventCallback callback, void *context) {
  size_t count = bell_ring_count();
  if (count == 0) return;
  size_t start = ((size_t) SILK_SHELL_STATE.models.BELL_RING_WRITE_INDEX - 1) % BELL_RING_CAP;
  for (size_t i = 0; i < count; ++i) {
    size_t index = (start + BELL_RING_CAP - i) % BELL_RING_CAP;
    const BellEvent *event = &SILK_SHELL_STATE.models.BELL_EVENTS[index];
    if (event->present) callback(event, context);
  }
}
void app_surface_registry_validate(void) {
  size_t count = sizeof APP_SURFACES / sizeof APP_SURFACES[0];
  bool valid = true;
  for (size_t i = 0; i < count; ++i) {
    for (size_t j = i + 1; j < count; ++j) {
      if (APP_SURFACES[i].surface_id == APP_SURFACES[j].surface_id) {
        sexos_line("[shell.app_registry.duplicate] surface_id=%llu entries=%zu,%zu", (unsigned long long) APP_SURFACES[i].surface_id, i, j);
        valid = false;
      }
      if (APP_SURFACES[i].frame_id == APP_SURFACES[j].frame_id) {
        sexos_line("[shell.app_registry.duplicate] frame_id=%u entries=%zu,%zu", (unsigned) APP_SURFACES[i].frame_id, i, j);
        valid = false;
      }
    }
  }
  if (valid) sexos_line("[shell.app_registry.valid] count=%zu", count);
  else sexos_line("[shell.app_registry.error] duplicate detected — check surface_id/frame_id allocation");
}
void push_token_preset(const uint32_t *tokens) {
  (void) sexos_pdx_call(SLOT_DISPLAY, OP_APPEARANCE_TOKENS, pack_u32_pair(tokens[APPEARANCE_TOKEN_FOCUS_SURFACE], tokens[APPEARANCE_TOKEN_FRAME_RIM]), pack_u32_pair(tokens[APPEARANCE_TOKEN_FRAME_TOP_BAR], tokens[APPEARANCE_TOKEN_ACTIVE_TAB]), pack_u32_pair(tokens[APPEARANCE_TOKEN_INACTIVE_TAB], tokens[APPEARANCE_TOKEN_CLOSE_LIGHT]));
  (void) sexos_pdx_call(SLOT_DISPLAY, OP_APPEARANCE_TOKENS, pack_u32_pair(tokens[APPEARANCE_TOKEN_MINIMIZE_LIGHT], tokens[APPEARANCE_TOKEN_ZOOM_LIGHT]), 0, 0);
}
void mesh_record_fact(uint8_t kind, uint64_t subject_id, uint64_t object_id, uint64_t reference_id) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  size_t index = (size_t) state->MESH_FACT_WRITE_INDEX % MESH_FACT_RING_CAP;
  uint64_t sequence = state->MESH_FACT_SEQUENCE ++;
  MeshFact previous = state->MESH_FACTS[index];
  state->MESH_FACTS[index] = (MeshFact) {
    .present = true,.fact_id = sequence,.kind = kind,.subject_id = subject_id,.object_id = object_id,.ref_id = reference_id,.sequence = state->MESH_FACT_WRITE_INDEX,
  };
  ++state->MESH_FACT_WRITE_INDEX;
  if (previous.present) sexos_line("[mesh.fact.overwrite] idx=%zu prev_fact_id=%llu", index, (unsigned long long) previous.fact_id);
  sexos_line("[mesh.fact.write] idx=%zu fact_id=%llu kind=ObjectLinkedToBuffer subject_id=%llu object_id=%llu ref_id=%llu", index, (unsigned long long) sequence, (unsigned long long) subject_id, (unsigned long long) object_id, (unsigned long long) reference_id);
  sexos_line("[mesh.fact.done] count=%zu fact_id=%llu", mesh_fact_count(), (unsigned long long) sequence);
  if (mesh_is_visible_in_active_scene()) {
    sexos_line("[mesh.render.refresh] reason=visible_after_fact kind=ObjectLinkedToBuffer");
    mesh_render_fact_list();
  }
}
void mesh_emit_linen_quil_links(void) {
  sexos_line("[mesh.object_link.start]");
  size_t links = 0;
  size_t stale = 0;
  for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) {
    const QuilBuffer *buffer = &SILK_SHELL_STATE.models.QUIL_BUFFERS[i];
    if (!buffer->present || buffer->linen_object_ref == 0) continue;
    LinenObject object;
    if (linen_object_by_id(buffer->linen_object_ref, &object)) {
      sexos_line("[mesh.object_link.row] object_id=%llu object_kind=%s buffer_id=%llu buffer_kind=%s surface_id=%llu", (unsigned long long) object.object_id, linen_object_kind_name(object.kind), (unsigned long long) buffer->buffer_id, quil_buffer_kind_name(buffer->kind), (unsigned long long) buffer->linked_surface_id);
      mesh_record_fact(MESH_FACT_OBJECT_LINKED_TO_BUFFER, object.object_id, buffer->buffer_id, buffer->linked_surface_id);
      ++links;
    } else {
      sexos_line("[mesh.object_link.reject.missing_object] buffer_id=%llu linen_object_ref=%llu", (unsigned long long) buffer->buffer_id, (unsigned long long) buffer->linen_object_ref);
      ++stale;
    }
  }
  sexos_line("[mesh.object_link.done] links=%zu stale=%zu", links, stale);
}
typedef struct ModelMeshRows {
  uint32_t width;
  uint8_t rows;
  uint8_t rectangles;
}
ModelMeshRows;
static void model_mesh_render_row(const MeshFact *fact, void *context) {
  ModelMeshRows *state = context;
  if (state->rows >= MESH_LIST_ROW_RECTS) {
    sexos_line("[mesh.fact_list.skip] fact_id=%llu reason=max_rows", (unsigned long long) fact->fact_id);
    return;
  }
  sexos_line("[mesh.fact_list.row] fact_id=%llu kind=ObjectLinkedToBuffer subject_id=%llu object_id=%llu ref_id=%llu", (unsigned long long) fact->fact_id, (unsigned long long) fact->subject_id, (unsigned long long) fact->object_id, (unsigned long long) fact->ref_id);
  uint64_t rectangle = ((uint64_t) state->rows + 1) & 0xf;
  uint32_t y = MESH_LIST_HEADER_H + (uint32_t) state->rows * (MESH_LIST_ROW_H + MESH_LIST_ROW_GAP);
  uint32_t base_color = mesh_fact_row_color(fact);
  uint32_t color = base_color;
  if (state->rows == SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW) {
    color = mesh_selected_row_highlight(base_color);
    sexos_line("[mesh.selection_visual.row] fact_id=%llu index=%u base=0x%08x highlight=0x%08x", (unsigned long long) fact->fact_id, (unsigned) state->rows, (unsigned) base_color, (unsigned) color);
  }(void) sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_MESH, (uint64_t) y << 32, rectangle << 56 | (uint64_t) color << 32 | (uint64_t) MESH_LIST_ROW_H << 16 | state->width);
  sexos_line("[mesh.row_visual.rect] index=%llu fact_id=%llu kind=ObjectLinkedToBuffer color=0x%08x", (unsigned long long) rectangle, (unsigned long long) fact->fact_id, (unsigned) color);
  ++state->rectangles;
  ++state->rows;
}
void mesh_render_fact_list(void) {
  uint32_t width = SILK_SHELL_STATE.input.SURFACE_202_W;
  uint32_t height = SILK_SHELL_STATE.input.SURFACE_202_H;
  if (width == 0 || height == 0) return;
  uint8_t visible = mesh_visible_fact_count();
  uint8_t *selected = &SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW;
  if (visible == 0) {
    *selected = 0;
  } else if (*selected >= visible) {
    uint8_t old = *selected;
    *selected = (uint8_t)(visible - 1);
    sexos_line("[mesh.selection.repair] old=%u new=%u count=%u", (unsigned) old, (unsigned) *selected, (unsigned) visible);
  }
  sexos_line("[mesh.selection.current] row=%u visible=%u", (unsigned) *selected, (unsigned) visible);
  sexos_line("[mesh.fact_list.render] w=%u h=%u count=%zu", (unsigned) width, (unsigned) height, mesh_fact_count());
  (void) sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_MESH, 0, (uint64_t) MESH_PLACEHOLDER_COLOR << 32 | (uint64_t) MESH_LIST_HEADER_H << 16 | width);
  ModelMeshRows rows = {
    .width = width
  };
  mesh_for_each_fact(model_mesh_render_row, &rows);
  sexos_line("[mesh.fact_list.done] count=%zu rows=%u rects=%u", mesh_fact_count(), (unsigned) rows.rows, (unsigned) rows.rectangles);
}
void mesh_render_pd_graph(void) {
  uint8_t buffer[120];
  memset(buffer, ' ', sizeof buffer);
  memcpy(buffer, "PD GRAPH RO", 11);
  const char *names[5] = {
    "SPINDLE", "QUIL", "LINEN", "MESH", "BELL"
  };
  uint64_t surfaces[5] = {
    SURFACE_ID_SPINDLE, SURFACE_ID_QUIL, SURFACE_ID_LINEN, SURFACE_ID_MESH, SURFACE_ID_BELL_PLACEHOLDER
  };
  for (size_t i = 0; i < 5; ++i) {
    size_t base = (i + 1) * 20;
    size_t length = strlen(names[i]);
    if (length > 8) length = 8;
    memcpy(buffer + base, names[i], length);
    uint64_t surface = surfaces[i];
    buffer[base + 8] = (uint8_t)('0' + surface / 100 % 10);
    buffer[base + 9] = (uint8_t)('0' + surface / 10 % 10);
    buffer[base + 10] = (uint8_t)('0' + surface % 10);
    uint8_t state = '-';
    uint32_t frame_id;
    if (frame_for_surface(surface, &frame_id)) {
      for (size_t j = 0; j < MAX_FRAMES; ++j) {
        const ShellFrame *frame = &SILK_SHELL_STATE.chrome.FRAMES[j];
        if (frame->present && frame->frame_id == frame_id) {
          state = (frame->flags & FRAME_FLAG_MINIMIZED) != 0 ? 'M' : 'V';
          break;
        }
      }
    }
    buffer[base + 12] = state;
    buffer[base + 14] = SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID == surface ? 'F' : '-';
  }(void) sexos_pdx_call(SLOT_DISPLAY, OP_TEXT_CLEAR, SURFACE_ID_MESH, 0, 0);
  size_t drawn;
  bool accepted = shell_draw_text(SURFACE_ID_MESH, buffer, sizeof buffer, UINT64_C(0x00E8FFFF), &drawn);
  sexos_line("[mesh.pd_graph.render.ok] rows=6 bytes=%zu ok=%u", drawn, (unsigned) accepted);
}
void bell_emit_object_link_event(uint64_t object_id, uint64_t buffer_id) {
  sexos_line("[bell.event.stub] kind=ObjectLinkedToBuffer object_id=%llu buffer_id=%llu", (unsigned long long) object_id, (unsigned long long) buffer_id);
  LinenObject object;
  QuilBuffer buffer;
  bool object_valid = linen_object_by_id(object_id, &object);
  bool buffer_valid = quil_buffer_by_id(buffer_id, &buffer);
  if (!object_valid || !buffer_valid) {
    sexos_line("[bell.event.reject.missing] object_valid=%s buffer_valid=%s", object_valid ? "true" : "false", buffer_valid ? "true" : "false");
    sexos_line("[bell.event.done] reason=rejected");
    return;
  }
  if (buffer.linen_object_ref != object_id) {
    sexos_line("[bell.event.reject.missing] reason=buffer_ref_mismatch expected=%llu actual=%llu", (unsigned long long) object_id, (unsigned long long) buffer.linen_object_ref);
    sexos_line("[bell.event.done] reason=rejected");
    return;
  }
  sexos_line("[bell.event.object_link] object_id=%llu object_kind=%s buffer_id=%llu buffer_kind=%s", (unsigned long long) object_id, linen_object_kind_name(object.kind), (unsigned long long) buffer_id, quil_buffer_kind_name(buffer.kind));
  bell_record_event(object_id, buffer_id);
  sexos_line("[bell.ring.done] count=%zu event_id=%llu", bell_ring_count(), (unsigned long long)(SILK_SHELL_STATE.models.BELL_EVENT_SEQUENCE - 1));
  if (bell_is_visible_in_active_scene()) {
    sexos_line("[bell.render.refresh] reason=visible_after_link object_id=%llu buffer_id=%llu", (unsigned long long) object_id, (unsigned long long) buffer_id);
    bell_render_event_list();
  }
  sexos_line("[bell.event.done] reason=emitted");
}
static bool model_take_budget(uint32_t *budget) {
  if (*budget == 0) return false;
  --*budget;
  return true;
}
static void model_send_current_tokens(void) {
  uint32_t tokens[8];
  resolve_scene_render_tokens(tokens);
  push_token_preset(tokens);
}
static void model_store_current_settings(void) {
  const SceneAppearanceState *state = &SILK_SHELL_STATE.models.SCENE_APPEARANCE_STATE;
  uint64_t blob = pack_scene_settings_blob(state->preset_idx, state->chrome_flags, state->accessibility_flags);
  (void) sexos_pdx_call(SLOT_SEXSTORE, OP_KV_PUT, SCENE_SETTINGS_KEY_APPEARANCE, blob, 0);
}
void send_scene_render_tokens(void) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  model_send_current_tokens();
  if (model_take_budget(&state->send_scene_render_tokens__SHELL_TOKEN_SEND_BUDGET)) sexos_line("[shell.appearance.tokens.send] seq=2 sent");
  if (model_take_budget(&state->send_scene_render_tokens__STATE_BUDGET)) {
    const SceneAppearanceState *appearance = &state->SCENE_APPEARANCE_STATE;
    sexos_line("[shell.appearance.state] preset=%u custom=%u chrome=%u access=%u", (unsigned) appearance->preset_idx, (unsigned) appearance->use_custom_colors, (unsigned) appearance->chrome_flags, (unsigned) appearance->accessibility_flags);
  }
}
void handle_sexstore_get_reply(uint64_t value) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  if (store_reply_is_status(value)) {
    uint64_t code = store_reply_status(value);
    if (model_take_budget(&state->handle_sexstore_get_reply__STATUS_BUDGET)) {
      const char *name = NULL;
      switch (code) {
        case STORE_KV_NOT_FOUND: name = "not_found";
        break;
        case STORE_KV_DENIED: name = "denied";
        break;
        case STORE_KV_FULL: name = "full";
        break;
        case STORE_KV_INVALID_KEY: name = "invalid_key";
        break;
        case STORE_KV_INVALID_VALUE: name = "invalid_value";
        break;
        case STORE_KV_OK: name = "ok";
        break;
        default: break;
      }
      if (name != NULL) sexos_line("[shell.store.reply.status] code=%s key=0x01", name);
      else sexos_line("[shell.store.reply.reject] code=0x%llx key=0x01", (unsigned long long) code);
    }
    if (model_take_budget(&state->handle_sexstore_get_reply__DEFAULT_BUDGET)) sexos_line("[shell.store.default] reason=status_reply");
    return;
  }
  sexos_line("[shell.store.reply.value] key=0x01");
  uint8_t preset, chrome, access;
  if (unpack_scene_settings_blob(value, &preset, &chrome, &access)) {
    uint8_t clamped = preset < PRESET_COUNT ? preset : 0;
    state->SCENE_APPEARANCE_STATE.preset_idx = clamped;
    state->SCENE_APPEARANCE_STATE.chrome_flags = chrome;
    state->SCENE_APPEARANCE_STATE.accessibility_flags = access;
    state->SCENE_APPEARANCE_STATE.use_custom_colors = 0;
    memset(state->SCENE_APPEARANCE_STATE.custom_colors, 0, sizeof state->SCENE_APPEARANCE_STATE.custom_colors);
    state->ACTIVE_TINT_IDX = 0;
    model_send_current_tokens();
    if (model_take_budget(&state->handle_sexstore_get_reply__LOAD_OK_BUDGET)) sexos_line("[shell.scene.settings.load] ok=1 preset=%u chrome=%u access=%u", (unsigned) clamped, (unsigned) chrome, (unsigned) access);
  } else if (model_take_budget(&state->handle_sexstore_get_reply__LOAD_FAIL_BUDGET)) {
    sexos_line("[shell.scene.settings.load] ok=0 %s", value == 0 ? "not-found" : "corrupt");
  }
}
void boot_load_scene_settings(void) {
  SexosReply reply = sexos_pdx_call(SLOT_SEXSTORE, OP_KV_GET, SCENE_SETTINGS_KEY_APPEARANCE, 0, 0);
  if (model_take_budget(&SILK_SHELL_STATE.models.boot_load_scene_settings__BOOT_LOAD_BUDGET)) {
    if (reply.status == 0) {
      SILK_SHELL_STATE.models.SEXSTORE_LOAD_PENDING = true;
      sexos_line("[shell.scene.settings.load.request] ok=1 pending");
    } else {
      sexos_line("[shell.scene.settings.load.request] ok=0 status=%llu", (unsigned long long) reply.status);
    }
  }
}
void cycle_scene_render_token_preset(void) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  state->SCENE_APPEARANCE_STATE.preset_idx = (uint8_t)(state->SCENE_APPEARANCE_STATE.preset_idx + 1) % PRESET_COUNT;
  state->SCENE_APPEARANCE_STATE.use_custom_colors = 0;
  state->ACTIVE_TINT_IDX = 0;
  model_send_current_tokens();
  if (model_take_budget(&state->cycle_scene_render_token_preset__CYCLE_BUDGET)) sexos_line("[shell.appearance.preset] idx=%u", (unsigned) state->SCENE_APPEARANCE_STATE.preset_idx);
  model_store_current_settings();
  if (model_take_budget(&state->cycle_scene_render_token_preset__SAVE_BUDGET)) sexos_line("[shell.scene.settings.save] preset=%u", (unsigned) state->SCENE_APPEARANCE_STATE.preset_idx);
}
void cycle_prev_scene_render_token_preset(void) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  uint8_t old = state->SCENE_APPEARANCE_STATE.preset_idx;
  state->SCENE_APPEARANCE_STATE.preset_idx = old == 0 ? PRESET_COUNT - 1 : (uint8_t)(old - 1);
  state->SCENE_APPEARANCE_STATE.use_custom_colors = 0;
  state->ACTIVE_TINT_IDX = 0;
  model_send_current_tokens();
  if (model_take_budget(&state->cycle_prev_scene_render_token_preset__CYCLE_PREV_BUDGET)) sexos_line("[shell.appearance.preset] idx=%u", (unsigned) state->SCENE_APPEARANCE_STATE.preset_idx);
  model_store_current_settings();
  if (model_take_budget(&state->cycle_prev_scene_render_token_preset__SAVE_PREV_BUDGET)) sexos_line("[shell.scene.settings.save] preset=%u", (unsigned) state->SCENE_APPEARANCE_STATE.preset_idx);
}
void apply_custom_tint_bundle(size_t index) {
  SceneAppearanceState *state = &SILK_SHELL_STATE.models.SCENE_APPEARANCE_STATE;
  if (index == 0) {
    state->use_custom_colors = 0;
  } else {
    memcpy(state->custom_colors, CUSTOM_TINT_BUNDLES[index], sizeof state->custom_colors);
    state->use_custom_colors = 1;
  }
}
void cycle_custom_tint(void) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  state->ACTIVE_TINT_IDX = (uint8_t)(state->ACTIVE_TINT_IDX + 1) % TINT_COUNT;
  apply_custom_tint_bundle(state->ACTIVE_TINT_IDX);
  model_send_current_tokens();
  if (model_take_budget(&state->cycle_custom_tint__TINT_BUDGET)) sexos_line("[shell.appearance.custom] mode=tint tint=%u", (unsigned) state->ACTIVE_TINT_IDX);
}
void handle_scene_settings_cmd(uint64_t command, uint64_t value, uint64_t flags) {
  (void) flags;
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  uint32_t *budget = &state->handle_scene_settings_cmd__CMD_BUDGET;
  switch (command) {
    case CMD_SET_PRESET: {
      uint8_t index = (size_t) value < PRESET_COUNT ? (uint8_t) value : 0;
      state->SCENE_APPEARANCE_STATE.preset_idx = index;
      state->SCENE_APPEARANCE_STATE.use_custom_colors = 0;
      memset(state->SCENE_APPEARANCE_STATE.custom_colors, 0, sizeof state->SCENE_APPEARANCE_STATE.custom_colors);
      state->ACTIVE_TINT_IDX = 0;
      model_send_current_tokens();
      model_store_current_settings();
      if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=1 preset=%u ok=1", (unsigned) index);
      break;
    }
    case CMD_CYCLE_PRESET: cycle_scene_render_token_preset();
    if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=2 ok=1");
    break;
    case CMD_SET_TINT: {
      uint8_t index = (uint8_t) value % TINT_COUNT;
      state->ACTIVE_TINT_IDX = index;
      apply_custom_tint_bundle(index);
      model_send_current_tokens();
      if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=3 tint=%u ok=1", (unsigned) index);
      break;
    }
    case CMD_CYCLE_TINT: cycle_custom_tint();
    if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=4 ok=1");
    break;
    case CMD_SET_CHROME_FLAGS: state->SCENE_APPEARANCE_STATE.chrome_flags = (uint8_t) value;
    model_send_current_tokens();
    model_store_current_settings();
    if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=5 flags=%u ok=1", (unsigned)(uint8_t) value);
    break;
    case CMD_TOGGLE_TOP_BAR: toggle_top_bar_for_active_frame();
    if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=6 ok=1");
    break;
    case CMD_SET_ACCESSIBILITY: state->SCENE_APPEARANCE_STATE.accessibility_flags = (uint8_t) value;
    model_send_current_tokens();
    model_store_current_settings();
    if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=7 flags=%u ok=1", (unsigned)(uint8_t) value);
    break;
    case CMD_RESET_DEFAULTS: state->SCENE_APPEARANCE_STATE = DEFAULT_SCENE_APPEARANCE;
    state->ACTIVE_TINT_IDX = 0;
    model_send_current_tokens();
    model_store_current_settings();
    if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=8 ok=1");
    break;
    default: if (model_take_budget(budget)) sexos_line("[shell.scene.settings.cmd] cmd=%llu ok=0 unknown", (unsigned long long) command);
    break;
  }
}
uint8_t shell_view_mode(void) {
  return SILK_SHELL_STATE.chrome.ATLAS_MODE_ENABLED ? SHELL_VIEW_ATLAS : SHELL_VIEW_DESKTOP;
}
uint8_t collar_check_operation(uint8_t operation, uint64_t object_id, uint64_t buffer_id) {
  uint64_t caller = SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID;
  sexos_line("[collar.policy.check] op=%u object_id=%llu buffer_id=%llu caller_sid=%llu", (unsigned) operation, (unsigned long long) object_id, (unsigned long long) buffer_id, (unsigned long long) caller);
  bool object_required = operation == COLLAR_OP_OPEN_OBJECT || operation == COLLAR_OP_LINK_OBJECT_TO_BUFFER || operation == COLLAR_OP_RENAME_OBJECT || operation == COLLAR_OP_ARCHIVE_OBJECT;
  if (object_required && object_id != 0 && !linen_object_by_id(object_id, NULL)) {
    sexos_line("[collar.gate.reject] reason=missing_object op=%u object_id=%llu", (unsigned) operation, (unsigned long long) object_id);
    record_collar_audit(operation, object_id, caller, COLLAR_DECISION_DENY_MISSING_OBJECT, 0, 1);
    return COLLAR_DECISION_DENY_MISSING_OBJECT;
  }
  if (operation == COLLAR_OP_LINK_OBJECT_TO_BUFFER && buffer_id != 0 && !quil_buffer_by_id(buffer_id, NULL)) {
    sexos_line("[collar.gate.reject] reason=missing_buffer op=%u buffer_id=%llu", (unsigned) operation, (unsigned long long) buffer_id);
    record_collar_audit(operation, buffer_id, caller, COLLAR_DECISION_DENY_MISSING_BUFFER, 0, 2);
    return COLLAR_DECISION_DENY_MISSING_BUFFER;
  }
  bool system_capability = operation == COLLAR_OP_ACCESS_BELL || operation == COLLAR_OP_ACCESS_SEXFILES;
  if (operation == COLLAR_OP_OPEN_OBJECT || operation == COLLAR_OP_LINK_OBJECT_TO_BUFFER || system_capability) {
    if (system_capability) {
      if (caller == SURFACE_ID_SPINDLE || caller == SURFACE_ID_LINEN || caller == SURFACE_ID_QUIL || caller == SURFACE_ID_MESH || caller == SURFACE_ID_COLLAR || caller == SURFACE_ID_BELL_PLACEHOLDER) {
        sexos_line("[collar.policy.allow] op=%u object=%llu caller=%llu grant=0 reason=core_app", (unsigned) operation, (unsigned long long) object_id, (unsigned long long) caller);
        record_collar_audit(operation, object_id, caller, COLLAR_DECISION_ALLOW, 0, 8);
        return COLLAR_DECISION_ALLOW;
      }
      if (caller < 300) {
        sexos_line("[collar.gate.reject] reason=unknown_app op=%u caller=%llu", (unsigned) operation, (unsigned long long) caller);
        record_collar_audit(operation, object_id, caller, COLLAR_DECISION_DENY, 0, 7);
        if (COLLAR_ENFORCE_PROOF_ENABLED) sexos_line("[collar.enforce.deny] op=%u caller=%llu target=%llu reason=unknown_app", (unsigned) operation, (unsigned long long) caller, (unsigned long long) object_id);
        return COLLAR_DECISION_DENY;
      }
    }
    bool explicit_denial=false;
    if(!system_capability){
      for(size_t i=0;i<COLLAR_GRANT_CAP;++i){const CollarGrant *g=&SILK_SHELL_STATE.models.COLLAR_GRANTS[i];
        if(g->present && g->subject_id==caller && g->object_id==object_id && (g->operation_mask&(UINT64_C(1)<<operation)) && g->state!=COLLAR_GRANT_ACTIVE){explicit_denial=true;break;}}
      if(explicit_denial){record_collar_audit(operation,object_id,caller,COLLAR_DECISION_DENY,0,9);return COLLAR_DECISION_DENY;}
    }
    for (size_t i = 0; i < COLLAR_GRANT_CAP; ++i) {
      const CollarGrant *grant = &SILK_SHELL_STATE.models.COLLAR_GRANTS[i];
      if(!grant->present || grant->state!=COLLAR_GRANT_ACTIVE || grant->subject_id!=caller)continue;
      if(grant->object_id!=object_id){
        bool workspace=false;
        if(!system_capability && grant->object_id==UINT64_MAX){
          for(size_t j=0;j<LINEN_MAX_OBJECTS;++j){const LinenObject *o=&SILK_SHELL_STATE.models.LINEN_OBJECTS[j];
            if(o->present && shell_workspace_allows(caller,operation,object_id,o->object_id,o->disk_path,SILK_SHELL_STATE.models.LINEN_REMOTE_REAL,true,explicit_denial)){workspace=true;break;}}
        }
        if(!workspace)continue;
        sexos_line("[collar.workspace.allow] subject=%llu object=%llu grant=%llu published=1 kernel_caps_minted=0",(unsigned long long)caller,(unsigned long long)object_id,(unsigned long long)grant->grant_id);
      }
      if((grant->operation_mask&(UINT64_C(1)<<operation))==0)continue;
      sexos_line("[collar.grant.match] grant_id=%llu subject=%llu object=%llu op=%u", (unsigned long long) grant->grant_id, (unsigned long long) grant->subject_id, (unsigned long long) grant->object_id, (unsigned) operation);
      sexos_line("[collar.policy.allow] op=%u object=%llu caller=%llu grant=%llu", (unsigned) operation, (unsigned long long) object_id, (unsigned long long) caller, (unsigned long long) grant->grant_id);
      record_collar_audit(operation, object_id, caller, COLLAR_DECISION_ALLOW, grant->grant_id, 0);
      if (system_capability && COLLAR_ENFORCE_PROOF_ENABLED) sexos_line("[collar.enforce.allow] op=%u caller=%llu target=%llu grant=%llu", (unsigned) operation, (unsigned long long) caller, (unsigned long long) object_id, (unsigned long long) grant->grant_id);
      return COLLAR_DECISION_ALLOW;
    }
    sexos_line("[collar.grant.reject] reason=no_grant op=%u object=%llu caller=%llu", (unsigned) operation, (unsigned long long) object_id, (unsigned long long) caller);
    sexos_line("[collar.policy.deny] op=%u object=%llu caller=%llu reason=no_grant", (unsigned) operation, (unsigned long long) object_id, (unsigned long long) caller);
    record_collar_audit(operation, object_id, caller, COLLAR_DECISION_DENY, 0, 3);
    if (system_capability && COLLAR_ENFORCE_PROOF_ENABLED) sexos_line("[collar.enforce.deny] op=%u caller=%llu target=%llu reason=missing_cap", (unsigned) operation, (unsigned long long) caller, (unsigned long long) object_id);
    return COLLAR_DECISION_DENY;
  }
  if (operation == COLLAR_OP_SAVE_BUFFER || operation == COLLAR_OP_BUILD_TARGET || operation == COLLAR_OP_RUN_TARGET) {
    sexos_line("[collar.gate.reject] reason=stop_first op=%u", (unsigned) operation);
    record_collar_audit(operation, object_id, caller, COLLAR_DECISION_BLOCKED_STOP_FIRST, 0, 4);
    return COLLAR_DECISION_BLOCKED_STOP_FIRST;
  }
  if (operation == COLLAR_OP_RENAME_OBJECT || operation == COLLAR_OP_ARCHIVE_OBJECT) {
    sexos_line("[collar.gate.needs_grant] op=%u", (unsigned) operation);
    record_collar_audit(operation, object_id, caller, COLLAR_DECISION_NEEDS_GRANT_LATER, 0, 5);
    return COLLAR_DECISION_NEEDS_GRANT_LATER;
  }
  sexos_line("[collar.gate.reject] reason=always_deny op=%u", (unsigned) operation);
  record_collar_audit(operation, object_id, caller, COLLAR_DECISION_DENY, 0, 6);
  if (COLLAR_ENFORCE_PROOF_ENABLED) sexos_line("[collar.enforce.deny] op=%u caller=%llu target=%llu reason=dangerous_cap", (unsigned) operation, (unsigned long long) caller, (unsigned long long) object_id);
  return COLLAR_DECISION_DENY;
}
void collar_init_grants(void) {
  SilkShellModelsState *state = &SILK_SHELL_STATE.models;
  uint64_t count = 0;
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
    const LinenObject *object = &state->LINEN_OBJECTS[i];
    if (!object->present) continue;
    uint64_t subjects[2] = {
      SURFACE_ID_LINEN, SURFACE_ID_MESH
    };
    for (size_t j = 0; j < 2; ++j) {
      uint64_t generation = state->COLLAR_GRANT_GENERATION ++;
      size_t index = ((size_t) generation - 1) % COLLAR_GRANT_CAP;
      state->COLLAR_GRANTS[index] = (CollarGrant) {
        .present = true,.grant_id = generation,.subject_id = subjects[j],.object_id = object->object_id,.operation_mask = UINT64_C(1) << COLLAR_OP_LINK_OBJECT_TO_BUFFER,.generation = generation,.state = COLLAR_GRANT_ACTIVE,
      };
      sexos_line("[collar.grant.auto] grant_id=%llu subject=%llu object=%llu op=LinkObjectToBuffer", (unsigned long long) generation, (unsigned long long) subjects[j], (unsigned long long) object->object_id);
      ++count;
    }
  }
  if(state->COLLAR_GRANT_GENERATION!=UINT64_MAX){
    uint64_t generation=state->COLLAR_GRANT_GENERATION;size_t index=((size_t)generation-1)%COLLAR_GRANT_CAP;
    if(state->COLLAR_GRANTS[index].present){sexos_fail();for(;;){}}
    state->COLLAR_GRANT_GENERATION=generation+1;
    state->COLLAR_GRANTS[index]=(CollarGrant){.present=true,.grant_id=generation,.subject_id=SURFACE_ID_LINEN,.object_id=UINT64_MAX,.operation_mask=(UINT64_C(1)<<COLLAR_OP_OPEN_OBJECT)|(UINT64_C(1)<<COLLAR_OP_LINK_OBJECT_TO_BUFFER),.generation=generation,.state=COLLAR_GRANT_ACTIVE};
    ++count;sexos_line("[collar.workspace.seed] subject=200 grant=%llu storage_boot_edge=1 kernel_caps_minted=0 revocable=1",(unsigned long long)generation);
  }
  sexos_line("[collar.grant.init] count=%llu generation=%llu", (unsigned long long) count, (unsigned long long) state->COLLAR_GRANT_GENERATION);
}
void collar_render_grants_text(void) {
  uint8_t buffer[120];
  memset(buffer, ' ', sizeof buffer);
  uint8_t total = collar_grant_count();
  memcpy(buffer, "COLLAR GRANTS", 13);
  buffer[14] = (uint8_t)('0' + total / 10 % 10);
  buffer[15] = (uint8_t)('0' + total % 10);
  size_t row = 0;
  uint8_t visible = 0;
  for (size_t i = 0; i < COLLAR_GRANT_CAP && row < 5; ++i) {
    const CollarGrant *grant = &SILK_SHELL_STATE.models.COLLAR_GRANTS[i];
    if (!grant->present || grant->state != COLLAR_GRANT_ACTIVE) continue;
    size_t base = (row + 1) * 20;
    buffer[base] = visible == SILK_SHELL_STATE.models.COLLAR_SELECTED_GRANT_IDX ? '>' : ' ';
    buffer[base + 1] = 'G';
    buffer[base + 2] = (uint8_t)('0' + grant->grant_id / 10 % 10);
    buffer[base + 3] = (uint8_t)('0' + grant->grant_id % 10);
    buffer[base + 5] = 'S';
    buffer[base + 6] = (uint8_t)('0' + grant->subject_id / 100 % 10);
    buffer[base + 7] = (uint8_t)('0' + grant->subject_id / 10 % 10);
    buffer[base + 8] = (uint8_t)('0' + grant->subject_id % 10);
    buffer[base + 10] = 'O';
    buffer[base + 11] = (uint8_t)('0' + grant->object_id / 10 % 10);
    buffer[base + 12] = (uint8_t)('0' + grant->object_id % 10);
    if(grant->object_id==UINT64_MAX){buffer[base+10]='W';buffer[base+11]='S';buffer[base+12]=' ';}
    buffer[base + 14] = 'A';
    ++row;
    ++visible;
  }(void) sexos_pdx_call(SLOT_DISPLAY, OP_TEXT_CLEAR, SURFACE_ID_COLLAR, 0, 0);
  bool accepted = shell_draw_text(SURFACE_ID_COLLAR, buffer, sizeof buffer, UINT64_C(0x00E8FFFF), NULL);
  uint32_t *budget = &SILK_SHELL_STATE.models.collar_render_grants_text__COLLAR_GRANTS_TEXT_BUDGET;
  if (*budget != 0) {
    --*budget;
    sexos_line("[collar.grants.render.ok] grants=%u ok=%u", (unsigned) total, (unsigned) accepted);
  }
}
bool open_linen_object_in_quil(uint64_t object_id) {
  sexos_line("[linen.quil.open.request] id=%llu", (unsigned long long) object_id);
  LinenObject object;
  if (!linen_object_by_id(object_id, &object)) {
    sexos_line("[linen.quil.open.reject.missing] id=%llu", (unsigned long long) object_id);
    return false;
  }
  if(object.disk_path!=0 && object.disk_path<3){sexos_line("[linen.quil.open.reject.workspace] object=%llu disk_path=%u protected=1",(unsigned long long)object_id,(unsigned)object.disk_path);return false;}
  if (object.grant_ref == 0) sexos_line("[linen.quil.open.no_grant] id=%llu kind=%u", (unsigned long long) object_id, (unsigned) object.kind);
  uint8_t capability = collar_check_operation(COLLAR_OP_ACCESS_SEXFILES, SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID, 0);
  if (capability != COLLAR_DECISION_ALLOW) {
    sexos_line("[linen.quil.open.reject.cap] op=AccessSexFiles decision=%u", (unsigned) capability);
    return false;
  }
  uint8_t decision = collar_check_operation(COLLAR_OP_LINK_OBJECT_TO_BUFFER, object_id, 0);
  if (decision != COLLAR_DECISION_ALLOW) {
    sexos_line("[linen.quil.open.reject.collar] decision=%u", (unsigned) decision);
    return false;
  }
  uint8_t kind = QUIL_BUF_KIND_TEXT;
  if (object.kind == LINEN_OBJ_KIND_CODE_FILE) kind = QUIL_BUF_KIND_CODE;
  else if (object.kind == LINEN_OBJ_KIND_MEDIA_ASSET) kind = QUIL_BUF_KIND_LINEN_OBJECT_VIEW;
  else if (object.kind == LINEN_OBJ_KIND_BUILD_ARTIFACT) kind = QUIL_BUF_KIND_BUILD_OUTPUT;
  uint64_t dynamic_id = QUIL_DYNAMIC_BUFFER_ID_BASE + object_id;
  uint64_t linked_id = dynamic_id;
  bool created = false;
  bool found = false;
  for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) {
    QuilBuffer *buffer = &SILK_SHELL_STATE.models.QUIL_BUFFERS[i];
    if (buffer->present && buffer->linen_object_ref == object_id) {
      buffer->state = QUIL_BUF_STATE_OPEN;
      buffer->linked_surface_id = SURFACE_ID_QUIL;
      found = true;
      linked_id = buffer->buffer_id;
      sexos_line("[linen.quil.open.reuse_existing] object_id=%llu buffer_id=%llu", (unsigned long long) object_id, (unsigned long long) buffer->buffer_id);
      break;
    }
  }
  if (!found) {
    for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) {
      const QuilBuffer *buffer = &SILK_SHELL_STATE.models.QUIL_BUFFERS[i];
      if (buffer->present && buffer->buffer_id == dynamic_id && buffer->linen_object_ref != object_id) {
        sexos_line("[linen.quil.open.reject.buffer_id_collision] dynamic_id=%llu existing_ref=%llu", (unsigned long long) dynamic_id, (unsigned long long) buffer->linen_object_ref);
        return false;
      }
    }
    for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) {
      QuilBuffer *buffer = &SILK_SHELL_STATE.models.QUIL_BUFFERS[i];
      if (!buffer->present) {
        sexos_line("[linen.quil.open.dynamic_id] object_id=%llu dynamic_buffer_id=%llu", (unsigned long long) object_id, (unsigned long long) dynamic_id);
        *buffer = (QuilBuffer) {
          .present = true,.buffer_id = dynamic_id,.kind = kind,.state = QUIL_BUF_STATE_OPEN,.linen_object_ref = object_id,.project_id = object.project_id,.grant_ref = object.grant_ref,.linked_surface_id = SURFACE_ID_QUIL,.display_name = object.display_name,
        };
        created = true;
        found = true;
        break;
      }
    }
  }
  if (!found) {
    sexos_line("[linen.quil.open.reject.full] object_id=%llu", (unsigned long long) object_id);
    return false;
  }
  for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
    LinenObject *entry = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
    if (entry->present && entry->object_id == object_id) {
      entry->linked_surface_id = SURFACE_ID_QUIL;
      break;
    }
  }
  sexos_line("[linen.quil.buffer.linked] object_id=%llu buffer_id=%llu kind=%u", (unsigned long long) object_id, (unsigned long long) linked_id, (unsigned) kind);
  if (object.disk_path > 0) {
    uint64_t path = (uint64_t) object.disk_path - 1;
    (void) sexos_pdx_call(SLOT_QUIL, OP_QUIL_OPEN_DISK_DOC, object_id, path == 2 ? 0 : path, 0);
    sexos_line("[linen.quil.disk_doc.intent] object_id=%llu path_id=%llu ok=1", (unsigned long long) object_id, (unsigned long long) path);
  }
  if (open_quil_in_active_scene()) sexos_line("[linen.quil.quil_opened] object_id=%llu", (unsigned long long) object_id);
  sexos_line("[linen.quil.done] object_id=%llu buffer_created=%s", (unsigned long long) object_id, created ? "true" : "false");
  mesh_emit_linen_quil_links();
  bell_emit_object_link_event(object_id, linked_id);
  quil_render_buffer_list();
  return true;
}
void quil_render_buffer_list(void) {
  uint32_t width = SILK_SHELL_STATE.input.SURFACE_201_W;
  uint32_t height = SILK_SHELL_STATE.input.SURFACE_201_H;
  if (width == 0 || height == 0) return;
  sexos_line("[quil.buffer_list.render] w=%u h=%u", (unsigned) width, (unsigned) height);
  (void) sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_QUIL, 0, (uint64_t) QUIL_LIST_HEADER_COLOR << 32 | (uint64_t) QUIL_LIST_HEADER_H << 16 | width);
  size_t count = quil_buffer_count();
  uint32_t background_height = (uint32_t) QUIL_LIST_ACCENT_BARS * (QUIL_LIST_ROW_H + QUIL_LIST_ROW_GAP) - QUIL_LIST_ROW_GAP;
  (void) sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_QUIL, (uint64_t) QUIL_LIST_HEADER_H << 32, UINT64_C(1) << 56 | (uint64_t) QUIL_LIST_BG_COLOR << 32 | (uint64_t) background_height << 16 | width);
  sexos_line("[quil.bg_rect] y=%u h=%u", (unsigned) QUIL_LIST_HEADER_H, (unsigned) background_height);
  sexos_line("[quil.row.reject] reason=no_selection_model");
  uint8_t rows = 0;
  for (size_t i = 0; i < QUIL_MAX_BUFFERS; ++i) {
    const QuilBuffer *buffer = &SILK_SHELL_STATE.models.QUIL_BUFFERS[i];
    if (!buffer->present) continue;
    if (rows >= QUIL_LIST_MAX_ROWS) {
      sexos_line("[quil.buffer_list.skip] id=%llu reason=max_rows", (unsigned long long) buffer->buffer_id);
      continue;
    }
    sexos_line("[quil.buffer_list.row] buffer_id=%llu kind=%s state=%s linen_ref=%llu surface_id=%llu name=%s", (unsigned long long) buffer->buffer_id, quil_buffer_kind_name(buffer->kind), quil_buffer_state_name(buffer->state), (unsigned long long) buffer->linen_object_ref, (unsigned long long) buffer->linked_surface_id, buffer->display_name);
    if (rows < QUIL_LIST_ACCENT_BARS) {
      uint64_t rectangle = ((uint64_t) rows + 3) & 7;
      uint32_t y = QUIL_LIST_HEADER_H + (uint32_t) rows * (QUIL_LIST_ROW_H + QUIL_LIST_ROW_GAP);
      uint32_t color = quil_buffer_kind_color(buffer->kind);
      (void) sexos_pdx_call(SLOT_DISPLAY, 0xEF, SURFACE_ID_QUIL, (uint64_t) y << 32, rectangle << 56 | (uint64_t) color << 32 | (uint64_t) QUIL_LIST_ROW_H << 16 | QUIL_ACCENT_BAR_W);
      sexos_line("[quil.row_visual.accent] index=%llu id=%llu kind=%s color=0x%08x", (unsigned long long) rectangle, (unsigned long long) buffer->buffer_id, quil_buffer_kind_name(buffer->kind), (unsigned) color);
    } else {
      sexos_line("[quil.row_visual.skip] id=%llu reason=accent_budget", (unsigned long long) buffer->buffer_id);
    }
    ++rows;
  }
  sexos_line("[quil.buffer_list.done] count=%zu rows=%u", count, (unsigned) rows);
}
