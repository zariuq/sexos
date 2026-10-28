#!/usr/bin/env bash
# Build the upstream Rust system at REV and keep its ISO.
#   cesta/oracle/reference/build.sh REV OUT_DIR LIMINE_DIR [TOOLCHAIN] [LOCKFILE]
# The source is exported with git archive (no hooks, no .git). Where unprivileged
# namespaces are available the build runs in bubblewrap in two phases: `cargo
# fetch` with network (it runs no crate code), then the upstream build with no
# network, the system directories and toolchain read-only, the export and its
# own CARGO_HOME writable, and no home directory. Otherwise it runs once with a
# cleared environment and HOME/CARGO_HOME inside OUT_DIR; the code it executes
# (three build scripts, two build.rs files, crates.io dependencies) was read
# first. With LOCKFILE, dependency versions come from that Cargo.lock, and the
# build fails if cargo had to change it.
# Outputs in OUT_DIR: sexos-REV.iso, cargo-locks.tar, build.log, manifest.txt.
set -euo pipefail
rev="${1:?usage: build.sh REV OUT_DIR LIMINE_DIR [TOOLCHAIN]}"
out="$(realpath -m "${2:?}")"
limine="$(realpath "${3:?}")"
toolchain="${4:-nightly-2026-10-21}"
lockfile="${5:+$(realpath "$5")}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo="$(git -C "$here" rev-parse --show-toplevel)"
full="$(git -C "$repo" rev-parse --verify "$rev^{commit}")"
epoch="$(git -C "$repo" log -1 --format=%ct "$full")"   # fixes the ISO's timestamps
tc="${RUSTUP_HOME:-$HOME/.rustup}/toolchains/$toolchain-x86_64-unknown-linux-gnu"
[[ -x "$tc/bin/cargo" ]] || { echo "missing toolchain $tc (rustup toolchain install $toolchain --component rust-src)"; exit 1; }

mkdir -p "$out"
rm -rf "$out/src" "$out/cargo-home"
mkdir -p "$out/src" "$out/cargo-home"
git -C "$repo" archive "$full" -- . ':(exclude)tools/sex-debug/target_host' | tar -x -C "$out/src"
[[ -n "$lockfile" ]] && cp "$lockfile" "$out/src/Cargo.lock"

proxy_env=()
for v in http_proxy https_proxy all_proxy no_proxy HTTP_PROXY HTTPS_PROXY ALL_PROXY NO_PROXY; do
  [[ -n "${!v:-}" ]] && proxy_env+=(--setenv "$v" "${!v}")
done

run=(bash -c 'ln -s "$LIMINE_DIR" limine && bash scripts/entrypoint_build.sh')
sandbox=(bwrap --die-with-parent --unshare-all --new-session
  --ro-bind /usr /usr --symlink usr/bin /bin --symlink usr/lib /lib --symlink usr/lib64 /lib64
  --ro-bind /etc /etc --proc /proc --dev /dev --tmpfs /tmp
  --ro-bind "$tc" /toolchain --ro-bind "$limine" /limine
  --bind "$out/src" /src --bind "$out/cargo-home" /cargo-home
  --chdir /src --clearenv
  --setenv PATH /toolchain/bin:/usr/bin --setenv HOME /tmp --setenv CARGO_HOME /cargo-home
  --setenv LIMINE_DIR /limine --setenv LANG C.UTF-8 --setenv SOURCE_DATE_EPOCH "$epoch")
if bwrap --unshare-all --ro-bind /usr /usr --symlink usr/bin /bin --symlink usr/lib /lib \
     --symlink usr/lib64 /lib64 true 2>/dev/null; then
  isolation="bwrap: fetch with network, build without"
  dns=()
  [[ -d /run/systemd/resolve ]] && dns=(--ro-bind /run/systemd/resolve /run/systemd/resolve)
  "${sandbox[@]}" --share-net "${dns[@]}" "${proxy_env[@]}" bash -c '
      target=$(sed -n "s/^target *= *\"\(.*\)\"/\1/p" sexos_build_spec.toml)
      cargo fetch && cargo fetch -Z build-std=core,compiler_builtins,alloc --target "$target"' \
    > "$out/fetch.log" 2>&1 || { tail -20 "$out/fetch.log"; exit 1; }
  "${sandbox[@]}" --setenv CARGO_NET_OFFLINE true "${run[@]}" \
    > "$out/build.log" 2>&1 || { tail -40 "$out/build.log"; exit 1; }
else
  isolation="cleared environment (namespaces unavailable)"
  mkdir -p "$out/home"
  proxy_vars=()
  for ((i = 0; i < ${#proxy_env[@]}; i += 3)); do proxy_vars+=("${proxy_env[i+1]}=${proxy_env[i+2]}"); done
  (cd "$out/src" && env -i PATH="$tc/bin:/usr/bin" HOME="$out/home" CARGO_HOME="$out/cargo-home" \
     LIMINE_DIR="$limine" LANG=C.UTF-8 SOURCE_DATE_EPOCH="$epoch" "${proxy_vars[@]}" "${run[@]}") \
    > "$out/build.log" 2>&1 || { tail -40 "$out/build.log"; exit 1; }
fi

if [[ -n "$lockfile" ]] && ! cmp -s "$lockfile" "$out/src/Cargo.lock"; then
  echo "cargo changed the given lockfile" >&2; exit 1
fi
cp "$out/src/sexos-v1.0.0.iso" "$out/sexos-${full:0:12}.iso"
(cd "$out/src" && find . -name Cargo.lock -not -path './target/*' -print0 | tar --null -cf "$out/cargo-locks.tar" -T -)
{
  echo "rev=$full"
  echo "isolation=$isolation"
  if [[ -n "$lockfile" ]]; then echo "lockfile=given $(sha256sum < "$lockfile" | cut -c1-16)"; else echo "lockfile=resolved"; fi
  "$tc/bin/rustc" --version
  echo "limine $(git -C "$limine" log -1 --format='%H %cs')"
  (cd "$out" && sha256sum "sexos-${full:0:12}.iso")
  (cd "$out/src/iso_root" && find . -type f -print0 | sort -z | xargs -0 sha256sum)
} > "$out/manifest.txt"
echo "$out/sexos-${full:0:12}.iso"
