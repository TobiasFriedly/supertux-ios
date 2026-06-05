#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
VCPKG_ROOT="${PORT_DIR}/deps/vcpkg"
TRIPLET="${1:-arm64-ios-simulator-supertux}"
INSTALL_ROOT="${PORT_DIR}/deps/vcpkg_installed/${TRIPLET}"

source "${SCRIPT_DIR}/toolchain-env.sh"

if [[ ! -x "${VCPKG_ROOT}/vcpkg" ]]; then
  rm -rf "${VCPKG_ROOT}"
  git clone --depth 1 https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}"
  "${VCPKG_ROOT}/bootstrap-vcpkg.sh" -disableMetrics
fi

(
  cd "${PORT_DIR}"
  "${VCPKG_ROOT}/vcpkg" install \
    --triplet "${TRIPLET}" \
    --overlay-triplets "${PORT_DIR}/triplets" \
    --x-install-root "${INSTALL_ROOT}"
)
