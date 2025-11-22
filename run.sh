#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
GENERATOR="${CMAKE_GENERATOR:-Ninja}"

configure() {
    if [[ ! -d "${BUILD_DIR}" ]]; then
        mkdir -p "${BUILD_DIR}"
    fi

    if [[ ! -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
        echo "[run.sh] Configuring project with ${GENERATOR}..."
        cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G "${GENERATOR}"
    fi
}

build_cli() {
    echo "[run.sh] Building library_cli target..."
    cmake --build "${BUILD_DIR}" --target library_cli --parallel
}

run_cli() {
    if [[ "${SKIP_CLI_RUN:-0}" == "1" ]]; then
        echo "[run.sh] SKIP_CLI_RUN=1 detected, skipping CLI execution."
        return
    fi

    echo "[run.sh] Launching CLI..."
    "${BUILD_DIR}/library_cli" "$@"
}

configure
build_cli
run_cli "$@"
