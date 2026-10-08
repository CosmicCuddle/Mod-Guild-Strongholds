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

"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdCatalog.cpp" "$root/src/StrongholdConstruction.cpp" \
  "$root/tests/construction_tests.cpp" -o "$work/construction_tests"
"$work/construction_tests"

# Diagnostic-only WorldScript — mock core compilation (not user core).
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/tests/startup_gate_tests.cpp" \
  -o "$work/startup_gate_tests"
"$work/startup_gate_tests"

"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/tests/fake_azerothcore" -I"$root/src" \
  "$root/src/StrongholdBootstrap.cpp" "$root/src/loader.cpp" \
  "$root/tests/bootstrap_smoke_tests.cpp" \
  -o "$work/bootstrap_smoke_tests"
"$work/bootstrap_smoke_tests"

"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdCatalog.cpp" "$root/src/StrongholdProperty.cpp" \
  "$root/tests/property_tests.cpp" -o "$work/property_tests"
"$work/property_tests"
