"""Current lifecycle/frame slice, with named generation corrections tested independently."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import re
import subprocess
from silk_shell_input_differential import declarations, identifiers, item_end

HERE=Path(__file__).resolve().parent.parent
ROOT=HERE.parent
FIX=HERE/'tests/silk_shell_chrome'
INVENTORY=HERE/'interfaces/silk_shell_chrome.source.json'
MASK=(1<<64)-1


def run(command,data=None):
    result=subprocess.run(list(map(str,command)),cwd=HERE,input=data,capture_output=True,
                          env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                               'UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})
    assert result.returncode==0,(command,result.returncode,result.stderr[-5000:])
    return result.stdout


def prepare(work,inventory,provider=None,module_overrides=None,extra_impls=()):
    source=run(['git','show',inventory['pin']+':servers/silk-shell/src/main.rs']).decode()
    assert hashlib.sha256(source.encode()).hexdigest()==inventory['source_sha256']
    for name,digest in inventory['dependencies'].items():
        assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest,name
    pdx_source=run(['git','show',inventory['pin']+':crates/sex-pdx/src/lib.rs']).decode()
    catalog=declarations(pdx_source)
    shell_library=''
    if extra_impls:
        shell_library=run(['git','show',inventory['pin']+':servers/silk-shell/src/lib.rs']).decode()
        catalog.update(declarations(shell_library))
    catalog.update(declarations(source))
    bodies=[];needed={'P','Rect','APP_CUSTODY','FRAMES','LIFECYCLE_TABLE','FOCUSED_SURFACE','FOCUSED_SURFACE_ID',
        'LIFECYCLE_GENERATION','HOVERED_FRAME_ID','HOVER_KIND','HOVERED_FRAME_LIGHT','ACTIVE_SCENE_IDX',
        'APP_SURFACES','ACTIVE_TINT_IDX','DESKTOP_WIDTH','DESKTOP_HEIGHT',*[f'SURFACE_{i}_{axis}'for i in ('200','201','202','203','204','205','0x99')for axis in 'XYWH']}
    for row in inventory['functions']:
        m=re.search(r'(?m)^(?:pub )?(?:unsafe )?(?:const )?fn '+row['name']+r'\b',source)
        raw=source[m.start():item_end(source,m.start())]
        assert hashlib.sha256((raw+'\n').encode()).hexdigest()==row['sha256'],row['name']
        bodies.append(raw);needed|=identifiers(raw)&catalog.keys()
    m=re.search(r'(?m)^const fn native_lifecycle_surface_count\b',source)
    bodies.append(source[m.start():item_end(source,m.start()+6)])
    m=re.search(r'(?m)^impl Rect\b',source)
    rect=source[m.start():item_end(source,m.start())]
    assert hashlib.sha256(rect.encode()).hexdigest()==inventory['rect_impl_sha256']
    bodies.append(rect)
    for name in extra_impls:
        implementation=source
        m=re.search(r'(?m)^impl '+re.escape(name)+r'\b',implementation)
        if not m:
            implementation=pdx_source
            m=re.search(r'(?m)^impl '+re.escape(name)+r'\b',implementation)
        if not m:
            implementation=shell_library
            m=re.search(r'(?m)^impl '+re.escape(name)+r'\b',implementation)
        assert m,('missing implementation',name)
        raw=implementation[m.start():item_end(implementation,m.start())]
        bodies.append(raw);needed|=identifiers(raw)&catalog.keys()
    pending=list(needed);selected={}
    while pending:
        name=pending.pop()
        if name in selected:continue
        selected[name]=catalog[name]
        pending.extend((identifiers(catalog[name])&catalog.keys())-selected.keys())
    prefix='''#![allow(dead_code,unused_imports,unused_variables,unused_mut,static_mut_refs,unused_attributes)]
extern crate self as sex_pdx;
extern crate alloc;use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool,AtomicU32,Ordering};
macro_rules! serial_println {($($arg:tt)*)=>{println!($($arg)*)};}
#[macro_export]macro_rules! serial_record {($($arg:tt)*)=>{println!($($arg)*)};}
'''
    # Preserve the production module bodies; append harness inspection and setup methods.
    for module in ('document_view','spindle_view'):
        raw=(ROOT/('crates/sex-pdx/src/'+module+'.rs')).read_text()
        extra='\nimpl Delivery { pub fn test_last(&mut self,w:u32,h:u32,sequence:u64){self.last=Some((w,h));self.sequence=sequence;} pub fn trace_chrome(&self){\n let n=self.desired.unwrap_or(Notice{target:0,task:0,w:0,h:0,sequence:0});let(w,h)=self.last.unwrap_or((0,0));\n print!("|V {} {} {} {} {} {} {} {} {} {} {}",self.sequence,self.dead as u8,self.desired.is_some()as u8,self.last.is_some()as u8,n.target,n.task,n.w,n.h,n.sequence,w,h);\n}}\n'
        path=work/(module+'.rs');path.write_text(raw+extra)
        prefix+='#[path='+json.dumps(str(path))+']mod '+module+';\n'
    delivery=(ROOT/'servers/silk-shell/src/linen_text_delivery.rs').read_text()
    values=declarations(delivery)
    selected_delivery=[values[name]for name in ('Operation','Admission','LatestOperation','LatestPoll','DESKTOP_SURFACES','DesktopStates')]
    for name in ('LatestOperation','DesktopStates'):
        m=re.search(r'(?m)^impl '+name+r'\b',delivery);selected_delivery.append(delivery[m.start():item_end(delivery,m.start())])
    if inventory.get('full_text_delivery'):
        selected_delivery=[delivery.replace('//!','//')+inventory.get('delivery_trace','')]
        for module,path_name in [('text_encoding','crates/sex-pdx/src/text_encoding.rs'),('text_batch','crates/sex-pdx/src/text_batch.rs')]:
            prefix+='#[path='+json.dumps(str(ROOT/path_name))+']mod '+module+';\n'
    path=work/'linen_text_delivery.rs';path.write_text('use crate as sex_pdx;\n'+'\n'.join(selected_delivery)+'\nimpl LatestOperation {pub fn trace_chrome(&self){\n if let Some(op)=self.operation{print!("|P 1 {} {} {} {}",op.opcode,op.surface,op.packed,op.control)}else{print!("|P 0")}\n}}\nimpl DesktopStates {pub fn test_ready(&mut self,surface:u64,denied:bool,pending:bool){\n if let Some(i)=DESKTOP_SURFACES.iter().position(|s|*s==surface){self.denied[i]=denied;self.slots[i]=if pending{Some(Operation{opcode:0xEE,surface,packed:0,control:0})}else{None};}\n}}\n')
    prefix+='#[path='+json.dumps(str(path))+']mod linen_text_delivery;\n'
    if 'ERR_SERVICE_NOT_READY'not in selected:
        prefix+='pub const ERR_SERVICE_NOT_READY:u64=0xfffffffffffffffe;\n'
    for name,path in [('desktop_style','crates/sex-pdx/src/desktop_style.rs'),('desktop_content','servers/silk-shell/src/desktop_content.rs'),('silkbar_model','crates/silkbar-model/src/lib.rs'),('app_catalog','crates/sex-pdx/src/app_catalog.rs'),('app_custody','servers/silk-shell/src/app_custody.rs'),
                      ('app_lifecycle','crates/sex-pdx/src/app_lifecycle.rs')]:
        module=(module_overrides or {}).get(name,ROOT/path)
        prefix+='#[path='+json.dumps(str(module))+'] mod '+name+';\n'
    prefix+='use silkbar_model::{UpdateKind,OPTION_MOVE,OPTION_CLOSE,OPTION_MINIMIZE,OPTION_ZOOM,Action,hit_test_action,DEFAULT_SILK_BAR,PANEL_X,PANEL_Y,PANEL_W,PANEL_H};\n'
    diagnostics=inventory.get('diagnostics',[])
    fields=[row['field']for row in diagnostics]
    (work/'chrome_budgets.h').write_text('static void init_budgets(void){atomic_init(&input.INTERACTION_LOG_BUDGET,16);'+''.join('chrome.'+row['field']+'='+str(row['value'])+';'for row in diagnostics)+'}\n'+
        'static void reset_chrome(void){'+''.join('uint32_t saved'+str(i)+'=chrome.'+field+';'for i,field in enumerate(fields))+
        'bool mesh_seen=atomic_load(&chrome.tile_active_scene_frames__MESH_VISIBLE_MARK);memset(&chrome,0,sizeof(chrome));atomic_init(&chrome.tile_active_scene_frames__MESH_VISIBLE_MARK,mesh_seen);'+''.join('chrome.'+field+'=saved'+str(i)+';'for i,field in enumerate(fields))+'}\n')
    (work/'oracle.rs').write_text(prefix+'\n'.join(selected.values())+'\n'+'\n'.join(bodies)+'\n'+(provider or FIX/'provider.rs').read_text())
    run(['rustc','--edition=2021','-O',work/'oracle.rs','-o',work/'rust'])


def corpus():
    rows=[]
    def add(*v):rows.append(' '.join(str(n&MASK)for n in v))
    for key in range(256):add(0,key)
    for action in range(47):add(1,action)
    for sid in (0,100,101,102,103,144,145,146,153,154,156,200,201,300,301,354,999):
        for op in (9,11,13,16,17,19,20,21,24):add(op,sid)
    for width in (1,320,1280,2147483647):
        for height in (50,51,720,2147483647):
            add(100);add(103,100,0,0,0,0,width,height)
            for op in (2,3,4,5):add(op)
            for x,y,w,h in [(-1,-1,0,0),(10,60,120,80),(2147483647,-2147483648,0xffffffff,0xffffffff),(0,0,300,100)]:
                add(6,x,y,w,h);add(7,w,h)
    for state in range(8):
        add(100);add(8,100,state);add(13,100);add(14)
        for next_state in range(8):
            add(10,100,next_state);add(9,100);add(11,100);add(14);add(16,100);add(17,100)
        add(12,100);add(15);add(12,999)
    add(100);add(18)
    add(100)
    for sid in range(1000,1043):add(8,sid,2)
    for seed in (53,71,313):
        rng=random.Random(seed)
        for scenario in range(64):
            add(100)
            for slot in range(9):
                count=rng.randrange(9);tabs=[100+slot*8+j if j<count else 0 for j in range(8)]
                add(101,slot,slot+1,rng.randrange(8),count,rng.randrange(5),rng.randrange(8),*tabs)
            add(103,rng.randrange(100,173),rng.randrange(5),rng.randrange(10),rng.randrange(5),rng.randrange(4),1280,720)
            for sid in range(100,173):add(104,sid,rng.randrange(4))
            for fid in range(11):
                for op in (22,26,28,30,33,34,36,37):add(op,fid)
                add(38,fid,rng.randrange(-10,450),rng.randrange(100))
                add(39,fid,rng.randrange(8),rng.randrange(8))
                add(27,fid,rng.randrange(2));add(29,fid,rng.randrange(2));add(31,fid,rng.randrange(2))
            for op in (23,24,25,32,35):add(op)
            for sid in (100,101,156,153,201,300,338,354,999):add(20,sid);add(21,sid)
    for active in range(3):
        for closed in range(3):
            add(100);add(18)
            for sid in (100,101,102):add(10,sid,2)
            add(101,0,19,active,3,0,0,100,101,102)
            add(47,100+closed)
    for index in range(28):
        for dead in (0,1):
            for closing in (0,1):
                for bounds in (0,1):
                    add(100);add(105,index,1000+index,dead,closing,bounds)
                    for sid in (153,154,156,201,300,301,302,306,338,339,343,352,353,354,355):add(20,sid)
    for sid in (100,101,102,103,200):
        for flags in range(8):
            add(100);add(101,0,1,0,3,0,flags,sid,sid+1,sid+2)
            for x in range(-10,410,5):
                for y in (sid%57-1,sid%57,sid%57+3,sid%57+4,sid%57+27,sid%57+28):add(38,1,x,y)
    for screen in (-2147483648,-1,0,1,32767,2147483647):
        for dim in (-2147483648,-1,0,1,2,1280,2147483647):add(41,screen,dim)
    for value in range(8):add(40,value)
    for seed in (97,317):
        rng=random.Random(seed)
        for _ in range(600):
            x,y,dx,dy=[rng.choice((-2147483648,-1,0,1,50,1000,2147483647))for _ in range(4)]
            w,h=[rng.choice((0,1,100,0x7fffffff,0x80000000,0xffffffff))for _ in range(2)]
            add(42,dx,dy,x,y,w,h);add(43,x,y,w,h,rng.randrange(8),dx,dy);add(44,x,y,w,h,dx,dy)
    # Exercise lifecycle actions on registered, initially visible metadata.
    boot={100,101,102,103,200,201,202,203,204,153,144,146,147,148,149,150,151}
    for sid in (100,101,102,103,200,201,202,203,204,152,153,300,301,306,354):
        add(100);add(18);add(121,1,3,0)
        for surface in (sid,sid+1,sid+2):
            add(10,surface,2)if surface in boot else add(8,surface,2)
        add(101,0,19,1,3,0,4,sid+1,sid,sid+2)
        add(103,sid,0,19,1,1,1280,720)
        add(45,sid);add(46,19);add(51,19);add(57,19);add(58,19)
        add(52,sid,10,60,400,250);add(53,19);add(54,19);add(55,19);add(55,19)
        add(48,19);add(49,19);add(48,19);add(50,19,0)
        add(120,1,sid);add(47,sid)
    for index in range(28):
        for dead,closing,bounds in ((0,0,1),(1,0,1),(0,1,1),(0,0,0)):
            add(100);add(18);add(105,index,1000+index,dead,closing,bounds)
            # The immutable catalog chooses the actual frame; child dimensions
            # and notices remain data, never a surface ownership bind.
            table=(HERE/'interfaces/shell_apps.data').read_text().split('(Table BootLifecycle')[0]
            values=re.findall(r'\(Row ([^\n)]+)\)',table)[index].split()
            surface=int(values[3]);frame=int(values[4])or surface
            for sid in dict.fromkeys((surface,frame)):
                if sid not in boot:add(8,sid,2)
                else:add(10,sid,2)
                for x,y,w,h in ((10,60,400,250),(-2147483648,2147483647,0xffffffff,0xffffffff),(0,0,0,0)):
                    add(52,sid,x,y,w,h);add(56,sid,x,y,w,h)
            add(45,frame)
            add(101,0,19,0,1,0,0,frame)
            add(48,19);add(49,19)
            if not dead and not closing:add(47,frame)
    for count in range(10):
        add(100);add(18)
        for index in range(count):
            sid=100+index
            add(10,sid,2)if sid<104 else add(8,sid,2)
            add(101,index,index+1,0,1,0,0,sid)
        add(59);add(60)
    for top in (0,4):
        for sid in (100,201,202,300):
            add(100);add(18)
            if sid==300:add(8,sid,2)
            else:add(10,sid,2)
            add(101,0,19,0,3,0,top,sid,sid+1,sid+2)
            for index in range(8):add(63,19,index)
            for x in range(0,100,4):
                for y in (sid%57-1,sid%57,sid%57+3,sid%57+4,sid%57+27,sid%57+28):add(61,19,x,y)
            add(103,sid,0,0,0,0,1280,720);add(62);add(65,sid,0)
    for sid in (0,100,101,102,103,145,152,153,156,200,201,202,203,204,300,301,999):
        for state in range(8):
            add(100);add(8,sid,state);add(64,sid)
    for kind in (0,1,2):
        add(100);add(18);add(10,100,2);add(101,0,19,0,2,0,4,100,101)
        add(124,kind,100 if kind==1 else 19,1,0);add(66,5,100);add(66,5,100)
    # Labels include embedded NULs, invalid UTF-8 and the entire bounded prefix.
    labels=[b'',b'ASCII',b'a\0b',bytes(range(32)),b'\xff'*32,
            'Žluťoučký'.encode(),b'\xc0\xaf',b'\xed\xa0\x80',b'\xf4\x90\x80\x80',b'\xe2\x82']
    for length in (0,1,15,16,30,31,32,64,MASK):
        for label in labels:add(67,length,0xA5,*label[:32].ljust(32,b'\0'))
    for active in range(5):
        add(100);add(103,0,active,0,0,0,1280,720)
        for scene in range(5):
            for flags in (0,1,2,3,31,255):
                for label in labels:
                    add(126,scene,flags,*label[:16].ljust(16,b'\0'))
                    add(69,scene,scene)
        for index in (63,64,65,MASK):
            for scene in (0,4,5,255):add(69,index,scene)
    for frame in (1,2,3,4,5,6,7,8,9,19,0xffffffff):
        for flags in range(8):
            for scene in range(5):
                add(100);add(101,0,frame,0,1,scene,flags,100)
                add(103,100,0,0,0,0,1280,720)
                for alive in range(4):
                    add(104,100,alive);add(70,0,0)
                add(70,63,0);add(70,64,0)
    for sid in (0,100,201,300,999):
        for alive in range(4):
            for action in range(47):
                add(100);add(18)
                add(10,sid,2)if sid in (100,201)else add(8,sid,2)
                add(101,0,19,0,1,0,0,sid)
                add(103,sid,0,0,0,0,1280,720);add(104,sid,alive)
                add(68,sid,19,0);add(71);add(73);add(72,action)
    for forward in (0,1):
        add(100);add(18);add(10,100,2);add(10,101,2)
        # Duplicate annotations must not trap traversal at the same target.
        for i,sid in enumerate((100,100,101,101)):
            add(127,i,1,4,1,sid,0,0)
        for length in (0,1,2,3,4,64,65,MASK):
            add(103,100,0,0,0,0,1280,720);add(74,length,forward)
    # All interaction transitions with nontrivial signed and unsigned payloads.
    def state(op,tag,seed=0):
        add(op,tag,100+seed,(-1 if seed else 0),2147483647,seed%5,seed%8,
            -2147483648,2147483647,0xffffffff,17,-2147483648,2147483647,19,7,6)
    for before in range(6):
        for after in range(6):
            for seed in range(4):add(100);state(129,before,seed);state(75,after,seed)
    deltas=[(0,0),(1,-1),(-1,1),(120,30),(-2000,1000),(2147483647,1),(-2147483648,-2147483648)]
    for sid in (100,101,102,103,200,201,202,203,204,300,999):
        for dx,dy in deltas:
            add(100);add(18);add(121,1,0,0);add(52,sid,10,60,300,200)
            add(76,sid,dx,dy);add(78,sid,-1,2);add(120,1,sid)
            add(103,999,0,0,0,0,1280,720);add(80,dx,dy);add(79,sid)
    for edge in range(8):
        for dx,dy in deltas:
            for flags in range(4):
                add(100);add(18);add(10,100,2);add(101,0,19,0,1,0,flags,100)
                add(129,4,100,0,0,0,edge,20,60,300,200,2147483647,-2147483648)
                add(81,dx,dy);add(84,dx,dy);add(104,100,0);add(82)
    for top in (0,4):
        add(100);add(18);add(10,100,2);add(101,0,19,0,2,0,top,100,101)
        add(124,1,100,0,0);add(130,100,1,10,60,400,250)
        for x in range(5,420,7):
            for y in (55,60,61,63,64,65,80,87,88,100,300,305,309,310):
                add(85,x,y,100);add(86,x,y)
        for x,y in ((11,61),(50,75),(150,75),(11,150),(200,305),(220,180),(0,0)):
            add(100);add(18);add(10,100,2);add(10,101,2)
            add(101,0,19,0,2,0,top,100,101);add(124,1,100,0,0);add(130,100,1,10,60,400,250)
            add(88,x,y,0);add(88,x,y,1)
    for width in (640,998,999,1024,1280,2147483647):
        add(100);add(18);add(103,0,0,0,0,0,width,720)
        for x in range(75,1205,11):
            for y in (9,10,18,25,39,47,48):add(91,x,y)
    for active in (0,1):
        for panel in range(5):
            for x,y,w,h in ((80,55,240,360),(0xffffffff,0xffffffff,0,0),(1000,55,240,300)):
                add(100);add(18);add(89,active,panel,144,x,y,w,h)
    add(100);add(18);add(90);add(90)
    for pick in range(6):
        add(100);add(18);add(131,1,int(pick<5),pick%5);add(88,100,100,1)
    for tag in range(3):
        for bar in (0,1):add(87,tag,201,3,2,bar)
    for x,y in ((0,50),(1279,50),(400,51),(640,500)):
        add(100);add(18);add(10,100,2);add(101,0,19,0,1,0,0,100)
        add(83,100,x,y);add(77)
    for sequence,pending in ((0,0),(5,1),(MASK,0),(MASK,1)):
        for x in (30,80,130,180,230):
            add(100);add(18);add(101,0,3,0,1,0,0,201);add(124,1,201,0,0)
            add(130,201,1,10,60,400,250);add(132,sequence,pending,77,16,4,5)
            add(88,x,130,1)
    return ('\n'.join(rows)+'\n').encode(),len(rows)


def results(binary,commands):
    out=run([binary], ('\n'.join(commands)+'\n').encode()).decode()
    return [line for line in out.splitlines()if line.startswith('R ')],out


def independent(binary,rust=False):
    cases=[
        ('re_registration',['100','8 100 2','13 100','10 100 7','14','8 100 2','14','11 100'],
         ['R 1','R 1 1','R 1','R 0','R 1','R 0','R 1 3']),
        ('exhaustion',['100',f'106 {MASK} 100 2 {MASK}','13 100','10 100 5','9 100','14','12 100','8 100 2','11 100'],
         ['R 1 '+str(MASK),'R 0','R 1 2','R 1','R 0','R 0','R 1 '+str(MASK)]),
        ('same_state_at_exhaustion',['100',f'106 {MASK} 100 2 {MASK}','10 100 2','9 100'],['R 1','R 1 2']),
        ('malformed_active_tab',['100','101 0 9 255 255 0 0 100','22 9','33 9','39 9 9 10'],['R 0 0','R 0','R 0']),
    ]
    if rust:
        first,_=results(binary,cases[0][1]);assert first[-2:]==['R 1','R 1 1'],first
        second,_=results(binary,cases[1][1]);assert second[1:4]==['R 1','R 1 5','R 1'],second
        invalid=subprocess.run([binary],input=('\n'.join(cases[3][1])+'\n').encode(),capture_output=True)
        assert invalid.returncode!=0 and b'index out of bounds' in invalid.stderr,invalid.stderr
        for before in (0,1,6,7):
            observed,_=results(binary,['100',f'8 100 {before}','13 100','10 100 5','11 100','14'])
            assert observed==['R 1','R 1 1','R 1','R 1 1','R 1'],observed
        return 4
    for name,commands,expected in cases:
        observed,out=results(binary,commands)
        assert observed==expected,(name,observed,expected)
        if name=='exhaustion':
            states=[line for line in out.splitlines()if line.startswith('S ')]
            # Rejected transition, bump and re-registration preserve the complete state.
            assert states[2]==states[3]==states[4]==states[5]==states[6]==states[7]==states[8],name
    # Every state pair is checked separately, including at stamp exhaustion.
    # This specification uses transition destinations, not the implementation's
    # predecessor whitelist. In particular Mapped -> Closing invalidates focus.
    for before in range(8):
        for after in range(8):
            fresh=before!=after and (after in (5,7) or (before,after)==(5,6))
            for full in (False,True):
                stamp=MASK if full else 1
                setup=f'106 {stamp} 100 {before} {stamp}' if full else f'8 100 {before}'
                commands=['100',setup,'13 100',f'10 100 {after}','11 100','14','9 100']
                accepted=not(full and fresh)
                next_stamp=stamp+1 if fresh and accepted else stamp
                state=after if accepted else before
                expected=['R 1 '+str(stamp),'R '+str(int(accepted)),'R 1 '+str(next_stamp),
                          'R '+str(int(next_stamp==stamp)),'R 1 '+str(state)]
                observed,_=results(binary,commands)
                if not full:expected.insert(0,'R 1')
                assert observed==expected,('state_pair',before,after,full,observed,expected)
    return len(cases)+128


def action_controls(binary,work,rust=False):
    def output(commands):
        return run([binary], ('\n'.join(commands)+'\n').encode()).decode()
    setup=['100','18','101 0 19 0 1 0 0 100']
    missing=['100','18','10 201 4','101 0 19 0 1 0 1 201','123 0 0 300 200 7','104 201 5','49 19']
    if rust:
        observed=output([*setup,f'106 {MASK} 100 2 {MASK}','47 100'])
        assert [x for x in observed.splitlines()if x.startswith('R ')][-1]=='R 1'
        assert 'peer request-close 100' in observed and 'call 5 238 100 0 0' in observed
        (work/'source-close-exhausted.out').write_text(observed)
        observed=output(missing);states=[x for x in observed.splitlines()if x.startswith('S ')]
        assert states[-1]!=states[-2] and 'call 'not in observed
        (work/'source-restore-partial.out').write_text(observed)
        return 2
    controls=[]
    for counter in (MASK,MASK-1,MASK-2):
        controls.append(('close_exhausted_'+str(counter),[*setup,f'106 {counter} 100 2 {counter}','120 1 100','47 100']))
    controls.extend([
        ('close_unknown',['100','101 0 19 0 1 0 0 999','47 999']),
        ('close_bad_frame',['100','18','10 100 2','101 0 19 255 255 0 0 100','47 100']),
        ('restore_missing_geometry',missing),
        ('minimize_unknown',['100','101 0 19 0 1 0 0 999','48 19']),
    ])
    for name,commands in controls:
        observed=output(commands);states=[x for x in observed.splitlines()if x.startswith('S ')]
        assert states[-1]==states[-2],(name,'failed action changed owned state')
        assert [x for x in observed.splitlines()if x.startswith('R ')][-1]=='R 0',name
        assert not any(x.startswith(('call ','peer request-close','peer transition','peer tomb-event'))for x in observed.splitlines()),name
        (work/(binary.name+'-'+name+'.out')).write_text(observed)
    observed=output([*setup,f'106 {MASK-3} 100 2 {MASK-3}','47 100'])
    assert [x for x in observed.splitlines()if x.startswith('R ')][-1]=='R 1'
    assert observed.count('call 5 238 100 0 0')==1
    assert '|L 100 7 '+str(MASK) in observed
    return len(controls)+1


def focus_controls(binary,work,rust=False):
    def observe(commands):
        out=run([binary], ('\n'.join(commands)+'\n').encode()).decode()
        return [x for x in out.splitlines()if x.startswith('R ')],out
    base=['100','18','10 100 2','10 101 2','101 0 19 0 2 0 4 100 101']
    if rust:
        values,out=observe([*base,'63 19 256']);assert values[-1]=='R 1',values
        (work/'source-tab-index-truncation.out').write_text(out)
        invalid=subprocess.run([binary],input=('\n'.join([*base,'63 19 257'])+'\n').encode(),capture_output=True)
        assert invalid.returncode!=0 and b'index out of bounds'in invalid.stderr,invalid.stderr
        values,out=observe([*base,'64 100','64 0','125']);assert values[-1]=='R 1 100 1',values
        (work/'source-focus-shadow.out').write_text(out)
        values,out=observe([*base,'106 10 100 2 0','64 100']);assert values[-1]=='R 1',values
        (work/'source-focus-zero-generation.out').write_text(out)
        commands=[*base,'101 0 19 0 1 0 4 100','101 1 20 0 1 0 4 101','103 100 0 19 1 1 1280 720','124 1 101 0 0','66 5 100']
        _,out=observe(commands);calls=[x for x in out.splitlines()if x.startswith('call 5 253 100 ')]
        assert not calls,out
        (work/'source-hover-stale-frame.out').write_text(out)
        return 5
    count=0
    for index in (8,255,256,257,511,512,0xffffffff):
        values,out=observe([*base,'63 19 '+str(index)])
        assert values[-1]=='R 0' and not any(x.startswith('call ')for x in out.splitlines()),index
        count+=1
    values,out=observe([*base,'64 100','64 0','125']);assert values[-1]=='R 0',values;count+=1
    values,out=observe([*base,'106 10 100 2 0','64 100']);assert values[-1]=='R 0'and'call 'not in out,values;count+=1
    for terminal,closing in ((1,0),(0,1)):
        values,out=observe(['100','18','105 0 71 '+str(terminal)+' '+str(closing)+' 1','64 201'])
        assert values[-1]=='R 0'and'call 'not in out,values;count+=1
    commands=[*base,'101 0 19 0 1 0 4 100','101 1 20 0 1 0 4 101','103 100 0 19 1 1 1280 720','124 1 101 0 0','66 5 100']
    values,out=observe(commands);calls=[x for x in out.splitlines()if x.startswith('call 5 253 100 ')]
    assert len(calls)==1 and (int(calls[0].split()[-1])>>8)&2==0,calls
    assert values[-1]=='R 1';count+=1
    values,out=observe(['100','18','101 0 19 255 255 0 0 100','59'])
    assert 'reason=invalid_tab'in out and not any(x.startswith('call ')for x in out.splitlines());count+=1
    return count


def access_controls(binary,work):
    def observe(commands):
        out=run([binary], ('\n'.join(commands)+'\n').encode()).decode()
        return [x for x in out.splitlines()if x.startswith('R ')],out
    count=0
    for length in (0,1,30,31,32,64,MASK):
        src=bytes(range(1,33));copied=min(length,31)
        want=bytearray([0xA5]*32);want[:copied]=src[:copied];want[copied]=0
        token=5381
        for b in want[:copied]:token=(token*33+b)&0xffffffff
        _,out=observe(['100','67 '+str(length)+' 165 '+' '.join(map(str,src))])
        assert next(x for x in out.splitlines()if x.startswith('B '))=='B '+want.hex()+' '+str(token),out
        count+=1
    for label in (b'\xc0\xaf',b'\xed\xa0\x80',b'\xf4\x90\x80\x80',b'\xe2\x82',b'\x80'):
        _,out=observe(['100','126 0 2 '+' '.join(map(str,label)), '69 0 0'])
        node=next(x.split()for x in out.splitlines()if x.startswith('N '))
        assert node[2:9]==['4096','2','15','0','0','0','0'] and node[9]==b'scene'.ljust(32,b'\0').hex(),node
        count+=1
    for label in (b'A\0B', '海'.encode(),b'\xf4\x8f\xbf\xbf',b'0123456789ABCDEF'):
        _,out=observe(['100','126 3 0 '+' '.join(map(str,label)), '69 0 3'])
        node=next(x.split()for x in out.splitlines()if x.startswith('N '))
        assert node[2:9]==['4099','2','0','128','0','0','3'] and node[9]==label.ljust(32,b'\0').hex(),node
        count+=1
    for index,scene in ((64,0),(65,0),(MASK,0),(0,5),(0,255)):
        values,out=observe(['100',f'69 {index} {scene}'])
        assert values==['R '+str(index)]and not any(x.startswith('N ')for x in out.splitlines()),out
        count+=1
    for sid,fid,scene,alive,want in ((0,0,255,1,0),(0,0,0,1,1),(0,19,255,1,1),
                                   (100,19,0,1,1),(100,19,0,0,0),(100,19,0,3,0),
                                   (100,20,0,1,0)):
        values,_=observe(['100','101 0 19 0 1 0 0 100',f'104 100 {alive}',f'68 {sid} {fid} {scene}'])
        assert values==['R '+str(want)],(sid,fid,scene,alive,values)
        count+=1
    base=['100','18','10 100 2','10 101 2','101 0 19 0 2 0 0 100 101']
    for forward,want in ((1,101),(0,101)):
        commands=[*base,'103 100 0 0 0 0 1280 720',
                  '127 0 1 4 1 100 19 0','127 1 1 4 1 100 19 0',
                  '127 2 1 4 1 101 19 0','74 64 '+str(forward),'125']
        values,out=observe(commands)
        assert values[-2:]==['R 1','R 1 '+str(want)+' 1'],values
        assert out.count('call 5 237 '+str(want)+' ')==1,out
        count+=1
    for state,action,alive in ((0,1,1),(8,1,1),(4,0,1),(4,1,0),(4,1,3)):
        values,out=observe([*base,f'104 100 {alive}',f'127 0 1 {state} {action} 100 19 0','74 1 1'])
        assert values[-1]=='R 0'and not any(x.startswith('call ')for x in out.splitlines()),out
        count+=1
    for action in (42,43,44):
        for alive in (0,3):
            values,out=observe([*base,'103 100 0 0 0 0 1280 720',f'104 100 {alive}',f'72 {action}'])
            assert values[-1]=='R 0'and not any(x.startswith('call ')for x in out.splitlines()),out
            count+=1
    (work/(binary.name+'-access-controls.json')).write_text(json.dumps(dict(controls=count))+'\n')
    return count


def interaction_controls(binary,work):
    count=0
    def observe(commands):
        out=run([binary], ('\n'.join(commands)+'\n').encode()).decode()
        return [x for x in out.splitlines()if x.startswith('R ')],out
    allowed={0:set(range(6)),1:{0,2,3,4,5},2:{0},3:{0,1},4:{0},5:{0}}
    for before in range(6):
        for after in range(6):
            _,out=observe(['100',f'129 {before} 100 0 0 0 0 0 0 200 100',f'75 {after} 101 0 0 0 0 0 0 300 200'])
            state=next(x for x in reversed(out.splitlines())if x.startswith('S '))
            want=after if after in allowed[before]else before
            assert state.split('|E ')[1].split()[0]==str(want),(before,after,state[-200:])
            assert ('[shell.interaction.forbidden]'in out)==(after not in allowed[before]),(before,after)
            count+=1
    base=['100','18','121 1 0 0','52 100 10 60 300 200','52 101 20 70 300 200',
          '120 1 100','103 101 0 0 0 0 1280 720']
    values,out=observe([*base,'80 7 0'])
    state=next(x for x in reversed(out.splitlines())if x.startswith('S '))
    assert values[-1]=='R 1'and'|W 17 60 300 200'in state and'|G 20 70 300 200'in state,state[-700:]
    assert '[shell.integrated.drag_target] id=100 focus=101'in out;count+=1
    for tag in (0,1,3,4,5):
        values,out=observe(['100',f'129 {tag} 100','80 20 20'])
        assert values[-1]=='R 0'and'[silk.drag.move]'not in out;count+=1
    resize=['100','18','101 0 19 0 1 0 2 100','129 4 100 0 0 0 1']
    values,out=observe([*resize,'84 50 50'])
    assert values[-1]=='R 0'and not any(x.startswith('call ')for x in out.splitlines());count+=1
    _,out=observe([*resize,'104 100 0','82'])
    assert '|E 0 0 'in out.splitlines()[-1];count+=1
    values,out=observe(['100','18','101 0 3 0 1 0 1 201','101 1 2 0 1 0 0 200','77'])
    assert values[-1].startswith('R 1 2 200 '),values;count+=1
    hit=['100','18','10 100 2','101 0 19 0 2 0 4 100 101','124 1 100 0 0','130 100 1 10 60 400 250']
    for x,y,want in ((150,75,'R 2 0 19 2'),(11,61,'R 2 0 19 3'),(200,180,'R 1 100 0 0')):
        values,_=observe([*hit,f'86 {x} {y}']);assert values[-1]==want,(x,y,values);count+=1
    toolbar=['100','18','101 0 3 0 1 0 0 201','124 1 201 0 0','130 201 1 10 60 400 250']
    for sequence,pending,expected in ((0,0,'|DP 1 1 11 9 1 1'),(5,1,'|DP 6 1 77 16 4 5'),
                                      (MASK,0,'|DP '+str(MASK)+' 0'),(MASK,1,'|DP '+str(MASK)+' 1 77 16 4 5')):
        _,out=observe([*toolbar,f'132 {sequence} {pending} 77 16 4 5','88 30 130 1'])
        assert out.splitlines()[-1].endswith(expected),out[-400:];count+=1
    for width,x,want in ((999,910,'target=clock'),(998,1100,'target=clock'),(1024,935,'target=clock'),(1280,1100,'target=clock')):
        values,out=observe(['100','18',f'103 0 0 0 0 0 {width} 720',f'91 {x} 25'])
        assert values[-1]=='R 1'and want in out,(width,x,out[-500:]);count+=1
    (work/(binary.name+'-interaction-controls.json')).write_text(json.dumps(dict(controls=count))+'\n')
    return count


def capacity_controls(binary,original=False):
    commands=['100','97']+[f'8 {sid} 2'for sid in range(1000,1060)]
    out=run([binary],('\n'.join(commands)+'\n').encode()).decode()
    results=[line for line in out.splitlines()if line.startswith('R ')]
    capacity=43 if original else 44
    assert results==[f'R {capacity}']+['R 1']*capacity+['R 0']*(60-capacity),(capacity,results)
    return 1


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-chrome-tests')
    parser.add_argument('--mutations',action='store_true');parser.add_argument('--proof-profile',action='store_true');args=parser.parse_args()
    if args.proof_profile:
        for key in ('SEXOS_VISIBLE_FOCUS_TOPBAR_PROOF','SEXOS_SHELL_INTERACTION_CONTRACT_PROOF','SEXOS_CLICK_DRAG_PROOF'):
            os.environ[key]='1'
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True);inventory=json.loads(INVENTORY.read_text())
    prepare(work,inventory);data,count=corpus();(work/'commands.txt').write_bytes(data)
    expected=run([work/'rust'],data);(work/'rust.out').write_bytes(expected)
    capacity_controls(work/'rust',True)
    original_counterexamples=independent(work/'rust',True)+action_controls(work/'rust',work,True)+focus_controls(work/'rust',work,True)
    include='-I'+str(HERE/'generated/include');exports={}
    defines=['-D'+key+'=1'for key in ('SEXOS_VISIBLE_FOCUS_TOPBAR_PROOF','SEXOS_SHELL_INTERACTION_CONTRACT_PROOF','SEXOS_CLICK_DRAG_PROOF')]if args.proof_profile else []
    for unit in ('silk_shell_chrome','silk_shell_chrome_atomic','silk_shell_custody','silk_shell_viewport','silkbar_geometry','silkbar_model'):
        target=work/(unit+'.c')
        plan=['--plan',work/'chrome.plan']if unit=='silk_shell_chrome'else []
        run([args.cesta,'export-c',HERE/'src'/(unit+'.cesta'),'--std=c11','-o',target,*plan,'--',include,*defines]);exports[unit]=target
    configurations=[]
    for compiler in ('gcc','clang'):
        for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=work/(compiler+'-'+mode)
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,*defines,include,'-I'+str(work),*exports.values(),FIX/'provider.c','-o',binary])
            observed=run([binary],data)
            if observed!=expected:
                (work/(compiler+'-'+mode+'.mismatch')).write_bytes(observed)
                left=expected.splitlines();right=observed.splitlines()
                first=next((i for i,(a,b)in enumerate(zip(left,right))if a!=b),min(len(left),len(right)))
                a=left[first]if first<len(left)else b'';b=right[first]if first<len(right)else b''
                column=next((i for i,(x,y)in enumerate(zip(a,b))if x!=y),min(len(a),len(b)))
                raise AssertionError((compiler,mode,'trace differs',first,column,
                                      a[max(column-40,0):column+160],b[max(column-40,0):column+160]))
            capacity_controls(binary)
            controls=independent(binary)
            actions=action_controls(binary,work)
            focus=focus_controls(binary,work)
            access=access_controls(binary,work)
            interaction=interaction_controls(binary,work)
            configurations.append(dict(compiler=compiler,mode=mode,sha256=hashlib.sha256(observed).hexdigest()))
    mutants=[]
    if args.mutations:
        body=exports['silk_shell_chrome'].read_text()
        changes=[
            ('resurrect_stamp','(SurfaceLifecycle){initial_state,LIFECYCLE_GENERATION}','(SurfaceLifecycle){initial_state,1}'),
            ('same_generation','++LIFECYCLE_GENERATION;','LIFECYCLE_GENERATION+=0;'),
            ('mapped_close_keeps_stamp','bool bump=next==LIFECYCLE_CLOSING','bool bump=(prev!=LIFECYCLE_MAPPED && next==LIFECYCLE_CLOSING)'),
            ('ignore_custody_terminal','(entry.dead || entry.close_pending) && shell_custody_owns_surface','false && shell_custody_owns_surface'),
            ('ignore_app_geometry','return !entry.dead && entry.has_bounds;','return !entry.dead;'),
            ('ignore_scene','frame.scene_id!=ACTIVE_SCENE_IDX ||','false ||'),
            ('no_active_swap','if(FRAMES[i].active_tab==idx_a)','if(false)'),
            ('inclusive_rect','x<chrome_add(rect->x,(int32_t)rect->w)','x<=chrome_add(rect->x,(int32_t)rect->w)'),
            ('close_without_preflight','if(!chrome_close_preflight(ctx,surface_id))return false;','if(false)return false;'),
            ('close_version_boundary','LIFECYCLE_GENERATION>UINT64_MAX-3','LIFECYCLE_GENERATION>UINT64_MAX-2'),
            ('restore_loses_frame','FRAMES[saved->frame_slot]=saved->prior_frame;','(void)saved->prior_frame;'),
            ('restore_loses_lifecycle','LIFECYCLE_TABLE[saved->life_slot].lc=saved->prior_life;','(void)saved->prior_life;'),
            ('restore_loses_view','ctx->apps->document[saved->document]=saved->prior_document;','(void)saved->prior_document;'),
            ('close_active_tab_shift','else if(FRAMES[i].active_tab>found)--FRAMES[i].active_tab;','else if(false)--FRAMES[i].active_tab;'),
            ('tab_narrow_before_validation','tab_index>=frame.tab_count || tab_index>=MAX_TABS_PER_FRAME','(uint8_t)tab_index>=frame.tab_count'),
            ('focus_shadow_stale','FOCUSED_SURFACE_ID=0;\n    sync_focus_ref();','FOCUSED_SURFACE_ID=0;'),
            ('focus_without_generation','if(!make_focus_ref(sid,&ref))','if(false)'),
            ('focus_wrong_scene','scene!=ACTIVE_SCENE_IDX','scene==ACTIVE_SCENE_IDX'),
            ('hover_previous_stale','if(previous && previous!=frame)','if(false)'),
            ('tile_inactive_scene','frame.scene_id!=ACTIVE_SCENE_IDX ||','false ||'),
            ('label_skips_terminator','dst->bytes[len]=0;','dst->bytes[len]=1;'),
            ('label_hash_ignores_nul','if(!b)break;','if(false)break;'),
            ('scene_invalid_utf8_accepted','if(chrome_access_utf8(text))','if(true)'),
            ('scene_active_not_visible','ACCESS_FOCUSED|ACCESS_SELECTED|ACCESS_VISIBLE','ACCESS_FOCUSED|ACCESS_SELECTED'),
            ('frame_dead_node_published','if(sid && (!surface_is_alive(sid) || is_tombstoned(sid)))','if(false)'),
            ('target_missing_frame_accepted','target->frame_id && chrome_frame_slot(ctx,target->frame_id)==MAX_FRAMES','false'),
            ('focus_duplicate_annotations','if(duplicate)continue;','if(false)continue;'),
            ('focus_ignores_action_right','!(node.actions&ACT_FOCUS)','false'),
            ('transition_allows_everything','if(allowed){','if(true){'),
            ('drag_uses_new_focus','uint64_t sid=INTERACTION.surface_id;ShellChromeMove result={0};','uint64_t sid=FOCUSED_SURFACE_ID;ShellChromeMove result={0};'),
            ('resize_ignores_zoom','frame_for_surface(sid,&frame) && frame_is_zoomed(frame)','false'),
            ('resize_delta_lost','state.pending_dx=chrome_add(state.pending_dx,dx)','state.pending_dx=dx'),
            ('hit_skips_focused_priority','if(point_in_surface(x,y,focus))','if(false)'),
            ('toolbar_overwrites_pending','if(!ctx->apps->document_pointer.pending)','if(true)'),
            ('toolbar_reuses_sequence','uint64_t sequence=ctx->apps->document_pointer_sequence+1','uint64_t sequence=ctx->apps->document_pointer_sequence'),
            ('panel_forgets_open','*active=true;','*active=false;'),
            ('debug_unsigned_negative','if(value<0)chrome_debug_byte(text,\'-\');','if(false)chrome_debug_byte(text,\'-\');'),
        ]
        for name,old,new in changes:
            pattern=r'\s+'.join(map(re.escape,re.split(r'\s+',old)))
            assert re.search(pattern,body),(name,old)
            changed=re.sub(pattern,lambda _:new,body)
            path=work/(name+'.c');path.write_text(changed);binary=work/(name+'.bin')
            run(['gcc','-std=c11','-O2',*defines,include,'-I'+str(work),path,exports['silk_shell_chrome_atomic'],exports['silk_shell_custody'],exports['silk_shell_viewport'],exports['silkbar_geometry'],exports['silkbar_model'],FIX/'provider.c','-o',binary])
            rejected=run([binary],data)!=expected
            if not rejected:
                try:independent(binary);action_controls(binary,work);focus_controls(binary,work);access_controls(binary,work);interaction_controls(binary,work)
                except AssertionError:rejected=True
            assert rejected,(name,'mutant survived')
            mutants.append(name)
        geometry=exports['silkbar_geometry'].read_text()
        for name,old,new in [
            ('geometry_default_only','(void)silkbar_geometry_fit(width,&bar);','(void)width;'),
            ('geometry_overlap','if(CHIP_X0<shift || left_end>SIZE_MAX-16 || CHIP_X0-shift<left_end+16)return false;','if(false)return false;'),
        ]:
            assert old in geometry,(name,old)
            path=work/(name+'.c');path.write_text(geometry.replace(old,new));binary=work/(name+'.bin')
            run(['gcc','-std=c11','-O2',*defines,include,'-I'+str(work),
                 exports['silk_shell_chrome'],exports['silk_shell_chrome_atomic'],exports['silk_shell_custody'],
                 exports['silk_shell_viewport'],path,exports['silkbar_model'],FIX/'provider.c','-o',binary])
            assert run([binary],data)!=expected,(name,'mutant survived')
            mutants.append(name)
    summary=dict(pin=inventory['pin'],functions=len(inventory['functions']),remaining=len(inventory['remaining']),commands=count,
                 ownership_plan_sha256=hashlib.sha256((work/'chrome.plan').read_bytes()).hexdigest(),
                 configurations=configurations,original_counterexamples=original_counterexamples,lifecycle_capacity_controls=1,independent_controls=controls,action_controls=actions,focus_controls=focus,access_controls=access,interaction_controls=interaction,mutants=mutants,
                 scope='Complete original Chrome partition plus current helpers, real viewport queues and SilkBar width dispatch; remaining subsystem peers are scripted. Not a joined shell boot.')
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('Chrome slice:',len(inventory['functions']),'current functions;',count,'commands per configuration;',len(mutants),'mutants caught')


if __name__=='__main__':main()
