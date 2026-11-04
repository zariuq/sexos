"""Deterministic Spindle state construction and observations for both adapters."""
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent


def prepare(rust,c):
    scalar=('cmd_len','output_count','history_count','history_pos','sb_write','sb_total')
    arrays=('cmd_buf','output_lines','history','sb_ring')
    rs='''fn spindle_hash(bytes:impl Iterator<Item=u8>)->u64{bytes.fold(14695981039346656037,|h,b|(h^b as u64).wrapping_mul(1099511628211))}
fn spindle_byte(i:usize,seed:u64,kind:u64)->u8{if kind==1{if i%7==3{32}else{b'a'+((i as u64+seed)%26)as u8}}else if kind==2{(i as u64+seed)as u8}else{b'a'+((i as u64+seed)%26)as u8}}
unsafe fn spindle_fixture_reset(){YARN=core::mem::zeroed();YARN.history_pos=-1;SPINDLE_VI_NORMAL=false;SPINDLE_VI_CUR=0;SPINDLE_VI_PREV_BUF=[0;YARN_CMD_BUF_CAP];SPINDLE_VI_PREV_LEN=0;SPINDLE_VI_PENDING_D=false;SPINDLE_LAST_CMD_OK=true;SPINDLE_HIST_DRAFT_BUF=[0;YARN_CMD_BUF_CAP];SPINDLE_HIST_DRAFT_LEN=0;SPINDLE_SESSIONS=core::mem::zeroed();for i in 0..SPINDLE_SESSION_COUNT{SPINDLE_SESSIONS[i].history_pos=-1;SPINDLE_SESSIONS[i].label=*b"session-0       ";SPINDLE_SESSIONS[i].label[8]=b'0'+i as u8;}SPIDERWEB_OPEN=false;SPIDERWEB_MODE=SpiderwebMode::History;SPIDERWEB_QUERY=[0;SPIDERWEB_QUERY_CAP];SPIDERWEB_QUERY_LEN=0;SPIDERWEB_RESULTS=[[0;YARN_CMD_BUF_CAP];SPIDERWEB_RESULT_CAP];SPIDERWEB_RESULT_LENS=[0;SPIDERWEB_RESULT_CAP];SPIDERWEB_RESULT_COUNT=0;SPIDERWEB_SELECTED=0;}
unsafe fn spindle_fixture_state(){
print!("Y {} {} {} {} {} {}",SPINDLE_ACTIVE_SESSION,SPINDLE_VI_NORMAL as u8,SPINDLE_VI_CUR,SPINDLE_VI_PENDING_D as u8,SPINDLE_VI_PREV_LEN,SPINDLE_LAST_CMD_OK as u8);
'''
    cs='''void shell_surfaces_debug_record(const ShellDebugRecord *record){if(record->len>sizeof(record->bytes))abort();fwrite(record->bytes,1,record->len,stdout);}
static uint64_t spindle_hash(const void *data,size_t len){const uint8_t *p=data;uint64_t h=UINT64_C(14695981039346656037);for(size_t i=0;i<len;++i)h=(h^p[i])*UINT64_C(1099511628211);return h;}
static uint8_t spindle_byte(size_t i,uint64_t seed,uint64_t kind){return kind==1?(i%7==3?32:(uint8_t)('a'+(i+seed)%26)):kind==2?(uint8_t)(i+seed):(uint8_t)('a'+(i+seed)%26);}
static void spindle_fixture_reset(void){surfaces.YARN.history_pos=-1;surfaces.SPINDLE_LAST_CMD_OK=true;for(size_t i=0;i<SPINDLE_SESSION_COUNT;++i){surfaces.SPINDLE_SESSIONS[i].history_pos=-1;memcpy(surfaces.SPINDLE_SESSIONS[i].label,"session-0       ",16);surfaces.SPINDLE_SESSIONS[i].label[8]=(uint8_t)('0'+i);}}
static void spindle_fixture_state(void){
printf("Y %zu %u %zu %u %zu %u",surfaces.SPINDLE_ACTIVE_SESSION,surfaces.SPINDLE_VI_NORMAL,surfaces.SPINDLE_VI_CUR,surfaces.SPINDLE_VI_PENDING_D,surfaces.SPINDLE_VI_PREV_LEN,surfaces.SPINDLE_LAST_CMD_OK);
'''
    for rbase,cbase in [('YARN','surfaces.YARN'),('SPINDLE_SESSIONS[0]','surfaces.SPINDLE_SESSIONS[0]'),('SPINDLE_SESSIONS[1]','surfaces.SPINDLE_SESSIONS[1]')]:
        rs+='print!("| '+(' '.join('{}'for _ in scalar))+'",'+','.join(rbase+'.'+f+' as i128'for f in scalar)+');\n'
        cs+='printf("| '+(' '.join('%lld'for _ in scalar))+'",'+','.join('(long long)'+cbase+'.'+f for f in scalar)+');\n'
        for field in arrays:
            iterator=rbase+'.'+field+'.iter()'+('.flatten()'if field!='cmd_buf'else '')+'.copied()'
            rs+='print!(" {}",spindle_hash('+iterator+'));\n'
            cs+='printf(" %" PRIu64,spindle_hash('+cbase+'.'+field+',sizeof('+cbase+'.'+field+')));\n'
    rs+='print!("| {} {} {} {} {}",YARN.sb_offset,SPIDERWEB_OPEN as u8,SPIDERWEB_MODE as u8,SPIDERWEB_QUERY_LEN,SPIDERWEB_SELECTED);\n'
    cs+='printf("| %u %u %u %zu %zu",surfaces.YARN.sb_offset,surfaces.SPIDERWEB_OPEN,surfaces.SPIDERWEB_MODE,surfaces.SPIDERWEB_QUERY_LEN,surfaces.SPIDERWEB_SELECTED);\n'
    for field,flatten in [('SPINDLE_VI_PREV_BUF',False),('SPINDLE_HIST_DRAFT_BUF',False),('SPIDERWEB_QUERY',False),('SPIDERWEB_RESULTS',True)]:
        rs+='print!(" {}",spindle_hash('+field+'.iter()'+('.flatten()'if flatten else '')+'.copied()));\n'
        cs+='printf(" %" PRIu64,spindle_hash(surfaces.'+field+',sizeof(surfaces.'+field+')));\n'
    rs+='print!(" {} {}",SPINDLE_HIST_DRAFT_LEN,SPIDERWEB_RESULT_COUNT);for len in SPIDERWEB_RESULT_LENS{print!(" {}",len)}println!();}\n'
    cs+='printf(" %zu %zu",surfaces.SPINDLE_HIST_DRAFT_LEN,surfaces.SPIDERWEB_RESULT_COUNT);for(size_t i=0;i<SPIDERWEB_RESULT_CAP;++i)printf(" %zu",surfaces.SPIDERWEB_RESULT_LENS[i]);putchar(10);}\n'
    cases_r='''
        450=>spindle_fixture_state(),
        451=>{YARN.cmd_buf=[0;YARN_CMD_BUF_CAP];YARN.cmd_len=n(1)as usize;SPINDLE_VI_CUR=n(2)as usize;for i in 0..YARN.cmd_len{YARN.cmd_buf[i]=spindle_byte(i,n(3),n(4));}},
        452=>YARN.cmd_buf[n(1)as usize]=n(2)as u8,
        453=>{YARN.cmd_len=n(1)as usize;SPINDLE_VI_CUR=n(2)as usize;},
        454=>{let at=n(1)as usize;YARN.history[at]=[0;YARN_CMD_BUF_CAP];for i in 0..n(2)as usize{YARN.history[at][i]=spindle_byte(i,n(3),n(4));}YARN.history_count=YARN.history_count.max(at+1);},
        455=>spindle_dispatch(),
        457=>{SURFACE_0x99_W=n(1)as u32;SURFACE_0x99_H=n(2)as u32;SPINDLE_VI_NORMAL=n(3)!=0;SPINDLE_LAST_CMD_OK=n(4)!=0;},
        458=>{let total=n(1)as usize;YARN.sb_ring=[[0;SPINDLE_SB_LINE_CAP];SPINDLE_SB_LINES];for i in total.saturating_sub(SPINDLE_SB_LINES)..total{for j in 0..SPINDLE_SB_LINE_CAP{YARN.sb_ring[i%SPINDLE_SB_LINES][j]=((i+1)>>(8*(j%4)))as u8;}}YARN.sb_write=total%SPINDLE_SB_LINES;YARN.sb_total=total as u32;},
        459=>{print!("B");for i in 0..YARN.sb_total.min(64)as usize{let at=(YARN.sb_write+SPINDLE_SB_LINES-YARN.sb_total.min(SPINDLE_SB_LINES as u32)as usize+i)%SPINDLE_SB_LINES;let b=YARN.sb_ring[at];print!(" {}",u32::from_le_bytes([b[0],b[1],b[2],b[3]]));}println!();},
        461=>{YARN.output_count=n(1)as usize;for i in 0..YARN_OUTPUT_LINES{for j in 0..YARN_OUTPUT_LINE_CAP{YARN.output_lines[i][j]=if j==0&&i%2==0{b'>'}else{spindle_byte(j,i as u64,0)};}}},
        463=>{let mut bytes=[0;10];let text=fmt_u32(n(1)as u32,&mut bytes);print!("D {} ",text.len());for b in text{print!("{:02x}",b)}println!();},
        464=>yarn_append_output(b"x"),
'''
    cases_c='''
        case 450:spindle_fixture_state();break;
        case 451:memset(surfaces.YARN.cmd_buf,0,sizeof(surfaces.YARN.cmd_buf));surfaces.YARN.cmd_len=n[1];surfaces.SPINDLE_VI_CUR=n[2];for(size_t i=0;i<n[1];++i)surfaces.YARN.cmd_buf[i]=spindle_byte(i,n[3],n[4]);break;
        case 452:surfaces.YARN.cmd_buf[n[1]]=n[2];break;
        case 453:surfaces.YARN.cmd_len=n[1];surfaces.SPINDLE_VI_CUR=n[2];break;
        case 454:{size_t at=n[1];memset(surfaces.YARN.history[at],0,YARN_CMD_BUF_CAP);for(size_t i=0;i<n[2];++i)surfaces.YARN.history[at][i]=spindle_byte(i,n[3],n[4]);if(surfaces.YARN.history_count<at+1)surfaces.YARN.history_count=at+1;break;}
        case 455:shell_surfaces_spindle_dispatch(&surface_ctx);break;
        case 457:input.SURFACE_0x99_W=n[1];input.SURFACE_0x99_H=n[2];surfaces.SPINDLE_VI_NORMAL=n[3];surfaces.SPINDLE_LAST_CMD_OK=n[4];break;
        case 458:{size_t total=n[1];memset(surfaces.YARN.sb_ring,0,sizeof(surfaces.YARN.sb_ring));for(size_t i=total>SPINDLE_SB_LINES?total-SPINDLE_SB_LINES:0;i<total;++i)for(size_t j=0;j<SPINDLE_SB_LINE_CAP;++j)surfaces.YARN.sb_ring[i%SPINDLE_SB_LINES][j]=(uint8_t)((i+1)>>(8*(j%4)));surfaces.YARN.sb_write=total%SPINDLE_SB_LINES;surfaces.YARN.sb_total=total;break;}
        case 459:{printf("B");size_t count=surfaces.YARN.sb_total;if(count>64)count=64;size_t retained=surfaces.YARN.sb_total;if(retained>SPINDLE_SB_LINES)retained=SPINDLE_SB_LINES;for(size_t i=0;i<count;++i){size_t at=(surfaces.YARN.sb_write+SPINDLE_SB_LINES-retained+i)%SPINDLE_SB_LINES;const uint8_t *b=surfaces.YARN.sb_ring[at];printf(" %u",(uint32_t)b[0]|((uint32_t)b[1]<<8)|((uint32_t)b[2]<<16)|((uint32_t)b[3]<<24));}puts("");break;}
        case 460:surfaces.YARN.cmd_len=n[1];surfaces.SPINDLE_VI_CUR=n[2];surfaces.YARN.history_count=n[3];surfaces.YARN.output_count=n[4];surfaces.YARN.sb_write=n[5];surfaces.SPIDERWEB_QUERY_LEN=n[6];surfaces.SPIDERWEB_RESULT_COUNT=n[7];break;
        case 461:surfaces.YARN.output_count=n[1];for(size_t i=0;i<YARN_OUTPUT_LINES;++i)for(size_t j=0;j<YARN_OUTPUT_LINE_CAP;++j)surfaces.YARN.output_lines[i][j]=j==0&&i%2==0?'>':spindle_byte(j,i,0);break;
        case 463:{ShellDecimalBytes bytes={0};size_t count=shell_surfaces_fmt_u32(n[1],&bytes);printf("D %zu ",count);for(size_t i=0;i<count;++i)printf("%02x",bytes[i]);puts("");break;}
        case 464:{const ShellCommandBytes bytes="x";shell_surfaces_yarn_append_output(&surface_ctx,&bytes,((ShellByteRange){0,1}));break;}
'''
    operations=[('spindle_vi_save_undo',''),('spindle_vi_undo',''),('spindle_vi_word_fwd',''),('spindle_vi_word_back',''),('spindle_vi_word_end',''),('spindle_vi_insert_at','n(2)as usize,n(3)as u8'),('spindle_vi_delete_at','n(2)as usize'),('spindle_history_load','n(2)as usize'),('spindle_history_up',''),('spindle_history_down',''),('spindle_ghost_accept',''),('spindle_vi_normal_key','n(2)as u8'),('spindle_session_save','n(2)as usize'),('spindle_session_load','n(2)as usize'),('spindle_session_switch',''),('spindle_render',''),('spindle_render_cmdline',''),('yarn_cmd_clear',''),('spiderweb_open','if n(2)==0{SpiderwebMode::History}else{SpiderwebMode::Command}'),('spiderweb_handle_key','n(2)as u8'),('spiderweb_search',''),('spiderweb_accept',''),('spiderweb_cancel',''),('spiderweb_render','')]
    cases_r+='        456=>match n(1){\n';cases_c+='        case 456:switch(n[1]){\n'
    for i,(name,arguments)in enumerate(operations):
        expr=name+'('+arguments+')'
        cases_r+=f'            {i}=>{{let _={expr};}},\n'
        cargs=arguments.replace('n(2)as usize','n[2]').replace('n(3)as u8','n[3]').replace('n(2)as u8','n[2]').replace('if n(2)==0{SpiderwebMode::History}else{SpiderwebMode::Command}','n[2]')
        cases_c+=f'            case {i}:(void)shell_surfaces_{name}(&surface_ctx'+(','+cargs if cargs else '')+');break;\n'
    cases_r+='            _=>panic!("spindle operation")},\n';cases_c+='            default:abort();}break;\n'
    rust=rs+rust.replace('unsafe fn spindle_render(){println!("surface spindle-render");}\n','')
    rust=rust.replace('unsafe fn reset(){','unsafe fn reset(){spindle_fixture_reset();')
    rust=rust.replace('        100=>reset(),',cases_r+'        100=>reset(),')
    c=c.replace('void shell_surfaces_peer_spindle_render(ShellSurfacesContext *c){(void)c;puts("surface spindle-render");}\n','')
    c=c.replace('static void reset(void) {',cs+'\nstatic void reset(void) {')
    c=c.replace('    surface_ctx=(ShellSurfacesContext)', '    spindle_fixture_reset();\n    surface_ctx=(ShellSurfacesContext)')
    c=c.replace('        case 100:',cases_c+'        case 100:')
    return rust,c


def corpus():
    rows=[]
    def add(*values):rows.append(' '.join(str(n)for n in values))
    def command(text):
        raw=text.encode();add(451,0,0,0,0)
        for i,b in enumerate(raw):add(452,i,b)
        add(453,len(raw),len(raw));add(455);add(450)
    for length in (0,1,2,7,31,32,39,40,63,64,254,255,256):
        for cursor in sorted({0,length//2,length}):
            add(100);add(451,length,cursor,5,1)
            for op in (0,2,3,4):add(456,op);add(450)
            add(456,5,min(cursor,length),ord('Z'));add(450)
            add(456,6,min(cursor,length));add(450);add(456,1);add(450)
    for scan in range(256):
        add(100);add(451,32,16,2,1);add(456,11,scan);add(450)
        add(456,11,scan);add(450)
    for count in (0,1,2,7,16):
        add(100)
        for at in range(count):add(454,at,at+8,at,1)
        add(451,12,4,9,1)
        for _ in range(count+2):add(456,8);add(450)
        for _ in range(count+2):add(456,9);add(450)
        add(456,18,0);add(450)
        for scan in (0x1E,0x12,0x0E,0x24,0x25,0x1C):add(456,19,scan);add(450)
    for length in (1,7,20,254,255):
        add(100);add(454,0,length+1,3,0);add(451,length,length,3,0)
        add(456,10);add(450)
    for width in (0,1,7,8,40,400,0xffffffff):
        for normal,ok in ((0,0),(0,1),(1,0),(1,1)):
            for count in (0,1,6,20):
                add(100);add(457,width,300,normal,ok);add(461,count);add(451,40,15,7,1);add(456,15);add(450)
    add(100)
    for text in ('help','clear','echo  one two  ','about','time','pd','scene','routes','faults','session','panes','history','status','not-a-command','   ','\thelp\n','help\t','echo\targ'):
        command(text)
    for _ in range(19):command('echo retained')
    for total in (0,1,63,64):
        add(100);add(458,total);add(451,15,7,0,1);add(456,0);add(456,12,0);add(450)
        add(451,9,4,11,0);add(456,14);add(450);add(456,14);add(450)
    for mode in (0,1):
        for scan in range(256):
            add(100);add(457,400,300,0,1);add(454,0,16,4,0);add(456,18,mode);add(456,19,scan);add(450)
        add(100);add(457,400,300,0,1);add(456,18,mode)
        for _ in range(70):add(456,19,0x1E)
        add(450);add(456,22);add(450)
    for value in (0,1,9,10,99,100,999,1000000000,0xffffffff):add(463,value)
    add(100);add(451,256,256,1,2);add(455);add(450)
    return rows


def tail_control(binary,run,source=False):
    count=0;counterexamples=0
    for total in (0,1,63,64,65,1023,1024,1025,2049):
        data=f'100\n458 {total}\n456 12 0\n456 13 0\n459\n'.encode()
        out=run([binary],data).decode()
        observed=[int(x)for x in next(x for x in out.splitlines()if x=='B'or x.startswith('B ')).split()[1:]]
        retained=min(total,64)
        expected=list(range(total-retained+1,total+1))
        if source and total>64:
            counterexamples+=1
            old_start=max(0,total-1024)+1
            assert observed==list(range(old_start,old_start+retained)),(total,observed)
            assert observed!=expected,(total,observed)
        else:assert observed==expected,(total,observed,expected)
        count+=1
    return dict(controls=count,counterexamples=counterexamples)if source else count


def boundary_controls(binary,run):
    count=0
    cases=[(0,257,(2,3,4,5,6,10,16)),(1,257,(2,3,4,10)),
           (2,17,(8,10,20)),(3,21,(15,)),(5,65,(20,23)),(6,8,(21,23)),
           (4,1024,(-1,))]
    for field,value,operations in cases:
        for operation in operations:
            values=[1,1,0,0,0,0,0];values[field]=value
            setup='460 '+' '.join(map(str,values))
            action='464'if operation==-1 else f'456 {operation} 0 0'
            data=f'100\n457 400 300 0 1\n{setup}\n450\n{action}\n450\n'.encode()
            out=run([binary],data).decode()
            assert not any(line.startswith('call ')for line in out.splitlines()),(field,operation,out[-1000:])
            states=[line for line in out.splitlines()if line.startswith('Y ')]
            assert len(states)==2 and states[0]==states[1],(field,operation,states)
            count+=1
    return count
