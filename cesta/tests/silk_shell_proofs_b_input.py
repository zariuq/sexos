#!/usr/bin/env python3
"""Qualify the current Proofs-B input/window partition."""
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
 pin=subprocess.run(['git','show',inventory['pin']+':servers/silk-shell/src/main.rs'],cwd=HERE.parent,capture_output=True,text=True,check=True).stdout;working=(HERE.parent/'servers/silk-shell/src/main.rs').read_text()
 for row in inventory['functions']:
  for label,source in (('pin',pin),('working',working)):
   match=re.search(r'(?m)^(?:unsafe )?fn '+re.escape(row['name'])+r'\b',source);assert match,(label,row['name']);body=source[match.start():item_end(source,match.start())];assert hashlib.sha256(body.encode()).hexdigest()==row['source_sha256'],(label,row['name'])
def provider():
 return r'''
#include "silk_shell_proofs_b_input.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static SilkShellChromeState chrome;static SilkShellModelsState models;static ShellChromeApps apps;static SilkShellInputState input;static SilkShellProofsAState proofs_a;static SilkShellAtlasState atlas;static SilkShellSurfacesState surfaces;static SilkShellProofsBState proofs_b;static unsigned calls,hid,actions,cursors,zooms;static uint64_t unknown_class;static bool minimized;
static ShellProofsBInputContext context(void){ShellChromeContext shell={&chrome,&models,&apps,&input,&proofs_a};return(ShellProofsBInputContext){{shell,&atlas,&surfaces},&proofs_b};}
static ShellFrame *frame_by_surface(uint64_t surface){for(size_t i=0;i<MAX_FRAMES;++i)if(chrome.FRAMES[i].present)for(size_t j=0;j<chrome.FRAMES[i].tab_count;++j)if(chrome.FRAMES[i].tabs[j].present&&chrome.FRAMES[i].tabs[j].surface_id==surface)return &chrome.FRAMES[i];return 0;}
ShellProofsBDragTarget shell_proofs_b_input_drag_target(ShellProofsBInputContext *ctx){(void)ctx;return(ShellProofsBDragTarget){true,1,SURFACE_ID_STATIC,10,20,300,200,30,60};}
ShellProofsBBounds shell_proofs_b_input_bounds(ShellProofsBInputContext *ctx,uint64_t surface){(void)ctx;ShellFrame *frame=frame_by_surface(surface);return frame?(ShellProofsBBounds){true,frame->normal_x,frame->normal_y,frame->normal_w,frame->normal_h}:(ShellProofsBBounds){0};}
ShellProofsBFrame shell_proofs_b_input_frame_for_surface(ShellProofsBInputContext *ctx,uint64_t surface){(void)ctx;ShellFrame *frame=frame_by_surface(surface);return frame?(ShellProofsBFrame){true,frame->frame_id}:(ShellProofsBFrame){0};}
ShellProofsBFrame shell_proofs_b_input_first_minimized(ShellProofsBInputContext *ctx){(void)ctx;return(ShellProofsBFrame){minimized,TEST3_FRAME_ID};}
ShellProofsBId shell_proofs_b_input_active_surface(ShellProofsBInputContext *ctx,uint32_t frame){(void)ctx;for(size_t i=0;i<MAX_FRAMES;++i)if(chrome.FRAMES[i].present&&chrome.FRAMES[i].frame_id==frame&&chrome.FRAMES[i].tab_count)return(ShellProofsBId){true,chrome.FRAMES[i].tabs[chrome.FRAMES[i].active_tab].surface_id};return(ShellProofsBId){0};}
bool shell_proofs_b_input_focus(ShellProofsBInputContext *ctx,uint64_t surface){++calls;if(!frame_by_surface(surface))return false;ctx->input.shell.chrome->FOCUSED_SURFACE_ID=surface;return true;}
bool shell_proofs_b_input_action(ShellProofsBInputContext *ctx,uint8_t action){(void)ctx;(void)action;++calls;++actions;return true;}
bool shell_proofs_b_input_surface_alive(ShellProofsBInputContext *ctx,uint64_t surface){(void)ctx;if(surface==SURFACE_ID_TEST3)return chrome.SURFACE_102_ALIVE;return surface==SURFACE_ID_QUIL||surface==SURFACE_ID_LINEN||surface==SURFACE_ID_STATIC;}
bool shell_proofs_b_input_tombstoned(ShellProofsBInputContext *ctx,uint64_t surface){(void)ctx;(void)surface;return false;}
bool shell_proofs_b_input_quil_has_focus(ShellProofsBInputContext *ctx){return ctx->input.shell.chrome->FOCUSED_SURFACE_ID==SURFACE_ID_QUIL;}
void shell_proofs_b_input_hid(ShellProofsBInputContext *ctx,uint64_t cls,uint64_t arg0,uint64_t arg1){++calls;++hid;if(cls!=EV_KEY&&cls!=EV_BTN&&cls!=EV_ABS&&cls!=EV_REL)unknown_class=cls;if(cls==EV_BTN){ctx->input.shell.input->INTERACTION.tag=arg1?INTERACTION_DRAGGING:INTERACTION_IDLE;return;}if(cls!=EV_KEY||arg1!=1)return;if(arg0==0x1c&&ctx->input.shell.chrome->FOCUSED_SURFACE_ID==SURFACE_ID_TEST3){minimized=true;ctx->input.shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_QUIL;}else if(arg0==0x49)minimized=false;else if(arg0==0x57&&ctx->input.shell.chrome->FOCUSED_SURFACE_ID==SURFACE_ID_TEST3){ctx->input.shell.chrome->SURFACE_102_ALIVE=false;ShellFrame *frame=frame_by_surface(SURFACE_ID_TEST3);if(frame)memset(frame,0,sizeof *frame);ctx->input.shell.chrome->FOCUSED_SURFACE_ID=SURFACE_ID_QUIL;}}
void shell_proofs_b_input_cursor(ShellProofsBInputContext *ctx,int32_t x,int32_t y,const char *source){(void)source;++calls;++cursors;ctx->input.shell.input->POINTER_X=x;ctx->input.shell.input->POINTER_Y=y;}
void shell_proofs_b_input_transition(ShellProofsBInputContext *ctx,uint8_t tag){++calls;ctx->input.shell.input->INTERACTION.tag=tag;}
bool shell_proofs_b_input_zoom_click(ShellProofsBInputContext *ctx,uint32_t frame){(void)ctx;(void)frame;++calls;++zooms;return true;}
void shell_proofs_b_input_display_create(ShellProofsBInputContext *ctx,uint64_t surface,int32_t x,int32_t y,uint32_t w,uint32_t h){(void)ctx;(void)surface;(void)x;(void)y;(void)w;(void)h;++calls;}
bool shell_proofs_b_input_lifecycle_visible(ShellProofsBInputContext *ctx,uint64_t surface){(void)ctx;(void)surface;++calls;return true;}
int32_t shell_proofs_b_atlas_desktop_width(ShellInputContext *ctx){(void)ctx;return 1280;}
int32_t shell_chrome_desktop_height(ShellChromeContext *ctx){(void)ctx;return 720;}
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);putchar('\n');va_end(ap);}
static void seed_frame(size_t i,uint32_t id,uint64_t surface,int32_t x,int32_t y,uint32_t w,uint32_t h){chrome.FRAMES[i]=(ShellFrame){.present=true,.frame_id=id,.active_tab=0,.tab_count=1,.scene_id=0,.flags=FRAME_FLAG_TOP_BAR,.normal_x=x,.normal_y=y,.normal_w=w,.normal_h=h};chrome.FRAMES[i].tabs[0]=(ShellTab){true,surface,0,0};}
static void reset(void){memset(&chrome,0,sizeof chrome);memset(&models,0,sizeof models);memset(&apps,0,sizeof apps);memset(&input,0,sizeof input);memset(&proofs_a,0,sizeof proofs_a);memset(&atlas,0,sizeof atlas);memset(&surfaces,0,sizeof surfaces);memset(&proofs_b,0,sizeof proofs_b);calls=hid=actions=cursors=zooms=0;unknown_class=0;minimized=false;input.ABS_SEEN_VALID=true;input.INTERACTION.tag=INTERACTION_IDLE;seed_frame(0,1,SURFACE_ID_STATIC,10,20,300,200);seed_frame(1,QUIL_FRAME_ID,SURFACE_ID_QUIL,400,30,300,200);seed_frame(2,LINEN_FRAME_ID,SURFACE_ID_LINEN,50,300,300,150);chrome.FOCUSED_SURFACE_ID=SURFACE_ID_QUIL;proofs_b.maybe_run_window_drag_synthetic_proof__DEFER_BUDGET=8;proofs_b.maybe_run_keyboard_window_synthetic_proof__SKIP_DISABLED_BUDGET=1;proofs_b.maybe_run_keyboard_window_synthetic_proof__SKIP_NO_FOCUS_BUDGET=16;proofs_b.maybe_run_visible_focus_topbar_proof__NOFOCUS_BUDGET=16;proofs_b.maybe_run_visible_focus_topbar_proof__NOFRAME_BUDGET=16;proofs_b.maybe_run_frame_light_zoom_synthetic_proof__DISABLED_LOG_BUDGET=1;proofs_b.maybe_run_frame_light_zoom_synthetic_proof__BUDGET=1;proofs_b.maybe_run_frame_light_zoom_synthetic_proof__ATTEMPTS=240;proofs_b.maybe_run_frame_light_zoom_synthetic_proof__DEFER_LOG_BUDGET=8;}
#if !SEXOS_WINDOW_DRAG_PROOF
static void defaults(void){reset();ShellProofsBInputContext ctx=context();shell_proofs_b_maybe_run_window_drag_synthetic_proof(&ctx);shell_proofs_b_maybe_run_silk_de_integrated_interaction_proof(&ctx);shell_proofs_b_maybe_run_keyboard_window_synthetic_proof(&ctx);shell_proofs_b_maybe_run_visible_focus_topbar_proof(&ctx);shell_proofs_b_maybe_run_keyboard_safe_close_proof(&ctx);shell_proofs_b_maybe_run_keyboard_gui_broad_action_proof(&ctx);shell_proofs_b_maybe_run_spindle_real_keyboard_focus_proof(&ctx);shell_proofs_b_maybe_run_mesh_keyboard_map_proof(&ctx);shell_proofs_b_maybe_run_frame_light_zoom_synthetic_proof(&ctx);shell_proofs_b_maybe_run_input_negative_proof(&ctx);shell_proofs_b_maybe_run_cursor_autodemo(&ctx);assert(!proofs_a.SILK_DE_INTEGRATED_PROOF_DONE&&!proofs_b.KEYBOARD_SAFE_CLOSE_PROOF_DONE);puts("DEFAULT ok");}
#endif
#if SEXOS_WINDOW_DRAG_PROOF
static void enabled(void){ShellProofsBInputContext ctx;
 reset();ctx=context();for(size_t i=0;i<5;++i)shell_proofs_b_maybe_run_window_drag_synthetic_proof(&ctx);assert(proofs_a.WINDOW_DRAG_PROOF_STAGE==5&&input.INTERACTION.tag==INTERACTION_IDLE&&proofs_b.maybe_run_window_drag_synthetic_proof__TARGET_SID==SURFACE_ID_STATIC);puts("DRAG ok");
 reset();ctx=context();shell_proofs_b_maybe_run_silk_de_integrated_interaction_proof(&ctx);assert(proofs_a.SILK_DE_INTEGRATED_PROOF_DONE&&hid==6&&input.INTERACTION.tag==INTERACTION_IDLE);puts("INTEGRATED ok");
 reset();ctx=context();shell_proofs_b_maybe_run_keyboard_window_synthetic_proof(&ctx);assert(proofs_a.KEYBOARD_WINDOW_PROOF_STAGE==6&&actions==5);puts("KEYBOARD ok");
 reset();ctx=context();shell_proofs_b_maybe_run_visible_focus_topbar_proof(&ctx);assert(proofs_b.VISIBLE_FOCUS_TOPBAR_PROOF_DONE&&proofs_b.VISIBLE_FOCUS_TOPBAR_PROOF_STAGE==7&&actions==6);puts("TOPBAR ok");
 reset();ctx=context();shell_proofs_b_maybe_run_keyboard_safe_close_proof(&ctx);assert(proofs_b.KEYBOARD_SAFE_CLOSE_PROOF_DONE&&!chrome.SURFACE_102_ALIVE&&!frame_by_surface(SURFACE_ID_TEST3)&&chrome.FOCUSED_SURFACE_ID==SURFACE_ID_QUIL);puts("SAFE CLOSE ok");
 reset();ctx=context();shell_proofs_b_maybe_run_keyboard_gui_broad_action_proof(&ctx);assert(proofs_a.KEYBOARD_GUI_BROAD_PROOF_DONE&&proofs_a.KEYBOARD_GUI_BROAD_PROOF_STAGE==14);puts("BROAD ok");
 reset();ctx=context();shell_proofs_b_maybe_run_spindle_real_keyboard_focus_proof(&ctx);assert(proofs_a.SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_DONE&&proofs_a.SPINDLE_REAL_KEYBOARD_FOCUS_PROOF_STAGE==9);puts("SPINDLE ok");
 reset();ctx=context();shell_proofs_b_maybe_run_mesh_keyboard_map_proof(&ctx);assert(proofs_a.MESH_KEYBOARD_MAP_PROOF_DONE&&proofs_a.MESH_KEYBOARD_MAP_PROOF_STAGE==8);puts("MESH ok");
 reset();ctx=context();shell_proofs_b_maybe_run_frame_light_zoom_synthetic_proof(&ctx);assert(!proofs_b.maybe_run_frame_light_zoom_synthetic_proof__BUDGET&&zooms==1&&input.INTERACTION.tag==INTERACTION_IDLE);puts("ZOOM ok");
 reset();ctx=context();unsigned before=hid;shell_proofs_b_maybe_run_input_negative_proof(&ctx);assert(proofs_b.maybe_run_input_negative_proof__DONE&&hid==before+3&&unknown_class==99);puts("NEGATIVE ok");
 reset();ctx=context();for(size_t i=0;i<24;++i)shell_proofs_b_maybe_run_cursor_autodemo(&ctx);assert(proofs_b.maybe_run_cursor_autodemo__AUTODEMO_DONE&&proofs_b.maybe_run_cursor_autodemo__AUTODEMO_MOVES==24&&cursors==24);puts("CURSOR ok");}
#endif
int main(void){
#if SEXOS_WINDOW_DRAG_PROOF
 enabled();
#else
 defaults();
#endif
 return 0;}
'''
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path);ap.add_argument('--lean-root',type=Path);ap.add_argument('--output',type=Path,default=HERE/'build/proofs-b-input');args=ap.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);inventory=json.loads((HERE/'interfaces/silk_shell_proofs_b_input.source.json').read_text());verify_source(inventory);(w/'provider.c').write_text(provider());plans={};sources={}
 for cc in ('gcc','clang'):
  source=w/f'input-{cc}.c';plan=w/f'input-{cc}.plan';run([args.cesta,'export-c','src/silk_shell_proofs_b_input.cesta','--cc',cc,'--std=c11','-o',source,'--plan',plan,'--','-Igenerated/include']);plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan;sources[cc]=source
 flags=['-D'+row['flag']+'=1' for row in inventory['functions']];records=[];outputs={}
 for profile,defines in (('default',[]),('all',flags)):
  for cc in ('gcc','clang'):
   for mode,opts in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
    binary=w/f'{profile}-{cc}-{mode}';run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*opts,*defines,'-Igenerated/include',sources[cc],w/'provider.c','-o',binary]);out=run([binary],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});outputs.setdefault(profile,out);assert out==outputs[profile],(profile,cc,mode);records.append(dict(profile=profile,compiler=cc,mode=mode,sha256=hashlib.sha256(out).hexdigest()))
 base=sources['gcc'].read_text();changes=[('wrong-drag-step','shell_proofs_b_maybe_run_window_drag_synthetic_proof','target.x+120','target.x+121'),('leave-integrated-drag','shell_proofs_b_maybe_run_silk_de_integrated_interaction_proof','shell_proofs_b_input_hid(ctx,EV_BTN,1,0);sexos_line("[silk.de.integrated.interaction.evidence]','shell_proofs_b_input_hid(ctx,EV_BTN,1,1);sexos_line("[silk.de.integrated.interaction.evidence]'),('short-keyboard','shell_proofs_b_maybe_run_keyboard_window_synthetic_proof','i<6','i<5'),('short-topbar','shell_proofs_b_maybe_run_visible_focus_topbar_proof','i<7','i<6'),('wrong-close-key','shell_proofs_b_maybe_run_keyboard_safe_close_proof','ctx,EV_KEY,0x57,1','ctx,EV_KEY,0x56,1'),('short-broad','shell_proofs_b_maybe_run_keyboard_gui_broad_action_proof','i<13','i<12'),('short-spindle','shell_proofs_b_maybe_run_spindle_real_keyboard_focus_proof','i<9','i<8'),('short-mesh','shell_proofs_b_maybe_run_mesh_keyboard_map_proof','i<9','i<7'),('skip-zoom','shell_proofs_b_maybe_run_frame_light_zoom_synthetic_proof','shell_proofs_b_input_zoom_click(ctx,QUIL_FRAME_ID)','true'),('skip-negative','shell_proofs_b_maybe_run_input_negative_proof','ctx,99,0,0','ctx,98,0,0'),('short-cursor','shell_proofs_b_maybe_run_cursor_autodemo','>=24','>=23')];mutants=[]
 for label,function,old,new in changes:
  path=w/f'mutant-{label}.c';path.write_text(mutate_function(base,function,old,new));binary=w/f'mutant-{label}';run(['gcc','-std=c11','-O2',*flags,'-Igenerated/include',path,w/'provider.c','-o',binary]);result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode or result.stdout!=outputs['all'],('survived',label);mutants.append(label)
 original=(HERE/'src/silk_shell_proofs_b_input.cesta').read_text();controls=(('readonly-proofs-b','(field proofs_b (borrow write))','(field proofs_b (borrow read))'),('const-drag-context','void shell_proofs_b_maybe_run_window_drag_synthetic_proof(ShellProofsBInputContext *ctx)','void shell_proofs_b_maybe_run_window_drag_synthetic_proof(const ShellProofsBInputContext *ctx)'))
 for label,old,new in controls:
  assert original.count(old)==1,(label,original.count(old));path=w/f'{label}.cesta';path.write_text(original.replace(old,new));run([args.cesta,'cc','--cc','gcc','-std=c11','-fsyntax-only','-Igenerated/include',path],accepted=False)
 admission=[]
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
  for i,plan in enumerate(plans.values()):record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,w/f'Input{i}.lean',args.lean_root.resolve()))
 (w/'summary.json').write_text(json.dumps(dict(pin=inventory['pin'],functions=len(inventory['functions']),configurations=records,mutants_caught=mutants,source_controls=len(controls),admission=admission),indent=2)+'\n');print('Proofs-B input:',len(inventory['functions']),'current bodies;',len(records),'configurations;',len(mutants),'mutants caught;',sum(x['functions'] for x in admission),'plans admitted')
if __name__=='__main__':main()
