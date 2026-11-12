(* A matching live entry changes only its phase byte, then returns through
   the actual ABI. Code and the return slot must exclude that byte. *)

let frame_lease_hit_statement mc gcc =
  let body=vsubst [lhs(concl mc),`detach_mc:byte list`;
    mk_small_numeral(if gcc then 96 else 124),`loop_offset:num`;
    frame_lease_loop_regs gcc,`loop_regs:num->x86state->bool`]
   `i < 4096 /\ frame_lease_matches_at table base holder incarnation ordr kind ram i /\
    nonoverlapping (word pc,LENGTH detach_mc) (table,131072) /\
    nonoverlapping (stack,8) (table,131072)
    ==> ensures x86
      (\s. bytes_loaded s (word pc) detach_mc /\ read RIP s = word(pc + loop_offset) /\
           loop_regs i s /\ read RSP s = stack /\ read memory s = ram /\
           read(memory :> bytes64 stack) s = returnaddress /\
           frame_lease_trace e table stack [(word_add table (word(32*i+25)),1)] s)
      (\s. read RIP s = returnaddress /\ read RSP s = word_add stack (word 8) /\
           read RAX s = word 0 /\
           read memory s = write (bytes8(word_add table (word(32*i+25)))) (word 2) ram /\
           frame_lease_trace e table stack [(word_add table (word(32*i+25)),1)] s)
      (MAYCHANGE [RIP; RSP; RAX; RDI; RSI; RDX; RCX; R8; R9; R10; R11] ,,
       MAYCHANGE SOME_FLAGS ,, MAYCHANGE [events] ,,
       MAYCHANGE [memory :> bytes(word_add table (word(32*i+25)),1)])` in
  list_mk_forall ([`table:int64`;`stack:int64`;`base:int64`;`holder:int32`;
    `incarnation:int32`;`ordr:byte`;`kind:byte`;`ram:int64->byte`;
    `i:num`;`pc:num`;`returnaddress:int64`;`e:uarch_event list`],body);;

let frame_lease_hit_prepare mc exec gcc =
  REWRITE_TAC[frame_lease_matches_at; frame_lease_match; frame_lease_trace;
              NONOVERLAPPING_CLAUSES; SOME_FLAGS; fst exec] THEN BETA_TAC THEN
  REPEAT GEN_TAC THEN STRIP_TAC THEN VAL_INT64_TAC `i:num` THEN
  VAL_INT64_TAC `32*i` THEN ENSURES_INIT_TAC "s0" THEN
  TRY STRIP_EXISTS_ASSUM_TAC THEN frame_lease_read_slot;;

let frame_lease_hit_rule mc exec gcc = prove(frame_lease_hit_statement mc gcc,
  frame_lease_hit_prepare mc exec gcc THEN
  frame_lease_steps exec (1--(if gcc then 16 else 15)) THEN
  ENSURES_FINAL_STATE_TAC THEN ASM_REWRITE_TAC[] THEN
  DISCHARGE_MEMSAFE_ASM_TAC);;
