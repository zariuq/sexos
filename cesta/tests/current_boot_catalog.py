#!/usr/bin/env python3
"""Validate the Cesta BOOT declarations against current Rust catalog and layout facts."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent.parent


def run(argv):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True,text=True)
    if p.returncode:raise RuntimeError((argv,p.returncode,p.stdout[-2000:],p.stderr[-5000:]))
    return p.stdout


def rows(tool,table,*columns):
    raw=run([tool,'tables','rows','interfaces/kernel_current.data',table,*columns])
    return [r.split() for r in re.findall(r'\(Row ([^()]*)\)',raw)]


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path);args=ap.parse_args()
    w=args.work or Path(tempfile.mkdtemp(prefix='boot-catalog-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/kernel_current.source.json').read_text())
    for entry in inventory['sources']:
        assert hashlib.sha256((HERE.parent/entry['path']).read_bytes()).hexdigest()==entry['sha256'],entry['path']
    domains=rows(args.cesta,'Domains','id','name','module','key','layout','active')
    names={name:int(ident) for ident,name,*_ in domains}
    assert sorted(names.values())==list(range(1,len(domains)+1))
    active=[d for d in domains if d[-1]=='true'];assert len({d[3] for d in active})==len(active)
    assert all(0<int(d[3])<16 for d in domains) and len({d[4] for d in domains})==len(domains)
    init=(HERE.parent/'kernel/src/init.rs').read_text()
    modules=json.loads(re.search(r'let module_paths = (\[[^;]*\]);',init).group(1))
    assert len(active)==len(modules) and [d[2].strip('"') for d in domains[:len(modules)]]==modules
    assert 'create_boot_domain(module.path(),Some(incarnation.target),layout,pkey)' in init
    assert 'plan_boot_apps(&sex_pdx::app_catalog::TARGETS)' in init
    routes=rows(args.cesta,'Routes','holder','slot','target');assert routes
    held={(names[h],int(slot)):names[t] for h,slot,t in routes}
    assert len(held)==len(routes) and all(0<slot<=1024 for _,slot in held)
    for ident,name,*_ in domains:assert held[(int(ident),2)]==(14 if name in ('sexbell','kaleidoscope') else int(ident))
    apps=rows(args.cesta,'Apps','target','ordinal','surface','shell_surface','lifecycle_slot','close_slot','activation_slot','predecessor','logical','route_slot')
    actual=[[names[a[0]],*map(int,a[1:])] for a in apps]
    (w/'actual_catalog.rs').write_bytes((HERE.parent/'crates/sex-pdx/src/app_catalog.rs').read_bytes())
    (w/'oracle.rs').write_text('''#[path="actual_catalog.rs"]mod catalog;
use catalog::*;
fn main(){for i in INCARNATIONS{
let root=INCARNATIONS.iter().find(|p|p.app==i.app&&p.ordinal==0).unwrap();
let pred=if i.ordinal==0{0}else{INCARNATIONS.iter().find(|p|p.app==i.app&&p.ordinal+1==i.ordinal).unwrap().target};
println!("{} {} {} {} {} {} {} {} {} {}",i.target,i.ordinal,i.surface,i.shell_surface.unwrap_or(0),i.lifecycle_slot,i.close_slot,i.activation_slot.unwrap_or(0),pred,root.target,i.route_slot);
}}
''')
    run(['rustc','--edition=2021','--cap-lints','allow',w/'oracle.rs','-o',w/'oracle'])
    catalog=[list(map(int,line.split())) for line in run([w/'oracle']).splitlines()]
    expected=catalog
    for row in catalog:assert held[(3,row[9])]==row[0],('catalog route',row)
    assert actual==expected,(actual,expected)
    reserves=[(str(a[0]),str(a[0]),str(a[8])) for a in expected if a[1]]
    assert [(d[0],d[4],d[3]) for d in domains[len(modules):]]==reserves
    for target,ordinal,_,_,_,close,activation,pred,logical,route in actual:
        assert held[(3,close)]==target
        if ordinal:
            assert pred and activation and target!=logical
            assert any(r[0]==pred and r[1]+1==ordinal and r[8]==logical for r in actual)
        else:assert pred==activation==0 and target==logical
    expected_boot={'domains':[int(d[0]) for d in domains],'apps':[a[0] for a in actual],
                   'dormant':[int(d[0]) for d in domains if d[-1]=='false'],
                   'initial':len(active),'routes':len(routes),'backings':12+sum(a[8]==9 for a in actual)+sum(a[8] in (12,15) for a in actual)}
    assert inventory['expected_boot']==expected_boot,(inventory.get('expected_boot'),expected_boot)
    source=(HERE/'src/kernel_current_boot.cesta').read_text()
    assert 'SEXOS_CURRENT_DOMAIN_ROWS' in source and 'SEXOS_CURRENT_ROUTE_ROWS' in source and 'SEXOS_CURRENT_APP_ROWS' in source
    summary={'pin':inventory['pin'],'domains':len(domains),'initial':len(active),'routes':len(routes),'apps':len(apps),
             'actual_rust_catalog_equal':True,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in ('interfaces/kernel_current.data','interfaces/kernel_current.export','interfaces/kernel_current.source.json','tests/current_boot_catalog.py')},
             'scope':'declaration equality and structural constraints; routing values beyond the independently checked catalog are source-reviewed and exercised by the BOOT audit, not a Rust-init interpretation theorem'}
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('current BOOT:',len(domains),'domains',len(active),'initial',len(routes),'routes',len(apps),'actual Rust catalog entries match')


if __name__=='__main__':main()
