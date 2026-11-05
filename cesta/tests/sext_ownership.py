#!/usr/bin/env python3
"""Admit actual Sext plans and refuse native-C-valid loan/escape controls."""
import argparse,hashlib,importlib,json,re,subprocess,sys,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent

def run(command,accepted=True):
    p=subprocess.run(list(map(str,command)),cwd=HERE,capture_output=True)
    assert (p.returncode==0)==accepted,(command,p.returncode,p.stdout[-1000:],p.stderr[-5000:])
    if not accepted:assert b'ownership' in p.stderr,p.stderr
    return p.stdout

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path)
    ap.add_argument('--lean-root',type=Path);ap.add_argument('--units',default='core,service');ap.add_argument('--work',type=Path)
    args=ap.parse_args()
    if args.lean_root and not args.cesta_source:ap.error('--lean-root requires --cesta-source')
    work=args.work or Path(tempfile.mkdtemp(prefix='sext-owned-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    units={'core':'sext','service':'sext_service'}
    names=args.units.split(',');assert all(n in units for n in names)
    files=[HERE/'src'/str(units[n]+'.cesta') for n in names]+list((HERE/'generated/include').glob('*.h'))
    frozen={p:p.read_bytes() for p in files};plans=[];controls=[];admission=[]
    for name in names:
        stem=units[name];source=HERE/'src'/str(stem+'.cesta');authored=LayoutText(frozen[source].decode())
        signatures=set(re.findall(r'(?m)^(?![ \t])[^;{}\n(=]*\b(sext_\w+)\([^;{}]*\)\s*\{',authored))
        assert signatures,(name,'no authored function definitions')
        for cc in ('gcc','clang'):
            plain=work/(name+'-'+cc+'.c');plan=work/(name+'-'+cc+'.plan')
            run([args.cesta,'export-c',source,'--cc',cc,'--std=c11','-o',plain,'--plan',plan,'--',
                 '-Igenerated/include','-DSEXOS_PDX_SENDER_TASK=1'])
            found=set(re.findall(r'\(function ([^\s()]+) ',plan.read_text()))
            assert signatures<=found,(name,'functions missing from plan',signatures-found)
            run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only','-Igenerated/include',plain])
            plans.append(dict(unit=name,compiler=cc,functions=len(found),sha256=hashlib.sha256(plan.read_bytes()).hexdigest(),path=plan.name))
        raw=LayoutText((work/(name+'-gcc.c')).read_text())
        if name=='core':
            changes=[
                ('allocator_parent_write','void sext_allocator_init(SextFrameAllocator *a, size_t quota)\n{',
                 'void sext_allocator_init(SextFrameAllocator *a, size_t quota)\n{\n    SextFrame *alias=&a->frames[0]; *a=(SextFrameAllocator){0}; (void)alias;'),
                ('query_stack_escape','SextFaultQuery query={0};',
                 'SextFaultQuery query={0}; sext_escaped=&query;'),
            ]
        else:
            changes=[
                ('service_parent_write','void sext_service_init(SextService *s)\n{',
                 'void sext_service_init(SextService *s)\n{\n    SextPager *alias=&s->pager; *s=(SextService){0}; (void)alias;'),
            ]
        for label,before,after in changes:
            assert authored.count(before)==raw.count(before)==1,(label,'source anchor')
            bad_authored=authored.replace(before,after);bad_plain=raw.replace(before,after)
            if label=='query_stack_escape':
                before_decl='bool sext_decode_fault('
                decl='static SextFaultQuery *sext_escaped;\n'
                assert bad_authored.count(before_decl)==bad_plain.count(before_decl)==1
                bad_authored=bad_authored.replace(before_decl,decl+before_decl)
                bad_plain=bad_plain.replace(before_decl,decl+before_decl)
            c=work/(label+'.c');c.write_text(bad_plain)
            cesta=work/(label+'.cesta');cesta.write_text(bad_authored)
            for cc in ('gcc','clang'):
                run([cc,'-std=c11','-fsyntax-only','-Igenerated/include',c])
                run([args.cesta,'export-c',cesta,'--cc',cc,'--std=c11','-o',work/(label+'-'+cc+'.export.c'),'--','-Igenerated/include'],False)
            controls.append(label)
    if args.lean_root:
        sys.path.insert(0,str(args.cesta_source.resolve()/'tests'))
        native=importlib.import_module('ownership_admission')
        for digest in sorted({p['sha256'] for p in plans}):
            item=next(p for p in plans if p['sha256']==digest)
            record,=native.parse((work/item['path']).read_bytes())
            admission.append(native.admit_record(record,work/('Admission'+str(len(admission))+'.lean'),args.lean_root.resolve()))
    assert all(p.read_bytes()==s for p,s in frozen.items()),'qualification inputs changed'
    (work/'summary.json').write_text(json.dumps(dict(plans=plans,source_controls=controls,admission=admission,
        sources={str(p.relative_to(HERE)):hashlib.sha256(s).hexdigest() for p,s in frozen.items() if p.suffix=='.cesta'},
        scope='declared native ownership; workspace address/retirement grants and syscall effects retain explicit obligations'),indent=2)+'\n')
    print('sext ownership:',len(controls),'native-C-valid source controls refused;',
          sum(a['functions'] for a in admission),'actual plans admitted')
if __name__=='__main__':main()

