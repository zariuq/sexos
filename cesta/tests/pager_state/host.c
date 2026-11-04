#include "sexos_pager_state.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
static void word(uint64_t n){printf(" %"PRIu64,n);}
static void fault(PgFaultRecord r){word(r.token);word(r.target_pd);word(r.pager_pd);word(r.task_id);word(r.fault_addr);word(r.page_addr);word(r.error_code);word(r.saved_rip);word(r.saved_frame);}
static void binding(PgBinding b){word(b.token);word(b.task_id);word(b.target_pd);word(b.pager_pd);word(b.range_id);word(b.page_index);word(b.frame_index);word(b.rights);}
static PgBinding get_binding(const uint64_t *v){return (PgBinding){v[1],(uint32_t)v[2],(uint32_t)v[3],(uint32_t)v[4],(uint32_t)v[5],(size_t)v[6],(size_t)v[7],(uint8_t)v[8]};}
int main(void){
 PgFaultState f={0};PgLedger l={0};uint64_t last=0,v[24];
 while(scanf("%"SCNu64,&v[0])==1){
  for(size_t i=1;i<24;++i)if(scanf("%"SCNu64,&v[i])!=1)return 2;
  uint64_t status=0,extra=0;PgFaultResult fr={0};PgCommit cr={0};PgBindingResult br={0};PgCleanupPlan plan={0};
  PgBinding b=get_binding(v);
  switch(v[0]){
   case 0:memset(&f,0,sizeof f);memset(&l,0,sizeof l);last=v[1];break;
   case 1:extra=pager_fault_eligible(v[1],v[2],v[3]);break;
   case 2:status=pager_token_issue(&last,&extra);break;
   case 3:status=pager_fault_prepare(&f,(PgFaultRecord){v[1],(uint32_t)v[2],(uint32_t)v[3],(uint32_t)v[4],v[5],v[6],v[7],v[8],v[9]});break;
   case 4:status=pager_fault_publish(&f,v[1]);break;
   case 5:status=pager_fault_cancel(&f,v[1]);break;
   case 6:fr=pager_fault_query(&f,(uint32_t)v[1],v[2]);status=fr.status;break;
   case 7:fr=pager_fault_claim(&f,(uint32_t)v[1],v[2]);status=fr.status;break;
   case 8:status=pager_fault_rollback(&f,(uint32_t)v[1],v[2]);break;
   case 9:fr=pager_fault_complete(&f,(uint32_t)v[1],v[2]);status=fr.status;break;
   case 10:status=pager_fault_retire(&f,(uint32_t)v[1],(uint32_t)v[2],(uint32_t)v[3]);break;
   case 11:fr=pager_fault_reject(&f,(uint32_t)v[1],v[2]);status=fr.status;break;
   case 20:br=pager_mapping_validate((PgMapping){(uint32_t)v[1],(uint32_t)v[2],(uint32_t)v[3],v[4],v[5],(uint8_t)v[6],(uint8_t)v[7]},(PgFramePool){(uint32_t)v[8],v[9],v[10]},(uint32_t)v[11],(PgFaultView){(uint32_t)v[12],v[13],(uint32_t)v[14],(uint32_t)v[15],v[16],v[17]},v[18],v[19]);status=br.status;break;
   case 21:status=pager_ledger_install(&l,b,(uint8_t)v[9]);break;
   case 22:status=pager_ledger_check(&l,b,(uint8_t)v[9]);break;
   case 23:status=pager_ledger_finalize(&l,b);break;
   case 24:status=pager_ledger_retire(&l,b);break;
   case 25:cr=pager_ledger_token(&l,v[1]);break;
   case 26:extra=pager_ledger_is_retired(&l,(uint32_t)v[1],v[2]);break;
   case 27:status=pager_ledger_cleanup_plan(&l,(uint32_t)v[1],(uint32_t)v[2],(uint32_t)v[3],(uint32_t)v[4],&plan);break;
   case 28:status=pager_ledger_close(&l,(uint32_t)v[1],(uint32_t)v[2],(uint32_t)v[3],(uint32_t)v[4]);break;
   default:return 3;
  }
  printf("R");word(status);word(extra);word(fr.present);if(fr.present)fault(fr.record);
  if(v[0]==20 && !status)binding(br.value);
  if(v[0]==25){word(cr.present);if(cr.present){word(cr.finalized);binding(cr.binding);}}
  if(v[0]==27 && !status)for(size_t i=0;i<PG_PAGES;++i){word(plan.entries[i].present);if(plan.entries[i].present)binding(plan.entries[i].value);}
  printf(" F");word(last);word(f.phase);if(f.phase!=PG_IDLE)fault(f.active);word(f.has_completed);if(f.has_completed)fault(f.completed);word(f.last_token);word(f.retired);
  printf(" L");for(size_t i=0;i<PG_PAGES;++i){word(l.entries[i].present);if(l.entries[i].present){word(l.entries[i].finalized);binding(l.entries[i].binding);}}
  for(size_t i=0;i<PG_PAGES;++i){word(l.retired[i].present);if(l.retired[i].present)binding(l.retired[i].value);}
  for(size_t i=0;i<PG_CLIENTS;++i){word(l.retired_highwater[i]);word(l.closed[i].present);if(l.closed[i].present){word(l.closed[i].target_pd);word(l.closed[i].task_id);word(l.closed[i].frames);}}
  putchar('\n');
 }
 return 0;
}
