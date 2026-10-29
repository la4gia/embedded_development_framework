# Project 1 — Traffic light state machine

Status: **Done, hardware-verified (2026-09-20).** Red PA1 (P1 pin 11), green PA2 (pin 14),
yellow PA3 (pin 13), each LED anode → 180 Ω → pin, cathode → GND. Pins verified free against
UM1472 Table 7 + schematic. 180 Ω = (2.9 V V_OH − 2 V Vf) / 5 mA, from DS8626 Table 49
(guaranteed V_OH at 8 mA, not the 25 mA absolute max in Table 12).

Bare-metal C, `stm32f407g-disc1` board, no new driver work — this project is about writing a proper
state machine, not about a new peripheral.

## What we're building

A real traffic-light sequencer — red → green → yellow → red, with distinct hold times
per state — instead of blinky's flat round-robin toggle. The point is the pattern: an
`enum` for states + a `switch` that decides both what to light and how long to hold,
which gets reused in later hobby-track projects (reaction-time game, IR remote, HC-SR04
ranger — anything with distinct behavioral states).

## Parts needed

- 1x red LED, 1x yellow LED, 1x green LED (3mm, from the kit)
- 3x current-limiting resistors (~220–330Ω — confirm what the kit shipped with, LEDs
  draw too much current straight off a 3.3V GPIO pin without one)
- Breadboard + jumper wires

## Code to create

- `apps/traffic_light/CMakeLists.txt` — same shape as `apps/blinky/CMakeLists.txt`:
  `add_firmware(NAME traffic_light BOARD ${BOARD} SOURCES src/main.c DRIVERS driver_gpio driver_rcc)`,
  then `add_flash_target(traffic_light ${BOARD})` / `add_debug_target(traffic_light ${BOARD})`.
- `apps/traffic_light/src/main.c` — the state machine itself, built on the existing
  `drivers/gpio` API (`gpio_init`, `gpio_set`, `gpio_clear` — see `apps/blinky/src/main.c:35-59`
  for the reference pattern of enabling the GPIO clock + configuring pins as outputs).

No new driver needed — `drivers/gpio` already has everything required.

## Config / tooling

No new OpenOCD config. `boards/stm32f407g-disc1/openocd.cfg` (ST-Link/SWD) is already
proven working via `blinky`. The only per-app "config" is the two `add_flash_target` /
`add_debug_target` calls in the new `CMakeLists.txt` above — they generate
`flash_traffic_light` / `debug_traffic_light` Ninja targets automatically, reusing the
same `openocd.cfg`.

## Open questions / next action

- **Pick 3 free GPIO pins** from the STM32F407G-DISC1 schematic for the three LEDs.
  Known-claimed pins to avoid: `GPIOD` 12-15 (onboard LEDs, see
  `boards/stm32f407g-disc1/include/board.h`), `PA0` (user button), `PA9`/`PA10` (ST-Link
  VCP UART), `PA13`/`PA14` (SWD). This hasn't been done yet — bring 3 candidate pins to
  the next session.
- These pin choices are project-specific breadboard wiring, not board-level facts, so
  they'll be defined in `apps/traffic_light/` (e.g. a local header or directly in
  `main.c`), not added to `boards/stm32f407g-disc1/include/board.h`.

## What this project teaches

- Reading a board schematic to find safe/free GPIO pins (new — prior apps only used
  pins `board.h` already specified)
- Ohm's-law resistor sizing for an LED on a 3.3V GPIO output (new, hardware-side)
- Structuring a state machine in C (`enum` + `switch`, timed transitions) — the
  reusable pattern for most of the rest of this hobby track
