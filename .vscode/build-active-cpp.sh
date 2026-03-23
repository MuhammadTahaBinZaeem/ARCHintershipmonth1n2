#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <source.cpp>" >&2
    exit 1
fi

src="$1"

if [ ! -f "$src" ]; then
    echo "Source file not found: $src" >&2
    exit 1
fi

compiler="${CXX:-/usr/bin/g++}"
output="${src%.*}"

args=(
    "-std=c++20"
    "-fdiagnostics-color=always"
    "-g"
    "-Wall"
    "-Wextra"
    "-pedantic"
    "$src"
    "-o"
    "$output"
)

# Link common Linux libraries only when the active file uses them.
if rg -q '^#include <X11/' "$src"; then
    args+=("-lX11")
fi

if rg -q '^#include <(thread|mutex|condition_variable|future)>' "$src" || rg -q 'std::thread' "$src"; then
    args+=("-pthread")
fi

if rg -q '^#include <omp\\.h>' "$src" || rg -q '#pragma omp' "$src"; then
    args+=("-fopenmp")
fi

exec "$compiler" "${args[@]}"
