---
name: fw-build-test
description: How to configure, build and verify the hdlcspy_fw Pico 2 firmware, including submodule setup, the toolchain, and the fresh-clone bootstrap. Use before building fw/ or when a build command fails.
---

# Building and Testing fw/

See `fw/README.md` for the user-facing version of this; this skill is the
condensed reference for an agent doing the build.

## Toolchain

`arm-none-eabi-gcc`, `cmake`, and `ninja` are required. On this machine they
come from Homebrew at `/opt/homebrew/bin` — make sure that's on `PATH` if a
build tool is reported missing even though `brew list` shows it installed.

## Submodules (fresh clone / missing directories)

The Pico SDK and clipilot are vendored under `fw/external/`:

```sh
git submodule update --init fw/external/pico-sdk fw/external/clipilot
cd fw/external/pico-sdk && git submodule update --init --depth 1 \
  lib/tinyusb lib/mbedtls lib/cyw43-driver lib/btstack lib/lwip
```

Depth 1 is enough for all nested Pico SDK submodules; this repo never needs
their full history.

If `cmake`/`ninja` fails referencing a path like `fw/pico-sdk` (old location)
instead of `fw/external/pico-sdk`, the working tree has stale submodule
checkouts left over from switching branches across the `fw/external/` move —
`rm -rf` the stray directory and re-run `git submodule update --init` for the
path the current branch's tree actually expects. Don't edit `CMakeLists.txt`
to work around a path mismatch; fix the submodule checkout instead.

## The verification loop

```sh
cd fw && mkdir -p build && cd build
cmake -G Ninja -DPICO_SDK_PATH=../external/pico-sdk ..
ninja hdlcspy_fw
```

Confirm the output exists and is a plausible size (tens of KB, grows a little
with each added feature):

```sh
ls -la hdlcspy_fw.uf2
```

A full clean build compiles roughly 90-100 Pico SDK / tinyusb / clipilot
objects and takes a couple of minutes; an incremental build after touching
only `fw/src/*.c` only rebuilds those 1-3 objects plus the final link.

**Leave `fw/build/` in place after verifying** — it's gitignored
(`fw/.gitignore` covers `build/` and `build-*/`) and kept around between
sessions by request, not deleted after a successful check.

## What a real build catches that reading code does not

* The static-array-initializer error described in the `fw-architecture` skill
  (`"initializer element is not constant"`) only shows up at compile time —
  it looks completely plausible when reading the diff.
* Missing `target_link_libraries` entries (e.g. forgetting `pico_rand` when a
  module calls `get_rand_32()`) fail at the link step, not configure.
* A stale submodule checkout (see above) fails at the `cmake` configure step
  with a missing-path error, before any compilation happens.

Treat "I read the diff and it looks right" as insufficient — always run the
loop above before calling a firmware change done.

## Flashing and the serial console (manual hardware step, not automatable here)

1. Hold **BOOTSEL** while plugging in the Pico 2, so it mounts as a USB mass
   storage device (`RP2350`).
2. Copy `hdlcspy_fw.uf2` onto it; the board reboots into the new firmware
   automatically.
3. Connect a serial terminal to the USB CDC port it enumerates as
   (`screen /dev/tty.usbmodemXXXX 115200` on macOS, `/dev/ttyACMx` on Linux)
   to reach the `hdlcspy>` clipilot prompt. `help` lists commands.

## Cleaning up after exploratory builds

If a throwaway `build-*/` directory was created to test an alternate
configuration (e.g. a different `PICO_SDK_PATH`), remove it before finishing
— only the primary `fw/build/` is meant to persist.
