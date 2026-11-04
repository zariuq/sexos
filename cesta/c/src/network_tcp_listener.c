#include "network_tcp_listener.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

#define LISTENER_HASH_BASIS UINT64_C(0xcbf29ce484222325)
static bool listener_mac_valid(NetworkEngineMac mac){bool any=false;for(size_t i=0;i<6;i++)if(mac.bytes[i])any=true;return any&&(mac.bytes[0]&1)==0;}
static uint64_t listener_hash_payload(uint64_t value,const NetworkEnginePayload *payload,size_t count){for(size_t i=0;i<count;i++)value=(value^payload->bytes[i])*UINT64_C(0x100000001b3);return value;}
static uint64_t listener_hash_frame(uint64_t value,const NetworkEngineFrame *frame,size_t offset,size_t count){for(size_t i=0;i<count;i++)value=(value^frame->bytes[offset+i])*UINT64_C(0x100000001b3);return value;}
static void listener_frame_clear(NetworkEngineFrame *frame){for(size_t i=0;i<NETWORK_ENGINE_MAX_FRAME;i++)frame->bytes[i]=0;frame->length=0;}
static void listener_rearm(NetworkTcpListener *l)
{
 l->state=NETWORK_LISTENER_LISTEN;l->peer_mac=(NetworkEngineMac){{0}};l->peer_ip=0;l->peer_port=0;l->snd_una=0;l->snd_nxt=0;l->rcv_nxt=0;l->receive_len=0;l->pending_present=false;l->pending_sent=false;l->pending_fin=false;l->immediate_present=false;l->close_requested=false;l->pending_end=0;l->pending_payload=0;l->retries=0;l->retransmit_at=0;
 for(size_t i=0;i<NETWORK_TCP_LISTENER_BUFFER;i++){l->receive[i]=0;}
 listener_frame_clear(&l->pending);listener_frame_clear(&l->immediate);
}
bool network_tcp_listener_init(NetworkTcpListener *l,NetConfig config,NetworkEngineMac local_mac,uint64_t seed,uint64_t now)
{
 if(!net_config_valid(config)||!listener_mac_valid(local_mac)||now>(uint64_t)INT64_MAX)return false;
 NetworkTcpListener next={0};next.config=config;next.local_mac=local_mac;next.seed=seed;next.last=now;next.stats.receive_hash=LISTENER_HASH_BASIS;next.stats.queue_hash=LISTENER_HASH_BASIS;next.stats.flow_receive_hash=LISTENER_HASH_BASIS;next.stats.flow_queue_hash=LISTENER_HASH_BASIS;next.state=NETWORK_LISTENER_LISTEN;*l=next;return true;
}
static NetworkTcpListenerOutput listener_build(const NetworkTcpListener *l,uint32_t seq,uint8_t flags,const NetworkEnginePayload *payload)
{NetworkEngineFrame frame={0};if(!network_engine_tcp_build(&frame,l->peer_mac,l->local_mac,l->config.local,l->peer_ip,l->config.listen_port,l->peer_port,seq,l->rcv_nxt,flags,8192,payload))return(NetworkTcpListenerOutput){0};return(NetworkTcpListenerOutput){true,frame};}
static bool listener_pending(NetworkTcpListener *l,uint8_t flags,size_t count,bool fin)
{
 NetworkEnginePayload payload={0};payload.len=count;for(size_t i=0;i<count;i++)payload.bytes[i]=l->receive[i];NetworkTcpListenerOutput built=listener_build(l,l->snd_nxt,flags,&payload);if(!built.present)return false;l->pending=built.frame;l->pending_present=true;l->pending_sent=false;l->pending_fin=fin;l->pending_payload=count;l->pending_end=l->snd_nxt+(uint32_t)count+(fin?1u:0u)+(flags&NETWORK_TCP_SYN?1u:0u);l->snd_nxt=l->pending_end;l->retries=0;l->retransmit_at=l->last;
 if(count){l->stats.queued+=count;l->stats.flow_queued+=count;l->stats.queue_hash=listener_hash_payload(l->stats.queue_hash,&payload,count);l->stats.flow_queue_hash=listener_hash_payload(l->stats.flow_queue_hash,&payload,count);}return true;
}
static bool listener_ack(NetworkTcpListener *l){NetworkEnginePayload empty={0};NetworkTcpListenerOutput built=listener_build(l,l->snd_nxt,NETWORK_TCP_ACK,&empty);if(!built.present)return false;l->immediate=built.frame;l->immediate_present=true;return true;}
static void listener_shift(NetworkTcpListener *l,size_t count){for(size_t i=count;i<l->receive_len;i++)l->receive[i-count]=l->receive[i];for(size_t i=l->receive_len-count;i<l->receive_len;i++)l->receive[i]=0;l->receive_len-=count;}
static bool listener_tuple(const NetworkTcpListener *l,NetworkIpv4Packet ip,NetworkTcpPacket tcp){return ip.source==l->peer_ip&&ip.destination==l->config.local&&tcp.source_port==l->peer_port&&tcp.destination_port==l->config.listen_port;}
static void listener_fail(NetworkTcpListener *l){l->state=NETWORK_LISTENER_FAILED;l->pending_present=false;l->immediate_present=false;l->receive_len=0;for(size_t i=0;i<NETWORK_TCP_LISTENER_BUFFER;i++)l->receive[i]=0;listener_frame_clear(&l->pending);listener_frame_clear(&l->immediate);}
bool network_tcp_listener_ingest(NetworkTcpListener *l,const NetworkEngineFrame *frame,uint64_t now)
{
 if(now<l->last||now>(uint64_t)INT64_MAX){listener_fail(l);return false;}l->last=now;NetworkIpv4Packet ip=network_engine_ipv4_decode(frame);NetworkTcpPacket tcp=network_engine_tcp_decode(frame);if(!ip.valid||!tcp.valid)return false;
 if(l->state==NETWORK_LISTENER_LISTEN){
  if(ip.destination!=l->config.local||tcp.destination_port!=l->config.listen_port||!net_config_contains(l->config,ip.source)||(tcp.flags&NETWORK_TCP_SYN)==0||(tcp.flags&(NETWORK_TCP_ACK|NETWORK_TCP_RST)))return false;
  NetworkEngineMac peer={{0}};for(size_t i=0;i<6;i++)peer.bytes[i]=frame->bytes[6+i];if(!listener_mac_valid(peer))return false;l->peer_mac=peer;l->peer_ip=ip.source;l->peer_port=tcp.source_port;l->rcv_nxt=tcp.sequence+1;l->stats.flow++;l->stats.flow_received=0;l->stats.flow_queued=0;l->stats.flow_receive_hash=LISTENER_HASH_BASIS;l->stats.flow_queue_hash=LISTENER_HASH_BASIS;
  uint32_t initial=(uint32_t)(l->seed^(l->seed>>32)^l->stats.flow*UINT64_C(0x9e3779b9));if(!initial)initial=1;l->snd_una=initial;l->snd_nxt=initial;l->state=NETWORK_LISTENER_SYN_RECEIVED;if(!listener_pending(l,NETWORK_TCP_SYN|NETWORK_TCP_ACK,0,false)){listener_fail(l);return false;}return true;
 }
 if(!listener_tuple(l,ip,tcp))return false;
 if(tcp.flags&NETWORK_TCP_RST){listener_rearm(l);return true;}
 if(l->state==NETWORK_LISTENER_SYN_RECEIVED){if((tcp.flags&NETWORK_TCP_ACK)&&tcp.acknowledgment==l->pending_end){l->snd_una=tcp.acknowledgment;l->pending_present=false;l->pending_sent=false;l->state=NETWORK_LISTENER_ESTABLISHED;listener_frame_clear(&l->pending);return true;}return false;}
 if((tcp.flags&NETWORK_TCP_ACK)&&l->pending_present&&tcp.acknowledgment==l->pending_end){size_t count=l->pending_payload;bool fin=l->pending_fin;l->snd_una=tcp.acknowledgment;l->pending_present=false;l->pending_sent=false;l->pending_fin=false;l->pending_payload=0;l->retries=0;listener_frame_clear(&l->pending);if(count)listener_shift(l,count);if(fin){listener_rearm(l);return true;}}
 if(tcp.payload_length){if(tcp.sequence==l->rcv_nxt){if(tcp.payload_length>NETWORK_TCP_LISTENER_BUFFER-l->receive_len){listener_fail(l);return false;}for(size_t i=0;i<tcp.payload_length;i++)l->receive[l->receive_len+i]=frame->bytes[tcp.payload_offset+i];l->receive_len+=tcp.payload_length;l->rcv_nxt+=tcp.payload_length;l->stats.received+=tcp.payload_length;l->stats.flow_received+=tcp.payload_length;l->stats.receive_hash=listener_hash_frame(l->stats.receive_hash,frame,tcp.payload_offset,tcp.payload_length);l->stats.flow_receive_hash=listener_hash_frame(l->stats.flow_receive_hash,frame,tcp.payload_offset,tcp.payload_length);}if(!listener_ack(l)){listener_fail(l);return false;}}
 if(tcp.flags&NETWORK_TCP_FIN){uint32_t at=tcp.sequence+(uint32_t)tcp.payload_length;if(at==l->rcv_nxt){l->rcv_nxt++;l->close_requested=true;}if(!listener_ack(l)){listener_fail(l);return false;}}
 return true;
}
static NetworkTcpListenerOutput listener_emit(NetworkEngineFrame frame){return(NetworkTcpListenerOutput){true,frame};}
NetworkTcpListenerOutput network_tcp_listener_advance(NetworkTcpListener *l,uint64_t now)
{
 if(l->state==NETWORK_LISTENER_FAILED)return(NetworkTcpListenerOutput){0};
 if(now<l->last||now>(uint64_t)INT64_MAX){listener_fail(l);return(NetworkTcpListenerOutput){0};}
 l->last=now;
 if(l->immediate_present){NetworkEngineFrame frame=l->immediate;l->immediate_present=false;listener_frame_clear(&l->immediate);return listener_emit(frame);}
 if(l->pending_present){if(!l->pending_sent||now>=l->retransmit_at){if(l->pending_sent&&l->retries>=NETWORK_TCP_LISTENER_RETRY_LIMIT){listener_fail(l);return(NetworkTcpListenerOutput){0};}l->pending_sent=true;l->retries++;l->retransmit_at=UINT64_MAX-now<NETWORK_TCP_LISTENER_RETRY_TICKS?UINT64_MAX:now+NETWORK_TCP_LISTENER_RETRY_TICKS;return listener_emit(l->pending);}return(NetworkTcpListenerOutput){0};}
 if(l->state==NETWORK_LISTENER_ESTABLISHED&&l->receive_len){size_t count=l->receive_len<NETWORK_TCP_LISTENER_TURN?l->receive_len:NETWORK_TCP_LISTENER_TURN;if(!listener_pending(l,NETWORK_TCP_PSH|NETWORK_TCP_ACK,count,false)){listener_fail(l);return(NetworkTcpListenerOutput){0};}return network_tcp_listener_advance(l,now);}
 if(l->state==NETWORK_LISTENER_ESTABLISHED&&l->close_requested){if(!listener_pending(l,NETWORK_TCP_FIN|NETWORK_TCP_ACK,0,true)){listener_fail(l);return(NetworkTcpListenerOutput){0};}l->state=NETWORK_LISTENER_LAST_ACK;return network_tcp_listener_advance(l,now);}
 return(NetworkTcpListenerOutput){0};
}
NetworkTcpListenerState network_tcp_listener_state(const NetworkTcpListener *l){return l->state;}
NetworkTcpListenerStats network_tcp_listener_statistics(const NetworkTcpListener *l){return l->stats;}
NetworkTcpListenerSnapshot network_tcp_listener_snapshot(const NetworkTcpListener *l){return(NetworkTcpListenerSnapshot){l->state,l->snd_una,l->snd_nxt,l->rcv_nxt,l->receive_len,l->pending_present,l->immediate_present,l->close_requested,l->retries,l->last,l->retransmit_at};}


