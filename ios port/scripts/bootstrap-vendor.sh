#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
VENDOR_DIR="${PORT_DIR}/vendor"

mkdir -p "${VENDOR_DIR}"

clone_if_missing() {
  local name="$1"
  local url="$2"
  local dest="${VENDOR_DIR}/${name}"

  if [[ -f "${dest}/CMakeLists.txt" ]]; then
    echo "${name} already present"
    if [[ -d "${dest}/.git" ]]; then
      git -C "${dest}" submodule update --init --recursive
    fi
    return
  fi

  rm -rf "${dest}"
  git clone --depth 1 --recurse-submodules --shallow-submodules "${url}" "${dest}"
}

clone_if_missing "tinygettext" "https://github.com/SuperTux/tinygettext.git"
clone_if_missing "sexp-cpp" "https://github.com/SuperTux/sexp-cpp.git"
clone_if_missing "simplesquirrel" "https://github.com/SuperTux/simplesquirrel.git"
