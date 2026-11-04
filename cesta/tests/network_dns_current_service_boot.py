#!/usr/bin/env python3
"""Qualify the correlated Cesta DNS service through QEMU and a real host UDP peer."""
import argparse,hashlib,json,socketserver,subprocess,threading,time
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent
QUESTION=b"\x05probe\x05sexos\x04test\x00\x00\x01\x00\x01"
ANSWER=b"\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x3c\x00\x04\x0a\x00\x02\x02"
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

class Peer(socketserver.UDPServer):
 allow_reuse_address=True
 def __init__(self,address):super().__init__(address,Handler);self.received=[]
class Handler(socketserver.BaseRequestHandler):
 def handle(self):
  data,socket=self.request;self.server.received.append(data)
  if len(data)==12+len(QUESTION) and data[2:12]==b"\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00" and data[12:]==QUESTION:
   response=data[:2]+b"\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00"+QUESTION+ANSWER
   socket.sendto(response,self.client_address)

def main():
 p=argparse.ArgumentParser();p.add_argument('--reference-tree',type=Path,required=True);p.add_argument('--kernel',type=Path,required=True);p.add_argument('--service',type=Path,required=True);p.add_argument('--probe',type=Path,required=True);p.add_argument('--limine',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--qemu',default='qemu-system-x86_64');p.add_argument('--seconds',type=int,default=180);a=p.parse_args();out=a.output.resolve()
 if out.exists():raise RuntimeError('choose a new output directory; boot evidence is immutable')
 out.mkdir(parents=True);package=out/'package'
 with (out/'package.log').open('wb') as log:subprocess.run(['python3',HERE/'tools/package_client.py','--reference-tree',a.reference_tree.resolve(),'--replace','apps/kaleidoscope='+str(a.probe.resolve()),'--replace','servers/sexnet='+str(a.service.resolve()),'--kernel',a.kernel.resolve(),'--limine',a.limine.resolve(),'--out',package],cwd=HERE,stdout=log,stderr=subprocess.STDOUT,check=True)
 image,=package.glob('*.iso');serial=out/'serial.log';pcap=out/'wire.pcap';command=[a.qemu,'-M','q35','-m','512M','-cdrom',image,'-device','nec-usb-xhci,id=xhci','-device','usb-tablet,bus=xhci.0','-netdev','user,id=net0','-device','e1000,netdev=net0','-object','filter-dump,id=f0,netdev=net0,file='+str(pcap),'-serial','file:'+str(serial),'-display','none','-no-reboot','-no-shutdown','-cpu','max,+pku']
 peer=Peer(('127.0.0.1',46079));threading.Thread(target=peer.serve_forever,daemon=True).start();process=None;started=time.monotonic()
 try:
  with (out/'qemu.log').open('wb') as log:
   process=subprocess.Popen(list(map(str,command)),cwd=HERE,stdout=log,stderr=subprocess.STDOUT);deadline=time.monotonic()+a.seconds
   while process.poll() is None and time.monotonic()<deadline:
    text=serial.read_text(errors='replace') if serial.exists() else ''
    if '[network.dns.protocol.probe.done]' in text or 'KERNEL PANIC' in text or 'fault.kill pd=13' in text or 'fault.kill pd=15' in text or '[network.dns.protocol.probe.failed]' in text:break
    time.sleep(.2)
 finally:
  if process is not None and process.poll() is None:
   process.terminate()
   try:process.wait(timeout=5)
   except subprocess.TimeoutExpired:process.kill();process.wait()
  peer.shutdown();peer.server_close()
 text=serial.read_text(errors='replace');required=('[network.device.current.ready]','[network.current.ready]','[network.dns.protocol.probe.begin]','[network.dns.protocol.probe.admitted] ticket=1','[network.dns.protocol.probe.result] ticket=1 address=10.0.2.2','[network.dns.protocol.probe.done] ticket=1 positive_release=1')
 for marker in required:assert marker in text,marker
 for marker in ('KERNEL PANIC','fault.kill pd=13','fault.kill pd=15','[network.dns.protocol.probe.failed]'):assert marker not in text,marker
 assert peer.received and all(len(query)==12+len(QUESTION) and query[:2]==b'\x00\x01' and query[12:]==QUESTION for query in peer.received),list(map(len,peer.received))
 assert pcap.stat().st_size>24
 summary={'scope':'Current correlated Cesta DNS protocol probe through the real client, kernel, service, e1000/slirp and controlled host UDP peer.','inputs':{'image_sha256':digest(image),'kernel_sha256':digest(a.kernel.resolve()),'service_sha256':digest(a.service.resolve()),'probe_sha256':digest(a.probe.resolve())},'queries':len(peer.received),'answer':'10.0.2.2','wire_pcap_sha256':digest(pcap),'markers':list(required),'seconds':time.monotonic()-started}
 (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Current Cesta DNS protocol probe passed:',len(peer.received),'canonical query, answer 10.0.2.2 and positive release')
if __name__=='__main__':main()
