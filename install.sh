#!/bin/sh
set -eu

REPO="gerardgarvan/qseries3"
INSTALL_DIR="${QSERIES_INSTALL_DIR:-$HOME/.local/bin}"

main() {
    VERSION=""
    while [ $# -gt 0 ]; do
        case "$1" in
            --version) VERSION="$2"; shift 2 ;;
            --version=*) VERSION="${1#--version=}"; shift ;;
            *) echo "Usage: install.sh [--version vX.Y]" >&2; exit 1 ;;
        esac
    done

    OS="$(uname -s)"
    ARCH="$(uname -m)"

    case "$OS" in
        Linux)  PLATFORM="linux" ;;
        Darwin) PLATFORM="darwin" ;;
        *)      echo "error: unsupported OS: $OS" >&2; exit 1 ;;
    esac

    case "$ARCH" in
        x86_64|amd64)  ARCH_NAME="amd64" ;;
        arm64|aarch64) ARCH_NAME="arm64" ;;
        *)             echo "error: unsupported architecture: $ARCH" >&2; exit 1 ;;
    esac

    BINARY="qseries-${PLATFORM}-${ARCH_NAME}"

    if [ -z "$VERSION" ]; then
        echo "Fetching latest release..."
        VERSION=$(fetch "https://api.github.com/repos/${REPO}/releases/latest" | grep '"tag_name"' | sed 's/.*"tag_name": *"\([^"]*\)".*/\1/')
        if [ -z "$VERSION" ]; then
            echo "error: could not determine latest version" >&2
            exit 1
        fi
    fi

    BASE_URL="https://github.com/${REPO}/releases/download/${VERSION}"

    echo "Installing qseries ${VERSION} (${PLATFORM}/${ARCH_NAME})..."

    TMPDIR=$(mktemp -d)
    trap 'rm -rf "$TMPDIR"' EXIT

    echo "Downloading ${BINARY}..."
    fetch "${BASE_URL}/${BINARY}" > "${TMPDIR}/${BINARY}"

    echo "Downloading checksums..."
    fetch "${BASE_URL}/checksums-sha256.txt" > "${TMPDIR}/checksums-sha256.txt"

    echo "Verifying checksum..."
    EXPECTED=$(grep "${BINARY}" "${TMPDIR}/checksums-sha256.txt" | awk '{print $1}')
    if [ -z "$EXPECTED" ]; then
        echo "error: no checksum found for ${BINARY}" >&2
        exit 1
    fi

    if command -v sha256sum >/dev/null 2>&1; then
        ACTUAL=$(sha256sum "${TMPDIR}/${BINARY}" | awk '{print $1}')
    elif command -v shasum >/dev/null 2>&1; then
        ACTUAL=$(shasum -a 256 "${TMPDIR}/${BINARY}" | awk '{print $1}')
    else
        echo "warning: no sha256sum or shasum found, skipping verification" >&2
        ACTUAL="$EXPECTED"
    fi

    if [ "$EXPECTED" != "$ACTUAL" ]; then
        echo "error: checksum mismatch" >&2
        echo "  expected: $EXPECTED" >&2
        echo "  actual:   $ACTUAL" >&2
        exit 1
    fi
    echo "Checksum verified."

    mkdir -p "$INSTALL_DIR"
    mv "${TMPDIR}/${BINARY}" "${INSTALL_DIR}/qseries"
    chmod +x "${INSTALL_DIR}/qseries"

    echo ""
    echo "qseries ${VERSION} installed to ${INSTALL_DIR}/qseries"

    case ":${PATH}:" in
        *":${INSTALL_DIR}:"*) ;;
        *)
            echo ""
            echo "Add to your PATH:"
            echo "  export PATH=\"${INSTALL_DIR}:\$PATH\""
            echo ""
            echo "Or add that line to your shell profile (~/.bashrc, ~/.zshrc, etc.)"
            ;;
    esac
}

fetch() {
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -qO- "$1"
    else
        echo "error: curl or wget required" >&2
        exit 1
    fi
}

main "$@"
