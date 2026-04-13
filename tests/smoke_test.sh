#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
PORT="18080"
SERVER="${BUILD_DIR}/hp_http_server"
LOG_FILE="${BUILD_DIR}/smoke_server.log"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" >/dev/null
cmake --build "${BUILD_DIR}" >/dev/null

"${SERVER}" --port "${PORT}" --root "${ROOT_DIR}/public" >"${LOG_FILE}" 2>&1 &
SERVER_PID=$!
cleanup() {
  kill "${SERVER_PID}" >/dev/null 2>&1 || true
  wait "${SERVER_PID}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

for _ in {1..50}; do
  if curl -sSf "http://127.0.0.1:${PORT}/" >/dev/null 2>&1; then
    break
  fi
  sleep 0.1
done

check_status() {
  local expected="$1"
  local method="$2"
  local url="$3"
  local status
  status="$(curl -sS -o /dev/null -w '%{http_code}' -X "${method}" --path-as-is "${url}")"
  if [[ "${status}" != "${expected}" ]]; then
    echo "expected ${expected}, got ${status}: ${method} ${url}" >&2
    echo "--- server log ---" >&2
    cat "${LOG_FILE}" >&2 || true
    exit 1
  fi
}

check_status 200 GET "http://127.0.0.1:${PORT}/"
check_status 200 GET "http://127.0.0.1:${PORT}/index.html"
check_status 404 GET "http://127.0.0.1:${PORT}/not-exist"
check_status 405 POST "http://127.0.0.1:${PORT}/"
check_status 403 GET "http://127.0.0.1:${PORT}/../../etc/passwd"

for _ in {1..5}; do
  check_status 200 GET "http://127.0.0.1:${PORT}/"
done

echo "smoke test passed"
