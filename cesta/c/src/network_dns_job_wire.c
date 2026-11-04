#include "network_dns_job_machine.h"

bool network_dns_job_wire_valid(NetworkDnsJobWire wire)
{return wire.begin&&wire.receive&&wire.abort;}
NetworkDnsJobWireResult network_dns_job_wire_begin(NetworkDnsJobWire wire,uint32_t ticket)
{uint32_t error=NETWORK_JOB_OFFLINE;if(!network_dns_job_wire_valid(wire))return(NetworkDnsJobWireResult){false,false,0,error};bool ok=wire.begin(wire.context,ticket,&error);return(NetworkDnsJobWireResult){ok,false,0,error};}
NetworkDnsJobWireResult network_dns_job_wire_receive(NetworkDnsJobWire wire,NetworkDnsJobWriteView out,size_t offset,size_t length)
{uint32_t error=NETWORK_JOB_OFFLINE;bool present=false;size_t count=0;if(!network_dns_job_wire_valid(wire)||offset>out.len||length>out.len-offset||length>NETWORK_DNS_MAX_BYTES)return(NetworkDnsJobWireResult){false,false,0,error};bool ok=wire.receive(wire.context,out.data+offset,length,&present,&count,&error);return(NetworkDnsJobWireResult){ok,present,count,error};}
void network_dns_job_wire_abort(NetworkDnsJobWire wire){if(network_dns_job_wire_valid(wire))wire.abort(wire.context);}


