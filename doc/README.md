# hdlcSpy Documentation

Project-level documentation lives here. This is user/developer facing
material about what the project *is* and how its pieces fit together.
Process docs (branching, PR, build-and-verify workflow) stay in
[`CONTRIBUTING.md`](../CONTRIBUTING.md); agent/skill recipes stay in
[`.github/agents/`](../.github/agents/) and
[`.github/skills/`](../.github/skills/).

## Contents

- [`architecture.md`](architecture.md) — repo layout, how `hw/` and `fw/`
  relate, and the firmware's runtime structure.
- [`hardware.md`](hardware.md) — board overview and key components, based
  on the KiCad project under `hw/hdlcSpy/`.
- [`firmware.md`](firmware.md) — firmware boot sequence, console/CLI usage,
  and the LED subsystem, from a user's point of view.

## Status

This is a living set of docs, started alongside the firmware's LED driver
and CLI work. Expect it to grow as new hardware peripherals and firmware
modules are added.
