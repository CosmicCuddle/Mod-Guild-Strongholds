#!/usr/bin/env bash
# Read-only module source inventory. Does not alter files, databases or config.
set -euo pipefail

if [[ $# -ne 1 ]]; then
  printf 'Usage: bash %s /path/to/azerothcore\n' "$0" >&2
  exit 2
fi

root="$1"
if [[ ! -d "$root/modules" ]]; then
  printf 'Not an AzerothCore checkout with modules/: %s\n' "$root" >&2
  exit 2
fi

if command -v git >/dev/null 2>&1 && git -C "$root" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  printf 'AzerothCore checkout: %s\n' "$(git -C "$root" rev-parse HEAD)"
else
  printf 'AzerothCore checkout: unknown Git revision\n'
fi
printf 'Module source directories (not proof modules are compiled or enabled):\n'

found=0
for path in "$root"/modules/*; do
  [[ -d "$path" ]] || continue
  found=1
  name="${path##*/}"
  if command -v git >/dev/null 2>&1 && git -C "$path" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    sha="$(git -C "$path" rev-parse HEAD)"
    printf '  %s | %s\n' "$name" "$sha"
  else
    printf '  %s | no Git revision found\n' "$name"
  fi
done

if [[ "$found" -eq 0 ]]; then
  printf '  (none found)\n'
fi
printf 'Read-only inventory finished. No server changes made.\n'
