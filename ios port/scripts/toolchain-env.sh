#!/usr/bin/env bash
set -euo pipefail

supertux_add_path_dir() {
  local path_dir="$1"
  if [[ -d "${path_dir}" ]]; then
    case ":${PATH:-}:" in
      *":${path_dir}:"*) ;;
      *) PATH="${path_dir}:${PATH:-}" ;;
    esac
  fi
}

supertux_find_tool() {
  local tool_name="$1"
  shift
  if command -v "${tool_name}" >/dev/null 2>&1; then
    command -v "${tool_name}"
    return 0
  fi
  local candidate
  for candidate in "$@"; do
    if [[ -x "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  done
  return 1
}

supertux_add_path_dir "/opt/homebrew/bin"
supertux_add_path_dir "/usr/local/bin"
supertux_add_path_dir "/Applications/CMake.app/Contents/bin"
export PATH

CMAKE_BIN="${CMAKE_BIN:-}"
if [[ -z "${CMAKE_BIN}" ]]; then
  if ! CMAKE_BIN="$(supertux_find_tool cmake \
    "/opt/homebrew/bin/cmake" \
    "/usr/local/bin/cmake" \
    "/Applications/CMake.app/Contents/bin/cmake")"; then
    echo "CMake was not found. Install it with Homebrew using 'brew install cmake' or install CMake.app from cmake.org, then reopen Xcode." >&2
    if [[ "${BASH_SOURCE[0]}" != "$0" ]]; then
      return 1
    fi
    exit 1
  fi
fi
export CMAKE_BIN
