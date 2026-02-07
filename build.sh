#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
BUILD_TYPE="${BUILD_TYPE:-Release}"
GENERATOR="${GENERATOR:-}"

print_help() {
  cat <<'USAGE'
Usage: ./build.sh [clean] [debug|release] [run]

Options:
  clean    Remove existing build directory before configuring.
  debug    Build with CMAKE_BUILD_TYPE=Debug.
  release  Build with CMAKE_BUILD_TYPE=Release (default).
  run      Run executable after successful build.

Environment variables:
  GENERATOR   Optional CMake generator, e.g. "Ninja".
  BUILD_TYPE  Build type override (Debug/Release).
USAGE
}

CLEAN=0
RUN_AFTER_BUILD=0

for arg in "$@"; do
  case "$arg" in
    -h|--help)
      print_help
      exit 0
      ;;
    clean)
      CLEAN=1
      ;;
    debug)
      BUILD_TYPE="Debug"
      ;;
    release)
      BUILD_TYPE="Release"
      ;;
    run)
      RUN_AFTER_BUILD=1
      ;;
    *)
      echo "Unknown argument: $arg"
      print_help
      exit 1
      ;;
  esac
done

if [[ "$CLEAN" -eq 1 && -d "$BUILD_DIR" ]]; then
  echo "[build.sh] Cleaning build directory: $BUILD_DIR"
  rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

CMAKE_ARGS=(
  -S "$PROJECT_ROOT"
  -B "$BUILD_DIR"
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
)

if [[ -n "$GENERATOR" ]]; then
  CMAKE_ARGS+=(-G "$GENERATOR")
fi

echo "[build.sh] Configuring with CMake (type=$BUILD_TYPE)"
cmake "${CMAKE_ARGS[@]}"

echo "[build.sh] Building"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

if [[ "$RUN_AFTER_BUILD" -eq 1 ]]; then
  APP_PATH="$BUILD_DIR/ClassAssistant"
  if [[ ! -x "$APP_PATH" ]]; then
    APP_PATH="$BUILD_DIR/$BUILD_TYPE/ClassAssistant"
  fi

  if [[ -x "$APP_PATH" ]]; then
    echo "[build.sh] Running: $APP_PATH"
    "$APP_PATH"
  else
    echo "[build.sh] Build succeeded, but executable not found automatically."
    echo "[build.sh] Please run it manually from $BUILD_DIR."
  fi
fi
