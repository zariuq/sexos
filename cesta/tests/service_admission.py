#!/usr/bin/env python3
"""Admit the network result's actual layout/accessor plans, including controls."""
import argparse
from pathlib import Path
import subprocess
import sys
import re
import tempfile

HERE=Path(__file__).resolve().parent.parent

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--cesta',type=Path,required=True)
    parser.add_argument('--cesta-source',type=Path,required=True)
    parser.add_argument('--lean-root',type=Path,required=True)
    parser.add_argument('--lean-memory-mb',type=int)
    args=parser.parse_args()
    sys.path.insert(0,str(args.cesta_source.resolve()/'tests'))
    from lean_contracts import named_theorems, code_without_comments, ALLOWED_AXIOMS
    from export_admission import atom
    from oracles.pro_compact_cesta import parse
    work=Path(tempfile.mkdtemp(prefix='service-admission-',dir=HERE/'build'))
    def run(command,cwd=HERE):
        result=subprocess.run(list(map(str,command)),cwd=cwd,capture_output=True)
        assert result.returncode==0,(command,result.stdout[-1000:],result.stderr[-3000:])
        return result.stdout
    for stem,module_name in (('rpc_wait','rpc_protocols'),('network_send','network_tx_protocol'),('network_device','network_rx_protocol')):
        actual=work/(stem+'.actual.plan');declared=work/(stem+'.module.plan')
        run([args.cesta,'export-c','--plan',actual,'src/'+stem+'.cesta','--','-Igenerated/include'])
        run([args.cesta,'protocol','export','interfaces/'+module_name+'.module','Protocols',
             '-o',work/(stem+'.h'),'--plan',declared])
        assert actual.read_bytes()==declared.read_bytes(),('compiled import differs from admitted module',stem)
    print('actual RPC, RX and TX imports equal the independently admitted module plans')
    module=HERE/'interfaces/network_layouts.module'
    layout=run([args.cesta,'layout','check',module,'Layouts'])
    generated=run([args.cesta,'layout','functions',module,'Layouts'])
    (work/'layout.plan').write_bytes(layout);(work/'functions.plan').write_bytes(generated)
    declarations=parse(layout);records=parse(generated)
    assert len(declarations)==1
    data=declarations[0]
    functions=next(r for r in records if r[0]=='cesta-layout-functions/1')
    views=next(r for r in records if r[0]=='cesta-layout-views/1')
    text='''import Mettapedia.GSLT.LanguageDef.LayoutViews
open Mettapedia.GSLT.LanguageDef.MacroExportPlan Mettapedia.GSLT.LanguageDef.LayoutDeclarations
open Mettapedia.GSLT.LanguageDef.LayoutLowering Mettapedia.GSLT.LanguageDef.LayoutViews
set_option Elab.async false
set_option maxRecDepth 4096
'''
    text+='def declared : Atom := '+atom(data)+'\n'
    text+='example : admitted declared = true := by decide\n'
    text+='def recorded : Atom := '+atom(functions)+'\n'
    text+='example : functionsAdmitted declared recorded = true := by decide\n'
    text+='example : viewsAdmitted declared ('+atom(views)+') = true := by decide\n'
    # A changed bit mask is a different, inadmissible lowering.
    changed=generated.replace(b'(lit 3)',b'(lit 2)',1)
    assert changed!=generated
    wrong=next(r for r in parse(changed) if r[0]=='cesta-layout-functions/1')
    text+='example : functionsAdmitted declared ('+atom(wrong)+') = false := by decide\n'
    for theorem in ('Mettapedia.GSLT.LanguageDef.LayoutLowering.lowering_of_admitted',
                    'Mettapedia.GSLT.LanguageDef.LayoutViews.views_of_admitted'):
        text+='#print axioms '+theorem+'\n'
    source=work/'Admission.lean';source.write_text(text)
    command=['lake','env','lean']
    if args.lean_memory_mb:command+=['-M',str(args.lean_memory_mb)]
    modules=['Mettapedia.Machines.Kernel.CorrelatedReplies','Mettapedia.Machines.Kernel.AgentCoordination']
    run(['lake','build','Mettapedia.Machines.Kernel',*modules],cwd=args.lean_root)
    genesis=work/'NetworkGenesis.lean'
    run(['python3','tools/genesis_lean.py','--cesta',args.cesta,'--network',genesis])
    with genesis.open('a') as stream:
        for name in ('memory_readers_sound','sexdisplay_memory_private','sexfiles_memory_confined','kaleidoscope_memory_confined'):
            stream.write('#print axioms Mettapedia.Machines.Kernel.SexOSNetworkGenesis.'+name+'\n')
    genesis_output=run([*command,'-DwarningAsError=true',genesis],cwd=args.lean_root)
    for axioms in re.findall(r'depends on axioms:\s*\[(.*?)\]',genesis_output.decode(),re.S):
        assert {value.strip() for value in axioms.split(',') if value.strip()}<=ALLOWED_AXIOMS
    assert b'sorryAx' not in genesis_output
    (work/'genesis.log').write_bytes(genesis_output)
    print('explicit network-profile genesis confinement kernel-checked')
    audited=[]
    for module in modules:
        path=args.lean_root/(module.replace('.','/')+'.lean')
        code=code_without_comments(path.read_text())
        assert not re.search(r'\b(?:sorry|axiom|native_decide|bv_decide|theorem_wanted)\b|_wanted',code)
        run([*command,'-DwarningAsError=true',path],cwd=args.lean_root)
        audited+=named_theorems(code)
    text=''.join('import '+name+'\n' for name in modules)+text
    text+=''.join('#print axioms '+name+'\n' for name in audited)
    source.write_text(text)
    output=run([*command,'-DwarningAsError=true',source],cwd=args.lean_root)
    assert b'sorryAx' not in output and b'Lean.ofReduceBool' not in output
    log=output.decode()
    checked=set(re.findall(r"'([^']+)' (?:depends on axioms|does not depend on any axioms)",log))
    assert set(audited)<=checked
    for axioms in re.findall(r'depends on axioms:\s*\[(.*?)\]',log,re.S):
        assert {value.strip() for value in axioms.split(',') if value.strip()}<=ALLOWED_AXIOMS
    (work/'lean.log').write_bytes(output)
    print(len(audited),'reply-window and agent-coordination theorem axioms audited')
    print('network result: actual declaration, accessors and views kernel-admitted; wrong mask refused')

if __name__=='__main__':main()
