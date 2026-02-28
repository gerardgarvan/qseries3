# Research Summary — Milestone v4.1: Distribution

**Project:** qseries REPL
**Domain:** Distribution & packaging (Docker image + install script)
**Researched:** 2026-02-28
**Confidence:** HIGH

## Executive Summary

The qseries binary is already statically linked at ~1.5MB with zero runtime dependencies, which makes distribution trivially simple compared to most software. The Docker image is a two-stage build producing a ~1.5MB image on `scratch`. The install script is a POSIX shell script that downloads a pre-built binary from GitHub Releases. No changes to the C++ source code or existing build are required — this milestone adds only DevOps files (Dockerfile, CI workflows, install script).

The recommended approach is: build a GitHub Actions CI pipeline first (prerequisite for everything), then add the Dockerfile (testable locally), then the release workflow (cross-platform matrix builds uploaded to GitHub Releases), then the install script (downloads from those releases). All four researchers agree on this dependency chain. The key risks are macOS rejecting `-static` (must be omitted on Darwin), Docker containers needing `-it` flags for the REPL, and the install script needing robust platform detection.

## Consensus Findings

All four researchers agree on these points:

1. **`scratch` is the correct Docker runtime base** — not distroless (~9MB overhead for unused features), not Alpine (unnecessary package manager). The static binary IS the entire image.
2. **GitHub Container Registry (ghcr.io)** over Docker Hub — free, no rate limits, integrated auth with GitHub Actions.
3. **GitHub Releases** for binary hosting — free, unlimited bandwidth, stable download URLs, built-in versioning.
4. **macOS cannot use `-static`** — Apple rejects static binaries. macOS builds must dynamically link libSystem (which is always present). This is standard practice.
5. **No package manager submissions** (Homebrew, Scoop, AUR) — maintenance overhead disproportionate to the niche user base. Defer indefinitely.
6. **Exec-form ENTRYPOINT** (`ENTRYPOINT ["/qseries"]`) — shell-form breaks signal handling, leaving users unable to Ctrl+C out of the container.
7. **Tag-triggered releases** (`on: push: tags: ['v*']`) — git tag drives release version, Docker tag, and binary version string.

## Stack Additions

No new C++ dependencies. Only DevOps files:

| File | Purpose | Notes |
|------|---------|-------|
| `Dockerfile` | Multi-stage build: `gcc:14-bookworm` → `scratch` | Repo root |
| `.dockerignore` | Exclude `.git/`, `.planning/`, `build/`, etc. | Keeps context small |
| `.github/workflows/ci.yml` | Build + test on push/PR | Single platform, fast feedback |
| `.github/workflows/release.yml` | Cross-platform matrix build → GitHub Release + Docker push | Triggered by `v*` tags |
| `install.sh` | One-liner Unix installer | POSIX sh, `curl \| bash` pattern |
| Makefile updates | `docker-build`, `docker-run` targets | Additive only |
| README.md updates | Install instructions, Docker usage, CI badge | User-facing docs |

**Key versions to pin:**
- `gcc:14-bookworm` (Docker build image)
- `actions/checkout@v4`, `softprops/action-gh-release@v2`, `docker/build-push-action@v6`
- `ubuntu-24.04` (CI runner — explicit, not `ubuntu-latest`)

## Feature Table Stakes

### Install Script — minimum viable
- Platform detection (`uname -s` + `uname -m` → OS/arch)
- HTTP tool fallback (curl → wget)
- SHA256 checksum verification
- Install to `~/.local/bin` (no sudo by default)
- `--version` flag for pinning
- `set -euo pipefail` + wrap in `main()` function (partial-download protection)
- POSIX sh compatible (`#!/bin/sh`)
- Idempotent (safe to re-run)

### Docker Image — minimum viable
- Multi-stage build: `gcc:14-bookworm` → `scratch`
- `ENTRYPOINT ["/qseries"]` (exec form, no shell wrapper)
- Tagged versions (`:v3.1`, `:latest`)
- Image size < 5MB
- Works with `docker run -it`

### CI/CD — minimum viable
- `ci.yml`: build + acceptance tests on push/PR (Linux only)
- `release.yml`: tag-triggered matrix build → GitHub Release with binaries + checksums → Docker push to ghcr.io
- Platforms: linux-amd64, linux-arm64, darwin-amd64, darwin-arm64, windows-amd64

### Defer
- Windows PowerShell install script (WSL is sufficient)
- Multi-arch Docker manifest (amd64-only image; ARM users use install script)
- Self-update mechanism
- `cosign` image signing
- Homebrew/Scoop/AUR packages

## Watch Out For

**Top 5 pitfalls, ranked by severity:**

1. **macOS rejects `-static`** — arm64 macOS kills static binaries with SIGKILL. CI matrix and Makefile must detect Darwin and omit `-static`. This blocks macOS distribution entirely if missed.

2. **Docker without `-it` breaks the REPL** — bare `docker run qseries` gets no TTY, REPL enters script mode, exits immediately. Document `-it` prominently. Consider a wrapper that detects missing TTY and prints a helpful error.

3. **Signal handling in Docker** — shell-form ENTRYPOINT wraps the binary in `/bin/sh`, swallowing SIGINT. Users can't Ctrl+C. Must use exec-form: `ENTRYPOINT ["/qseries"]`.

4. **Install script `curl | bash` security** — wrap entire script in a `main()` function called at the end (prevents partial-download execution). Use `set -euo pipefail`. Download to temp dir, verify checksum, then move to install dir. Never require sudo.

5. **PATH management across shells** — `~/.local/bin` may not be on PATH. Don't silently modify shell configs. Print clear instructions for bash/zsh/fish. Let the user decide.

**Secondary concerns:**
- QEMU-emulated arm64 builds are 10-50x slower than native — use `ubuntu-24.04-arm` runner or cross-compile with `aarch64-linux-gnu-g++`
- Docker `-t` flag converts `\n` → `\r\n` — document that scripted usage needs `-i` without `-t`
- Binary naming: Linux/macOS builds must NOT have `.exe` extension

## Proposed Phase Order

### Phase 1: Docker Image
**Files:** `Dockerfile`, `.dockerignore`, Makefile docker targets
**Rationale:** Self-contained, testable locally with `docker build . && docker run -it`. No CI dependency. Validates the static binary + scratch pattern immediately.
**Delivers:** Working `docker run -it qseries` experience
**Avoids:** Pitfalls #2 (TTY), #3 (signals), #14 (image bloat) by using exec-form ENTRYPOINT and multi-stage build from the start
**Research needed:** No — straightforward, well-documented pattern

### Phase 2: CI/CD Pipeline
**Files:** `.github/workflows/ci.yml`, `.github/workflows/release.yml`
**Rationale:** Prerequisite for install script. Establishes quality gate (CI) and produces the release artifacts (binaries + checksums on GitHub Releases + Docker image on ghcr.io).
**Delivers:** Automated cross-platform builds, GitHub Releases with binaries, Docker image pushed to ghcr.io
**Avoids:** Pitfall #1 (macOS static) by conditionally omitting `-static` in the matrix. Pitfall #8 (slow QEMU) by using native ARM runners or cross-compilation.
**Research needed:** Possibly — verify exact GitHub Actions runner availability and MinGW g++ path on `windows-latest`

### Phase 3: Install Script + README
**Files:** `install.sh`, `README.md` updates
**Rationale:** Depends on Phase 2 (needs GitHub Releases to exist). Cannot be tested until at least one release is published.
**Delivers:** `curl -fsSL .../install.sh | bash` one-liner install for Linux/macOS
**Avoids:** Pitfall #4 (security) with `main()` wrapper and checksums. Pitfall #7 (PATH) by defaulting to `~/.local/bin` and printing instructions.
**Research needed:** No — well-established patterns from Starship, Deno, etc.

### Phase Ordering Rationale
- Docker → CI → Install follows the strict dependency chain: install script needs releases, releases need CI, Docker is independent but validates the build first
- Each phase has a clear deliverable that can be verified before moving on
- Docker first gives fastest local feedback loop

## Open Questions

Decisions to make during requirements/planning:

1. **gcc/glibc vs Alpine/musl for Docker build stage?** STACK.md recommends gcc:14-bookworm (matches project toolchain). PITFALLS.md warns glibc-static isn't truly static (NSS issue). Since qseries does zero networking, the NSS concern is moot — but musl would produce a "purer" static binary. Recommendation: start with gcc:14-bookworm for consistency; switch to musl only if portability issues arise.

2. **Multi-arch Docker image (amd64 + arm64) or amd64-only?** FEATURES.md calls multi-arch a table stake. STACK.md says skip it. Recommendation: amd64-only for v4.1, add arm64 later if requested.

3. **Version string injection at build time?** Currently hardcoded as `"qseries 2.0"` in main.cpp. CI should inject the git tag via `-DVERSION=...`. Needs a small source change (use `#ifndef VERSION` guard).

4. **Should install script support `--global` (sudo to `/usr/local/bin`)?** STACK.md says yes. PITFALLS.md says never require sudo. Recommendation: default to `~/.local/bin`, offer `--global` as opt-in.

5. **Windows install script (PowerShell)?** STACK.md includes one. FEATURES.md marks it as anti-feature. Recommendation: defer. Provide direct download link on GitHub Releases instead.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Docker, GitHub Actions, GitHub Releases are mature, well-documented |
| Features | HIGH | Clear table stakes; many reference implementations (Starship, Deno) |
| Architecture | HIGH | Multi-stage Docker, CI matrix, install script are standard patterns |
| Pitfalls | HIGH | Verified against official docs (Apple QA1118, Docker docs, glibc docs) |

**Overall confidence:** HIGH

### Gaps to Address
- Windows CI build: MinGW g++ availability on `windows-latest` runner needs verification at implementation time
- ARM64 cross-compilation: acceptance tests can't run for cross-compiled binaries without QEMU
- Rosetta detection on macOS: `uname -m` may report x86_64 on ARM hardware under Rosetta

## Sources

### Primary (HIGH confidence)
- Docker multi-stage build guide — https://docs.docker.com/guides/cpp/multistage
- Apple QA1118: Static linking not supported — https://developer.apple.com/library/archive/qa/qa1118
- GitHub Actions runner images — https://github.com/actions/runner-images
- softprops/action-gh-release — https://github.com/softprops/action-gh-release
- Docker glibc/musl docs — https://docs.docker.com/dhi/core-concepts/glibc-musl/

### Secondary (MEDIUM confidence)
- Starship install.sh — reference implementation for single-binary installers
- PARI/GP Docker — same domain (number theory REPL in Docker)
- scratch vs distroless comparison — https://oneuptime.com/blog/post/2026-02-08

---
*Research completed: 2026-02-28*
*Ready for roadmap: yes*
