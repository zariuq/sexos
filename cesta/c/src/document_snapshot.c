#include "sexos_document_snapshot.h"

static const uint32_t SNAPSHOT_TARGETS[16]={9,16,18,19,20,21,22,23,32,33,34,35,36,37,38,39};
static size_t snapshot_index(uint32_t subject){for(size_t i=0;i<16;i++)if(SNAPSHOT_TARGETS[i]==subject)return i;return SIZE_MAX;}
static uint32_t snapshot_region_id(size_t index){return index<3?(uint32_t)(6+index):(uint32_t)(0x2000+index-3);}
static uint64_t snapshot_load(const DocumentSnapshotRegion *region,size_t index)
{DocumentSnapshotWords *words=(DocumentSnapshotWords *)(uintptr_t)region->address;return __atomic_load_n(&words->words[index],__ATOMIC_SEQ_CST);}
static void snapshot_store(DocumentSnapshotRegion *region,size_t index,uint64_t value)
{DocumentSnapshotWords *words=(DocumentSnapshotWords *)(uintptr_t)region->address;__atomic_store_n(&words->words[index],value,__ATOMIC_SEQ_CST);}
static uint64_t snapshot_name_word(const DocumentName *name,size_t word)
{uint64_t value=0;for(size_t i=0;i<8;i++)value|=(uint64_t)name->bytes[word*8+i]<<(i*8);return value;}
static uint64_t snapshot_payload_word(const SexosDocumentPayload *payload,size_t word)
{uint64_t value=0;for(size_t i=0;i<8;i++){size_t offset=word*8+i;if(offset<payload->len)value|=(uint64_t)payload->bytes[offset]<<(i*8);}return value;}
bool document_snapshot_resolve(uint32_t subject,bool writer,DocumentSnapshotRegion *out)
{
 size_t index=snapshot_index(subject);if(index==SIZE_MAX||!out)return false;uint64_t slot=writer?64+index:64;uint64_t rights=REGION_QUERY|REGION_READ|(writer?REGION_WRITE:0);uint32_t id=snapshot_region_id(index);
 MemoryRegionReply info=sexos_memory_region_info(slot);if(info.status||info.value0!=DOCUMENT_SNAPSHOT_BYTES||info.value1!=rights||info.value2!=(writer?subject:11)||info.value3!=id)return false;
 MemoryRegionReply view=sexos_memory_region_view(slot,0,DOCUMENT_SNAPSHOT_BYTES,rights);if(view.status||!view.value0||view.value0%4096||view.value0>=REGION_USER_END||DOCUMENT_SNAPSHOT_BYTES>REGION_USER_END-view.value0||view.value1!=DOCUMENT_SNAPSHOT_BYTES||view.value2||view.value3!=id)return false;
 *out=(DocumentSnapshotRegion){view.value0,subject,writer};return true;
}
bool document_snapshot_resolve_client(DocumentSnapshotRegion *out)
{
 if(!out)return false;
 MemoryRegionReply info=sexos_memory_region_info(64);if(info.status||info.value0!=DOCUMENT_SNAPSHOT_BYTES||info.value1!=(REGION_QUERY|REGION_READ)||info.value2!=11)return false;
 size_t index=SIZE_MAX;if(info.value3>=6&&info.value3<=8)index=(size_t)(info.value3-6);else if(info.value3>=0x2000&&info.value3<0x2000+13)index=(size_t)(info.value3-0x2000+3);if(index>=16)return false;
 uint32_t subject=SNAPSHOT_TARGETS[index];MemoryRegionReply view=sexos_memory_region_view(64,0,DOCUMENT_SNAPSHOT_BYTES,REGION_QUERY|REGION_READ);if(view.status||!view.value0||view.value0%4096||view.value0>=REGION_USER_END||DOCUMENT_SNAPSHOT_BYTES>REGION_USER_END-view.value0||view.value1!=DOCUMENT_SNAPSHOT_BYTES||view.value2||view.value3!=info.value3)return false;
 *out=(DocumentSnapshotRegion){view.value0,subject,false};return true;
}
bool document_snapshot_clear(DocumentSnapshotRegion *region,uint32_t *error)
{
 if(!region||!region->address||!region->writer){if(error)*error=SEXOS_DOCUMENT_DENIED;return false;}snapshot_store(region,0,1);for(size_t i=1;i<DOCUMENT_SNAPSHOT_WORDS;i++)snapshot_store(region,i,0);snapshot_store(region,0,0);return true;
}
bool document_snapshot_publish(DocumentSnapshotRegion *region,SexosDocumentRequest request,uint32_t generation,const DocumentName *name,const SexosDocumentPayload *payload,uint32_t *error)
{
 if(!region||!region->address||!region->writer||!name||!payload||request.object<2||request.object>=15||!request.seq||request.seq>SEXOS_DOCUMENT_SEQ_MAX||payload->len>SEXOS_DOCUMENT_MAX){if(error)*error=SEXOS_DOCUMENT_DENIED;return false;}
 uint64_t token=(uint64_t)request.seq<<1,previous=snapshot_load(region,0);if(previous&1){if(error)*error=SEXOS_DOCUMENT_BUSY;return false;}if(previous&&token<=previous){if(error)*error=SEXOS_DOCUMENT_STALE;return false;}
 snapshot_store(region,0,token|1);for(size_t i=1;i<DOCUMENT_SNAPSHOT_WORDS;i++)snapshot_store(region,i,0);
 snapshot_store(region,1,DOCUMENT_SNAPSHOT_MAGIC);snapshot_store(region,2,(UINT64_C(11)<<32)|region->subject);snapshot_store(region,3,(uint64_t)generation<<32|(uint64_t)request.object<<24|request.seq);snapshot_store(region,4,payload->len);snapshot_store(region,5,sexos_document_hash(payload));
 snapshot_store(region,6,snapshot_name_word(name,0));snapshot_store(region,7,snapshot_name_word(name,1));size_t words=(payload->len+7)/8;for(size_t i=0;i<words;i++)snapshot_store(region,DOCUMENT_SNAPSHOT_DATA_WORD+i,snapshot_payload_word(payload,i));snapshot_store(region,0,token);return true;
}
bool document_snapshot_copy(const DocumentSnapshotRegion *region,uint8_t object,uint32_t generation,uint32_t sequence,size_t length,DocumentName *name,SexosDocumentPayload *payload,uint32_t *error)
{
 if(!region||!region->address||region->writer||!name||!payload||object<2||object>=15||!sequence||sequence>SEXOS_DOCUMENT_SEQ_MAX||length>SEXOS_DOCUMENT_MAX){if(error)*error=SEXOS_DOCUMENT_DENIED;return false;}
 uint64_t token=(uint64_t)sequence<<1;if(snapshot_load(region,0)!=token||snapshot_load(region,1)!=DOCUMENT_SNAPSHOT_MAGIC||snapshot_load(region,2)!=(UINT64_C(11)<<32|region->subject)||snapshot_load(region,3)!=((uint64_t)generation<<32|(uint64_t)object<<24|sequence)||snapshot_load(region,4)!=length){if(error)*error=SEXOS_DOCUMENT_STALE;goto failed;}
 uint64_t hash=snapshot_load(region,5);for(size_t word=0;word<2;word++){uint64_t value=snapshot_load(region,6+word);for(size_t i=0;i<8;i++)name->bytes[word*8+i]=(uint8_t)(value>>(i*8));}
 for(size_t i=0;i<SEXOS_DOCUMENT_MAX;i++)payload->bytes[i]=0;
 payload->len=length;for(size_t word=0;word<(length+7)/8;word++){uint64_t value=snapshot_load(region,DOCUMENT_SNAPSHOT_DATA_WORD+word);for(size_t i=0;i<8&&word*8+i<length;i++)payload->bytes[word*8+i]=(uint8_t)(value>>(i*8));}
 if(snapshot_load(region,0)!=token||sexos_document_hash(payload)!=hash){if(error)*error=SEXOS_DOCUMENT_STALE;goto failed;}return true;
failed: memset(name,0,sizeof *name);memset(payload,0,sizeof *payload);return false;
}
DocumentSnapshotCopyResult document_snapshot_copy_value(DocumentSnapshotRegion region,uint8_t object,uint32_t generation,uint32_t sequence,size_t length)
{DocumentSnapshotCopyResult result={0};result.ok=document_snapshot_copy(&region,object,generation,sequence,length,&result.name,&result.payload,&result.error);return result;}


