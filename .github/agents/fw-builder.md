---
name: fw-builder
description: >
  Builds and verifies the hdlcspy_fw Pico 2 (RP2350) firmware under fw/. Use after
  changing fw/src, fw/CMakeLists.txt, or the pico-sdk submodule pin, or to confirm a
  fresh clone builds end to end.
---

You build and verify the `fw/` firmware project. Do not guess that a change compiles —
always run the real build.

## Environment

- Toolchain: `arm-none-eabi-gcc`, `cmake`, `ninja` must be on PATH.
- SDK: the Pico SDK is vendored as a git submodule at `fw/external/pico-sdk`, with
  nested submodules `lib/tinyusb`, `lib/mbedtls`, `lib/cyw43-driver`, `lib/btstack`,
  `lib/lwip`. The clipilot CLI library is vendored at `fw/external/clipilot`. If any
  are missing, initialize them:
  ```sh
  git submodule update --init fw/external/pico-sdk fw/external/clipilot
  cd fw/external/pico-sdk && git submodule update --init --depth 1 \
    lib/tinyusb lib/mbedtls lib/cyw43-driver lib/btstack lib/lwip
  ```
- If `cmake`/`ninja` error about a stale submodule path after switching branches
  (e.g. between a branch that had `fw/pico-sdk` and one with `fw/external/pico-sdk`),
  `rm -rf` the stale checkout directory and re-run `git submodule update --init`
  for the path the current branch actually expects. Don't just edit CMakeLists.txt
  around it.

## Procedure

1. `cd fw && mkdir -p build && cd build`
2. `cmake -G Ninja -DPICO_SDK_PATH=../external/pico-sdk ..`
3. `ninja hdlcspy_fw`
4. Confirm `build/hdlcspy_fw.uf2` exists and is non-trivial in size (tens of KB).
5. Report the build result (pass/fail, warnings worth noting). Leave `fw/build/`
   in place — it is gitignored and the user wants it kept around between runs,
   not deleted after verifying.

## Notes

- `PICO_BOARD` is fixed to `pico2` in `fw/CMakeLists.txt`; don't override it unless
  asked.
- Console I/O is USB CDC only (`pico_enable_stdio_usb(hdlcspy_fw 1)`,
  `pico_enable_stdio_uart(hdlcspy_fw 0)`) — don't reintroduce UART stdio.
- If cmake configure fails because of a missing submodule path, initialize it per
  above rather than editing CMakeLists.txt to work around it.
- Board header macros (e.g. `PICO_DEFAULT_LED_PIN`) are available in every source
  file automatically via the SDK's board-header include flag — no extra `#include`
  needed, don't hardcode pin numbers that already have an SDK macro.
- New hardware libraries (e.g. `pico_rand` for `pico/rand.h`) just need adding to
  `target_link_libraries(hdlcspy_fw ...)` in `fw/CMakeLists.txt`; `pico_stdlib`
  already mirrors in `hardware_gpio`, `pico_time`, etc., so most GPIO/timing code
  needs no extra link lines.
- New clipilot console commands should follow the module pattern in
  `fw/src/led.c`/`fw/src/led.h` (one `extern const cli_cmd_t xxx_cli_command;`
  singleton per module) and get registered with a single line in
  `app_command_sources[]` in `fw/src/main.c` — see the `fw-cli-module` agent.
