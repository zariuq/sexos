#!/usr/bin/env python3
"""Actual workspace memory, rotation, guards and overflow against upstream Rust."""
import argparse,hashlib,json,re,subprocess,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(command,**kw):
    p=subprocess.run(list(map(str,command)),cwd=HERE,capture_output=True,**kw)
    assert p.returncode==0,(command,p.returncode,p.stderr[-4000:])
    return p.stdout
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path);args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='sext-workspace-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    inv=json.loads((HERE/'interfaces/sext.source.json').read_text())
    name='servers/sext/src/workspace.rs'
    source=run(['git','show',inv['pin']+':'+name]);expected=next(x['sha256'] for x in inv['sources'] if x['path']==name)
    assert hashlib.sha256(source).hexdigest()==expected
    reference=(HERE/'tests/sext/workspace.rs').read_text().replace('@SOURCE@',re.sub(r'(?m)^//!.*\n','',source.decode()))
    (work/'oracle.rs').write_text(reference);run(['rustc','--edition=2021','-O',work/'oracle.rs','-o',work/'rust'])
    golden={mode:run([work/'rust',str(mode)]) for mode in range(6)}
    exported=work/'sext_workspace.c'
    run([args.cesta,'export-c','src/sext_workspace.cesta','--std=c11','-o',exported,'--','-Igenerated/include'])
    builds=[]
    for cc in ('gcc','clang'):
        for sanitized in (False,True):
            flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include']
            flags+=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if sanitized else ['-O2']
            binary=work/(cc+('-sanitized' if sanitized else '-ordinary'))
            run([cc,*flags,HERE/'tests/sext/workspace.c',exported,'-o',binary])
            for mode,expected in golden.items():
                actual=run([binary,str(mode)])
                assert actual==expected,(cc,sanitized,mode,'actual pool or workspace state differs')
            if not sanitized:
                authored=work/(cc+'-authored')
                run([args.cesta,'cc','--cc',cc,*flags,HERE/'tests/sext/workspace.c',HERE/'src/sext_workspace.cesta','-o',authored])
                assert authored.read_bytes()==binary.read_bytes(),('workspace binary identity',cc)
            builds.append(dict(compiler=cc,sanitized=sanitized,cases=6))
    mutations=[('skip_zero','for(uint64_t i=0;i<bytes;++i)p[i]=0;','for(uint64_t i=0;i<bytes;++i)p[i]=0xa5;'),
               ('rewind_count','void sext_workspace_rewind(SextWorkspace *w){w->next=0;}','void sext_workspace_rewind(SextWorkspace *w){w->next=0;w->records=0;}'),
               ('record_wrap','if(w->records==UINT64_MAX)','if(false)')]
    original=exported.read_text();killed=[]
    for name,before,after in mutations:
        assert original.count(before)==1,(name,'anchor')
        mutant=work/(name+'.c');mutant.write_text(original.replace(before,after));binary=work/name
        run(['gcc','-std=c11','-O2','-Igenerated/include',HERE/'tests/sext/workspace.c',mutant,'-o',binary])
        assert any(run([binary,str(mode)])!=expected for mode,expected in golden.items()),('workspace mutation survived',name)
        killed.append(name)
    (work/'summary.json').write_text(json.dumps(dict(pin=inv['pin'],builds=builds,mutations=killed,
        scope='actual aligned pool bytes; unsafe-memory grant validity remains the caller obligation'),indent=2)+'\n')
    print('sext workspace: 24 ordinary/sanitized real-memory executions, 2 export identities, 3 mutations detected')
if __name__=='__main__':main()

