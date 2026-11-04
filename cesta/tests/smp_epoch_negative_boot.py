#!/usr/bin/env python3
"""Prove that a mismatched parked-core generation stops the real current kernel."""
import argparse,hashlib,json,subprocess,time
from pathlib import Path
from silk_shell_boot import REPLACEMENTS
HERE=Path(__file__).resolve().parent.parent
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser();p.add_argument('--reference-tree',type=Path,required=True);p.add_argument('--kernel',type=Path,required=True)
    p.add_argument('--limine',type=Path,required=True);p.add_argument('--work',type=Path,required=True);a=p.parse_args();w=a.work.resolve();w.mkdir(parents=True,exist_ok=False)
    command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(a.reference_tree.resolve()),'--kernel',str(a.kernel.resolve()),'--limine',str(a.limine.resolve()),'--out',str(w/'package')]
    inputs={'kernel':digest(a.kernel.resolve())}
    for path,target in REPLACEMENTS.items():
        binary=(HERE/'build'/target/target).resolve();inputs[path]=digest(binary);command+=['--replace',path+'='+str(binary)]
    package=subprocess.run(command,cwd=HERE,capture_output=True,text=True,check=True);image=Path(package.stdout.strip());serial=w/'serial.log';started=time.monotonic()
    subprocess.run(['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'60','--fixed-clock','--smp','4','--until','KERNEL PANIC: SMP epoch quiescence invariant'],cwd=HERE,check=True,capture_output=True)
    text=serial.read_text(errors='replace');park=text.find('[smp.parked] supported=1 cpus=4 aps=3');panic=text.find('KERNEL PANIC: SMP epoch quiescence invariant')
    assert 0<=park<panic and '[kernel.epoch.commit]' not in text[:panic]
    summary={'inputs':inputs,'image_sha256':digest(image),'seconds':time.monotonic()-started,'park_offset':park,'panic_offset':panic,
      'scope':'test-only generation corruption after real four-vCPU AP enrollment; proves fail-stop coupling before the first BSP epoch publication, not arbitrary SMP fault recovery'}
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('SMP epoch negative boot passed: 3 APs parked; mismatched generation stopped before publication')
if __name__=='__main__':main()
