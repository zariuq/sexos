#include "network_job_machine.h"

bool network_job_wire_valid(NetworkJobWire wire)
{return wire.begin&&wire.receive&&wire.send&&wire.queued&&wire.link&&wire.finish_send&&wire.abort;}
NetworkJobWireResult network_job_wire_begin(NetworkJobWire wire,uint32_t ticket)
{uint32_t error=NETWORK_JOB_IO;bool ok=wire.begin(wire.context,ticket,&error);return(NetworkJobWireResult){ok,0,error};}
NetworkJobWireResult network_job_wire_receive(NetworkJobWire wire,NetworkJobWriteView out,size_t offset,size_t length)
{if(offset>out.len||length>out.len-offset)return(NetworkJobWireResult){false,0,NETWORK_JOB_INVALID};size_t count=0;uint32_t error=NETWORK_JOB_IO;bool ok=wire.receive(wire.context,out.data+offset,length,&count,&error);return(NetworkJobWireResult){ok,count,error};}
NetworkJobWireResult network_job_wire_send(NetworkJobWire wire,NetworkJobReadView data,size_t offset,size_t length)
{if(offset>data.len||length>data.len-offset)return(NetworkJobWireResult){false,0,NETWORK_JOB_INVALID};size_t count=0;uint32_t error=NETWORK_JOB_IO;bool ok=wire.send(wire.context,data.data+offset,length,&count,&error);return(NetworkJobWireResult){ok,count,error};}
size_t network_job_wire_queued(NetworkJobWire wire){return wire.queued(wire.context);}
NetworkJobLink network_job_wire_link(NetworkJobWire wire){return wire.link(wire.context);}
void network_job_wire_finish_send(NetworkJobWire wire){wire.finish_send(wire.context);}
void network_job_wire_abort(NetworkJobWire wire){wire.abort(wire.context);}


