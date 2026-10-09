#!/usr/bin/env python3
"""Do not let a synthetic drain signal trigger source/schema removal."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
header=(root/"src/StrongholdUninstallDrain.h").read_text()
cpp=(root/"src/StrongholdUninstallDrain.cpp").read_text()
runner=(root/"tests/run-catalog-tests.sh").read_text()
assert "MayRemoveRecoveryHandler = false" in header
assert "MayPurgeVisitTickets = false" in header
assert "MayModifyLiveDatabase = false" in header
assert "CandidateForManualOperatorReview" in cpp
assert "StrongholdUninstallDrain.cpp" in runner
assert "uninstall_drain_tests.cpp" in runner
for text in (header,cpp):
    for forbidden in ("DROP TABLE","DELETE FROM ","CharacterDatabase.",
                      "WorldDatabase.","LoginDatabase.","TeleportTo(",
                      "SaveToDB(", "SetPhaseMask(", "SummonCreature("):
        assert forbidden not in text, forbidden
print("PASS: offline uninstaller refuses SQL, teleport and removal authorization")
