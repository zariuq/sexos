#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "rpc_runtime.c"
#include "rpc_wait.c"

static unsigned scenario, registrations, begins, polls, cancels, legacy_calls, listens, yields;
static uint64_t active = 731;
static bool legacy(void) { return scenario >= 6 && scenario <= 8; }

uint64_t sexos_rpc_register(void) { ++registrations; return legacy() ? UINT64_MAX : 1; }
SexosReply sexos_rpc_begin(uint64_t slot,uint64_t operation,uint64_t a,uint64_t b,uint64_t c)
{
    assert(slot==18 && operation==512 && a==524 && b==0 && c==0);
    ++begins;
    if (scenario==3) return (SexosReply){SEXOS_RPC_BUSY,0};
    if (scenario==4 || (scenario==5 && begins<3)) return (SexosReply){SEXOS_RPC_UNSUPPORTED,0};
    return (SexosReply){0,active};
}
SexosReply sexos_rpc_poll(uint64_t id)
{
    assert(id==active); ++polls;
    if (scenario==2) return (SexosReply){SEXOS_RPC_INVALID,0};
    if (scenario==1 || polls<4) return (SexosReply){SEXOS_RPC_BUSY,0};
    return (SexosReply){0,UINT64_MAX};
}
uint64_t sexos_rpc_cancel(uint64_t id) { assert(id==active); ++cancels; return 0; }
SexosRequest sexos_rpc_listen_once(uint64_t slot) { (void)slot; abort(); }
uint64_t sexos_rpc_reply(uint32_t caller,uint64_t id,uint64_t value)
{ (void)caller; (void)id; (void)value; abort(); }
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c)
{
    assert(slot==18 && op==512 && a==524 && b==0 && c==0);
    ++legacy_calls;
    if (scenario==11 && legacy_calls<3) return (SexosReply){SEXOS_RPC_BUSY,0};
    if (scenario==12) return (SexosReply){SEXOS_RPC_INVALID,0};
    if (scenario==13) return (SexosReply){SEXOS_RPC_BUSY,0};
    return (SexosReply){0,0};
}
SexosMessage sexos_listen_once(uint64_t slot)
{
    assert(slot==0); ++listens;
    if (scenario==7) return (SexosMessage){0};
    if (scenario==8 || listens==1) return (SexosMessage){.type_id=0x500,.arg0=listens,.caller_pd=4};
    return (SexosMessage){.type_id=1,.arg0=19,.caller_pd=1};
}
void sexos_yield(void) { ++yields; }
uint64_t sexos_reply(uint32_t target,uint64_t value) { (void)target; (void)value; abort(); }
_Noreturn void sexos_fail(void) { exit(77); }

int main(int argc,char **argv)
{
    assert(argc==2); scenario=(unsigned)strtoul(argv[1],NULL,10);
    if (scenario==10) {
        RpcWaitContext invalid={.id=1,.sealed=2,.budget=1,.owned=true};
        (void)rpc_wait_run(&invalid);
        abort();
    }
    if (scenario>=11) {
        SexosRequestResult sent=sexos_send(18,512,524,0,0,4);
        if (scenario==11) assert(sent.status==SEXOS_REQUEST_COMPLETE && legacy_calls==3 && yields==2);
        else if (scenario==12) assert(sent.status==SEXOS_REQUEST_FAILED && legacy_calls==1 && yields==0);
        else assert(sent.status==SEXOS_REQUEST_TIMEOUT && legacy_calls==4 && yields==4);
        assert(registrations==0 && begins==0 && polls==0);
        puts("notification: exact accepted send and bounded queue refusal passed");
        return 0;
    }
    uint64_t budget=(scenario==1 || scenario==7) ? 2 : scenario==4 ? 3 : scenario==9 ? 0 : 16;
    SexosRequestResult result=sexos_request(18,512,524,0,0,budget);
    switch(scenario) {
    case 0: case 5:
        assert(result.status==SEXOS_REQUEST_COMPLETE && result.value==UINT64_MAX);
        assert(begins==(scenario==5?3u:1u) && polls==4 && cancels==0 && listens==0); break;
    case 1:
        assert(result.status==SEXOS_REQUEST_TIMEOUT && polls==2 && cancels==1); break;
    case 2:
        assert(result.status==SEXOS_REQUEST_FAILED && result.detail==SEXOS_RPC_INVALID && cancels==1); break;
    case 3:
        assert(result.status==SEXOS_REQUEST_FAILED && result.detail==SEXOS_RPC_BUSY && polls==0 && cancels==0); break;
    case 4:
        assert(result.status==SEXOS_REQUEST_TIMEOUT && begins==3 && polls==0 && cancels==0); break;
    case 6: {
        assert(result.status==SEXOS_REQUEST_COMPLETE && result.value==19 && legacy_calls==1 && listens==2);
        SexosMessage message;
        assert(sexos_rpc_take_deferred(&message) && message.type_id==0x500 && message.arg0==1);
        assert(!sexos_rpc_take_deferred(&message)); break;
    }
    case 7:
        assert(result.status==SEXOS_REQUEST_TIMEOUT && listens==2);
        result=sexos_request(18,512,524,0,0,16);
        assert(result.status==SEXOS_REQUEST_FAILED && legacy_calls==1 && listens==2); break;
    case 8: {
        assert(result.status==SEXOS_REQUEST_FAILED && result.detail==SEXOS_RPC_BUSY && listens==8);
        SexosMessage message;
        for(unsigned i=1;i<=8;++i) assert(sexos_rpc_take_deferred(&message) && message.arg0==i);
        assert(!sexos_rpc_take_deferred(&message)); break;
    }
    case 9:
        assert(result.status==SEXOS_REQUEST_TIMEOUT && registrations==0 && begins==0 && polls==0); break;
    default: abort();
    }
    printf("RPC scenario %u: typed result, exact receipt and bounded cleanup passed\n",scenario);
    return 0;
}
