---
phase: 59-install-script
plan: 01
status: complete
started: 2026-02-28
completed: 2026-02-28
---

## What Was Done

Created `install.sh` — POSIX shell install script:
- Platform detection via `uname -s` (Linux/Darwin) + `uname -m` (x86_64/arm64)
- Downloads binary from GitHub Releases
- SHA256 checksum verification (sha256sum or shasum fallback)
- Installs to `~/.local/bin` (configurable via `QSERIES_INSTALL_DIR`)
- `--version vX.Y` flag for specific version, defaults to latest
- `main()` wrapper for partial-download protection
- `curl`/`wget` fallback for HTTP fetching
- Prints PATH instructions if install dir not on PATH
- Syntax validated with `bash -n`

## Files Created
- `install.sh`

## Verification
- Syntax valid (`bash -n` passes)
- All 10 acceptance tests pass
