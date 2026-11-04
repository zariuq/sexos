#!/usr/bin/env python3
"""Run the current kernel's pre-user-entry BOOT/PTE audit with frozen payloads."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time

HERE=Path(__file__).resolve().parent.parent


def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True)
    ap.add_argument('--kernel',type=Path,required=True);ap.add_argument('--limine',type=Path,required=True)
    ap.add_argument('--work',type=Path,required=True);ap.add_argument('--network',action='store_true');ap.add_argument('--nvme-image',type=Path);ap.add_argument('--smp',type=int)
    args=ap.parse_args();w=args.work.resolve();w.mkdir(parents=True,exist_ok=False)
    kernel=args.kernel.resolve();before=digest(kernel)
    subprocess.run(['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),
        '--kernel',str(kernel),'--limine',str(args.limine.resolve()),'--out',str(w/'package')],cwd=HERE,check=True,capture_output=True)
    image=next((w/'package').glob('*.iso'));serial=w/'serial.log';started=time.monotonic()
    command=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'120','--fixed-clock','--until','[boot.current.audit.done]']
    if args.smp:
        assert 1<=args.smp<=128
        command+=['--smp',str(args.smp)]
    if args.network:command+=['--network']
    if args.nvme_image:command+=['--nvme-image',str(args.nvme_image.resolve())]
    with (w/'qemu.log').open('wb') as f:subprocess.run(command,cwd=HERE,stdout=f,stderr=subprocess.STDOUT,check=True)
    text=serial.read_text(errors='replace')
    expected=json.loads((HERE/'interfaces/kernel_current.source.json').read_text())['expected_boot']
    domains=[tuple(map(int,r)) for r in re.findall(r'\[boot.audit.domain\] id=(\d+) key=(\d+) layout=(\d+) active=(\d+) task=(\d+)',text)]
    assert len(domains)==len(expected['domains']) and {r[0] for r in domains}==set(expected['domains']),domains
    assert len([d for d in domains if d[3]])==expected['initial'] and len({d[1] for d in domains if d[3]})==expected['initial']
    assert all((d[3]==0)==(d[0] in expected['dormant']) and d[4]!=0 for d in domains)
    apps=[tuple(map(int,r)) for r in re.findall(r'\[boot.audit.app\] target=(\d+) task=(\d+) pages=(\d+) dormant=(\d+)',text)]
    assert len(apps)==len(expected['apps']) and {r[0] for r in apps}==set(expected['apps'])
    marker=re.search(r'\[boot.current.audit.done\] domains=(\d+) apps=(\d+) dormant=(\d+) routes=(\d+) backings=(\d+) hidden_pages=(\d+) active_pages=(\d+) user_entries=0',text)
    assert marker and tuple(map(int,marker.groups()[:5]))==(len(expected['domains']),len(expected['apps']),len(expected['dormant']),expected['routes'],expected['backings'])
    assert int(marker[6])==sum(r[2] for r in apps if r[3]) and int(marker[7])==sum(r[2] for r in apps if not r[3])
    audited_prefix=text[:marker.end()]
    for bad in ('KERNEL PANIC','fault.kill','#PF','#GP','first scheduled pd_id='):
        assert bad not in audited_prefix,(bad,'pre-user fence failed')
    assert digest(kernel)==before
    smp=re.search(r'\[smp\.parked\] supported=(\d+) cpus=(\d+) aps=(\d+) bsp_lapic=(\d+) generation=(\d+) x2apic=(\d+)',text)
    if args.smp:
        assert smp and tuple(map(int,smp.groups()[:3]))==(1,args.smp,args.smp-1),smp.groups() if smp else None
        assert int(smp[5])==0
    summary={'kernel_sha256':before,'image_sha256':digest(image),'domains':domains,'apps':apps,'seconds':time.monotonic()-started,
        'network':args.network,'nvme':bool(args.nvme_image),'smp':tuple(map(int,smp.groups())) if smp else None,'scope':'actual QEMU BOOT construction, permanent AP park/quiescence and PTE audit before user entry; no AP scheduling, shootdown, current userland, device transfer or whole-kernel qualification'}
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('current QEMU BOOT audit passed:',len(domains),'domains',len(apps),'manifests',len(expected['dormant']),'hidden successors;',args.smp or 1,'cores;0 user entries')


if __name__=='__main__':main()
