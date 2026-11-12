(* The scan advances only past entries that do not match the independent
   allocation-identity predicate. No table write occurs on these paths. *)

let frame_lease_loop_regs gcc = if gcc then
 `\i s. read R9 s = table /\
    read RAX s = word_add table (word(32*i)) /\ read RDX s = word i /\
    read RCX s = base /\ read RSI s = word_zx (holder:int32) /\
    read RDI s = word_zx (incarnation:int32) /\ read R8 s = word_zx (kind:byte) /\
    read R10 s = word_zx (ordr:byte)`
 else
 `\i s. read RDI s = table /\ read RAX s = word(32*i) /\ read R8 s = base /\
    read RDX s = word_zx (holder:int32) /\ read RSI s = word_zx (incarnation:int32) /\
    read RCX s = word_zx (kind:byte) /\ read R9 s = word_zx (ordr:byte)`;;

let frame_lease_miss_statement mc gcc =
  let body=vsubst [lhs(concl mc),`detach_mc:byte list`;
    mk_small_numeral(if gcc then 96 else 124),`loop_offset:num`;
    frame_lease_loop_regs gcc,`loop_regs:num->x86state->bool`]
   `i < 4095 /\ ~frame_lease_matches_at table base holder incarnation ordr kind ram i
    ==> ensures x86
      (\s. bytes_loaded s (word pc) detach_mc /\ read RIP s = word(pc + loop_offset) /\
           loop_regs i s /\ read RSP s = stack /\ read memory s = ram /\
           read(memory :> bytes64 stack) s = returnaddress /\
           frame_lease_trace e table stack [] s)
      (\s. bytes_loaded s (word pc) detach_mc /\
           read RIP s = word(pc + loop_offset) /\ loop_regs (i+1) s /\
           read RSP s = stack /\ read memory s = ram /\
           read(memory :> bytes64 stack) s = returnaddress /\
           frame_lease_trace e table stack [] s)
      (MAYCHANGE [RIP; RAX; RDI; RSI; RDX; RCX; R8; R9; R10; R11] ,,
       MAYCHANGE SOME_FLAGS ,, MAYCHANGE [events])` in
  list_mk_forall ([`table:int64`;`stack:int64`;`base:int64`;`holder:int32`;
    `incarnation:int32`;`ordr:byte`;`kind:byte`;`ram:int64->byte`;
    `i:num`;`pc:num`;`returnaddress:int64`;`e:uarch_event list`],body);;

let frame_lease_read_slot_at slot : tactic =
  MAP_EVERY (fun fact -> SUBGOAL_THEN fact ASSUME_TAC THENL
    [ASM_REWRITE_TAC[READ_COMPONENT_COMPOSE]; ALL_TAC])
   (map (vsubst[slot,`i:num`])
   [`read(memory :> bytes64(word_add table (word(32*i)))) s0 =
       read(bytes64(word_add table (word(32*i)))) (ram:int64->byte)`;
    `read(memory :> bytes32(word_add table (word(32*i+16)))) s0 =
       read(bytes32(word_add table (word(32*i+16)))) (ram:int64->byte)`;
    `read(memory :> bytes32(word_add table (word(32*i+20)))) s0 =
       read(bytes32(word_add table (word(32*i+20)))) (ram:int64->byte)`;
    `read(memory :> bytes8(word_add table (word(32*i+24)))) s0 =
       read(bytes8(word_add table (word(32*i+24)))) (ram:int64->byte)`;
    `read(memory :> bytes8(word_add table (word(32*i+25)))) s0 =
       read(bytes8(word_add table (word(32*i+25)))) (ram:int64->byte)`;
    `read(memory :> bytes8(word_add table (word(32*i+26)))) s0 =
       read(bytes8(word_add table (word(32*i+26)))) (ram:int64->byte)`]);;

let frame_lease_read_slot = frame_lease_read_slot_at `i:num`;;

let frame_lease_miss_finish exec gcc step =
  frame_lease_steps exec (step--(step+(if gcc then 3 else 2))) THEN
  ENSURES_FINAL_STATE_TAC THEN
  ASM_REWRITE_TAC[ARITH_RULE `32*(i+1) = 32*i+32`; GSYM WORD_ADD_ASSOC; GSYM WORD_ADD] THEN
  DISCHARGE_MEMSAFE_ASM_TAC;;

let frame_lease_guard_cases exec slot finish =
  let checks = map (vsubst[slot,`i:num`])
   [`read(bytes8(word_add table (word(32*i+25)))) (ram:int64->byte) = word 1`;
    `read(bytes64(word_add table (word(32*i)))) (ram:int64->byte) = base`;
    `read(bytes8(word_add table (word(32*i+24)))) (ram:int64->byte) = (ordr:byte)`;
    `read(bytes8(word_add table (word(32*i+26)))) (ram:int64->byte) = (kind:byte)`;
    `(holder:int32) = read(bytes32(word_add table (word(32*i+16)))) (ram:int64->byte)`;
    `(incarnation:int32) = read(bytes32(word_add table (word(32*i+20)))) (ram:int64->byte)`] in
  let rec walk step = function
    | [] ->
        UNDISCH_TAC(vsubst[slot,`i:num`]
          `~frame_lease_matches_at table base holder incarnation ordr kind (ram:int64->byte) i`) THEN
        REWRITE_TAC[frame_lease_matches_at; frame_lease_match] THEN
        CONV_TAC NUM_REDUCE_CONV THEN ASM_REWRITE_TAC[EQ_SYM_EQ]
    | check::rest -> ASM_CASES_TAC check THENL
        [frame_lease_steps exec [step;step+1] THEN walk (step+2) rest;
         frame_lease_steps exec [step;step+1] THEN
         finish (step+2)] in
  walk 1 checks;;

let frame_lease_miss_cases exec gcc =
  frame_lease_guard_cases exec `i:num` (frame_lease_miss_finish exec gcc);;

let frame_lease_miss_prepare mc exec gcc =
  REWRITE_TAC[frame_lease_trace; SOME_FLAGS; fst exec] THEN BETA_TAC THEN
  REPEAT GEN_TAC THEN STRIP_TAC THEN
  VAL_INT64_TAC `i:num` THEN VAL_INT64_TAC `i + 1` THEN
  VAL_INT64_TAC `32 * i` THEN VAL_INT64_TAC `32 * i + 32` THEN
  (SUBGOAL_THEN `~(word(i+1):int64 = word 4096) /\
                ~(word(32*i+32):int64 = word 131072)` STRIP_ASSUME_TAC THENL
   [REWRITE_TAC[GSYM VAL_EQ] THEN ASM_REWRITE_TAC[] THEN
    CONV_TAC WORD_REDUCE_CONV THEN ASM_ARITH_TAC; ALL_TAC]) THEN
  ENSURES_INIT_TAC "s0" THEN TRY STRIP_EXISTS_ASSUM_TAC THEN
  frame_lease_read_slot;;

let frame_lease_miss_rule mc exec gcc = prove(frame_lease_miss_statement mc gcc,
  frame_lease_miss_prepare mc exec gcc THEN frame_lease_miss_cases exec gcc);;

let frame_lease_prefix_statement mc gcc =
  let body=vsubst [lhs(concl mc),`detach_mc:byte list`;
    mk_small_numeral(if gcc then 96 else 124),`loop_offset:num`;
    frame_lease_loop_regs gcc,`loop_regs:num->x86state->bool`]
   `n < 4096 /\
    (!i. i < n ==> ~frame_lease_matches_at table base holder incarnation ordr kind ram i)
    ==> ensures x86
      (\s. bytes_loaded s (word pc) detach_mc /\ read RIP s = word(pc + loop_offset) /\
           loop_regs 0 s /\ read RSP s = stack /\ read memory s = ram /\
           read(memory :> bytes64 stack) s = returnaddress /\
           frame_lease_trace e table stack [] s)
      (\s. read RIP s = word(pc + loop_offset) /\ loop_regs n s /\
           read RSP s = stack /\ read memory s = ram /\
           read(memory :> bytes64 stack) s = returnaddress /\
           frame_lease_trace e table stack [] s)
      (MAYCHANGE [RIP; RAX; RDI; RSI; RDX; RCX; R8; R9; R10; R11] ,,
       MAYCHANGE SOME_FLAGS ,, MAYCHANGE [events])` in
  list_mk_forall ([`table:int64`;`stack:int64`;`base:int64`;`holder:int32`;
    `incarnation:int32`;`ordr:byte`;`kind:byte`;`ram:int64->byte`;
    `n:num`;`pc:num`;`returnaddress:int64`;`e:uarch_event list`],body);;

let frame_lease_prefix_rule mc exec gcc miss =
  let loop=mk_comb(`(+)(pc:num)`,mk_small_numeral(if gcc then 96 else 124)) in
  let invariant=vsubst[frame_lease_loop_regs gcc,`loop_regs:num->x86state->bool`]
   `\(i:num) s. loop_regs i s /\ read RSP s = stack /\ read memory s = ram /\
           read(memory :> bytes64 stack) s = returnaddress /\
           frame_lease_trace e table stack [] s` in
  prove(frame_lease_prefix_statement mc gcc,
    REWRITE_TAC[SOME_FLAGS; fst exec] THEN BETA_TAC THEN
    REPEAT GEN_TAC THEN STRIP_TAC THEN ASM_CASES_TAC `n = 0` THENL
     [ASM_REWRITE_TAC[] THEN X86_SIM_TAC exec [];
      ENSURES_WHILE_UP_TAC `n:num` loop loop invariant THEN
      ASM_REWRITE_TAC[] THEN REPEAT CONJ_TAC THENL
       [X86_SIM_TAC exec [];
        X_GEN_TAC `i:num` THEN STRIP_TAC THEN
        MATCH_MP_TAC(REWRITE_RULE[SOME_FLAGS; fst exec]
          (CONV_RULE (DEPTH_CONV BETA_CONV) miss)) THEN
        CONJ_TAC THENL [ASM_ARITH_TAC; ASM_MESON_TAC[]];
        X_GEN_TAC `i:num` THEN STRIP_TAC THEN X86_SIM_TAC exec [];
        X86_SIM_TAC exec []]]);;
