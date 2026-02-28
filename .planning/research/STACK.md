# Technology Stack: Distribution Features (Docker Image + Install Script)

**Project:** q-series REPL — Docker image creation and one-liner install scripts
**Researched:** 2026-02-28
**Overall confidence:** HIGH (Docker and GitHub Releases are mature, well-documented ecosystems)

## Executive Summary

The qseries binary is already statically linked (~1.5MB), which makes distribution trivially simple. The Docker image uses a two-stage build producing a ~3.5MB image. Install scripts are self-contained shell/PowerShell scripts that download pre-built binaries from GitHub Releases. No new C++ dependencies are needed. The only additions are DevOps files: a Dockerfile, two install scripts, and a GitHub Actions workflow.

---

## Docker Stack

### Base Images

| Stage | Image | Tag | Size | Why |
|-------|-------|-----|------|-----|
| Build | `gcc` | `14-bookworm` | ~1.4 GB | Official GCC image with C++20 support. Debian Bookworm base provides stable `g++ -static` support. Pin to `14` not `latest` for reproducible builds. |
| Runtime | `scratch` | (empty) | 0 B | The qseries binary is fully static (`-static` flag), needs no libc, no CA certs, no timezone data. `scratch` is the correct choice. |

**Why `scratch` over `distroless/static-debian12`?**
- qseries is a REPL that does math. It makes no network calls, reads no TLS certificates, needs no timezone data.
- `distroless` adds ~9MB for features we don't use.
- `scratch` + static binary = smallest possible image (~3.5MB total).

**Why `gcc:14-bookworm` over Alpine musl build?**
- The existing codebase compiles with glibc's `g++ -static`. Alpine uses musl libc, which could introduce subtle behavioral differences.
- The `gcc` official image matches the project's existing compile toolchain exactly.
- Build stage size doesn't matter (it's discarded in multi-stage).

### Dockerfile Pattern

```dockerfile
FROM gcc:14-bookworm AS builder
WORKDIR /build
COPY src/ src/
COPY Makefile .
RUN g++ -std=c++20 -O2 -static -Wall -Wextra -o qseries src/main.cpp

FROM scratch
COPY --from=builder /build/qseries /qseries
ENTRYPOINT ["/qseries"]
```

Key decisions:
- **Direct `g++` call, not `make`**: The Dockerfile doesn't need the full Makefile (demo files, test targets). One compilation command is clearer and more reproducible.
- **`ENTRYPOINT` not `CMD`**: Users run `docker run -it qseries` and land directly in the REPL. No shell needed (scratch has none anyway).
- **`-it` required**: The REPL reads stdin. Docker docs confirm interactive containers need `-it` flags.
- **No `.dockerignore` needed initially**: The build context is small (src/ is header files + main.cpp). If the repo grows, add one later.

### Docker Image Size Estimate

| Component | Size |
|-----------|------|
| scratch base | 0 B |
| qseries static binary | ~1.5 MB |
| **Total image** | **~1.5 MB** |

### Docker Hub / GHCR Hosting

| Option | Recommendation | Why |
|--------|---------------|-----|
| GitHub Container Registry (ghcr.io) | **Use this** | Free for public repos, integrated with GitHub Actions, no separate account needed |
| Docker Hub | Skip | Requires separate account, rate limits on free tier, unnecessary for a niche academic tool |

Image name: `ghcr.io/gerardgarvan/qseries:latest` plus version tags like `:v2.0`.

---

## Install Script Stack

### Binary Naming Convention

Use the standard pattern: `qseries-{os}-{arch}{ext}`

| Platform | Binary Name | Notes |
|----------|-------------|-------|
| Linux x86_64 | `qseries-linux-amd64` | Static binary, runs on any Linux distro |
| Linux ARM64 | `qseries-linux-arm64` | For ARM servers/Raspberry Pi |
| macOS x86_64 | `qseries-darwin-amd64` | Intel Macs |
| macOS ARM64 | `qseries-darwin-arm64` | Apple Silicon Macs |
| Windows x86_64 | `qseries-windows-amd64.exe` | Cross-compiled via MinGW (already supported in build.sh) |

No `.tar.gz` wrapping — the binary is a single file (~1.5MB), small enough to download directly. Tarballs add complexity for zero benefit at this size.

### Hosting: GitHub Releases

| Aspect | Decision | Why |
|--------|----------|-----|
| Where to host binaries | GitHub Releases | Free, unlimited bandwidth for public repos, versioned, integrates with `gh` CLI and API |
| Release naming | `v2.0`, `v2.1`, etc. | Matches git tags. Semantic versioning. |
| Asset upload | GitHub Actions `softprops/action-gh-release` | Automates uploading binaries on tag push |

**Why NOT raw repo files or a separate CDN:**
- GitHub Releases is the standard for open-source binary distribution
- Provides stable download URLs: `https://github.com/gerardgarvan/qseries3/releases/download/v2.0/qseries-linux-amd64`
- Built-in checksums (SHA256) via release notes
- No hosting costs, no CDN configuration

### Unix Install Script (`install.sh`)

**Pattern:** `curl -fsSL https://raw.githubusercontent.com/gerardgarvan/qseries3/main/install.sh | bash`

Core logic:
1. Detect OS via `uname -s` → `linux` or `darwin`
2. Detect arch via `uname -m` → `amd64` or `arm64`
3. Fetch latest release tag from GitHub API (or accept `--version` arg)
4. Download binary with `curl -fsSL` from GitHub Releases
5. Install to `~/.local/bin/qseries` (no sudo required)
6. Warn if `~/.local/bin` is not in `$PATH`

**Key design choices:**
- **`~/.local/bin` as default**: Follows XDG convention, no root required. Falls back to `/usr/local/bin` with sudo if user passes `--global`.
- **No `jq` dependency**: Parse GitHub API JSON with `grep` + `cut` (the response format is stable and simple).
- **POSIX sh, not bash**: Use `#!/bin/sh` for maximum compatibility (Alpine, BusyBox, etc.).
- **Verify with `sha256sum`/`shasum`**: Download the checksum file from the release and verify. Skip if tools aren't available (warn but don't fail).

### Windows Install Script (`install.ps1`)

**Pattern:** `irm https://raw.githubusercontent.com/gerardgarvan/qseries3/main/install.ps1 | iex`

(`irm` = `Invoke-RestMethod`, `iex` = `Invoke-Expression` — standard PowerShell shorthand)

Core logic:
1. Detect architecture via `$env:PROCESSOR_ARCHITECTURE`
2. Query GitHub API for latest release
3. Download `qseries-windows-amd64.exe` with `Invoke-WebRequest`
4. Place in `$env:LOCALAPPDATA\qseries\` and add to user PATH
5. Print success message with usage instructions

**Key design choices:**
- **`$env:LOCALAPPDATA\qseries\`**: Standard Windows per-user app location. No admin required.
- **User PATH modification**: `[Environment]::SetEnvironmentVariable("Path", ..., "User")` — persists across sessions without admin.
- **No Scoop/Chocolatey/WinGet**: Overengineering for a niche tool. A 1.5MB binary doesn't need a package manager. Add later only if demand warrants.

---

## Cross-Platform Build (GitHub Actions)

### CI/CD Workflow

The GitHub Actions workflow builds binaries for all platforms on each tagged release.

| Runner | Target | Compiler | Flags | Notes |
|--------|--------|----------|-------|-------|
| `ubuntu-24.04` | linux-amd64 | `g++-13` | `-std=c++20 -O2 -static` | Pin g++-13 explicitly (GCC 14 had C++20 libstdc++ issues on Ubuntu 24.04) |
| `ubuntu-24.04` + QEMU | linux-arm64 | Cross-compiler | `-std=c++20 -O2 -static` | Via `aarch64-linux-gnu-g++` package or `uraimo/run-on-arch-action` |
| `macos-latest` (arm64) | darwin-arm64 | `clang++` | `-std=c++20 -O2` | macOS doesn't support `-static`. Binary links dynamically to libSystem (Apple requirement). Still works — libSystem is always present. |
| `macos-13` (x86_64) | darwin-amd64 | `clang++` | `-std=c++20 -O2` | Intel Mac runner. Same dynamic linking caveat. |
| `ubuntu-24.04` | windows-amd64 | `x86_64-w64-mingw32-g++` | `-std=c++20 -O2 -static` | Cross-compile Windows binary on Linux. Already proven in existing `build.sh`. |

### macOS Static Linking Caveat

**Apple does not support `-static` for user programs.** All macOS binaries must dynamically link `libSystem.dylib` (the kernel interface). This is a hard platform constraint, not a toolchain issue.

**Impact on qseries:** Minimal. The binary has zero external dependencies beyond the C++ standard library. On macOS, `libc++.dylib` and `libSystem.dylib` are always present (they ship with the OS). The binary will work on any macOS version with C++20 runtime support (macOS 13+).

**For the install script:** This means the macOS binary is not portable across arbitrary systems like the Linux static binary is, but that's fine — macOS users always have the required dylibs.

### GitHub Actions Key Configuration

| Action | Version | Purpose |
|--------|---------|---------|
| `actions/checkout` | `v4` | Check out repo |
| `softprops/action-gh-release` | `v2` | Create release and upload binaries |
| `docker/login-action` | `v3` | Authenticate to GHCR for Docker push |
| `docker/build-push-action` | `v6` | Build and push Docker image |

### Trigger Strategy

```yaml
on:
  push:
    tags: ['v*']
```

Tag-based releases only. No CI on every push (the project doesn't have a test suite that runs in CI yet). When a version tag is pushed, the workflow:
1. Builds all platform binaries in parallel (matrix strategy)
2. Creates a GitHub Release with all binaries attached
3. Builds and pushes Docker image to GHCR

---

## Makefile Integration

### New Targets

| Target | Command | Purpose |
|--------|---------|---------|
| `docker` | `docker build -t qseries .` | Build Docker image locally |
| `docker-run` | `docker run -it qseries` | Run REPL in container |

These integrate with the existing Makefile without modifying any existing targets. The `all` target remains unchanged.

### No Changes to Existing Build

The Dockerfile calls `g++` directly, not `make`. The install scripts download pre-built binaries. Neither requires changes to the existing Makefile, `build.sh`, or source code.

---

## What NOT to Add

| Temptation | Why Skip It |
|------------|-------------|
| Homebrew formula | Requires maintaining a tap repo, approval process. For a niche academic tool, `curl \| bash` is simpler and works today. Add only if there's demand. |
| Scoop/Chocolatey/WinGet packages | Same reasoning. Package manager integration is maintenance overhead with little payoff for a specialized tool. |
| Nix flake | Niche within niche. Skip. |
| Flatpak/Snap/AppImage | These are for GUI apps with desktop integration. A CLI REPL doesn't benefit. |
| Multi-arch Docker image | `linux/amd64` only for now. ARM Docker users are rare enough that building multi-arch is unnecessary complexity. The install script covers ARM Linux natively. |
| `cosign` image signing | Nice for enterprise security. Overkill for an academic math tool. |
| Auto-update mechanism | The binary is 1.5MB. Users can re-run the install script. No need for a built-in updater. |

---

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| Docker runtime base | `scratch` | `distroless/static-debian12` | qseries makes no network calls, needs no CA certs. scratch saves ~9MB and is simpler. |
| Docker build base | `gcc:14-bookworm` | `alpine:3.20` + `build-base` | Alpine uses musl; project is tested with glibc. Avoid introducing a new libc variant. |
| Binary hosting | GitHub Releases | S3/R2 + CDN | Unnecessary infrastructure for a free open-source tool. GitHub Releases has unlimited bandwidth. |
| Install approach | `curl \| bash` script | Homebrew/Scoop | Scripts are zero-overhead. Package managers require ongoing maintenance of formula/manifests. |
| CI | GitHub Actions | GitLab CI / CircleCI | Repo is on GitHub. Native integration wins. |
| Container registry | GHCR (ghcr.io) | Docker Hub | Free, no rate limits for public repos, same GitHub auth. |
| Windows cross-compile | MinGW on Linux runner | MSVC on Windows runner | MinGW cross-compile is already proven in `build.sh`. Windows runners are slower and more expensive (2x minutes). |

---

## Version Pinning Summary

| Tool/Image | Pin To | Why |
|------------|--------|-----|
| `gcc` Docker image | `14-bookworm` | Matches project's C++20 needs. Don't use `latest` — major GCC version bumps can break builds. |
| `ubuntu` runner | `ubuntu-24.04` | Explicit over `ubuntu-latest` to avoid surprise migrations. |
| `macos` runner | `macos-latest` + `macos-13` | `macos-latest` = ARM64 (Apple Silicon), `macos-13` = last Intel runner. |
| `actions/checkout` | `v4` | Current stable. |
| `softprops/action-gh-release` | `v2` | Current stable. |
| `docker/build-push-action` | `v6` | Current stable. |
| `g++` on Ubuntu | `g++-13` | Explicit install to avoid GCC 14 libstdc++ issues on Ubuntu 24.04. |

---

## Sources

| Source | Confidence | URL |
|--------|------------|-----|
| Docker official C++ multi-stage guide | HIGH | https://docs.docker.com/guides/cpp/multistage |
| GCC Docker Hub official image | HIGH | https://hub.docker.com/_/gcc |
| Apple static linking documentation | HIGH | https://developer.apple.com/library/archive/qa/qa1118/_index.html |
| GitHub Actions runner images | HIGH | https://github.com/actions/runner-images |
| GitHub Actions ubuntu-latest changelog | HIGH | https://github.blog/changelog/2024-09-25-actions-new-images-and-ubuntu-latest-changes/ |
| scratch vs distroless comparison (2026) | MEDIUM | https://oneuptime.com/blog/post/2026-02-08-how-to-choose-between-scratch-and-distroless-base-images/view |
| install.sh patterns | MEDIUM | Community gists and release-lab/install repo |
| softprops/action-gh-release | HIGH | https://github.com/softprops/action-gh-release |
