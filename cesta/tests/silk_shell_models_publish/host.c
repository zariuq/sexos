#include "silk_shell_models_linen.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static const char seed0[]="seed-zero";
static const char seed1[]="seed-one";
const LinenObject LINEN_SEED_OBJECTS[6]={
 {.present=true,.object_id=100,.kind=LINEN_OBJ_KIND_DOCUMENT,.state=LINEN_OBJ_STATE_SAVED,.display_name=seed0},
 {.present=true,.object_id=101,.kind=LINEN_OBJ_KIND_PROJECT,.state=LINEN_OBJ_STATE_SAVED,.display_name=seed1},
};

static unsigned label(const char *value)
{
 if(!value)return 0;
 if(value==seed0)return 2;
 if(value==seed1)return 3;
 if(!strcmp(value,"[linen.remote]"))return 1;
 return 255;
}

int main(void)
{
 SilkShellModelsState models={0};ShellLinenRuntime linen={0};
 unsigned legacy;uint64_t revision,selected;size_t declared;
 while(scanf("%u %"SCNu64" %"SCNu64" %zu",&legacy,&revision,&selected,&declared)==4){
  linen.snapshot=(ShellLinenSnapshot){0};linen.publication_revision=revision;
  models.SELECTED_LINEN_OBJECT_ID=selected;linen.snapshot.count=declared;
  for(size_t i=0;i<SHELL_LINEN_CAPACITY;++i){
   unsigned present,kind,disk,length,byte;
   assert(scanf("%u %"SCNu64" %u %u %u",&present,&linen.snapshot.objects[i].id,&kind,&disk,&length)==5);
   linen.snapshot.present[i]=present!=0;linen.snapshot.objects[i].kind=(uint8_t)kind;
   linen.snapshot.objects[i].disk=(uint8_t)disk;linen.snapshot.objects[i].len=(uint8_t)length;
   for(size_t j=0;j<24;++j){assert(scanf("%u",&byte)==1);linen.snapshot.objects[i].name[j]=(uint8_t)byte;}
  }
  bool ok=shell_models_publish_snapshot_checked(&models,&linen,&linen.snapshot,&LINEN_SEED_OBJECTS,legacy!=0);
  printf("%u %"PRIu64" %"PRIu64" %u %u",(unsigned)ok,linen.publication_revision,
   models.SELECTED_LINEN_OBJECT_ID,(unsigned)models.LINEN_REMOTE_FETCHED,(unsigned)models.LINEN_REMOTE_REAL);
  for(size_t i=0;i<LINEN_MAX_OBJECTS;++i){
   const LinenObject *row=&models.LINEN_OBJECTS[i];
   printf("|%u,%"PRIu64",%u,%u,%u,%u,%u,",(unsigned)row->present,row->object_id,
    (unsigned)row->kind,(unsigned)row->state,label(row->display_name),(unsigned)row->name_len,(unsigned)row->disk_path);
   for(size_t j=0;j<24;++j)printf("%02x",row->name[j]);
  }
  putchar('\n');
 }
}
