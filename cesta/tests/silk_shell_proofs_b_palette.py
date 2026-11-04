#!/usr/bin/env python3
"""Qualify the first current Proofs-B partition against its source and controls."""
import argparse,hashlib,importlib,json,os,re,subprocess,sys
from pathlib import Path
from silk_shell_input_differential import item_end
from silk_shell_models_differential import mutate_function

HERE=Path(__file__).resolve().parent.parent

def run(command,data=None,env=None,accepted=True):
 result=subprocess.run([str(x) for x in command],cwd=HERE,input=data,capture_output=True,env=env)
 if accepted and result.returncode:
  raise AssertionError((command,result.stdout.decode(errors='replace'),result.stderr.decode(errors='replace')))
 if not accepted and not result.returncode:
  raise AssertionError(('unexpected acceptance',command))
 return result.stdout

def verify_source(inventory):
 pinned=subprocess.run(['git','show',inventory['pin']+':servers/silk-shell/src/main.rs'],cwd=HERE.parent,capture_output=True,text=True,check=True).stdout
 working=(HERE.parent/'servers/silk-shell/src/main.rs').read_text()
 for row in inventory['functions']:
  for label,source in (('pin',pinned),('working',working)):
   match=re.search(r'(?m)^unsafe fn '+re.escape(row['name'])+r'\b',source);assert match,(label,row['name'])
   body=source[match.start():item_end(source,match.start())]
   assert hashlib.sha256(body.encode()).hexdigest()==row['source_sha256'],(label,row['name'])

def provider():
 return r'''
#include "silk_shell_proofs_b_palette.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static SilkShellChromeState chrome;static SilkShellModelsState models;static ShellChromeApps apps;
static SilkShellInputState input;static SilkShellProofsAState proofs;static SilkShellAtlasState atlas;static SilkShellSurfacesState surfaces;
const uint32_t ATLAS_ACCENT_COLORS[5]={UINT32_C(0x112233),UINT32_C(0x223344),UINT32_C(0x334455),UINT32_C(0x445566),UINT32_C(0x556677)};
static uint16_t available_mask,execute_mask;static bool toggle_ok;static size_t bell_total;static uint8_t bell_visible;
static bool minimized_present;
static unsigned calls,toggles,renders,executes,nexts,previous,yields,snapshots,switches;
static ShellInputContext context(void){ShellChromeContext shell={&chrome,&models,&apps,&input,&proofs};return(ShellInputContext){shell,&atlas,&surfaces};}
static ShellPaletteString text(const char *value){ShellPaletteString out={0};while(value[out.len]&&out.len+1<sizeof out.bytes){out.bytes[out.len]=value[out.len];++out.len;}return out;}
size_t shell_proofs_b_palette_command_count(ShellInputContext *ctx){(void)ctx;return 11;}
uint8_t shell_proofs_b_palette_command_code(ShellInputContext *ctx,size_t index){(void)ctx;return index<11?(uint8_t)index:UINT8_MAX;}
ShellPaletteString shell_proofs_b_palette_command_label(ShellInputContext *ctx,size_t index){(void)ctx;static const char *names[11]={"Spindle","Quil","Linen","Atlas","Bell","Collar","Mesh","Restore","Zoom","Minimize","Browser"};return text(index<11?names[index]:"invalid");}
ShellPaletteStatus shell_proofs_b_palette_status(ShellInputContext *ctx,uint8_t command){(void)ctx;bool ok=command<11 && (available_mask&(UINT16_C(1)<<command));return(ShellPaletteStatus){ok,text(ok?"ready":"blocked"),text(ok?"available":"policy")};}
bool shell_proofs_b_palette_toggle(ShellInputContext *ctx){++calls;++toggles;if(!toggle_ok)return false;ctx->surfaces->COMMAND_PALETTE_OPEN=!ctx->surfaces->COMMAND_PALETTE_OPEN;return true;}
void shell_proofs_b_palette_render(ShellInputContext *ctx){(void)ctx;++calls;++renders;}
bool shell_proofs_b_palette_execute(ShellInputContext *ctx){++calls;++executes;uint8_t selected=ctx->surfaces->COMMAND_PALETTE_SELECTED;bool ok=selected<11 && (execute_mask&(UINT16_C(1)<<selected));if(ok&&selected<7){static const uint64_t sids[7]={SURFACE_ID_SPINDLE,SURFACE_ID_QUIL,SURFACE_ID_LINEN,SURFACE_ID_ATLAS_OVERLAY,SURFACE_ID_BELL_PLACEHOLDER,SURFACE_ID_COLLAR,SURFACE_ID_MESH};if(selected==3)ctx->shell.chrome->ATLAS_MODE_ENABLED=true;else ctx->shell.chrome->FOCUSED_SURFACE_ID=sids[selected];}return ok;}
void shell_proofs_b_palette_next(ShellInputContext *ctx){++calls;++nexts;ctx->surfaces->COMMAND_PALETTE_SELECTED=(uint8_t)((ctx->surfaces->COMMAND_PALETTE_SELECTED+1)%11);}
void shell_proofs_b_palette_previous(ShellInputContext *ctx){++calls;++previous;ctx->surfaces->COMMAND_PALETTE_SELECTED=(uint8_t)((ctx->surfaces->COMMAND_PALETTE_SELECTED+10)%11);}
size_t shell_proofs_b_linen_selected_index(ShellInputContext *ctx){size_t seen=0;for(size_t i=0;i<LINEN_MAX_OBJECTS;++i)if(ctx->shell.models->LINEN_OBJECTS[i].present){if(ctx->shell.models->LINEN_OBJECTS[i].object_id==ctx->shell.models->SELECTED_LINEN_OBJECT_ID)return seen;++seen;}return 0;}
size_t shell_proofs_b_bell_ring_count(ShellInputContext *ctx){(void)ctx;return bell_total;}
uint8_t shell_proofs_b_bell_visible_count(ShellInputContext *ctx){(void)ctx;return bell_visible;}
void shell_proofs_b_bell_next(ShellInputContext *ctx){++calls;uint8_t count=bell_visible?bell_visible:1;ctx->surfaces->BELL_SELECTED_ROW=(uint8_t)((ctx->surfaces->BELL_SELECTED_ROW+1)%count);}
void shell_proofs_b_atlas_toggle(ShellInputContext *ctx){++calls;++toggles;ctx->shell.chrome->ATLAS_MODE_ENABLED=!ctx->shell.chrome->ATLAS_MODE_ENABLED;}
void shell_proofs_b_atlas_switch_scene(ShellInputContext *ctx,uint8_t scene){++calls;++switches;if(scene<WORKSPACE_COUNT)ctx->shell.chrome->ACTIVE_SCENE_IDX=scene;}
void shell_proofs_b_atlas_collect_snapshot(ShellInputContext *ctx){(void)ctx;++calls;++snapshots;puts("SNAPSHOT");}
void shell_proofs_b_yield(ShellInputContext *ctx){(void)ctx;++calls;++yields;}
const char *shell_proofs_b_registry_name(ShellInputContext *ctx,size_t index){if(index>=LINEN_MAX_OBJECTS || !ctx->shell.models->LINEN_OBJECTS[index].present)return 0;return ctx->shell.models->LINEN_OBJECTS[index].display_name;}
ShellProofsBFrame shell_proofs_b_first_minimized_frame(ShellInputContext *ctx){(void)ctx;return(ShellProofsBFrame){minimized_present,77};}
ShellProofsBFrame shell_proofs_b_frame_for_surface(ShellInputContext *ctx,uint64_t surface){(void)ctx;return(ShellProofsBFrame){surface!=0,(uint32_t)(surface&UINT64_C(0xffffffff))};}
bool shell_proofs_b_minimize_frame(ShellInputContext *ctx,uint32_t frame){(void)ctx;(void)frame;minimized_present=true;return true;}
uint64_t shell_proofs_b_browser_active_frame(ShellInputContext *ctx){(void)ctx;return UINT64_C(999);}
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);putchar('\n');va_end(ap);}
static void reset(void){memset(&chrome,0,sizeof chrome);memset(&models,0,sizeof models);memset(&apps,0,sizeof apps);memset(&input,0,sizeof input);memset(&proofs,0,sizeof proofs);memset(&atlas,0,sizeof atlas);memset(&surfaces,0,sizeof surfaces);available_mask=execute_mask=UINT16_C(0x7ff);toggle_ok=true;bell_total=bell_visible=0;minimized_present=false;calls=toggles=renders=executes=nexts=previous=yields=snapshots=switches=0;chrome.FOCUSED_SURFACE_ID=SURFACE_ID_SPINDLE;chrome.ACTIVE_SCENE_IDX=1;chrome.SCENES[1].accent=3;proofs.COMMAND_PALETTE_DAILY_PROOF_SKIP_BUDGET=3;chrome.FRAMES[0]=(ShellFrame){.present=true,.frame_id=1,.scene_id=1,.normal_x=10,.normal_y=20,.normal_w=300,.normal_h=200};chrome.FRAMES[1]=(ShellFrame){.present=true,.frame_id=2,.scene_id=2,.normal_x=30,.normal_y=40,.normal_w=500,.normal_h=400};}
#if !SEXOS_COMMAND_PALETTE_STATUS_PROOF
static void defaults(void){reset();ShellInputContext ctx=context();shell_proofs_b_maybe_run_command_palette_status_proof(&ctx);shell_proofs_b_maybe_run_command_palette_linen_status_proof(&ctx);shell_proofs_b_maybe_run_quil_status_unblock_proof(&ctx);shell_proofs_b_maybe_run_command_palette_daily_proof(&ctx);shell_proofs_b_maybe_run_app_launcher_proof(&ctx);shell_proofs_b_maybe_run_app_launcher_help_proof(&ctx);shell_proofs_b_maybe_run_linen_search_filter_proof(&ctx);shell_proofs_b_maybe_run_bell_filter_proof(&ctx);shell_proofs_b_maybe_run_atlas_preview_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_a_state_model_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_b_snapshot_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_c_render_stub_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_d_frame_preview_stub_proof(&ctx);shell_proofs_b_maybe_run_app_registry_readonly_proof(&ctx);shell_proofs_b_maybe_run_app_registry_filter_sort_proof(&ctx);shell_proofs_b_maybe_run_app_registry_launch_intent_proof(&ctx);shell_proofs_b_maybe_run_app_launcher_multi_exec_proof(&ctx);shell_proofs_b_maybe_run_palette_rejects_app_open_batch_proof(&ctx);assert(!calls);assert(!proofs.COMMAND_PALETTE_STATUS_PROOF_DONE);assert(!atlas.ATLAS_PREVIEW_PROOF_DONE);puts("DEFAULT ok");}
#endif
#if SEXOS_COMMAND_PALETTE_STATUS_PROOF
static void enabled(void){
 ShellInputContext ctx;
 reset();ctx=context();chrome.FOCUSED_SURFACE_ID=0;shell_proofs_b_maybe_run_command_palette_status_proof(&ctx);assert(!proofs.COMMAND_PALETTE_STATUS_PROOF_DONE && proofs.COMMAND_PALETTE_STATUS_PROOF_STAGE==0);chrome.FOCUSED_SURFACE_ID=SURFACE_ID_SPINDLE;for(size_t i=0;i<13;++i)shell_proofs_b_maybe_run_command_palette_status_proof(&ctx);assert(proofs.COMMAND_PALETTE_STATUS_PROOF_DONE&&!proofs.COMMAND_PALETTE_STATUS_PROOF_ACTIVE&&!surfaces.COMMAND_PALETTE_OPEN);unsigned frozen=calls;shell_proofs_b_maybe_run_command_palette_status_proof(&ctx);assert(calls==frozen);puts("STATUS ok");
 reset();ctx=context();available_mask&=~(UINT16_C(1)<<CMD_FOCUS_LINEN);shell_proofs_b_maybe_run_command_palette_linen_status_proof(&ctx);assert(proofs.COMMAND_PALETTE_LINEN_STATUS_PROOF_DONE&&!surfaces.COMMAND_PALETTE_OPEN);puts("LINEN blocked");
 reset();ctx=context();shell_proofs_b_maybe_run_command_palette_linen_status_proof(&ctx);assert(proofs.COMMAND_PALETTE_LINEN_STATUS_PROOF_DONE&&!surfaces.COMMAND_PALETTE_OPEN&&surfaces.COMMAND_PALETTE_SELECTED==CMD_FOCUS_LINEN);puts("LINEN ready");
 reset();ctx=context();available_mask&=~(UINT16_C(1)<<CMD_FOCUS_LINEN);shell_proofs_b_maybe_run_quil_status_unblock_proof(&ctx);assert(proofs.QUIL_STATUS_UNBLOCK_PROOF_DONE&&!surfaces.COMMAND_PALETTE_OPEN);puts("QUIL ready");
 reset();ctx=context();execute_mask=UINT16_C(0x155);shell_proofs_b_maybe_run_command_palette_daily_proof(&ctx);assert(proofs.COMMAND_PALETTE_DAILY_PROOF_DONE&&!proofs.COMMAND_PALETTE_DAILY_PROOF_ACTIVE);assert((unsigned)proofs.COMMAND_PALETTE_DAILY_PROOF_EXECUTED+(unsigned)proofs.COMMAND_PALETTE_DAILY_PROOF_REJECTED+(unsigned)proofs.COMMAND_PALETTE_DAILY_PROOF_SKIPPED==11);puts("DAILY ok");
 reset();ctx=context();toggle_ok=false;shell_proofs_b_maybe_run_command_palette_daily_proof(&ctx);assert(proofs.COMMAND_PALETTE_DAILY_PROOF_DONE&&proofs.COMMAND_PALETTE_DAILY_PROOF_SKIPPED==11);puts("DAILY full");
 reset();ctx=context();shell_proofs_b_maybe_run_app_launcher_proof(&ctx);assert(proofs.APP_LAUNCHER_PROOF_DONE&&!proofs.APP_LAUNCHER_PROOF_ACTIVE&&!surfaces.COMMAND_PALETTE_OPEN);assert(nexts==6&&previous==6&&executes==1);puts("LAUNCHER ok");
 reset();ctx=context();shell_proofs_b_maybe_run_app_launcher_help_proof(&ctx);assert(proofs.APP_LAUNCHER_HELP_PROOF_DONE);puts("HELP ok");
 reset();ctx=context();models.LINEN_OBJECTS[0]=(LinenObject){.present=true,.object_id=9,.kind=LINEN_OBJ_KIND_PROJECT};models.LINEN_OBJECTS[1]=(LinenObject){.present=true,.object_id=10,.kind=LINEN_OBJ_KIND_DOCUMENT};models.LINEN_OBJECTS[2]=(LinenObject){.present=true,.object_id=11,.kind=LINEN_OBJ_KIND_DOCUMENT};models.SELECTED_LINEN_OBJECT_ID=10;shell_proofs_b_maybe_run_linen_search_filter_proof(&ctx);assert(proofs.LINEN_SEARCH_FILTER_PROOF_DONE);puts("LINEN filter");
 reset();ctx=context();shell_proofs_b_maybe_run_bell_filter_proof(&ctx);assert(proofs.BELL_FILTER_PROOF_DONE&&surfaces.BELL_SELECTED_ROW==0);puts("BELL empty");
 reset();ctx=context();bell_total=bell_visible=3;shell_proofs_b_maybe_run_bell_filter_proof(&ctx);assert(proofs.BELL_FILTER_PROOF_DONE&&surfaces.BELL_SELECTED_ROW==1);puts("BELL cycle");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_preview_proof(&ctx);assert(atlas.ATLAS_PREVIEW_PROOF_DONE);puts("ATLAS preview");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_a_state_model_proof(&ctx);assert(atlas.ATLAS_PHASE_A_STATE_MODEL_PROOF_DONE&&atlas.ATLAS_PHASE_A_STATE_MODEL_PROOF_STAGE==10&&!chrome.ATLAS_MODE_ENABLED&&chrome.ACTIVE_SCENE_IDX==2);assert(yields==10&&switches==1);puts("ATLAS A");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_b_snapshot_proof(&ctx);assert(atlas.ATLAS_PHASE_B_SNAPSHOT_PROOF_DONE&&atlas.ATLAS_PHASE_B_SNAPSHOT_PROOF_STAGE==6&&!chrome.ATLAS_MODE_ENABLED);assert(yields==6&&snapshots==1);puts("ATLAS B");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_c_render_stub_proof(&ctx);assert(atlas.ATLAS_PHASE_C_RENDER_STUB_PROOF_DONE&&atlas.ATLAS_PHASE_C_RENDER_STUB_PROOF_STAGE==4);assert(yields==4&&snapshots==1);puts("ATLAS C");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_d_frame_preview_stub_proof(&ctx);assert(atlas.ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_DONE&&atlas.ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_STAGE==4);assert(yields==4&&snapshots==1);puts("ATLAS D");
 reset();ctx=context();models.LINEN_OBJECTS[0]=(LinenObject){.present=true,.object_id=2,.kind=LINEN_OBJ_KIND_DOCUMENT,.state=LINEN_OBJ_STATE_SAVED,.display_name="Doc"};models.LINEN_OBJECTS[1]=(LinenObject){.present=true,.object_id=3,.kind=LINEN_OBJ_KIND_FOLDER,.state=LINEN_OBJ_STATE_LOADED,.display_name="Folder"};models.LINEN_OBJECTS[2]=(LinenObject){.present=true,.object_id=9,.kind=LINEN_OBJ_KIND_REFERENCE,.state=LINEN_OBJ_STATE_ALLOCATED,.display_name="Ref"};shell_proofs_b_maybe_run_app_registry_readonly_proof(&ctx);assert(proofs.APP_REGISTRY_READONLY_PROOF_DONE);shell_proofs_b_maybe_run_app_registry_filter_sort_proof(&ctx);assert(proofs.APP_REGISTRY_FILTER_SORT_PROOF_DONE);shell_proofs_b_maybe_run_app_registry_launch_intent_proof(&ctx);assert(proofs.APP_REGISTRY_LAUNCH_INTENT_PROOF_DONE);puts("REGISTRY ok");
 reset();ctx=context();shell_proofs_b_maybe_run_app_launcher_multi_exec_proof(&ctx);assert(proofs.APP_LAUNCHER_MULTI_EXEC_PROOF_DONE&&!proofs.APP_LAUNCHER_MULTI_EXEC_PROOF_ACTIVE&&!surfaces.COMMAND_PALETTE_OPEN);assert(executes==7&&chrome.FOCUSED_SURFACE_ID==SURFACE_ID_MESH);puts("MULTI ok");
 reset();ctx=context();execute_mask=UINT16_C(0x555);shell_proofs_b_maybe_run_palette_rejects_app_open_batch_proof(&ctx);assert(proofs.PALETTE_BATCH_PROOF_DONE&&!proofs.PALETTE_BATCH_PROOF_ACTIVE&&!surfaces.COMMAND_PALETTE_OPEN);assert(executes==11&&minimized_present);puts("BATCH ok");
}
#endif
int main(void){
#if SEXOS_COMMAND_PALETTE_STATUS_PROOF
 enabled();
#else
 defaults();
#endif
 return 0;
}
'''

def compile_matrix(cesta,work,source):
 flags=['-D'+row['flag']+'=1' for row in json.loads((HERE/'interfaces/silk_shell_proofs_b_palette.source.json').read_text())['functions']]
 rows=[];outputs={}
 for profile,defines in (('default',[]),('all',flags)):
  for compiler in ('gcc','clang'):
   for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
    binary=work/f'{profile}-{compiler}-{mode}'
    run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*options,*defines,'-Igenerated/include',source,work/'provider.c','-o',binary])
    output=run([binary],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'})
    outputs.setdefault(profile,output);assert output==outputs[profile],(profile,compiler,mode)
    rows.append(dict(profile=profile,compiler=compiler,mode=mode,sha256=hashlib.sha256(output).hexdigest()))
 return rows,outputs,flags

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True);parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path);parser.add_argument('--output',type=Path,default=HERE/'build/proofs-b-palette');args=parser.parse_args()
 work=args.output.resolve();work.mkdir(parents=True,exist_ok=True);inventory=json.loads((HERE/'interfaces/silk_shell_proofs_b_palette.source.json').read_text());verify_source(inventory);(work/'provider.c').write_text(provider())
 plans={};exports={}
 for compiler in ('gcc','clang'):
  source=work/f'palette-{compiler}.c';plan=work/f'palette-{compiler}.plan';run([args.cesta,'export-c','src/silk_shell_proofs_b_palette.cesta','--cc',compiler,'--std=c11','-o',source,'--plan',plan,'--','-Igenerated/include']);plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan;exports[compiler]=source
 rows,outputs,flags=compile_matrix(args.cesta,work,exports['gcc'])
 base=exports['gcc'].read_text();mutants=[]
 changes=[
  ('skip-status-stage','shell_proofs_b_maybe_run_command_palette_status_proof','if(stage==0)','if(stage==1)'),
  ('wrong-linen-command','shell_proofs_b_maybe_run_command_palette_linen_status_proof','ShellPaletteStatus status=shell_proofs_b_palette_status(ctx,CMD_FOCUS_LINEN);','ShellPaletteStatus status=shell_proofs_b_palette_status(ctx,CMD_FOCUS_QUIL);'),
  ('wrong-quil-command','shell_proofs_b_maybe_run_quil_status_unblock_proof','CMD_FOCUS_QUIL);','CMD_FOCUS_LINEN);'),
  ('short-daily','shell_proofs_b_maybe_run_command_palette_daily_proof','step<count','step+1<count'),
  ('short-launcher-nav','shell_proofs_b_maybe_run_app_launcher_proof','i<6','i<5'),
  ('short-help','shell_proofs_b_maybe_run_app_launcher_help_proof','count<7?count:7','count<6?count:6'),
  ('wrong-filter-kind','shell_proofs_b_maybe_run_linen_search_filter_proof','LINEN_OBJ_KIND_DOCUMENT','LINEN_OBJ_KIND_PROJECT'),
  ('skip-bell-next','shell_proofs_b_maybe_run_bell_filter_proof','shell_proofs_b_bell_next(ctx)','(void)ctx'),
  ('wrong-preview-color','shell_proofs_b_maybe_run_atlas_preview_proof','(size_t)accent%5','((size_t)accent+1)%5'),
  ('skip-atlas-a-switch','shell_proofs_b_maybe_run_atlas_phase_a_state_model_proof','shell_proofs_b_atlas_switch_scene(ctx,populated>1?target:old)','shell_proofs_b_atlas_switch_scene(ctx,old)'),
  ('skip-atlas-b-snapshot','shell_proofs_b_maybe_run_atlas_phase_b_snapshot_proof','else if(stage==2)shell_proofs_b_atlas_collect_snapshot(ctx);','else if(stage==2)(void)ctx;'),
  ('skip-atlas-c-snapshot','shell_proofs_b_maybe_run_atlas_phase_c_render_stub_proof','shell_proofs_b_atlas_collect_snapshot(ctx);','(void)ctx;'),
  ('skip-atlas-d-snapshot','shell_proofs_b_maybe_run_atlas_phase_d_frame_preview_stub_proof','shell_proofs_b_atlas_collect_snapshot(ctx);','(void)ctx;'),
  ('invert-registry-present','shell_proofs_b_maybe_run_app_registry_readonly_proof','if(!present)continue','if(present)continue'),
  ('invert-registry-sort','shell_proofs_b_maybe_run_app_registry_filter_sort_proof','if(previous>object->object_id)sorted=0','if(previous<object->object_id)sorted=0'),
  ('drop-folder-intent','shell_proofs_b_runnable_kind','kind<=LINEN_OBJ_KIND_FOLDER','kind<LINEN_OBJ_KIND_FOLDER'),
  ('short-multi','shell_proofs_b_maybe_run_app_launcher_multi_exec_proof','index<7','index<6'),
  ('short-batch','shell_proofs_b_maybe_run_palette_rejects_app_open_batch_proof','index<count','index+1<count')]
 for label,function,old,new in changes:
  path=work/f'mutant-{label}.c';path.write_text(mutate_function(base,function,old,new));binary=work/f'mutant-{label}';run(['gcc','-std=c11','-O2',*flags,'-Igenerated/include',path,work/'provider.c','-o',binary]);result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode or result.stdout!=outputs['all'],('survived',label);mutants.append(label)
 original=(HERE/'src/silk_shell_proofs_b_palette.cesta').read_text();controls=(('readonly-proof-state','(field proofs_a (borrow write))','(field proofs_a (borrow read))'),('readonly-surfaces','(field surfaces (borrow write))','(field surfaces (borrow read))'))
 for label,old,new in controls:
  assert original.count(old)==1,(label,original.count(old));path=work/f'{label}.cesta';path.write_text(original.replace(old,new));run([args.cesta,'cc','--cc','gcc','-std=c11','-fsyntax-only','-Igenerated/include',path],accepted=False)
 admission=[]
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
  for i,plan in enumerate(plans.values()):record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,work/f'Palette{i}.lean',args.lean_root.resolve()))
 summary=dict(pin=inventory['pin'],functions=len(inventory['functions']),configurations=rows,mutants_caught=mutants,source_controls=len(controls),plans=len(plans),admission=admission)
 (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Proofs-B palette:',len(inventory['functions']),'current bodies;',len(rows),'configurations;',len(mutants),'mutants caught;',sum(x['functions'] for x in admission),'plans admitted')
if __name__=='__main__':main()
