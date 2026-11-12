(* Compose validation, the bounded first-match search, the exact byte update
   or refusal, and the ABI return. The specification chooses its own index. *)

let frame_lease_function_statement mc =
  vsubst[lhs(concl mc),`detach_mc:byte list`]
   `!table stack (base:int64) (holder:int32) (incarnation:int32) (ordr:byte) (kind:byte)
      (ram:int64->byte) pc returnaddress e.
    nonoverlapping (word pc,LENGTH detach_mc) (table,131072) /\
    nonoverlapping (stack,32) (table,131072)
    ==> ensures x86
      (\s. bytes_loaded s (word pc) detach_mc /\ read RIP s = word pc /\
           read RDI s = table /\ read RSP s = stack /\ read memory s = ram /\
           frame_lease_arguments stack base holder incarnation ordr kind s /\
           read(memory :> bytes64 stack) s = returnaddress /\ read events s = e)
      (\s. read RIP s = returnaddress /\ read RSP s = word_add stack (word 8) /\
           (val(read RAX s),read memory s) =
             frame_lease_result table base holder incarnation ordr kind ram /\
           frame_lease_trace e table stack
             (frame_lease_writes table base holder incarnation ordr kind ram) s)
      (MAYCHANGE [RIP; RSP; RAX; RDI; RSI; RDX; RCX; R8; R9; R10; R11] ,,
       MAYCHANGE SOME_FLAGS ,, MAYCHANGE [events] ,,
       MAYCHANGE [memory :> bytes(table,131072)])`;;

let frame_lease_composition_trace = ref false;;

let frame_lease_function_tac mc exec entry prefix hit exhaust invalid =
  let common=[`table:int64`;`stack:int64`;`base:int64`;`holder:int32`;
    `incarnation:int32`;`ordr:byte`;`kind:byte`;`ram:int64->byte`] in
  let tail=[`pc:num`;`returnaddress:int64`;`e:uarch_event list`] in
  let matches=`frame_lease_matches_at (table:int64) base holder incarnation ordr kind
                                       (ram:int64->byte)` in
  let apply theorem args state =
    W(fun _ -> if !frame_lease_composition_trace then
      (Printf.printf "frame composition: %s\n%!" state; ALL_TAC) else ALL_TAC) THEN
    MP_TAC(CONV_RULE (DEPTH_CONV BETA_CONV)
      (REWRITE_RULE[SOME_FLAGS; MULT_CLAUSES; ADD_CLAUSES; WORD_ADD_0;
                    NONOVERLAPPING_CLAUSES; fst exec; ARITH_LT; ARITH_LE]
        (SPECL args theorem))) THEN
    ASM_REWRITE_TAC[] THEN cesta_bigstep_tac exec state in
  let finish = ENSURES_FINAL_STATE_TAC THEN
    ASM_REWRITE_TAC[VAL_WORD_0; VAL_WORD_1; VAL_WORD; DIMINDEX_64] THEN
    CONV_TAC NUM_REDUCE_CONV THEN ASM_REWRITE_TAC[] THEN
    W(fun (_,goal) -> if !frame_lease_composition_trace then
      (Printf.printf "frame remaining: %s\n%!" (string_of_term goal); ALL_TAC) else ALL_TAC) in
  (
    REWRITE_TAC[SOME_FLAGS; NONOVERLAPPING_CLAUSES; fst exec] THEN
    REPEAT GEN_TAC THEN STRIP_TAC THEN
    (SUBGOAL_THEN `nonoverlapping (stack:int64,8) (table:int64,131072)`
      (ASSUME_TAC o REWRITE_RULE[NONOVERLAPPING_CLAUSES]) THENL
     [NONOVERLAPPING_TAC; ALL_TAC]) THEN
    ABBREV_TAC `lease_slot = frame_lease_selected
       (frame_lease_matches_at (table:int64) base holder incarnation ordr kind (ram:int64->byte))` THEN
    (SUBGOAL_THEN (mk_comb(mk_comb(`frame_lease_first`,matches),`lease_slot:num`)) MP_TAC THENL
     [EXPAND_TAC "lease_slot" THEN MATCH_ACCEPT_TAC(SPEC matches FRAME_LEASE_SELECTED_FIRST);
      REWRITE_TAC[frame_lease_first] THEN
      DISCH_THEN(CONJUNCTS_THEN2 ASSUME_TAC
        (CONJUNCTS_THEN2 (LABEL_TAC "lease_misses") (LABEL_TAC "lease_found")))]) THEN
    ASM_CASES_TAC `frame_lease_valid (base:int64) (holder:int32) incarnation (ordr:byte) kind` THENL
     [ASM_CASES_TAC `lease_slot < 4096` THENL
       [(SUBGOAL_THEN
          `frame_lease_matches_at (table:int64) base holder incarnation ordr kind ram lease_slot`
          ASSUME_TAC THENL
         [USE_THEN "lease_found" MATCH_MP_TAC THEN ASM_REWRITE_TAC[]; ALL_TAC]) THEN
        ASM_REWRITE_TAC[frame_lease_result; frame_lease_writes; LET_DEF; LET_END_DEF] THEN
        ENSURES_INIT_TAC "s0" THEN
        apply entry (common @ [`returnaddress:int64`;`pc:num`;`e:uarch_event list`]) "s1" THEN
        apply prefix (common @ [`lease_slot:num`] @ tail) "s2" THEN
        (SUBGOAL_THEN `frame_lease_trace e (table:int64) stack
          [(word_add table (word(32*lease_slot+25)),1)] (s2:x86state)` ASSUME_TAC THENL
         [MATCH_MP_TAC FRAME_LEASE_TRACE_WRITES THEN ASM_REWRITE_TAC[]; ALL_TAC]) THEN
        apply hit (common @ [`lease_slot:num`] @ tail) "s3" THEN finish;
        (SUBGOAL_THEN `lease_slot = 4096` ASSUME_TAC THENL [ASM_ARITH_TAC; ALL_TAC]) THEN
        (SUBGOAL_THEN
          `!i. i < 4095 ==> ~frame_lease_matches_at (table:int64) base holder incarnation ordr kind ram i`
          ASSUME_TAC THENL
         [X_GEN_TAC `i:num` THEN DISCH_TAC THEN
          USE_THEN "lease_misses" (MATCH_MP_TAC o SPEC `i:num`) THEN ASM_ARITH_TAC; ALL_TAC]) THEN
        (SUBGOAL_THEN
          `~frame_lease_matches_at (table:int64) base holder incarnation ordr kind ram 4095`
          ASSUME_TAC THENL
         [USE_THEN "lease_misses" (MATCH_MP_TAC o SPEC `4095`) THEN ASM_ARITH_TAC; ALL_TAC]) THEN
        ASM_REWRITE_TAC[frame_lease_result; frame_lease_writes; LET_DEF; LET_END_DEF] THEN
        ENSURES_INIT_TAC "s0" THEN
        apply entry (common @ [`returnaddress:int64`;`pc:num`;`e:uarch_event list`]) "s1" THEN
        apply prefix (common @ [`4095`] @ tail) "s2" THEN
        apply exhaust (common @ tail) "s3" THEN finish];
      ASM_REWRITE_TAC[frame_lease_result; frame_lease_writes; LET_DEF; LET_END_DEF] THEN
      ENSURES_INIT_TAC "s0" THEN apply invalid (common @ tail) "s1" THEN
      (SUBGOAL_THEN `frame_lease_trace e (table:int64) stack [] (s1:x86state)` ASSUME_TAC THENL
       [MATCH_MP_TAC FRAME_LEASE_INPUT_TRACE_PROMOTE THEN ASM_REWRITE_TAC[]; ALL_TAC]) THEN
      finish]);;

let frame_lease_function_rule mc exec entry prefix hit exhaust invalid =
  prove(frame_lease_function_statement mc,
    frame_lease_function_tac mc exec entry prefix hit exhaust invalid);;
