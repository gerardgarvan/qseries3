# Domain Pitfalls — Distribution Features (Docker Image + Install Script)

**Domain:** Adding Docker image and install script distribution to an existing zero-dependency C++20 REPL
**Researched:** 2026-02-28
**Overall confidence:** HIGH (verified with official docs, Docker docs, Apple Developer docs, community experience)

---

## Critical Pitfalls

Mistakes that cause broken installs, non-functional Docker containers, or security vulnerabilities.

---

### Pitfall 1: macOS Does Not Support Fully Static Binaries

**What goes wrong:** Using `-static` on macOS produces a binary that either fails to link or is killed on execution. On Apple Silicon (arm64), the XNU kernel's Mach-O loader explicitly rejects static binaries.

**Why it happens:** Apple does not guarantee binary compatibility at the kernel syscall level. All macOS programs must dynamically link to `libSystem.dylib` (which wraps libc, libpthread, etc.). Apple's official documentation (QA1118) states: "Apple does not support statically linked binaries on Mac OS X. A statically linked binary assumes binary compatibility at the kernel syscall interface, and we do not make any guarantees on that front." On arm64/Apple Silicon, the kernel enforces this — static executables are killed with `SIGKILL`.

**Consequences:**
- The build command `g++ -std=c++20 -O2 -static -o qseries main.cpp` that works on Linux **will fail on macOS**
- x86_64 macOS: binary may compile but is officially unsupported and may break on OS updates
- arm64 macOS: binary is killed immediately on execution
- Users or CI running on macOS get confusing build failures or runtime crashes

**Prevention:**
1. On macOS, omit `-static` entirely — the resulting binary will dynamically link to system `libSystem.dylib` but nothing else (no external dependencies to worry about since the project has zero deps)
2. In the Makefile, detect the OS and conditionally set `LDFLAGS`:
   ```makefile
   UNAME_S := $(shell uname -s)
   ifeq ($(UNAME_S),Darwin)
       LDFLAGS =
   else
       LDFLAGS = -static
   endif
   ```
3. In the install script, do NOT pass `-static` when `uname` reports `Darwin`
4. Document in README that macOS binaries are dynamically linked to system libraries (this is expected and fine)

**Detection:** CI builds on macOS runners will fail or produce killed binaries if `-static` is passed.
**Severity:** CRITICAL — blocks macOS distribution entirely if not handled
**Phase:** Docker/install script phase — must be addressed in build matrix and install script logic

---

### Pitfall 2: Docker Container Lacks TTY — REPL Hangs or Crashes

**What goes wrong:** Running `docker run qseries` (without `-it` flags) causes the REPL to either hang waiting for input, produce no output, or crash because `tcgetattr()` fails on a non-TTY stdin.

**Why it happens:** Docker containers don't allocate a pseudo-TTY by default. The qseries REPL uses `termios.h` functions (`tcgetattr`, `tcsetattr`) to enter raw mode for tab completion and arrow key navigation. Without a TTY, these calls fail. The REPL also checks `stdin_is_tty()` (`isatty(fileno(stdin))`) which returns false in a bare container.

**Consequences:**
- `docker run qseries` with no flags: REPL detects non-TTY stdin, falls into script mode (reads stdin line-by-line), then exits immediately since stdin is empty
- Users unfamiliar with Docker (mathematicians!) think the tool is broken
- If the REPL doesn't gracefully handle `tcgetattr` failure, it could crash or enter an undefined state

**Prevention:**
1. In `README.md` and Dockerfile, prominently document: `docker run -it qseries`
2. In the Docker image's `ENTRYPOINT` or a wrapper script, detect missing TTY and print a helpful error:
   ```bash
   if [ ! -t 0 ]; then
     echo "Error: qseries is interactive. Run with: docker run -it qseries"
     exit 1
   fi
   ```
3. Verify that the REPL's `RawModeGuard` constructor gracefully handles `tcgetattr` returning -1 (it currently does — sets `active = false`)
4. Consider supporting `docker run qseries <<< "etaq(0,20)"` for non-interactive single-expression mode (script mode already handles this)
5. Set `ENTRYPOINT ["/usr/local/bin/qseries"]` (exec form) in Dockerfile so signals propagate correctly

**Detection:** Test `docker run qseries` without `-it` — it should produce a clear error message or sensible fallback, not a hang.
**Severity:** CRITICAL — default `docker run` experience is completely broken without mitigation
**Phase:** Docker image phase — Dockerfile and entry point script

---

### Pitfall 3: glibc Static Linking Warnings and NSS Runtime Failures

**What goes wrong:** Compiling with `-static` on a glibc-based Linux system (Ubuntu, Debian) produces warnings about NSS (Name Service Switch) and may cause runtime failures for DNS-related functions. More importantly, the resulting "static" binary may still require shared libraries at runtime.

**Why it happens:** glibc's NSS subsystem (`/etc/nsswitch.conf`) dynamically loads `libnss_*.so` libraries at runtime, even when the binary is compiled with `-static`. This is an architectural decision in glibc — static linking doesn't fully work. The compiler emits: "Using 'getaddrinfo' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking."

**Why this matters for qseries:** The qseries REPL itself does zero networking, so NSS/getaddrinfo isn't a direct concern. However, a glibc-static binary may not be truly portable across Linux distributions because glibc version compatibility is not guaranteed. The binary baked in one glibc version's symbols and layout.

**Prevention:**
1. Use Alpine Linux (musl libc) as the Docker build base — musl has zero issues with static linking and produces truly portable binaries
2. Docker multi-stage build: compile on `alpine:3.20` with `apk add g++`, producing a genuinely portable static binary
3. For the install script's pre-built binaries, compile Linux binaries on Alpine/musl to guarantee portability
4. Do NOT compile release binaries on Ubuntu/Debian if distributing as static — use Alpine or cross-compile with musl

**Detection:** Run `ldd dist/qseries` — should report "not a dynamic executable" (truly static). If it lists shared libraries, the binary isn't actually static.
**Severity:** CRITICAL — "static" binaries that secretly require shared libs will fail on other distros
**Phase:** Docker image phase (build stage) and CI/CD phase (release binary builds)

---

### Pitfall 4: Install Script Security — curl | bash Risks

**What goes wrong:** The `curl -sSL https://example.com/install.sh | bash` pattern exposes users to supply-chain attacks, partial-download execution, and content-sniffing attacks.

**Why it happens:** Three distinct attack vectors:
1. **Server-side user-agent sniffing:** A compromised server can detect when a script is being piped to bash (vs. viewed in a browser) and serve different content — malicious payload to `curl`, clean script to browsers
2. **Partial download:** If the connection drops mid-transfer, bash executes the incomplete script, potentially leaving the system in a broken state (e.g., PATH modified but binary not installed)
3. **MITM without verification:** If the URL uses HTTP or the user disables certificate verification, the script can be modified in transit

**Consequences:**
- Mathematician users (not security-savvy) may blindly run `curl | sudo bash`
- A compromised GitHub Pages or CDN could distribute malware
- Partial downloads could corrupt user's PATH or leave orphaned files

**Prevention:**
1. Always use HTTPS URLs — never offer HTTP install URLs
2. Provide checksum verification in install instructions:
   ```bash
   curl -sSL https://github.com/.../install.sh -o install.sh
   sha256sum install.sh  # User verifies against published hash
   bash install.sh
   ```
3. Make the install script idempotent — re-running it should be safe
4. Use `set -euo pipefail` at the top of the install script so partial execution fails fast
5. Wrap the entire script in a function that's called at the end, preventing partial-download execution:
   ```bash
   #!/bin/bash
   set -euo pipefail
   main() {
       # ... all install logic ...
   }
   main "$@"
   ```
6. Never require `sudo` in the install script — install to `~/.local/bin` or `~/bin` instead
7. Offer alternative install methods: manual download, `brew tap`, or just `git clone && make`

**Detection:** Code review the install script for all these patterns before publishing.
**Severity:** CRITICAL — security vulnerability affecting user trust
**Phase:** Install script phase

---

## Major Pitfalls

Mistakes that cause significant usability problems or broken CI pipelines.

---

### Pitfall 5: Docker CRLF Line Ending Corruption with -t Flag

**What goes wrong:** When Docker allocates a TTY (`-t` flag), it converts `\n` to `\r\n` (CRLF) in output. This corrupts output when piped from a Docker container, breaking scripts that parse qseries output.

**Why it happens:** Docker's `-t` flag puts the terminal in raw mode and enables the TTY's `onlcr` (output NL to CR-NL) translation. This is standard TTY behavior but breaks programmatic output parsing.

**Consequences:**
- `docker run -it qseries <<< "etaq(0,10)" | head -1` contains invisible `\r` characters
- Scripts that compare Docker output to expected output fail on line endings
- CI tests piping into Docker may produce spurious failures

**Prevention:**
1. Document that `-it` is for interactive use only; for scripted use, use `-i` without `-t`:
   ```bash
   echo "etaq(0,10)" | docker run -i qseries  # Correct for scripting
   docker run -it qseries                       # Correct for interactive
   ```
2. In CI tests against Docker, always use `-i` (not `-it`) and strip trailing `\r` defensively
3. The REPL already handles script mode (non-TTY stdin) correctly, so `-i` without `-t` works naturally

**Detection:** Compare `docker run -it qseries <<< "1+1" | xxd | head` output — look for `0d 0a` (CRLF) vs `0a` (LF).
**Severity:** MAJOR — breaks scripted usage of Docker image
**Phase:** Docker image phase — document in README, test in CI

---

### Pitfall 6: Alpine/musl Locale and UTF-8 Issues

**What goes wrong:** The qseries REPL running in an Alpine Docker container may mishandle Unicode characters (mathematical symbols in output or user input) because Alpine's musl libc has limited locale support compared to glibc.

**Why it happens:** musl libc doesn't support the full glibc locale system. By default, Alpine containers have no locale set, and `setlocale(LC_ALL, "")` may not enable UTF-8 as expected.

**Consequences:**
- Mathematical symbols or special characters in output may display as garbage
- Users typing Unicode in the REPL may see corrupted input echoing
- `std::locale` operations may behave differently than on desktop Linux

**Prevention:**
1. Set `ENV LANG=C.UTF-8` in the Dockerfile — musl has built-in support for C.UTF-8
2. The qseries REPL doesn't currently use `setlocale` or `std::locale` heavily (it deals in ASCII math notation), so impact is low
3. Test with non-ASCII input to verify behavior

**Detection:** Run `docker run -it qseries` and type/paste a Unicode character — verify it doesn't crash.
**Severity:** MAJOR if the REPL processes any non-ASCII, MINOR for pure ASCII math
**Phase:** Docker image phase — single ENV line in Dockerfile

---

### Pitfall 7: PATH Management Across Shells (bash, zsh, fish)

**What goes wrong:** The install script adds the binary to `PATH` by modifying the wrong shell config file, or modifies a file that isn't sourced in the user's shell, leaving the binary not on `PATH` despite a "successful" install.

**Why it happens:** Different shells read different config files:
- **bash**: `~/.bashrc` (interactive) vs `~/.bash_profile` (login) — and bash login shells DON'T read `~/.bashrc` unless `~/.bash_profile` sources it
- **zsh** (default on macOS since Catalina): `~/.zshrc`
- **fish**: `~/.config/fish/config.fish` (completely different syntax: `set PATH $PATH /new/path` instead of `export PATH=`)

Modifying only `~/.bashrc` fails for macOS users (who use zsh by default). Modifying `~/.profile` works for bash login shells but not zsh or fish.

**Consequences:**
- User installs successfully, opens a new terminal, types `qseries`, gets "command not found"
- macOS users (zsh default) are the most likely to be affected
- fish users need completely different syntax and are always forgotten

**Prevention:**
1. Install binary to a directory already on most PATH configurations:
   - `~/.local/bin` (XDG standard, often already on PATH in modern distros)
   - `/usr/local/bin` (requires sudo — avoid if possible)
2. Detect the user's current shell from `$SHELL` and modify the correct file:
   ```bash
   case "$(basename "$SHELL")" in
       bash) RC_FILE="$HOME/.bashrc" ;;
       zsh)  RC_FILE="$HOME/.zshrc" ;;
       fish) RC_FILE="$HOME/.config/fish/config.fish" ;;
       *)    RC_FILE="$HOME/.profile" ;;
   esac
   ```
3. For fish, use `fish_add_path` or `set -Ua fish_user_paths ~/.local/bin` instead of `export PATH=...`
4. Check if the directory is already on PATH before modifying any config files
5. Print clear instructions at the end: "Restart your terminal or run: `source ~/.zshrc`"
6. Prefer `~/.local/bin` and tell users to add it themselves if it's not on PATH — less magical, more reliable

**Detection:** Test the install script with bash, zsh, and fish shells. Verify `which qseries` works in a new terminal session for each.
**Severity:** MAJOR — "command not found" after successful install is a terrible first experience
**Phase:** Install script phase

---

### Pitfall 8: GitHub Actions arm64 Linux Build — QEMU Emulation is Extremely Slow

**What goes wrong:** Cross-compiling or building arm64 Linux binaries on GitHub Actions using QEMU emulation takes 10-50x longer than native builds, causing CI timeouts and wasted minutes.

**Why it happens:** GitHub now offers native arm64 Linux runners (`ubuntu-24.04-arm`), but many tutorials and existing workflows still use `docker/setup-qemu-action` + `docker buildx` for multi-arch builds. QEMU user-space emulation of aarch64 on x86_64 runners is functional but extremely slow for compilation.

**Consequences:**
- A 30-second native build becomes 10-25 minutes under QEMU
- CI minutes quota depletes rapidly
- Flaky timeouts on compilation-heavy builds

**Prevention:**
1. Use GitHub's native arm64 runners: `runs-on: ubuntu-24.04-arm` (GA since Jan 2026)
2. For Docker multi-arch images, use a build matrix with native runners per arch instead of QEMU:
   ```yaml
   strategy:
     matrix:
       include:
         - runner: ubuntu-24.04
           arch: amd64
         - runner: ubuntu-24.04-arm
           arch: arm64
   runs-on: ${{ matrix.runner }}
   ```
3. If native runners aren't available (e.g., in public repos with limited minutes), cross-compile instead of emulating: use `aarch64-linux-gnu-g++` on an x86_64 runner

**Detection:** Monitor CI build times — if arm64 steps take >5 minutes for a simple C++ compile, you're using emulation.
**Severity:** MAJOR — wastes CI time and budget, may cause timeout failures
**Phase:** CI/CD phase (if adding GitHub Actions for release builds)

---

### Pitfall 9: Docker Signal Handling — Ctrl+C Doesn't Kill the REPL

**What goes wrong:** Pressing Ctrl+C in `docker run -it qseries` doesn't send SIGINT to the REPL process, leaving users stuck with no way to exit except `docker kill`.

**Why it happens:** Two common causes:
1. Using shell form `ENTRYPOINT` (`ENTRYPOINT /usr/local/bin/qseries`) wraps the process in `/bin/sh -c`, so signals go to shell PID 1, not the REPL
2. PID 1 in a container has special signal semantics — it doesn't receive the default signal handlers, so SIGINT/SIGTERM are ignored unless explicitly handled

**Consequences:**
- Users can't exit the REPL with Ctrl+C (expected behavior)
- Users can't exit with Ctrl+D if the REPL doesn't handle EOF
- `docker stop qseries` waits the full 10-second timeout before force-killing

**Prevention:**
1. Use exec form for ENTRYPOINT: `ENTRYPOINT ["/usr/local/bin/qseries"]` — this runs the binary as PID 1 directly, no shell wrapper
2. Alternatively, use `tini` as an init process (Alpine: `apk add tini`, then `ENTRYPOINT ["/sbin/tini", "--", "/usr/local/bin/qseries"]`) — tini handles signal forwarding properly
3. Verify the REPL handles SIGINT (Ctrl+C) and EOF (Ctrl+D) gracefully — the current code already handles these in the raw mode input loop
4. Add a `STOPSIGNAL SIGINT` to the Dockerfile if SIGTERM isn't handled

**Detection:** Run `docker run -it qseries`, press Ctrl+C — verify it exits cleanly. Run `docker stop <container>` — verify it stops within 1-2 seconds, not 10.
**Severity:** MAJOR — users get stuck in the container
**Phase:** Docker image phase — ENTRYPOINT format and signal handling

---

## Moderate Pitfalls

Mistakes that cause confusing errors or partial failures.

---

### Pitfall 10: Docker Multi-Stage Build — Missing Static Libs in Alpine

**What goes wrong:** The Alpine build stage fails with linker errors because static libraries for libstdc++ or libgcc aren't installed.

**Why it happens:** Alpine's `g++` package doesn't always include `libstdc++.a` (the static version). By default, `apk add g++` installs the shared library. Static linking requires `gcc-static` (for libgcc) and possibly building with specific flags.

**Prevention:**
1. Install the right packages: `apk add g++ musl-dev` (musl-dev provides static libc)
2. Verify with a test build in the Docker stage that `ldd` reports "not a dynamic executable"
3. Multi-stage Dockerfile pattern:
   ```dockerfile
   FROM alpine:3.20 AS builder
   RUN apk add --no-cache g++ musl-dev make
   COPY src/ src/
   COPY Makefile .
   RUN make LDFLAGS=-static

   FROM alpine:3.20
   COPY --from=builder /app/dist/qseries /usr/local/bin/qseries
   ENTRYPOINT ["/usr/local/bin/qseries"]
   ```
4. The final stage can even use `FROM scratch` since the binary is fully static — smallest possible image

**Detection:** Build fails at link time with "cannot find -lstdc++" or similar.
**Severity:** MODERATE — blocks Docker build but easy to fix once diagnosed
**Phase:** Docker image phase

---

### Pitfall 11: Install Script Assumes `curl` or `wget` Exists

**What goes wrong:** The install script uses `curl` to download the binary, but the user's system doesn't have `curl` installed (some minimal Linux installs, some Docker base images, older systems).

**Prevention:**
1. Try `curl` first, fall back to `wget`:
   ```bash
   if command -v curl &>/dev/null; then
       curl -sSL "$URL" -o "$DEST"
   elif command -v wget &>/dev/null; then
       wget -q "$URL" -O "$DEST"
   else
       echo "Error: curl or wget required. Install one and retry."
       exit 1
   fi
   ```
2. Also check for `tar` or `unzip` if distributing archives
3. Check for `chmod` availability (it's standard but good to be defensive)

**Detection:** Test install script on minimal Docker images (`debian:slim`, `alpine`).
**Severity:** MODERATE — install fails with confusing error
**Phase:** Install script phase

---

### Pitfall 12: Binary Naming Convention — `.exe` Extension on Linux/macOS

**What goes wrong:** The current Makefile builds `dist/qseries.exe` — this works on Cygwin/Windows but looks wrong on Linux/macOS and confuses users.

**Why it happens:** The Makefile was written in a Cygwin environment where `.exe` is conventional. On Linux/macOS, executable binaries typically have no extension.

**Consequences:**
- Linux/macOS users must type `./qseries.exe` which feels wrong
- Package managers and install scripts typically expect extensionless binaries
- Docker images carrying `qseries.exe` look like Windows artifacts

**Prevention:**
1. In the Makefile, conditionally name the binary:
   ```makefile
   ifeq ($(OS),Windows_NT)
       BINARY = dist/qseries.exe
   else
       BINARY = dist/qseries
   endif
   ```
2. In the install script, always install as `qseries` (no extension)
3. In the Dockerfile, the built binary should be `/usr/local/bin/qseries`

**Detection:** Check `ls dist/` after building on Linux — `.exe` extension is the warning sign.
**Severity:** MODERATE — cosmetic but affects user perception and scripting
**Phase:** Docker + install script phase — both need the correct binary name

---

### Pitfall 13: Install Script Doesn't Verify Architecture Match

**What goes wrong:** User on arm64 macOS downloads the x86_64 binary (or vice versa), resulting in "Exec format error" or silent Rosetta 2 emulation with degraded performance.

**Prevention:**
1. Detect architecture in the install script:
   ```bash
   ARCH=$(uname -m)
   case "$ARCH" in
       x86_64|amd64)  ARCH="x86_64" ;;
       aarch64|arm64) ARCH="arm64" ;;
       *) echo "Unsupported architecture: $ARCH"; exit 1 ;;
   esac
   ```
2. Construct download URL with both OS and arch: `qseries-${OS}-${ARCH}`
3. On macOS with Rosetta 2, `uname -m` may report `x86_64` even on arm64 hardware — check `sysctl -n sysctl.proc_translated 2>/dev/null` to detect Rosetta

**Detection:** Run `file qseries` to check the binary's architecture matches the host.
**Severity:** MODERATE — "Exec format error" is confusing for non-technical users
**Phase:** Install script phase and CI release matrix

---

### Pitfall 14: Docker Image Too Large — Shipping Build Tools in Runtime Image

**What goes wrong:** A single-stage Dockerfile includes `g++`, `make`, header files, and source code in the final image, inflating it from ~5 MB to 500+ MB.

**Prevention:**
1. Always use multi-stage builds — build stage has compilers, runtime stage has only the binary
2. For a fully static binary, the final stage can be `FROM scratch` (0 bytes base) or `FROM alpine:3.20` (~7 MB base)
3. Expected image sizes:
   - `FROM scratch` + static binary: ~2-4 MB
   - `FROM alpine` + static binary: ~10-12 MB
   - `FROM ubuntu` + dynamic binary: ~80+ MB (bad)
   - Single-stage with build tools: ~500+ MB (terrible)

**Detection:** Run `docker images qseries` — if size exceeds 20 MB, investigate.
**Severity:** MODERATE — wastes bandwidth, slow pulls, bad first impression
**Phase:** Docker image phase

---

### Pitfall 15: Cross-Platform GitHub Release Matrix Misses a Target

**What goes wrong:** The CI release workflow builds binaries for some OS/arch combos but misses one (e.g., Linux arm64, or macOS x86_64), leaving some users without a pre-built binary.

**Prevention:**
1. Define the full target matrix explicitly:
   | OS | Arch | Runner | Static? | Notes |
   |---|---|---|---|---|
   | Linux | x86_64 | `ubuntu-24.04` | Yes (musl) | Build on Alpine in Docker |
   | Linux | arm64 | `ubuntu-24.04-arm` | Yes (musl) | Native ARM runner |
   | macOS | arm64 | `macos-26` | No (-static omitted) | Apple Silicon native |
   | macOS | x86_64 | `macos-26-large` | No (-static omitted) | Intel Mac |
   | Windows | x86_64 | `windows-latest` | Yes | MinGW or MSVC |
2. Name artifacts consistently: `qseries-linux-x86_64`, `qseries-linux-arm64`, `qseries-darwin-arm64`, `qseries-darwin-x86_64`, `qseries-windows-x86_64.exe`
3. Test each artifact in CI (at least `./qseries --version` or `echo "1+1" | ./qseries`)

**Detection:** Check GitHub release assets after each release — count should match the matrix.
**Severity:** MODERATE — some user cohort can't install
**Phase:** CI/CD phase

---

## Minor Pitfalls

Mistakes that cause cosmetic issues or minor confusion.

---

### Pitfall 16: Docker termios Warning on Container Startup

**What goes wrong:** When the Alpine-based container starts, the REPL's `tcgetattr()` call may log a warning or debug message if `/dev/tty` isn't available in certain container runtimes (Podman, rootless Docker, CI environments).

**Prevention:**
1. The existing `RawModeGuard` already handles `tcgetattr` failure gracefully (sets `active = false`)
2. Ensure no error messages are printed on TTY detection failure — silent fallback to line mode is correct
3. Test with `docker run --rm qseries` (no `-it`) to verify clean error handling

**Severity:** MINOR — only affects unusual container runtimes
**Phase:** Docker image phase (verification)

---

### Pitfall 17: Install Script Doesn't Clean Up on Failure

**What goes wrong:** If the install script fails partway through (e.g., download succeeds but `chmod` fails), it leaves a non-executable binary in `~/.local/bin` and a modified `~/.zshrc` with a PATH entry pointing to a broken install.

**Prevention:**
1. Use a temporary directory for downloads, only move to final location on success:
   ```bash
   TMPDIR=$(mktemp -d)
   trap "rm -rf $TMPDIR" EXIT
   download_to "$TMPDIR/qseries"
   chmod +x "$TMPDIR/qseries"
   mv "$TMPDIR/qseries" "$INSTALL_DIR/qseries"
   ```
2. Only modify PATH config files AFTER binary is confirmed working (`$INSTALL_DIR/qseries --version`)
3. `trap` cleanup ensures temp files are removed even on Ctrl+C

**Severity:** MINOR — leaves system in inconsistent state, but easy to manually fix
**Phase:** Install script phase

---

### Pitfall 18: Dockerfile COPY Context — Accidentally Including .git or Build Artifacts

**What goes wrong:** `COPY . .` in the Dockerfile includes `.git/` (potentially hundreds of MB), `build/`, `dist/`, and other non-essential directories, dramatically slowing builds and inflating image size.

**Prevention:**
1. Create a `.dockerignore` file:
   ```
   .git
   .planning
   build
   dist
   *.exe
   qseries_debug
   qseries_bench
   demo
   tests
   website
   .cursor
   ```
2. Alternatively, COPY only what's needed: `COPY src/ src/` and `COPY Makefile .`

**Severity:** MINOR — slow builds but no functional impact
**Phase:** Docker image phase

---

### Pitfall 19: Windows Users Can't Run `install.sh`

**What goes wrong:** Windows users (without WSL or Cygwin) can't execute the bash install script. They need a separate installation method.

**Prevention:**
1. Provide a `.zip` download with the Windows binary and simple instructions
2. Consider a PowerShell install script for Windows:
   ```powershell
   Invoke-WebRequest -Uri $url -OutFile qseries.exe
   ```
3. Or simply provide direct download links on the website/GitHub releases page — Windows users are accustomed to downloading `.exe` files directly
4. The Cygwin development environment is separate from the end-user experience

**Severity:** MINOR — Windows users have alternative install paths (direct download)
**Phase:** Install script phase (Windows variant)

---

### Pitfall 20: Version String Not Embedded in Binary

**What goes wrong:** The install script downloads and installs a binary, but there's no way to verify which version was installed. The Docker image tag and the binary version can drift.

**Prevention:**
1. Ensure `qseries --version` (or the `version` REPL command) reports a version string
2. In CI release builds, inject the git tag into the binary at compile time:
   ```makefile
   VERSION ?= $(shell git describe --tags --always)
   CXXFLAGS += -DVERSION=\"$(VERSION)\"
   ```
3. Docker image tags should match binary versions: `qseries:1.0.0`
4. The install script should report what version it installed

**Severity:** MINOR — makes debugging version mismatches harder
**Phase:** CI/CD phase + install script phase

---

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation | Ref |
|---|---|---|---|
| Dockerfile creation | Missing `-it` docs; users can't run REPL | Wrapper script + prominent docs | #2, #9 |
| Dockerfile creation | Image bloat from build tools | Multi-stage build, `FROM scratch` or Alpine runtime | #14 |
| Dockerfile creation | Signal handling broken | Exec-form ENTRYPOINT, consider tini | #9 |
| Dockerfile creation | Build fails on static libstdc++ | Install `musl-dev`, verify with `ldd` | #10 |
| Dockerfile creation | CRLF corruption in piped output | Document `-i` vs `-it` usage | #5 |
| Dockerfile creation | Locale/UTF-8 problems | `ENV LANG=C.UTF-8` | #6 |
| Dockerfile creation | Slow context, huge layers | `.dockerignore` file | #18 |
| Install script | macOS rejects static binary | OS detection, omit `-static` on Darwin | #1 |
| Install script | Wrong shell config modified | Detect `$SHELL`, handle bash/zsh/fish | #7 |
| Install script | Security concerns | HTTPS, checksums, idempotent, `set -euo pipefail` | #4 |
| Install script | Missing curl/wget | Feature detection with fallback | #11 |
| Install script | Wrong architecture downloaded | `uname -m` detection, Rosetta check | #13 |
| Install script | Partial failure leaves mess | Temp dir + trap cleanup | #17 |
| Install script | Windows users excluded | Direct download or PowerShell script | #19 |
| CI release builds | Binary named `.exe` on Linux | Conditional naming in Makefile | #12 |
| CI release builds | Missing target in matrix | Explicit full matrix with test step | #15 |
| CI release builds | arm64 builds via QEMU are slow | Use native `ubuntu-24.04-arm` runner | #8 |
| CI release builds | No version in binary | Compile-time `-DVERSION` injection | #20 |

---

## Integration Pitfalls

### Docker + termios Interaction
The qseries REPL has three terminal modes: raw mode (interactive with tab/arrows), line mode (script/pipe), and WASM mode (no terminal). Docker must correctly trigger raw mode (via `-it`) or line mode (via `-i`). The existing `stdin_is_tty()` check handles this, but the Docker documentation must guide users to the right flags.

### Install Script + macOS + Build-from-Source
If the install script offers a "build from source" fallback (when no pre-built binary matches), it must NOT pass `-static` on macOS. This is the most common build failure for macOS users compiling from source.

### Docker Image + CI Release Matrix
The Docker image should use the same build process as the CI-built Linux binary (Alpine/musl, static linking). Don't maintain two different build recipes — divergence leads to one being tested and the other broken.

### Install Script + History File Permissions
The qseries REPL writes `~/.qseries_history`. If the install script runs as root (via `sudo`) or creates files as root, the history file could end up owned by root, preventing the normal user from writing to it. The install script should NEVER need sudo.

---

## Sources

- Apple Developer QA1118: Statically linked binaries on Mac OS X (https://developer.apple.com/library/archive/qa/qa1118/_index.html) — HIGH confidence
- Apple Developer Forums Thread 706419: Static linking not supported — HIGH confidence
- Docker Documentation: glibc and musl (https://docs.docker.com/dhi/core-concepts/glibc-musl/) — HIGH confidence
- Alpine Linux wiki: Static linking with musl (https://build-your-own.org/blog/20221229_alpine/) — HIGH confidence
- Docker run interactive/TTY options (https://www.baeldung.com/linux/docker-run-interactive-tty-options) — HIGH confidence
- Docker PTY raw mode CRLF issue (https://stackoverflow.com/questions/54709650/) — HIGH confidence
- GitHub Changelog: arm64 standard runners in private repos (Jan 2026) — HIGH confidence
- GitHub Changelog: macOS 26 GA (Feb 2026) — HIGH confidence
- Julia Evans: How to add a directory to PATH (https://jvns.ca/blog/2025/02/13/how-to-add-to-your-path/) — MEDIUM confidence
- curl|bash security analysis (multiple sources) — HIGH confidence
