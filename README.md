# Q-Series REPL — Build

Standalone C++ q-series REPL. Single `qseries.exe` with zero runtime dependencies.

## Prerequisites

- **Cygwin** (or similar Unix environment on Windows)
- **MinGW-w64 cross-compiler**: `mingw64-x86_64-gcc-g++`  
  Install via Cygwin Setup → Search `mingw64` → Install `mingw64-x86_64-gcc-g++`  
  Verify: `x86_64-w64-mingw32-g++ --version`

## Build

```bash
cd qseries
make
```

Output: `qseries.exe`

## Verification (static linkage)

Confirm no Cygwin or C++ runtime DLLs:

```bash
objdump -p qseries.exe | grep "DLL Name"
```

**Acceptable:** `KERNEL32.DLL`, `NTDLL.DLL`, and other Windows system DLLs.  
**Unacceptable:** `cygwin1.dll`, `libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`.

If `libwinpthread-1.dll` appears, add `-Wl,-Bstatic -lwinpthread` to `LDFLAGS` in the Makefile and rebuild.

## Reproducible build (optional)

```bash
export SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct 2>/dev/null || echo 0)
make
```

## Clean

```bash
make clean
```
