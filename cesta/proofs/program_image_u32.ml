(* Exact linked-kernel byte decoder. The input contract is independent of the
   compiler: return the little-endian 32-bit word at a bounded byte offset. *)

let image_read_trace = new_definition
 `image_read_trace e (p:int64) count stackpointer s <=>
    ?added. read events s = APPEND added e /\
      memaccess_inbounds added
        [(p,count); (word_sub stackpointer (word 8),16)]
        [(word_sub stackpointer (word 8),8)]`;;

let image_u32_statement mc = vsubst [lhs(concl mc),`reader_mc:byte list`]
 `!p count offset (value:int32) pc stackpointer returnaddress e.
    count < 2 EXP 63 /\ offset + 4 <= count /\ ~(p = word 0) /\
    val p + count <= 2 EXP 64 /\ 8 <= val stackpointer /\
    ALL (nonoverlapping (word_sub stackpointer (word 8),16))
      [(word pc,LENGTH reader_mc); (p,count)]
    ==> ensures x86
      (\s. bytes_loaded s (word pc) reader_mc /\ read RIP s = word pc /\
        read RSP s = stackpointer /\
        read (memory :> bytes64 stackpointer) s = returnaddress /\
        C_ARGUMENTS [p; word count; word offset] s /\
        read (memory :> bytes32 (word_add p (word offset))) s = value /\
        read events s = e)
      (\s. read RIP s = returnaddress /\ read RSP s = word_add stackpointer (word 8) /\
        read RAX s = word_zx value /\ image_read_trace e p count stackpointer s)
      (MAYCHANGE [RSP] ,, MAYCHANGE_REGS_AND_FLAGS_PERMITTED_BY_ABI ,,
       MAYCHANGE [memory :> bytes(word_sub stackpointer (word 8),8)])`;;

let IMAGE_COUNT_SIGN = prove
 (`!n. n < 2 EXP 63 ==> ~(ival(word n:int64) < &0)`,
  GEN_TAC THEN DISCH_TAC THEN
  ASM_SIMP_TAC[ival; VAL_WORD_EQ; DIMINDEX_64;
    ARITH_RULE `n < 2 EXP 63 ==> n < 2 EXP 64`;
    ARITH_RULE `64 - 1 = 63`; INT_OF_NUM_LT; LT]);;

let image_u32_rule mc exec steps = prove(image_u32_statement mc,
  REWRITE_TAC[C_ARGUMENTS; SOME_FLAGS; ALL; NONOVERLAPPING_CLAUSES;
              MAYCHANGE_REGS_AND_FLAGS_PERMITTED_BY_ABI; fst exec; image_read_trace] THEN
  REPEAT GEN_TAC THEN STRIP_TAC THEN
  SUBGOAL_THEN `offset <= count /\ 4 <= count /\ 4 <= count - offset /\
               count < 2 EXP 64 /\ offset < 2 EXP 64 /\
               ~(count = 0) /\ ~(val(p:int64) = 0)` STRIP_ASSUME_TAC THENL
   [ASM_SIMP_TAC[VAL_EQ_0] THEN ASM_ARITH_TAC; ALL_TAC] THEN
  VAL_INT64_TAC `count:num` THEN VAL_INT64_TAC `offset:num` THEN
  SUBGOAL_THEN `~(count < offset) /\ ~(count = offset) /\
    ~(count - offset < 3) /\ ~(count - offset = 3)` STRIP_ASSUME_TAC THENL
   [ASM_ARITH_TAC; ALL_TAC] THEN
  SUBGOAL_THEN `word_sub (word count:int64) (word offset) = word(count - offset)`
  ASSUME_TAC THENL [ASM_SIMP_TAC[WORD_SUB]; ALL_TAC] THEN
  VAL_INT64_TAC `count - offset` THEN
  SUBGOAL_THEN `~(ival(word count:int64) < &0)` ASSUME_TAC THENL
   [ASM_SIMP_TAC[IMAGE_COUNT_SIGN]; ALL_TAC] THEN
  GHOST_INTRO_TAC `rax_before:int64` `read RAX` THEN
  GHOST_INTRO_TAC `r8_before:int64` `read R8` THEN
  GHOST_INTRO_TAC `rcx_before:int64` `read RCX` THEN
  ENSURES_INIT_TAC "s0" THEN
  MP_TAC(CONV_RULE (READ_MEMORY_SPLIT_CONV 2)
    (ASSUME `read (memory :> bytes32 (word_add p (word offset))) s0 = (value:int32)`)) THEN
  REWRITE_TAC[WORD_SUBWORD_SUBWORD; GSYM WORD_ADD_ASSOC; GSYM WORD_ADD] THEN STRIP_TAC THEN
  cesta_steps exec (1--steps) THEN ENSURES_FINAL_STATE_TAC THEN
  ASM_REWRITE_TAC[CESTA_STACK_RESTORE] THEN
  SUBGOAL_THEN
   `(EventLoad(stackpointer:int64,8):uarch_event) =
    EventLoad(word_add (word_sub stackpointer (word 8)) (word 8),8)` ASSUME_TAC THENL
   [REWRITE_TAC[CESTA_STACK_RESTORE]; ALL_TAC] THEN
  ASM_REWRITE_TAC[] THEN
  TRY(CONJ_TAC THENL [CONV_TAC WORD_BLAST; ALL_TAC]) THEN
  DISCHARGE_SAFETY_PROPERTY_TAC);;

let SEXOS_IMAGE_U32_GCC_CORRECT = image_u32_rule image_u32_gcc_mc IMAGE_U32_GCC_EXEC 47;;
let SEXOS_IMAGE_U32_CLANG_CORRECT = image_u32_rule image_u32_clang_mc IMAGE_U32_CLANG_EXEC 16;;
