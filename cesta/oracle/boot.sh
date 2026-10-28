#!/usr/bin/env bash
# Boot an ISO with the fixed clock until every milestone has appeared (at most
# 300 s), then record its observation.
#   cesta/oracle/boot.sh ISO OUT_PREFIX     → OUT_PREFIX.log, OUT_PREFIX.json
set -euo pipefail
iso="${1:?usage: boot.sh ISO OUT_PREFIX}"
out="${2:?usage: boot.sh ISO OUT_PREFIX}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo="$(git -C "$here" rev-parse --show-toplevel)"
until=()
while IFS= read -r m; do [[ -n "$m" ]] && until+=(--until "$m"); done < "$here/milestones.txt"
"$here/run.sh" "$iso" "$out.log" 300 --fixed-clock "${until[@]}" 2>/dev/null
python3 -I "$here/oracle.py" observe "$out.log" --checker "$repo/scripts/check_bootgraph_log.py" > "$out.json"
