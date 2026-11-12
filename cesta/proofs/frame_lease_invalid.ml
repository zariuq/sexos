(* Forward symbolic execution of validation. Every branch split concerns the
   condition derived from the actual instructions. A reached loop entry must
   contradict the invalid-request hypothesis; the return proves no table read. *)

let frame_lease_invalid_statement mc =
  vsubst[lhs(concl mc),`detach_mc:byte list`]
   `!table stack (base:int64) (holder:int32) (incarnation:int32) (ordr:byte) (kind:byte)
      (ram:int64->byte) pc returnaddress e.
    ~frame_lease_valid base holder incarnation ordr kind
    ==> ensures x86
      (\s. bytes_loaded s (word pc) detach_mc /\ read RIP s = word pc /\
           read RDI s = table /\ read RSP s = stack /\ read memory s = ram /\
           frame_lease_arguments stack base holder incarnation ordr kind s /\
           read(memory :> bytes64 stack) s = returnaddress /\ read events s = e)
      (\s. read RIP s = returnaddress /\ read RSP s = word_add stack (word 8) /\
           read RAX s = word 1 /\ read memory s = ram /\
           frame_lease_input_trace e stack s)
      (MAYCHANGE [RIP; RSP; RAX; RDI; RSI; RDX; RCX; R8; R9; R10; R11] ,,
       MAYCHANGE SOME_FLAGS ,, MAYCHANGE [events])`;;

let frame_lease_invalid_loop_unreachable : tactic =
  DISCARD_ASSUMPTIONS_TAC (fun theorem ->
    exists (fun variable -> type_of variable = `:x86state`) (frees(concl theorem))) THEN
  UNDISCH_TAC `~frame_lease_valid (base:int64) (holder:int32) (incarnation:int32)
                                (ordr:byte) (kind:byte)` THEN
  REWRITE_TAC[frame_lease_valid; FRAME_LEASE_BASE_ALIGNMENT] THEN
  ASM_REWRITE_TAC[] THEN ASM_ARITH_TAC;;

let frame_lease_invalid_forward exec gcc =
  let at offset = mk_comb(`word:num->int64`,
    mk_comb(mk_comb(`(+):num->num->num`,`pc:num`),mk_small_numeral offset)) in
  let loop=at(if gcc then 96 else 124) and ret=at(if gcc then 61 else 175) in
  let rec walk step cases = W(fun (asl,_) ->
    if step > 40 then failwith "validation recipe exceeded forward bound";
    if cases > 12 then failwith "validation recipe retained an unresolved branch";
    let state=mk_var("s" ^ string_of_int step,`:x86state`) in
    let lhs_pc=mk_comb(`read RIP:x86state->int64`,state) in
    let pc=tryfind (fun (_,th) ->
      let left,right=dest_eq(concl th) in
      if left=lhs_pc then right else failwith "different state") asl in
    if is_cond pc then
      let condition,_=dest_cond pc in
      ASM_CASES_TAC condition THEN
      W(fun (facts,_) ->
        let stateful th=exists (fun v -> type_of v = `:x86state`) (frees(concl th)) in
        let pure=filter (fun th -> not(stateful th)) (map snd facts) in
        RULE_ASSUM_TAC(fun th -> if stateful th then
          simplify_conditionals_with_facts pure th else th)) THEN
      frame_lease_normalize THEN walk step (cases+1)
    else if pc=loop then frame_lease_invalid_loop_unreachable
    else if pc=ret then
      frame_lease_steps exec [step+1] THEN ENSURES_FINAL_STATE_TAC THEN
      ASM_REWRITE_TAC[] THEN DISCHARGE_SAFETY_PROPERTY_TAC
    else frame_lease_steps exec [step+1] THEN walk (step+1) cases) in
  walk 0 0;;

let frame_lease_invalid_rule mc exec gcc = prove(frame_lease_invalid_statement mc,
  REWRITE_TAC[frame_lease_arguments; frame_lease_input_trace; SOME_FLAGS; fst exec] THEN
  REPEAT GEN_TAC THEN DISCH_TAC THEN
  GHOST_INTRO_TAC `rax_initial:int64` `read RAX` THEN
  GHOST_INTRO_TAC `rdx_initial:int64` `read RDX` THEN
  GHOST_INTRO_TAC `r8_initial:int64` `read R8` THEN
  GHOST_INTRO_TAC `r9_initial:int64` `read R9` THEN
  GHOST_INTRO_TAC `r10_initial:int64` `read R10` THEN
  GHOST_INTRO_TAC `r11_initial:int64` `read R11` THEN
  ENSURES_INIT_TAC "s0" THEN frame_lease_invalid_forward exec gcc);;
