#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
SOURCE_DIR="${PORT_DIR}/source/supertux"
VCPKG_ROOT="${PORT_DIR}/deps/vcpkg"

PLATFORM="${1:-simulator}"
CONFIGURATION="${2:-Debug}"

source "${SCRIPT_DIR}/toolchain-env.sh"

case "${PLATFORM}" in
  device)
    SDK="iphoneos"
    ARCHS="arm64"
    TRIPLET="arm64-ios-supertux"
    BUILD_DIR="${PORT_DIR}/build/iphoneos"
    ;;
  simulator)
    SDK="iphonesimulator"
    if [[ "$(uname -m)" == "arm64" ]]; then
      ARCHS="arm64"
      TRIPLET="arm64-ios-simulator-supertux"
    else
      ARCHS="x86_64"
      TRIPLET="x64-ios-simulator-supertux"
    fi
    BUILD_DIR="${PORT_DIR}/build/iphonesimulator"
    ;;
  *)
    echo "Usage: $0 [simulator|device] [Debug|Release]" >&2
    exit 2
    ;;
esac

DEVELOPMENT_TEAM="${DEVELOPMENT_TEAM:-${SUPERTUX_IOS_DEVELOPMENT_TEAM:-}}"
BUNDLE_IDENTIFIER="${BUNDLE_IDENTIFIER:-${SUPERTUX_IOS_BUNDLE_IDENTIFIER:-org.supertux.supertux2}}"
if [[ "${PLATFORM}" == "device" && -z "${DEVELOPMENT_TEAM}" && -f "${BUILD_DIR}/SUPERTUX.xcodeproj/project.pbxproj" ]]; then
  DEVELOPMENT_TEAM="$(sed -n 's/^[[:space:]]*DEVELOPMENT_TEAM = \([^;]*\);/\1/p' \
    "${BUILD_DIR}/SUPERTUX.xcodeproj/project.pbxproj" | head -n 1)"
fi
if [[ "${PLATFORM}" == "device" && -z "${DEVELOPMENT_TEAM}" ]] && command -v security >/dev/null 2>&1; then
  DEVELOPMENT_TEAM="$(security find-identity -v -p codesigning 2>/dev/null | \
    sed -n 's/.*"Apple Development:.*(\([[:alnum:]]*\))".*/\1/p' | head -n 1)"
fi

CMAKE_SIGNING_ARG=""
if [[ "${PLATFORM}" == "device" && -n "${DEVELOPMENT_TEAM}" ]]; then
  CMAKE_SIGNING_ARG="-DSUPERTUX_IOS_DEVELOPMENT_TEAM=${DEVELOPMENT_TEAM}"
fi

if [[ ! -x "${VCPKG_ROOT}/vcpkg" ]]; then
  "${SCRIPT_DIR}/bootstrap-vcpkg.sh" "${TRIPLET}"
fi

VCPKG_INSTALLED_DIR="${PORT_DIR}/deps/vcpkg_installed/${TRIPLET}"

if [[ ! -d "${VCPKG_INSTALLED_DIR}/${TRIPLET}" ]]; then
  "${SCRIPT_DIR}/bootstrap-vcpkg.sh" "${TRIPLET}"
fi

if [[ ! -f "${VCPKG_INSTALLED_DIR}/${TRIPLET}/lib/pkgconfig/libcurl.pc" ]]; then
  "${SCRIPT_DIR}/bootstrap-vcpkg.sh" "${TRIPLET}"
fi

"${SCRIPT_DIR}/bootstrap-vendor.sh"

"${CMAKE_BIN}" --fresh -S "${SOURCE_DIR}" -B "${BUILD_DIR}" -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT="${SDK}" \
  -DCMAKE_OSX_ARCHITECTURES="${ARCHS}" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
  -DCMAKE_MAP_IMPORTED_CONFIG_DEBUG=Release \
  -DCMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO=Release \
  -DCMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL=Release \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_MANIFEST_MODE=OFF \
  -DVCPKG_TARGET_TRIPLET="${TRIPLET}" \
  -DVCPKG_OVERLAY_TRIPLETS="${PORT_DIR}/triplets" \
  -DVCPKG_INSTALLED_DIR="${VCPKG_INSTALLED_DIR}" \
  -DSUPERTUX_IOS_PORT_DIR="${PORT_DIR}" \
  -DSUPERTUX_EXTERNAL_ROOT="${PORT_DIR}/vendor" \
  -DENABLE_OPENGL=OFF \
  -DENABLE_NETWORKING=ON \
  -DSUPERTUX_IOS_BUNDLE_IDENTIFIER="${BUNDLE_IDENTIFIER}" \
  -DUSE_SYSTEM_SDL2_TTF=ON \
  -DHIDE_NONMOBILE_OPTIONS=ON \
  -DBUILD_TESTING=OFF \
  -DSUPERTUX_PCH=OFF \
  -DSSQ_BUILD_INSTALL=OFF \
  -DSQ_DISABLE_INTERPRETER=ON \
  -DSQ_DISABLE_INSTALLER=ON \
  ${CMAKE_SIGNING_ARG:+"${CMAKE_SIGNING_ARG}"} \
  -DCMAKE_XCODE_ATTRIBUTE_SYMROOT="${BUILD_DIR}/xcode-products" \
  -DCMAKE_XCODE_ATTRIBUTE_OBJROOT="${BUILD_DIR}/xcode-intermediates" \
  -DCMAKE_XCODE_ATTRIBUTE_CLANG_MODULE_CACHE_PATH="${BUILD_DIR}/module-cache" \
  -DCMAKE_XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE=NO \
  -DCMAKE_CONFIGURATION_TYPES="${CONFIGURATION}"

echo "Xcode project: ${BUILD_DIR}/SUPERTUX.xcodeproj"
