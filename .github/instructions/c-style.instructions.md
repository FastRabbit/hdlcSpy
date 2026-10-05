---
description: C coding conventions for the hdlcspy_fw Pico 2 firmware sources under fw/src.
applyTo: 'fw/src/**/*.{c,h}'
---

# C Style for fw/src

Target is C11 on the Pico 2 (RP2350, Cortex-M33), built via the Pico SDK's
default `arm-none-eabi-gcc` flags (`-O3`/`-Os` release, warnings on). This
firmware runs on bare metal with no RTOS — a single `main()` loop — so the
constraints below matter more than on a hosted build.

## Embedded constraints

* **No blocking in a clipilot command handler.** `cli_poll()` and the LED tick
  both run from the same `main()` loop; a handler that calls `sleep_ms()` or
  spins freezes the console and the LEDs for its duration. Record intent
  (e.g. a mode enum) and let the main loop act on it, the way `led_set_mode()`
  hands off to `led_driver_tick()`.
* Prefer the Pico SDK's non-blocking primitives (`stdio_usb_connected()`,
  `getchar_timeout_us(0)`, `to_ms_since_boot(get_absolute_time())`) over
  anything that can block waiting for I/O or a timer.
* Command tables (`cli_cmd_t`, `cli_arg_spec_t`) are `static const` — no
  mutable globals for configuration data. Runtime state (LED mode, timing)
  lives in a small file-static struct array sized by a `#define` constant
  (see `led.c`'s `leds[LED_COUNT]`), not `malloc`.
* Use board header macros (`PICO_DEFAULT_LED_PIN`, etc.) instead of hardcoding
  pin numbers that already have an SDK-provided name — they're available in
  every source file automatically via the SDK's board-header include, no
  extra `#include` needed.

## Formatting

* 4 spaces, no tabs.
* K&R braces; braces on every `if`/`for`/`while`, including one-liners.
* Declarations at the top of a block, separated from statements by a blank
  line (matches the existing `led.c`/`main.c` style).
* `/* ... */` comments. Doxygen-style `/** ... */` on public declarations in
  headers (see `led.h`).
* Comment only what needs clarifying — the *why*, not a restatement of the
  code.

## Types and conversions

* Use `uint32_t` for GPIO pin numbers, millisecond timestamps and periods —
  matches what `to_ms_since_boot()` and `gpio_init()` expect.
* Suffix unsigned literals with `u` when they meet an unsigned operand:
  `for (i = 0u; i < LED_COUNT; i++)`.
* Compare pointers/values explicitly: `if (led->state)` for `bool` is fine,
  but `if (p != NULL)` for pointers, not `if (p)`.
* Timestamp comparisons that must survive wraparound use signed subtraction:
  `if ((int32_t)(now_ms - led->next_ms) >= 0)`, not `now_ms >= led->next_ms`.

## Functions

* Use `(void)` for empty parameter lists, never `()`.
* Silence deliberately unused parameters with `(void)name;` at the top of the
  body — clipilot handlers routinely ignore `user`.
* Keep static helpers `static`; only functions declared in a module's header
  (e.g. `led_driver_init`, `led_cli_command`) are non-static.

## clipilot command modules

Follow the pattern in `led.c`/`led.h` rather than inventing a new shape — see
the `fw-architecture` skill and the `fw-cli-module` custom agent for the full
recipe (one module, one `cli_cmd_t` singleton, registered through
`app_command_sources[]` in `main.c`).

## Naming

* Feature modules: `snake_case` prefix matching the file (`led_driver_init`,
  `led_set_mode`, `led_mode_name`).
* clipilot command objects: `<module>_cli_command` (e.g. `led_cli_command`).
* Types: `_t` suffix (`led_mode_t`, matching clipilot's own `cli_cmd_t` style).
* Macros/enumerators: `SCREAMING_SNAKE_CASE`, module-prefixed
  (`LED_MODE_BLINK_SLOW`, `LED_COUNT`).
