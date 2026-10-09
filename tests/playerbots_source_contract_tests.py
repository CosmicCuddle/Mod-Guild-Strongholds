#!/usr/bin/env python3
"""Audit a pinned PUBLIC mod-playerbots source checkout without running bots.

The real Naxxramas fork may differ. In particular GetPlayerbotAI()==nullptr
MUST NOT count as human because it also returns null if the module is off.
"""
import pathlib
import sys

if len(sys.argv) != 2:
    raise SystemExit("Usage: python3 tests/playerbots_source_contract_tests.py /pinned-source")
root = pathlib.Path(sys.argv[1])
header = (root / "src/Bot/PlayerbotMgr.h").read_text()
source = (root / "src/Bot/PlayerbotMgr.cpp").read_text()
hooks = (root / "src/Script/Playerbots.cpp").read_text()

assert "PlayerbotAI* GetPlayerbotAI(Player* player);" in header
start = source.index("PlayerbotAI* PlayerbotsMgr::GetPlayerbotAI(Player* player)")
end = source.index("PlayerbotMgr* PlayerbotsMgr::GetPlayerbotMgr(", start)
method = source[start:end]
assert "if (!(sPlayerbotAIConfig.enabled) || !player)" in method
assert "return nullptr;" in method
assert "_playerbotsAIMap.find(player->GetGUID())" in method
assert "itr->second->IsBotAI()" in method
assert "OnPlayerCreatureKillCredit(Player* player, Creature* killed)" in hooks
assert "GetSession()->IsHeadless()" in hooks

local = pathlib.Path("src/StrongholdRaidParticipation.cpp").read_text()
assert "ControlClassificationSourceVerified" in local
assert "ParticipantControl::Unknown" in local
read = pathlib.Path("src/StrongholdStagingPlayerbotsRead.cpp").read_text()
assert "sPlayerbotsMgr.GetPlayerbotAI(player)" in read
assert "sPlayerbotAIConfig.enabled" in read
policy = pathlib.Path("src/StrongholdPlayerbotsRead.cpp").read_text()
assert "return ParticipantControl::VerifiedPlayerbot;" in policy
assert "return ParticipantControl::VerifiedHuman;" not in policy

print("PASS: pinned public Playerbots has a bot-AI lookup but NULL is ambiguous")
print("NOT VERIFIED: installed Playerbots fork; human status remains UNKNOWN without an audited positive test")
