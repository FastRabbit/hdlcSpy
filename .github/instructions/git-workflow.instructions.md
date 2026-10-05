---
description: Branch, commit and merge conventions used in this repository. Load before creating any branch, commit or pull request.
---

# Git Workflow

See `CONTRIBUTING.md` for the full write-up; this is the condensed version for
quick reference while working.

## Branches

Short-lived, kebab-case, scoped to one feature/fix:

```
fw-led-commands             good
update-clipilot-submodule   good
github-instructions-and-skills good
```

Never commit directly to `main`. Branch from an up-to-date `main`:

```sh
git checkout main
git pull --ff-only
git checkout -b <short-kebab-case-name>
```

## Commits

Conventional-ish prefixes (`feat:`, `fix:`, `chore:`, `docs:`), optionally
scoped (`chore(fw): ...`). No enforced hook in this repo (unlike some vendored
submodules — see below), but keep the same discipline: short imperative
title, blank line, body explaining *why*. Always include the trailer on
agent-authored commits:

```
Co-authored-by: Copilot App <223556219+Copilot@users.noreply.github.com>
```

Write multi-line messages with `git commit -m "$(cat <<'EOF' ... EOF)"` and
avoid apostrophes/contractions in the message — they have broken this
heredoc pattern in this shell before ("tool's", "doesn't"). Prefer "the PR
tool" over "the tool's", and "does not" over "doesn't".

## Verify before committing

For any change under `fw/`, actually build it (see the `fw-builder` agent /
`fw-build-test` skill) and confirm `hdlcspy_fw.uf2` is produced before
committing. Don't commit unverified firmware changes. Docs-only changes don't
need a firmware rebuild.

## Landing on `main`

Preferred path, used for every change so far in this repo:

```sh
git push -u origin <branch>
gh pr create --title "..." --body "..." --base main --head <branch>
# after review:
gh pr merge <number> --merge --delete-branch=false
```

This repo merges with a real **merge commit** (`--merge`), not squash or
rebase-merge — keep that consistent so `git log --graph` stays meaningful.

The app's built-in `create_pull_request` tool can intermittently fail with
*"This project's GitHub account is no longer available"* even though the
repository is reachable. When that happens, fall back to the `gh` CLI
(`gh auth status` to confirm a working token, then `gh pr create` /
`gh pr merge`) — it uses its own token independently of that link.

Local-only alternative (no PR), only when explicitly asked for:

```sh
git checkout <branch>
git rebase main
git checkout main
git merge --ff-only <branch>
git branch -d <branch>
```

## Cleaning up after a merge

Delete the feature branch both locally and on the remote once merged, and
re-sync the local checkout (including submodules, which can otherwise be left
pointing at a stale commit after a fast-forward):

```sh
git checkout main
git pull --ff-only
git branch -d <branch>
git push origin --delete <branch>
git submodule update --init --recursive
```

## Submodule moves

Use `git mv <old-path> <new-path>` to relocate a submodule — it updates
`.gitmodules` and the gitlink in one clean commit. After switching branches
that disagree about a submodule's path, git can leave a stale/empty directory
behind ("directory not empty" warnings); `rm -rf` it and re-run
`git submodule update --init` for the path the checked-out branch actually
expects.

## Session checkpoint refs

The Copilot app stores its own local-only session checkpoints under
`refs/copilot/checkpoints/...`, visible via `git log --all` /
`git for-each-ref`. They are never pushed and are safe to delete:

```sh
git for-each-ref refs/copilot/checkpoints --format='%(refname)' | \
  xargs -n1 git update-ref -d
git gc --prune=now
```

## Vendored submodules have their own conventions

`fw/external/clipilot` and `fw/external/pico-sdk` are independent upstream
projects with their own (sometimes stricter) git conventions — e.g. clipilot's
own `.github/instructions/git-workflow.instructions.md` documents enforced
conventional-commit hooks that do not apply here. Don't apply this repo's
rules when committing *inside* a submodule's own history upstream; only this
repository's top-level commits (including submodule pointer bumps) follow the
conventions above.
