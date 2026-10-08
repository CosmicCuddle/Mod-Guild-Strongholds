#!/usr/bin/env bash
# C++ domain tests; no live AzerothCore installation or database needed.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdCatalog.cpp" "$root/tests/catalog_tests.cpp" \
  -o "$work/catalog_tests"
"$work/catalog_tests"

"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdActivities.cpp" "$root/tests/activity_tests.cpp" \
  -o "$work/activity_tests"
"$work/activity_tests"
