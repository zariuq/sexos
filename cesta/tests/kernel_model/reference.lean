import Mettapedia.Machines.Kernel.TraceReplay

open Mettapedia.Machines.Kernel
open Mettapedia.Machines.Kernel.SexOSGenesis
open Mettapedia.Machines.Kernel.TraceReplay

private partial def replay (s : State Dom) (input output : IO.FS.Stream) : IO Unit := do
  let line ← input.getLine
  if line.isEmpty then return
  match parse line with
  | .error message => throw (IO.userError message)
  | .ok (d, op) =>
    let (next, result) := step config s d op
    output.putStrLn (resultText result ++ snapshot next)
    replay next input output

def main : IO Unit := do
  replay initial (← IO.getStdin) (← IO.getStdout)
