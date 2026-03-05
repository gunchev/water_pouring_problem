# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Solves the classic water pouring puzzle with three jugs, a tap, and a sink. Implemented in both C++ (`src/`) and Python 3 (`python/`). Usage: `water LIMIT_1 LIMIT_2 LIMIT_3 TARGET` — finds the minimum number of steps to measure `TARGET` liters using three jugs of the given capacities.

## Build (C++)

Out-of-source CMake build. Requires `fmt` library.

```bash
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug   # configure (once)
cmake --build build/debug                        # build
./build/debug/water 3 5 8 4                      # run
```

Or simply `make debug`. Use `make release` for an optimized build (`./build/release/water`).

Build types: `Debug`, `DebugCov`, `RelWithDebInfo`, `Release`, `MinSizeRel`.

## Run (Python)

```bash
python3 python/water.py 3 5 8 4
```

## Code Style

- **C++**: clang-format enforced (LLVM-based, 120 col limit, 4-space indent, `IndentWidth: 4`). Run: `clang-format -i src/*.cpp src/*.h`
- **C++**: clang-tidy runs automatically during CMake build via `CMAKE_CXX_CLANG_TIDY`. Config in `.clang-tidy`.
- On non-Fedora systems, `-Werror` is active; on Fedora only `-Wall -Wextra`.

## Architecture

Both implementations use the same BFS (breadth-first search) approach:

**C++ (`src/water.cpp` + `src/utils.h`):**
- `water = uint16_t` — type alias for water volume measurements
- `VesselsState` — extends `std::array<water, 3>`, represents water levels in three jugs. Inherits comparison operators. Provides `transfer()`, `next_states()`, `contains()` methods. Acts as its own hash functor for `std::unordered_set`.
- `WaterPouringPuzzleSolver` — BFS solver. Stores `m_history` (vector of state + parent index pairs) and `m_visited` (unordered_set). Initial and fully-filled states are pre-excluded. Solution path is reconstructed by walking parent indices backward.
- `utils.h` — variadic template `gcd<T>()` with `static_assert` tests.
- `static_assert` used extensively for compile-time unit tests.
- `#if __cplusplus` version guards are intentional: the C++ code is written to compile on older standards too (C++14/17), not just the C++20 currently set in CMakeLists.txt. Do not remove these guards.

**Python (`python/water.py`):**
- `VesselsState` — immutable-by-convention, `__slots__`, hashable, total ordering.
- `Actor` — generates next states (fill/drain/transfer).
- `Puzzle` — BFS solver equivalent to C++ `WaterPouringPuzzleSolver`.
- `PuzzleStep` — stores state + previous index, `__slots__` for memory efficiency.
