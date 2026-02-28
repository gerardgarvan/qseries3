# Q-Series Demo

A self-contained demo of the Q-Series REPL: exact q-series computation, Rogers–Ramanujan identities, product conversion, and more (based on Frank Garvan's qseriesdoc tutorial).

## Contents

- **qseries.exe** (or **qseries**) — the Q-Series program
- **garvan-demo.sh** — demo script that runs example computations
- **README.md** — this file

## Requirements

- **Bash** — Git Bash, Cygwin, or WSL on Windows; built-in on Linux/macOS
- **Windows:** Command Prompt cannot run `.sh` scripts; use one of the above

## How to Run

1. Download or unzip this folder.
2. Open a terminal with bash.
3. Go into the folder:
   ```
   cd qseries-demo
   ```
4. Run the demo:
   ```
   bash garvan-demo.sh
   ```

The demo runs a sequence of q-series examples: Rogers–Ramanujan, product conversion (qfactor, etamake, jacprodmake), relation finding (findhom, findhomcombo, findnonhomcombo), and sifting/product identities (triple product, quintuple product, Winquist).

## Interactive REPL

To use the REPL yourself:
```
./qseries.exe
```
(Or `./qseries` on Linux/macOS.)

## Rebuild (optional)

If you have the source and need to rebuild:
```
cd qseries3
bash build.sh
cp dist/qseries.exe qseries-demo/
```
