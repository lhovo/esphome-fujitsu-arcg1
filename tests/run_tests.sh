#!/usr/bin/env bash
set -euo pipefail
test_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
test_binary="$(mktemp)"
trap 'rm -f "$test_binary"' EXIT
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined \
  "$test_dir/test_codec.cpp" -o "$test_binary"
# LeakSanitizer cannot inspect /proc in some containers. Override if desired.
ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}" "$test_binary"
