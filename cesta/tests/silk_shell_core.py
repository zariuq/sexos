#!/usr/bin/env python3
"""Qualify root state defaults and complete Proofs-B dispatcher wiring."""
import argparse,hashlib,json,os,re,subprocess,sys
from pathlib import Path
from silk_shell_input_differential import item_end
from silk_shell_models_differential import mutate_function
HERE=Path(__file__).resolve().parent.parent
def run(command,env=None,accepted=True):
 result=subprocess.run([str(x) for x in command],cwd=HERE,capture_output=True,env=env)
 if accepted and result.returncode:raise AssertionError((command,result.stdout.decode(errors='replace'),result.stderr.decode(errors='replace')))
 if not accepted and not result.returncode:raise AssertionError(('unexpected acceptance',command))
 return result.stdout
def names():
 complete=json.loads((HERE/'interfaces/silk_shell_proofs_b.complete.json').read_text());out=[]
 for part in complete['partitions']:out.extend(row['name'] for row in json.loads((HERE/part['manifest']).read_text())['functions'])
 return out
def provider(items):
 functions='\n'.join(f'void {name}(void){{++seen[{i}];}}' for i,name in enumerate(items))
 checks=''.join(f'assert(seen[{i}]==1);' for i in range(len(items)))
 return f'''#include "silk_shell_core.h"
#include "silk_shell_state_defaults.h"
#include "silk_shell_surfaces.h"
#include "silk_shell_models_linen.h"
#include "silk_shell_models_workspace.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned seen[{len(items)}],autodemo,hid_calls,surface_calls,launch_calls,document_refreshes,document_geometry,linen_open,scene_commands,store_replies;static uint64_t hid_class,hid_a0,hid_a1,launch_surface;
{functions}
bool shell_models_workspace_receive(ShellLinenWorkspaceContext *ctx,const SexosMessage *message){{(void)ctx;return message->type_id==UINT64_C(0x900);}}
bool shell_models_linen_async_reply(ShellLinenModelsContext *ctx,const SexosMessage *message){{(void)ctx;return message->type_id==UINT64_C(0x901);}}
void shell_proofs_b_maybe_run_cursor_autodemo(ShellProofsBInputContext *ctx){{(void)ctx;++autodemo;}}
void shell_input_handle_hid_event(ShellInputContext *ctx,uint64_t event_class,uint64_t arg0,uint64_t arg1){{(void)ctx;++hid_calls;hid_class=event_class;hid_a0=arg0;hid_a1=arg1;}}
int32_t shell_chrome_desktop_width(ShellChromeContext *ctx){{(void)ctx;return 1280;}}
int32_t shell_chrome_desktop_height(ShellChromeContext *ctx){{(void)ctx;return 720;}}
bool shell_surfaces_handle_app_surface_req(ShellSurfacesContext *ctx,uint64_t sid,uint64_t title,uint64_t packed,uint32_t caller){{(void)ctx;(void)title;(void)packed;(void)caller;++surface_calls;return sid!=0;}}
void shell_surfaces_open_app_in_active_scene_by_sid(ShellSurfacesContext *ctx,uint64_t sid){{(void)ctx;++launch_calls;launch_surface=sid;}}
bool shell_surfaces_focus_or_open_linen(ShellSurfacesContext *ctx){{(void)ctx;++linen_open;return true;}}
void sexos_line(const char *format,...){{(void)format;}}
ShellAppCatalogRow shell_custody_catalog_at(size_t index){{return index==0?(ShellAppCatalogRow){{.present=true,.app=SHELL_APP_QUIL,.target=9,.surface=156,.shell_surface=201}}:(ShellAppCatalogRow){{0}};}}
bool shell_custody_allows_operation(const ShellAppCustody *entry,uint32_t task,uint64_t opcode,uint64_t a,uint64_t b,uint64_t c){{(void)opcode;(void)a;(void)b;(void)c;return entry->task==task;}}
bool shell_custody_buffer_sender(uint32_t caller){{return caller==2;}}
void shell_document_delivery_refresh(ShellDocumentDelivery *delivery){{(void)delivery;++document_refreshes;}}
bool shell_chrome_peer_get_surface_bounds(ShellChromeContext *ctx,uint64_t sid,int32_t *x,int32_t *y,uint32_t *w,uint32_t *h){{(void)ctx;assert(sid==201);*x=10;*y=20;*w=640;*h=480;return true;}}
void shell_chrome_native_child_geometry(ShellChromeContext *ctx,uint64_t frame,int32_t x,int32_t y,uint32_t w,uint32_t h){{(void)ctx;assert(frame==201&&x==10&&y==20&&w==640&&h==480);++document_geometry;}}
void handle_scene_settings_cmd(uint64_t command,uint64_t value,uint64_t flags){{assert(command==2&&value==3&&flags==4);++scene_commands;}}
void handle_sexstore_get_reply(uint64_t value){{assert(value==UINT64_C(0x1234));++store_replies;}}
int main(void){{SilkShellState state;memset(&state,0xa5,sizeof state);silk_shell_state_init(&state);
assert(SILK_SHELL_STATIC_DEFAULT_COUNT>=300);assert(state.models.boot_load_scene_settings__BOOT_LOAD_BUDGET==1);assert(state.models.handle_scene_settings_cmd__CMD_BUDGET==32);assert(state.chrome.WINDOWS_len==2);assert(state.chrome.WINDOWS[0].desc.window_id==1);assert(state.chrome.WINDOWS[1].desc.window_id==2);assert(state.chrome.FOCUS_ID==2);
assert(state.chrome.FRAMES[0].present&&state.chrome.FRAMES[0].frame_id==1&&state.chrome.FRAMES[0].tab_count==2);assert(state.chrome.FRAMES[0].tabs[0].surface_id==SURFACE_ID_APP);assert(state.chrome.FRAMES[0].tabs[1].surface_id==SURFACE_ID_STATIC);
assert(state.input.INTERACTION.tag==INTERACTION_IDLE);assert(state.proofs_b.maybe_run_window_drag_synthetic_proof__DEFER_BUDGET==8);assert(state.proofs_b.maybe_run_keyboard_window_synthetic_proof__STATE_BUDGET==64);assert(state.proofs_b.maybe_run_frame_light_zoom_synthetic_proof__ATTEMPTS==240);
SilkShellDispatchResult d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0x900)}});assert(d.handled&&!d.mutated);d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0x901)}});assert(d.handled&&!d.mutated);
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_AUTODEMO_TICK}});assert(d.handled&&d.reply&&autodemo==1);
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_HID_EVENT,.arg0=7,.arg1=1,.arg2=EV_KEY,.caller_pd=1,.caller_task=1}});assert(d.handled&&!d.mutated&&hid_calls==0);d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_HID_EVENT,.arg0=7,.arg1=1,.arg2=EV_KEY,.caller_pd=4,.caller_task=4}});assert(d.handled&&d.mutated&&hid_calls==1&&hid_class==EV_KEY&&hid_a0==7&&hid_a1==1);
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0x260),.arg1=1,.arg2=(uint64_t)2<<16,.caller_pd=4,.caller_task=4}});assert(d.handled&&d.mutated&&state.input.POINTER_USB_STATE_INIT&&state.input.POINTER_BUTTONS==1&&state.input.POINTER_WHEEL_ACCUM==2&&hid_calls==2&&hid_class==EV_BTN);
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_APP_SURFACE_REQ,.arg0=310,.caller_pd=7}});assert(d.handled&&d.mutated&&d.reply&&surface_calls==1);d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_SHELL_LAUNCH_REQUEST,.arg0=2}});assert(d.handled&&d.mutated&&launch_calls==1&&launch_surface==SURFACE_ID_LINEN);
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0x271),.arg0=((uint64_t)12<<32)|12,.arg1=1,.arg2=1,.caller_pd=12,.caller_task=12}});assert(d.handled);state.apps.present[0]=true;state.apps.entries[0]=(ShellAppCustody){{.target=9,.task=9}};
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0x275),.caller_pd=9,.caller_task=8}});assert(d.handled&&document_refreshes==0);d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0x275),.caller_pd=9,.caller_task=9}});assert(d.handled&&document_refreshes==1&&document_geometry==1);d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0x274),.caller_pd=9,.caller_task=9}});assert(d.handled&&linen_open==1);
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_SCENE_SETTINGS_CMD,.arg0=2,.arg1=3,.arg2=4,.caller_pd=1,.caller_task=1}});assert(d.handled&&scene_commands==0);d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_SCENE_SETTINGS_CMD,.arg0=2,.arg1=3,.arg2=4,.caller_pd=3,.caller_task=3}});assert(d.handled&&d.mutated&&scene_commands==1);
state.models.SEXSTORE_LOAD_PENDING=true;d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=1,.arg0=UINT64_C(0x1234),.caller_pd=7}});assert(d.handled&&state.models.SEXSTORE_LOAD_PENDING&&store_replies==0);d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=1,.arg0=UINT64_C(0x1234),.caller_pd=8}});assert(d.handled&&!state.models.SEXSTORE_LOAD_PENDING&&store_replies==1);
d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_SHELL_BIND_BUFFER,.arg0=UINT64_C(0xbad),.caller_pd=7}});assert(d.handled&&!d.mutated&&state.chrome.WINDOWS[1].desc.buffer_handle!=UINT64_C(0xbad));d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=OP_SHELL_BIND_BUFFER,.arg0=UINT64_C(0xabc),.caller_pd=2}});assert(d.handled&&d.mutated&&state.chrome.WINDOWS[1].desc.buffer_handle==UINT64_C(0xabc));d=silk_shell_dispatch(&state,&(SexosMessage){{.type_id=UINT64_C(0xffff)}});assert(!d.handled);
silk_shell_proofs_tick();{checks}printf("PASS core defaults=%u proofs_b={len(items)}\\n",(unsigned)SILK_SHELL_STATIC_DEFAULT_COUNT);return 0;}}
'''
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--output',type=Path,default=HERE/'build/silk-shell-core-test');args=ap.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);manifest=json.loads((HERE/'interfaces/silk_shell_core.source.json').read_text());source=(HERE.parent/'servers/silk-shell/src/main.rs').read_text();match=re.search(r'(?m)^pub extern "C" fn _start\b',source);assert match;body=source[match.start():item_end(source,match.start())];assert hashlib.sha256(body.encode()).hexdigest()==manifest['start_sha256'];run([sys.executable,'tools/shell_state_defaults.py','--check']);defaults=int(re.search(r'SILK_SHELL_STATIC_DEFAULT_COUNT (\d+)',(HERE/'generated/include/silk_shell_state_defaults.h').read_text()).group(1));items=names();assert len(items)==37;(w/'provider.c').write_text(provider(items));exports={}
 dispatches={}
 for cc in ('gcc','clang'):
  target=w/f'core-{cc}.c';dispatch=w/f'dispatch-{cc}.c';run([args.cesta,'export-c','src/silk_shell_core_root.cesta','--cc',cc,'--std=c11','-o',target,'--','-Igenerated/include']);run([args.cesta,'export-c','src/silk_shell_core_dispatch.cesta','--cc',cc,'--std=c11','-o',dispatch,'--','-Igenerated/include']);exports[cc]=target;dispatches[cc]=dispatch
 records=[];reference=None
 for cc in ('gcc','clang'):
  for mode,opts in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   binary=w/f'{cc}-{mode}';run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*opts,'-Igenerated/include',exports[cc],dispatches[cc],w/'provider.c','-o',binary]);out=run([binary],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});reference=reference or out;assert out==reference;records.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(out).hexdigest()))
 base=exports['gcc'].read_text();changes=[('one-tab','silk_shell_state_init','base.tab_count=2','base.tab_count=1'),('skip-last-driver','silk_shell_proofs_tick','maybe_run_cursor_autodemo();','(void)0;')];mutants=[]
 for label,function,old,new in changes:
  path=w/f'mutant-{label}.c';path.write_text(mutate_function(base,function,old,new));binary=w/f'mutant-{label}';run(['gcc','-std=c11','-O2','-Igenerated/include',path,dispatches['gcc'],w/'provider.c','-o',binary]);result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode or result.stdout!=reference,('survived',label);mutants.append(label)
 dispatch_base=dispatches['gcc'].read_text();dispatch_mutations=[('hid-caller','if(message->type_id==OP_HID_EVENT){\n  if(message->caller_pd!=4','if(message->type_id==OP_HID_EVENT){\n  if(message->caller_pd!=1'),('buffer-caller','if(!shell_custody_buffer_sender(message->caller_pd))','if(false&&!shell_custody_buffer_sender(message->caller_pd))'),('settings-caller','message->caller_pd!=3','message->caller_pd!=2'),('store-caller','message->caller_pd==8','message->caller_pd==7')]
 for label,old,new in dispatch_mutations:
  path=w/f'mutant-{label}.c';path.write_text(mutate_function(dispatch_base,'silk_shell_dispatch',old,new));binary=w/f'mutant-{label}';run(['gcc','-std=c11','-O2','-Igenerated/include',exports['gcc'],path,w/'provider.c','-o',binary]);result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode or result.stdout!=reference;mutants.append(label)
 (w/'summary.json').write_text(json.dumps(dict(pin=manifest['pin'],configurations=records,drivers=len(items),scalar_defaults=defaults,mutants_caught=mutants,remaining=manifest['remaining']),indent=2)+'\n');print('Silk Shell Core root:',len(items),'Proofs-B drivers;',defaults,'scalar defaults; 6 configurations;',len(mutants),'mutants caught')
if __name__=='__main__':main()
