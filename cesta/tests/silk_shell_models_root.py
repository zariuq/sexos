"""Current Linen path through its actual global Models/root bindings."""
import argparse,json,hashlib,subprocess
from pathlib import Path
from silk_shell_models_render import prepare,corpus,independent,run,HERE


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--output',type=Path,default=HERE/'build/models-root-tests');args=parser.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True)
    summary=prepare(w);data,count=corpus();expected=run([w/'oracle'],data)
    units=('silk_shell_models','silk_shell_models_root','silk_shell_models_render','silk_shell_models_render_native','silk_shell_models_paint','silk_shell_models_paint_native','silk_shell_models_linen','silk_shell_models_publish','silk_shell_models_projection','shell_linen_snapshot','shell_linen_list','shell_text_delivery','shell_text_delivery_native');exports=[]
    for name in units:
        out=w/(name+'.c');run([args.cesta,'export-c','src/'+name+'.cesta','--std=c11','-o',out,'--','-Igenerated/include']);exports.append(out)
    provider=HERE/'tests/silk_shell_models_current/root_provider.c';configs=[]
    for cc in ('gcc','clang'):
        for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
            binary=w/(cc+mode);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-ffunction-sections','-fdata-sections',*options,'-Igenerated/include',*exports,provider,'-Wl,--gc-sections','-o',binary]);actual=run([binary],data)
            if actual!=expected:
                left,right=expected.splitlines(),actual.splitlines();i=next((i for i,pair in enumerate(zip(left,right))if pair[0]!=pair[1]),min(len(left),len(right)));raise AssertionError((cc,mode,i,left[max(0,i-2):i+2],right[max(0,i-2):i+2]))
            for operation in (13,14):
                r=subprocess.run([binary],input=f'0\n{operation}\n'.encode(),capture_output=True)
                assert r.returncode==88 and b'\nD 'not in r.stdout and b'\nT 'not in r.stdout,(operation,cc,mode,r.returncode,r.stdout)
            configs.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(actual).hexdigest(),independent_controls=independent(binary)+2))
    mutants=[];base=(w/'silk_shell_models_root.c').read_text()
    for name,old,new in (
        ('ignore_startup_fifo','shell_models_paint_surface(&view,startup_messages_pending())','shell_models_paint_surface(&view,false)'),
        ('lost_reply','return shell_models_linen_async_reply(&view,message)','return false'),
        ('skip_publication_repaint','linen_paint_surface_fast();','return;'),
    ):
        assert base.count(old)==1,(name,base.count(old));path=w/('mutant-'+name+'.c');path.write_text(base.replace(old,new));binary=w/('mutant-'+name)
        linked=[path if x.stem=='silk_shell_models_root'else x for x in exports];run(['gcc','-std=c11','-O2','-ffunction-sections','-fdata-sections','-Igenerated/include',*linked,provider,'-Wl,--gc-sections','-o',binary]);r=subprocess.run([binary],input=data,capture_output=True);assert r.returncode!=0 or r.stdout!=expected,('survived',name);mutants.append(name)
    summary.update(commands=count,configurations=configs,mutants_caught=mutants,boundary='Actual global Models/root bindings and component implementations; Chrome and transport scripted. Core event loop and joined boot are not exercised.')
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Actual Models root:',count,'commands/configuration; six compiler modes;',len(mutants),'mutations caught')


if __name__=='__main__':main()
