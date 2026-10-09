#!/usr/bin/env bash
# READ-ONLY local source inventory for a future SEPARATE staging clone.
# Does not print modified paths, Git remotes, config files, user credentials,
# secret content, data records or database connection details.
# Untracked files are NOT covered by the dirty state; report separately.
set -euo pipefail
export GIT_OPTIONAL_LOCKS=0

if [[ $# -ne 1 ]]; then
    printf 'Usage: bash %s /path/to/azerothcore-STAGING-SOURCE\n' "$0" >&2
    exit 2
fi

root="$1"
if [[ ! -d "$root/modules" ]]; then
    printf 'BLOCKED: Directory is not an AzerothCore modules checkout\n' >&2
    exit 2
fi

# Git's SHA does not include uncommitted tracked modifications. We report a
# compact flag only; never echo file paths or the contents of local patches.
dirty_state() {
    local path="$1"
    if git -C "$path" diff --no-ext-diff --quiet -- &&
       git -C "$path" diff --cached --no-ext-diff --quiet --; then
        printf 'CLEAN_TRACKED'
    else
        printf 'MODIFIED_OR_UNVERIFIED_TRACKED'
    fi
}

if command -v git >/dev/null 2>&1 &&
    git -C "$root" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    printf 'AzerothCore Git SHA: %s\n' "$(git -C "$root" rev-parse HEAD)"
    printf 'AzerothCore tracked-file state: %s\n' "$(dirty_state "$root")"
else
    printf 'AzerothCore Git SHA: UNKNOWN (no Git source)\n'
    printf 'AzerothCore tracked-file state: UNVERIFIED\n'
fi
printf 'Installed-module source directories (NOT proof compiled/enabled):\n'

found=0
for path in "$root"/modules/*; do
    [[ -d "$path" ]] || continue
    found=1
    name="${path##*/}"
    revision='NO_INDEPENDENT_GIT'
    tracked='UNVERIFIED'

    if command -v git >/dev/null 2>&1 &&
        git -C "$path" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        top="$(git -C "$path" rev-parse --show-toplevel)"
        physical="$(cd "$path" && pwd -P)"
        if [[ "$(cd "$top" && pwd -P)" == "$physical" ]]; then
            revision="$(git -C "$path" rev-parse HEAD)"
            tracked="$(dirty_state "$path")"
        fi
    fi
    printf '  %s | SHA %s | %s\n' "$name" "$revision" "$tracked"
done

if [[ "$found" -eq 0 ]]; then
    printf '  (no module source directories)\n'
fi
printf 'READ_ONLY: No files, live game records or databases modified.\n'
printf 'REVIEW_REQUIRED: Untracked files, local config/patches, compile flags,\n'
printf '  dynamic binaries and actual module enablement were NOT checked.\n'
printf 'REVIEW_REQUIRED: This inventory does NOT certify module compatibility.\n'
