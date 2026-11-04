"""Exercise the actual native diagnostic adapter; never invoke activation."""
import argparse
from pathlib import Path
import subprocess
import json
import os

HERE=Path(__file__).resolve().parent.parent


def run(command,data=None,code=0):
    result=subprocess.run(list(map(str,command)),input=data,capture_output=True,
                          env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                               'UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})
    assert result.returncode==code,(command,result.returncode,result.stderr[-3000:])
    return result.stdout


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-surfaces-native')
    args=parser.parse_args();work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    native=work/'native.c';host=work/'host.c'
    include='-I'+str(HERE/'generated/include')
    run([args.cesta,'export-c',HERE/'src/silk_shell_surfaces_native.cesta','--std=c11','-o',native,'--',include])
    host.write_text('''#include <stdio.h>
#include <stdlib.h>
#include "silk_shell_surfaces.h"
_Noreturn void sexos_fail(void){exit(77);}
void sexos_raw_print(const char *bytes,size_t len){if(len>4096)abort();fwrite(bytes,1,len,stdout);}
int main(int argc,char **argv){if(argc!=2)return 2;ShellDebugRecord record={0};record.len=(size_t)strtoul(argv[1],NULL,10);for(size_t i=0;i<sizeof(record.bytes);++i)record.bytes[i]=(uint8_t)(i*17);shell_surfaces_debug_record(&record);return 0;}
''')
    checks=0
    for cc in ('gcc','clang'):
        for mode,flags in [('O2',['-O2']),('SAN',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=work/(cc+'-'+mode)
            run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,native,host,'-o',binary])
            for count in (0,1,511,512,1283,1535,1536):
                expected=bytes((i*17)&255 for i in range(count))
                assert run([binary,count])==expected,(cc,mode,count);checks+=1
            assert run([binary,1537],code=77)==b'';checks+=1
    (work/'summary.json').write_text(json.dumps(dict(checks=checks,scope='Actual diagnostic writer with synchronous raw-print peer. Activation syscall is compiled but never executed on the host.'),indent=2)+'\n')
    print('Surfaces native diagnostic:',checks,'controls')


if __name__=='__main__':main()
