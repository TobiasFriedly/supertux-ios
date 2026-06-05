#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PORT_DIR}/build/iphoneos"
CONFIGURATION="${1:-Release}"

"${SCRIPT_DIR}/configure-xcode.sh" device "${CONFIGURATION}"

XCODEBUILD_FLAGS=()
if [[ "${ALLOW_PROVISIONING_UPDATES:-YES}" != "NO" ]]; then
  XCODEBUILD_FLAGS+=("-allowProvisioningUpdates" "-allowProvisioningDeviceRegistration")
fi
if [[ -n "${DEVICE_ID:-}" ]]; then
  XCODEBUILD_FLAGS+=("-destination" "id=${DEVICE_ID}")
fi

xcodebuild \
  -project "${BUILD_DIR}/SUPERTUX.xcodeproj" \
  -scheme supertux2 \
  -configuration "${CONFIGURATION}" \
  -sdk iphoneos \
  "${XCODEBUILD_FLAGS[@]}" \
  build

echo "Built app: ${BUILD_DIR}/${CONFIGURATION}-iphoneos/supertux2.app"
