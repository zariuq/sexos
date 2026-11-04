#!/usr/bin/env python3
"""Exercise an epoch-bound shared-region writer lease on real x86 PKU mappings."""
import argparse,hashlib,json,re,subprocess,sys,time
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True)
    ap.add_argument('--kernel',type=Path,required=True);ap.add_argument('--probe',type=Path,required=True)
    ap.add_argument('--limine',type=Path,required=True);ap.add_argument('--work',type=Path,required=True)
    args=ap.parse_args();work=args.work.resolve();work.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(HERE.parent/'scripts'));from desktop_close_gate import QMP
    expected=json.loads((HERE/'interfaces/kernel_current.source.json').read_text())['expected_boot']
    command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),
        '--kernel',str(args.kernel.resolve()),'--limine',str(args.limine.resolve()),'--out',str(work/'package')]
    for sub in ('servers','apps'):
        for path in sorted((args.reference_tree/sub).iterdir()):
            if path.is_file():command+=['--replace',str(path.relative_to(args.reference_tree))+'='+str(args.probe.resolve())]
    with (work/'package.log').open('wb') as stream:
        subprocess.run(command,cwd=HERE,stdout=stream,stderr=subprocess.STDOUT,check=True)
    image=next((work/'package').glob('*.iso'));serial=work/'serial.log';socket=work/'control.sock'
    run=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'90','--fixed-clock','--qmp-socket',str(socket)]
    with (work/'qemu.log').open('wb') as stream:
        process=subprocess.Popen(run,cwd=HERE,stdout=stream,stderr=subprocess.STDOUT);qmp=None
        try:
            deadline=time.monotonic()+90
            while not socket.exists():
                if process.poll() is not None or time.monotonic()>deadline:raise RuntimeError('QMP startup failed')
                time.sleep(.05)
            qmp=QMP(socket)
            while time.monotonic()<deadline:
                text=serial.read_text(errors='replace') if serial.exists() else ''
                if re.search(r'\[region\.writer\.probe\.failed\]|KERNEL PANIC|fault\.kill|#PF|#GP',text):
                    raise AssertionError(text[-7000:])
                if ('[region.writer.probe.home.done]' in text and '[region.writer.probe.peer.done]' in text and
                    text.count('[region.writer.probe.unheld] rejected=1')==expected['initial']-2):break
                if process.poll() is not None:raise RuntimeError('QEMU stopped before writer return')
                time.sleep(.05)
            qmp.cmd('quit');assert process.wait(timeout=10)==0
        finally:
            if qmp:
                if process.poll() is None:
                    try:qmp.cmd('quit')
                    except (OSError,EOFError,RuntimeError):pass
                qmp.file.close();qmp.sock.close()
            if process.poll() is None:process.terminate();process.wait(timeout=10)
    text=serial.read_text(errors='replace')
    events=[tuple(map(int,row)) for row in re.findall(
        r'\[region\.writer\.epoch\] action=(\d+) subject=(\d+) peer=(\d+) region=(\d+) current=(\d+) revision=(\d+) job=(\d+) epoch=(\d+)',text)]
    assert len(events)==6,events
    assert [event[:7] for event in events]==[(1,11,2,4,11,2,99),(2,2,11,4,2,3,99),(3,2,11,4,11,4,0),
        (1,11,2,4,11,5,100),(2,2,11,4,2,6,100),(5,11,2,4,11,7,0)],events
    assert all(events[i][7]<events[i+1][7] for i in range(len(events)-1)),events
    home=re.search(r'\[region\.writer\.probe\.home\.done\] revision=(\d+) value=(0x[0-9a-f]+) recovered=1',text)
    peer=re.search(r'\[region\.writer\.probe\.peer\.done\] revision=(\d+) job=0',text)
    assert home and peer and int(home[1])==7 and int(peer[1])==4 and int(home[2],16)==0x4444555566667777
    assert '[task.finish] pd=2 task=2 reason=99 terminal=1' in text
    summary={'inputs':{'kernel':digest(args.kernel),'probe':digest(args.probe),'iso':digest(image)},
        'events':events,'unheld_callers':expected['initial']-2,
        'scope':'actual x86 page-table aliases, PKU keys, capability query, offer/claim/release/recover, PDX request/reply, peer task finish, synthetic failure reply and CPU reads/writes; single-core current cohort, no DMA-lease or cross-reboot recovery'}
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('region-writer CPU probe passed: offer/claim/release and dead-peer recovery, two PKU aliases, 13 unheld callers')

if __name__=='__main__':main()
