# Firmware

User-facing overview of what the `hdlcspy_fw` firmware does once flashed.
For build/flash instructions see [`fw/README.md`](../fw/README.md); for
internal structure see [`architecture.md`](architecture.md).

## Boot and console

On boot the firmware brings up USB CDC (a virtual serial port over the
Pico 2's USB connector) and waits for a host to open it. Once a terminal
connects, it prints a banner and starts a `clipilot`-based command-line
console:

```
 _          _ _      ____
| |__    __| | | ___/ ___| _ __  _   _
| '_ \  / _` | |/ __\___ \| '_ \| | | |
| | | || (_| | | (__ ___) | |_) | |_| |
|_| |_| \__,_|_|\___|____/| .__/ \__, |
                          |_|    |___/
 USB CDC connected -- hdlcspy_fw ready

hdlcspy>
```

Connect with any serial terminal (e.g. `screen`, `minicom`, PuTTY) at the
port the OS assigns the device; no particular baud rate matters since this
is USB CDC, not a real UART.

Reconnecting (e.g. re-opening the terminal) reprints the banner and resets
the CLI instance, since any output written before the host opens the port
is discarded.

## Console commands

Type `help` at the prompt for the full list (built in to `clipilot`) plus
every module's commands. Currently available:

### `led` — control the three board LEDs

```
led <on|off|blink-slow|blink-fast|traffic> [led0|led1|led2|all]
```

- The target argument defaults to `all` when omitted.
- `on` / `off` — steady state.
- `blink-slow` — ~1 Hz square wave.
- `blink-fast` — ~5 Hz square wave.
- `traffic` — irregular flicker, modeled after router activity LEDs
  (uses `pico_rand` for timing jitter).

LED patterns keep animating in the main loop even when no terminal is
attached, so `traffic` mode is visible without a host connected.

See [`architecture.md`](architecture.md) for how `led` (and future
modules' commands) get wired into the CLI, and
[`.github/agents/fw-cli-module.md`](../.github/agents/fw-cli-module.md)
for the recipe to add a new command module.

## Adding firmware features

Start with [`.github/skills/fw-architecture/SKILL.md`](../.github/skills/fw-architecture/SKILL.md)
for the module map and extension recipe, and
[`.github/skills/fw-build-test/SKILL.md`](../.github/skills/fw-build-test/SKILL.md)
for the configure/build/verify loop (including submodule bootstrap on a
fresh clone).
