#!/usr/bin/env python3
"""Exercise real typed-source syscalls/CPU providers with RDSEED on and off."""
import argparse,datetime,hashlib,json,re,subprocess,sys,time
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True)
    ap.add_argument('--kernel',type=Path,required=True);ap.add_argument('--probe',type=Path,required=True)
    ap.add_argument('--limine',type=Path,required=True);ap.add_argument('--work',type=Path,required=True)
    args=ap.parse_args();w=args.work.resolve();w.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(HERE.parent/'scripts'));from desktop_close_gate import QMP
    expected=json.loads((HERE/'interfaces/kernel_current.source.json').read_text())['expected_boot']
    command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),
             '--kernel',str(args.kernel.resolve()),'--limine',str(args.limine.resolve()),'--out',str(w/'package')]
    for sub in ('servers','apps'):
        for p in sorted((args.reference_tree/sub).iterdir()):
            if p.is_file():command+=['--replace',str(p.relative_to(args.reference_tree))+'='+str(args.probe.resolve())]
    with (w/'package.log').open('wb') as f:subprocess.run(command,cwd=HERE,stdout=f,stderr=subprocess.STDOUT,check=True)
    image=next((w/'package').glob('*.iso'));runs=[]
    for available in (True,False):
        label='rdseed-on' if available else 'rdseed-off';serial=w/(label+'.serial.log')
        cpu='max,+pku'+('' if available else ',-rdseed')
        socket=w/(label+'.sock')
        cmd=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'60','--fixed-clock','--cpu',cpu,
             '--qmp-socket',str(socket)]
        with (w/(label+'.qemu.log')).open('wb') as f:
            proc=subprocess.Popen(cmd,cwd=HERE,stdout=f,stderr=subprocess.STDOUT);q=None
            try:
                deadline=time.monotonic()+60
                while not socket.exists():
                    if proc.poll() is not None or time.monotonic()>deadline:raise RuntimeError('QMP startup failed')
                    time.sleep(.05)
                q=QMP(socket)
                while time.monotonic()<deadline:
                    text=serial.read_text(errors='replace') if serial.exists() else ''
                    if '[source.probe.failed]' in text:raise AssertionError(text[-4000:])
                    if all('[source.probe.done] role='+role in text for role in ('6','13')) and text.count('[source.probe.unheld] rejected=1 data_zero=1')==expected['initial']-2:break
                    if proc.poll() is not None:raise RuntimeError('QEMU stopped before all roles')
                    time.sleep(.05)
                q.cmd('quit');assert proc.wait(timeout=10)==0
            finally:
                if q:
                    if proc.poll() is None:
                        try:q.cmd('quit')
                        except (OSError,EOFError):pass
                    q.file.close();q.sock.close()
                if proc.poll() is None:proc.terminate();proc.wait(timeout=10)
        text=serial.read_text(errors='replace')
        for bad in ('KERNEL PANIC','panicked at','fault.kill','[source.probe.failed]'):
            assert bad not in text,(label,bad,text[-4000:])
        assert text.count('[source.probe.done] role=13')==text.count('[source.probe.done] role=6')==1
        m=re.findall(r'\[source.probe.entropy\] supported=([01]) sampled=([01]) private=1',text)
        assert m==[('1','1') if available else ('0','0')],(label,m)
        assert text.count('[source.probe.unheld] rejected=1 data_zero=1')==expected['initial']-2
        trust=re.findall(r'\[source.probe.trust\] profile=(\d+) root_bytes=(\d+) version=(\d+) consumer=(\d+) task=(\d+)',text)
        assert trust==[('1','376','1','15','15')],trust
        clocks=[tuple(map(int,r)) for r in re.findall(r'\[source.probe.clock\] role=(6|13) seconds=(\d+) attempts=(\d+)',text)]
        assert sorted(c[0] for c in clocks)==[6,13] and all(1<=c[2]<=4 for c in clocks),clocks
        # The fixed virtual RTC, not the host clock, is the independent witness.
        base=int(datetime.datetime(2026,1,1,10,tzinfo=datetime.timezone.utc).timestamp())
        assert all(base<=c[1]<=base+60 for c in clocks),clocks
        samples=re.findall(r'\[entropy.source.sample\]',text)
        assert len(samples)==int(available),(label,samples)
        runs.append({'rdseed_available':available,'clock_snapshots':clocks,'trust_metadata':trust,'unheld_callers':expected['initial']-2,'serial_sha256':digest(serial)})
    (w/'summary.json').write_text(json.dumps({'inputs':{'kernel':digest(args.kernel),'probe':digest(args.probe),'iso':digest(image)},'runs':runs,
        'scope':'real ordinary kernel entry/source instructions in a controlled q35 image with inert payloads; availability off cannot sample. No entropy quality, physical RTC integrity, application protocol or full-image equivalence claim.'},indent=2)+'\n')
    print('CPU source probes passed:held entropy/clock/trust,unheld failures,RDSEED on/off',flush=True)

if __name__=='__main__':main()
