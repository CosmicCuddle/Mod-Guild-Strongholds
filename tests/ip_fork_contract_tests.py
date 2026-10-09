#!/usr/bin/env python3
"""Synthetic positive and negative checks of reviewed IP source scanner."""
from __future__ import annotations

import importlib.util
from pathlib import Path

SOURCE = Path(__file__).resolve().parents[1] / "scripts/check-grimfeather-ip-contract.py"
spec = importlib.util.spec_from_file_location("ip_contract", SOURCE)
assert spec and spec.loader
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


def sample_sources() -> tuple[str, str, str]:
    header = "\n".join(
        f"{key} = {value},"
        for key, value in {**checker.EXPECTED_STAGES,
                           **checker.EXPECTED_PHASE_SPELLS}.items()
    )
    header += """
    bool enabled;
    int progressionLimit;
    uint8 GetPlayerProgressionFromQuests(Player* player) const;
    bool hasPassedProgression(Player* player, ProgressionState state) const;
    """
    cpp = """
    uint8 IndividualProgression::GetPlayerProgressionFromQuests(Player* player) const
    {
        if (!player || !player->IsInWorld()) return 0;
        for (uint8 i = PROGRESSION_MOLTEN_CORE; i <= PROGRESSION_WOTLK_TIER_5; ++i)
        {
            uint32 quest = 66000 + i;
            if (player->GetQuestStatus(quest) == QUEST_STATUS_REWARDED) return i;
        }
        return 0;
    }

    bool IndividualProgression::hasPassedProgression(Player* player, ProgressionState state) const
    {
        if (!enabled || !state || !player || !player->IsInWorld())
            return false;
        if (progressionLimit && (state > progressionLimit))
            return false;
        return sIndividualProgression->GetPlayerProgressionFromQuests(player) >= state;
    }
    bool IndividualProgression::isBeforeProgression(Player* player, ProgressionState state)
    { return false; }
    """
    player = "OnPlayerUpdateZone OnPlayerUpdateArea checkIPPhasing"
    return header, cpp, player


def main() -> None:
    cases = 0

    def test(ok: bool, description: str) -> None:
        nonlocal cases
        cases += 1
        if not ok:
            raise AssertionError(description)

    h, c, p = sample_sources()
    test(not checker.validate(h, c, p), "Unchanged reviewed source-shape accepted")
    test(checker.validate(h.replace("PROGRESSION_PRE_TBC = 8",
                                    "PROGRESSION_PRE_TBC = 9"), c, p),
         "Different TBC milestone fails closed")
    test(checker.validate(h.replace("IPP_PHASE = 89509", "IPP_PHASE = 10000"), c, p),
         "Different phase spell requires review")
    test(checker.validate(h, c.replace("66000 + i", "99999 + i"), p),
         "Hidden quest ID scheme change blocked")
    test(checker.validate(h, c.replace("QUEST_STATUS_REWARDED", "QUEST_STATUS_COMPLETE"), p),
         "Changed quest reward semantics blocked")
    test(checker.validate(h, c.replace(
        "if (progressionLimit && (state > progressionLimit))",
        "if (false)"), p), "Removed progression cap detected")
    test(checker.validate(h, c, p.replace("OnPlayerUpdateArea", "")),
         "Changed area hook requires review")
    test(checker.validate(h.replace("bool enabled;", "bool differentFlag;"), c, p),
         "Renamed enabled API is rejected")
    test(checker.validate(h.replace("PROGRESSION_PRE_TBC = 8,",
                                    "// PROGRESSION_PRE_TBC = 8,"), c, p),
         "Commented-out constant cannot pass scanner")
    test(checker.validate(h, c.replace("!player->IsInWorld()", "false"), p),
         "Removed off-world guard rejected")

    print(f"PASS: {cases} Grimfeather source contract shape/negative tests")


if __name__ == "__main__":
    main()
