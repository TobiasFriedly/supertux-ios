#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
CONFIGURATION="${1:-Debug}"
APP_PATH="${PORT_DIR}/build/iphonesimulator/${CONFIGURATION}-iphonesimulator/supertux2.app"
BUNDLE_ID="${BUNDLE_IDENTIFIER:-${SUPERTUX_IOS_BUNDLE_IDENTIFIER:-org.supertux.supertux2}}"
DEVICE_UDID="${SIM_DEVICE:-}"

if [[ ! -d "${APP_PATH}" ]]; then
  "${SCRIPT_DIR}/build-simulator.sh" "${CONFIGURATION}"
fi

if [[ -z "${DEVICE_UDID}" ]]; then
  DEVICE_UDID="$(
    xcrun simctl list devices available |
      awk '/iPhone/ {
        for (i = 1; i <= NF; i++) {
          if ($i ~ /^\([0-9A-Fa-f-]{36}\)$/) {
            gsub(/[()]/, "", $i)
            print $i
            exit
          }
        }
      }'
  )"
fi

if [[ -z "${DEVICE_UDID}" ]]; then
  echo "No available iPhone simulator found. Set SIM_DEVICE to a simulator UDID." >&2
  exit 1
fi

xcrun simctl boot "${DEVICE_UDID}" >/dev/null 2>&1 || true
xcrun simctl bootstatus "${DEVICE_UDID}" -b
xcrun simctl install "${DEVICE_UDID}" "${APP_PATH}"
xcrun simctl launch "${DEVICE_UDID}" "${BUNDLE_ID}"
