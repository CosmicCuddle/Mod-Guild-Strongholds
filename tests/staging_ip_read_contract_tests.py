#!/usr/bin/env python3
"""Static defense for staging-only direct Grimfeather IP inspection.

Source shapes are necessary but NOT proof of the user's deployed fork.
This never runs AzerothCore, the IP module or a realm.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
adapter = (root / "src/StrongholdStagingIpAdapter.cpp").read_text()
header = (root / "src/StrongholdStagingIpAdapter.h").read_text()
steward = (root / "src/StrongholdStagingSteward.cpp").read_text()
config = (root / "conf/mod_naxx_guild_strongholds.conf.dist").read_text()

assert "#include \"IndividualProgression.h\"" in adapter
assert "sIndividualProgression" in adapter
assert "ip->GetPlayerProgressionFromQuests(player)" in adapter
assert "ip->enabled" in adapter
assert "ip->progressionLimit" in adapter
assert 'NaxxGuildStrongholds.StagingIpRead.Enabled", false' in adapter
assert "guildVerified" in adapter and "player->IsGameMaster()" in adapter
assert "player->IsInWorld()" in adapter
assert "#if defined(NAXX_GS_BUILD_STAGING_IP_READ)" in adapter
assert '#error "IP reads require staging Steward' in header
assert "ReadStagingIpState(player, guild.IsVerified())" in steward
assert "NaxxGuildStrongholds.StagingIpRead.Enabled = 0" in config
assert "evidence.Ip = ReadStagingIpState" in steward
assert "NAXX_GS_BUILD_STAGING_IP_READ" in steward

for bad in ("UpdateProgressionState(", "ForceUpdateProgressionState(",
            "SetPhaseMask(", "AddQuest(", "CompleteQuest(",
            "RewardQuest(", "TeleportTo(", "ModifyMoney(",
            "CharacterDatabase.", "WorldDatabase.", "LoginDatabase."):
    assert bad not in adapter, f"Staging IP adapter contains mutation: {bad}"

print("PASS: opt-in direct Grimfeather IP source read; no quests/phases mutated")
print("Installed-fork source and runtime compatibility still unverified")
