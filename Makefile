# Use g++ by default (Cygwin/Linux). Override: make CXX=x86_64-w64-mingw32-g++
# LDFLAGS: -static for static linking (add make LDFLAGS=-static if needed)
CXX ?= g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?= 

.PHONY: all clean test acceptance acceptance-qol acceptance-wins acceptance-v18 acceptance-suppress-output acceptance-arrow-keys acceptance-optional-args acceptance-history demo test-package wasm

all: dist/qseries.exe dist-demo

dist:
	mkdir -p dist

dist/qseries.exe: src/main.cpp | dist
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o dist/qseries.exe src/main.cpp

# Copy demo files into dist so dist/ is self-contained
dist-demo: | dist
	mkdir -p dist/demo
	cp demo/garvan-demo.sh dist/demo/
	cp demo/README.md dist/demo/

# Debug build (Cygwin g++, with sanitizers)
debug: src/main.cpp
	g++ -std=c++20 -g -O0 -Wall -Wextra -fsanitize=address,undefined -o qseries_debug src/main.cpp

# Quick test: run the Rogers-Ramanujan acceptance test
test: dist/qseries.exe
	echo 'x := sum(q^(n^2) / aqprod(q, q, n, 50), n, 0, 8)' | ./dist/qseries.exe
	echo 'prodmake(x, 40)' | ./dist/qseries.exe

# Run all 9 SPEC acceptance tests via REPL
acceptance: dist/qseries.exe
	./tests/acceptance.sh

# Run QoL acceptance tests (multi-line, error messages, demo packaging)
acceptance-qol: dist/qseries.exe
	./tests/qol-acceptance.sh

# Run Phase 23 quick-wins acceptance tests
acceptance-wins: dist/qseries.exe
	./tests/acceptance-wins.sh

# Run Phase 24 mprodmake acceptance tests
acceptance-mprodmake: dist/qseries.exe
	./tests/acceptance-mprodmake.sh

# Run Phase 25 checkprod/checkmult acceptance tests
acceptance-checkprod-checkmult: dist/qseries.exe
	./tests/acceptance-checkprod-checkmult.sh

# Run Phase 27 suppress-output acceptance tests
acceptance-suppress-output: dist/qseries.exe
	./tests/acceptance-suppress-output.sh

# Run Phase 28 arrow-key navigation acceptance tests (TTY simulation; skips if script unavailable)
acceptance-arrow-keys: dist/qseries.exe
	./tests/acceptance-arrow-keys.sh

# Run Phase 31 up/down arrow history acceptance tests (TTY simulation; skips if script unavailable)
acceptance-history: dist/qseries.exe
	./tests/acceptance-history.sh

# Run v1.8 function acceptance tests
acceptance-v18: dist/qseries.exe
	./tests/acceptance-v18.sh

# Run Phase 29 optional-arg variants acceptance tests
acceptance-optional-args: dist/qseries.exe
	./tests/acceptance-optional-args.sh

# Run Garvan demo (human demonstration, no assertions)
demo: dist/qseries.exe dist-demo
	cd dist && ./demo/garvan-demo.sh

# Create test-package: binary + all tests for third-party verification
test-package: dist/qseries.exe
	mkdir -p test-package
	cp dist/qseries.exe test-package/ 2>/dev/null || cp dist/qseries test-package/

# Create qseries-demo/: self-contained folder to zip and share (binary + demo + README)
package-demo: dist/qseries.exe
	mkdir -p qseries-demo
	cp dist/qseries.exe qseries-demo/ 2>/dev/null || cp dist/qseries qseries-demo/
	cp demo/garvan-demo.sh qseries-demo/
	cp demo/README-dist.md qseries-demo/README.md
	@echo "qseries-demo/ ready. Zip and share. Run: cd qseries-demo && bash garvan-demo.sh"

clean:
	rm -f dist/qseries.exe qseries_debug
	rm -rf build/

# Wasm build (requires emsdk activated: source emsdk_env.sh)
EMXX ?= em++
WASM_FLAGS = -std=c++20 -Oz -lembind \
    -s MODULARIZE=1 \
    -s EXPORT_NAME=createQSeries \
    -s FILESYSTEM=0 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ENVIRONMENT=web,worker,node \
    -fwasm-exceptions

wasm: build/wasm/qseries.js

build/wasm:
	mkdir -p build/wasm

build/wasm/qseries.js: src/main_wasm.cpp src/repl.h src/parser.h src/series.h src/frac.h src/bigint.h src/qfuncs.h src/convert.h src/linalg.h src/relations.h | build/wasm
	$(EMXX) $(WASM_FLAGS) -o build/wasm/qseries.js src/main_wasm.cpp
