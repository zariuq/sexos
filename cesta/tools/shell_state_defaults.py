#!/usr/bin/env python3
"""Generate scalar Silk Shell defaults from the current Rust declarations."""
import argparse,hashlib,json,re
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
ROOT_COMPONENTS={
 'SilkShellModelsState':'models',
 'SilkShellChromeState':'chrome',
 'SilkShellAtlasState':'atlas',
 'SilkShellSurfacesState':'surfaces',
 'SilkShellInputState':'input',
 'SilkShellProofsAState':'proofs_a',
 'SilkShellProofsBState':'proofs_b',
 'SilkShellCoreState':'core',
}
PROOFS_B={
 'proofs_b.maybe_run_window_drag_synthetic_proof__DEFER_BUDGET':'8',
 'proofs_b.maybe_run_keyboard_window_synthetic_proof__SKIP_DISABLED_BUDGET':'1',
 'proofs_b.maybe_run_keyboard_window_synthetic_proof__SKIP_NO_FOCUS_BUDGET':'16',
 'proofs_b.maybe_run_keyboard_window_synthetic_proof__SKIP_NO_FRAME_BUDGET':'16',
 'proofs_b.maybe_run_keyboard_window_synthetic_proof__SKIP_ALREADY_DONE_BUDGET':'1',
 'proofs_b.maybe_run_keyboard_window_synthetic_proof__STATE_BUDGET':'64',
 'proofs_b.maybe_run_keyboard_window_synthetic_proof__DEFER_BUDGET':'64',
 'proofs_b.maybe_run_keyboard_window_synthetic_proof__FRAME_SURFACE_MAP_BUDGET':'24',
 'proofs_b.maybe_run_visible_focus_topbar_proof__NOFOCUS_BUDGET':'16',
 'proofs_b.maybe_run_visible_focus_topbar_proof__NOFRAME_BUDGET':'16',
 'proofs_b.maybe_run_keyboard_gui_broad_action_proof__SKIP_DISABLED_BUDGET':'1',
 'proofs_b.maybe_run_keyboard_gui_broad_action_proof__SKIP_NO_FOCUS_BUDGET':'16',
 'proofs_b.maybe_run_keyboard_gui_broad_action_proof__SKIP_NO_FRAME_BUDGET':'16',
 'proofs_b.maybe_run_keyboard_gui_broad_action_proof__SKIP_DONE_BUDGET':'1',
 'proofs_b.maybe_run_keyboard_gui_broad_action_proof__STATE_BUDGET':'64',
 'proofs_b.maybe_run_spindle_real_keyboard_focus_proof__SKIP_DISABLED_BUDGET':'1',
 'proofs_b.maybe_run_mesh_keyboard_map_proof__SKIP_DISABLED_BUDGET':'1',
 'proofs_b.maybe_run_frame_light_zoom_synthetic_proof__DISABLED_LOG_BUDGET':'1',
 'proofs_b.maybe_run_frame_light_zoom_synthetic_proof__BUDGET':'1',
 'proofs_b.maybe_run_frame_light_zoom_synthetic_proof__ATTEMPTS':'240',
 'proofs_b.maybe_run_frame_light_zoom_synthetic_proof__DEFER_LOG_BUDGET':'8',
}
def c_value(value):
 value=re.sub(r'(?<=\d)_(?=\d)','',value.strip())
 value=re.sub(r'(?<=\b\d)(?:u8|u16|u32|u64|usize|i8|i16|i32|i64)\b','',value)
 value=value.replace('usize::MAX','SIZE_MAX')
 value=value.replace('u8::MAX','UINT8_MAX').replace('u16::MAX','UINT16_MAX')
 value=value.replace('u32::MAX','UINT32_MAX').replace('u64::MAX','UINT64_MAX')
 value=value.replace('i8::MAX','INT8_MAX').replace('i16::MAX','INT16_MAX')
 value=value.replace('i32::MAX','INT32_MAX').replace('i64::MAX','INT64_MAX')
 if '::' in value or any(c in value for c in '{}[]') or '\n' in value:return None
 if not re.fullmatch(r'[A-Za-z0-9_xXa-fA-F()+\-*/%<>&|!~. ]+',value):return None
 return value
def item_end(text,start):
 depth=brackets=parentheses=0;seen_brace=False;i=start
 while i<len(text):
  if text.startswith('//',i):
   end=text.find('\n',i);i=len(text) if end<0 else end+1;continue
  if text.startswith('/*',i):
   level=1;i+=2
   while level and i<len(text):
    if text.startswith('/*',i):level+=1;i+=2
    elif text.startswith('*/',i):level-=1;i+=2
    else:i+=1
   if level:raise AssertionError(('unterminated block comment',start))
   continue
  if text[i]=='"':
   i+=1
   while i<len(text) and text[i]!='"':i+=2 if text[i]=='\\' else 1
   i+=1;continue
  if text[i]=='[':brackets+=1
  elif text[i]==']':brackets-=1
  elif text[i]=='(':parentheses+=1
  elif text[i]==')':parentheses-=1
  elif text[i]=='{':depth+=1;seen_brace=True
  elif text[i]=='}':
   depth-=1
   if seen_brace and depth==0 and brackets==0 and parentheses==0:return i+1
  i+=1
 raise AssertionError(('unterminated function',text[start:start+100]))
def function_body(source,name):
 pattern=rf'(?m)^\s*(?:pub(?:\([^)]*\))?\s+)?(?:unsafe\s+)?(?:extern\s+"C"\s+)?fn\s+{re.escape(name)}\b'
 matches=list(re.finditer(pattern,source))
 if len(matches)!=1:return None
 start=matches[0].start()
 return source[start:item_end(source,start)]
def component_locals(source):
 header=(HERE/'generated/include/silk_shell.h').read_text()
 assign={}
 for record,member in ROOT_COMPONENTS.items():
  match=re.search(r'typedef struct '+re.escape(record)+r' \{(.*?)\n\}\n'+re.escape(record)+r';',header,re.S)
  if not match:raise AssertionError(('missing shell state record',record))
  fields=re.findall(r'\b([A-Za-z_][A-Za-z0-9_]*__[A-Za-z0-9_]+)\s*;',match.group(1))
  for field in fields:
   function,local=field.split('__',1)
   body=function_body(source,function)
   if body is None:continue
   candidates=[local]
   if re.search(r'_L\d+$',local):candidates.append(re.sub(r'_L\d+$','',local))
   found=[]
   for candidate in candidates:
    found=list(re.finditer(r'(?m)^\s*static\s+mut\s+'+re.escape(candidate)+r'\s*:\s*(?:bool|u8|u16|u32|u64|usize|i8|i16|i32|i64)\s*=\s*([^;]+);',body))
    if found:break
   if len(found)!=1:continue
   value=c_value(found[0].group(1))
   if value is not None:assign[member+'.'+field]=value
 return assign
def render():
 inventory=json.loads((HERE/'interfaces/silk_shell_input.source.json').read_text());source=(HERE.parent/'servers/silk-shell/src/main.rs').read_text();assign={}
 for name,path in inventory['globals'].items():
  match=re.search(r'(?m)^\s*static\s+mut\s+'+re.escape(name)+r'\s*:\s*(?:bool|u8|u16|u32|u64|usize|i8|i16|i32|i64)\s*=\s*([^;]+);',source)
  if match:
   value=c_value(match.group(1))
   if value is not None:assign[path]=value
 for item in inventory['locals']:
  value=c_value(item['initial'])
  if value is not None:assign[item['field']]=value
 for field,value in component_locals(source).items():
  if field in assign and assign[field]!=value:raise AssertionError(('conflicting current scalar default',field,assign[field],value))
  assign[field]=value
 assign.update(PROOFS_B)
 lines=['#ifndef SEXOS_SILK_SHELL_STATE_DEFAULTS_H','#define SEXOS_SILK_SHELL_STATE_DEFAULTS_H','#include "silk_shell.h"','static inline void silk_shell_apply_static_defaults(SilkShellState *state)','{']
 for path,value in sorted(assign.items()):lines.append(' state->'+path+' = '+value+';')
 lines+=['}','#define SILK_SHELL_STATIC_DEFAULT_COUNT '+str(len(assign)),'#define SILK_SHELL_STATIC_DEFAULT_SOURCE_SHA256 "'+hashlib.sha256(source.encode()).hexdigest()+'"','#endif','']
 return '\n'.join(lines).encode(),len(assign)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--output',type=Path,default=HERE/'generated/include/silk_shell_state_defaults.h');ap.add_argument('--check',action='store_true');args=ap.parse_args();content,count=render()
 if args.check:assert args.output.read_bytes()==content,'Silk Shell defaults changed; regenerate and qualify'
 else:args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(content)
 print('Silk Shell scalar defaults:',count)
if __name__=='__main__':main()
