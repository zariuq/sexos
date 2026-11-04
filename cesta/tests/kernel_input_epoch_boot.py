#!/usr/bin/env python3
"""Drive one real PS/2 key through the current kernel's revisioned input epochs."""
import argparse,hashlib,json,socket,subprocess,time
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def qmp_reply(stream):
 while True:
  message=json.loads(stream.readline())
  if 'return' in message or 'error' in message:return message
def qmp_execute(stream,execute,arguments=None):
 request={'execute':execute}
 if arguments is not None:request['arguments']=arguments
 stream.write((json.dumps(request)+'\n').encode());stream.flush()
 reply=qmp_reply(stream)
 if 'error' in reply:raise RuntimeError(reply)
 return reply['return']
def connect_qmp(path,deadline):
 while time.monotonic()<deadline:
  try:
   client=socket.socket(socket.AF_UNIX,socket.SOCK_STREAM);client.connect(str(path));stream=client.makefile('rwb')
   greeting=json.loads(stream.readline())
   if 'QMP' not in greeting:raise RuntimeError(greeting)
   qmp_execute(stream,'qmp_capabilities')
   return client,stream
  except (FileNotFoundError,ConnectionRefusedError):
   time.sleep(.05)
 raise RuntimeError('QMP socket unavailable')

def main():
 p=argparse.ArgumentParser();p.add_argument('--reference-tree',type=Path,required=True);p.add_argument('--kernel',type=Path,required=True);p.add_argument('--limine',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--qemu',default='qemu-system-x86_64');p.add_argument('--seconds',type=int,default=120);a=p.parse_args();out=a.output.resolve()
 if out.exists():raise RuntimeError('choose a new output directory; boot evidence is immutable')
 out.mkdir(parents=True);package=out/'package'
 with (out/'package.log').open('wb') as log:subprocess.run(['python3',HERE/'tools/package_client.py','--reference-tree',a.reference_tree.resolve(),'--kernel',a.kernel.resolve(),'--limine',a.limine.resolve(),'--out',package],cwd=HERE,stdout=log,stderr=subprocess.STDOUT,check=True)
 image,=package.glob('*.iso');serial=out/'serial.log';qmp=out/'qmp.sock';command=[a.qemu,'-M','q35','-m','512M','-cdrom',image,'-device','nec-usb-xhci,id=xhci','-device','usb-tablet,bus=xhci.0','-nic','none','-serial','file:'+str(serial),'-display','none','-no-reboot','-no-shutdown','-cpu','max,+pku','-qmp','unix:'+str(qmp)+',server=on,wait=off']
 process=None;client=None;stream=None;started=time.monotonic()
 try:
  with (out/'qemu.log').open('wb') as log:
   process=subprocess.Popen(list(map(str,command)),cwd=HERE,stdout=log,stderr=subprocess.STDOUT);deadline=time.monotonic()+a.seconds
   client,stream=connect_qmp(qmp,deadline)
   while process.poll() is None and time.monotonic()<deadline:
    text=serial.read_text(errors='replace') if serial.exists() else ''
    if '[silk-shell.ready]' in text or 'KERNEL PANIC' in text:break
    time.sleep(.1)
   text=serial.read_text(errors='replace')
   assert '[silk-shell.ready]' in text and 'KERNEL PANIC' not in text
   start=len(text)
   qmp_execute(stream,'send-key',{'keys':[{'type':'qcode','data':'a'}]})
   key_deadline=min(deadline,time.monotonic()+20)
   while process.poll() is None and time.monotonic()<key_deadline:
    text=serial.read_text(errors='replace')
    suffix=text[start:]
    if 'kind=push' in suffix and 'kind=pop' in suffix and 'scancode=30' in suffix:break
    time.sleep(.05)
 finally:
  if stream is not None:stream.close()
  if client is not None:client.close()
  if process is not None and process.poll() is None:
   process.terminate()
   try:process.wait(timeout=5)
   except subprocess.TimeoutExpired:process.kill();process.wait()
 text=serial.read_text(errors='replace');suffix=text[start:]
 import re
 events=[(kind,int(revision),int(code),int(count)) for kind,revision,code,count in re.findall(r'\[kernel\.input\.epoch\.commit\] kind=(push|pop) revision=(\d+) scancode=(\d+) count=(\d+)',suffix)]
 press_push=next((event for event in events if event[0]=='push' and event[2]==30),None)
 press_pop=next((event for event in events if event[0]=='pop' and event[2]==30),None)
 assert press_push and press_pop and press_push[1]<press_pop[1],events
 assert [event[1] for event in events]==sorted(set(event[1] for event in events)),events
 for marker in ('KERNEL PANIC','fault.kill','#PF','#GP'):assert marker not in suffix,marker
 summary={'scope':'One QMP-injected physical PS/2 key through the current kernel input IRQ, revisioned push/pop epochs, Rust SexInput and Rust Silk Shell.','kernel_sha256':digest(a.kernel.resolve()),'image_sha256':digest(image),'events':events,'seconds':time.monotonic()-started}
 (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('current kernel input epoch boot passed:',len(events),'commits; scancode 30 push/pop observed')
if __name__=='__main__':main()
