# External Integrations

**Analysis Date:** 2025-03-04

## APIs & External Services

**None** — The qseries REPL is fully offline. No external APIs, cloud services, or network calls.

## Data Storage

**Databases:**
- None

**File Storage:**
- Local filesystem — Script mode reads `.qs` files via REPL `source` command; demo scripts in `demo/`
- No persistent state between sessions

**Caching:**
- None — Eta memoization mentioned in SPEC stretch goals but not detected in current implementation

## Authentication & Identity

**Auth Provider:**
- None — No authentication or user identity

## Monitoring & Observability

**Error Tracking:**
- None — Errors printed to stderr; no remote logging

**Logs:**
- stdout/stderr only — No structured logging or log files

## CI/CD & Deployment

**Hosting:**
- GitHub Releases — Binaries attached to version tags
- GitHub Container Registry — `ghcr.io` for Docker images (per `release.yml`)
- `install.sh` fetches releases from `https://api.github.com/repos/gerardgarvan/qseries3/releases/latest`

**CI Pipeline:**
- `.github/workflows/release.yml` — On push to tags `v*`:
  - Builds Linux (ubuntu-24.04) and macOS (macos-14)
  - Runs `tests/acceptance.sh` on Linux
  - Uploads artifacts, creates GitHub Release with checksums
  - Builds and pushes Docker image

## Environment Configuration

**Required env vars:**
- None for core functionality

**Secrets location:**
- GitHub secrets for release (repo permissions)

## Webhooks & Callbacks

**Incoming:**
- None

**Outgoing:**
- None (except `install.sh` curl/wget to GitHub API for release download)

---

*Integration audit: 2025-03-04*
