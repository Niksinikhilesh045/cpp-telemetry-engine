cmake_minimum_required(VERSION 3.20)
project(cpp_telemetry_engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TELEMETRY_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(TELEMETRY_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(TELEMETRY_ENABLE_TSAN "Enable ThreadSanitizer" OFF)

if(TELEMETRY_ENABLE_TSAN AND (TELEMETRY_ENABLE_ASAN OR TELEMETRY_ENABLE_UBSAN))
    message(FATAL_ERROR "ThreadSanitizer must be enabled separately from ASan/UBSan")
endif()

find_package(Threads REQUIRED)

# CMake 3.30+ removed the legacy FindBoost module under CMP0167. Prefer
# BoostConfig.cmake when available, while retaining a fallback for older Boost
# installations and CMake versions.
if(POLICY CMP0167)
    find_package(Boost 1.70 CONFIG QUIET)
    if(NOT Boost_FOUND)
        cmake_policy(SET CMP0167 OLD)
        find_package(Boost 1.70 REQUIRED)
    endif()
else()
    find_package(Boost 1.70 REQUIRED)
endif()

function(telemetry_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    endif()
endfunction()

function(telemetry_apply_sanitizers target)
    if(NOT MSVC)
        if(TELEMETRY_ENABLE_ASAN)
            target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif()
        if(TELEMETRY_ENABLE_UBSAN)
            target_compile_options(${target} PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=undefined)
        endif()
        if(TELEMETRY_ENABLE_TSAN)
            target_compile_options(${target} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=thread)
        endif()
    endif()
endfunction()

add_library(telemetry_core
    src/parser.cpp
    src/thread_pool.cpp
    src/rule.cpp
    src/alert_sink.cpp
    src/event_processor.cpp
    src/network/tcp_server.cpp
    src/network/udp_server.cpp
    src/engine.cpp
)

target_include_directories(telemetry_core PUBLIC include)
target_link_libraries(telemetry_core PUBLIC Threads::Threads Boost::headers)
target_compile_definitions(telemetry_core PUBLIC
    BOOST_ERROR_CODE_HEADER_ONLY
    BOOST_SYSTEM_NO_DEPRECATED
)
telemetry_apply_warnings(telemetry_core)
telemetry_apply_sanitizers(telemetry_core)

add_executable(telemetry_engine src/main.cpp)
target_link_libraries(telemetry_engine PRIVATE telemetry_core)
telemetry_apply_warnings(telemetry_engine)
telemetry_apply_sanitizers(telemetry_engine)

add_executable(device_simulator simulator/device_simulator.cpp)
target_link_libraries(device_simulator PRIVATE Threads::Threads Boost::headers)
target_compile_definitions(device_simulator PRIVATE
    BOOST_ERROR_CODE_HEADER_ONLY
    BOOST_SYSTEM_NO_DEPRECATED
)
telemetry_apply_warnings(device_simulator)
telemetry_apply_sanitizers(device_simulator)

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(telemetry_tests
        tests/test_parser.cpp
        tests/test_blocking_queue.cpp
        tests/test_thread_pool.cpp
        tests/test_rule_engine.cpp
        tests/test_event_processor.cpp
    )
    target_link_libraries(telemetry_tests PRIVATE telemetry_core GTest::gtest_main)
    telemetry_apply_warnings(telemetry_tests)
    telemetry_apply_sanitizers(telemetry_tests)

    include(GoogleTest)
    gtest_discover_tests(telemetry_tests)
endif()

if(WIN32)
    target_compile_definitions(telemetry_core PUBLIC
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_compile_definitions(device_simulator PRIVATE
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()
