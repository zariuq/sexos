#!/usr/bin/env bash
# Boot an ISO headless and keep its serial log.
#   cesta/oracle/run.sh ISO LOG SECONDS [--fixed-clock] [--until MARKER]...
#     [--post-ready SECONDS] [--accel tcg|kvm]
#     [--cpu MODEL] [--smp COUNT] [--nvme-image RAW] [--nvme-persistent RAW]
#     [--network] [--network-gateway IPV4] [--usb-keyboard]
#     [--qmp-socket PATH]
# The machine is the upstream runtime gate's: q35, 512 MiB, PKU, an xHCI
# controller with a USB tablet. Network and writable storage are opt-in.
#   --fixed-clock  time comes from instruction counts (-icount) and the RTC
#                  starts at a fixed date, so the run does not depend on host
#                  speed or wall time.
#   --until M      stop as soon as every given marker has appeared in the log
#                  (SECONDS is then the upper bound).
#   --post-ready N after all --until markers appear, keep running for N more
#                  wall-clock seconds. This is useful for short sustained-loop
#                  smoke checks without weakening the full-soak gates.
#   --accel A      select TCG or KVM explicitly. Instruction-count time is a
#                  TCG-only deterministic mode and is refused with KVM.
set -euo pipefail
iso="${1:?usage: run.sh ISO LOG SECONDS [--fixed-clock] [--until MARKER]...}"
log="${2:?}"
secs="${3:?}"
shift 3
cpu=max,+pku
cpu_explicit=0
accel=
fixed_clock=0
post_ready=0
network=0
nvme_mode=0
nic_model=e1000
gateway=
args=(-M q35 -m 512M -cdrom "$iso"
      -device nec-usb-xhci,id=xhci -device usb-tablet,bus=xhci.0
      -nic none -serial "file:$log" -display none -no-reboot -no-shutdown)
until_markers=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --fixed-clock) fixed_clock=1; args+=(-icount shift=4,sleep=off -rtc base=2026-01-01T10:00:00,clock=vm); shift ;;
    --accel)
      accel="${2:?--accel needs tcg or kvm}"
      [[ "$accel" == tcg || "$accel" == kvm ]] || { echo "unsupported accelerator" >&2; exit 2; }
      shift 2 ;;
    --post-ready)
      post_ready="${2:?--post-ready needs a duration}"
      [[ "$post_ready" =~ ^[0-9]+$ ]] || { echo "invalid post-ready duration" >&2; exit 2; }
      shift 2 ;;
    --qmp-socket)
      qmp_socket="${2:?--qmp-socket needs a path}"
      [[ ! -e "$qmp_socket" ]] || { echo "choose a new QMP socket path" >&2; exit 2; }
      args+=(-qmp "unix:$qmp_socket,server=on,wait=off"); shift 2 ;;
    --until) until_markers+=("${2:?--until needs a marker}"); shift 2 ;;
    --cpu) cpu="${2:?--cpu needs a model}"; cpu_explicit=1; shift 2 ;;
    --smp)
      smp="${2:?--smp needs a count}"
      [[ "$smp" =~ ^[1-9][0-9]*$ ]] || { echo "invalid SMP count" >&2; exit 2; }
      args+=(-smp "$smp"); shift 2 ;;
    --nvme-image)
      (( nvme_mode == 0 )) || { echo "choose one NVMe mode" >&2; exit 2; }
      nvme_mode=1
      args+=(-drive "file=${2:?--nvme-image needs a raw image},format=raw,if=none,id=nvme0,snapshot=on"
             -device nvme,drive=nvme0,serial=SEXOS-CHECK); shift 2 ;;
    --nvme-persistent)
      (( nvme_mode == 0 )) || { echo "choose one NVMe mode" >&2; exit 2; }
      nvme_mode=2
      args+=(-drive "file=${2:?--nvme-persistent needs a disposable raw image},format=raw,if=none,id=nvme0,snapshot=off"
             -device nvme,drive=nvme0,serial=SEXOS-CHECK); shift 2 ;;
    --network) network=1; shift ;;
    --nic-model)
      nic_model="${2:?--nic-model needs e1000 or e1000e}"
      [[ "$nic_model" == e1000 || "$nic_model" == e1000e ]] || { echo "unsupported NIC fixture" >&2; exit 2; }
      network=1; shift 2 ;;
    --network-gateway)
      network=1; gateway="${2:?--network-gateway needs an IPv4 address}"
      if [[ ! "$gateway" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        echo "invalid network gateway" >&2; exit 2
      fi
      shift 2 ;;
    --usb-keyboard) args+=(-device usb-kbd,bus=xhci.0); shift ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done
[[ "$post_ready" == 0 || ${#until_markers[@]} -gt 0 ]] || {
  echo "--post-ready requires at least one --until marker" >&2
  exit 2
}
[[ "$accel" != kvm || "$fixed_clock" == 0 ]] || {
  echo "--fixed-clock is incompatible with KVM" >&2
  exit 2
}
if [[ "$accel" == kvm ]]; then
  [[ -e /dev/kvm ]] || { echo "KVM is unavailable" >&2; exit 2; }
  (( cpu_explicit )) || cpu=host,+pku
fi
[[ -z "$accel" ]] || args+=(-accel "$accel")
args+=(-cpu "$cpu")
if (( network )); then
  backend=user,id=net0,restrict=on
  [[ -z "$gateway" ]] || backend+=",host=$gateway"
  args+=(-netdev "$backend" -device "$nic_model,netdev=net0")
fi

if [[ -e "$log" ]]; then
  echo "choose a new serial log; existing evidence is preserved" >&2
  exit 2
fi
qemu-system-x86_64 "${args[@]}" &
qemu=$!
trap 'kill "$qemu" 2>/dev/null || true' EXIT
deadline=$((SECONDS + secs))
marker_deadline=-1
while kill -0 "$qemu" 2>/dev/null && (( SECONDS < deadline )); do
  if (( ${#until_markers[@]} )) && [[ -f "$log" ]]; then
    all=1
    for m in "${until_markers[@]}"; do
      grep -a -q -F -- "$m" "$log" || { all=0; break; }
    done
    if (( all )); then
      if (( marker_deadline < 0 )); then
        # SECONDS is integral: one extra tick guarantees the requested
        # post-readiness interval rather than rounding it down.
        marker_deadline=$((SECONDS + post_ready + (post_ready > 0)))
      fi
      (( SECONDS >= marker_deadline )) && break
    fi
  fi
  sleep 0.5
done
if ! kill -0 "$qemu" 2>/dev/null; then
  wait "$qemu" || { rc=$?; echo "qemu exited with $rc" >&2; exit "$rc"; }
fi
kill "$qemu" 2>/dev/null || true
wait "$qemu" 2>/dev/null || true
trap - EXIT
if (( ${#until_markers[@]} )); then
  for m in "${until_markers[@]}"; do
    if [[ ! -f "$log" ]] || ! grep -a -q -F -- "$m" "$log"; then
      echo "boot interval ended before readiness: $m" >&2
      exit 3
    fi
  done
  if (( post_ready > 0 && (marker_deadline < 0 || SECONDS < marker_deadline) )); then
    echo "boot interval ended before the complete post-readiness soak" >&2
    exit 3
  fi
elif (( SECONDS < deadline )); then
  echo "qemu exited before the complete soak interval" >&2
  exit 3
fi
