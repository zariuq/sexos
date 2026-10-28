use std::io::{self, Read};

#[derive(Clone, Copy)]
struct HostMessage { type_id: u64, caller_pd: u32, arg0: u64, arg1: u64, arg2: u64 }
static mut HOST_MESSAGES: [HostMessage; 1024] = [HostMessage {type_id: 0, caller_pd: 0, arg0: 0, arg1: 0, arg2: 0}; 1024];
static mut HOST_MESSAGE_COUNT: usize = 0;
static mut HOST_MESSAGE_CURSOR: usize = 0;
static mut HOST_CALL_STATUS: u64 = 0;
static mut HOST_DRAW_ACCEPTED: bool = true;
static mut HOST_ALIVE: bool = false;
static mut HOST_BELL_VISIBLE: bool = false;
static mut HOST_OPEN_QUIL: bool = false;
static mut HOST_SURFACE_PRESENT: bool = false;
static mut HOST_SURFACE: u64 = 0;
static mut HOST_FRAME: u32 = 0;

unsafe fn host_listen(slot: u64) -> HostMessage {
    assert!(HOST_MESSAGE_CURSOR < HOST_MESSAGE_COUNT);
    let message = HOST_MESSAGES[HOST_MESSAGE_CURSOR];
    HOST_MESSAGE_CURSOR += 1;
    println!("listen {} {} {} {} {} {}", slot, message.type_id, message.caller_pd,
             message.arg0, message.arg1, message.arg2);
    message
}

unsafe fn shell_draw_text(surface: u64, text: &[u8], color: u64) -> (usize, bool) {
    print!("draw {} {} {} ", surface, color, text.len());
    for byte in text { print!("{:02x}", byte); }
    println!();
    (text.len(), HOST_DRAW_ACCEPTED)
}
unsafe fn active_surface_for_frame(frame: u32) -> Option<u64> {
    if HOST_SURFACE_PRESENT && frame == HOST_FRAME { Some(HOST_SURFACE) } else { None }
}
unsafe fn frame_for_surface(surface: u64) -> Option<u32> {
    if HOST_SURFACE_PRESENT && surface == HOST_SURFACE { Some(HOST_FRAME) } else { None }
}
unsafe fn surface_is_alive(surface: u64) -> bool { HOST_ALIVE && surface == HOST_SURFACE }
unsafe fn bell_is_visible_in_active_scene() -> bool { HOST_BELL_VISIBLE }
unsafe fn bell_render_event_list() { println!("peer bell-render"); }
unsafe fn open_linen_in_active_scene() -> bool { println!("peer open-linen"); true }
unsafe fn open_quil_in_active_scene() -> bool { println!("peer open-quil"); HOST_OPEN_QUIL }
unsafe fn toggle_top_bar_for_active_frame() -> bool { println!("peer toggle-topbar"); true }
unsafe fn handle_hid_event(event: u64, a: u64, b: u64) { println!("peer hid {} {} {}", event, a, b); }

fn number(words: &mut std::str::SplitWhitespace<'_>) -> u64 {
    words.next().expect("missing number").parse().expect("invalid number")
}

unsafe fn snapshot() {
    println!("selection {} {} {} {} {} {}", SELECTED_LINEN_OBJECT_ID,
             linen_selected_index(), linen_object_count(), quil_buffer_count(),
             LINEN_OBJECT_DETAIL_OPEN as u8, linen_selected_object_accent());
    for (index, slot) in LINEN_OBJECTS.iter().enumerate() {
        if let Some(object) = slot {
            println!("object {} {} {} {} {}", index, object.object_id,
                     object.kind as u8, object.state as u8, object.linked_surface_id);
        }
    }
}

unsafe fn reset() {
    LINEN_OBJECTS = [None; LINEN_MAX_OBJECTS];
    QUIL_BUFFERS = [None; QUIL_MAX_BUFFERS];
    SELECTED_LINEN_OBJECT_ID = 0;
    FOCUSED_SURFACE_ID = 0;
    LINEN_OBJECT_DETAIL_OPEN = false;
    SCENE_APPEARANCE_STATE = DEFAULT_SCENE_APPEARANCE;
    COLLAR_GRANTS = [None; COLLAR_GRANT_CAP];
    COLLAR_GRANT_GENERATION = 1;
    COLLAR_AUDIT_EVENTS = [None; COLLAR_AUDIT_CAP];
    COLLAR_AUDIT_WRITE_INDEX = 0;
    LINEN_REMOTE_FETCHED = false;
    LINEN_REMOTE_REAL = false;
    LINEN_UI_SELECTED = 0;
    COLLAR_SELECTED_GRANT_IDX = 0;
    MESH_FACTS = [None; MESH_FACT_RING_CAP];
    MESH_FACT_WRITE_INDEX = 0;
    MESH_FACT_SEQUENCE = 0;
    BELL_EVENTS = [None; BELL_RING_CAP];
    BELL_RING_WRITE_INDEX = 0;
    BELL_EVENT_SEQUENCE = 0;
    ACTIVE_TINT_IDX = 0;
    SEXSTORE_LOAD_PENDING = false;
    MESH_SELECTED_ROW = 0;
    FRAMES = [None; MAX_FRAMES];
    ACTIVE_SCENE_IDX = 0;
    ATLAS_MODE_ENABLED = false;
    SURFACE_200_W = 0;
    SURFACE_200_H = 0;
    SURFACE_201_W = 0;
    SURFACE_201_H = 0;
    SURFACE_202_W = 0;
    SURFACE_202_H = 0;
    HOST_MESSAGE_COUNT = 0;
    HOST_MESSAGE_CURSOR = 0;
    HOST_CALL_STATUS = 0;
    HOST_DRAW_ACCEPTED = true;
    HOST_ALIVE = false;
    HOST_BELL_VISIBLE = false;
    HOST_OPEN_QUIL = false;
    HOST_SURFACE_PRESENT = false;
    HOST_SURFACE = 0;
    HOST_FRAME = 0;
}

fn main() {
    let mut input = String::new();
    io::stdin().read_to_string(&mut input).unwrap();
    let mut words = input.split_whitespace();
    while let Some(command) = words.next() {
        unsafe {
            match command {
                "caps" => {
                    let bits = number(&mut words) as u8;
                    let unchecked = AppCapabilityBits(bits);
                    println!("caps {} {} {} {} {}", AppCapabilityBits::validate(bits).is_ok() as u8,
                             unchecked.bits(), unchecked.has_bell() as u8,
                             unchecked.has_sexfiles() as u8, unchecked.describe());
                }
                "manifest" => {
                    let surface = number(&mut words);
                    let title = number(&mut words);
                    let word = number(&mut words);
                    match AppManifest::unpack(surface, title, word) {
                        Ok(result) => {
                            let (a, b, c) = result.pack();
                            println!("manifest 1 {} {} {} {} {} {}", a, b, c,
                                     result.app_id, result.capabilities.bits(), result.has_identity() as u8);
                        }
                        Err(()) => println!("manifest 0"),
                    }
                }
                "canvas" => {
                    let w = number(&mut words) as u32;
                    let h = number(&mut words) as u32;
                    let x = number(&mut words) as u32;
                    let y = number(&mut words) as u32;
                    let rw = number(&mut words) as u32;
                    let rh = number(&mut words) as u32;
                    let color = number(&mut words) as u32;
                    let panel = number(&mut words);
                    let mut buffer = [0x531ba92eu32; 5202];
                    assert!(w <= 80 && h <= 65);
                    let mut canvas = Canvas::new(buffer.as_mut_ptr().add(1), w, h);
                    canvas.fill_rect(x, y, rw, rh, color);
                    if panel != 0 {
                        canvas.draw_panel(&ShellState {panel_window_id: 0, launcher_window_id: 0,
                            bg_color: 0, is_launcher_open: panel == 2,
                            current_mouse_x: 0, current_mouse_y: 0});
                    }
                    let mut hash = 0xcbf29ce484222325u64;
                    for pixel in buffer {
                        hash = (hash ^ pixel as u64).wrapping_mul(0x100000001b3);
                    }
                    println!("canvas {}", hash);
                }
                "compositor" => {
                    let x = number(&mut words) as i32;
                    let y = number(&mut words) as i32;
                    let w = number(&mut words) as u32;
                    let h = number(&mut words) as u32;
                    let client = PdxCompositorClient;
                    let result = client.create_window(x, y, w, h);
                    println!("window {}", result);
                    client.set_bg(w);
                    client.render_bar(result);
                }
                "reset" => {
                    reset();
                    linen_object_table_init();
                    quil_buffer_table_init();
                }
                "clear" => reset(),
                "erase" => {
                    let index = number(&mut words) as usize;
                    LINEN_OBJECTS[index] = None;
                }
                "put" => {
                    let index = number(&mut words) as usize;
                    let id = number(&mut words);
                    let mut object = LINEN_SEED_OBJECTS[index % 6];
                    object.object_id = id;
                    LINEN_OBJECTS[index] = Some(object);
                }
                "select" => SELECTED_LINEN_OBJECT_ID = number(&mut words),
                "focus" => FOCUSED_SURFACE_ID = number(&mut words),
                "first" => linen_select_first_valid_object(),
                "next" => linen_select_next_object(),
                "prev" => linen_select_prev_object(),
                "selected" => {
                    let id = linen_selected_object_id();
                    println!("selected {}", id);
                }
                "detail" => linen_object_detail_open(),
                "close" => linen_object_detail_close(),
                "cohere" => linen_quil_seed_coherence_init(),
                "snapshot" => snapshot(),
                "lookup" => {
                    let id = number(&mut words);
                    let object = linen_object_by_id(id);
                    let buffer = quil_buffer_by_id(id);
                    println!("lookup {} {} {} {}", object.is_some() as u8,
                             object.map(|v| v.object_id).unwrap_or(0), buffer.is_some() as u8,
                             buffer.map(|v| v.buffer_id).unwrap_or(0));
                }
                "kind" => {
                    let kind = number(&mut words) as u8;
                    let state = number(&mut words) as u8;
                    assert!(kind < 11 && state < 30);
                    let object_kind: LinenObjectKind = core::mem::transmute(kind);
                    let object_state: LinenObjectState = core::mem::transmute(state % 5);
                    let buffer_kind: QuilBufferKind = core::mem::transmute(kind % 8);
                    let buffer_state: QuilBufferState = core::mem::transmute(state % 6);
                    println!("linen {} {} {}", linen_object_kind_name(object_kind),
                             linen_object_state_name(object_state), linen_kind_color(object_kind));
                    println!("quil {} {} {}", quil_buffer_kind_name(buffer_kind),
                             quil_buffer_state_name(buffer_state), quil_buffer_kind_color(buffer_kind));
                }
                "ascii" => println!("ascii {}", sanitize_ascii(number(&mut words) as u8)),
                "preset" => println!("preset {}", get_preset_name(number(&mut words) as u8)),
                "tokens" => {
                    SCENE_APPEARANCE_STATE.preset_idx = number(&mut words) as u8;
                    SCENE_APPEARANCE_STATE.use_custom_colors = number(&mut words) as u8;
                    for index in 0..8 {
                        SCENE_APPEARANCE_STATE.custom_colors[index] = number(&mut words) as u32;
                    }
                    print!("tokens");
                    for token in resolve_scene_render_tokens() {
                        print!(" {}", token);
                    }
                    println!();
                }
                "pack" => {
                    let preset = number(&mut words) as u8;
                    let chrome = number(&mut words) as u8;
                    let access = number(&mut words) as u8;
                    println!("pack {}", pack_scene_settings_blob(preset, chrome, access));
                }
                "blob" => {
                    match unpack_scene_settings_blob(number(&mut words)) {
                        Some((preset, chrome, access)) => println!("blob 1 {} {} {}", preset, chrome, access),
                        None => println!("blob 0 0 0 0"),
                    }
                }
                "reply" => {
                    let value = number(&mut words);
                    println!("reply {} {} {}", store_reply_is_status(value) as u8,
                             store_reply_status(value), store_reply_is_value(value) as u8);
                }
                "pair" => {
                    let low = number(&mut words) as u32;
                    let high = number(&mut words) as u32;
                    println!("pair {}", pack_u32_pair(low, high));
                }
                "surface" => {
                    let id = number(&mut words);
                    println!("surface {} {}", app_surface_spec(id).map(|s| s.name).unwrap_or("none"),
                             app_surface_spec_by_frame(id as u32).map(|s| s.name).unwrap_or("none"));
                }
                "grants" => collar_init_grants(),
                "grant" => {
                    let index = number(&mut words) as usize;
                    let subject = number(&mut words);
                    let object = number(&mut words);
                    let mask = number(&mut words);
                    let state = number(&mut words) as u8;
                    assert!(index < COLLAR_GRANT_CAP && state < 4);
                    COLLAR_GRANTS[index] = Some(CollarGrant {
                        grant_id: index as u64 + 1, subject_id: subject, object_id: object,
                        operation_mask: mask, generation: index as u64 + 1,
                        state: core::mem::transmute(state),
                    });
                }
                "grant-generation" => COLLAR_GRANT_GENERATION = number(&mut words),
                "audit-index" => COLLAR_AUDIT_WRITE_INDEX = number(&mut words),
                "autogrant" | "review" => {
                    let surface_id = number(&mut words);
                    let app_id = number(&mut words) as u16;
                    let capabilities = AppCapabilityBits(number(&mut words) as u8);
                    let manifest = AppManifest {surface_id, title_id: 0, app_id, capabilities};
                    if command == "autogrant" {
                        collar_auto_grant_from_manifest(&manifest);
                    } else {
                        let review = collar_review_manifest(&manifest);
                        println!("review {} {} {}", review.allowed as u8, review.granted_caps, review.denied_caps);
                    }
                }
                "policy" => {
                    let operation = number(&mut words) as u8;
                    let object = number(&mut words);
                    let buffer = number(&mut words);
                    assert!(operation < 11);
                    println!("policy {}", collar_check_operation(core::mem::transmute(operation), object, buffer) as u8);
                }
                "grant-visible" => {
                    let grant = collar_grant_at_visible_index(number(&mut words) as u8);
                    println!("visible {} {}", grant.is_some() as u8, grant.map(|g| g.grant_id).unwrap_or(0));
                }
                "grant-dump" => {
                    println!("grants {} {}", COLLAR_GRANT_GENERATION, collar_grant_count());
                    for (index, slot) in COLLAR_GRANTS.iter().enumerate() {
                        if let Some(grant) = slot {
                            println!("grant {} {} {} {} {} {} {}", index, grant.grant_id, grant.subject_id,
                                     grant.object_id, grant.operation_mask, grant.generation, grant.state as u8);
                        }
                    }
                }
                "audit-dump" => {
                    println!("audits {}", COLLAR_AUDIT_WRITE_INDEX);
                    for (index, slot) in COLLAR_AUDIT_EVENTS.iter().enumerate() {
                        if let Some(event) = slot {
                            println!("audit {} {} {} {} {} {} {} {}", index, event.event_id,
                                     event.operation as u8, event.object_id, event.subject_id,
                                     event.decision as u8, event.grant_ref, event.reason);
                        }
                    }
                }
                "geometry" => {
                    SURFACE_200_W = number(&mut words) as u32; SURFACE_200_H = number(&mut words) as u32;
                    SURFACE_201_W = number(&mut words) as u32; SURFACE_201_H = number(&mut words) as u32;
                    SURFACE_202_W = number(&mut words) as u32; SURFACE_202_H = number(&mut words) as u32;
                }
                "peer" => {
                    HOST_FRAME = number(&mut words) as u32; HOST_SURFACE = number(&mut words);
                    HOST_SURFACE_PRESENT = number(&mut words) != 0; HOST_ALIVE = number(&mut words) != 0;
                    HOST_BELL_VISIBLE = number(&mut words) != 0; HOST_OPEN_QUIL = number(&mut words) != 0;
                    HOST_DRAW_ACCEPTED = number(&mut words) != 0; HOST_CALL_STATUS = number(&mut words);
                }
                "frame" => {
                    let index = number(&mut words) as usize; let frame_id = number(&mut words) as u32;
                    let scene_id = number(&mut words) as u8; let flags = number(&mut words) as u32;
                    let present = number(&mut words) != 0;
                    FRAMES[index] = if present {Some(ShellFrame {frame_id, scene_id, flags,
                        active_tab: 0, tab_count: 0, tabs: [None; MAX_TABS_PER_FRAME as usize],
                        normal_x: 0, normal_y: 0, normal_w: 0, normal_h: 0})} else {None};
                }
                "scene" => ACTIVE_SCENE_IDX = number(&mut words) as u8,
                "name" => {
                    let index = number(&mut words) as usize; let length = number(&mut words) as usize;
                    assert!(length <= 24);
                    let object = LINEN_OBJECTS[index].as_mut().unwrap();
                    object.name_len = length as u8; object.name = [0; 24];
                    for index in 0..length { object.name[index] = number(&mut words) as u8; }
                }
                "disk" => {
                    let index = number(&mut words) as usize;
                    LINEN_OBJECTS[index].as_mut().unwrap().disk_path = number(&mut words) as u8;
                }
                "qput" => {
                    let index = number(&mut words) as usize; let buffer_id = number(&mut words);
                    let linen_object_ref = number(&mut words); let kind = number(&mut words) as u8;
                    let state = number(&mut words) as u8; let linked_surface_id = number(&mut words);
                    assert!(kind < 8 && state < 6);
                    QUIL_BUFFERS[index] = Some(QuilBuffer {buffer_id, linen_object_ref,
                        kind: core::mem::transmute(kind), state: core::mem::transmute(state),
                        linked_surface_id, project_id: 0, grant_ref: 0, flags: 0, display_name: "fixture"});
                }
                "qerase" => QUIL_BUFFERS[number(&mut words) as usize] = None,
                "message" => {
                    let type_id = number(&mut words); let caller_pd = number(&mut words) as u32;
                    let arg0 = number(&mut words); let arg1 = number(&mut words); let arg2 = number(&mut words);
                    assert!(HOST_MESSAGE_COUNT < 1024);
                    HOST_MESSAGES[HOST_MESSAGE_COUNT] = HostMessage {type_id, caller_pd, arg0, arg1, arg2};
                    HOST_MESSAGE_COUNT += 1;
                }
                "sync" => println!("sync {}", linen_sync_reply()),
                "remote" => linen_fetch_remote_snapshot(),
                "paint" => linen_paint_surface(),
                "paint-fast" => linen_paint_surface_fast(),
                "linen-render" => linen_render_object_list(),
                "linen-static" => linen_render_static_ui(),
                "quil-render" => quil_render_buffer_list(),
                "mesh-render" => mesh_render_fact_list(),
                "mesh-graph" => mesh_render_pd_graph(),
                "collar-render" => collar_render_grants_text(),
                "grant-next" => collar_select_next_grant(),
                "grant-prev" => collar_select_prev_grant(),
                "grant-detail" => println!("grant-detail {}", collar_emit_selected_grant_detail() as u8),
                "grant-selected" => COLLAR_SELECTED_GRANT_IDX = number(&mut words) as u8,
                "mesh-selected" => MESH_SELECTED_ROW = number(&mut words) as u8,
                "mesh-next" => mesh_select_next_row(),
                "mesh-prev" => mesh_select_prev_row(),
                "mesh-detail" => println!("mesh-detail {}", mesh_emit_selected_fact_detail_proof() as u8),
                "mesh-snapshot" => {
                    let fact = mesh_selected_fact_snapshot();
                    println!("mesh-snapshot {} {}", fact.is_some() as u8, fact.map(|f| f.fact_id).unwrap_or(0));
                }
                "mesh-focus" => { if let Some(fact) = mesh_selected_fact_snapshot() {mesh_focus_linen_at_selected_fact(&fact);} }
                "mesh-link" => mesh_emit_linen_quil_links(),
                "mesh-add" => {
                    let subject = number(&mut words); let object = number(&mut words); let reference = number(&mut words);
                    mesh_record_fact(MeshFactKind::ObjectLinkedToBuffer, subject, object, reference);
                }
                "bell-add" | "bell-link" => {
                    let object = number(&mut words); let buffer = number(&mut words);
                    if command == "bell-add" {bell_record_event(object, buffer);}
                    else {bell_emit_object_link_event(object, buffer);}
                }
                "ring-indices" => {
                    MESH_FACT_WRITE_INDEX = number(&mut words); MESH_FACT_SEQUENCE = number(&mut words);
                    BELL_RING_WRITE_INDEX = number(&mut words); BELL_EVENT_SEQUENCE = number(&mut words);
                }
                "rings" => {
                    println!("rings {} {} {} {}", mesh_fact_count(), bell_ring_count(),
                             mesh_visible_fact_count(), mesh_is_visible_in_active_scene() as u8);
                    mesh_for_each_fact(|fact| println!("mesh-row {} {} {} {} {} {} {}", fact.fact_id,
                        fact.kind as u8, fact.subject_id, fact.object_id, fact.ref_id, fact.sequence,
                        mesh_fact_row_color(fact)));
                    bell_for_each_event(|event| println!("bell-row {} {} {} {} {}", event.event_id,
                        event.kind as u8, event.object_id, event.buffer_id, event.sequence));
                }
                "highlight" => println!("highlight {}", mesh_selected_row_highlight(number(&mut words) as u32)),
                "open" => println!("opened {}", open_linen_object_in_quil(number(&mut words)) as u8),
                "registry" => app_surface_registry_validate(),
                "push" => {
                    let mut tokens = [0; 8];
                    for index in 0..8 { tokens[index] = number(&mut words) as u32; }
                    push_token_preset(&tokens);
                }
                "send" => send_scene_render_tokens(),
                "get-reply" => handle_sexstore_get_reply(number(&mut words)),
                "boot-load" => boot_load_scene_settings(),
                "preset-next" => cycle_scene_render_token_preset(),
                "preset-prev" => cycle_prev_scene_render_token_preset(),
                "tint-next" => cycle_custom_tint(),
                "tint" => apply_custom_tint_bundle(number(&mut words) as usize),
                "settings-command" => {
                    let command = number(&mut words); let value = number(&mut words); let flags = number(&mut words);
                    handle_scene_settings_cmd(command, value, flags);
                }
                "settings" => {
                    print!("settings {} {} {} {} {} {}", SCENE_APPEARANCE_STATE.preset_idx,
                        SCENE_APPEARANCE_STATE.use_custom_colors, SCENE_APPEARANCE_STATE.chrome_flags,
                        SCENE_APPEARANCE_STATE.accessibility_flags, ACTIVE_TINT_IDX, SEXSTORE_LOAD_PENDING as u8);
                    for color in SCENE_APPEARANCE_STATE.custom_colors { print!(" {}", color); }
                    println!();
                }
                "view" => {
                    ATLAS_MODE_ENABLED = number(&mut words) != 0;
                    println!("view {}", shell_view_mode() as u8);
                }
                _ => panic!("unknown command {}", command),
            }
        }
    }
}
