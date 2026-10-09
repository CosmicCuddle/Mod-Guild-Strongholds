#!/usr/bin/env python3
"""Read-only raid/Playerbots source preflight. NEVER grants readiness or rewards.

Text signature matches (including comments) are not source validation.
No SQL, config, account, player, network or other repository writes/reads.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import sys

EXTENSIONS = {".cpp", ".cc", ".cxx", ".h", ".hpp"}
SKIP_DIRS = {".git", "build", "_deps", "node_modules", "tests", "test",
             "cmake-build-debug", "cmake-build-release"}
MAX_BYTES = 1024 * 1024
MAX_FILES = 15000
SIGNALS = {
    "bot_registry_lookup": r"\bGetPlayerbotAI\s*\(",
    "bot_config": r"\bsPlayerbotAIConfig\b",
    "headless_marker": r"\bIsHeadless\s*\(",
    "session_lookup": r"\bGetSession\s*\(",
    "group_lookup": r"\bGetGroup\s*\(",
    "guild_identity": r"\b(?:GetGuild|GetGuildId|GetCreatedDate)\s*\(",
    "guild_roster": r"\b(?:GetMember|GetMemberSlots)\s*\(",
    "unit_death_hook": r"\bOnUnitDeath\s*\(",
    "creature_kill_credit": r"\bOnPlayerCreatureKillCredit\s*\(",
    "instance_identity": r"\bGetInstanceId\s*\(",
    "boss_state_write": r"\bSetBossState\s*\(",
    "boss_state_read": r"\bGetBossState\s*\(",
    "damage_event": r"\b(?:DealDamage|DamageDealt|OnDamage|DamageTaken)\s*\(",
    "healing_event": r"\b(?:HealBySpell|HealReceived|OnHeal|HealDone)\s*\(",
    "absorb_event": r"\b(?:CalcAbsorb|AbsorbDamage|OnAbsorb)\s*\(",
    "summon_owner": r"\b(?:GetCharmerOrOwner|GetOwner|GetResponsiblePlayer)\s*\(",
    "attempt_reset": r"\b(?:EnterEvadeMode|Reset|JustReachedHome)\s*\(",
}
PATTERNS = {key: re.compile(expr) for key, expr in SIGNALS.items()}


def scan_cpp(path: Path) -> dict:
    found = {"files_scanned": 0, "files_skipped": 0,
             "file_limit_reached": False,
             "signal_counts": {key: 0 for key in PATTERNS}}
    if not path.is_dir() or path.is_symlink():
        return found
    for base, dirs, names in os.walk(path, followlinks=False):
        dirs[:] = sorted(name for name in dirs
                         if name not in SKIP_DIRS
                         and not (Path(base) / name).is_symlink())
        for name in sorted(names):
            item = Path(base) / name
            if item.suffix.lower() not in EXTENSIONS:
                continue
            if item.is_symlink():
                found["files_skipped"] += 1
                continue
            if found["files_scanned"] >= MAX_FILES:
                found["file_limit_reached"] = True
                found["files_skipped"] += 1
                continue
            try:
                if not item.is_file() or item.stat().st_size > MAX_BYTES:
                    found["files_skipped"] += 1
                    continue
                text = item.read_text(encoding="utf-8")
            except (OSError, UnicodeError):
                found["files_skipped"] += 1
                continue
            found["files_scanned"] += 1
            for key, pattern in PATTERNS.items():
                found["signal_counts"][key] += len(pattern.findall(text))
    return found


def audit_checkout(root: Path) -> dict:
    if root.is_symlink() or not (root / "modules").is_dir():
        raise ValueError("Provide an AzerothCore source checkout containing modules/")
    root = root.resolve()
    scopes = {
        "core_game": scan_cpp(root / "src/server/game"),
        "core_scripts": scan_cpp(root / "src/server/scripts"),
    }
    for module in sorted((root / "modules").iterdir(),
                         key=lambda p: p.name.casefold()):
        if module.is_dir() and not module.is_symlink():
            scopes["module:" + module.name] = scan_cpp(module / "src")

    missing_core = (scopes["core_game"]["files_scanned"] == 0
                    and scopes["core_scripts"]["files_scanned"] == 0)
    return {
        "read_only": True,
        "decision": "REVIEW_REQUIRED",
        "human_control": "UNVERIFIED",
        "boss_completion": "UNVERIFIED",
        "member_contribution": "UNVERIFIED",
        "original_guild_generation": "UNVERIFIED",
        "trophy_award_allowed": False,
        "missing_core_source": missing_core,
        "incomplete_scan": missing_core or any(
            entry["files_skipped"] or entry["file_limit_reached"]
            for entry in scopes.values()
        ),
        "scopes": scopes,
        "limitations": [
            "Text-only hits include comments and disabled code, not just compiled APIs.",
            "Zero signatures does not establish that a fork lacks an equivalent implementation.",
            "GetPlayerbotAI() nullptr is UNKNOWN and NEVER positive human proof.",
            "Non-headless sessions alone cannot prove human control.",
            "Boss death and kill credit cannot prove encounter completion or contribution.",
            "Pet/guardian/totem attribution, instance, guild generation and attempt resets require manual review.",
            "No configuration, patches, binaries or live runtime state were verified.",
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("azerothcore_source", type=Path,
                        help="Path to a separate STAGING source checkout")
    parser.add_argument("--json", action="store_true",
                        help="Print only anonymised counts/status as JSON")
    args = parser.parse_args()
    try:
        result = audit_checkout(args.azerothcore_source)
    except ValueError as exc:
        parser.error(str(exc))
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print("GUILD STRONGHOLDS: READ-ONLY RAID SOURCE INVENTORY")
        print("REVIEW_REQUIRED; human/boss/contribution proof: UNVERIFIED")
        print("Trophy awards allowed: NO; source modifications: NONE")
        if result["incomplete_scan"]:
            print("WARNING: source scan is INCOMPLETE")
        for name, item in result["scopes"].items():
            print(f"  {name}: scanned={item['files_scanned']}; skipped={item['files_skipped']}")
            signals = [f"{key}={count}" for key, count in item["signal_counts"].items()
                       if count]
            print("    " + (", ".join(signals) if signals
                              else "no matches (NOT evidence of absence)"))
        print("Separate manual source and staging runtime verification required.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
