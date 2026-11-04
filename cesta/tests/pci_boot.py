#!/usr/bin/env python3
"""Read back real held PCI command updates on both supported NIC fixtures."""
import argparse,hashlib,json,re,subprocess,sys,time
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
    for model,product in [('none',None),('e1000',0x100e),('e1000e',0x10d3)]:
        serial=w/(model+'.serial.log');sock=w/(model+'.sock');held=int(product is not None)
        cmd=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'90','--fixed-clock','--qmp-socket',str(sock)]
        if held:cmd+=['--nic-model',model]
        with (w/(model+'.qemu.log')).open('wb') as f:
            proc=subprocess.Popen(cmd,cwd=HERE,stdout=f,stderr=subprocess.STDOUT);q=None
            try:
                deadline=time.monotonic()+90
                while not sock.exists():
                    if proc.poll() is not None or time.monotonic()>deadline:raise RuntimeError('QMP startup failed')
                    time.sleep(.05)
                q=QMP(sock)
                while time.monotonic()<deadline:
                    text=serial.read_text(errors='replace') if serial.exists() else ''
                    if re.search(r'\[pci.probe.failed\]|KERNEL PANIC|panicked at|fault.kill',text):raise AssertionError(text[-6000:])
                    if text.count('[pci.probe.unheld] rejected=1 data_zero=1')==expected['initial']-held and text.count('[pci.probe.done]')==held:break
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
        assert text.count('[pci.probe.unheld] rejected=1 data_zero=1')==expected['initial']-held
        done=re.findall(r'\[pci.probe.done\] vendor=(\d+) product=(\d+) enabled=(\d+) dma=held config=boot query_noop=1 disable=1 reenable=1 repeated_noop=1',text)
        assert len(done)==held,(model,done)
        if held:
            assert tuple(map(int,done[0][:2]))==(0x8086,product) and int(done[0][2])&6==6
            assert text.count('[pci.command.apply]')==5
            epochs=[tuple(map(int,row)) for row in re.findall(
                r'\[pci\.command\.epoch\] pd=(\d+) slot=(\d+) status=(\d+) attempted=(\d+) before=(\d+) expected=(\d+) after=(\d+) epoch=(\d+)',text)]
            assert len(epochs)==5,(model,epochs)
            assert all(status==0 and after==expected and attempted==int(expected!=before)
                       for _,_,status,attempted,before,expected,after,_ in epochs),(model,epochs)
            assert all(epochs[i][7]<epochs[i+1][7] for i in range(len(epochs)-1)),(model,epochs)
            assert '[pci.network.config]' in text and '[pci.device.identity]' in text
        else:
            assert '[pci.command.apply]' not in text and '[pci.command.epoch]' not in text
            assert '[pci.network.config]' not in text
        runs.append({'model':model,'product':product,'unheld_callers':expected['initial']-held,
            'command_updates':text.count('[pci.command.apply]'),
            'epoch_updates':text.count('[pci.command.epoch]'),'serial_sha256':digest(serial)})
    (w/'summary.json').write_text(json.dumps({'inputs':{'kernel':digest(args.kernel),'probe':digest(args.probe),'iso':digest(image)},'runs':runs,
        'scope':'real normal-kernel PCI entry/config-port readback,exact BOOT MMIO/DMA metadata,recorded command epochs and absent/e1000/e1000e fixtures. Payloads are inert;no packet/DMA-transfer,interrupt-delivery or application-network qualification.'},indent=2)+'\n')
    print('PCI CPU probes passed:absent/e1000/e1000e,held config/DMA,recorded query/enable/disable/re-enable/idempotence',flush=True)
if __name__=='__main__':main()
