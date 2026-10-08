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
  revision="no separate Git revision found"
  if command -v git >/dev/null 2>&1 && git -C "$path" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    module_top="$(git -C "$path" rev-parse --show-toplevel)"
    module_dir="$(cd "$path" && pwd -P)"
    if [[ "$(cd "$module_top" && pwd -P)" == "$module_dir" ]]; then
      revision="$(git -C "$path" rev-parse HEAD)"
    fi
  fi
  printf '  %s | %s\n' "$name" "$revision"
done

if [[ "$found" -eq 0 ]]; then
  printf '  (none found)\n'
fi
printf 'Read-only inventory finished. No server changes made.\n'
