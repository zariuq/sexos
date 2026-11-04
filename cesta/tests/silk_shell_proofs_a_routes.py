"""Current Proofs-A route/status drivers against their pinned Rust bodies."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

from silk_shell_input_differential import item_end
from silk_shell_models_differential import mutate_function

HERE = Path(__file__).resolve().parent.parent


def run(command, data=None, env=None, accepted=True):
    result = subprocess.run(list(map(str, command)), cwd=HERE, input=data,
                            capture_output=True, env=env)
    assert (result.returncode == 0) == accepted, (command, result.returncode,
                                                   result.stdout[-2000:], result.stderr[-6000:])
    return result.stdout


def extract(inventory):
    source = run(['git', 'show', inventory['pin'] + ':servers/silk-shell/src/main.rs']).decode()
    bodies = []
    for row in inventory['functions']:
        match = re.search(r'(?m)^(?:unsafe )?fn ' + re.escape(row['name']) + r'\b', source)
        assert match, row['name']
        body = source[match.start():item_end(source, match.start())]
        assert hashlib.sha256(body.encode()).hexdigest() == row['source_sha256']
        bodies.append(body)
    match = re.search(r'(?m)^unsafe fn bell_send_app_event\b', source); assert match
    return bodies, source[match.start():item_end(source, match.start())]


def rust_source(inventory, bodies, bell, enabled):
    rows = inventory['functions']
    definitions = '\n'.join('static mut ' + row['done'] + ':bool=false;' for row in rows)
    constants = '\n'.join('const ' + ({
        'maybe_run_linen_keyboard_route_proof': 'LINEN_KEYBOARD_NAV_PROOF_ENABLED',
        'maybe_run_physical_keyboard_to_quil_focus_proof': 'PHYSICAL_KEYBOARD_TO_QUIL_PROOF_ENABLED',
        'maybe_run_bell_app_event_integration_proof': 'BELL_APP_EVENT_INTEGRATION_PROOF_ENABLED',
        'maybe_run_bell_workflow_event_proof': 'BELL_WORKFLOW_EVENT_PROOF_ENABLED',
        'maybe_run_window_workflow_v2_proof': 'WINDOW_WORKFLOW_V2_PROOF_ENABLED',
        'maybe_run_bell_keyboard_detail_proof': 'BELL_KEYBOARD_DETAIL_PROOF_ENABLED',
        'maybe_run_bell_detail_seed_proof': 'BELL_DETAIL_SEED_PROOF_ENABLED',
        'maybe_run_bell_system_events_proof': 'BELL_SYSTEM_EVENTS_PROOF_ENABLED',
        'maybe_run_atlas_scene_keyboard_proof': 'ATLAS_SCENE_KEYBOARD_PROOF_ENABLED',
        'maybe_run_atlas_theme_visual_proof': 'ATLAS_THEME_VISUAL_PROOF_ENABLED',
        'maybe_run_atlas_theme_presets_proof': 'ATLAS_THEME_PRESETS_PROOF_ENABLED',
        'maybe_run_silkbar_keyboard_status_proof': 'SILKBAR_KEYBOARD_STATUS_PROOF_ENABLED',
        'maybe_run_silkbar_palette_status_proof': 'SILKBAR_PALETTE_STATUS_PROOF_ENABLED',
        'maybe_run_silkbar_phase2_shell_proof': 'SILKBAR_PHASE2_SHELL_PROOF_ENABLED',
        'maybe_run_frame_lights_stub_proof': 'FRAME_LIGHTS_STUB_PROOF_ENABLED',
        'maybe_run_frame_lights_keyboard_proof': 'FRAME_LIGHTS_KEYBOARD_PROOF_ENABLED',
        'maybe_run_silk_focus_reject_proof': 'SILK_FOCUS_REJECT_PROOF_ENABLED',
        'maybe_run_silk_drag_reject_proof': 'SILK_DRAG_REJECT_PROOF_ENABLED',
        'maybe_run_silk_window_move_proof': 'SILK_WINDOW_MOVE_PROOF_ENABLED',
        'maybe_run_quil_visible_typing_e2e_proof': 'QUIL_VISIBLE_TYPING_E2E_PROOF_ENABLED',
        'maybe_run_linen_object_detail_proof': 'LINEN_OBJECT_DETAIL_PROOF_ENABLED',
        'maybe_run_linen_nonblocking_open_proof': 'LINEN_NONBLOCKING_OPEN_PROOF_ENABLED',
        'maybe_run_collar_keyboard_grants_proof': 'COLLAR_KEYBOARD_GRANTS_PROOF_ENABLED'}[row['name']]) + ':bool=' +
        ('true;' if enabled else 'false;') for row in rows)
    mask = '|'.join('((' + row['done'] + ' as u64)<<' + str(i) + ')' for i, row in enumerate(rows))
    assign = ''.join(row['done'] + '=n&(1<<' + str(i) + ')!=0;' for i, row in enumerate(rows))
    dispatch = ''.join(str(i) + '=>' + row['name'] + '(),' for i, row in enumerate(rows))
    return r'''#![allow(dead_code,unused_variables,unused_mut,static_mut_refs)]
macro_rules! serial_println {($($arg:tt)*)=>{println!($($arg)*)};}
mod sex_pdx {pub const SLOT_LINEN:u64=13;}
const SLOT_BELL:u64=12;const OP_BELL_NOTIFY:u64=0xc0;const OP_HID_EVENT:u64=0x202;
const SLOT_DISPLAY:u64=5;const OP_SILKBAR_UPDATE:u64=0xf2;
const EV_KEY:u64=1;const SURFACE_ID_LINEN:u64=200;const SURFACE_ID_BELL_PLACEHOLDER:u64=204;
const SURFACE_ID_SPINDLE:u64=0x99;const SURFACE_ID_MESH:u64=202;const BELL_RING_CAP:usize=16;
const SLOT_QUIL:u64=11;const EV_REL:u64=2;const EV_ABS:u64=3;const EV_BTN:u64=4;
const SURFACE_ID_APP:u64=100;const SURFACE_ID_BROWSER:u64=205;const SURFACE_ID_QUIL:u64=201;
const SPINDLE_FRAME_ID:u32=9;const QUIL_FRAME_ID:u32=3;const LINEN_FRAME_ID:u32=2;const COMMAND_PALETTE_FRAME_ID:u32=7;
''' + definitions + '\n' + constants + r'''
static mut STATUS:u64=0;static mut FOCUS:bool=false;static mut SEED:u64=0;static mut CALLS:u64=0;
#[derive(Clone,Copy)]struct BellEvent{present:bool,event_id:u64,object_id:u64,buffer_id:u64}
const EMPTY_EVENT:BellEvent=BellEvent{present:false,event_id:0,object_id:0,buffer_id:0};
static mut BELL_EVENTS:[BellEvent;BELL_RING_CAP]=[EMPTY_EVENT;BELL_RING_CAP];static mut BELL_RING_WRITE_INDEX:u64=0;static mut BELL_EVENT_SEQUENCE:u64=0;static mut BELL_SELECTED_ROW:u8=0;static mut BELL_DETAIL_OPEN:bool=false;static mut FOCUSED_SURFACE_ID:u64=0;
#[derive(Clone,Copy)]struct Scene{accent:u8} const EMPTY_SCENE:Scene=Scene{accent:0};
#[derive(Clone,Copy)]struct Appearance{preset_idx:u8}
#[derive(Clone,Copy)]struct CommandDef{command:u8}
static mut ATLAS_MODE_ENABLED:bool=false;static mut ATLAS_SELECTED_SCENE:u8=0;
static mut SCENES:[Scene;5]=[EMPTY_SCENE;5];static mut ACTIVE_TINT_IDX:u8=0;
static mut SCENE_APPEARANCE_STATE:Appearance=Appearance{preset_idx:0};
static mut COMMAND_PALETTE_OPEN:bool=false;static mut COMMAND_PALETTE_SELECTED:u8=0;
static mut SILKBAR_PHASE2_SHELL_PROOF_STAGE:u8=0;
#[derive(Clone,Copy,PartialEq)]enum InteractionState{Idle,Dragging}
static mut INPUT_TRACE_DRAG_MOVES:u32=0;static mut INTERACTION:InteractionState=InteractionState::Idle;
static mut ABS_SEEN_VALID:bool=true;static mut DRAG_TARGET_ACTIVE:bool=false;
static mut WINDOW_X:i32=10;static mut WINDOW_Y:i32=20;
#[derive(Clone,Copy)]struct LinenObject{object_id:u64}
#[derive(Clone,Copy)]struct CollarGrant{grant_id:u64}
static mut LINEN_OBJECTS:[Option<LinenObject>;1]=[None];static mut LINEN_OBJECT_DETAIL_OPEN:bool=false;
static mut COLLAR_OVERLAY_ENABLED:bool=false;static mut COLLAR_SELECTED_GRANT_IDX:u8=0;
static mut COLLAR_GRANTS:[Option<CollarGrant>;1]=[None];
const COMMAND_LIST:[CommandDef;11]=[CommandDef{command:0},CommandDef{command:1},CommandDef{command:2},
 CommandDef{command:3},CommandDef{command:4},CommandDef{command:5},CommandDef{command:6},
 CommandDef{command:7},CommandDef{command:8},CommandDef{command:9},CommandDef{command:10}];
#[repr(u32)]enum UpdateKind{SetActiveApp=8,SetTintAccent=9,SetPaletteState=10}
fn pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64){unsafe{let status=STATUS;CALLS+=1;let value=a^b^c^op^SEED;println!("P {} {} {} {} {} {} {}",slot,op,a,b,c,status,value);(status,value)}}
unsafe fn try_set_focus(surface:u64)->bool{if surface==SURFACE_ID_BROWSER{return false;}if FOCUS{FOCUSED_SURFACE_ID=surface;}FOCUS}
unsafe fn focus_or_open_quil()->bool{FOCUS}
unsafe fn focus_or_open_bell()->bool{if FOCUS{FOCUSED_SURFACE_ID=SURFACE_ID_BELL_PLACEHOLDER;}FOCUS}
unsafe fn open_spindle_in_active_scene()->bool{FOCUS}
unsafe fn focus_or_open_mesh()->bool{if FOCUS{FOCUSED_SURFACE_ID=SURFACE_ID_MESH;}FOCUS}
unsafe fn focus_or_open_linen()->bool{if FOCUS{FOCUSED_SURFACE_ID=SURFACE_ID_LINEN;}FOCUS}
unsafe fn atlas_toggle(){ATLAS_MODE_ENABLED=!ATLAS_MODE_ENABLED;}
unsafe fn handle_atlas_keyboard(sc:u8)->bool{if !ATLAS_MODE_ENABLED{return false;}match sc{
 0x4d=>ATLAS_SELECTED_SCENE=(ATLAS_SELECTED_SCENE+1)%5,
 0x4b=>ATLAS_SELECTED_SCENE=(ATLAS_SELECTED_SCENE+4)%5,
 0x1e=>{let at=ATLAS_SELECTED_SCENE as usize;SCENES[at].accent=(SCENES[at].accent+1)%8;},
 0x2c=>{let at=ATLAS_SELECTED_SCENE as usize;SCENES[at].accent=(SCENES[at].accent+7)%8;},
 0x1f=>SCENE_APPEARANCE_STATE.preset_idx=(SCENE_APPEARANCE_STATE.preset_idx+1)%4,
 0x11=>SCENE_APPEARANCE_STATE.preset_idx=(SCENE_APPEARANCE_STATE.preset_idx+3)%4,
 0x1c=>{ACTIVE_TINT_IDX=SCENES[ATLAS_SELECTED_SCENE as usize].accent;ATLAS_MODE_ENABLED=false;},
 1=>ATLAS_MODE_ENABLED=false,_=>return false}true}
fn get_preset_name(idx:u8)->&'static str{match idx%4{0=>"Default",1=>"Warm",2=>"Cool",_=>"HighContrast"}}
unsafe fn toggle_command_palette(){COMMAND_PALETTE_OPEN=!COMMAND_PALETTE_OPEN;}
unsafe fn palette_item_status(command:u8)->(bool,&'static str){(command%3!=2,if command%3!=2{"ok"}else{"unavailable"})}
unsafe fn send_silkbar_phase2_update(kind:u32,a:u64,b:u64)->bool{if !SILKBAR_PHASE2_SHELL_PROOF_ENABLED{return false;}let _=pdx_call(SLOT_DISPLAY,OP_SILKBAR_UPDATE,kind as u64,a,b);let name=match kind{8=>"SetActiveApp",9=>"SetTintAccent",10=>"SetPaletteState",_=>"Unknown"};println!("[shell.silkbar.phase2.send] kind={} a={} b={} ok=1 reason=sent",name,a,b);true}
unsafe fn frame_close_allowed(frame:u32)->bool{frame==SPINDLE_FRAME_ID||frame==QUIL_FRAME_ID}
unsafe fn lifecycle_state(surface:u64)->Option<u8>{if FOCUS&&surface==310{Some(2)}else{None}}
unsafe fn is_closeable_surface(surface:u64)->bool{surface==310||surface==SURFACE_ID_APP}
unsafe fn close_surface_from_frame_light(surface:u64)->bool{is_closeable_surface(surface)}
fn desktop_width()->i32{1280} fn desktop_height()->i32{720}
fn abs_screen_to_raw(screen:i32,dimension:i32)->u64{if dimension<=1{0}else{((screen.max(0).min(dimension-1)as u64)*65535)/((dimension-1)as u64)}}
unsafe fn synthetic_window_drag_target()->Option<(u32,u64,i32,i32,u32,u32,i32,i32)>{if !FOCUS{return None;}DRAG_TARGET_ACTIVE=true;Some((SPINDLE_FRAME_ID,SURFACE_ID_SPINDLE,WINDOW_X,WINDOW_Y,320,200,WINDOW_X+4,WINDOW_Y+4))}
unsafe fn get_surface_bounds(surface:u64)->Option<(i32,i32,u32,u32)>{if surface==SURFACE_ID_SPINDLE{Some((WINDOW_X,WINDOW_Y,320,200))}else{None}}
unsafe fn handle_hid_event(class:u64,a:u64,b:u64){match class{EV_ABS=>{let _=(a,b);},EV_BTN=>{if b==1&&DRAG_TARGET_ACTIVE{INTERACTION=InteractionState::Dragging;}else if b==0{INTERACTION=InteractionState::Idle;DRAG_TARGET_ACTIVE=false;}},EV_REL=>{if INTERACTION==InteractionState::Dragging{WINDOW_X=WINDOW_X.wrapping_add(a as i32);WINDOW_Y=WINDOW_Y.wrapping_add(b as i32);INPUT_TRACE_DRAG_MOVES=INPUT_TRACE_DRAG_MOVES.wrapping_add(1);}},_=>{}}}
unsafe fn linen_object_count()->usize{LINEN_OBJECTS.iter().filter(|x|x.is_some()).count()}
unsafe fn linen_selected_object_id()->u64{LINEN_OBJECTS[0].map(|x|x.object_id).unwrap_or(0)}
unsafe fn linen_select_next_object(){} unsafe fn linen_select_prev_object(){}
unsafe fn linen_object_detail_open(){LINEN_OBJECT_DETAIL_OPEN=linen_selected_object_id()!=0;}
unsafe fn linen_object_detail_close(){LINEN_OBJECT_DETAIL_OPEN=false;}
unsafe fn open_linen_in_active_scene()->bool{FOCUS}
unsafe fn linen_queue_open(_object:u64,_choice:u64){}
unsafe fn open_linen_object_in_quil(_object:u64)->bool{true}
unsafe fn focus_or_open_collar()->bool{FOCUS}
unsafe fn collar_select_next_grant(){COLLAR_SELECTED_GRANT_IDX=0;}
unsafe fn collar_select_prev_grant(){COLLAR_SELECTED_GRANT_IDX=0;}
unsafe fn collar_emit_selected_grant_detail()->bool{COLLAR_GRANTS[COLLAR_SELECTED_GRANT_IDX as usize].is_some()}
unsafe fn collar_grant_at_visible_index(idx:u8)->Option<CollarGrant>{COLLAR_GRANTS.get(idx as usize).copied().flatten()}
unsafe fn toggle_collar()->bool{COLLAR_OVERLAY_ENABLED=!COLLAR_OVERLAY_ENABLED;true}
unsafe fn bell_record_event(object_id:u64,buffer_id:u64){let at=BELL_RING_WRITE_INDEX as usize%BELL_RING_CAP;let event_id=BELL_EVENT_SEQUENCE;BELL_EVENT_SEQUENCE+=1;BELL_EVENTS[at]=BellEvent{present:true,event_id,object_id,buffer_id};BELL_RING_WRITE_INDEX+=1;}
unsafe fn bell_ring_count()->usize{(BELL_RING_WRITE_INDEX as usize).min(BELL_RING_CAP)}
unsafe fn bell_select_next_row(){let count=bell_ring_count()as u8;if count>1{BELL_SELECTED_ROW=if BELL_SELECTED_ROW+1>=count{0}else{BELL_SELECTED_ROW+1};}}
unsafe fn bell_select_prev_row(){let count=bell_ring_count()as u8;if count>1{BELL_SELECTED_ROW=if BELL_SELECTED_ROW>0{BELL_SELECTED_ROW-1}else{count-1};}}
unsafe fn bell_selected_event_snapshot()->Option<BellEvent>{let count=bell_ring_count();if count==0{return None;}let start=(BELL_RING_WRITE_INDEX as usize-1)%BELL_RING_CAP;for i in 0..count{let at=(start+BELL_RING_CAP-i)%BELL_RING_CAP;if BELL_EVENTS[at].present&&i==BELL_SELECTED_ROW as usize{return Some(BELL_EVENTS[at]);}}None}
unsafe fn bell_emit_selected_event_detail_proof(){BELL_DETAIL_OPEN=FOCUSED_SURFACE_ID==SURFACE_ID_BELL_PLACEHOLDER&&bell_selected_event_snapshot().is_some();}
unsafe fn bell_close_detail(){BELL_DETAIL_OPEN=false;}
unsafe fn bell_cycle_lane()->bool{true}
''' + bell + '\n' + '\n'.join(bodies) + r'''
unsafe fn mask()->u64{MASK}
unsafe fn assign(n:u64){ASSIGN}
unsafe fn dispatch(i:usize){match i{DISPATCH _=>panic!("bad index")}}
fn main(){use std::io::BufRead;let mut other=0u64;let mut other_phase_stage=0u8;for line in std::io::stdin().lock().lines(){let line=line.unwrap();let args:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();let n=|i:usize|args.get(i).copied().unwrap_or(0);unsafe{match n(0){0=>{assign(0);other=0;other_phase_stage=0;STATUS=0;FOCUS=false;SEED=0;CALLS=0;BELL_EVENTS=[EMPTY_EVENT;BELL_RING_CAP];BELL_RING_WRITE_INDEX=0;BELL_EVENT_SEQUENCE=0;BELL_SELECTED_ROW=0;BELL_DETAIL_OPEN=false;FOCUSED_SURFACE_ID=0;ATLAS_MODE_ENABLED=false;ATLAS_SELECTED_SCENE=0;SCENES=[EMPTY_SCENE;5];ACTIVE_TINT_IDX=0;SCENE_APPEARANCE_STATE=Appearance{preset_idx:0};COMMAND_PALETTE_OPEN=false;COMMAND_PALETTE_SELECTED=0;SILKBAR_PHASE2_SHELL_PROOF_STAGE=0;INPUT_TRACE_DRAG_MOVES=0;INTERACTION=InteractionState::Idle;ABS_SEEN_VALID=true;DRAG_TARGET_ACTIVE=false;WINDOW_X=10;WINDOW_Y=20;LINEN_OBJECTS=[None];LINEN_OBJECT_DETAIL_OPEN=false;COLLAR_OVERLAY_ENABLED=false;COLLAR_SELECTED_GRANT_IDX=0;COLLAR_GRANTS=[None];},1=>dispatch(n(1)as usize),2=>{STATUS=n(1);FOCUS=n(2)!=0;SEED=n(3);CALLS=0;FOCUSED_SURFACE_ID=if FOCUS{SURFACE_ID_LINEN}else{0};ATLAS_SELECTED_SCENE=(SEED%5)as u8;SCENES[ATLAS_SELECTED_SCENE as usize].accent=((SEED>>3)%8)as u8;ACTIVE_TINT_IDX=((SEED>>6)%8)as u8;SCENE_APPEARANCE_STATE.preset_idx=((SEED>>9)%4)as u8;COMMAND_PALETTE_OPEN=(SEED&1)!=0;COMMAND_PALETTE_SELECTED=((SEED>>1)%11)as u8;LINEN_OBJECTS=if FOCUS{[Some(LinenObject{object_id:(SEED|1)})]}else{[None]};COLLAR_GRANTS=if FOCUS{[Some(CollarGrant{grant_id:(SEED|1)})]}else{[None]};},3=>assign(n(1)),4=>{let saved=mask();let saved_stage=SILKBAR_PHASE2_SHELL_PROOF_STAGE;assign(other);SILKBAR_PHASE2_SHELL_PROOF_STAGE=other_phase_stage;dispatch(n(1)as usize);other=mask();other_phase_stage=SILKBAR_PHASE2_SHELL_PROOF_STAGE;assign(saved);SILKBAR_PHASE2_SHELL_PROOF_STAGE=saved_stage;},5=>other=n(1),_=>panic!("bad operation")}println!("S {} {} {}",mask(),other,CALLS);}}}
'''.replace('MASK', mask).replace('ASSIGN', assign).replace('DISPATCH', dispatch)


def provider(inventory):
    rows = inventory['functions']
    def field(row):
        if row.get('owner') == 'atlas':
            return '(state==&other?other_atlas.' + row['done'] + ':atlas.' + row['done'] + ')'
        return 'state->' + row['done']
    mask = '|'.join('((uint64_t)' + field(row) + '<<' + str(i) + ')' for i, row in enumerate(rows))
    assign = ''.join(
        (('(state==&other?&other_atlas:&atlas)->' + row['done'])
         if row.get('owner') == 'atlas' else 'state->' + row['done']) +
        '=(n&(UINT64_C(1)<<' + str(i) + '))!=0;' for i, row in enumerate(rows))
    dispatch = ''.join('case ' + str(i) + ':shell_proofs_a_' + row['name'] + '(ctx);break;' for i, row in enumerate(rows))
    return r'''#include "silk_shell_proofs_a_routes.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static SilkShellProofsAState state,other;static SilkShellChromeState chrome;static SilkShellModelsState models;
static ShellChromeApps apps;static SilkShellInputState input;static SilkShellAtlasState atlas,other_atlas;static SilkShellSurfacesState surfaces;
static uint64_t status,seed,calls;static bool focus;
static bool drag_target_active;static int32_t window_x,window_y;
static ShellInputContext context(SilkShellProofsAState *proofs){return(ShellInputContext){.shell={.chrome=&chrome,.models=&models,.apps=&apps,.input=&input,.proofs_a=proofs},.atlas=&atlas,.surfaces=&surfaces};}
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);va_end(ap);putchar('\n');}
bool shell_proofs_a_route_try_set_focus(ShellInputContext *ctx,uint64_t surface){if(surface==SURFACE_ID_BROWSER)return false;if(focus)ctx->shell.chrome->FOCUSED_SURFACE_ID=surface;return focus;}
bool shell_proofs_a_route_focus_or_open_quil(ShellInputContext *ctx){(void)ctx;return focus;}
SexosReply shell_proofs_a_route_pdx_call(ShellInputContext *ctx,uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c){(void)ctx;uint64_t value=a^b^c^op^seed;++calls;printf("P %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",slot,op,a,b,c,status,value);return(SexosReply){status,value};}
void shell_proofs_a_route_bell_event(ShellInputContext *ctx,const ShellDiagnosticLabel *source,uint64_t event){(void)ctx;uint64_t value=(event&255)^OP_BELL_NOTIFY^UINT64_C(0x100)^seed;++calls;printf("P %u %u %u %" PRIu64 " 0 %" PRIu64 " %" PRIu64 "\n",(unsigned)SLOT_BELL,(unsigned)OP_BELL_NOTIFY,0x100,event&255,status,value);printf("[bell.app.event] source=%.*s event_id=%" PRIu64 " ok=%u reason=%s\n",(int)source->len,source->bytes,event,(unsigned)(status==0),status==0?"fire_and_forget":"enqueue_fail");}
bool shell_proofs_a_route_focus_or_open_bell(ShellInputContext *ctx){if(focus)ctx->shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_BELL_PLACEHOLDER;return focus;}
void shell_proofs_a_route_bell_select_next(ShellInputContext *ctx){size_t count=ctx->shell.models->BELL_RING_WRITE_INDEX<BELL_RING_CAP?(size_t)ctx->shell.models->BELL_RING_WRITE_INDEX:BELL_RING_CAP;if(count>1)ctx->surfaces->BELL_SELECTED_ROW=(uint8_t)(((size_t)ctx->surfaces->BELL_SELECTED_ROW+1>=count)?0:ctx->surfaces->BELL_SELECTED_ROW+1);}
void shell_proofs_a_route_bell_select_previous(ShellInputContext *ctx){size_t count=ctx->shell.models->BELL_RING_WRITE_INDEX<BELL_RING_CAP?(size_t)ctx->shell.models->BELL_RING_WRITE_INDEX:BELL_RING_CAP;if(count>1)ctx->surfaces->BELL_SELECTED_ROW=ctx->surfaces->BELL_SELECTED_ROW?ctx->surfaces->BELL_SELECTED_ROW-1:(uint8_t)(count-1);}
bool shell_proofs_a_route_bell_selected(ShellInputContext *ctx,BellEvent *event){size_t count=ctx->shell.models->BELL_RING_WRITE_INDEX<BELL_RING_CAP?(size_t)ctx->shell.models->BELL_RING_WRITE_INDEX:BELL_RING_CAP;if(!count)return false;size_t start=(size_t)(ctx->shell.models->BELL_RING_WRITE_INDEX-1)%BELL_RING_CAP;for(size_t i=0;i<count;++i){size_t at=(start+BELL_RING_CAP-i)%BELL_RING_CAP;if(ctx->shell.models->BELL_EVENTS[at].present&&i==ctx->surfaces->BELL_SELECTED_ROW){*event=ctx->shell.models->BELL_EVENTS[at];return true;}}return false;}
void shell_proofs_a_route_bell_emit_detail(ShellInputContext *ctx){BellEvent event={0};ctx->surfaces->BELL_DETAIL_OPEN=ctx->shell.chrome->FOCUSED_SURFACE_ID==SURFACE_ID_BELL_PLACEHOLDER&&shell_proofs_a_route_bell_selected(ctx,&event);}
void shell_proofs_a_route_bell_close_detail(ShellInputContext *ctx){ctx->surfaces->BELL_DETAIL_OPEN=false;}
bool shell_proofs_a_route_bell_cycle_lane(ShellInputContext *ctx){(void)ctx;return true;}
void shell_proofs_a_route_bell_record_event(ShellInputContext *ctx,uint64_t object_id,uint64_t buffer_id){size_t at=(size_t)ctx->shell.models->BELL_RING_WRITE_INDEX%BELL_RING_CAP;ctx->shell.models->BELL_EVENTS[at]=(BellEvent){.present=true,.event_id=ctx->shell.models->BELL_EVENT_SEQUENCE++,.kind=BELL_EVENT_OBJECT_LINKED_TO_BUFFER,.object_id=object_id,.buffer_id=buffer_id,.sequence=ctx->shell.models->BELL_RING_WRITE_INDEX};++ctx->shell.models->BELL_RING_WRITE_INDEX;}
size_t shell_proofs_a_route_bell_count(ShellInputContext *ctx){return ctx->shell.models->BELL_RING_WRITE_INDEX<BELL_RING_CAP?(size_t)ctx->shell.models->BELL_RING_WRITE_INDEX:BELL_RING_CAP;}
void shell_proofs_a_route_atlas_toggle(ShellInputContext *ctx){ctx->shell.chrome->ATLAS_MODE_ENABLED=!ctx->shell.chrome->ATLAS_MODE_ENABLED;}
bool shell_proofs_a_route_atlas_keyboard(ShellInputContext *ctx,uint8_t sc){if(!ctx->shell.chrome->ATLAS_MODE_ENABLED)return false;size_t at=ctx->shell.chrome->ATLAS_SELECTED_SCENE;switch(sc){case 0x4d:ctx->shell.chrome->ATLAS_SELECTED_SCENE=(uint8_t)((at+1)%ATLAS_MAX_SCENES);break;case 0x4b:ctx->shell.chrome->ATLAS_SELECTED_SCENE=(uint8_t)((at+ATLAS_MAX_SCENES-1)%ATLAS_MAX_SCENES);break;case 0x1e:ctx->shell.chrome->SCENES[at].accent=(uint8_t)((ctx->shell.chrome->SCENES[at].accent+1)%8);break;case 0x2c:ctx->shell.chrome->SCENES[at].accent=(uint8_t)((ctx->shell.chrome->SCENES[at].accent+7)%8);break;case 0x1f:ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx=(uint8_t)((ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx+1)%4);break;case 0x11:ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx=(uint8_t)((ctx->shell.models->SCENE_APPEARANCE_STATE.preset_idx+3)%4);break;case 0x1c:ctx->shell.models->ACTIVE_TINT_IDX=ctx->shell.chrome->SCENES[at].accent;ctx->shell.chrome->ATLAS_MODE_ENABLED=false;break;case 1:ctx->shell.chrome->ATLAS_MODE_ENABLED=false;break;default:return false;}return true;}
const char *shell_proofs_a_route_preset_name(ShellInputContext *ctx,uint8_t preset){(void)ctx;static const char *const names[4]={"Default","Warm","Cool","HighContrast"};return names[preset%4];}
bool shell_proofs_a_route_open_spindle(ShellInputContext *ctx){(void)ctx;return focus;}
bool shell_proofs_a_route_focus_or_open_mesh(ShellInputContext *ctx){if(focus)ctx->shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_MESH;return focus;}
bool shell_proofs_a_route_focus_or_open_linen(ShellInputContext *ctx){if(focus)ctx->shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_LINEN;return focus;}
bool shell_proofs_a_route_toggle_palette(ShellInputContext *ctx){ctx->surfaces->COMMAND_PALETTE_OPEN=!ctx->surfaces->COMMAND_PALETTE_OPEN;return true;}
size_t shell_proofs_a_route_command_count(ShellInputContext *ctx){(void)ctx;return 11;}
bool shell_proofs_a_route_palette_available(ShellInputContext *ctx,size_t index){(void)ctx;return index<11&&index%3!=2;}
bool shell_proofs_a_route_silkbar_update(ShellInputContext *ctx,uint32_t kind,uint64_t a,uint64_t b){(void)ctx;uint64_t value=(uint64_t)kind^a^b^OP_SILKBAR_UPDATE^seed;++calls;printf("P %u %u %u %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",(unsigned)SLOT_DISPLAY,(unsigned)OP_SILKBAR_UPDATE,kind,a,b,status,value);const char *name=kind==8?"SetActiveApp":kind==9?"SetTintAccent":kind==10?"SetPaletteState":"Unknown";printf("[shell.silkbar.phase2.send] kind=%s a=%" PRIu64 " b=%" PRIu64 " ok=1 reason=sent\n",name,a,b);return true;}
bool shell_proofs_a_route_frame_close_allowed(ShellInputContext *ctx,uint32_t frame){(void)ctx;return frame==SPINDLE_FRAME_ID||frame==QUIL_FRAME_ID;}
bool shell_proofs_a_route_lifecycle_present(ShellInputContext *ctx,uint64_t surface){(void)ctx;return focus&&surface==310;}
bool shell_proofs_a_route_is_closeable_surface(ShellInputContext *ctx,uint64_t surface){(void)ctx;return surface==310||surface==SURFACE_ID_APP;}
bool shell_proofs_a_route_close_surface(ShellInputContext *ctx,uint64_t surface){return shell_proofs_a_route_is_closeable_surface(ctx,surface);}
uint64_t shell_proofs_a_route_abs_screen_to_raw(ShellInputContext *ctx,int32_t screen,int32_t dimension){(void)ctx;if(dimension<=1)return 0;if(screen<0)screen=0;if(screen>=dimension)screen=dimension-1;return(uint64_t)screen*65535/(uint64_t)(dimension-1);}
int32_t shell_proofs_a_route_desktop_width(ShellInputContext *ctx){(void)ctx;return 1280;}
int32_t shell_proofs_a_route_desktop_height(ShellInputContext *ctx){(void)ctx;return 720;}
ShellProofDragTarget shell_proofs_a_route_synthetic_drag_target(ShellInputContext *ctx){(void)ctx;if(!focus)return(ShellProofDragTarget){0};drag_target_active=true;return(ShellProofDragTarget){true,SPINDLE_FRAME_ID,SURFACE_ID_SPINDLE,window_x,window_y,320,200,window_x+4,window_y+4};}
ShellProofBounds shell_proofs_a_route_surface_bounds(ShellInputContext *ctx,uint64_t surface){(void)ctx;if(surface!=SURFACE_ID_SPINDLE)return(ShellProofBounds){0};return(ShellProofBounds){true,window_x,window_y,320,200};}
void shell_proofs_a_route_handle_hid(ShellInputContext *ctx,uint64_t event_class,uint64_t a,uint64_t b){if(event_class==EV_BTN){if(b==1&&drag_target_active)ctx->shell.input->INTERACTION=(InteractionState){.tag=INTERACTION_DRAGGING};else if(!b){ctx->shell.input->INTERACTION=(InteractionState){.tag=INTERACTION_IDLE};drag_target_active=false;}}else if(event_class==EV_REL&&ctx->shell.input->INTERACTION.tag==INTERACTION_DRAGGING){window_x+=(int32_t)a;window_y+=(int32_t)b;++ctx->shell.input->INPUT_TRACE_DRAG_MOVES;}}
void shell_proofs_a_route_linen_select_next(ShellInputContext *ctx){(void)ctx;}
void shell_proofs_a_route_linen_select_previous(ShellInputContext *ctx){(void)ctx;}
uint64_t shell_proofs_a_route_linen_selected_id(ShellInputContext *ctx){return ctx->shell.models->LINEN_OBJECTS[0].present?ctx->shell.models->LINEN_OBJECTS[0].object_id:0;}
void shell_proofs_a_route_linen_detail_open(ShellInputContext *ctx){ctx->shell.models->LINEN_OBJECT_DETAIL_OPEN=shell_proofs_a_route_linen_selected_id(ctx)!=0;}
void shell_proofs_a_route_linen_detail_close(ShellInputContext *ctx){ctx->shell.models->LINEN_OBJECT_DETAIL_OPEN=false;}
bool shell_proofs_a_route_open_linen(ShellInputContext *ctx){(void)ctx;return focus;}
ShellProofLinenFirst shell_proofs_a_route_linen_first_object(ShellInputContext *ctx){LinenObject object=ctx->shell.models->LINEN_OBJECTS[0];return(ShellProofLinenFirst){object.present?1:0,object.present,object.object_id};}
void shell_proofs_a_route_linen_queue_open(ShellInputContext *ctx,uint64_t object){(void)ctx;(void)object;}
bool shell_proofs_a_route_linen_open_in_quil(ShellInputContext *ctx,uint64_t object){(void)ctx;(void)object;return true;}
bool shell_proofs_a_route_focus_or_open_collar(ShellInputContext *ctx){(void)ctx;return focus;}
void shell_proofs_a_route_collar_select_next(ShellInputContext *ctx){ctx->shell.models->COLLAR_SELECTED_GRANT_IDX=0;}
void shell_proofs_a_route_collar_select_previous(ShellInputContext *ctx){ctx->shell.models->COLLAR_SELECTED_GRANT_IDX=0;}
bool shell_proofs_a_route_collar_emit_detail(ShellInputContext *ctx){return ctx->shell.models->COLLAR_GRANTS[ctx->shell.models->COLLAR_SELECTED_GRANT_IDX].present;}
ShellProofId shell_proofs_a_route_collar_selected_grant(ShellInputContext *ctx){CollarGrant grant=ctx->shell.models->COLLAR_GRANTS[ctx->shell.models->COLLAR_SELECTED_GRANT_IDX];return(ShellProofId){grant.present,grant.grant_id};}
bool shell_proofs_a_route_toggle_collar(ShellInputContext *ctx){ctx->shell.models->COLLAR_OVERLAY_ENABLED=!ctx->shell.models->COLLAR_OVERLAY_ENABLED;return true;}
void sexos_fail(void){abort();}
static uint64_t state_mask(const SilkShellProofsAState *state){return MASK;}
static void state_assign(SilkShellProofsAState *state,uint64_t n){ASSIGN}
static void dispatch(ShellInputContext *ctx,size_t index){switch(index){DISPATCH default:exit(3);}}
int main(void){char line[256];while(fgets(line,sizeof(line),stdin)){uint64_t n[4]={0};char *at=line,*end;for(size_t i=0;i<4;++i){n[i]=strtoull(at,&end,10);if(at==end)break;at=end;}switch(n[0]){case 0:memset(&state,0,sizeof state);memset(&other,0,sizeof other);memset(&chrome,0,sizeof chrome);memset(&models,0,sizeof models);memset(&atlas,0,sizeof atlas);memset(&other_atlas,0,sizeof other_atlas);memset(&surfaces,0,sizeof surfaces);memset(&input,0,sizeof input);input.ABS_SEEN_VALID=true;input.INTERACTION=(InteractionState){.tag=INTERACTION_IDLE};drag_target_active=false;window_x=10;window_y=20;status=seed=calls=0;focus=false;break;case 1:{ShellInputContext ctx=context(&state);dispatch(&ctx,n[1]);break;}case 2:status=n[1];focus=n[2]!=0;seed=n[3];calls=0;chrome.FOCUSED_SURFACE_ID=focus?SURFACE_ID_LINEN:0;chrome.ATLAS_SELECTED_SCENE=(uint8_t)(seed%ATLAS_MAX_SCENES);chrome.SCENES[chrome.ATLAS_SELECTED_SCENE].accent=(uint8_t)((seed>>3)%8);models.ACTIVE_TINT_IDX=(uint8_t)((seed>>6)%8);models.SCENE_APPEARANCE_STATE.preset_idx=(uint8_t)((seed>>9)%4);surfaces.COMMAND_PALETTE_OPEN=(seed&1)!=0;surfaces.COMMAND_PALETTE_SELECTED=(uint8_t)((seed>>1)%11);models.LINEN_OBJECTS[0]=(LinenObject){.present=focus,.object_id=seed|1};models.COLLAR_GRANTS[0]=(CollarGrant){.present=focus,.grant_id=seed|1};break;case 3:state_assign(&state,n[1]);break;case 4:{ShellInputContext ctx=context(&other);dispatch(&ctx,n[1]);break;}case 5:state_assign(&other,n[1]);break;default:return 3;}printf("S %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",state_mask(&state),state_mask(&other),calls);}return 0;}
'''.replace('MASK', mask).replace('ASSIGN', assign).replace('DISPATCH', dispatch)


def corpus(entries):
    count=len(entries)
    rows=[]
    def add(*values): rows.append(' '.join(map(str, values)))
    for i in range(count):
        for status in (0,1,2,2**64-1):
            for focus in (0,1):
                add(0);add(2,status,focus,0xfedcba9876543210);add(1,i);add(1,i)
                if entries[i].get('owner') != 'atlas':
                    add(4,i);add(4,i)
        add(0);add(3,(1<<count)-1);add(1,i);add(5,(1<<count)-1);add(4,i)
    return ('\n'.join(rows)+'\n').encode(),len(rows)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True)
    parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path)
    parser.add_argument('--output',type=Path,default=HERE/'build/proofs-a-routes');args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/silk_shell_proofs_a_routes.source.json').read_text())
    bodies,bell=extract(inventory);(work/'provider.c').write_text(provider(inventory));data,commands=corpus(inventory['functions'])
    exports=[];plans={}
    for compiler in ('gcc','clang'):
        c=work/('routes-'+compiler+'.c');plan=work/('routes-'+compiler+'.plan')
        run([args.cesta,'export-c','src/silk_shell_proofs_a_routes.cesta','--cc',compiler,'--std=c11','-o',c,'--plan',plan,'--','-Igenerated/include'])
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
        if compiler=='gcc':exports.append(c)
    native=work/'routes-native.c';run([args.cesta,'export-c','src/silk_shell_proofs_a_routes_native.cesta','--std=c11','-o',native,'--','-Igenerated/include'])
    command_count=work/'command-count.c';run([args.cesta,'export-c','src/silk_shell_command_count.cesta','--std=c11','-o',command_count,'--','-Igenerated/include'])
    bell_models=work/'bell-models.c';bell_plan=work/'bell-models.plan';run([args.cesta,'export-c','src/silk_shell_models_bell.cesta','--std=c11','-o',bell_models,'--plan',bell_plan,'--','-Igenerated/include']);plans[hashlib.sha256(bell_plan.read_bytes()).hexdigest()]=bell_plan
    records=[]
    for profile,enabled in (('default',False),('all',True)):
        rust=work/('oracle-'+profile+'.rs');rust.write_text(rust_source(inventory,bodies,bell,enabled));oracle=work/('oracle-'+profile);run(['rustc','--edition=2021','-O',rust,'-o',oracle]);expected=run([oracle],data)
        flags=['-D'+row['flag']+'=1'for row in inventory['functions']]if enabled else[]
        for compiler in ('gcc','clang'):
            for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
                binary=work/(profile+'-'+compiler+'-'+mode);source=work/('routes-'+compiler+'.c')
                run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*options,*flags,'-Igenerated/include',source,work/'provider.c','-o',binary]);actual=run([binary],data,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'})
                assert actual==expected,(profile,compiler,mode);records.append(dict(profile=profile,compiler=compiler,mode=mode,sha256=hashlib.sha256(actual).hexdigest()))
    joined=[];all_flags=['-D'+row['flag']+'=1'for row in inventory['functions']]
    for compiler in ('gcc','clang'):
        for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
            binary=work/('joined-'+compiler+'-'+mode);run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*options,*all_flags,'-Igenerated/include',work/('routes-'+compiler+'.c'),native,bell_models,command_count,'tests/silk_shell_proofs_a_routes/root_host.c','-o',binary]);run([binary],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});joined.append(dict(compiler=compiler,mode=mode))
    original=(HERE/'src/silk_shell_proofs_a_routes.cesta').read_text();mutants=[];reference=run([work/'oracle-all'],data);base=(work/'routes-gcc.c').read_text();flags=['-D'+row['flag']+'=1'for row in inventory['functions']]
    repeat_targets={
      'maybe_run_bell_keyboard_detail_proof':'sexos_line("[bell.keyboard.detail.proof.done] ok=1");ctx->shell.proofs_a->BELL_KEYBOARD_DETAIL_PROOF_DONE=true',
      'maybe_run_bell_detail_seed_proof':'ctx->shell.proofs_a->BELL_DETAIL_SEED_PROOF_DONE=true;\n}',
      'maybe_run_atlas_scene_keyboard_proof':'sexos_line("[atlas.scene.keyboard.proof.done] ok=1");ctx->atlas->ATLAS_SCENE_KEYBOARD_PROOF_DONE=true',
      'maybe_run_atlas_theme_visual_proof':'(unsigned)(open&&accent&&apply));ctx->atlas->ATLAS_THEME_VISUAL_PROOF_DONE=true',
      'maybe_run_atlas_theme_presets_proof':'(unsigned)(s1&&s2&&s3&&apply));ctx->atlas->ATLAS_THEME_PRESETS_PROOF_DONE=true',
      'maybe_run_collar_keyboard_grants_proof':'sexos_line("[collar.keyboard.grants.proof.done] ok=1");ctx->shell.proofs_a->COLLAR_KEYBOARD_GRANTS_PROOF_DONE=true',
    }
    changes=[]
    for row in inventory['functions']:
        owner='ctx->atlas->' if row.get('owner')=='atlas' else 'ctx->shell.proofs_a->'
        target=repeat_targets.get(row['name'],owner+row['done']+'=true')
        changes.append(('repeat-'+row['name'],'shell_proofs_a_'+row['name'],target,target.replace('=true','=false')))
    changes += [
      ('wrong-linen-key','shell_proofs_a_maybe_run_linen_keyboard_route_proof','UINT64_C(0x24)','UINT64_C(0x23)'),
      ('skip-quil-focus','shell_proofs_a_maybe_run_physical_keyboard_to_quil_focus_proof','bool focused=shell_proofs_a_route_focus_or_open_quil(ctx)','bool focused=true'),
      ('wrong-app-event','shell_proofs_a_maybe_run_bell_app_event_integration_proof','&route_atlas_theme,1004','&route_atlas_theme,1005'),
      ('wrong-window-count','shell_proofs_a_maybe_run_window_workflow_v2_proof','passed=6 failed=1','passed=5 failed=2'),
      ('wrong-atlas-scene-key','shell_proofs_a_maybe_run_atlas_scene_keyboard_proof','route_atlas_stage(ctx,1,0x4d','route_atlas_stage(ctx,1,0x4e'),
      ('wrong-atlas-accent-key','shell_proofs_a_maybe_run_atlas_theme_visual_proof','shell_proofs_a_route_atlas_keyboard(ctx,0x1e)','shell_proofs_a_route_atlas_keyboard(ctx,0x1f)'),
      ('wrong-atlas-preset-key','shell_proofs_a_maybe_run_atlas_theme_presets_proof','shell_proofs_a_route_atlas_keyboard(ctx,0x11)','shell_proofs_a_route_atlas_keyboard(ctx,0x10)'),
      ('skip-silkbar-linen','shell_proofs_a_maybe_run_silkbar_keyboard_status_proof','bool linen=shell_proofs_a_route_focus_or_open_linen(ctx)','bool linen=false'),
      ('short-palette-scan','shell_proofs_a_maybe_run_silkbar_palette_status_proof','i<total','i+1<total'),
      ('wrong-silkbar-kind','shell_proofs_a_maybe_run_silkbar_phase2_shell_proof','SILKBAR_UPDATE_SET_ACTIVE_APP','SILKBAR_UPDATE_SET_TINT_ACCENT'),
      ('wrong-frame-stub-id','shell_proofs_a_maybe_run_frame_lights_stub_proof','SPINDLE_FRAME_ID,QUIL_FRAME_ID','LINEN_FRAME_ID,QUIL_FRAME_ID'),
      ('wrong-frame-close-target','shell_proofs_a_maybe_run_frame_lights_keyboard_proof','UINT64_C(310):SURFACE_ID_APP','UINT64_C(311):SURFACE_ID_APP'),
      ('wrong-focus-reject-target','shell_proofs_a_maybe_run_silk_focus_reject_proof','SURFACE_ID_BROWSER);','SURFACE_ID_APP);'),
      ('skip-drag-idle-check','shell_proofs_a_maybe_run_silk_drag_reject_proof','bool idle=ctx->shell.input->INTERACTION.tag==INTERACTION_IDLE','bool idle=false'),
      ('wrong-window-delta','shell_proofs_a_maybe_run_silk_window_move_proof','ctx,EV_REL,40,24','ctx,EV_REL,41,24'),
      ('wrong-quil-scan','shell_proofs_a_maybe_run_quil_visible_typing_e2e_proof','{0x1f,0x12,0x2d}','{0x1f,0x12,0x2c}'),
      ('drop-linen-selection','shell_proofs_a_maybe_run_linen_object_detail_proof','uint64_t object=shell_proofs_a_route_linen_selected_id(ctx)','uint64_t object=0'),
      ('drop-linen-first','shell_proofs_a_maybe_run_linen_nonblocking_open_proof','if(first.count>0)','if(false)'),
      ('reject-collar-open','shell_proofs_a_maybe_run_collar_keyboard_grants_proof','bool open=shell_proofs_a_route_focus_or_open_collar(ctx)','bool open=false')]
    for label,function,old,new in changes:
        path=work/('mutant-'+label+'.c');path.write_text(mutate_function(base,function,old,new));binary=work/('mutant-'+label);run(['gcc','-std=c11','-O2',*flags,'-Igenerated/include',path,work/'provider.c','-o',binary]);result=subprocess.run([binary],cwd=HERE,input=data,capture_output=True);assert result.returncode!=0 or result.stdout!=reference,('survived',label);mutants.append(label)
    source_controls=(('readonly-proof-state','(field proofs_a (borrow write))','(field proofs_a (borrow read))'),('const-route-context','void shell_proofs_a_maybe_run_linen_keyboard_route_proof(ShellInputContext *ctx)','void shell_proofs_a_maybe_run_linen_keyboard_route_proof(const ShellInputContext *ctx)'))
    for label,old,new in source_controls:
        assert original.count(old)==1,(label,original.count(old));path=work/(label+'.cesta');path.write_text(original.replace(old,new));run([args.cesta,'cc','--cc','gcc','-std=c11','-fsyntax-only','-Igenerated/include',path],accepted=False)
    admission=[]
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
        for i,plan in enumerate(plans.values()):record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,work/f'Routes{i}.lean',args.lean_root.resolve()))
    (work/'summary.json').write_text(json.dumps(dict(pin=inventory['pin'],functions=len(bodies),commands=commands,configurations=records,joined_root=joined,mutants_caught=mutants,source_controls=len(source_controls),admission=admission),indent=2)+'\n')
    print('Proofs-A routes:',len(bodies),'current bodies;',commands,'commands/configuration;',len(records),'configurations;',len(mutants),'mutants caught;',sum(x['functions']for x in admission),'plans admitted')


if __name__=='__main__':main()
