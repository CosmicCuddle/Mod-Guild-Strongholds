#!/usr/bin/env python3
"""Pure source check: staging death hook observes only, without rewards.

This test does not prove real raid participation or installed-fork APIs.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
listener = (root / "src/StrongholdStagingRaidObserver.cpp").read_text()
policy = (root / "src/StrongholdRaidDeathObservation.cpp").read_text()
header = (root / "src/StrongholdRaidDeathObservation.h").read_text()
loader = (root / "src/loader.cpp").read_text()
config = (root / "conf/mod_naxx_guild_strongholds.conf.dist").read_text()

assert "#if defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)" in listener
assert "UNITHOOK_ON_UNIT_DEATH" in listener
assert "void OnUnitDeath(Unit* unit, Unit* /*killer*/) override" in listener
assert "creature->IsDungeonBoss()" in listener
assert "map->IsRaid()" in listener
assert "NaxxGuildStrongholds.StagingRaidObserver.Enabled" in listener
assert "NaxxGuildStrongholds.StagingRaidObserver.Enabled = 0" in config
assert "#if defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)" in loader
assert "AddStagingRaidObserverScripts();" in loader
assert "BossEncounterVerified = false" in header
assert "GuildParticipationVerified = false" in header
assert "GuildTrophyGranted = false" in header
assert "RaidDeathObservationDecision::CandidateOnly" in policy
assert "NO proof of a specific" in listener

for source in (listener, policy, header):
    for token in (
        "CharacterDatabase.", "WorldDatabase.", "LoginDatabase.",
        "SetPhaseMask(", "TeleportTo(", "SummonCreature(",
        "SummonGameObject(", "AddQuest(", "CompleteQuest(",
        "RewardQuest(", "ModifyMoney(", "SaveToDB(",
        "ProposeTrophyUnlock(", "record_trophy(", "GetPlayerProgressionFromQuests(",
        "GetGuildId(", "GetMemberSlots(", "GetPlayers()",
    ):
        assert token not in source, f"Observer became effectful: {token}"

print("PASS: staging raid observer is death-candidate metadata only, no guild credit")
