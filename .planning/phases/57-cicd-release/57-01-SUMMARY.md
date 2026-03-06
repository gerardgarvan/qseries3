---
phase: 57-cicd-release
plan: 01
status: complete
started: 2026-02-28
completed: 2026-02-28
---

## What Was Done

Created `.github/workflows/release.yml` — tag-triggered release workflow:
- Triggers on `v*` tag push
- Matrix build: Linux x86_64 (ubuntu-24.04, static) + macOS arm64 (macos-14, dynamic)
- Runs acceptance tests on Linux
- Generates SHA256 checksums file
- Creates GitHub Release via softprops/action-gh-release@v2 with auto-generated notes
- Uploads binaries + checksums as release assets

## Files Created
- `.github/workflows/release.yml`

## Verification
- YAML structure valid
- macOS build correctly omits `-static`
- Committed to repository
