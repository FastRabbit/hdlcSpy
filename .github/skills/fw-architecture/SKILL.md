---
name: fw-architecture
description: Architecture, invariants and extension recipes for the hdlcspy_fw Pico 2 firmware under fw/. Use when adding or changing console commands, LED behavior, or anything touching fw/src or fw/CMakeLists.txt.
---

# fw/ Architecture

`hdlcspy_fw` is a Pico SDK application for the Raspberry Pi Pico 2 (RP2350)
that exposes a [clipilot](https://github.com/FastRabbit/clipilot) command
console over the native USB CDC virtual serial port, plus a small LED driver.

## Module map

| File | Responsibility |
|---|---|
| `fw/CMakeLists.txt` | `PICO_BOARD pico2` project definition; builds `hdlcspy_fw` from `src/*.c`; links `pico_stdlib`, `pico_rand`, `clipilot::clipilot`; `add_subdirectory(external/clipilot)`. |
| `fw/pico_sdk_import.cmake` | Standard Pico SDK import boilerplate (unmodified from upstream template). |
| `fw/src/main.c` | USB CDC stdio bring-up, connect banner, the `app_commands[]` registry, and the main loop (`cli_poll()` + `led_driver_tick()`). |
| `fw/src/led.c` / `fw/src/led.h` | LED driver: GPIO init, non-blocking mode state machine, and the `led` clipilot command module. |
| `fw/external/pico-sdk` | Vendored Pico SDK submodule (branch `master`), with nested `lib/tinyusb`, `lib/mbedtls`, `lib/cyw43-driver`, `lib/btstack`, `lib/lwip` at depth 1. |
| `fw/external/clipilot` | Vendored clipilot CLI library submodule. See its own `clipilot-architecture` skill (in that submodule's `.github/skills/`) for the library's internals. |

Data flow in `main()`: `stdio_usb_connected()` toggling drives
`print_banner()` + `console_init()` on (re)connect; every loop iteration polls
`cli_poll(&cli)` (only while connected) and unconditionally calls
`led_driver_tick(to_ms_since_boot(get_absolute_time()))` so LED patterns keep
running even with nothing attached.

## Hard invariants

1. **Console is USB CDC only.** `pico_enable_stdio_usb(hdlcspy_fw 1)` /
   `pico_enable_stdio_uart(hdlcspy_fw 0)` in `CMakeLists.txt`. Don't
   reintroduce physical UART stdio without an explicit request.
2. **Handlers never block.** See the `c-style.instructions.md` note — a
   clipilot handler and the LED tick share the same loop iteration.
3. **`cfg.commands` must be one real contiguous `cli_cmd_t` array.** clipilot's
   `cli_lookup()` indexes `table[i]`, so per-module command objects from
   different translation units cannot be listed directly; see "Adding a
   console command" below for why `app_commands[]` exists.
4. **LEDs keep working without a host attached.** `led_driver_tick()` is
   called every loop iteration unconditionally, not gated on
   `stdio_usb_connected()`.
5. **Build before committing.** Every change under `fw/` must be verified with
   a real `cmake`/`ninja` build producing `hdlcspy_fw.uf2` — see the
   `fw-build-test` skill.

## Adding a console command

Create a new module following `led.c`/`led.h`:

```c
/* led.h */
#include "cli.h"
extern const cli_cmd_t led_cli_command;

/* led.c */
static int cmd_led_on(cli_t *cli, const cli_args_t *args, void *user) { ... }

static const cli_cmd_t led_subs[] = {
    { "on", "steady on", NULL, 0u, led_which_args, 1u, cmd_led_on },
    /* ... */
};

const cli_cmd_t led_cli_command = {
    "led", "LED control", led_subs,
    sizeof(led_subs) / sizeof(led_subs[0]), NULL, 0u, NULL
};
```

Then register it with **one line** in `fw/src/main.c`'s registry:

```c
static const cli_cmd_t *const app_command_sources[] = {
    &led_cli_command,
    &your_new_cli_command,
};
```

`app_commands_init()` copies each singleton into the real `app_commands[]`
array at startup (`app_commands[i] = *app_command_sources[i];`) — this is a
**runtime** copy, not a static initializer, because C rejects
`static const cli_cmd_t commands[] = { led_cli_command, other_cli_command };`
with *"initializer element is not constant"*: copying an `extern const`
struct's value into static storage is not a constant expression in C, even
though taking its address is. Don't try to work around this by inlining the
command table directly in `main.c` instead — keep the table with its feature
module and let the registry do the assembly.

Also add the new `.c` file to `add_executable(hdlcspy_fw ...)` and any new
Pico SDK library it needs to `target_link_libraries(...)` in
`fw/CMakeLists.txt` (most GPIO/timing code needs nothing extra —
`pico_stdlib` already mirrors in `hardware_gpio`, `pico_time`, etc.; `pico_rand`
was the one addition needed so far, for `led.c`'s traffic-flicker mode).

## Adding a new LED mode / hardware feature

Keep the pattern used by `led.c`: a `typedef enum` for modes, a small
file-static struct array (`pin`, `mode`, `state`, `next_ms`, ...), an
`xxx_driver_init()` that sets up GPIO once, and an `xxx_driver_tick(now_ms)`
state machine called every main-loop iteration. Expose setters
(`led_set_mode`) for the CLI layer to call rather than letting command
handlers manipulate GPIO directly — keeps the hardware logic testable/reusable
independent of the console.

## Learnings

* **`cfg.commands` requires one contiguous array — discovered the hard way.**
  The first attempt at a second module would have been
  `static const cli_cmd_t commands[] = { led_cli_command };` directly in
  `main.c`. That compiles for exactly one entry only by accident (it's still
  a non-constant initializer, caught immediately as a build error) — see
  "Adding a console command" above for the actual fix
  (`app_command_sources[]` + runtime copy into `app_commands[]`).
* **Board header macros need no include.** `PICO_DEFAULT_LED_PIN` resolved
  correctly in `led.c` with no explicit `#include` for the board header —
  the Pico SDK's CMake machinery injects it via a compiler `-include` flag
  for every target source file once `PICO_BOARD` is set.
* **`stdio_usb_connected()` state changes drive re-init, not a one-shot
  `main()` setup.** The console (and banner) are (re)created every time the
  host (re)opens the CDC port, because anything written before that is
  silently discarded and a terminal attached long after boot still deserves
  a prompt. `led_driver_init()`/`app_commands_init()`, by contrast, run once
  at boot since LED state and the command registry don't depend on the host
  connection.
* **`putchar_raw()`, not `putchar()`, for the clipilot sink.** `cli_putc()`
  already expands `'\n'` to `"\r\n"` when `cfg.crlf` is set, so the sink must
  write the byte as-is; `putchar()` performs its own translation and doubles
  the carriage return.
