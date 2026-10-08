#!/usr/bin/env python3
"""Source-only guard: Guild Steward may READ identity, not mutate AzerothCore.

This deliberately rejects a future developer accidentally adding gameplay
writes to the GM staging preview. Not a runtime security proof.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ADAPTER = (ROOT / "src/StrongholdStagingGuildAdapter.cpp").read_text(encoding="utf-8")
GOSSIP = (ROOT / "src/StrongholdStagingSteward.cpp").read_text(encoding="utf-8")
POLICY = (ROOT / "src/StrongholdGuildReadOnly.cpp").read_text(encoding="utf-8")
PROPERTY = (ROOT / "src/StrongholdStagingPropertyAdapter.cpp").read_text(encoding="utf-8")

REQUIRED = {
    "real Guild API membership": "guild->GetMember(player->GetGUID())",
    "real Guild ID": "guild->GetId()",
    "real Guild creation generation": "guild->GetCreatedDate()",
    "player's current Guild reference": "player->GetGuild()",
    "generation result consumed by preview": "ReadCurrentGuildIdentity(player)",
    "staging compile guard": "#if defined(NAXX_GS_BUILD_STAGING_STEWARD)",
}
COMBINED = ADAPTER + GOSSIP + POLICY + PROPERTY
for label, api in REQUIRED.items():
    assert api in COMBINED, f"BLOCKED: Missing {label}: {api}"

DANGEROUS = (
    "WorldDatabase.", "LoginDatabase.",
    "SetGuildId(", "SetRank(", "TeleportTo(", "SetPhaseMask(",
    "AddQuest(", "CompleteQuest(", "RewardQuest(", "ModifyMoney(",
    "DestroyItem(", "StoreNewItem(", "SaveToDB(", "Disband(",
    "ForceUpdateProgressionState(", "UpdateProgressionState(",
    "SummonCreature(", "SummonGameObject(", "CreateInstance(",
)
for name, content in (("Guild staging adapter", ADAPTER),
                      ("Steward gossip", GOSSIP),
                      ("Guild read-only policy", POLICY),
                      ("Staging property adapter", PROPERTY)):
    for api in DANGEROUS:
        assert api not in content, f"BLOCKED: {name} includes mutator token {api}"
    if name != "Staging property adapter":
        assert "CharacterDatabase." not in content, (
            f"BLOCKED: {name} directly accessed character database")

assert "evidence.PropertySnapshotLoaded = true;" not in GOSSIP, (
    "BLOCKED: Staging gossip must not fabricate property ownership")
assert "evidence.Ip.SourceContractVerified = true;" not in GOSSIP, (
    "BLOCKED: Staging gossip must not fabricate deployed IP proof")
assert "evidence.GuildGenerationVerified = true;" in GOSSIP, (
    "Staging evidence page no longer connects verified guild generation")
assert "guild.IsVerified()" in GOSSIP, (
    "Staging menu must fail closed when membership/creation is unverifiable")

assert PROPERTY.count("CharacterDatabase.Query(") == 1, (
    "BLOCKED: Property adapter may only use one SELECT-only query")
assert "SELECT guild_id, guild_created_at, lifecycle_state" in PROPERTY
assert "NaxxGuildStrongholds.StagingPropertyRead.Enabled" in PROPERTY
assert "if (!enabled || !verifiedGuild.GuildId || !verifiedGuild.CreatedAt)" in PROPERTY
assert "#if defined(NAXX_GS_BUILD_STAGING_PROPERTY_READ)" in PROPERTY
for token in ("INSERT ", "UPDATE ", "DELETE ", "CREATE ", "ALTER ", "DROP "):
    assert token not in PROPERTY.upper().split("CHARACTERDATABASE.QUERY(")[1].split(");")[0], (
        "BLOCKED: Staging DB statement changed to mutation")
print("PASS: Guild read adapters only use verified core state and one gated SELECT")
print("No actual property/IP source or housing side effects enabled")
