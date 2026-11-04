#include "sexos_document_client_transport.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");

void document_client_transport_init(DocumentClientTransport *state){memset(state,0,sizeof *state);}
static size_t document_transport_free(const DocumentClientTransport *state)
{for(size_t i=0;i<DOCUMENT_CLIENT_TRANSPORT_SLOTS;i++)if(!state->entries[i].present)return i;return SIZE_MAX;}
bool document_client_transport_step(DocumentClient *client,DocumentClientTransport *transport,DocumentClientEvent *event)
{
 if(event)*event=(DocumentClientEvent){0};
 size_t pending=0;
 for(size_t i=0;i<DOCUMENT_CLIENT_TRANSPORT_SLOTS;i++){DocumentClientTransportEntry entry=transport->entries[i];if(!entry.present)continue;SexosReply reply=sexos_rpc_poll(entry.request_id);if(!reply.status){memset(&transport->entries[i],0,sizeof transport->entries[i]);(void)document_client_reply(client,SEXOS_DOCUMENT_SERVER_PD,reply.value,event);if(event&&event->present)return true;}else if(reply.status!=SEXOS_RPC_BUSY){memset(&transport->entries[i],0,sizeof transport->entries[i]);(void)document_client_transport_lost(client,entry.sequence,SEXOS_DOCUMENT_DENIED,event);if(event&&event->present)return true;}}
 for(size_t i=0;i<DOCUMENT_CLIENT_TRANSPORT_SLOTS;i++)if(transport->entries[i].present)pending++;
 if(pending&&(client->phase!=DOCUMENT_PHASE_READ&&client->phase!=DOCUMENT_PHASE_WRITE))return false;
 if(pending>=SEXOS_DOCUMENT_WINDOW)return false;
 DocumentClientAction action=document_client_action(client,event);if(event&&event->present)return true;if(!action.present)return false;size_t slot=document_transport_free(transport);if(slot==SIZE_MAX)return false;
 SexosReply admitted=sexos_rpc_begin(SEXOS_SLOT_storage,SEXOS_DOCUMENT_OP,sexos_document_header(action.request),action.request.a,action.request.b);
 if(!admitted.status&&admitted.value){transport->entries[slot]=(DocumentClientTransportEntry){true,action.request.seq,admitted.value};(void)document_client_sent(client,action.request.seq,true,false,event);}
 else (void)document_client_sent(client,action.request.seq,false,admitted.status==SEXOS_RPC_BUSY||admitted.status==SEXOS_RPC_UNSUPPORTED,event);
 return event&&event->present;
}
bool document_client_transport_pending(const DocumentClientTransport *transport)
{for(size_t i=0;i<DOCUMENT_CLIENT_TRANSPORT_SLOTS;i++)if(transport->entries[i].present)return true;return false;}
void document_client_transport_close(DocumentClient *client,DocumentClientTransport *transport)
{for(size_t i=0;i<DOCUMENT_CLIENT_TRANSPORT_SLOTS;i++)if(transport->entries[i].present)(void)sexos_rpc_cancel(transport->entries[i].request_id);memset(transport,0,sizeof *transport);document_client_close(client);}


