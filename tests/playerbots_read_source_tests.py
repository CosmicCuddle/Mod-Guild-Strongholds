#!/usr/bin/env python3
"""No human-status inference, player privacy leak or effects in opt-in bot read.

Does NOT certify installed Playerbots/individual-progression forks, or in-game
participation. The 'ReviewedSource' config is an attestation, not a fingerprint.
"""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
adapter = (root / "src/StrongholdStagingPlayerbotsRead.cpp").read_text()
header = (root / "src/StrongholdStagingPlayerbotsRead.h").read_text()
policy = (root / "src/StrongholdPlayerbotsRead.cpp").read_text()
observer = (root / "src/StrongholdStagingRaidKillCreditObserver.cpp").read_text()
config = (root / "conf/mod_naxx_guild_strongholds.conf.dist").read_text()

assert "#include \"PlayerbotMgr.h\"" in adapter
assert "#include \"PlayerbotAIConfig.h\"" in adapter
assert "sPlayerbotAIConfig.enabled" in adapter
assert "sPlayerbotsMgr.GetPlayerbotAI(player)" in adapter
assert "return ParticipantControl::Unknown;" in adapter
assert "return ParticipantControl::VerifiedPlayerbot;" in policy
assert "ParticipantControl::VerifiedHuman" not in policy.replace(
    "return ParticipantControl::VerifiedHuman;", "")
assert "return ParticipantControl::VerifiedHuman;" not in policy
assert "NAXX_GS_BUILD_STAGING_PLAYERBOTS_READ" in header
assert "#error" in header
assert "NAXX_GS_BUILD_STAGING_PLAYERBOTS_READ" in observer
assert "ReadStagingPlayerbotsControl(player)" in observer
assert "StagingPlayerbotsRead.Enabled = 0" in config
assert "StagingPlayerbotsRead.ReviewedSource = 0" in config

for source in (adapter, policy, observer):
    for action in (
        "CharacterDatabase.", "WorldDatabase.", "LoginDatabase.",
        "TeleportTo(", "SetPhaseMask(", "RewardQuest(", "CompleteQuest(",
        "AddQuest(", "ProposeTrophyUnlock(", "GetMemberSlots(",
        "SummonCreature(", "SummonGameObject(", "SaveToDB(",
    ):
        assert action not in source, f"Forbidden gameplay mutation: {action}"

print("PASS: positive-bot-only read, default unknown, no gameplay effects")
