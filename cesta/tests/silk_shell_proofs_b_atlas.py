#!/usr/bin/env python3
"""Qualify the Atlas interaction slice of current Proofs-B."""
import argparse,hashlib,importlib,json,os,re,subprocess,sys
from pathlib import Path
from silk_shell_input_differential import item_end
from silk_shell_models_differential import mutate_function
HERE=Path(__file__).resolve().parent.parent
def run(command,env=None,accepted=True):
 result=subprocess.run([str(x) for x in command],cwd=HERE,capture_output=True,env=env)
 if accepted and result.returncode:raise AssertionError((command,result.stdout.decode(errors='replace'),result.stderr.decode(errors='replace')))
 if not accepted and not result.returncode:raise AssertionError(('unexpected acceptance',command))
 return result.stdout
def verify_source(inventory):
 pinned=subprocess.run(['git','show',inventory['pin']+':servers/silk-shell/src/main.rs'],cwd=HERE.parent,capture_output=True,text=True,check=True).stdout;working=(HERE.parent/'servers/silk-shell/src/main.rs').read_text()
 for row in inventory['functions']:
  for label,source in (('pin',pinned),('working',working)):
   match=re.search(r'(?m)^unsafe fn '+re.escape(row['name'])+r'\b',source);assert match,(label,row['name']);body=source[match.start():item_end(source,match.start())];assert hashlib.sha256(body.encode()).hexdigest()==row['source_sha256'],(label,row['name'])
def provider():
 return r'''
#include "silk_shell_proofs_b_atlas.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static SilkShellChromeState chrome;static SilkShellModelsState models;static ShellChromeApps apps;static SilkShellInputState input;static SilkShellProofsAState proofs;static SilkShellAtlasState atlas;static SilkShellSurfacesState surfaces;static SilkShellProofsBState proofs_b;static unsigned calls,toggles,yields,cycles,noops,reparents,hid_events;
static ShellInputContext context(void){ShellChromeContext shell={&chrome,&models,&apps,&input,&proofs};return(ShellInputContext){shell,&atlas,&surfaces};}
void shell_proofs_b_atlas_toggle(ShellInputContext *ctx){++calls;++toggles;ctx->shell.chrome->ATLAS_MODE_ENABLED=!ctx->shell.chrome->ATLAS_MODE_ENABLED;}
void shell_proofs_b_atlas_switch_scene(ShellInputContext *ctx,uint8_t scene){++calls;if(scene<WORKSPACE_COUNT)ctx->shell.chrome->ACTIVE_SCENE_IDX=scene;}
void shell_proofs_b_yield(ShellInputContext *ctx){(void)ctx;++calls;++yields;}
int32_t shell_proofs_b_atlas_desktop_width(ShellInputContext *ctx){(void)ctx;return 1280;}
int32_t shell_proofs_b_atlas_bar_height(ShellInputContext *ctx){(void)ctx;return 20;}
ShellProofsBCard shell_proofs_b_atlas_card(ShellInputContext *ctx,size_t scene,uint32_t width){(void)ctx;(void)width;return(ShellProofsBCard){10+(int32_t)scene*100,30,80,60};}
ShellProofsBScene shell_proofs_b_atlas_scene_at(ShellInputContext *ctx,int32_t x,int32_t y){(void)ctx;if(y<50||y>=110)return(ShellProofsBScene){0};for(uint8_t scene=0;scene<ATLAS_MAX_SCENES;++scene)if(x>=10+(int32_t)scene*100&&x<90+(int32_t)scene*100)return(ShellProofsBScene){true,scene};return(ShellProofsBScene){0};}
void shell_proofs_b_atlas_exit(ShellInputContext *ctx){++calls;ctx->shell.chrome->ATLAS_MODE_ENABLED=false;}
bool shell_proofs_b_atlas_cycle(ShellInputContext *ctx,int32_t delta,const char *reason){(void)reason;++calls;++cycles;uint8_t scene=ctx->shell.chrome->ACTIVE_SCENE_IDX;if(delta>0)scene=(uint8_t)((scene+1)%WORKSPACE_COUNT);else if(delta<0)scene=scene?scene-1:(uint8_t)(WORKSPACE_COUNT-1);ctx->shell.chrome->ACTIVE_SCENE_IDX=scene;return true;}
bool shell_proofs_b_atlas_same_scene_noop(ShellInputContext *ctx,uint32_t frame,uint8_t source,uint8_t target){(void)ctx;(void)frame;++calls;++noops;return source==target;}
bool shell_proofs_b_atlas_reparent(ShellInputContext *ctx,uint32_t frame,uint8_t target,const char *reason){(void)reason;++calls;++reparents;for(size_t i=0;i<MAX_FRAMES;++i)if(ctx->shell.chrome->FRAMES[i].present&&ctx->shell.chrome->FRAMES[i].frame_id==frame){ctx->shell.chrome->FRAMES[i].scene_id=target;return true;}return false;}
bool shell_proofs_b_atlas_surface_in_active_scene(ShellInputContext *ctx,uint64_t surface){if(!surface)return true;for(size_t i=0;i<MAX_FRAMES;++i)if(ctx->shell.chrome->FRAMES[i].present&&ctx->shell.chrome->FRAMES[i].scene_id==ctx->shell.chrome->ACTIVE_SCENE_IDX)for(size_t j=0;j<ctx->shell.chrome->FRAMES[i].tab_count;++j)if(ctx->shell.chrome->FRAMES[i].tabs[j].surface_id==surface)return true;return false;}
void shell_proofs_b_atlas_input_hid(ShellInputContext *ctx,uint64_t event_class,uint64_t arg0,uint64_t arg1){++calls;++hid_events;if(event_class!=EV_BTN||arg0!=1)return;if(arg1==1){for(size_t i=0;i<MAX_FRAMES;++i)if(ctx->shell.chrome->FRAMES[i].present&&ctx->shell.chrome->FRAMES[i].scene_id==ctx->shell.chrome->ACTIVE_SCENE_IDX){ctx->atlas->ATLAS_DRAG_INTENT=(AtlasDragIntent){true,ctx->shell.chrome->ACTIVE_SCENE_IDX,ctx->shell.chrome->FRAMES[i].frame_id,ctx->shell.input->POINTER_X,ctx->shell.input->POINTER_Y};break;}}else if(ctx->atlas->ATLAS_DRAG_INTENT.active){ShellProofsBScene scene=shell_proofs_b_atlas_scene_at(ctx,ctx->shell.input->POINTER_X,ctx->shell.input->POINTER_Y);if(scene.present)(void)shell_proofs_b_atlas_reparent(ctx,ctx->atlas->ATLAS_DRAG_INTENT.frame_id,scene.scene,"pointer");ctx->atlas->ATLAS_DRAG_INTENT.active=false;ctx->shell.chrome->ATLAS_MODE_ENABLED=false;}}
bool shell_proofs_b_atlas_cancel_drag(ShellInputContext *ctx,const char *reason){(void)reason;bool active=ctx->atlas->ATLAS_DRAG_INTENT.active;ctx->atlas->ATLAS_DRAG_INTENT.active=false;return active;}
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);putchar('\n');va_end(ap);}
static void reset(void){memset(&chrome,0,sizeof chrome);memset(&models,0,sizeof models);memset(&apps,0,sizeof apps);memset(&input,0,sizeof input);memset(&proofs,0,sizeof proofs);memset(&atlas,0,sizeof atlas);memset(&surfaces,0,sizeof surfaces);memset(&proofs_b,0,sizeof proofs_b);calls=toggles=yields=cycles=noops=reparents=hid_events=0;chrome.ACTIVE_SCENE_IDX=1;chrome.FRAMES[0]=(ShellFrame){.present=true,.frame_id=7,.scene_id=1,.normal_x=10,.normal_y=20,.normal_w=300,.normal_h=200};chrome.FRAMES[1]=(ShellFrame){.present=true,.frame_id=8,.scene_id=2,.normal_x=30,.normal_y=40,.normal_w=500,.normal_h=400};}
#if !SEXOS_ATLAS_PHASE_E1_CLICK_SCENE_SWITCH_PROOF
static void defaults(void){reset();ShellInputContext ctx=context();shell_proofs_b_maybe_run_atlas_phase_e1_click_scene_switch_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_e2_keyboard_scene_cycle_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_e3_drag_begin_marker_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_e4b_same_scene_noop_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_e4c_cross_scene_reparent_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_e4c2_true_reparent_proof(&ctx);shell_proofs_b_maybe_run_atlas_phase_e4d_real_pointer_drop_proof(&ctx);shell_proofs_b_maybe_run_atlas_overview_final_closeout_proof(&ctx,&proofs_b);assert(!calls&&!atlas.ATLAS_PHASE_E1_CLICK_SCENE_SWITCH_PROOF_DONE);assert(proofs_b.maybe_run_atlas_overview_final_closeout_proof__ATLAS_OVERVIEW_FINAL_CLOSEOUT_CALLPATH_MARKERS_EMITTED);puts("DEFAULT ok");}
#endif
#if SEXOS_ATLAS_PHASE_E1_CLICK_SCENE_SWITCH_PROOF
static void enabled(void){ShellInputContext ctx;
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_e1_click_scene_switch_proof(&ctx);assert(atlas.ATLAS_PHASE_E1_CLICK_SCENE_SWITCH_PROOF_DONE&&atlas.ATLAS_PHASE_E1_CLICK_SCENE_SWITCH_PROOF_STAGE==10&&!chrome.ATLAS_MODE_ENABLED&&chrome.ACTIVE_SCENE_IDX==0&&yields==10);puts("E1 ok");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_e2_keyboard_scene_cycle_proof(&ctx);assert(atlas.ATLAS_PHASE_E2_KEYBOARD_SCENE_CYCLE_PROOF_DONE&&atlas.ATLAS_PHASE_E2_KEYBOARD_SCENE_CYCLE_PROOF_STAGE==10&&!chrome.ATLAS_MODE_ENABLED&&chrome.ACTIVE_SCENE_IDX==1&&cycles==2&&yields==10);puts("E2 ok");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_e3_drag_begin_marker_proof(&ctx);assert(atlas.ATLAS_PHASE_E3_DRAG_BEGIN_MARKER_PROOF_DONE&&atlas.ATLAS_PHASE_E3_DRAG_BEGIN_MARKER_PROOF_STAGE==10&&!atlas.ATLAS_DRAG_INTENT.active&&!chrome.ATLAS_MODE_ENABLED&&yields==10);puts("E3 ok");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_e4b_same_scene_noop_proof(&ctx);assert(atlas.ATLAS_PHASE_E4B_SAME_SCENE_NOOP_PROOF_DONE&&atlas.ATLAS_PHASE_E4B_SAME_SCENE_NOOP_PROOF_STAGE==10&&!chrome.ATLAS_MODE_ENABLED&&chrome.FRAMES[0].scene_id==1&&noops==1&&yields==10);puts("E4B ok");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_e4c_cross_scene_reparent_proof(&ctx);assert(atlas.ATLAS_PHASE_E4C_CROSS_SCENE_REPARENT_PROOF_DONE&&atlas.ATLAS_PHASE_E4C_CROSS_SCENE_REPARENT_PROOF_STAGE==20&&!chrome.ATLAS_MODE_ENABLED&&chrome.FRAMES[0].scene_id==1&&reparents==2);puts("E4C ok");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_e4c2_true_reparent_proof(&ctx);assert(atlas.ATLAS_PHASE_E4C2_TRUE_REPARENT_PROOF_DONE&&atlas.ATLAS_PHASE_E4C2_TRUE_REPARENT_PROOF_STAGE==20&&!chrome.ATLAS_MODE_ENABLED&&chrome.FRAMES[0].scene_id==1&&reparents==2);puts("E4C2 ok");
 reset();ctx=context();shell_proofs_b_maybe_run_atlas_phase_e4d_real_pointer_drop_proof(&ctx);assert(atlas.ATLAS_PHASE_E4D_REAL_POINTER_DROP_PROOF_DONE&&atlas.ATLAS_PHASE_E4D_REAL_POINTER_DROP_PROOF_STAGE==20&&!chrome.ATLAS_MODE_ENABLED&&chrome.FRAMES[0].scene_id==1&&reparents==2&&hid_events==2);puts("E4D ok");
 reset();ctx=context();atlas.ATLAS_PHASE_A_STATE_MODEL_PROOF_DONE=atlas.ATLAS_PHASE_B_SNAPSHOT_PROOF_DONE=atlas.ATLAS_PHASE_C_RENDER_STUB_PROOF_DONE=atlas.ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF_DONE=true;atlas.ATLAS_PHASE_E1_CLICK_SCENE_SWITCH_PROOF_DONE=atlas.ATLAS_PHASE_E2_KEYBOARD_SCENE_CYCLE_PROOF_DONE=atlas.ATLAS_PHASE_E3_DRAG_BEGIN_MARKER_PROOF_DONE=atlas.ATLAS_PHASE_E4B_SAME_SCENE_NOOP_PROOF_DONE=atlas.ATLAS_PHASE_E4C_CROSS_SCENE_REPARENT_PROOF_DONE=atlas.ATLAS_PHASE_E4C2_TRUE_REPARENT_PROOF_DONE=true;shell_proofs_b_maybe_run_atlas_overview_final_closeout_proof(&ctx,&proofs_b);assert(!atlas.ATLAS_OVERVIEW_FINAL_CLOSEOUT_PROOF_DONE);atlas.ATLAS_PHASE_E4D_REAL_POINTER_DROP_PROOF_DONE=true;shell_proofs_b_maybe_run_atlas_overview_final_closeout_proof(&ctx,&proofs_b);assert(atlas.ATLAS_OVERVIEW_FINAL_CLOSEOUT_PROOF_DONE);puts("CLOSEOUT ok");}
#endif
int main(void){
#if SEXOS_ATLAS_PHASE_E1_CLICK_SCENE_SWITCH_PROOF
 enabled();
#else
 defaults();
#endif
 return 0;}
'''
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path);ap.add_argument('--lean-root',type=Path);ap.add_argument('--output',type=Path,default=HERE/'build/proofs-b-atlas');args=ap.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);inventory=json.loads((HERE/'interfaces/silk_shell_proofs_b_atlas.source.json').read_text());verify_source(inventory);(w/'provider.c').write_text(provider());plans={};sources={}
 for cc in ('gcc','clang'):
  source=w/f'atlas-{cc}.c';plan=w/f'atlas-{cc}.plan';run([args.cesta,'export-c','src/silk_shell_proofs_b_atlas.cesta','--cc',cc,'--std=c11','-o',source,'--plan',plan,'--','-Igenerated/include']);plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan;sources[cc]=source
 flags=['-D'+row['flag']+'=1' for row in inventory['functions']];records=[];outputs={}
 for profile,defines in (('default',[]),('all',flags)):
  for cc in ('gcc','clang'):
   for mode,opts in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
    binary=w/f'{profile}-{cc}-{mode}';run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*opts,*defines,'-Igenerated/include',sources[cc],w/'provider.c','-o',binary]);out=run([binary],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});outputs.setdefault(profile,out);assert out==outputs[profile],(profile,cc,mode);records.append(dict(profile=profile,compiler=cc,mode=mode,sha256=hashlib.sha256(out).hexdigest()))
 base=sources['gcc'].read_text();changes=[('skip-e1-switch','shell_proofs_b_maybe_run_atlas_phase_e1_click_scene_switch_proof','shell_proofs_b_atlas_switch_scene(ctx,target)','shell_proofs_b_atlas_switch_scene(ctx,from)'),('skip-e2-prev','shell_proofs_b_maybe_run_atlas_phase_e2_keyboard_scene_cycle_proof','shell_proofs_b_atlas_cycle(ctx,-1','shell_proofs_b_atlas_cycle(ctx,1'),('skip-e3-drag','shell_proofs_b_maybe_run_atlas_phase_e3_drag_begin_marker_proof','ctx->atlas->ATLAS_DRAG_INTENT=(AtlasDragIntent){true,active,frame.frame,x,y}','ctx->atlas->ATLAS_DRAG_INTENT=(AtlasDragIntent){false,active,frame.frame,x,y}'),('mutate-e4b','shell_proofs_b_maybe_run_atlas_phase_e4b_same_scene_noop_proof','before.scene,before.scene','before.scene,(uint8_t)(before.scene+1)'),('skip-e4c-transaction','shell_proofs_b_maybe_run_atlas_phase_e4c_cross_scene_reparent_proof','transaction_ok=atlas_proof_move_and_restore','transaction_ok=true || atlas_proof_move_and_restore'),('skip-e4c2-transaction','shell_proofs_b_maybe_run_atlas_phase_e4c2_true_reparent_proof','transaction_ok=atlas_proof_move_and_restore','transaction_ok=true || atlas_proof_move_and_restore'),('wrong-e4d-release','shell_proofs_b_maybe_run_atlas_phase_e4d_real_pointer_drop_proof','ctx,EV_BTN,1,0','ctx,EV_BTN,1,1'),('early-closeout','shell_proofs_b_maybe_run_atlas_overview_final_closeout_proof','&&ctx->atlas->ATLAS_PHASE_E4D_REAL_POINTER_DROP_PROOF_DONE','||ctx->atlas->ATLAS_PHASE_E4D_REAL_POINTER_DROP_PROOF_DONE')];mutants=[]
 for label,function,old,new in changes:
  path=w/f'mutant-{label}.c';path.write_text(mutate_function(base,function,old,new));binary=w/f'mutant-{label}';run(['gcc','-std=c11','-O2',*flags,'-Igenerated/include',path,w/'provider.c','-o',binary]);result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode or result.stdout!=outputs['all'],('survived',label);mutants.append(label)
 original=(HERE/'src/silk_shell_proofs_b_atlas.cesta').read_text();controls=(('readonly-atlas','(field atlas (borrow write))','(field atlas (borrow read))'),('const-e1-context','void shell_proofs_b_maybe_run_atlas_phase_e1_click_scene_switch_proof(ShellInputContext *ctx)','void shell_proofs_b_maybe_run_atlas_phase_e1_click_scene_switch_proof(const ShellInputContext *ctx)'))
 for label,old,new in controls:
  assert original.count(old)==1,(label,original.count(old));path=w/f'{label}.cesta';path.write_text(original.replace(old,new));run([args.cesta,'cc','--cc','gcc','-std=c11','-fsyntax-only','-Igenerated/include',path],accepted=False)
 admission=[]
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
  for i,plan in enumerate(plans.values()):record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,w/f'Atlas{i}.lean',args.lean_root.resolve()))
 (w/'summary.json').write_text(json.dumps(dict(pin=inventory['pin'],functions=len(inventory['functions']),configurations=records,mutants_caught=mutants,source_controls=len(controls),admission=admission),indent=2)+'\n');print('Proofs-B Atlas:',len(inventory['functions']),'current bodies;',len(records),'configurations;',len(mutants),'mutants caught;',sum(x['functions'] for x in admission),'plans admitted')
if __name__=='__main__':main()
