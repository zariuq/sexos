#!/usr/bin/env python3
"""Read the upstream BOOT incarnation table through Rust, without granting authority."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

HERE = Path(__file__).resolve().parent.parent


def render(work):
    source = HERE.parent / 'crates/sex-pdx/src/app_catalog.rs'
    shell = (HERE.parent/'servers/silk-shell/src/main.rs').read_text()
    boot = re.search(r'const BOOT_LIFECYCLE_SURFACES:.*?= \[.*?\n\];', shell, re.S).group()
    constants = '\n'.join(re.search(r'(?:pub )?const '+name+r': u64 = [^;]+;',shell).group()
                          for name in sorted(set(re.findall(r'SURFACE_ID_\w+',boot))|{'SURFACE_ID_COMMAND_PALETTE'}))
    state = re.search(r'enum LifecycleState \{.*?\n\}',shell,re.S).group()
    probe = work / 'catalog.rs'
    probe.write_text('#![allow(dead_code)]\n#[path=' + json.dumps(str(source)) + '] mod app_catalog;\n' +
                     constants+'\n#[derive(Clone,Copy)]\n'+state+'\n'+boot+'\n'+r'''
fn main() {
    for row in app_catalog::INCARNATIONS {
        assert!(row.children.len() <= 3);
        let mut children = [0; 3];
        children[..row.children.len()].copy_from_slice(row.children);
        println!("A {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
            row.app as u32, row.ordinal, row.target, row.surface,
            row.shell_surface.unwrap_or(0), row.lifecycle_slot, row.close_slot,
            row.activation_slot.is_some(), row.activation_slot.unwrap_or(0),
            row.route_slot, row.children.len(), children[0], children[1], children[2]);
    }
    for (surface,state) in BOOT_LIFECYCLE_SURFACES {println!("L {} {}",surface,state as u32);}
    if !BOOT_LIFECYCLE_SURFACES.iter().any(|(surface,_)|*surface==SURFACE_ID_COMMAND_PALETTE){println!("S {} 0",SURFACE_ID_COMMAND_PALETTE);}
}
''')
    subprocess.run(['rustc', '--edition=2021', '-O', probe, '-o', work/'catalog'], check=True)
    output = subprocess.check_output([work/'catalog'], text=True).splitlines()
    rows = [line[2:] for line in output if line.startswith('A ')]
    lifecycle = [line[2:] for line in output if line.startswith('L ')]
    lazy = [line[2:] for line in output if line.startswith('S ')]
    columns = ('app ordinal target surface shell_surface lifecycle_slot close_slot '
               'has_activation activation_slot route_slot child_count child0 child1 child2').split()
    header = '(cesta-tables/1\n  (Table Apps\n    (Columns ' + ' '.join(
        '(Column '+name+(' Bool)' if name == 'has_activation' else ' Nat)') for name in columns)
    return (header + ')\n    (Key target) (Unique surface) (Unique lifecycle_slot)\n    (Rows\n' +
            ''.join('      (Row '+row+')\n' for row in rows) + '))\n' +
            '  (Table BootLifecycle\n    (Columns (Column surface Nat) (Column state Nat))\n' +
            '    (Key surface)\n    (Rows\n' +
            ''.join('      (Row '+row+')\n' for row in lifecycle) + '))\n' +
            '  (Table ShellLifecycle\n    (Columns (Column surface Nat) (Column state Nat))\n' +
            '    (Key surface)\n    (Rows\n' +
            ''.join('      (Row '+row+')\n' for row in lazy) + ')))\n').encode(), len(rows)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=HERE/'build/shell-catalog')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    content, count = render(args.output)
    target = HERE/'interfaces/shell_apps.data'
    if args.check:
        assert target.read_bytes() == content, 'upstream Shell catalog changed; regenerate and qualify it'
    else:
        target.write_bytes(content)
    evidence = dict(count=count, source_sha256=hashlib.sha256(
        (HERE.parent/'crates/sex-pdx/src/app_catalog.rs').read_bytes()).hexdigest(),
        table_sha256=hashlib.sha256(content).hexdigest())
    (args.output/'summary.json').write_text(json.dumps(evidence, indent=2)+'\n')
    print('Shell BOOT catalog:', count, 'descriptive associations')


if __name__ == '__main__':
    main()
