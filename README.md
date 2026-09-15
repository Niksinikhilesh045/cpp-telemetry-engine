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

### Linux (Ubuntu/Debian)

Install prerequisites:

```bash
sudo apt update
sudo apt install -y build-essential cmake libboost-dev
```

Configure, build, and run the tests:

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

### Windows (MSVC + vcpkg)

The project has been validated on Windows using Visual Studio 2022 Build Tools, MSVC, CMake, and Boost installed through vcpkg.

#### Prerequisites

Install Visual Studio 2022 Build Tools with the **Desktop development with C++** workload. The installation should provide:

- MSVC C++ compiler (`cl.exe`)
- Windows SDK
- CMake support for Visual Studio
- MSBuild

Git is also required.

Open **Developer Command Prompt for VS 2022** or **x64 Native Tools Command Prompt for VS 2022** before configuring the project. Verify the toolchain:

```cmd
cl
cmake --version
git --version
```

#### Install vcpkg and Boost.Asio

Clone and bootstrap vcpkg:

```cmd
cd /d C:\
git clone https://github.com/microsoft/vcpkg.git
cd C:\vcpkg
bootstrap-vcpkg.bat
```

Install Boost.Asio for 64-bit Windows:

```cmd
C:\vcpkg\vcpkg.exe install boost-asio:x64-windows
```

The vcpkg package installs the Boost headers and required Boost dependencies used by the project.

#### Configure the Windows build

From the repository root:

```cmd
cd /d D:\cpp-telemetry-engine
cmake -S . -B build-win -A x64 -DBUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Replace `D:\cpp-telemetry-engine` with the location where you cloned the repository.

A successful configuration should finish with messages similar to:

```text
-- Found Boost: ...
-- Configuring done
-- Generating done
-- Build files have been written to: .../build-win
```

#### Build with MSVC

```cmd
cmake --build build-win --config Release
```

The main executables are generated under:

```text
build-win\Release\telemetry_engine.exe
build-win\Release\device_simulator.exe
build-win\Release\telemetry_tests.exe
```

#### Run the test suite

```cmd
ctest --test-dir build-win -C Release --output-on-failure
```

The current test suite covers telemetry parsing, blocking-queue behavior, thread-pool execution, rule evaluation, and event processing.

#### Reconfigure after changing generator/platform settings

CMake caches generator and platform information inside the build directory. If you change the generator, architecture, or toolchain file, delete the existing Windows build directory before configuring again:

```cmd
rmdir /s /q build-win
```

Then rerun the configure command.

> `BUILD_TESTING=ON` uses CMake `FetchContent` to obtain GoogleTest during configuration, so the first test-enabled configure requires network access unless the dependency is already cached.

## Run the engine

### Linux

```bash
./build/telemetry_engine --tcp-port 9000 --udp-port 9001 --workers 4
```

### Windows

```cmd
build-win\Release\telemetry_engine.exe --tcp-port 9000 --udp-port 9001 --workers 4
```

Default rules:

- temperature `> 90.0` -> critical
- pressure `< 100.0` -> warning
- RPM `> 4000` -> warning

Write alerts to JSON Lines instead of stdout.

Linux:

```bash
./build/telemetry_engine --alerts-file alerts.jsonl
```

Windows:

```cmd
build-win\Release\telemetry_engine.exe --alerts-file alerts.jsonl
```

## Simulate devices

### Linux

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

### Windows

Start the engine in one Developer Command Prompt, then use another prompt to run the simulator.

TCP:

```cmd
build-win\Release\device_simulator.exe --protocol tcp --host 127.0.0.1 --port 9000 --devices 5 --messages 25
```

UDP:

```cmd
build-win\Release\device_simulator.exe --protocol udp --host 127.0.0.1 --port 9001 --devices 5 --messages 25
```

Press `Ctrl+C` in the engine terminal for a graceful shutdown and final processing summary.

Example summary:

```text
Engine stopped. received=40 parsed=40 parse_errors=0 alerts=45
```

The alert count can exceed the received-event count because one telemetry event may trigger multiple rules.

### Windows validation

The Windows/MSVC build has been functionally validated with both TCP and UDP synthetic workloads, including 5,000-message zero-delay local runs. In the validated runs, all 5,000 received events were parsed with zero parse errors and the JSONL alert count matched the engine's internal alert count.

These checks are functional stress tests, not formal throughput benchmarks. UDP itself does not guarantee packet delivery even though no packet loss was observed in the local validation run.

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

The sanitizer options are applied on non-MSVC toolchains. For Windows/MSVC validation, use the standard Release build and GoogleTest commands above.

## Scope

This is a cross-platform portfolio/reference implementation using simulated telemetry. It has been validated with MSVC on Windows and is also designed for Linux-based development and CI. It intentionally demonstrates embedded/industrial-adjacent systems concepts without claiming RTOS or real-hardware validation.
