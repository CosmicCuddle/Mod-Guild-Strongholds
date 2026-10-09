#!/usr/bin/env python3
"""Read-only compatibility contract for the reviewed Grimfeather IP source.

It checks only public API and reviewed semantics, never alters the fork or
proves full gameplay/runtime compatibility. The default CLI fails closed if
contract expectations differ. The explicit --revision requirement in CI pins
the reviewed source snapshot.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import subprocess
import sys

PINNED_REVIEWED_COMMIT = "706740808fee328b8557607f87b0548cf961e047"

EXPECTED_STAGES = {
    "PROGRESSION_START": 0,
    "PROGRESSION_MOLTEN_CORE": 1,
    "PROGRESSION_ONYXIA": 2,
    "PROGRESSION_BLACKWING_LAIR": 3,
    "PROGRESSION_NAXX40": 7,
    "PROGRESSION_PRE_TBC": 8,
    "PROGRESSION_TBC_TIER_1": 9,
    "PROGRESSION_TBC_TIER_5": 13,
    "PROGRESSION_WOTLK_TIER_1": 14,
    "PROGRESSION_WOTLK_TIER_3": 16,
    "PROGRESSION_WOTLK_TIER_4": 17,
    "PROGRESSION_WOTLK_TIER_5": 18,
}

EXPECTED_PHASE_SPELLS = {
    "IPP_PHASE": 89509,
    "IPP_PHASE_II": 89511,
    "IPP_PHASE_III": 89513,
    "IPP_PHASE_IV": 89515,
    "IPP_PHASE_V": 89517,
    "IPP_PHASE_VI": 89519,
}


def strip_comments(code: str) -> str:
    """Remove ordinary C++ comments to avoid matching inactive enum lines."""
    return re.sub(
        r"//[^\n]*|/\*.*?\*/", "", code, flags=re.DOTALL
    )


def function_body(code: str, signature: str, next_signature: str) -> str:
    start = code.find(signature)
    stop = code.find(next_signature, start + len(signature))
    return code[start:stop] if start >= 0 and stop > start else ""


def validate(header: str, implementation: str, player_script: str) -> list[str]:
    failures: list[str] = []
    header_code = strip_comments(header)
    impl_code = strip_comments(implementation)
    player_code = strip_comments(player_script)

    for name, value in {**EXPECTED_STAGES, **EXPECTED_PHASE_SPELLS}.items():
        match = re.search(r"\b" + re.escape(name) + r"\s*=\s*(\d+)\b",
                          header_code)
        if not match or int(match.group(1)) != value:
            failures.append(f"Stage/spell constant changed or missing: {name}={value}")

    for text, title in (
        (r"\bbool\s+enabled\b", "public enabled flag"),
        (r"\bint\s+progressionLimit\b", "public progressionLimit"),
        (r"GetPlayerProgressionFromQuests\s*\(\s*Player\s*\*",
         "read-only progression getter"),
        (r"hasPassedProgression\s*\(\s*Player\s*\*", "limit-aware progress check"),
    ):
        if not re.search(text, header_code):
            failures.append(f"IP public API missing: {title}")

    reader = function_body(
        impl_code,
        "IndividualProgression::GetPlayerProgressionFromQuests",
        "IndividualProgression::hasPassedProgression",
    )
    for pattern, label in (
        (r"if\s*\(\s*!player\s*\|\|\s*!player->IsInWorld\s*\(\s*\)\s*\)",
         "off-world getter must not grant progression"),
        (r"\b66000\s*\+\s*i\b", "hidden quest ID scheme 66000+i"),
        (r"\bQUEST_STATUS_REWARDED\b", "rewarded quest state source"),
        (r"\bPROGRESSION_WOTLK_TIER_5\b", "highest progression stage"),
    ):
        if not re.search(pattern, reader):
            failures.append(f"IP progression reader changed: {label}")

    checker = function_body(
        impl_code,
        "IndividualProgression::hasPassedProgression",
        "IndividualProgression::isBeforeProgression",
    )
    for pattern, label in (
        (r"!\s*enabled\b", "IP disabled gate"),
        (r"!player->IsInWorld\s*\(\s*\)", "player in world check"),
        (r"progressionLimit\s*&&\s*\(\s*state\s*>\s*progressionLimit\s*\)",
         "configured limit gate"),
        (r"GetPlayerProgressionFromQuests\s*\(\s*player\s*\)\s*>=\s*state",
         "effective milestone comparison"),
    ):
        if not re.search(pattern, checker):
            failures.append(f"IP progression limits changed: {label}")

    for name in ("OnPlayerUpdateZone", "OnPlayerUpdateArea", "checkIPPhasing"):
        if name not in player_code:
            failures.append(f"IP phasing hook missing or changed: {name}")

    return failures


def current_revision(root: Path) -> str | None:
    try:
        return subprocess.run(
            ["git", "-C", str(root), "rev-parse", "HEAD"],
            check=True, capture_output=True, text=True, timeout=5,
        ).stdout.strip()
    except (OSError, subprocess.CalledProcessError, subprocess.TimeoutExpired):
        return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ip_repository", type=Path,
                        help="Local SOURCE checkout of Grimfeather IP (read only)")
    parser.add_argument("--expect-revision", type=str,
                        help="Require this exact commit, e.g. the pinned review")
    args = parser.parse_args()

    root = args.ip_repository.resolve()
    try:
        header = (root / "src/IndividualProgression.h").read_text(encoding="utf-8")
        cpp = (root / "src/IndividualProgression.cpp").read_text(encoding="utf-8")
        player = (root / "src/IndividualProgressionPlayer.cpp").read_text(encoding="utf-8")
    except OSError as exc:
        print(f"BLOCKED: source files missing or unreadable: {exc}", file=sys.stderr)
        return 1

    issues = validate(header, cpp, player)
    commit = current_revision(root)
    if args.expect_revision and commit != args.expect_revision:
        issues.append(
            f"Actual Git revision {commit or 'UNKNOWN'} != "
            f"required reviewed revision {args.expect_revision}"
        )

    if issues:
        print("BLOCKED: Grimfeather IP source contract mismatched")
        for issue in issues:
            print(" -", issue)
        return 1

    print("PASS: Reviewed IP source API and progression-cap contract matches")
    print("Revision:", commit or "UNKNOWN (not an approval)")
    print("Runtime compatibility: UNVERIFIED — housing remains disabled")
    print("No files, quests, spells, settings or characters were changed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
