# hdlcspy_fw

Raspberry Pi Pico 2 (RP2350) firmware for hdlcSpy, built with the Pico SDK.
The console is exposed over the native USB CDC virtual serial port (no
physical UART is used for stdio).

## Fresh clone: initialize submodules

This repo vendors the Pico SDK as a git submodule at `fw/pico-sdk`, along
with the nested submodules it needs (tinyusb, mbedtls, cyw43-driver,
btstack, lwip). After cloning hdlcSpy, run:

```sh
git submodule update --init --recursive fw/pico-sdk
```

Or, if you prefer to fetch only the nested libs this firmware needs:

```sh
git submodule update --init fw/pico-sdk
cd fw/pico-sdk
git submodule update --init --depth 1 lib/tinyusb lib/mbedtls lib/cyw43-driver lib/btstack lib/lwip
```

## Build

Requires the `arm-none-eabi` GCC toolchain, CMake, and Ninja (or Make).

```sh
cd fw
mkdir build && cd build
cmake -G Ninja -DPICO_SDK_PATH=../pico-sdk ..
ninja
```

This produces `hdlcspy_fw.uf2` (plus `.elf`/`.bin`/`.hex`) in `fw/build/`.

## Flash

1. Hold the **BOOTSEL** button on the Pico 2 while plugging it into USB (or
   while resetting it), so it mounts as a USB mass storage device (e.g.
   `RP2350`).
2. Copy `hdlcspy_fw.uf2` onto that mass storage drive. The board will
   automatically reboot and run the firmware.

## Serial console

After flashing, the board enumerates as a USB CDC virtual serial port.
Connect to it (e.g. `screen /dev/tty.usbmodemXXXX 115200` on macOS, or the
corresponding `/dev/ttyACMx` on Linux) to see the counter printed once per
second.
