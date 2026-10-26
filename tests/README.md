# PC tests

The tests compile the real `src/AsyncTimer.cpp` against a small Arduino stub.
They need a C++11 compiler and CMake 3.16 or newer. No Arduino SDK, board,
PlatformIO, downloaded test framework, or real-time waiting is required.

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

These commands also work with a Visual Studio developer terminal on Windows.
Checks remain enabled in Release builds. CTest runs 11 executables; each reports
its individual cases on failure (or when CTest is run with `--verbose`).

To run one suite or one case:

```sh
ctest --test-dir build -C Release -R host.callbacks --output-on-failure
./build/tests/test_callbacks timeout_can_schedule_replacement
```

With Visual Studio, the executable is under `build/tests/Release/` and ends in
`.exe`. The optional argument matches part of a case name; no matches is an error.

## Coverage

- Timing: exact deadlines, overdue timers, zero delays, repeated intervals,
  simultaneous timers, remaining time, delay changes, postponement, reset,
  invalid/expired IDs, and the deprecated `setup()` entry point.
- Capacity and IDs: default/custom/zero capacity, full-array rejection, slot
  reuse, and more than 65,535 registrations while another timer remains active.
- Cancellation: individual, repeated, empty, partial, and bulk cancellation,
  including preserving intervals with `cancelAll(false)`.
- Callbacks: function pointers, lambdas, mutable captures, callable objects,
  capture destruction, registration failure, scheduling/cancelling inside a
  callback, nested `handle()`, and exception cleanup on hosts with exceptions.
- Rollover: timeout/interval boundaries, delayed/reset timers, large delays,
  deadlines exactly at zero, and multiple clock wraps.
- Model tests: 75,000 seeded operations compared with a separate model using
  absolute deadlines. A failure reports the seed and operation number.
- All five example sketches: the actual `.ino` files are included and exercised
  with simulated GPIO and serial output.

The clock advances only when a test asks it to. Rollover tests use `ULONG_MAX`,
matching the width of the library's `unsigned long` arithmetic: usually 64 bits
on Linux/macOS hosts and 32 bits on Windows, ESP32, and AVR. The CI workflow runs
Linux, macOS, Windows, sanitizers, and a Linux 32-bit build. On a Linux PC with
multilib development libraries installed, run that build locally with:

```sh
cmake -S . -B build/32bit -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-m32
cmake --build build/32bit --parallel
ctest --test-dir build/32bit --output-on-failure
```

The tests check cooperative scheduling, not interrupt safety or physical board
I/O. The total wait for a firing must fit in `unsigned long`, and timers must be serviced
before a full clock cycle elapses. Ordering between timers due simultaneously
is deliberately not asserted.

## Memory and undefined-behavior checks

With GCC or Clang:

```sh
cmake -S . -B build/sanitizers -DCMAKE_BUILD_TYPE=Debug -DASYNCTIMER_SANITIZERS=ON
cmake --build build/sanitizers --parallel
UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build/sanitizers --output-on-failure
```

AddressSanitizer and UndefinedBehaviorSanitizer instrument both the library and
tests. If a sandbox or debugger blocks LeakSanitizer's process inspection, run
with `ASAN_OPTIONS=detect_leaks=0` in that environment; bounds and use-after-free
checks still run. Normal CI leaves leak detection enabled.
