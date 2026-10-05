# Contributing

## Feature branch workflow

Changes land on `main` through short-lived feature branches, not direct commits.

1. **Branch from `main`:**
   ```sh
   git checkout main
   git pull
   git checkout -b <short-kebab-case-name>
   ```
2. **Make the change.** Keep branches focused on one feature/fix.
3. **Verify before committing.** For firmware changes under `fw/`, actually build it
   (see `fw/README.md`, or use the `fw-builder` custom agent) and confirm
   `hdlcspy_fw.uf2` is produced before committing. Don't commit unverified firmware
   changes.
4. **Commit** with a clear, conventional message (`feat:`, `fix:`, `chore:`, etc.),
   scoped where helpful (e.g. `feat(fw): ...`).
5. **Integrate back into `main`:**
   - Preferred: open a pull request (`gh pr create` or the app's PR tool) for review,
     then merge on GitHub.
   - Local-only alternative: rebase the feature branch onto current `main`, then
     fast-forward merge:
     ```sh
     git checkout <branch>
     git rebase main
     git checkout main
     git merge --ff-only <branch>
     git branch -d <branch>
     ```
6. **Push** `main` (and any still-open branches) to `origin` once merged.

## Repo layout

- `fw/` — Raspberry Pi Pico 2 (RP2350) firmware, Pico SDK-based. See `fw/README.md`
  for build/flash instructions and submodule setup.
- `hw/` — KiCad hardware design (schematic/PCB).
- `doc/` — project-level documentation (architecture, hardware, firmware
  behavior); start at `doc/README.md`.

## Custom agents

`.github/agents/` defines repo-specific custom agents for recurring tasks:

- `fw-builder` — configures, builds, and verifies the `fw/` firmware (use after any
  firmware source or build-config change).
- `fw-reviewer` — read-only review of `fw/` changes before merging a feature branch
  (stdio/UART config, blocking waits, submodule pins, build hygiene).
- `fw-cli-module` — scaffolds a new clipilot console command module (e.g. the `led`
  command) and wires it into `fw/src/main.c`'s command registry.

## Instructions and skills

Mirroring the structure used by the vendored `clipilot` submodule:

- `.github/instructions/git-workflow.instructions.md` — condensed version of this
  file's workflow, loaded automatically for branch/commit/PR work.
- `.github/instructions/c-style.instructions.md` — C conventions for `fw/src/**`.
- `.github/skills/fw-architecture/SKILL.md` — module map, invariants, and the
  recipe for adding a new console command or LED-style driver feature.
- `.github/skills/fw-build-test/SKILL.md` — the build/verify loop, submodule
  bootstrap, and flashing steps.
## Gotchas learned

- **PR creation/merge tool outage**: if the app's PR tool reports the GitHub account
  needs re-linking, fall back to the `gh` CLI (`gh pr create`, `gh pr merge --merge`)
  — it uses its own token and keeps working independently of that link.
- **Moving a submodule**: use `git mv <old-path> <new-path>` — it updates
  `.gitmodules` and relocates the gitlink in one clean commit. After checking out a
  branch on either side of such a move, stale/empty submodule directories can be
  left behind; `rm -rf` them and re-run `git submodule update --init` for the path
  the current branch expects.
- **Session checkpoint refs**: the app stores local-only session checkpoints under
  `refs/copilot/checkpoints/...`. They're safe to delete with
  `git update-ref -d <ref>` (+ `git gc --prune=now`) without touching real branch
  history — they are never pushed.
- **clipilot command modules**: see the `fw-cli-module` agent — a new top-level
  command needs a module exposing one `cli_cmd_t` singleton, registered with one
  line in `app_command_sources[]` in `main.c`, not a hand-built static array (which
  won't compile — see that agent for why).
