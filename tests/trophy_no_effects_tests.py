#!/usr/bin/env python3
"""Source-level guard against accidental real housing/trophy effects.

C++ tests check the trophy eligibility policy. This test checks that the
domain/preview cannot mutate SQL, quest credit, bots or world objects.
Not a security proof against an unknown deployed fork.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
trophy = (root / "src/StrongholdTrophies.cpp").read_text(encoding="utf-8")
header = (root / "src/StrongholdTrophies.h").read_text(encoding="utf-8")
view = (root / "src/StrongholdStewardReadOnlyContent.cpp").read_text(encoding="utf-8")
startup = (root / "src/StrongholdStartupGate.h").read_text(encoding="utf-8")

for source in (trophy, header, view):
    for forbidden in (
        "CharacterDatabase.", "WorldDatabase.", "LoginDatabase.",
        "SetPhaseMask(", "TeleportTo(", "SummonGameObject(",
        "SummonCreature(", "SetQuestStatus(", "RewardQuest(",
        "CompleteQuest(", "ModifyMoney(", "AddQuest(", "Execute(",
        "PExecute(", "SaveToDB(", "CastSpell(", "Kill(",
    ):
        assert forbidden not in source, f"Trophy preview contains forbidden effect: {forbidden}"

assert 'case StewardPreviewAction::Trophies:\n            return PlannedTrophies();' in view
assert '"[PLAN] " + std::string(item.DisplayName)' in view
assert "GetTrophyCatalog()" in trophy
assert "RewardGranted = false" in header
assert "ObjectSpawned = false" in header
assert "DevelopmentCapabilities{}" in startup
assert "result.Decision = TrophyUnlockDecision::ProposalOnly;" in trophy
assert "result.Decision = TrophyPlacementDecision::ProposalOnly;" in trophy
assert "UniqueServerEventVerified" in header
assert "EncounterParticipationVerified" in header
assert "MinimumGuildHumans = 0" in header
print("PASS: trophy preview and proposals contain no DB/world/quest mutations")
print("Cannot award trophies or spawn decorations in this development build")
