#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PORT_DIR}/build/iphoneos"
CONFIGURATION="${1:-Release}"
DEVICE_ID="${DEVICE_ID:-${2:-}}"

if [[ -z "${DEVICE_ID}" ]]; then
  echo "Usage: DEVICE_ID=<iphone-identifier> $0 [Release|Debug]" >&2
  echo "The device identifier is shown by Xcode in the run error metadata." >&2
  exit 2
fi

DEVICE_ID="${DEVICE_ID}" "${SCRIPT_DIR}/build-device.sh" "${CONFIGURATION}"

APP_PATH="${BUILD_DIR}/${CONFIGURATION}-iphoneos/supertux2.app"
BUNDLE_IDENTIFIER="${BUNDLE_IDENTIFIER:-${SUPERTUX_IOS_BUNDLE_IDENTIFIER:-org.supertux.supertux2}}"
xcrun devicectl device install app --device "${DEVICE_ID}" "${APP_PATH}"
xcrun devicectl device process launch --device "${DEVICE_ID}" "${BUNDLE_IDENTIFIER}"
