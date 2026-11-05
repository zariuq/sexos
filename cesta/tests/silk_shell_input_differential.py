"""Pinned input functions against scripted shell peers and both C compilers."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import re
import random
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
INVENTORY = HERE / 'interfaces/silk_shell_input.source.json'


def run(command, *, cwd=HERE, data=None, env=None):
    result = subprocess.run(list(map(str, command)), cwd=cwd, input=data,
                            capture_output=True, env=env)
    if result.returncode:
        raise AssertionError((command, result.returncode, result.stdout[-2000:], result.stderr[-8000:]))
    return result.stdout


def item_end(text, start):
    """Find a declaration terminator while respecting strings and comments."""
    depth = 0
    brackets = 0
    parentheses = 0
    seen_brace = False
    i = start
    while i < len(text):
        if text.startswith('//', i):
            j = text.find('\n', i)
            i = len(text) if j < 0 else j + 1
            continue
        if text.startswith('/*', i):
            level = 1; i += 2
            while level and i < len(text):
                if text.startswith('/*', i): level += 1; i += 2
                elif text.startswith('*/', i): level -= 1; i += 2
                else: i += 1
            if level:raise AssertionError(('unterminated block comment',start))
            continue
        if text[i] == '"':
            i += 1
            while i < len(text) and text[i] != '"':
                i += 2 if text[i] == '\\' else 1
            i += 1
            continue
        if text[i] == '[': brackets += 1
        elif text[i] == ']': brackets -= 1
        elif text[i] == '(': parentheses += 1
        elif text[i] == ')': parentheses -= 1
        elif text[i] == '{': depth += 1; seen_brace = True
        elif text[i] == '}':
            depth -= 1
            if seen_brace and depth == 0 and brackets == 0 and parentheses == 0:
                j = i + 1
                while j < len(text) and text[j].isspace(): j += 1
                # Constant structure/array initializers end at a semicolon.
                if re.match(r'(?:pub\s+)?(?:const(?!\s+(?:unsafe\s+)?fn\b)|static)\b', text[start:]):
                    if j < len(text) and text[j] == ';': return j + 1
                else: return i + 1
        elif text[i] == ';' and depth == 0 and brackets == 0 and parentheses == 0: return i + 1
        i += 1
    raise AssertionError(('unterminated pinned declaration', text[start:start+100]))


def declarations(text):
    result = {}
    pattern = r'(?m)^(?:#\[[^\n]*\]\s*)*(?P<item>(?:pub\s+)?(?:(?:static\s+(?:mut\s+)?)|const\s+(?!(?:unsafe\s+)?fn\b)|struct\s+|enum\s+|type\s+))(?P<name>\w+)\b'
    for match in re.finditer(pattern, text):
        if match['name'] == '_':
            continue  # An anonymous const has no binding a function can depend on.
        start = match.start()
        # Keep derives and representation attributes attached to data types.
        prefix = text[:start].splitlines(keepends=True)
        while prefix and (prefix[-1].lstrip().startswith(('#[', '///', '//')) or not prefix[-1].strip()):
            start -= len(prefix.pop())
        result[match['name']] = text[start:item_end(text, match.start('item'))]
    return result


def identifiers(text):
    text = re.sub(r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*.*?\*/', ' ', text, flags=re.S)
    return set(re.findall(r'\b[A-Za-z_]\w*\b', text))


def pinned_sources():
    inventory = json.loads(INVENTORY.read_text())
    source = run(['git', 'show', inventory['pin'] + ':servers/silk-shell/src/main.rs'], cwd=HERE.parent).decode()
    assert hashlib.sha256(source.encode()).hexdigest() == inventory['source_sha256']
    pdx = run(['git', 'show', inventory['pin'] + ':crates/sex-pdx/src/lib.rs'], cwd=HERE.parent).decode()
    lines = source.splitlines(keepends=True)
    functions = {}
    for item in inventory['functions'] + inventory.get('support_functions', []):
        text = ''.join(lines[item['start']-1:item['end']])
        assert hashlib.sha256(text.encode()).hexdigest() == item['sha256'], item['name']
        functions[item['name']] = text
    return inventory, source, pdx, functions


def rust_prelude():
    inventory, source, pdx, functions = pinned_sources()
    catalog = declarations(pdx)
    catalog.update(declarations(source))
    # Only the provider is substituted. All eighteen input bodies are intact.
    needed = set(inventory['globals']) | {'P'}
    for text in functions.values(): needed |= identifiers(text) & catalog.keys()
    pending = sorted(needed); selected = {}
    while pending:
        name = pending.pop()
        if name in selected: continue
        assert name in catalog, name
        selected[name] = catalog[name]
        pending.extend(sorted((identifiers(catalog[name]) & catalog.keys()) - selected.keys()))
    header = LayoutText((HERE/'generated/include/silk_shell_input.h').read_text())
    names = re.findall(r'\bshell_input_peer_(\w+)\(', header)
    signatures = {}
    for name in names:
        match = re.search(r'(?m)^(?:pub\s+)?(?:const\s+)?(?:unsafe\s+)?fn\s+'+name+r'\b', source)
        assert match, name
        end = source.find('{', match.start())
        signature = source[match.start():end].strip()
        signatures[name] = signature
        pending.extend(sorted((identifiers(signature) & catalog.keys()) - selected.keys()))
    while pending:
        name = pending.pop()
        if name in selected: continue
        selected[name] = catalog[name]
        pending.extend(sorted((identifiers(catalog[name]) & catalog.keys()) - selected.keys()))
    text = '''#![allow(dead_code, unused_imports, unused_variables, unused_mut, static_mut_refs)]
extern crate self as sex_pdx;
extern crate alloc;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU32, Ordering};
macro_rules! serial_println { ($($arg:tt)*) => { println!($($arg)*) }; }
#[macro_export] macro_rules! serial_record { ($($arg:tt)*) => { println!($($arg)*) }; }
'''
    for name, filename in (('app_catalog','crates/sex-pdx/src/app_catalog.rs'),
                           ('app_key','crates/sex-pdx/src/app_key.rs'),
                           ('app_custody','servers/silk-shell/src/app_custody.rs'),
                           ('app_lifecycle','crates/sex-pdx/src/app_lifecycle.rs')):
        expected=inventory.get('dependencies',{}).get(filename)
        if expected:
            path=HERE.parent/filename
            assert hashlib.sha256(path.read_bytes()).hexdigest()==expected,filename
            text+='#[path='+json.dumps(str(path))+']mod '+name+';\n'
    text += '\n'.join(selected.values()) + '\n' + '\n'.join(functions.values())
    return inventory, source, signatures, text


def prepare(work):
    inventory, original, signatures, prelude = rust_prelude()
    (work/'prelude.rs').write_text(prelude)
    (work/'signatures.json').write_text(json.dumps(signatures, indent=2)+'\n')
    (work/'inventory.json').write_text(json.dumps(inventory, indent=2)+'\n')
    return signatures, prelude


def provider_sources(signatures, prelude):
    header = LayoutText((HERE/'generated/include/silk_shell_input.h').read_text())
    inventory = json.loads(INVENTORY.read_text())
    _, original, _, _ = pinned_sources()
    catalog = declarations(original)
    enum = catalog['SurfaceAction']
    enum = re.sub(r'//[^\n]*', '', enum)
    actions = [name.strip() for name in enum[enum.index('{')+1:enum.rindex('}')].split(',') if name.strip()]
    assert len(actions) == 47
    rs = ['static mut PEER_MASK: u64 = 0;', 'static mut PEER_MESH: bool = false;', 'static mut CALL_STATUS:u64=0;',
          'fn peer_allowed(index: u32) -> bool { unsafe { PEER_MASK & (1u64 << (index % 61)) != 0 } }',
          'fn interaction_tag(s: InteractionState) -> u8 { match s { InteractionState::Idle=>0, InteractionState::ClickPending=>1, InteractionState::Dragging{..}=>2, InteractionState::PanelActive{..}=>3, InteractionState::Resizing{..}=>4, InteractionState::TabDragging{..}=>5 } }',
          'fn hit_tag(t: HitTarget) -> u8 { match t { HitTarget::None=>0, HitTarget::Surface(_)=>1, HitTarget::FrameChrome{..}=>2 } }',
          'const ACTIONS: [SurfaceAction;47] = ['+', '.join('SurfaceAction::'+x for x in actions)+'];']
    c = ['#include <assert.h>', '#include <stdio.h>', '#include <stdlib.h>', '#include <stdarg.h>',
         '#include <string.h>', '#include "silk_shell_input.h"', '#include "silk_shell_surfaces.h"',
         'SilkShellState SILK_SHELL_STATE;',
         'void sexos_fail(void) {_Exit(88);}',
         'const DesktopPolicy P = {.width=1280,.height=720,.bar_height=50,.move_step=10,.resize_step=20,.min_width=120,.min_height=80};',
         'static uint64_t peer_mask,call_status; static bool peer_mesh;',
         'static bool peer_allowed(unsigned index) {return (peer_mask & (UINT64_C(1) << (index % 61))) != 0;}',
         'void sexos_line(const char *format, ...) {va_list ap;va_start(ap,format);vprintf(format,ap);va_end(ap);putchar(10);}',
         'void write_str(const char *s) {fputs(s,stdout);}',
         'void write_u64_dec(uint64_t n) {printf("%llu",(unsigned long long)n);}',
         'void write_debug_lifecycle_state(uint8_t n) {static const char *const names[]={"Allocated","Mapped","Visible","Hidden","Minimized","Closing","Tombstoned","Destroyed"};assert(n<8);fputs(names[n],stdout);}',
         'void write_debug_tombstone_reason(uint8_t n) {static const char *const names[]={"CloseRequested","FocusCleared","DragCancelled","DestroyCommand","FinalDestroy"};assert(n<5);fputs(names[n],stdout);}',
         'void write_debug_resize_edge(uint8_t n) {static const char *const names[]={"Left","Right","Top","Bottom","TopLeft","TopRight","BottomLeft","BottomRight"};assert(n<8);fputs(names[n],stdout);}']
    for index, (name, signature) in enumerate(signatures.items()):
        if name in ('quil_has_focus','spindle_has_focus'):
            continue  # Actual pinned Rust bodies and the checked Cesta bridge are linked below.
        match = re.search(r'\(([^)]*)\)\s*(?:->\s*(.*))?$', signature, re.S)
        parameters = [x.strip() for x in match[1].split(',') if x.strip()]
        result = (match[2] or '()').strip()
        args = [x.split(':',1)[0].strip() for x in parameters]
        types = [x.split(':',1)[1].strip() for x in parameters]
        formats, values = [], []
        for arg, typ in zip(args, types):
            formats.append('{}')
            values.append(arg if typ == '&str' else 'interaction_tag('+arg+') as u64' if typ == 'InteractionState' else
                          'hit_tag('+arg+') as u64' if typ == 'HitTarget' else arg+' as u64')
        rbody = ['println!("peer '+name+(' '+' '.join(formats) if formats else '')+'"'+(' ,'+','.join(values) if values else '')+');']
        rbody.append('let accepted = peer_allowed('+str(index)+');')
        prototype = ' '.join(re.search(r'(?m)^([^\n;{}]*\bshell_input_peer_'+name+r'\(([^;{}]*)\));', header)[1].split())
        cargparts = [x.strip() for x in prototype[prototype.index('(')+1:-1].split(',')]
        cargs = [re.search(r'(\w+)\s*$', x)[1] for x in cargparts]
        cbody = ['(void)ctx;', 'bool accepted=peer_allowed('+str(index)+');', '(void)accepted;']
        fmt, cvalues = [], []
        for arg, typ, cpart, cname in zip(args, types, cargparts[1:], cargs[1:]):
            if typ == '&str':
                fmt.append('%s'); cvalues.append(cname+'->bytes' if 'ShellDiagnosticLabel' in cpart else cname)
            else:
                fmt.append('%llu')
                expression = cname+'.tag' if typ in ('InteractionState','HitTarget') else cname
                cvalues.append('(unsigned long long)'+expression)
        cbody.append('printf("peer '+name+(' '+' '.join(fmt) if fmt else '')+'\\n"'+(','+','.join(cvalues) if cvalues else '')+');')
        cbody.extend('(void)'+name+';' for name in cargs[1:])
        # State-changing peers exercise the interface instead of returning only constants.
        rside, cside = [], []
        if name == 'desktop_shortcut':
            rside=['unsafe { APP_MODIFIERS.observe(code,value); }']
            cside=[]
        if name == 'try_transition': rside=['INTERACTION=next;']; cside=['ctx->shell.input->INTERACTION=next;']
        if name == 'try_set_focus':
            rside=['if accepted {FOCUSED_SURFACE_ID=sid;FOCUS_ID=sid;}']
            cside=['if(accepted){ctx->shell.chrome->FOCUSED_SURFACE_ID=sid;ctx->shell.chrome->FOCUS_ID=sid;}']
        if name == 'atlas_toggle': rside=['ATLAS_MODE_ENABLED=!ATLAS_MODE_ENABLED;']; cside=['ctx->shell.chrome->ATLAS_MODE_ENABLED=!ctx->shell.chrome->ATLAS_MODE_ENABLED;']
        if name == 'toggle_command_palette': rside=['if accepted {COMMAND_PALETTE_OPEN=!COMMAND_PALETTE_OPEN;}']; cside=['if(accepted)ctx->surfaces->COMMAND_PALETTE_OPEN=!ctx->surfaces->COMMAND_PALETTE_OPEN;']
        if name == 'toggle_mesh': rside=['if accepted {PEER_MESH=!PEER_MESH;}']; cside=['if(accepted)peer_mesh=!peer_mesh;']
        if name == 'mesh_select_next_row': rside=['MESH_SELECTED_ROW=(MESH_SELECTED_ROW+1)%8;']; cside=['ctx->surfaces->MESH_SELECTED_ROW=(ctx->surfaces->MESH_SELECTED_ROW+1)%8;']
        if name == 'mesh_select_prev_row': rside=['MESH_SELECTED_ROW=(MESH_SELECTED_ROW+7)%8;']; cside=['ctx->surfaces->MESH_SELECTED_ROW=(ctx->surfaces->MESH_SELECTED_ROW+7)%8;']
        if name == 'linen_select_next_object': rside=['SELECTED_LINEN_OBJECT_ID=SELECTED_LINEN_OBJECT_ID.wrapping_add(1);']; cside=['++ctx->shell.models->SELECTED_LINEN_OBJECT_ID;']
        if name == 'linen_select_prev_object': rside=['SELECTED_LINEN_OBJECT_ID=SELECTED_LINEN_OBJECT_ID.saturating_sub(1);']; cside=['if(ctx->shell.models->SELECTED_LINEN_OBJECT_ID)--ctx->shell.models->SELECTED_LINEN_OBJECT_ID;']
        rbody += rside; cbody += cside
        if result == 'bool':
            rval = 'unsafe {PEER_MESH}' if name == 'mesh_is_visible_in_active_scene' else 'accepted'
            cval = 'peer_mesh' if name == 'mesh_is_visible_in_active_scene' else 'accepted'
            rbody.append(rval); cbody.append('return '+cval+';')
        elif result in ('u8','u64'):
            rbody.append('unsafe {SELECTED_LINEN_OBJECT_ID}' if name == 'linen_selected_object_id' else '8')
            cbody.append('return ctx->shell.models->SELECTED_LINEN_OBJECT_ID;' if name == 'linen_selected_object_id' else 'return 8;')
        elif result == "&'static str": rbody.append('"action"'); cbody.append('return "action";')
        elif result == "(&'static str, u64)":
            rbody.append('("target",31)'); cbody += ['if(out_id)*out_id=31;', 'return "target";']
        elif result == '(HitTarget, bool)':
            rbody.append('(HitTarget::None,accepted)'); cbody += ['if(out_silkbar_handled)*out_silkbar_handled=accepted;', 'return (HitTarget){0};']
        elif result.startswith('Option<'):
            kind = result[7:-1]
            rvalue = {'u64':'777','u32':'3','LifecycleState':'LifecycleState::Visible',
                      'SurfaceAction':'ACTIONS[scancode as usize % 47]',
                      'MeshFact':'MeshFact{fact_id:7,kind:MeshFactKind::ObjectLinkedToBuffer,subject_id:0,object_id:0,ref_id:0,sequence:0}'}[kind]
            rbody.append('if accepted {Some('+rvalue+')} else {None}')
            out = cargs[-1]
            cvalue = {'u64':'777','u32':'3','LifecycleState':'2','SurfaceAction':'(uint8_t)(scancode%47)',
                      'MeshFact':'(MeshFact){.present=true,.fact_id=7}'}[kind]
            cbody += ['if(accepted && '+out+')*'+out+'='+cvalue+';', 'return accepted;']
        else: assert result == '()', (name, result)
        rs.append(signature.replace('const fn','fn')+' { unsafe { '+' '.join(rbody)+' } }')
        c.append(prototype+' { '+' '.join(cbody)+' }')
    # Real descriptor bytes are observed at publication, not host addresses.
    rs.append('''fn pdx_call(slot:u64,op:u64,a:u64,b:u64,c:u64)->(u64,u64) {
        if op==OP_DISPLAY_SET_SNAPSHOT { unsafe {print!("snapshot {}",b);for w in core::slice::from_raw_parts(a as *const WindowDescriptor,b as usize){print!(" {}:{}:{}:{}:{}:{}:{}:{}",w.window_id,w.buffer_handle,w.x,w.y,w.width,w.height,w.z_index,w.focus_state);}println!();} }
        else {println!("call {} {} {} {} {}",slot,op,a,b,c);} (unsafe{CALL_STATUS},0) }
''')
    rs.append('''static mut ACTIVATION_STATUS:[u64;2]=[1;2];
static mut ACTIVATION_TARGET:[u32;2]=[0;2];
static mut ACTIVATION_SURFACE:[u64;2]=[0;2];
static mut ACTIVATION_STATE:[u64;2]=[0;2];
static mut ACTIVATION_FOCUS:Option<u64>=None;
fn observed_activation(app:usize,commit:bool)->Result<app_lifecycle::ActivationInfo,u64>{unsafe{
    println!("activation {} {}",app,commit as u8);
    if let Some(next)=ACTIVATION_FOCUS{FOCUSED_SURFACE_ID=next;}
    if ACTIVATION_STATUS[app]!=0{Err(ACTIVATION_STATUS[app])}else{Ok(app_lifecycle::ActivationInfo{successor:ACTIVATION_TARGET[app],task:700+app as u32,surface:ACTIVATION_SURFACE[app],state:ACTIVATION_STATE[app]})}
}}
fn quil_activation_info(commit:bool)->Result<app_lifecycle::ActivationInfo,u64>{observed_activation(0,commit)}
fn spindle_activation_info(commit:bool)->Result<app_lifecycle::ActivationInfo,u64>{observed_activation(1,commit)}
''')
    c.append('''static uint64_t activation_status[2]={1,1},activation_surface[2],activation_state[2];
static uint32_t activation_target[2];
static bool activation_changes_focus;static uint64_t activation_focus;
ShellActivationResult shell_surfaces_app_activation_info(ShellSurfacesContext *ctx,uint8_t app,bool commit){
    printf("activation %u %u\\n",(unsigned)app,(unsigned)commit);
    if(activation_changes_focus)ctx->shell.chrome->FOCUSED_SURFACE_ID=activation_focus;
    return (ShellActivationResult){activation_status[app],{activation_target[app],700+app,activation_surface[app],activation_state[app]}};
}
''')
    c.append('''SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c) {
        printf("call %llu %llu %llu %llu %llu\\n",(unsigned long long)slot,(unsigned long long)op,(unsigned long long)a,(unsigned long long)b,(unsigned long long)c);return (SexosReply){call_status,0};}
void shell_input_send_snapshot(const SexosWindowDescriptor *p,size_t n) {
        printf("snapshot %zu",n);for(size_t i=0;i<n;++i)printf(" %llu:%llu:%d:%d:%u:%u:%u:%u",(unsigned long long)p[i].window_id,(unsigned long long)p[i].buffer_handle,p[i].x,p[i].y,p[i].width,p[i].height,p[i].z_index,p[i].focus_state);putchar(10);}
''')
    initial = []
    for name, member in inventory['globals'].items():
        text = catalog[name]
        match = re.search(r'\b'+name+r'\s*:\s*(bool|u8|u16|u32|u64|usize|i32|i64)\s*=\s*(.*?);', text, re.S)
        if match:
            value = re.sub(r'(?<=\d)_(?=\d)', '', match[2].strip())
            initial.append('SILK_SHELL_STATE.'+member+'='+value+';')
    for local in inventory['locals']:
        initial.append('SILK_SHELL_STATE.'+local['field']+'='+local['initial']+';')
    c.append('static void initialize(void){memset(&SILK_SHELL_STATE,0,sizeof SILK_SHELL_STATE);atomic_init(&SILK_SHELL_STATE.apps.width,1280);atomic_init(&SILK_SHELL_STATE.apps.height,800);'+' '.join(initial)+'}')
    return '\n'.join(rs), '\n'.join(c)


def drivers():
    inventory, original, _, _ = pinned_sources()
    catalog = declarations(original)
    scalars = []
    for name, field in sorted(inventory['globals'].items()):
        match = re.search(r'\b'+name+r'\s*:\s*(bool|u8|u16|u32|u64|usize|i32|i64)\s*=', catalog[name])
        if match: scalars.append((name, field, match[1]))
    rs = r'''
use std::io::BufRead;
fn life(n:u64)->LifecycleState {match n%8 {0=>LifecycleState::Allocated,1=>LifecycleState::Mapped,2=>LifecycleState::Visible,3=>LifecycleState::Hidden,4=>LifecycleState::Minimized,5=>LifecycleState::Closing,6=>LifecycleState::Tombstoned,_=>LifecycleState::Destroyed}}
fn reason(n:u64)->TombstoneReason {match n%5 {0=>TombstoneReason::CloseRequested,1=>TombstoneReason::FocusCleared,2=>TombstoneReason::DragCancelled,3=>TombstoneReason::DestroyCommand,_=>TombstoneReason::FinalDestroy}}
unsafe fn summary() {
'''
    c = r'''
static ShellInputContext context;
static void summary(void) {
'''
    rs += 'print!("state");\n'; c += 'printf("state");\n'
    for name, field, typ in scalars:
        rs += 'print!(" {}",'+name+' as u64);\n'
        c += 'printf(" %llu",(unsigned long long)SILK_SHELL_STATE.'+field+');\n'
    rs += r'''
    print!(" |interaction {}",interaction_tag(INTERACTION));
    match INTERACTION {InteractionState::Dragging{surface_id,current_x,current_y}=>print!(" {} {} {}",surface_id,current_x,current_y),
      InteractionState::PanelActive{panel}=>print!(" {}",panel as u8),
      InteractionState::Resizing{surface_id,edge,start_x,start_y,start_w,start_h,pending_dx,pending_dy}=>print!(" {} {} {} {} {} {} {} {}",surface_id,edge as u8,start_x,start_y,start_w,start_h,pending_dx,pending_dy),
      InteractionState::TabDragging{frame_id,start_tab,start_x,start_y,current_tab}=>print!(" {} {} {} {} {}",frame_id,start_tab,start_x,start_y,current_tab),_=>{}}
    print!(" |tombs");for entry in TOMBSTONE_RING {if let Some(e)=entry {print!(" {}:{}:{}:{}:{}:{}:{}",e.surface_id,e.generation,e.old_state as u8,e.new_state as u8,e.reason as u8,e.frame_id,e.tab_index);}else {print!(" -");}}
    println!();
}
fn main() {unsafe {
for line in std::io::stdin().lock().lines() {
 let line=line.unwrap();let v:Vec<u64>=line.split_whitespace().map(|x|x.parse().unwrap()).collect();
 assert_eq!(v.len(),5);let(op,a,b,c,d)=(v[0],v[1],v[2],v[3],v[4]);
 println!("command {} {} {} {} {}",op,a,b,c,d);
 match op {
 0=>{PEER_MASK=a;FOCUSED_SURFACE_ID=b;FOCUS_ID=b;COMMAND_PALETTE_OPEN=a&1!=0;ATLAS_MODE_ENABLED=a&2!=0;
   INTERACTION=match c%6 {0=>InteractionState::Idle,1=>InteractionState::ClickPending,2=>InteractionState::Dragging{surface_id:b,current_x:11,current_y:12},
   3=>InteractionState::PanelActive{panel:PanelKind::Launcher},4=>InteractionState::Resizing{surface_id:b,edge:ResizeEdge::Left,start_x:0,start_y:0,start_w:120,start_h:80,pending_dx:3,pending_dy:4},
   _=>InteractionState::TabDragging{frame_id:3,start_tab:0,start_x:0,start_y:0,current_tab:1}};
   WINDOWS.clear();for i in 0..d.min(64) {WINDOWS.push(WindowState{desc:WindowDescriptor{window_id:100+i,buffer_handle:7+i,x:(i*7) as i32,y:(i*9) as i32,width:120,height:80,z_index:0,focus_state:0}});}}
 1=>handle_hid_event(a,b,c),2=>{let(x,y)=apply_rel_pointer(a as i32,b as i32);println!("result {} {}",x,y);},
 3=>process_abs_tablet(a as i32,b as i32),4=>send_cursor_checked(a as i32,b as i32,"test"),5=>flush_pending_cursor_send(),
 6=>println!("result {}",normalize_abs_coord(a as i32,b as i32)),7=>{let(w,h)=clamp_surface_size(a as i32,b as i32,c as u32,d as u32);println!("result {} {}",w,h);},
 8=>{if let Some((x,y,w,h))=get_surface_bounds(a){println!("bounds 1 {} {} {} {}",x,y,w,h);}else{println!("bounds 0");}},
 9=>println!("result {}",point_in_surface(a as i32,b as i32,c) as u8),10=>println!("result {}",surface_is_alive(a) as u8),
 11=>record_tombstone_event(a,life(b),life(c),reason(d)),12=>println!("result {}",is_tombstoned(a) as u8),
 13=>clear_focus_if_dead(),14=>clear_drag_if_dead(),15=>clear_hover_if_dead(),16=>clear_drag_if_wrong_scene(),17=>emit_snapshot(),18=>input_trace_shell_summary(),
 19=>{match a {
'''
    c += r'''
    InteractionState s=SILK_SHELL_STATE.input.INTERACTION;
    printf(" |interaction %u",(unsigned)s.tag);
    if(s.tag==INTERACTION_DRAGGING)printf(" %llu %d %d",(unsigned long long)s.surface_id,s.current_x,s.current_y);
    else if(s.tag==INTERACTION_PANEL_ACTIVE)printf(" %u",(unsigned)s.panel);
    else if(s.tag==INTERACTION_RESIZING)printf(" %llu %u %d %d %u %u %d %d",(unsigned long long)s.surface_id,(unsigned)s.edge,s.start_x,s.start_y,s.start_w,s.start_h,s.pending_dx,s.pending_dy);
    else if(s.tag==INTERACTION_TAB_DRAGGING)printf(" %u %u %d %d %u",s.frame_id,(unsigned)s.start_tab,s.start_x,s.start_y,(unsigned)s.current_tab);
    printf(" |tombs");for(size_t i=0;i<TOMBSTONE_RING_SIZE;++i){TombstoneEvent e=SILK_SHELL_STATE.chrome.TOMBSTONE_RING[i];if(e.present)printf(" %llu:%llu:%u:%u:%u:%u:%u",(unsigned long long)e.surface_id,(unsigned long long)e.generation,(unsigned)e.old_state,(unsigned)e.new_state,(unsigned)e.reason,e.frame_id,(unsigned)e.tab_index);else printf(" -");}putchar(10);
}
int main(void) {
 initialize();context=(ShellInputContext){.shell={.input=&SILK_SHELL_STATE.input,.chrome=&SILK_SHELL_STATE.chrome,.models=&SILK_SHELL_STATE.models,.apps=&SILK_SHELL_STATE.apps,.proofs_a=&SILK_SHELL_STATE.proofs_a},.atlas=&SILK_SHELL_STATE.atlas,.surfaces=&SILK_SHELL_STATE.surfaces};
 unsigned long long raw[5];
 while(scanf("%llu %llu %llu %llu %llu",&raw[0],&raw[1],&raw[2],&raw[3],&raw[4])==5) {
 uint64_t op=raw[0],a=raw[1],b=raw[2],c=raw[3],d=raw[4];
 printf("command %llu %llu %llu %llu %llu\n",raw[0],raw[1],raw[2],raw[3],raw[4]);
 ShellInputContext *p=&context;
 switch(op) {
 case 0:peer_mask=a;SILK_SHELL_STATE.chrome.FOCUSED_SURFACE_ID=b;SILK_SHELL_STATE.chrome.FOCUS_ID=b;
 SILK_SHELL_STATE.surfaces.COMMAND_PALETTE_OPEN=(a&1)!=0;SILK_SHELL_STATE.chrome.ATLAS_MODE_ENABLED=(a&2)!=0;
 SILK_SHELL_STATE.input.INTERACTION=(InteractionState){.tag=(uint8_t)(c%6),.surface_id=b,.current_x=11,.current_y=12,
 .edge=0,.start_w=120,.start_h=80,.pending_dx=3,.pending_dy=4,.frame_id=3,.start_tab=0,.current_tab=1};
 SILK_SHELL_STATE.chrome.WINDOWS_len=d<64?(size_t)d:64;
 for(size_t i=0;i<SILK_SHELL_STATE.chrome.WINDOWS_len;++i)SILK_SHELL_STATE.chrome.WINDOWS[i]=(WindowState){.desc={.window_id=100+i,.buffer_handle=7+i,.x=(int32_t)(i*7),.y=(int32_t)(i*9),.width=120,.height=80}};
 break;
 case 1:shell_input_handle_hid_event(p,a,b,c);break;
 case 2:{int32_t x=0,y=0;shell_input_apply_rel_pointer(p,(int32_t)a,(int32_t)b,&x,&y);printf("result %d %d\n",x,y);break;}
 case 3:shell_input_process_abs_tablet(p,(int32_t)a,(int32_t)b);break;
 case 4:shell_input_send_cursor_checked(p,(int32_t)a,(int32_t)b,"test");break;
 case 5:shell_input_flush_pending_cursor_send(p);break;
 case 6:printf("result %d\n",shell_input_normalize_abs_coord(p,(int32_t)a,(int32_t)b));break;
 case 7:{uint32_t w=0,h=0;shell_input_clamp_surface_size(p,(int32_t)a,(int32_t)b,(uint32_t)c,(uint32_t)d,&w,&h);printf("result %u %u\n",w,h);break;}
 case 8:{int32_t x=0,y=0;uint32_t w=0,h=0;if(shell_input_get_surface_bounds(p,a,&x,&y,&w,&h))printf("bounds 1 %d %d %u %u\n",x,y,w,h);else puts("bounds 0");break;}
 case 9:printf("result %u\n",(unsigned)shell_input_point_in_surface(p,(int32_t)a,(int32_t)b,c));break;
 case 10:printf("result %u\n",(unsigned)shell_input_surface_is_alive(p,a));break;
 case 11:shell_input_record_tombstone_event(p,a,(uint8_t)(b%8),(uint8_t)(c%8),(uint8_t)(d%5));break;
 case 12:printf("result %u\n",(unsigned)shell_input_is_tombstoned(p,a));break;
 case 13:shell_input_clear_focus_if_dead(p);break;
 case 14:shell_input_clear_drag_if_dead(p);break;
 case 15:shell_input_clear_hover_if_dead(p);break;
 case 16:shell_input_clear_drag_if_wrong_scene(p);break;
 case 17:shell_input_emit_snapshot(p);break;
 case 18:shell_input_input_trace_shell_summary(p);break;
 case 19:switch(a) {
'''
    casts = {'bool':'bool','u8':'uint8_t','u16':'uint16_t','u32':'uint32_t','u64':'uint64_t','usize':'size_t','i32':'int32_t','i64':'int64_t'}
    for index, (name, field, typ) in enumerate(scalars):
        value = 'b%16' if name == 'TOMBSTONE_RING_NEXT' else 'b'
        rs += str(index)+'=>'+name+'='+('b!=0' if typ=='bool' else '('+value+') as '+typ)+',\n'
        c += 'case '+str(index)+':SILK_SHELL_STATE.'+field+'=('+casts[typ]+')'+(' (b!=0)' if typ=='bool' else '('+value+')')+';break;\n'
    rs += '_=>panic!("unknown field")}},20=>{TOMBSTONE_RING_NEXT=a as usize;record_tombstone_event(100,life(1),life(2),reason(0));},_=>panic!("unknown operation")};summary();}}}\n'
    c += 'default:abort();}break;case 20:SILK_SHELL_STATE.chrome.TOMBSTONE_RING_NEXT=(size_t)a;shell_input_record_tombstone_event(p,100,1,2,0);break;default:abort();}summary();}return 0;}\n'
    rs = rs.replace('    print!(" |tombs");', r'''
    print!(" |desktop {} {} {}",DESKTOP_WIDTH.load(Ordering::Acquire),DESKTOP_HEIGHT.load(Ordering::Acquire),CALL_STATUS);
    print!(" |apps");for entry in APP_CUSTODY{if let Some(e)=entry{let(x,y,w,h)=e.bounds.unwrap_or((0,0,0,0));print!(" {}:{}:{}:{}:{}:{}:{}:{}:{}:{}",e.target,e.task,e.surface,e.dead as u8,e.close_pending as u8,e.bounds.is_some()as u8,x,y,w,h);}else{print!(" -");}}
    print!(" |tombs");''')
    c = c.replace('    printf(" |tombs");', r'''
    printf(" |desktop %u %u %llu",atomic_load_explicit(&SILK_SHELL_STATE.apps.width,memory_order_acquire),atomic_load_explicit(&SILK_SHELL_STATE.apps.height,memory_order_acquire),(unsigned long long)call_status);
    printf(" |apps");for(size_t i=0;i<SHELL_APP_COUNT;++i){ShellAppCustody e=SILK_SHELL_STATE.apps.entries[i];if(SILK_SHELL_STATE.apps.present[i])printf(" %u:%u:%llu:%u:%u:%u:%d:%d:%u:%u",e.target,e.task,(unsigned long long)e.surface,(unsigned)e.dead,(unsigned)e.close_pending,(unsigned)e.has_bounds,e.has_bounds?e.x:0,e.has_bounds?e.y:0,e.has_bounds?e.w:0,e.has_bounds?e.h:0);else printf(" -");}
    printf(" |tombs");''')
    extra_rs = r'''
21=>CALL_STATUS=a,
22=>{DESKTOP_WIDTH.store(a as u32,Ordering::Release);DESKTOP_HEIGHT.store(b as u32,Ordering::Release);},
23=>{let mut entry=app_custody::Entry::new(app_lifecycle::SLOTS[a as usize],b as u32,900+a as u32,c);entry.dead=d&1!=0;entry.close_pending=d&2!=0;if d&4!=0{entry.bounds=Some((10,20,120,80));}APP_CUSTODY[a as usize]=Some(entry);},
24=>{let entry=APP_CUSTODY[a as usize].as_mut().unwrap();entry.bounds=Some((b as i32,c as i32,d as u32,(d>>32)as u32));},
25=>APP_CUSTODY[a as usize]=None,
26=>{let row=app_catalog::INCARNATIONS[d as usize];ACTIVATION_STATUS[a as usize]=b;ACTIVATION_STATE[a as usize]=c;ACTIVATION_TARGET[a as usize]=row.target;ACTIVATION_SURFACE[a as usize]=row.surface;},
27=>ACTIVATION_FOCUS=if a!=0{Some(b)}else{None},
28=>println!("result {}",if a==0{quil_has_focus()}else{spindle_has_focus()}as u8),
'''
    extra_c = r'''
case 21:call_status=a;break;
case 22:atomic_store_explicit(&SILK_SHELL_STATE.apps.width,(uint32_t)a,memory_order_release);atomic_store_explicit(&SILK_SHELL_STATE.apps.height,(uint32_t)b,memory_order_release);break;
case 23:{if(a>=SHELL_APP_COUNT)abort();ShellAppCatalogRow row=shell_custody_catalog_at(a);ShellAppCustody entry=shell_custody_new(row.lifecycle_slot,(uint32_t)b,(uint32_t)(900+a),c);entry.dead=(d&1)!=0;entry.close_pending=(d&2)!=0;entry.has_bounds=(d&4)!=0;entry.x=10;entry.y=20;entry.w=120;entry.h=80;SILK_SHELL_STATE.apps.entries[a]=entry;SILK_SHELL_STATE.apps.present[a]=true;break;}
case 24:if(a>=SHELL_APP_COUNT || !SILK_SHELL_STATE.apps.present[a])abort();SILK_SHELL_STATE.apps.entries[a].has_bounds=true;SILK_SHELL_STATE.apps.entries[a].x=(int32_t)b;SILK_SHELL_STATE.apps.entries[a].y=(int32_t)c;SILK_SHELL_STATE.apps.entries[a].w=(uint32_t)d;SILK_SHELL_STATE.apps.entries[a].h=(uint32_t)(d>>32);break;
case 25:if(a>=SHELL_APP_COUNT)abort();SILK_SHELL_STATE.apps.present[a]=false;break;
case 26:{if(a>=2 || d>=SHELL_APP_COUNT)abort();ShellAppCatalogRow row=shell_custody_catalog_at(d);activation_status[a]=b;activation_state[a]=c;activation_target[a]=row.target;activation_surface[a]=row.surface;break;}
case 27:activation_changes_focus=a!=0;activation_focus=b;break;
case 28:printf("result %u\n",(unsigned)(a==0?shell_input_peer_quil_has_focus(p):shell_input_peer_spindle_has_focus(p)));break;
'''
    rs = rs.replace('_=>panic!("unknown operation")',extra_rs+'_=>panic!("unknown operation")')
    c = c.replace('default:abort();}summary();',extra_c+'default:abort();}summary();')
    return rs, c, scalars


def vectors(scalars, seeds, cases):
    result = []
    def add(op, a=0, b=0, c=0, d=0):
        result.append(tuple(int(x) & ((1<<64)-1) for x in (op,a,b,c,d)))
    # Every function, with both accepted and rejected geometry/lifetime cases.
    add(0,0,100,0,3)
    for op in range(1,19):
        if op == 1: add(op,2,1,2)
        elif op == 6: add(op,15000,1280)
        elif op == 7: add(op,12,17,400,200)
        elif op in (8,10,12):
            for sid in (0,100,101,102,103,200,201,202,203,204,205,0x99,0xfe,999):add(op,sid)
        elif op == 9:
            for sid in (100,101,200,205,0x99,0xfe,999):add(op,200,100,sid)
        elif op == 11:
            for i in range(20):add(op,100+i%5,i%8,(i+1)%8,i%5)
        else:add(op,17,-3)
    fields = {name:i for i,(name,_,_) in enumerate(scalars)}
    for field in ('INPUT_TRACE_SEQ','INPUT_TRACE_APPLIES','SHELL_APPLY_TICK','CURSOR_SEND_COUNT'):
        add(19,fields[field],(1<<32)-2)
    for i in range(140):add(2,(-1 if i%2 else 1)*i,(i%7)-3)
    for mask in (0,(1<<61)-1,0x1555555555555555):
        for sid in (0,100,200,201,202,203,204,0x99,999):
            for interaction in range(6):
                add(0,mask,sid,interaction,24)
                for code in (0x1d,0x43,0x1c,0x01,0x29,0x24,0x25,0x39,0x4b,0x4d,0x48,0x50,0x1e,0x20,0x57,0x0e,0x1a,0x1b,0x44):
                    add(1,1,code,1);add(1,1,code,0)
                for button in (0,1,2,7,8,255):add(1,4,button,1);add(1,4,button,0)
                for raw_x,raw_y in ((0,0),(32767,32767),(15000,16000)):
                    add(1,3,raw_x,raw_y)
                add(1,99,0,1);add(13);add(14);add(15);add(16)
    boundary = [0,1,2,3,16,17,127,32766,32767,2**31-1,2**31,2**32-1,-1,-127,-2**31]
    for seed in range(seeds):
        rng=random.Random(0x51C0A+seed)
        for _ in range(cases):
            op=rng.randrange(20)
            if op==0:add(op,rng.getrandbits(61),rng.choice([100,101,200,201,202,203,204,0x99,999]),rng.randrange(6),rng.randrange(65))
            elif op==1:add(op,rng.choice([1,2,3,4,99]),rng.choice(boundary),rng.choice([0,1,2,255]))
            elif op==6:add(op,rng.choice(boundary),rng.choice([1,2,120,1280,65536]))
            elif op==7:add(op,rng.choice(boundary),rng.choice(boundary),rng.getrandbits(32),rng.getrandbits(32))
            elif op==19:
                name=rng.choice(list(fields));add(op,fields[name],rng.getrandbits(32))
            else:add(op,rng.choice(boundary),rng.choice(boundary),rng.choice([0,100,200,0x99,999]),rng.randrange(5))
    for width,height in ((1,1),(320,240),(1280,800),(2147483647,2147483647)):
        add(22,width,height)
        for status in (0,1,(1<<64)-2,(1<<64)-1):
            add(21,status);add(4,-17,2147483647);add(5);add(2,4,7);add(5)
            add(4,123,89);add(4,124,90);add(5);add(21,0);add(5)
    add(21,0);add(22,1280,800)
    for width,height in ((0x80000000,240),(320,0x80000000)):
        add(22,width,height);add(4,10,20)
    for raw in (1,32766,32767):add(6,raw,0x80000000)
    add(22,1280,800)
    catalog=(HERE/'interfaces/shell_apps.data').read_text().split('(Table BootLifecycle')[0]
    for index,text in enumerate(re.findall(r'\(Row ([^\n)]+)\)',catalog)):
        row=text.split();target,surface,frame=map(int,row[2:5])
        for flags in range(8):
            for allowed in (0,(1<<61)-1):
                add(0,allowed,surface,0,3);add(23,index,target,surface,flags)
                for sid in sorted({surface,frame}):
                    add(8,sid);add(10,sid)
                    for x,y in ((9,20),(10,20),(129,99),(130,100),(-1,-1)):
                        add(9,x,y,sid)
                add(25,index)
        add(0,(1<<61)-1,surface,0,3);add(23,index,target,surface,4)
        add(24,index,(1<<31)-10,(1<<31)-10,(0xffffffff<<32)|0xffffffff)
        add(9,(1<<31)-1,(1<<31)-1,surface);add(25,index)
        app=int(row[0])
        if app<2:
            for status in (0,1,(1<<64)-1):
                for state in (0,1,2,3):
                    add(26,app,status,state,index)
                    for focused in (0,surface,frame):
                        add(0,0,focused,0,3)
                        for code in (1,0x1c,0x1e,0x39,0x51):add(1,1,code,1)
            add(26,app,1,0,index)
    for app,index,focused in ((0,3,301),(1,4,306)):
        add(26,app,0,2,index)
        for before,after in ((0,focused),(focused,0)):
            add(0,0,before,0,3);add(27,1,after);add(28,app)
        add(27,0,0);add(26,app,1,0,index)
    return (''.join(' '.join(map(str,row))+'\n' for row in result).encode(),len(result))


def abi_provider(provider):
    """Run the real checked-to-legacy adapters against the same scripted peers."""
    text=provider
    public=(HERE/'generated/include/silk_shell.h').read_text()
    pattern=r'(?m)^([^\n;{}]*\bshell_input_peer_(\w+)\([^\n;{}]*\))\s*\{'
    functions=list(re.finditer(pattern,text))
    checked=set(re.findall(r'\bshell_input_peer_(\w+)\(', (HERE/'src/silk_shell_input_diagnostics.cesta').read_text()))
    binding='ShellInputContext storage={.shell={.input=&SILK_SHELL_STATE.input,.chrome=&SILK_SHELL_STATE.chrome,.models=&SILK_SHELL_STATE.models,.apps=&SILK_SHELL_STATE.apps,.proofs_a=&SILK_SHELL_STATE.proofs_a},.atlas=&SILK_SHELL_STATE.atlas,.surfaces=&SILK_SHELL_STATE.surfaces};ShellInputContext *ctx=&storage;(void)ctx;'
    for match in reversed(functions):
        name=match[2]
        if name in checked:
            continue  # These boundaries have their own real checked-bridge runtime gate.
        original=[' '.join(x.split()) for x in re.findall(r'(?m)^([^\n;{}]*\b'+name+r'\([^;{}]*\));',public)]
        assert len(original)==1,name
        end=item_end(text,match.end()-1)
        body=text[match.end():end-1]
        assert body.count('(void)ctx;')==1
        body=body.replace('(void)ctx;',binding)
        text=text[:match.start()]+original[0]+' {'+body+'}'+text[end:]
    text=text.replace('void shell_input_send_snapshot(', 'void fixture_snapshot(')
    text=text.replace('SexosReply sexos_pdx_call(', 'void fixture_snapshot(const SexosWindowDescriptor *,size_t);\nSexosReply sexos_pdx_call(',1)
    needle='uint64_t c) {\n        printf("call '
    assert needle in text
    text=text.replace(needle,'uint64_t c) {\n        if(op==OP_DISPLAY_SET_SNAPSHOT){fixture_snapshot((const SexosWindowDescriptor *)(uintptr_t)a,(size_t)b);return (SexosReply){0};}\n        printf("call ',1)
    text=re.sub(r'\bshell_input_(?!peer_)(\w+)\(p,',r'\1(',text)
    text=re.sub(r'\bshell_input_(?!peer_)(\w+)\(p\)',r'\1()',text)
    text=text.replace('ShellInputContext *p=&context;', 'ShellInputContext *p=&context;(void)p;')
    text=text.replace('int main(void) {','int main(int argc,char **argv) {(void)argv;')
    needle=' unsigned long long raw[5];'
    assert needle in text
    text=text.replace(needle,' if(argc>1){ShellInputContext bad=context;SilkShellInputState local={0};bad.shell.input=&local;(void)shell_input_peer_action_name(&bad,0);return 90;}\n'+needle)
    return text


def compare(expected, actual, work, label):
    if expected==actual:return
    (work/(label+'.reference.out')).write_bytes(expected)
    (work/(label+'.candidate.out')).write_bytes(actual)
    left,right=expected.splitlines(),actual.splitlines()
    index=next((i for i,(a,b) in enumerate(zip(left,right)) if a!=b),min(len(left),len(right)))
    raise AssertionError((label,'first difference',index+1,left[index:index+1],right[index:index+1]))


def invalid_cursor(binary, expected):
    for index in (16,(1<<64)-1):
        result=subprocess.run([binary],input=f'20 {index} 0 0 0\n'.encode(),capture_output=True)
        assert result.returncode==expected,('invalid cursor was not trapped',binary,index,result.returncode,result.stderr[-1000:])
        if expected==101:assert b'index out of bounds' in result.stderr
        else:assert b'runtime error:' not in result.stderr and b'AddressSanitizer' not in result.stderr
    return 2


def invalid_geometry(binary, expected):
    cases=[f'22 {width} {height} 0 0\n4 10 20 0 0\n'for width,height in ((0,240),(0xffffffff,240),(320,0),(320,0xffffffff))]
    cases+=['6 1 0 0 0\n','6 1 4294967295 0 0\n']
    for data in cases:
        result=subprocess.run([binary],input=data.encode(),capture_output=True)
        assert result.returncode==expected,('invalid dimensions were not rejected',binary,data,result.returncode,result.stderr[-1000:])
        if expected==101:assert b'min > max'in result.stderr
        else:assert b'runtime error:'not in result.stderr and b'AddressSanitizer'not in result.stderr
    return len(cases)


def independent(binary, scalars):
    fields={name:index for index,(name,_,_) in enumerate(scalars)}
    count=0
    def observe(commands):
        nonlocal count
        data=(''.join(' '.join(str(int(x)&((1<<64)-1)) for x in (*row,*([0]*(5-len(row)))))+'\n' for row in commands)).encode()
        text=run([binary],data=data).decode()
        state=[line for line in text.splitlines()if line.startswith('state ')][-1].split(' |')[0].split()[1:]
        count+=1
        return {name:int(state[index])for name,index in fields.items()},text
    state,text=observe([(22,320,240),(4,-10,999)])
    assert (state['POINTER_X'],state['POINTER_Y'],state['CURSOR_SEND_COUNT'],state['CURSOR_SEND_PENDING'])==(0,239,1,0)
    state,text=observe([(19,fields['CURSOR_APPLIES_SINCE_SEND'],7),(21,(1<<64)-2),(4,10,20),(4,90,70),(21,0),(5,)])
    calls=[line for line in text.splitlines()if line.startswith('call ')]
    assert calls==['call 5 235 144 10 20','call 5 235 144 90 70','call 5 235 144 90 70'],calls
    assert (state['CURSOR_SEND_PENDING'],state['CURSOR_SEND_COUNT'],state['CURSOR_APPLIES_SINCE_SEND'])==(0,1,0)
    for status in (1,(1<<64)-1):
        state,text=observe([(19,fields['CURSOR_APPLIES_SINCE_SEND'],7),(21,status),(4,10,20),(5,)])
        assert len([line for line in text.splitlines()if line.startswith('call ')])==1
        assert (state['CURSOR_SEND_PENDING'],state['CURSOR_SEND_COUNT'],state['CURSOR_APPLIES_SINCE_SEND'])==(0,0,7)
        assert '[shell.cursor.rejected]'in text
    catalog=(HERE/'interfaces/shell_apps.data').read_text().split('(Table BootLifecycle')[0]
    rows=[text.split()for text in re.findall(r'\(Row ([^\n)]+)\)',catalog)]
    for index in (0,1,2,4):
        row=rows[index];target,surface=map(int,row[2:4])
        for flags in range(8):
            _,text=observe([(0,(1<<61)-1,surface,0,3),(23,index,target,surface,flags),(8,surface),(10,surface),(9,10,20,surface)])
            accepted=bool(flags&4)and not flags&3
            bounds=[line for line in text.splitlines()if line.startswith('bounds ')]
            result=[line for line in text.splitlines()if line.startswith('result ')]
            assert bounds==(['bounds 1 10 20 120 80']if accepted else['bounds 0']),text[-1500:]
            assert result==['result '+str(int(accepted))]*2,text[-1500:]
    _,text=observe([(0,(1<<61)-1,302,0,3),(23,4,17,302,4),(24,4,(1<<31)-10,(1<<31)-10,0xffffffffffffffff),(9,(1<<31)-1,(1<<31)-1,302)])
    assert 'result 1\n'in text
    _,text=observe([(0,1,0,0,3),(1,1,0x1c,1)])
    assert 'peer palette_execute_selected'in text and 'peer toggle_command_palette'not in text,text[-1500:]
    header=LayoutText((HERE/'generated/include/silk_shell_input.h').read_text())
    peers=re.findall(r'\bshell_input_peer_(\w+)\(',header)
    spindle_text=1<<(peers.index('is_spindle_text_key')%61)
    _,text=observe([(0,spindle_text,0x99,0,3),(1,1,0x2a,1),(1,1,0x12,1),
                    (1,1,0x2a,0),(1,1,0x13,1)])
    spindle_calls=[line for line in text.splitlines()if line.startswith('call 14 514 ')]
    assert any(line.startswith('call 14 514 274 1 1')for line in spindle_calls),spindle_calls
    assert any(line.startswith('call 14 514 19 1 1')for line in spindle_calls),spindle_calls
    life=1<<(peers.index('surface_is_lifecycle_live')%61)
    for app,index,focused,slot in ((0,3,301,11),(1,4,306,14)):
        row=rows[index];target,surface=map(int,row[2:4])
        _,text=observe([(26,app,0,2,index),(0,life,focused,0,3),(23,index,target,surface,4),(1,1,128,1)])
        assert f'call {slot} 514 128 1 1\n'in text,text[-2000:]
        for before,after,want in ((0,focused,1),(focused,0,0)):
            _,text=observe([(26,app,0,2,index),(0,0,before,0,3),(27,1,after),(28,app)])
            assert [line for line in text.splitlines()if line.startswith('result ')]==['result '+str(want)],text[-2000:]
    return count


def mutations(work, provider, exported, data, expected, native):
    original=LayoutText(exported.read_text())
    changes=[
        ('relative_gain', ' : 18;\n    return raw < 0', ' : 17;\n    return raw < 0'),
        ('absolute_edge', 'if (raw >= TABLET_RAW_MAX) return input_wrap_sub(screen_dim,1);', 'if (raw >= TABLET_RAW_MAX) return input_wrap_sub(screen_dim,2);'),
        ('pending_cursor', 'CURSOR_SEND_PENDING = true;', 'CURSOR_SEND_PENDING = false;'),
        ('tombstone_ring', 'TOMBSTONE_RING_NEXT = (index + 1) % TOMBSTONE_RING_SIZE;', 'TOMBSTONE_RING_NEXT = (index + 2) % TOMBSTONE_RING_SIZE;'),
        ('tombstone_identity', 'TOMBSTONE_RING[i].surface_id == sid) return true;', 'TOMBSTONE_RING[i].surface_id != sid) return true;'),
        ('snapshot_order', '!= (pass == 1)) continue;', '!= (pass == 0)) continue;'),
        ('snapshot_depth', 'SNAPSHOT[count].z_index = (uint32_t)count;', 'SNAPSHOT[count].z_index = (uint32_t)count + 1;'),
        ('snapshot_focus', 'SNAPSHOT[count].focus_state = WINDOWS[i].desc.window_id == focused ? 1 : 0;', 'SNAPSHOT[count].focus_state = WINDOWS[i].desc.window_id == focused ? 0 : 1;'),
        ('button_mask', 'button > 0 ? (unsigned)button - 1 : 0;', 'button > 0 ? (unsigned)button : 0;'),
        ('surface_liveness', 'if (sid == SURFACE_ID_APP) return SURFACE_100_ALIVE;', 'if (sid == SURFACE_ID_APP) return !SURFACE_100_ALIVE;'),
        ('full_loses_latest', 'CURSOR_SEND_PENDING=admission.status==ERR_SERVICE_NOT_READY;', 'CURSOR_SEND_PENDING=false;'),
        ('fatal_retried', 'CURSOR_SEND_PENDING=admission.status==ERR_SERVICE_NOT_READY;', 'CURSOR_SEND_PENDING=true;'),
        ('count_rejected_send', 'if(admission.status){', 'if(false){'),
        ('failed_send_resets_applies', 'if(admission.status){', 'if(admission.status){CURSOR_APPLIES_SINCE_SEND=0;'),
        ('legacy_width', 'input_clamp(x, 0, input_wrap_sub(input_dimension(&ctx->shell.apps->width),1))', 'input_clamp(x, 0, P.width-1)'),
        ('legacy_height', 'input_clamp(y, 0, input_wrap_sub(input_dimension(&ctx->shell.apps->height),1))', 'input_clamp(y, 0, P.height-1)'),
        ('pending_custody_accepted', 'if((entry.dead || entry.close_pending) && shell_custody_owns_surface(&entry,sid))return false;', 'if(entry.dead && shell_custody_owns_surface(&entry,sid))return false;'),
        ('missing_custody_bounds', 'if(entry.dead || !entry.has_bounds)return false;', 'if(entry.dead)return false;'),
        ('narrow_successor_bounds', '(int64_t)px<(int64_t)x+w && (int64_t)py<(int64_t)y+h', 'px<input_wrap_add(x,input_i32(w)) && py<input_wrap_add(y,input_i32(h))'),
        ('failed_palette_closes', 'if(shell_input_peer_palette_execute_selected(ctx))shell_input_peer_toggle_command_palette(ctx);', '(void)shell_input_peer_palette_execute_selected(ctx);shell_input_peer_toggle_command_palette(ctx);'),
        ('shortcut_bypassed', 'if(event_class==EV_KEY && shell_input_peer_desktop_shortcut(ctx,code,value))return;', 'if(false && shell_input_peer_desktop_shortcut(ctx,code,value))return;'),
        ('quil_successor_lost', 'shell_input_peer_quil_has_focus(ctx)', '(FOCUSED_SURFACE_ID==SURFACE_ID_QUIL)'),
        ('spindle_successor_lost', 'shell_input_peer_spindle_has_focus(ctx)', '(FOCUSED_SURFACE_ID==SURFACE_ID_SPINDLE)'),
        ('spindle_shift_bit', 'ctx->shell.input->APP_MODIFIER_RIGHT_SHIFT) << 8, value, EV_KEY)',
         'ctx->shell.input->APP_MODIFIER_RIGHT_SHIFT) << 7, value, EV_KEY)'),
        ('spindle_left_shift_lost', 'if(code==0x2a)ctx->shell.input->APP_MODIFIER_LEFT_SHIFT=held;',
         'if(code==0x2a)ctx->shell.input->APP_MODIFIER_LEFT_SHIFT=false;'),
        ('invalid_clamp_accepted','if(low>high){sexos_fail();for(;;){}}',''),
    ]
    results=[]
    for name,old,new in changes:
        wanted=2 if name in ('pending_custody_accepted','quil_successor_lost','spindle_successor_lost')else 1
        assert original.count(old)==wanted,(name,'mutation anchor',original.count(old))
        source=work/(name+'.c');source.write_text(original.replace(old,new))
        binary=work/name
        run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include',provider,source,*native,'-o',binary])
        actual=run([binary],data=data)
        rejected=actual!=expected
        if not rejected:
            try:independent(binary,drivers()[2]);invalid_geometry(binary,88)
            except AssertionError:rejected=True
        assert rejected,('semantic mutation survived',name)
        results.append(name)
    guard='if (index >= TOMBSTONE_RING_SIZE) { sexos_fail(); for (;;) {} }'
    assert original.count(guard)==1
    source=work/'missing_cursor_guard.c';source.write_text(original.replace(guard,''))
    binary=work/'missing_cursor_guard'
    run(['gcc','-std=c11','-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all',
         '-Igenerated/include',provider,source,*native,'-o',binary])
    try:invalid_cursor(binary,88)
    except AssertionError:pass
    else:raise AssertionError('missing cursor guard survived')
    results.append('missing_cursor_guard')
    signed=work/'signed_dimension_overflow.c'
    signed.write_text(original.replace('input_wrap_sub(input_dimension(&ctx->shell.apps->width),1)','(input_dimension(&ctx->shell.apps->width)-1)'))
    binary=work/'signed_dimension_overflow'
    run(['gcc','-std=c11','-O1','-fsanitize=undefined','-fno-sanitize-recover=all','-Igenerated/include',provider,signed,*native,'-o',binary])
    broken=subprocess.run([binary],input=b'22 2147483648 240 0 0\n4 10 20 0 0\n',capture_output=True)
    assert broken.returncode!=0 and b'overflow'in broken.stderr,('signed dimension mutation survived',broken.stderr)
    results.append('signed_dimension_overflow')
    app_source=native[-1]
    assert app_source.name=='silk_shell_input_apps.c'
    apps=LayoutText(app_source.read_text())
    for name,mutated in (
        ('focus_before_query',apps.replace('if(ctx->shell.chrome->FOCUSED_SURFACE_ID==','uint64_t cached=ctx->shell.chrome->FOCUSED_SURFACE_ID;\n    if(ctx->shell.chrome->FOCUSED_SURFACE_ID==').replace('uint64_t focused=ctx->shell.chrome->FOCUSED_SURFACE_ID;','uint64_t focused=cached;')),
        ('activation_error_ignored',apps.replace('if(result.status || result.info.state!=2)return false;','if(result.info.state!=2)return false;')),
        ('spindle_frame_ignored',apps.replace('(row.shell_surface?row.shell_surface:row.surface)','row.surface')),
        ('quil_focus_uses_domain',apps.replace('return focused==result.info.surface;','return focused==result.info.successor;')),
    ):
        assert mutated!=apps,name
        path=work/(name+'.c');path.write_text(mutated);binary=work/name
        run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include',provider,exported,*native[:-1],path,'-o',binary])
        assert run([binary],data=data)!=expected,('app focus mutation survived',name)
        results.append(name)
    return results


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--work', type=Path)
    parser.add_argument('--seeds',type=int,default=4)
    parser.add_argument('--cases',type=int,default=256)
    parser.add_argument('--quick',action='store_true')
    parser.add_argument('--mutations',action='store_true')
    parser.add_argument('--all-profiles',action='store_true')
    args = parser.parse_args()
    inputs=[HERE/'src/silk_shell_input.cesta',HERE/'src/silk_shell_input_adapter.cesta',
            HERE/'src/silk_shell_input_apps.cesta',HERE/'src/silk_shell_chrome_atomic.cesta',HERE/'src/silk_shell_custody.cesta',
            INVENTORY,*sorted((HERE/'generated/include').glob('*.h'))]
    frozen={path:path.read_bytes() for path in inputs}
    work = args.work or Path(tempfile.mkdtemp(prefix='silk-shell-input-', dir=HERE/'build'))
    work.mkdir(parents=True, exist_ok=True)
    signatures, prelude = prepare(work)
    if args.prepare_only:
        print('pinned input reference prepared:', len(signatures), 'scripted peer boundaries')
        return
    rs_provider, c_provider = provider_sources(signatures, prelude)
    rs_driver, c_driver, scalars = drivers()
    reference = work/'reference.rs'
    reference.write_text(prelude+'\n'+rs_provider+'\n'+rs_driver)
    provider = work/'provider.c'
    provider.write_text(c_provider+'\n'+c_driver)
    adapted_provider=work/'adapted_provider.c'
    adapted_provider.write_text(abi_provider(provider.read_text()))
    exported = work/'silk_shell_input.c'
    run([args.cesta,'export-c','src/silk_shell_input.cesta','--std=c11','-o',exported,'--','-Igenerated/include'])
    adapter=work/'silk_shell_input_adapter.c'
    run([args.cesta,'export-c','src/silk_shell_input_adapter.cesta','--std=c11','-o',adapter,'--','-Igenerated/include'])
    native=[]
    for name in ('silk_shell_chrome_atomic','silk_shell_custody','silk_shell_input_apps'):
        out=work/(name+'.c');run([args.cesta,'export-c','src/'+name+'.cesta','--std=c11','-o',out,'--','-Igenerated/include']);native.append(out)
    data,commands=vectors(scalars,args.seeds,args.cases)
    (work/'vectors.txt').write_bytes(data)
    feature_names=['SEXOS_KEYBOARD_CURSOR','SEXOS_SHELL_INTERACTION_CONTRACT_PROOF','SEXOS_INPUT_NEGATIVE_PROOF','SEXOS_CLICK_DRAG_PROOF']
    profiles=[[],feature_names,[feature_names[0]],[feature_names[1]]]
    if args.all_profiles:
        profiles=[[name for name,enabled in zip(feature_names,bits) if enabled]
                  for bits in itertools.product((False,True),repeat=len(feature_names))]
    rows=[]
    mutation_results=[]
    cursor_controls=0
    independent_controls=0
    for profile,features in enumerate(profiles[:1] if args.quick else profiles):
        environment=dict(os.environ)
        for name in feature_names:environment.pop(name,None)
        environment.update({name:'1' for name in features})
        ref=work/('reference-'+str(profile))
        run(['rustc','--edition=2021','-O','--cap-lints','allow',reference,'-o',ref],env=environment)
        expected=run([ref],data=data)
        cursor_controls+=invalid_cursor(ref,101)
        cursor_controls+=invalid_geometry(ref,101)
        if profile==0 and args.mutations:
            mutation_results=mutations(work,provider,exported,data,expected,native)
        for compiler in ('gcc',) if args.quick else ('gcc','clang'):
            for mode in ('O2',) if args.quick else ('O0','O2','sanitize'):
                options=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
                flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include',*['-D'+name for name in features],*options]
                binary=work/f'{profile}-{compiler}-{mode}'
                run([compiler,*flags,provider,exported,*native,'-o',binary])
                actual=run([binary],data=data)
                compare(expected,actual,work,binary.name)
                cursor_controls+=invalid_cursor(binary,88)
                cursor_controls+=invalid_geometry(binary,88)
                if mode=='O2':independent_controls=independent(binary,scalars)
                adapted=work/(binary.name+'-abi')
                run([compiler,*flags,adapted_provider,exported,adapter,*native,'-o',adapted])
                compare(expected,run([adapted],data=data),work,adapted.name)
                cursor_controls+=invalid_cursor(adapted,88)
                cursor_controls+=invalid_geometry(adapted,88)
                invalid=subprocess.run([adapted,'bad-binding'],capture_output=True)
                assert invalid.returncode==88,('context binding escaped',invalid.returncode)
                if mode=='O2':
                    authored=work/(binary.name+'-authored')
                    run([args.cesta,'cc','--cc',compiler,*flags,provider,'src/silk_shell_input.cesta',*native,'-o',authored])
                    assert binary.read_bytes()==authored.read_bytes(),('authored/export identity',binary.name)
                    compare(expected,run([authored],data=data),work,authored.name)
                    cursor_controls+=invalid_cursor(authored,88)
                    cursor_controls+=invalid_geometry(authored,88)
                rows.append(dict(profile=profile,features=features,compiler=compiler,mode=mode,commands=commands,lines=len(actual.splitlines())))
        print('input profile',profile,commands,'commands,',len(expected.splitlines()),'Rust trace lines agree',flush=True)
    assert all(path.read_bytes()==content for path,content in frozen.items()),'differential inputs changed during qualification'
    inventory=json.loads(frozen[INVENTORY])
    summary=dict(builds=rows,functions=18,pin=inventory['pin'],reference_sha256=inventory['source_sha256'],
                 source_sha256=hashlib.sha256(frozen[HERE/'src/silk_shell_input.cesta']).hexdigest(),
                 vector_sha256=hashlib.sha256(data).hexdigest(),semantic_mutations=mutation_results,
                 abi_adapter_checked=True,invalid_binding_refused=True,invalid_cursor_controls=cursor_controls)
    summary['independent_controls']=independent_controls
    summary['actual_app_focus_helpers']=2
    summary['peer_boundary']='Current desktop shortcut and remaining subsystem peers are scripted; app focus uses the real helpers with scripted activation-query results. Custody predicates and acquire dimension adapters are real.'
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('silk-shell input:',len(rows),'build variants match all 18 pinned functions;',len(mutation_results),'semantic mutations detected')


if __name__ == '__main__':
    main()
