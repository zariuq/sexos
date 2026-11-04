"""Current Shell custody policy against its actual Rust module and independent controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import re
import subprocess

HERE=Path(__file__).resolve().parent.parent
INVENTORY=HERE/'interfaces/silk_shell_custody.source.json'
MASK=(1<<64)-1


def run(command, data=None):
    result=subprocess.run(list(map(str,command)),cwd=HERE,input=data,capture_output=True,
                          env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                               'UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})
    assert result.returncode==0,(command,result.returncode,result.stderr[-5000:])
    return result.stdout


def corpus(rows, seeds):
    commands=[]
    def put(*args):commands.append(' '.join(map(str,args)))
    for index,row in enumerate(rows):
        app,ordinal,target,surface,frame,slot,close,has_activate,activate,route,n,*children=row
        task=target+1000
        put(15,index);put(16,target,surface);put(16,target,surface+1)
        for flags in range(256):
            put(100,slot,target,task,surface);put(101,flags, MASK-7, 0xffffffff,2,333,222)
            put(0);put(1,frame);put(2,surface);put(2,frame);put(2,surface+9000);put(3)
            put(7);put(12,target,surface,target);put(12,target,surface+1,target)
            put(11,task,0xFA,surface,1,0);put(11,target,0xFA,surface,1,0)
            put(11,task,0xFA,surface+1,1,0);put(11,task,0xFA,surface,0,0)
            for opcode in (0,0x15,0x274,0x275,0x279,0xFFFF):
                put(11,task,opcode,target,task,0)
            put(5,7);put(5,8);put(5,MASK-8);put(4);put(6,MASK-7);put(5,7);put(5,8)
            put(8,target,target,task,1);put(7);put(4)
            put(9,target,target,task,0);put(9,0,target,task+1,0)
            put(9,0,target,task,0);put(10,target,task+1,1);put(10,target,task,1);put(13)
        # Every reserved identity takes the real close / death / acknowledgement route.
        put(100,slot,target,task,surface);put(4);put(8,target,target,task,1);put(4)
        put(9,0,target,task,0);put(10,target,task,31);put(101,2|4|1);put(13)
        put(9,0,target,task,0);put(11,task,0xFA,surface,99,0)
    for seed in seeds:
        rng=random.Random(seed)
        for _ in range(1500):
            if rng.randrange(12)==0:
                row=rng.choice(rows);put(100,row[5],row[2],rng.choice((row[2],row[2]+1000,0)),row[3])
            op=rng.randrange(17)
            if op in (15,16):
                put(op,rng.randrange(len(rows)+4),rng.randrange(360));continue
            a=[rng.choice((0,1,2,3,7,8,15,16,17,31,1009,1012,1015,MASK,1<<32)) for _ in range(6)]
            if op==11:a[1]=rng.choice((0,0xFA,0x15,0x274,0x275,0x279,0xFFFF))
            put(op,*a)
    for target,surface in ((0,0),(1000,300),(15,301)):
        put(100,999,target,71,surface)
        for op in (0,3,4,7,13):put(op)
        put(11,71,0xFA,surface,1,0)
    for caller in (0,1,2,3,0xffffffff):put(14,caller)
    for index in (len(rows),MASK):put(15,index)
    return ('\n'.join(commands)+'\n').encode(),len(commands)


def independent_controls(binary):
    controls=[
        ('task_namespace', ['100 40 15 71 300','11 71 250 300 1 0','11 15 250 300 1 0','11 0 250 300 1 0'], ['R','R 1','R 0','R 0']),
        ('wrong_surface_and_unknown_opcode',['100 40 15 71 300','11 71 250 301 1 0','11 71 999 300 1 0'],['R','R 0','R 0']),
        ('document_identity',['100 40 15 71 300','11 71 628 15 71 0','11 71 628 15 15 0','11 71 628 15 71 1'],['R','R 1','R 0','R 0']),
        ('spindle_launch',['100 39 12 71 154','11 71 21 7 0 0','11 71 21 8 0 0','11 71 21 7 1 0','11 71 0 0 0 0'],['R','R 1','R 0','R 0','R 1']),
        ('unsaved_close',['100 38 9 71 156','4','8 9 9 71 1','7','4','7'],['R','R 1','R 1','R 1','R 1','R 1']),
        ('forged_decline',['100 38 9 71 156','4','8 9 9 9 1','11 71 250 156 1 0'],['R','R 1','R 0','R 0']),
        ('hint_is_not_death',['100 40 15 71 300','9 0 15 71 0','11 71 250 300 1 0','10 15 71 0','11 71 250 300 1 0'],['R','R 1','R 1','R 0','R 1']),
        ('attested_terminal',['100 40 15 71 300','9 0 15 71 0','10 15 71 1','11 71 250 300 1 0','4','13','101 7','13','9 0 15 71 0'],['R','R 1','R 1','R 0','R 0','R 0','R','R 1','R 1']),
        ('forged_hint',['100 40 15 71 300','9 15 15 71 0','10 15 71 1','9 0 15 15 0','9 0 15 71 1'],['R','R 0','R 0','R 0','R 0']),
        ('wrapping_retry',['100 39 12 71 154','4','6 18446744073709551608','5 7','5 8'],['R','R 1','R','R 0','R 1']),
        ('unknown_catalog',['100 99 99 71 999','0','3','4'],['R','R 0 0','R 0 0','R 0']),
        ('paired_frame_not_child',['100 39 12 71 154','2 153','2 160','1 153','100 50 17 72 302','1 306','2 306'],['R','R 1','R 0','R 0','R','R 1','R 1']),
    ]
    for name,commands,expected in controls:
        output=run([binary], ('\n'.join(commands)+'\n').encode()).decode().splitlines()
        observed=output[::2]
        assert observed==expected,(name,observed,expected)
    return len(controls)


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-custody-tests')
    parser.add_argument('--mutations',action='store_true')
    args=parser.parse_args();work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    inventory=json.loads(INVENTORY.read_text())
    sources={HERE.parent/name:(HERE.parent/name).read_bytes() for name in inventory['sources']}
    assert all(hashlib.sha256(data).hexdigest()==inventory['sources'][str(path.relative_to(HERE.parent))] for path,data in sources.items()),'upstream source changed; preserve its pin and qualify the new source'
    rows=[]
    for text in re.findall(r'\(Row ([^\n)]+)\)',(HERE/'interfaces/shell_apps.data').read_text().split('(Table BootLifecycle')[0]):
        rows.append([1 if word=='true' else 0 if word=='false' else int(word) for word in text.split()])
    include='-I'+str(HERE/'generated/include')
    rust=work/'rust'
    run(['rustc','--edition=2021','-O',HERE/'tests/silk_shell_custody/provider.rs','-o',rust])
    data,commands=corpus(rows,[53,71,97,313]);(work/'commands.txt').write_bytes(data)
    expected=run([rust],data);(work/'rust.out').write_bytes(expected)
    independent=independent_controls(rust)
    results=[]
    source=HERE/'src/silk_shell_custody.cesta'
    for compiler in ('gcc','clang'):
        exported=work/(compiler+'.c')
        run([args.cesta,'export-c',source,'--cc',compiler,'--std=c11','-o',exported,'--',include])
        for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=work/(compiler+'-'+mode)
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,exported,HERE/'tests/silk_shell_custody/provider.c','-o',binary])
            observed=run([binary],data)
            if observed!=expected:
                (work/(compiler+'-'+mode+'.mismatch')).write_bytes(observed)
                raise AssertionError((compiler,mode,'Rust trace differs'))
            independent_controls(binary)
            results.append(dict(compiler=compiler,mode=mode,sha256=hashlib.sha256(observed).hexdigest()))
    mutants=[]
    if args.mutations:
        body=(work/'gcc.c').read_text()
        changes=[
            ('wrong_task','caller_task!=entry->task','caller_task!=entry->target'),
            ('ignore_dead','|| entry->dead || entry->close_pending','|| entry->close_pending'),
            ('ignore_pending_close','|| entry->dead || entry->close_pending','|| entry->dead'),
            ('wrong_surface','a==entry->surface && b!=0','b!=0'),
            ('forged_hint','caller!=0 || target!=entry->target','target!=entry->target'),
            ('wrong_attestation','task!=entry->task || (required&1)==0','(required&1)==0'),
            ('early_ack','!entry->dead || !entry->retired','!entry->dead'),
            ('retry_boundary','tick-entry->close_attempt>=16','tick-entry->close_attempt>16'),
            ('wrong_close_kind','entry->close_declined ? SHELL_CLOSE_CONFIRMED : SHELL_CLOSE_ASK','SHELL_CLOSE_ASK'),
            ('revive_on_ack','entry->acked=true;','entry->acked=true; entry->dead=false;'),
        ]
        for name,old,new in changes:
            assert body.count(old)==1,(name,old,body.count(old))
            path=work/(name+'.c');path.write_text(body.replace(old,new));binary=work/(name+'.bin')
            run(['gcc','-std=c11','-O2',include,path,HERE/'tests/silk_shell_custody/provider.c','-o',binary])
            observed=run([binary],data)
            assert observed!=expected,(name,'mutant survived')
            left=expected.splitlines();right=observed.splitlines()
            first=next(index for index,(a,b) in enumerate(zip(left,right)) if a!=b)
            mutants.append(dict(name=name,first_different_line=first+1))
    assert all(path.read_bytes()==data for path,data in sources.items()),'source changed during qualification'
    summary=dict(pin=inventory['pin'],catalog_count=len(rows),commands=commands,configurations=results,
                 independent_controls=independent,mutants=mutants,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest())
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('Shell custody:',len(rows),'incarnations;',commands,'commands per configuration;',independent,'independent controls;',len(mutants),'mutants caught')


if __name__=='__main__':main()
