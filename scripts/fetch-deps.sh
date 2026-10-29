#!/usr/bin/env bash
#
# fetch-deps.sh — Fetch CMSIS headers, startup files, and SVD files
#
# Downloads into external/ (git-ignored). Idempotent by default;
# pass --force to re-download everything.
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EXTERNAL_DIR="$PROJECT_ROOT/external"

# Pinned versions
CMSIS6_TAG="v6.3.0"
CMSIS_F4_TAG="v2.6.11"
CMSIS_N6_TAG="v1.4.0"
CMSIS_SVD_COMMIT="master"

FORCE=0
if [[ "${1:-}" == "--force" ]]; then
    FORCE=1
fi

info() { printf "\033[1;34m[INFO]\033[0m %s\n" "$1"; }
warn() { printf "\033[1;33m[WARN]\033[0m %s\n" "$1"; }

clone_shallow() {
    local repo="$1"
    local tag="$2"
    local dest="$3"
    local name="$(basename "$repo")"

    if [[ -d "$dest" && $FORCE -eq 0 ]]; then
        info "$name — already present, skipping (use --force to re-fetch)"
        return
    fi

    rm -rf "$dest"
    info "$name @ $tag — cloning..."
    git clone --depth 1 --branch "$tag" "https://github.com/$repo.git" "$dest" --quiet
    rm -rf "$dest/.git"
    info "$name — done"
}

# --- CMSIS Core (ARM) ---
if [[ -d "$EXTERNAL_DIR/cmsis-core" && $FORCE -eq 0 ]]; then
    info "cmsis-core — already present, skipping"
else
    info "cmsis-core — fetching CMSIS_6 Core headers..."
    TMPDIR="$(mktemp -d)"
    git clone --depth 1 --branch "$CMSIS6_TAG" \
        "https://github.com/ARM-software/CMSIS_6.git" "$TMPDIR/cmsis6" --quiet
    rm -rf "$EXTERNAL_DIR/cmsis-core"
    mkdir -p "$EXTERNAL_DIR/cmsis-core"
    cp -r "$TMPDIR/cmsis6/CMSIS/Core/Include/"* "$EXTERNAL_DIR/cmsis-core/"
    rm -rf "$TMPDIR"
    info "cmsis-core — done"
fi

# --- STM32F4 CMSIS Device ---
clone_shallow "STMicroelectronics/cmsis-device-f4" "$CMSIS_F4_TAG" "$EXTERNAL_DIR/cmsis-device-f4"

# --- STM32N6 CMSIS Device ---
clone_shallow "STMicroelectronics/cmsis-device-n6" "$CMSIS_N6_TAG" "$EXTERNAL_DIR/cmsis-device-n6"

# --- STM32C5 CMSIS Device ---
# NOTE: Not yet publicly available from STMicroelectronics.
# When released, add:
#   clone_shallow "STMicroelectronics/cmsis-device-c5" "<tag>" "$EXTERNAL_DIR/cmsis-device-c5"
warn "cmsis-device-c5 — not yet available from ST. NUCLEO-C562RE headers must be added manually."

# --- SVD Files ---
SVD_DIR="$EXTERNAL_DIR/svd"
if [[ -d "$SVD_DIR" && $FORCE -eq 0 ]]; then
    info "svd — already present, skipping"
else
    info "svd — fetching SVD files..."
    TMPDIR="$(mktemp -d)"
    git clone --depth 1 "https://github.com/cmsis-svd/cmsis-svd-data.git" "$TMPDIR/cmsis-svd-data" --quiet
    rm -rf "$SVD_DIR"
    mkdir -p "$SVD_DIR"

    # Copy relevant SVD files
    SVD_SRC="$TMPDIR/cmsis-svd-data/data/STMicro"
    for svd in STM32F407.svd STM32F40x.svd; do
        if [[ -f "$SVD_SRC/$svd" ]]; then
            cp "$SVD_SRC/$svd" "$SVD_DIR/"
            info "  copied $svd"
        fi
    done

    # N6 SVD might not be in the community repo yet
    if ls "$SVD_SRC"/STM32N6*.svd &>/dev/null; then
        cp "$SVD_SRC"/STM32N6*.svd "$SVD_DIR/"
        info "  copied STM32N6 SVDs"
    else
        warn "  STM32N6 SVD not found in community repo — check ST's official releases"
    fi

    rm -rf "$TMPDIR"
    info "svd — done"
fi

echo ""
info "All dependencies fetched to: $EXTERNAL_DIR/"
info "Contents:"
ls -1 "$EXTERNAL_DIR/" | grep -v .gitkeep | sed 's/^/  /'
