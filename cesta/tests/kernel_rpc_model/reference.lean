import Mettapedia.Machines.Kernel.CorrelatedReplies
import Mettapedia.Machines.Kernel.TraceReplay

open Mettapedia.Machines.Kernel
open Mettapedia.Machines.Kernel.SexOSGenesis
open Mettapedia.Machines.Kernel.TraceReplay (domains)

structure Message where
  sender : Dom
  id : Nat
  kind : Nat
  value : Nat

structure RpcState where
  windows : Dom → CorrelatedReplies.Window Dom
  enabled : Dom → Bool
  queues : Dom → List Message

def start : RpcState := ⟨fun _ => ⟨0,[]⟩,fun _ => false,fun _ => []⟩

def peerAt (d : Dom) (slot : Nat) : Option Dom :=
  match caps d slot with
  | some ⟨.ring peer,_⟩ => some peer
  | _ => none

def windowConfig (capacity : Nat) (caller : Dom) : CorrelatedReplies.Config Dom :=
  ⟨capacity,2^64-1,fun peer => (List.range 32).any fun slot => peerAt caller slot == some peer⟩

def replyText : CorrelatedReplies.Result → String
  | .admitted id => s!"B{id}"
  | .value value => s!"V{value}"
  | .pending => "P"
  | .refused => "R"
  | .unavailable => "U"
  | .exhausted => "E"
  | .done => "D0"

def receive (s : RpcState) (peer : Dom) : List Message → RpcState × String
  | [] => ({s with queues:=upd s.queues peer []},"Z")
  | message::rest =>
      let (window,result):=CorrelatedReplies.claim (s.windows message.sender) peer message.id
      let next:={s with queues:=upd s.queues peer rest}
      if result = .done then
        ({next with windows:=upd s.windows message.sender window},
         s!"M{key message.sender}:{message.id}:{message.kind}:{message.value}")
      else receive next peer rest

def execute (capacity : Nat) (s : RpcState) (actor : Dom) (operation a b c : Nat) : RpcState × String :=
  match operation with
  | 0 => ({s with enabled:=upd s.enabled actor true},"D1")
  | 1 =>
      if b = 0 then (s,"R") else
      match peerAt actor a with
      | none => (s,"R")
      | some peer =>
          let (window,result):=CorrelatedReplies.admit (windowConfig capacity actor) (s.windows actor) peer
            (s.enabled peer) (decide ((s.queues peer).length < 256))
          let next:={s with windows:=upd s.windows actor window}
          match result with
          | .admitted id =>
              ({next with queues:=upd s.queues peer (s.queues peer ++ [⟨actor,id,b,c⟩])},replyText result)
          | _ => (next,replyText result)
  | 2 => receive s actor (s.queues actor)
  | 3 =>
      match domains.find? (fun d => key d == a) with
      | none => (s,"R")
      | some caller =>
          let (window,result):=CorrelatedReplies.complete (s.windows caller) actor b c
          ({s with windows:=upd s.windows caller window},replyText result)
  | 4 =>
      let (window,result):=CorrelatedReplies.poll (s.windows actor) a
      ({s with windows:=upd s.windows actor window},replyText result)
  | _ =>
      let (window,result):=CorrelatedReplies.cancel (s.windows actor) a
      ({s with windows:=upd s.windows actor window},replyText result)

def snapshot (s : RpcState) : String :=
  String.join (domains.map fun d =>
    let calls:=String.join ((s.windows d).calls.map fun call =>
      let (phase,value):=match call.phase with
        | .queued => (1,0) | .heard => (2,0) | .ready value => (3,value)
      s!"{call.id}:{key call.peer}:{phase}:{value},")
    let queued:=String.join ((s.queues d).map fun message =>
      s!"{key message.sender}:{message.id}:{message.kind}:{message.value},")
    s!"|D{key d}:{if s.enabled d then 1 else 0}:{(s.windows d).issued}:{calls}q{queued}")

private partial def replay (capacity : Nat) (s : RpcState) (input output : IO.FS.Stream) : IO Unit := do
  let line ← input.getLine
  if line.isEmpty then return
  let words := (line.trimAscii.toString.splitOn " ").filter (· != "")
  let values ← words.mapM fun word => match word.toNat? with
    | some n => pure n | none => throw (IO.userError "invalid command")
  match values with
  | [actor,operation,a,b,c] =>
      match domains.find? (fun d => key d == actor) with
      | none => throw (IO.userError "unknown caller")
      | some d =>
          let (next,result):=execute capacity s d operation a b c
          output.putStrLn (result ++ snapshot next)
          replay capacity next input output
  | _ => throw (IO.userError "five command words required")

def main (args : List String) : IO Unit := do
  let capacity := (args.headD "8").toNat!
  replay capacity start (← IO.getStdin) (← IO.getStdout)
