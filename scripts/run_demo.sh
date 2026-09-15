#!/usr/bin/env bash
set -euo pipefail
BUILD_DIR="${1:-build}"
"${BUILD_DIR}/telemetry_engine" --tcp-port 9000 --udp-port 9001 --workers 4 &
PID=$!
trap 'kill -INT ${PID} 2>/dev/null || true; wait ${PID} 2>/dev/null || true' EXIT
sleep 1
"${BUILD_DIR}/device_simulator" --protocol tcp --port 9000 --devices 4 --messages 12 --interval-ms 25
"${BUILD_DIR}/device_simulator" --protocol udp --port 9001 --devices 4 --messages 12 --interval-ms 25
sleep 1
