(* Proof recipes for the exact compiled lease-table operation. *)

let FRAME_LEASE_SHIFT_ALIGNMENT = prove
 (`!base:int64. word_shl base 52 = word 0 <=> val base MOD 4096 = 0`,
  GEN_TAC THEN REWRITE_TAC[FRAME_LEASE_BASE_ALIGNMENT] THEN CONV_TAC WORD_BLAST);;

let FRAME_LEASE_ZX_CHAIN = prove
 (`!x:M word. dimindex(:P) <= dimindex(:N)
    ==> (word_zx(word_zx x:N word):P word) = word_zx x`,
  REPEAT STRIP_TAC THEN REWRITE_TAC[GSYM VAL_EQ; VAL_WORD_ZX_GEN; MOD_MOD_EXP_MIN] THEN
  ASM_SIMP_TAC[ARITH_RULE `p <= n ==> MIN n p = p`]);;

let FRAME_LEASE_ZX_WIDEN_CHAIN = prove
 (`!x:M word. dimindex(:M) <= dimindex(:N)
    ==> (word_zx(word_zx x:N word):P word) = word_zx x`,
  REPEAT STRIP_TAC THEN ONCE_REWRITE_TAC[word_zx] THEN ASM_SIMP_TAC[VAL_WORD_ZX]);;

let FRAME_LEASE_ZX_CHAIN_SAFE = prove
 (`!x:M word. dimindex(:P) <= dimindex(:N) \/ dimindex(:M) <= dimindex(:N)
    ==> (word_zx(word_zx x:N word):P word) = word_zx x`,
  REPEAT GEN_TAC THEN DISCH_THEN(DISJ_CASES_THEN ASSUME_TAC) THEN
  ASM_SIMP_TAC[FRAME_LEASE_ZX_CHAIN; FRAME_LEASE_ZX_WIDEN_CHAIN]);;

let FRAME_LEASE_ALIGN32 = prove
 (`!base:int64. word_and (word_zx base:int32) (word 4095) = word 0 <=>
               word_and base (word 4095) = word 0`,
  CONV_TAC WORD_BLAST);;

let FRAME_LEASE_SUB_CARRY = prove
 (`!(a:N word) b.
    ~(&(val a) - &(val b):int = &(val(word_sub a b))) <=> val a < val b`,
  REPEAT GEN_TAC THEN GEN_REWRITE_TAC RAND_CONV [GSYM INT_OF_NUM_LT] THEN
  REWRITE_TAC[INT_VAL_WORD_SUB_CASES] THEN COND_CASES_TAC THEN ASM_REWRITE_TAC[] THENL
   [ASM_INT_ARITH_TAC;
    MP_TAC(SPEC `dimindex(:N)` INT_LT_POW2) THEN ASM_INT_ARITH_TAC]);;

let FRAME_LEASE_BYTE_MOD = CONV_RULE NUM_REDUCE_CONV
  (REWRITE_RULE[DIMINDEX_8] (INST_TYPE [`:8`,`:N`] VAL_MOD_REFL));;

let FRAME_LEASE_ABOVE = prove
 (`!(a:N word) n. n < 2 EXP dimindex(:N)
    ==> (~(val a < n) /\ ~(a = word n) <=> n < val a)`,
  REPEAT STRIP_TAC THEN ASM_SIMP_TAC[GSYM VAL_EQ; VAL_WORD_EQ] THEN ARITH_TAC);;

let FRAME_LEASE_SUB_CARRY_CONST = prove
 (`!(a:N word) n. n < 2 EXP dimindex(:N)
    ==> (~(&(val a) - &n:int = &(val(word_sub a (word n)))) <=> val a < n)`,
  REPEAT STRIP_TAC THEN
  MP_TAC(SPECL [`a:N word`; `word n:N word`] FRAME_LEASE_SUB_CARRY) THEN
  ASM_SIMP_TAC[VAL_WORD_EQ]);;

let FRAME_LEASE_INDEX32 = prove
 (`!i. word_shl (word i:int64) 5 = word(32*i)`, CONV_TAC WORD_RULE);;

let FRAME_LEASE_KIND_BYTE = prove
 (`!kind:byte. val(word_sub kind (word 1)) <= 1 <=> kind = word 1 \/ kind = word 2`,
  GEN_TAC THEN REWRITE_TAC[GSYM VAL_EQ; VAL_WORD_SUB_CASES; VAL_WORD; DIMINDEX_8] THEN
  CONV_TAC NUM_REDUCE_CONV THEN COND_CASES_TAC THEN ASM_ARITH_TAC);;

let FRAME_LEASE_BOOL_AND = prove
 (`!p q. word_and (if p then word 1 else word 0:byte)
                    (if q then word 1 else word 0) =
          (if p /\ q then word 1 else word 0)`,
  REPEAT GEN_TAC THEN BOOL_CASES_TAC `p:bool` THEN BOOL_CASES_TAC `q:bool` THEN
  ASM_REWRITE_TAC[] THEN CONV_TAC WORD_REDUCE_CONV);;

let FRAME_LEASE_BOOL_ZERO = prove
 (`!p. ((if p then word 1 else word 0):byte) = word 0 <=> ~p`,
  GEN_TAC THEN BOOL_CASES_TAC `p:bool` THEN REWRITE_TAC[] THEN CONV_TAC WORD_REDUCE_CONV);;

let FRAME_LEASE_BOOL_ONE = prove
 (`!p. ((if p then word 1 else word 0):byte) = word 1 <=> p`,
  GEN_TAC THEN BOOL_CASES_TAC `p:bool` THEN REWRITE_TAC[] THEN CONV_TAC WORD_REDUCE_CONV);;

let FRAME_LEASE_SIGN_LOW_BYTE = prove
 (`!x:int32. (word_zx(word_sx x:int64):byte) = word_zx x`, CONV_TAC WORD_BLAST);;

let FRAME_LEASE_BYTE_RESIDUE = prove
 (`!x:M word. (word(val x MOD 256):byte) = word_zx x`,
  GEN_TAC THEN REWRITE_TAC[word_zx] THEN
  GEN_REWRITE_TAC RAND_CONV [GSYM WORD_MOD_SIZE] THEN
  REWRITE_TAC[DIMINDEX_8] THEN CONV_TAC NUM_REDUCE_CONV);;

let FRAME_LEASE_BYTE_MINUS_ONE = prove
 (`!kind:byte. word_add kind (word 255) = word_sub kind (word 1)`,
  GEN_TAC THEN REWRITE_TAC[WORD_RULE
    `word_sub x y:N word = word_add x (word_neg y)`] THEN
  CONV_TAC WORD_REDUCE_CONV);;

let FRAME_LEASE_BELOW_EQ = prove
 (`!(a:N word) n. n < 2 EXP dimindex(:N)
    ==> (val a < n \/ a = word n <=> val a <= n)`,
  REPEAT STRIP_TAC THEN ASM_SIMP_TAC[GSYM VAL_EQ; VAL_WORD_EQ] THEN ARITH_TAC);;

let frame_lease_normalize : tactic =
  RULE_ASSUM_TAC(fun theorem ->
    let spatial tm = match strip_comb tm with
      | Const(name,_),_ -> mem name
          ["nonoverlapping";"nonoverlapping_modulo";"contained";"contained_modulo"]
      | _ -> false in
    if spatial(concl theorem) then theorem else
    SIMP_RULE[FRAME_LEASE_ZX_CHAIN_SAFE; WORD_ZX_ZX;
    DIMINDEX_8; DIMINDEX_32; DIMINDEX_64; ARITH_LE; ARITH_LT;
    VAL_EQ_0; VAL_WORD_ZX; FRAME_LEASE_ALIGN32; FRAME_LEASE_BYTE_MOD;
    FRAME_LEASE_SUB_CARRY; FRAME_LEASE_SUB_CARRY_CONST; FRAME_LEASE_ABOVE;
    WORD_SUB_EQ_0; WORD_VAL; ARITH; DE_MORGAN_THM;
    FRAME_LEASE_SHIFT_ALIGNMENT; FRAME_LEASE_BASE_ALIGNMENT;
    FRAME_LEASE_INDEX32;
    WORD_OR_EQ_0; FRAME_LEASE_BOOL_AND; FRAME_LEASE_BOOL_ZERO; FRAME_LEASE_BOOL_ONE;
    WORD_ZX_SUB; FRAME_LEASE_KIND_BYTE; ARITH_RULE `n < 2 <=> n <= 1`;
    FRAME_LEASE_SIGN_LOW_BYTE; FRAME_LEASE_BYTE_RESIDUE; WORD_SX_ZX; WORD_SX_TRIVIAL;
    WORD_ZX_ADD; WORD_ZX_WORD; WORD_ZX_1; FRAME_LEASE_BYTE_MINUS_ONE;
    FRAME_LEASE_BELOW_EQ;
    WORD_ZX_OR; WORD_ZX_0; CESTA_LOW_BYTE_JOIN] theorem) THEN
  cesta_normalize_state;;

let frame_lease_steps exec indices =
  MAP_EVERY (fun index ->
    let state = "s" ^ string_of_int index in
    frame_lease_normalize THEN cesta_memory_step_tac exec state THEN
    cesta_normalize_step state THEN
    frame_lease_normalize THEN cesta_discard_oldstate_tac state THEN
    CLARIFY_TAC THEN frame_lease_normalize) indices;;

let frame_detach_entry_statement mc gcc =
  let offset=if gcc then 96 else 124 in
  let registers=if gcc then
   `read R9 s = table /\ read RAX s = table /\ read RDX s = word 0 /\
    read RCX s = base /\ read RSI s = word_zx (holder:int32) /\
    read RDI s = word_zx (incarnation:int32) /\ read R8 s = word_zx (kind:byte) /\
    read R10 s = word_zx (ordr:byte)`
  else
   `read RDI s = table /\ read RAX s = word 0 /\ read R8 s = base /\
    read RDX s = word_zx (holder:int32) /\ read RSI s = word_zx (incarnation:int32) /\
    read RCX s = word_zx (kind:byte) /\ read R9 s = word_zx (ordr:byte)` in
  let body = vsubst [lhs(concl mc),`detach_mc:byte list`;mk_small_numeral offset,`entry_offset:num`;
          mk_abs(`s:x86state`,registers),`post_regs:x86state->bool`]
   `frame_lease_valid base holder incarnation ordr kind
      ==> ensures x86
        (\s. bytes_loaded s (word pc) detach_mc /\ read RIP s = word pc /\
             read RDI s = table /\ read RSP s = stack /\
             read memory s = ram /\ read(memory :> bytes64 stack) s = returnaddress /\
             frame_lease_arguments stack base holder incarnation ordr kind s /\
             read events s = e)
        (\s. read RIP s = word(pc + entry_offset) /\ post_regs s /\
             read RSP s = stack /\ read memory s = ram /\
             read(memory :> bytes64 stack) s = returnaddress /\
             frame_lease_trace e table stack [] s)
        (MAYCHANGE [RIP; RAX; RDI; RSI; RDX; RCX; R8; R9; R10; R11] ,,
         MAYCHANGE SOME_FLAGS ,, MAYCHANGE [events])` in
  list_mk_forall ([`table:int64`;`stack:int64`;`base:int64`;`holder:int32`;
    `incarnation:int32`;`ordr:byte`;`kind:byte`;`ram:int64->byte`;`returnaddress:int64`;
    `pc:num`;`e:uarch_event list`],body);;

let frame_detach_entry_prepare mc exec gcc =
  REWRITE_TAC[frame_lease_valid; FRAME_LEASE_BASE_ALIGNMENT; frame_lease_arguments;
              frame_lease_trace; SOME_FLAGS; fst exec] THEN BETA_TAC THEN
  REPEAT GEN_TAC THEN STRIP_TAC THEN
  (SUBGOAL_THEN `~(18 < val(ordr:byte)) /\ val ordr < 19` STRIP_ASSUME_TAC THENL
   [ASM_ARITH_TAC; ALL_TAC]) THEN
  GHOST_INTRO_TAC `rax_initial:int64` `read RAX` THEN
  GHOST_INTRO_TAC `rdx_initial:int64` `read RDX` THEN
  GHOST_INTRO_TAC `r8_initial:int64` `read R8` THEN
  GHOST_INTRO_TAC `r9_initial:int64` `read R9` THEN
  GHOST_INTRO_TAC `r10_initial:int64` `read R10` THEN
  GHOST_INTRO_TAC `r11_initial:int64` `read R11` THEN
  ENSURES_INIT_TAC "s0";;

let frame_detach_entry_rule mc exec gcc = prove(frame_detach_entry_statement mc gcc,
  frame_detach_entry_prepare mc exec gcc THEN
  frame_lease_steps exec (1--(if gcc then 26 else 30)) THEN
  ENSURES_FINAL_STATE_TAC THEN
  ASM_REWRITE_TAC[WORD_ZX_1; WORD_ZX_WORD; DIMINDEX_8] THEN
  CONV_TAC NUM_REDUCE_CONV THEN
  TRY(CONJ_TAC THENL [CONV_TAC WORD_BLAST; ALL_TAC]) THEN
  DISCHARGE_SAFETY_PROPERTY_TAC);;
