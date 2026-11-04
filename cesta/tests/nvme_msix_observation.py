#!/usr/bin/env python3
"""Check a real all-Cesta NVMe MSI-X boot trace."""
import argparse,json,re
from pathlib import Path
def main():
    p=argparse.ArgumentParser();p.add_argument('--serial',type=Path,required=True);p.add_argument('--summary',type=Path);p.add_argument('--output',type=Path);a=p.parse_args()
    text=a.serial.read_text(errors='replace')
    assert '[kernel.nvme.reset.bound] pd=2 vector=34 generation=1 acknowledged=0' in text
    ready=re.search(r'\[kernel\.nvme\.msix\.ready\] pd=2 vector=34 cap=0x40 table=0x2000 entries=65 address=0xfee00000 data=0x22',text);assert ready
    assert '[sexdrive.nvme.interrupt.ready] vector=34 iv=0 ien=1 intmc=1' in text
    rows=[tuple(map(int,m))for m in re.findall(r'\[sexdrive\.interrupt\.ack\] vector=(\d+) revision=(\d+) generation=(\d+) status=(\d+) acknowledged=(\d+)',text)]
    assert len(rows)>=10,rows
    assert all(vector==34 and revision==i+1 and generation>=1 and status==0 and acknowledged==revision
        for i,(vector,revision,generation,status,acknowledged)in enumerate(rows)),rows
    epochs=[tuple(map(int,m))for m in re.findall(r'\[device\.reset\.epoch\] pd=2 action=4 generation=(\d+) phase=(\d+) vector=34 interrupt_revision=(\d+)',text)]
    assert len(epochs)>=len(rows) and all(phase in(0,1,2,3)for _,phase,_ in epochs)
    completions=len(re.findall(r'\[sexdrive\.(?:block\.read\.handoff\.nvme|nvme\.write|block\.nvme)\.cqe\]',text))
    assert completions>=10,completions
    forbidden=('KERNEL PANIC','pdx.opcode.unknown','[sexdrive.dma.quarantine]',
      '[sexdrive.dma.reset.failed]','cqe_timeout','reason=no_ioq_ready')
    assert not {term:text.count(term)for term in forbidden if term in text}
    boot=json.loads(a.summary.read_text())if a.summary else None
    if boot:assert boot['nvme'] and len(boot['markers'])==18 and not boot['forbidden_after_ready']
    result={'receipts':len(rows),'first_revision':rows[0][1],'last_revision':rows[-1][1],
      'generations':sorted(set(row[2]for row in rows)),'completion_records':completions,
      'reset_epochs':len(epochs),'scope':'real QEMU NVMe MSI-X production, kernel provenance, SexDrive dequeue and exact generation-bound acknowledgment; completion remains CQ-checked'}
    if a.output:a.output.write_text(json.dumps(result,indent=2)+'\n')
    print('NVMe MSI-X observation passed:',len(rows),'sequential receipts,',completions,'completion records')
if __name__=='__main__':main()
