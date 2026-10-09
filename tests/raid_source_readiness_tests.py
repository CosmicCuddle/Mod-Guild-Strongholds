#!/usr/bin/env python3
"""Synthetic preflight tests: positive matches never certify human/raid proofs."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SCANNER = ROOT / "scripts/audit-raid-source-readiness.py"
spec = importlib.util.spec_from_file_location("audit_raid", SCANNER)
assert spec and spec.loader
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def write(root: Path, path: str, content: str) -> None:
    target = root / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(content, encoding="utf-8")


def main() -> None:
    count = 0

    def check(expr: bool, why: str) -> None:
        nonlocal count
        count += 1
        assert expr, why

    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp) / "core"
        try:
            mod.audit_checkout(root)
        except ValueError:
            count += 1
        else:
            raise AssertionError("Non-checkout path accepted")
        write(root, "src/server/game/FakeScriptMgr.h",
              "OnUnitDeath(); OnPlayerCreatureKillCredit(); "
              "SetBossState(); GetBossState(); GetInstanceId(); DealDamage(); "
              "HealBySpell(); GetCharmerOrOwner();")
        write(root, "modules/mod-playerbots/src/Bot.cpp",
              "GetPlayerbotAI(player); sPlayerbotAIConfig.enabled; IsHeadless();")
        write(root, "modules/mod-individual-progression/src/Progress.cpp",
              "GetGroup(); GetGuild(); GetCreatedDate();")
        write(root, "modules/mod-playerbots/src/Comment.cpp",
              "// GetPlayerbotAI(player); not necessarily a real implementation")
        write(root, "modules/mod-playerbots/conf/server.conf",
              "DB_PASSWORD=SENSITIVE_SENTINEL")
        write(root, "modules/mod-playerbots/src/ignored.txt",
              "OTHER_SECRET_SENTINEL")
        invalid = root / "modules/mod-playerbots/src/nonutf8.cpp"
        invalid.write_bytes(b"\xff\xfe")
        outside = Path(temp) / "outside.cpp"
        outside.write_text("GetPlayerbotAI(); LINK_SECRET_SENTINEL", encoding="utf-8")
        (root / "modules/mod-playerbots/src/symlink.cpp").symlink_to(outside)
        original = (root / "modules/mod-playerbots/src/Bot.cpp").read_bytes()

        data = mod.audit_checkout(root)
        for field in ("human_control", "boss_completion", "member_contribution",
                      "original_guild_generation"):
            check(data[field] == "UNVERIFIED", field + " must always deny")
        check(data["decision"] == "REVIEW_REQUIRED", "No compatibility certification")
        check(data["trophy_award_allowed"] is False, "No gameplay authorization")
        check(data["read_only"], "Read-only report")
        check(not data["missing_core_source"], "Core fixture was scanned")
        check(data["incomplete_scan"], "Unreadable sources flagged")
        check(data["scopes"]["core_game"]["signal_counts"]["unit_death_hook"] == 1,
              "Boss-like generic death hook discovered")
        check(data["scopes"]["core_game"]["signal_counts"]["creature_kill_credit"] == 1,
              "Kill credit signature discovered")
        check(data["scopes"]["module:mod-playerbots"]
              ["signal_counts"]["bot_registry_lookup"] == 2,
              "Comments count as hits; still no proof")
        check(data["scopes"]["module:mod-playerbots"]["files_skipped"] == 2,
              "Non-UTF8 and symlink skipped")
        check("module:mod-individual-progression" in data["scopes"],
              "Additional modules enumerated")
        check(original == (root / "modules/mod-playerbots/src/Bot.cpp").read_bytes(),
              "All existing source unchanged")
        combined = json.dumps(data)
        for secret in ("SENSITIVE_SENTINEL", "OTHER_SECRET_SENTINEL",
                       "LINK_SECRET_SENTINEL", "Bot.cpp", str(root)):
            check(secret not in combined, "No secrets, excerpts or paths")
        check("example_source_paths" not in combined,
              "No source path leakage")
        report = subprocess.run(
            [sys.executable, str(SCANNER), str(root), "--json"],
            capture_output=True, text=True, check=True)
        encoded = json.loads(report.stdout)
        check(encoded["human_control"] == "UNVERIFIED", "CLI never proves human")
        check(encoded["trophy_award_allowed"] is False, "CLI never grants award")
        check("SENSITIVE_SENTINEL" not in report.stdout, "No secret prints")
        check(report.stderr == "", "Successful audit reports no errors")

        empty = Path(temp) / "empty"
        (empty / "modules").mkdir(parents=True)
        result = mod.audit_checkout(empty)
        check(result["incomplete_scan"] and result["missing_core_source"],
              "Missing core source cannot silently pass")
        check(result["trophy_award_allowed"] is False, "Missing source refuses award")

        write(root, "modules/mod-playerbots/src/large.cpp",
              "padding" * (mod.MAX_BYTES // 5))
        second = mod.audit_checkout(root)
        check(second["incomplete_scan"], "Oversize source reported")
        previous_limit = mod.MAX_FILES
        try:
            mod.MAX_FILES = 1
            third = mod.audit_checkout(root)
            check(third["scopes"]["module:mod-playerbots"]["file_limit_reached"],
                  "File limit reported as incomplete")
        finally:
            mod.MAX_FILES = previous_limit

        invalid_call = subprocess.run(
            [sys.executable, str(SCANNER), str(Path(temp) / "bad")],
            capture_output=True, text=True)
        check(invalid_call.returncode != 0, "Invalid root returns error")
        check("REVIEW_REQUIRED" not in invalid_call.stdout,
              "Invalid root never reports a successful scan")
    print(f"PASS: {count} synthetic raid source preflight negative/privacy checks")


if __name__ == "__main__":
    main()
