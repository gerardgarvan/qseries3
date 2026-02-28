# Architecture: Distribution Features (Docker + Install Script + CI/CD)

**Project:** qseries3
**Researched:** 2026-02-28
**Overall confidence:** HIGH

## Executive Summary

The qseries binary is a fully static, zero-dependency C++20 executable. This makes distribution exceptionally straightforward: Docker images can use `scratch` (empty) as the runtime base, install scripts need only download a single file, and CI builds require nothing beyond `g++`. The main architectural decisions concern: (1) cross-platform build matrix in GitHub Actions, (2) Docker image layering, and (3) naming conventions for release assets.

---

## New Files

| File | Purpose | Location Rationale |
|------|---------|-------------------|
| `Dockerfile` | Multi-stage build → scratch-based runtime image | Repo root (Docker convention) |
| `.github/workflows/release.yml` | Build + test + release on tag push | GitHub Actions standard path |
| `.github/workflows/ci.yml` | Build + test on every push/PR | GitHub Actions standard path |
| `install.sh` | One-liner install: `curl -sSL … \| bash` | Repo root (discoverable, linked from README) |
| `.dockerignore` | Exclude `.planning/`, `.git/`, `build/`, `dist/`, etc. from Docker context | Repo root (Docker convention) |

## Modified Files

| File | Change | Reason |
|------|--------|--------|
| `Makefile` | Add targets: `docker-build`, `docker-push`, `docker-run` | Convenience wrappers; keeps all build commands in one place |
| `.gitignore` | Add `!.github/` exclusion (currently ignores dotfiles implicitly via `.cursor/`) | Ensure `.github/workflows/` is tracked |
| `README.md` | Add install instructions, Docker usage, badge for CI status | User-facing documentation |

---

## Integration Points

### 1. Makefile ↔ Docker

New Makefile targets wrap Docker CLI commands so developers don't need to remember image names/tags:

```makefile
DOCKER_IMAGE ?= ghcr.io/gerardgarvan/qseries
DOCKER_TAG   ?= latest

docker-build:
	docker build -t $(DOCKER_IMAGE):$(DOCKER_TAG) .

docker-run:
	docker run -it --rm $(DOCKER_IMAGE):$(DOCKER_TAG)

docker-push:
	docker push $(DOCKER_IMAGE):$(DOCKER_TAG)
```

These targets are additive — they don't modify existing targets. The `all` target remains unchanged.

### 2. Makefile ↔ GitHub Actions

GitHub Actions invokes the same `make` commands used locally. This is critical: the CI build must be identical to the local build. The workflow uses:

```yaml
- run: make CXX=g++ LDFLAGS=-static    # build
- run: make acceptance                   # test
```

The existing Makefile already supports `CXX` and `LDFLAGS` overrides, so no changes are needed for the build step itself.

### 3. GitHub Actions ↔ GitHub Releases

The release workflow triggers on version tags (`v*`), builds for each platform, then uploads binaries as release assets using `softprops/action-gh-release@v2`. Asset naming convention:

| Platform | Asset Name |
|----------|-----------|
| Linux x86_64 | `qseries-linux-amd64` |
| Linux ARM64 | `qseries-linux-arm64` |
| macOS x86_64 | `qseries-darwin-amd64` |
| macOS ARM64 | `qseries-darwin-arm64` |
| Windows x86_64 | `qseries-windows-amd64.exe` |

### 4. Install Script ↔ GitHub Releases

`install.sh` calls the GitHub API to find the latest release, detects OS/arch via `uname`, downloads the matching asset, and places it in `/usr/local/bin` (or `~/.local/bin` without root). It depends on the asset naming convention above.

### 5. Docker Image ↔ Binary

The Dockerfile builds the binary inside the container (ensuring reproducibility) and copies it to `scratch`. The REPL's TTY detection (`stdin_is_tty()` in `repl.h`) already handles non-TTY mode gracefully — when piped, it suppresses the prompt. Docker interactive mode (`docker run -it`) provides a TTY, so the REPL works normally.

### 6. Acceptance Tests ↔ CI

The existing `tests/acceptance.sh` already locates the binary flexibly (checks `./dist/qseries.exe`, `./dist/qseries`, `./qseries.exe`, `./qseries`). This works in CI without modification. The script uses `set -e` and exits non-zero on failure, which integrates cleanly with GitHub Actions step failure detection.

---

## Build/Release Pipeline

### Continuous Integration (every push / PR)

```
┌─────────────┐    ┌──────────────┐    ┌──────────────────┐
│  git push /  │───▶│ Build binary │───▶│ Run acceptance   │
│  open PR     │    │ (Linux g++)  │    │ tests            │
└─────────────┘    └──────────────┘    └──────────────────┘
```

Single platform (Linux) for speed. Goal: catch regressions within minutes.

### Release (on tag push `v*`)

```
┌──────────┐    ┌─────────────────────────────────────────┐
│ git tag  │    │          Matrix Build (parallel)         │
│ v3.1.0   │───▶│  ┌─────────┐ ┌─────────┐ ┌──────────┐  │
│ git push │    │  │ Linux   │ │ macOS   │ │ Windows  │  │
│ --tags   │    │  │ amd64   │ │ arm64   │ │ amd64    │  │
│          │    │  │ + arm64 │ │ + amd64 │ │          │  │
│          │    │  └────┬────┘ └────┬────┘ └─────┬────┘  │
│          │    │       │          │             │        │
│          │    └───────┼──────────┼─────────────┼────────┘
│          │            ▼          ▼             ▼
│          │    ┌─────────────────────────────────────────┐
│          │    │  Run acceptance tests on each platform  │
│          │    └────────────────┬────────────────────────┘
│          │                    ▼
│          │    ┌─────────────────────────────────────────┐
│          │    │  Upload binaries to GitHub Release      │
│          │    │  Build + push Docker image (linux/amd64 │
│          │    │  + linux/arm64 multi-arch)              │
│          │    └─────────────────────────────────────────┘
└──────────┘
```

### Docker Image Build (part of release)

```
┌───────────────────────────────────────────────────────────┐
│  Stage 1: Build (gcc:14 or ubuntu:24.04)                  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ apt-get install g++                                  │  │
│  │ COPY src/ /build/src/                                │  │
│  │ g++ -std=c++20 -O2 -static -o qseries src/main.cpp  │  │
│  │ strip qseries                                        │  │
│  └──────────────────────────────────────────────────────┘  │
│                          │                                 │
│                          ▼                                 │
│  Stage 2: Runtime (scratch)                                │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ COPY --from=build /build/qseries /qseries            │  │
│  │ ENTRYPOINT ["/qseries"]                               │  │
│  │ Final image size: ~3-5 MB                             │  │
│  └──────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────┘
```

---

## Detailed Component Design

### Dockerfile

```dockerfile
# Build stage
FROM gcc:14 AS build
WORKDIR /build
COPY src/ src/
RUN g++ -std=c++20 -O2 -static -o qseries src/main.cpp && strip qseries

# Runtime stage — empty image, only the binary
FROM scratch
COPY --from=build /build/qseries /qseries
ENTRYPOINT ["/qseries"]
```

**Why `scratch` over distroless or alpine:**
- The binary is fully static (`-static` flag), no runtime dependencies
- `scratch` produces the smallest image (~3-5 MB vs ~9 MB distroless vs ~80 MB alpine)
- The REPL only needs stdin/stdout, which Docker provides
- No shell is needed in the container — the binary IS the shell (it's a REPL)
- Debugging: `docker cp` can extract the binary if needed; for interactive debugging, build with the `build` stage target directly (`docker build --target build`)

**Why `gcc:14` for build stage (not alpine/musl):**
- `gcc:14` provides glibc-based g++ which is the project's primary compiler
- Static linking with glibc via `-static` produces universally compatible Linux binaries
- Alpine/musl can produce slightly smaller binaries but risks subtle C++ stdlib behavior differences
- Build stage size is irrelevant (discarded after build)

### .dockerignore

```
.git
.github
.planning
.cursor
build/
dist/
emsdk/
*.exe
*.o
tests/
demo/
docs/
*.md
```

Keeps the Docker build context minimal (only `src/` directory is needed).

### GitHub Actions: ci.yml

```yaml
name: CI
on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: g++ -std=c++20 -O2 -static -o dist/qseries src/main.cpp
      - name: Unit tests
        run: ./dist/qseries --test
      - name: Acceptance tests
        run: make acceptance
```

Lightweight — single platform, fast feedback.

### GitHub Actions: release.yml

```yaml
name: Release
on:
  push:
    tags: ['v*']

jobs:
  build:
    strategy:
      matrix:
        include:
          - os: ubuntu-latest
            target: linux-amd64
            cc: g++
            flags: "-static"
            ext: ""
          - os: ubuntu-latest
            target: linux-arm64
            cc: aarch64-linux-gnu-g++
            flags: "-static"
            ext: ""
            cross: true
          - os: macos-latest         # arm64 runner
            target: darwin-arm64
            cc: g++-14
            flags: ""               # macOS doesn't support -static
            ext: ""
          - os: macos-13             # x86_64 runner
            target: darwin-amd64
            cc: g++-14
            flags: ""
            ext: ""
          - os: windows-latest
            target: windows-amd64
            cc: g++
            flags: "-static"
            ext: ".exe"
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v4

      - name: Install cross-compiler (ARM64)
        if: matrix.cross
        run: sudo apt-get update && sudo apt-get install -y g++-aarch64-linux-gnu

      - name: Build
        run: >
          ${{ matrix.cc }} -std=c++20 -O2 ${{ matrix.flags }}
          -o qseries-${{ matrix.target }}${{ matrix.ext }} src/main.cpp

      - name: Test (native only)
        if: "!matrix.cross"
        run: ./qseries-${{ matrix.target }}${{ matrix.ext }} --test

      - uses: actions/upload-artifact@v4
        with:
          name: qseries-${{ matrix.target }}
          path: qseries-${{ matrix.target }}${{ matrix.ext }}

  release:
    needs: build
    runs-on: ubuntu-latest
    permissions:
      contents: write
    steps:
      - uses: actions/download-artifact@v4
        with:
          merge-multiple: true

      - name: Create Release
        uses: softprops/action-gh-release@v2
        with:
          files: qseries-*
          generate_release_notes: true

  docker:
    needs: build
    runs-on: ubuntu-latest
    permissions:
      packages: write
    steps:
      - uses: actions/checkout@v4
      - uses: docker/setup-buildx-action@v3
      - uses: docker/login-action@v3
        with:
          registry: ghcr.io
          username: ${{ github.actor }}
          password: ${{ secrets.GITHUB_TOKEN }}
      - uses: docker/build-push-action@v6
        with:
          push: true
          tags: |
            ghcr.io/gerardgarvan/qseries:${{ github.ref_name }}
            ghcr.io/gerardgarvan/qseries:latest
          platforms: linux/amd64
```

### install.sh

```bash
#!/usr/bin/env bash
set -euo pipefail

REPO="gerardgarvan/qseries3"
INSTALL_DIR="${INSTALL_DIR:-/usr/local/bin}"

# Detect platform
OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"
case "$OS" in
  linux*)  OS="linux" ;;
  darwin*) OS="darwin" ;;
  *)       echo "Unsupported OS: $OS" >&2; exit 1 ;;
esac
case "$ARCH" in
  x86_64|amd64) ARCH="amd64" ;;
  aarch64|arm64) ARCH="arm64" ;;
  *)             echo "Unsupported arch: $ARCH" >&2; exit 1 ;;
esac

ASSET="qseries-${OS}-${ARCH}"
TAG=$(curl -fsSL "https://api.github.com/repos/${REPO}/releases/latest" \
  | grep '"tag_name"' | sed -E 's/.*"([^"]+)".*/\1/')

URL="https://github.com/${REPO}/releases/download/${TAG}/${ASSET}"

echo "Installing qseries ${TAG} (${OS}/${ARCH})..."
if [ -w "$INSTALL_DIR" ]; then
  curl -fsSL -o "${INSTALL_DIR}/qseries" "$URL"
  chmod +x "${INSTALL_DIR}/qseries"
else
  sudo curl -fsSL -o "${INSTALL_DIR}/qseries" "$URL"
  sudo chmod +x "${INSTALL_DIR}/qseries"
fi

echo "Installed: $(qseries --version)"
```

---

## Build Order

Build order considers both dependency relationships and risk profile:

### Phase 1: CI Pipeline (ci.yml)
**Dependencies:** None — works with existing Makefile and tests.
**Why first:** Establishes quality gate before any other changes. Every subsequent PR gets tested.

### Phase 2: Dockerfile + .dockerignore + Makefile docker targets
**Dependencies:** None technically, but CI should exist first so the Dockerfile PR itself gets tested.
**Why second:** Self-contained, easy to validate locally. `docker build . && docker run -it` provides immediate feedback.

### Phase 3: Release Pipeline (release.yml)
**Dependencies:** Needs the asset naming convention finalized (Phase 2 validates the Linux build path). Needs CI passing.
**Why third:** Most complex piece. Cross-compilation matrix needs careful testing. Start with Linux-only, then expand the matrix.

### Phase 4: Install Script (install.sh)
**Dependencies:** Requires at least one release to exist in GitHub Releases (needs Phase 3).
**Why last:** Cannot be tested until binaries are actually published. The script's correctness depends entirely on the asset naming convention from Phase 3.

### Dependency Graph

```
Phase 1: ci.yml
    │
    ▼
Phase 2: Dockerfile + .dockerignore + Makefile targets
    │
    ▼
Phase 3: release.yml (cross-platform builds → GitHub Releases)
    │
    ▼
Phase 4: install.sh (downloads from GitHub Releases)
```

---

## Key Constraints & Decisions

### macOS Static Linking
macOS does not support fully static linking (`-static` is rejected by Apple's linker). macOS binaries will be dynamically linked against system libraries (libSystem). This is standard practice — Homebrew, Rust, Go all ship dynamically-linked macOS binaries. The binary still has zero *external* dependencies; it links only against OS-provided libraries.

### Container Registry
Use GitHub Container Registry (`ghcr.io`) rather than Docker Hub. Rationale:
- Integrated with GitHub (same auth, same permissions)
- Free for public repositories
- No rate limiting for pulls from the same org

### Version Tagging Strategy
Git tags drive everything: `git tag v3.1.0 && git push --tags` triggers the release workflow. The tag becomes both the GitHub Release name and the Docker image tag. The binary's `--version` output should match. Currently hardcoded as `"qseries 2.0"` in `main.cpp` — consider making this build-time configurable via `-DVERSION=...` preprocessor define.

### Windows Build in CI
GitHub Actions `windows-latest` runners have MSYS2/MinGW g++ available. Use `g++` directly (the runner has it on PATH). The `-static` flag works on Windows/MinGW to produce a standalone `.exe`.

### ARM64 Linux Cross-Compilation
Use `g++-aarch64-linux-gnu` on Ubuntu runners. Static linking ensures the cross-compiled binary runs on any ARM64 Linux. Cannot run acceptance tests for cross-compiled binaries without QEMU, so unit tests (`--test`) are skipped for cross targets.

---

## Confidence Assessment

| Area | Confidence | Rationale |
|------|------------|-----------|
| Dockerfile multi-stage with scratch | HIGH | Verified with Docker official docs; pattern is standard for static C++ binaries |
| GitHub Actions matrix build | HIGH | Well-documented, widely used pattern for multi-platform C++ projects |
| Install script platform detection | HIGH | Standard `uname` approach used by virtually every CLI tool installer |
| macOS build constraints | HIGH | Apple's `-static` limitation is well-documented |
| Windows CI build | MEDIUM | MinGW g++ is available on GitHub Actions runners but exact path/version may need verification at implementation time |
| ARM64 cross-compilation | MEDIUM | `g++-aarch64-linux-gnu` package is standard but acceptance test coverage is limited for cross targets |
| GHCR multi-arch images | MEDIUM | `docker buildx` multi-arch is mature but adds complexity; recommend starting with amd64-only |
