# Feature Landscape: Distribution (Docker Image + Install Script)

**Domain:** Distribution & packaging for a niche academic single-binary CLI tool
**Researched:** 2026-02-28
**Confidence:** HIGH (well-established patterns; many reference implementations exist)

## Context

Kangaroo (`qseries`) is a ~1.5MB statically-linked C++ binary with zero runtime dependencies. Target users are mathematicians (number theory / partition theory researchers). The audience is small but technically literate enough to use a terminal. Current distribution: compile from source via `g++ -std=c++20 -O2 -static -o qseries main.cpp`.

---

## Install Script

### Table Stakes

Features users expect from a `curl | sh` installer. Missing any = script feels broken or untrustworthy.

| Feature | Why Expected | Complexity | Dependencies |
|---------|-------------|------------|--------------|
| **Platform detection** (`uname -s` / `uname -m`) | Must download correct binary for OS+arch | Low | GitHub Releases with platform-named assets |
| **HTTP tool fallback** (curl → wget → fetch) | Not all systems have curl | Low | None |
| **Error handling with clear messages** | Silent failures destroy trust | Low | None |
| **`-f`, `-s`, `-S`, `-L` curl flags** | Standard safety: fail on HTTP errors, follow redirects, show errors | Low | None |
| **SHA256 checksum verification** | Binary integrity after download; users piping to bash need assurance | Medium | Checksums file published alongside release assets |
| **Permission handling** (sudo detection) | `/usr/local/bin` requires root on most systems; user shouldn't guess | Low | None |
| **Default install to `/usr/local/bin`** | Standard location, already on PATH | Low | None |
| **Custom install dir via `--bin-dir`** | Some users lack root or want `$HOME/.local/bin` | Low | None |
| **Idempotent execution** | Running twice must not break anything | Low | None |
| **Colored terminal output** | Distinguishes success/error/info; every modern installer does this | Low | tput / ANSI codes |
| **POSIX sh compatibility** | Script must work on dash, ash, bash, zsh — not just bash | Low | None |

### Differentiators

Nice-to-have features that elevate the experience but aren't expected for a niche tool.

| Feature | Value Proposition | Complexity | Dependencies |
|---------|-------------------|------------|--------------|
| **Version selection** (`--version v1.2`) | Power users / CI pin versions; latest by default | Low | GitHub Releases API |
| **`--dry-run` flag** | Shows what will happen without doing it; builds trust | Low | None |
| **Post-install PATH advice** | If custom dir isn't on PATH, tell user what to add to `.bashrc`/`.zshrc` | Low | None |
| **Self-update** (`qseries --update`) | Saves user from re-running install script | Medium | Baked into binary or wrapper script |
| **Uninstall instructions** | Print `rm /usr/local/bin/qseries` on `--help` or in script output | Low | None |
| **Progress indicator during download** | curl's `-#` flag or a spinner; helpful for slow connections | Low | None |
| **Verification of install** (run `qseries --version` after install) | Confirms the binary actually works on this system | Low | None |
| **Fish shell PATH support** | Mathematicians on macOS may use fish | Low | None |

### Anti-Features

Features to explicitly NOT build. Over-engineering for a niche tool wastes effort.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| **GPG signature verification** | Requires users to import a public key; SHA256 is sufficient for this threat model | SHA256 checksums file alongside release assets |
| **Auto-PATH modification** (silently appending to `.bashrc`) | Surprising, contentious — many install scripts get criticized for this | Print instructions; let user decide |
| **Package manager submission** (Homebrew, apt, AUR, etc.) | Massive ongoing maintenance burden for a niche tool with tiny user base | Provide install script + manual compile |
| **Windows install script** (PowerShell) | Adds significant complexity; Windows users can use Cygwin/MSYS2 or WSL | Document WSL + compile-from-source for Windows |
| **npm/pip/cargo install wrapper** | Wrong ecosystem; adds dependencies where zero exist | Direct binary download |
| **Automatic shell restart / source** | Can't reliably `exec $SHELL` across all environments | Print "restart your terminal" |

---

## Docker Image

### Table Stakes

Features users expect from a Docker-distributed CLI tool. Missing = image feels broken.

| Feature | Why Expected | Complexity | Dependencies |
|---------|-------------|------------|--------------|
| **`FROM scratch` or `FROM alpine`** base | Static binary = no runtime deps; image should be < 5MB | Low | Static binary |
| **`docker run -it` interactive support** | REPL requires stdin/tty; must work with `-it` flags | Low | `ENTRYPOINT ["/qseries"]` |
| **Multi-arch manifest** (amd64 + arm64) | Apple Silicon Macs are now majority of dev machines; Rosetta adds startup lag | Medium | CI cross-compilation or QEMU builds |
| **Tagged versions** (`qseries:1.2`, `qseries:latest`) | Users need to pin versions in scripts/Dockerfiles | Low | CI tagging |
| **Minimal image size** | Static C++ binary → < 3MB image from scratch; anything bigger signals bloat | Low | Already static |
| **Non-root user** | Security best practice; some environments enforce it | Low | `USER` directive in Dockerfile |

### Differentiators

| Feature | Value Proposition | Complexity | Dependencies |
|---------|-------------------|------------|--------------|
| **Demo script baked in** | `docker run -it qseries demo` runs the Garvan tutorial automatically | Low | Copy `garvan-demo.sh` into image |
| **Volume mount for scripts** | `docker run -it -v ./scripts:/scripts qseries -f /scripts/my-computation.sh` | Low | Script mode already exists |
| **GitHub Container Registry** (ghcr.io) | Free for public repos; co-located with source; no Docker Hub rate limits | Low | GitHub Actions workflow |
| **OCI labels** (org.opencontainers.image.*) | Shows description, source URL, version in `docker inspect` | Low | Dockerfile LABEL directives |
| **Health check / smoke test in CI** | CI runs `docker run qseries --version` to verify image works before push | Low | CI workflow step |

### Anti-Features

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| **Docker Hub publishing** | Rate limits on pulls; requires separate Docker Hub account/org; GitHub Container Registry is free and better integrated | Use ghcr.io exclusively |
| **Alpine with apk packages** | Binary is static — adding a package manager adds attack surface for zero benefit | Use `FROM scratch` |
| **Docker Compose file** | Single binary, no services, no ports, no databases — Compose adds confusion | Just `docker run -it` |
| **Jupyter/notebook integration in Docker** | Wrong audience; mathematicians who want Jupyter use SageMath | Keep it a CLI REPL |
| **Docker-in-Docker or sidecar patterns** | No use case; this is a computational tool, not infrastructure | Single container |

---

## Feature Dependencies

```
GitHub Releases with platform-named assets
  ├── Install script (downloads from Releases)
  │     ├── Platform detection (uname)
  │     ├── Checksum verification (checksums.txt asset)
  │     └── Version selection (GitHub API /releases/latest)
  │
  └── Docker image (built in CI)
        ├── Multi-arch build (CI matrix or docker buildx)
        ├── ghcr.io push (GitHub Actions + GITHUB_TOKEN)
        └── Version tagging (git tag → image tag)

CI/CD workflow (GitHub Actions)
  ├── Builds binaries for linux-amd64, linux-arm64, (optionally macos)
  ├── Creates checksums.txt
  ├── Publishes GitHub Release with assets
  ├── Builds + pushes Docker image to ghcr.io
  └── Runs acceptance tests before release
```

**Critical dependency:** Both the install script and Docker image require **GitHub Releases with properly-named binary assets** and a **GitHub Actions CI workflow** that produces them. This CI workflow is the prerequisite for both features and should be built first.

---

## MVP Recommendation

### Priority 1: GitHub Actions CI → Release pipeline
Without this, neither install script nor Docker image has binaries to distribute. Build a workflow that:
- Triggers on git tag push (`v*`)
- Compiles static binaries (at minimum: `linux-amd64`)
- Creates `checksums.txt` (SHA256)
- Publishes a GitHub Release with binary + checksums as assets

### Priority 2: Install script
Higher value than Docker for this audience. Mathematicians are more likely to run `curl | sh` than to have Docker installed. Implement table stakes only:
- Platform detection, download, checksum verify, install to `/usr/local/bin`
- `--bin-dir` and `--version` flags
- POSIX sh compatible

### Priority 3: Docker image
Lower priority — Docker is less common among pure-math researchers. But trivial to add once CI exists:
- `FROM scratch`, copy static binary, `ENTRYPOINT ["/qseries"]`
- Push to `ghcr.io/gerardgarvan/qseries3`
- Multi-arch (amd64 + arm64)

### Defer

- Self-update mechanism (medium complexity, low value for niche tool)
- Package manager submissions (Homebrew, AUR, etc.) — only if user base grows significantly
- Windows-native install script — WSL is sufficient

---

## Reference Implementations

| Project | Install Pattern | Why Relevant |
|---------|----------------|--------------|
| [Starship](https://github.com/starship/starship/blob/master/install/install.sh) | Full-featured POSIX install.sh with platform detection, fallbacks, colored output | Gold standard for single-binary install scripts |
| [Deno](https://deno.land/x/install/install.sh) | Minimal install script, GitHub Releases | Clean, minimal reference |
| [PARI/GP Docker](https://github.com/piotr-semenov/parigp-docker) | Docker image for math REPL | Same domain (number theory tool in Docker) |
| [SageMath Docker](https://hub.docker.com/r/sagemath/sagemath) | Academic math tool in Docker | Larger but same audience |
| [Oh My Zsh](https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh) | Widely-copied install script pattern | PATH handling, shell detection |

---

## Sources

- Starship install.sh: https://github.com/starship/starship/blob/master/install/install.sh (HIGH confidence)
- curl best practices: https://www.joyfulbikeshedding.com/blog/2020-05-11-best-practices-when-using-curl-in-shell-scripts.html (HIGH confidence)
- GitHub Releases verification: https://docs.github.com/en/code-security/supply-chain-security/understanding-your-software-supply-chain/verifying-the-integrity-of-a-release (HIGH confidence)
- Docker minimal images: https://docs.docker.com/dhi/core-concepts/distroless (HIGH confidence)
- GitHub Container Registry vs Docker Hub: https://cloudonaut.io/versus/container-registry/docker-hub-vs-github-container-registry (MEDIUM confidence)
- GitHub Actions multi-platform builds: https://oneuptime.com/blog/post/2026-02-02-github-actions-multi-platform-builds/view (MEDIUM confidence)
- PARI/GP Docker: https://github.com/piotr-semenov/parigp-docker (HIGH confidence — same domain)
