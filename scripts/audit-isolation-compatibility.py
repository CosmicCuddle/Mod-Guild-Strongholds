#!/usr/bin/env python3
"""Read-only AzerothCore module source audit for housing isolation risks.

No network, database access, file writes, config reads, or uploads.
Finds possible phase/teleport/instance interference for HUMAN review.
It never certifies a build safe or reports "compatible".
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

SKIP_DIRS = {".git", ".cache", "build", "cmake-build-debug",
             "cmake-build-release", "node_modules", "_deps"}
EXTENSIONS = {".cpp", ".cc", ".cxx", ".h", ".hpp"}
MAX_FILE_SIZE = 8 * 1024 * 1024
MAX_FILES_PER_MODULE = 10000
SIGNATURES = {
    "global_phase_override": re.compile(r"\bOnBeforeWorldObjectSetPhaseMask\b"),
    "phase_assignment": re.compile(r"\bSetPhaseMask\s*\("),
    "phase_related_spell": re.compile(r"\bIPP_PHASE(?:_[A-Z0-9_]+)?\b"),
    "teleport": re.compile(r"\bTeleportTo\s*\("),
    "instance_routing": re.compile(
        r"\b(?:CreateInstanceForPlayer|PlayerGetDestinationInstanceId)\b"
    ),
    "guild_lifecycle": re.compile(
        r"\b(?:OnGuildDisband|OnGuildRemoveMember)\b"
    ),
}
CORE_FILES = [
    "src/server/game/Maps/MapMgr.cpp",
    "src/server/game/Maps/MapInstanced.cpp",
    "src/server/game/Entities/Object/Object.h",
    "src/server/game/Entities/Player/Player.cpp",
]


def git_revision(directory: Path) -> str | None:
    """Only report a module SHA when that directory is its own git root."""
    try:
        root = subprocess.run(
            ["git", "-C", str(directory), "rev-parse", "--show-toplevel"],
            check=True, capture_output=True, text=True, timeout=3,
        ).stdout.strip()
        if Path(root).resolve() != directory.resolve():
            return None
        return subprocess.run(
            ["git", "-C", str(directory), "rev-parse", "HEAD"],
            check=True, capture_output=True, text=True, timeout=3,
        ).stdout.strip()
    except (OSError, subprocess.CalledProcessError, subprocess.TimeoutExpired):
        return None


def scan_cpp(directory: Path) -> dict:
    counts = {name: 0 for name in SIGNATURES}
    files_by_signal = {name: [] for name in SIGNATURES}
    examined = 0
    skipped = 0

    for base, dirs, files in os.walk(directory, followlinks=False):
        dirs[:] = sorted(
            d for d in dirs if d not in SKIP_DIRS and
            not (Path(base) / d).is_symlink()
        )
        for filename in sorted(files):
            path = Path(base) / filename
            if path.suffix.lower() not in EXTENSIONS or path.is_symlink():
                continue
            if examined >= MAX_FILES_PER_MODULE:
                skipped += 1
                continue
            try:
                if path.stat().st_size > MAX_FILE_SIZE:
                    skipped += 1
                    continue
                contents = path.read_text(encoding="utf-8", errors="replace")
            except OSError:
                skipped += 1
                continue
            examined += 1
            relpath = path.relative_to(directory).as_posix()
            for label, regex in SIGNATURES.items():
                count = len(regex.findall(contents))
                if count:
                    counts[label] += count
                    # Never emit source text (or config secrets), only a path.
                    if len(files_by_signal[label]) < 12:
                        files_by_signal[label].append(relpath)

    return {
        "files_examined": examined,
        "files_skipped_or_truncated": skipped,
        "signal_counts": counts,
        "example_source_paths": files_by_signal,
    }


def audit_checkout(root: Path) -> dict:
    root = root.resolve()
    modules_dir = root / "modules"
    if not modules_dir.is_dir():
        raise ValueError("Expected an AzerothCore checkout containing modules/")

    missing = [name for name in CORE_FILES if not (root / name).is_file()]
    modules = []

    for directory in sorted(modules_dir.iterdir(), key=lambda d: d.name.lower()):
        if not directory.is_dir() or directory.is_symlink():
            continue
        analysis = scan_cpp(directory)
        modules.append({
            "name": directory.name,
            "own_git_revision": git_revision(directory),
            **analysis,
        })

    return {
        "read_only": True,
        "result": "REVIEW_REQUIRED",  # never auto-certify any fork
        "core_checkout_revision": git_revision(root),
        "missing_reference_core_files": missing,
        "module_count": len(modules),
        "modules": modules,
        "limitations": [
            "Matches are textual; comments can cause false positives.",
            "No module list proves which scripts were compiled or enabled.",
            "A missing signal is NOT evidence of absence of conflict.",
            "Phasing spells, server patches, DB scripts and client changes need manual audit.",
            "Only a tested two-guild staging realm can prove actual isolation.",
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("azerothcore_source", type=Path,
                        help="Path to local AzerothCore source checkout")
    parser.add_argument("--json", action="store_true",
                        help="Emit machine-readable results to stdout")
    args = parser.parse_args()
    try:
        result = audit_checkout(args.azerothcore_source)
    except ValueError as error:
        parser.error(str(error))

    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print("Naxxramas Guild Strongholds — READ-ONLY source audit")
        print("Decision: REVIEW REQUIRED (never automatic approval)")
        print("Core SHA:", result["core_checkout_revision"] or "unavailable")
        if result["missing_reference_core_files"]:
            print("Unrecognised/missing core paths:",
                  ", ".join(result["missing_reference_core_files"]))
        print("Module folders:", result["module_count"])
        for module in result["modules"]:
            print(f"  {module['name']}: "
                  f"revision={module['own_git_revision'] or 'unavailable'}")
            matches = [
                f"{key}={number}"
                for key, number in module["signal_counts"].items()
                if number
            ]
            print("    Signals:", ", ".join(matches) if matches else
                  "none detected (NOT proof of safety)")
            if module["files_skipped_or_truncated"]:
                print("    Warning: some source files could not be scanned")
        print("No changes made. Do NOT enable housing based on this scan.")
        print("Use --json to review source file paths for possible conflicts.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
