(* At the last slot, a mismatch exhausts the table and returns DENIED without
   writing any memory. The guard cases are shared with the advancing proof. *)

let frame_lease_exhaust_statement mc gcc =
  let body=vsubst [lhs(concl mc),`detach_mc:byte list`;
    mk_small_numeral(if gcc then 96 else 124),`loop_offset:num`;
    frame_lease_loop_regs gcc,`loop_regs:num->x86state->bool`]
   `~frame_lease_matches_at table base holder incarnation ordr kind ram 4095
    ==> ensures x86
      (\s. bytes_loaded s (word pc) detach_mc /\ read RIP s = word(pc + loop_offset) /\
           loop_regs 4095 s /\ read RSP s = stack /\ read memory s = ram /\
           read(memory :> bytes64 stack) s = returnaddress /\
           frame_lease_trace e table stack [] s)
      (\s. read RIP s = returnaddress /\ read RSP s = word_add stack (word 8) /\
           read RAX s = word 3 /\ read memory s = ram /\
           frame_lease_trace e table stack [] s)
      (MAYCHANGE [RIP; RSP; RAX; RDI; RSI; RDX; RCX; R8; R9; R10; R11] ,,
       MAYCHANGE SOME_FLAGS ,, MAYCHANGE [events])` in
  list_mk_forall ([`table:int64`;`stack:int64`;`base:int64`;`holder:int32`;
    `incarnation:int32`;`ordr:byte`;`kind:byte`;`ram:int64->byte`;
    `pc:num`;`returnaddress:int64`;`e:uarch_event list`],body);;

let frame_lease_exhaust_rule mc exec gcc = prove(frame_lease_exhaust_statement mc gcc,
  REWRITE_TAC[frame_lease_trace; SOME_FLAGS; fst exec] THEN BETA_TAC THEN
  REPEAT GEN_TAC THEN DISCH_TAC THEN ENSURES_INIT_TAC "s0" THEN
  TRY STRIP_EXISTS_ASSUM_TAC THEN frame_lease_read_slot_at `4095` THEN
  frame_lease_guard_cases exec `4095`
    (fun step -> frame_lease_steps exec (step--(step+(if gcc then 5 else 4))) THEN
     ENSURES_FINAL_STATE_TAC THEN ASM_REWRITE_TAC[] THEN DISCHARGE_MEMSAFE_ASM_TAC));;
