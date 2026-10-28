#!/usr/bin/env bash
# Boot an ISO headless and keep its serial log.
#   cesta/oracle/run.sh ISO LOG SECONDS [--fixed-clock] [--until MARKER]...
#     [--cpu MODEL] [--nvme-image RAW] [--network] [--network-gateway IPV4] [--usb-keyboard]
# The machine is the upstream runtime gate's: q35, 512 MiB, PKU, an xHCI
# controller with a USB tablet. No network, no writable disk.
#   --fixed-clock  time comes from instruction counts (-icount) and the RTC
#                  starts at a fixed date, so the run does not depend on host
#                  speed or wall time.
#   --until M      stop as soon as every given marker has appeared in the log
#                  (SECONDS is then the upper bound).
set -euo pipefail
iso="${1:?usage: run.sh ISO LOG SECONDS [--fixed-clock] [--until MARKER]...}"
log="${2:?}"
secs="${3:?}"
shift 3
cpu=max,+pku
network=0
gateway=
args=(-M q35 -m 512M -cdrom "$iso"
      -device nec-usb-xhci,id=xhci -device usb-tablet,bus=xhci.0
      -nic none -serial "file:$log" -display none -no-reboot -no-shutdown)
until_markers=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --fixed-clock) args+=(-icount shift=4,sleep=off -rtc base=2026-01-01T10:00:00,clock=vm); shift ;;
    --until) until_markers+=("${2:?--until needs a marker}"); shift 2 ;;
    --cpu) cpu="${2:?--cpu needs a model}"; shift 2 ;;
    --nvme-image)
      args+=(-drive "file=${2:?--nvme-image needs a raw image},format=raw,if=none,id=nvme0,snapshot=on"
             -device nvme,drive=nvme0,serial=SEXOS-CHECK); shift 2 ;;
    --network) network=1; shift ;;
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
args+=(-cpu "$cpu")
if (( network )); then
  backend=user,id=net0,restrict=on
  [[ -z "$gateway" ]] || backend+=",host=$gateway"
  args+=(-netdev "$backend" -device e1000,netdev=net0)
fi

if [[ -e "$log" ]]; then
  echo "choose a new serial log; existing evidence is preserved" >&2
  exit 2
fi
qemu-system-x86_64 "${args[@]}" &
qemu=$!
trap 'kill "$qemu" 2>/dev/null || true' EXIT
deadline=$((SECONDS + secs))
while kill -0 "$qemu" 2>/dev/null && (( SECONDS < deadline )); do
  if (( ${#until_markers[@]} )) && [[ -f "$log" ]]; then
    all=1
    for m in "${until_markers[@]}"; do
      grep -a -q -F -- "$m" "$log" || { all=0; break; }
    done
    (( all )) && break
  fi
  sleep 0.5
done
if ! kill -0 "$qemu" 2>/dev/null; then
  wait "$qemu" || { rc=$?; echo "qemu exited with $rc" >&2; exit "$rc"; }
fi
kill "$qemu" 2>/dev/null || true
wait "$qemu" 2>/dev/null || true
trap - EXIT
