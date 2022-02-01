#!/usr/bin/env bash
set -eou pipefail
rm -rf build
cmake -S. -Bbuild
cmake --build build
./build/main
