"""Current bounded Linen snapshot and workspace policy against their Rust modules."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import subprocess

HERE=Path(__file__).resolve().parent.parent
FIX=HERE/'tests/shell_linen_snapshot'
MASK=(1<<64)-1


def run(args,data=None):
    result=subprocess.run(list(map(str,args)),cwd=HERE,input=data,capture_output=True)
    assert result.returncode==0,(args,result.returncode,result.stdout[-1000:],result.stderr[-6000:])
    return result.stdout


def prepare(work):
    inventory=json.loads((HERE/'interfaces/shell_linen_snapshot.source.json').read_text())
    for name,filename in (('snapshot','servers/silk-shell/src/linen_async.rs'),('policy','servers/silk-shell/src/document_policy.rs')):
        raw=run(['git','show',inventory['pin']+':'+filename])
        assert hashlib.sha256(raw).hexdigest()==inventory['dependencies'][filename]
        if name=='snapshot':raw+=b'\n'+(FIX/'inspect.rs').read_bytes()
        (work/(name+'.rs')).write_bytes(raw)
    (work/'oracle.rs').write_bytes((FIX/'provider.rs').read_bytes())
    run(['rustc','--edition=2021','-C','opt-level=1',work/'oracle.rs','-o',work/'rust'])
    return inventory


def corpus():
    rows=[]
    def add(*values):rows.append(' '.join(str(x&MASK)for x in values))
    def request():add(4);add(4);add(5)
    add(0);add(3);add(4)
    add(0);add(1,0);add(4);add(6,7,1,19);add(3)
    for rescan in (0,1):
        for length in (0,1,7,8,9,16,23,24):
            for kind in range(3):
                add(0);add(1,rescan);add(3)
                if rescan:request();add(6,7,1,0)
                for i in range(16):
                    request();add(6,8,1,MASK);add(6,7,2,MASK)
                    add(6,7,1,(i+1)|(kind<<32)|(length<<40)|((i%16)<<48))
                    for offset in range(0,length,8):
                        request();packed=sum((65+(i+offset+j)%26)<<(j*8)for j in range(min(8,length-offset)))
                        add(6,7,1,packed)
                    add(3)
                add(4);add(6,7,1,99);add(3)
    for bad in (1<<32,3<<32|1,25<<40|1,MASK):
        add(0);add(1,0);request();add(6,7,1,bad);add(3)
    for value in (0,1<<63,MASK,0x01021f20417e7f7f):
        add(0);add(1,0);request();add(6,7,1,1|(8<<40));request();add(6,7,1,value);add(3)
    add(0);add(1,0);request();add(6,7,1,19);request();add(6,7,1,19);add(3)
    for count in range(7):
        add(0)
        for i in range(count):add(2,i+1,MASK-i)
        add(3)
        add(0);add(1,1)
        for i in range(count):add(2,i+1,MASK-i)
        add(2,0,123);add(3)
        for i in range(min(count,4)):
            request();add(6,0,1,0);add(6,7,1,100+i);add(3)
        request();add(6,7,1,MASK);add(3)
    for subject,op,requested,published_id,disk,bits in itertools.product((0,200,201,MASK),(0,1,6,7,255),(0,1,2,MASK),(0,1,2,MASK),(0,2,3,15,16,255),range(8)):
        add(7,subject,op,requested,published_id,disk,bits&1,(bits>>1)&1,(bits>>2)&1)
    for op in range(256):add(7,200,op,1,1,3,1,1,0)
    return ('\n'.join(rows)+'\n').encode(),len(rows)


def independent(binary):
    def observe(commands):
        text=run([binary],(''.join(' '.join(str(x&MASK)for x in row)+'\n'for row in commands)).encode()).decode()
        return [line for line in text.splitlines()if line.startswith('R ')],text
    count=0
    values,text=observe([(0,),(2,11,21),(2,12,22),(2,13,23),(2,14,24),(2,15,25),(3,)])
    assert values==['R 1','R 1','R 1','R 1','R 0','R 0 1'],values;count+=1
    values,text=observe([(0,),(1,0),(4,),(4,)])
    assert values==['R 1 68 0 0 0']*2,values;count+=1
    values,text=observe([(0,),(1,0),(4,),(6,7,1,19),(3,)])
    assert values[-2:]==['R 0 0','R 1 1'],values;count+=1
    values,text=observe([(0,),(1,0),(4,),(5,),(6,8,1,0),(6,7,2,0),(3,)])
    assert values[-3:]==['R 0 0','R 0 0','R 1 0'],values;count+=1
    values,text=observe([(0,),(1,0),(4,),(5,),(6,7,1,1|(1<<40)),(4,),(5,),(6,7,1,31)])
    assert values[-1]=='R 1 0' and '3f'in text,text[-800:];count+=1
    for flags,want in ((0,1),(1,0),(2,0),(4,0)):
        values,_=observe([(7,200,0,1,1,3,not(flags&1),not(flags&2),bool(flags&4))])
        assert values==['R '+str(want)],values;count+=1
    return count


def negative(binary,rust=False):
    names=('sent-idle','sent-twice')if rust else('sent-idle','sent-twice','bad-phase','bad-count','bad-open-count','bad-offset')
    for name in names:
        result=subprocess.run([binary,name],capture_output=True)
        assert result.returncode==(101 if rust else 88),(name,result.returncode,result.stderr[-1000:])
        if not rust:assert b'runtime error:'not in result.stderr and b'AddressSanitizer'not in result.stderr
    return len(names)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/linen-snapshot-tests');parser.add_argument('--mutations',action='store_true');args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True);inventory=prepare(work)
    include='-I'+str(HERE/'generated/include');source=work/'snapshot.c'
    run([args.cesta,'export-c',HERE/'src/shell_linen_snapshot.cesta','--std=c11','-o',source,'--',include])
    data,commands=corpus();expected=run([work/'rust'],data);negative(work/'rust',True);configs=[]
    for cc in ('gcc','clang'):
        for mode,flags in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer','-fno-pie','-no-pie'])):
            binary=work/(cc+'-'+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',include,*flags,source,FIX/'provider.c','-o',binary])
            observed=run([binary],data)
            if observed!=expected:
                left,right=expected.splitlines(),observed.splitlines();line=next((i for i,pair in enumerate(zip(left,right))if pair[0]!=pair[1]),min(len(left),len(right)))
                raise AssertionError((cc,mode,line,left[max(0,line-2):line+2],right[max(0,line-2):line+2]))
            controls=independent(binary);refusals=negative(binary);configs.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(observed).hexdigest()))
    mutations=[]
    if args.mutations:
        body=source.read_text()
        for name,old,new in (
            ('accept_zero_open','if(!id || s->open_count==SHELL_LINEN_OPEN_CAPACITY)','if(s->open_count==SHELL_LINEN_OPEN_CAPACITY)'),
            ('queued_work_quiescent',' || s->open_count!=0',''),
            ('accept_foreign_reply','caller!=7 || opcode!=1','caller==0 || opcode!=1'),
            ('ignore_waiting',' || !s->awaiting',''),
            ('omit_duplicate_guard',' || duplicate',''),
            ('wrong_name_offset','s->offset+i','i'),
            ('keep_controls','(b>=0x20 && b<=0x7e)?b:\'?\'','b'),
            ('workspace_overrides_denial','active && !explicit_denial && published','active && published'),
            ('workspace_protected_disk','disk_path>=3 && disk_path<=15','disk_path>=2 && disk_path<=15'),
        ):
            assert old in body,(name,old);mutant=work/(name+'.c');mutant.write_text(body.replace(old,new));binary=work/name
            run(['gcc','-std=c11','-O2',include,mutant,FIX/'provider.c','-o',binary])
            changed=subprocess.run([binary],cwd=HERE,input=data,capture_output=True)
            rejected=changed.returncode!=0 or changed.stdout!=expected
            if not rejected:
                try:independent(binary)
                except AssertionError:rejected=True
            assert rejected,('mutant survived',name);mutations.append(name)
    summary=dict(pin=inventory['pin'],commands=commands,configurations=configs,mutants=mutations,independent_controls=controls,invalid_state_controls=refusals,scope='Actual current snapshot and workspace-policy modules. Shell publication/transport adapters and Models integration remain separate required work.')
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Linen snapshot:',commands,'commands/configuration;',len(mutations),'mutations caught;',controls,'independent controls')


if __name__=='__main__':main()
