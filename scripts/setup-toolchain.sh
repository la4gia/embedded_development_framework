#!/usr/bin/env bash
#
# setup-toolchain.sh — Install ARM bare-metal development toolchain
# Target: Debian-based systems (Ubuntu, Debian, etc.)
#

set -euo pipefail

PACKAGES=(
    gcc-arm-none-eabi
    gdb-multiarch
    openocd
    cmake
    ninja-build
    clangd
    stlink-tools
    git
)

info() { printf "\033[1;34m[INFO]\033[0m %s\n" "$1"; }
warn() { printf "\033[1;33m[WARN]\033[0m %s\n" "$1"; }
err()  { printf "\033[1;31m[ERROR]\033[0m %s\n" "$1"; exit 1; }

if [[ $EUID -ne 0 ]]; then
    SUDO="sudo"
    info "Not running as root — will use sudo"
else
    SUDO=""
fi

info "Updating package lists..."
$SUDO apt-get update -qq

info "Installing packages:"
for pkg in "${PACKAGES[@]}"; do
    if dpkg -s "$pkg" &>/dev/null; then
        info "  $pkg — already installed"
    else
        info "  $pkg — installing..."
        $SUDO apt-get install -y -qq "$pkg"
    fi
done

echo ""
info "Toolchain versions:"
arm-none-eabi-gcc --version | head -1
cmake --version | head -1
ninja --version
openocd --version 2>&1 | head -1
clangd --version | head -1

echo ""
info "Setup complete. Run ./scripts/fetch-deps.sh next to pull CMSIS headers."
