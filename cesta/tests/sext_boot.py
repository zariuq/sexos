#!/usr/bin/env python3
"""Paired current-image paging and three-app retirement through hardware input."""
import argparse,hashlib,json,re,subprocess,sys,time
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--reference-iso',type=Path,required=True)
    ap.add_argument('--reference-tree',type=Path,required=True)
    ap.add_argument('--candidate',type=Path,required=True)
    ap.add_argument('--kernel',type=Path,help='also replace the reference kernel for a joined-kernel qualification')
    ap.add_argument('--startup-timeout',type=int,default=90)
    ap.add_argument('--limine',type=Path,required=True)
    ap.add_argument('--work',type=Path,required=True)
    ap.add_argument('--mode',choices=['paging','retirement','both'],default='both')
    args=ap.parse_args();repo=HERE.parent
    sys.path.insert(0,str(repo/'scripts'))
    from desktop_close_gate import QMP
    from check_desktop_close_log import check,responsiveness
    out=args.work.resolve();out.mkdir(parents=True,exist_ok=False)
    source_inputs={'reference_iso':digest(args.reference_iso),'candidate':digest(args.candidate)}
    if args.kernel:source_inputs['kernel']=digest(args.kernel)
    package=out/'package'
    with (out/'package.log').open('wb') as log:
        command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),
            '--replace','servers/sext='+str(args.candidate.resolve()),'--limine',str(args.limine.resolve()),
            '--out',str(package)]
        if args.kernel:command+=['--kernel',str(args.kernel.resolve())]
        subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT,check=True)
    manifest=json.loads((package/'manifest.json').read_text())
    changed=['servers/sext']+(['sexos-kernel']if args.kernel else[])
    assert sorted(manifest['changed_files'])==sorted(changed),'unexpected reference-tree payload changes'
    # The image/tree pair is a caller-provided frozen reference. Retain hashes
    # and the complete packaging manifest rather than infer source provenance.
    records=[]
    for mode in (['paging','retirement'] if args.mode=='both' else [args.mode]):
        for language,image in [('rust',args.reference_iso.resolve()),('cesta',next(package.glob('*.iso')))]:
            d=out/(mode+'-'+language);d.mkdir();serial=d/'serial.log';started=time.monotonic()
            if mode=='paging':
                with (d/'qemu.log').open('wb') as log:
                    subprocess.run(['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'120',
                        '--until','[paging.client.retained]'],cwd=repo,stdout=log,stderr=subprocess.STDOUT,check=True)
                checked=subprocess.run(['python3',str(repo/'scripts/check_multi_client_paging_log.py'),str(serial)],
                                       cwd=repo,capture_output=True)
                (d/'checker.log').write_bytes(checked.stdout+checked.stderr)
                assert checked.returncode==0,(language,checked.stdout,checked.stderr)
                assert not re.search(r'KERNEL PANIC|GP FAULT|fault\.kill|\[sext.fatal\]|\[cesta.client.fault\]',serial.read_text(errors='replace'))
            else:
                disk=d/'disk.raw';sock=d/'qmp.sock'
                assert len(str(sock.resolve()).encode())<104,'choose a shorter output path for the QMP socket'
                with disk.open('wb') as file:file.truncate(8*1024*1024)
                command=['qemu-system-x86_64','-M','q35','-cpu','max,+pku','-m','512M','-cdrom',str(image),
                    '-drive','if=none,id=nvm,file='+str(disk)+',format=raw','-device','nvme,serial=sexos01,drive=nvm',
                    '-serial','file:'+str(serial),'-qmp','unix:'+str(sock)+',server=on,wait=off',
                    '-display','none','-no-reboot','-no-shutdown']
                process=None;console=None
                def text():return serial.read_text(errors='replace') if serial.exists() else ''
                def wait(pattern,start=0,timeout=90):
                    end=time.monotonic()+timeout
                    while time.monotonic()<end:
                        value=text();match=re.search(pattern,value[start:])
                        if match:return match
                        if process.poll() is not None:raise RuntimeError('QEMU exited')
                        if re.search(r'KERNEL PANIC|GP FAULT|fault\.kill|\[sext.fatal\]|panicked at',value):raise RuntimeError('guest fatal')
                        time.sleep(.1)
                    raise RuntimeError('missing '+pattern)
                try:
                    console=(d/'qemu.log').open('wb');process=subprocess.Popen(command,stdout=console,stderr=subprocess.STDOUT)
                    deadline=time.monotonic()+20
                    while not sock.exists():
                        assert time.monotonic()<deadline,'QMP startup';time.sleep(.1)
                    q=QMP(sock)
                    def keys(codes):
                        q.cmd('input-send-event',{'events':[{'type':'key','data':{'down':down,
                            'key':{'type':'qcode','data':code}}} for code in codes for down in (True,False)]})
                    def focus(sid,native=True):
                        for _ in range(24):
                            events=list(re.finditer(r'\[shell.focus.(?:set\] id=(\d+)|clear\])',text()))
                            current=int(events[-1][1]) if events and events[-1][1] else 0
                            if current==sid:return
                            start=len(text())
                            if sid==153:
                                keys(['scroll_lock']);wait(r'\[shell.focus.set\] id=153',start,15);return
                            if native:
                                q.cmd('input-send-event',{'events':[{'type':'key','data':{'down':down,
                                    'key':{'type':'qcode','data':code}}} for code,down in
                                    [('alt',True),('tab',True),('tab',False),('alt',False)]]})
                            else:keys(['tab'])
                            wait(r'\[shell.focus.set\] id=\d+',start,15);time.sleep(.1)
                        raise RuntimeError('focus '+str(sid))
                    wait(r'\[app.surface.presented\] target=15 surface=300 ',timeout=args.startup_timeout)
                    history=wait(r'\[spindle.history.receipt\] target=12 .*kind=(loaded|missing) .*error=(\d+)')
                    assert (history[1],history[2]) in [('loaded','0'),('missing','4')],history.group(0)
                    for target,sid in ((15,300),(9,201),(12,153)):
                        focus(sid);start=len(text());keys(['f11'])
                        wait(r'\[sext.app.pool.ready\] target='+str(target)+r' .*ok=1',start,120)
                    focus(200,False)
                    for _ in range(8):
                        start=len(text());keys(['grave_accent']);wait(r'\[shell.palette.item\] idx=0',start,15)
                        start=len(text());keys(['j']);wait(r'\[shell.palette.select\] old=0 new=1',start,15)
                        start=len(text());keys(['j']);wait(r'\[shell.palette.select\] old=1 new=2',start,15)
                        start=len(text());keys(['ret','j'])
                        wait(r'\[linen.remote.snapshot.ok\] .*complete=1',start,60)
                        after_snapshot=len(text());keys(['j'])
                        wait(r'\[shell.action.select_next_linen\] id=[1-9]\d*',after_snapshot,15)
                        if responsiveness(text(),start):break
                    errors=check(text())
                    (d/'checker.json').write_text(json.dumps({'errors':errors},indent=2)+'\n')
                    assert not errors,errors
                finally:
                    if process and process.poll() is None:
                        process.terminate()
                        try:process.wait(timeout=5)
                        except subprocess.TimeoutExpired:process.kill();process.wait()
                    if console:console.close()
            record=dict(mode=mode,implementation=language,seconds=time.monotonic()-started,
                        image_sha256=digest(image),checker='passed')
            (d/'summary.json').write_text(json.dumps(record,indent=2)+'\n');records.append(record)
            print(mode,language,'upstream correctness checker passed',flush=True)
    assert digest(args.reference_iso)==source_inputs['reference_iso'] and digest(args.candidate)==source_inputs['candidate'],'boot inputs changed'
    if args.kernel:assert digest(args.kernel)==source_inputs['kernel'],'kernel changed'
    (out/'summary.json').write_text(json.dumps(dict(inputs=source_inputs,records=records,
        scope='frozen Rust userland with Sext component swap'+(' and candidate kernel join'if args.kernel else' on the reference Rust kernel')+'; separate from all-Cesta userland'),indent=2)+'\n')
if __name__=='__main__':main()

