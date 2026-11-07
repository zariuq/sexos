#!/usr/bin/env python3
"""Replay the original Models cohort, independently of the live current-source gate."""
import argparse
import filecmp
import hashlib
import itertools
import os
from pathlib import Path
import random
import re
import shutil
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
FIXTURES = HERE / 'tests/silk_shell_models'
REVISION = '4a040a86'
HASHES = {
    'main.rs': 'cca39fa1d39581f7a0f4f52faa9258a2ac5cae443867c5bebe37735bc96d4a29',
    'lib.rs': '22cd4d000b47a4bde5f980d6fc0e142a237fee819b0fc2593266b329e677d7ac',
}
DEPENDENCIES = {
    'crates/sex-pdx/src/lib.rs': ('1bf128fd0eed0343427ebde9e920ad921f49996e57e103d1d27fcae9e995445e',
        ('SLOT_DISPLAY', 'SLOT_QUIL', 'SLOT_SEXSTORE', 'SLOT_LINEN', 'OP_APPEARANCE_TOKENS', 'OP_TEXT_CLEAR', 'OP_QUIL_OPEN_DISK_DOC')),
    'crates/silkbar-model/src/lib.rs': ('d9ca50aef07ebdfdeb09ea4ef8bbd87b003d301ea865118b23ed85a98530f973',
        ('APPEARANCE_TOKEN_FOCUS_SURFACE', 'APPEARANCE_TOKEN_FRAME_RIM', 'APPEARANCE_TOKEN_FRAME_TOP_BAR',
         'APPEARANCE_TOKEN_ACTIVE_TAB', 'APPEARANCE_TOKEN_INACTIVE_TAB', 'APPEARANCE_TOKEN_CLOSE_LIGHT',
         'APPEARANCE_TOKEN_MINIMIZE_LIGHT', 'APPEARANCE_TOKEN_ZOOM_LIGHT')),
}

MAIN_ITEMS = [
    ('OP_KV_GET', 32, 32),
    ('OP_KV_PUT', 33, 33),
    ('CMD_SET_PRESET', 39, 39),
    ('CMD_CYCLE_PRESET', 40, 40),
    ('CMD_SET_TINT', 41, 41),
    ('CMD_CYCLE_TINT', 42, 42),
    ('CMD_SET_CHROME_FLAGS', 43, 43),
    ('CMD_TOGGLE_TOP_BAR', 44, 44),
    ('CMD_SET_ACCESSIBILITY', 45, 45),
    ('CMD_RESET_DEFAULTS', 46, 46),
    ('COLLAR_ENFORCE_PROOF_ENABLED', 82, 83),
    ('SCENE_SETTINGS_KEY_APPEARANCE', 4309, 4309),
    ('SCENE_BLOB_MAGIC', 4312, 4312),
    ('SCENE_BLOB_VERSION', 4313, 4313),
    ('OP_HID_EVENT', 4316, 4316),
    ('OP_LINEN_GET_PUBLIC_SNAPSHOT', 4318, 4318),
    ('OP_LINEN_GET_PUBLIC_NAME', 4319, 4319),
    ('SURFACE_ID_LINEN', 4338, 4338),
    ('SURFACE_ID_QUIL', 4339, 4339),
    ('SURFACE_ID_MESH', 4340, 4340),
    ('SURFACE_ID_COLLAR', 4341, 4341),
    ('SURFACE_ID_BELL_PLACEHOLDER', 4342, 4342),
    ('SURFACE_ID_BROWSER', 4343, 4343),
    ('SURFACE_ID_COMMAND_PALETTE', 4366, 4366),
    ('SURFACE_ID_SPINDLE', 4367, 4367),
    ('AppSurfaceSpec', 4387, 4397),
    ('APP_SURFACES', 4400, 4489),
    ('LINEN_MAX_OBJECTS', 4496, 4496),
    ('LinenObjectKind', 4499, 4513),
    ('LinenObjectState', 4516, 4524),
    ('LinenObject', 4527, 4544),
    ('LINEN_OBJECTS', 4548, 4548),
    ('SELECTED_LINEN_OBJECT_ID', 4553, 4553),
    ('LINEN_OBJECT_DETAIL_OPEN', 4555, 4555),
    ('LINEN_REMOTE_FETCHED', 4559, 4559),
    ('LINEN_REMOTE_REAL', 4562, 4562),
    ('LINEN_SEED_OBJECTS', 4565, 4650),
    ('linen_object_table_init', 4654, 4662),
    ('linen_object_count', 4665, 4673),
    ('linen_object_by_id', 4676, 4685),
    ('linen_object_kind_name', 4688, 4702),
    ('linen_object_state_name', 4705, 4713),
    ('linen_selected_object_id', 4718, 4729),
    ('linen_select_first_valid_object', 4732, 4741),
    ('linen_select_next_object', 4746, 4773),
    ('linen_select_prev_object', 4778, 4803),
    ('linen_object_detail_open', 4808, 4839),
    ('linen_object_detail_close', 4842, 4849),
    ('linen_selected_index', 4852, 4862),
    ('LINEN_LIST_MAX_ROWS', 4865, 4865),
    ('LINEN_LIST_ROW_H', 4867, 4867),
    ('LINEN_LIST_ROW_GAP', 4869, 4869),
    ('LINEN_LIST_HEADER_COLOR', 4871, 4871),
    ('LINEN_LIST_HEADER_H', 4873, 4873),
    ('LINEN_LIST_ACCENT_BARS', 4875, 4875),
    ('LINEN_LIST_BG_COLOR', 4877, 4877),
    ('LINEN_ACCENT_BAR_W', 4879, 4879),
    ('LINEN_UI_ROW_COUNT', 4882, 4882),
    ('LINEN_UI_ROW_COLORS', 4883, 4889),
    ('LINEN_UI_SELECTED', 4890, 4890),
    ('LINEN_SURFACE_VISUAL_H', 4893, 4894),
    ('linen_kind_color', 4898, 4912),
    ('linen_selected_object_accent', 4917, 4930),
    ('linen_render_object_list', 4938, 5045),
    ('linen_render_static_ui', 5051, 5121),
    ('linen_sync_reply', 5127, 5153),
    ('sanitize_ascii', 5156, 5158),
    ('linen_fetch_remote_snapshot', 5166, 5255),
    ('linen_paint_surface', 5263, 5273),
    ('linen_paint_surface_fast', 5279, 5289),
    ('QUIL_MAX_BUFFERS', 5296, 5296),
    ('QUIL_DYNAMIC_BUFFER_ID_BASE', 5301, 5301),
    ('QUIL_LIST_MAX_ROWS', 5304, 5304),
    ('QUIL_LIST_HEADER_COLOR', 5306, 5306),
    ('QUIL_LIST_HEADER_H', 5308, 5308),
    ('QUIL_LIST_ACCENT_BARS', 5310, 5310),
    ('QUIL_LIST_ROW_H', 5312, 5312),
    ('QUIL_LIST_ROW_GAP', 5314, 5314),
    ('QUIL_LIST_BG_COLOR', 5316, 5316),
    ('QUIL_ACCENT_BAR_W', 5318, 5318),
    ('QuilBufferKind', 5320, 5331),
    ('QuilBufferState', 5334, 5343),
    ('QuilBuffer', 5346, 5357),
    ('QUIL_BUFFERS', 5361, 5361),
    ('QUIL_SEED_BUFFERS', 5364, 5431),
    ('quil_buffer_table_init', 5435, 5443),
    ('quil_buffer_count', 5446, 5454),
    ('quil_buffer_by_id', 5457, 5466),
    ('quil_buffer_kind_name', 5469, 5480),
    ('quil_buffer_state_name', 5483, 5492),
    ('quil_buffer_kind_color', 5496, 5507),
    ('quil_render_buffer_list', 5520, 5587),
    ('linen_quil_seed_coherence_init', 5596, 5616),
    ('open_linen_object_in_quil', 5626, 5786),
    ('CollarOperation', 5795, 5813),
    ('CollarDecision', 5817, 5832),
    ('collar_check_operation', 5843, 5998),
    ('CollarGrantState', 6006, 6013),
    ('CollarGrant', 6016, 6025),
    ('CollarAuditEvent', 6028, 6038),
    ('COLLAR_GRANT_CAP', 6040, 6040),
    ('COLLAR_AUDIT_CAP', 6041, 6041),
    ('COLLAR_GRANTS', 6043, 6043),
    ('COLLAR_GRANT_GENERATION', 6044, 6044),
    ('COLLAR_AUDIT_EVENTS', 6046, 6046),
    ('COLLAR_AUDIT_WRITE_INDEX', 6047, 6047),
    ('COLLAR_SELECTED_GRANT_IDX', 6048, 6048),
    ('record_collar_audit', 6052, 6081),
    ('collar_init_grants', 6085, 6126),
    ('collar_grant_count', 6128, 6138),
    ('collar_grant_at_visible_index', 6140, 6152),
    ('collar_select_next_grant', 6154, 6165),
    ('collar_select_prev_grant', 6167, 6178),
    ('collar_render_grants_text', 6184, 6226),
    ('collar_emit_selected_grant_detail', 6228, 6243),
    ('collar_auto_grant_from_manifest', 6252, 6291),
    ('CollarReview', 6294, 6302),
    ('collar_review_manifest', 6306, 6323),
    ('MeshFactKind', 6331, 6336),
    ('MeshFact', 6340, 6355),
    ('MESH_FACT_RING_CAP', 6358, 6358),
    ('MESH_FACTS', 6360, 6360),
    ('MESH_FACT_WRITE_INDEX', 6362, 6362),
    ('MESH_FACT_SEQUENCE', 6364, 6364),
    ('mesh_record_fact', 6368, 6394),
    ('mesh_fact_count', 6397, 6401),
    ('mesh_for_each_fact', 6405, 6417),
    ('mesh_emit_linen_quil_links', 6429, 6469),
    ('mesh_fact_row_color', 6474, 6484),
    ('mesh_is_visible_in_active_scene', 6488, 6504),
    ('mesh_render_fact_list', 6508, 6574),
    ('mesh_render_pd_graph', 6582, 6633),
    ('mesh_visible_fact_count', 6636, 6640),
    ('mesh_selected_row_highlight', 6644, 6649),
    ('mesh_select_next_row', 6652, 6663),
    ('mesh_select_prev_row', 6666, 6677),
    ('mesh_selected_fact_snapshot', 6683, 6697),
    ('mesh_emit_selected_fact_detail_proof', 6703, 6726),
    ('mesh_focus_linen_at_selected_fact', 6731, 6738),
    ('BellEventKind', 6747, 6754),
    ('BellEvent', 6758, 6771),
    ('BELL_RING_CAP', 6775, 6775),
    ('BELL_EVENTS', 6777, 6777),
    ('BELL_RING_WRITE_INDEX', 6779, 6779),
    ('BELL_EVENT_SEQUENCE', 6781, 6781),
    ('bell_record_event', 6785, 6803),
    ('bell_ring_count', 6806, 6810),
    ('bell_for_each_event', 6814, 6826),
    ('bell_emit_object_link_event', 6832, 6879),
    ('app_surface_registry_validate', 6981, 7003),
    ('app_surface_spec', 7006, 7008),
    ('app_surface_spec_by_frame', 7011, 7014),
    ('PRESET_COUNT', 7020, 7020),
    ('TokenPreset', 7021, 7021),
    ('TOKEN_PRESETS', 7023, 7032),
    ('PRESET_NAMES', 7036, 7042),
    ('get_preset_name', 7045, 7048),
    ('SceneAppearanceState', 7052, 7066),
    ('DEFAULT_SCENE_APPEARANCE', 7068, 7074),
    ('SCENE_APPEARANCE_STATE', 7076, 7076),
    ('TINT_COUNT', 7078, 7078),
    ('TintBundle', 7079, 7079),
    ('CUSTOM_TINT_BUNDLES', 7086, 7097),
    ('ACTIVE_TINT_IDX', 7099, 7099),
    ('SEXSTORE_LOAD_PENDING', 7103, 7103),
    ('pack_u32_pair', 7105, 7108),
    ('push_token_preset', 7113, 7124),
    ('resolve_scene_render_tokens', 7128, 7142),
    ('send_scene_render_tokens', 7145, 7164),
    ('pack_scene_settings_blob', 7176, 7192),
    ('unpack_scene_settings_blob', 7198, 7208),
    ('STORE_REPLY_STATUS_BIT', 7211, 7211),
    ('STORE_KV_OK', 7212, 7212),
    ('STORE_KV_NOT_FOUND', 7213, 7213),
    ('STORE_KV_FULL', 7214, 7214),
    ('STORE_KV_INVALID_KEY', 7215, 7215),
    ('STORE_KV_INVALID_VALUE', 7216, 7216),
    ('STORE_KV_DENIED', 7217, 7217),
    ('store_reply_is_status', 7219, 7220),
    ('store_reply_status', 7221, 7222),
    ('store_reply_is_value', 7223, 7224),
    ('handle_sexstore_get_reply', 7228, 7291),
    ('boot_load_scene_settings', 7296, 7310),
    ('cycle_scene_render_token_preset', 7313, 7342),
    ('cycle_prev_scene_render_token_preset', 7345, 7377),
    ('apply_custom_tint_bundle', 7379, 7389),
    ('cycle_custom_tint', 7391, 7403),
    ('handle_scene_settings_cmd', 7413, 7495),
    ('ShellViewMode', 7543, 7547),
    ('shell_view_mode', 7550, 7554),
    ('MAX_TABS_PER_FRAME', 8135, 8135),
    ('MAX_FRAMES', 8137, 8137),
    ('ShellTab', 8141, 8149),
    ('ShellFrame', 8156, 8175),
    ('FRAMES', 8199, 8199),
    ('FRAME_FLAG_MINIMIZED', 8346, 8346),
    ('FOCUSED_SURFACE_ID', 9094, 9094),
    ('ACTIVE_SCENE_IDX', 9096, 9096),
    ('ATLAS_MODE_ENABLED', 9124, 9124),
    ('SURFACE_200_W', 10439, 10439),
    ('SURFACE_200_H', 10440, 10440),
    ('SURFACE_201_W', 10444, 10444),
    ('SURFACE_201_H', 10445, 10445),
    ('SURFACE_202_W', 10449, 10449),
    ('SURFACE_202_H', 10450, 10450),
    ('LINEN_FRAME_ID', 12419, 12419),
    ('LINEN_BOOT_X', 12421, 12421),
    ('LINEN_BOOT_Y', 12422, 12422),
    ('LINEN_BOOT_W', 12423, 12423),
    ('LINEN_BOOT_H', 12424, 12424),
    ('QUIL_FRAME_ID', 12636, 12636),
    ('QUIL_BOOT_X', 12638, 12638),
    ('QUIL_BOOT_Y', 12639, 12639),
    ('QUIL_BOOT_W', 12640, 12640),
    ('QUIL_BOOT_H', 12641, 12641),
    ('MESH_FRAME_ID', 12866, 12866),
    ('MESH_BOOT_X', 12868, 12868),
    ('MESH_BOOT_Y', 12869, 12869),
    ('MESH_BOOT_W', 12870, 12870),
    ('MESH_BOOT_H', 12871, 12871),
    ('MESH_PLACEHOLDER_COLOR', 12874, 12874),
    ('MESH_LIST_HEADER_H', 12876, 12876),
    ('MESH_LIST_ROW_H', 12878, 12878),
    ('MESH_LIST_ROW_GAP', 12880, 12880),
    ('MESH_LIST_ROW_RECTS', 12882, 12882),
    ('MESH_SELECTED_ROW', 12886, 12886),
    ('COLLAR_FRAME_ID', 13067, 13067),
    ('COLLAR_BOOT_X', 13069, 13069),
    ('COLLAR_BOOT_Y', 13070, 13070),
    ('COLLAR_BOOT_W', 13071, 13071),
    ('COLLAR_BOOT_H', 13072, 13072),
    ('BELL_FRAME_ID', 13279, 13279),
    ('BELL_BOOT_X', 13281, 13281),
    ('BELL_BOOT_Y', 13282, 13282),
    ('BELL_BOOT_W', 13283, 13283),
    ('BELL_BOOT_H', 13284, 13284),
    ('BROWSER_FRAME_ID', 13287, 13287),
    ('BROWSER_BOOT_X', 13289, 13289),
    ('BROWSER_BOOT_Y', 13290, 13290),
    ('BROWSER_BOOT_W', 13291, 13291),
    ('BROWSER_BOOT_H', 13292, 13292),
    ('COMMAND_PALETTE_FRAME_ID', 13315, 13315),
    ('COMMAND_PALETTE_BOOT_X', 13317, 13317),
    ('COMMAND_PALETTE_BOOT_Y', 13318, 13318),
    ('COMMAND_PALETTE_BOOT_W', 13319, 13319),
    ('COMMAND_PALETTE_BOOT_H', 13320, 13320),
    ('SPINDLE_FRAME_ID', 13336, 13336),
    ('SPINDLE_BOOT_X', 13341, 13341),
    ('SPINDLE_BOOT_Y', 13342, 13342),
    ('SPINDLE_BOOT_W', 13343, 13343),
    ('SPINDLE_BOOT_H', 13344, 13344),
]

RUST_PROVIDER = r'''
macro_rules! serial_println { ($($arg:tt)*) => { println!($($arg)*) }; }
mod sex_pdx {
    /* PDX_CONSTANTS */
    pub fn pdx_call(slot: u64, operation: u64, a: u64, b: u64, c: u64) -> (u64, u64) {
        println!("call {} {} {} {} {}", slot, operation, a, b, c);
        (unsafe { super::HOST_CALL_STATUS }, a ^ b ^ c ^ operation)
    }
    pub fn pdx_listen_raw(slot: u64) -> super::HostMessage { unsafe { super::host_listen(slot) } }
    pub fn pdx_reply(target: u32, value: u64) -> u64 {
        println!("peer reply {} {}", target, value);
        0
    }
}
use sex_pdx::{pdx_listen_raw, pdx_reply, SLOT_QUIL, SLOT_SEXSTORE, OP_APPEARANCE_TOKENS};
/* TOKEN_CONSTANTS */
'''


def rust_source():
    sources = {}
    for name, expected in HASHES.items():
        source = subprocess.check_output(
            ['git', 'show', f'{REVISION}:servers/silk-shell/src/{name}'], cwd=HERE.parent)
        assert hashlib.sha256(source).hexdigest() == expected, f'changed reference {name}'
        sources[name] = source.decode()
    library = sources['lib.rs']
    assert library.count('#![no_std]') == 1
    library = library.replace('#![no_std]', '')
    lines = sources['main.rs'].splitlines(keepends=True)
    items = []
    for name, start, end in MAIN_ITEMS:
        item = ''.join(lines[start - 1:end])
        assert name in item, (name, start, end)
        items.append(item)
    constants = []
    for path, (expected, names) in DEPENDENCIES.items():
        source = subprocess.check_output(['git', 'show', f'{REVISION}:{path}'], cwd=HERE.parent)
        assert hashlib.sha256(source).hexdigest() == expected, f'changed reference {path}'
        selected = []
        for name in names:
            matches = re.findall(r'(?m)^pub const\s+' + name + r'\s*:[^;]+;', source.decode())
            assert len(matches) == 1, (path, name)
            selected.append(matches[0])
        constants.append('\n'.join(selected))
    provider = RUST_PROVIDER.replace('/* PDX_CONSTANTS */', constants[0]).replace('/* TOKEN_CONSTANTS */', constants[1])
    return library + provider + '\n' + '\n'.join(items) + (FIXTURES / 'oracle.rs').read_text()


def model_scenarios(emit):
    emit('registry')
    emit('geometry', 320, 240, 400, 300, 500, 320)
    emit('push', *range(8))
    emit('boot-load')
    for _ in range(7):
        emit('send')
    emit('settings')
    for command in range(10):
        for value in (0, 1, 3, 4, 255, 256, (1 << 64) - 1):
            emit('settings-command', command, value, 0xffff)
            emit('settings')
    for _ in range(40):
        for command in ('preset-next', 'preset-prev', 'tint-next'):
            emit(command)
            emit('settings')
    for index in range(5):
        emit('tint', index)
        emit('send')
    emit('get-reply', 0)
    for code in range(9):
        emit('get-reply', (1 << 63) | code)
    for preset in (0, 3, 4, 255):
        checksum = (0xac ^ 1 ^ preset ^ 7 ^ 15) & 0x7f
        emit('get-reply', 0x01ac | (preset << 16) | (7 << 24) | (15 << 32) | (checksum << 56))
        emit('settings')
    for atlas in (0, 1): emit('view', atlas)
    emit('reset')
    emit('geometry', 320, 240, 400, 300, 500, 320)
    emit('linen-static')
    for index in range(16): emit('put', index, index + 1)
    for identity in (0, 1, 6, 16, 999):
        emit('select', identity)
        emit('linen-render')
    names = (b'a\0b', 'αβ'.encode(), '𐀀\U0010ffff'.encode(), b'\xc2\x80', b'\xe0\xa0\x80',
             b'\xed\x9f\xbf', b'\xc0\x80', b'\xed\xa0\x80', b'\xf4\x90\x80\x80',
             b'\xf0\x80\x80\x80', b'\xe2\x82', b'\x80', b'\xff', b'z' * 24)
    for name in names:
        emit('name', 0, len(name), *name)
        emit('linen-render')
    for index in range(16): emit('qput', index, index + 10, 0, index % 8, index % 6, 201)
    emit('quil-render')
    for command in ('grant-next', 'grant-prev', 'grant-detail', 'collar-render',
                    'mesh-next', 'mesh-prev', 'mesh-detail', 'mesh-snapshot', 'rings'):
        emit(command)
    emit('grants')
    for index in (0, 1, 31, 255):
        emit('grant-selected', index)
        for command in ('grant-next', 'grant-prev', 'grant-detail'): emit(command)
    emit('reset')
    emit('geometry', 320, 240, 400, 300, 500, 320)
    for index in range(12): emit('message', 2, index + 1, index, 0, 0)
    emit('message', 0x202, 7, 10, 20, 3)
    emit('message', 1, 0, 0xffff, 0, 0)
    emit('sync')
    for _ in range(16): emit('message', 1, 0, 0, 0, 0)
    emit('remote')
    emit('snapshot')
    for _ in range(16): emit('message', 1, 0, 0, 0, 0)
    emit('paint')
    for slot in range(16):
        if slot > 5:
            emit('message', 1, 0, 0, 0, 0)
            continue
        length = (0, 1, 24, 31, 9, 8)[slot]
        packed = (slot + 1) | ((slot % 3) << 32) | (length << 40) | ((slot + 1) << 48)
        emit('message', 1, 0, packed, 0, 0)
        for offset in range(0, min(length, 24), 8):
            if slot == 4 and offset == 8: chunk = (1 << 64) - 1
            elif slot == 5: chunk = 0
            else: chunk = int.from_bytes(bytes((32 + slot, 0, 31, 127, 65, 66, 67, 68)), 'little')
            emit('message', 1, 0, chunk, 0, 0)
            if chunk in (0, (1 << 64) - 1): break
    emit('remote')
    emit('snapshot')
    emit('paint')
    emit('paint-fast')
    emit('reset')
    emit('geometry', 320, 240, 400, 300, 500, 320)
    emit('open', 999)
    emit('open', 1)
    emit('focus', 200)
    emit('open', 1)
    emit('grants')
    emit('peer', 4, 202, 1, 1, 1, 1, 1, 0)
    emit('frame', 0, 4, 0, 0, 1)
    for identity in (1, 2, 3, 4, 5, 6):
        emit('open', identity)
        emit('open', identity)
    for path in (1, 3, 6):
        emit('disk', 1, path)
        emit('open', 2)
    for object_id, buffer_id in ((2, 1), (2, 2), (999, 2), (2, 999)):
        emit('bell-link', object_id, buffer_id)
    for count in range(100):
        emit('mesh-add', count % 8, count + 100, count + 200)
        emit('bell-add', count, count + 1)
    emit('rings')
    for row in (0, 1, 6, 7, 255):
        emit('mesh-selected', row)
        emit('mesh-snapshot')
        for command in ('mesh-render', 'mesh-next', 'mesh-prev', 'mesh-detail', 'mesh-focus'): emit(command)
    emit('focus', 202)
    emit('mesh-detail')
    emit('mesh-graph')
    for flags in (0, 1, 2, 3):
        emit('frame', 0, 4, 0, flags, 1)
        emit('rings')
    for value in (0, 0x3f4041, 0xbfc0c1, 0xffffff, 0xffffffff): emit('highlight', value)
    for total in (0, 1, 31, 32, 33, (1 << 64) - 1):
        emit('ring-indices', total, (1 << 64) - 1, total, (1 << 64) - 1)
        emit('mesh-add', 1, 2, 3)
        emit('bell-add', 1, 2)
        emit('rings')
    emit('reset')
    emit('put', 0, 999)
    emit('focus', 200)
    emit('grant', 0, 200, 999, 1 << 6, 0)
    for index in range(16): emit('qput', index, 10 + index, 1, 0, 0, 0)
    emit('open', 999)
    emit('qput', 0, 1999, 1, 0, 0, 0)
    emit('open', 999)
    emit('qerase', 0)
    emit('open', 999)


def vectors(path, seeds, cases):
    count = 0
    with path.open('w') as output:
        def emit(*values):
            nonlocal count
            print(*values, file=output)
            count += 1
        emit('reset')
        emit('snapshot')
        model_scenarios(emit)
        for bits in range(256):
            emit('caps', bits)
            emit('ascii', bits)
            emit('preset', bits)
            for identity in (0, 1, 255, 256, 32767, 32768, 65534, 65535):
                emit('manifest', (1 << 64) - 1, 0, (identity << 8) | bits)
        for bit in range(64):
            emit('manifest', 200, 1, (1 << bit) | 3)
            emit('reply', 1 << bit)
        for kind in range(11):
            for state in range(30):
                emit('kind', kind, state)
        for identity in itertools.chain(range(220), ((1 << 32) - 1, (1 << 64) - 1)):
            emit('surface', identity)
        for x in (0, 1, 0x7fffffff, 0x80000000, 0xffffffff):
            for y in (0, 1, 0x7fffffff, 0x80000000, 0xffffffff):
                emit('compositor', x, y, 0xffffffff, 7)
        for w, h in ((0, 0), (1, 1), (4, 4), (80, 65)):
            for x, y, rw, rh in ((0, 0, 0, 0), (0, 0, 0xffffffff, 0xffffffff),
                                  (1, 1, 2, 3), (0xfffffffe, 0xfffffffe, 4, 4),
                                  (1, 1, 0xffffffff, 0xffffffff)):
                for panel in range(3):
                    emit('canvas', w, h, x, y, rw, rh, 0xfedcba98, panel)
        for preset, chrome, access in ((0, 0, 0), (255, 255, 255), (3, 7, 15), (128, 129, 130)):
            emit('pack', preset, chrome, access)
            checksum = (0xac ^ 0x01 ^ preset ^ chrome ^ access) & 0x7f
            blob = 0xac | (0x01 << 8) | (preset << 16) | (chrome << 24) | (access << 32) | (checksum << 56)
            emit('blob', blob)
            for bit in range(64):
                emit('blob', blob ^ (1 << bit))
        for caller in (0, 100, 200, 201, 202, 203, 204, 205, 299, 300, 301, (1 << 64) - 1):
            emit('reset')
            emit('grants')
            emit('focus', caller)
            for operation in range(11):
                for object_id in (0, 1, 999):
                    for buffer_id in (0, 1, 999):
                        emit('policy', operation, object_id, buffer_id)
            emit('audit-dump')
        for state in range(4):
            for subject in (0, 200, 300, 301):
                emit('reset')
                emit('focus', 300)
                for operation in range(11):
                    emit('grant', 0, subject, 1, 1 << operation, state)
                    emit('policy', operation, 1, 1)
                    emit('grant', 0, subject, 300, 1 << operation, state)
                    emit('policy', operation, 300, 1)
                emit('grant-dump')
                emit('audit-dump')
        emit('reset')
        emit('grants')
        for index in range(256):
            emit('grant-visible', index)
            emit('review', 300, 65535, index)
        for generation in (0, 1, 31, 32, 33, (1 << 64) - 2, (1 << 64) - 1):
            emit('reset')
            emit('grant-generation', generation)
            for bits in (0, 1, 2, 3):
                emit('autogrant', 300, 1, bits)
                emit('grant-dump')
            emit('audit-index', generation)
            emit('focus', 300)
            for operation in range(11):
                emit('policy', operation, 300, 1)
            emit('audit-dump')
        for seed in range(seeds):
            generator = random.Random(0x53494c4b + seed)
            emit('reset')
            for iteration in range(cases):
                emit('manifest', generator.getrandbits(64), generator.getrandbits(64),
                     generator.getrandbits(24 if iteration % 2 == 0 else 64))
                emit('pair', generator.getrandbits(32), generator.getrandbits(32))
                emit('pack', generator.randrange(256), generator.randrange(256), generator.randrange(256))
                emit('blob', generator.getrandbits(64))
                emit('tokens', generator.randrange(256), generator.randrange(3),
                     *(generator.getrandbits(32) if generator.randrange(2) else 0 for _ in range(8)))
                emit('canvas', generator.randrange(81), generator.randrange(66),
                     generator.randrange(100), generator.randrange(80),
                     generator.randrange(100), generator.randrange(80), generator.getrandbits(32), 0)
                slot = generator.randrange(16)
                if generator.randrange(3):
                    emit('put', slot, generator.choice((0, 1, 2, 6, 999, (1 << 64) - 1)))
                else:
                    emit('erase', slot)
                emit('select', generator.choice((0, 1, 2, 6, 999, (1 << 64) - 1)))
                emit(generator.choice(('first', 'next', 'prev', 'selected')))
                emit('lookup', generator.choice((0, 1, 2, 6, 999, (1 << 64) - 1)))
                emit('focus', generator.choice((0, 100, 200)))
                emit('detail')
                emit('close')
                emit('cohere')
                emit('snapshot')
            emit('clear')
            for command in ('snapshot', 'detail', 'selected', 'first', 'next', 'prev', 'close', 'cohere', 'snapshot'):
                emit(command)
    return count


def run(command, work, label, *, stdin=None, stdout=None, check=True, flags=()):
    log = work / (label + '.log')
    with log.open('wb') as diagnostics:
        with (stdin.open('rb') if stdin else open(os.devnull, 'rb')) as incoming:
            with (stdout.open('wb') if stdout else open(os.devnull, 'wb')) as outgoing:
                environment = {key: value for key, value in os.environ.items()
                               if not key.startswith(('SEXOS_', 'SILK_DE_'))}
                environment.update({flag: '1' for flag in flags})
                process = subprocess.run(command, cwd=HERE, stdin=incoming, stdout=outgoing,
                                         stderr=diagnostics, timeout=180, env=environment)
    if check and process.returncode:
        raise AssertionError(f'{label} failed ({process.returncode}):\n' + log.read_text(errors='replace')[-8000:])
    return process.returncode


def compare(reference, candidate, label):
    if filecmp.cmp(reference, candidate, shallow=False):
        return
    with reference.open('rb') as expected, candidate.open('rb') as actual:
        for index, (left, right) in enumerate(itertools.zip_longest(expected, actual), 1):
            if left != right:
                raise AssertionError(f'{label}, first difference at line {index}:\nRust: {left!r}\nCesta: {right!r}')
    raise AssertionError(label)


def mutate_function(text, function, target, replacement):
    pattern = r'(?m)^[\w *]+\b' + re.escape(function) + r'\s*\([^;{}]*\)\s*\{'
    matches = list(re.finditer(pattern, text))
    assert len(matches) == 1, (function, len(matches))
    start = matches[0].end() - 1
    end = start + 1
    depth = 1
    while depth:
        if text[end] == '{':
            depth += 1
        elif text[end] == '}':
            depth -= 1
        end += 1
    body = LayoutText(text[start:end])
    assert body.count(target) == 1, (function, target, body.count(target))
    return text[:start] + body.replace(target, replacement) + text[end:]


def isolated_scenarios(work):
    cases = []
    for code in range(9):
        cases.append((f'status-{code}', [('reset',), *(('get-reply', (1 << 63) | code) for _ in range(6)), ('settings',)]))
    cases.append(('corrupt-first', [('reset',), ('get-reply', 1), ('settings',)]))
    cases.append(('boot-failure', [('reset',), ('peer', 0, 0, 0, 0, 0, 0, 1, 5),
        ('boot-load',), ('settings',), ('peer', 0, 0, 0, 0, 0, 0, 1, 0), ('boot-load',), ('settings',)]))
    result = []
    commands = 0
    for name, rows in cases:
        path = work / (name + '.txt')
        path.write_text(''.join(' '.join(map(str, row)) + '\n' for row in rows))
        result.append((name, path))
        commands += len(rows)
    return result, commands


def check_variant(work, scenarios, oracle, exported, compiler, source, flags, include):
    reference = work / 'rust'
    run(['rustc', '--edition=2021', '-O', '--cap-lints=allow', str(oracle), '-o', str(reference)],
        work, 'compile-rust', flags=flags)
    expected = {}
    for name, inputs in scenarios:
        trace = work / ('rust-' + name + '.trace')
        run([str(reference)], work, 'run-rust-' + name, stdin=inputs, stdout=trace)
        expected[name] = trace

    def check_binary(binary, label):
        for name, inputs in scenarios:
            trace = work / (label + '-' + name + '.trace')
            run([str(binary)], work, 'run-' + label + '-' + name, stdin=inputs, stdout=trace)
            compare(expected[name], trace, label + '/' + name)
    common = ['-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Wformat=2', '-Werror',
              '-ffunction-sections', '-fdata-sections', '-I', str(include), '-I', str(HERE / 'generated/include'),
              *(f'-D{flag}=1' for flag in flags)]
    provider = str(FIXTURES / 'provider.c')
    for cc in ('gcc', 'clang'):
        for optimization in ('-O0', '-O2'):
            for authored in (True, False):
                label = cc + optimization + ('-authored' if authored else '-exported')
                binary = work / label
                command = [compiler, 'cc', '--cc', cc] if authored else [cc]
                translation = source if authored else exported
                run([*command, *common, optimization, provider, str(translation), '-Wl,--gc-sections', '-o', str(binary)],
                    work, 'compile-' + label)
                check_binary(binary, label)
        label = cc + '-sanitized'
        binary = work / label
        run([cc, *common, '-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
             provider, str(exported), '-Wl,--gc-sections', '-o', str(binary)], work, 'compile-' + label)
        check_binary(binary, label)
    mutations = (
        ('accept-reserved-bit', 'app_manifest_unpack', '0xFFFFFFFFFF000000', '0xFFFFFFFFFE000000'),
        ('accept-unknown-capability', 'app_capability_bits_validate', 'APP_CAP_KNOWN', 'UINT8_MAX'),
        ('wrong-project-color', 'linen_kind_color', '0x004080C0', '0x004080C1'),
        ('wrong-document-color', 'linen_kind_color', '0x0040C080', '0x0040C081'),
        ('wrong-text-color', 'quil_buffer_kind_color', '0x00808080', '0x00808081'),
        ('checksum-status-bit', 'pack_scene_settings_blob', '0x7f', '0xff'),
        ('short-checksum', 'unpack_scene_settings_blob', '0x7f', '0x3f'),
        ('accept-revoked-grant', 'collar_check_operation',
         'grant->state != COLLAR_GRANT_ACTIVE', 'grant->state > COLLAR_GRANT_TOMBSTONED'),
        ('accept-wrong-subject', 'collar_check_operation', 'grant->subject_id != caller', 'false'),
        ('count-revoked-grants', 'collar_grant_count', 'COLLAR_GRANT_ACTIVE', 'COLLAR_GRANT_REVOKED'),
        ('wrong-audit-reason', 'record_collar_audit', '.reason = reason', '.reason = 0'),
        ('reverse-mesh-order', 'mesh_for_each_fact', 'MESH_FACT_RING_CAP - i', 'MESH_FACT_RING_CAP + i'),
        ('reverse-bell-order', 'bell_for_each_event', 'BELL_RING_CAP - i', 'BELL_RING_CAP + i'),
        ('wrong-reused-buffer-id', 'open_linen_object_in_quil', 'linked_id = buffer->buffer_id', 'linked_id = dynamic_id'),
        ('ignore-buffer-id-collision', 'open_linen_object_in_quil', 'buffer->buffer_id == dynamic_id', 'buffer->buffer_id == UINT64_MAX'),
        ('accept-overlong-utf8', 'model_valid_utf8', 'first >= 0xc2', 'first >= 0xc0'),
        ('accept-surrogate-utf8', 'model_valid_utf8', 'second >= 0xa0', 'second >= 0xe0'),
        ('drop-name-bytes', 'model_linen_row', 'write_bytes(object->name, object->name_len)', 'write_bytes(object->name, 0)'),
        ('draw-eighth-mesh-row', 'model_mesh_render_row', 'state->rows >= MESH_LIST_ROW_RECTS', 'state->rows > MESH_LIST_ROW_RECTS'),
    )
    text = LayoutText(exported.read_text())
    for name, function, target, replacement in mutations:
        mutant = work / (name + '.c')
        mutant.write_text(mutate_function(text, function, target, replacement))
        binary = work / name
        run(['gcc', *common, '-O2', provider, str(mutant), '-Wl,--gc-sections', '-o', str(binary)],
            work, 'compile-' + name)
        trace = work / (name + '.trace')
        status = run([str(binary)], work, 'run-' + name, stdin=scenarios[0][1], stdout=trace, check=False)
        assert status != 0 or not filecmp.cmp(expected[scenarios[0][0]], trace, shallow=False), f'mutation survived: {name}'
    lines = 0
    for path in expected.values():
        with path.open('rb') as stream:
            lines += sum(1 for _ in stream)
    return lines, len(mutations)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--source', type=Path, help='Explicit candidate for the historical Rust cohort; default replays the committed original port.')
    parser.add_argument('--interface', type=Path, help='Interface matching an explicit historical candidate.')
    parser.add_argument('--seeds', type=int, default=4)
    parser.add_argument('--cases', type=int, default=256)
    args = parser.parse_args()
    compiler = str(Path(shutil.which(str(args.cesta)) or args.cesta).resolve())
    watched = (Path(compiler), *((args.source.resolve(),)if args.source else()), HERE / 'generated/include/silk_shell.h')
    fingerprints = {path: hashlib.sha256(path.read_bytes()).hexdigest() for path in watched}
    with tempfile.TemporaryDirectory(prefix='silk-shell-models-') as temporary:
        work = Path(temporary)
        include=work/'include';include.mkdir()
        if args.source is None:
            commit='b5ee992f5fe01ed67b5e4acc60c7dd30a768fa4c'
            args.source=work/'historical-models.cesta'
            args.source.write_bytes(subprocess.check_output(['git','show',commit+':cesta/src/silk_shell_models.cesta'],cwd=HERE.parent))
            interface_source=work/'historical-interface.cesta'
            interface_source.write_bytes(subprocess.check_output(['git','show',commit+':cesta/src/silk_shell_interface.cesta'],cwd=HERE.parent))
            (include/'silk_shell.h').write_bytes(subprocess.check_output(['git','show',commit+':cesta/generated/include/silk_shell.h'],cwd=HERE.parent))
        else:
            assert args.interface,'an explicit historical candidate requires its matching --interface'
            interface_source=None
            (include/'silk_shell.h').write_bytes(args.interface.read_bytes())
        candidate_hash=hashlib.sha256(args.source.read_bytes()).hexdigest()
        inputs = work / 'vectors.txt'
        count = vectors(inputs, args.seeds, args.cases)
        isolated, isolated_commands = isolated_scenarios(work)
        scenarios = [('main', inputs), *isolated]
        count += isolated_commands
        oracle = work / 'oracle.rs'
        oracle.write_text(rust_source())
        exported = work / 'models.c'
        run([compiler, 'export-c', str(args.source.resolve()), '--'], work, 'export', stdout=exported)
        interface = work / 'silk_shell.h'
        if interface_source:
            run([compiler, 'export-c', str(interface_source), '--'], work, 'export-interface', stdout=interface)
            assert interface.read_bytes().rstrip(b'\n')+b'\n'==(include/'silk_shell.h').read_bytes(),'historical interface export differs'
        const_write = work / 'const_write.c'
        const_write.write_text('#include "silk_shell.h"\nvoid invalid(void) { PRESET_NAMES[0] = "changed"; }\n')
        for cc in ('gcc', 'clang'):
            label = cc + '-const-write'
            status = run([cc, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                          '-I', str(include), '-I', str(HERE / 'generated/include'), '-fsyntax-only', str(const_write)], work, label, check=False)
            assert status != 0 and 'PRESET_NAMES' in (work / (label + '.log')).read_text(), 'writable preset table'
        lines, mutations = 0, 0
        for name, flags in (('default', ()), ('enforce', ('SEXOS_COLLAR_ENFORCE_PROOF',))):
            variant = work / name
            variant.mkdir()
            checked_lines, checked_mutations = check_variant(variant, scenarios, oracle, exported,
                                                             compiler, args.source.resolve(), flags, include)
            lines += checked_lines
            mutations += checked_mutations
        assert all(hashlib.sha256(path.read_bytes()).hexdigest() == digest for path, digest in fingerprints.items()), 'inputs changed during verification'
        print(f'silk-shell models: {count} commands in {len(scenarios)} scenarios, {lines} exact Rust trace lines, '
              f'default/enforce, GCC/Clang authored/exported O0/O2 and sanitizers, {mutations} mutations detected, '
              f'2 const-write refusals, cesta={fingerprints[Path(compiler)][:16]}, '
              f'historical-models={candidate_hash[:16]}')


if __name__ == '__main__':
    main()
