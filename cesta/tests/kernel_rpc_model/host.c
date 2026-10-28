#define main legacy_trace_main
#include "../kernel_model/host.c"
#undef main

static void rpc_snapshot(void)
{
    for(uint32_t id=1;domain_get(id);++id) {
        Domain *d=domain_get(id);
        printf("|D%u:%u:%" PRIu64 ":",d->key,d->rpc_enabled?1:0,d->rpc_next);
        uint64_t previous=0;
        for(unsigned n=0;n<KERNEL_RPC_CAPACITY;++n) {
            RpcSlot *next=NULL;
            for(unsigned j=0;j<KERNEL_RPC_CAPACITY;++j) {
                RpcSlot *slot=&d->rpc[j];
                if(slot->state!=RPC_FREE && slot->id>previous && (!next || slot->id<next->id)) next=slot;
            }
            if(!next)break;
            printf("%" PRIu64 ":%u:%u:%" PRIu64 ",",next->id,domain_key(next->peer),
                   (unsigned)next->state,next->state==RPC_READY?next->value:0);
            previous=next->id;
        }
        printf("q");
        for(unsigned j=0;j<d->ring.count;++j) {
            RingEntry *m=&d->ring.entries[(d->ring.head+j)%KERNEL_RING_CAPACITY];
            assert(m->request_id && !m->from_call && m->arg1==m->arg0+1000 && m->arg2==m->arg0+2000);
            printf("%u:%" PRIu64 ":%" PRIu64 ":%" PRIu64 ",",domain_key(m->sender),m->request_id,m->type_id,m->arg0);
        }
    }
    putchar('\n');
}

int main(void)
{
    initialize();
    unsigned key,operation;uint64_t a,b,c;
    while(scanf("%u %u %" SCNu64 " %" SCNu64 " %" SCNu64,&key,&operation,&a,&b,&c)==5) {
        Domain *d=domain_of_key(key);assert(d);
        kernel.current=d->id;
        TrapFrame f={0};
        switch(operation) {
        case 0:f.rax=SEXOS_RPC_register;break;
        case 1:f.rax=SEXOS_RPC_call;f.rdi=a;f.rsi=b;f.rdx=c;f.r10=c+1000;f.r8=c+2000;break;
        case 2:f.rax=SEXOS_RPC_listen;break;
        case 3:f.rax=SEXOS_RPC_reply;f.rdi=domain_of_key((unsigned)a)->id;f.rsi=b;f.rdx=c;break;
        case 4:f.rax=SEXOS_RPC_poll;f.rdi=a;break;
        case 5:f.rax=SEXOS_RPC_cancel;f.rdi=a;break;
        default:abort();
        }
        kernel_syscall(&f);
        if(operation==2) {
            if(!f.rax)printf("Z");
            else {
                assert(f.r9 && f.r10==f.rdx+1000 && f.r8==f.rdx+2000);
                printf("M%u:%" PRIu64 ":%" PRIu64 ":%" PRIu64,domain_key((uint32_t)f.rsi),f.r9,f.rax,f.rdx);
            }
        } else if(operation==1 && f.rax==0) printf("B%" PRIu64,f.rsi);
        else if(operation==4 && f.rax==0)printf("V%" PRIu64,f.rsi);
        else if(f.rax==NO_ADDRESS)printf("U");
        else if(f.rax==ERR_CAP_INVALID)printf("R");
        else if(f.rax==ERR_SERVICE_NOT_READY)printf(operation==4?"P":"E");
        else printf("D%" PRIu64,f.rax);
        rpc_snapshot();
    }
    return 0;
}
