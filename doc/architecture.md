# Architecture

## Repo layout

```
hdlcSpy/
├── hw/                   KiCad hardware project (schematic, PCB, datasheets)
│   ├── hdlcSpy/          KiCad project files (.kicad_pro/_sch/_pcb/_prl)
│   └── datasheets/       Component datasheets referenced by the schematic
├── fw/                   Pico SDK firmware application
│   ├── src/              Application sources (main.c, led.c/h, ...)
│   ├── external/         Vendored submodules: pico-sdk, clipilot
│   ├── CMakeLists.txt    Build definition (PICO_BOARD pico2)
│   └── pico_sdk_import.cmake
├── doc/                  This folder: project-level documentation
├── .github/
│   ├── agents/            Agent recipes (fw-builder, fw-reviewer, fw-cli-module)
│   ├── instructions/      Always-apply conventions (git workflow, C style)
│   └── skills/            On-demand runbooks (fw-architecture, fw-build-test)
└── CONTRIBUTING.md        Workflow entry point, cross-references the above
```

`hw/` and `fw/` are developed together but are independent build units:
`hw/` is a KiCad project with no build step beyond KiCad/fabrication tooling;
`fw/` is a standalone CMake project that only needs its own submodules
(`pico-sdk`, `clipilot`) to configure and build.

## Firmware runtime structure

The firmware is a single-threaded, non-blocking `main()` loop (see
`fw/src/main.c`):

1. `stdio_init_all()` brings up USB CDC (console over the Pico's USB port,
   not a physical UART — see `fw/CMakeLists.txt`'s
   `pico_enable_stdio_usb`/`pico_enable_stdio_uart` calls).
2. `led_driver_init()` configures the LED GPIOs and starts all LEDs off.
3. `tx_clock_driver_init()` configures the TX clock enable GPIO and the PWM
   slice backing the clock signal, both starting disabled.
4. `app_commands_init()` assembles the CLI command table (see below).
5. The loop polls `stdio_usb_connected()`. On a fresh connection it prints a
   banner and (re-)initializes the `clipilot` CLI instance, since bytes
   written before a host opens the CDC port are discarded.
6. Every iteration calls `cli_poll()` (when connected) and unconditionally
   calls `led_driver_tick()`, so LED patterns keep animating even with no
   terminal attached. The TX clock's PWM output runs in hardware and needs
   no per-loop tick.

### CLI command registry

`clipilot`'s `cli_lookup()` indexes its command table as one contiguous
`cli_cmd_t` array, so command objects contributed by different modules
cannot simply be scattered pointers. Each module instead exposes a single
`extern const cli_cmd_t xxx_cli_command;` singleton (see
`fw/src/led.h`/`led.c` for the `led` command), and `main.c` holds:

- `app_command_sources[]` — a small array of pointers to each module's
  singleton, one line per module.
- `app_commands[]` — a real, contiguous array populated at boot by
  `app_commands_init()`, which is what gets handed to `clipilot`.

Adding a new console command module only requires adding the module's own
`xxx_cli_command` singleton and one line in `app_command_sources[]`; no
other file changes. The full recipe (and the "why", including the C
static-initializer constraint that rules out a simpler approach) is in
[`.github/agents/fw-cli-module.md`](../.github/agents/fw-cli-module.md)
and [`.github/skills/fw-architecture/SKILL.md`](../.github/skills/fw-architecture/SKILL.md).

## See also

- [`hardware.md`](hardware.md) for the board this firmware targets.
- [`firmware.md`](firmware.md) for user-facing console/LED behavior.
- [`CONTRIBUTING.md`](../CONTRIBUTING.md) for the branch/build/PR workflow.
