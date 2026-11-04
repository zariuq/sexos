"""Current Rust display outbox versus its checked, register-only Cesta port."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import subprocess

HERE=Path(__file__).resolve().parent.parent
ROOT=HERE.parent
FIX=HERE/'tests/shell_text_delivery'


def run(command,data=None):
    result=subprocess.run(list(map(str,command)),cwd=HERE,input=data,capture_output=True,
                          env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                               'UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})
    assert result.returncode==0,(command,result.returncode,result.stderr[-5000:])
    return result.stdout


def prepare(work):
    path=ROOT/'servers/silk-shell/src/linen_text_delivery.rs'
    production=path.read_text()
    extra='''
impl Outbox {
 pub fn test_sequence(&mut self,n:u32){self.sequence=n;}
 pub fn trace(&self){
  fn batch(b:&Batch){print!(" {} {} {} {} {} {} {} {} {} {} {} {} {} {}",b.len,b.color,b.offset,b.sequence,b.cleared as u8,b.awaiting as u8,b.selection.0,b.selection.1,b.metadata.count,b.metadata.first,b.metadata.rows,b.metadata.selected,sex_pdx::text_batch::hash(&b.bytes[..b.len]),b.selection_sent as u8);}
  print!("S {} {} {} {} {}",self.surface,self.atomic as u8,self.sequence,self.active.is_some()as u8,self.latest.is_some()as u8);if let Some(b)=&self.active{batch(b)}print!("|");if let Some(b)=&self.latest{batch(b)}println!();
 }
}
pub fn test_erase()->bool{let mut b=Batch{bytes:[255;CAPACITY],len:17,color:77,metadata:Metadata{count:1,first:2,rows:3,selected:4},cleared:false,offset:0,sequence:1,awaiting:false,selection:(2,3),selection_sent:true};b.erase();b.bytes.iter().all(|v|*v==0)&&b.selection==(0,0)&&!b.selection_sent&&b.len==17&&b.color==77&&b.metadata.selected==4}
'''
    module=work/'delivery.rs';module.write_text(production+extra)
    source=(FIX/'host.rs').read_text()
    for key,path in [('TEXT_ENCODING_PATH',ROOT/'crates/sex-pdx/src/text_encoding.rs'),('TEXT_BATCH_PATH',ROOT/'crates/sex-pdx/src/text_batch.rs'),('APP_CATALOG_PATH',ROOT/'crates/sex-pdx/src/app_catalog.rs'),('DELIVERY_PATH',module)]:source=source.replace(key,str(path))
    (work/'host.rs').write_text(source)
    run(['rustc','--edition=2021','-O',work/'host.rs','-o',work/'rust'])
    return {str(path.relative_to(ROOT)):hashlib.sha256(path.read_bytes()).hexdigest()for path in [ROOT/'servers/silk-shell/src/linen_text_delivery.rs',ROOT/'crates/sex-pdx/src/text_encoding.rs',ROOT/'crates/sex-pdx/src/text_batch.rs']}


def corpus():
    rows=[]
    def add(*n):rows.append(' '.join(map(str,n)))
    def ack(surface,sequence):return 0x56|(surface<<8)|(sequence<<24)
    for atomic in (0,1):
        for length in (0,1,7,8,9,255,256,511,512,513):
            for fail in (0,1,2,64):
                add(0,152,atomic);add(1,length,7,1);add(2,0xabcdef,7,2,3,4)
                add(4,1,fail,0);add(3,100);add(3,1);add(4,0,0,0)
                add(3,0);add(3,1);add(3,100);add(5,1,ack(152,1));add(5,1,ack(152,1));add(3,100)
    for atomic in (0,1):
        for error in (0,1,0xfffffffffffffffe,0xffffffffffffffff):
            for accepted in (0,1,2,65):
                add(0,152,atomic);add(1,33,1,1);add(2,1,2,3,4,5)
                add(1,17,3,1);add(2,6,7,8,9,10);add(1,9,5,1);add(2,11,12,13,14,15)
                add(4,2,accepted,error);add(3,100);add(4,0,0,0);add(3,100)
                add(5,1,ack(152,1));add(3,100);add(5,1,ack(152,3));add(7);add(3,100)
    for surface in (0,1,152,65535,65536,0xffffffffffffffff):
        for sequence in (0,0xfffffffe,0xffffffff):
            add(0,surface,1);add(8,sequence);add(1,8,0,1);add(2,7,8,9,10,11);add(3,100);add(2,12,13,14,15,16)
    for start,end in ((0,0),(0,4),(1,4),(4,4),(5,2),(0,513)):
        add(0,152,1);add(1,17,0,1);add(2,7,8,9,10,11,start,end);add(3,100);add(5,1,ack(152,1))
    # Valid/invalid UTF-8 and control scalar boundaries; bytes remain immutable.
    examples=[b'',b'a',b'\t\n ',b'\0',b'\x7f',b'\xc2\x80',b'\xc2\xa0',b'\xe2\x82\xac',b'\xf0\x9f\x98\x80',b'\xed\xa0\x80',b'\xc0\x80',b'\xf4\x90\x80\x80',b'\xff',b'\xe2\x82',b'\xef\xbf\xbd']
    for text in examples:
        add(0,152,1);add(1,len(text),0,1)
        for i,b in enumerate(text):add(6,i,b)
        for at in range(len(text)+2):add(9,at,0,len(text))
        for at in range(len(text)+1):add(2,7,8,9,10,11,0,at)
        add(3,100);add(7)
    rng=random.Random(73019)
    add(0,152,1)
    for _ in range(2000):
        kind=rng.randrange(7)
        if kind<2:
            add(1,rng.randrange(514),rng.randrange(256),rng.randrange(2));add(2,rng.randrange(1<<32),rng.randrange(12),rng.randrange(12),rng.randrange(6),rng.randrange(256))
        elif kind==2:add(3,rng.randrange(70))
        elif kind==3:add(4,rng.randrange(3),rng.randrange(6),rng.randrange(5))
        elif kind==4:add(5,rng.randrange(4),ack(rng.choice((0,152,153)),rng.randrange(1,600)))
        elif kind==5:add(7)
        else:add(3,0)
    add(10)
    return ('\n'.join(rows)+'\n').encode(),len(rows)


def independent(binary):
    def observe(rows):return run([binary],('\n'.join(rows)+'\n').encode()).decode().splitlines()
    ack=lambda surface,sequence:0x56|(surface<<8)|(sequence<<24)
    count=0
    for length in (0,1,8,9,512):
        out=observe(['0 152 1',f'1 {length} 0 1','2 7 2 3 4 5','3 100',f'5 2 {ack(152,1)}',f'5 1 {ack(153,1)}',f'5 1 {ack(152,2)}',f'5 1 {ack(152,1)}',f'5 1 {ack(152,1)}'])
        operations=[line.split()for line in out if line.startswith('O ')]
        assert [int(x[1])for x in operations]==[0x276]+[0x277]*((length+7)//8)+[0x278]
        expected=f'R 1 {length} 2 3 4 5'
        replies=[x for x in out if x.startswith('R ')]
        assert replies==['R 0 0 0 0 0 0']*3+[expected,'R 0 0 0 0 0 0'],replies
        text=bytes(ord('a')+i%26 for i in range(length));h=0xcbf29ce484222325
        for byte in text:h=((h^byte)*0x100000001b3)&((1<<64)-1)
        assert int(operations[-1][4])==h,(length,operations[-1],h)
        assert all(not x.startswith('P 3 ')for x in out),out
        count+=1
    out=observe(['0 152 1','1 17 0 1','2 1 1 1 1 1','4 1 0 0','3 1','3 1','3 1'])
    ops=[x for x in out if x.startswith('O ')];assert len(ops)==3 and len(set(ops))==1,ops;count+=1
    out=observe(['0 152 1','8 4294967295','1 8 0 1','2 1 1 1 1 1']);assert 'Q 0'in out and not any(x.startswith('O ')for x in out);count+=1
    out=observe(['0 152 1','1 512 0 1','2 1 1 1 1 1','1 513 0 1','2 2 2 2 2 2']);states=[x for x in out if x.startswith('S ')];assert states[-1]==states[-3],states[-3:];count+=1
    out=observe(['0 152 0','1 0 0 1','2 1 2 3 4 5','3 1']);assert 'P 3 0 0 2 3 4 5'in out and sum(x.startswith('O ')for x in out)==1;count+=1
    assert 'E 1'in observe(['10']);count+=1
    # The actual C boundary guards malformed native offsets before any send.
    if binary.name!='rust':
        out=observe(['0 152 1','1 8 0 1','2 1 2 3 4 5','11 513 514','3 1']);assert not any(x.startswith('O ')for x in out)and'P 4 1 0 0 0 0 0'in out;count+=1
    return count


def adapter_controls(work,cesta):
    include='-I'+str(HERE/'generated/include');source=work/'display-adapter.c';host=work/'display-host.c'
    run([cesta,'export-c',HERE/'src/shell_text_delivery_display.cesta','--std=c11','-o',source,'--plan',work/'display.plan','--',include])
    host.write_text('''#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "shell_text_delivery.h"
static uint64_t status;
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c){printf("O %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\\n",slot,op,a,b,c);return(SexosReply){status,123};}
int main(int argc,char **argv){if(argc!=2)return 2;status=strtoull(argv[1],NULL,10);ShellTextAdmission result=shell_text_display_admit((ShellTextOperation){0x276,152,UINT64_MAX,UINT64_MAX});printf("A %u %" PRIu64 "\\n",result.kind,result.status);return 0;}
''')
    controls=0
    for cc in ('gcc','clang'):
        for mode,flags in [('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=work/('display-'+cc+'-'+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,source,host,'-o',binary])
            for status in (0,1,2,0xfffffffffffffffe,0xffffffffffffffff):
                lines=run([binary,status]).decode().splitlines()
                assert lines[0]=='O 5 630 152 18446744073709551615 18446744073709551615',lines
                kind=0 if status==0 else 1 if status==0xfffffffffffffffe else 2
                assert lines[1]==f'A {kind} {status if kind==2 else 0}',lines
                controls+=1
    return controls


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/shell-text-delivery');parser.add_argument('--mutations',action='store_true');args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True);dependencies=prepare(work)
    data,count=corpus();(work/'commands.txt').write_bytes(data);expected=run([work/'rust'],data);(work/'rust.out').write_bytes(expected);independent(work/'rust')
    include='-I'+str(HERE/'generated/include');exports=[]
    for name in ('shell_text_delivery','shell_text_delivery_native'):
        target=work/(name+'.c');run([args.cesta,'export-c',HERE/'src'/(name+'.cesta'),'--std=c11','-o',target,*(['--plan',work/'text.plan']if name=='shell_text_delivery'else []),'--',include]);exports.append(target)
    configs=[]
    for cc in ('gcc','clang'):
        for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=work/(cc+'-'+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,*exports,FIX/'host.c','-o',binary]);observed=run([binary],data)
            if observed!=expected:
                (work/(binary.name+'.mismatch')).write_bytes(observed);left=expected.splitlines();right=observed.splitlines();i=next((i for i,(a,b)in enumerate(zip(left,right))if a!=b),min(len(left),len(right)));raise AssertionError((cc,mode,i,left[i:i+1],right[i:i+1]))
            controls=independent(binary);configs.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(observed).hexdigest()))
    mutants=[]
    if args.mutations:
        source=exports[0].read_text()
        for name,old,new in [('queue_replaces_active','if(!out->active){out->current=batch','if(true){out->current=batch'),('full_changes_offset','if(result.kind==SHELL_TEXT_FULL)return','if(result.kind==SHELL_TEXT_FULL){++out->current.offset;}if(result.kind==SHELL_TEXT_FULL)return'),('forged_reply','caller!=1 ||','false ||'),('wrong_receipt_sequence','sequence!=out->current.sequence','false'),('early_atomic_completion','else out->current.awaiting=true;','else {out->active=false;return(ShellTextPoll){.kind=SHELL_TEXT_COMPLETE};}'),('sequence_wraps','if(out->sequence==UINT32_MAX)return false;','if(false)return false;'),('empty_is_complete','if(out->atomic){\n        if(!batch->cleared)','if(out->atomic && batch->text.len){\n        if(!batch->cleared)')]:
            assert old in source,(name,old);path=work/(name+'.c');path.write_text(source.replace(old,new));binary=work/(name+'.bin');run(['gcc','-std=c11','-O2',include,path,exports[1],FIX/'host.c','-o',binary]);rejected=run([binary],data)!=expected
            if not rejected:
                try:independent(binary)
                except AssertionError:rejected=True
            assert rejected,(name,'mutant survived');mutants.append(name)
    adapters=adapter_controls(work,args.cesta)
    summary=dict(dependencies=dependencies,commands=count,configurations=configs,mutants=mutants,independent_controls=controls,adapter_controls=adapters,scope='Actual Outbox/UTF-8/text-batch reference bodies. Display send is scripted admission; the actual adapter is separately tested against a register-call peer. Applied receipts attest text state, not pixels or authority.')
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Text outbox:',count,'commands/configuration;',len(mutants),'mutations caught;',controls,'independent controls')


if __name__=='__main__':main()
