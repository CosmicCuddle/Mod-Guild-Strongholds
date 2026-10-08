#!/usr/bin/env python3
"""Tests read-only source audit on synthetic core/module fixtures."""
from __future__ import annotations

import importlib.util
from pathlib import Path
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "scripts/audit-isolation-compatibility.py"
spec = importlib.util.spec_from_file_location("isolation_audit", SOURCE)
module = importlib.util.module_from_spec(spec)
assert spec and spec.loader
spec.loader.exec_module(module)


def write(root: Path, filename: str, text: str) -> None:
    target = root / filename
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(text, encoding="utf-8")


def main() -> None:
    checks = 0

    def test(value: bool, title: str) -> None:
        nonlocal checks
        checks += 1
        if not value:
            raise AssertionError(title)

    with tempfile.TemporaryDirectory() as temp:
        path = Path(temp) / "azerothcore"
        try:
            module.audit_checkout(path)
        except ValueError:
            checks += 1
        else:
            raise AssertionError("Missing modules directory not rejected")

        write(path, "modules/mod-individual-progression/src/IndividualProgression.cpp",
              "player->CastSpell(player, IPP_PHASE_III, false);")
        write(path, "modules/mod-guildhouse/src/GuildHouse.cpp",
              "void OnBeforeWorldObjectSetPhaseMask();\n"
              "player->SetPhaseMask(123, true);")
        write(path, "modules/mod-playerbots/src/Basic.cpp",
              "void Basic() {}")
        write(path, "modules/Mod-Guild-Strongholds/src/StrongholdBootstrap.cpp",
              "void OnBeforeConfigLoad();")
        write(path, "modules/mod-guildhouse/secret.conf",
              "password=SHOULD_NEVER_APPEAR_IN_AUDIT")
        write(path, "src/server/game/Maps/MapMgr.cpp", "CreateMap(...)")
        write(path, "src/server/game/Maps/MapInstanced.cpp",
              "PlayerGetDestinationInstanceId")
        write(path, "src/server/game/Entities/Object/Object.h",
              "InSamePhase(uint32)")
        write(path, "src/server/game/Entities/Player/Player.cpp",
              "TeleportTo()")

        result = module.audit_checkout(path)
        by_name = {m["name"]: m for m in result["modules"]}
        test(result["read_only"], "Tool guarantees read-only intent")
        test(result["result"] == "REVIEW_REQUIRED", "No fake compatibility approval")
        test(result["module_count"] == 4, "All module folders identified")
        test(not result["missing_reference_core_files"], "Expected core paths present")
        test(by_name["mod-guildhouse"]["signal_counts"]["global_phase_override"] == 1,
             "Zone-global phase override detected")
        test(by_name["mod-guildhouse"]["signal_counts"]["phase_assignment"] == 1,
             "Direct phase write detected")
        test(by_name["mod-individual-progression"]
             ["signal_counts"]["phase_related_spell"] == 1,
             "Individual Progression phase spell detected")
        test(sum(by_name["mod-playerbots"]["signal_counts"].values()) == 0,
             "Absence of source matches does not block scanner")
        test(all(m["own_git_revision"] is None for m in result["modules"]),
             "Do not misreport core commit as module commit")
        test("SHOULD_NEVER_APPEAR" not in str(result),
             "Configuration credentials never emitted")
        test((path / "modules/mod-guildhouse/secret.conf").read_text() ==
             "password=SHOULD_NEVER_APPEAR_IN_AUDIT",
             "No source files changed")

        (path / "src/server/game/Maps/MapMgr.cpp").unlink()
        second = module.audit_checkout(path)
        test("src/server/game/Maps/MapMgr.cpp" in
             second["missing_reference_core_files"],
             "Missing core reference file reported")
        test(second["result"] == "REVIEW_REQUIRED",
             "Missing source can never be certified")

    print(f"PASS: {checks} read-only core/module audit safety checks")


if __name__ == "__main__":
    main()
