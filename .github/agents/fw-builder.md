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
