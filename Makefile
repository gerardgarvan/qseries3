# Use g++ by default (Cygwin/Linux). Override for MinGW: make CXX=x86_64-w64-mingw32-g++
CXX ?= g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Wpedantic
LDFLAGS = -static

.PHONY: all clean test acceptance acceptance-qol acceptance-wins demo test-package

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
