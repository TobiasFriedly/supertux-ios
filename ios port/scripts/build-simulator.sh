#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PORT_DIR}/build/iphonesimulator"
CONFIGURATION="${1:-Debug}"

source "${SCRIPT_DIR}/toolchain-env.sh"

"${SCRIPT_DIR}/configure-xcode.sh" simulator "${CONFIGURATION}"
"${CMAKE_BIN}" --build "${BUILD_DIR}" --config "${CONFIGURATION}" --target supertux2

echo "Built app: ${BUILD_DIR}/${CONFIGURATION}-iphonesimulator/supertux2.app"
