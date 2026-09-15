# Architecture

The telemetry engine is intentionally designed as a systems-programming project rather than a web application. It demonstrates C++17 resource management, concurrency, asynchronous networking, object-oriented design, and automated testing.

## Runtime flow

```text
TCP devices ----> Boost.Asio TCP server ---+
                                          |
UDP devices ----> Boost.Asio UDP server ---+--> ThreadPool
                                                 |
                                                 v
                                          TelemetryParser
                                                 |
                                                 v
                                            RuleEngine
                                                 |
                                                 v
                                          AsyncAlertSink
                                                 |
                                      +----------+----------+
                                      |                     |
                                   console              JSONL file
```

## Concurrency model

Boost.Asio owns socket readiness and asynchronous I/O on the main event loop. Network callbacks do minimal work and move payloads into a fixed-size worker pool. CPU-side parsing and rule evaluation occur on worker threads so parsing cannot block the network event loop.

`ThreadPool` and `AsyncAlertSink` both use the reusable `BlockingQueue<T>` abstraction. The queue is protected by a mutex and condition variable and supports explicit closure so sleeping consumers wake during shutdown. Worker lifetime is managed using RAII: destructors close queues and join threads.

The configured rule set is immutable while the engine is running, so worker threads can evaluate the same rules concurrently without per-event locking. Atomic counters maintain lightweight runtime statistics.

## Networking

The TCP server accepts multiple simultaneous clients. Each connection is represented by a `TcpSession` whose lifetime is managed by `std::shared_ptr` and `enable_shared_from_this`; newline-delimited payloads are read asynchronously with Boost.Asio.

The UDP server receives independent datagrams on a dedicated socket. TCP and UDP payloads enter the same processing pipeline while retaining their transport type in event and alert metadata.

## Ownership and memory management

The project avoids manual `new`/`delete`. Exclusive ownership is represented with `std::unique_ptr`; asynchronous TCP session lifetime uses `std::shared_ptr` because callbacks share ownership; stack/RAII objects own sockets, streams, mutexes, threads, queues, and file handles.

## Design patterns

- **Strategy:** `IRule` allows independent rule implementations.
- **Dependency inversion:** `EventProcessor` depends on `IAlertSink`, not a concrete output mechanism.
- **Producer-consumer:** network handlers, worker queues, and the alert writer decouple ingestion, processing, and output.
- **Asynchronous wrapper:** `AsyncAlertSink` wraps a concrete downstream sink and moves output I/O to a dedicated writer thread.

## Shutdown

SIGINT/SIGTERM are handled by Boost.Asio. The engine closes network listeners, stops the `io_context`, closes worker queues, joins workers, then drains and joins the asynchronous alert writer. This ordering prevents leaked threads and sockets and preserves accepted work as far as practical.

## Scope

This project uses simulated telemetry and standard Linux networking. It is intentionally relevant to industrial/embedded-adjacent software engineering, but it does not claim RTOS execution or validation against physical industrial hardware.
