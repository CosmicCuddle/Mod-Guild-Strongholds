#!/usr/bin/env bash
# Tests only within mktemp; no network/server/DB interaction.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
scratch="$(mktemp -d)"
trap 'rm -rf "$scratch"' EXIT

mkdir -p "$scratch/core/modules/mod-sample" "$scratch/core/modules/mod-unversioned"
git -C "$scratch/core" init -q
printf 'initial\n' > "$scratch/core/core.txt"
git -C "$scratch/core" add core.txt
git -C "$scratch/core" -c user.name='CI Test' -c user.email='ci@example.invalid' \
    commit -qm 'Core source fixture'

git -C "$scratch/core/modules/mod-sample" init -q
printf 'module initial\n' > "$scratch/core/modules/mod-sample/module.txt"
git -C "$scratch/core/modules/mod-sample" add module.txt
git -C "$scratch/core/modules/mod-sample" -c user.name='CI Test' \
    -c user.email='ci@example.invalid' commit -qm 'Module source fixture'
printf 'uncommitted tracked edit\n' >> "$scratch/core/modules/mod-sample/module.txt"
output="$(bash "$root/scripts/check-module-inventory.sh" "$scratch/core")"
echo "$output" | grep -Fq 'AzerothCore Git SHA:'
echo "$output" | grep -Fq 'AzerothCore tracked-file state: CLEAN_TRACKED'
echo "$output" | grep -Fq 'mod-sample | SHA'
echo "$output" | grep -Fq 'MODIFIED_OR_UNVERIFIED_TRACKED'
echo "$output" | grep -Fq 'mod-unversioned | SHA NO_INDEPENDENT_GIT | UNVERIFIED'
echo "$output" | grep -Fq 'REVIEW_REQUIRED'
if grep -Fq 'uncommitted tracked edit' <<< "$output"; then
    echo 'FAILED: inventory leaked private file contents' >&2
    exit 1
fi

# Modification of the actual core checkout must not be hidden by its SHA.
printf 'core modified\n' >> "$scratch/core/core.txt"
output="$(bash "$root/scripts/check-module-inventory.sh" "$scratch/core")"
echo "$output" | grep -Fq 'AzerothCore tracked-file state: MODIFIED_OR_UNVERIFIED_TRACKED'

# Missing directory must fail safely without guessing the user's checkout.
if bash "$root/scripts/check-module-inventory.sh" "$scratch/missing" 2>/dev/null; then
    echo 'FAILED: accepted a missing checkout' >&2
    exit 1
fi
printf 'PASS: read-only core and module SHA/dirty/unversioned inventory fixtures\n'
