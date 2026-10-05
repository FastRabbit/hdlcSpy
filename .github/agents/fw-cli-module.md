---
name: fw-cli-module
description: >
  Scaffolds a new clipilot console command module for the fw/ firmware (e.g. a new
  top-level command like `led`). Use whenever a new clipilot command is needed,
  so it is wired into the console the same way as existing modules.
---

You add new clipilot commands to `hdlcspy_fw` by creating a self-contained module,
never by hand-writing command tables inline in `fw/src/main.c`.

## Pattern (see `fw/src/led.c` / `fw/src/led.h` as the reference implementation)

1. **Header** (`fw/src/<module>.h`): declares the module's public API plus one
   line at the end:
   ```c
   #include "cli.h"
   extern const cli_cmd_t <module>_cli_command;
   ```
2. **Source** (`fw/src/<module>.c`): implements the feature logic, then the
   clipilot wiring in the same file:
   - `cli_arg_spec_t` tables for any arguments (e.g. an enum selecting a
     sub-target), using `CLI_ARG_ENUM` with a `NULL`-terminated name list for
     closed choices.
   - One static handler function per subcommand:
     `static int cmd_<module>_<verb>(cli_t *cli, const cli_args_t *args, void *user)`.
     Handlers must be non-blocking — never `sleep_ms()`/spin inside a handler, it
     freezes the whole console.
   - A `static const cli_cmd_t <module>_subs[] = { ... };` table listing the
     subcommands.
   - The single exported object:
     ```c
     const cli_cmd_t <module>_cli_command = {
         "<name>", "<one line help>", <module>_subs,
         sizeof(<module>_subs) / sizeof(<module>_subs[0]), NULL, 0u, NULL
     };
     ```
3. **Register it** in `fw/src/main.c`: add `#include "<module>.h"` and one line
   to `app_command_sources[]`:
   ```c
   static const cli_cmd_t *const app_command_sources[] = {
       &led_cli_command,
       &<module>_cli_command,
   };
   ```
   Do not touch anything else in `main.c` — `app_commands_init()` and
   `console_init()` already handle any number of entries.
4. **CMakeLists.txt**: add the new `.c` file to `add_executable(hdlcspy_fw ...)`,
   and add any new Pico SDK library (e.g. `pico_rand`) to
   `target_link_libraries(hdlcspy_fw ...)` if the module needs one not already
   pulled in by `pico_stdlib`/`clipilot::clipilot`.
5. Build and verify via the `fw-builder` agent before committing.

## Why the registry step exists

clipilot's core walks `cfg.commands` as a single contiguous `cli_cmd_t` array
(`table[i]` indexing in `cli_lookup()`), so command objects from different
modules cannot be referenced as scattered pointers at the top level. `main.c`
copies each module's singleton into one real array (`app_commands[]`) at
startup via `app_commands_init()` — this is why the registration step is a
runtime copy (`app_commands[i] = *app_command_sources[i];`), not a static
initializer list. Do not attempt
`static const cli_cmd_t commands[] = { led_cli_command, other_cli_command };`
directly — that fails to compile ("initializer element is not constant"),
because copying an `extern const` struct by value into static storage isn't
a constant expression in C.

## Do not

- Do not add application commands by editing `console_init()`'s `cfg.commands`
  assignment directly, or by hand-building a one-off array in `main.c`.
- Do not put a module's subcommand handlers/table in `main.c` — keep them next
  to the feature they operate on (e.g. LED logic + its CLI table both live in
  `led.c`).
