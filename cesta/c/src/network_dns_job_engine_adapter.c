#include "network_dns_job_engine.h"

static uint32_t dns_engine_error(NetworkEngineError error)
{return error==NETWORK_ENGINE_STOPPED?NETWORK_JOB_OFFLINE:NETWORK_JOB_IO;}
static bool dns_engine_begin(void *opaque,uint32_t ticket,uint32_t *error)
{NetworkDnsJobEngineContext *context=opaque;if(!context||!context->engine){*error=NETWORK_JOB_OFFLINE;return false;}NetworkEngineError result=network_engine_transport_begin_dns(context->engine,ticket);*error=dns_engine_error(result);return result==NETWORK_ENGINE_OK;}
static bool dns_engine_receive(void *opaque,uint8_t *out,size_t capacity,bool *present,size_t *count,uint32_t *error)
{
 NetworkDnsJobEngineContext *context=opaque;if(!context||!context->engine||!out||capacity<NETWORK_DNS_MAX_BYTES){*error=NETWORK_JOB_IO;return false;}NetworkEngineDnsResult result=network_engine_transport_dns_receive(context->engine);if(!result.ok){*error=dns_engine_error(result.error);return false;}
 *present=result.present;*count=result.present?result.frame.len:0;if(*count>capacity){*error=NETWORK_JOB_IO;return false;}for(size_t i=0;i<*count;i++)out[i]=result.frame.bytes[i];*error=0;return true;
}
static void dns_engine_abort(void *opaque){NetworkDnsJobEngineContext *context=opaque;if(context&&context->engine)network_engine_transport_dns_abort(context->engine);}
NetworkDnsJobWire network_dns_job_engine_wire(NetworkDnsJobEngineContext *context)
{return(NetworkDnsJobWire){context,dns_engine_begin,dns_engine_receive,dns_engine_abort};}


