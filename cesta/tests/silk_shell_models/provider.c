#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "silk_shell.h"

SilkShellState SILK_SHELL_STATE = {.models = {
    .linen_sync_reply__LINEN_SYNC_NONREPLY_BUDGET = 8,
    .collar_render_grants_text__COLLAR_GRANTS_TEXT_BUDGET = 8,
    .send_scene_render_tokens__SHELL_TOKEN_SEND_BUDGET = 4,
    .send_scene_render_tokens__STATE_BUDGET = 1,
    .handle_sexstore_get_reply__STATUS_BUDGET = 4,
    .handle_sexstore_get_reply__DEFAULT_BUDGET = 2,
    .handle_sexstore_get_reply__LOAD_OK_BUDGET = 1,
    .handle_sexstore_get_reply__LOAD_FAIL_BUDGET = 1,
    .boot_load_scene_settings__BOOT_LOAD_BUDGET = 1,
    .cycle_scene_render_token_preset__CYCLE_BUDGET = 16,
    .cycle_scene_render_token_preset__SAVE_BUDGET = 16,
    .cycle_prev_scene_render_token_preset__CYCLE_PREV_BUDGET = 16,
    .cycle_prev_scene_render_token_preset__SAVE_PREV_BUDGET = 16,
    .cycle_custom_tint__TINT_BUDGET = 32,
    .handle_scene_settings_cmd__CMD_BUDGET = 32,
}};

static uint64_t call_status;
static bool draw_accepted = true;
static bool peer_alive;
static bool peer_bell_visible;
static bool peer_open_quil;
static bool peer_surface_present;
static uint64_t peer_surface;
static uint32_t peer_frame;
static SexosMessage messages[1024];
static size_t message_count;
static size_t message_cursor;

void write_str(const char *text) { fputs(text, stdout); }
void write_bytes(const uint8_t *bytes, size_t length) { assert(fwrite(bytes, 1, length, stdout) == length); }
void write_u64_dec(uint64_t number) { printf("%llu", (unsigned long long)number); }
void write_bool_str(bool value) { fputs(value ? "true" : "false", stdout); }
void sexos_fail(void) { abort(); }

bool shell_draw_text(uint64_t surface, const uint8_t *text, size_t length,
                      uint64_t color, size_t *drawn)
{
    printf("draw %llu %llu %zu ", (unsigned long long)surface, (unsigned long long)color, length);
    for (size_t i = 0; i < length; ++i) printf("%02x", (unsigned)text[i]);
    putchar('\n');
    if (drawn != NULL) *drawn = length;
    return draw_accepted;
}

bool active_surface_for_frame(uint32_t frame, uint64_t *surface)
{
    if (!peer_surface_present || frame != peer_frame) return false;
    if (surface != NULL) *surface = peer_surface;
    return true;
}

bool frame_for_surface(uint64_t surface, uint32_t *frame)
{
    if (!peer_surface_present || surface != peer_surface) return false;
    if (frame != NULL) *frame = peer_frame;
    return true;
}

bool surface_is_alive(uint64_t surface) { return peer_alive && surface == peer_surface; }
bool bell_is_visible_in_active_scene(void) { return peer_bell_visible; }
void bell_render_event_list(void) { puts("peer bell-render"); }
bool open_linen_in_active_scene(void) { puts("peer open-linen"); return true; }
bool open_quil_in_active_scene(void) { puts("peer open-quil"); return peer_open_quil; }
bool toggle_top_bar_for_active_frame(void) { puts("peer toggle-topbar"); return true; }
void handle_hid_event(uint64_t event, uint64_t a, uint64_t b)
{
    printf("peer hid %llu %llu %llu\n", (unsigned long long)event,
           (unsigned long long)a, (unsigned long long)b);
}

SexosMessage sexos_listen(SexosListener *listener, uint64_t slot)
{
    (void)listener;
    assert(message_cursor < message_count);
    SexosMessage message = messages[message_cursor++];
    printf("listen %llu %llu %u %llu %llu %llu\n", (unsigned long long)slot,
           (unsigned long long)message.type_id, (unsigned)message.caller_pd,
           (unsigned long long)message.arg0, (unsigned long long)message.arg1,
           (unsigned long long)message.arg2);
    return message;
}

uint64_t sexos_reply(uint32_t target, uint64_t value)
{
    printf("peer reply %llu %llu\n", (unsigned long long)target, (unsigned long long)value);
    return 0;
}

void sexos_line(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);
    putchar('\n');
}

SexosReply sexos_pdx_call(uint64_t slot, uint64_t operation, uint64_t a,
                         uint64_t b, uint64_t c)
{
    printf("call %llu %llu %llu %llu %llu\n", (unsigned long long)slot,
           (unsigned long long)operation, (unsigned long long)a,
           (unsigned long long)b, (unsigned long long)c);
    return (SexosReply){.status = call_status, .value = a ^ b ^ c ^ operation};
}

static void clear_state(void)
{
    SilkShellModelsState retained = SILK_SHELL_STATE.models;
    memset(&SILK_SHELL_STATE, 0, sizeof(SILK_SHELL_STATE));
    SILK_SHELL_STATE.models.COLLAR_GRANT_GENERATION = 1;
    SILK_SHELL_STATE.models.linen_sync_reply__LINEN_SYNC_NONREPLY_BUDGET = retained.linen_sync_reply__LINEN_SYNC_NONREPLY_BUDGET;
    SILK_SHELL_STATE.models.collar_render_grants_text__COLLAR_GRANTS_TEXT_BUDGET = retained.collar_render_grants_text__COLLAR_GRANTS_TEXT_BUDGET;
    SILK_SHELL_STATE.models.send_scene_render_tokens__SHELL_TOKEN_SEND_BUDGET = retained.send_scene_render_tokens__SHELL_TOKEN_SEND_BUDGET;
    SILK_SHELL_STATE.models.send_scene_render_tokens__STATE_BUDGET = retained.send_scene_render_tokens__STATE_BUDGET;
    SILK_SHELL_STATE.models.handle_sexstore_get_reply__STATUS_BUDGET = retained.handle_sexstore_get_reply__STATUS_BUDGET;
    SILK_SHELL_STATE.models.handle_sexstore_get_reply__DEFAULT_BUDGET = retained.handle_sexstore_get_reply__DEFAULT_BUDGET;
    SILK_SHELL_STATE.models.handle_sexstore_get_reply__LOAD_OK_BUDGET = retained.handle_sexstore_get_reply__LOAD_OK_BUDGET;
    SILK_SHELL_STATE.models.handle_sexstore_get_reply__LOAD_FAIL_BUDGET = retained.handle_sexstore_get_reply__LOAD_FAIL_BUDGET;
    SILK_SHELL_STATE.models.boot_load_scene_settings__BOOT_LOAD_BUDGET = retained.boot_load_scene_settings__BOOT_LOAD_BUDGET;
    SILK_SHELL_STATE.models.cycle_scene_render_token_preset__CYCLE_BUDGET = retained.cycle_scene_render_token_preset__CYCLE_BUDGET;
    SILK_SHELL_STATE.models.cycle_scene_render_token_preset__SAVE_BUDGET = retained.cycle_scene_render_token_preset__SAVE_BUDGET;
    SILK_SHELL_STATE.models.cycle_prev_scene_render_token_preset__CYCLE_PREV_BUDGET = retained.cycle_prev_scene_render_token_preset__CYCLE_PREV_BUDGET;
    SILK_SHELL_STATE.models.cycle_prev_scene_render_token_preset__SAVE_PREV_BUDGET = retained.cycle_prev_scene_render_token_preset__SAVE_PREV_BUDGET;
    SILK_SHELL_STATE.models.cycle_custom_tint__TINT_BUDGET = retained.cycle_custom_tint__TINT_BUDGET;
    SILK_SHELL_STATE.models.handle_scene_settings_cmd__CMD_BUDGET = retained.handle_scene_settings_cmd__CMD_BUDGET;
    message_count = 0;
    message_cursor = 0;
    call_status = 0;
    draw_accepted = true;
    peer_alive = false;
    peer_bell_visible = false;
    peer_open_quil = false;
    peer_surface_present = false;
    peer_surface = 0;
    peer_frame = 0;
}

static uint64_t number(void)
{
    unsigned long long value;
    if (scanf("%llu", &value) != 1)
        exit(2);
    return value;
}

static void snapshot(void)
{
    printf("selection %llu %zu %zu %zu %u %u\n",
           (unsigned long long)SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID,
           linen_selected_index(), linen_object_count(), quil_buffer_count(),
           (unsigned)SILK_SHELL_STATE.models.LINEN_OBJECT_DETAIL_OPEN,
           (unsigned)linen_selected_object_accent());
    for (size_t i = 0; i < LINEN_MAX_OBJECTS; ++i) {
        const LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[i];
        if (object->present)
            printf("object %zu %llu %u %u %llu\n", i,
                   (unsigned long long)object->object_id, (unsigned)object->kind,
                   (unsigned)object->state, (unsigned long long)object->linked_surface_id);
    }
}

static void mesh_row(const MeshFact *fact, void *context)
{
    (void)context;
    printf("mesh-row %llu %u %llu %llu %llu %llu %u\n", (unsigned long long)fact->fact_id,
           (unsigned)fact->kind, (unsigned long long)fact->subject_id,
           (unsigned long long)fact->object_id, (unsigned long long)fact->ref_id,
           (unsigned long long)fact->sequence, (unsigned)mesh_fact_row_color(fact));
}

static void bell_row(const BellEvent *event, void *context)
{
    (void)context;
    printf("bell-row %llu %u %llu %llu %llu\n", (unsigned long long)event->event_id,
           (unsigned)event->kind, (unsigned long long)event->object_id,
           (unsigned long long)event->buffer_id, (unsigned long long)event->sequence);
}

int main(void)
{
    char command[24];
    while (scanf("%23s", command) == 1) {
        if (strcmp(command, "caps") == 0) {
            uint8_t bits = (uint8_t)number();
            AppCapabilityBits result = {.bits = 173};
            bool valid = app_capability_bits_validate(bits, &result);
            assert(app_capability_bits_validate(bits, NULL) == valid);
            if (!valid)
                assert(result.bits == 173);
            AppCapabilityBits unchecked = {.bits = bits};
            printf("caps %u %u %u %u %s\n", (unsigned)valid,
                   (unsigned)app_capability_bits_bits(unchecked),
                   (unsigned)app_capability_bits_has_bell(unchecked),
                   (unsigned)app_capability_bits_has_sexfiles(unchecked),
                   app_capability_bits_describe(unchecked));
        } else if (strcmp(command, "manifest") == 0) {
            uint64_t surface = number(), title = number(), word = number();
            AppManifest result = {.surface_id = 17, .title_id = 19,
                                  .app_id = 23, .capabilities = {.bits = 29}};
            bool valid = app_manifest_unpack(surface, title, word, &result);
            assert(app_manifest_unpack(surface, title, word, NULL) == valid);
            if (valid) {
                uint64_t a, b, c;
                app_manifest_pack(&result, &a, &b, &c);
                app_manifest_pack(&result, NULL, NULL, NULL);
                printf("manifest 1 %llu %llu %llu %u %u %u\n",
                       (unsigned long long)a, (unsigned long long)b,
                       (unsigned long long)c, (unsigned)result.app_id,
                       (unsigned)result.capabilities.bits,
                       (unsigned)app_manifest_has_identity(&result));
            } else {
                assert(result.surface_id == 17 && result.title_id == 19 &&
                       result.app_id == 23 && result.capabilities.bits == 29);
                puts("manifest 0");
            }
        } else if (strcmp(command, "canvas") == 0) {
            uint32_t w = (uint32_t)number(), h = (uint32_t)number();
            uint32_t x = (uint32_t)number(), y = (uint32_t)number();
            uint32_t rw = (uint32_t)number(), rh = (uint32_t)number();
            uint32_t color = (uint32_t)number();
            unsigned panel = (unsigned)number();
            uint32_t buffer[5202];
            for (size_t i = 0; i < 5202; ++i)
                buffer[i] = UINT32_C(0x531ba92e);
            assert(w <= 80 && h <= 65);
            Canvas canvas = canvas_new(buffer + 1, w, h);
            canvas_fill_rect(&canvas, x, y, rw, rh, color);
            if (panel != 0) {
                ShellState state = {.is_launcher_open = panel == 2};
                canvas_draw_panel(&canvas, &state);
            }
            uint64_t hash = UINT64_C(0xcbf29ce484222325);
            for (size_t i = 0; i < 5202; ++i)
                hash = (hash ^ buffer[i]) * UINT64_C(0x100000001b3);
            assert(buffer[0] == UINT32_C(0x531ba92e) && buffer[5201] == UINT32_C(0x531ba92e));
            printf("canvas %llu\n", (unsigned long long)hash);
        } else if (strcmp(command, "compositor") == 0) {
            int32_t x = (int32_t)(uint32_t)number(), y = (int32_t)(uint32_t)number();
            uint32_t w = (uint32_t)number(), h = (uint32_t)number();
            uint32_t result = pdx_compositor_client_create_window(x, y, w, h);
            printf("window %u\n", (unsigned)result);
            pdx_compositor_client_set_bg(w);
            pdx_compositor_client_render_bar(result);
        } else if (strcmp(command, "reset") == 0) {
            clear_state();
            linen_object_table_init();
            quil_buffer_table_init();
        } else if (strcmp(command, "clear") == 0) {
            clear_state();
        } else if (strcmp(command, "erase") == 0) {
            size_t index = (size_t)number();
            assert(index < LINEN_MAX_OBJECTS);
            SILK_SHELL_STATE.models.LINEN_OBJECTS[index].present = false;
        } else if (strcmp(command, "put") == 0) {
            size_t index = (size_t)number();
            uint64_t id = number();
            assert(index < LINEN_MAX_OBJECTS);
            LinenObject object = LINEN_SEED_OBJECTS[index % 6];
            object.present = true;
            object.object_id = id;
            SILK_SHELL_STATE.models.LINEN_OBJECTS[index] = object;
        } else if (strcmp(command, "select") == 0) {
            SILK_SHELL_STATE.models.SELECTED_LINEN_OBJECT_ID = number();
        } else if (strcmp(command, "focus") == 0) {
            SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID = number();
        } else if (strcmp(command, "first") == 0) {
            linen_select_first_valid_object();
        } else if (strcmp(command, "next") == 0) {
            linen_select_next_object();
        } else if (strcmp(command, "prev") == 0) {
            linen_select_prev_object();
        } else if (strcmp(command, "selected") == 0) {
            uint64_t id = linen_selected_object_id();
            printf("selected %llu\n", (unsigned long long)id);
        } else if (strcmp(command, "detail") == 0) {
            linen_object_detail_open();
        } else if (strcmp(command, "close") == 0) {
            linen_object_detail_close();
        } else if (strcmp(command, "cohere") == 0) {
            linen_quil_seed_coherence_init();
        } else if (strcmp(command, "snapshot") == 0) {
            snapshot();
        } else if (strcmp(command, "lookup") == 0) {
            uint64_t id = number();
            LinenObject object = {.object_id = 991};
            QuilBuffer buffer = {.buffer_id = 993};
            bool found_object = linen_object_by_id(id, &object);
            bool found_buffer = quil_buffer_by_id(id, &buffer);
            assert(linen_object_by_id(id, NULL) == found_object);
            assert(quil_buffer_by_id(id, NULL) == found_buffer);
            if (!found_object) assert(object.object_id == 991);
            if (!found_buffer) assert(buffer.buffer_id == 993);
            printf("lookup %u %llu %u %llu\n", (unsigned)found_object,
                   (unsigned long long)(found_object ? object.object_id : 0),
                   (unsigned)found_buffer, (unsigned long long)(found_buffer ? buffer.buffer_id : 0));
        } else if (strcmp(command, "kind") == 0) {
            unsigned kind = (unsigned)number(), state = (unsigned)number();
            assert(kind < 11 && state < 30);
            printf("linen %s %s %u\n", linen_object_kind_name((uint8_t)kind),
                   linen_object_state_name((uint8_t)(state % 5)), (unsigned)linen_kind_color((uint8_t)kind));
            printf("quil %s %s %u\n", quil_buffer_kind_name((uint8_t)(kind % 8)),
                   quil_buffer_state_name((uint8_t)(state % 6)),
                   (unsigned)quil_buffer_kind_color((uint8_t)(kind % 8)));
        } else if (strcmp(command, "ascii") == 0) {
            printf("ascii %u\n", (unsigned)sanitize_ascii((uint8_t)number()));
        } else if (strcmp(command, "preset") == 0) {
            printf("preset %s\n", get_preset_name((uint8_t)number()));
        } else if (strcmp(command, "tokens") == 0) {
            SceneAppearanceState *state = &SILK_SHELL_STATE.models.SCENE_APPEARANCE_STATE;
            state->preset_idx = (uint8_t)number();
            state->use_custom_colors = (uint8_t)number();
            for (size_t i = 0; i < 8; ++i)
                state->custom_colors[i] = (uint32_t)number();
            uint32_t result[8];
            resolve_scene_render_tokens(result);
            printf("tokens");
            for (size_t i = 0; i < 8; ++i)
                printf(" %u", (unsigned)result[i]);
            putchar('\n');
        } else if (strcmp(command, "pack") == 0) {
            uint8_t preset = (uint8_t)number(), chrome = (uint8_t)number(), access = (uint8_t)number();
            printf("pack %llu\n", (unsigned long long)pack_scene_settings_blob(preset, chrome, access));
        } else if (strcmp(command, "blob") == 0) {
            uint64_t blob = number();
            uint8_t preset = 17, chrome = 19, access = 23;
            bool valid = unpack_scene_settings_blob(blob, &preset, &chrome, &access);
            assert(unpack_scene_settings_blob(blob, NULL, NULL, NULL) == valid);
            if (!valid)
                assert(preset == 17 && chrome == 19 && access == 23);
            printf("blob %u %u %u %u\n", (unsigned)valid,
                   (unsigned)(valid ? preset : 0), (unsigned)(valid ? chrome : 0),
                   (unsigned)(valid ? access : 0));
        } else if (strcmp(command, "reply") == 0) {
            uint64_t value = number();
            printf("reply %u %llu %u\n", (unsigned)store_reply_is_status(value),
                   (unsigned long long)store_reply_status(value), (unsigned)store_reply_is_value(value));
        } else if (strcmp(command, "pair") == 0) {
            uint32_t low = (uint32_t)number(), high = (uint32_t)number();
            printf("pair %llu\n", (unsigned long long)pack_u32_pair(low, high));
        } else if (strcmp(command, "surface") == 0) {
            uint64_t id = number();
            const AppSurfaceSpec *by_surface = app_surface_spec(id);
            const AppSurfaceSpec *by_frame = app_surface_spec_by_frame((uint32_t)id);
            printf("surface %s %s\n", by_surface ? by_surface->name : "none",
                   by_frame ? by_frame->name : "none");
        } else if (strcmp(command, "grants") == 0) {
            collar_init_grants();
        } else if (strcmp(command, "grant") == 0) {
            size_t index = (size_t)number();
            uint64_t subject = number(), object = number(), mask = number();
            uint8_t state = (uint8_t)number();
            assert(index < COLLAR_GRANT_CAP && state <= COLLAR_GRANT_TOMBSTONED);
            SILK_SHELL_STATE.models.COLLAR_GRANTS[index] = (CollarGrant){
                .present = true, .grant_id = index + 1, .subject_id = subject,
                .object_id = object, .operation_mask = mask,
                .generation = index + 1, .state = state,
            };
        } else if (strcmp(command, "grant-generation") == 0) {
            SILK_SHELL_STATE.models.COLLAR_GRANT_GENERATION = number();
        } else if (strcmp(command, "audit-index") == 0) {
            SILK_SHELL_STATE.models.COLLAR_AUDIT_WRITE_INDEX = number();
        } else if (strcmp(command, "autogrant") == 0 || strcmp(command, "review") == 0) {
            AppManifest manifest = {.surface_id = number()};
            manifest.app_id = (uint16_t)number();
            manifest.capabilities.bits = (uint8_t)number();
            if (strcmp(command, "autogrant") == 0) {
                collar_auto_grant_from_manifest(&manifest);
            } else {
                CollarReview result = collar_review_manifest(&manifest);
                printf("review %u %u %u\n", (unsigned)result.allowed,
                       (unsigned)result.granted_caps, (unsigned)result.denied_caps);
            }
        } else if (strcmp(command, "policy") == 0) {
            uint8_t operation = (uint8_t)number();
            uint64_t object = number(), buffer = number();
            assert(operation <= COLLAR_OP_ACCESS_SHELL_POLICY);
            uint8_t decision = collar_check_operation(operation, object, buffer);
            printf("policy %u\n", (unsigned)decision);
        } else if (strcmp(command, "grant-visible") == 0) {
            uint8_t index = (uint8_t)number();
            CollarGrant grant = {.grant_id = 771};
            bool found = collar_grant_at_visible_index(index, &grant);
            assert(collar_grant_at_visible_index(index, NULL) == found);
            if (!found) assert(grant.grant_id == 771);
            printf("visible %u %llu\n", (unsigned)found,
                   (unsigned long long)(found ? grant.grant_id : 0));
        } else if (strcmp(command, "grant-dump") == 0) {
            printf("grants %llu %u\n", (unsigned long long)SILK_SHELL_STATE.models.COLLAR_GRANT_GENERATION,
                   (unsigned)collar_grant_count());
            for (size_t i = 0; i < COLLAR_GRANT_CAP; ++i) {
                const CollarGrant *grant = &SILK_SHELL_STATE.models.COLLAR_GRANTS[i];
                if (grant->present)
                    printf("grant %zu %llu %llu %llu %llu %llu %u\n", i,
                           (unsigned long long)grant->grant_id, (unsigned long long)grant->subject_id,
                           (unsigned long long)grant->object_id, (unsigned long long)grant->operation_mask,
                           (unsigned long long)grant->generation, (unsigned)grant->state);
            }
        } else if (strcmp(command, "audit-dump") == 0) {
            printf("audits %llu\n", (unsigned long long)SILK_SHELL_STATE.models.COLLAR_AUDIT_WRITE_INDEX);
            for (size_t i = 0; i < COLLAR_AUDIT_CAP; ++i) {
                const CollarAuditEvent *event = &SILK_SHELL_STATE.models.COLLAR_AUDIT_EVENTS[i];
                if (event->present)
                    printf("audit %zu %llu %u %llu %llu %u %llu %llu\n", i,
                           (unsigned long long)event->event_id, (unsigned)event->operation,
                           (unsigned long long)event->object_id, (unsigned long long)event->subject_id,
                           (unsigned)event->decision, (unsigned long long)event->grant_ref,
                           (unsigned long long)event->reason);
            }
        } else if (strcmp(command, "geometry") == 0) {
            SILK_SHELL_STATE.input.SURFACE_200_W = (uint32_t)number();
            SILK_SHELL_STATE.input.SURFACE_200_H = (uint32_t)number();
            SILK_SHELL_STATE.input.SURFACE_201_W = (uint32_t)number();
            SILK_SHELL_STATE.input.SURFACE_201_H = (uint32_t)number();
            SILK_SHELL_STATE.input.SURFACE_202_W = (uint32_t)number();
            SILK_SHELL_STATE.input.SURFACE_202_H = (uint32_t)number();
        } else if (strcmp(command, "peer") == 0) {
            peer_frame = (uint32_t)number(); peer_surface = number();
            peer_surface_present = number() != 0; peer_alive = number() != 0;
            peer_bell_visible = number() != 0; peer_open_quil = number() != 0;
            draw_accepted = number() != 0; call_status = number();
        } else if (strcmp(command, "frame") == 0) {
            size_t index = (size_t)number(); uint32_t frame = (uint32_t)number();
            uint8_t scene = (uint8_t)number(); uint32_t flags = (uint32_t)number();
            bool present = number() != 0; assert(index < MAX_FRAMES);
            SILK_SHELL_STATE.chrome.FRAMES[index] = (ShellFrame){.present = present,
                .frame_id = frame, .scene_id = scene, .flags = flags};
        } else if (strcmp(command, "scene") == 0) {
            SILK_SHELL_STATE.chrome.ACTIVE_SCENE_IDX = (uint8_t)number();
        } else if (strcmp(command, "name") == 0) {
            size_t index = (size_t)number(), length = (size_t)number();
            assert(index < LINEN_MAX_OBJECTS && length <= 24);
            LinenObject *object = &SILK_SHELL_STATE.models.LINEN_OBJECTS[index];
            object->name_len = (uint8_t)length; memset(object->name, 0, sizeof(object->name));
            for (size_t i = 0; i < length; ++i) object->name[i] = (uint8_t)number();
        } else if (strcmp(command, "disk") == 0) {
            size_t index = (size_t)number(); assert(index < LINEN_MAX_OBJECTS);
            SILK_SHELL_STATE.models.LINEN_OBJECTS[index].disk_path = (uint8_t)number();
        } else if (strcmp(command, "qput") == 0) {
            size_t index = (size_t)number(); uint64_t id = number(), ref = number();
            uint8_t kind = (uint8_t)number(), state = (uint8_t)number(); uint64_t surface = number();
            assert(index < QUIL_MAX_BUFFERS && kind < 8 && state < 6);
            SILK_SHELL_STATE.models.QUIL_BUFFERS[index] = (QuilBuffer){.present = true,
                .buffer_id = id, .linen_object_ref = ref, .kind = kind, .state = state,
                .linked_surface_id = surface, .display_name = "fixture"};
        } else if (strcmp(command, "qerase") == 0) {
            size_t index = (size_t)number(); assert(index < QUIL_MAX_BUFFERS);
            SILK_SHELL_STATE.models.QUIL_BUFFERS[index].present = false;
        } else if (strcmp(command, "message") == 0) {
            SexosMessage message = {.type_id = number()}; message.caller_pd = (uint32_t)number();
            message.arg0 = number(); message.arg1 = number(); message.arg2 = number();
            assert(message_count < 1024); messages[message_count++] = message;
        } else if (strcmp(command, "sync") == 0) {
            uint64_t value = linen_sync_reply(); printf("sync %llu\n", (unsigned long long)value);
        } else if (strcmp(command, "remote") == 0) { linen_fetch_remote_snapshot();
        } else if (strcmp(command, "paint") == 0) { linen_paint_surface();
        } else if (strcmp(command, "paint-fast") == 0) { linen_paint_surface_fast();
        } else if (strcmp(command, "linen-render") == 0) { linen_render_object_list();
        } else if (strcmp(command, "linen-static") == 0) { linen_render_static_ui();
        } else if (strcmp(command, "quil-render") == 0) { quil_render_buffer_list();
        } else if (strcmp(command, "mesh-render") == 0) { mesh_render_fact_list();
        } else if (strcmp(command, "mesh-graph") == 0) { mesh_render_pd_graph();
        } else if (strcmp(command, "collar-render") == 0) { collar_render_grants_text();
        } else if (strcmp(command, "grant-next") == 0) { collar_select_next_grant();
        } else if (strcmp(command, "grant-prev") == 0) { collar_select_prev_grant();
        } else if (strcmp(command, "grant-detail") == 0) {
            printf("grant-detail %u\n", (unsigned)collar_emit_selected_grant_detail());
        } else if (strcmp(command, "grant-selected") == 0) {
            SILK_SHELL_STATE.models.COLLAR_SELECTED_GRANT_IDX = (uint8_t)number();
        } else if (strcmp(command, "mesh-selected") == 0) {
            SILK_SHELL_STATE.surfaces.MESH_SELECTED_ROW = (uint8_t)number();
        } else if (strcmp(command, "mesh-next") == 0) { mesh_select_next_row();
        } else if (strcmp(command, "mesh-prev") == 0) { mesh_select_prev_row();
        } else if (strcmp(command, "mesh-detail") == 0) {
            printf("mesh-detail %u\n", (unsigned)mesh_emit_selected_fact_detail_proof());
        } else if (strcmp(command, "mesh-snapshot") == 0) {
            MeshFact fact = {.fact_id = 901}; bool found = mesh_selected_fact_snapshot(&fact);
            assert(mesh_selected_fact_snapshot(NULL) == found);
            if (!found) assert(fact.fact_id == 901);
            printf("mesh-snapshot %u %llu\n", (unsigned)found, (unsigned long long)(found ? fact.fact_id : 0));
        } else if (strcmp(command, "mesh-focus") == 0) {
            MeshFact fact; if (mesh_selected_fact_snapshot(&fact)) mesh_focus_linen_at_selected_fact(&fact);
        } else if (strcmp(command, "mesh-link") == 0) { mesh_emit_linen_quil_links();
        } else if (strcmp(command, "mesh-add") == 0) {
            uint64_t subject = number(), object = number(), ref = number();
            mesh_record_fact(MESH_FACT_OBJECT_LINKED_TO_BUFFER, subject, object, ref);
        } else if (strcmp(command, "bell-add") == 0 || strcmp(command, "bell-link") == 0) {
            uint64_t object = number(), buffer = number();
            if (strcmp(command, "bell-add") == 0) bell_record_event(object, buffer);
            else bell_emit_object_link_event(object, buffer);
        } else if (strcmp(command, "ring-indices") == 0) {
            SILK_SHELL_STATE.models.MESH_FACT_WRITE_INDEX = number();
            SILK_SHELL_STATE.models.MESH_FACT_SEQUENCE = number();
            SILK_SHELL_STATE.models.BELL_RING_WRITE_INDEX = number();
            SILK_SHELL_STATE.models.BELL_EVENT_SEQUENCE = number();
        } else if (strcmp(command, "rings") == 0) {
            printf("rings %zu %zu %u %u\n", mesh_fact_count(), bell_ring_count(),
                   (unsigned)mesh_visible_fact_count(), (unsigned)mesh_is_visible_in_active_scene());
            mesh_for_each_fact(mesh_row, NULL); bell_for_each_event(bell_row, NULL);
        } else if (strcmp(command, "highlight") == 0) {
            printf("highlight %u\n", (unsigned)mesh_selected_row_highlight((uint32_t)number()));
        } else if (strcmp(command, "open") == 0) {
            bool opened = open_linen_object_in_quil(number()); printf("opened %u\n", (unsigned)opened);
        } else if (strcmp(command, "registry") == 0) { app_surface_registry_validate();
        } else if (strcmp(command, "push") == 0) {
            uint32_t tokens[8]; for (size_t i = 0; i < 8; ++i) tokens[i] = (uint32_t)number();
            push_token_preset(tokens);
        } else if (strcmp(command, "send") == 0) { send_scene_render_tokens();
        } else if (strcmp(command, "get-reply") == 0) { handle_sexstore_get_reply(number());
        } else if (strcmp(command, "boot-load") == 0) { boot_load_scene_settings();
        } else if (strcmp(command, "preset-next") == 0) { cycle_scene_render_token_preset();
        } else if (strcmp(command, "preset-prev") == 0) { cycle_prev_scene_render_token_preset();
        } else if (strcmp(command, "tint-next") == 0) { cycle_custom_tint();
        } else if (strcmp(command, "tint") == 0) {
            size_t index = (size_t)number(); assert(index < TINT_COUNT); apply_custom_tint_bundle(index);
        } else if (strcmp(command, "settings-command") == 0) {
            uint64_t operation = number(), value = number(), flags = number();
            handle_scene_settings_cmd(operation, value, flags);
        } else if (strcmp(command, "settings") == 0) {
            const SceneAppearanceState *state = &SILK_SHELL_STATE.models.SCENE_APPEARANCE_STATE;
            printf("settings %u %u %u %u %u %u", (unsigned)state->preset_idx,
                   (unsigned)state->use_custom_colors, (unsigned)state->chrome_flags,
                   (unsigned)state->accessibility_flags, (unsigned)SILK_SHELL_STATE.models.ACTIVE_TINT_IDX,
                   (unsigned)SILK_SHELL_STATE.models.SEXSTORE_LOAD_PENDING);
            for (size_t i = 0; i < 8; ++i) printf(" %u", (unsigned)state->custom_colors[i]);
            putchar('\n');
        } else if (strcmp(command, "view") == 0) {
            SILK_SHELL_STATE.chrome.ATLAS_MODE_ENABLED = number() != 0;
            printf("view %u\n", (unsigned)shell_view_mode());
        } else {
            fprintf(stderr, "Unknown command: %s\n", command);
            return 2;
        }
    }
    return 0;
}
