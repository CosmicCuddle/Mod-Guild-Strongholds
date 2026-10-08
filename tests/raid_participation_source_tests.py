#!/usr/bin/env python3
"""Fail closed: the new staging PlayerScript may observe credit, not grant it."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
listener = (root / "src/StrongholdStagingRaidKillCreditObserver.cpp").read_text()
header = (root / "src/StrongholdRaidKillCreditObservation.h").read_text()
policy = (root / "src/StrongholdRaidParticipation.cpp").read_text()
provenance = (root / "src/StrongholdRaidParticipation.h").read_text()
loader = (root / "src/loader.cpp").read_text()
config = (root / "conf/mod_naxx_guild_strongholds.conf.dist").read_text()

assert "PLAYERHOOK_ON_CREATURE_KILL_CREDIT" in listener
assert "void OnPlayerCreatureKillCredit(Player* player, Creature* killed) override" in listener
assert "#if defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)" in listener
assert "NaxxGuildStrongholds.StagingRaidObserver.Enabled" in listener
assert "NaxxGuildStrongholds.StagingRaidObserver.Enabled = 0" in config
assert "AddStagingRaidKillCreditObserverScripts();" in loader
assert "SameNonzeroInstance" in listener
assert "HumanVerified = false" in header
assert "EncounterContributionVerified = false" in header
assert "TrophyGranted = false" in header
assert "ControlClassificationSourceVerified" in provenance
assert "PlayerbotsSourceVerified" in provenance
assert "EncounterParticipationVerified" in provenance
assert "ParticipantControl::Unknown" in policy
assert "RaidParticipationDecision::ProposalOnly" in policy

for src in (listener, header, policy, provenance):
    for forbidden in (
        "CharacterDatabase.", "WorldDatabase.", "LoginDatabase.",
        "TeleportTo(", "SetPhaseMask(", "SummonGameObject(",
        "SummonCreature(", "Award", "RewardQuest(", "CompleteQuest(",
        "ModifyMoney(", "AddQuest(", "StoreNewItem(", "SaveToDB(",
        "ProposeTrophyUnlock(", "GetPlayerbotAI(", "IsHeadless()",
        "GetGuildId(", "GetPlayerProgressionFromQuests(",
    ):
        assert forbidden not in src, f"Unsafe candidate source: {forbidden}"

print("PASS: PlayerScript kill-credit observer never infers humans or grants trophies")
