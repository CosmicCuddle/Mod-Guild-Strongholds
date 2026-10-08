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


"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdLifecycle.cpp" \
  "$root/tests/lifecycle_tests.cpp" -o "$work/lifecycle_tests"
"$work/lifecycle_tests"


"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdCatalog.cpp" \
  "$root/src/StrongholdLifecycle.cpp" "$root/src/StrongholdVisitGate.cpp" \
  "$root/tests/visit_gate_tests.cpp" -o "$work/visit_gate_tests"
"$work/visit_gate_tests"

"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdCatalog.cpp" \
  "$root/src/StrongholdLifecycle.cpp" "$root/src/StrongholdVisitGate.cpp" \
  "$root/src/StrongholdVisitRecovery.cpp" "$root/tests/visit_recovery_tests.cpp" \
  -o "$work/visit_recovery_tests"
"$work/visit_recovery_tests"

# Synthetic visibility model: no map, object, player or phase mutations.
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdIsolationProbe.cpp" \
  "$root/tests/isolation_probe_tests.cpp" -o "$work/isolation_probe_tests"
"$work/isolation_probe_tests"

# Opt-in staging command compiled using fake interfaces ONLY.
# No live character/phase/map/teleport code is run by this test.
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -DNAXX_GS_BUILD_STAGING_DIAGNOSTICS \
  -I"$root/tests/fake_diagnostics" -I"$root/src" \
  "$root/src/StrongholdStagingDiagnostics.cpp" \
  "$root/tests/staging_diagnostics_tests.cpp" \
  -o "$work/staging_diagnostics_tests"
"$work/staging_diagnostics_tests"


# Fork-aware read-only effective IP stage and progression-cap tests.
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdActivities.cpp" \
  "$root/src/StrongholdIpCompatibility.cpp" \
  "$root/tests/ip_compatibility_tests.cpp" \
  -o "$work/ip_compatibility_tests"
"$work/ip_compatibility_tests"

# Domain-only phase-slot allocator: no real-world phase assignments.
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdIsolationProbe.cpp" \
  "$root/src/StrongholdPhaseLease.cpp" "$root/tests/phase_lease_tests.cpp" \
  -o "$work/phase_lease_tests"
"$work/phase_lease_tests"

# Native instance-routing preflight only; never allocates real core instances.
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$root/src" "$root/src/StrongholdInstanceRoute.cpp" \
  "$root/tests/instance_route_tests.cpp" -o "$work/instance_route_tests"
"$work/instance_route_tests"
