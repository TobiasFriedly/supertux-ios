#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

CONFIGURATION="${CONFIGURATION:-Debug}"
PRODUCT_BUNDLE_IDENTIFIER="${PRODUCT_BUNDLE_IDENTIFIER:-org.supertux.supertux2}"
DEVELOPMENT_TEAM="${DEVELOPMENT_TEAM:-}"

case "${PLATFORM_NAME:-iphonesimulator}" in
  iphoneos)
    PLATFORM="device"
    SDK="iphoneos"
    BUILD_DIR="${PORT_DIR}/build/iphoneos"
    ;;
  iphonesimulator)
    PLATFORM="simulator"
    SDK="iphonesimulator"
    BUILD_DIR="${PORT_DIR}/build/iphonesimulator"
    ;;
  *)
    echo "Unsupported platform: ${PLATFORM_NAME:-unknown}" >&2
    exit 2
    ;;
esac

if [[ "${ACTION:-build}" == "clean" ]]; then
  rm -rf "${BUILD_DIR}"
  exit 0
fi

export SUPERTUX_IOS_BUNDLE_IDENTIFIER="${PRODUCT_BUNDLE_IDENTIFIER}"
if [[ -n "${DEVELOPMENT_TEAM}" ]]; then
  export SUPERTUX_IOS_DEVELOPMENT_TEAM="${DEVELOPMENT_TEAM}"
else
  unset SUPERTUX_IOS_DEVELOPMENT_TEAM
fi

"${SCRIPT_DIR}/configure-xcode.sh" "${PLATFORM}" "${CONFIGURATION}"

XCODEBUILD_FLAGS=("-hideShellScriptEnvironment")
XCODEBUILD_SETTINGS=("PRODUCT_BUNDLE_IDENTIFIER=${PRODUCT_BUNDLE_IDENTIFIER}")

if [[ "${PLATFORM}" == "simulator" ]]; then
  XCODEBUILD_SETTINGS+=(
    "CODE_SIGNING_ALLOWED=NO"
    "CODE_SIGNING_REQUIRED=NO"
    "DEVELOPMENT_TEAM="
  )
else
  if [[ -n "${DEVELOPMENT_TEAM}" ]]; then
    XCODEBUILD_SETTINGS+=("DEVELOPMENT_TEAM=${DEVELOPMENT_TEAM}")
  fi
  if [[ "${ALLOW_PROVISIONING_UPDATES:-YES}" != "NO" ]]; then
    XCODEBUILD_FLAGS+=("-allowProvisioningUpdates" "-allowProvisioningDeviceRegistration")
  fi
fi

xcodebuild \
  -project "${BUILD_DIR}/SUPERTUX.xcodeproj" \
  -scheme supertux2 \
  -configuration "${CONFIGURATION}" \
  -sdk "${SDK}" \
  "${XCODEBUILD_FLAGS[@]}" \
  "${XCODEBUILD_SETTINGS[@]}" \
  build

BUILT_APP="${BUILD_DIR}/${CONFIGURATION}-${SDK}/supertux2.app"

if [[ ! -d "${BUILT_APP}" ]]; then
  echo "Expected built app was not created: ${BUILT_APP}" >&2
  exit 1
fi

if [[ -n "${TARGET_BUILD_DIR:-}" && -n "${WRAPPER_NAME:-}" ]]; then
  WRAPPER_APP="${TARGET_BUILD_DIR}/${WRAPPER_NAME}"
  rm -rf "${WRAPPER_APP}"
  mkdir -p "${TARGET_BUILD_DIR}"
  /usr/bin/ditto "${BUILT_APP}" "${WRAPPER_APP}"
fi
