(* Allocation identity and the first matching live table entry. These predicates
   describe the contract independently of the instruction selection. *)

let frame_lease_valid = new_definition
 `frame_lease_valid (base:int64) (holder:int32) (incarnation:int32) (ordr:byte) (kind:byte) <=>
    ~(base = word 0) /\ val base MOD 4096 = 0 /\
    ~(holder = word 0) /\ ~(incarnation = word 0) /\ val ordr <= 18 /\
    (kind = word 1 \/ kind = word 2)`;;

let frame_lease_match = new_definition
 `frame_lease_match (base:int64) (holder:int32) (incarnation:int32) (ordr:byte) (kind:byte)
                    entry_base entry_holder entry_incarnation entry_order (phase:byte) entry_kind <=>
    phase = word 1 /\ entry_base = base /\ entry_holder = holder /\
    entry_incarnation = incarnation /\ entry_order = ordr /\ entry_kind = kind`;;

let frame_lease_first = new_definition
 `frame_lease_first matches n <=>
    n <= 4096 /\ (!i. i < n ==> ~matches i) /\ (n < 4096 ==> matches n)`;;

let FRAME_LEASE_FIRST_EXISTS = prove
 (`!matches:num->bool. ?n. frame_lease_first matches n`,
  GEN_TAC THEN ASM_CASES_TAC `?n. n < 4096 /\ (matches:num->bool) n` THENL
   [FIRST_X_ASSUM(MP_TAC o GEN_REWRITE_RULE I [num_WOP]) THEN
    MATCH_MP_TAC MONO_EXISTS THEN X_GEN_TAC `n:num` THEN
    REWRITE_TAC[frame_lease_first] THEN STRIP_TAC THEN
    REPEAT CONJ_TAC THENL [ASM_ARITH_TAC; ASM_MESON_TAC[LT_TRANS]; ASM_REWRITE_TAC[]];
    EXISTS_TAC `4096` THEN REWRITE_TAC[frame_lease_first; LE_REFL; LT_REFL] THEN
    ASM_MESON_TAC[]]);;

let FRAME_LEASE_FIRST_UNIQUE = prove
 (`!matches m n. frame_lease_first matches m /\ frame_lease_first matches n ==> m = n`,
  REWRITE_TAC[frame_lease_first] THEN REPEAT STRIP_TAC THEN
  ASM_CASES_TAC `m:num < n` THENL
   [ASM_MESON_TAC[ARITH_RULE `m < n /\ n <= 4096 ==> m < 4096`];
    ASM_CASES_TAC `n:num < m` THENL
     [ASM_MESON_TAC[ARITH_RULE `n < m /\ m <= 4096 ==> n < 4096`]; ASM_ARITH_TAC]]);;

let frame_lease_arguments = new_definition
 `frame_lease_arguments (stack:int64) (base:int64) (holder:int32) (incarnation:int32)
                       (ordr:byte) (kind:byte) s <=>
    read (memory :> bytes64 (word_add stack (word 8))) s = base /\
    read (memory :> bytes32 (word_add stack (word 16))) s = holder /\
    read (memory :> bytes32 (word_add stack (word 20))) s = incarnation /\
    read (memory :> bytes8 (word_add stack (word 24))) s = ordr /\
    read (memory :> bytes8 (word_add stack (word 25))) s = kind`;;

let frame_lease_matches_at = new_definition
 `frame_lease_matches_at (table:int64) base holder incarnation ordr kind
                        (ram:int64->byte) slot <=>
    frame_lease_match base holder incarnation ordr kind
      (read (bytes64(word_add table (word(32*slot)))) ram)
      (read (bytes32(word_add table (word(32*slot+16)))) ram)
      (read (bytes32(word_add table (word(32*slot+20)))) ram)
      (read (bytes8(word_add table (word(32*slot+24)))) ram)
      (read (bytes8(word_add table (word(32*slot+25)))) ram)
      (read (bytes8(word_add table (word(32*slot+26)))) ram)`;;

let frame_lease_trace = new_definition
 `frame_lease_trace e (table:int64) stack writes s <=>
    ?added. read events s = APPEND added e /\
      memaccess_inbounds added [(table,131072); (stack,32)] writes`;;

let frame_lease_input_trace = new_definition
 `frame_lease_input_trace e (stack:int64) s <=>
    ?added. read events s = APPEND added e /\
      memaccess_inbounds added [(stack,32)] []`;;

let FRAME_LEASE_TRACE_WRITES = prove
 (`!e table stack s writes.
    frame_lease_trace e table stack [] s ==> frame_lease_trace e table stack writes s`,
  REWRITE_TAC[frame_lease_trace] THEN REPEAT STRIP_TAC THEN
  EXISTS_TAC `added:uarch_event list` THEN ASM_REWRITE_TAC[] THEN
  MATCH_MP_TAC(REWRITE_RULE[ALL; MEM] (ISPECL [`added:uarch_event list`;
    `[(table:int64,131072);(stack:int64,32)]`;
    `[(table:int64,131072);(stack:int64,32)]`;
    `[]:(int64#num)list`;`writes:(int64#num)list`] MEMACCESS_INBOUNDS_MEM)) THEN
  ASM_REWRITE_TAC[ALL; MEM]);;

let FRAME_LEASE_INPUT_TRACE_PROMOTE = prove
 (`!e table stack s. frame_lease_input_trace e stack s
    ==> frame_lease_trace e table stack [] s`,
  REWRITE_TAC[frame_lease_input_trace; frame_lease_trace] THEN REPEAT STRIP_TAC THEN
  EXISTS_TAC `added:uarch_event list` THEN ASM_REWRITE_TAC[] THEN
  MATCH_MP_TAC(REWRITE_RULE[ALL; MEM] (ISPECL [`added:uarch_event list`;`[(stack:int64,32)]`;
    `[(table:int64,131072);(stack:int64,32)]`;
    `[]:(int64#num)list`;`[]:(int64#num)list`] MEMACCESS_INBOUNDS_MEM)) THEN
  ASM_REWRITE_TAC[ALL; MEM]);;

let frame_lease_selected = new_definition
 `frame_lease_selected matches = @n. frame_lease_first matches n`;;

let FRAME_LEASE_SELECTED_FIRST = prove
 (`!matches. frame_lease_first matches (frame_lease_selected matches)`,
  GEN_TAC THEN REWRITE_TAC[frame_lease_selected] THEN
  MATCH_ACCEPT_TAC(SELECT_RULE(SPEC `matches:num->bool` FRAME_LEASE_FIRST_EXISTS)));;

let frame_lease_result = new_definition
 `frame_lease_result table base holder incarnation ordr kind (ram:int64->byte) =
    let slot = frame_lease_selected
      (frame_lease_matches_at table base holder incarnation ordr kind ram) in
    if frame_lease_valid base holder incarnation ordr kind then
      if slot < 4096 then
        (0, write (bytes8(word_add table (word(32*slot+25)))) (word 2:byte) ram)
      else (3, ram)
    else (1, ram)`;;

let frame_lease_writes = new_definition
 `frame_lease_writes table base holder incarnation ordr kind (ram:int64->byte) =
    let slot = frame_lease_selected
      (frame_lease_matches_at table base holder incarnation ordr kind ram) in
    if frame_lease_valid base holder incarnation ordr kind /\ slot < 4096
    then [(word_add table (word(32*slot+25)),1)] else []`;;

let FRAME_LEASE_BASE_ALIGNMENT = prove
 (`!base:int64. val base MOD 4096 = 0 <=> word_and base (word 4095) = word 0`,
  GEN_TAC THEN MP_TAC(ISPECL [`base:int64`; `12`] VAL_WORD_AND_MASK_WORD) THEN
  CONV_TAC NUM_REDUCE_CONV THEN REWRITE_TAC[GSYM VAL_EQ; VAL_WORD_0] THEN MESON_TAC[]);;

let FRAME_LEASE_PTE_PROJECTION = prove
 (`!(observed:int64) expected.
    word_and observed (word_not(word 96)) = word_and expected (word_not(word 96))
    ==> word_and observed (word 0x000ffffffffff000) =
        word_and expected (word 0x000ffffffffff000) /\
        word_and observed (word 0xf800000000000007) =
        word_and expected (word 0xf800000000000007)`,
  REPEAT GEN_TAC THEN DISCH_TAC THEN CONJ_TAC THENL
   [FIRST_X_ASSUM(MP_TAC o AP_TERM `(\w:int64. word_and w (word 0x000ffffffffff000))`);
    FIRST_X_ASSUM(MP_TAC o AP_TERM `(\w:int64. word_and w (word 0xf800000000000007))`)] THEN
  REWRITE_TAC[GSYM WORD_AND_ASSOC] THEN CONV_TAC WORD_REDUCE_CONV THEN REWRITE_TAC[]);;
