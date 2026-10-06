# hdlcspy_fw

Raspberry Pi Pico 2 (RP2350) firmware for hdlcSpy, built with the Pico SDK.
The console is exposed over the native USB CDC virtual serial port (no
physical UART is used for stdio).

## Fresh clone: initialize submodules

This repo vendors the Pico SDK as a git submodule at `fw/external/pico-sdk`,
along with the nested submodules it needs (tinyusb, mbedtls, cyw43-driver,
btstack, lwip), plus the [clipilot](https://github.com/FastRabbit/clipilot)
CLI library as a submodule at `fw/external/clipilot`. After cloning hdlcSpy,
run:

```sh
git submodule update --init --recursive fw/external/pico-sdk fw/external/clipilot
```

Or, if you prefer to fetch only the nested libs the Pico SDK needs:

```sh
git submodule update --init fw/external/pico-sdk fw/external/clipilot
cd fw/external/pico-sdk
git submodule update --init --depth 1 lib/tinyusb lib/mbedtls lib/cyw43-driver lib/btstack lib/lwip
```

## Build

Requires the `arm-none-eabi` GCC toolchain, CMake, and Ninja (or Make).

```sh
cd fw
mkdir build && cd build
cmake -G Ninja -DPICO_SDK_PATH=../external/pico-sdk ..
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
corresponding `/dev/ttyACMx` on Linux) to get an interactive
[clipilot](https://github.com/FastRabbit/clipilot) console (`hdlcspy> `
prompt). Available commands:

- `help` — lists available commands
- `history` — shows recently entered lines
- `led <on|off|blink-slow|blink-fast|traffic> [which]` — LED control, see below
- `tx-clock <enable|disable|on|off|invert|frequency|status>` — TX clock control, see below

### LED control

Three LEDs are driven:

| name   | location                  |
|--------|----------------------------|
| `led0` | onboard Pico 2 LED (`PICO_DEFAULT_LED_PIN`) |
| `led1` | GPIO17                     |
| `led2` | GPIO18                     |

`which` is optional and defaults to `all`. Examples:

```
hdlcspy> led on led1
hdlcspy> led blink-slow          # all LEDs, ~1 Hz
hdlcspy> led blink-fast led2     # led2 only, ~5 Hz
hdlcspy> led traffic             # irregular router-activity-style flicker
hdlcspy> led off
```

### TX clock control

Two signals are involved:

| name     | GPIO | purpose                                        |
|----------|------|-------------------------------------------------|
| enable   | 1    | gates an external clock buffer/driver           |
| signal   | 0    | the clock itself, a PWM square wave, 1kHz..1MHz |

The enable line and the clock signal are independent: `enable`/`disable`
only drives GPIO1, `on`/`off` only starts/stops the PWM output on GPIO0.
Frequency changes apply immediately, whether or not the output is running.
`invert` flips the signal's polarity in hardware and also applies
immediately, whether or not the output is running.

```
hdlcspy> tx-clock frequency 100000   # set to 100 kHz (clamped to 1000..1000000)
hdlcspy> tx-clock invert on          # flip the signal's polarity
hdlcspy> tx-clock on                 # start the PWM signal on GPIO0
hdlcspy> tx-clock enable             # drive the GPIO1 enable line
hdlcspy> tx-clock status             # enable=on output=on invert=on frequency=100000 Hz
hdlcspy> tx-clock off
hdlcspy> tx-clock disable
```

