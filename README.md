# Embedded Development Framework

A small bare-metal firmware framework for STM32: register-level C, no HAL, no LL.
CMake + Ninja build, one source tree, multiple boards, and a handful of reusable
peripheral drivers that you link into per-application firmware.

It is meant to be read. The drivers are short, commented, and talk to the
registers directly, so you can use them as-is or as a reference for writing your own.

### Moving to [bare-metal-embedded-lab](https://github.com/la4gia/bare-metal-embedded-lab)

## Contents

- [Supported boards](#supported-boards)
- [Quick start](#quick-start)
- [How the framework works](#how-the-framework-works)
- [Drivers](#drivers)
- [Applications](#applications)
- [Adding an application](#adding-an-application)
- [Adding a board](#adding-a-board)
- [Adding or porting a driver](#adding-or-porting-a-driver)
- [Flashing and debugging](#flashing-and-debugging)
- [Known limitations](#known-limitations)
- [Toolchain and philosophy](#toolchain-and-philosophy)
- [License](#license)

## Supported boards

| Board | MCU | Core | Status |
|-------|-----|------|--------|
| `stm32f407g-disc1` | STM32F407VGT6 | Cortex-M4F | Builds; all apps and drivers supported |
| `stm32n6570-dk` | STM32N657xx | Cortex-M55 | `SystemInit()` (HSE → PLL1 → IC1/IC2), RCC driver, and `blinky` verified on hardware. Also builds `n6_clock_test` and `i2c_camera_test` |
| `nucleo-c562re` | STM32C562RE | Cortex-M33 | Placeholder. ST has not published the CMSIS device headers, so it does not configure yet |

The board name is the directory name under `boards/` and is what you pass as `-DBOARD=`.

## Quick start

Debian/Ubuntu (or WSL):

```bash
# 1. Install the toolchain (gcc-arm-none-eabi, gdb-multiarch, openocd, cmake, ninja, clangd, stlink-tools)
./scripts/setup-toolchain.sh

# 2. Fetch CMSIS headers and SVD files into external/ (git-ignored, pinned versions)
./scripts/fetch-deps.sh

# 3. Configure for a board
cmake -B build/stm32f407g-disc1 \
      -DBOARD=stm32f407g-disc1 \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-none-eabi.cmake \
      -G Ninja

# 4. Build everything, or one app
cmake --build build/stm32f407g-disc1
cmake --build build/stm32f407g-disc1 --target blinky

# 5. Flash
./scripts/flash.sh stm32f407g-disc1 blinky
# or: ninja -C build/stm32f407g-disc1 flash_blinky
```

Each board gets its own build directory (`build/<board>`). A build directory is
bound to one board; to target a different board, configure a different directory.

Build output for an app lands in `build/<board>/apps/<app>/`:

| File | What |
|------|------|
| `<app>` | ELF (no extension), used by GDB and OpenOCD |
| `<app>.bin`, `<app>.hex` | Raw and Intel-hex images |
| `<app>.map` | Linker map |

A size report and linker memory-usage summary print at the end of each link.

## How the framework works

### The layers

```
        apps/<app>              your main.c
           │  add_firmware(... DRIVERS driver_gpio driver_uart ...)
           ▼
   ┌────────────────┐  ┌────────────────┐  ┌────────────────┐
   │ drivers/<periph>│  │  lib/common    │  │ boards/<board> │
   │ driver_gpio ... │  │  lib_common    │  │ board_<name>   │
   └───────┬────────┘  └────────────────┘  └───────┬────────┘
           │ links                                 │ provides
           └──────────────► board ◄────────────────┘
                     startup file, SystemInit, linker script,
                     CMSIS include paths, CPU flags, board.h
```

- **`boards/<board>`**: everything hardware-specific that isn't a peripheral
  driver: the startup file and CMSIS device headers (from `external/`),
  `SystemInit()` (clock tree), the linker script, `board.h` (clock frequencies,
  pin names), and the OpenOCD config.
- **`drivers/<peripheral>`**: reusable peripheral drivers, each a `driver_<name>`
  static library with a small C interface in `include/`.
- **`lib/common`**: header-only helpers (`BIT()`, `ASSERT()`, barriers, IRQ
  enable/disable), exposed as the `lib_common` interface library.
- **`apps/<app>`**: firmware images. Each is one `add_firmware()` call plus your
  source.

### What the build does

1. **Toolchain file** (`cmake/toolchains/arm-none-eabi.cmake`) selects
   `arm-none-eabi-gcc`, sets `-mthumb`, warnings, `-ffunction-sections`,
   `--gc-sections`, newlib-nano, and the Debug/Release flags. The default build
   type is `Debug` (`-Og -g3`).
2. **Top-level `CMakeLists.txt`** validates `-DBOARD=`, then applies the CPU,
   FPU, and float-ABI flags for that board globally so every object file agrees
   on the ABI.
3. **`boards/CMakeLists.txt`** only adds the selected board's subdirectory.
   Unselected boards are never configured.
4. The board's `CMakeLists.txt` defines two things:
   - an `OBJECT` library with the startup `.s` and `system.c`
   - an `INTERFACE` library `board_<name_with_underscores>` that carries include
     paths, the family define (`STM32F407xx`, `STM32N657xx`), CPU flags, the
     `-T<linker script>` link option, and the object files above
5. **`add_firmware()`** (in `cmake/stm32.cmake`) creates the executable, links
   the board library, your requested drivers, and `lib_common`, and adds
   post-build steps that produce `.bin`, `.hex`, a size report, and a map file.
6. **`add_flash_target()` / `add_debug_target()`** (in `cmake/openocd.cmake`)
   create `flash_<app>` and `debug_<app>` targets that drive OpenOCD with the
   board's `openocd.cfg`.

### Writing portable application code

Apps include the framework headers plus the family CMSIS header. The family
define set by the board library tells you which one you are on:

```c
#include "board.h"      // resolved to the selected board's include/board.h
#include "gpio.h"
#include "rcc.h"

#if defined(STM32F407xx)
#include "stm32f407xx.h"
#elif defined(STM32N657xx)
#include "stm32n657xx.h"
#endif
```

Peripherals are passed to drivers as `void *` (for example `GPIOD`, `USART2`),
so driver headers never need to include a family header. Board-specific
facts (LED pins, clock frequencies) live in `board.h`:

```c
RCC_EnableGPIOClock(LED_PORT);
gpio_init(LED_PORT, LED_GREEN_PIN, &(gpio_config_t){ .mode = GPIO_MODE_OUTPUT });
gpio_toggle(LED_PORT, LED_GREEN_PIN);
uart_init(USART3, 115200, BOARD_PCLK1_FREQ);
```

### Per-family driver selection

Register layouts differ between STM32 families. GPIO is the same across all of
the ones used here, so there is a single `gpio.c`. RCC, UART, and I2C differ, so
each has one source file per family that implements the same header, and the
driver's `CMakeLists.txt` picks the file for the selected `BOARD` at configure
time:

```cmake
if(BOARD STREQUAL "stm32f407g-disc1")
    set(UART_SRC src/uart_f4.c)
elseif(BOARD STREQUAL "stm32n6570-dk")
    set(UART_SRC src/uart_n6.c)
...
```

An unmapped board is a hard configure error rather than a silent miss.

## Drivers

| Target | Header | Provides | F4 | N6 |
|--------|--------|----------|:--:|:--:|
| `driver_rcc` | `rcc.h` | `RCC_EnableGPIOClock`, `RCC_DisableGPIOClock`, `RCC_ResetGPIO` | yes | yes |
| `driver_gpio` | `gpio.h` | `gpio_init`, `gpio_set`, `gpio_clear`, `gpio_toggle`, `gpio_read` (set/clear via BSRR, atomic) | yes | yes |
| `driver_uart` | `uart.h` | `uart_init`, `uart_write_byte`, `uart_write`, `uart_read_byte`, `uart_data_available` (blocking) | yes | yes |
| `driver_exti` | `exti.h` | `EXTI_ConfigLine`, `EXTI_EnableIRQ`, `EXTI_DisableIRQ`, `EXTI_SetPriority`, `EXTI_ClearPending` | yes | no |
| `driver_i2c` | `i2c.h` | `i2c_init`, `i2c_write_reg`, `i2c_read_reg` (7-bit address, 16-bit register address) | no | yes |
| `lib_common` | `common.h` | `BIT`, `ARRAY_SIZE`, `UNUSED`, `ASSERT`, `DSB/ISB/DMB`, `enable_irq/disable_irq` | yes | yes |

`lib_common` is linked into every firmware automatically; request the others
with the `DRIVERS` argument. Include paths propagate, so linking
`driver_gpio` is enough to `#include "gpio.h"`.

GPIO example, using the config struct:

```c
gpio_config_t cfg = {
    .mode  = GPIO_MODE_OUTPUT,
    .otype = GPIO_OTYPE_PP,
    .speed = GPIO_SPEED_LOW,
    .pull  = GPIO_PULL_NONE,
    .af    = 0,               // alternate function number, used with GPIO_MODE_AF
};
gpio_init(GPIOD, 12, &cfg);
```

## Applications

| App | Boards | Drivers | What it does |
|-----|--------|---------|--------------|
| `blinky` | F4, N6 | gpio, rcc | Toggles the user LEDs |
| `traffic_light` | F4 | gpio, rcc | LED traffic-light state machine |
| `reaction_game` | F4 | gpio, rcc, exti, uart | Button/LED reaction timer, EXTI interrupt, result over UART |
| `simon_game` | F4 | gpio, rcc, exti, uart | Four-button Simon game (in progress): both-edge EXTI input with per-button debounce, ISR-to-main handoff |
| `n6_clock_test` | F4, N6 | none | Empty `main` for bring-up of `SystemInit()` under a debugger |
| `i2c_camera_test` | N6 | gpio, rcc, i2c | I2C register access to an IMX335 camera sensor |

The `hobby-projects/` directory holds write-ups for some of the apps above.

## Adding an application

1. Create `apps/<app-name>/` with a `CMakeLists.txt`:
   ```cmake
   add_firmware(
       NAME <app-name>
       BOARD ${BOARD}
       SOURCES src/main.c
       DRIVERS driver_gpio driver_rcc
   )
   add_flash_target(<app-name> ${BOARD})
   add_debug_target(<app-name> ${BOARD})
   ```
2. Write `apps/<app-name>/src/main.c`.
3. Register it in `apps/CMakeLists.txt`:
   ```cmake
   add_subdirectory(<app-name>)
   ```
   Apps are **not** discovered automatically. If your app needs a driver that
   only some boards provide, guard it the way `i2c_camera_test` does:
   ```cmake
   if(TARGET driver_i2c)
       add_subdirectory(i2c_camera_test)
   endif()
   ```
4. Re-run `cmake --build build/<board> --target <app-name>`.

An app that is only valid on some boards will fail to compile on the others.
Build specific targets (`--target <app>`), or guard the app as above, if you
build more than one board.

## Adding a board

1. Create `boards/<board-name>/` containing:
   - `CMakeLists.txt`: an `OBJECT` library (startup + `system.c`) and an
     `INTERFACE` library named `board_<board_name_with_underscores>`.
     `boards/stm32f407g-disc1/CMakeLists.txt` is the simplest template.
   - `include/board.h`: clock frequencies, pin and port names.
   - `src/system.c`: `SystemInit()`, called by the startup code before `main`.
   - `<mcu>.ld`: linker script (memory regions, vector table placement).
   - `openocd.cfg`: interface and target config.
2. Add the board's CPU/FPU flags to the `if(BOARD STREQUAL ...)` chain in the
   top-level `CMakeLists.txt`, and to the list in its "BOARD not specified" message.
   Keep `-mcpu` / `-mfpu` / `-mfloat-abi` identical in three places: that chain,
   the board's `OBJECT` library, and its `INTERFACE` library. A mismatch is an
   ABI hazard (hard vs. soft float, different FPU), not a build error.
3. Link the board object files through the `INTERFACE` library with
   `$<TARGET_OBJECTS:<board>_obj>`, **not** by naming the `OBJECT` library
   target. Linking an `OBJECT` library through an `INTERFACE` library only
   propagates includes, defines and flags, not the objects. The failure is
   silent: the build succeeds but the startup file, vector table and
   `SystemInit` are missing from the binary. After touching this, check
   `arm-none-eabi-size` and `nm` for `Reset_Handler`, and don't trust a clean build.
4. Add a branch for the new board in each driver `CMakeLists.txt` that selects a
   per-family source (`drivers/rcc`, `drivers/uart`, `drivers/i2c`).
5. Add the family's CMSIS device pack to `scripts/fetch-deps.sh`, and an SVD
   file if you want the peripheral view in the debugger.
6. Add a launch configuration to `.vscode/launch.json` (optional).
7. Build with `-DBOARD=<board-name>`.

## Adding or porting a driver

- **Same register layout across families** (like GPIO): one source file, one
  `add_library`. Take `void *` for the peripheral.
- **Different layout per family** (like UART): one `<periph>_<family>.c` per
  family implementing the same header, and the board `if/elseif` in the
  driver's `CMakeLists.txt`. Keep the header family-neutral; do not include a
  `stm32*.h` from it.
- Link `${BOARD_TARGET}` (computed as `board_<board_with_underscores>`) if the
  driver needs the CMSIS include paths. See `drivers/rcc/CMakeLists.txt`.

## Flashing and debugging

### Flash

```bash
./scripts/flash.sh <board> [app]        # app defaults to blinky
ninja -C build/<board> flash_<app>
```

Both programme the ELF through OpenOCD, then verify and reset.

### Debug, command line

Terminal 1:
```bash
ninja -C build/stm32f407g-disc1 debug_blinky     # starts OpenOCD, GDB server on :3333
```

Terminal 2:
```bash
gdb-multiarch build/stm32f407g-disc1/apps/blinky/blinky
(gdb) target remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) break main
(gdb) continue
```

### Debug, VSCodium / VS Code

`.vscode/launch.json` has a `cortex-debug` configuration per board. Install the
recommended extensions (`clangd` and `cortex-debug`). Peripheral registers show
up in the debug sidebar from the SVD files in `external/svd/`.

### STM32N6 needs a newer OpenOCD

The packaged OpenOCD (0.12) does not include `target/stm32n6x.cfg`. Build one
pinned to a known commit:

```bash
./scripts/build-openocd.sh              # installs to external/openocd/
export PATH="$PWD/external/openocd/bin:$PATH"
```

Both `flash.sh` and the CMake flash/debug targets call `openocd` from `PATH`,
so put the built binary first. The N6 config uses raw SWD (not `hla_swd`)
because the chip is debugged in TrustZone Secure state.

## Known limitations

- **`nucleo-c562re` does not configure.** There are no public CMSIS device
  headers for STM32C5 yet. In addition, `drivers/rcc/CMakeLists.txt` sets
  `I2C_SRC` (not `RCC_SRC`) for this board and `rcc_c5.c` / `i2c_c5.c` do not
  exist, so `driver_rcc` and `driver_i2c` have no sources.
- **`driver_exti` is F4-only.** `exti.h` includes `stm32f407xx.h` directly. A
  second family needs the header made family-neutral and a per-family source,
  like UART. As a result, `traffic_light`, `reaction_game` and `simon_game` do not build for N6.
- **`driver_i2c` has no F4 implementation.** The driver is skipped for that board.
- **N6 clock tree is minimal.** `SystemInit()` runs HSE → PLL1, then IC1 (CPU
  clock) and IC2 (bus clock, divided by 3). Other IC dividers and peripheral
  kernel clocks are not configured.
- Drivers are polled and blocking. There is no DMA, no RTOS, no interrupt-driven UART.
- Interrupt handlers go in your app. The startup file provides weak defaults
  that you override by defining a function with the vector's name.

## Toolchain and philosophy

- **Compiler:** `arm-none-eabi-gcc` from distro packages
- **Build:** CMake (3.20+) and Ninja
- **Debug:** OpenOCD + GDB (`gdb-multiarch`), or `cortex-debug` in an editor
- **Editor:** anything; `clangd` is configured from `compile_commands.json`
- **Style:** `.clang-format` (4-space indent), `.editorconfig`

Design choices:

- **No HAL or LL.** Registers are accessed directly so the code shows what the
  hardware is doing.
- **Bare-metal.** No RTOS unless you add one to an app.
- **Readable over clever.** Short, commented C that can serve as documentation.
- **No telemetry.** Editor settings disable it; the toolchain is open-source.

## License

Public domain, under [The Unlicense](LICENSE). Use it for anything.

Vendor material (CMSIS headers, SVD files, ST datasheets and reference manuals)
is **not** included in this repository. `scripts/fetch-deps.sh` downloads the
CMSIS and SVD files from their upstream repositories under their own licenses.

**One exception:** `apps/i2c_camera_test/src/imx335_regs.h` contains register
tables copied from the mainline Linux `imx335` driver
(`drivers/media/i2c/imx335.c`, GPL-2.0-only, Copyright (C) 2021 Intel
Corporation). That file stays under GPL-2.0-only, and so does a firmware image
built from it. The rest of the tree is unaffected.
