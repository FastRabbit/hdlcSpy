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

## Custom agents

`.github/agents/` defines repo-specific custom agents for recurring tasks:

- `fw-builder` — configures, builds, and verifies the `fw/` firmware (use after any
  firmware source or build-config change).
- `fw-reviewer` — read-only review of `fw/` changes before merging a feature branch
  (stdio/UART config, blocking waits, submodule pins, build hygiene).
