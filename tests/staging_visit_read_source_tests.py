#!/usr/bin/env python3
"""The optional GM-self staging reader must stay disabled and SELECT-only."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
adapter=(root/"src/StrongholdStagingVisitAdapter.cpp").read_text()
ah=(root/"src/StrongholdStagingVisitAdapter.h").read_text()
diagnostic=(root/"src/StrongholdStagingDiagnostics.cpp").read_text()
policy=(root/"src/StrongholdVisitTicketRead.cpp").read_text()
ph=(root/"src/StrongholdVisitTicketRead.h").read_text()
conf=(root/"conf/mod_naxx_guild_strongholds.conf.dist").read_text()
workflow=(root/".github/workflows/upstream-compile.yml").read_text()
runner=(root/"tests/run-catalog-tests.sh").read_text()
for literal in (
    "#if defined(NAXX_GS_BUILD_STAGING_VISIT_READ)",
    "self->IsGameMaster()", "self->IsInWorld()",
    "self->GetGUID().GetCounter()",
    "CharacterDatabase.Query(",
    "SELECT player_guid, guild_id, guild_created_at, visit_nonce, ",
    "FROM naxx_gs_visit WHERE player_guid = {} LIMIT 1",
    "NaxxGuildStrongholds.StagingVisitRead.Enabled",
):
    assert literal in adapter, literal
assert adapter.count("CharacterDatabase.Query(")==1
assert "#error" in ah and "NAXX_GS_BUILD_STAGING_DIAGNOSTICS" in ah
assert "ReadStagingVisitTicket(self)" in diagnostic
assert "rbac::RBAC_PERM_COMMAND_DEBUG_INFO" in diagnostic
assert "VisitTicketStatusText(inspected.Status)" in diagnostic
assert "SessionKey" not in diagnostic and "ReturnPoint" not in diagnostic
assert "NaxxGuildStrongholds.StagingVisitRead.Enabled = 0" in conf
assert "-DNAXX_GS_BUILD_STAGING_VISIT_READ" in workflow
assert "Staging ticket query unexpectedly present in normal build" in workflow
assert "StrongholdVisitTicketRead.cpp" in runner
assert "AuthorisesClear = false" in ph
assert "AuthorisesTeleport = false" in ph
sql=adapter.split("CharacterDatabase.Query(",1)[1].split(");",1)[0]
for keyword in ("INSERT ", "UPDATE ", "DELETE ", "CREATE ", "DROP ",
                "ALTER ", "TRUNCATE ", "REPLACE "):
    assert keyword not in sql.upper(), keyword
for body in (adapter,policy,ph):
    for forbidden in ("CharacterDatabase.Execute(", "SaveToDB(", "TeleportTo(",
                      "SetPhaseMask(", "SummonCreature(", "SummonGameObject(",
                      "RewardQuest(", "DELETE FROM naxx_gs_visit"):
        assert forbidden not in body, forbidden
print("PASS: real staging visit status read is GM-self, opt-in and SELECT-only")
