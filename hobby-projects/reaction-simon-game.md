# Project 3 — Reaction-time game → Simon-says

Status: **Phase A complete, hardware-verified 2026-10-10; phase B in progress — input layer
(`apps/simon_game` ISR: both-edge EXTI on PB12-15, per-button RELEASED/PRESSED state, per-button
`last_edge_ts`, 50 ms window gating only the press) hardware-verified 2026-10-18 on all four
buttons incl. chord pairs. ISR -> `main` handoff done
2026-10-20 (`pressed[4]` flag per button, clear-then-act; `main` toggles the matching on-board LED
GPIOD12-15, user-verified). Next: enum mapping for button/LED slots, hardcoded `play_sequence()`,
stale-flag flush between playing/listening, random step generation, compare logic, the "2+ presses =
loss" rule (needs a press timestamp). Wrap-up topic requested by user: sleep-until-interrupt
(`__WFI()`) in the main loop.** Two phases in one app track: a
single-button reaction-timer first (the EXTI/interrupt warm-up), then extended into a
4-button Simon-says memory game once basic interrupt handling is solid. Decided
2026-09-21: build both, in that order, rather than picking one.

`apps/reaction_game` created — `PA1` (LED), `PA2` (button, pulled to GND externally,
`GPIO_PULL_UP` internal so idle reads high / `EXTI_TRIG_FALLING` on press). GPIO +
EXTI config written and builds clean (`cmake --build build/stm32f407g-disc1 --target
reaction_game`). `EXTI2_IRQHandler` + `volatile` flag handoff written 2026-09-24
(main loop toggles the LED per press as a smoke test). **Flashed + hardware-tested
2026-09-24:** ISR fires and hands off correctly
(confirmed at register level via OpenOCD), but only 7/10 presses visibly toggled → switch bounce (multiple falling edges per press, incl. on release).
Breadboard: button on PA2 (P1-14) ↔ GND, traffic-light green LED removed from PA2.

**Debounce done 2026-09-26, hardware-verified.** Bounce measured with `isr_count` +
OpenOCD `mdw` (press = 1 edge, release = 1 extra falling edge in 3/3 quick trials, 7 in one
fumbled release). `main` now waits for 50 consecutive 1 ms high readings before clearing the
flag. Test: 10 presses incl. slow releases → LED toggled exactly 10 times, `isr_count` = 22.
The 1 ms delay constant was measured, not guessed: `delay()` costs 8 cycles/iteration
(`DWT->CYCCNT`), so 21,000 iterations = 1 ms.

**Reaction-time logic done 2026-09-29, hardware-verified.** `WAITING`/`ARMED` state machine in
`apps/reaction_game/src/main.c`: random 1-4 s dark wait (seeded from `DWT->CYCCNT`), LED on +
`start_ts`, `armed`/`hit` handshake with the ISR (timestamps taken in the ISR), `elapsed_ts =
end_ts - start_ts`. Verified via OpenOCD `mdw`: `end_ts - start_ts == elapsed_ts` exactly; fast
press ~205 ms; early press during the dark wait ignored, next round normal. Result is only
readable with a debugger so far. Open: first round after reset is predictable (CYCCNT at
boot is deterministic); the old `button_pressed` debounce block is being removed in the rewrite.

**Result output done 2026-10-10, hardware-verified.** UART TX on USART3 (PD8/PD9, AF7,
115200, `screen /dev/ttyUSB0 115200`); `send_digit()` int-to-text; integer-only cycles -> ms
via `CYCLES_PER_MS = BOARD_SYSCLK_FREQ / 1000`; 10 s round deadline (`CAP_MS`,
`CAP_IN_CYCLES`, polled in `STATE_ARMED`; round abandoned, ISR disarmed, no output). Three
cases checked: (A) press after LED prints a ms value, (B) no press for 10 s -> LED off, new
round, (C) press after timeout and before the next LED is ignored. The cap is a *deadline*,
not a post-hoc filter, because `end - start` aliases past 25.57 s.

Bare-metal C, `stm32f407g-disc1` board. First app in the repo to use `drivers/exti` — every prior
app has been polling-only (blinky, traffic_light).

## What we're building

**Phase A — reaction timer:** an LED lights after a random-ish delay; the player
presses a button as fast as possible; a busy-wait loop counter (same `delay()`-style
counting as `traffic_light`, just counting up instead of down) measures elapsed time
between LED-on and the button's EXTI interrupt firing. Single button, single LED,
one EXTI line. The point of this phase is entirely: configure one EXTI line, write
one ISR, and get from "interrupt fires" to "main loop knows about it" cleanly
(a `volatile` flag set in the ISR, read in `main`).

**Phase B — Simon-says:** 4 buttons + 4 LEDs (reuse the traffic-light-style
LED-per-color idea, but 4 channels instead of 3). Device plays a growing sequence of
LED flashes; player repeats it back on the matching buttons; wrong press or timeout
ends the game (flash all LEDs / reset). This phase's new content is mostly C, not
hardware: an array holding the growing sequence, a "device is playing" vs "waiting for
input" mode, and the loop that appends one step and replays the whole sequence each
round — same enum-driven state-machine shape as `traffic_light`, but with data
(the sequence array) driving what the state machine outputs instead of a fixed cycle.

## Parts needed

- Phase A: 1 pushbutton, 1 LED (kit has 5x pushbuttons, red/green/yellow 3mm LEDs)
- Phase B: 4 pushbuttons, 4 LEDs (any 4 of: red, yellow, green 3mm + one channel of
  the RGB LED, or just reuse 4 discrete LEDs if there are enough — confirm what's
  left unused from `traffic_light`'s wiring)
- Breadboard + jumper wires + resistors (LED resistor math is already solved in
  `hobby-projects/traffic-light.md` §1 — reuse it, don't re-derive)
- Pushbuttons need a pull resistor (internal pull-up/down via `drivers/gpio`'s
  `GPIO_PULL_*`, or external — decide when wiring, same "read the datasheet section
  on internal pull config" exercise as before)

## Code to create

- `apps/reaction_game/CMakeLists.txt` and `apps/simon_game/CMakeLists.txt` (or one
  app that grows from A to B — open question below) — same shape as
  `apps/traffic_light/CMakeLists.txt`, but `DRIVERS driver_gpio driver_rcc driver_exti`.
- `src/main.c` for each phase, built on:
  - existing `drivers/gpio` (`gpio_init`, `gpio_set`, `gpio_clear`, `GPIO_PULL_*`)
  - existing `drivers/exti` (`EXTI_ConfigLine`, `EXTI_EnableIRQ`, `EXTI_SetPriority`,
    `EXTI_ClearPending`) — see `drivers/exti/include/exti.h` for the full API
  - a hand-written ISR: `void EXTI<N>_IRQHandler(void)` (exact name depends on which
    pin number gets picked — see open questions), which must call
    `EXTI_ClearPending()` for its pin and set a `volatile` flag for `main` to poll

No new driver needed — `drivers/exti` already exists and has everything required.
It hasn't been used by any app yet, so this is genuinely new despite not being new
driver *code*.

## Config / tooling

No new OpenOCD config, no new CMake helper macros — same `add_flash_target()` /
`add_debug_target()` pattern as every prior app.

## Open questions / next action

- **EXTI line-sharing constraint (read `drivers/exti/src/exti.c` first, don't take my
  word for it):** an EXTI line number is shared across *all* ports — `SYSCFG->EXTICR`
  picks which port's pin 3, say, drives EXTI line 3, but only one port can own line 3
  at a time. So N buttons need N *different pin numbers*, not just N different pins.
  Also: pins 0-4 each get their own IRQ handler (`EXTI0_IRQHandler` ...
  `EXTI4_IRQHandler`), but pins 5-9 share `EXTI9_5_IRQHandler` and pins 10-15 share
  `EXTI15_10_IRQHandler` — a shared handler has to check `EXTI->PR` to figure out
  which pin actually fired. For Simon-says' 4 buttons, picking pin numbers 0-4 (across
  any ports) avoids the shared-handler complexity entirely — worth doing deliberately
  for phase B rather than discovering the shared-IRQ case by accident.
- ~~Pick GPIO pins for phase A~~ — decided 2026-09-21: `PA1` (LED), `PA2` (button).
  Phase B's 4 buttons + 4 LEDs still open — same process (check `board.h`/schematic
  for claimed pins), and remember the pin-*number* 0-4 constraint above when picking.
- ~~One app or two?~~ Decided 2026-10-10: **two apps.** Phase B is a separate game in its
  own `apps/simon_game/`; `apps/reaction_game` stays as the finished phase A. Phase A is
  complete on its own and the repo's app-per-firmware-image convention fits two small apps.

- **Fix `EXTI_ClearPending` before phase B** (`drivers/exti/src/exti.c:78`): `PR |=` is a
  read-modify-write on a write-1-to-clear register — clears *every* pending line, not just
  the requested one (seen in the disassembly: ldr/orrs/str). Harmless with one line; loses
  interrupts with four. Should be a plain write of only the wanted bit.

## What this project teaches

- Interrupts vs polling: configuring a GPIO as an interrupt source (`SYSCFG->EXTICR`,
  `EXTI->IMR`, `EXTI->RTSR`/`FTSR`) instead of reading it in a loop
- Writing an ISR and the `volatile`-flag handoff pattern to main-loop code
- The EXTI line-sharing hazard (pin *number*, not pin, claims a line) — a real hazard
  class distinct from anything hit in `blinky`/`traffic_light`
- Extending the enum+switch state machine pattern (from `traffic_light`) to a
  data-driven sequence rather than a fixed cycle
