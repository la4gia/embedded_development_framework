#!/usr/bin/env bash
#
# flash.sh — Flash firmware to a board via OpenOCD
#
# Usage: ./scripts/flash.sh <board-name> [app-name]
#   board-name: e.g., stm32f407g-disc1
#   app-name:   defaults to "blinky"
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <board-name> [app-name]"
    echo ""
    echo "Available boards:"
    ls -1 "$PROJECT_ROOT/boards/" | grep -v CMakeLists
    exit 1
fi

BOARD="$1"
APP="${2:-blinky}"

# Locate the ELF file
ELF=""
for candidate in \
    "$PROJECT_ROOT/build/$BOARD/apps/$APP/$APP" \
    "$PROJECT_ROOT/build/$BOARD/$APP.elf" \
    "$PROJECT_ROOT/build/$BOARD/apps/$APP/${APP}.elf"; do
    if [[ -f "$candidate" ]]; then
        ELF="$candidate"
        break
    fi
done

if [[ -z "$ELF" ]]; then
    echo "Error: Cannot find $APP.elf for board $BOARD"
    echo "Expected at: build/$BOARD/apps/$APP/$APP.elf"
    echo "Have you built it? Run:"
    echo "  cmake -B build/$BOARD -DBOARD=$BOARD -G Ninja"
    echo "  cmake --build build/$BOARD"
    exit 1
fi

# Locate OpenOCD config
OPENOCD_CFG="$PROJECT_ROOT/boards/$BOARD/openocd.cfg"
if [[ ! -f "$OPENOCD_CFG" ]]; then
    echo "Error: No openocd.cfg found at $OPENOCD_CFG"
    exit 1
fi

echo "Flashing: $ELF"
echo "Board:    $BOARD"
echo "Config:   $OPENOCD_CFG"
echo ""

openocd \
    -f "$OPENOCD_CFG" \
    -c "program \"$ELF\" verify reset exit"
