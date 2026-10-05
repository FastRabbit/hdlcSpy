# Hardware

The board lives in `hw/hdlcSpy/` as a KiCad 8 project
(`hdlcSpy.kicad_pro`/`_sch`/`_pcb`/`_prl`). This page is a starting index
into that project, not a replacement for reading the schematic — open
`hw/hdlcSpy/hdlcSpy.kicad_pro` in KiCad for the authoritative design.

## MCU

The firmware in `fw/` targets a **Raspberry Pi Pico 2 (RP2350)** module
(`PICO_BOARD pico2` in `fw/CMakeLists.txt`). Console I/O is over the
Pico's USB port (USB CDC virtual serial), not a physical UART.

## Onboard LEDs

Three LEDs are driven by the firmware's LED subsystem
(`fw/src/led.c`/`led.h`):

| Name   | Pin                      | Notes                        |
|--------|--------------------------|-------------------------------|
| `led0` | `PICO_DEFAULT_LED_PIN`   | Onboard Pico 2 LED            |
| `led1` | GPIO17                   | Board LED                     |
| `led2` | GPIO18                   | Board LED                     |

See [`firmware.md`](firmware.md) for the console `led` command that
controls them.

## TX clock

The firmware's TX clock subsystem (`fw/src/tx_clock.c`/`tx_clock.h`)
drives two signals:

| Signal   | Pin    | Notes                                              |
|----------|--------|------------------------------------------------------|
| enable   | GPIO1  | digital output, gates an external clock buffer/driver |
| signal   | GPIO0  | PWM square wave, 1kHz..1MHz, 50% duty                 |

See [`firmware.md`](firmware.md) for the console `tx-clock` command that
controls them.

## Key components (from `hw/datasheets/`)

The datasheets vendored alongside the schematic indicate the following
component families are used on the board. This list is derived from the
datasheets present in the repo, not from a full schematic trace — treat it
as a pointer to go look things up in KiCad, not a verified bill of
materials:

- **RP2350** — the Pico 2's MCU (`RP-008299-DS-3-pico-2-datasheet.pdf`,
  `RP-008280-DS-2-hardware-design-with-rp2350.pdf`).
- **SN65HVD1782** — RS-485/RS-422 transceiver.
- **SN75HVD12** — RS-485 transceiver.
- **SN74LVC1G14** — single Schmitt-trigger inverter (confirmed as `U3` in
  the schematic).
- **NC7S14** — single Schmitt-trigger inverter.
- **PMEG3005EGW** — Schottky diode.
- **LM2936** — low-dropout linear regulator.
- **VLMTG1400** — see datasheet for exact part family.

Given the RS-485/RS-422 transceivers and the project's name, the board is
a serial-bus tap/sniffer front end feeding the RP2350 (which the `fw/`
firmware runs), but that intent has not yet been written up from the
schematic in this doc. Expand this section once the signal chain has been
walked and confirmed, rather than guessing further here.

## Fabrication

`hw/hdlcSpy/bom/ibom.html` is a generated interactive BOM; regenerate it
from KiCad rather than hand-editing it.
