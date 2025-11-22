#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
GENERATOR="${CMAKE_GENERATOR:-Ninja}"

cmake_configure() {
    if [[ ! -d "${BUILD_DIR}" ]]; then
        mkdir -p "${BUILD_DIR}"
    fi

    if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
        cache_src=$(grep -E "^CMAKE_HOME_DIRECTORY:INTERNAL=" "${BUILD_DIR}/CMakeCache.txt" | cut -d= -f2- || true)
        if [[ -n "${cache_src}" && "${cache_src}" != "${ROOT_DIR}" ]]; then
            echo "[gui.sh] Detected stale CMake cache at ${cache_src}; cleaning build directory..."
            rm -rf "${BUILD_DIR}"
            mkdir -p "${BUILD_DIR}"
        fi
    fi

    if [[ ! -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
        echo "[gui.sh] Configuring project with ${GENERATOR}..."
        cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G "${GENERATOR}"
    fi
}

cmake_build() {
    echo "[gui.sh] Building project..."
    cmake --build "${BUILD_DIR}" --parallel
}

run_app() {
    if [[ "${SKIP_GUI_RUN:-0}" == "1" ]]; then
        echo "[gui.sh] SKIP_GUI_RUN=1 detected, skipping application launch."
        return
    fi

    echo "[gui.sh] Launching GUI..."
    pushd "${BUILD_DIR}" >/dev/null
    ./library_gui "$@"
    popd >/dev/null
}

cmake_configure
cmake_build
run_app "$@"
