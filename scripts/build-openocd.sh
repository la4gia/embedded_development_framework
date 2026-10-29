#!/usr/bin/env bash
#
# build-openocd.sh — Build OpenOCD from source with STM32N6 target support
#
# Debian's packaged OpenOCD (0.12.0) predates tcl/target/stm32n6x.cfg, which is
# needed to debug the stm32n6570-dk board. Builds into external/openocd/
# (git-ignored, same convention as fetch-deps.sh). Idempotent by default;
# pass --force to rebuild.
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EXTERNAL_DIR="$PROJECT_ROOT/external"
PREFIX="$EXTERNAL_DIR/openocd"

# Pinned to a specific commit (not a tagged release yet) because
# tcl/target/stm32n6x.cfg has not shipped in an OpenOCD release.
# Re-pin by running: git ls-remote https://github.com/openocd-org/openocd.git master
OPENOCD_COMMIT="da3920b0a52dc2d394afb222c688dac7e57acc1b"

FORCE=0
if [[ "${1:-}" == "--force" ]]; then
    FORCE=1
fi

info() { printf "\033[1;34m[INFO]\033[0m %s\n" "$1"; }

if [[ -x "$PREFIX/bin/openocd" && $FORCE -eq 0 ]]; then
    info "openocd — already built at $PREFIX, skipping (use --force to rebuild)"
    exit 0
fi

DEPS=(libusb-1.0-0-dev libhidapi-dev libftdi1-dev autoconf automake libtool pkg-config texinfo)
MISSING=()
for pkg in "${DEPS[@]}"; do
    dpkg -s "$pkg" >/dev/null 2>&1 || MISSING+=("$pkg")
done
if [[ ${#MISSING[@]} -gt 0 ]]; then
    info "installing missing build deps: ${MISSING[*]}"
    sudo apt-get update
    sudo apt-get install -y "${MISSING[@]}"
fi

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

info "openocd @ $OPENOCD_COMMIT — cloning..."
git init -q "$TMPDIR/openocd"
git -C "$TMPDIR/openocd" fetch --depth 1 https://github.com/openocd-org/openocd.git "$OPENOCD_COMMIT"
git -C "$TMPDIR/openocd" checkout -q FETCH_HEAD
git -C "$TMPDIR/openocd" submodule update --init --depth 1 --recursive -q

info "openocd — bootstrapping + configuring (prefix=$PREFIX)..."
(cd "$TMPDIR/openocd" && ./bootstrap)
(cd "$TMPDIR/openocd" && ./configure --prefix="$PREFIX" --enable-stlink --enable-internal-jimtcl --disable-werror)

info "openocd — building (this takes a few minutes)..."
make -C "$TMPDIR/openocd" -j"$(nproc)"

info "openocd — installing to $PREFIX..."
rm -rf "$PREFIX"
make -C "$TMPDIR/openocd" install

info "openocd — done. Binary at $PREFIX/bin/openocd"
"$PREFIX/bin/openocd" --version
