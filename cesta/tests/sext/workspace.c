#include "sext_service.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <inttypes.h>
#include <stdlib.h>
static _Alignas(4096) uint8_t storage[5*4096];
static SextWorkspace workspace;
static bool admitted;
static jmp_buf fault;
_Noreturn void sext_fatal(const char *reason,uint64_t status){(void)reason;(void)status;longjmp(fault,1);}
static void observe(void){
    printf("workspace %u",(unsigned)admitted);
    if(admitted)printf(" %"PRIu64" %zu %zu %"PRIu64,workspace.base-(uint64_t)(uintptr_t)storage,workspace.bytes,workspace.next,workspace.records);
    putchar('\n');printf("bytes ");for(size_t i=0;i<sizeof storage;++i)printf("%02x",storage[i]);putchar('\n');
}
int main(int argc,char **argv){
    if(argc!=2)return 2;
    unsigned mode=(unsigned)strtoul(argv[1],NULL,10);
    memset(storage,0xa5,sizeof storage);
    if(setjmp(fault)){puts("failure");observe();return 0;}
    uint64_t base=(uint64_t)(uintptr_t)(storage+4096),bytes=mode<3?(uint64_t)(mode+1)*4096:8192;
    if(mode==3)++base;
    if(mode==4)++bytes;
    sext_workspace_admit(&workspace,base,bytes);admitted=true;
    if(mode==5)workspace.records=UINT64_MAX-1;
    unsigned turns=mode==5?2:1000;
    for(unsigned i=1;i<=turns;++i){
        uint64_t result=sext_workspace_record(&workspace,10,i);
        printf("record %"PRIu64"\n",result);
    }
    sext_workspace_rewind(&workspace);
    printf("rewound %"PRIu64"\n",sext_workspace_record(&workspace,14,123));
    observe();return 0;
}

