#!/usr/bin/env python3
"""Check real Browser geometry/glyphs in an existing ordinary desktop image."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time

HERE=Path(__file__).resolve().parent.parent


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--seconds',type=int,default=120);ap.add_argument('--iso',type=Path,required=True);ap.add_argument('--work',type=Path,required=True)
    ap.add_argument('--civil-clock',action='store_true',help='also require the existing held-source civil Snapshot pixel oracle')
    args=ap.parse_args();w=args.work.resolve();w.mkdir(parents=True,exist_ok=False);image=args.iso.resolve()
    sys.path.insert(0,str(HERE.parent/'scripts'))
    from desktop_close_gate import QMP
    from check_desktop_pixels import verify_kaleido_layout,font_from_source
    from silkbar_clock_gate import verify_pixels as verify_clock,source_contract
    font_source=HERE.parent/'servers/sexdisplay/src/main.rs';font=font_from_source(font_source.read_text())
    clock_contract=source_contract(HERE.parent) if args.civil_clock else None
    serial=w/'serial.log';sock=w/'control.sock';started=time.monotonic()
    command=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),str(args.seconds),'--fixed-clock','--qmp-socket',str(sock)]
    q=None;checks=[];clock_checks=[]
    with (w/'qemu.log').open('wb') as log:
        proc=subprocess.Popen(command,cwd=HERE,stdout=log,stderr=subprocess.STDOUT)
        try:
            deadline=time.monotonic()+args.seconds
            while not sock.exists():
                if proc.poll() is not None or time.monotonic()>deadline:raise RuntimeError('QMP unavailable')
                time.sleep(.05)
            q=QMP(sock);errors=['not captured'];picture=None
            while time.monotonic()<deadline:
                text=serial.read_text(errors='replace') if serial.exists() else ''
                if re.search(r'panicked at|KERNEL PANIC|fault\.kill|\[sext.fatal\]',text):raise AssertionError(text[-5000:])
                if '[browser.visible.polish.done]' not in text or not re.search(r'\[shell.tile.apply\][^\n]*sid=300',text):
                    time.sleep(.1);continue
                # Bind a moving civil-time draw to a stopped CPU snapshot.
                if args.civil_clock:
                    q.cmd('stop');text=serial.read_text(errors='replace')
                picture=w/'desktop.ppm';q.cmd('screendump',{'filename':str(picture)})
                advertised=re.findall(r'init: FB handed to sexdisplay \((\d+)x(\d+) @',text)
                if not advertised:raise AssertionError('missing framebuffer handoff geometry')
                resolution=tuple(map(int,advertised[-1]))
                pixels=picture.read_bytes()
                errors=verify_kaleido_layout(text,pixels,font,resolution=resolution)
                if args.civil_clock:
                    clock=verify_clock(text,pixels,clock_contract,resolution=resolution,expected_state='Snapshot')
                    clock_checks.append(clock)
                    errors+=['civil clock: '+message for message in clock['errors']]
                checks.append(errors)
                if len(checks)==1:(w/'first-desktop.ppm').write_bytes(picture.read_bytes())
                if not errors:break
                if args.civil_clock:q.cmd('cont')
                time.sleep(1)
            assert picture and not errors,errors
            q.cmd('quit');assert proc.wait(timeout=10)==0
        finally:
            if q:q.file.close();q.sock.close()
            if proc.poll() is None:proc.terminate();proc.wait(timeout=10)
    text=serial.read_text(errors='replace')
    paging=subprocess.run(['python3',str(HERE.parent/'scripts/check_multi_client_paging_log.py'),str(serial)],capture_output=True)
    (w/'paging-checker.log').write_bytes(paging.stdout+paging.stderr)
    summary={'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'font_source_sha256':hashlib.sha256(font_source.read_bytes()).hexdigest(),'screenshot':picture.name,'screenshot_sha256':hashlib.sha256(picture.read_bytes()).hexdigest(),'resolution':resolution,'pixel_checks':checks,'civil_clock_checks':clock_checks,'clock_contract_sha256':clock_contract['source_sha256'] if clock_contract else None,'paging_checker_passed':paging.returncode==0,'seconds':time.monotonic()-started,'scope':'ordinary current-image Browser tile bounds and exact title/status glyph pixels; optional existing held-source civil Snapshot pixel oracle; separate from other windows, all results, lifecycle and complete desktop equivalence'}
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('ordinary Browser glyph/geometry pixels passed',flush=True)


if __name__=='__main__':main()
