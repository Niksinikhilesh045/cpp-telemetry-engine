# C++ Telemetry Engine

A multithreaded C++17 telemetry and event-processing service built to demonstrate production-oriented systems programming: asynchronous TCP/UDP networking with Boost.Asio, STL-based concurrency, RAII ownership, configurable rules, automated tests, sanitizers, and CI.

## Highlights

- Modern C++17, OOP/OOD, STL containers and algorithms
- `std::thread`, mutexes, condition variables, atomics, smart pointers, RAII
- Boost.Asio asynchronous TCP and UDP networking
- Producer-consumer worker-pool architecture
- Interface-based rule engine and alert sinks
- Asynchronous console/JSONL alert output
- GoogleTest unit coverage
- CMake and GitHub Actions CI
- ASan/UBSan/TSan build options

## Telemetry protocol

```text
device=DEVICE_001;temperature=92.50;pressure=98.40;rpm=4200;timestamp=1700000000000
```

## Build

Ubuntu/Debian prerequisites:

```bash
sudo apt update
sudo apt install -y build-essential cmake libboost-dev
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

For an offline build without tests:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build -j
```

## Run the engine

```bash
./build/telemetry_engine --tcp-port 9000 --udp-port 9001 --workers 4
```

Default rules:

- temperature `> 90.0` -> critical
- pressure `< 100.0` -> warning
- RPM `> 4000` -> warning

Write alerts to JSON Lines instead of stdout:

```bash
./build/telemetry_engine --alerts-file alerts.jsonl
```

## Simulate devices

TCP:

```bash
./build/device_simulator --protocol tcp --host 127.0.0.1 --port 9000 --devices 5 --messages 25
```

UDP:

```bash
./build/device_simulator --protocol udp --host 127.0.0.1 --port 9001 --devices 5 --messages 25
```

Or run both:

```bash
./scripts/run_demo.sh build
```

## Architecture

```text
TCP clients ----> Boost.Asio TCP server ---+
                                         |
UDP clients ----> Boost.Asio UDP server ---+--> ThreadPool
                                                |
                                                v
                                         TelemetryParser
                                                |
                                                v
                                           RuleEngine
                                                |
                                                v
                                         AsyncAlertSink
```

The networking event loop remains lightweight while CPU-side parsing and rule evaluation run in the worker pool. Alert I/O is also decoupled through an asynchronous sink.

See `docs/ARCHITECTURE.md` for the detailed concurrency, lifetime, and design-pattern discussion.

## Sanitizers

ASan + UBSan:

```bash
cmake -S . -B build-asan -DBUILD_TESTING=ON -DTELEMETRY_ENABLE_ASAN=ON -DTELEMETRY_ENABLE_UBSAN=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSan separately:

```bash
cmake -S . -B build-tsan -DBUILD_TESTING=ON -DTELEMETRY_ENABLE_TSAN=ON
cmake --build build-tsan -j
ctest --test-dir build-tsan --output-on-failure
```

## Scope

This is a Linux-oriented portfolio/reference implementation using simulated telemetry. It intentionally demonstrates embedded/industrial-adjacent systems concepts without claiming RTOS or real-hardware validation.
